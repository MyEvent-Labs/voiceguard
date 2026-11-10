#include "dsp/AutoController.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

namespace
{
constexpr float kMinimumSpeechSeconds = 5.0f;
}

void AutoController::prepare (double sampleRate)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void AutoController::reset() noexcept
{
    state_ = State::idle;
    elapsed_ = 0.0f;
    speechTime_ = 0.0f;
    noiseFloorDb_ = -100.0f;
    speechLevelDb_ = -100.0f;
    maxHarshness_ = 0.0f;
    maxSibilance_ = 0.0f;
    frames_ = 0;
    suggestions_ = AutoSuggestions {};
}

void AutoController::start() noexcept
{
    reset();
    state_ = State::listening;
}

void AutoController::cancel() noexcept
{
    reset();
}

float AutoController::progress() const noexcept
{
    if (state_ == State::ready)
        return 1.0f;

    return juce::jlimit (0.0f, 1.0f, speechTime_ / kMinimumSpeechSeconds);
}

void AutoController::process (const SpeechFeatures& features, float blockSeconds) noexcept
{
    if (state_ != State::listening)
        return;

    elapsed_ += blockSeconds;

    if (features.speechActivity > 0.3f)
    {
        speechTime_ += blockSeconds;
        speechLevelDb_ = frames_ == 0 ? features.rmsShortDb
                                      : juce::jmax (speechLevelDb_, features.rmsShortDb);
        noiseFloorDb_ = frames_ == 0 ? features.noiseFloorDb
                                     : juce::jmin (noiseFloorDb_, features.noiseFloorDb);
        maxHarshness_ = juce::jmax (maxHarshness_, features.harshnessConfidence);
        maxSibilance_ = juce::jmax (maxSibilance_, features.sibilanceConfidence);
        ++frames_;
    }

    if (speechTime_ >= kMinimumSpeechSeconds && elapsed_ >= kMinimumSpeechSeconds)
    {
        finalise();
        state_ = State::ready;
    }
}

void AutoController::finalise() noexcept
{
    const float snr = juce::jlimit (0.0f, 60.0f, speechLevelDb_ - noiseFloorDb_);

    // Noisier rooms get a little more cleaning; clean speech gets less.
    suggestions_.cleanAmount = juce::jlimit (0.35f, 0.70f,
                                             snr < 20.0f ? 0.60f : 0.45f);
    suggestions_.eqStrength = 0.5f;
    suggestions_.deEsserAmount = juce::jlimit (0.40f, 0.70f, 0.50f + maxSibilance_ * 0.25f);
    suggestions_.harshnessAmount = juce::jlimit (0.40f, 0.70f, 0.50f + maxHarshness_ * 0.25f);
    suggestions_.levelTargetDb = juce::jlimit (-30.0f, -12.0f, speechLevelDb_ - 6.0f);
    suggestions_.protectAmount = 0.5f;
}

} // namespace voiceguard
