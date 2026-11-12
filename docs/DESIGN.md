# DESIGN.md — VoiceGuard UI system

Mode: Operate. Direction: **instrument panel** — restrained, precise,
meter-forward. Dark ground forced by the use scene (FOH / control room, low
light), not by category.

## Colour tokens
Neutral ground tinted cool; one accent for live data; semantic colours for
state. No gradients, no glass, no neon.

| Token | Hex | Use |
|---|---|---|
| `bg` | `#0F1216` | window ground |
| `surface` | `#181C22` | panels, cards |
| `surface-sunken` | `#0B0D11` | meter tracks, wells |
| `outline` | `#30373F` | borders (elevation is border, not shadow) |
| `text` | `#EEF1F5` | primary text, values |
| `text-dim` | `#9AA4B2` | secondary labels |
| `text-faint` | `#7D8898` | tertiary/units |
| `accent` | `#5AA9FF` | active state, EQ/primary gain reduction |
| `safe` | `#57D9A3` | safe level, protection OK |
| `warning` | `#E8B04A` | near-ceiling, harshness |
| `danger` | `#E5645F` | clipping/over |

Contrast floor: primary text ≥ 4.5:1 on `surface`; secondary ≥ 4.5:1;
tertiary ≥ 4.5:1 for anything that is read as text. Values are never grey on a
coloured ground — tint from the hue.

## Type
System sans (JUCE default). Small, deliberate scale; tabular numerals for
readouts. No monospace costume except numeric meters, which may use tabular
figures.

| Role | Size | Weight | Tracking |
|---|---|---|---|
| Product title | 17 | bold | -0.01em |
| Section header (uppercase) | 10.5 | semibold | +0.08em |
| Label | 11 | regular | 0 |
| Value / readout | 12 | semibold | +0.01em (tabular) |
| Macro label | 11 | semibold | +0.06em |

## Space & shape
4/8 rhythm. Panel padding 14–16; gap between groups 12; label→control 6;
control→next group 16. Radii: control 6, panel 10, pill 999. Elevation is a
1px `outline` border only — never border *and* wide shadow.

## Components
- **Macro knob**: rotary, accent value arc, grip; live value under the label
  (`DEFAULT 50%`). Rotates with a pointer, not a stack of ticks.
- **Meter (vertical)**: sunken track, accent/safe fill, peak-hold line, dB
  ticks at −12/−24/−36/−48.
- **Gain-reduction bar**: label left, sunken track, fill, value right;
  bipolar for LEVEL (grows from centre).
- **Button**: filled when on, tonal when off; hover lifts brightness, press
  scales 0.985 (no layout shift); visible focus ring.
- **Preset select / combo**: sunken, outline, chevron.
- **States**: hover, pressed, focused, disabled, plus real values everywhere.

## Motion
Restraint. Press feedback 120ms ease-out; value/level transitions are the
data's own smoothing, never decorative. No entrance animations on an Operate
surface; nothing moves that the engineer is reading.

## Bans
No faux metal, rack screws, neon gradients, glare, emoji-as-icons, gradient
text, decorative glows, knob walls, or identical entrances. One accent only.
