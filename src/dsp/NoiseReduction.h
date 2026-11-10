#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/DynamicCutBank.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Conservative multiband noise reduction.

    Uses the shared DynamicCutBank: bandpass sidechains track a per-band noise
    floor and drive serial dynamic peaking cuts. A 0 dB peaking filter is
    bit-exact unity, so zero amount is transparent; cuts compound serially, so
    reduction is monotonic in amount. Detection uses the averaged stereo
    envelope and applies the same cut to both channels, preserving image
    stability.

    Threading: setters and process() are audio-thread only. */
class NoiseReduction
{
public:
    static constexpr int kNumBands = 10;
    static constexpr float kMaxReductionDb = 12.0f;

    NoiseReduction() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;

    void setAmount (float amount01) noexcept;
    void setAdaptation (float adaptation01) noexcept;

    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    /** Positive dB of steady reduction currently applied (for the UI). */
    float currentReductionDb() const noexcept { return currentReductionDb_; }

private:
    void updateGains (const SpeechFeatures& features, float blockSeconds) noexcept;

    DynamicCutBank bank_;
    float floorDb_[kNumBands] {};
    float cutDb_[kNumBands] {};

    float amount_ = 0.5f;
    float adaptation_ = 0.5f;
    float currentReductionDb_ = 0.0f;
    double sampleRate_ = 48000.0;
    float riseTau_ = 0.75f;
    int blocksSeen_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseReduction)
};

} // namespace voiceguard
