#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "ProductInfo.h"

namespace voiceguard
{

/** MyEvent Labs VoiceGuard - live speech enhancement.

    v0.0.1 Alpha scaffold. The full DSP engine, parameter tree and AUTO
    controller are added by their own tasks; this stage keeps a valid,
    click-free passthrough so the plugin targets build and validate.
*/
class VoiceGuardProcessor : public juce::AudioProcessor
{
public:
    VoiceGuardProcessor();
    ~VoiceGuardProcessor() override = default;

    // --- AudioProcessor -----------------------------------------------------
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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceGuardProcessor)
};

} // namespace voiceguard
