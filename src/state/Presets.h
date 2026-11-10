#pragma once

#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

namespace voiceguard
{

/** A constrained starting point, not a fixed EQ gimmick. **/
struct Preset
{
    juce::String name;

    float macroClean = 0.5f;
    float macroClarity = 0.5f;
    float macroLevel = 0.5f;
    float macroProtect = 0.5f;

    int hpfIndex = 2;          // 2 = 80 Hz
    float inputTrimDb = 0.0f;

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
};

/** The ten factory presets, in UI order. **/
const std::vector<Preset>& factoryPresets();

/** Apply a preset to the live parameter tree. Message thread only. **/
void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& preset);

} // namespace voiceguard
