#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** MyEvent Labs palette — deep cool-neutral ground, layered surfaces, one
    refined accent, semantic state colours. Depth comes from layered fills and
    hairlines, never from heavy shadows. */
struct Palette
{
    static const juce::Colour background;
    static const juce::Colour panel;         // card surface (gradient bottom)
    static const juce::Colour panelHigh;     // card surface (gradient top)
    static const juce::Colour panelDark;     // sunken well / meter track
    static const juce::Colour outline;       // hairline
    static const juce::Colour outlineBright; // hairline highlight
    static const juce::Colour text;
    static const juce::Colour textDim;
    static const juce::Colour textFaint;
    static const juce::Colour accent;
    static const juce::Colour safe;
    static const juce::Colour warning;
    static const juce::Colour danger;

    /** A card: subtle vertical gradient, hairline, top inner highlight. */
    static void drawCard (juce::Graphics&, juce::Rectangle<float> bounds, float radius);
    /** A sunken well: flat dark fill with a top inner shadow. */
    static void drawWell (juce::Graphics&, juce::Rectangle<float> bounds, float radius);
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
