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
    echo "For detailed information, see: docs/README_UTILITY_SCRIPTS.md"
    exit 0
fi

# Source the common setup functions
source "$SCRIPT_DIR/setup_common.sh"

# Main setup function for CI
main() {
    echo "Setting up CI environment..."
    
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
        install_esp_idf
    fi
    
    # Source ESP-IDF environment for CI builds
    echo "Setting up ESP-IDF environment for CI builds..."
    if [[ -f "$HOME/.espressif/export.sh" ]]; then
        source "$HOME/.espressif/export.sh"
        echo "ESP-IDF environment sourced successfully"
        
        # Verify idf.py is available
        if command_exists idf.py; then
            echo "idf.py verified: $(idf.py --version | head -1)"
        else
            echo "ERROR: idf.py not found after sourcing ESP-IDF environment"
            exit 1
        fi
    else
        echo "ERROR: ESP-IDF export.sh not found"
        exit 1
    fi
    
    # Cache-aware Python dependencies installation
    if [[ -d "$HOME/.cache/pip" && -d "$HOME/.local/lib" ]]; then
        echo "Python dependencies found in cache, skipping installation"
    else
        echo "Python dependencies not found in cache, installing..."
        install_python_deps
    fi
    
    # Optimize cache for CI environment
    ci_optimize_cache
    
    ci_setup_environment
    
    # Check cache status
    ci_check_cache_status
    
    # Verify essential tools
    verify_installation
    
    # Show cache statistics
    echo ""
    echo "======================================================="
    echo "CACHE STATISTICS SUMMARY"
    echo "======================================================="
    
    if [[ -d "$HOME/.espressif" ]]; then
        local esp_size=$(du -sh "$HOME/.espressif" 2>/dev/null | cut -f1)
        echo "ESP-IDF Tools: $esp_size"
    fi
    
    if [[ -d "$HOME/esp" ]]; then
        local esp_idf_size=$(du -sh "$HOME/esp" 2>/dev/null | cut -f1)
        echo "ESP-IDF Source: $esp_idf_size"
    fi
    
    if [[ -d "$HOME/.cache/pip" ]]; then
        local pip_size=$(du -sh "$HOME/.cache/pip" 2>/dev/null | cut -f1)
        echo "Python Cache: $pip_size"
    fi
    
    if [[ -d "$HOME/.ccache" ]]; then
        local ccache_size=$(du -sh "$HOME/.ccache" 2>/dev/null | cut -f1)
        echo "ccache: $ccache_size"
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
}



# Run main function
main "$@"
