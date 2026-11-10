#pragma once

// Deterministic, redistributable synthetic fixtures for offline tests.

#include <cmath>
#include <random>
#include <vector>

namespace vgfixture
{

inline std::vector<float> silence (int n)
{
    return std::vector<float> ((size_t) n, 0.0f);
}

inline std::vector<float> tone (double sr, int n, double freq, float amp)
{
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
        out[(size_t) i] = amp * (float) std::sin (juce::MathConstants<double>::twoPi * freq * i / sr);
    return out;
}

/** Speech-like: four formants with syllabic amplitude modulation, optional
    extra tonal component at boostFreq for colouration tests. */
inline std::vector<float> speechLike (double sr, int n, float amp,
                                      double boostFreq = 0.0, float boostAmp = 0.0f)
{
    std::vector<float> out ((size_t) n);
    const double formants[4] = { 150.0, 700.0, 1200.0, 2500.0 };
    const float weights[4] = { 0.40f, 0.30f, 0.25f, 0.15f };

    for (int i = 0; i < n; ++i)
    {
        const double t = i / sr;
        const double am = 0.5 + 0.5 * std::sin (juce::MathConstants<double>::twoPi * 4.0 * t);
        double s = 0.0;
        for (int f = 0; f < 4; ++f)
            s += weights[f] * std::sin (juce::MathConstants<double>::twoPi * formants[f] * t);

        if (boostFreq > 0.0)
            s += boostAmp * std::sin (juce::MathConstants<double>::twoPi * boostFreq * t);

        out[(size_t) i] = amp * (float) (s * am);
    }
    return out;
}

inline std::vector<float> noise (int n, float amp, uint32_t seed)
{
    std::mt19937 rng (seed);
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
        out[(size_t) i] = amp * dist (rng);
    return out;
}

/** High-passed noise, standing in for a sibilant burst. */
inline std::vector<float> hfNoise (double sr, int n, float amp, uint32_t seed)
{
    auto out = noise (n, amp, seed);
    const float a = std::exp (-juce::MathConstants<float>::twoPi * 5000.0f / (float) sr);
    float lp = 0.0f;
    for (auto& s : out)
    {
        lp += (s - lp) * (1.0f - a);
        s = s - lp;
    }
    return out;
}

/** Add two equal-length signals in place. */
inline void addInto (std::vector<float>& a, const std::vector<float>& b)
{
    const auto n = std::min (a.size(), b.size());
    for (size_t i = 0; i < n; ++i)
        a[i] += b[i];
}

/** Amplitude of a single frequency component. */
inline float componentAmp (const std::vector<float>& v, double sr, double freq)
{
    double re = 0.0;
    double im = 0.0;
    const double w = juce::MathConstants<double>::twoPi * freq / sr;

    for (size_t i = 0; i < v.size(); ++i)
    {
        re += (double) v[i] * std::cos (w * (double) i);
        im += (double) v[i] * std::sin (w * (double) i);
    }

    return (float) (2.0 * std::sqrt (re * re + im * im) / (double) v.size());
}

inline float rms (const std::vector<float>& v)
{
    double sum = 0.0;
    for (float s : v)
        sum += (double) s * s;
    return v.empty() ? 0.0f : (float) std::sqrt (sum / (double) v.size());
}

} // namespace vgfixture
