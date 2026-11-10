# MyEvent Labs VoiceGuard

Real-time speech enhancement for live events. VoiceGuard makes lectern,
lavalier, handheld, headset, panel, webinar and broadcast speech cleaner,
clearer, more consistent and safer, with as little added latency and
coloration as possible.

**Version:** v0.0.1 Alpha · **Status:** ALPHA — controlled testing only, not for show-critical use.

## What it is

A live-production tool, not a studio vocal-effects suite. The default
workflow is:

```
Insert VoiceGuard -> AUTO -> mix the show.
```

All processing is deterministic, local and offline. No account, no DRM, no
licence activation, no internet dependency. Microphone audio never leaves
the computer.

## Signal path

```
Input -> conditioning/HPF -> conservative noise reduction -> speech-aware
dynamic EQ -> harshness control -> de-esser -> slow level rider -> gentle
dynamics -> [optional De-Feedback] -> safety peak control -> Output
```

Every stage is internally bypassable. There is no intentional lookahead by
default; added algorithmic latency is 0 samples and is reported honestly to
the host.

## Platforms and formats

| Platform | Formats |
| --- | --- |
| Windows x64 | VST3, Standalone |
| macOS (Apple Silicon + Intel, Universal where supported) | VST3, Audio Unit v2, Standalone |

Built with C++20, JUCE 8 and CMake 3.22+.

## Building

Prerequisites: CMake >= 3.22, a C++20 compiler, Git, and network access for
the pinned JUCE fetch (or a local JUCE 8 checkout).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Offline / air-gapped build against a local JUCE checkout:

```bash
cmake -S . -B build -DVOICEGUARD_JUCE_PATH=/path/to/JUCE
```

Useful options: `-DVOICEGUARD_BUILD_TESTS=ON`, `-DVOICEGUARD_BUILD_TOOLS=ON`,
`-DVOICEGUARD_ENABLE_SANITIZERS=ON`.

## Installing

- Windows VST3: copy `VoiceGuard.vst3` to `C:\Program Files\Common Files\VST3\`.
- macOS VST3: copy `VoiceGuard.vst3` to `/Library/Audio/Plug-Ins/VST3/`.
- macOS AU: copy `VoiceGuard.component` to `/Library/Audio/Plug-Ins/Components/`.

See `release/INSTALL.txt` for the full instructions that ship with a build.

## Testing

```bash
cmake --build build --target voiceguard_tests
ctest --test-dir build --output-on-failure
```

## Documentation

- `docs/ARCHITECTURE.md` — module and threading architecture
- `docs/DSP.md` — signal chain and module behaviour
- `docs/VALIDATION.md` — build, test and plugin validation procedure
- `docs/SECURITY.md` — privacy, local-processing and reporting guidance
- `docs/ALPHA-LIMITATIONS.md` — what Alpha does and does not do
- `CHANGELOG.md`

## Licence

AGPL-3.0-or-later. See `LICENSE` and `THIRD-PARTY-NOTICES.md`.

## Known Alpha limitations

See `docs/ALPHA-LIMITATIONS.md`. In short: unsigned test builds, no AAX/LV2,
De-Feedback integration experimental and off by default, and no
show-critical use without controlled testing.
