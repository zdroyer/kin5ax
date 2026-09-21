# Builds Debug|x64 and Release|x64, runs the GoogleTest exe (kin5axTests, Debug), optionally starts the application
# with a chain file (-Start). Exit code 0 = everything passed.
# Usage:  powershell -ExecutionPolicy Bypass -File kin5axTests/build_and_test.ps1 [-Start]
param([switch]$Start)
$root = Split-Path -Parent $PSScriptRoot
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
$bad = 0
foreach ($cfg in "Debug","Release") {
  $o = & $msbuild "$root\Universal5CMillingMachinePostprocessor.sln" /p:Configuration=$cfg /p:Platform=x64 /m /nologo /v:minimal
  $code = $LASTEXITCODE
  $e = @($o | Select-String ': (fatal )?error '); $w = @($o | Select-String ': warning ')
  "BUILD ${cfg}|x64  exit $code  errors $($e.Count)  warnings $($w.Count)"
  ($e + $w) | ForEach-Object { "    " + (($_.Line -replace '\s*\[.*vcxproj\]','') -replace [regex]::Escape("$root\"),'').Trim() } | Sort-Object -Unique | Select-Object -First 12
  if ($code -ne 0) { $bad++ }
}
if ($bad -eq 0) {
  $t = & "$root\x64\Debug\kin5axTests.exe" --gtest_color=no 2>&1
  $code = $LASTEXITCODE
  $t | Select-String "^\[  FAILED  \]|Failure|^\[  PASSED  \]|tests ran" | ForEach-Object { "TESTS " + $_.Line.Trim() } | Select-Object -First 30
  if ($code -ne 0) { "TESTS exit code $code"; $bad++ }
}
if ($Start -and $bad -eq 0) {
  foreach ($cfg in "Debug","Release") {
    $p = Start-Process "$root\x64\$cfg\Universal5CMillingMachinePostprocessor.exe" -ArgumentList "`"$root\3CWithSphericalWrist_withBoxes.kc`"" -WorkingDirectory $root -PassThru
    Start-Sleep -Seconds 4; $p.Refresh()
    if ($p.HasExited) { "START ${cfg}: exited early, code $($p.ExitCode)"; $bad++ }
    else { "START ${cfg}: running, responding=$($p.Responding), '$($p.MainWindowTitle)'"; $p.CloseMainWindow() | Out-Null; Start-Sleep 2; $p.Refresh(); if (-not $p.HasExited) { $p.Kill() } }
  }
}
exit $bad
