@echo off
setlocal EnableDelayedExpansion
set "BUILD_DIR=cmake-build-release"
set "FALLBACK_BUILD_DIR=cmake-build-release-%USERNAME%"

echo [1/4] Checking for CMake...
where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake was not found on this system.
    echo Attempting automatic install via winget...

    where winget >nul 2>nul
    if errorlevel 1 (
        echo.
        echo winget is not available, so CMake cannot be installed automatically.
        echo Install CMake manually from https://cmake.org/download/ and run this script again.
        exit /b 1
    )

    winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
    if errorlevel 1 (
        echo.
        echo Automatic CMake install failed.
        echo Install CMake manually from https://cmake.org/download/ and run this script again.
        exit /b 1
    )

    if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
        set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
    )
)

echo [2/4] Verifying CMake...
where cmake >nul 2>nul
if errorlevel 1 (
    echo.
    echo CMake is still not available in PATH.
    echo Open a new terminal and run this script again.
    exit /b 1
)

echo [3/4] Configuring project...
cmake -S . -B !BUILD_DIR!
if errorlevel 1 (
    echo.
    echo Configure failed in !BUILD_DIR!.
    echo Retrying with fallback folder: !FALLBACK_BUILD_DIR!
    set "BUILD_DIR=!FALLBACK_BUILD_DIR!"
    cmake -S . -B !BUILD_DIR!
    if errorlevel 1 (
        echo.
        echo CMake configure failed.
        exit /b 1
    )
)

echo [4/4] Building Release...
cmake --build !BUILD_DIR! --config Release
if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build completed successfully.
echo Run: !BUILD_DIR!\Release\2D_GameEngine.exe
exit /b 0
