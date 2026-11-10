#include "../TestRegistry.h"

#include "dsp/Biquad.h"

#include <cmath>
#include <vector>

using namespace voiceguard;

namespace
{
constexpr double kSr = 48000.0;
}

VG_TEST (biquadHighpassAttenuatesLowFrequencies)
{
    Biquad hpf;
    hpf.setHighpass (kSr, 80.0, 0.7071);

    VG_CHECK (hpf.magnitudeAt (kSr, 20.0) < 0.4f);
    VG_CHECK (hpf.magnitudeAt (kSr, 1000.0) > 0.9f);
    VG_CHECK (hpf.magnitudeAt (kSr, 80.0) > 0.6f);
    VG_CHECK (hpf.magnitudeAt (kSr, 80.0) < 0.8f);
}

VG_TEST (biquadLowpassAttenuatesHighFrequencies)
{
    Biquad lpf;
    lpf.setLowpass (kSr, 1000.0, 0.7071);

    VG_CHECK (lpf.magnitudeAt (kSr, 100.0) > 0.95f);
    VG_CHECK (lpf.magnitudeAt (kSr, 10000.0) < 0.1f);
}

VG_TEST (biquadPeakingBoostIsBounded)
{
    Biquad peak;
    peak.setPeaking (kSr, 1000.0, 1.0, 6.0);

    VG_CHECK_CLOSE (peak.magnitudeAt (kSr, 1000.0), 1.9953f, 0.05f);
    VG_CHECK (peak.magnitudeAt (kSr, 100.0) > 0.9f);
    VG_CHECK (peak.magnitudeAt (kSr, 100.0) < 1.1f);
}

VG_TEST (biquadImpulseResponseIsStableAndFinite)
{
    Biquad hpf;
    hpf.setHighpass (kSr, 80.0, 0.7071);

    std::vector<float> block (48000, 0.0f);
    block[0] = 1.0f;
    hpf.processBlock (block.data(), (int) block.size(), 0);

    for (float s : block)
        VG_CHECK (std::isfinite (s));

    // Energy must decay, not grow.
    VG_CHECK (std::abs (block[0]) > std::abs (block[40000]));
    VG_CHECK (std::abs (block[40000]) < 1.0e-3f);
}
