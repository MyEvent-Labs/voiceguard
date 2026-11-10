#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** Vertical level meter, -60..+6 dB, with a peak-hold line. */
class LevelMeter : public juce::Component
{
public:
    LevelMeter() = default;

    void setLevelDb (float db);

    void paint (juce::Graphics&) override;

private:
    float levelDb_ = -100.0f;
    float peakDb_ = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

} // namespace voiceguard::ui
