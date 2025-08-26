#!/bin/bash
# Prepare Build Directory Script for ESP32 CI
# This script only prepares the build directory structure without building
# It follows clean principles by separating directory preparation from building

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default values
BUILD_PATH="ci_project"

# Function to show help
show_help() {
    echo "ESP32 Build Directory Preparation Script"
    echo ""
    echo "Usage: ./prepare_build_directory.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --build-path PATH    Build directory path (default: ci_project)"
    echo "  -h, --help               Show this help message"
    echo ""
    echo "Purpose: Prepare build directory structure for ESP32 CI builds"
    echo ""
    echo "What it does:"
    echo "  • Creates ESP-IDF project structure"
    echo "  • Copies source files and components"
    echo "  • Sets up CMakeLists.txt and configuration files"
    echo "  • Does NOT build the project (use setup_build_directory.sh for that)"
    echo ""
    echo "Example:"
    echo "  ./prepare_build_directory.sh -p my_build"
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

# Function to check if required files exist
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Use ESP32_PROJECT_PATH environment variable if set, otherwise fall back to default
    local esp32_project_path="${ESP32_PROJECT_PATH:-examples/esp32}"
    
    local required_files=(
        "$esp32_project_path/CMakeLists.txt"
        "$esp32_project_path/main"
        "$esp32_project_path/components"
        "$esp32_project_path/app_config.yml"
        "$esp32_project_path/sdkconfig"
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
    # Use ESP32_PROJECT_PATH environment variable if set, otherwise fall back to default
    local esp32_project_path="${ESP32_PROJECT_PATH:-examples/esp32}"
    
    cp "$esp32_project_path/CMakeLists.txt" "$BUILD_PATH/CMakeLists.txt"
    rm -rf "$BUILD_PATH/main"
    cp -r "$esp32_project_path/main" "$BUILD_PATH/main"
    cp -r "$esp32_project_path/components" "$BUILD_PATH/components"
    # Note: Not copying scripts directory to avoid conflicts with CI scripts
    cp "$esp32_project_path/app_config.yml" "$BUILD_PATH/app_config.yml"
    cp -r src "$BUILD_PATH/src"
    cp -r inc "$BUILD_PATH/inc"
    cp "$esp32_project_path/sdkconfig" "$BUILD_PATH/sdkconfig"
    
    print_success "Build directory setup complete"
}

# Function to display preparation information
display_preparation_info() {
    echo ""
    echo "======================================================="
    echo "BUILD DIRECTORY PREPARATION COMPLETE"
    echo "======================================================="
    echo "Build Path: $BUILD_PATH"
    echo "Status: Ready for building"
    echo ""
    echo "Next steps:"
    echo "  • Use 'idf.py -C $BUILD_PATH build' to build"
    echo "  • Or use 'setup_build_directory.sh' for full setup + build"
    echo "======================================================="
}

# Main function
main() {
    print_status "Starting ESP32 build directory preparation..."
    
    # Parse command line arguments
    parse_args "$@"
    
    # Check prerequisites
    check_prerequisites
    
    # Setup build directory
    setup_build_directory
    
    # Display preparation information
    display_preparation_info
    
    print_success "Build directory preparation completed successfully!"
}

# Run main function with all arguments
main "$@"
