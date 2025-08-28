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
    echo "  • Sets up CI build directory structure with all necessary files"
    echo "  • Prepares environment for ESP-IDF CI action"
    echo ""
    echo "WHAT IT DOES NOT DO:"
    echo "  • Install ESP-IDF (handled by ESP-IDF CI action)"
    echo "  • Source ESP-IDF environment (handled by ESP-IDF CI action)"
    echo "  • Set up build tools (handled by ESP-IDF CI action)"
    echo "  • Build the project (handled by ESP-IDF CI action)"
    echo ""
    echo "CI WORKFLOW:"
    echo "  1. This script runs in setup-environment job"
    echo "  2. This script sets up CI build directory structure with all files"
    echo "  3. ESP-IDF CI action handles ESP-IDF setup and builds the project"
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
        # Install to user-writable directory for better caching
        mkdir -p ~/.local/bin
        wget -qO ~/.local/bin/yq https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64
        chmod +x ~/.local/bin/yq
        # Add to PATH for this session
        export PATH="$HOME/.local/bin:$PATH"
        echo "yq installed successfully to ~/.local/bin"
    else
        echo "yq already installed"
    fi
}

# Function to verify CI setup
verify_ci_setup() {
    echo "Verifying CI setup..."
    
    # Ensure user bin directory is in PATH for verification
    export PATH="$HOME/.local/bin:$PATH"
    
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
    
    # Check build structure
    local structure_ok=true
    if verify_ci_build_structure; then
        echo "✓ Build structure verification passed"
    else
        echo "✗ Build structure verification failed"
        structure_ok=false
    fi
    
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
    
    # Copy all necessary project files (following the working pipeline pattern)
    echo "Copying project files to CI build path..."
    
    # Copy ESP32 project files
    cp "$project_dir/CMakeLists.txt" "$ci_build_path/"
    echo "✓ CMakeLists.txt copied"
    
    # Handle main directory (remove existing, copy fresh)
    rm -rf "$ci_build_path/main"
    cp -r "$project_dir/main" "$ci_build_path/"
    echo "✓ main directory copied"
    
    # Copy other project directories
    cp -r "$project_dir/components" "$ci_build_path/"
    echo "✓ components directory copied"
    
    cp -r "$project_dir/scripts" "$ci_build_path/"
    echo "✓ scripts directory copied"
    
    # Copy configuration files
    cp "$project_dir/app_config.yml" "$ci_build_path/"
    echo "✓ app_config.yml copied"
    
    cp "$project_dir/sdkconfig" "$ci_build_path/"
    echo "✓ sdkconfig copied"
    
    # Copy source and include files from workspace root (needed for building)
    local workspace_root="$SCRIPT_DIR/../../.."
    echo "Copying source and include files from workspace root..."
    echo "Workspace root path: $workspace_root"
    echo "Current script directory: $SCRIPT_DIR"
    
    if [[ -d "$workspace_root/src" ]]; then
        cp -r "$workspace_root/src" "$ci_build_path/"
        echo "✓ Source files copied from $workspace_root/src"
    else
        echo "⚠️  Warning: src directory not found at $workspace_root/src"
    fi
    
    if [[ -d "$workspace_root/inc" ]]; then
        cp -r "$workspace_root/inc" "$ci_build_path/"
        echo "✓ Include files copied from $workspace_root/inc"
    else
        echo "⚠️  Warning: inc directory not found at $workspace_root/inc"
    fi
    
    echo "CI build directory structure setup complete"
    echo "Build directory: $ci_build_path"
    ls -la "$ci_build_path"
}

# Function to verify CI build structure
verify_ci_build_structure() {
    echo "Verifying CI build structure..."
    
    local ci_build_path="${BUILD_PATH:-ci_build_path}"
    local structure_ok=true
    
    # Check required files and directories
    local required_items=(
        "CMakeLists.txt"
        "main"
        "components" 
        "scripts"
        "app_config.yml"
        "sdkconfig"
        "src"
        "inc"
    )
    
    for item in "${required_items[@]}"; do
        if [[ -e "$ci_build_path/$item" ]]; then
            echo "✓ Build directory: $item"
        else
            echo "✗ Build directory: $item: not found"
            structure_ok=false
        fi
    done
    
    return $([ "$structure_ok" = true ] && echo 0 || echo 1)
}

# Main CI setup function
main() {
    echo "Setting up CI environment..."
    echo ""
    
    # Ensure user bin directory is in PATH for all operations
    export PATH="$HOME/.local/bin:$PATH"
    
    # Show only relevant environment variables for CI setup
    echo "CI Setup Environment:"
    echo "  BUILD_PATH: ${BUILD_PATH:-ci_build_path}"
    echo "  ESP32_PROJECT_PATH: ${ESP32_PROJECT_PATH:-examples/esp32}"
    echo "  PATH: $PATH"
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
        echo "  2. ESP-IDF CI action will build using the prepared CI environment"
        echo "  3. Builds will use the prepared CI environment with all files copied"
        echo ""
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
