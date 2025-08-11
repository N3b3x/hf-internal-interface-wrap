#!/bin/bash
# Build script for different ESP32 examples (Bash version)
# Usage: ./build_example.sh [example_type] [build_type]
# 
# Example types and build types are loaded from examples_config.yml
# Use './build_example.sh list' to see all available examples

set -e  # Exit on any error

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$PROJECT_DIR/scripts/config_loader.sh"

# Configuration
EXAMPLE_TYPE=${1:-$CONFIG_DEFAULT_EXAMPLE}
BUILD_TYPE=${2:-$CONFIG_DEFAULT_BUILD_TYPE}

# Handle special commands
if [ "$EXAMPLE_TYPE" = "list" ]; then
    echo "=== Available Example Types ==="
    echo "Featured examples:"
    for example in $(get_featured_example_types); do
        description=$(get_example_description "$example")
        echo "  $example - $description"
    done
    echo ""
    echo "All examples:"
    for example in $(get_example_types); do
        description=$(get_example_description "$example")
        echo "  $example - $description"
    done
    echo ""
    echo "Build types: $(get_build_types)"
    exit 0
fi

# Ensure ESP32-C6 target is set
export IDF_TARGET=$CONFIG_TARGET

echo "=== ESP32 HardFOC Interface Wrapper Build System ==="
echo "Project Directory: $PROJECT_DIR"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "Target: $IDF_TARGET"
echo "======================================================"

# Validate example type
if is_valid_example_type "$EXAMPLE_TYPE"; then
    echo "Valid example type: $EXAMPLE_TYPE"
    description=$(get_example_description "$EXAMPLE_TYPE")
    echo "Description: $description"
else
    echo "ERROR: Invalid example type: $EXAMPLE_TYPE"
    echo "Available types: $(get_example_types)"
    echo "Use './build_example.sh list' to see all examples with descriptions"
    exit 1
fi

# Validate build type
if is_valid_build_type "$BUILD_TYPE"; then
    echo "Valid build type: $BUILD_TYPE"
else
    echo "ERROR: Invalid build type: $BUILD_TYPE"
    echo "Available types: $(get_build_types)"
    exit 1
fi

# Switch to project directory
cd "$PROJECT_DIR"

# Set build directory using configuration
BUILD_DIR=$(get_build_directory "$EXAMPLE_TYPE" "$BUILD_TYPE")
echo "Build directory: $BUILD_DIR"

# Clean previous build if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Configure and build with proper error handling
echo "Configuring project for $IDF_TARGET..."
if ! idf.py -B "$BUILD_DIR" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" -D EXAMPLE_TYPE="$EXAMPLE_TYPE" reconfigure; then
    echo "ERROR: Configuration failed"
    exit 1
fi

echo "Building project..."
if ! idf.py -B "$BUILD_DIR" build; then
    echo "ERROR: Build failed"
    exit 1
fi

# Get actual binary information using configuration
PROJECT_NAME=$(get_project_name "$EXAMPLE_TYPE")
BIN_FILE="$BUILD_DIR/$PROJECT_NAME.bin"

echo "======================================================"
echo "BUILD COMPLETED SUCCESSFULLY"
echo "======================================================"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "Target: $IDF_TARGET"
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
