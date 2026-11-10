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
constexpr float kBandQ = 1.3f;
constexpr float kSpeechMarginDb = 6.0f;

float sampleCoeff (double sampleRate, float tauSeconds) noexcept
{
    return 1.0f - std::exp (-1.0f / (float) (sampleRate * (double) tauSeconds));
}
} // namespace

void NoiseReduction::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    numChannels_ = juce::jlimit (1, Biquad::kMaxChannels, numChannels);

    for (int band = 0; band < kNumBands; ++band)
    {
        centreHz_[band] = juce::jmin (kBandCentresHz[band], (float) (sampleRate_ * 0.45));
        detectors_[band].setBandpass (sampleRate_, centreHz_[band], kBandQ);
        eq_[band].setPeaking (sampleRate_, centreHz_[band], kBandQ, 0.0);
    }

    envAttack_ = sampleCoeff (sampleRate_, 0.005f);
    envRelease_ = sampleCoeff (sampleRate_, 0.060f);
    cutSmoothCoeff_ = sampleCoeff (sampleRate_, 0.040f);

    prepared_ = true;
    reset();
}

void NoiseReduction::reset() noexcept
{
    for (int band = 0; band < kNumBands; ++band)
    {
        detectors_[band].reset();
        eq_[band].setPeaking (sampleRate_, centreHz_[band], kBandQ, 0.0);
        env_[band] = 0.0f;
        floorDb_[band] = -100.0f;
        cutDb_[band] = 0.0f;
        cutSmoothed_[band] = 0.0f;
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
    if (! prepared_ || buffer.getNumSamples() <= 0)
        return;

    ScopedDenormalDisable noDenormals;

    const int numChannels = juce::jmin (buffer.getNumChannels(), numChannels_);
    const int numSamples = buffer.getNumSamples();

    auto* left = buffer.getWritePointer (0);
    auto* right = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    // 1. Detect on the dry signal, averaged across channels.
    for (int i = 0; i < numSamples; ++i)
    {
        const float in0 = left[i];
        const float in1 = right != nullptr ? right[i] : in0;
        const float mono = 0.5f * (in0 + in1);

        for (int band = 0; band < kNumBands; ++band)
        {
            const float y = detectors_[band].processSample (mono, 0);
            const float detector = std::abs (y);
            const float coeff = detector > env_[band] ? envAttack_ : envRelease_;
            env_[band] += (detector - env_[band]) * coeff;
        }
    }

    updateGains (features, (float) ((double) numSamples / sampleRate_));

    // 2. Apply the dynamic cuts serially.
    for (int band = 0; band < kNumBands; ++band)
        eq_[band].setPeaking (sampleRate_, centreHz_[band], kBandQ, -(double) cutSmoothed_[band]);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        for (int band = 0; band < kNumBands; ++band)
            eq_[band].processBlock (data, numSamples, ch);
    }
}

void NoiseReduction::updateGains (const SpeechFeatures& features, float blockSeconds) noexcept
{
    const float riseCoeff = 1.0f - std::exp (-blockSeconds / juce::jmax (0.05f, riseTau_));
    const float fallCoeff = 1.0f - std::exp (-blockSeconds / 0.15f);
    const float smooth = 1.0f - std::exp (-blockSeconds / 0.040f);
    const float protect = 1.0f - 0.5f * juce::jlimit (0.0f, 1.0f, features.speechActivity);

    float reductionSum = 0.0f;

    for (int band = 0; band < kNumBands; ++band)
    {
        const float envDb = env_[band] > 1.0e-7f ? gainToDb (env_[band]) : -100.0f;

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

        cutSmoothed_[band] += (cutDb_[band] - cutSmoothed_[band]) * smooth;
        reductionSum += cutSmoothed_[band];
    }

    currentReductionDb_ = reductionSum / (float) kNumBands;
    ++blocksSeen_;
}

} // namespace voiceguard
