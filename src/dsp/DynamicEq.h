#pragma once

#include "dsp/DynamicCutBank.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Speech-aware dynamic EQ.

    Six program-dependent regions (rumble/mud, boxiness, nasality,
    intelligibility, harshness, sibilance) are compared against the live
    intelligibility band with expected speech tilt. Only excess energy is cut,
    only while speech is present, so neutral speech is left intact and speaker
    identity is preserved. These are search regions, not fixed curves. */
class DynamicEq
{
public:
    static constexpr int kNumBands = 6;

    DynamicEq() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setStrength (float strength01) noexcept;
    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    float currentCutDb() const noexcept { return bank_.totalCutDb(); }

private:
    DynamicCutBank bank_;
    float strength_ = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicEq)
};

} // namespace voiceguard
