#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/Biquad.h"

namespace voiceguard
{

/** Input conditioning: smoothed input trim followed by an optional high-pass
    filter. RT-safe once prepared. */
class InputConditioning
{
public:
    InputConditioning() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;

    /** Set the trim target in dB; the gain is smoothed across blocks. */
    void setTrimDb (float db) noexcept;

    /** Set the HPF corner in Hz; 0 disables the filter. */
    void setHpfFrequency (float hz) noexcept;

    void process (juce::AudioBuffer<float>& buffer) noexcept;

    float trimDb() const noexcept { return trimDb_; }
    float hpfFrequency() const noexcept { return hpfHz_; }

private:
    double sampleRate_ = 48000.0;
    int numChannels_ = 2;

    Biquad hpf_;
    juce::SmoothedValue<float> trimGain_ { 1.0f };
    float trimDb_ = 0.0f;
    float hpfHz_ = 80.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputConditioning)
};

} // namespace voiceguard
