#include "dsp/DeEsser.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

namespace
{
// Band 0 is the mid reference; bands 1-2 are the sibilance region.
constexpr float kCentres[DeEsser::kNumBands] = { 2500.0f, 5500.0f, 8500.0f };
constexpr float kQs[DeEsser::kNumBands] = { 1.0f, 1.0f, 1.0f };
constexpr float kWeight[DeEsser::kNumBands] = { 0.0f, 1.0f, 0.9f };
// Sibilance is likely once the high band is within ~6 dB of the mid band; the
// cut then ramps to full over the next 12 dB.
constexpr float kSibilanceMarginDb = 6.0f;
constexpr float kSibilanceRangeDb = 12.0f;
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
    const float referenceDb = bank_.envelopeDb (0);
    const float highDb = 0.5f * (bank_.envelopeDb (1) + bank_.envelopeDb (2));
    const float excess = highDb - referenceDb;
    const float sibilance = juce::jlimit (0.0f, 1.0f,
                                         (excess + kSibilanceMarginDb) / kSibilanceRangeDb);
    const float cut = juce::jmin (kMaxCutDb, amount_ * kMaxCutDb * sibilance * gate);

    for (int band = 0; band < kNumBands; ++band)
        bank_.setCutDb (band, cut * kWeight[band]);

    bank_.apply (buffer);
}

} // namespace voiceguard
