#include "../TestRegistry.h"

#include "dsp/DynamicEq.h"
#include "dsp/DspUtils.h"
#include "dsp/SpeechAnalyzer.h"
#include "fixtures/Signals.h"

#include <algorithm>
#include <vector>

using namespace voiceguard;
using namespace vgfixture;

namespace
{
constexpr double kSr = 48000.0;

std::vector<float> runEq (const std::vector<float>& in, int block, float strength)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (kSr, block, 1);

    DynamicEq eq;
    eq.prepare (kSr, block, 1);
    eq.setStrength (strength);

    juce::AudioBuffer<float> buffer (1, (int) in.size());
    std::copy (in.begin(), in.end(), buffer.getWritePointer (0));

    for (int i = 0; i < (int) in.size(); i += block)
    {
        const int n = juce::jmin (block, (int) in.size() - i);
        const float* ptrs[1] = { buffer.getReadPointer (0) + i };
        analyzer.process (ptrs, 1, n);

        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 1, i, n);
        eq.process (chunk, analyzer.features());
    }

    return std::vector<float> (buffer.getReadPointer (0),
                               buffer.getReadPointer (0) + in.size());
}
} // namespace

VG_TEST (dynamicEqLeavesNeutralSpeechNearlyUntouched)
{
    const auto signal = speechLike (kSr, 96000, 0.3f);
    const auto out = runEq (signal, 512, 1.0f);

    VG_CHECK (std::abs (gainToDb (rms (out)) - gainToDb (rms (signal))) < 1.5f);
}

VG_TEST (dynamicEqCutsProgramDependentBoxiness)
{
    auto neutral = speechLike (kSr, 96000, 0.3f);
    auto colored = speechLike (kSr, 96000, 0.3f, 550.0, 0.4f);

    const auto neutralOut = runEq (neutral, 512, 1.0f);
    const auto coloredOut = runEq (colored, 512, 1.0f);

    const float neutralReduction = gainToDb (rms (neutral)) - gainToDb (rms (neutralOut));
    const float coloredReduction = gainToDb (rms (colored)) - gainToDb (rms (coloredOut));

    VG_CHECK (coloredReduction > 0.3f);
    VG_CHECK (coloredReduction > neutralReduction + 0.2f);
}

VG_TEST (dynamicEqPreservesStereoImage)
{
    const auto mono = speechLike (kSr, 48000, 0.3f, 550.0, 0.3f);

    juce::AudioBuffer<float> buffer (2, (int) mono.size());
    std::copy (mono.begin(), mono.end(), buffer.getWritePointer (0));
    std::copy (mono.begin(), mono.end(), buffer.getWritePointer (1));

    SpeechAnalyzer analyzer;
    analyzer.prepare (kSr, 512, 2);

    DynamicEq eq;
    eq.prepare (kSr, 512, 2);
    eq.setStrength (1.0f);

    for (int i = 0; i < (int) mono.size(); i += 512)
    {
        const int n = juce::jmin (512, (int) mono.size() - i);
        const float* ptrs[2] = { buffer.getReadPointer (0) + i, buffer.getReadPointer (1) + i };
        analyzer.process (ptrs, 2, n);

        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 2, i, n);
        eq.process (chunk, analyzer.features());
    }

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        VG_CHECK_CLOSE (buffer.getSample (0, i), buffer.getSample (1, i), 1.0e-5);
}
