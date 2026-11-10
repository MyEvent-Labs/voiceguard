#include "../TestRegistry.h"

#include "dsp/Biquad.h"
#include "dsp/DeEsser.h"
#include "dsp/DspUtils.h"
#include "dsp/HarshnessControl.h"
#include "dsp/SpeechAnalyzer.h"
#include "fixtures/Signals.h"

#include <algorithm>
#include <functional>
#include <vector>

using namespace voiceguard;
using namespace vgfixture;

namespace
{
constexpr double kSr = 48000.0;

float highBandRms (const std::vector<float>& in)
{
    Biquad bp;
    bp.setBandpass (kSr, 5500.0, 1.0);

    auto copy = in;
    bp.processBlock (copy.data(), (int) copy.size(), 0);
    return rms (copy);
}

std::vector<float> runMono (const std::vector<float>& in, int block,
                            std::function<void (juce::AudioBuffer<float>&, const SpeechFeatures&)> process)
{
    SpeechAnalyzer analyzer;
    analyzer.prepare (kSr, block, 1);

    juce::AudioBuffer<float> buffer (1, (int) in.size());
    std::copy (in.begin(), in.end(), buffer.getWritePointer (0));

    for (int i = 0; i < (int) in.size(); i += block)
    {
        const int n = juce::jmin (block, (int) in.size() - i);
        const float* ptrs[1] = { buffer.getReadPointer (0) + i };
        analyzer.process (ptrs, 1, n);

        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 1, i, n);
        process (chunk, analyzer.features());
    }

    return std::vector<float> (buffer.getReadPointer (0), buffer.getReadPointer (0) + in.size());
}

std::vector<float> runDeEsser (const std::vector<float>& in, float amount)
{
    DeEsser deEsser;
    deEsser.prepare (kSr, 512, 1);
    deEsser.setAmount (amount);

    return runMono (in, 512, [&deEsser] (juce::AudioBuffer<float>& b, const SpeechFeatures& f)
                    { deEsser.process (b, f); });
}

std::vector<float> runHarshness (const std::vector<float>& in, float amount)
{
    HarshnessControl harshness;
    harshness.prepare (kSr, 512, 1);
    harshness.setAmount (amount);

    return runMono (in, 512, [&harshness] (juce::AudioBuffer<float>& b, const SpeechFeatures& f)
                    { harshness.process (b, f); });
}
} // namespace

VG_TEST (deEsserReducesSibilantEnergy)
{
    auto signal = speechLike (kSr, 96000, 0.3f);
    addInto (signal, hfNoise (kSr, 96000, 0.8f, 41));

    const auto out = runDeEsser (signal, 1.0f);

    const float reduction = gainToDb (highBandRms (signal)) - gainToDb (highBandRms (out));
    VG_CHECK (reduction > 1.0f);
    VG_CHECK (reduction < 8.5f);
}

VG_TEST (deEsserDoesNotDullNeutralSpeech)
{
    const auto signal = speechLike (kSr, 96000, 0.3f);
    const auto out = runDeEsser (signal, 1.0f);

    const float reduction = gainToDb (highBandRms (signal)) - gainToDb (highBandRms (out));
    VG_CHECK (reduction < 1.0f);
}

VG_TEST (harshnessControlCutsBrightSpeech)
{
    const auto signal = speechLike (kSr, 96000, 0.3f, 4000.0, 0.5f);
    const auto out = runHarshness (signal, 1.0f);

    const float change = gainToDb (componentAmp (signal, kSr, 4000.0))
                         - gainToDb (componentAmp (out, kSr, 4000.0));
    VG_CHECK (change > 0.2f);
}
