#include "../TestRegistry.h"

#include "dsp/DspUtils.h"
#include "dsp/Parameters.h"
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
