#include "dsp/SpeechAnalyzer.h"

#include "dsp/DspUtils.h"

#include <algorithm>
#include <cmath>

namespace voiceguard
{

namespace
{
constexpr float kShortTauSeconds = 0.050f;
constexpr float kLongTauSeconds = 0.500f;
constexpr float kActivityAttackSeconds = 0.020f;
constexpr float kActivityReleaseSeconds = 0.250f;
constexpr float kPeakDecayDbPerSecond = 10.0f;
constexpr float kEpsilon = 1.0e-12f;

// Band edges in Hz. These are search regions, not mandatory processing bands.
constexpr float kBandEdges[SpeechAnalyzer::kNumBands + 1] = {
    0.0f, 100.0f, 250.0f, 400.0f, 700.0f, 1500.0f, 2500.0f, 4500.0f, 7000.0f, 10000.0f, 20000.0f
};

float onePoleCoeff (float seconds, float tau) noexcept
{
    if (tau <= 0.0f)
        return 1.0f;

    return 1.0f - std::exp (-seconds / tau);
}

int hzToBin (float hz, double sampleRate, int fftSize) noexcept
{
    const auto bin = (int) std::lround (hz * (double) fftSize / sampleRate);
    return juce::jlimit (1, fftSize / 2, bin);
}
} // namespace

float SpeechAnalyzer::bandLowHz (int band) noexcept
{
    return kBandEdges[(size_t) juce::jlimit (0, kNumBands - 1, band)];
}

float SpeechAnalyzer::bandHighHz (int band) noexcept
{
    return kBandEdges[(size_t) juce::jlimit (0, kNumBands - 1, band) + 1];
}

float SpeechAnalyzer::bandCentreHz (int band) noexcept
{
    const auto lo = bandLowHz (band);
    const auto hi = bandHighHz (band);
    return std::sqrt (juce::jmax (1.0f, lo) * hi);
}

void SpeechAnalyzer::prepare (double sampleRate, int /*maxBlockSize*/, int /*numChannels*/)
{
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 48000.0;
    prepared_ = true;

    fft_ = std::make_unique<juce::dsp::FFT> (kFftOrder);

    window_.resize ((size_t) kFftSize);
    for (int i = 0; i < kFftSize; ++i)
        window_[(size_t) i] = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) i
                                                      / (float) (kFftSize - 1));

    fftData_.assign ((size_t) kFftSize * 2, 0.0f);
    frame_.assign ((size_t) kFftSize, 0.0f);
    prevMag_.assign ((size_t) kFftSize / 2 + 1, 0.0f);

    for (int band = 0; band < kNumBands; ++band)
    {
        bandLo_[(size_t) band] = hzToBin (bandLowHz (band), sampleRate_, kFftSize);
        bandHi_[(size_t) band] = hzToBin (bandHighHz (band), sampleRate_, kFftSize);
        bandHi_[(size_t) band] = juce::jmax (bandLo_[(size_t) band], bandHi_[(size_t) band]);
    }

    slopeLo_ = hzToBin (500.0f, sampleRate_, kFftSize);
    slopeHi_ = hzToBin (8000.0f, sampleRate_, kFftSize);
    flatLo_ = hzToBin (100.0f, sampleRate_, kFftSize);
    flatHi_ = hzToBin (8000.0f, sampleRate_, kFftSize);

    reset();
}

void SpeechAnalyzer::reset() noexcept
{
    fill_ = 0;
    meanSqShort_ = 0.0f;
    meanSqLong_ = 0.0f;
    peakHoldDb_ = -100.0f;
    features_ = SpeechFeatures {};
    bandDb_.fill (-100.0f);
    std::fill (prevMag_.begin(), prevMag_.end(), 0.0f);
    std::fill (frame_.begin(), frame_.end(), 0.0f);
}

void SpeechAnalyzer::process (const float* const* channels, int numChannels, int numSamples) noexcept
{
    if (! prepared_ || channels == nullptr || numChannels <= 0 || numSamples <= 0)
        return;

    ScopedDenormalDisable noDenormals;

    double blockSumSq = 0.0;
    float blockPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float mono = 0.0f;

        for (int ch = 0; ch < numChannels; ++ch)
            if (channels[ch] != nullptr)
                mono += channels[ch][i];

        mono /= (float) numChannels;
        sanitize (mono);

        blockSumSq += (double) mono * (double) mono;
        blockPeak = juce::jmax (blockPeak, std::abs (mono));

        frame_[(size_t) fill_++] = mono;

        if (fill_ >= kFftSize)
        {
            processFrame();
            fill_ = 0;
        }
    }

    const auto blockSeconds = (float) ((double) numSamples / sampleRate_);
    const auto blockMeanSq = (float) (blockSumSq / (double) juce::jmax (1, numSamples));

    meanSqShort_ += (blockMeanSq - meanSqShort_) * onePoleCoeff (blockSeconds, kShortTauSeconds);
    meanSqLong_ += (blockMeanSq - meanSqLong_) * onePoleCoeff (blockSeconds, kLongTauSeconds);

    const float blockPeakDb = gainToDb (blockPeak);
    peakHoldDb_ = juce::jmax (blockPeakDb, peakHoldDb_ - kPeakDecayDbPerSecond * blockSeconds);

    features_.rmsShortDb = gainToDb (std::sqrt (juce::jmax (kEpsilon, meanSqShort_)));
    features_.rmsLongDb = gainToDb (std::sqrt (juce::jmax (kEpsilon, meanSqLong_)));
    features_.peakDb = peakHoldDb_;
    features_.crestFactorDb = juce::jlimit (0.0f, 40.0f, peakHoldDb_ - features_.rmsShortDb);

    // Noise floor is tracked per FFT frame; see processFrame().
    features_.snrDb = juce::jlimit (0.0f, 60.0f, features_.rmsShortDb - features_.noiseFloorDb);

    // Speech activity from SNR, with an asymmetric smoother.
    const float activityTarget = juce::jlimit (0.0f, 1.0f, (features_.snrDb - 3.0f) / 12.0f);
    const float tau = activityTarget > features_.speechActivity ? kActivityAttackSeconds
                                                                : kActivityReleaseSeconds;
    features_.speechActivity += (activityTarget - features_.speechActivity)
                                * onePoleCoeff (blockSeconds, tau);
    features_.speechActivity = juce::jlimit (0.0f, 1.0f, features_.speechActivity);

    if (! std::isfinite (features_.speechActivity))
        features_.speechActivity = 0.0f;
}

void SpeechAnalyzer::processFrame() noexcept
{
    if (fft_ == nullptr || fftData_.size() < (size_t) kFftSize * 2)
        return;

    // Noise floor: fall quickly to the true floor, rise slowly so speech does
    // not drag the estimate up. Per frame, so it is block-size independent.
    double frameSumSq = 0.0;
    for (int i = 0; i < kFftSize; ++i)
        frameSumSq += (double) frame_[(size_t) i] * (double) frame_[(size_t) i];

    const float frameDb = gainToDb ((float) std::sqrt (frameSumSq / (double) kFftSize));

    if (frameDb < features_.noiseFloorDb)
        features_.noiseFloorDb += (frameDb - features_.noiseFloorDb) * 0.35f;
    else
        features_.noiseFloorDb += (frameDb - features_.noiseFloorDb) * 0.02f;

    features_.noiseFloorDb = juce::jlimit (-100.0f, -20.0f, features_.noiseFloorDb);

    for (int i = 0; i < kFftSize; ++i)
    {
        fftData_[(size_t) i] = frame_[(size_t) i] * window_[(size_t) i];
        fftData_[(size_t) (kFftSize + i)] = 0.0f;
    }

    fft_->performFrequencyOnlyForwardTransform (fftData_.data());

    const int numBins = kFftSize / 2 + 1;
    const float binHz = (float) (sampleRate_ / (double) kFftSize);

    float totalEnergy = 0.0f;
    float centroidNum = 0.0f;
    float centroidDen = 0.0f;
    float logSum = 0.0f;
    int flatCount = 0;

    float slopeSumX = 0.0f, slopeSumY = 0.0f, slopeSumXX = 0.0f, slopeSumXY = 0.0f;
    int slopeCount = 0;

    float stabilityNum = 0.0f, stabilityDenA = 0.0f, stabilityDenB = 0.0f;

    for (int band = 0; band < kNumBands; ++band)
    {
        float energy = 0.0f;
        for (int bin = bandLo_[(size_t) band]; bin <= bandHi_[(size_t) band]; ++bin)
            energy += fftData_[(size_t) bin] * fftData_[(size_t) bin];

        bandDb_[(size_t) band] = 10.0f * std::log10 (energy + kEpsilon);
        totalEnergy += energy;
    }

    for (int bin = 1; bin < numBins; ++bin)
    {
        const float mag = fftData_[(size_t) bin];
        const float freq = (float) bin * binHz;

        centroidNum += freq * mag;
        centroidDen += mag;

        const float prev = prevMag_[(size_t) bin];
        stabilityNum += mag * prev;
        stabilityDenA += mag * mag;
        stabilityDenB += prev * prev;

        if (bin >= flatLo_ && bin <= flatHi_)
        {
            logSum += std::log (mag + 1.0e-9f);
            ++flatCount;
        }

        if (bin >= slopeLo_ && bin <= slopeHi_)
        {
            const float x = std::log2 (juce::jmax (1.0f, freq));
            const float y = 20.0f * std::log10 (mag + 1.0e-9f);
            slopeSumX += x;
            slopeSumY += y;
            slopeSumXX += x * x;
            slopeSumXY += x * y;
            ++slopeCount;
        }

        prevMag_[(size_t) bin] = mag;
    }

    features_.spectralCentroidHz = centroidDen > kEpsilon
                                       ? juce::jlimit (0.0f, 20000.0f, centroidNum / centroidDen)
                                       : 0.0f;

    if (slopeCount > 1)
    {
        const float n = (float) slopeCount;
        const float denom = n * slopeSumXX - slopeSumX * slopeSumX;
        features_.spectralSlopeDbPerOct = std::abs (denom) > kEpsilon
                                              ? (n * slopeSumXY - slopeSumX * slopeSumY) / denom
                                              : 0.0f;
    }

    // Tonality via spectral flatness over the speech range.
    float flatness = 1.0f;
    if (flatCount > 0)
    {
        const float geometric = std::exp (logSum / (float) flatCount);
        float arithSum = 0.0f;
        for (int bin = flatLo_; bin <= flatHi_; ++bin)
            arithSum += fftData_[(size_t) bin];

        const float mean = arithSum / (float) flatCount;
        flatness = mean > kEpsilon ? juce::jlimit (0.0f, 1.0f, geometric / mean) : 1.0f;
    }

    features_.tonality = juce::jlimit (0.0f, 1.0f, 1.0f - flatness);

    if (stabilityDenA > kEpsilon && stabilityDenB > kEpsilon)
    {
        const float corr = stabilityNum / (std::sqrt (stabilityDenA) * std::sqrt (stabilityDenB));
        features_.spectralStability = juce::jlimit (0.0f, 1.0f, corr);
    }

    // Sibilance: 4.5-10 kHz share of total energy, gated by speech activity.
    const float sibEnergy = std::pow (10.0f, bandDb_[(size_t) 7] / 10.0f)
                            + std::pow (10.0f, bandDb_[(size_t) 8] / 10.0f);
    const float sibRatio = sibEnergy / (totalEnergy + kEpsilon);
    const float sibRaw = juce::jlimit (0.0f, 1.0f, (sibRatio - 0.12f) / 0.30f);
    features_.sibilanceConfidence = sibRaw * juce::jlimit (0.0f, 1.0f, features_.speechActivity * 1.5f);

    // Harshness: 2.5-6 kHz emphasis relative to the 1.5-2.5 kHz region.
    const float harshEnergy = std::pow (10.0f, bandDb_[(size_t) 6] / 10.0f)
                              + std::pow (10.0f, bandDb_[(size_t) 7] / 10.0f);
    const float refEnergy = std::pow (10.0f, bandDb_[(size_t) 5] / 10.0f);
    const float harshRatio = harshEnergy / (refEnergy + kEpsilon);
    const float harshRaw = juce::jlimit (0.0f, 1.0f, (harshRatio - 0.6f) / 0.9f);
    features_.harshnessConfidence = harshRaw * juce::jlimit (0.0f, 1.0f, features_.speechActivity * 1.5f);
}

} // namespace voiceguard
