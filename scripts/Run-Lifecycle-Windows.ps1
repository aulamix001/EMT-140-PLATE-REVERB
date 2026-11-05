param([Parameter(Mandatory=$true)][string]$Executable)
$ErrorActionPreference = 'Stop'
$Executable = (Resolve-Path $Executable).Path
function Show-DumpStacks([string]$DumpPath, [string]$Prefix) {
  $cdb = Get-ChildItem 'C:\Program Files (x86)\Windows Kits','C:\Program Files\Windows Kits' -Filter cdb.exe -Recurse -ErrorAction SilentlyContinue | Where-Object FullName -Match '\\x64\\' | Select-Object -First 1
  if ($cdb -and (Test-Path $DumpPath)) {
    $debugger = Start-Process -FilePath $cdb.FullName -ArgumentList @('-z',"`"$DumpPath`"",'-c','"~* k; q"') -PassThru -RedirectStandardOutput "$Prefix-stacks.log" -RedirectStandardError "$Prefix-stacks-error.log"
    if (!$debugger.WaitForExit(30000)) { $debugger.Kill() }
    Get-Content "$Prefix-stacks.log" | Select-Object -Last 120
  } else { Write-Output 'CDB unavailable or no minidump; dump attempt recorded.' }
}
foreach ($scale in @('100','125')) {
  $prefix = "debug-lifetime-$scale"
  $arguments = @{FilePath=$Executable;PassThru=$true;RedirectStandardOutput="$prefix-out.log";RedirectStandardError="$prefix-err.log"}
  if ($scale -eq '125') { $arguments.ArgumentList = '--scale125' }
  $run = Start-Process @arguments
  $lastProgress = [DateTime]::UtcNow
  $lastSize = -1L
  $started = [DateTime]::UtcNow
  while (!$run.WaitForExit(250)) {
    $size = 0L
    foreach ($log in @("$prefix-out.log","$prefix-err.log")) { if (Test-Path $log) { $size += (Get-Item $log).Length } }
    if ($size -ne $lastSize) { $lastProgress=[DateTime]::UtcNow; $lastSize=$size }
    if (([DateTime]::UtcNow-$lastProgress).TotalSeconds -gt 45 -or ([DateTime]::UtcNow-$started).TotalSeconds -gt 900) {
      $dumpPath = Join-Path $env:GITHUB_WORKSPACE "$prefix-hang.dmp"
      $dumper = Start-Process -FilePath $Executable -ArgumentList @('--dump-process',"$($run.Id)","`"$dumpPath`"") -PassThru -RedirectStandardOutput "$prefix-dump.log" -RedirectStandardError "$prefix-dump-error.log"
      if (!$dumper.WaitForExit(15000)) { $dumper.Kill() }
      if (!$run.HasExited) { $run.Kill() };$run.WaitForExit()
      Get-Content "$prefix-out.log","$prefix-err.log" | Select-Object -Last 60
      Show-DumpStacks $dumpPath $prefix
      throw "Native lifecycle stalled at scale $scale; diagnostics saved"
    }
  }
  $run.WaitForExit()
  Get-Content "$prefix-out.log","$prefix-err.log" | Select-Object -Last 20
  if ($run.ExitCode -ne 0) {
    Show-DumpStacks (Join-Path $env:GITHUB_WORKSPACE 'plate-lifecycle-hang.dmp') $prefix
    throw "Native lifecycle failed at scale $scale with exit $($run.ExitCode)"
  }
  if (Select-String -Path "$prefix-out.log","$prefix-err.log" -Pattern 'JUCE Assertion failure' -Quiet) { throw 'JUCE assertion in lifecycle log' }
}
