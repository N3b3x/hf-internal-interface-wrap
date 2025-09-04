#!/bin/bash
# ESP32 CI Environment Setup Script
# This script sets up ONLY what's needed for CI builds
# ESP-IDF installation and environment is handled by espressif/esp-idf-ci-action@v1

set -e  # Exit on any error

# Set setup mode for plain output (no colors)
export SETUP_MODE="ci"

# Parse --project-path flag first (before other argument parsing)
FILTERED_ARGS=()
i=1
while [[ $i -le $# ]]; do
    arg="${!i}"
    case "$arg" in
        --project-path)
            # Check if next argument exists and is not another flag
            if [[ $((i+1)) -le $# ]] && [[ "${!((i+1))}" != -* ]]; then
                PROJECT_PATH="${!((i+1))}"
                ((i++))  # Skip the next argument since we consumed it
            else
                echo "ERROR: --project-path requires a path argument" >&2
                echo "Usage: --project-path /path/to/project" >&2
                exit 1
            fi
            ;;
        *)
            FILTERED_ARGS+=("$arg")
            ;;
    esac
    ((i++))
done

# Export PROJECT_PATH if it was set via --project-path flag
if [[ -n "$PROJECT_PATH" ]]; then
    export PROJECT_PATH
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Replace $@ with filtered arguments for the rest of the script
set -- "${FILTERED_ARGS[@]}"

# Show help if requested
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "ESP32 CI Environment Setup Script"
    echo ""
    echo "Usage: ./setup_ci.sh [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "  --project-path <path>  Path to project directory (allows scripts to be placed anywhere)"
    echo "  --help, -h          Show this help message"
    echo ""
    echo "PURPOSE:"
    echo "  Set up minimal CI environment for ESP32 builds"
    echo "  ESP-IDF installation is handled by espressif/esp-idf-ci-action@v1"
    echo ""
    echo "REQUIRED ENVIRONMENT VARIABLES:"
    echo "  ESP32_PROJECT_PATH      Path to ESP32 project directory (e.g., 'examples/esp32')"
    echo ""
    echo "OPTIONAL ENVIRONMENT VARIABLES:"
    echo "  BUILD_PATH              CI build directory path (default: 'ci_build_path')"
    echo ""
    echo "WHAT IT DOES:"
    echo "  • Sets up CI build directory structure with all necessary files"
    echo "  • Prepares environment for ESP-IDF CI action"
    echo ""
    echo "WHAT IT DOES NOT DO:"
    echo "  • Install ESP-IDF (handled by ESP-IDF CI action)"
    echo "  • Source ESP-IDF environment (handled by ESP-IDF CI action)"
    echo "  • Set up build tools (handled by ESP-IDF CI action)"
    echo "  • Install code quality tools (clang-format, clang-tidy handled by Docker actions)"
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

# Function to setup CI build directory structure
setup_ci_build_structure() {
    print_status "Setting up CI build directory structure..."
    
    # Get project paths
    if [[ -n "$PROJECT_PATH" ]]; then
        # Use provided project path (can be absolute or relative)
        if [[ "$PROJECT_PATH" = /* ]]; then
            # Absolute path
            local project_dir="$PROJECT_PATH"
        else
            # Relative path - resolve from current working directory
            local project_dir="$(cd "$PROJECT_PATH" && pwd)"
        fi
    else
        # Default behavior: assume scripts are in project/scripts/
        local project_dir="$SCRIPT_DIR/.."
    fi
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
    
    # Verify the setup
    print_status "Verifying CI setup..."
    
    # Check project directory contents
    if verify_ci_project_directory; then
        echo "✓ Project directory verification passed"
        echo ""
        print_info "CI Setup Verification Summary:"
        print_success "All components ready for CI builds"
        return 0
    else
        echo "✗ Project directory verification failed"
        echo ""
        print_info "CI Setup Verification Summary:"
        print_error "Some components missing - CI builds may fail"
        return 1
    fi
}

# Function to verify CI project directory contents
verify_ci_project_directory() {
    echo "Verifying CI project directory contents..."
    
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
            echo "✓ Project directory: $item"
        else
            echo "✗ Project directory: $item: not found"
            structure_ok=false
        fi
    done
    
    return $([ "$structure_ok" = true ] && echo 0 || echo 1)
}

# Main CI setup function
main() {
    echo "Setting up CI environment..."
    echo ""
    
    # Validate required environment variables
    if [[ -z "${ESP32_PROJECT_PATH:-}" ]]; then
        print_error "ESP32_PROJECT_PATH environment variable is required but not set"
        print_error "This should point to the ESP32 project directory (e.g., 'examples/esp32')"
        exit 1
    fi
    
    if [[ ! -d "$ESP32_PROJECT_PATH" ]]; then
        print_error "ESP32_PROJECT_PATH directory does not exist: $ESP32_PROJECT_PATH"
        exit 1
    fi
    
    # Show only relevant environment variables for CI setup
    echo "CI Setup Environment:"
    echo "  BUILD_PATH: ${BUILD_PATH:-ci_build_path}"
    echo "  ESP32_PROJECT_PATH: $ESP32_PROJECT_PATH"
    echo "  PATH: $PATH"
    echo ""
    
    # Setup CI build directory structure and verify
    if setup_ci_build_structure; then
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
