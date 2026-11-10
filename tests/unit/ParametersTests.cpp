#include "../TestRegistry.h"

#include "dsp/Parameters.h"

using namespace voiceguard;

namespace
{
/** Bare AudioProcessor used only to host an APVTS in these tests. */
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

VG_TEST (parameterLayoutHasAllStableIds)
{
    DummyProcessor dummy;
    juce::AudioProcessorValueTreeState apvts (dummy, nullptr, "state", params::createParameterLayout());

    const char* ids[] = {
        params::bypass, params::autoMode, params::autoState,
        params::macroClean, params::macroClarity, params::macroLevel, params::macroProtect,
        params::inputTrimDb, params::hpfHz,
        params::cleanAmount, params::cleanAdaptation,
        params::eqStrength, params::deEsserAmount, params::harshnessAmount,
        params::levelTargetDb, params::levelRangeDb, params::levelSpeed,
        params::dynThresholdDb, params::dynRatio, params::dynAttackMs, params::dynReleaseMs, params::dynKneeDb,
        params::protectAmount, params::outputCeilingDb, params::outputTrimDb,
        params::deFeedbackEnable
    };

    for (auto* id : ids)
        VG_CHECK (apvts.getParameter (id) != nullptr);
}

VG_TEST (parameterRangesAndDefaults)
{
    DummyProcessor dummy;
    juce::AudioProcessorValueTreeState apvts (dummy, nullptr, "state", params::createParameterLayout());

    const auto trimRange = apvts.getParameter (params::inputTrimDb)->getNormalisableRange();
    VG_CHECK_CLOSE (trimRange.start, -18.0f, 1.0e-6);
    VG_CHECK_CLOSE (trimRange.end, 18.0f, 1.0e-6);

    VG_CHECK_CLOSE (apvts.getRawParameterValue (params::inputTrimDb)->load(), 0.0f, 1.0e-6);
    VG_CHECK_CLOSE (apvts.getRawParameterValue (params::levelTargetDb)->load(), -18.0f, 1.0e-6);
    VG_CHECK_CLOSE (apvts.getRawParameterValue (params::dynRatio)->load(), 2.0f, 1.0e-6);

    // Default HPF is the conservative 80 Hz (index 2).
    VG_CHECK_CLOSE (apvts.getRawParameterValue (params::hpfHz)->load(), 2.0f, 1.0e-6);
    VG_CHECK (params::hpfFrequenciesHz[2] == 80.0f);
}

VG_TEST (parametersSnapshotMatchesApvts)
{
    DummyProcessor dummy;
    juce::AudioProcessorValueTreeState apvts (dummy, nullptr, "state", params::createParameterLayout());

    // +6 dB on a -18..18 range is normalised 0.6667.
    apvts.getParameter (params::inputTrimDb)->setValueNotifyingHost (0.6666667f);
    apvts.getParameter (params::cleanAmount)->setValueNotifyingHost (0.8f);
    apvts.getParameter (params::bypass)->setValueNotifyingHost (1.0f);

    const auto p = params::Parameters::fromApvts (apvts);
    VG_CHECK_CLOSE (p.inputTrimDb, 6.0f, 0.02);
    VG_CHECK_CLOSE (p.cleanAmount, 0.8f, 0.01);
    VG_CHECK (p.bypass);
    VG_CHECK_CLOSE (p.macroClean, 0.5f, 1.0e-6);
    VG_CHECK_CLOSE (p.hpfFrequencyHz(), 80.0f, 1.0e-6);
}
