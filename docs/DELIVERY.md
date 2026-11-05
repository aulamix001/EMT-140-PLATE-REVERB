# Current delivery: PLATE 140 0.2.1 crackle fixes

[Download Windows x64 v0.2.1-dev.1](https://github.com/aulamix001/EMT-140-PLATE-REVERB/releases/download/v0.2.1-dev.1/PLATE140-Windows-x64.zip). Production commit `fea40a4eec01314e1d61e3bc20f54f1a320088e9`; verified PE32+ AMD64, ZIP CRC/SHA256 and all 100 preset mappings. Windows Debug/Release gates and pluginval 10 passed in [run 42085132758](https://github.com/aulamix001/EMT-140-PLATE-REVERB/actions/runs/42085132758).

Close the DAW, replace the full PLATE 140.vst3 folder under C:\Program Files\Common Files\VST3, rescan and check plugin version 0.2.1. Existing parameter IDs/ranges and preset formats are unchanged. See CRACKLE_AUDIT.md for exact fixes and measurements. Studio One 8, Reaper, physical ASIO/DPC and listening remain untested. PR creation is blocked by the repository's Actions permission; build/release succeeded.

The following record describes the previous 0.2.0 delivery.

# Development delivery: 100 presets and floating browser
C++17/JUCE 8 source includes 100 embedded factory programs, 100 portable XML presets, a JSON manifest, 20-parameter CSV mapping and Portuguese usage guide in Presets/PLATE140-100.

Top preset bar opens a draggable nonmodal desktop browser with search, categories, tags, favourites and user save/rename/delete. A/B and clipboard functions are active. images/plate140.png and images/preset-browser.png are actual compiled UI captures. images/hardware-concept.png is historical artwork. The current panel follows the latest original-reference request, with fixed knob shading and live pointers; its embedded plate/VU assets are documented in GUI_SPECIFICATION.md. The Windows workflow delivers rack-ui.png as a screenshot of the compiled editor.

Native Linux compilation, the DSP suite, 100-preset integration tests and pluginval level 10 passed. Logs: docs/validation. Windows x64 build script: scripts/Build-Windows.ps1; CI includes Windows build, integration and pluginval. Windows native CI passed all gates for e6b13a3 (v0.2.0-dev.1); the final e6b647b revision also passed all Windows gates in run 42061095886 and was published as v0.2.0-dev.2. The downloaded ZIP was verified as PE32+ AMD64 with the correct commit, SHA256 and all 100 matching presets. Studio One 8, Reaper and listening remain untested. Do not describe Linux binaries or source archives as Windows binaries.

The preset bank is original sound design based on the supplied specification and implemented parameter ranges. Official UA manual URLs were blocked by HTTP 403; no online manual consultation or proprietary preset equivalence is claimed. The fix-presets-audibility branch was pushed. Release publication requires the native Windows job to pass; docs/VALIDATION.md records which revision was actually executed.

Download: https://github.com/aulamix001/EMT-140-PLATE-REVERB/releases/download/v0.2.0-dev.2/PLATE140-Windows-x64.zip

Close the DAW, extract the ZIP and copy the full PLATE 140.vst3 folder from VST3
to C:\Program Files\Common Files\VST3, then rescan. Factory presets are embedded.
Reaper, Studio One 8 and listening remain untested. The only failed workflow job
was automatic PR creation, rejected by the repository's GitHub Actions permission.
The branch and docs/PR_DESCRIPTION.md are available for review.
