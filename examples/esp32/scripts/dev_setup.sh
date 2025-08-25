#!/bin/bash
# Developer Setup Script
# This script helps developers quickly set up their environment for a specific app

set -e

# Load configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$PROJECT_DIR/scripts/config_loader.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show app information
show_app_info() {
    local app_type="$1"
    
    if ! is_valid_app_type "$app_type"; then
        print_error "Invalid app type: $app_type"
        return 1
    fi
    
    local description=$(get_app_description "$app_type")
    local source_file=$(get_app_source_file "$app_type")
    local preferred_idf_version=$(get_app_preferred_idf_version "$app_type")
    local preferred_build_type=$(get_app_preferred_default_build_type "$app_type")
    local build_types=$(get_app_types | grep -o "\b$app_type\b" | head -1)
    
    echo "=== App Information: $app_type ==="
    echo "Description: $description"
    echo "Source file: $source_file"
    echo "Preferred ESP-IDF version: $preferred_idf_version"
    echo "Preferred default build type: $preferred_build_type"
    echo "Available build types: $(get_build_types)"
    echo "======================================"
}

# Function to setup environment for a specific app
setup_app_environment() {
    local app_type="$1"
    local build_type="${2:-}"
    
    print_status "Setting up environment for app: $app_type"
    
    # Show app information
    show_app_info "$app_type"
    
    # Get app preferences
    local preferred_idf_version=$(get_app_preferred_idf_version "$app_type")
    local preferred_build_type=$(get_app_preferred_default_build_type "$app_type")
    
    # Use provided build type or app's preferred default
    local target_build_type="${build_type:-$preferred_build_type}"
    
    print_status "Target build type: $target_build_type"
    print_status "Target ESP-IDF version: $preferred_idf_version"
    
    # Switch to the correct ESP-IDF version
    print_status "Switching to ESP-IDF version: $preferred_idf_version"
    if ! "$PROJECT_DIR/scripts/manage_idf_versions.sh" switch "$preferred_idf_version"; then
        print_error "Failed to switch to ESP-IDF version: $preferred_idf_version"
        exit 1
    fi
    
    # Source the ESP-IDF environment
    print_status "Sourcing ESP-IDF environment..."
    source "$HOME/esp/esp-idf/export.sh"
    
    # Verify environment
    if [[ -n "$IDF_PATH" ]] && command -v idf.py &> /dev/null; then
        print_success "ESP-IDF environment loaded successfully"
        print_status "IDF_PATH: $IDF_PATH"
        print_status "IDF_TARGET: $IDF_TARGET"
        print_status "idf.py version: $(idf.py --version)"
    else
        print_error "Failed to load ESP-IDF environment"
        exit 1
    fi
    
    # Set target
    export IDF_TARGET="$CONFIG_TARGET"
    print_status "Target set to: $IDF_TARGET"
    
    print_success "Environment setup complete for app: $app_type"
    echo ""
    echo "Next steps:"
    echo "  Build:     ./scripts/build_app.sh $app_type $target_build_type"
    echo "  Flash:     ./scripts/flash_app.sh flash $app_type $target_build_type"
    echo "  Monitor:   ./scripts/flash_app.sh monitor"
    echo "  All:       ./scripts/flash_app.sh flash_monitor $app_type $target_build_type"
    echo ""
    echo "Current ESP-IDF version: $(cd "$HOME/esp/esp-idf" && git describe --tags --exact-match 2>/dev/null || git rev-parse --abbrev-ref HEAD)"
}

# Function to list all apps with their preferences
list_all_apps() {
    echo "=== All Available Apps ==="
    echo ""
    
    for app_type in $(get_app_types); do
        local description=$(get_app_description "$app_type")
        local preferred_idf_version=$(get_app_preferred_idf_version "$app_type")
        local preferred_build_type=$(get_app_preferred_default_build_type "$app_type")
        local featured=$(if [[ "$(get_featured_app_types)" == *"$app_type"* ]]; then echo "⭐"; else echo "  "; fi)
        
        printf "%-20s %s %s\n" "$app_type" "$featured" "$description"
        printf "%-20s   IDF: %s, Default: %s\n" "" "$preferred_idf_version" "$preferred_build_type"
        echo ""
    done
    
    echo "⭐ = Featured app"
    echo "======================================"
}

# Function to show current environment status
show_environment_status() {
    echo "=== Current Environment Status ==="
    
    # Check ESP-IDF installation
    if [[ -d "$HOME/esp/esp-idf" ]]; then
        local current_idf_version=$(cd "$HOME/esp/esp-idf" && git describe --tags --exact-match 2>/dev/null || git rev-parse --abbrev-ref HEAD)
        echo "ESP-IDF: Installed (version: $current_idf_version)"
    else
        echo "ESP-IDF: Not installed"
    fi
    
    # Check if environment is sourced
    if [[ -n "$IDF_PATH" ]] && command -v idf.py &> /dev/null; then
        echo "Environment: Loaded"
        echo "IDF_PATH: $IDF_PATH"
        echo "IDF_TARGET: $IDF_TARGET"
    else
        echo "Environment: Not loaded"
    fi
    
    echo "Target: $CONFIG_TARGET"
    echo "Default app: $CONFIG_DEFAULT_APP"
    echo "Default build type: $CONFIG_DEFAULT_BUILD_TYPE"
    echo "======================================"
}

# Main function
main() {
    case "${1:-}" in
        "setup")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 setup <app_type> [build_type]"
                exit 1
            fi
            setup_app_environment "$2" "${3:-}"
            ;;
        "info")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 info <app_type>"
                exit 1
            fi
            show_app_info "$2"
            ;;
        "list")
            list_all_apps
            ;;
        "status")
            show_environment_status
            ;;
        "idf")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 idf <command> [options]"
                echo "Available commands: install, switch, source, current, list, cleanup, app"
                exit 1
            fi
            # Pass through to manage_idf_versions.sh
            "$PROJECT_DIR/scripts/manage_idf_versions.sh" "${@:2}"
            ;;
        *)
            echo "Developer Setup Script"
            echo ""
            echo "Usage: $0 <command> [options]"
            echo ""
            echo "Commands:"
            echo "  setup <app_type> [build_type]  - Setup environment for specific app"
            echo "  info <app_type>                - Show information about an app"
            echo "  list                           - List all available apps"
            echo "  status                         - Show current environment status"
            echo "  idf <command> [options]       - Manage ESP-IDF versions"
            echo ""
            echo "Examples:"
            echo "  $0 setup gpio_test            - Setup environment for GPIO test app"
            echo "  $0 setup ascii_art Debug      - Setup for ASCII art with Debug build"
            echo "  $0 info pio_test              - Show PIO test app information"
            echo "  $0 list                       - List all apps with preferences"
            echo "  $0 status                     - Show current environment status"
            echo "  $0 idf app gpio_test          - Source ESP-IDF for GPIO test app"
            echo "  $0 idf list                   - List available ESP-IDF versions"
            echo ""
            echo "Quick start:"
            echo "  $0 setup <app_type>           - Setup environment for your app"
            echo "  ./scripts/build_app.sh <app>  - Build your app"
            echo "  ./scripts/flash_app.sh flash <app> - Flash your app"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"