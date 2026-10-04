# QUALITY.md v1 (draft — not yet founder-approved)

Numeric quality targets for the Bespoke Developer PlugOut plugin factory. Every
number below has a stated measurement method so a target can be argued with on
its reasoning, not just its digit. Targets marked **(provisional)** are best
estimates pending measurement on real hardware we do not yet have in CI
(physical macOS/Windows machines, real audio interfaces) and will be replaced
once CI-D1 gives us real runner data.

## 1. Aliasing (gate 4)

- **Target:** aliased energy at least **-60 dBFS** below the fundamental,
  measured with a logarithmic sine sweep 20 Hz–20 kHz at 44.1 kHz sample rate,
  plugin driven to maximum saturation/drive, 4x oversampling enabled.
- **Reason:** -60 dB is below typical perceptual threshold for aliasing
  artefacts in a mixed track (Pirkle §22.5 cites -60 dB as a practical
  oversampling target); it is strict enough to force real oversampling design,
  not loose enough to pass an un-oversampled naive waveshaper.
- **Method:** FFT the output, find the highest aliased partial below Nyquist,
  compare its level to the fundamental's.

## 2. CPU per instance (gate 5)

- **Target:** **< 3% of one CPU core** (single logical core, not total system)
  per plugin instance, at 48 kHz / 128-sample buffer, on the fixed GitHub
  Actions `ubuntu-latest` runner used for CI-C3. **(provisional — "3%" is a
  CI-runner number, not a promise about a producer's actual machine; needs a
  real-hardware baseline before launch.)**
- **Reason:** producers typically run 20–50+ plugin instances in a session; at
  3% per instance that is 60–150% of one core for a full session, which is
  usable alongside everything else running. Measured on a fixed runner type so
  the number is comparable run to run, not a fair absolute promise to a
  customer (CI-C3 risk, noted in the plan).
- **Method:** process 10 seconds of audio with the plugin under realistic
  automation (parameter sweeps, not held still), measure wall-clock CPU time
  used, divide by wall-clock audio time.

## 3. Round-trip latency (gate 4/7)

- **Target:** reported plugin latency (`AudioProcessor::setLatencySamples`)
  **within ±1 sample** of actual measured output delay, for every oversampling
  ratio and buffer size the plugin supports.
- **Reason:** a wrong latency report breaks host automatic delay compensation,
  which breaks every other track in the producer's session, not just ours.
  This is pass/fail, not a target to loosen.
- **Method:** impulse test — send a unit impulse, measure the sample offset of
  the output impulse, compare to the reported latency.

## 4. UI frame rate

- **Target:** **≥ 55 fps sustained**, **≥ 45 fps minimum** during continuous
  parameter automation, measured in the WebView at the plugin's default
  window size.
- **Reason:** below ~45 fps a knob being dragged visibly stutters; 55 fps
  sustained leaves headroom so a host under load doesn't drop us below the
  45 fps floor.
- **Method:** instrument `requestAnimationFrame` deltas in the WebView over a
  10-second automated parameter sweep; report mean and 1st-percentile fps.

## 5. UI load time

- **Target:** **< 500 ms** from plugin instantiation to first interactive
  frame (WebView visible and responsive to input).
- **Reason:** a producer loading 10+ instances while building a session
  notices anything above half a second per instance; 500 ms is the point
  where loading stops feeling instant.
- **Method:** timestamp from `createEditor()` call to the WebView's first
  `pointerdown`-ready state, averaged over 10 cold loads.

## 6. Null-test residual (gate 4/7)

- **Target:** with the effect bypassed, or all processing at unity/zero-drive,
  output nulled against input (after latency compensation) to **below
  -90 dBFS** residual.
- **Reason:** -90 dB is below the noise floor of a 16-bit file and far below
  any audible difference; it catches a broken bypass or a DC/phase bug without
  demanding literal bit-exactness, which floating-point DSP chains rarely give
  for free.
- **Method:** null test — invert the bypassed/zero-drive output and sum
  against the input, measure peak and RMS residual.

## 7. Memory ceiling per instance

- **Target:** **< 50 MB** resident memory per instance after
  `prepareToPlay`, at the largest supported buffer size and highest supported
  sample rate (192 kHz). **(provisional — no real product DSP exists yet to
  measure against; revisit once the saturation plugin's oversampling and
  look-ahead buffers are known.)**
- **Reason:** 50 MB per instance keeps 40 instances under 2 GB, a reasonable
  ceiling for a producer's session before memory becomes the limiting factor
  ahead of CPU.
- **Method:** measure process RSS delta between plugin construction and 10
  seconds after `prepareToPlay`, on the fixed CI runner.

## 8. Visual-diff tolerance (gate 6)

- **Target:** **≤ 0.1%** of pixels differing beyond a **5/255** per-channel
  threshold, comparing a headless Chromium screenshot against the committed
  baseline, pinned browser version and container image.
- **Reason:** tight enough to catch a real layout or style regression, loose
  enough to absorb anti-aliasing and sub-pixel font-hinting differences that
  are not real regressions. Numbers taken from common visual-regression-testing
  practice (e.g. Percy/Chromatic defaults sit in a similar range); revisit if
  CI-C4 proves it flaky.
- **Method:** pixelmatch (or equivalent) diff, Linux-only runner for
  determinism, baseline changes only via a reviewed commit.

## 9. Supported sample rates and buffer sizes (gate 4)

- **Target:** correct, click-free operation at sample rates **44.1, 48, 88.2,
  96, 176.4, 192 kHz**, and buffer sizes **1 to 4096 samples inclusive**,
  including non-power-of-two ("prime") sizes.
- **Reason:** this is the full range JUCE hosts commonly request; a filter or
  delay with a hardcoded sample-rate assumption fails silently at the edges
  of this range, which is exactly where it's hardest to debug in the field.
- **Method:** parametrised test sweep in CI-C2 across the full matrix; failure
  at any single combination fails the gate.

## How to update this document

Any change to a number here needs founder approval (same gate as v1). Propose
the change with the same reasoning format as above, as a pull request to
`QUALITY.md`, linked to a Paperclip task. CI gates 4, 5 and 6 are written
against the numbers in this file; a change here is not complete until the
gates that encode it are updated too.
