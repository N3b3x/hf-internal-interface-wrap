#!/bin/bash
# Flash and monitor script for different ESP32 examples (Bash version)
# Usage: ./flash_example.sh [example_type] [build_type] [operation]
# 
# Example types and build types are loaded from examples_config.yml
# Use './flash_example.sh list' to see all available examples
# Operations: flash, monitor, flash_monitor (default: flash_monitor)

set -e  # Exit on any error

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$PROJECT_DIR/scripts/config_loader.sh"

# Configuration
EXAMPLE_TYPE=${1:-$CONFIG_DEFAULT_EXAMPLE}
BUILD_TYPE=${2:-$CONFIG_DEFAULT_BUILD_TYPE}
OPERATION=${3:-flash_monitor}

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
    echo "Operations: flash, monitor, flash_monitor"
    exit 0
fi

# Ensure ESP32-C6 target is set
export IDF_TARGET=$CONFIG_TARGET

echo "=== ESP32 HardFOC Interface Wrapper Flash System ==="
echo "Project Directory: $PROJECT_DIR"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "Operation: $OPERATION"
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
    echo "Use './flash_example.sh list' to see all examples with descriptions"
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

# Validate operation
case $OPERATION in
    flash|monitor|flash_monitor)
        echo "Valid operation: $OPERATION"
        ;;
    *)
        echo "ERROR: Invalid operation: $OPERATION"
        echo "Available operations: flash, monitor, flash_monitor"
        exit 1
        ;;
esac

# Switch to project directory
cd "$PROJECT_DIR"

# Set build directory using configuration
BUILD_DIR=$(get_build_directory "$EXAMPLE_TYPE" "$BUILD_TYPE")
echo "Build directory: $BUILD_DIR"

# Get project information using configuration
PROJECT_NAME=$(get_project_name "$EXAMPLE_TYPE")
BIN_FILE="$BUILD_DIR/$PROJECT_NAME.bin"

# Check if build exists and is valid
BUILD_EXISTS=false
if [ -d "$BUILD_DIR" ]; then
    if [ -f "$BIN_FILE" ] || [ -f "$BUILD_DIR/bootloader/bootloader.bin" ]; then
        echo "Found existing build in $BUILD_DIR"
        BUILD_EXISTS=true
    else
        echo "Build directory exists but no valid binary found"
    fi
else
    echo "No build directory found"
fi

# Auto-build if necessary
if [ "$BUILD_EXISTS" = false ]; then
    echo ""
    echo "======================================================"
    echo "NO VALID BUILD FOUND - STARTING AUTO-BUILD"
    echo "======================================================"
    echo "Building $EXAMPLE_TYPE ($BUILD_TYPE) before flashing..."
    
    # Clean any existing incomplete build
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning incomplete build..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Configure and build
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
    
    echo "Build completed successfully!"
    echo "======================================================"
else
    echo "Using existing build in $BUILD_DIR"
fi

# Verify binary exists after build
if [ ! -f "$BIN_FILE" ] && [ ! -f "$BUILD_DIR/bootloader/bootloader.bin" ]; then
    echo "ERROR: No valid binary found after build attempt"
    echo "Expected: $BIN_FILE"
    echo "Build directory contents:"
    ls -la "$BUILD_DIR" 2>/dev/null || echo "Build directory not accessible"
    exit 1
fi

# Execute the requested operation
echo ""
echo "======================================================"
echo "EXECUTING OPERATION: $OPERATION"
echo "======================================================"

case $OPERATION in
    flash)
        echo "Flashing $EXAMPLE_TYPE example..."
        if ! idf.py -B "$BUILD_DIR" flash; then
            echo "ERROR: Flash operation failed"
            exit 1
        fi
        echo "Flash completed successfully!"
        ;;
    monitor)
        echo "Starting monitor for $EXAMPLE_TYPE example..."
        echo "Press Ctrl+] to exit monitor"
        if ! idf.py -B "$BUILD_DIR" monitor; then
            echo "ERROR: Monitor operation failed"
            exit 1
        fi
        ;;
    flash_monitor)
        echo "Flashing and monitoring $EXAMPLE_TYPE example..."
        echo "Press Ctrl+] to exit monitor after flashing"
        if ! idf.py -B "$BUILD_DIR" flash monitor; then
            echo "ERROR: Flash and monitor operation failed"
            exit 1
        fi
        ;;
esac

echo ""
echo "======================================================"
echo "OPERATION COMPLETED SUCCESSFULLY"
echo "======================================================"
echo "Example Type: $EXAMPLE_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "Operation: $OPERATION"
echo "Target: $IDF_TARGET"
echo "Build Directory: $BUILD_DIR"
echo "Project Name: $PROJECT_NAME"
if [ -f "$BIN_FILE" ]; then
    echo "Binary: $BIN_FILE"
fi
echo ""
echo "Available operations:"
echo "  Flash only:        ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE flash"
echo "  Monitor only:      ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE monitor"
echo "  Flash & monitor:   ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE flash_monitor"
echo "  Build only:        ./build_example.sh $EXAMPLE_TYPE $BUILD_TYPE"
echo "======================================================"

