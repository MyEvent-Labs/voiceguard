#include "../TestRegistry.h"

#include "dsp/SpeechAnalyzer.h"

#include <cmath>
#include <random>
#include <vector>

using namespace voiceguard;

namespace
{
constexpr double kSr = 48000.0;

std::vector<float> makeSilence (int n)
{
    return std::vector<float> ((size_t) n, 0.0f);
}

std::vector<float> makeTone (double sr, int n, double freq, float amp)
{
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
        out[(size_t) i] = amp * (float) std::sin (juce::MathConstants<double>::twoPi * freq * i / sr);
    return out;
}

std::vector<float> makeSpeechLike (double sr, int n, float amp)
{
    std::vector<float> out ((size_t) n);
    const double formants[4] = { 150.0, 700.0, 1200.0, 2500.0 };
    const float weights[4] = { 0.40f, 0.30f, 0.25f, 0.15f };

    for (int i = 0; i < n; ++i)
    {
        const double t = i / sr;
        const double am = 0.5 + 0.5 * std::sin (juce::MathConstants<double>::twoPi * 4.0 * t);
        double s = 0.0;
        for (int f = 0; f < 4; ++f)
            s += weights[f] * std::sin (juce::MathConstants<double>::twoPi * formants[f] * t);

        out[(size_t) i] = amp * (float) (s * am);
    }

    return out;
}

std::vector<float> makeNoise (int n, float amp, uint32_t seed)
{
    std::mt19937 rng (seed);
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
        out[(size_t) i] = amp * dist (rng);
    return out;
}

/** High-passed noise, standing in for a sibilant burst. */
std::vector<float> makeHfNoise (double sr, int n, float amp, uint32_t seed)
{
    auto out = makeNoise (n, amp, seed);
    const float a = std::exp (-juce::MathConstants<float>::twoPi * 5000.0f / (float) sr);
    float lp = 0.0f;
    for (auto& s : out)
    {
        lp += (s - lp) * (1.0f - a);
        s = s - lp;
    }
    return out;
}

SpeechFeatures analyze (const std::vector<float>& signal, double sr, int blockSize)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (sr, blockSize, 1);

    const float* ptrs[1] = { signal.data() };
    for (int i = 0; i < (int) signal.size(); i += blockSize)
    {
        const int n = juce::jmin (blockSize, (int) signal.size() - i);
        ptrs[0] = signal.data() + i;
        analyzer.process (ptrs, 1, n);
    }

    return analyzer.features();
}

bool allFinite (const SpeechFeatures& f)
{
    return std::isfinite (f.speechActivity) && std::isfinite (f.noiseFloorDb)
           && std::isfinite (f.rmsShortDb) && std::isfinite (f.rmsLongDb)
           && std::isfinite (f.peakDb) && std::isfinite (f.crestFactorDb)
           && std::isfinite (f.snrDb) && std::isfinite (f.spectralCentroidHz)
           && std::isfinite (f.spectralSlopeDbPerOct) && std::isfinite (f.sibilanceConfidence)
           && std::isfinite (f.harshnessConfidence) && std::isfinite (f.tonality)
           && std::isfinite (f.spectralStability);
}
} // namespace

VG_TEST (analyzerSilenceIsQuietAndFinite)
{
    const auto f = analyze (makeSilence (24000), kSr, 512);
    VG_CHECK (allFinite (f));
    VG_CHECK (f.speechActivity < 0.1f);
    VG_CHECK (f.rmsShortDb <= -90.0f);
}

VG_TEST (analyzerDetectsSpeechLikeActivity)
{
    const auto f = analyze (makeSpeechLike (kSr, 60000, 0.3f), kSr, 512);
    VG_CHECK (allFinite (f));
    VG_CHECK (f.speechActivity > 0.5f);
    VG_CHECK (f.rmsShortDb > -40.0f);
}

VG_TEST (analyzerTracksSteadyNoiseFloor)
{
    const auto f = analyze (makeNoise (144000, 0.01f, 1), kSr, 512);
    VG_CHECK (allFinite (f));
    // 0.01 amplitude uniform noise is roughly -45 dBFS RMS.
    VG_CHECK (std::abs (f.noiseFloorDb - f.rmsShortDb) < 8.0f);
}

VG_TEST (analyzerSurvivesOneSampleBlocks)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (kSr, 1, 1);

    auto signal = makeNoise (4096, 0.05f, 7);
    const float* ptrs[1] = { signal.data() };

    for (int i = 0; i < (int) signal.size(); ++i)
    {
        ptrs[0] = signal.data() + i;
        analyzer.process (ptrs, 1, 1);
    }

    VG_CHECK (allFinite (analyzer.features()));
}

VG_TEST (analyzerTonalityDistinguishesToneFromNoise)
{
    const auto tone = analyze (makeTone (kSr, 48000, 500.0, 0.3f), kSr, 512);
    const auto noise = analyze (makeNoise (48000, 0.05f, 3), kSr, 512);

    VG_CHECK (allFinite (tone));
    VG_CHECK (allFinite (noise));
    VG_CHECK (tone.tonality > 0.5f);
    VG_CHECK (tone.tonality > noise.tonality + 0.15f);
}

VG_TEST (analyzerSibilanceAndHarshnessRespond)
{
    const auto sibilant = analyze (makeHfNoise (kSr, 48000, 0.08f, 11), kSr, 512);
    const auto lowTone = analyze (makeTone (kSr, 48000, 150.0, 0.3f), kSr, 512);
    const auto brightTone = analyze (makeTone (kSr, 48000, 4000.0, 0.3f), kSr, 512);

    VG_CHECK (allFinite (sibilant));
    VG_CHECK (allFinite (lowTone));
    VG_CHECK (allFinite (brightTone));

    VG_CHECK (sibilant.sibilanceConfidence > lowTone.sibilanceConfidence);
    VG_CHECK (brightTone.harshnessConfidence > lowTone.harshnessConfidence);
}
