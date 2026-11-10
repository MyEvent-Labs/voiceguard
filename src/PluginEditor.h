#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

namespace voiceguard
{

/** Minimal editor scaffold. The MyEvent Labs UI system (macros, meters,
    AUTO, advanced panel) is added by the UI task. */
class VoiceGuardEditor : public juce::AudioProcessorEditor
{
public:
    explicit VoiceGuardEditor (VoiceGuardProcessor&);
    ~VoiceGuardEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VoiceGuardProcessor& processor_;
    juce::Label title_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceGuardEditor)
};

} // namespace voiceguard
