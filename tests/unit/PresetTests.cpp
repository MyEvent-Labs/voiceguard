#include "../TestRegistry.h"

#include "dsp/Parameters.h"
#include "state/Presets.h"
#include "state/StateIO.h"

using namespace voiceguard;

namespace
{
class DummyProcessor : public juce::AudioProcessor
{
public:
    DummyProcessor()
        : juce::AudioProcessor (BusesProperties()
                                    .withInput ("In", juce::AudioChannelSet::stereo(), true)
                                    .withOutput ("Out", juce::AudioChannelSet::stereo(), true))
    {
    }

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    const juce::String getName() const override { return "Dummy"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
};
} // namespace

VG_TEST (factoryPresetsAreCompleteAndUnique)
{
    const auto& presets = factoryPresets();
    VG_CHECK (presets.size() == 10);

    const char* expected[] = {
        "Auto", "Lectern Cleanup", "Wireless Lav", "Handheld Speech", "Panel Discussion",
        "Broadcast Voice", "Streaming", "Noisy Room", "Bright Lavalier", "Quiet Presenter"
    };

    for (size_t i = 0; i < presets.size(); ++i)
    {
        VG_CHECK (presets[i].name == expected[i]);

        for (size_t j = i + 1; j < presets.size(); ++j)
            VG_CHECK (presets[i].name != presets[j].name);
    }
}

VG_TEST (everyPresetAppliesToTheTree)
{
    DummyProcessor dummy;
    juce::AudioProcessorValueTreeState apvts (dummy, nullptr, kStateRootType, params::createParameterLayout());

    for (const auto& preset : factoryPresets())
    {
        applyPreset (apvts, preset);
        const auto p = params::Parameters::fromApvts (apvts);

        VG_CHECK_CLOSE (p.macroClean, preset.macroClean, 1.0e-4);
        VG_CHECK_CLOSE (p.macroClarity, preset.macroClarity, 1.0e-4);
        VG_CHECK_CLOSE (p.macroLevel, preset.macroLevel, 1.0e-4);
        VG_CHECK_CLOSE (p.macroProtect, preset.macroProtect, 1.0e-4);

        VG_CHECK (p.hpfIndex == preset.hpfIndex);
        VG_CHECK_CLOSE (p.cleanAmount, preset.cleanAmount, 1.0e-4);
        VG_CHECK_CLOSE (p.eqStrength, preset.eqStrength, 1.0e-4);
        VG_CHECK_CLOSE (p.deEsserAmount, preset.deEsserAmount, 1.0e-4);
        VG_CHECK_CLOSE (p.harshnessAmount, preset.harshnessAmount, 1.0e-4);
        VG_CHECK_CLOSE (p.levelTargetDb, preset.levelTargetDb, 0.02);

        // Applying a preset must not leave the plugin bypassed or in AUTO.
        VG_CHECK (! p.bypass);
        VG_CHECK (! p.autoMode);
    }
}

VG_TEST (presetStateRoundTrips)
{
    DummyProcessor dummy;
    juce::AudioProcessorValueTreeState apvts (dummy, nullptr, kStateRootType, params::createParameterLayout());

    applyPreset (apvts, factoryPresets()[8]); // Bright Lavalier

    const auto xml = stateToXml (apvts.copyState());
    juce::Result result = juce::Result::ok();
    const auto restored = stateFromXml (xml, result);

    VG_CHECK (result.wasOk());
    VG_CHECK (restored.isValid());

    apvts.replaceState (restored);
    const auto p = params::Parameters::fromApvts (apvts);

    VG_CHECK_CLOSE (p.deEsserAmount, factoryPresets()[8].deEsserAmount, 1.0e-4);
    VG_CHECK_CLOSE (p.harshnessAmount, factoryPresets()[8].harshnessAmount, 1.0e-4);
}
