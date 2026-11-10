#include "dsp/OutputSafety.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

void OutputSafety::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, 2, numChannels);
    gain_.reset (sampleRate_, 0.005);
    reset();
}

void OutputSafety::reset() noexcept
{
    gainReductionDb_ = 0.0f;
    gain_.setCurrentAndTargetValue (1.0f);
}

void OutputSafety::setCeilingDb (float db) noexcept
{
    ceilingLin_ = dbToGain (juce::jlimit (-12.0f, 0.0f, db));
}

void OutputSafety::process (juce::AudioBuffer<float>& buffer) noexcept
{
    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0 || ceilingLin_ <= 0.0f)
        return;

    ScopedDenormalDisable noDenormals;

    float peak = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* data = buffer.getReadPointer (ch);

        for (int i = 0; i < numSamples; ++i)
            peak = juce::jmax (peak, std::abs (data[i]));
    }

    const float targetGr = peak > ceilingLin_ && peak > 0.0f
                               ? juce::jlimit (0.0f, 24.0f, gainToDb (peak / ceilingLin_))
                               : 0.0f;

    const float blockSeconds = (float) ((double) numSamples / sampleRate_);
    const float attack = 1.0f - std::exp (-blockSeconds / 0.001f);
    const float release = 1.0f - std::exp (-blockSeconds / 0.050f);
    const float coeff = targetGr > gainReductionDb_ ? attack : release;

    gainReductionDb_ += (targetGr - gainReductionDb_) * coeff;
    gain_.setTargetValue (dbToGain (-gainReductionDb_));

    for (int i = 0; i < numSamples; ++i)
    {
        const float g = gain_.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            float x = data[i] * g;

            if (! sanitize (x))
                continue;

            data[i] = juce::jlimit (-ceilingLin_, ceilingLin_, x);
        }
    }
}

} // namespace voiceguard
