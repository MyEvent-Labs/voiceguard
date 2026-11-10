#include "dsp/Biquad.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

void Biquad::reset() noexcept
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        z1_[ch] = 0.0f;
        z2_[ch] = 0.0f;
    }
}

void Biquad::setBypass() noexcept
{
    b0_ = 1.0f;
    b1_ = b2_ = a1_ = a2_ = 0.0f;
    reset();
}

void Biquad::setCoefficients (double b0, double b1, double b2, double a0, double a1, double a2) noexcept
{
    if (std::abs (a0) < 1.0e-12)
    {
        setBypass();
        return;
    }

    const double inv = 1.0 / a0;

    b0_ = (float) juce::jlimit (-100.0, 100.0, b0 * inv);
    b1_ = (float) juce::jlimit (-100.0, 100.0, b1 * inv);
    b2_ = (float) juce::jlimit (-100.0, 100.0, b2 * inv);
    a1_ = (float) juce::jlimit (-100.0, 100.0, a1 * inv);
    a2_ = (float) juce::jlimit (-100.0, 100.0, a2 * inv);
}

void Biquad::setHighpass (double sampleRate, double frequencyHz, double q) noexcept
{
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));

    const double b0 = (1.0 + cosw) * 0.5;
    const double b1 = -(1.0 + cosw);
    const double b2 = (1.0 + cosw) * 0.5;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosw;
    const double a2 = 1.0 - alpha;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

void Biquad::setLowpass (double sampleRate, double frequencyHz, double q) noexcept
{
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));

    const double b0 = (1.0 - cosw) * 0.5;
    const double b1 = 1.0 - cosw;
    const double b2 = (1.0 - cosw) * 0.5;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosw;
    const double a2 = 1.0 - alpha;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

void Biquad::setPeaking (double sampleRate, double frequencyHz, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));

    const double b0 = 1.0 + alpha * A;
    const double b1 = -2.0 * cosw;
    const double b2 = 1.0 - alpha * A;
    const double a0 = 1.0 + alpha / A;
    const double a1 = -2.0 * cosw;
    const double a2 = 1.0 - alpha / A;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

void Biquad::setBandpass (double sampleRate, double frequencyHz, double q) noexcept
{
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));

    const double b0 = alpha;
    const double b1 = 0.0;
    const double b2 = -alpha;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosw;
    const double a2 = 1.0 - alpha;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

void Biquad::setLowShelf (double sampleRate, double frequencyHz, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));
    const double twoSqrtAalpha = 2.0 * std::sqrt (A) * alpha;

    const double b0 = A * ((A + 1.0) - (A - 1.0) * cosw + twoSqrtAalpha);
    const double b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw);
    const double b2 = A * ((A + 1.0) - (A - 1.0) * cosw - twoSqrtAalpha);
    const double a0 = (A + 1.0) + (A - 1.0) * cosw + twoSqrtAalpha;
    const double a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw);
    const double a2 = (A + 1.0) + (A - 1.0) * cosw - twoSqrtAalpha;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

void Biquad::setHighShelf (double sampleRate, double frequencyHz, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double w0 = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;
    const double cosw = std::cos (w0);
    const double alpha = std::sin (w0) / (2.0 * juce::jmax (0.05, q));
    const double twoSqrtAalpha = 2.0 * std::sqrt (A) * alpha;

    const double b0 = A * ((A + 1.0) + (A - 1.0) * cosw + twoSqrtAalpha);
    const double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw);
    const double b2 = A * ((A + 1.0) + (A - 1.0) * cosw - twoSqrtAalpha);
    const double a0 = (A + 1.0) - (A - 1.0) * cosw + twoSqrtAalpha;
    const double a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw);
    const double a2 = (A + 1.0) - (A - 1.0) * cosw - twoSqrtAalpha;

    setCoefficients (b0, b1, b2, a0, a1, a2);
}

float Biquad::processSample (float x, int channel) noexcept
{
    const int ch = juce::jlimit (0, kMaxChannels - 1, channel);

    const float y = b0_ * x + z1_[ch];
    z1_[ch] = b1_ * x - a1_ * y + z2_[ch];
    z2_[ch] = b2_ * x - a2_ * y;

    if (! std::isfinite (y))
    {
        z1_[ch] = z2_[ch] = 0.0f;
        return 0.0f;
    }

    return y;
}

void Biquad::processBlock (float* data, int numSamples, int channel) noexcept
{
    for (int i = 0; i < numSamples; ++i)
        data[i] = processSample (data[i], channel);
}

float Biquad::magnitudeAt (double sampleRate, double frequencyHz) const noexcept
{
    const double w = juce::MathConstants<double>::twoPi * frequencyHz / sampleRate;

    const double numRe = b0_ + b1_ * std::cos (w) + b2_ * std::cos (2.0 * w);
    const double numIm = -(b1_ * std::sin (w) + b2_ * std::sin (2.0 * w));
    const double denRe = 1.0 + a1_ * std::cos (w) + a2_ * std::cos (2.0 * w);
    const double denIm = -(a1_ * std::sin (w) + a2_ * std::sin (2.0 * w));

    const double den = std::sqrt (denRe * denRe + denIm * denIm);

    if (den < 1.0e-12)
        return 0.0f;

    return (float) (std::sqrt (numRe * numRe + numIm * numIm) / den);
}

} // namespace voiceguard
