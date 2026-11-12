#include "ui/SignalRibbon.h"

#include "dsp/DspUtils.h"
#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

void SignalRibbon::push (float levelDb, float activity01)
{
    history_[(size_t) write_] = juce::jlimit (0.0f, 1.0f, (levelDb + 60.0f) / 60.0f);
    write_ = (write_ + 1) % kPoints;
    activity_ = activity01;
    repaint();
}

void SignalRibbon::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    Palette::drawWell (g, bounds, 9.0f);

    auto inner = bounds.reduced (8.0f, 7.0f);

    // Horizon grid.
    g.setColour (Palette::outline.withAlpha (0.45f));
    for (int i = 1; i < 4; ++i)
    {
        const auto y = inner.getY() + inner.getHeight() * (float) i / 4.0f;
        g.drawHorizontalLine ((int) y, inner.getX(), inner.getRight());
    }

    if (inner.getWidth() < 4.0f)
        return;

    juce::Path area;
    juce::Path line;
    const auto dx = inner.getWidth() / (float) (kPoints - 1);

    for (int i = 0; i < kPoints; ++i)
    {
        const int index = (write_ + i) % kPoints;
        const auto value = history_[(size_t) index];
        const auto x = inner.getX() + dx * (float) i;
        const auto y = inner.getBottom() - inner.getHeight() * juce::jmax (0.02f, value);

        if (i == 0)
        {
            area.startNewSubPath (x, inner.getBottom());
            line.startNewSubPath (x, y);
        }

        area.lineTo (x, y);
        line.lineTo (x, y);
    }

    area.lineTo (inner.getRight(), inner.getBottom());
    area.closeSubPath();

    const auto warm = Palette::accent.interpolatedWith (Palette::safe, juce::jlimit (0.0f, 1.0f, activity_));

    // Aurora fill: cyan at the crest, violet sinking to transparent.
    juce::ColourGradient fill (warm.withAlpha (0.50f), inner.getX(), inner.getY(),
                               Palette::accent2.withAlpha (0.02f), inner.getX(), inner.getBottom(), false);
    g.setGradientFill (fill);
    g.fillPath (area);

    // Emissive line: three widening strokes for a bloom.
    const auto lineWidth = [&] (float scale, float alpha)
    {
        g.setColour (warm.withAlpha (alpha));
        g.strokePath (line, juce::PathStrokeType (1.8f * scale, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    };

    lineWidth (4.0f, 0.05f);
    lineWidth (2.6f, 0.10f);
    lineWidth (1.0f, 0.95f);

    // A cool accent underglow at the base of the well.
    g.setColour (Palette::accent.withAlpha (0.06f));
    g.drawHorizontalLine ((int) inner.getBottom(), inner.getX(), inner.getRight());
}

} // namespace voiceguard::ui
