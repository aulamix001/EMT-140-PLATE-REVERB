$ErrorActionPreference = 'Stop'
# Run from Windows x64 with Visual Studio 2022 C++ desktop tools, CMake and Git.
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $ProjectRoot
try {
  cmake -S . -B Builds/Windows -G 'Visual Studio 17 2022' -A x64
  if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }
  cmake --build Builds/Windows --config Release --parallel 2
  if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
  ctest --test-dir Builds/Windows -C Release --output-on-failure
  if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
  Write-Host 'VST3: Builds/Windows/Plate140_artefacts/Release/VST3/PLATE 140.vst3'
  Write-Host '100 presets are embedded. Click the preset name to open the floating browser.'
} finally { Pop-Location }
