#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** Horizontal gain-reduction bar: a stage name, a filled bar and a value.

    Unipolar (default) shows reduction from 0 to maxDb. Bipolar (the level
    ride) grows out from the centre and accepts signed dB. */
class ReductionMeter : public juce::Component
{
public:
    ReductionMeter (const juce::String& name, bool bipolar = false, float maxDb = 12.0f);

    void setDb (float dbDb);

    void paint (juce::Graphics&) override;

private:
    juce::String name_;
    bool bipolar_;
    float maxDb_;
    float db_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReductionMeter)
};

} // namespace voiceguard::ui
