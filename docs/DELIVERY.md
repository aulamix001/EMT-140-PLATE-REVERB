# Development delivery: 100 presets and floating browser
C++17/JUCE 8 source includes 100 embedded factory programs, 100 portable XML presets, a JSON manifest, 20-parameter CSV mapping and Portuguese usage guide in Presets/PLATE140-100.

Top preset bar opens a draggable nonmodal desktop browser with search, categories, tags, favourites and user save/rename/delete. A/B and clipboard functions are active. images/plate140.png and images/preset-browser.png are actual compiled UI captures. images/hardware-concept.png remains a generated design concept, not an interactive background.

Native Linux compilation, the DSP suite, 100-preset integration tests and pluginval level 10 passed. Logs: docs/validation. Windows x64 build script: scripts/Build-Windows.ps1; CI includes Windows build, integration and pluginval. No Windows executable/VST3 was compiled or validated here, and Studio One remains untested. Do not describe Linux binaries or source archives as Windows binaries.

The preset bank is original sound design based on the supplied specification and implemented parameter ranges. Official UA manual URLs were blocked by HTTP 403; no online manual consultation or proprietary preset equivalence is claimed. No remote push, GitHub Actions run or release was performed.
