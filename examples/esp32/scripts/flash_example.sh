#!/bin/bash
# Flash and monitor script for different ESP32 examples (Bash version)
# Usage: ./flash_example.sh [example_type] [build_type] [operation] [enable_logging]
# 
# Example types and build types are loaded from examples_config.yml
# Use './flash_example.sh list' to see all available examples
# Operations: flash, monitor, flash_monitor (default: flash_monitor)
# Logging: true/false (default: false) - saves monitor output to logs/

set -e  # Exit on any error

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$(dirname "${BASH_SOURCE[0]}")/config_loader.sh"

# Configuration
EXAMPLE_TYPE=${1:-$CONFIG_DEFAULT_EXAMPLE}
BUILD_TYPE=${2:-$CONFIG_DEFAULT_BUILD_TYPE}
OPERATION=${3:-flash_monitor}
ENABLE_LOGGING=${4:-false}  # Fourth parameter to enable logging

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
    echo "Logging: true/false (saves monitor output to logs/ directory)"
    echo ""
    echo "Examples:"
    echo "  ./flash_example.sh gpio Debug flash_monitor true  # Enable logging"
    echo "  ./flash_example.sh i2c Release monitor false     # Disable logging"
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
echo "Logging Enabled: $ENABLE_LOGGING"
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
echo "Expected binary: $BIN_FILE"
echo "Project name: $PROJECT_NAME"

# Check if build exists and is valid
BUILD_EXISTS=false
if [ -d "$BUILD_DIR" ]; then
    # Check for multiple indicators of a valid build
    if [ -f "$BIN_FILE" ]; then
        echo "Found existing build with main binary: $BIN_FILE"
        BUILD_EXISTS=true
    elif [ -f "$BUILD_DIR/bootloader/bootloader.bin" ]; then
        echo "Found existing build with bootloader binary"
        BUILD_EXISTS=true
    elif [ -f "$BUILD_DIR/.bin_timestamp" ] && [ -f "$BUILD_DIR/build.ninja" ]; then
        echo "Found existing build with build artifacts (.bin_timestamp and build.ninja)"
        BUILD_EXISTS=true
    elif [ -f "$BUILD_DIR/CMakeCache.txt" ] && [ -f "$BUILD_DIR/build.ninja" ]; then
        echo "Found existing build with CMake artifacts"
        BUILD_EXISTS=true
    else
        echo "Build directory exists but no clear indicators of valid build found"
        echo "Checking for any binary files..."
        # Look for any .bin files in the build directory
        if find "$BUILD_DIR" -name "*.bin" -type f | grep -q .; then
            echo "Found .bin files, considering build valid"
            echo "Available .bin files:"
            find "$BUILD_DIR" -name "*.bin" -type f | sed 's/^/  /'
            BUILD_EXISTS=true
        else
            echo "No .bin files found, build may be incomplete"
            echo "Build directory contents:"
            ls -la "$BUILD_DIR" 2>/dev/null | head -20 || echo "Cannot list build directory contents"
        fi
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
    echo ""
    echo "Build validation failed because:"
    if [ ! -d "$BUILD_DIR" ]; then
        echo "  - Build directory does not exist: $BUILD_DIR"
    else
        echo "  - Build directory exists but validation failed"
        echo "  - Expected binary: $BIN_FILE"
        echo "  - Build directory: $BUILD_DIR"
    fi
    echo ""
    
    # Clean any existing incomplete build
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning incomplete build..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Use build_example.sh as the single source of truth for building
    echo "Calling build_example.sh to ensure consistent build process..."
    echo ""
    
    # Call build_example.sh with the same parameters
    if ! "$(dirname "${BASH_SOURCE[0]}")/build_example.sh" "$EXAMPLE_TYPE" "$BUILD_TYPE"; then
        echo "ERROR: Build failed - see build_example.sh output above"
        exit 1
    fi
    
    echo "Build completed successfully via build_example.sh!"
    echo "=================================================="
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

# Function to detect operating system
detect_os() {
    case "$(uname -s)" in
        Darwin*)    echo "macos" ;;
        Linux*)     echo "linux" ;;
        CYGWIN*|MINGW*|MSYS*) echo "windows" ;;
        *)          echo "unknown" ;;
    esac
}

# Function to find ESP32 devices using system-specific methods
find_esp32_devices() {
    local os=$(detect_os)
    local devices=()
    
    case "$os" in
        "macos")
            # macOS: Look for ESP32 devices in /dev/cu.* (callout devices are better for serial)
            # ESP32 devices typically appear as usbmodem, usbserial, or similar
            for port in /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.SLAB_USBtoUART* /dev/cu.CP210* /dev/cu.CH340*; do
                if [ -e "$port" ]; then
                    devices+=("$port")
                fi
            done
            
            # Also check /dev/tty.* as fallback
            for port in /dev/tty.usbmodem* /dev/tty.usbserial* /dev/tty.SLAB_USBtoUART* /dev/tty.CP210* /dev/tty.CH340*; do
                if [ -e "$port" ]; then
                    devices+=("$port")
                fi
            done
            ;;
            
        "linux")
            # Linux: Check for ESP32 devices in /dev/ttyACM* and /dev/ttyUSB*
            # ESP32-C6 typically uses /dev/ttyACM*, older ESP32 uses /dev/ttyUSB*
            for port in /dev/ttyACM* /dev/ttyUSB*; do
                if [ -e "$port" ]; then
                    devices+=("$port")
                fi
            done
            
            # Also check for specific ESP32 device names if available
            if command -v lsusb &> /dev/null; then
                # Look for ESP32-related USB devices
                if lsusb | grep -q "Silicon Labs\|CP210\|CH340\|ESP\|Espressif"; then
                    # Silent detection - no output needed here
                    :
                fi
            fi
            ;;
            
        *)
            # Silent warning for unsupported OS
            :
            ;;
    esac
    
    # Return devices as space-separated string
    echo "${devices[@]}"
}

# Function to find the best available port
find_best_port() {
    local os=$(detect_os)
    local esp32_devices=($(find_esp32_devices))
    local fallback_ports=()
    
    # First priority: ESP32-specific devices
    if [ ${#esp32_devices[@]} -gt 0 ]; then
        # Prefer callout devices on macOS (cu.* over tty.*)
        if [ "$os" = "macos" ]; then
            for port in "${esp32_devices[@]}"; do
                if [[ "$port" == "/dev/cu."* ]]; then
                    echo "$port"
                    return 0
                fi
            done
        fi
        
        # Use first ESP32 device found
        echo "${esp32_devices[0]}"
        return 0
    fi
    
    # Second priority: Fallback to common serial ports
    case "$os" in
        "macos")
            # macOS fallback ports
            for port in /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.*; do
                if [ -e "$port" ] && [[ "$port" != "/dev/cu.Bluetooth"* ]] && [[ "$port" != "/dev/cu.debug"* ]] && [[ "$port" != "/dev/cu.wlan"* ]]; then
                    fallback_ports+=("$port")
                fi
            done
            ;;
        "linux")
            # Linux fallback ports
            for port in /dev/ttyACM* /dev/ttyUSB* /dev/ttyS*; do
                if [ -e "$port" ]; then
                    fallback_ports+=("$port")
                fi
            done
            ;;
    esac
    
    if [ ${#fallback_ports[@]} -gt 0 ]; then
        echo "Using fallback port: ${fallback_ports[0]}"
        echo "${fallback_ports[0]}"
        return 0
    fi
    
    # No ports found
    return 1
}

# Function to fix port permissions (Linux-specific)
fix_port_permissions() {
    local port="$1"
    local os=$(detect_os)
    
    if [ "$os" = "linux" ] && [ -e "$port" ]; then
        # Check if user can access the port
        if ! [ -r "$port" ]; then
            echo "Fixing permissions for $port..."
            sudo chmod 666 "$port" 2>/dev/null || {
                echo "WARNING: Could not fix permissions for $port"
                echo "You may need to run: sudo chmod 666 $port"
                echo "Or add your user to the dialout group: sudo usermod -a -G dialout $USER"
            }
        fi
    elif [ "$os" = "macos" ]; then
        # On macOS, permissions are usually handled by system
        echo "macOS detected - checking port accessibility..."
        if [ -r "$port" ]; then
            echo "Port $port is accessible"
        else
            echo "WARNING: Port $port is not accessible"
            echo "This might be a permission issue. Try:"
            echo "  - Disconnecting and reconnecting the device"
            echo "  - Checking System Preferences > Security & Privacy > Privacy > Full Disk Access"
        fi
    fi
}

# Function to validate port and get device info
validate_port() {
    local port="$1"
    local os=$(detect_os)
    
    if [ ! -e "$port" ]; then
        echo "ERROR: Port $port does not exist"
        return 1
    fi
    
    if [ ! -r "$port" ]; then
        echo "ERROR: Port $port is not readable"
        return 1
    fi
    
    # Try to get device info
    case "$os" in
        "macos")
            if [[ "$port" == "/dev/cu."* ]] || [[ "$port" == "/dev/tty."* ]]; then
                echo "Port $port appears to be a valid serial device"
                return 0
            fi
            ;;
        "linux")
            if [[ "$port" == "/dev/tty"* ]]; then
                echo "Port $port appears to be a valid serial device"
                return 0
            fi
            ;;
    esac
    
    echo "WARNING: Port $port format is unexpected for this OS"
    return 0  # Still allow it as it might work
}

# Find and configure the best available port
echo "Searching for ESP32 devices..."
BEST_PORT=$(find_best_port)

if [ -z "$BEST_PORT" ]; then
    echo ""
    echo "ERROR: No suitable serial ports found!"
    echo ""
    echo "Troubleshooting steps:"
    echo "1. Ensure your ESP32 device is connected via USB"
    echo "2. Check if the device appears in your system:"
    
    case "$(detect_os)" in
        "macos")
            echo "   - System Information > USB"
            echo "   - Terminal: ls /dev/cu.* /dev/tty.*"
            echo "   - Look for usbmodem, usbserial, or similar devices"
            ;;
        "linux")
            echo "   - lsusb (if available)"
            echo "   - ls /dev/ttyACM* /dev/ttyUSB*"
            echo "   - dmesg | tail (after connecting device)"
            ;;
    esac
    
    echo "3. Try disconnecting and reconnecting the device"
    echo "4. Check if you need to install USB-to-UART drivers"
    echo "5. Ensure the device is not being used by another application"
    echo ""
    
    # Offer manual port specification
    echo "You can also try manually specifying a port:"
    echo "  export ESPPORT=/dev/your_port_here"
    echo "  ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE $OPERATION"
    echo ""
    echo "Or run the port detection script for help:"
    echo "  ./examples/esp32/scripts/detect_ports.sh --verbose --test-connection"
    echo ""
    
    # Check if ESPPORT is already set
    if [ -n "$ESPPORT" ]; then
        echo "ESPPORT is currently set to: $ESPPORT"
        if [ -e "$ESPPORT" ]; then
            echo "This port exists. Would you like to use it? (y/n)"
            read -r response
            if [[ "$response" =~ ^[Yy]$ ]]; then
                BEST_PORT="$ESPPORT"
                echo "Using manually specified port: $BEST_PORT"
            else
                exit 1
            fi
        else
            echo "WARNING: ESPPORT is set to $ESPPORT but this port does not exist"
            exit 1
        fi
    else
        exit 1
    fi
fi

echo "Detected port: $BEST_PORT"

# Validate the port
if ! validate_port "$BEST_PORT"; then
    echo "ERROR: Port validation failed for $BEST_PORT"
    exit 1
fi

# Fix permissions if needed
fix_port_permissions "$BEST_PORT"

# Set the port for ESP-IDF
export ESPPORT="$BEST_PORT"
echo "Using port: $ESPPORT"

# Setup logging if enabled
LOG_FILE=""
if [[ "$ENABLE_LOGGING" == "true" ]] && [[ "$OPERATION" == "monitor" || "$OPERATION" == "flash_monitor" ]]; then
    # Create logs directory if it doesn't exist
    LOGS_DIR="$PROJECT_DIR/logs"
    mkdir -p "$LOGS_DIR"
    
    # Generate timestamped log filename
    TIMESTAMP=$(date '+%Y%m%d_%H%M%S')
    LOG_FILE="$LOGS_DIR/${EXAMPLE_TYPE}_${BUILD_TYPE}_${TIMESTAMP}.log"
    
    echo ""
    echo "======================================================"
    echo "LOGGING SETUP"
    echo "======================================================"
    echo "Monitor output will be saved to: $LOG_FILE"
    echo "Real-time viewing: enabled (using tee)"
    echo "Log directory: $LOGS_DIR"
    echo "======================================================"
fi

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
        if [[ -n "$LOG_FILE" ]]; then
            echo "Monitor output being logged to: $LOG_FILE"
            if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" monitor 2>&1 | tee "$LOG_FILE"; then
                echo "ERROR: Monitor operation failed"
                exit 1
            fi
        else
            if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" monitor; then
                echo "ERROR: Monitor operation failed"
                exit 1
            fi
        fi
        ;;
    flash_monitor)
        echo "Flashing and monitoring $EXAMPLE_TYPE example on $BEST_PORT..."
        echo "Press Ctrl+] to exit monitor after flashing"
        if [[ -n "$LOG_FILE" ]]; then
            echo "Monitor output being logged to: $LOG_FILE"
            if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" flash monitor 2>&1 | tee "$LOG_FILE"; then
                echo "ERROR: Flash and monitor operation failed"
                exit 1
            fi
        else
            if ! idf.py -B "$BUILD_DIR" -p "$BEST_PORT" flash monitor; then
                echo "ERROR: Flash and monitor operation failed"
                exit 1
            fi
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
if [[ -n "$LOG_FILE" && -f "$LOG_FILE" ]]; then
    echo "Monitor Output Log: $LOG_FILE"
    echo "Log File Size: $(du -h "$LOG_FILE" | cut -f1)"
fi
echo ""
echo "Available operations:"
echo "  Flash only:        ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE flash"
echo "  Monitor only:      ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE monitor [true|false]"
echo "  Flash & monitor:   ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE flash_monitor [true|false]"
echo "  Build only:        ./build_example.sh $EXAMPLE_TYPE $BUILD_TYPE"
echo ""
echo "Logging examples:"
echo "  With logging:      ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE monitor true"
echo "  Without logging:   ./flash_example.sh $EXAMPLE_TYPE $BUILD_TYPE monitor false"
echo "======================================================"

