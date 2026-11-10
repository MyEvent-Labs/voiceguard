#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace voiceguard
{

/** Lightweight, lookahead-free output peak safety.

    A fast peak gain keeps the signal at the ceiling, and a final hard clamp
    guarantees the output is bounded even for a single-sample overshoot. Never
    applies positive gain.

    Threading: audio-thread only. */
class OutputSafety
{
public:
    OutputSafety() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setCeilingDb (float db) noexcept;
    void process (juce::AudioBuffer<float>& buffer) noexcept;

    float gainReductionDb() const noexcept { return gainReductionDb_; }
    bool isActive() const noexcept { return gainReductionDb_ > 0.1f; }

private:
    double sampleRate_ = 48000.0;
    int numChannels_ = 2;
    float ceilingLin_ = 0.891251f; // -1 dBFS
    float gainReductionDb_ = 0.0f;

    juce::SmoothedValue<float> gain_ { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputSafety)
};

} // namespace voiceguard
