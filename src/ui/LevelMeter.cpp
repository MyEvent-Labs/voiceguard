#include "ui/LevelMeter.h"

#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

void LevelMeter::setLevelDb (float db)
{
    levelDb_ = juce::jlimit (-100.0f, 6.0f, db);
    peakDb_ = juce::jmax (levelDb_, peakDb_ - 0.6f);

    if (peakDb_ < -99.0f)
        peakDb_ = -100.0f;

    repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    const auto full = getLocalBounds().toFloat();

    g.setColour (Palette::panelDark);
    g.fillRoundedRectangle (full, 3.0f);
    g.setColour (Palette::outline);
    g.drawRoundedRectangle (full, 3.0f, 1.0f);

    if (levelDb_ <= -99.0f)
        return;

    const auto norm = juce::jlimit (0.0f, 1.0f, (levelDb_ + 60.0f) / 66.0f);
    const auto fill = full.withTop (full.getBottom() - full.getHeight() * norm);

    g.setColour (levelDb_ > -1.0f ? Palette::warning : Palette::safe);
    g.fillRoundedRectangle (fill, 3.0f);

    if (peakDb_ > -99.0f)
    {
        const auto pnorm = juce::jlimit (0.0f, 1.0f, (peakDb_ + 60.0f) / 66.0f);
        const auto y = full.getBottom() - full.getHeight() * pnorm;
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.fillRect (full.getX(), y - 1.0f, full.getWidth(), 2.0f);
    }
}

} // namespace voiceguard::ui
