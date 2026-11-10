#include "PluginEditor.h"

#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"
#include "state/Presets.h"

namespace voiceguard
{

namespace
{
juce::Font boldFont (float size)
{
    return juce::Font (juce::FontOptions (size, juce::Font::bold));
}

void styleCaption (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, ui::Palette::textDim);
}

/** Give a control a screen-reader name, a description and a matching tooltip. */
void nameControl (juce::Component& component, const juce::String& title, const juce::String& description)
{
    component.setTitle (title);
    component.setDescription (description);

    if (auto* tooltip = dynamic_cast<juce::SettableTooltipClient*> (&component))
        tooltip->setTooltip (title + ": " + description);
}
} // namespace

VoiceGuardEditor::VoiceGuardEditor (VoiceGuardProcessor& processor)
    : juce::AudioProcessorEditor (&processor),
      processor_ (processor)
{
    setLookAndFeel (&lookAndFeel_);

    addAndMakeVisible (title_);
    title_.setText ("MyEvent Labs  VoiceGuard", juce::dontSendNotification);
    title_.setFont (boldFont (20.0f));
    title_.setColour (juce::Label::textColourId, ui::Palette::text);

    addAndMakeVisible (subtitle_);
    subtitle_.setText (juce::String (product::versionTag) + " ALPHA  -  insert, AUTO, mix",
                       juce::dontSendNotification);
    subtitle_.setColour (juce::Label::textColourId, ui::Palette::textDim);

    styleCaption (inCaption_, "IN  L / R");
    styleCaption (outCaption_, "OUT  L / R");
    addAndMakeVisible (inCaption_);
    addAndMakeVisible (outCaption_);

    for (auto* m : { &inL_, &inR_, &outL_, &outR_ })
        addAndMakeVisible (*m);

    configureKnob (clean_, cleanLabel_, "CLEAN");
    configureKnob (clarity_, clarityLabel_, "CLARITY");
    configureKnob (level_, levelLabel_, "LEVEL");
    configureKnob (protect_, protectLabel_, "PROTECT");

    cleanAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::macroClean, clean_);
    clarityAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::macroClarity, clarity_);
    levelAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::macroLevel, level_);
    protectAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::macroProtect, protect_);

    addAndMakeVisible (autoButton_);
    autoButton_.setButtonText ("AUTO");
    autoButton_.setClickingTogglesState (true);
    autoAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor_.apvts, params::autoMode, autoButton_);

    addAndMakeVisible (bypassButton_);
    bypassButton_.setButtonText ("BYPASS");
    bypassButton_.setClickingTogglesState (true);
    bypassAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor_.apvts, params::bypass, bypassButton_);

    addAndMakeVisible (advancedButton_);
    advancedButton_.setButtonText ("ADVANCED");
    advancedButton_.setClickingTogglesState (true);
    advancedButton_.onClick = [this] { setAdvancedVisible (advancedButton_.getToggleState()); };

    addAndMakeVisible (presetBox_);
    presetBox_.setTextWhenNothingSelected ("Preset");
    for (size_t i = 0; i < factoryPresets().size(); ++i)
        presetBox_.addItem (factoryPresets()[i].name, (int) i + 1);
    presetBox_.onChange = [this]
    {
        const int index = presetBox_.getSelectedItemIndex();
        if (index >= 0)
            applyPreset (processor_.apvts, factoryPresets()[(size_t) index]);
    };
    nameControl (presetBox_, "Preset", "load a factory starting point");

    for (auto* m : { &nrMeter_, &eqMeter_, &deEsserMeter_, &harshMeter_, &levelMeter_, &dynMeter_, &safetyMeter_ })
        addAndMakeVisible (*m);

    autoStatus_.setColour (juce::Label::textColourId, ui::Palette::accent);
    protectStatus_.setColour (juce::Label::textColourId, ui::Palette::safe);
    addAndMakeVisible (autoStatus_);
    addAndMakeVisible (protectStatus_);

    hpfLabel_.setText ("HPF", juce::dontSendNotification);
    hpfLabel_.setColour (juce::Label::textColourId, ui::Palette::textDim);
    hpfBox_.addItemList ({ "Off", "60 Hz", "80 Hz", "100 Hz", "120 Hz", "160 Hz" }, 1);
    addAndMakeVisible (hpfBox_);
    addAndMakeVisible (hpfLabel_);
    hpfAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor_.apvts, params::hpfHz, hpfBox_);

    configureAdvancedSlider (cleanAdv_, cleanAdvLabel_, "Clean");
    configureAdvancedSlider (eqAdv_, eqAdvLabel_, "EQ");
    configureAdvancedSlider (deEsserAdv_, deEsserAdvLabel_, "De-Ess");
    configureAdvancedSlider (levelTargetAdv_, levelTargetAdvLabel_, "Level Target");
    configureAdvancedSlider (outputTrimAdv_, outputTrimAdvLabel_, "Output Trim");

    cleanAdvAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::cleanAmount, cleanAdv_);
    eqAdvAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::eqStrength, eqAdv_);
    deEsserAdvAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::deEsserAmount, deEsserAdv_);
    levelTargetAdvAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::levelTargetDb, levelTargetAdv_);
    outputTrimAdvAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor_.apvts, params::outputTrimDb, outputTrimAdv_);

    nameControl (clean_, "CLEAN macro", "noise reduction and low-mid cleanup");
    nameControl (clarity_, "CLARITY macro", "intelligibility, de-essing and harshness");
    nameControl (level_, "LEVEL macro", "level rider and gentle dynamics");
    nameControl (protect_, "PROTECT macro", "feedback and output safety");
    nameControl (autoButton_, "AUTO", "listen to speech and set conservative starting values");
    nameControl (bypassButton_, "Bypass", "pass audio through unchanged");
    nameControl (advancedButton_, "Advanced panel", "show or hide the detailed controls");
    nameControl (hpfLabel_, "High-pass", "low-frequency filter");
    nameControl (hpfBox_, "High-pass frequency", "rumble filter corner frequency");
    nameControl (cleanAdv_, "Clean amount", "noise reduction depth");
    nameControl (eqAdv_, "EQ strength", "dynamic EQ depth");
    nameControl (deEsserAdv_, "De-esser amount", "sibilance control depth");
    nameControl (levelTargetAdv_, "Level target", "target speech level in dB");
    nameControl (outputTrimAdv_, "Output trim", "output gain in dB");

    nameControl (inL_, "Input level left", "input meter, -60 to +6 dB");
    nameControl (inR_, "Input level right", "input meter, -60 to +6 dB");
    nameControl (outL_, "Output level left", "output meter, -60 to +6 dB");
    nameControl (outR_, "Output level right", "output meter, -60 to +6 dB");
    nameControl (nrMeter_, "Noise reduction", "current noise reduction in dB");
    nameControl (eqMeter_, "Dynamic EQ", "current EQ reduction in dB");
    nameControl (deEsserMeter_, "De-esser", "current sibilance reduction in dB");
    nameControl (harshMeter_, "Harshness", "current harshness reduction in dB");
    nameControl (levelMeter_, "Level ride", "current level ride in dB");
    nameControl (dynMeter_, "Dynamics", "current compression gain reduction in dB");
    nameControl (safetyMeter_, "Safety", "current peak-safety gain reduction in dB");

    setAdvancedVisible (false);

    setResizable (true, true);
    setResizeLimits (900, 470, 1400, 960);
    setSize (980, 500);

    startTimerHz (30);
}

VoiceGuardEditor::~VoiceGuardEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void VoiceGuardEditor::configureKnob (juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, ui::Palette::textDim);
    addAndMakeVisible (label);
}

void VoiceGuardEditor::configureAdvancedSlider (juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 20);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, ui::Palette::textDim);
    addAndMakeVisible (label);
}

void VoiceGuardEditor::setAdvancedVisible (bool shouldBeVisible)
{
    advancedVisible_ = shouldBeVisible;
    advancedButton_.setToggleState (shouldBeVisible, juce::dontSendNotification);

    hpfBox_.setVisible (shouldBeVisible);
    hpfLabel_.setVisible (shouldBeVisible);

    juce::Component* const advancedControls[] = {
        &cleanAdv_, &cleanAdvLabel_, &eqAdv_, &eqAdvLabel_,
        &deEsserAdv_, &deEsserAdvLabel_, &levelTargetAdv_, &levelTargetAdvLabel_,
        &outputTrimAdv_, &outputTrimAdvLabel_ };

    for (auto* control : advancedControls)
        control->setVisible (shouldBeVisible);

    setSize (getWidth(), shouldBeVisible ? 690 : 500);
}

void VoiceGuardEditor::timerCallback()
{
    refreshReadouts();
}

void VoiceGuardEditor::refreshReadouts()
{
    auto& snap = processor_.snapshot();

    inL_.setLevelDb (snap.inputLevelDb[0].load());
    inR_.setLevelDb (snap.inputLevelDb[1].load());
    outL_.setLevelDb (snap.outputLevelDb[0].load());
    outR_.setLevelDb (snap.outputLevelDb[1].load());

    const float inPeak = juce::jmax (snap.inputLevelDb[0].load(), snap.inputLevelDb[1].load());
    const float outPeak = juce::jmax (snap.outputLevelDb[0].load(), snap.outputLevelDb[1].load());
    inCaption_.setText (juce::String::formatted ("IN  L/R   %.1f dB", inPeak), juce::dontSendNotification);
    outCaption_.setText (juce::String::formatted ("OUT  L/R  %.1f dB", outPeak), juce::dontSendNotification);

    nrMeter_.setDb (snap.noiseReductionDb.load());
    eqMeter_.setDb (snap.dynamicEqDb.load());
    deEsserMeter_.setDb (snap.deEsserDb.load());
    harshMeter_.setDb (snap.harshnessDb.load());
    levelMeter_.setDb (snap.levelRideDb.load());
    dynMeter_.setDb (snap.dynamicsGainReductionDb.load());
    safetyMeter_.setDb (snap.outputGainReductionDb.load());

    protectStatus_.setText (snap.protectionActive.load() ? "PROTECTION  ACTIVE" : "PROTECTION  SAFE",
                            juce::dontSendNotification);
    protectStatus_.setColour (juce::Label::textColourId,
                              snap.protectionActive.load() ? ui::Palette::warning : ui::Palette::safe);

    const int state = snap.autoState.load();

    if (state == 1)
        autoStatus_.setText (juce::String::formatted ("AUTO  listening  %d%%",
                                                      (int) (snap.autoProgress.load() * 100.0f)),
                             juce::dontSendNotification);
    else if (state == 2)
        autoStatus_.setText ("AUTO  ready", juce::dontSendNotification);
    else
        autoStatus_.setText ("AUTO  idle", juce::dontSendNotification);
}

void VoiceGuardEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::Palette::background);

    auto header = getLocalBounds().removeFromTop (56);
    g.setColour (ui::Palette::panelDark);
    g.fillRect (header);

    auto panel = [&g] (juce::Rectangle<int> r)
    {
        g.setColour (ui::Palette::panel);
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (ui::Palette::outline);
        g.drawRoundedRectangle (r.toFloat(), 6.0f, 1.0f);
    };

    auto area = getLocalBounds().reduced (14);
    area.removeFromTop (42);   // header row
    area.removeFromTop (14);

    panel (area.removeFromLeft (150).withHeight (402));  // meters
    area.removeFromLeft (12);
    panel (area.removeFromTop (140));                    // macros
    area.removeFromTop (10);
    panel (area.removeFromTop (34));                     // buttons
    area.removeFromTop (10);
    panel (area.removeFromTop (176));                    // gain reduction
}

void VoiceGuardEditor::resized()
{
    auto area = getLocalBounds().reduced (14);
    auto header = area.removeFromTop (42);
    title_.setBounds (header.removeFromLeft (juce::jmin (380, header.getWidth() / 2)));
    subtitle_.setBounds (header.withTrimmedTop (4));

    area.removeFromTop (14);

    // Input/output meters, per channel.
    auto metersArea = area.removeFromLeft (150).reduced (10, 12);
    auto inGroup = metersArea.removeFromTop (150);
    inCaption_.setBounds (inGroup.removeFromTop (18));
    auto inRow = inGroup;
    inL_.setBounds (inRow.removeFromLeft (inRow.getWidth() / 2).reduced (3, 0));
    inR_.setBounds (inRow.reduced (3, 0));

    metersArea.removeFromTop (10);

    auto outGroup = metersArea.removeFromTop (150);
    outCaption_.setBounds (outGroup.removeFromTop (18));
    auto outRow = outGroup;
    outL_.setBounds (outRow.removeFromLeft (outRow.getWidth() / 2).reduced (3, 0));
    outR_.setBounds (outRow.reduced (3, 0));

    area.removeFromLeft (12);
    auto right = area;

    // Macros.
    auto macros = right.removeFromTop (140).reduced (10, 12);
    const int knobWidth = macros.getWidth() / 4;

    auto placeMacro = [&] (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> cell)
    {
        label.setBounds (cell.removeFromBottom (20));
        slider.setBounds (cell.reduced (8));
    };

    placeMacro (clean_, cleanLabel_, macros.removeFromLeft (knobWidth));
    placeMacro (clarity_, clarityLabel_, macros.removeFromLeft (knobWidth));
    placeMacro (level_, levelLabel_, macros.removeFromLeft (knobWidth));
    placeMacro (protect_, protectLabel_, macros);

    right.removeFromTop (10);

    // Buttons + AUTO/PROTECTION status.
    auto buttons = right.removeFromTop (34);
    autoButton_.setBounds (buttons.removeFromLeft (120).reduced (0, 2));
    bypassButton_.setBounds (buttons.removeFromLeft (120).reduced (6, 2));
    advancedButton_.setBounds (buttons.removeFromRight (150).reduced (0, 2));
    presetBox_.setBounds (buttons.reduced (12, 2));

    right.removeFromTop (10);

    auto status = right.removeFromTop (22);
    autoStatus_.setBounds (status.removeFromLeft (status.getWidth() / 2));
    protectStatus_.setBounds (status);

    right.removeFromTop (10);

    // Gain-reduction row.
    auto grArea = right.removeFromTop (176);
    for (auto* meter : { &nrMeter_, &eqMeter_, &deEsserMeter_, &harshMeter_, &levelMeter_, &dynMeter_, &safetyMeter_ })
    {
        auto row = grArea.removeFromTop (25).reduced (0, 1);
        meter->setBounds (row);
    }

    if (! advancedVisible_)
        return;

    right.removeFromTop (10);
    auto adv = right;

    auto hpfRow = adv.removeFromTop (28);
    hpfLabel_.setBounds (hpfRow.removeFromLeft (110));
    hpfBox_.setBounds (hpfRow.removeFromLeft (170));

    auto placeAdv = [&] (juce::Slider& slider, juce::Label& label)
    {
        auto row = adv.removeFromTop (30);
        label.setBounds (row.removeFromLeft (110));
        slider.setBounds (row);
    };

    placeAdv (cleanAdv_, cleanAdvLabel_);
    placeAdv (eqAdv_, eqAdvLabel_);
    placeAdv (deEsserAdv_, deEsserAdvLabel_);
    placeAdv (levelTargetAdv_, levelTargetAdvLabel_);
    placeAdv (outputTrimAdv_, outputTrimAdvLabel_);
}

} // namespace voiceguard
