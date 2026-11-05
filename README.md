# PLATE 140
Original C++17/JUCE 8 plate-inspired reverb by Aulamix. The panel displays the historical EMT 140 / Elektromesstechnik Wilhelm Franz legends at the user’s request; the plugin remains an independent development. Native VST3 and Standalone; AU on macOS. Scalable original-reference panel editor with embedded artwork and live controls, three independent plate tanks, 100 original factory presets, XML state, A/B comparison and user preset import/export.

## Download Windows x64
[PLATE 140 v0.2.1-dev.1 — compiled VST3 and Standalone](https://github.com/aulamix001/EMT-140-PLATE-REVERB/releases/download/v0.2.1-dev.1/PLATE140-Windows-x64.zip). Close the DAW, replace the full PLATE 140.vst3 folder and rescan. Native Windows automated gates passed; Reaper, Studio One 8 and listening remain untested.

## Build
Install CMake ≥3.22, a C++17 compiler, Git and Python 3.12. Install metric dependencies with `python -m pip install -r requirements-metrics.txt`. Linux also needs ALSA, X11/Xext/Xinerama/Xrandr/Xcursor/Xrender, FreeType/fontconfig development packages. JUCE 8.0.6 is fetched at a fixed commit if `JUCE_PATH` is not set.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure # Linux GUI tests need DISPLAY / Xvfb
```
Windows: Visual Studio 2022 x64 C++ desktop workload, then configure with `-G "Visual Studio 17 2022" -A x64`. Plugin output: `build/Plate140_artefacts/Release/VST3`. Copy the PLATE 140.vst3 bundle to the host's VST3 folder and rescan. Standalone lives beside it in `Standalone`.

Linux cloud instance: source `/workspace/tools/build-env.sh` first; use the configured build cache. Run `PlatePreview /absolute/output.png` under Xvfb to render the actual native editor without audio hardware. No web server is needed.

## Crackle fixes in 0.2.1
Host reset preserves the loaded preset and only clears DSP history. Dry MIX 0/bypass starts correctly, pre-delay changes crossfade fixed taps, filters interpolate coefficients and preset changes use coherent snapshots. Inactive plates and static GUI areas avoid continuous processing/repainting. See [crackle audit](docs/CRACKLE_AUDIT.md) for actual tests and remaining DAW/driver limitations.

## Controls
Plate A/B/C buttons select a tank, each retaining its 0.5–5.5 s time. Drag the vertical time meters, scroll, use arrow keys or ± buttons. All knobs support drag, wheel, keyboard arrows, tooltips and double-click default. Host parameter controls provide precise numeric entry. Input filter is a shared 10-position selector: OFF, M90/125/180/250/270/360, E−4/−10/−16. EQ and filter do not change on plate selection. Wet Solo defaults on; disable it for insert dry/wet use. Power is a smoothed dry bypass. VU reads output RMS with 300 ms smoothing and decays when bypassed.

The top preset bar opens a floating, draggable, nonmodal browser with search, category and tag filters, favourites, and user preset save/rename/delete. IN controls Power; A/B compares snapshots while the editor stays open; COPY/PASTE exchanges full XML state. MAP lists stable host automation IDs (not MIDI Learn). Right-click also provides preset/import/export options. User presets/favourites are saved atomically under JUCE's per-user application data directory (`Aulamix/PLATE140/library.xml`).

The browser and toolbar label AUX (73 presets, fully wet) versus INSERT (27 presets, stored dry/wet). Factory programs retain indices 0–99; Default is appended at index 100 so the first factory program is never confused with processor defaults. The 100 factory presets are embedded, without per-preset disk reads or graphics. `Presets/PLATE140-100` also contains individual XML presets, `Bank.json`, a full 20-parameter CSV map and Portuguese instructions. XML uses raw physical values and stable IDs; selecting/importing a preset maps every control automatically. Windows presets use the same XML schema. Build Windows x64 with `scripts/Build-Windows.ps1` or the existing CI workflow; the previous e6b13a3 revision passed the Windows CI gates and produced v0.2.0-dev.1. See docs/VALIDATION.md for the current revision status.

## Status
See `docs/VALIDATION.md` for executed checks and release gates. This is a development version, not a claim of UAD or historical hardware equivalence. Native editor and DSP are implemented together. Linux pluginval level 10 has passed; Studio One 8, Reaper and listening tests remain pending. VST2 is excluded without a legitimate SDK and license.

Architecture and assumptions: `docs/DSP_ARCHITECTURE.md`, `docs/REFERENCE_ANALYSIS.md`, `docs/PARAMETER_MAP.md`, `TODO.md`. JUCE licensing must be satisfied before distributing binaries.

## Photographic rack interface
The original-reference panel artwork is now embedded in the plugin through the PlateArtwork
binary resource target. Source/GUI/Assets/PanelBackground.png is the static plate;
knob indices, time pointers, toggles, input-filter cursor, lamp and VU needle are live
controls drawn over it. The preset toolbar remains functional. No external image
file is required when installing the VST3. IDs, ranges, parameter defaults and host normalization remain unchanged.
The individual factory voicings are documented in docs/PRESET_METRICS_BEFORE_AFTER.md. Windows CI saves a compiled editor screenshot and includes rack-ui.png plus BUILD-COMMIT.txt in the VST3 ZIP.

Windows graphics default to CPU/GDI with CPU image storage (`CHI_WINDOWS_SOFTWARE_GRAPHICS=ON`). Options → Graphics selects Direct2D explicitly; OFF compares upstream factories. See [editor lifetime audit](docs/EDITOR_LIFETIME.md), [measured before/after](docs/PRESET_METRICS_BEFORE_AFTER.md) and [validation](docs/VALIDATION.md).
