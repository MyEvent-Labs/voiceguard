#include "../TestRegistry.h"

#include "dsp/Parameters.h"
#include "dsp/VoiceGuardEngine.h"
#include "fixtures/Signals.h"

#include <cmath>
#include <random>

using namespace voiceguard;

// Bounded soak: a few minutes of audio with bypass/preset automation and
// repeated lifecycle reinitialisation. The full 2-hour soak is a documented
// manual step; this catches regressions cheaply on every run.

VG_TEST (engineSurvivesSoakWithAutomation)
{
    constexpr double sr = 48000.0;
    constexpr int block = 256;
    constexpr int seconds = 120; // 2 minutes of audio, processed offline

    VoiceGuardEngine engine;
    engine.prepare (sr, block, 2);

    std::mt19937 rng (99);
    std::uniform_real_distribution<float> dist (-0.5f, 0.5f);

    auto p = params::Parameters {};

    for (int n = 0; n < seconds * (int) (sr / block); ++n)
    {
        juce::AudioBuffer<float> buffer (2, block);

        // Alternate speech-like, noise and silence every 4 blocks.
        const int phase = (n / 4) % 3;

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < block; ++i)
            {
                const double t = (double) (n * block + i) / sr;
                float s = 0.0f;

                if (phase == 0)
                    s = 0.25f * (float) std::sin (juce::MathConstants<double>::twoPi * 220.0 * t)
                        + 0.10f * (float) std::sin (juce::MathConstants<double>::twoPi * 1800.0 * t);
                else if (phase == 1)
                    s = 0.02f * dist (rng);

                buffer.setSample (ch, i, s);
            }
        }

        // Automate bypass and a few parameters periodically.
        if (n % 100 == 0)
        {
            p.bypass = (n / 100) % 2 == 0;
            p.macroClean = 0.3f + 0.4f * (float) ((n / 100) % 3) / 2.0f;
            p.macroClarity = 0.4f;
            p.levelTargetDb = -18.0f + (float) ((n / 100) % 4);
            engine.setParameters (p);
        }

        engine.process (buffer);

        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < block; ++i)
                VG_CHECK (std::isfinite (buffer.getSample (ch, i)));

        VG_CHECK (buffer.getMagnitude (0, block) <= 1.0f + 1.0e-3f);
    }
}

VG_TEST (engineSurvivesRepeatedLifecycle)
{
    for (int cycle = 0; cycle < 50; ++cycle)
    {
        VoiceGuardEngine engine;
        const double sr = (cycle % 2 == 0) ? 44100.0 : 96000.0;
        engine.prepare (sr, 128, 2);
        engine.setParameters (params::Parameters {});

        juce::AudioBuffer<float> buffer (2, 128);

        for (int i = 0; i < 128; ++i)
        {
            buffer.setSample (0, i, 0.1f);
            buffer.setSample (1, i, 0.1f);
        }

        engine.process (buffer);
        VG_CHECK (buffer.getMagnitude (0, 128) <= 1.0f);
        engine.reset();
    }
}
