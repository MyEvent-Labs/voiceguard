#include "dsp/Parameters.h"

namespace voiceguard::params
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using Choice = juce::AudioParameterChoice;
    using Float = juce::AudioParameterFloat;
    using Bool = juce::AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto ranged = [] (float lo, float hi, float def, float step = 0.0f)
    {
        return juce::NormalisableRange<float> (lo, hi, step);
    };

    layout.add (std::make_unique<Bool> (juce::ParameterID { bypass, 1 }, "Bypass", false));
    layout.add (std::make_unique<Bool> (juce::ParameterID { autoMode, 1 }, "AUTO", false));
    layout.add (std::make_unique<Choice> (juce::ParameterID { autoState, 1 }, "AUTO State",
                                          juce::StringArray { "Idle", "Listening", "Ready" }, 0));

    layout.add (std::make_unique<Float> (juce::ParameterID { macroClean, 1 }, "CLEAN",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { macroClarity, 1 }, "CLARITY",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { macroLevel, 1 }, "LEVEL",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { macroProtect, 1 }, "PROTECT",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));

    layout.add (std::make_unique<Float> (juce::ParameterID { inputTrimDb, 1 }, "Input Trim",
                                         ranged (-18.0f, 18.0f, 0.0f, 0.01f), 0.0f));
    layout.add (std::make_unique<Choice> (juce::ParameterID { hpfHz, 1 }, "High-Pass",
                                          juce::StringArray { "Off", "60 Hz", "80 Hz", "100 Hz",
                                                              "120 Hz", "160 Hz" },
                                          2));

    layout.add (std::make_unique<Float> (juce::ParameterID { cleanAmount, 1 }, "Clean Amount",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { cleanAdaptation, 1 }, "Clean Adaptation",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));

    layout.add (std::make_unique<Float> (juce::ParameterID { eqStrength, 1 }, "EQ Strength",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { deEsserAmount, 1 }, "De-Esser",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { harshnessAmount, 1 }, "Harshness",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));

    layout.add (std::make_unique<Float> (juce::ParameterID { levelTargetDb, 1 }, "Level Target",
                                         ranged (-30.0f, -6.0f, -18.0f, 0.1f), -18.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { levelRangeDb, 1 }, "Level Range",
                                         ranged (0.0f, 6.0f, 6.0f, 0.1f), 6.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { levelSpeed, 1 }, "Level Speed",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));

    layout.add (std::make_unique<Float> (juce::ParameterID { dynThresholdDb, 1 }, "Dynamics Threshold",
                                         ranged (-40.0f, 0.0f, -18.0f, 0.1f), -18.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { dynRatio, 1 }, "Dynamics Ratio",
                                         ranged (1.0f, 6.0f, 2.0f, 0.01f), 2.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { dynAttackMs, 1 }, "Dynamics Attack",
                                         ranged (1.0f, 100.0f, 20.0f, 0.1f), 20.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { dynReleaseMs, 1 }, "Dynamics Release",
                                         ranged (50.0f, 1000.0f, 250.0f, 1.0f), 250.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { dynKneeDb, 1 }, "Dynamics Knee",
                                         ranged (0.0f, 18.0f, 12.0f, 0.1f), 12.0f));

    layout.add (std::make_unique<Float> (juce::ParameterID { protectAmount, 1 }, "Protection",
                                         ranged (0.0f, 1.0f, 0.5f), 0.5f));
    layout.add (std::make_unique<Float> (juce::ParameterID { outputCeilingDb, 1 }, "Output Ceiling",
                                         ranged (-6.0f, 0.0f, -1.0f, 0.1f), -1.0f));
    layout.add (std::make_unique<Float> (juce::ParameterID { outputTrimDb, 1 }, "Output Trim",
                                         ranged (-18.0f, 18.0f, 0.0f, 0.01f), 0.0f));

    layout.add (std::make_unique<Bool> (juce::ParameterID { deFeedbackEnable, 1 }, "De-Feedback Interface", false));

    return layout;
}

Parameters Parameters::fromApvts (const juce::AudioProcessorValueTreeState& apvts)
{
    const auto value = [&apvts] (const char* id, float fallback)
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();

        return fallback;
    };

    Parameters p;

    p.bypass = value (params::bypass, 0.0f) > 0.5f;
    p.autoMode = value (params::autoMode, 0.0f) > 0.5f;
    p.autoState = (int) value (params::autoState, 0.0f);

    p.macroClean = value (params::macroClean, 0.5f);
    p.macroClarity = value (params::macroClarity, 0.5f);
    p.macroLevel = value (params::macroLevel, 0.5f);
    p.macroProtect = value (params::macroProtect, 0.5f);

    p.inputTrimDb = value (params::inputTrimDb, 0.0f);
    p.hpfIndex = (int) value (params::hpfHz, 2.0f);

    p.cleanAmount = value (params::cleanAmount, 0.5f);
    p.cleanAdaptation = value (params::cleanAdaptation, 0.5f);

    p.eqStrength = value (params::eqStrength, 0.5f);
    p.deEsserAmount = value (params::deEsserAmount, 0.5f);
    p.harshnessAmount = value (params::harshnessAmount, 0.5f);

    p.levelTargetDb = value (params::levelTargetDb, -18.0f);
    p.levelRangeDb = value (params::levelRangeDb, 6.0f);
    p.levelSpeed = value (params::levelSpeed, 0.5f);

    p.dynThresholdDb = value (params::dynThresholdDb, -18.0f);
    p.dynRatio = value (params::dynRatio, 2.0f);
    p.dynAttackMs = value (params::dynAttackMs, 20.0f);
    p.dynReleaseMs = value (params::dynReleaseMs, 250.0f);
    p.dynKneeDb = value (params::dynKneeDb, 12.0f);

    p.protectAmount = value (params::protectAmount, 0.5f);
    p.outputCeilingDb = value (params::outputCeilingDb, -1.0f);
    p.outputTrimDb = value (params::outputTrimDb, 0.0f);

    p.deFeedbackEnable = value (params::deFeedbackEnable, 0.0f) > 0.5f;

    return p;
}

} // namespace voiceguard::params
