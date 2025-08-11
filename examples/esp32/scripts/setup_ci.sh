#!/bin/bash
# CI Environment Setup Script for ESP32 HardFOC Interface Wrapper
# This script sets up a minimal development environment for CI builds

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source the common setup functions
source "$SCRIPT_DIR/setup_common.sh"

# Main setup function for CI
main() {
    echo "Setting up CI environment..."
    
    # Install essential components for CI
    install_system_deps
    install_clang_tools
    install_yq
    install_esp_idf
    install_python_deps
    setup_ci_environment
    
    # Verify essential tools
    verify_installation
    
    echo "CI environment setup complete!"
}

# Run main function
main "$@"
