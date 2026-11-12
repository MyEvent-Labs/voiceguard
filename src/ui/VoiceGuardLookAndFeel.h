#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** MyEvent Labs palette. Cool-tinted neutrals, one accent, semantic state
    colours. Elevation is a border, never a shadow. */
struct Palette
{
    static const juce::Colour background;    // window ground
    static const juce::Colour panel;         // raised surface
    static const juce::Colour panelDark;     // sunken well / meter track
    static const juce::Colour outline;       // borders
    static const juce::Colour text;          // primary
    static const juce::Colour textDim;       // secondary
    static const juce::Colour textFaint;     // tertiary / units
    static const juce::Colour accent;        // active, primary data
    static const juce::Colour safe;          // nominal
    static const juce::Colour warning;       // near ceiling
    static const juce::Colour danger;        // clip / over
};

class VoiceGuardLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoiceGuardLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
};

} // namespace voiceguard::ui
