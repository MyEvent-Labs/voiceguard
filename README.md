# MyEvent Labs VoiceGuard

Real-time speech enhancement for live events. VoiceGuard makes lectern,
lavalier, handheld, headset, panel, webinar and broadcast speech cleaner,
clearer, more consistent and safer, with as little added latency and
coloration as possible.

**Version:** v0.0.1 Alpha · **Status:** ALPHA — controlled testing only, not for
show-critical use.

## What it is

A live-production tool, not a studio vocal-effects suite. The default workflow:

```
Insert VoiceGuard -> AUTO -> mix the show.
```

The four macros — **CLEAN, CLARITY, LEVEL, PROTECT** — are the primary controls;
the advanced panel is there when an engineer wants it. Ten factory presets
(Auto, Lectern Cleanup, Wireless Lav, Handheld Speech, Panel Discussion,
Broadcast Voice, Streaming, Noisy Room, Bright Lavalier, Quiet Presenter) are
constrained starting points.

All processing is deterministic, local and offline. No account, no DRM, no
licence activation, no internet dependency. Microphone audio never leaves the
computer.

## Signal path

```
Input -> conditioning/HPF -> conservative noise reduction -> speech-aware
dynamic EQ -> harshness control -> de-esser -> slow level rider -> gentle
dynamics -> [optional De-Feedback] -> safety peak control -> output trim
```

Every stage is internally bypassable. There is no intentional lookahead; added
algorithmic latency is 0 samples and is reported honestly to the host.

## Platforms and formats

| Platform | Formats |
| --- | --- |
| Windows x64 | VST3, Standalone |
| macOS (Apple Silicon + Intel, Universal where supported) | VST3, Audio Unit v2, Standalone |

Built with C++20, JUCE 8 (pinned) and CMake 3.22+.

## Building

Prerequisites: CMake >= 3.22, a C++20 compiler, Git, and network access for the
pinned JUCE fetch (or a local JUCE 8 checkout).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Offline / air-gapped build against a local JUCE checkout:

```bash
cmake -S . -B build -DVOICEGUARD_JUCE_PATH=/path/to/JUCE
```

Options: `-DVOICEGUARD_BUILD_TESTS=ON`, `-DVOICEGUARD_BUILD_TOOLS=ON`,
`-DVOICEGUARD_ENABLE_SANITIZERS=ON`.

## Installing

Packaged builds and their SHA-256 checksums are attached to the pre-release on
the MyEvent Labs GitHub organisation. From the Windows installer or manual copy:

- Windows VST3: `C:\Program Files\Common Files\VST3\`
- macOS VST3: `/Library/Audio/Plug-Ins/VST3/`
- macOS AU: `/Library/Audio/Plug-Ins/Components/`

See `installer/INSTALL.txt` for the instructions that ship with a build.

## Testing

```bash
cmake --build build --config Release --target voiceguard_tests
ctest --test-dir build -C Release --output-on-failure
```

## Documentation

- `docs/ARCHITECTURE.md` — targets, layering, threading contract
- `docs/DSP.md` — signal chain, macros and modules
- `docs/VALIDATION.md` — build, test, CI and plugin validation
- `docs/SECURITY.md` — local-processing and privacy posture
- `docs/ALPHA-LIMITATIONS.md` — what Alpha does and does not do
- `docs/DESIGN.md` — the interface design system
- `PRODUCT.md` — product truth and audience
- `CHANGELOG.md`, `THIRD-PARTY-NOTICES.md`

## Licence

AGPL-3.0-or-later. See `LICENSE` and `THIRD-PARTY-NOTICES.md`.

## Known Alpha limitations

See `docs/ALPHA-LIMITATIONS.md`. In short: unsigned test builds (macOS not
notarised), no AAX/LV2, De-Feedback integration experimental and off by default,
the 2-hour soak is a manual step, and no show-critical use without controlled
testing.
