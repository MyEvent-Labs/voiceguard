#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/Biquad.h"

namespace voiceguard
{

/** Reusable band bank for program-dependent dynamic processing.

    Each band owns an analysis band-pass (detector) and a peaking filter. Cuts
    are supplied by the owning module per block, smoothed, then applied as a
    serial chain so a 0 dB cut is exactly unity and cuts compound
    monotonically. Detection uses the averaged stereo signal. */
class DynamicCutBank
{
public:
    static constexpr int kMaxBands = 12;

    DynamicCutBank() = default;

    void configure (double sampleRate, int numChannels,
                    const float* centresHz, const float* qs, int count);

    void reset() noexcept;

    /** Refresh band envelopes from the averaged dry signal. */
    void detect (const juce::AudioBuffer<float>& buffer) noexcept;

    float envelopeDb (int band) const noexcept;

    void setCutDb (int band, float cutDb) noexcept;

    /** Apply the current (smoothed) cuts to the buffer. */
    void apply (juce::AudioBuffer<float>& buffer) noexcept;

    int numBands() const noexcept { return numBands_; }

    /** Sum of the smoothed cuts currently applied (positive dB). */
    float totalCutDb() const noexcept
    {
        float sum = 0.0f;
        for (int band = 0; band < numBands_; ++band)
            sum += cutSmoothed_[band];
        return sum;
    }

private:
    double sampleRate_ = 48000.0;
    int numChannels_ = 2;
    int numBands_ = 0;

    Biquad detectors_[kMaxBands];
    Biquad eq_[kMaxBands];
    float centreHz_[kMaxBands] {};
    float q_[kMaxBands] {};
    float env_[kMaxBands] {};
    float cutTarget_[kMaxBands] {};
    float cutSmoothed_[kMaxBands] {};

    float envAttack_ = 0.0f;
    float envRelease_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicCutBank)
};

} // namespace voiceguard
