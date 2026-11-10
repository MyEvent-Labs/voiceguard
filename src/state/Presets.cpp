#include "state/Presets.h"

#include "dsp/Parameters.h"

namespace voiceguard
{

namespace
{
void setParam (juce::AudioProcessorValueTreeState& apvts, const char* id, float value)
{
    if (auto* parameter = apvts.getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}

Preset make (const char* name)
{
    Preset p;
    p.name = name;
    return p;
}
} // namespace

const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> presets = []
    {
        std::vector<Preset> list;

        list.push_back (make ("Auto"));

        {
            auto p = make ("Lectern Cleanup");
            p.macroClean = 0.75f; p.macroClarity = 0.55f; p.macroLevel = 0.5f; p.macroProtect = 0.6f;
            p.cleanAmount = 0.7f; p.eqStrength = 0.5f; p.deEsserAmount = 0.4f; p.harshnessAmount = 0.5f;
            p.hpfIndex = 2; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Wireless Lav");
            p.macroClean = 0.6f; p.macroClarity = 0.65f; p.macroLevel = 0.55f; p.macroProtect = 0.5f;
            p.cleanAmount = 0.6f; p.eqStrength = 0.6f; p.deEsserAmount = 0.65f; p.harshnessAmount = 0.6f;
            p.hpfIndex = 2; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Handheld Speech");
            p.macroClean = 0.55f; p.macroClarity = 0.7f; p.macroLevel = 0.55f; p.macroProtect = 0.6f;
            p.cleanAmount = 0.55f; p.eqStrength = 0.55f; p.deEsserAmount = 0.5f; p.harshnessAmount = 0.7f;
            p.hpfIndex = 3; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Panel Discussion");
            p.macroClean = 0.65f; p.macroClarity = 0.55f; p.macroLevel = 0.5f; p.macroProtect = 0.55f;
            p.cleanAmount = 0.65f; p.eqStrength = 0.5f; p.deEsserAmount = 0.5f; p.harshnessAmount = 0.5f;
            p.levelRangeDb = 4.0f; p.dynRatio = 2.5f; p.hpfIndex = 2;
            list.push_back (p);
        }
        {
            auto p = make ("Broadcast Voice");
            p.macroClean = 0.5f; p.macroClarity = 0.65f; p.macroLevel = 0.6f; p.macroProtect = 0.55f;
            p.eqStrength = 0.6f; p.deEsserAmount = 0.5f; p.harshnessAmount = 0.55f;
            p.levelTargetDb = -16.0f; p.dynRatio = 3.0f; p.dynThresholdDb = -20.0f; p.hpfIndex = 2;
            list.push_back (p);
        }
        {
            auto p = make ("Streaming");
            p.macroClean = 0.7f; p.macroClarity = 0.55f; p.macroLevel = 0.55f; p.macroProtect = 0.5f;
            p.cleanAmount = 0.7f; p.cleanAdaptation = 0.6f; p.eqStrength = 0.5f;
            p.hpfIndex = 2; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Noisy Room");
            p.macroClean = 0.85f; p.macroClarity = 0.55f; p.macroLevel = 0.55f; p.macroProtect = 0.6f;
            p.cleanAmount = 0.8f; p.cleanAdaptation = 0.75f; p.eqStrength = 0.5f;
            p.hpfIndex = 3; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Bright Lavalier");
            p.macroClean = 0.55f; p.macroClarity = 0.75f; p.macroLevel = 0.5f; p.macroProtect = 0.5f;
            p.deEsserAmount = 0.7f; p.harshnessAmount = 0.7f; p.eqStrength = 0.55f;
            p.hpfIndex = 2; p.levelTargetDb = -18.0f;
            list.push_back (p);
        }
        {
            auto p = make ("Quiet Presenter");
            p.macroClean = 0.6f; p.macroClarity = 0.55f; p.macroLevel = 0.85f; p.macroProtect = 0.55f;
            p.cleanAmount = 0.6f; p.eqStrength = 0.5f; p.levelTargetDb = -20.0f; p.levelRangeDb = 6.0f;
            p.hpfIndex = 2;
            list.push_back (p);
        }

        return list;
    }();

    return presets;
}

void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& p)
{
    setParam (apvts, params::bypass, 0.0f);
    setParam (apvts, params::autoMode, 0.0f);

    setParam (apvts, params::macroClean, p.macroClean);
    setParam (apvts, params::macroClarity, p.macroClarity);
    setParam (apvts, params::macroLevel, p.macroLevel);
    setParam (apvts, params::macroProtect, p.macroProtect);

    setParam (apvts, params::hpfHz, (float) p.hpfIndex);
    setParam (apvts, params::inputTrimDb, p.inputTrimDb);

    setParam (apvts, params::cleanAmount, p.cleanAmount);
    setParam (apvts, params::cleanAdaptation, p.cleanAdaptation);
    setParam (apvts, params::eqStrength, p.eqStrength);
    setParam (apvts, params::deEsserAmount, p.deEsserAmount);
    setParam (apvts, params::harshnessAmount, p.harshnessAmount);

    setParam (apvts, params::levelTargetDb, p.levelTargetDb);
    setParam (apvts, params::levelRangeDb, p.levelRangeDb);
    setParam (apvts, params::levelSpeed, p.levelSpeed);

    setParam (apvts, params::dynThresholdDb, p.dynThresholdDb);
    setParam (apvts, params::dynRatio, p.dynRatio);
    setParam (apvts, params::dynAttackMs, p.dynAttackMs);
    setParam (apvts, params::dynReleaseMs, p.dynReleaseMs);
    setParam (apvts, params::dynKneeDb, p.dynKneeDb);

    setParam (apvts, params::protectAmount, p.protectAmount);
    setParam (apvts, params::outputCeilingDb, p.outputCeilingDb);
    setParam (apvts, params::outputTrimDb, p.outputTrimDb);
}

} // namespace voiceguard
