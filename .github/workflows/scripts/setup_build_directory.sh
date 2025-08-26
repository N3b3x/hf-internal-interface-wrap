#!/bin/bash
# Build Directory Setup Script for ESP32 CI
# This script handles the creation and setup of the build directory for CI builds
# It follows clean principles by separating build directory setup from the main CI workflow
# NEW: Now intelligently reads app_config.yml to understand app-specific configuration

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default values
BUILD_PATH="ci_project"
IDF_TARGET="esp32c6"
BUILD_TYPE="Release"
APP_TYPE="hardfoc_interface"
IDF_VERSION="release/v5.5"  # NEW: ESP-IDF version parameter

# Function to show help
show_help() {
    echo "ESP32 Build Directory Setup Script"
    echo ""
    echo "Usage: ./setup_build_directory.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --build-path PATH    Build directory path (default: ci_project)"
    echo "  -t, --target TARGET      ESP-IDF target (default: esp32c6)"
    echo "  -b, --build-type TYPE    Build type: Debug, Release, RelWithDebInfo, MinSizeRel (default: Release)"
    echo "  -a, --app-type APP       Application type (default: hardfoc_interface)"
    echo "  -v, --idf-version VER    ESP-IDF version (default: release/v5.5)"
    echo "  -h, --help               Show this help message"
    echo ""
    echo "Purpose: Set up build directory structure for ESP32 CI builds"
    echo ""
    echo "What it does:"
    echo "  • Creates ESP-IDF project structure"
    echo "  • Copies source files and components"
    echo "  • Sets up CMakeLists.txt and configuration files"
    echo "  • Configures build parameters"
    echo "  • Prepares for compilation"
    echo "  • NEW: Reads app_config.yml for app-specific configuration"
    echo ""
    echo "Example:"
    echo "  ./setup_build_directory.sh -p my_build -t esp32c6 -b Release -a my_app -v release/v5.5"
    exit 0
}

# Function to print status messages
print_status() {
    echo "[INFO] $1"
}

print_success() {
    echo "[SUCCESS] $1"
}

print_error() {
    echo "[ERROR] $1"
}

# Function to parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -p|--build-path)
                BUILD_PATH="$2"
                shift 2
                ;;
            -t|--target)
                IDF_TARGET="$2"
                shift 2
                ;;
            -b|--build-type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            -a|--app-type)
                APP_TYPE="$2"
                shift 2
                ;;
            -v|--idf-version)
                IDF_VERSION="$2"
                shift 2
                ;;
            -h|--help)
                show_help
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                ;;
        esac
    done
}

# Function to validate build type
validate_build_type() {
    local valid_types=("Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    local valid=false
    
    for type in "${valid_types[@]}"; do
        if [[ "$BUILD_TYPE" == "$type" ]]; then
            valid=true
            break
        fi
    done
    
    if [[ "$valid" == false ]]; then
        print_error "Invalid build type: $BUILD_TYPE"
        print_error "Valid types: ${valid_types[*]}"
        exit 1
    fi
}

# NEW: Function to read and understand app configuration
read_app_config() {
    local app_config_file="examples/esp32/app_config.yml"
    
    if [[ ! -f "$app_config_file" ]]; then
        print_error "App configuration file not found: $app_config_file"
        exit 1
    fi
    
    print_status "Reading app configuration from $app_config_file..."
    
    # Check if Python and PyYAML are available for parsing
    if command -v python3 >/dev/null 2>&1 && python3 -c "import yaml" >/dev/null 2>&1; then
        # Use Python to parse YAML and extract app-specific configuration
        local app_config=$(python3 -c "
import yaml
import sys

try:
    with open('$app_config_file', 'r') as f:
        config = yaml.safe_load(f)
    
    app_name = '$APP_TYPE'
    if 'apps' in config and app_name in config['apps']:
        app_config = config['apps'][app_name]
        
        # Extract app-specific IDF versions and build types
        app_idf_versions = app_config.get('idf_versions', [])
        app_build_types = app_config.get('build_types', [])
        
        # Check if current IDF version is supported by this app
        current_idf = '$IDF_VERSION'
        if app_idf_versions and current_idf not in app_idf_versions:
            print(f'ERROR: App {app_name} does not support ESP-IDF version {current_idf}')
            print(f'Supported versions: {app_idf_versions}')
            sys.exit(1)
        
        # Check if current build type is supported by this app
        current_build = '$BUILD_TYPE'
        if app_build_types and current_build not in app_build_types:
            print(f'ERROR: App {app_name} does not support build type {current_build}')
            print(f'Supported build types: {app_build_types}')
            sys.exit(1)
        
        # Extract app metadata
        description = app_config.get('description', 'No description')
        category = app_config.get('category', 'Unknown')
        source_file = app_config.get('source_file', 'Unknown')
        
        print(f'App: {app_name}')
        print(f'Description: {description}')
        print(f'Category: {category}')
        print(f'Source File: {source_file}')
        print(f'IDF Versions: {app_idf_versions}')
        print(f'Build Types: {app_build_types}')
        print('CONFIG_VALID')
        
    else:
        print(f'ERROR: App {app_name} not found in configuration')
        print(f'Available apps: {list(config.get(\"apps\", {}).keys())}')
        sys.exit(1)
        
except Exception as e:
    print(f'ERROR: Failed to parse app configuration: {e}')
    sys.exit(1)
")
        
        if [[ $? -eq 0 ]] && [[ "$app_config" == *"CONFIG_VALID"* ]]; then
            print_success "App configuration validated successfully"
            echo "$app_config" | grep -v "CONFIG_VALID"
        else
            print_error "App configuration validation failed"
            echo "$app_config"
            exit 1
        fi
    else
        print_status "Python/PyYAML not available, skipping app configuration validation"
        print_status "Continuing with basic build setup..."
    fi
}

# Function to check if required files exist
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    local required_files=(
        "examples/esp32/CMakeLists.txt"
        "examples/esp32/main"
        "examples/esp32/components"
        "examples/esp32/app_config.yml"
        "examples/esp32/sdkconfig"
        "src"
        "inc"
    )
    
    for file in "${required_files[@]}"; do
        if [[ ! -e "$file" ]]; then
            print_error "Required file/directory not found: $file"
            exit 1
        fi
    done
    
    print_success "All prerequisites found"
}

# Function to create and setup build directory
setup_build_directory() {
    print_status "Setting up build directory: $BUILD_PATH"
    
    # Remove existing build directory if it exists
    if [[ -d "$BUILD_PATH" ]]; then
        print_status "Removing existing build directory: $BUILD_PATH"
        rm -rf "$BUILD_PATH"
    fi
    
    # Create ESP-IDF project
    print_status "Creating ESP-IDF project..."
    idf.py create-project "$BUILD_PATH"
    
    # Copy project files
    print_status "Copying project files..."
    cp examples/esp32/CMakeLists.txt "$BUILD_PATH/CMakeLists.txt"
    rm -rf "$BUILD_PATH/main"
    cp -r examples/esp32/main "$BUILD_PATH/main"
    cp -r examples/esp32/components "$BUILD_PATH/components"
    # Note: Not copying scripts directory to avoid conflicts with CI scripts
    cp examples/esp32/app_config.yml "$BUILD_PATH/app_config.yml"
    cp -r src "$BUILD_PATH/src"
    cp -r inc "$BUILD_PATH/inc"
    cp examples/esp32/sdkconfig "$BUILD_PATH/sdkconfig"
    
    print_success "Build directory setup complete"
}

# Function to configure and build project
configure_and_build() {
    print_status "Configuring and building project..."
    
    # Configure and build
    idf.py -C "$BUILD_PATH" \
        -DIDF_TARGET="$IDF_TARGET" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DAPP_TYPE="$APP_TYPE" \
        --ccache reconfigure build
    
    print_success "Build completed successfully"
}

# Function to generate size reports
generate_size_reports() {
    print_status "Generating size reports..."
    
    # Generate size reports
    idf.py -C "$BUILD_PATH" size-components > "$BUILD_PATH/build/size.txt"
    idf.py -C "$BUILD_PATH" size --format json > "$BUILD_PATH/build/size.json"
    
    # Generate ccache statistics
    ccache -s > "$BUILD_PATH/build/ccache_stats.txt"
    
    print_success "Size reports generated"
}

# Function to display build information
display_build_info() {
    echo ""
    echo "======================================================="
    echo "BUILD DIRECTORY SETUP COMPLETE"
    echo "======================================================="
    echo "Build Path: $BUILD_PATH"
    echo "Target: $IDF_TARGET"
    echo "Build Type: $BUILD_TYPE"
    echo "App Type: $APP_TYPE"
    echo "ESP-IDF Version: $IDF_VERSION"
    echo "======================================================="
    
    if [[ -d "$BUILD_PATH/build" ]]; then
        echo "Build artifacts:"
        ls -la "$BUILD_PATH/build/" | grep -E '\.(bin|elf|map|txt|json)$' || echo "No build artifacts found"
    fi
    
    echo "======================================================="
}

# Main function
main() {
    print_status "Starting ESP32 build directory setup..."
    
    # Parse command line arguments
    parse_args "$@"
    
    # Validate inputs
    validate_build_type
    
    # NEW: Read and validate app configuration
    read_app_config
    
    # Check prerequisites
    check_prerequisites
    
    # Setup build directory
    setup_build_directory
    
    # Configure and build
    configure_and_build
    
    # Generate size reports
    generate_size_reports
    
    # Display build information
    display_build_info
    
    print_success "Build directory setup completed successfully!"
}

# Run main function with all arguments
main "$@"
