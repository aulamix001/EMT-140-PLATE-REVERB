# Remaining validation and approximations

- Validate loading, program selection, restoration, A/B and removal in Studio One 8 and Reaper with the actual Windows bundle. Neither DAW nor real listening has been tested here.
- Obtain legitimate hardware/UAD transfer-function references and recordings. The engine is an independently authored approximation; no proprietary equivalence is claimed.
- Run listening comparisons of the 100 factory voicings. The objective gate now requires every pair to differ in RT500 by 20% or brightness/bass by 2.5 dB; that does not establish perceptual uniqueness for every source.
- Extend RT/band calibration and CPU profiling across real systems and sample rates. Audio callback allocation instrumentation passes at four sample rates; no measured real-machine CPU target is claimed.
- Characterize THD/aliasing of the tanh driver before deciding whether oversampling is justified.
- EMT input cut remains an 80 Hz shelf with the stored attenuations. MARTECH uses -12 dB shelves at the inherited six frequencies; no hardware transfer curve was supplied.
- The actual delay-read modulation is implemented and quantitatively tested. Rate/depth OFF, early density, A/B/C contrast, memory safety, host program/state and individual controls are automated gates; see docs/VALIDATION.md.
- Native Windows graphics/lifecycle is covered by CI. Physical Intel/AMD/NVIDIA devices, mixed-monitor DPI and attached VST3 SDK scale sequences remain untested.
- AU is configured conditionally but not validated on macOS. VST2 requires a licensed SDK; CLAP is not configured.
- Floating browser, favourites, atomic user storage, clipboard, A/B and all 100 factory presets are implemented. Undo/redo and on-panel typed numeric entry are future work; the host parameter UI supports numeric entry.
