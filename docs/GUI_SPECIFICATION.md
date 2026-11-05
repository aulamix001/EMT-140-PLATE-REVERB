# EMT reference-panel GUI revision

The 1043 × 366 logical panel now uses the latest supplied EMT panel reference rather
than the previous vector approximation. The live 32 px preset toolbar remains
above it; the initial editor is 1565 × 597 and retains proportional resizing.

CMake embeds a clean static plate and a corrected VU face in PlateArtwork.
The clean plate contains lettering, grain, wear, scale markings and recessed
meter housings. The concave knob bodies stay fixed in the background so the top-left lighting never rotates; only the white indices move. Interactive
sliders, time pointers, toggle levers, lamp and a VU needle following the non-linear dB scale render above the plate.
All existing audio parameter attachments remain. A screenshot alone is never
used as a replacement for working controls. Assets are decoded/cached on the UI
thread and require no runtime disk paths or audio-thread filesystem work.

The panel controls retain their parameter IDs and ranges. Preset navigation,
A/B, clipboard and floating browser keep their existing behavior. The Windows
workflow uploads the actual compiled editor snapshot independently and includes
it in the downloadable package with the exact build commit.

Preset toolbar opens a separate 390 x 700 desktop window (resizable 350 x 500 to 650 x 1000), with native drag title bar and close button. No modal loop: panel controls remain usable. Browser uses lightweight vector controls and visible-row list painting; it does not create 100 graphic assets. Search, category, single tag and favourite filters combine. User category is always available. Data is saved outside the audio callback, with a per-library interprocess lock, reload-before-mutate and atomic replacement. MAP is an automation-ID reference, not MIDI Learn.

The ten physical dial mappings live in Source/GUI/ControlMapping.h. Low/high
frequency scales are logarithmic with 200 Hz/2 kHz at noon. Rate is logarithmic
above the OFF detent, with 0.1 Hz at noon and exactly stationary 0 Hz. Gain is
linear with 0 dB at noon. Predelay/mix retain 50 ms/15% centers. This overrides
only Slider's physical conversion: all host normalization, IDs, steps, ranges
and defaults remain unchanged. Live numeric readouts follow actual values,
including balance instead of a permanently printed C. PanelControlAudit tests
all attachments, hit areas, all filter/plate positions and all 20 audio effects.
