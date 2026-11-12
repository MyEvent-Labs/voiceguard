#include "PluginEditor.h"

#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"
#include "state/Presets.h"

namespace voiceguard
{

namespace
{
juce::Font fontOf (float size, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (size, style));
}

void styleCaption (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, ui::Palette::textDim);
}

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
    title_.setText ("VoiceGuard", juce::dontSendNotification);
    title_.setFont (fontOf (19.0f, juce::Font::bold));
    title_.setColour (juce::Label::textColourId, ui::Palette::text);

    addAndMakeVisible (subtitle_);
    subtitle_.setText ("MyEvent Labs", juce::dontSendNotification);
    subtitle_.setFont (fontOf (11.0f));
    subtitle_.setColour (juce::Label::textColourId, ui::Palette::textDim);

    addAndMakeVisible (ribbon_);
    nameControl (ribbon_, "Input level", "rolling input level and speech activity");

    styleCaption (inCaption_, "INPUT");
    styleCaption (outCaption_, "OUTPUT");
    addAndMakeVisible (inCaption_);
    addAndMakeVisible (outCaption_);

    for (auto* m : { &inL_, &inR_, &outL_, &outR_ })
        addAndMakeVisible (*m);

    configureKnob (clean_, cleanLabel_, "CLEAN");
    configureKnob (clarity_, clarityLabel_, "CLARITY");
    configureKnob (level_, levelLabel_, "LEVEL");
    configureKnob (protect_, protectLabel_, "PROTECT");

    for (auto* value : { &cleanValue_, &clarityValue_, &levelValue_, &protectValue_ })
    {
        value->setJustificationType (juce::Justification::centred);
        value->setFont (fontOf (11.5f, juce::Font::bold));
        value->setColour (juce::Label::textColourId, ui::Palette::textFaint);
        addAndMakeVisible (*value);
    }

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

    autoStatus_.setFont (fontOf (11.0f, juce::Font::bold));
    protectStatus_.setFont (fontOf (11.0f, juce::Font::bold));
    autoStatus_.setColour (juce::Label::textColourId, ui::Palette::accent);
    protectStatus_.setColour (juce::Label::textColourId, ui::Palette::safe);
    addAndMakeVisible (autoStatus_);
    addAndMakeVisible (protectStatus_);

    hpfLabel_.setText ("High-pass", juce::dontSendNotification);
    hpfLabel_.setColour (juce::Label::textColourId, ui::Palette::textDim);
    hpfBox_.addItemList ({ "Off", "60 Hz", "80 Hz", "100 Hz", "120 Hz", "160 Hz" }, 1);
    addAndMakeVisible (hpfBox_);
    addAndMakeVisible (hpfLabel_);
    hpfAtt_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor_.apvts, params::hpfHz, hpfBox_);

    configureAdvancedSlider (cleanAdv_, cleanAdvLabel_, "Clean amount");
    configureAdvancedSlider (eqAdv_, eqAdvLabel_, "EQ strength");
    configureAdvancedSlider (deEsserAdv_, deEsserAdvLabel_, "De-esser");
    configureAdvancedSlider (levelTargetAdv_, levelTargetAdvLabel_, "Level target");
    configureAdvancedSlider (outputTrimAdv_, outputTrimAdvLabel_, "Output trim");

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
    nameControl (hpfBox_, "High-pass frequency", "rumble filter corner frequency");
    nameControl (cleanAdv_, "Clean amount", "noise reduction depth");
    nameControl (eqAdv_, "EQ strength", "dynamic EQ depth");
    nameControl (deEsserAdv_, "De-esser amount", "sibilance control depth");
    nameControl (levelTargetAdv_, "Level target", "target speech level in dB");
    nameControl (outputTrimAdv_, "Output trim", "output gain in dB");

    nameControl (inL_, "Input level left", "input meter");
    nameControl (inR_, "Input level right", "input meter");
    nameControl (outL_, "Output level left", "output meter");
    nameControl (outR_, "Output level right", "output meter");

    setAdvancedVisible (false);

    setResizable (true, true);
    setResizeLimits (960, 520, 1500, 1000);
    setSize (1040, 600);

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
    label.setFont (fontOf (11.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, ui::Palette::textDim);
    addAndMakeVisible (label);
}

void VoiceGuardEditor::configureAdvancedSlider (juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 68, 20);
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

    setSize (getWidth(), shouldBeVisible ? 820 : 600);
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
    inCaption_.setText (juce::String::formatted ("INPUT   %.1f dB", inPeak), juce::dontSendNotification);
    outCaption_.setText (juce::String::formatted ("OUTPUT  %.1f dB", outPeak), juce::dontSendNotification);

    ribbon_.push (inPeak, snap.speechActivity.load());

    const auto percent = [this] (const char* id)
    {
        if (auto* parameter = processor_.apvts.getRawParameterValue (id))
            return juce::roundToInt (parameter->load() * 100.0f);

        return 0;
    };

    cleanValue_.setText (juce::String (percent (params::macroClean)) + "%", juce::dontSendNotification);
    clarityValue_.setText (juce::String (percent (params::macroClarity)) + "%", juce::dontSendNotification);
    levelValue_.setText (juce::String (percent (params::macroLevel)) + "%", juce::dontSendNotification);
    protectValue_.setText (juce::String (percent (params::macroProtect)) + "%", juce::dontSendNotification);

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

    // Header band.
    {
        juce::ColourGradient grad (ui::Palette::panelHigh, 0.0f, 0.0f,
                                   ui::Palette::background, 0.0f, (float) headerRect_.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (headerRect_);
        g.setColour (ui::Palette::outline);
        g.drawHorizontalLine (headerRect_.getBottom() - 1, 0.0f, (float) getWidth());
    }

    // Brand mark: a badge with a small waveform.
    {
        auto badge = juce::Rectangle<int> (16, headerRect_.getY() + (headerRect_.getHeight() - 34) / 2, 34, 34).toFloat();
        juce::ColourGradient bgrad (ui::Palette::accent.brighter (0.1f), badge.getX(), badge.getY(),
                                    ui::Palette::accent.darker (0.25f), badge.getX(), badge.getBottom(), false);
        g.setGradientFill (bgrad);
        g.fillRoundedRectangle (badge, 9.0f);

        g.setColour (juce::Colours::white.withAlpha (0.92f));
        const auto c = badge.getCentre();
        const float hs[5] = { 4.0f, 9.0f, 13.0f, 8.0f, 5.0f };

        for (int i = 0; i < 5; ++i)
        {
            const auto x = c.x - 8.0f + (float) i * 4.0f;
            const auto h = hs[i];
            g.drawLine (x, c.y - h * 0.5f, x, c.y + h * 0.5f, 1.8f);
        }
    }

    // Cards.
    ui::Palette::drawCard (g, metersCard_.toFloat(), 10.0f);
    ui::Palette::drawCard (g, macrosCard_.toFloat(), 10.0f);
    ui::Palette::drawCard (g, grCard_.toFloat(), 10.0f);

    // Card captions.
    g.setColour (ui::Palette::textFaint);
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
    auto caption = [&g] (const juce::String& text, juce::Rectangle<int> card)
    {
        g.drawText (text, card.reduced (16, 12).removeFromTop (12), juce::Justification::topLeft, false);
    };
    caption ("LEVELS", metersCard_);
    caption ("MACROS", macrosCard_);
    caption ("GAIN REDUCTION", grCard_);

    // Version pill.
    {
        auto pill = juce::Rectangle<int> (getWidth() - 132, headerRect_.getY() + (headerRect_.getHeight() - 22) / 2, 116, 22);
        g.setColour (ui::Palette::panelDark);
        g.fillRoundedRectangle (pill.toFloat(), 11.0f);
        g.setColour (ui::Palette::outline);
        g.drawRoundedRectangle (pill.toFloat().reduced (0.5f), 11.0f, 1.0f);
        g.setColour (ui::Palette::textDim);
        g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        g.drawText ("v0.0.1  ALPHA", pill, juce::Justification::centred, false);
    }
}

void VoiceGuardEditor::resized()
{
    auto area = getLocalBounds().reduced (16);

    headerRect_ = area.removeFromTop (58);
    {
        auto header = headerRect_.withTrimmedLeft (60).withTrimmedRight (330);
        auto titleRow = header.removeFromTop (header.getHeight() / 2);
        title_.setBounds (titleRow.withTrimmedBottom (2));
        subtitle_.setBounds (header);
    }

    presetBox_.setBounds (getWidth() - 132 - 196, headerRect_.getY() + (headerRect_.getHeight() - 28) / 2, 186, 28);

    area.removeFromTop (12);
    ribbonRect_ = area.removeFromTop (78);
    ribbon_.setBounds (ribbonRect_);
    area.removeFromTop (14);

    // Reserve the advanced panel and footer, then lay the three cards in the rest.
    auto advArea = advancedVisible_ ? area.removeFromBottom (200) : juce::Rectangle<int>();
    auto footerArea = area.removeFromBottom (120);
    footerRect_ = footerArea.removeFromTop (46);

    auto main = area;
    metersCard_ = main.removeFromLeft (160);
    main.removeFromLeft (14);
    grCard_ = main.removeFromRight (286);
    main.removeFromRight (14);
    macrosCard_ = main;

    // --- Meters card ---
    {
        auto m = metersCard_.reduced (16, 30);
        auto inGroup = m.removeFromTop (m.getHeight() / 2 - 8);
        inCaption_.setBounds (inGroup.removeFromTop (18));
        inL_.setBounds (inGroup.removeFromLeft (inGroup.getWidth() / 2).reduced (4, 2));
        inR_.setBounds (inGroup.reduced (4, 2));

        m.removeFromTop (16);
        auto outGroup = m;
        outCaption_.setBounds (outGroup.removeFromTop (18));
        outL_.setBounds (outGroup.removeFromLeft (outGroup.getWidth() / 2).reduced (4, 2));
        outR_.setBounds (outGroup.reduced (4, 2));
    }

    // --- Macros card ---
    {
        auto k = macrosCard_.reduced (24, 34);
        const int knobWidth = k.getWidth() / 4;

        auto place = [&] (juce::Slider& slider, juce::Label& label, juce::Label& value, juce::Rectangle<int> cell)
        {
            value.setBounds (cell.removeFromBottom (16));
            label.setBounds (cell.removeFromBottom (18));
            slider.setBounds (cell.reduced (6, 2));
        };

        place (clean_, cleanLabel_, cleanValue_, k.removeFromLeft (knobWidth));
        place (clarity_, clarityLabel_, clarityValue_, k.removeFromLeft (knobWidth));
        place (level_, levelLabel_, levelValue_, k.removeFromLeft (knobWidth));
        place (protect_, protectLabel_, protectValue_, k);
    }

    // --- Gain-reduction card ---
    {
        auto gr = grCard_.reduced (16, 30);
        const int rowHeight = juce::jmax (18, gr.getHeight() / 7);

        for (auto* meter : { &nrMeter_, &eqMeter_, &deEsserMeter_, &harshMeter_, &levelMeter_, &dynMeter_, &safetyMeter_ })
            meter->setBounds (gr.removeFromTop (rowHeight).reduced (0, 1));
    }

    // --- Footer ---
    autoButton_.setBounds (footerRect_.removeFromLeft (150).reduced (0, 4));
    bypassButton_.setBounds (footerRect_.removeFromLeft (128).reduced (10, 4));
    advancedButton_.setBounds (footerRect_.removeFromRight (150).reduced (0, 4));

    auto statusRow = footerArea.removeFromTop (24);
    autoStatus_.setBounds (statusRow.removeFromLeft (statusRow.getWidth() / 2));
    protectStatus_.setBounds (statusRow);

    // --- Advanced panel ---
    if (! advancedVisible_)
        return;

    auto adv = advArea.reduced (16, 0);
    adv.removeFromTop (10);

    auto hpfRow = adv.removeFromTop (30);
    hpfLabel_.setBounds (hpfRow.removeFromLeft (130));
    hpfBox_.setBounds (hpfRow.removeFromLeft (180));

    auto placeAdv = [&] (juce::Slider& slider, juce::Label& label)
    {
        auto row = adv.removeFromTop (32);
        label.setBounds (row.removeFromLeft (130));
        slider.setBounds (row);
    };

    placeAdv (cleanAdv_, cleanAdvLabel_);
    placeAdv (eqAdv_, eqAdvLabel_);
    placeAdv (deEsserAdv_, deEsserAdvLabel_);
    placeAdv (levelTargetAdv_, levelTargetAdvLabel_);
    placeAdv (outputTrimAdv_, outputTrimAdvLabel_);
}

} // namespace voiceguard
