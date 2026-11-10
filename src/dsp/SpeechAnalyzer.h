#pragma once

#include <array>
#include <memory>
#include <vector>

#include <juce_dsp/juce_dsp.h>

namespace voiceguard
{

/** Features shared by every processing module. All values are bounded and
    finite; [-100 dB ..] floors match the rest of the DSP core. */
struct SpeechFeatures
{
    float speechActivity = 0.0f;        // 0..1
    float noiseFloorDb = -100.0f;
    float rmsShortDb = -100.0f;
    float rmsLongDb = -100.0f;
    float peakDb = -100.0f;
    float crestFactorDb = 0.0f;
    float snrDb = 0.0f;
    float spectralCentroidHz = 0.0f;
    float spectralSlopeDbPerOct = 0.0f;
    float sibilanceConfidence = 0.0f;   // 0..1
    float harshnessConfidence = 0.0f;   // 0..1
    float tonality = 0.0f;              // 0..1
    float spectralStability = 1.0f;     // 0..1
};

/** Reusable speech analysis. Owns one FFT frame accumulator and all
    intermediate state; process() never allocates, locks or does IO. */
class SpeechAnalyzer
{
public:
    SpeechAnalyzer() = default;

    static constexpr int kFftOrder = 10;
    static constexpr int kFftSize = 1 << kFftOrder;
    static constexpr int kNumBands = 10;

    static float bandCentreHz (int band) noexcept;
    static float bandLowHz (int band) noexcept;
    static float bandHighHz (int band) noexcept;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset() noexcept;

    /** Analyse interleaved-per-channel planar audio. Any numSamples is valid,
        including 1. */
    void process (const float* const* channels, int numChannels, int numSamples) noexcept;

    const SpeechFeatures& features() const noexcept { return features_; }
    const std::array<float, kNumBands>& bandEnergyDb() const noexcept { return bandDb_; }

private:
    void processFrame() noexcept;

    double sampleRate_ = 48000.0;
    bool prepared_ = false;

    std::unique_ptr<juce::dsp::FFT> fft_;
    std::vector<float> window_;
    std::vector<float> fftData_;   // 2 * kFftSize
    std::vector<float> frame_;     // kFftSize
    std::vector<float> prevMag_;   // kFftSize / 2 + 1
    int fill_ = 0;

    std::array<int, kNumBands> bandLo_ {};
    std::array<int, kNumBands> bandHi_ {};
    int slopeLo_ = 0, slopeHi_ = 0;
    int flatLo_ = 0, flatHi_ = 0;

    float meanSqShort_ = 0.0f;
    float meanSqLong_ = 0.0f;
    float peakHoldDb_ = -100.0f;

    SpeechFeatures features_ {};
    std::array<float, kNumBands> bandDb_ {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpeechAnalyzer)
};

} // namespace voiceguard
