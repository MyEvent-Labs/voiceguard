#include "../TestRegistry.h"

#include "dsp/NoiseReduction.h"
#include "dsp/SpeechAnalyzer.h"
#include "dsp/DspUtils.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

using namespace voiceguard;

namespace
{
constexpr double kSr = 48000.0;

std::vector<float> makeNoise (int n, float amp, uint32_t seed)
{
    std::mt19937 rng (seed);
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
        out[(size_t) i] = amp * dist (rng);
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

float rms (const std::vector<float>& v, int start, int n)
{
    double sum = 0.0;
    for (int i = start; i < start + n; ++i)
        sum += (double) v[(size_t) i] * v[(size_t) i];
    return (float) std::sqrt (sum / n);
}

struct NrResult
{
    std::vector<float> out;
    float reductionDb = 0.0f;
};

NrResult runNr (const std::vector<float>& in, double sr, int block, float amount, float adaptation)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (sr, block, 1);

    NoiseReduction nr;
    nr.prepare (sr, block, 1);
    nr.setAmount (amount);
    nr.setAdaptation (adaptation);

    juce::AudioBuffer<float> buffer (1, (int) in.size());
    std::copy (in.begin(), in.end(), buffer.getWritePointer (0));

    for (int i = 0; i < (int) in.size(); i += block)
    {
        const int n = juce::jmin (block, (int) in.size() - i);
        const float* ptrs[1] = { buffer.getReadPointer (0) + i };
        analyzer.process (ptrs, 1, n);

        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 1, i, n);
        nr.process (chunk, analyzer.features());
    }

    std::vector<float> out ((size_t) in.size());
    std::copy (buffer.getReadPointer (0), buffer.getReadPointer (0) + in.size(), out.begin());

    NrResult result;
    result.out = std::move (out);

    const int window = juce::jmin (48000, (int) in.size());
    const float inRms = rms (in, (int) in.size() - window, window);
    const float outRms = rms (result.out, (int) result.out.size() - window, window);
    result.reductionDb = gainToDb (inRms) - gainToDb (outRms);
    return result;
}
} // namespace

VG_TEST (noiseReductionReducesSteadyNoiseWithinRange)
{
    const auto signal = makeNoise (144000, 0.05f, 21);
    const auto result = runNr (signal, kSr, 512, 0.5f, 0.5f);

    VG_CHECK (result.reductionDb >= 3.0f);
    VG_CHECK (result.reductionDb <= 12.0f);
}

VG_TEST (noiseReductionIsTransparentAtZeroAmount)
{
    const auto signal = makeNoise (48000, 0.05f, 5);
    const auto result = runNr (signal, kSr, 512, 0.0f, 0.5f);

    for (size_t i = 0; i < signal.size(); ++i)
        VG_CHECK_CLOSE (result.out[i], signal[i], 1.0e-6);
}

VG_TEST (noiseReductionPreservesSpeechLikeSignal)
{
    const auto signal = makeSpeechLike (kSr, 144000, 0.3f);
    const auto result = runNr (signal, kSr, 512, 0.5f, 0.5f);

    VG_CHECK (result.reductionDb < 1.5f);
}

VG_TEST (noiseReductionReductionIsBoundedAtMaxAmount)
{
    const auto signal = makeNoise (192000, 0.05f, 33);
    const auto result = runNr (signal, kSr, 512, 1.0f, 0.5f);

    VG_CHECK (result.reductionDb >= 6.0f);
    VG_CHECK (result.reductionDb <= 13.0f);
}
