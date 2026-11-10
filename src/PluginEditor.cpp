#include "PluginEditor.h"

#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"

namespace voiceguard
{

namespace
{
juce::Font boldFont (float size)
{
    return juce::Font (juce::FontOptions (size, juce::Font::bold));
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

    addAndMakeVisible (meterIn_);
    addAndMakeVisible (meterOut_);
    inCaption_.setText ("IN", juce::dontSendNotification);
    outCaption_.setText ("OUT", juce::dontSendNotification);
    inCaption_.setColour (juce::Label::textColourId, ui::Palette::textDim);
    outCaption_.setColour (juce::Label::textColourId, ui::Palette::textDim);
    addAndMakeVisible (inCaption_);
    addAndMakeVisible (outCaption_);

    for (auto* readout : { &noiseReadout_, &levelReadout_, &harshReadout_, &protectReadout_, &autoStatus_ })
    {
        readout->setColour (juce::Label::textColourId, ui::Palette::text);
        addAndMakeVisible (*readout);
    }

    autoStatus_.setColour (juce::Label::textColourId, ui::Palette::accent);

    addAndMakeVisible (advancedButton_);
    advancedButton_.setButtonText ("ADVANCED");
    advancedButton_.setClickingTogglesState (true);
    advancedButton_.onClick = [this] { setAdvancedVisible (advancedButton_.getToggleState()); };

    hpfLabel_.setText ("HPF", juce::dontSendNotification);
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

    setAdvancedVisible (false);

    setResizable (true, true);
    setResizeLimits (760, 400, 1280, 760);
    setSize (860, 440);

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

    setSize (getWidth(), shouldBeVisible ? 640 : 440);
}

void VoiceGuardEditor::timerCallback()
{
    auto& snap = processor_.snapshot();

    meterIn_.setLevelDb (snap.inputLevelDb.load());
    meterOut_.setLevelDb (snap.outputLevelDb.load());

    noiseReadout_.setText (juce::String::formatted ("NOISE  -%.1f dB", snap.noiseReductionDb.load()),
                           juce::dontSendNotification);
    levelReadout_.setText (juce::String::formatted ("LEVEL  %+.1f dB", snap.levelRideDb.load()),
                           juce::dontSendNotification);
    harshReadout_.setText (juce::String::formatted ("HARSH  -%.1f dB", snap.harshnessDb.load()),
                           juce::dontSendNotification);
    protectReadout_.setText (snap.protectionActive.load() ? "PROTECTION  ACTIVE" : "PROTECTION  SAFE",
                             juce::dontSendNotification);
    protectReadout_.setColour (juce::Label::textColourId,
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

    auto bounds = getLocalBounds().reduced (14).withTrimmedTop (56);
    panel (bounds.removeFromTop (150));           // macros row
    panel (bounds.removeFromTop (92).reduced (0, 6)); // meters + readouts
}

void VoiceGuardEditor::resized()
{
    auto area = getLocalBounds().reduced (14);
    auto header = area.removeFromTop (42);
    title_.setBounds (header.removeFromLeft (juce::jmin (360, header.getWidth() / 2)));
    subtitle_.setBounds (header.withTrimmedTop (4));

    area.removeFromTop (14);

    // Macros row.
    auto macros = area.removeFromTop (136);
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

    area.removeFromTop (10);

    // Buttons + meters + readouts.
    auto controls = area.removeFromTop (86);
    auto buttons = controls.removeFromLeft (150);
    autoButton_.setBounds (buttons.removeFromTop (34).reduced (0, 2));
    bypassButton_.setBounds (buttons.removeFromTop (34).reduced (0, 2));

    controls.removeFromLeft (12);
    auto meters = controls.removeFromLeft (260);
    auto inRow = meters.removeFromTop (22);
    inCaption_.setBounds (inRow.removeFromLeft (40));
    meterIn_.setBounds (inRow);
    auto outRow = meters.removeFromTop (22).withTrimmedTop (6);
    outCaption_.setBounds (outRow.removeFromLeft (40));
    meterOut_.setBounds (outRow);

    controls.removeFromLeft (16);
    auto readouts = controls;
    noiseReadout_.setBounds (readouts.removeFromTop (20));
    levelReadout_.setBounds (readouts.removeFromTop (20));
    harshReadout_.setBounds (readouts.removeFromTop (20));
    protectReadout_.setBounds (readouts.removeFromTop (20));

    area.removeFromTop (10);
    advancedButton_.setBounds (area.removeFromTop (28).removeFromRight (140));

    if (! advancedVisible_)
        return;

    area.removeFromTop (8);
    auto adv = area;

    auto hpfRow = adv.removeFromTop (28);
    hpfLabel_.setBounds (hpfRow.removeFromLeft (110));
    hpfBox_.setBounds (hpfRow.removeFromLeft (160));

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
