@echo off
setlocal enabledelayedexpansion

:: * parsing arguments
set "BUILD_TYPE=Debug"
if "%~1"=="--release" (
set "BUILD_TYPE=Release"
)
:: * parsing arguments end

:: * variables

:: Colors (ANSI)
for /f "tokens=2 delims==" %%I in ('"echo prompt $E ^| cmd"') do set "ESC=%%I"
set "GREEN=%ESC%[0;32m"
set "RED=%ESC%[0;31m"
set "YELLOW=%ESC%[1;33m"
set "CYAN=%ESC%[0;36m"
set "PURPLE=%ESC%[0;35m"
set "RESET=%ESC%[0m"

:: * variables end

echo %CYAN%Assuming Platform is Windows...%RESET%

cls
cls
cls

:: stop on errors
setlocal enabledelayedexpansion
set ERRLEV=0

:: ensure build folder exists
if not exist build (
mkdir build
)

:: setting preset and overriding build type
echo %YELLOW%Generating build files using CMake preset...%RESET%
cmake --preset default-configure -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

echo %YELLOW%Compiling using CMake preset...%RESET%
cmake --build --preset default-build --config %BUILD_TYPE%

echo %PURPLE%Executable Started...%RESET%

set "EXE_PATH=build\main.exe"

if not exist "!EXE_PATH!" (
    echo %RED%Executable not found: !EXE_PATH!%RESET%
    exit /b 1
)

set "starttime=%time%"
build\main.exe
set "endtime=%time%"

:: * print time
for /f "tokens=1-4 delims=:.," %%a in ("%starttime%") do (
    set /a startsecs=(%%a*3600)+(%%b*60)+%%c
)
for /f "tokens=1-4 delims=:.," %%a in ("%endtime%") do (
    set /a endsecs=(%%a*3600)+(%%b*60)+%%c
)

set /a elapsedsecs=!endsecs!-!startsecs!
if !elapsedsecs! LSS 0 set /a elapsedsecs+=86400

echo %PURPLE%Elapsed Time: !elapsedsecs! seconds%RESET%

:: * print time end

endlocal
pause
