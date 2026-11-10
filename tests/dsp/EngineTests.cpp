#include "../TestRegistry.h"

#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"
#include "fixtures/Signals.h"

#include <algorithm>
#include <cmath>
#include <random>

using namespace voiceguard;

namespace
{
params::Parameters defaults()
{
    return params::Parameters {};
}

void fillNoise (juce::AudioBuffer<float>& buffer, uint32_t seed)
{
    std::mt19937 rng (seed);
    std::uniform_real_distribution<float> dist (-0.5f, 0.5f);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample (ch, i, dist (rng));
}
} // namespace

VG_TEST (engineStableAcrossRatesAndBlocks)
{
    const double rates[] = { 44100.0, 48000.0, 88200.0, 96000.0 };
    const int blocks[] = { 32, 64, 128, 256, 512, 1024 };

    for (double sr : rates)
    {
        for (int block : blocks)
        {
            VoiceGuardEngine engine;
            engine.prepare (sr, block, 2);
            engine.setParameters (defaults());

            for (int iter = 0; iter < 8; ++iter)
            {
                juce::AudioBuffer<float> buffer (2, block);
                fillNoise (buffer, (uint32_t) (11 + iter));
                engine.process (buffer);

                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < block; ++i)
                        VG_CHECK (std::isfinite (buffer.getSample (ch, i)));

                VG_CHECK (buffer.getMagnitude (0, block) <= 1.0f + 1.0e-3f);
            }
        }
    }
}

VG_TEST (engineDoesNotBoostSilence)
{
    VoiceGuardEngine engine;
    engine.prepare (48000.0, 512, 2);
    engine.setParameters (defaults());

    for (int iter = 0; iter < 60; ++iter)
    {
        juce::AudioBuffer<float> buffer (2, 512);
        buffer.clear();
        engine.process (buffer);
        VG_CHECK (buffer.getMagnitude (0, 512) < 1.0e-6f);
    }
}

VG_TEST (engineBypassIsTransparent)
{
    constexpr int block = 512;
    const int total = block * 60;

    auto signal = [] (int n)
    {
        std::vector<float> v ((size_t) n);
        for (int i = 0; i < n; ++i)
            v[(size_t) i] = 0.3f * (float) std::sin (juce::MathConstants<double>::twoPi * 700.0 * i / 48000.0);
        return v;
    } (total);

    VoiceGuardEngine engine;
    engine.prepare (48000.0, block, 1);

    auto p = defaults();
    p.bypass = true;
    engine.setParameters (p);

    juce::AudioBuffer<float> buffer (1, total);
    std::copy (signal.begin(), signal.end(), buffer.getWritePointer (0));

    for (int i = 0; i < total; i += block)
    {
        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), 1, i, block);
        engine.process (chunk);
    }

    // After the crossfade settles, bypassed output equals the dry input.
    for (int i = total - block; i < total; ++i)
        VG_CHECK_CLOSE (buffer.getSample (0, i), signal[(size_t) i], 1.0e-4);
}

VG_TEST (engineMonoAndStereoRemainFinite)
{
    for (int channels = 1; channels <= 2; ++channels)
    {
        VoiceGuardEngine engine;
        engine.prepare (48000.0, 256, channels);
        engine.setParameters (defaults());

        for (int iter = 0; iter < 20; ++iter)
        {
            juce::AudioBuffer<float> buffer (channels, 256);
            fillNoise (buffer, (uint32_t) (100 + iter));
            engine.process (buffer);

            VG_CHECK (std::isfinite (buffer.getMagnitude (0, 256)));
        }
    }
}
