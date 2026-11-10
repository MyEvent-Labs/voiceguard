#include "dsp/NoiseReduction.h"

#include "dsp/DspUtils.h"

#include <cmath>

namespace voiceguard
{

namespace
{
constexpr float kBandCentresHz[NoiseReduction::kNumBands] = {
    80.0f, 160.0f, 320.0f, 640.0f, 1280.0f, 2560.0f, 5120.0f, 8000.0f, 11000.0f, 16000.0f
};
constexpr float kBandQs[NoiseReduction::kNumBands] = {
    1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f, 1.3f
};
constexpr float kSpeechMarginDb = 6.0f;
} // namespace

void NoiseReduction::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    bank_.configure (sampleRate_, numChannels, kBandCentresHz, kBandQs, kNumBands);
    reset();
}

void NoiseReduction::reset() noexcept
{
    bank_.reset();

    for (int band = 0; band < kNumBands; ++band)
    {
        floorDb_[band] = -100.0f;
        cutDb_[band] = 0.0f;
    }

    currentReductionDb_ = 0.0f;
    blocksSeen_ = 0;
}

void NoiseReduction::setAmount (float amount01) noexcept
{
    amount_ = juce::jlimit (0.0f, 1.0f, amount01);
}

void NoiseReduction::setAdaptation (float adaptation01) noexcept
{
    adaptation_ = juce::jlimit (0.0f, 1.0f, adaptation01);
    riseTau_ = juce::jmap (adaptation_, 0.0f, 1.0f, 1.2f, 0.30f);
}

void NoiseReduction::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    if (buffer.getNumSamples() <= 0 || bank_.numBands() == 0)
        return;

    bank_.detect (buffer);

    const float blockSeconds = (float) ((double) buffer.getNumSamples() / sampleRate_);
    updateGains (features, blockSeconds);

    bank_.apply (buffer);
}

void NoiseReduction::updateGains (const SpeechFeatures& features, float blockSeconds) noexcept
{
    const float riseCoeff = 1.0f - std::exp (-blockSeconds / juce::jmax (0.05f, riseTau_));
    const float fallCoeff = 1.0f - std::exp (-blockSeconds / 0.15f);
    const float protect = 1.0f - 0.5f * juce::jlimit (0.0f, 1.0f, features.speechActivity);

    float reductionSum = 0.0f;

    for (int band = 0; band < kNumBands; ++band)
    {
        const float envDb = bank_.envelopeDb (band);

        if (blocksSeen_ < 16)
            floorDb_[band] = envDb;
        else if (envDb < floorDb_[band])
            floorDb_[band] += (envDb - floorDb_[band]) * fallCoeff;
        else if (features.speechActivity < 0.3f)
            floorDb_[band] += (envDb - floorDb_[band]) * riseCoeff;

        floorDb_[band] = juce::jlimit (-100.0f, -10.0f, floorDb_[band]);

        const float snr = envDb - floorDb_[band];
        const float depth = juce::jlimit (0.0f, 1.0f, (kSpeechMarginDb - snr) / kSpeechMarginDb);
        cutDb_[band] = amount_ * kMaxReductionDb * depth * protect;

        bank_.setCutDb (band, cutDb_[band]);
        reductionSum += cutDb_[band];
    }

    currentReductionDb_ = reductionSum / (float) kNumBands;
    ++blocksSeen_;
}

} // namespace voiceguard
