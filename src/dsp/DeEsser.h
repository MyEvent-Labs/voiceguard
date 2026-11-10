#pragma once

#include "dsp/DynamicCutBank.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Transparent speech-oriented de-esser.

    A three-band detector compares the 5.5-8.5 kHz sibilance region against a
    2.5 kHz mid reference. Sibilance is only cut when the high band approaches
    the mid band, gated by speech activity, with a bounded 8 dB maximum and
    smoothed application so sibilants are controlled without obvious lisping.

    Threading: setters and process() are audio-thread only. */
class DeEsser
{
public:
    static constexpr int kNumBands = 3;
    static constexpr float kMaxCutDb = 8.0f;

    DeEsser() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setAmount (float amount01) noexcept;
    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    float currentCutDb() const noexcept { return bank_.totalCutDb(); }

private:
    DynamicCutBank bank_;
    float amount_ = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeEsser)
};

} // namespace voiceguard
