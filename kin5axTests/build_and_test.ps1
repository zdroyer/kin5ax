# Builds everything on Windows and runs the tests. Exit code 0 = everything passed.
#   1. CMake: the kin5ax library and the kin5axTests suite into <root>\build (Debug and Release), run the Debug tests
#   2. msbuild: the Visual Studio solution of the MFC application (Debug|x64 and Release|x64); its kin5ax project
#      is a Makefile project that links the library CMake built
#   3. -Start: launch both application builds with a chain file for a few seconds
# Usage:  powershell -ExecutionPolicy Bypass -File kin5axTests/build_and_test.ps1 [-Start]
param([switch]$Start)
$root = Split-Path -Parent $PSScriptRoot
$sol = "$root\Universal5CMillingMachinePostprocessor"   # the Visual Studio solution lives with the MFC application
$build = "$root\build"
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
$vsMajor = (& $vswhere -latest -property installationVersion | Select-Object -First 1).Split('.')[0]
$generator = if ($vsMajor -eq "17") { "Visual Studio 17 2022" } else { "Visual Studio 18 2026" }
$bad = 0

# 1. CMake: library and tests
if (-not (Test-Path "$build\CMakeCache.txt")) {
  # -T v143: the toolset of the application project, so that the library links into it
  $o = & cmake -S $root -B $build -G $generator -A x64 -T v143
  if ($LASTEXITCODE -ne 0) { "CMAKE configure failed"; $o | Select-Object -Last 15; exit 1 }
}
foreach ($cfg in "Debug","Release") {
  $o = & cmake --build $build --config $cfg --parallel
  $code = $LASTEXITCODE
  $e = @($o | Select-String ': (fatal )?error '); $w = @($o | Select-String ': warning ')
  "CMAKE ${cfg}|x64  exit $code  errors $($e.Count)  warnings $($w.Count)"
  ($e + $w) | ForEach-Object { "    " + (($_.Line -replace '\s*\[.*vcxproj\]','') -replace [regex]::Escape("$root\"),'').Trim() } | Sort-Object -Unique | Select-Object -First 12
  if ($code -ne 0) { $bad++ }
}
if ($bad -eq 0) {
  $t = & "$build\kin5axTests\Debug\kin5axTests.exe" --gtest_color=no 2>&1
  $code = $LASTEXITCODE
  $t | Select-String "^\[  FAILED  \]|Failure|^\[  PASSED  \]|tests ran" | ForEach-Object { "TESTS " + $_.Line.Trim() } | Select-Object -First 30
  if ($code -ne 0) { "TESTS exit code $code"; $bad++ }
}

# 2. msbuild: the application
foreach ($cfg in "Debug","Release") {
  $o = & $msbuild "$sol\Universal5CMillingMachinePostprocessor.sln" /p:Configuration=$cfg /p:Platform=x64 /m /nologo /v:minimal
  $code = $LASTEXITCODE
  $e = @($o | Select-String ': (fatal )?error '); $w = @($o | Select-String ': warning ')
  "APP ${cfg}|x64  exit $code  errors $($e.Count)  warnings $($w.Count)"
  ($e + $w) | ForEach-Object { "    " + (($_.Line -replace '\s*\[.*vcxproj\]','') -replace [regex]::Escape("$root\"),'').Trim() } | Sort-Object -Unique | Select-Object -First 12
  if ($code -ne 0) { $bad++ }
}

# 3. the application starts
if ($Start -and $bad -eq 0) {
  foreach ($cfg in "Debug","Release") {
    $p = Start-Process "$sol\x64\$cfg\Universal5CMillingMachinePostprocessor.exe" -ArgumentList "`"$sol\3CWithSphericalWrist_withBoxes.kc`"" -WorkingDirectory $sol -PassThru
    Start-Sleep -Seconds 4; $p.Refresh()
    if ($p.HasExited) { "START ${cfg}: exited early, code $($p.ExitCode)"; $bad++ }
    else { "START ${cfg}: running, responding=$($p.Responding), '$($p.MainWindowTitle)'"; $p.CloseMainWindow() | Out-Null; Start-Sleep 2; $p.Refresh(); if (-not $p.HasExited) { $p.Kill() } }
  }
}
exit $bad
