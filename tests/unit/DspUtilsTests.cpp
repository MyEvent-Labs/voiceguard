#include "../TestRegistry.h"

#include "dsp/DspUtils.h"

#include <cmath>
#include <limits>

using namespace voiceguard;

VG_TEST (dbGainRoundTrip)
{
    VG_CHECK_CLOSE (dbToGain (0.0f), 1.0f, 1.0e-6);
    VG_CHECK_CLOSE (dbToGain (6.0f), 1.9952623f, 1.0e-4);
    VG_CHECK_CLOSE (gainToDb (1.0f), 0.0f, 1.0e-5);
    VG_CHECK_CLOSE (gainToDb (0.5f), -6.0206f, 1.0e-3);
    VG_CHECK (dbToGain (kMinDb) == 0.0f);
    VG_CHECK (gainToDb (0.0f) <= kMinDb);
}

VG_TEST (sanitizeRejectsNonFinite)
{
    float nan = std::numeric_limits<float>::quiet_NaN();
    float inf = std::numeric_limits<float>::infinity();

    VG_CHECK (! sanitize (nan));
    VG_CHECK (nan == 0.0f);
    VG_CHECK (! sanitize (inf));
    VG_CHECK (inf == 0.0f);

    float good = 0.25f;
    VG_CHECK (sanitize (good));
    VG_CHECK_CLOSE (good, 0.25f, 1.0e-9);

    VG_CHECK (! isFinite (std::numeric_limits<float>::quiet_NaN()));
    VG_CHECK (isFinite (1.0f));
}

VG_TEST (sanitizeBufferRepairsAndReports)
{
    juce::AudioBuffer<float> buffer (2, 8);
    buffer.clear();

    float nan = std::numeric_limits<float>::quiet_NaN();
    buffer.setSample (0, 0, nan);
    buffer.setSample (1, 4, std::numeric_limits<float>::infinity());

    VG_CHECK (! sanitizeBuffer (buffer));
    VG_CHECK (buffer.getSample (0, 0) == 0.0f);
    VG_CHECK (buffer.getSample (1, 4) == 0.0f);

    juce::AudioBuffer<float> clean (1, 4);
    clean.clear();
    VG_CHECK (sanitizeBuffer (clean));
}

VG_TEST (clampOrHandlesGarbage)
{
    VG_CHECK_CLOSE (clampOr (5.0f, 0.0f, 10.0f, 1.0f), 5.0f, 1.0e-9);
    VG_CHECK_CLOSE (clampOr (-5.0f, 0.0f, 10.0f, 1.0f), 0.0f, 1.0e-9);
    VG_CHECK_CLOSE (clampOr (50.0f, 0.0f, 10.0f, 1.0f), 10.0f, 1.0e-9);
    VG_CHECK_CLOSE (clampOr (std::numeric_limits<float>::quiet_NaN(), 0.0f, 10.0f, 3.0f), 3.0f, 1.0e-9);
}
