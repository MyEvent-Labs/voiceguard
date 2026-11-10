#include "ui/LevelMeter.h"

#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

void LevelMeter::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour (Palette::panelDark);
    g.fillRoundedRectangle (bounds, 3.0f);

    if (levelDb_ <= -99.0f)
        return;

    const float norm = juce::jlimit (0.0f, 1.0f, juce::jmap (levelDb_, -60.0f, 6.0f, 0.0f, 1.0f));

    g.setColour (levelDb_ > -1.0f ? Palette::warning : Palette::safe);
    g.fillRoundedRectangle (bounds.withWidth (bounds.getWidth() * norm), 3.0f);
}

} // namespace voiceguard::ui
