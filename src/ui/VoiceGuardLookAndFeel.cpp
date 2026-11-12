#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

// Cool-tinted neutral ground; one accent carries live data.
const juce::Colour Palette::background { 0xff0f1216 };
const juce::Colour Palette::panel      { 0xff181c22 };
const juce::Colour Palette::panelDark  { 0xff0b0d11 };
const juce::Colour Palette::outline    { 0xff30373f };
const juce::Colour Palette::text       { 0xffeef1f5 };
const juce::Colour Palette::textDim    { 0xff9aa4b2 };
const juce::Colour Palette::textFaint  { 0xff7d8898 };
const juce::Colour Palette::accent     { 0xff5aa9ff };
const juce::Colour Palette::safe       { 0xff57d9a3 };
const juce::Colour Palette::warning    { 0xffe8b04a };
const juce::Colour Palette::danger     { 0xffe5645f };

namespace
{
constexpr float kRadius = 6.0f;

juce::Font uiFont (float size, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (size, style));
}
} // namespace

VoiceGuardLookAndFeel::VoiceGuardLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, Palette::accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, Palette::panelDark);
    setColour (juce::Slider::textBoxTextColourId, Palette::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::trackColourId, Palette::accent);
    setColour (juce::Label::textColourId, Palette::text);
    setColour (juce::TextButton::buttonColourId, Palette::panel);
    setColour (juce::TextButton::buttonOnColourId, Palette::accent);
    setColour (juce::TextButton::textColourOffId, Palette::textDim);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ComboBox::backgroundColourId, Palette::panelDark);
    setColour (juce::ComboBox::outlineColourId, Palette::outline);
    setColour (juce::ComboBox::textColourId, Palette::text);
    setColour (juce::ComboBox::arrowColourId, Palette::textDim);
    setColour (juce::PopupMenu::backgroundColourId, Palette::panel);
    setColour (juce::PopupMenu::textColourId, Palette::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.25f));
}

void VoiceGuardLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const auto trackThickness = juce::jmax (3.0f, radius * 0.14f);
    const auto arcRadius = juce::jmax (2.0f, radius - trackThickness * 0.5f - 1.0f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Palette::panelDark);
    g.strokePath (track, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                         rotaryStartAngle, angle, true);
    g.setColour (slider.isEnabled() ? Palette::accent : Palette::outline);
    g.strokePath (value, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Knob body: one border for elevation, no shadow.
    const auto bodyRadius = juce::jmax (1.0f, arcRadius - trackThickness - 1.0f);
    g.setColour (Palette::panel);
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (Palette::outline);
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

    juce::Path grip;
    const auto gripLength = bodyRadius * 0.5f;
    grip.addRoundedRectangle (-1.25f, -gripLength, 2.5f, gripLength * 0.8f, 1.25f);
    g.setColour (Palette::text);
    g.fillPath (grip, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void VoiceGuardLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused (backgroundColour);

    const bool on = button.getToggleState();
    auto fill = on ? Palette::accent : Palette::panel;

    if (shouldDrawButtonAsDown)
        fill = on ? Palette::accent.darker (0.12f) : Palette::panelDark;
    else if (shouldDrawButtonAsHighlighted)
        fill = on ? Palette::accent.brighter (0.06f) : Palette::panel.brighter (0.12f);

    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, kRadius);
    g.setColour (on ? Palette::accent.brighter (0.12f) : Palette::outline);
    g.drawRoundedRectangle (bounds, kRadius, 1.0f);

    if (button.hasKeyboardFocus (true))
    {
        g.setColour (Palette::accent);
        g.drawRoundedRectangle (bounds.reduced (1.5f), kRadius - 1.0f, 1.5f);
    }
}

void VoiceGuardLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                          int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
    g.setColour (box.isEnabled() ? Palette::panelDark : Palette::panel);
    g.fillRoundedRectangle (bounds, kRadius);
    g.setColour (box.hasKeyboardFocus (true) ? Palette::accent : Palette::outline);
    g.drawRoundedRectangle (bounds, kRadius, 1.0f);

    juce::Path chevron;
    const auto cx = (float) width - 14.0f;
    const auto cy = (float) height * 0.5f;
    chevron.startNewSubPath (cx - 3.5f, cy - 1.5f);
    chevron.lineTo (cx, cy + 2.0f);
    chevron.lineTo (cx + 3.5f, cy - 1.5f);
    g.setColour (Palette::textDim);
    g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
}

juce::Font VoiceGuardLookAndFeel::getLabelFont (juce::Label& label)
{
    return uiFont (juce::jlimit (10.0f, 18.0f, (float) label.getHeight() * 0.62f));
}

juce::Font VoiceGuardLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return uiFont (juce::jlimit (10.5f, 14.0f, (float) buttonHeight * 0.42f), juce::Font::bold);
}

juce::Font VoiceGuardLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return uiFont (juce::jlimit (11.0f, 14.0f, (float) box.getHeight() * 0.42f));
}

} // namespace voiceguard::ui
