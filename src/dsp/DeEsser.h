#pragma once

#include "dsp/DynamicCutBank.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Transparent speech-oriented de-esser.

    A split/dynamic-band processor with smoothed attack/release. The maximum
    high-frequency cut is bounded so sibilants are controlled without
    obvious lisping, and the cut is released smoothly. */
class DeEsser
{
public:
    static constexpr int kNumBands = 2;
    static constexpr float kMaxCutDb = 8.0f;

    DeEsser() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setAmount (float amount01) noexcept;
    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

private:
    DynamicCutBank bank_;
    float amount_ = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeEsser)
};

} // namespace voiceguard
