#!/bin/bash
# Local Development Setup Script for ESP32 HardFOC Interface Wrapper
# This script sets up a complete local development environment

set -e  # Exit on any error

# Set setup mode for colored output
export SETUP_MODE="local"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source the common setup functions
source "$SCRIPT_DIR/setup_common.sh"

# Main setup function for local development
main() {
    echo "🚀 ESP32 HardFOC Interface Wrapper - Local Development Setup"
    echo "================================================================"
    echo ""
    
    # Check if running as root
    if [[ $EUID -eq 0 ]]; then
        print_error "This script should not be run as root. Please run as a regular user."
        exit 1
    fi
    
    # Detect OS and inform user
    local os=$(detect_os)
    print_status "Detected operating system: $os"
    
    # Check for sudo access
    if ! sudo -n true 2>/dev/null; then
        print_warning "This script requires sudo access to install packages."
        print_status "You may be prompted for your password during installation."
        echo ""
    fi
    
    # Confirm with user
    echo "This script will install:"
    echo "  • System dependencies (build tools, libraries)"
    echo "  • Clang-20 toolchain (compiler, formatter, analyzer)"
    echo "  • ESP-IDF v5.5 (ESP32 development framework)"
    echo "  • Python dependencies (PyYAML)"
    echo "  • yq (YAML processor)"
    echo "  • Development aliases and environment variables"
    echo ""
    
    read -p "Do you want to continue? (y/N): " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Setup cancelled by user."
        exit 0
    fi
    
    echo ""
    print_status "Starting installation..."
    echo ""
    
    # Install all components
    install_system_deps
    echo ""
    
    install_clang_tools
    echo ""
    
    install_yq
    echo ""
    
    install_esp_idf
    echo ""
    
    install_python_deps
    echo ""
    
    setup_local_environment
    echo ""
    
    # Verify everything is working
    verify_installation
    echo ""
    
    echo "================================================================"
    print_success "🎉 Local development environment setup complete!"
    echo "================================================================"
    echo ""
    print_status "Next steps:"
    print_status "1. Restart your terminal or run: source ~/.bashrc"
    print_status "2. Navigate to the examples/esp32 directory"
    print_status "3. Run: get_idf"
    print_status "4. Build examples with: ./scripts/build_example.sh <example_type> <build_type>"
    print_status "5. List available examples with: ./scripts/build_example.sh list"
    echo ""
    print_status "Useful aliases have been added to your ~/.bashrc:"
    print_status "  • build_example - Build examples from anywhere"
    print_status "  • flash_example - Flash examples to ESP32"
    print_status "  • list_examples - Show all available examples"
    echo ""
    print_status "Happy coding! 🚀"
}

# Run main function
main "$@"
