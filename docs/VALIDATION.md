# Validation record — development version 0.1.0

Executed on Debian 13 x86-64 with GCC 14.2, JUCE 8.0.6 and Release builds. Windows/Studio One results are not implied.

- Native CMake build: VST3, Standalone, DSP test executable and native editor snapshot runner.
- DSP suite: silence, finite/bounded impulse tails at 44.1/48/96/192 kHz, block-size loops 32/64/128/256/512/1024/2048, Wet Solo invariance to stored mix, mono width after smoothing, bypass unity, zero-gain shelf unity, exact delay sample indexing.
- Native integration runner: independent plate RT, versioned XML state recall, all 12 factory presets, exact nonlinear midpoint mappings, real stereo audio callback, actual editor screenshot. It exits nonzero on assertion failure.
- pluginval 1.0.4 strictness 10: Linux VST3, sample rates 44.1/48/96/192 kHz, block sizes 32/64/128/256/512/1024, seed 140, GUI tests included.
- Offline octave-band Schroeder integration, 354–707 Hz, T30 fit from −5 to −35 dB extrapolated to RT60, 48 kHz, nine-second impulses: all nine cases within ±8%. Exact data and stereo correlations in `measurements.json`; rerun using `python Tests/measure.py` with NumPy/SciPy.

| Plate | Target 0.5 s | Target 2 s | Target 5.5 s |
|---|---:|---:|---:|
| A | 0.5280 | 1.9908 | 5.2808 |
| B | 0.5105 | 1.9727 | 5.3773 |
| C | 0.5238 | 2.0492 | 5.4457 |

Plate B uses a short-time decay correction derived from this measurement (45 ms × exp(−3 × (RT−0.5))). These results cover the stated sampled targets at 48 kHz; they do not establish the full frequency/rate/parameter acceptance envelope or equivalence with hardware.

Remaining release gates: Windows x64 CI execution, Studio One load/session recall, authoritative manual analysis, licensed reference audio comparisons, full-band calibration, THD/aliasing and CPU budget measurements, allocation instrumentation, corrupted XML hardening, macOS AU validation. Configured GitHub Actions are source configuration, not an executed remote build. No VST2 SDK or compatibility is claimed.

GUI revision: recompiled and relinked editor, Standalone and VST3; native parameter/state/preset/audio integration passed; pluginval strictness 10 repeated successfully with GUI tests at the same rates and block sizes. Current screenshot uses the 1043:366 panel with context-menu preset controls.

## 100-preset bank and floating browser
Native integration passed for all 100 embedded presets and all 100 generated XML files: each of 20 parameter values matches the real processor, ranges are valid, session recall works and representative audio blocks remain finite. Manifest/XML agreement and 100 unique parameter states also checked.

Browser is a real desktop DocumentWindow (not an embedded overlay), draggable and nonmodal. Automated checks cover toolbar opening, closing/reopening, Snare search (3 matches), clearing search (100 entries), A/B snapshots and clipboard mapping. User-library tests cover save, empty-name rejection/recovery, rename/delete, factory protections, favourites across reloads and preservation of two instances' independently saved presets. Invalid NaN/out-of-range states are rejected.

Pluginval 1.0.4 strictness 10 passed after a state-restoration regression was fixed: JUCE bool raw values can be snapped while host-facing values remain fractional. restoreState now restores every host-facing parameter explicitly. A regression test reproduces the original Wet Solo/Power fractions and requires exact restored values. Same sample rates and block sizes as above; see validation logs in docs/validation/.

Windows VST3 build, Windows GUI/clipboard behaviour and Studio One remain unexecuted. CI now includes the 100-preset native integration runner, Windows builds and pluginval, but workflow source is not proof of a remote run. Bank XML is platform-neutral and uses the same parameter schema on Windows.
