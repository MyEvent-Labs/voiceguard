# Alpha limitations

VoiceGuard v0.0.1 is an **Alpha**. It is free, local and experimental. Do not
use it for show-critical work without controlled testing.

## What it does not yet include

- **AAX, LV2 and other formats** are not built. v0.0.1 ships Windows VST3 +
  Standalone and macOS VST3 + AU v2 + Standalone. AAX/LV2/Fourier/Waves are
  future research, not v0.0.1.
- **De-Feedback** integration is an experimental interface and is **off by
  default**; VoiceGuard is fully stable without it.
- **Factory presets** are constrained starting points, not finished looks.

## Validation gaps

- The **2-hour soak** is a documented manual step; the automated suite runs a
  bounded soak.
- **CPU, memory and maximum simultaneous instances** are not yet benchmarked, so
  no instance count is advertised.
- The Steinberg VST3 validator is not part of the pipeline (pluginval and `auval`
  are).

## Signing and notarisation

- Builds are **unsigned**. macOS is ad-hoc signed only and **not notarised**.
  SmartScreen / Gatekeeper will warn; verify the published SHA-256 checksums
  rather than weakening OS security.

## Character

- DSP voicing is deliberately conservative and still being tuned by ear.
- AUTO establishes conservative starting settings after roughly five seconds of
  representative speech; it never rewrites the sound aggressively.
