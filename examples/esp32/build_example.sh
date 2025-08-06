#!/bin/bash
# Build script for different ESP32 examples (Bash version)
# Usage: ./build_example.sh [example_type] [build_type]
# 
# Example types:
#   comprehensive  - Main integration test (default)
#   ascii_art      - ASCII art generator example
#   nimble_test    - NimBLE test example
#
# Build types: Debug, Release (default: Release)

set -e  # Exit on any error

# Configuration
EXAMPLE_TYPE=${1:-comprehensive}
BUILD_TYPE=${2:-Release}
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== ESP32 HardFOC Interface Wrapper Build System ==="
echo "Project Directory: $PROJECT_DIR"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "======================================================"

# Validate example type
case $EXAMPLE_TYPE in
    comprehensive|ascii_art|nimble_test)
        echo "Valid example type: $EXAMPLE_TYPE"
        ;;
    *)
        echo "ERROR: Invalid example type: $EXAMPLE_TYPE"
        echo "Available types: comprehensive, ascii_art, nimble_test"
        exit 1
        ;;
esac

# Validate build type
case $BUILD_TYPE in
    Debug|Release)
        echo "Valid build type: $BUILD_TYPE"
        ;;
    *)
        echo "ERROR: Invalid build type: $BUILD_TYPE"
        echo "Available types: Debug, Release"
        exit 1
        ;;
esac

# Switch to project directory
cd "$PROJECT_DIR"

# Set build directory
BUILD_DIR="build_${EXAMPLE_TYPE}_${BUILD_TYPE}"
echo "Build directory: $BUILD_DIR"

# Clean previous build if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Configure and build with proper error handling
echo "Configuring project..."
if ! idf.py -B "$BUILD_DIR" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" -D EXAMPLE_TYPE="$EXAMPLE_TYPE" reconfigure; then
    echo "ERROR: Configuration failed"
    exit 1
fi

echo "Building project..."
if ! idf.py -B "$BUILD_DIR" build; then
    echo "ERROR: Build failed"
    exit 1
fi

# Get actual binary information
PROJECT_NAME="esp32_iid_${EXAMPLE_TYPE}_example"
BIN_FILE="$BUILD_DIR/$PROJECT_NAME.bin"

echo "======================================================"
echo "BUILD COMPLETED SUCCESSFULLY"
echo "======================================================"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "Project Name: $PROJECT_NAME"
if [ -f "$BIN_FILE" ]; then
    echo "Binary: $BIN_FILE"
else
    echo "Binary: Check $BUILD_DIR for output files"
fi
echo ""
echo "Next steps:"
echo "  Flash and monitor: idf.py -B $BUILD_DIR flash monitor"
echo "  Monitor only:      idf.py -B $BUILD_DIR monitor"
echo "  Size analysis:     idf.py -B $BUILD_DIR size"
echo "======================================================"
