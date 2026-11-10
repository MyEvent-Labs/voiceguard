#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

/** Stable parameter identity and the plain parameter snapshot consumed by
    the DSP core. IDs must never change after v0.0.1. */
namespace voiceguard::params
{

inline constexpr const char* bypass           = "bypass";
inline constexpr const char* autoMode         = "autoMode";
inline constexpr const char* autoState        = "autoState";
inline constexpr const char* macroClean       = "macroClean";
inline constexpr const char* macroClarity     = "macroClarity";
inline constexpr const char* macroLevel       = "macroLevel";
inline constexpr const char* macroProtect     = "macroProtect";
inline constexpr const char* inputTrimDb      = "inputTrimDb";
inline constexpr const char* hpfHz            = "hpfHz";
inline constexpr const char* cleanAmount      = "cleanAmount";
inline constexpr const char* cleanAdaptation  = "cleanAdaptation";
inline constexpr const char* eqStrength       = "eqStrength";
inline constexpr const char* deEsserAmount    = "deEsserAmount";
inline constexpr const char* harshnessAmount  = "harshnessAmount";
inline constexpr const char* levelTargetDb    = "levelTargetDb";
inline constexpr const char* levelRangeDb     = "levelRangeDb";
inline constexpr const char* levelSpeed       = "levelSpeed";
inline constexpr const char* dynThresholdDb   = "dynThresholdDb";
inline constexpr const char* dynRatio         = "dynRatio";
inline constexpr const char* dynAttackMs      = "dynAttackMs";
inline constexpr const char* dynReleaseMs     = "dynReleaseMs";
inline constexpr const char* dynKneeDb        = "dynKneeDb";
inline constexpr const char* protectAmount    = "protectAmount";
inline constexpr const char* outputCeilingDb  = "outputCeilingDb";
inline constexpr const char* outputTrimDb     = "outputTrimDb";
inline constexpr const char* deFeedbackEnable = "deFeedbackEnable";

enum class AutoState { idle = 0, listening, ready };
enum class HpfChoice { off = 0, hz60, hz80, hz100, hz120, hz160 };

inline constexpr std::array<float, 6> hpfFrequenciesHz { 0.0f, 60.0f, 80.0f, 100.0f, 120.0f, 160.0f };

/** Plain snapshot of every live parameter, safe to copy to the audio thread.
    Non-finite values can never appear here (the APVTS clamps). */
struct Parameters
{
    bool  bypass = false;
    bool  autoMode = false;
    int   autoState = 0;

    float macroClean = 0.5f;
    float macroClarity = 0.5f;
    float macroLevel = 0.5f;
    float macroProtect = 0.5f;

    float inputTrimDb = 0.0f;
    int   hpfIndex = 2;

    float cleanAmount = 0.5f;
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

    float protectAmount = 0.5f;
    float outputCeilingDb = -1.0f;
    float outputTrimDb = 0.0f;

    bool  deFeedbackEnable = false;

    float hpfFrequencyHz() const noexcept
    {
        return hpfFrequenciesHz[(size_t) juce::jlimit (0, 5, hpfIndex)];
    }

    /** Read the current values out of a live APVTS. Message thread only. */
    static Parameters fromApvts (const juce::AudioProcessorValueTreeState&);
};

/** Build the full parameter layout. */
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace voiceguard::params
