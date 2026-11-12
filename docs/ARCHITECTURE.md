# Architecture

VoiceGuard is a JUCE 8 / C++20 audio plugin. The code is split so the DSP can be
built and tested without any GUI or plugin-format code.

## Targets

| Target | Role |
| --- | --- |
| `voiceguard_core` | GUI-free static library: DSP modules, shared analysis, parameter snapshot, state IO, presets. |
| `VoiceGuard` | The plugin: VST3, AU (macOS) and Standalone, each with its editor. |
| `voiceguard_tests` | Offline console test harness (no audio device, no GUI). |
| `voiceguard_ui_snapshot` | Offline tool that renders the editor to a PNG for visual review. |

## Layering

```
PluginEditor  ->  PluginProcessor  ->  VoiceGuardEngine  ->  DSP modules
   (atomics)        (APVTS/state)        (chain + macros)     (per stage)
```

- `VoiceGuardEngine` owns the signal chain and resolves macros/advanced/AUTO into
  concrete module settings. It is the only place that calls module setters.
- `PluginProcessor` owns the `AudioProcessorValueTreeState` (26 stable IDs),
  versioned state IO and the bus/latency contract.
- `PluginEditor` only ever reads the atomic `UiSnapshot`; it never touches DSP
  state directly.

## Threading contract

- `VoiceGuardEngine::prepare`, `reset`, `setParameters` and `process` are
  **audio-thread only**. `setParameters` is called once per block by the
  processor, after reading the APVTS.
- `VoiceGuardEngine::snapshot()` returns plain atomics and is safe to read from
  any thread (the editor's timer).
- `applyPreset` writes APVTS parameters and must run on the message thread.

## Real-time rules

On the audio thread the plugin never allocates, locks, logs, or does file/network
IO. Everything is preallocated in `prepareToPlay`. Buffers are sanitised
(NaN/Inf), coefficients are clamped, denormals are flushed, and the output is
bounded by a final safety stage.

## State

`AudioProcessorValueTreeState` with stable parameter IDs. State is serialised as
versioned XML (root `VoiceGuardState`), rejects corrupt, wrong-root and
newer-version blobs, migrates older schemas forward, and caps the incoming blob
size. Unknown state falls back to the current safe state.

## Build

CMake 3.22+, JUCE 8 pinned to an immutable tag (`cmake/VoiceGuardJuce.cmake`),
with `VOICEGUARD_JUCE_PATH` for offline builds. CI builds Windows and macOS.
