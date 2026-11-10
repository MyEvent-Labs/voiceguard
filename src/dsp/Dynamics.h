#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Gentle, open adaptive compression with a soft knee.

    Detector uses the shared short-term RMS; the same gain is applied to both
    channels to keep the stereo image stable.

    Threading: audio-thread only. */
class Dynamics
{
public:
    Dynamics() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;

    void setParameters (float thresholdDb, float ratio, float attackMs,
                        float releaseMs, float kneeDb) noexcept;

    void process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept;

    float gainReductionDb() const noexcept { return gainReductionDb_; }

private:
    double sampleRate_ = 48000.0;
    int numChannels_ = 2;

    float thresholdDb_ = -18.0f;
    float ratio_ = 2.0f;
    float attackMs_ = 20.0f;
    float releaseMs_ = 250.0f;
    float kneeDb_ = 12.0f;
    float gainReductionDb_ = 0.0f;

    juce::SmoothedValue<float> gain_ { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dynamics)
};

} // namespace voiceguard
