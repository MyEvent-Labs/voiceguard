#pragma once

#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Conservative AUTO starting settings derived from a short listening window. */
struct AutoSuggestions
{
    float cleanAmount = 0.5f;
    float eqStrength = 0.5f;
    float deEsserAmount = 0.5f;
    float harshnessAmount = 0.5f;
    float levelTargetDb = -18.0f;
    float protectAmount = 0.5f;
};

/** AUTO mode state machine.

    Listens for a bounded window (>= 5 s of representative speech), estimates
    noise floor, speech level, harshness and sibilance, then settles on
    conservative starting settings. It never rewrites the sound aggressively
    and does not act until enough speech has been seen.

    Threading: audio-thread only. */
class AutoController
{
public:
    enum class State { idle = 0, listening = 1, ready = 2 };

    AutoController() = default;

    void prepare (double sampleRate);
    void reset() noexcept;
    void start() noexcept;
    void cancel() noexcept;
    void process (const SpeechFeatures& features, float blockSeconds) noexcept;

    State state() const noexcept { return state_; }
    float progress() const noexcept;
    const AutoSuggestions& suggestions() const noexcept { return suggestions_; }

private:
    void finalise() noexcept;

    State state_ = State::idle;
    double sampleRate_ = 48000.0;

    float elapsed_ = 0.0f;
    float speechTime_ = 0.0f;
    float noiseFloorDb_ = -100.0f;
    float speechLevelDb_ = -100.0f;
    float maxHarshness_ = 0.0f;
    float maxSibilance_ = 0.0f;
    int frames_ = 0;

    AutoSuggestions suggestions_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoController)
};

} // namespace voiceguard
