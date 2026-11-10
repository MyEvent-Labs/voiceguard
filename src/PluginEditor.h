#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"
#include "ui/LevelMeter.h"
#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard
{

/** MyEvent Labs VoiceGuard editor: four macros, AUTO, meters, readouts and an
    advanced panel. Reads only atomics from the processor. */
class VoiceGuardEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit VoiceGuardEditor (VoiceGuardProcessor&);
    ~VoiceGuardEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureKnob (juce::Slider&, juce::Label&, const juce::String& name);
    void configureAdvancedSlider (juce::Slider&, juce::Label&, const juce::String& name);
    void setAdvancedVisible (bool shouldBeVisible);

    VoiceGuardProcessor& processor_;
    ui::VoiceGuardLookAndFeel lookAndFeel_;

    juce::Label title_;
    juce::Label subtitle_;

    juce::Slider clean_, clarity_, level_, protect_;
    juce::Label cleanLabel_, clarityLabel_, levelLabel_, protectLabel_;
    juce::TextButton autoButton_, bypassButton_;

    ui::LevelMeter meterIn_, meterOut_;
    juce::Label inCaption_, outCaption_;

    juce::Label noiseReadout_, levelReadout_, harshReadout_, protectReadout_, autoStatus_;

    juce::TextButton advancedButton_;
    juce::ComboBox hpfBox_;
    juce::Label hpfLabel_;
    juce::Slider cleanAdv_, eqAdv_, deEsserAdv_, levelTargetAdv_, outputTrimAdv_;
    juce::Label cleanAdvLabel_, eqAdvLabel_, deEsserAdvLabel_, levelTargetAdvLabel_, outputTrimAdvLabel_;
    bool advancedVisible_ = false;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cleanAtt_, clarityAtt_,
        levelAtt_, protectAtt_, cleanAdvAtt_, eqAdvAtt_, deEsserAdvAtt_, levelTargetAdvAtt_, outputTrimAdvAtt_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoAtt_, bypassAtt_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hpfAtt_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceGuardEditor)
};

} // namespace voiceguard
