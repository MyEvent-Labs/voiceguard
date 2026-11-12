# DSP

## Signal chain

```
Input conditioning (HPF + trim)
  -> conservative multiband noise reduction
  -> speech-aware dynamic EQ
  -> harshness control
  -> de-esser
  -> slow level rider
  -> gentle dynamics
  -> [optional De-Feedback, off by default]
  -> output safety peak control
  -> output trim
```

Every stage is internally bypassable. There is no intentional lookahead; added
algorithmic latency is 0 samples (lookahead-free) and is reported to the host.

## Shared analysis

`SpeechAnalyzer` runs once per block on the dry input and produces the features
every module consumes: speech activity, noise floor, short/long RMS, peak and
crest factor, spectral centroid/slope, sibilance and harshness confidence,
tonality and spectral stability. Modules never recompute these.

## Macros

The four macros are the primary control. A convex curve
(`gain = 2 * macro^1.585`, clamped to 0..2) keeps the default (0.5) gentle and
transparent while letting the top of travel reach full effect.

- **CLEAN** — noise reduction depth, adaptation, low-mid cleanup.
- **CLARITY** — dynamic EQ, de-esser and harshness strength.
- **LEVEL** — level target, ride range and dynamics depth.
- **PROTECT** — output safety ceiling, feedback gate, and a more conservative ride.

## Modules (brief)

- `InputConditioning` — smoothed +/-18 dB trim, HPF (Off/60/80/100/120/160 Hz).
- `NoiseReduction` — 10-band floor tracker driving serial dynamic cuts; strongly
  protects speech, so pauses keep the reduction and steady speech is not dulled.
- `DynamicEq` — six program-dependent search regions, cuts only excess energy.
- `HarshnessControl` / `DeEsser` — bounded dynamic cuts with confidence dead bands.
- `LevelRider` — slow, bounded ride on the long-term level with a dead band and a
  fast peak path; returns to unity when speech stops so room tone is never boosted.
- `Dynamics` — gentle soft-knee compression with open defaults.
- `OutputSafety` — lookahead-free peak control with a final hard clamp; the output
  stays bounded regardless of trim.

## Quality targets

`docs/VALIDATION.md` describes the objective scorecard that guards these
(pause-noise reduction, level spread, clean-speech transparency, sibilance,
musical-noise, bounded output, macro audibility).
