#!/bin/bash
# ESP32 CI Environment Setup Script
# This script sets up ONLY what's needed for CI builds
# ESP-IDF installation and environment is handled by espressif/esp-idf-ci-action@v1

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Show help if requested
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "ESP32 CI Environment Setup Script"
    echo ""
    echo "Usage: ./setup_ci.sh [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "  --help, -h          Show this help message"
    echo ""
    echo "PURPOSE:"
    echo "  Set up minimal CI environment for ESP32 builds"
    echo "  ESP-IDF installation is handled by espressif/esp-idf-ci-action@v1"
    echo ""
    echo "WHAT IT DOES:"
    echo "  • Installs essential build tools (clang-20, clang-format, clang-tidy)"
    echo "  • Installs Python dependencies (PyYAML, yq)"
    echo "  • Sets up CI build directory structure"
    echo "  • Prepares environment for build_app.sh"
    echo ""
    echo "WHAT IT DOES NOT DO:"
    echo "  • Install ESP-IDF (handled by ESP-IDF CI action)"
    echo "  • Source ESP-IDF environment (handled by ESP-IDF CI action)"
    echo "  • Set up build tools (handled by ESP-IDF CI action)"
    echo ""
    echo "CI WORKFLOW:"
    echo "  1. This script runs in setup-environment job"
    echo "  2. ESP-IDF CI action handles ESP-IDF setup in build jobs"
    echo "  3. build_app.sh creates build directories and builds"
    echo ""
    exit 0
fi

# Source the common setup functions for utility functions only
if ! source "$SCRIPT_DIR/setup_common.sh"; then
    echo "ERROR: Failed to source setup_common.sh"
    echo "This script requires the common setup functions to be available"
    exit 1
fi

# Function to install essential build tools for CI
install_ci_build_tools() {
    echo "Installing essential build tools for CI..."
    
    # Add LLVM APT repository for clang-20
    if ! command -v clang-20 &> /dev/null; then
        echo "Installing Clang-20 toolchain..."
        wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | \
            sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
        ubuntu_codename=$(lsb_release -cs)
        echo "deb http://apt.llvm.org/${ubuntu_codename}/ llvm-toolchain-${ubuntu_codename}-20 main" | \
            sudo tee /etc/apt/sources.list.d/llvm.list
        
        sudo apt-get update
        sudo apt-get install -y \
            clang-20 \
            clang-format-20 \
            clang-tidy-20
        
        # Set clang-20 as default
        sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100
        sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-20 100
        sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-20 100
        
        echo "Clang-20 toolchain installed successfully"
    else
        echo "Clang-20 toolchain already installed"
    fi
}

# Function to install Python dependencies for CI
install_ci_python_deps() {
    echo "Installing Python dependencies for CI..."
    
    # Install PyYAML for configuration parsing
    if ! python3 -c "import yaml" 2>/dev/null; then
        echo "Installing PyYAML..."
        pip3 install PyYAML
    else
        echo "PyYAML already installed"
    fi
    
    # Install yq for YAML processing
    if ! command -v yq &> /dev/null; then
        echo "Installing yq..."
        sudo wget -qO /usr/bin/yq https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64
        sudo chmod +x /usr/bin/yq
        echo "yq installed successfully"
    else
        echo "yq already installed"
    fi
}

# Function to setup CI build directory structure
setup_ci_build_structure() {
    echo "Setting up CI build directory structure..."
    
    # Get project paths
    local project_dir="$SCRIPT_DIR/.."
    local ci_build_path="${BUILD_PATH:-ci_build_path}"
    
    echo "Project directory: $project_dir"
    echo "CI build path: $ci_build_path"
    
    # Create CI build directory
    mkdir -p "$ci_build_path"
    
    # Copy the entire project directory (much simpler and ensures nothing is missed)
    echo "Copying entire project directory to CI build path..."
    cp -r "$project_dir"/* "$ci_build_path/"
    echo "✓ Entire project directory copied"
    
    # Copy source and include files from workspace root (needed for building)
    local workspace_root="$SCRIPT_DIR/../.."
    echo "Copying source and include files from workspace root..."
    if [[ -d "$workspace_root/src" ]]; then
        cp -r "$workspace_root/src" "$ci_build_path/"
        echo "✓ Source files copied"
    fi
    if [[ -d "$workspace_root/inc" ]]; then
        cp -r "$workspace_root/inc" "$ci_build_path/"
        echo "✓ Include files copied"
    fi
    
    echo "CI build directory structure setup complete"
    echo "Build directory: $ci_build_path"
    ls -la "$ci_build_path"
}

# Function to verify CI setup
verify_ci_setup() {
    echo "Verifying CI setup..."
    
    local project_dir="$SCRIPT_DIR/.."
    local ci_build_path="${BUILD_PATH:-ci_build_path}"
    
    # Check essential tools
    local tools_ok=true
    for tool in clang-20 clang-format-20 clang-tidy-20 python3 yq; do
        if command -v "$tool" &> /dev/null; then
            echo "✓ $tool: $(command -v "$tool")"
        else
            echo "✗ $tool: not found"
            tools_ok=false
        fi
    done
    
    # Check Python dependencies
    local python_ok=true
    for module in yaml; do
        if python3 -c "import $module" 2>/dev/null; then
            echo "✓ Python module: $module"
        else
            echo "✗ Python module: $module: not found"
            python_ok=false
        fi
    done
    
    # Check build directory structure
    local structure_ok=true
    local required_files=("scripts" "app_config.yml" "src" "inc" "examples")
    for item in "${required_files[@]}"; do
        if [[ -e "$ci_build_path/$item" ]]; then
            echo "✓ Build directory: $item"
        else
            echo "✗ Build directory: $item: not found"
            structure_ok=false
        fi
    done
    
    # Summary
    echo ""
    echo "CI Setup Verification Summary:"
    if $tools_ok && $python_ok && $structure_ok; then
        echo "✅ All components ready for CI builds"
        return 0
    else
        echo "❌ Some components missing - CI builds may fail"
        return 1
    fi
}

# Main CI setup function
main() {
    echo "Setting up CI environment..."
    echo ""
    
    # Debug environment variables
    echo "CI Environment Variables:"
    echo "  BUILD_PATH: ${BUILD_PATH:-'not set (using default: ci_build_path)'}"
    echo "  ESP32_PROJECT_PATH: ${ESP32_PROJECT_PATH:-'not set'}"
    echo "  IDF_TARGET: ${IDF_TARGET:-'not set'}"
    echo "  BUILD_TYPE: ${BUILD_TYPE:-'not set'}"
    echo "  APP_TYPE: ${APP_TYPE:-'not set'}"
    echo "  IDF_VERSION: ${IDF_VERSION:-'not set (handled by ESP-IDF CI action)'}"
    echo ""
    
    # Install essential build tools
    install_ci_build_tools
    
    # Install Python dependencies
    install_ci_python_deps
    
    # Setup CI build directory structure
    setup_ci_build_structure
    
    # Verify setup
    if verify_ci_setup; then
        echo ""
        echo "======================================================="
        echo "CI ENVIRONMENT SETUP COMPLETED SUCCESSFULLY"
        echo "======================================================="
        echo ""
        echo "What happens next:"
        echo "  1. ESP-IDF CI action will handle ESP-IDF installation"
        echo "  2. build_app.sh will create build directories in $ci_build_path"
        echo "  3. Builds will use the prepared CI environment"
        echo ""
        echo "Build directory: ${BUILD_PATH:-ci_build_path}"
        echo "Ready for CI builds!"
    else
        echo ""
        echo "======================================================="
        echo "CI ENVIRONMENT SETUP COMPLETED WITH WARNINGS"
        echo "======================================================="
        echo "Some components may not be available for builds"
        echo "Check the verification output above for details"
    fi
}

# Run main function with error handling
if main "$@"; then
    exit 0
else
    echo ""
    echo "======================================================="
    echo "CI ENVIRONMENT SETUP FAILED"
    echo "======================================================="
    echo "Please check the error messages above for details"
    exit 1
fi
