#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** MyEvent Labs visual system: dark neutral surface, one restrained accent. */
struct Palette
{
    static const juce::Colour background;
    static const juce::Colour panel;
    static const juce::Colour panelDark;
    static const juce::Colour outline;
    static const juce::Colour text;
    static const juce::Colour textDim;
    static const juce::Colour accent;
    static const juce::Colour safe;
    static const juce::Colour warning;
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

    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};

} // namespace voiceguard::ui
