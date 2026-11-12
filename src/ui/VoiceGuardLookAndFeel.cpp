#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

const juce::Colour Palette::background { 0xff05070c };
const juce::Colour Palette::panel      { 0xff0b111e };
const juce::Colour Palette::panelHigh  { 0xff131c30 };
const juce::Colour Palette::panelDark  { 0xff04060a };
const juce::Colour Palette::outline    { 0xff1d2839 };
const juce::Colour Palette::outlineBright { 0xff2e3f5c };
const juce::Colour Palette::text       { 0xffeaf2ff };
const juce::Colour Palette::textDim    { 0xff93a6c4 };
const juce::Colour Palette::textFaint  { 0xff5f7392 };
const juce::Colour Palette::accent     { 0xff41e0e6 };
const juce::Colour Palette::accent2    { 0xff8b7cff };
const juce::Colour Palette::safe       { 0xff3ee6a0 };
const juce::Colour Palette::warning    { 0xffffc24b };
const juce::Colour Palette::danger     { 0xffff5c7a };

namespace
{
constexpr float kRadius = 8.0f;

juce::Font uiFont (float size, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (size, style));
}
} // namespace

juce::ColourGradient Palette::spectral (juce::Rectangle<float> bounds, bool horizontal)
{
    if (horizontal)
        return juce::ColourGradient (accent, bounds.getX(), bounds.getCentreY(),
                                     accent2, bounds.getRight(), bounds.getCentreY(), false);

    return juce::ColourGradient (accent, bounds.getCentreX(), bounds.getY(),
                                 accent2, bounds.getCentreX(), bounds.getBottom(), false);
}

void Palette::drawGlow (juce::Graphics& g, juce::Rectangle<float> bounds, float radius,
                        juce::Colour colour, float strength)
{
    for (int i = 6; i >= 1; --i)
    {
        g.setColour (colour.withAlpha (strength * (float) i / 42.0f));
        g.drawRoundedRectangle (bounds.expanded ((float) i * 1.6f), radius + (float) i, (float) i * 1.5f);
    }
}

void Palette::drawCard (juce::Graphics& g, juce::Rectangle<float> bounds, float radius)
{
    drawGlow (g, bounds, radius, accent, 0.35f);

    juce::ColourGradient grad (panelHigh, bounds.getX(), bounds.getY(),
                               panel, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (outlineBright.withAlpha (0.55f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + radius * 0.7f, bounds.getRight() - radius * 0.7f);

    // A faint spectral seam along the top edge.
    g.setColour (accent.withAlpha (0.10f));
    g.drawHorizontalLine ((int) bounds.getY() + 2, bounds.getX() + radius, bounds.getX() + bounds.getWidth() * 0.35f);
}

void Palette::drawWell (juce::Graphics& g, juce::Rectangle<float> bounds, float radius)
{
    g.setColour (panelDark);
    g.fillRoundedRectangle (bounds, radius);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
    g.setColour (accent.withAlpha (0.05f));
    g.drawHorizontalLine ((int) bounds.getBottom() - 1, bounds.getX() + radius, bounds.getRight() - radius);
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
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.20f));
}

void VoiceGuardLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto enabled = slider.isEnabled();

    const auto trackThickness = juce::jmax (3.0f, radius * 0.12f);
    const auto arcRadius = juce::jmax (2.0f, radius - trackThickness * 0.5f - 1.0f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Palette::panelDark);
    g.strokePath (track, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);

    if (enabled && sliderPos > 0.001f)
    {
        // Emissive halo, then the spectral arc.
        g.setColour (Palette::accent.withAlpha (0.20f));
        g.strokePath (value, juce::PathStrokeType (trackThickness * 3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        g.setColour (Palette::accent.withAlpha (0.30f));
        g.strokePath (value, juce::PathStrokeType (trackThickness * 1.9f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        juce::ColourGradient grad (Palette::accent, centre.x - arcRadius, centre.y,
                                   Palette::accent2, centre.x + arcRadius, centre.y, false);
        g.setGradientFill (grad);
        g.strokePath (value, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }
    else if (! enabled)
    {
        g.setColour (Palette::outline);
        g.strokePath (value, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // Knob body: glass sphere.
    const auto bodyRadius = juce::jmax (1.0f, arcRadius - trackThickness - 1.5f);
    const juce::ColourGradient body (Palette::panelHigh,
                                     centre.x - bodyRadius * 0.4f, centre.y - bodyRadius * 0.5f,
                                     Palette::panelDark,
                                     centre.x + bodyRadius, centre.y + bodyRadius, true);
    g.setGradientFill (body);
    g.fillEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
    g.setColour (Palette::outlineBright.withAlpha (0.7f));
    g.drawEllipse (centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (centre.x - bodyRadius + 1.5f, centre.y - bodyRadius + 1.5f,
                   bodyRadius * 2.0f - 3.0f, bodyRadius * 2.0f - 3.0f, 1.0f);

    // Glowing grip.
    const auto gripAngle = angle;
    const auto tipX = centre.x + std::sin (gripAngle) * (bodyRadius * 0.68f);
    const auto tipY = centre.y - std::cos (gripAngle) * (bodyRadius * 0.68f);
    g.setColour (Palette::accent.withAlpha (0.35f));
    g.fillEllipse (tipX - 3.2f, tipY - 3.2f, 6.4f, 6.4f);
    g.setColour (enabled ? Palette::text : Palette::textFaint);
    g.fillEllipse (tipX - 1.6f, tipY - 1.6f, 3.2f, 3.2f);
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
        Palette::drawGlow (g, bounds, kRadius, Palette::accent, shouldDrawButtonAsHighlighted ? 0.9f : 0.6f);

        juce::ColourGradient grad (Palette::accent, bounds.getX(), bounds.getY(),
                                   Palette::accent2, bounds.getRight(), bounds.getBottom(), false);
        if (shouldDrawButtonAsDown)
            grad.multiplyOpacity (0.85f);

        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, kRadius);
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + kRadius, bounds.getRight() - kRadius);
    }
    else
    {
        auto base = shouldDrawButtonAsDown ? Palette::panelDark
                                           : (shouldDrawButtonAsHighlighted ? Palette::panelHigh : Palette::panel);
        g.setColour (base);
        g.fillRoundedRectangle (bounds, kRadius);
        g.setColour (shouldDrawButtonAsHighlighted ? Palette::outlineBright : Palette::outline);
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
