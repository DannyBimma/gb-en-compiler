#!/bin/bash
# Build script for Linux and macOS

set -e

echo "=================================="
echo "C to British English Compiler"
echo "Build Script for Linux/macOS"
echo "=================================="
echo ""

# Check for build system preference
if command -v cmake &> /dev/null && [ "$1" == "--cmake" ]; then
    echo "Using CMake build system..."
    echo ""

    # Create build directory
    mkdir -p build
    cd build

    # Configure
    cmake .. -DCMAKE_BUILD_TYPE=Release

    # Build
    cmake --build .

    # Move executable to root
    mv c2en ../c2en
    cd ..

    echo ""
    echo "Build successful! Executable: ./c2en"
else
    echo "Using Make build system..."
    echo ""

    # Build with make
    make clean
    make
fi

echo ""
echo "=================================="
echo "Build completed successfully!"
echo ""
echo "To test the compiler:"
echo "  ./c2en examples/hello.c"
echo ""
echo "To install system-wide:"
echo "  sudo make install"
echo "=================================="
