#include "../TestRegistry.h"

#include "dsp/Biquad.h"
#include "dsp/DspUtils.h"
#include "dsp/Parameters.h"
#include "dsp/SpeechAnalyzer.h"
#include "dsp/VoiceGuardEngine.h"
#include "fixtures/Signals.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace voiceguard;
using namespace vgfixture;

// Objective scorecard for the full chain. Prints metrics so the algorithm can
// be tuned against numbers, then asserts the guarantees.

namespace
{
constexpr double kSr = 48000.0;
constexpr int kBlock = 256;

std::vector<float> runEngine (const std::vector<float>& in, const params::Parameters& p, int channels = 1)
{
    VoiceGuardEngine engine;
    engine.prepare (kSr, kBlock, channels);
    engine.setParameters (p);

    juce::AudioBuffer<float> buffer (channels, (int) in.size());
    std::copy (in.begin(), in.end(), buffer.getWritePointer (0));

    if (channels > 1)
        std::copy (in.begin(), in.end(), buffer.getWritePointer (1));

    for (int i = 0; i < (int) in.size(); i += kBlock)
    {
        const int n = juce::jmin (kBlock, (int) in.size() - i);
        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), channels, i, n);
        engine.process (chunk);
    }

    return std::vector<float> (buffer.getReadPointer (0), buffer.getReadPointer (0) + in.size());
}

float peakOf (const std::vector<float>& v)
{
    float peak = 0.0f;
    for (float s : v)
        peak = juce::jmax (peak, std::abs (s));
    return peak;
}

/** RMS in dB of the last n samples (the settled part). */
float tailLevelDb (const std::vector<float>& v, int n)
{
    n = juce::jmin (n, (int) v.size());

    if (n <= 0)
        return -100.0f;

    double sum = 0.0;
    for (int i = (int) v.size() - n; i < (int) v.size(); ++i)
        sum += (double) v[(size_t) i] * (double) v[(size_t) i];

    return gainToDb ((float) std::sqrt (sum / n));
}

/** Std-dev in dB of the settling part (last second) of each step. This
    measures presenter-distance stability, not syllable variation. */
float segmentSpreadDb (const std::vector<float>& v, int segmentSamples)
{
    const int window = (int) (kSr * 1.0);
    std::vector<float> means;

    for (int s = 0; s + segmentSamples <= (int) v.size(); s += segmentSamples)
    {
        const int start = juce::jmax (s, s + segmentSamples - window);
        double sum = 0.0;
        int count = 0;

        for (int i = start; i < s + segmentSamples; ++i)
        {
            sum += (double) v[(size_t) i] * (double) v[(size_t) i];
            ++count;
        }

        const float db = gainToDb ((float) std::sqrt (sum / juce::jmax (1, count)));

        if (db > -55.0f)
            means.push_back (db);
    }

    if (means.size() < 2)
        return 0.0f;

    double mean = 0.0;
    for (float l : means)
        mean += l;
    mean /= (double) means.size();

    double var = 0.0;
    for (float l : means)
        var += (l - mean) * (l - mean);

    return (float) std::sqrt (var / (double) means.size());
}

/** Speech-like signal whose level steps, as a presenter moves on/off mic. */
std::vector<float> steppedSpeech()
{
    const int segment = (int) (kSr * 2.0);
    const float gains[4] = { 1.0f, 0.4f, 1.0f, 0.25f };
    std::vector<float> out;

    for (int s = 0; s < 4; ++s)
    {
        auto seg = speechLike (kSr, segment, 0.3f * gains[s]);
        out.insert (out.end(), seg.begin(), seg.end());
    }

    return out;
}
/** Speech then room tone, twice: does the noise drop in the pauses without
    the speech being damaged? */
std::vector<float> speechThenNoise()
{
    auto speech = speechLike (kSr, 96000, 0.3f);
    auto room = noise (192000, 0.04f, 9);
    std::vector<float> out;

    for (int rep = 0; rep < 2; ++rep)
    {
        out.insert (out.end(), speech.begin(), speech.end());
        out.insert (out.end(), room.begin(), room.end());
    }

    return out;
}

float regionLevelDb (const std::vector<float>& v, int start, int end)
{
    end = juce::jmin (end, (int) v.size());
    start = juce::jlimit (0, (int) v.size(), start);

    if (end <= start)
        return -100.0f;

    double sum = 0.0;
    for (int i = start; i < end; ++i)
        sum += (double) v[(size_t) i] * (double) v[(size_t) i];

    return gainToDb ((float) std::sqrt (sum / (end - start)));
}

/** Tonality (1 - spectral flatness) of a whole signal. Musical noise from
    spectral processing raises this on noise-only input. */
float tonalityOf (const std::vector<float>& v)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (kSr, 256, 1);
    const float* ptrs[1] = { nullptr };

    for (int i = 0; i < (int) v.size(); i += 256)
    {
        const int n = juce::jmin (256, (int) v.size() - i);
        ptrs[0] = v.data() + i;
        analyzer.process (ptrs, 1, n);
    }

    return analyzer.features().tonality;
}

} // namespace

VG_TEST (engineReducesSteadyNoise)
{
    auto noiseSignal = noise (240000, 0.05f, 5);
    auto out = runEngine (noiseSignal, params::Parameters {});
    const float reduction = gainToDb (rms (noiseSignal)) - gainToDb (rms (out));
    std::cout << "[score] steadyNoiseReductionDb = " << reduction << "\n";
    VG_CHECK (reduction > 2.0f);
    VG_CHECK (reduction < 13.0f);
}

VG_TEST (enginePreservesCleanSpeechLevel)
{
    auto speech = speechLike (kSr, 240000, 0.3f);
    auto out = runEngine (speech, params::Parameters {});
    const float change = gainToDb (rms (out)) - gainToDb (rms (speech));
    std::cout << "[score] cleanSpeechLevelChangeDb = " << change << "\n";
    VG_CHECK (std::abs (change) < 1.5f);
}

VG_TEST (engineStabilisesLevelSpread)
{
    auto stepped = steppedSpeech();
    auto p = params::Parameters {};
    p.levelRangeDb = 6.0f;
    p.macroLevel = 1.0f;
    auto out = runEngine (stepped, p);

    const int segment = (int) (kSr * 2.0);
    const float spreadIn = segmentSpreadDb (stepped, segment);
    const float spreadOut = segmentSpreadDb (out, segment);
    std::cout << "[score] levelSpreadInDb = " << spreadIn
              << "  levelSpreadOutDb = " << spreadOut << "\n";
    VG_CHECK (spreadOut <= spreadIn);
}

VG_TEST (engineRidesQuietSpeechUp)
{
    auto quiet = speechLike (kSr, 240000, 0.1f);
    auto p = params::Parameters {};
    p.levelTargetDb = -18.0f;
    p.levelRangeDb = 6.0f;
    p.macroLevel = 1.0f;
    auto out = runEngine (quiet, p);

    const float inLevel = tailLevelDb (quiet, 48000);
    const float outLevel = tailLevelDb (out, 48000);
    std::cout << "[score] quietSpeechInDb = " << inLevel << "  outDb = " << outLevel << "\n";

    VG_CHECK (outLevel > inLevel + 2.0f);      // rode the quiet presenter up
    VG_CHECK (outLevel <= -17.0f);             // but never past the target
}

VG_TEST (engineReducesNoiseDuringPauses)
{
    auto signal = speechThenNoise();
    auto out = runEngine (signal, params::Parameters {});

    const int s = 96000;
    const int n = 192000;
    const auto measure = [&] (const std::vector<float>& v)
    {
        return 0.5f * (regionLevelDb (v, s + n - 48000, s + n)
                       + regionLevelDb (v, 2 * s + 2 * n - 48000, 2 * s + 2 * n));
    };

    const float inNoise = measure (signal);
    const float outNoise = measure (out);
    const float reduction = inNoise - outNoise;
    std::cout << "[score] noiseInPausesReductionDb = " << reduction << "\n";

    // Isolation: run the same signal with the rider range at zero to see what
    // the noise reduction alone achieves in the pauses.
    auto pNoRider = params::Parameters {};
    pNoRider.levelRangeDb = 0.0f;
    auto outNoRider = runEngine (signal, pNoRider);
    std::cout << "[score] noiseInPausesReductionNoRiderDb = " << (inNoise - measure (outNoRider)) << "\n";

    VG_CHECK (reduction > 3.0f);
}

VG_TEST (engineReducesSibilance)
{
    auto signal = speechLike (kSr, 96000, 0.3f);
    addInto (signal, hfNoise (kSr, 96000, 0.2f, 41));
    auto out = runEngine (signal, params::Parameters {});

    const auto bandLevelDb = [] (const std::vector<float>& v)
    {
        Biquad bp;
        bp.setBandpass (kSr, 5500.0, 1.0);
        auto copy = v;
        bp.processBlock (copy.data(), (int) copy.size(), 0);
        return tailLevelDb (copy, 24000);
    };

    const float reduction = bandLevelDb (signal) - bandLevelDb (out);
    std::cout << "[score] sibilanceReductionDb = " << reduction << "\n";
    VG_CHECK (reduction > 1.0f);
    VG_CHECK (reduction < 8.5f);
}

VG_TEST (engineNoiseReductionDoesNotAddTonality)
{
    auto noiseSig = noise (240000, 0.05f, 5);
    auto out = runEngine (noiseSig, params::Parameters {});

    const float tin = tonalityOf (noiseSig);
    const float tout = tonalityOf (out);
    std::cout << "[score] tonalityIn = " << tin << "  tonalityOut = " << tout << "\n";
    VG_CHECK (tout < tin + 0.25f);
}

VG_TEST (engineDynamicsIsTransparent)
{
    auto speech = speechLike (kSr, 240000, 0.3f);
    auto p = params::Parameters {};
    p.levelRangeDb = 0.0f;
    auto out = runEngine (speech, p);

    const float base = tailLevelDb (speech, 48000);
    const float change = tailLevelDb (out, 48000) - base;
    std::cout << "[score] dynamicsLevelChangeDb = " << change << "\n";

    const auto isolate = [&] (params::Parameters q)
    {
        return tailLevelDb (runEngine (speech, q), 48000) - base;
    };

    auto pNoNr = p; pNoNr.cleanAmount = 0.0f;
    auto pNoEq = p; pNoEq.eqStrength = 0.0f;
    auto pNoDyn = p; pNoDyn.dynRatio = 1.0f;
    auto pNoDeEss = p; pNoDeEss.deEsserAmount = 0.0f;
    auto pNoHarsh = p; pNoHarsh.harshnessAmount = 0.0f;
    std::cout << "[score] isolate noNr=" << isolate (pNoNr) << " noEq=" << isolate (pNoEq)
              << " noDyn=" << isolate (pNoDyn) << " noDeEss=" << isolate (pNoDeEss)
              << " noHarsh=" << isolate (pNoHarsh) << "\n";

    VG_CHECK (std::abs (change) < 1.5f);
}

VG_TEST (engineSilenceStaysSilent)
{
    auto silent = silence (96000);
    auto out = runEngine (silent, params::Parameters {});
    std::cout << "[score] silencePeakDb = " << gainToDb (peakOf (out)) << "\n";
    VG_CHECK (peakOf (out) < 1.0e-6f);
}

VG_TEST (engineOutputIsBounded)
{
    auto hot = tone (kSr, 48000, 300.0, 0.98f);
    auto p = params::Parameters {};
    p.outputTrimDb = 12.0f;
    auto out = runEngine (hot, p);
    std::cout << "[score] hotOutputPeakDb = " << gainToDb (peakOf (out)) << "\n";
    VG_CHECK (peakOf (out) <= 1.0f);
}
