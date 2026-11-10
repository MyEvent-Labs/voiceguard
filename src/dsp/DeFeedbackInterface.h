#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace voiceguard
{

/** Clean seam for the MyEvent Labs De-Feedback engine.

    v0.0.1 ships a null implementation and keeps integration off by default.
    A future version can inject a validated engine without touching VoiceGuard
    core. Implementations must be RT-safe in process(). */
class IDeFeedbackEngine
{
public:
    virtual ~IDeFeedbackEngine() = default;

    virtual void prepare (double sampleRate, int maxBlockSize, int numChannels) = 0;
    virtual void reset() = 0;
    virtual void setEnabled (bool shouldBeEnabled) = 0;
    virtual void process (juce::AudioBuffer<float>& buffer) = 0;
};

/** No-op implementation used by v0.0.1. */
class NullDeFeedbackEngine final : public IDeFeedbackEngine
{
public:
    void prepare (double, int, int) override {}
    void reset() override {}
    void setEnabled (bool) override {}
    void process (juce::AudioBuffer<float>&) override {}
};

} // namespace voiceguard
