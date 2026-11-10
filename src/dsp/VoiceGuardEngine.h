#pragma once

#include <atomic>

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/AutoController.h"
#include "dsp/DeEsser.h"
#include "dsp/DeFeedbackInterface.h"
#include "dsp/DynamicEq.h"
#include "dsp/Dynamics.h"
#include "dsp/HarshnessControl.h"
#include "dsp/InputConditioning.h"
#include "dsp/LevelRider.h"
#include "dsp/NoiseReduction.h"
#include "dsp/OutputSafety.h"
#include "dsp/Parameters.h"
#include "dsp/SpeechAnalyzer.h"

namespace voiceguard
{

/** Message-thread-safe readouts for the editor. Plain atomics only. */
struct UiSnapshot
{
    static constexpr int kMaxChannels = 2;

    std::atomic<float> inputLevelDb[kMaxChannels] {};
    std::atomic<float> outputLevelDb[kMaxChannels] {};

    // Per-stage gain reduction (positive dB), plus the signed level ride.
    std::atomic<float> noiseReductionDb { 0.0f };
    std::atomic<float> dynamicEqDb { 0.0f };
    std::atomic<float> harshnessDb { 0.0f };
    std::atomic<float> deEsserDb { 0.0f };
    std::atomic<float> levelRideDb { 0.0f };
    std::atomic<float> dynamicsGainReductionDb { 0.0f };
    std::atomic<float> outputGainReductionDb { 0.0f };

    std::atomic<float> speechActivity { 0.0f };
    std::atomic<float> autoProgress { 0.0f };
    std::atomic<int> autoState { 0 };
    std::atomic<bool> protectionActive { false };
};

/** The full VoiceGuard signal chain.

    Threading contract: prepare(), reset() and setParameters() are called from
    the audio thread only (setParameters is called once per block by the
    processor after reading the APVTS). process() is audio-thread only and does
    no allocation, locks or IO. snapshot() is safe to read from any thread.

    Chain: input -> conditioning/HPF -> noise reduction -> dynamic EQ ->
    harshness -> de-esser -> level rider -> dynamics -> [De-Feedback] ->
    output safety -> output trim -> click-free bypass mix. */
class VoiceGuardEngine
{
public:
    VoiceGuardEngine() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;
    void setParameters (const params::Parameters& p) noexcept;
    void process (juce::AudioBuffer<float>& buffer) noexcept;

    UiSnapshot& snapshot() noexcept { return snapshot_; }
    int latencySamples() const noexcept { return 0; }

    /** Injection point for a future validated De-Feedback engine. */
    void setFeedbackEngine (IDeFeedbackEngine* engine) noexcept { feedback_ = engine != nullptr ? engine : &nullFeedback_; }

private:
    struct Effective
    {
        float inputTrimDb = 0.0f;
        float hpfHz = 80.0f;
        float cleanAmount = 0.0f;
        float cleanAdaptation = 0.5f;
        float eqStrength = 0.5f;
        float deEsserAmount = 0.5f;
        float harshnessAmount = 0.5f;
        float levelTargetDb = -18.0f;
        float levelRangeDb = 6.0f;
        float levelSpeed = 0.5f;
        float dynThresholdDb = -18.0f;
        float dynRatio = 2.0f;
        float dynAttackMs = 20.0f;
        float dynReleaseMs = 250.0f;
        float dynKneeDb = 12.0f;
        float outputCeilingDb = -1.0f;
        float outputTrimDb = 0.0f;
        bool deFeedback = false;
        bool bypass = false;
    };

    void resolve() noexcept;

    double sampleRate_ = 48000.0;
    int maxChannels_ = 2;
    bool prepared_ = false;

    params::Parameters params_;
    Effective eff_;
    bool autoWasOn_ = false;

    SpeechAnalyzer analyzer_;
    InputConditioning input_;
    NoiseReduction noiseReduction_;
    DynamicEq dynamicEq_;
    HarshnessControl harshness_;
    DeEsser deEsser_;
    LevelRider levelRider_;
    Dynamics dynamics_;
    OutputSafety outputSafety_;
    AutoController auto_;
    NullDeFeedbackEngine nullFeedback_;
    IDeFeedbackEngine* feedback_ = &nullFeedback_;

    juce::AudioBuffer<float> dryBuffer_;
    juce::SmoothedValue<float> bypassMix_ { 0.0f };
    UiSnapshot snapshot_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceGuardEngine)
};

} // namespace voiceguard
