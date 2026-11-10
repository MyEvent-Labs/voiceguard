# Changelog

All notable changes to MyEvent Labs VoiceGuard are recorded here. The
format follows Keep a Changelog; the project aims for Semantic Versioning.

## [0.0.1-alpha] - unreleased

First public Alpha. Real-time speech enhancement for live events, 100% local.

### Added

- Project scaffold: C++20, JUCE 8.0.17-security (pinned), CMake 3.22+.
- Windows VST3 + Standalone targets; macOS VST3 + AU v2 + Standalone targets.
- Shared DSP core, offline test harness and CI for Windows and macOS.

### Notes

- Alpha builds are unsigned test artifacts unless signing credentials are
  supplied. See `docs/ALPHA-LIMITATIONS.md`.
- De-Feedback integration is an experimental interface, off by default.
- Not for show-critical use without controlled testing.
