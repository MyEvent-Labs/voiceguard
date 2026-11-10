#include "dsp/HarshnessControl.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

namespace
{
constexpr float kCentres[HarshnessControl::kNumBands] = { 3200.0f, 4500.0f, 6000.0f };
constexpr float kQs[HarshnessControl::kNumBands] = { 1.0f, 1.0f, 1.0f };
constexpr float kWeight[HarshnessControl::kNumBands] = { 0.8f, 1.0f, 0.9f };
constexpr float kMaxCutDb = 5.0f;
} // namespace

void HarshnessControl::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    bank_.configure (sampleRate, numChannels, kCentres, kQs, kNumBands);
}

void HarshnessControl::reset() noexcept
{
    bank_.reset();
}

void HarshnessControl::setAmount (float amount01) noexcept
{
    amount_ = juce::jlimit (0.0f, 1.0f, amount01);
}

void HarshnessControl::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    if (buffer.getNumSamples() <= 0 || bank_.numBands() == 0)
        return;

    bank_.detect (buffer);

    const float gate = juce::jlimit (0.0f, 1.0f, features.speechActivity * 1.5f);

    // Dead band: only act on real harshness, so neutral speech is left alone.
    const float confidence = juce::jlimit (0.0f, 1.0f, (features.harshnessConfidence - 0.35f) / 0.5f);
    const float base = amount_ * kMaxCutDb * confidence * gate;

    for (int band = 0; band < kNumBands; ++band)
        bank_.setCutDb (band, base * kWeight[band]);

    bank_.apply (buffer);
}

} // namespace voiceguard
