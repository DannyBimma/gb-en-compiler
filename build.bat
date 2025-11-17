@echo off
REM Build script for Windows

echo ==================================
echo C to British English Compiler
echo Build Script for Windows
echo ==================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Please install CMake.
    echo Download from: https://cmake.org/download/
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

REM Move executable to root
if exist Release\c2en.exe (
    copy Release\c2en.exe ..\c2en.exe
) else if exist Debug\c2en.exe (
    copy Debug\c2en.exe ..\c2en.exe
) else if exist c2en.exe (
    copy c2en.exe ..\c2en.exe
)

cd ..

echo.
echo ==================================
echo Build completed successfully!
echo.
echo To test the compiler:
echo   c2en.exe examples\hello.c
echo ==================================

pause
