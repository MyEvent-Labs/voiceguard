#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "ProductInfo.h"
#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"

namespace voiceguard
{

/** MyEvent Labs VoiceGuard - live speech enhancement.

    Owns the APVTS (26 stable parameters), the DSP engine and the versioned
    state IO. The audio thread only reads raw parameter atomics and runs the
    engine; no allocation, locks or IO. */
class VoiceGuardProcessor : public juce::AudioProcessor
{
public:
    VoiceGuardProcessor();
    ~VoiceGuardProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return product::productName; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    UiSnapshot& snapshot() noexcept { return engine_.snapshot(); }

private:
    VoiceGuardEngine engine_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceGuardProcessor)
};

} // namespace voiceguard
