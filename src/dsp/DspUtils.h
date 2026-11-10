#pragma once

#include <cmath>
#include <cstdint>

#include <juce_audio_basics/juce_audio_basics.h>

namespace voiceguard
{

/** Floor for dB conversions; at or below this is treated as silence. */
inline constexpr float kMinDb = -100.0f;

/** Convert dB to linear gain. */
float dbToGain (float db) noexcept;

/** Convert linear gain to dB. */
float gainToDb (float gain) noexcept;

/** True when x is finite (not NaN or +/-Inf). */
bool isFinite (float x) noexcept;

/** Replace NaN/Inf with 0 and return true when the original was finite. */
bool sanitize (float& x) noexcept;

/** Sanitize every sample of a buffer in place. Returns false if any were repaired. */
bool sanitizeBuffer (juce::AudioBuffer<float>& buffer) noexcept;

/** Clamp to [lo, hi]; a non-finite value becomes fallback. */
float clampOr (float value, float lo, float hi, float fallback) noexcept;

/** RAII guard that flushes denormals to zero for the current thread. */
using ScopedDenormalDisable = juce::ScopedNoDenormals;

} // namespace voiceguard
