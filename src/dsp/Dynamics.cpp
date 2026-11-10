#include "dsp/Dynamics.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

void Dynamics::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, 2, numChannels);
    gain_.reset (sampleRate_, 0.010);
    reset();
}

void Dynamics::reset() noexcept
{
    gainReductionDb_ = 0.0f;
    gain_.setCurrentAndTargetValue (1.0f);
}

void Dynamics::setParameters (float thresholdDb, float ratio, float attackMs,
                              float releaseMs, float kneeDb) noexcept
{
    thresholdDb_ = juce::jlimit (-40.0f, 0.0f, thresholdDb);
    ratio_ = juce::jlimit (1.0f, 6.0f, ratio);
    attackMs_ = juce::jlimit (1.0f, 100.0f, attackMs);
    releaseMs_ = juce::jlimit (50.0f, 1000.0f, releaseMs);
    kneeDb_ = juce::jlimit (0.0f, 18.0f, kneeDb);
}

void Dynamics::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0 || ratio_ <= 1.0f)
        return;

    ScopedDenormalDisable noDenormals;

    const float blockSeconds = (float) ((double) numSamples / sampleRate_);
    const float over = features.rmsShortDb - thresholdDb_;

    float targetGr = 0.0f;
    const float slope = 1.0f - 1.0f / ratio_;

    if (kneeDb_ > 0.0f && 2.0f * std::abs (over) <= kneeDb_)
        targetGr = slope * (over + kneeDb_ * 0.5f) * (over + kneeDb_ * 0.5f) / (2.0f * kneeDb_);
    else if (over > kneeDb_ * 0.5f)
        targetGr = slope * over;

    targetGr = juce::jlimit (0.0f, 24.0f, targetGr);

    const float attack = 1.0f - std::exp (-blockSeconds / juce::jmax (0.001f, attackMs_ * 0.001f));
    const float release = 1.0f - std::exp (-blockSeconds / juce::jmax (0.001f, releaseMs_ * 0.001f));
    const float coeff = targetGr > gainReductionDb_ ? attack : release;

    gainReductionDb_ += (targetGr - gainReductionDb_) * coeff;
    gain_.setTargetValue (dbToGain (-gainReductionDb_));

    for (int i = 0; i < numSamples; ++i)
    {
        const float g = gain_.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer (ch)[i] *= g;
    }
}

} // namespace voiceguard
