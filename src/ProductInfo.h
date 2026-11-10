#pragma once

/** Compile-time product identity. Kept independent of the JUCE-generated
    JuceHeader so that the DSP core and the offline tests can use these
    values without depending on plugin-format macros. */
namespace voiceguard::product
{

inline constexpr const char* companyName = "MyEvent Labs";
inline constexpr const char* productName = "MyEvent Labs VoiceGuard";
inline constexpr const char* shortName   = "VoiceGuard";
inline constexpr const char* version     = "0.0.1";
inline constexpr const char* versionTag  = "v0.0.1-alpha";
inline constexpr const char* bundleId    = "com.myeventlabs.voiceguard";

} // namespace voiceguard::product
