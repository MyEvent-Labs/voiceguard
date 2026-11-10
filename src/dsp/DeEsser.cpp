#include "dsp/DeEsser.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

namespace
{
constexpr float kCentres[DeEsser::kNumBands] = { 5500.0f, 8500.0f };
constexpr float kQs[DeEsser::kNumBands] = { 1.0f, 1.0f };
constexpr float kWeight[DeEsser::kNumBands] = { 1.0f, 0.9f };
} // namespace

void DeEsser::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    bank_.configure (sampleRate, numChannels, kCentres, kQs, kNumBands);
}

void DeEsser::reset() noexcept
{
    bank_.reset();
}

void DeEsser::setAmount (float amount01) noexcept
{
    amount_ = juce::jlimit (0.0f, 1.0f, amount01);
}

void DeEsser::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    if (buffer.getNumSamples() <= 0 || bank_.numBands() == 0)
        return;

    bank_.detect (buffer);

    const float gate = juce::jlimit (0.0f, 1.0f, features.speechActivity * 1.5f);
    const float confidence = juce::jlimit (0.0f, 1.0f, features.sibilanceConfidence);
    const float base = juce::jmin (kMaxCutDb, amount_ * kMaxCutDb * confidence * gate);

    for (int band = 0; band < kNumBands; ++band)
        bank_.setCutDb (band, base * kWeight[band]);

    bank_.apply (buffer);
}

} // namespace voiceguard
