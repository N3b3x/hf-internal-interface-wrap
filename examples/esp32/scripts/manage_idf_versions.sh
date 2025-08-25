#!/bin/bash
# ESP-IDF Version Management Script
# This script handles switching between different ESP-IDF versions for different apps

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

# Function to check if ESP-IDF version is available
check_idf_version_available() {
    local idf_version="$1"
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ ! -d "$idf_dir" ]]; then
        return 1
    fi
    
    cd "$idf_dir"
    if git rev-parse --verify "$idf_version" >/dev/null 2>&1; then
        cd - > /dev/null
        return 0
    else
        cd - > /dev/null
        return 1
    fi
}

# Function to install/update ESP-IDF version
install_idf_version() {
    local idf_version="$1"
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    print_status "Installing/updating ESP-IDF version: $idf_version"
    
    # Create esp directory if it doesn't exist
    mkdir -p "$esp_dir"
    
    if [[ -d "$idf_dir" ]]; then
        print_status "ESP-IDF already exists, updating to version: $idf_version"
        cd "$idf_dir"
        
        # Fetch all branches and tags
        git fetch --all --tags
        
        # Check if the version exists
        if ! git rev-parse --verify "$idf_version" >/dev/null 2>&1; then
            print_error "ESP-IDF version $idf_version not found in repository"
            print_status "Available versions:"
            git branch -r | grep -v HEAD | sed 's/origin\///' | head -10
            git tag | tail -10
            exit 1
        fi
        
        # Checkout the specific version
        git checkout "$idf_version"
        git pull origin "$idf_version"
        
        # Ensure submodules are synced and updated
        git submodule sync --recursive
        git submodule update --init --recursive
        
        cd - > /dev/null
    else
        print_status "Cloning ESP-IDF version: $idf_version"
        cd "$esp_dir"
        git clone --recursive --branch "$idf_version" https://github.com/espressif/esp-idf.git
        cd esp-idf
        
        # Extra safety to keep submodules in sync
        git submodule sync --recursive
        git submodule update --init --recursive
        
        cd - > /dev/null
    fi
    
    # Install ESP-IDF tools for the target
    print_status "Installing ESP-IDF tools for target: $CONFIG_TARGET"
    cd "$idf_dir"
    ./install.sh "$CONFIG_TARGET"
    cd - > /dev/null
    
    print_success "ESP-IDF version $idf_version installed/updated successfully"
}

# Function to switch to a specific ESP-IDF version
switch_to_idf_version() {
    local idf_version="$1"
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ ! -d "$idf_dir" ]]; then
        print_error "ESP-IDF not found at $idf_dir"
        print_status "Installing ESP-IDF version: $idf_version"
        install_idf_version "$idf_version"
        return
    fi
    
    cd "$idf_dir"
    
    # Check current version
    local current_version=$(git describe --tags --exact-match 2>/dev/null || git rev-parse --abbrev-ref HEAD)
    
    if [[ "$current_version" == "$idf_version" ]]; then
        print_status "Already on ESP-IDF version: $idf_version"
        cd - > /dev/null
        return
    fi
    
    print_status "Switching from ESP-IDF version $current_version to $idf_version"
    
    # Fetch all branches and tags
    git fetch --all --tags
    
    # Check if the target version exists
    if ! git rev-parse --verify "$idf_version" >/dev/null 2>&1; then
        print_error "ESP-IDF version $idf_version not found"
        cd - > /dev/null
        exit 1
    fi
    
    # Checkout the target version
    git checkout "$idf_version"
    git pull origin "$idf_version"
    
    # Ensure submodules are synced
    git submodule sync --recursive
    git submodule update --init --recursive
    
    cd - > /dev/null
    
    print_success "Switched to ESP-IDF version: $idf_version"
}

# Function to source ESP-IDF environment for a specific version
source_idf_environment() {
    local idf_version="$1"
    
    # First, ensure we're on the correct version
    switch_to_idf_version "$idf_version"
    
    # Source the ESP-IDF environment
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ -f "$idf_dir/export.sh" ]]; then
        print_status "Sourcing ESP-IDF environment for version: $idf_version"
        source "$idf_dir/export.sh"
        
        # Verify the environment is loaded
        if [[ -n "$IDF_PATH" ]] && command -v idf.py &> /dev/null; then
            print_success "ESP-IDF environment loaded successfully"
            print_status "IDF_PATH: $IDF_PATH"
            print_status "IDF_TARGET: $IDF_TARGET"
            print_status "idf.py version: $(idf.py --version)"
        else
            print_error "Failed to load ESP-IDF environment"
            exit 1
        fi
    else
        print_error "ESP-IDF export.sh not found at $idf_dir/export.sh"
        exit 1
    fi
}

# Function to get current ESP-IDF version
get_current_idf_version() {
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ -d "$idf_dir" ]]; then
        cd "$idf_dir"
        local current_version=$(git describe --tags --exact-match 2>/dev/null || git rev-parse --abbrev-ref HEAD)
        cd - > /dev/null
        echo "$current_version"
    else
        echo "not_installed"
    fi
}

# Function to list available ESP-IDF versions
list_available_idf_versions() {
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ ! -d "$idf_dir" ]]; then
        print_error "ESP-IDF not installed"
        return 1
    fi
    
    cd "$idf_dir"
    
    print_status "Available ESP-IDF versions:"
    echo ""
    echo "Branches:"
    git branch -r | grep -v HEAD | sed 's/origin\///' | sort
    echo ""
    echo "Tags:"
    git tag | sort -V | tail -20
    echo ""
    echo "Current version: $(get_current_idf_version)"
    
    cd - > /dev/null
}

# Function to clean up old ESP-IDF versions (optional)
cleanup_old_idf_versions() {
    local esp_dir="$HOME/esp"
    local idf_dir="$esp_dir/esp-idf"
    
    if [[ ! -d "$idf_dir" ]]; then
        print_warning "ESP-IDF not installed, nothing to clean"
        return
    fi
    
    print_warning "This will remove old branches and tags to save space"
    print_warning "Current version will be preserved"
    read -p "Continue? [y/N]: " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Cleanup cancelled"
        return
    fi
    
    cd "$idf_dir"
    
    # Remove old branches (keep only current and main/master)
    local current_version=$(get_current_idf_version)
    git branch -r | grep -v HEAD | grep -v "main\|master\|$current_version" | sed 's/origin\///' | xargs -r git branch -D || true
    
    # Remove old tags (keep only recent ones)
    git tag | sort -V | head -n -10 | xargs -r git tag -d || true
    
    # Clean up reflog and garbage collect
    git reflog expire --expire=now --all
    git gc --prune=now --aggressive
    
    cd - > /dev/null
    
    print_success "Cleanup completed"
}

# Main function
main() {
    case "${1:-}" in
        "install")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 install <idf_version>"
                exit 1
            fi
            install_idf_version "$2"
            ;;
        "switch")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 switch <idf_version>"
                exit 1
            fi
            switch_to_idf_version "$2"
            ;;
        "source")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 source <idf_version>"
                exit 1
            fi
            source_idf_environment "$2"
            ;;
        "current")
            local current=$(get_current_idf_version)
            echo "Current ESP-IDF version: $current"
            ;;
        "list")
            list_available_idf_versions
            ;;
        "cleanup")
            cleanup_old_idf_versions
            ;;
        "app")
            if [[ -z "$2" ]]; then
                print_error "Usage: $0 app <app_type>"
                exit 1
            fi
            local app_type="$2"
            local preferred_idf_version=$(get_app_preferred_idf_version "$app_type")
            print_status "App '$app_type' prefers ESP-IDF version: $preferred_idf_version"
            source_idf_environment "$preferred_idf_version"
            ;;
        *)
            echo "ESP-IDF Version Management Script"
            echo ""
            echo "Usage: $0 <command> [options]"
            echo ""
            echo "Commands:"
            echo "  install <version>     - Install/update ESP-IDF version"
            echo "  switch <version>      - Switch to ESP-IDF version"
            echo "  source <version>      - Source ESP-IDF environment for version"
            echo "  app <app_type>        - Source ESP-IDF environment for app's preferred version"
            echo "  current               - Show current ESP-IDF version"
            echo "  list                  - List available ESP-IDF versions"
            echo "  cleanup               - Clean up old versions (interactive)"
            echo ""
            echo "Examples:"
            echo "  $0 install release/v5.5"
            echo "  $0 switch release/v5.4"
            echo "  $0 source release/v5.5"
            echo "  $0 app gpio_test"
            echo "  $0 current"
            echo "  $0 list"
            echo ""
            echo "Note: Use 'source $0 app <app_type>' to load environment in current shell"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"