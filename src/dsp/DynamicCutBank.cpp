#include "dsp/DynamicCutBank.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

namespace
{
float sampleCoeff (double sampleRate, float tauSeconds) noexcept
{
    return 1.0f - std::exp (-1.0f / (float) (sampleRate * (double) tauSeconds));
}
} // namespace

void DynamicCutBank::configure (double sampleRate, int numChannels,
                                const float* centresHz, const float* qs, int count)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, Biquad::kMaxChannels, numChannels);
    numBands_ = juce::jlimit (0, kMaxBands, count);

    for (int band = 0; band < numBands_; ++band)
    {
        centreHz_[band] = juce::jmin (centresHz[band], (float) (sampleRate_ * 0.45));
        q_[band] = juce::jmax (0.1f, qs[band]);
        detectors_[band].setBandpass (sampleRate_, centreHz_[band], q_[band]);
        eq_[band].setPeaking (sampleRate_, centreHz_[band], q_[band], 0.0);
    }

    envAttack_ = sampleCoeff (sampleRate_, 0.005f);
    envRelease_ = sampleCoeff (sampleRate_, 0.060f);

    reset();
}

void DynamicCutBank::reset() noexcept
{
    for (int band = 0; band < numBands_; ++band)
    {
        detectors_[band].reset();
        eq_[band].setPeaking (sampleRate_, centreHz_[band], q_[band], 0.0);
        env_[band] = 0.0f;
        cutTarget_[band] = 0.0f;
        cutSmoothed_[band] = 0.0f;
    }
}

void DynamicCutBank::detect (const juce::AudioBuffer<float>& buffer) noexcept
{
    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0)
        return;

    const auto* left = buffer.getReadPointer (0);
    const auto* right = numChannels > 1 ? buffer.getReadPointer (1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float mono = right != nullptr ? 0.5f * (left[i] + right[i]) : left[i];

        for (int band = 0; band < numBands_; ++band)
        {
            const float detector = std::abs (detectors_[band].processSample (mono, 0));
            const float coeff = detector > env_[band] ? envAttack_ : envRelease_;
            env_[band] += (detector - env_[band]) * coeff;
        }
    }
}

float DynamicCutBank::envelopeDb (int band) const noexcept
{
    if (band < 0 || band >= numBands_)
        return -100.0f;

    return env_[band] > 1.0e-7f ? gainToDb (env_[band]) : -100.0f;
}

void DynamicCutBank::setCutDb (int band, float cutDb) noexcept
{
    if (band >= 0 && band < numBands_)
        cutTarget_[band] = juce::jlimit (0.0f, 24.0f, cutDb);
}

void DynamicCutBank::apply (juce::AudioBuffer<float>& buffer) noexcept
{
    if (numBands_ == 0)
        return;

    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    // Smoothing runs once per block, so the coefficient must be per block.
    const float blockSeconds = (float) ((double) numSamples / sampleRate_);
    const float smooth = 1.0f - std::exp (-blockSeconds / 0.040f);

    for (int band = 0; band < numBands_; ++band)
    {
        cutSmoothed_[band] += (cutTarget_[band] - cutSmoothed_[band]) * smooth;
        eq_[band].setPeaking (sampleRate_, centreHz_[band], q_[band], -(double) cutSmoothed_[band]);
    }

    ScopedDenormalDisable noDenormals;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        for (int band = 0; band < numBands_; ++band)
            eq_[band].processBlock (data, numSamples, ch);
    }
}

} // namespace voiceguard
