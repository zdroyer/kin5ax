@echo off
rem Builds the kin5ax library with CMake for the Visual Studio solution of the application.
rem   %1 Configuration (Debug|Release)   %2 Platform (x64|Win32)   %3 empty | rebuild | clean
rem The build tree is <repository root>\build (x64) or build-win32; it is configured on first use with the
rem generator of the Visual Studio that runs this script (VisualStudioVersion 18 -> "Visual Studio 18 2026")
rem and the toolset v143 of the application project: a library built with a newer STL does not link into it.
setlocal
set ROOT=%~dp0..
set CONFIG=%1
set BUILD=%ROOT%\build
set ARCH=x64
if /I "%2"=="Win32" set BUILD=%ROOT%\build-win32
if /I "%2"=="Win32" set ARCH=Win32
set GEN=Visual Studio 18 2026
if "%VisualStudioVersion%"=="17.0" set GEN=Visual Studio 17 2022
set EXTRA=
if "%3"=="rebuild" set EXTRA=--clean-first

if "%3"=="clean" (
  if exist "%BUILD%\kin5ax\%CONFIG%" rmdir /s /q "%BUILD%\kin5ax\%CONFIG%"
  exit /b 0
)
if not exist "%BUILD%\CMakeCache.txt" (
  echo Configuring the CMake build tree %BUILD% with the generator "%GEN%", toolset v143 ...
  cmake -S "%ROOT%" -B "%BUILD%" -G "%GEN%" -A %ARCH% -T v143 || exit /b 1
)
cmake --build "%BUILD%" --config %CONFIG% --target kin5ax %EXTRA% || exit /b 1
exit /b 0
