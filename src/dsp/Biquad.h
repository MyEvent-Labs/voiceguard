#pragma once

#include <complex>

namespace voiceguard
{

/** Direct-form-II transposed biquad with per-channel state.

    Coefficient setters are RT-safe (no allocation) and must be called from
    the audio thread only when the desired response changes. */
class Biquad
{
public:
    static constexpr int kMaxChannels = 2;

    Biquad() = default;

    /** Reset filter state but keep coefficients. */
    void reset() noexcept;

    void setBypass() noexcept;
    void setHighpass (double sampleRate, double frequencyHz, double q) noexcept;
    void setLowpass (double sampleRate, double frequencyHz, double q) noexcept;
    void setPeaking (double sampleRate, double frequencyHz, double q, double gainDb) noexcept;
    void setBandpass (double sampleRate, double frequencyHz, double q) noexcept;
    void setLowShelf (double sampleRate, double frequencyHz, double q, double gainDb) noexcept;
    void setHighShelf (double sampleRate, double frequencyHz, double q, double gainDb) noexcept;

    float processSample (float x, int channel) noexcept;
    void processBlock (float* data, int numSamples, int channel) noexcept;

    /** Magnitude of the transfer function at a frequency (for tests). */
    float magnitudeAt (double sampleRate, double frequencyHz) const noexcept;

private:
    void setCoefficients (double b0, double b1, double b2, double a0, double a1, double a2) noexcept;

    float b0_ = 1.0f, b1_ = 0.0f, b2_ = 0.0f, a1_ = 0.0f, a2_ = 0.0f;
    float z1_[kMaxChannels] {};
    float z2_[kMaxChannels] {};
};

} // namespace voiceguard
