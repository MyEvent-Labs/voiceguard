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
    auto nameArea = r.removeFromLeft (66);
    auto valueArea = r.removeFromRight (62);
    auto track = r.reduced (0, 6);

    g.setColour (Palette::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
    g.drawText (name_, nameArea, juce::Justification::centredLeft);

    Palette::drawWell (g, track.toFloat(), 4.0f);

    auto trackF = track.toFloat().reduced (2.0f, 2.0f);

    if (bipolar_)
    {
        const float centre = trackF.getCentreX();
        const float half = trackF.getWidth() * 0.5f;
        const float t = juce::jlimit (-1.0f, 1.0f, db_ / maxDb_);

        juce::Rectangle<float> fill;

        if (t >= 0.0f)
            fill = trackF.withLeft (centre).withWidth (half * t);
        else
            fill = trackF.withRight (centre).withLeft (centre - half * -t);

        const auto colour = t < 0.0f ? Palette::warning : Palette::safe;
        juce::ColourGradient grad (colour.brighter (0.16f), 0.0f, fill.getY(),
                                   colour.darker (0.10f), 0.0f, fill.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (fill, 3.0f);

        g.setColour (Palette::outlineBright);
        g.fillRect (centre - 0.5f, trackF.getY(), 1.0f, trackF.getHeight());
    }
    else if (db_ > 0.02f)
    {
        const float t = juce::jlimit (0.0f, 1.0f, db_ / maxDb_);
        auto fill = trackF.withWidth (trackF.getWidth() * t);
        const auto colour = t > 0.85f ? Palette::warning : Palette::accent;
        juce::ColourGradient grad (colour.brighter (0.16f), 0.0f, fill.getY(),
                                   colour.darker (0.10f), 0.0f, fill.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (fill, 3.0f);
    }

    g.setColour (Palette::text);
    g.setFont (juce::Font (juce::FontOptions (11.5f, juce::Font::bold)));

    juce::String text;
    if (bipolar_)
        text = juce::String::formatted ("%+.1f", db_);
    else if (std::abs (db_) < 0.05f)
        text = "0.0";
    else
        text = juce::String::formatted ("-%.1f", db_);

    g.drawText (text, valueArea, juce::Justification::centredRight, false);
}

} // namespace voiceguard::ui
