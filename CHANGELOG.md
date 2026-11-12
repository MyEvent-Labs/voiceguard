# Changelog

All notable changes to MyEvent Labs VoiceGuard are recorded here. The format
follows Keep a Changelog; the project aims for Semantic Versioning.

## [0.0.1-alpha] - 2026-11

First public Alpha. Real-time speech enhancement for live events, 100% local.

### Added

- Project scaffold: C++20, JUCE 8 (pinned), CMake 3.22+; Windows VST3 +
  Standalone and macOS VST3 + AU v2 + Standalone targets.
- Shared DSP core: input conditioning/HPF, conservative 10-band noise
  reduction, speech-aware dynamic EQ, harshness control, de-esser, slow level
  rider, gentle dynamics, output safety, and a shared `SpeechAnalyzer`.
- Four macros (CLEAN, CLARITY, LEVEL, PROTECT), AUTO mode, bypass and an
  advanced panel; per-channel meters and a per-stage gain-reduction row.
- Ten factory presets.
- `AudioProcessorValueTreeState` with 26 stable parameter IDs and versioned
  state IO (migration, corrupt/newer-state rejection, size cap).
- Offline test harness (56 tests), objective quality scorecard, bounded soak,
  an offline UI-snapshot tool, and CI for Windows and macOS.
- Documentation: architecture, DSP, validation, security, Alpha limitations and
  the interface design system.

### Fixed

- Macro knobs were too weak to hear; a convex macro curve plus wider effect
  ranges makes CLEAN, CLARITY, LEVEL and PROTECT clearly audible, and PROTECT
  now spans the safety ceiling.
- Noise reduction no longer dulls steady speech and no longer boosts room tone
  in pauses.
- Output safety now bounds the output regardless of the output trim.
- Dry-buffer overrun when a host supplies a block larger than the prepared size.

### Notes

- Alpha builds are unsigned test artifacts; macOS is ad-hoc signed and not
  notarised. Verify published SHA-256 checksums.
- De-Feedback integration is an experimental interface, off by default.
- Not for show-critical use without controlled testing.
