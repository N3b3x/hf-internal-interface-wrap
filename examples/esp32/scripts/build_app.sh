#!/bin/bash
# Build script for different ESP32 apps (Bash version)
# Usage: ./build_app.sh [app_type] [build_type]
# 
# App types and build types are loaded from app_config.yml
# Use './build_app.sh list' to see all available apps

set -e  # Exit on any error

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$PROJECT_DIR/scripts/config_loader.sh"

# Usage helper
print_usage() {
	echo "Usage: ./build_app.sh [app_type] [build_type] [idf_version] [--clean|--no-clean] [--use-cache|--no-cache]"
	echo "Examples:"
	echo "  ./build_app.sh                                    # defaults from config"
	echo "  ./build_app.sh list                               # list available apps/build types"
	echo "  ./build_app.sh gpio_test Release                  # specific app and build type"
	echo "  ./build_app.sh adc_test Debug release/v5.5        # specific app, build type, and IDF version"
	echo "  ./build_app.sh gpio_test Release --clean          # with clean build"
	echo "  ./build_app.sh adc_test Debug --no-cache          # without cache"
	echo ""
	echo "ESP-IDF Versions: $(get_idf_versions)"
	echo "Build Types: $(get_build_types)"
}

# Defaults (env overrides allowed); flags below can override these
CLEAN=${CLEAN:-0}
USE_CCACHE=${USE_CCACHE:-1}

# Parse arguments: collect non-flag args as positionals
POSITIONAL_ARGS=()
for arg in "$@"; do
	case "$arg" in
		--clean)
			CLEAN=1
			;;
		--no-clean)
			CLEAN=0
			;;
		--use-cache)
			USE_CCACHE=1
			;;
		--no-cache)
			USE_CCACHE=0
			;;
		-h|--help)
			print_usage
			exit 0
			;;
		*)
			POSITIONAL_ARGS+=("$arg")
			;;
	esac
done

# Configuration derived from positionals or config defaults
APP_TYPE=${POSITIONAL_ARGS[0]:-$CONFIG_DEFAULT_APP}
BUILD_TYPE=${POSITIONAL_ARGS[1]:-$CONFIG_DEFAULT_BUILD_TYPE}
IDF_VERSION=${POSITIONAL_ARGS[2]:-$CONFIG_DEFAULT_IDF_VERSION}  # NEW: ESP-IDF version parameter

# Handle special commands

# Handle special commands
if [ "$APP_TYPE" = "list" ]; then
    echo "=== Available App Types ==="
    echo "Featured apps:"
    for app in $(get_featured_app_types); do
        description=$(get_app_description "$app")
        echo "  $app - $description"
    done
    echo ""
    echo "All apps:"
    for app in $(get_app_types); do
        description=$(get_app_description "$app")
        echo "  $app - $description"
    done
    echo ""
    echo "Build types: $(get_build_types)"
    echo "ESP-IDF versions: $(get_idf_versions)"
    echo ""
    echo "Flags: --clean | --no-clean | --use-cache | --no-cache"
    exit 0
fi

# NEW: Validate ESP-IDF version compatibility with app
if ! validate_app_idf_version "$APP_TYPE" "$IDF_VERSION"; then
    echo "ERROR: App '$APP_TYPE' does not support ESP-IDF version '$IDF_VERSION'"
    echo "Supported versions for '$APP_TYPE': $(get_app_idf_versions "$APP_TYPE")"
    echo "Global ESP-IDF versions: $(get_idf_versions)"
    exit 1
fi

# NEW: Validate build type compatibility with app
if ! validate_app_build_type "$APP_TYPE" "$BUILD_TYPE"; then
    echo "ERROR: App '$APP_TYPE' does not support build type '$BUILD_TYPE'"
    echo "Supported build types for '$APP_TYPE': $(get_app_build_types "$APP_TYPE")"
    echo "Global build types: $(get_build_types)"
    exit 1
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

echo "=== ESP32 HardFOC Interface Wrapper Build System ==="
echo "Project Directory: $PROJECT_DIR"
echo "App Type: $APP_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "ESP-IDF Version: $IDF_VERSION"  # NEW: Show ESP-IDF version
echo "Target: $CONFIG_TARGET"
echo "Build Directory: build_${APP_TYPE}_${BUILD_TYPE}"
echo "======================================================="

# Validate app type
if is_valid_app_type "$APP_TYPE"; then
    echo "Valid app type: $APP_TYPE"
    description=$(get_app_description "$APP_TYPE")
    echo "Description: $description"
else
    echo "ERROR: Invalid app type: $APP_TYPE"
    echo "Available types: $(get_app_types)"
    echo "Use './build_app.sh list' to see all apps with descriptions"
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
BUILD_DIR=$(get_build_directory "$APP_TYPE" "$BUILD_TYPE")
echo "Build directory: $BUILD_DIR"

# Clean previous build only if explicitly requested
if [ "$CLEAN" = "1" ] && [ -d "$BUILD_DIR" ]; then
    echo "CLEAN=1 set: removing previous build directory..."
    rm -rf "$BUILD_DIR"
else
    if [ -d "$BUILD_DIR" ]; then
        echo "Incremental build: preserving existing build directory"
    fi
fi

# Configure and build with proper error handling
echo "Configuring project for $IDF_TARGET..."
if ! idf.py -B "$BUILD_DIR" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" -D APP_TYPE="$APP_TYPE" -D IDF_CCACHE_ENABLE="$USE_CCACHE" reconfigure; then
    echo "ERROR: Configuration failed"
    exit 1
fi

echo "Building project..."
if ! idf.py -B "$BUILD_DIR" build; then
    echo "ERROR: Build failed"
    exit 1
fi

# Get actual binary information using configuration
PROJECT_NAME=$(get_project_name "$APP_TYPE")
BIN_FILE="$BUILD_DIR/$PROJECT_NAME.bin"

echo "======================================================"
echo "BUILD COMPLETED SUCCESSFULLY"
echo "======================================================"
echo "App Type: $APP_TYPE"
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
echo "  Flash and monitor: ./scripts/flash_app.sh flash_monitor $APP_TYPE $BUILD_TYPE"
echo "  Flash only:        ./scripts/flash_app.sh flash $APP_TYPE $BUILD_TYPE"
echo "  Monitor only:      ./scripts/flash_app.sh monitor"
echo "  Size analysis:     idf.py -B $BUILD_DIR size"
echo ""
echo "======================================================"
echo "BUILD SIZE INFORMATION"
echo "======================================================"
# Show size information
if ! idf.py -B "$BUILD_DIR" size; then
    echo "WARNING: Could not display size information"
fi
echo "======================================================"
