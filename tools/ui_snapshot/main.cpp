#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <cmath>

/** Renders the VoiceGuard editor offscreen to a PNG so the layout can be
    reviewed without a host. Usage: voiceguard_ui_snapshot [out.png] */
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;

    voiceguard::VoiceGuardProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    // Feed a few seconds of speech-like signal so the meters and readouts are
    // populated in the snapshot.
    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;

    for (int iter = 0; iter < 600; ++iter)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 512; ++i)
            {
                const double t = (double) (iter * 512 + i) / 48000.0;
                const double am = 0.5 + 0.5 * std::sin (juce::MathConstants<double>::twoPi * 4.0 * t);
                const double s = 0.25 * std::sin (juce::MathConstants<double>::twoPi * 250.0 * t)
                                 + 0.12 * std::sin (juce::MathConstants<double>::twoPi * 1400.0 * t)
                                 + 0.02 * std::sin (juce::MathConstants<double>::twoPi * 6000.0 * t);
                buffer.setSample (ch, i, (float) (s * am));
            }
        }

        processor.processBlock (buffer, midi);
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    editor->setSize (980, 540);

    if (auto* vg = dynamic_cast<voiceguard::VoiceGuardEditor*> (editor.get()))
        vg->refreshReadouts();

    const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);

    const auto file = juce::File::getCurrentWorkingDirectory()
                          .getChildFile (argc > 1 ? argv[1] : "ui-snapshot.png");

    juce::PNGImageFormat png;

    if (auto stream = std::unique_ptr<juce::FileOutputStream> (file.createOutputStream()))
    {
        png.writeImageToStream (image, *stream);
        std::cout << "wrote " << file.getFullPathName() << "  (" << image.getWidth()
                  << "x" << image.getHeight() << ")\n";
    }

    return 0;
}
