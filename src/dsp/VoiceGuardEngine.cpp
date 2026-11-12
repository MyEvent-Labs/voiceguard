#include "dsp/VoiceGuardEngine.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

namespace
{
// Convex macro curve: the default (0.5) stays gentle and transparent, while
// the top of the travel reaches full effect. Turning a macro is clearly
// audible without making the default sound over-processed.
float macroGain (float macro01) noexcept
{
    const float m = juce::jlimit (0.0f, 1.0f, macro01);
    return juce::jlimit (0.0f, 2.0f, 2.0f * std::pow (m, 1.585f));
}
} // namespace

void VoiceGuardEngine::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    maxChannels_ = juce::jlimit (1, 2, numChannels);

    analyzer_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    input_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    noiseReduction_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    dynamicEq_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    harshness_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    deEsser_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    levelRider_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    dynamics_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    outputSafety_.prepare (sampleRate_, maxBlockSize, maxChannels_);
    auto_.prepare (sampleRate_);
    feedback_->prepare (sampleRate_, maxBlockSize, maxChannels_);

    dryBuffer_.setSize (maxChannels_, juce::jmax (1, maxBlockSize));
    dryBuffer_.clear();

    bypassMix_.reset (sampleRate_, 0.020);
    prepared_ = true;

    resolve();
}

void VoiceGuardEngine::reset() noexcept
{
    analyzer_.reset();
    input_.reset();
    noiseReduction_.reset();
    dynamicEq_.reset();
    harshness_.reset();
    deEsser_.reset();
    levelRider_.reset();
    dynamics_.reset();
    outputSafety_.reset();
    auto_.reset();
    feedback_->reset();
    bypassMix_.setCurrentAndTargetValue (0.0f);
}

void VoiceGuardEngine::setParameters (const params::Parameters& p) noexcept
{
    params_ = p;

    if (params_.autoMode && ! autoWasOn_)
    {
        auto_.start();
        autoWasOn_ = true;
    }
    else if (! params_.autoMode && autoWasOn_)
    {
        auto_.cancel();
        autoWasOn_ = false;
    }

    resolve();
}

void VoiceGuardEngine::resolve() noexcept
{
    const auto& p = params_;

    eff_.inputTrimDb = p.inputTrimDb;
    eff_.hpfHz = p.hpfFrequencyHz();

    eff_.cleanAmount = juce::jlimit (0.0f, 1.0f, p.cleanAmount * macroGain (p.macroClean));
    eff_.cleanAdaptation = p.cleanAdaptation;

    const float clarity = macroGain (p.macroClarity);
    eff_.eqStrength = juce::jlimit (0.0f, 1.0f, p.eqStrength * clarity);
    eff_.deEsserAmount = juce::jlimit (0.0f, 1.0f, p.deEsserAmount * clarity);
    eff_.harshnessAmount = juce::jlimit (0.0f, 1.0f, p.harshnessAmount * clarity);

    const float level = macroGain (p.macroLevel);
    // LEVEL also moves the target level itself, so the macro is clearly
    // audible as a level change and not only as ride behaviour.
    eff_.levelTargetDb = juce::jlimit (-30.0f, -6.0f,
                                       p.levelTargetDb + (p.macroLevel - 0.5f) * 12.0f);
    // PROTECT also makes the ride more conservative (less gain movement).
    eff_.levelRangeDb = juce::jlimit (0.0f, 6.0f,
                                      p.levelRangeDb * level * (1.0f - 0.3f * p.macroProtect));
    eff_.levelSpeed = p.levelSpeed;
    eff_.dynThresholdDb = p.dynThresholdDb;
    eff_.dynRatio = juce::jlimit (1.0f, 6.0f, 1.0f + (p.dynRatio - 1.0f) * level);
    eff_.dynAttackMs = p.dynAttackMs;
    eff_.dynReleaseMs = p.dynReleaseMs;
    eff_.dynKneeDb = p.dynKneeDb;

    // PROTECT raises safety: turning it up lowers the ceiling.
    eff_.outputCeilingDb = juce::jlimit (-18.0f, 0.0f,
                                         p.outputCeilingDb - macroGain (p.macroProtect) * 6.0f);
    eff_.outputTrimDb = p.outputTrimDb;
    eff_.deFeedback = p.deFeedbackEnable;
    eff_.bypass = p.bypass;

    // AUTO, once it has heard enough speech, supplies conservative starting
    // values. The user's advanced values are otherwise untouched.
    if (p.autoMode && auto_.state() == AutoController::State::ready)
    {
        const auto& s = auto_.suggestions();
        eff_.cleanAmount = juce::jlimit (0.0f, 1.0f, s.cleanAmount * macroGain (p.macroClean));
        eff_.eqStrength = juce::jlimit (0.0f, 1.0f, s.eqStrength * clarity);
        eff_.deEsserAmount = juce::jlimit (0.0f, 1.0f, s.deEsserAmount * clarity);
        eff_.harshnessAmount = juce::jlimit (0.0f, 1.0f, s.harshnessAmount * clarity);
        eff_.levelTargetDb = s.levelTargetDb;
    }

    input_.setTrimDb (eff_.inputTrimDb);
    input_.setHpfFrequency (eff_.hpfHz);
    noiseReduction_.setAmount (eff_.cleanAmount);
    noiseReduction_.setAdaptation (eff_.cleanAdaptation);
    dynamicEq_.setStrength (eff_.eqStrength);
    harshness_.setAmount (eff_.harshnessAmount);
    deEsser_.setAmount (eff_.deEsserAmount);
    levelRider_.setTargetDb (eff_.levelTargetDb);
    levelRider_.setRangeDb (eff_.levelRangeDb);
    levelRider_.setSpeed (eff_.levelSpeed);
    dynamics_.setParameters (eff_.dynThresholdDb, eff_.dynRatio, eff_.dynAttackMs,
                             eff_.dynReleaseMs, eff_.dynKneeDb);
    outputSafety_.setCeilingDb (eff_.outputCeilingDb);
    feedback_->setEnabled (eff_.deFeedback);
}

void VoiceGuardEngine::process (juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples = buffer.getNumSamples();

    if (! prepared_ || numSamples <= 0 || buffer.getNumChannels() <= 0)
        return;

    ScopedDenormalDisable noDenormals;

    // Never resize the host buffer on the audio thread; only process what fits.
    // A host may hand us more samples than we prepared for, so clamp the dry
    // copy to the preallocated buffer (otherwise it is an out-of-bounds write).
    const int numChannels = juce::jmin (buffer.getNumChannels(), maxChannels_);
    const int drySamples = juce::jmin (numSamples, dryBuffer_.getNumSamples());

    sanitizeBuffer (buffer);

    const float* analyse[2] = { buffer.getReadPointer (0),
                                numChannels > 1 ? buffer.getReadPointer (1) : nullptr };
    analyzer_.process (analyse, numChannels, numSamples);

    const SpeechFeatures& features = analyzer_.features();
    const float blockSeconds = (float) ((double) numSamples / sampleRate_);
    auto_.process (features, blockSeconds);

    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom (ch, 0, buffer, ch, 0, drySamples);

    input_.process (buffer);

    if (eff_.cleanAmount > 0.0005f)
        noiseReduction_.process (buffer, features);

    dynamicEq_.process (buffer, features);
    harshness_.process (buffer, features);
    deEsser_.process (buffer, features);
    levelRider_.process (buffer, features);
    dynamics_.process (buffer, features);

    if (eff_.deFeedback)
        feedback_->process (buffer);

    // Output trim is part of the output stage, so the safety ceiling always
    // has the final word: the output is bounded regardless of trim.
    if (std::abs (eff_.outputTrimDb) > 0.01f)
        buffer.applyGain (dbToGain (eff_.outputTrimDb));

    outputSafety_.process (buffer);

    // Click-free bypass crossfade against the dry copy.
    bypassMix_.setTargetValue (eff_.bypass ? 1.0f : 0.0f);

    if (bypassMix_.isSmoothing() || bypassMix_.getCurrentValue() > 1.0e-4f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wet = buffer.getWritePointer (ch);
            const auto* dry = dryBuffer_.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
            {
                const float mix = bypassMix_.getNextValue();

                if (i < drySamples)
                    wet[i] = wet[i] * (1.0f - mix) + dry[i] * mix;
            }
        }
    }

    // Readouts: per-channel levels and per-stage gain reduction.
    for (int ch = 0; ch < UiSnapshot::kMaxChannels; ++ch)
    {
        const int source = juce::jmin (ch, numChannels - 1);
        const auto* dry = dryBuffer_.getReadPointer (source);
        const auto* wet = buffer.getReadPointer (source);

        float dryPeak = 0.0f;
        float wetPeak = 0.0f;

        for (int i = 0; i < drySamples; ++i)
            dryPeak = juce::jmax (dryPeak, std::abs (dry[i]));

        for (int i = 0; i < numSamples; ++i)
            wetPeak = juce::jmax (wetPeak, std::abs (wet[i]));

        snapshot_.inputLevelDb[ch].store (gainToDb (dryPeak));
        snapshot_.outputLevelDb[ch].store (gainToDb (wetPeak));
    }

    snapshot_.noiseReductionDb.store (noiseReduction_.currentReductionDb());
    snapshot_.levelRideDb.store (levelRider_.rideGainDb());
    snapshot_.harshnessDb.store (harshness_.currentCutDb());
    snapshot_.deEsserDb.store (deEsser_.currentCutDb());
    snapshot_.dynamicEqDb.store (dynamicEq_.currentCutDb());
    snapshot_.dynamicsGainReductionDb.store (dynamics_.gainReductionDb());
    snapshot_.outputGainReductionDb.store (outputSafety_.gainReductionDb());
    snapshot_.protectionActive.store (outputSafety_.isActive());
    snapshot_.speechActivity.store (features.speechActivity);
    snapshot_.autoState.store ((int) auto_.state());
    snapshot_.autoProgress.store (auto_.progress());
}

} // namespace voiceguard
