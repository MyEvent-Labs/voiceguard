#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** Simple horizontal level meter, -60..+6 dB. */
class LevelMeter : public juce::Component
{
public:
    LevelMeter() = default;

    void setLevelDb (float db)
    {
        const float clamped = juce::jlimit (-100.0f, 6.0f, db);

        if (std::abs (clamped - levelDb_) > 0.05f)
        {
            levelDb_ = clamped;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override;

private:
    float levelDb_ = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

} // namespace voiceguard::ui
