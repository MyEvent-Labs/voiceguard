#include "PluginEditor.h"

namespace voiceguard
{

VoiceGuardEditor::VoiceGuardEditor (VoiceGuardProcessor& processor)
    : juce::AudioProcessorEditor (processor),
      processor_ (processor)
{
    title_.setText (juce::String (product::productName) + "  " + product::versionTag + " ALPHA",
                    juce::dontSendNotification);
    title_.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title_);

    setSize (720, 420);
}

void VoiceGuardEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff14161a));
}

void VoiceGuardEditor::resized()
{
    title_.setBounds (getLocalBounds().reduced (16).removeFromTop (28));
}

} // namespace voiceguard
