# Photographic rack GUI revision

The 1043 × 366 logical panel now uses the approved photographic artwork rather
than the previous vector approximation. The live 32 px preset toolbar remains
above it; the initial editor is 1565 × 597 and retains proportional resizing.

CMake embeds the approved source image and a clean static plate in PlateArtwork.
The clean plate contains lettering, grain, wear, scale markings and recessed
meter housings. Knob faces use the approved photographed texture; interactive
sliders, time pointers, toggle levers, lamp and VU needle render above the plate.
All existing audio parameter attachments remain. A screenshot alone is never
used as a replacement for working controls. Assets are decoded/cached on the UI
thread and require no runtime disk paths or audio-thread filesystem work.

The panel controls retain their parameter IDs and ranges. Preset navigation,
A/B, clipboard and floating browser keep their existing behavior. The Windows
workflow uploads the actual compiled editor snapshot independently and includes
it in the downloadable package with the exact build commit.

Preset toolbar opens a separate 390 x 700 desktop window (resizable 350 x 500 to 650 x 1000), with native drag title bar and close button. No modal loop: panel controls remain usable. Browser uses lightweight vector controls and visible-row list painting; it does not create 100 graphic assets. Search, category, single tag and favourite filters combine. User category is always available. Data is saved outside the audio callback, with a per-library interprocess lock, reload-before-mutate and atomic replacement. MAP is an automation-ID reference, not MIDI Learn.
