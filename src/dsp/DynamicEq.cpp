#include "dsp/DynamicEq.h"

#include "dsp/DspUtils.h"

namespace voiceguard
{

namespace
{
constexpr float kCentres[DynamicEq::kNumBands] = { 250.0f, 550.0f, 1100.0f, 2800.0f, 4000.0f, 7000.0f };
constexpr float kQs[DynamicEq::kNumBands] = { 1.0f, 1.2f, 1.4f, 1.2f, 1.2f, 1.0f };
// Expected level of each region relative to the intelligibility band (2800 Hz)
// for neutral speech. Excess above this is treated as program-dependent
// coloration and becomes a candidate for a dynamic cut.
constexpr float kTiltDb[DynamicEq::kNumBands] = { 2.0f, 1.0f, 0.0f, 0.0f, -1.0f, -2.0f };
constexpr float kMaxCutDb[DynamicEq::kNumBands] = { 6.0f, 5.0f, 5.0f, 3.0f, 6.0f, 5.0f };
} // namespace
void DynamicEq::prepare (double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    bank_.configure (sampleRate, numChannels, kCentres, kQs, kNumBands);
}

void DynamicEq::reset() noexcept
{
    bank_.reset();
}

void DynamicEq::setStrength (float strength01) noexcept
{
    strength_ = juce::jlimit (0.0f, 1.0f, strength01);
}

void DynamicEq::process (juce::AudioBuffer<float>& buffer, const SpeechFeatures& features) noexcept
{
    if (buffer.getNumSamples() <= 0 || bank_.numBands() == 0)
        return;

    bank_.detect (buffer);

    float referenceDb = 0.0f;

    for (int band = 0; band < kNumBands; ++band)
        referenceDb += bank_.envelopeDb (band);

    referenceDb /= (float) kNumBands;

    const float gate = juce::jlimit (0.0f, 1.0f, features.speechActivity * 1.5f);

    if (referenceDb <= -95.0f || gate <= 0.0f)
    {
        for (int band = 0; band < kNumBands; ++band)
            bank_.setCutDb (band, 0.0f);

        bank_.apply (buffer);
        return;
    }

    for (int band = 0; band < kNumBands; ++band)
    {
        const float excess = bank_.envelopeDb (band) - (referenceDb + kTiltDb[band]);
        const float amount = juce::jlimit (0.0f, 1.0f, (excess - 1.0f) / 6.0f);
        bank_.setCutDb (band, strength_ * kMaxCutDb[band] * amount * gate);
    }

    bank_.apply (buffer);
}

} // namespace voiceguard
