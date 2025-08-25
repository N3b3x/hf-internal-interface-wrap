#!/bin/bash
# CI Environment Setup Script for ESP32 HardFOC Interface Wrapper
# This script sets up a minimal development environment for CI builds

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Show help if requested (before sourcing common functions)
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "ESP32 CI Environment Setup Script"
    echo ""
    echo "Usage: ./setup_ci.sh [--help]"
    echo ""
    echo "Purpose: Set up minimal ESP32 development environment for CI/CD builds"
    echo ""
    echo "What it installs:"
    echo "  • System dependencies (build tools, libraries)"
    echo "  • Clang-20 toolchain (compiler, formatter, analyzer)"
    echo "  • ESP-IDF v5.5 (ESP32 development framework)"
    echo "  • Python dependencies (PyYAML)"
    echo "  • yq (YAML processor)"
    echo "  • Development aliases and environment variables"
    echo ""
    echo "CI-specific features:"
    echo "  • Skips installation if components found in cache"
    echo "  • Optimizes cache usage for CI environments"
    echo "  • Provides detailed cache statistics"
    echo ""
    echo "For local development setup, use: ./setup_repo.sh"
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
    
    # Cache-aware Python dependencies installation
    if [[ -d "$HOME/.cache/pip" && -d "$HOME/.local/lib" ]]; then
        echo "Python dependencies found in cache, skipping installation"
    else
        echo "Python dependencies not found in cache, installing..."
        install_python_deps
    fi
    
    # Optimize cache for CI environment
    optimize_cache_for_ci
    
    setup_ci_environment
    
    # Check cache status
    check_cache_status
    
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
}



# Run main function
main "$@"
