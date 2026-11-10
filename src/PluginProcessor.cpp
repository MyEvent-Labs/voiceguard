#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace voiceguard
{

VoiceGuardProcessor::VoiceGuardProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void VoiceGuardProcessor::prepareToPlay (double, int)
{
}

void VoiceGuardProcessor::releaseResources()
{
}

bool VoiceGuardProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    // Mono -> mono, stereo -> stereo, and the common host-forced mono -> stereo.
    return in == out || (in == juce::AudioChannelSet::mono() && out == juce::AudioChannelSet::stereo());
}

void VoiceGuardProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Temporary passthrough scaffold; replaced by the VoiceGuardEngine chain.
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* VoiceGuardProcessor::createEditor()
{
    return new VoiceGuardEditor (*this);
}

void VoiceGuardProcessor::getStateInformation (juce::MemoryBlock&)
{
}

void VoiceGuardProcessor::setStateInformation (const void*, int)
{
}

} // namespace voiceguard
