#include "dsp/LevelRider.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

void LevelRider::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, 2, numChannels);
    gain_.reset (sampleRate_, 0.020);
    reset();
}

void LevelRider::reset() noexcept
{
    rideGainDb_ = 0.0f;
    gain_.setCurrentAndTargetValue (1.0f);
}

void LevelRider::setTargetDb (float db) noexcept { targetDb_ = juce::jlimit (-30.0f, -6.0f, db); }
void LevelRider::setRangeDb (float db) noexcept { rangeDb_ = juce::jlimit (0.0f, 6.0f, db); }
void LevelRider::setSpeed (float speed01) noexcept { speed_ = juce::jlimit (0.0f, 1.0f, speed01); }

void LevelRider::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0)
        return;

    ScopedDenormalDisable noDenormals;

    const float blockSeconds = (float) ((double) numSamples / sampleRate_);

    // Speech gate: never boost silence or room tone.
    float desiredDb = 0.0f;

    if (features.speechActivity > 0.25f && features.rmsShortDb > -60.0f)
        desiredDb = juce::jlimit (-rangeDb_, rangeDb_, targetDb_ - features.rmsShortDb);

    // Fast peak path, separate from the slow ride.
    if (features.peakDb > -3.0f)
        desiredDb = juce::jmin (desiredDb, -3.0f - features.peakDb);

    desiredDb = juce::jlimit (-rangeDb_, rangeDb_, desiredDb);

    // Slow ride: speed 0 -> ~4 s, speed 1 -> ~0.5 s.
    const float tau = juce::jmap (speed_, 4.0f, 0.5f);
    const float coeff = 1.0f - std::exp (-blockSeconds / tau);
    rideGainDb_ += (desiredDb - rideGainDb_) * coeff;
    rideGainDb_ = juce::jlimit (-rangeDb_, rangeDb_, rideGainDb_);

    gain_.setTargetValue (dbToGain (rideGainDb_));

    for (int i = 0; i < numSamples; ++i)
    {
        const float g = gain_.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer (ch)[i] *= g;
    }
}

} // namespace voiceguard
