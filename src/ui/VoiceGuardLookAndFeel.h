#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** MyEvent Labs "aurora" palette — a deep space ground with glass surfaces,
    a cyan→violet spectral accent and soft emissive edges. Futuristic but
    controlled: glow earns its place on live data, not decoration. */
struct Palette
{
    static const juce::Colour background;
    static const juce::Colour panel;
    static const juce::Colour panelHigh;
    static const juce::Colour panelDark;
    static const juce::Colour outline;
    static const juce::Colour outlineBright;
    static const juce::Colour text;
    static const juce::Colour textDim;
    static const juce::Colour textFaint;
    static const juce::Colour accent;   // cyan
    static const juce::Colour accent2;  // violet
    static const juce::Colour safe;
    static const juce::Colour warning;
    static const juce::Colour danger;

    /** Soft emissive glow behind a rounded rect (no shadow blur available). */
    static void drawGlow (juce::Graphics&, juce::Rectangle<float> bounds, float radius,
                          juce::Colour colour, float strength);
    /** Glass card: emissive edge, gradient fill, hairline, top rim light. */
    static void drawCard (juce::Graphics&, juce::Rectangle<float> bounds, float radius);
    /** Sunken well with an inner shadow and a faint accent floor glow. */
    static void drawWell (juce::Graphics&, juce::Rectangle<float> bounds, float radius);

    /** The signature spectral gradient (cyan -> violet). */
    static juce::ColourGradient spectral (juce::Rectangle<float> bounds, bool horizontal);
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
