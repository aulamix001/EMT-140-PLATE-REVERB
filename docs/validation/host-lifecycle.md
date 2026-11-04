# Host lifecycle crash regression

Baseline: `552d85f`. With a prepared processor, calling processBlock with an
AudioBuffer<float>(0, 64) reproduced a segmentation fault (Linux exit 139):

```
CHECK: zero audio channels, 64 samples
Host lifecycle baseline exit: 139
```

processBlock accessed channel zero without checking the number of channels.
It could also use uninitialised delay storage before prepareToPlay. The fix
returns before accessing empty audio buffers, outputs silence outside the prepared
lifecycle, clears readiness on releaseResources, and rejects invalid sample rates.
Valid prepared mono/stereo processing, parameters, presets and artwork are unchanged.

Tests/HostLifecycle.h exercises zero channels, zero samples, processing before
preparation and after release, zero/negative/non-finite sample rates, and repeated
mono/stereo activation at 44.1/48/96/192 kHz. The same callback that crashed now
passes. Native DSP, 100-preset/state/audio/UI integration, clipboard and A/B tests
also passed locally after this change.

Run the existing PlatePreview executable with --host-lifecycle on a desktop
(or under Xvfb on Linux). Windows CI now runs this regression with a 60-second
process timeout before the existing UI integration and pluginval checks.

Studio One 8 is unavailable in this environment. No Studio One crash log or
call stack was supplied. This reproduces and fixes a concrete processor crash;
it does not prove that this callback caused the reported Studio One freeze.
Direct insertion and session loading in Studio One 8 still require confirmation.
