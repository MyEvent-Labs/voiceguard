#include "ui/LevelMeter.h"

#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

void LevelMeter::setLevelDb (float db)
{
    levelDb_ = juce::jlimit (-100.0f, 6.0f, db);
    peakDb_ = juce::jmax (levelDb_, peakDb_ - 0.55f);

    if (peakDb_ < -99.0f)
        peakDb_ = -100.0f;

    repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    const auto full = getLocalBounds().toFloat();
    Palette::drawWell (g, full, 6.0f);

    const auto inner = full.reduced (2.5f, 2.5f);
    const auto norm = juce::jlimit (0.0f, 1.0f, (levelDb_ + 60.0f) / 66.0f);

    if (levelDb_ > -99.0f && norm > 0.001f)
    {
        const auto fill = inner.withTop (inner.getBottom() - inner.getHeight() * norm);
        const auto colour = levelDb_ > -1.0f ? Palette::warning
                                             : (levelDb_ > -6.0f ? Palette::accent : Palette::safe);

        // Emissive bloom above the crest.
        Palette::drawGlow (g, fill, 4.0f, colour, 0.5f);

        juce::ColourGradient grad (colour.brighter (0.22f), 0.0f, fill.getY(),
                                   colour.darker (0.15f), 0.0f, fill.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (fill, 4.0f);

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawHorizontalLine ((int) fill.getY() + 1, fill.getX() + 2.0f, fill.getRight() - 2.0f);
    }

    // dB scale ticks.
    g.setColour (juce::Colours::white.withAlpha (0.09f));

    for (float db : { -12.0f, -24.0f, -36.0f, -48.0f })
    {
        const auto n = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 66.0f);
        const auto y = inner.getBottom() - inner.getHeight() * n;
        g.fillRect (inner.getX(), y, inner.getWidth(), 1.0f);
    }

    if (peakDb_ > -99.0f)
    {
        const auto pnorm = juce::jlimit (0.0f, 1.0f, (peakDb_ + 60.0f) / 66.0f);
        const auto y = inner.getBottom() - inner.getHeight() * pnorm;
        g.setColour (juce::Colours::white.withAlpha (0.20f));
        g.fillRect (inner.getX(), y - 2.0f, inner.getWidth(), 4.0f);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.fillRect (inner.getX(), y - 1.0f, inner.getWidth(), 2.0f);
    }
}

} // namespace voiceguard::ui
