#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/Biquad.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Conservative multiband noise reduction.

    A zero-latency bank of bandpass sidechains tracks the noise floor per band
    and drives a serial chain of dynamic peaking cuts. A 0 dB peaking filter
    is bit-exact unity, so zero amount is transparent; cuts compound serially,
    so the reduction is monotonic in amount. Detection uses the averaged
    stereo envelope and the same cut is applied to both channels, preserving
    image stability.

    Targets ~0-12 dB of steady-noise reduction before artefacts.
*/
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

    double sampleRate_ = 48000.0;
    int numChannels_ = 2;
    bool prepared_ = false;

    Biquad detectors_[kNumBands];
    Biquad eq_[kNumBands];
    float centreHz_[kNumBands] {};
    float env_[kNumBands] {};
    float floorDb_[kNumBands] {};
    float cutDb_[kNumBands] {};
    float cutSmoothed_[kNumBands] {};

    float amount_ = 0.5f;
    float adaptation_ = 0.5f;
    float currentReductionDb_ = 0.0f;

    float envAttack_ = 0.0f;
    float envRelease_ = 0.0f;
    float cutSmoothCoeff_ = 0.0f;
    float riseTau_ = 0.75f;
    int blocksSeen_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseReduction)
};

} // namespace voiceguard
