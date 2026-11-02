# Preset audibility: objective before/after

Baseline: `e3fe3b3c89178f46bd15b03ac7810a678a9be649`.
Measurements are offline C++ engine renders at 48 kHz, not listening or DAW tests.
The same measurement program was run before and after the DSP/bank edits.

## Method

`Tests/MeasureAudibility.cpp` loads all 20 values of each embedded factory preset.
Each of the 100 presets is rendered twice: L=R and L=−R. A seeded MT19937
produces identical white noise (peak 0.1) in both runs. After one second of
silence to settle parameter smoothing, render two seconds of noise and measure
the last 1.5 seconds. RMS wet gain is relative to one input channel, averaged
across both output channels. Wet Solo is forced on to isolate the return;
`insert_wet_gain_db` scales this by the preset's saved mix (unity for AUX).
It measures the wet contribution, not the RMS difference of the full dry/wet output.
Peak is the isolated wet output peak; correlation is normalized L/R dot product.

| Metric across 100 presets | Before | After |
| --- | --- | --- |
| Mono wet RMS gain | −22.407 to −12.872 dB | −12.865 to −3.330 dB |
| Mono wet contribution at saved mix | −50.509 to −12.872 dB | −32.447 to −3.330 dB |
| Opposite-polarity wet RMS gain | silence | −12.862 to −3.328 dB |
| Maximum wet peak, both polarities | 0.062 | 0.212 |

## Changes and rationale

The engine formerly averaged L/R before the tanks, cancelling stereo-only
(opposite-polarity) input. Separate mid/side diffusion and orthogonal tank
injection now retain that input energy. Mono injection coefficients are unchanged.
A fixed 3× wet-return trim (+9.54 dB) makes the return more prominent without
changing feedback gains, dry gain or bypass gain. It applies to recalled sessions
as well as factory presets, so existing sessions can sound louder/wetter.

Seven insert mixes below 8% are raised to 8%: Spoken Close, Heavy Guitar Depth,
Dialogue Depth, Mix Bus Subtle, Drum Bus Glue, Vintage Cohesion and Master Dark
Space. This is an engineering audition starting point, not a listening-approved
sound-design decision. All other parameter values, preset names/order, 20 IDs,
parameter ranges, XML schema and host program count are preserved. Generator,
embedded bank, XML, JSON and CSV are synchronized.

`rt-before.json` and `rt-after.json` contain the existing nine 500 Hz octave-band
Schroeder T30 measurements (A/B/C × 0.5/2/5.5 s). All nine pass ±8% after the
change. Single-left-channel impulse results change because side energy now
reaches the tank; decay results are not assumed identical to the baseline.

## Reproduce

From the repository root, with a C++17 compiler:

```sh
g++ -std=c++17 -O2 -I Source Tests/MeasureAudibility.cpp -o /tmp/measure-audibility
/tmp/measure-audibility > docs/validation/audibility/after.csv
python3 Tests/compare_audibility.py
g++ -std=c++17 -O2 -I Source Tests/TestPlateEngine.cpp -o /tmp/PlateTests
/tmp/PlateTests
python3 Tests/measure.py
```

The last command needs NumPy/SciPy and refreshes `docs/measurements.json`.
`PlateAudibility` is also available when configuring CMake with BUILD_TESTING.
To reproduce `before.csv`, compile the same measurement source against baseline
DSP/bank headers and redirect its stdout; do not reset a checkout containing work.
The comparison checks count, finiteness, trim, side/mono energy and stimulus
headroom. The DSP suite passes 628,968 assertions, including stereo-only tail
energy/finite output for all three plates, multi-rate stability, bypass and Wet Solo.

## Not tested

Reaper, Studio One 8, real listening, Windows VST3 compilation, native plugin
integration and pluginval after this change. No release was published.
Noise peak headroom is not a guarantee against clipping for all input levels or
EQ settings. Constant return trim increases output level and needs auditioning
with music. CPU overhead of the extra diffusion path has not been benchmarked.
