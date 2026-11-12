#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"
#include "ui/LevelMeter.h"
#include "ui/ReductionMeter.h"
#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard
{

/** MyEvent Labs VoiceGuard editor.

    Four macros, AUTO, bypass, per-channel input/output meters, a gain-reduction
    row for every processing stage, and an advanced panel. Reads only atomics
    from the processor. */
class VoiceGuardEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit VoiceGuardEditor (VoiceGuardProcessor&);
    ~VoiceGuardEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Pull the latest engine readouts into the meters and labels. Called by
        the timer, and directly by the offline snapshot tool. */
    void refreshReadouts();

private:
    void timerCallback() override;
    void configureKnob (juce::Slider&, juce::Label&, const juce::String& name);
    void configureAdvancedSlider (juce::Slider&, juce::Label&, const juce::String& name);
    void setAdvancedVisible (bool shouldBeVisible);

    VoiceGuardProcessor& processor_;
    ui::VoiceGuardLookAndFeel lookAndFeel_;
    juce::TooltipWindow tooltipWindow_ { this, 600 };

    // Gain-reduction row.
    ui::ReductionMeter nrMeter_ { "NOISE", false, 12.0f };
    ui::ReductionMeter eqMeter_ { "EQ", false, 6.0f };
    ui::ReductionMeter deEsserMeter_ { "DE-ESS", false, 8.0f };
    ui::ReductionMeter harshMeter_ { "HARSH", false, 5.0f };
    ui::ReductionMeter levelMeter_ { "LEVEL", true, 6.0f };
    ui::ReductionMeter dynMeter_ { "DYN", false, 12.0f };
    ui::ReductionMeter safetyMeter_ { "SAFETY", false, 6.0f };

    juce::Label title_;
    juce::Label subtitle_;

    juce::Label inCaption_, outCaption_;
    ui::LevelMeter inL_, inR_, outL_, outR_;

    juce::Slider clean_, clarity_, level_, protect_;
    juce::Label cleanLabel_, clarityLabel_, levelLabel_, protectLabel_;
    juce::Label cleanValue_, clarityValue_, levelValue_, protectValue_;
    juce::TextButton autoButton_, bypassButton_, advancedButton_;

    juce::Label autoStatus_, protectStatus_;

    juce::ComboBox presetBox_;

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
