#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

const juce::Colour Palette::background { 0xff14161a };
const juce::Colour Palette::panel      { 0xff1c1f25 };
const juce::Colour Palette::panelDark  { 0xff101216 };
const juce::Colour Palette::outline    { 0xff2c313a };
const juce::Colour Palette::text       { 0xffe8eaee };
const juce::Colour Palette::textDim    { 0xff8b93a1 };
const juce::Colour Palette::accent     { 0xff4c8dff };
const juce::Colour Palette::safe       { 0xff4fd08a };
const juce::Colour Palette::warning    { 0xffe0a24f };

VoiceGuardLookAndFeel::VoiceGuardLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, Palette::accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, Palette::outline);
    setColour (juce::Slider::textBoxTextColourId, Palette::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, Palette::text);
    setColour (juce::TextButton::buttonColourId, Palette::panel);
    setColour (juce::TextButton::buttonOnColourId, Palette::accent);
    setColour (juce::TextButton::textColourOffId, Palette::text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ComboBox::backgroundColourId, Palette::panelDark);
    setColour (juce::ComboBox::outlineColourId, Palette::outline);
    setColour (juce::ComboBox::textColourId, Palette::text);
    setColour (juce::PopupMenu::backgroundColourId, Palette::panelDark);
    setColour (juce::PopupMenu::textColourId, Palette::text);
}

void VoiceGuardLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto thickness = juce::jmax (3.0f, radius * 0.20f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - thickness * 0.5f, radius - thickness * 0.5f,
                         0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Palette::outline);
    g.strokePath (track, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - thickness * 0.5f, radius - thickness * 0.5f,
                         0.0f, rotaryStartAngle, angle, true);
    g.setColour (Palette::accent);
    g.strokePath (value, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    juce::Path pointer;
    const auto pointerLength = radius * 0.55f;
    pointer.addRoundedRectangle (-1.5f, -pointerLength, 3.0f, pointerLength * 0.75f, 1.5f);
    g.setColour (Palette::text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void VoiceGuardLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    auto colour = backgroundColour;

    if (button.getToggleState())
        colour = Palette::accent;
    else if (shouldDrawButtonAsDown)
        colour = colour.brighter (0.25f);
    else if (shouldDrawButtonAsHighlighted)
        colour = colour.brighter (0.12f);

    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (colour);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (Palette::outline);
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
}

juce::Font VoiceGuardLookAndFeel::getLabelFont (juce::Label& label)
{
    return juce::Font (juce::FontOptions (juce::jlimit (10.0f, 20.0f, (float) label.getHeight() * 0.6f)));
}

juce::Font VoiceGuardLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jlimit (11.0f, 16.0f, (float) buttonHeight * 0.5f)));
}

} // namespace voiceguard::ui
