# VoiceGuard v0.0.1 Alpha Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build MyEvent Labs VoiceGuard v0.0.1 Alpha, a cross-platform JUCE 8 real-time speech-enhancement audio plugin (VST3/AU/Standalone) with a deterministic, ultra-low-latency local DSP chain.

**Architecture:** A GUI-free `voiceguard_core` static library holds all DSP (`SpeechAnalyzer` + nine processing modules + a `VoiceGuardEngine` chain orchestrator) and state IO. The JUCE plugin target wraps the engine, exposing an `AudioProcessorValueTreeState` with stable IDs, an atomic `UiSnapshot` for the editor, and click-free bypass. AUTO mode is a slow, bounded analysis state machine inside the engine; never a rewrite of the sound.

**Tech Stack:** C++20, JUCE 8.0.17-security (pinned, FetchContent with local-path escape hatch), CMake 3.22+, GitHub Actions (Windows + macOS).

**Spec:** VoiceGuard v0.0.1 Alpha build brief (private project brief).

## Global Constraints

- C++20; JUCE 8 pinned to `8.0.17-security` (commit `e548e97a54a083acbfc330d584419fd074409045`); CMake >= 3.22.
- Product `MyEvent Labs VoiceGuard`; bundle id `com.myeventlabs.voiceguard`; manufacturer code `MyEv`; plugin code `Vgrd`.
- Formats: VST3, AU, Standalone. `IS_SYNTH FALSE`. Mic permission enabled (standalone input).
- No intentional lookahead by default; target 0 added algorithmic latency; report latency honestly to host (`setLatencySamples`).
- Audio thread: no allocation, locks, file/network IO, logging, UI access, JSON/DB, or unbounded processing. Preallocate in `prepareToPlay`; guard denormals; bounded algorithms; lock-free/atomic message-thread comms.
- 100% local processing. No telemetry, no cloud, no transcription, no mic audio leaving the machine.
- Fail toward clean passthrough: sanitize NaN/Inf, clamp coefficients/params, click-free bypass, never introduce unexpected positive gain.
- Default workflow: `insert -> AUTO -> mix`. Defaults must not destructively colour clean speech.
- Presets are constrained starting points, not fixed EQ gimmicks.
- Do not commit secrets or build binaries to source history.
- Release is clearly labelled ALPHA; do not claim production readiness.

## Review Focus

- **Silence / room-tone only:** the Level Rider must not ride up background noise; ride gain must stay near 0 dB and speech-activity gating must hold.
- **Very loud / hot input:** the output safety stage must keep output bounded and recover quickly; no runaway; no NaN/Inf.
- **DC / near-DC and non-finite input:** HPF and every module must reject DC and sanitize NaN/Inf without state corruption.
- **Extreme/invalid parameters and corrupt state blobs:** clamps and state migration must fall back safely, never crash or produce gain.
- **Rapid bypass/mode/preset automation and editor open/close:** transitions click-free, no races, bounded memory across many reinitialisations.

---

### Task 1: Project scaffold, JUCE pin, CMake, core library skeleton

**Files:**
- Create: `CMakeLists.txt`, `cmake/VoiceGuardJuce.cmake`, `.gitignore`, `.gitattributes`, `LICENSE`, `README.md`, `THIRD-PARTY-NOTICES.md`, `CHANGELOG.md`
- Create: `src/dsp/DspUtils.h`, `src/dsp/DspUtils.cpp`
- Create: `tests/CMakeLists.txt`, `tests/TestMain.cpp`, `tests/TestRegistry.h`
- Create: `tools/CMakeLists.txt`

**Interfaces:**
- Produces: `voiceguard_core` static lib target; `voiceguard::dbToGain(float)`/`gainToDb(float)`; `voiceguard::sanitize(float&)` -> returns true if finite; `voiceguard::kMinDb = -100.f`; test macros `VG_CHECK`, `VG_CHECK_CLOSE`, `VG_TEST(name)`.

- [ ] **Step 1:** Write `cmake/VoiceGuardJuce.cmake` pinning JUCE, with `VOICEGUARD_JUCE_PATH` escape hatch (mirror De-Feedback module).
- [ ] **Step 2:** Write root `CMakeLists.txt` declaring `voiceguard_core` (initial source `DspUtils.cpp`) and `juce_add_plugin(VoiceGuard ...)`, tests/tools subdirs.
- [ ] **Step 3:** Implement `DspUtils.{h,cpp}` with db<->gain, sanitize, denormal scope guard.
- [ ] **Step 4:** Write `tests/TestMain.cpp` + `TestRegistry.h` (tiny registry, exit non-zero on failure).
- [ ] **Step 5:** Add first test asserting `dbToGain(0)==1`, `gainToDb(1)==0`, `sanitize` clears NaN/Inf.
- [ ] **Step 6:** Configure + build tests; run; expect pass. Commit.

### Task 2: Parameter layout and state IO

**Files:**
- Create: `src/dsp/Parameters.h`, `src/state/StateIO.h`, `src/state/StateIO.cpp`
- Test: `tests/unit/StateTests.cpp`

**Interfaces:**
- Produces: `voiceguard::parameters::*` stable id constants; `voiceguard::createParameterLayout()`; `voiceguard::kStateVersion`; `stateToXml(ValueTree)->String`; `stateFromXml(String, Result&)->ValueTree`; `migrateState(ValueTree&, int)`. A plain `struct Parameters` snapshot with all current values, and `Parameters::fromApvts(const APVTS&)`.

- [ ] **Step 1:** Write failing state round-trip + corrupt-blob + newer-version tests.
- [ ] **Step 2:** Implement `Parameters.h` (ids + `ParameterLayout` + snapshot struct) and `StateIO`.
- [ ] **Step 3:** Run tests; expect pass. Commit.

### Task 3: Shared `SpeechAnalyzer`

**Files:**
- Create: `src/dsp/SpeechAnalyzer.h`, `src/dsp/SpeechAnalyzer.cpp`, `src/dsp/Fft.h` (thin juce::dsp::FFT wrapper)
- Test: `tests/unit/SpeechAnalyzerTests.cpp`

**Interfaces:**
- Produces: `struct SpeechFeatures { float speechActivity, noiseFloorDb, rmsShortDb, rmsLongDb, peakDb, crestFactorDb, spectralCentroidHz, spectralSlopeDbPerOct, sibilanceConfidence, harshnessConfidence, tonality, spectralStability; }`; `class SpeechAnalyzer { void prepare(double,int,int); void reset(); void process(const float* const*, int, int) noexcept; const SpeechFeatures& features() const noexcept; }`.

- [ ] **Step 1:** Write failing tests: silence -> low activity; loud speech-like tone -> high activity; noise floor tracks broadband noise; no recalculation crash on 1-sample block.
- [ ] **Step 2:** Implement analyzer (mono sum, 1024-point FFT, band energies, tonality via spectral flatness, stability via frame-to-frame correlation).
- [ ] **Step 3:** Run tests; pass. Commit.

### Task 4: Input conditioning (trim + HPF) and Biquad primitives

**Files:**
- Create: `src/dsp/Biquad.h`, `src/dsp/Biquad.cpp`, `src/dsp/InputConditioning.h`, `src/dsp/InputConditioning.cpp`
- Test: `tests/unit/BiquadTests.cpp`, `tests/unit/InputConditioningTests.cpp`

**Interfaces:**
- Produces: `class Biquad { void setLowpassHighpass..., void processBlock(float*,int) noexcept; float magnitudeAt(double) const; }` RBJ coefficients with TDF2; `class InputConditioning { void prepare(double,int,int); void reset(); void setTrimDb(float); void setHpfHz(float); void process(AudioBuffer<float>&) noexcept; }`.

- [ ] **Step 1:** Failing tests: HPF at 80 Hz attenuates 20 Hz, passes 1 kHz; trim +6 dB doubles amplitude; DC rejected.
- [ ] **Step 2:** Implement Biquad + InputConditioning with smoothed trim.
- [ ] **Step 3:** Pass. Commit.

### Task 5: Conservative noise reduction

**Files:**
- Create: `src/dsp/NoiseReduction.h`, `src/dsp/NoiseReduction.cpp`
- Test: `tests/dsp/NoiseReductionTests.cpp`

**Interfaces:**
- Produces: `class NoiseReduction { void prepare(double,int,int); void reset(); void setAmount(float); void setAdaptation(float); void process(AudioBuffer<float>&, const SpeechFeatures&) noexcept; float currentReductionDb() const noexcept; }`. STFT spectral subtraction with over-subtraction control, noise-floor tracking with slow/fast alpha, speech-protect floor by band, Wiener-style gain floor.

- [ ] **Step 1:** Failing tests: steady pink noise reduced >=3 dB and <=12 dB at amount 0.5; clean speech tone not reduced >1.5 dB; no metallic artifact proxy (gain floor >= -12 dB).
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 6: Speech-aware dynamic EQ

**Files:**
- Create: `src/dsp/DynamicEq.h`, `src/dsp/DynamicEq.cpp`
- Test: `tests/dsp/DynamicEqTests.cpp`

**Interfaces:**
- Produces: `class DynamicEq { void prepare(double,int,int); void reset(); void setStrength(float); void setRegion(int idx, bool enabled) with regions enum Rumple/Mud,Boxiness,Nasality,Intelligibility,Harshness,Sibilance; void process(AudioBuffer<float>&, const SpeechFeatures&) noexcept; }`.

- [ ] **Step 1:** Failing tests: excess energy in a region yields bounded cut (<=6 dB) only while present; neutral speech barely moves; stereo image preserved (L/R gain matched).
- [ ] **Step 2:** Implement per-region state-variable filters with program-dependent detectors. Pass. Commit.

### Task 7: Harshness control and de-esser

**Files:**
- Create: `src/dsp/HarshnessControl.h/.cpp`, `src/dsp/DeEsser.h/.cpp`
- Test: `tests/dsp/DeEsserTests.cpp`

**Interfaces:**
- Produces: `class HarshnessControl { prepare/reset/setAmount/process(buffer, features) }`; `class DeEsser { prepare/reset/setAmount/process(buffer, features); }` split-band with smoothed sidechain gain, lisping guard (release floor).

- [ ] **Step 1:** Failing tests: sibilant burst reduced when amount>0 and neutral otherwise; broadband speech not dulled >1 dB; no lisping proxy (HF gain floor >= -8 dB).
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 8: Level Rider

**Files:**
- Create: `src/dsp/LevelRider.h`, `src/dsp/LevelRider.cpp`
- Test: `tests/dsp/LevelRiderTests.cpp`

**Interfaces:**
- Produces: `class LevelRider { prepare/reset/setTargetDb/setRangeDb/setSpeed/process(buffer, features) noexcept; float rideGainDb() const; }`. Bounded +/-6 dB default, speech-activity gating, separate fast peak path.

- [ ] **Step 1:** Failing tests (Review Focus): silence/room tone -> |rideGainDb| < 0.5 dB; quiet speech rises but never exceeds range+cushion; loud speech falls within range; no allocation; ride rate bounded.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 9: Gentle dynamics and output safety

**Files:**
- Create: `src/dsp/Dynamics.h/.cpp`, `src/dsp/OutputSafety.h/.cpp`
- Test: `tests/dsp/DynamicsTests.cpp`, `tests/dsp/OutputSafetyTests.cpp`

**Interfaces:**
- Produces: `class Dynamics { prepare/reset/setParameters(threshold,ratio,attackMs,releaseMs,knee)/process(buffer, features) }`; `class OutputSafety { prepare/reset/setCeilingDb/process(buffer) noexcept; float grDb() const; }` fast peak control, no lookahead by default.

- [ ] **Step 1:** Failing tests: dynamics GR bounded and recovers; output never exceeds ceiling+epsilon under 0 dBFS hot input; NaN input sanitized; no positive gain.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 10: De-Feedback interface seam + `VoiceGuardEngine` chain

**Files:**
- Create: `src/dsp/DeFeedbackInterface.h`, `src/dsp/VoiceGuardEngine.h/.cpp`
- Test: `tests/dsp/EngineTests.cpp`

**Interfaces:**
- Produces: `class IFeedbackEngine { virtual prepare/reset/process/setEnabled }` with a null implementation; `class VoiceGuardEngine { prepare/reset/setParameters(Parameters)/process(AudioBuffer<float>&, MidiBuffer&) noexcept; UiSnapshot& snapshot(); }` chaining conditioning->NR->dynEQ->deEsser->rider->dynamics->[de-feedback]->safety. Per-stage bypass flags.

- [ ] **Step 1:** Failing tests: full-chain impulse/finite-burst stability at 44.1/48/88.2/96 kHz and blocks 32..1024; no NaN/Inf; per-stage bypass; mono and stereo.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 11: AUTO mode state machine

**Files:**
- Create: `src/dsp/AutoController.h/.cpp`
- Test: `tests/dsp/AutoTests.cpp`

**Interfaces:**
- Produces: `class AutoController { prepare/reset; void start(); void cancel(); void process(const SpeechFeatures&) noexcept; bool isListening() const; bool isReady() const; AutoResult result() const; }` 5-15 s representative-speech window, conservative estimates, slow safe adaptation only.

- [ ] **Step 1:** Failing tests: silence doesn't complete; speech completes within window; outputs clamped conservative; after ready, changes bounded per second.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 12: Plugin processor (APVTS, lifecycle, bypass, snapshot)

**Files:**
- Create: `src/PluginProcessor.h/.cpp`
- Test: `tests/integration/ProcessorTests.cpp`

**Interfaces:**
- Consumes: `VoiceGuardEngine`, `createParameterLayout`, `StateIO`, `AutoController`.
- Produces: `class VoiceGuardProcessor : juce::AudioProcessor`; `apvts`; `UiSnapshot`; `getStateInformation/setStateInformation`; honest `getLatencySamples()`.

- [ ] **Step 1:** Failing integration tests: prepare at odd sample rates/layouts; process 1000 blocks without NaN; automatable bypass click-free; state round-trips; corrupt state falls back; layout rejection for unsupported.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 13: UI — LookAndFeel, macros, meters, AUTO, readouts, advanced panel

**Files:**
- Create: `src/ui/LookAndFeel.h/.cpp`, `src/ui/Meter.h/.cpp`, `src/ui/MacroSection.h/.cpp`, `src/ui/AdvancedPanel.h/.cpp`, `src/PluginEditor.h/.cpp`
- Test: manual + `tests/integration/EditorTests.cpp` (editor create/destroy 200x)

**Interfaces:**
- Produces: `VoiceGuardLookAndFeel` (MyEvent Labs palette), four macro rotary controls, AUTO toggle, bypass, level readouts, advanced panel toggling.

- [ ] **Step 1:** Failing test: create/show/destroy editor 200 times with timers, no leak assertion trip.
- [ ] **Step 2:** Implement UI. Pass. Commit.

### Task 14: Factory presets and migration

**Files:**
- Create: `src/state/Presets.h/.cpp`
- Test: `tests/unit/PresetTests.cpp`

**Interfaces:**
- Produces: `const std::vector<Preset>& factoryPresets()` for Auto, Lectern Cleanup, Wireless Lav, Handheld Speech, Panel Discussion, Broadcast Voice, Streaming, Noisy Room, Bright Lavalier, Quiet Presenter; `applyPreset(AudioProcessorValueTreeState&, const Preset&)`.

- [ ] **Step 1:** Failing tests: 10 presets exist; each sets valid in-range values; applying then serialising round-trips.
- [ ] **Step 2:** Implement. Pass. Commit.

### Task 15: Offline fixtures + DSP regression suite

**Files:**
- Create: `tests/fixtures/Signals.h/.cpp`, `tests/dsp/RegressionTests.cpp`
- Test: above.

**Interfaces:**
- Produces: synthetic generators for quiet/loud speech-like, sibilance, plosive, tonal, HVAC, pink/white, hum, silence, transients, music, narrowband feedback tone.

- [ ] **Step 1:** Write regression assertions: level stability, noise reduction range, spectral damage bound, peak safety, filter stability, NaN/Inf, mono/stereo, bounded memory.
- [ ] **Step 2:** Pass. Commit.

### Task 16: Tools — signal generator and benchmark

**Files:**
- Create: `tools/signal-generator/`, `tools/benchmark/`

- [ ] **Step 1:** Implement offline generator writing WAVs; benchmark measuring CPU for N instances at 48k/128.
- [ ] **Step 2:** Run benchmark, record measured numbers in `docs/BENCHMARKS.md`. Commit.

### Task 17: Docs, CI, packaging

**Files:**
- Create: `docs/ARCHITECTURE.md`, `docs/DSP.md`, `docs/ALPHA-LIMITATIONS.md`, `docs/SECURITY.md`, `docs/VALIDATION.md`, `.github/workflows/build.yml`, `scripts/package.ps1`/`.sh`, `release/INSTALL.txt`
- Modify: `README.md`, `CHANGELOG.md`

- [ ] **Step 1:** Write docs; CI builds Windows + macOS jobs; packaging scripts with checksums.
- [ ] **Step 2:** Build Windows VST3/Standalone locally; run pluginval if available; commit.

### Task 18: Release gate

- [ ] **Step 1:** Full local build + tests + soak smoke; clang-format/tidy; secret scan.
- [ ] **Step 2:** Create GitHub repo, push, tag `v0.0.1-alpha`, Pre-release with ZIPs and checksums.
- [ ] **Step 3:** Final report.

## Self-Review Notes

- Task 15 owns the Review Focus fixtures; Tasks 8/9/12/5 own the specific Review Focus tests.
- De-Feedback integration is a seam only (Task 10), off by default — matches spec.
- AAX/LV2 left for later; architecture keeps format-agnostic core.
