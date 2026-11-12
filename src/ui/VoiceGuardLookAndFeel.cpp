#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

const juce::Colour Palette::background { 0xff0b0e13 };
const juce::Colour Palette::panel      { 0xff12171f };
const juce::Colour Palette::panelHigh  { 0xff181f29 };
const juce::Colour Palette::panelDark  { 0xff090b0f };
const juce::Colour Palette::outline    { 0xff252c37 };
const juce::Colour Palette::outlineBright { 0xff333c4a };
const juce::Colour Palette::text       { 0xfff4f7fb };
const juce::Colour Palette::textDim    { 0xff9ba6b4 };
const juce::Colour Palette::textFaint  { 0xff6e7887 };
const juce::Colour Palette::accent     { 0xff4d8dff };
const juce::Colour Palette::safe       { 0xff38d39f };
const juce::Colour Palette::warning    { 0xffe8b04a };
const juce::Colour Palette::danger     { 0xffef5a52 };

namespace
{
constexpr float kRadius = 7.0f;

juce::Font uiFont (float size, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (size, style));
}
} // namespace

void Palette::drawCard (juce::Graphics& g, juce::Rectangle<float> bounds, float radius)
{
    juce::ColourGradient grad (panelHigh, bounds.getX(), bounds.getY(),
                               panel, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, radius);
    g.setColour (outline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.045f));
    g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + radius * 0.6f, bounds.getRight() - radius * 0.6f);
}

void Palette::drawWell (juce::Graphics& g, juce::Rectangle<float> bounds, float radius)
{
    g.setColour (panelDark);
    g.fillRoundedRectangle (bounds, radius);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + 2.0f, bounds.getRight() - 2.0f);
}

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
    setColour (juce::PopupMenu::backgroundColourId, Palette::panelHigh);
    setColour (juce::PopupMenu::textColourId, Palette::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.22f));
}

void VoiceGuardLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const auto trackThickness = juce::jmax (3.0f, radius * 0.13f);
    const auto arcRadius = juce::jmax (2.0f, radius - trackThickness * 0.5f - 1.0f);

    // Track.
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Palette::panelDark);
    g.strokePath (track, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Value arc: soft under-glow then the crisp gradient arc.
    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);

    const auto enabled = slider.isEnabled();

    if (enabled && sliderPos > 0.001f)
    {
        g.setColour (Palette::accent.withAlpha (0.22f));
        g.strokePath (value, juce::PathStrokeType (trackThickness * 2.4f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    g.setColour (enabled ? Palette::accent : Palette::outline);
    g.strokePath (value, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Knob body: radial sheen for depth.
    const auto bodyRadius = juce::jmax (1.0f, arcRadius - trackThickness - 1.5f);
    const juce::ColourGradient body (Palette::panelHigh,
                                     centre.x - bodyRadius * 0.4f, centre.y - bodyRadius * 0.45f,
                                     Palette::panel,
                                     centre.x + bodyRadius, centre.y + bodyRadius, true);
    g.setGradientFill (body);
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (Palette::outline);
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawEllipse (centre.x - bodyRadius + 1.5f, centre.y - bodyRadius + 1.5f,
                   bodyRadius * 2.0f - 3.0f, bodyRadius * 2.0f - 3.0f, 1.0f);

    // Grip.
    juce::Path grip;
    const auto gripLength = bodyRadius * 0.52f;
    grip.addRoundedRectangle (-1.25f, -gripLength, 2.5f, gripLength * 0.8f, 1.25f);
    g.setColour (enabled ? Palette::text : Palette::textFaint);
    g.fillPath (grip, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void VoiceGuardLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused (backgroundColour);

    const bool on = button.getToggleState();
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

    if (on)
    {
        juce::ColourGradient grad (Palette::accent.brighter (0.10f), bounds.getX(), bounds.getY(),
                                   Palette::accent.darker (0.10f), bounds.getX(), bounds.getBottom(), false);
        if (shouldDrawButtonAsDown)
            grad.multiplyOpacity (0.85f);
        else if (shouldDrawButtonAsHighlighted)
            grad.multiplyOpacity (1.0f);

        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, kRadius);
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + kRadius, bounds.getRight() - kRadius);
    }
    else
    {
        auto base = shouldDrawButtonAsDown ? Palette::panelDark
                                           : (shouldDrawButtonAsHighlighted ? Palette::panelHigh : Palette::panel);
        g.setColour (base);
        g.fillRoundedRectangle (bounds, kRadius);
        g.setColour (Palette::outline);
        g.drawRoundedRectangle (bounds, kRadius, 1.0f);
    }

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
    const auto cx = (float) width - 15.0f;
    const auto cy = (float) height * 0.5f;
    chevron.startNewSubPath (cx - 4.0f, cy - 1.75f);
    chevron.lineTo (cx, cy + 2.25f);
    chevron.lineTo (cx + 4.0f, cy - 1.75f);
    g.setColour (Palette::textDim);
    g.strokePath (chevron, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
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
