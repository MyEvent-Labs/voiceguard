#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Slow, transparent level rider.

    Compensates for presenters and mic distance with a bounded slow gain ride,
    gated by speech activity so silence and room tone are never boosted. A fast
    peak path is separate from the slow ride.

    Threading: audio-thread only. */
class LevelRider
{
public:
    LevelRider() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;

    void setTargetDb (float db) noexcept;
    void setRangeDb (float db) noexcept;
    void setSpeed (float speed01) noexcept;

    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    /** Current ride gain in dB (signed). */
    float rideGainDb() const noexcept { return rideGainDb_; }

private:
    double sampleRate_ = 48000.0;
    int numChannels_ = 2;

    float targetDb_ = -18.0f;
    float rangeDb_ = 6.0f;
    float speed_ = 0.5f;
    float rideGainDb_ = 0.0f;

    juce::SmoothedValue<float> gain_ { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelRider)
};

} // namespace voiceguard
