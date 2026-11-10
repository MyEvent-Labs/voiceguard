#pragma once

#include "dsp/DynamicCutBank.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Dynamic harshness control.

    Catches microphone/PA resonance, shouting, bright lavaliers and aggressive
    consonants in the 2.5-6 kHz region without permanently dulling speech. */
class HarshnessControl
{
public:
    static constexpr int kNumBands = 3;

    HarshnessControl() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setAmount (float amount01) noexcept;
    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    float currentCutDb() const noexcept { return bank_.totalCutDb(); }

private:
    DynamicCutBank bank_;
    float amount_ = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarshnessControl)
};

} // namespace voiceguard
