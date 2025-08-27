#!/bin/bash
# ESP32 CI Environment Setup Script
# This script sets up minimal ESP32 development environment for CI/CD builds

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Show help if requested (before sourcing common functions)
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "ESP32 CI Environment Setup Script"
    echo ""
    echo "Usage: ./setup_ci.sh [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "  --help, -h          Show this help message"
    echo ""
    echo "PURPOSE:"
    echo "  Set up minimal ESP32 development environment for CI/CD builds"
    echo ""
    echo "WHAT IT INSTALLS:"
    echo "  • System dependencies (build tools, libraries, minimal packages)"
    echo "  • Clang-20 toolchain (compiler, formatter, analyzer, tools)"
    echo "  • ESP-IDF v5.5 (ESP32 development framework with tools)"
    echo "  • Python dependencies (PyYAML, pip packages)"
    echo "  • yq (YAML processor for configuration parsing)"
    echo "  • CI-optimized environment configuration"
    echo "  • Cache optimization for CI environments"
    echo ""
    echo "WHAT IT DOES:"
    echo "  • Installs minimal required dependencies for CI builds"
    echo "  • Sets up ESP-IDF development framework"
    echo "  • Configures Clang toolchain for CI builds"
    echo "  • Optimizes environment for CI/CD pipelines"
    echo "  • Provides cache optimization and statistics"
    echo "  • Skips installation if components found in cache"
    echo "  • Sets up environment variables for CI builds"
    echo ""
    echo "CI-SPECIFIC FEATURES:"
    echo "  • Cache-aware installation (skips if already present)"
    echo "  • Minimal dependency installation (CI-optimized)"
    echo "  • Cache usage optimization and statistics"
    echo "  • Environment variable configuration for CI"
    echo "  • Build directory management for CI builds"
    echo "  • Automated project building and testing"
    echo ""
    echo "BUILD APPROACH:"
    echo "  • Environment setup: setup_ci.sh (this script)"
    echo "  • Building: build_app.sh (for consistency with local development)"
    echo "  • Environment-based configuration via variables"
    echo "  • Cache optimization for faster CI builds"
    echo ""
    echo "REQUIREMENTS:"
    echo "  • sudo access for package installation"
    echo "  • Internet connection for downloads"
    echo "  • CI environment with caching support"
    echo "  • At least 1GB free disk space"
    echo "  • Supported operating systems: Ubuntu, Fedora, CentOS"
    echo ""
    echo "CI VARIABLES:"
    echo "  • BUILD_PATH: Custom build directory path"
    echo "  • ESP32_PROJECT_PATH: Path to ESP32 project"
    echo "  • IDF_TARGET: Target MCU (default: esp32c6)"
    echo "  • BUILD_TYPE: Build configuration (default: Release)"
    echo "  • APP_TYPE: Application type (default: hardfoc_interface)"
    echo "  • IDF_VERSION: ESP-IDF version (default: release/v5.5)"
    echo ""
    echo "CACHE OPTIMIZATION:"
    echo "  • ESP-IDF git history cleaning (saves ~100-200MB)"
    echo "  • Build file cleanup for cache optimization"
    echo "  • Pip cache management and optimization"
    echo "  • ccache integration for build acceleration"
    echo ""
    echo "ALTERNATIVES:"
    echo "  • For local development setup, use: ./setup_repo.sh"
    echo "  • For shared functions only, use: source ./setup_common.sh"
    echo ""
    echo "TROUBLESHOOTING:"
    echo "  • Check network connectivity to GitHub"
    echo "  • Verify IDF_VERSION format (e.g., release/v5.5)"
    echo "  • Ensure sufficient disk space (>1GB free)"
    echo "  • Check script permissions and sudo access"
    echo ""
    echo "For detailed information, see: docs/README_UTILITY_SCRIPTS.md"
    exit 0
fi

# Source the common setup functions
if ! source "$SCRIPT_DIR/setup_common.sh"; then
    echo "ERROR: Failed to source setup_common.sh"
    echo "This script requires the common setup functions to be available"
    exit 1
fi

# Function to validate and sanitize IDF_VERSION
validate_idf_version() {
    local idf_version="$1"
    
    if [[ -z "$idf_version" ]]; then
        return 0  # Empty is valid (will use default)
    fi
    
    # Check if it's a valid git branch/tag format
    if [[ "$idf_version" =~ ^[a-zA-Z0-9._/-]+$ ]]; then
        echo "Valid IDF_VERSION format: $idf_version"
        return 0
    else
        echo "ERROR: Invalid IDF_VERSION format: $idf_version"
        echo "IDF_VERSION should only contain letters, numbers, dots, underscores, hyphens, and forward slashes"
        return 1
    fi
}

# Function to check network connectivity
check_network() {
    echo "Checking network connectivity..."
    
    # Check if we can reach GitHub
    if ping -c 1 github.com >/dev/null 2>&1; then
        echo "Network connectivity: OK (github.com reachable)"
        return 0
    else
        echo "WARNING: Network connectivity issues detected"
        echo "This may cause ESP-IDF installation to fail"
        return 1
    fi
}

# Main setup function for CI
main() {
    echo "Setting up CI environment..."
    
    # Debug environment variables for CI troubleshooting
    echo "CI Environment Variables:"
    echo "  IDF_VERSION: ${IDF_VERSION:-'not set'}"
    echo "  BUILD_PATH: ${BUILD_PATH:-'not set'}"
    echo "  ESP32_PROJECT_PATH: ${ESP32_PROJECT_PATH:-'not set'}"
    echo "  IDF_TARGET: ${IDF_TARGET:-'not set'}"
    echo "  BUILD_TYPE: ${BUILD_TYPE:-'not set'}"
    echo "  APP_TYPE: ${APP_TYPE:-'not set'}"
    echo ""
    
    # Check network connectivity before proceeding
    check_network
    
    # Install essential components for CI
    install_system_deps
    install_clang_tools
    install_yq
    
    # Cache-aware ESP-IDF installation
    if [[ -d "$HOME/.espressif" && -d "$HOME/esp/esp-idf" ]]; then
        echo "ESP-IDF found in cache, skipping installation"
        echo "ESP-IDF path: $HOME/esp/esp-idf"
        echo "Tools path: $HOME/.espressif"
    else
        echo "ESP-IDF not found in cache, installing..."
        
        # Ensure ESP directory exists and has proper permissions
        local esp_dir="$HOME/esp"
        if ! mkdir -p "$esp_dir"; then
            echo "ERROR: Failed to create ESP directory: $esp_dir"
            exit 1
        fi
        if ! chmod 755 "$esp_dir"; then
            echo "WARNING: Failed to set permissions on ESP directory, continuing anyway..."
        fi
        echo "Ensured ESP directory exists: $esp_dir"
        
        # CI: Use IDF_VERSION from Docker action instead of reading global config
        if [[ -n "$IDF_VERSION" ]]; then
            echo "CI mode: Installing ESP-IDF version from environment: $IDF_VERSION"
            
            # Validate IDF_VERSION before installation
            if ! validate_idf_version "$IDF_VERSION"; then
                echo "ERROR: Invalid IDF_VERSION: $IDF_VERSION"
                echo "Valid format examples: release/v5.5, v5.5.0, master"
                echo "Exiting due to invalid IDF_VERSION"
                exit 1
            fi
            
            # Try to install specific version, fallback to default if it fails
            echo "Attempting to install ESP-IDF version: $IDF_VERSION"
            if install_esp_idf_version "$IDF_VERSION"; then
                echo "ESP-IDF version $IDF_VERSION installed successfully"
            else
                echo "WARNING: Failed to install ESP-IDF version $IDF_VERSION"
                echo "This may be due to network issues, invalid version, or other problems"
                echo "Falling back to default ESP-IDF installation..."
                
                if install_esp_idf; then
                    echo "Default ESP-IDF installation completed successfully"
                else
                    echo "ERROR: Both specific version and default ESP-IDF installation failed"
                    echo "CI environment setup cannot continue"
                    exit 1
                fi
            fi
        else
            echo "Local mode: Installing ESP-IDF from global config"
            install_esp_idf
        fi
    fi
    
    # Source ESP-IDF environment for CI builds
    echo "Setting up ESP-IDF environment for CI builds..."
    if [[ -n "$IDF_VERSION" ]]; then
        # CI mode: Source specific ESP-IDF version
        local esp_dir="$HOME/esp"
        # Use same sanitization logic as install_esp_idf_version
        local sanitized_version=$(echo "$IDF_VERSION" | sed 's/[^a-zA-Z0-9._-]/-/g')
        local idf_dir="$esp_dir/esp-idf-${sanitized_version}"
        
        echo "Looking for ESP-IDF at: $idf_dir"
        echo "Sanitized version: $sanitized_version"
        
        if [[ -f "$idf_dir/export.sh" ]]; then
            if source "$idf_dir/export.sh"; then
                echo "ESP-IDF environment sourced successfully for version: $IDF_VERSION"
            else
                echo "ERROR: Failed to source ESP-IDF environment from $idf_dir/export.sh"
                exit 1
            fi
        else
            echo "ERROR: ESP-IDF export.sh not found at $idf_dir/export.sh"
            echo "Available directories in $esp_dir:"
            ls -la "$esp_dir" 2>/dev/null || echo "Could not list $esp_dir"
            
            # Check if there's a symlink that might point to the right place
            if [[ -L "$esp_dir/esp-idf" ]]; then
                echo "Found esp-idf symlink: $(readlink "$esp_dir/esp-idf")"
                local symlink_target="$esp_dir/esp-idf"
                if [[ -f "$symlink_target/export.sh" ]]; then
                    echo "Symlink target has export.sh, trying to source from there..."
                    if source "$symlink_target/export.sh"; then
                        echo "ESP-IDF environment sourced successfully from symlink"
                    else
                        echo "ERROR: Failed to source ESP-IDF environment from symlink"
                        exit 1
                    fi
                else
                    echo "ERROR: Symlink target also missing export.sh"
                    exit 1
                fi
            else
                echo "ERROR: No esp-idf symlink found"
                exit 1
            fi
        fi
    elif [[ -f "$HOME/.espressif/export.sh" ]]; then
        # Local mode: Source default ESP-IDF environment
        if source "$HOME/.espressif/export.sh"; then
            echo "ESP-IDF environment sourced successfully"
        else
            echo "ERROR: Failed to source default ESP-IDF environment"
            exit 1
        fi
    else
        echo "ERROR: ESP-IDF export.sh not found"
        echo "This suggests that ESP-IDF tools were not properly installed"
        exit 1
    fi
    
    # Verify idf.py is available
    echo "Verifying ESP-IDF tools availability..."
    if command_exists idf.py; then
        if idf.py --version >/dev/null 2>&1; then
            echo "idf.py verified: $(idf.py --version | head -1)"
            echo "idf.py location: $(which idf.py)"
            
            # Additional verification: check if ESP-IDF tools are properly installed
            if [[ -d "$HOME/.espressif/tools" ]]; then
                echo "ESP-IDF tools directory exists"
                local tool_count=$(find "$HOME/.espressif/tools" -type f -name "*.py" 2>/dev/null | wc -l)
                echo "Found $tool_count Python tool files"
            else
                echo "WARNING: ESP-IDF tools directory not found"
            fi
        else
            echo "ERROR: idf.py found but failed to execute"
            echo "This suggests a corrupted or incomplete ESP-IDF installation"
            exit 1
        fi
    else
        echo "ERROR: idf.py not found after sourcing ESP-IDF environment"
        echo "Current PATH: $PATH"
        echo "Available ESP-IDF tools:"
        ls -la "$HOME/.espressif/tools" 2>/dev/null || echo "Could not list ESP-IDF tools"
        
        # Try to find idf.py in other locations
        echo "Searching for idf.py in common locations..."
        find "$HOME" -name "idf.py" 2>/dev/null | head -5 || echo "No idf.py found"
        
        echo "ERROR: ESP-IDF environment setup failed"
        echo "Please check the installation logs above for more details"
        exit 1
    fi
    
    # Cache-aware Python dependencies installation
    if [[ -d "$HOME/.cache/pip" && -d "$HOME/.local/lib" ]]; then
        echo "Python dependencies found in cache, skipping installation"
    else
        echo "Python dependencies not found in cache, installing..."
        echo "Installing Python dependencies..."
        if install_python_deps; then
            echo "Python dependencies installed successfully"
        else
            echo "WARNING: Python dependencies installation failed"
            echo "This may cause build issues, but continuing anyway..."
        fi
    fi
    
    # Verify Python dependencies
    echo "Verifying Python dependencies..."
    if python3 -c "import yaml; print('PyYAML version:', yaml.__version__)" 2>/dev/null; then
        echo "PyYAML verification: OK"
    else
        echo "WARNING: PyYAML verification failed"
        echo "This may cause configuration parsing issues"
    fi
    
    # Optimize cache for CI environment
    echo "Optimizing cache for CI environment..."
    if ci_optimize_cache; then
        echo "Cache optimization completed successfully"
    else
        echo "WARNING: Cache optimization failed"
        echo "This may reduce CI build performance, but continuing anyway..."
    fi
    
    echo "Setting up CI environment..."
    if ci_setup_environment; then
        echo "CI environment setup completed successfully"
    else
        echo "WARNING: CI environment setup failed"
        echo "This may cause build environment issues, but continuing anyway..."
    fi
    
    # Check cache status
    echo "Checking cache status..."
    if ci_check_cache_status; then
        echo "Cache status check completed successfully"
    else
        echo "WARNING: Cache status check failed"
        echo "This may indicate cache corruption, but continuing anyway..."
    fi
    
    # Verify essential tools
    echo "Verifying essential tools..."
    if verify_installation; then
        echo "Tool verification completed successfully"
    else
        echo "WARNING: Tool verification failed"
        echo "Some tools may not be available for builds"
        echo "This may cause build failures, but continuing with setup..."
    fi
    
    # Show cache statistics
    echo ""
    echo "======================================================="
    echo "CACHE STATISTICS SUMMARY"
    echo "======================================================="
    
    echo "Gathering cache statistics..."
    
    if [[ -d "$HOME/.espressif" ]]; then
        local esp_size=$(du -sh "$HOME/.espressif" 2>/dev/null | cut -f1 || echo "unknown")
        echo "ESP-IDF Tools: $esp_size"
    else
        echo "ESP-IDF Tools: not found"
    fi
    
    if [[ -d "$HOME/esp" ]]; then
        local esp_idf_size=$(du -sh "$HOME/esp" 2>/dev/null | cut -f1 || echo "unknown")
        echo "ESP-IDF Source: $esp_idf_size"
    else
        echo "ESP-IDF Source: not found"
    fi
    
    if [[ -d "$HOME/.cache/pip" ]]; then
        local pip_size=$(du -sh "$HOME/.cache/pip" 2>/dev/null | cut -f1 || echo "unknown")
        echo "Python Cache: $pip_size"
    else
        echo "Python Cache: not found"
    fi
    
    if [[ -d "$HOME/.ccache" ]]; then
        local ccache_size=$(du -sh "$HOME/.ccache" 2>/dev/null | cut -f1 || echo "unknown")
        echo "ccache: $ccache_size"
    else
        echo "ccache: not found"
    fi
    
    echo "======================================================="
    echo "CI environment setup complete!"
    
    # Show build capabilities
    echo ""
    echo "======================================================="
    echo "BUILD CAPABILITIES AVAILABLE"
    echo "======================================================="
    echo "Environment variables for builds:"
    echo "  • BUILD_PATH - Build directory path (default: ci_build_path)"
    echo "  • ESP32_PROJECT_PATH - ESP32 project path (default: examples/esp32)"
    echo "  • IDF_TARGET - ESP-IDF target (default: esp32c6)"
    echo "  • BUILD_TYPE - Build type (default: Release)"
    echo "  • APP_TYPE - Application type (default: hardfoc_interface)"
    echo "  • IDF_VERSION - ESP-IDF version (default: release/v5.5)"
    echo ""
    echo "Build approach:"
    echo "  • Environment setup: setup_ci.sh (this script)"
    echo "  • Building: build_app.sh (for consistency with local development)"
    echo "======================================================="
    
    # Final verification that we can actually build
    echo ""
    echo "Final build capability verification..."
    if command_exists idf.py && [[ -n "$IDF_TARGET" ]]; then
        echo "✓ ESP-IDF tools available (idf.py found)"
        echo "✓ Target specified: $IDF_TARGET"
        echo "✓ Ready for builds!"
    else
        echo "⚠ Some build capabilities may be limited"
        if ! command_exists idf.py; then
            echo "  - idf.py not found"
        fi
        if [[ -z "$IDF_TARGET" ]]; then
            echo "  - IDF_TARGET not set"
        fi
    fi
    
    echo ""
    echo "CI environment setup summary:"
    if command_exists idf.py; then
        echo "✓ ESP-IDF environment: READY"
    else
        echo "✗ ESP-IDF environment: FAILED"
    fi
    
    if [[ -n "$IDF_TARGET" ]]; then
        echo "✓ Build target: READY ($IDF_TARGET)"
    else
        echo "✗ Build target: NOT SET"
    fi
    
    if [[ -d "$HOME/.espressif" ]]; then
        echo "✓ ESP-IDF tools: READY"
    else
        echo "✗ ESP-IDF tools: MISSING"
    fi
}

# Run main function with error handling
if main "$@"; then
    echo ""
    echo "======================================================="
    echo "CI ENVIRONMENT SETUP COMPLETED SUCCESSFULLY"
    echo "======================================================="
    exit 0
else
    echo ""
    echo "======================================================="
    echo "CI ENVIRONMENT SETUP FAILED"
    echo "======================================================="
    echo "Please check the error messages above for details"
    echo "Common issues:"
    echo "  • Network connectivity problems"
    echo "  • Invalid IDF_VERSION format"
    echo "  • Insufficient disk space"
    echo "  • Permission issues"
    echo "======================================================="
    exit 1
fi
