#include "ui/ReductionMeter.h"

#include "ui/VoiceGuardLookAndFeel.h"

namespace voiceguard::ui
{

ReductionMeter::ReductionMeter (const juce::String& name, bool bipolar, float maxDb)
    : name_ (name), bipolar_ (bipolar), maxDb_ (juce::jmax (1.0f, maxDb))
{
}

void ReductionMeter::setDb (float dbDb)
{
    const float clamped = bipolar_ ? juce::jlimit (-maxDb_, maxDb_, dbDb)
                                   : juce::jlimit (0.0f, maxDb_, dbDb);

    if (std::abs (clamped - db_) > 0.05f)
    {
        db_ = clamped;
        repaint();
    }
}

void ReductionMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds();
    auto nameArea = r.removeFromLeft (64);
    auto valueArea = r.removeFromRight (60);
    auto track = r.reduced (2, 5);

    g.setColour (Palette::textDim);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText (name_, nameArea, juce::Justification::centredLeft);

    g.setColour (Palette::panelDark);
    g.fillRoundedRectangle (track.toFloat(), 3.0f);

    if (bipolar_)
    {
        const int centre = track.getCentreX();
        const float half = (float) track.getWidth() * 0.5f;
        const float t = juce::jlimit (-1.0f, 1.0f, db_ / maxDb_);

        juce::Rectangle<int> fill;

        if (t >= 0.0f)
            fill = track.withLeft (centre).withWidth ((int) (half * t));
        else
            fill = track.withRight (centre).withLeft (centre - (int) (half * -t));

        g.setColour (t < 0.0f ? Palette::warning : Palette::safe);
        g.fillRoundedRectangle (fill.toFloat(), 3.0f);

        g.setColour (Palette::outline);
        g.fillRect (centre, track.getY(), 1, track.getHeight());
    }
    else
    {
        const float t = juce::jlimit (0.0f, 1.0f, db_ / maxDb_);
        auto fill = track.withWidth ((int) ((float) track.getWidth() * t));

        g.setColour (t > 0.85f ? Palette::warning : Palette::accent);
        g.fillRoundedRectangle (fill.toFloat(), 3.0f);
    }

    g.setColour (Palette::text);

    juce::String text;

    if (bipolar_)
        text = juce::String::formatted ("%+.1f dB", db_);
    else if (std::abs (db_) < 0.05f)
        text = "0.0 dB";
    else
        text = juce::String::formatted ("-%.1f dB", db_);

    g.drawText (text, valueArea, juce::Justification::centredRight);
}

} // namespace voiceguard::ui
