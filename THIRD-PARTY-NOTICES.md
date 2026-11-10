# Third-Party Notices

VoiceGuard v0.0.1 Alpha is distributed under the GNU Affero General Public
License v3.0 or later (AGPL-3.0-or-later). The following third-party
components are used.

## JUCE

- Component: JUCE (Jules' Utility Class Extensions) 8.0.17-security
- Source: https://github.com/juce-framework/JUCE
- Pinned commit: e548e97a54a083acbfc330d584419fd074409045
- Licence: dual-licensed — GNU Affero General Public License v3.0, or a
  commercial JUCE licence from Raw Material Software Limited.

**This build uses JUCE under the AGPL-3.0 option.** That is why VoiceGuard
itself is released under AGPL-3.0-or-later and its complete corresponding
source is published at https://github.com/MyEvent-Labs/voiceguard.

If MyEvent Labs later obtains a commercial JUCE licence, the project may be
relicensed; until then AGPL-3.0 obligations apply to every distributed build.

## Standard library

The C++ standard library is used under the terms of the toolchain that
provides it (Microsoft Visual C++ runtime on Windows, libc++ on macOS).
These are not bundled with the plugin.

## No other runtime dependencies

VoiceGuard v0.0.1 links no cloud SDKs, telemetry libraries, analytics,
transcription engines or network clients. `JUCE_USE_CURL=0`,
`JUCE_WEB_BROWSER=0` and `JUCE_REPORT_APP_USAGE=0` are set for all targets.

If a future version adds a dependency, its licence and purpose are added
here and reviewed before release.
