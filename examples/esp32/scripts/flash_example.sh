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

# Ensure ESP-IDF environment is sourced
if [ -z "$IDF_PATH" ] || ! command -v idf.py &> /dev/null; then
    echo "ESP-IDF environment not found, attempting to source..."
    if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
        source "$HOME/esp/esp-idf/export.sh"
        echo "ESP-IDF environment sourced successfully"
    else
        echo "ERROR: ESP-IDF export.sh not found at $HOME/esp/esp-idf/export.sh"
        echo "Please ensure ESP-IDF is installed and IDF_PATH is set"
        exit 1
    fi
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

# Set build directory using configuration (same logic as build_example.sh)
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
    
    # Configure and build with retry logic (matching build_example.sh configuration)
    echo "Configuring project for $IDF_TARGET..."
    
    # Enable ccache by default (matching build script behavior)
    export IDF_CCACHE_ENABLE=1
    
    local config_attempts=0
    local max_config_attempts=3
    
    while [ $config_attempts -lt $max_config_attempts ]; do
        if idf.py -B "$BUILD_DIR" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" -D EXAMPLE_TYPE="$EXAMPLE_TYPE" -D IDF_CCACHE_ENABLE=1 reconfigure; then
            break
        else
            config_attempts=$((config_attempts + 1))
            if [ $config_attempts -lt $max_config_attempts ]; then
                echo "Configuration attempt $config_attempts failed, retrying..."
                sleep 2
            else
                echo "ERROR: Configuration failed after $max_config_attempts attempts"
                echo "This might be due to:"
                echo "  - CMake version incompatibility"
                echo "  - Missing dependencies"
                echo "  - Corrupted build files"
                exit 1
            fi
        fi
    done
    
    echo "Building project..."
    if ! idf.py -B "$BUILD_DIR" build; then
        echo "ERROR: Build failed"
        echo "Build logs are available in: $BUILD_DIR/log/"
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

# Smart port detection and permission handling
echo ""
echo "======================================================"
echo "SMART PORT DETECTION AND PERMISSION HANDLING"
echo "======================================================"

# Function to find the best available port
find_best_port() {
    local ports=()
    
    # Check for ESP32-C6 native USB port first (preferred)
    if [ -e "/dev/ttyACM0" ]; then
        ports+=("/dev/ttyACM0")
    fi
    
    # Check for other USB serial ports
    for i in {1..10}; do
        if [ -e "/dev/ttyACM$i" ]; then
            ports+=("/dev/ttyACM$i")
        fi
    done
    
    # Check for traditional serial ports (fallback)
    for i in {0..31}; do
        if [ -e "/dev/ttyS$i" ]; then
            ports+=("/dev/ttyS$i")
        fi
    done
    
    # Return the first available port
    if [ ${#ports[@]} -gt 0 ]; then
        echo "${ports[0]}"
        return 0
    else
        return 1
    fi
}

# Function to fix port permissions
fix_port_permissions() {
    local port="$1"
    if [ -e "$port" ]; then
        # Check if user can access the port
        if ! [ -r "$port" ]; then
            echo "Fixing permissions for $port..."
            sudo chmod 666 "$port" 2>/dev/null || {
                echo "WARNING: Could not fix permissions for $port"
                echo "You may need to run: sudo chmod 666 $port"
            }
        fi
    fi
}

# Find and configure the best available port
BEST_PORT=$(find_best_port)
if [ -z "$BEST_PORT" ]; then
    echo "ERROR: No serial ports found. Please connect your ESP32 device."
    exit 1
fi

echo "Detected port: $BEST_PORT"
fix_port_permissions "$BEST_PORT"

# Set the port for ESP-IDF
export ESPPORT="$BEST_PORT"
echo "Using port: $ESPPORT"

# Execute the requested operation
echo ""
echo "======================================================"
echo "EXECUTING OPERATION: $OPERATION"
echo "======================================================"

case $OPERATION in
    flash)
        echo "Flashing $EXAMPLE_TYPE example to $BEST_PORT..."
        if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" flash; then
            echo "ERROR: Flash operation failed"
            exit 1
        fi
        echo "Flash completed successfully!"
        ;;
    monitor)
        echo "Starting monitor for $EXAMPLE_TYPE example on $BEST_PORT..."
        echo "Press Ctrl+] to exit monitor"
        if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" monitor; then
            echo "ERROR: Monitor operation failed"
            exit 1
        fi
        ;;
    flash_monitor)
        echo "Flashing and monitoring $EXAMPLE_TYPE example on $BEST_PORT..."
        echo "Press Ctrl+] to exit monitor after flashing"
        if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" flash monitor; then
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

