#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

namespace voiceguard::ui
{

/** A live input-level ribbon: a rolling area graph of recent level with a
    soft activity glow. The signature graphic of the interface. */
class SignalRibbon : public juce::Component
{
public:
    SignalRibbon() = default;

    /** Push one level sample in dB (-60..0) and the current speech activity. */
    void push (float levelDb, float activity01);

    void paint (juce::Graphics&) override;

private:
    static constexpr int kPoints = 220;
    std::array<float, kPoints> history_ {};
    int write_ = 0;
    float activity_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalRibbon)
};

} // namespace voiceguard::ui
