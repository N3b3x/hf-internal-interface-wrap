#!/bin/bash
# Configuration loader for ESP32 apps
# This script provides functions to load and parse the app_config.yml file

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG_FILE="$PROJECT_DIR/app_config.yml"

# Check if yq is available for YAML parsing and detect version
check_yq() {
    # Always check for yq availability (no caching) - this allows detecting newly installed yq
    if ! command -v yq &> /dev/null; then
        # Only show warning if we haven't shown it in this script execution
        if [[ -z "$YQ_WARNING_SHOWN" ]]; then
            echo "Warning: yq not found. Falling back to basic parsing." >&2
            export YQ_WARNING_SHOWN=1
        fi
        return 1
    fi
    
    # Detect yq version and set appropriate syntax
    local yq_version=$(yq --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+' | head -1)
    if [[ -n "$yq_version" ]]; then
        local major_version=$(echo "$yq_version" | cut -d. -f1)
        if [[ "$major_version" -ge 4 ]]; then
            # yq v4+ uses 'eval' syntax
            export YQ_SYNTAX="eval"
        else
            # yq v3.x uses direct syntax
            export YQ_SYNTAX="direct"
        fi
        # echo "Detected yq version $yq_version, using $YQ_SYNTAX syntax" >&2
    else
        # Fallback to direct syntax for unknown versions
        export YQ_SYNTAX="direct"
        echo "Could not detect yq version, using direct syntax" >&2
    fi
    
    return 0
}

# Helper function to execute yq with appropriate syntax
run_yq() {
    local query="$1"
    local raw_flag="$2"
    
    if [[ "$YQ_SYNTAX" == "eval" ]]; then
        if [[ "$raw_flag" == "-r" ]]; then
            yq eval "$query" "$CONFIG_FILE" -r
        else
            yq eval "$query" "$CONFIG_FILE"
        fi
    else
        if [[ "$raw_flag" == "-r" ]]; then
            yq "$query" "$CONFIG_FILE" -r
        else
            yq "$query" "$CONFIG_FILE"
        fi
    fi
}

# Load configuration using yq (preferred method)
load_config_yq() {
    if ! check_yq; then
        return 1
    fi
    
    # Export configuration as environment variables (raw output, no quotes)
    export CONFIG_DEFAULT_APP=$(run_yq '.metadata.default_app' -r)
    export CONFIG_DEFAULT_BUILD_TYPE=$(run_yq '.metadata.default_build_type' -r)
    export CONFIG_TARGET=$(run_yq '.metadata.target' -r)
    
    return 0
}

# Fallback: Basic parsing without yq
load_config_basic() {
    # Extract basic configuration using grep and sed (cleaner quote handling)
    export CONFIG_DEFAULT_APP=$(grep -A 10 "metadata:" "$CONFIG_FILE" | grep "default_app:" | sed 's/.*default_app: *"*\([^"]*\)"*.*/\1/')
    export CONFIG_DEFAULT_BUILD_TYPE=$(grep -A 10 "metadata:" "$CONFIG_FILE" | grep "default_build_type:" | sed 's/.*default_build_type: *"*\([^"]*\)"*.*/\1/')
    export CONFIG_TARGET=$(grep -A 10 "metadata:" "$CONFIG_FILE" | grep "target:" | sed 's/.*target: *"*\([^"]*\)"*.*/\1/')
    
    return 0
}

# Get list of valid app types
get_app_types() {
    if check_yq; then
        run_yq '.apps | keys | .[]' -r | tr '\n' ' '
    else
        # Fallback: extract from apps section (use more specific range)
        sed -n '/^apps:/,/^build_config:/p' "$CONFIG_FILE" | grep '^  [a-z_]*:$' | sed 's/^  \(.*\):$/\1/' | sort | tr '\n' ' '
    fi
}

# Get list of valid build types
get_build_types() {
    if check_yq; then
        run_yq '.build_config.build_types | keys | .[]' -r | tr '\n' ' '
    else
        # Fallback: assume Debug and Release
        echo "Debug Release "
    fi
}

# Get description for an app type
get_app_description() {
    local app_type="$1"
    if check_yq; then
        run_yq ".apps.${app_type}.description" -r
    else
        # Fallback: extract description using grep (improved regex)
        sed -n "/^  ${app_type}:/,/^  [a-z_]*:/p" "$CONFIG_FILE" | grep "description:" | sed 's/.*description: *["\x27]*\([^"\x27]*\)["\x27]*.*/\1/' | head -1
    fi
}

# Get source file for an app type
get_app_source_file() {
    local app_type="$1"
    if check_yq; then
        run_yq ".apps.${app_type}.source_file" -r
    else
        # Fallback: extract source_file using grep
        sed -n "/^  ${app_type}:/,/^  [a-z_]*:/p" "$CONFIG_FILE" | grep "source_file:" | sed 's/.*source_file: *"\(.*\)".*/\1/'
    fi
}

# Check if app type is valid
is_valid_app_type() {
    local app_type="$1"
    local valid_types=$(get_app_types)
    echo "$valid_types" | grep -q "\b$app_type\b"
}

# Check if build type is valid
is_valid_build_type() {
    local build_type="$1"
    local valid_types=$(get_build_types)
    echo "$valid_types" | grep -q "\b$build_type\b"
}

# Get build directory pattern
get_build_directory() {
    local app_type="$1"
    local build_type="$2"
    if check_yq; then
        local pattern=$(run_yq '.build_config.build_directory_pattern' -r)
        echo "${pattern}" | sed "s/{app_type}/${app_type}/g" | sed "s/{build_type}/${build_type}/g"
    else
        echo "build_${app_type}_${build_type}"
    fi
}

# Get project name pattern
get_project_name() {
    local app_type="$1"
    if check_yq; then
        local pattern=$(run_yq '.build_config.project_name_pattern' -r)
        echo "${pattern}" | sed "s/{app_type}/${app_type}/g"
    else
        echo "esp32_iid_${app_type}_app"
    fi
}

# Get CI-enabled app types
get_ci_app_types() {
    if check_yq; then
        run_yq '.apps | to_entries | map(select(.value.ci_enabled == true)) | .[].key' -r | tr '\n' ' '
    else
        # Fallback: return all app types
        get_app_types
    fi
}

# Get featured app types
get_featured_app_types() {
    if check_yq; then
        run_yq '.apps | to_entries | map(select(.value.featured == true)) | .[].key' -r | tr '\n' ' '
    else
        # Fallback: return default featured apps
        echo "ascii_art gpio_test adc_test pio_test bluetooth_test utils_test "
    fi
}

# Get IDF version from config
get_idf_version() {
    if check_yq; then
        # Get the first IDF version from the array
        run_yq '.metadata.idf_versions[0]' -r
    else
        # Fallback: extract IDF version using grep
        grep -A 5 "metadata:" "$CONFIG_FILE" | grep "idf_versions:" | sed 's/.*idf_versions: *\["*\([^"]*\)"*\].*/\1/' | head -1
    fi
}

# Load configuration
load_config() {
    if ! [ -f "$CONFIG_FILE" ]; then
        echo "Error: Configuration file not found: $CONFIG_FILE" >&2
        return 1
    fi
    
    if load_config_yq; then
        return 0
    else
        load_config_basic
        return 0
    fi
}

# Initialize configuration
init_config() {
    load_config
    
    # Set defaults if not already set
    : ${CONFIG_DEFAULT_APP:="ascii_art"}
    : ${CONFIG_DEFAULT_BUILD_TYPE:="Release"}
    : ${CONFIG_TARGET:="esp32c6"}
}

# Export functions for use in other scripts
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    # Script is being executed directly, not sourced
    echo "ESP32 Apps Configuration Loader"
    echo "Usage: source this script to use configuration functions"
    echo ""
    echo "Available functions:"
    echo "  init_config               - Initialize configuration"
    echo "  get_app_types             - Get all valid app types"
    echo "  get_build_types           - Get all valid build types"
    echo "  get_app_description       - Get description for app type"
    echo "  get_app_source_file       - Get source file for app type"
    echo "  is_valid_app_type         - Check if app type is valid"
    echo "  is_valid_build_type       - Check if build type is valid"
    echo "  get_build_directory       - Get build directory for app/build type"
    echo "  get_project_name          - Get project name for app type"
    echo "  get_ci_app_types          - Get CI-enabled app types"
    echo "  get_featured_app_types    - Get featured app types"
else
    # Script is being sourced, initialize configuration
    init_config
fi

