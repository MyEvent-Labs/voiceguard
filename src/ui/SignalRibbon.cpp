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
    Palette::drawWell (g, bounds, 7.0f);

    auto inner = bounds.reduced (6.0f, 5.0f);

    // Grid thirds.
    g.setColour (Palette::outline.withAlpha (0.5f));
    for (int i = 1; i < 3; ++i)
    {
        const auto y = inner.getY() + inner.getHeight() * (float) i / 3.0f;
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
        const auto y = inner.getBottom() - inner.getHeight() * value;

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

    const auto accent = Palette::accent.interpolatedWith (Palette::safe, juce::jlimit (0.0f, 1.0f, activity_));

    juce::ColourGradient fill (accent.withAlpha (0.42f), inner.getX(), inner.getY(),
                               accent.withAlpha (0.02f), inner.getX(), inner.getBottom(), false);
    g.setGradientFill (fill);
    g.fillPath (area);

    g.setColour (accent.withAlpha (0.95f));
    g.strokePath (line, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
}

} // namespace voiceguard::ui
