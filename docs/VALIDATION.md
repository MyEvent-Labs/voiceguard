# Validation

Everything below is expected to pass before a build is published.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Windows uses the Visual Studio generator or Ninja with the MSVC environment.

## Offline tests

```bash
cmake --build build --config Release --target voiceguard_tests
ctest --test-dir build -C Release --output-on-failure
```

The suite (56 tests) covers, among others:

- stability across 44.1 / 48 / 88.2 / 96 kHz and 32–1024 sample blocks
- NaN/Inf sanitising, bounded output, silence stays silent
- click-free bypass, mono/stereo correctness
- parameter clamps and versioned state round-trip, corrupt-state fallback
- presets apply and serialise
- the quality scorecard: pause-noise reduction, level spread, clean-speech
  transparency, sibilance, musical-noise (tonality), macro audibility
- a bounded soak (2 minutes of automated audio plus 50 lifecycle reinits)

The full 2-hour soak is a manual step on production hardware.

## CI

`.github/workflows/build.yml` builds and tests on every push:

- **Windows x64**: VST3 + Standalone.
- **macOS (universal)**: VST3 + AU + Standalone, and runs `auval` on the AU.

## Plugin validation

- `pluginval` v1.0.4, strictness level 5, GUI tests skipped — **SUCCESS**.
- `auval` on macOS — passes in CI.
- Steinberg VST3 validator — not part of the pipeline.

## Visual review

`tools/ui_snapshot` renders the editor offscreen to a PNG
(`voiceguard_ui_snapshot ui-snapshot.png`) for layout review without a host.

## Latency

0 added algorithmic samples by design (lookahead-free) and reported to the host
with `setLatencySamples`. It is not host-measured in CI.
