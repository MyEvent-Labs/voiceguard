#include "dsp/DspUtils.h"

namespace voiceguard
{

float dbToGain (float db) noexcept
{
    return db <= kMinDb ? 0.0f : std::pow (10.0f, db * 0.05f);
}

float gainToDb (float gain) noexcept
{
    const auto magnitude = std::abs (gain);
    return magnitude <= 1.0e-10f ? kMinDb : 20.0f * std::log10 (magnitude);
}

bool isFinite (float x) noexcept
{
    return std::isfinite (x);
}

bool sanitize (float& x) noexcept
{
    const bool ok = std::isfinite (x);
    if (! ok)
        x = 0.0f;
    return ok;
}

bool sanitizeBuffer (juce::AudioBuffer<float>& buffer) noexcept
{
    bool allFinite = true;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (! sanitize (data[i]))
                allFinite = false;
    }

    return allFinite;
}

float clampOr (float value, float lo, float hi, float fallback) noexcept
{
    if (! std::isfinite (value))
        return fallback;

    return juce::jlimit (lo, hi, value);
}

} // namespace voiceguard
