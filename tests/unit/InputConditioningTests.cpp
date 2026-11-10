#include "../TestRegistry.h"

#include "dsp/InputConditioning.h"

#include <cmath>

using namespace voiceguard;

namespace
{
constexpr double kSr = 48000.0;

juce::AudioBuffer<float> makeSine (double sr, int n, double freq, float amp)
{
    juce::AudioBuffer<float> buffer (1, n);
    for (int i = 0; i < n; ++i)
        buffer.setSample (0, i, amp * (float) std::sin (juce::MathConstants<double>::twoPi * freq * i / sr));
    return buffer;
}

float rmsOfLastBlock (const juce::AudioBuffer<float>& buffer, int start, int n)
{
    double sum = 0.0;
    for (int i = start; i < start + n; ++i)
        sum += (double) buffer.getSample (0, i) * buffer.getSample (0, i);
    return (float) std::sqrt (sum / n);
}

/** Process a signal in blocks, returning the final buffer. */
juce::AudioBuffer<float> run (InputConditioning& ic, juce::AudioBuffer<float> buffer, int blockSize)
{
    for (int i = 0; i < buffer.getNumSamples(); i += blockSize)
    {
        const int n = juce::jmin (blockSize, buffer.getNumSamples() - i);
        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 1, i, n);
        ic.process (chunk);
    }
    return buffer;
}
} // namespace

VG_TEST (inputTrimBoostsBySixDb)
{
    InputConditioning ic;
    ic.prepare (kSr, 512, 1);
    ic.setHpfFrequency (0.0f);
    ic.setTrimDb (6.0f);

    auto signal = makeSine (kSr, 24000, 1000.0, 0.1f);
    const float inRms = rmsOfLastBlock (signal, 24000 - 512, 512);
    signal = run (ic, signal, 512);
    const float outRms = rmsOfLastBlock (signal, 24000 - 512, 512);

    VG_CHECK_CLOSE (outRms / juce::jmax (1.0e-6f, inRms), 2.0f, 0.05f);
}

VG_TEST (inputHpfRejectsDc)
{
    InputConditioning ic;
    ic.prepare (kSr, 512, 1);
    ic.setHpfFrequency (80.0f);

    juce::AudioBuffer<float> dc (1, 48000);
    dc.clear();
    for (int i = 0; i < dc.getNumSamples(); ++i)
        dc.setSample (0, i, 1.0f);

    dc = run (ic, dc, 512);

    VG_CHECK (std::abs (dc.getSample (0, dc.getNumSamples() - 1)) < 1.0e-3f);
}

VG_TEST (inputHpfOffPassesDc)
{
    InputConditioning ic;
    ic.prepare (kSr, 512, 1);
    ic.setHpfFrequency (0.0f);

    juce::AudioBuffer<float> dc (1, 48000);
    for (int i = 0; i < dc.getNumSamples(); ++i)
        dc.setSample (0, i, 1.0f);

    dc = run (ic, dc, 512);

    VG_CHECK_CLOSE (dc.getSample (0, dc.getNumSamples() - 1), 1.0f, 1.0e-3);
}

VG_TEST (inputHpfAttenuatesRumble)
{
    InputConditioning ic;
    ic.prepare (kSr, 512, 1);
    ic.setHpfFrequency (80.0f);
    ic.setTrimDb (0.0f);

    auto signal = makeSine (kSr, 48000, 20.0, 0.5f);
    const float inRms = rmsOfLastBlock (signal, 48000 - 512, 512);
    signal = run (ic, signal, 512);
    const float outRms = rmsOfLastBlock (signal, 48000 - 512, 512);

    VG_CHECK (outRms < inRms * 0.6f);
}
