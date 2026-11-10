#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "state/StateIO.h"

namespace voiceguard
{

namespace
{
// Cap the state blob so a corrupt or hostile host value cannot exhaust memory.
constexpr int kMaxStateBytes = 1 << 20; // 1 MiB
} // namespace

VoiceGuardProcessor::VoiceGuardProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, juce::Identifier (kStateRootType), params::createParameterLayout())
{
}

void VoiceGuardProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int channels = juce::jmax (1, juce::jmax (getTotalNumInputChannels(),
                                                    getTotalNumOutputChannels()));
    engine_.prepare (sampleRate, samplesPerBlock, channels);
    engine_.reset();
    setLatencySamples (engine_.latencySamples());
}

void VoiceGuardProcessor::releaseResources()
{
    engine_.reset();
}

bool VoiceGuardProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return in == out || (in == juce::AudioChannelSet::mono() && out == juce::AudioChannelSet::stereo());
}

void VoiceGuardProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();

    if (numSamples <= 0)
        return;

    // Host-forced mono -> stereo: duplicate, and clear any extra output.
    if (numIn == 1 && numOut == 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, numSamples);

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);

    engine_.setParameters (params::Parameters::fromApvts (apvts));
    engine_.process (buffer);
}

juce::AudioProcessorEditor* VoiceGuardProcessor::createEditor()
{
    return new VoiceGuardEditor (*this);
}

void VoiceGuardProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    const auto xml = stateToXml (apvts.copyState());
    destData.replaceAll (xml.toRawUTF8(), xml.getNumBytesAsUTF8());
}

void VoiceGuardProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0 || sizeInBytes > kMaxStateBytes)
        return;

    const juce::String text (juce::String::fromUTF8 (static_cast<const char*> (data), sizeInBytes));
    juce::Result result = juce::Result::ok();
    const auto tree = stateFromXml (text, result);

    if (! result.wasOk() || ! tree.isValid())
        return; // fail toward the current (safe) state

    apvts.replaceState (tree);
}

} // namespace voiceguard

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new voiceguard::VoiceGuardProcessor();
}
