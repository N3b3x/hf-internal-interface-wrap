#!/bin/bash
# Build Directory Setup Script for ESP32 CI
# This script handles the creation and setup of the build directory for CI builds
# It follows clean principles by separating build directory setup from the main CI workflow

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
    echo ""
    echo "Example:"
    echo "  ./setup_build_directory.sh -p my_build -t esp32c6 -b Release -a my_app"
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
        echo "Valid build types: ${valid_types[*]}"
        exit 1
    fi
}

# Function to check if required files exist
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    local required_files=(
        "examples/esp32/CMakeLists.txt"
        "examples/esp32/main"
        "examples/esp32/components"
        "examples/esp32/scripts"
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
    cp -r examples/esp32/scripts "$BUILD_PATH/scripts"
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