#!/bin/bash
# Build script for different ESP32 apps (Bash version)
# Usage: ./build_app.sh [app_type] [build_type]
# 
# App types and build types are loaded from app_config.yml
# Use './build_app.sh list' to see all available apps

set -e  # Exit on any error

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
			HELP_REQUESTED=1
			;;
		*)
			POSITIONAL_ARGS+=("$arg")
			;;
	esac
done

# Configuration derived from positionals or config defaults
APP_TYPE=${POSITIONAL_ARGS[0]:-$CONFIG_DEFAULT_APP}
BUILD_TYPE=${POSITIONAL_ARGS[1]:-$CONFIG_DEFAULT_BUILD_TYPE}

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$PROJECT_DIR/scripts/config_loader.sh"

# Usage helper
print_usage() {
    echo "ESP32 HardFOC Interface Wrapper - Build System"
    echo ""
    echo "Usage: ./build_app.sh [COMMAND] [OPTIONS]"
    echo ""
    echo "COMMANDS:"
    echo "  <app_type> <build_type> [idf_version]  - Build specific app with options"
    echo "  list                                    - List all available apps and build types"
    echo "  info <app_name>                         - Show detailed information for specific app"
    echo "  combinations                            - Show all valid build combinations"
    echo "  validate <app> <type> [idf]            - Validate specific build combination"
    echo ""
    echo "OPTIONS:"
    echo "  --clean                                - Clean build (remove existing build directory)"
    echo "  --no-clean                             - Incremental build (preserve existing build directory)"
    echo "  --use-cache                            - Enable ccache for faster builds (default)"
    echo "  --no-cache                             - Disable ccache"
    echo "  -h, --help                             - Show this help message"
    echo ""
    echo "ARGUMENT PATTERNS:"
    echo "  Positional Arguments:"
    echo "    app_type     - Application type (e.g., gpio_test, adc_test)"
    echo "    build_type   - Build configuration (Debug, Release)"
    echo "    idf_version  - ESP-IDF version (e.g., release/v*.*, release/v*.*)"
    echo ""
    echo "  Environment Variables:"
    echo "    CLEAN        - Set to 1 for clean builds, 0 for incremental"
    echo "    USE_CCACHE   - Set to 1 to enable ccache, 0 to disable"
    echo ""
    echo "EXAMPLES:"
    echo "  # Basic usage with defaults"
    echo "  ./build_app.sh                                    # Use config defaults"
    echo "  ./build_app.sh gpio_test Release                  # Specific app and build type"
    echo "  ./build_app.sh adc_test Debug release/v5.5        # Full specification"
    echo ""
    echo "  # Build options"
    echo "  ./build_app.sh gpio_test Release --clean          # Clean build"
    echo "  ./build_app.sh adc_test Debug --no-cache          # Without cache"
    echo "  ./build_app.sh gpio_test Release --no-clean       # Incremental build"
    echo ""
    echo "  # Information commands"
    echo "  ./build_app.sh list                               # List all apps and types"
    echo "  ./build_app.sh info gpio_test                     # App-specific details"
    echo "  ./build_app.sh combinations                       # Valid combinations"
    echo "  ./build_app.sh validate gpio_test Release         # Validate combination"
    echo ""
    echo "  # Environment-based usage"
    echo "  CLEAN=1 ./build_app.sh gpio_test Release          # Clean build via env var"
    echo "  USE_CCACHE=0 ./build_app.sh adc_test Debug       # No cache via env var"
    echo ""
    echo "TROUBLESHOOTING:"
    echo "  # Common Build Issues"
    echo "  • Build fails with 'command not found':"
    echo "    - Run: source scripts/setup_repo.sh"
    echo "    - Or: get_idf (if ESP-IDF is installed)"
    echo "    - Verify: which idf.py"
    echo ""
    echo "  • Configuration errors:"
    echo "    - Check: ./build_app.sh validate <app> <type>"
    echo "    - Verify: cat app_config.yml"
    echo "    - Test: python3 scripts/generate_matrix.py --validate"
    echo ""
    echo "  • ESP-IDF version issues:"
    echo "    - List versions: ./manage_idf.sh list"
    echo "    - Switch version: ./manage_idf.sh switch release/v5.5"
    echo "    - Export version: source <(./manage_idf.sh export release/v5.5)"
    echo ""
    echo "  • Build directory problems:"
    echo "    - Clean build: ./build_app.sh <app> <type> --clean"
    echo "    - Check permissions: ls -la build_*/"
    echo "    - Remove manually: rm -rf build_*/"
    echo ""
    echo "  # Cache and Performance Issues"
    echo "  • Slow builds:"
    echo "    - Enable ccache: ./build_app.sh <app> <type> --use-cache"
    echo "    - Check ccache: ccache -s"
    echo "    - Clear ccache: ccache -C"
    echo ""
    echo "  • Build failures after ESP-IDF update:"
    echo "    - Clean all builds: find . -name 'build_*' -type d -exec rm -rf {} +"
    echo "    - Reinstall ESP-IDF: ./manage_idf.sh install --force"
    echo "    - Verify environment: idf.py --version"
    echo ""
    echo "  # Environment and Path Issues"
    echo "  • PATH not set correctly:"
    echo "    - Source ESP-IDF: source ~/esp/esp-idf/export.sh"
    echo "    - Check variables: echo \$IDF_PATH, echo \$PATH"
    echo "    - Restart terminal after setup"
    echo ""
    echo "  • Permission denied errors:"
    echo "    - Check ownership: ls -la build_*/"
    echo "    - Fix permissions: chmod -R 755 build_*/"
    echo "    - Run as user, not root"
    echo ""
    echo "  # Debugging Commands"
    echo "  • Show build info: ./build_app.sh info <app_name>"
    echo "  • List combinations: ./build_app.sh combinations"
    echo "  • Validate config: ./build_app.sh validate <app> <type>"
    echo "  • Check ESP-IDF: idf.py --version"
    echo "  • Verify target: idf.py set-target esp32c6"
    echo ""
    echo "  # Getting Help"
    echo "  • Script help: ./build_app.sh --help"
    echo "  • ESP-IDF help: idf.py --help"
    echo "  • Configuration help: python3 scripts/generate_matrix.py --help"
    echo "  • Documentation: docs/README_BUILD_SYSTEM.md"
    echo ""
    echo "BUILD TYPES: Debug, Release"
    echo "ESP-IDF VERSIONS: release/v5.5, release/v5.4"
    echo ""
    echo "For detailed information, see: docs/README_BUILD_SYSTEM.md"
}

# Show help if requested (after config loading)
if [ "$HELP_REQUESTED" = "1" ]; then
    print_usage
    exit 0
fi

# Handle special commands first (before validation)
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
    echo ""
    echo "Additional Commands:"
    echo "  info <app_name>        - Show detailed information for specific app"
    echo "  combinations            - Show all valid build combinations"
    echo "  validate <app> <type> [idf] - Validate specific build combination"
    exit 0
fi

# NEW: Show app-specific information
if [ "$APP_TYPE" = "info" ] && [ -n "${POSITIONAL_ARGS[1]}" ]; then
    app_name="${POSITIONAL_ARGS[1]}"
    echo "=== App Information: $app_name ==="
    show_valid_combinations "$app_name"
    exit 0
fi

# NEW: Show all valid combinations
if [ "$APP_TYPE" = "combinations" ]; then
    echo "=== All Valid Build Combinations ==="
    for app in $(get_app_types); do
        echo ""
        show_valid_combinations "$app"
    done
    exit 0
fi

# NEW: Validate specific combination
if [ "$APP_TYPE" = "validate" ] && [ -n "${POSITIONAL_ARGS[1]}" ] && [ -n "${POSITIONAL_ARGS[2]}" ]; then
    app_name="${POSITIONAL_ARGS[1]}"
    build_type="${POSITIONAL_ARGS[2]}"
    idf_version="${POSITIONAL_ARGS[3]:-$(get_idf_version_smart "$app_name" "$build_type")}"
    
    echo "=== Validating Build Combination ==="
    echo "App: $app_name"
    echo "Build Type: $build_type"
    echo "ESP-IDF Version: $idf_version"
    echo ""
    
    if is_valid_combination "$app_name" "$build_type" "$idf_version"; then
        echo "✅ VALID: This combination is allowed!"
    else
        echo "❌ INVALID: This combination is not allowed!"
        echo ""
        echo "Valid combinations for '$app_name':"
        show_valid_combinations "$app_name"
    fi
    exit 0
fi

# Basic validation after command parsing (before smart default selection)
# Validate app type
if ! is_valid_app_type "$APP_TYPE"; then
    echo "ERROR: Invalid app type: $APP_TYPE"
    echo "Available types: $(get_app_types)"
    echo "Use './build_app.sh list' to see all apps with descriptions"
    exit 1
fi

# Smart IDF version selection (before build type validation)
if [ -n "${POSITIONAL_ARGS[2]}" ]; then
    IDF_VERSION="${POSITIONAL_ARGS[2]}"
else
    # Use smart default based on app and build type
    source "$PROJECT_DIR/scripts/config_loader.sh"
    IDF_VERSION=$(get_idf_version_smart "$APP_TYPE" "$BUILD_TYPE")
    echo "No IDF version specified, using smart default: $IDF_VERSION"
fi

# Validate build type against the specific ESP-IDF version
if ! is_valid_build_type "$BUILD_TYPE" "$IDF_VERSION"; then
    echo "ERROR: Invalid build type: $BUILD_TYPE"
    echo "Available types for ESP-IDF $IDF_VERSION: $(get_build_types_for_idf_version "$IDF_VERSION")"
    echo "All available types: $(get_build_types)"
    exit 1
fi

# Ensure ESP-IDF environment is sourced for the specified version
if [ -z "$IDF_PATH" ] || ! command -v idf.py &> /dev/null; then
    echo "ESP-IDF environment not found, attempting to auto-setup version $IDF_VERSION..."
    
    # Source the common setup functions to use export_esp_idf_version
    source "$PROJECT_DIR/scripts/setup_common.sh"
    
    # Try to auto-install and source the required ESP-IDF version
    if export_esp_idf_version "$IDF_VERSION" "true"; then
        echo "ESP-IDF environment sourced successfully for version $IDF_VERSION"
    else
        echo "ERROR: Failed to setup ESP-IDF environment for version $IDF_VERSION"
        echo ""
        echo "Available versions:"
        list_esp_idf_versions
        echo ""
        echo "To manually install required versions, run: ./scripts/setup_repo.sh"
        exit 1
    fi
fi

# Ensure target is set from config
if [[ -z "$CONFIG_TARGET" ]]; then
    # Source config_loader to get target from app_config.yml
    source "$PROJECT_DIR/scripts/config_loader.sh"
    export IDF_TARGET=$(get_target)
    echo "Target set from config: $IDF_TARGET"
else
    export IDF_TARGET="$CONFIG_TARGET"
    echo "Target set from environment: $IDF_TARGET"
fi

echo "=== ESP32 HardFOC Interface Wrapper Build System ==="
echo "Project Directory: $PROJECT_DIR"
echo "App Type: $APP_TYPE"
echo "Build Type: $BUILD_TYPE"
echo "ESP-IDF Version: $IDF_VERSION"  # NEW: Show ESP-IDF version
echo "Target: $CONFIG_TARGET"
echo "Build Directory: $(get_build_directory "$APP_TYPE" "$BUILD_TYPE")"
echo "======================================================="

# Enhanced validation with combination checking
validate_build_combination() {
    local app_type="$1"
    local build_type="$2"
    local idf_version="$3"
    
    echo "Validating build combination: $app_type + $build_type + $idf_version"
    
    # Check if combination is allowed
    if ! is_valid_combination "$app_type" "$build_type" "$idf_version"; then
        echo "❌ ERROR: Invalid build combination!"
        echo ""
        echo "The combination '$app_type + $build_type + $idf_version' is not allowed."
        echo ""
        
        # Show what IS allowed for this app
        echo "✅ Valid combinations for '$app_type':"
        show_valid_combinations "$app_type"
        
        echo ""
        echo "💡 To see all valid combinations:"
        echo "   ./scripts/build_app.sh list"
        echo ""
        echo "💡 To see app-specific options:"
        echo "   ./scripts/build_app.sh info $app_type"
        
        exit 1
    fi
    
    echo "✅ Build combination validated successfully!"
}

# Basic validations moved above - show app info here
echo "Valid app type: $APP_TYPE"
description=$(get_app_description "$APP_TYPE")
echo "Description: $description"
echo "Valid build type: $BUILD_TYPE"

# Validate the complete combination
validate_build_combination "$APP_TYPE" "$BUILD_TYPE" "$IDF_VERSION"

# Switch to project directory
cd "$PROJECT_DIR"

# Set build directory using configuration (includes target and IDF version)
BUILD_DIR=$(get_build_directory "$APP_TYPE" "$BUILD_TYPE" "$IDF_TARGET" "$IDF_VERSION")
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

# Export build directory for CI and other scripts to use
# This allows CI pipelines and other scripts to know where the build artifacts are located
export ESP32_BUILD_APP_MOST_RECENT_DIRECTORY="$BUILD_DIR"
echo "ESP32_BUILD_APP_MOST_RECENT_DIRECTORY=$BUILD_DIR"

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
