#include "dsp/InputConditioning.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

void InputConditioning::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, Biquad::kMaxChannels, numChannels);

    trimGain_.reset (sampleRate_, 0.020);
    reset();

    if (hpfHz_ > 0.0f)
        hpf_.setHighpass (sampleRate_, hpfHz_, 0.7071);
    else
        hpf_.setBypass();
}

void InputConditioning::reset() noexcept
{
    hpf_.reset();
    trimGain_.setCurrentAndTargetValue (trimGain_.getTargetValue());
}

void InputConditioning::setTrimDb (float db) noexcept
{
    trimDb_ = juce::jlimit (-18.0f, 18.0f, db);
    trimGain_.setTargetValue (dbToGain (trimDb_));
}

void InputConditioning::setHpfFrequency (float hz) noexcept
{
    const float clamped = hz <= 0.0f ? 0.0f : juce::jlimit (20.0f, 400.0f, hz);

    if (clamped == hpfHz_)
        return;

    hpfHz_ = clamped;

    if (hpfHz_ > 0.0f)
        hpf_.setHighpass (sampleRate_, hpfHz_, 0.7071);
    else
        hpf_.setBypass();
}

void InputConditioning::process (juce::AudioBuffer<float>& buffer) noexcept
{
    ScopedDenormalDisable noDenormals;

    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        hpf_.processBlock (data, numSamples, ch);
    }

    if (trimGain_.isSmoothing())
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float gain = trimGain_.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.getWritePointer (ch)[i] *= gain;
        }
    }
    else
    {
        const float gain = trimGain_.getTargetValue();

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.applyGain (ch, 0, numSamples, gain);
    }
}

} // namespace voiceguard
