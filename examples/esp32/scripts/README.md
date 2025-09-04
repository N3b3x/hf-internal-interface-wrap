# 🔧 ESP32 Scripts - Build System Documentation

<div align="center">

![Scripts](https://img.shields.io/badge/Scripts-Build%20System-blue?style=for-the-badge&logo=bash)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-Auto%20Management-green?style=for-the-badge&logo=espressif)
![CI/CD](https://img.shields.io/badge/CI%2FCD-GitHub%20Actions-orange?style=for-the-badge&logo=github)

**🎯 Build System Scripts for ESP32 Development**

*Scripts providing automated ESP-IDF management, build automation, and CI/CD integration*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Script Architecture**](#️-script-architecture)
- [📁 **Script Reference**](#-script-reference)
- [🔧 **Core Scripts**](#-core-scripts)
- [⚙️ **Setup Scripts**](#️-setup-scripts)
- [🔄 **Utility Scripts**](#-utility-scripts)
- [📊 **Configuration Scripts**](#-configuration-scripts)
- [🚀 **Usage Examples**](#-usage-examples)
- [🔍 **Troubleshooting**](#-troubleshooting)
- [🤝 **Contributing**](#-contributing)

---

## 🎯 **Overview**

The scripts directory contains a build system designed for ESP32 development, featuring automated ESP-IDF management, build processes, and CI/CD integration.

### 🏆 **Key Features**

- **🚀 Portable Scripts** - Scripts can be placed anywhere and work with any project via `--project-path`
- **🔧 Automated ESP-IDF Management** - Auto-detection, installation, and environment setup
- **📊 Dynamic Configuration Loading** - YAML-based configuration with hierarchical overrides
- **🔄 Intelligent Build System** - Incremental builds with caching and optimization
- **🛡️ Enhanced Validation System** - Smart combination validation and error prevention
- **🧠 Smart Defaults** - Automatic ESP-IDF version selection based on app and build type
- **🌐 CI/CD Integration** - Seamless GitHub Actions integration
- **📁 Structured Output** - Parseable build directories and artifact management
- **🔍 Comprehensive Logging** - Detailed build logs and error reporting
- **🆕 Environment Separation** - Clear separation between local development and CI environments
- **🚀 CI Pipeline Optimization** - optimized CI pipeline with parallel execution, smart caching, and reliable tool installation

---

## 🚀 **Portable Scripts**

All scripts in this directory are designed to be **completely portable** and can be placed anywhere on your system while still working with any ESP32 project.

### **Key Features**
- **`--project-path` Flag**: All scripts support specifying the project directory
- **Dynamic Script Detection**: Scripts automatically detect their own location
- **Flexible Configuration**: Works with absolute or relative project paths
- **Environment Variables**: Support for `PROJECT_PATH` environment variable
- **Error Handling**: Clear error messages when project or config files are not found

### **Usage Examples**

```bash
# Default behavior (scripts in project/scripts/)
./build_app.sh gpio_test Release

# Portable usage with --project-path
./build_app.sh --project-path /path/to/project gpio_test Release
./flash_app.sh --project-path ../project flash_monitor adc_test
./manage_idf.sh --project-path /opt/esp32-project list

# Environment variable usage
export PROJECT_PATH=/path/to/project
./build_app.sh gpio_test Release
./flash_app.sh flash_monitor adc_test

# Python scripts
python3 get_app_info.py list --project-path /path/to/project
python3 generate_matrix.py --project-path /path/to/project
```

### **Portability Scenarios**

```bash
# Scenario 1: Multiple ESP32 projects
./scripts/build_app.sh --project-path ~/projects/robot-controller gpio_test Release
./scripts/build_app.sh --project-path ~/projects/sensor-node adc_test Debug

# Scenario 2: Shared build tools
# Place scripts in /opt/esp32-tools/
/opt/esp32-tools/build_app.sh --project-path ~/my-project gpio_test Release

# Scenario 3: Renamed script directories
mv scripts tools
./tools/build_app.sh gpio_test Release  # Still works!

# Scenario 4: CI/CD flexibility
./ci-scripts/build_app.sh --project-path $GITHUB_WORKSPACE/examples/esp32 gpio_test Release
```

---

## 🏗️ **Script Architecture**

### **Script Dependencies**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           📁 CONFIGURATION LAYER                            │
├─────────────────────────────────────────────────────────────────────────────┤
│  app_config.yml   ──┐                                                       │
│                     │                                                       │
│  generate_matrix.py │                                                       │
│                     │                                                       │
│  config_loader.sh ──┘                                                       │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             🔧 SETUP LAYER                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  setup_common.sh   ──┐                                                      │
│                      │                                                      │
│  setup_ci.sh         │                                                      │
│                      │                                                      │
│  setup_repo.sh     ──┘                                                      │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                            🔄 CI/CD LAYER                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│  GitHub Actions   ────┐                                                     │
│                       │                                                     │
│  Matrix Generation    │                                                     │
│                       │                                                     │
│  Parallel Execution   │                                                     │
│                       │                                                     │
│  Smart Caching        │                                                     │
│                       │                                                     │
│  Artifact Upload    ──┘                                                     │
└─────────────────────────────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             🚀 BUILD LAYER                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  build_app.sh  ──────┐                                                      │
│                      │                                                      │
│  flash_app.sh        │                                                      │
│                      │                                                      │
│  manage_idf.sh     ──┘                                                      │
└─────────────────────┬───────────────────────────────────────────────────────┘

Data Flow:
app_config.yml → config_loader.sh → build_app.sh
setup_common.sh → setup_ci.sh/setup_repo.sh → build_app.sh
  -local: build_app.sh → flash_app.sh
  -ci: GitHub Actions → Matrix Generation →build_app.sh → Artifact Upload
```

### **🆕 New Environment Setup Architecture**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SETUP COMMON FUNCTIONS                            │
│                    (setup_common.sh - shared utilities)                     │
├─────────────────────────────────────────────────────────────────────────────┤
│  • System dependency installation                                           │
│  • Clang toolchain setup                                                    │
│  • ESP-IDF installation and management                                      │
│  • Python dependency management                                             │
│  • Cross-platform compatibility functions                                   │
│  • Cache optimization and management                                        │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ENVIRONMENT-SPECIFIC SETUP                               │
├─────────────────────────────────────────────────────────────────────────────┤
│  🏠 LOCAL DEVELOPMENT (setup_repo.sh)        🏭 CI/CD (setup_ci.sh)         │
│  • Full development environment              • Minimal CI dependencies      │
│  • Interactive user setup                    • Non-interactive operation    │
│  • Complete tool installation                • Cache-aware installation     │
│  • Environment variables setup               • Build directory preparation  │
│  • Development aliases                       • CI-specific optimizations    │
│  • ESP-IDF auto-installation                • ESP-IDF handled by CI action  │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           BUILD SYSTEM INTEGRATION                          │
├─────────────────────────────────────────────────────────────────────────────┤
│  • build_app.sh uses environment from setup                                 │
│  • flash_app.sh integrates with setup                                       │
│  • CI workflows use setup_ci.sh for environment                             │
│  • Local development uses setup_repo.sh for environment                     │
└─────────────────────────────────────────────────────────────────────────────┘
```

### **Script Categories**

1. **Configuration Scripts** - Load and parse configuration files
2. **Setup Scripts** - Environment setup with clear separation of concerns
3. **Build Scripts** - Application building and management
4. **Utility Scripts** - Helper functions and tools
5. **CI/CD Scripts** - Automation and integration

---

## 🛡️ **Enhanced Validation System**

The build system now includes a comprehensive validation system that prevents invalid build combinations and provides clear guidance to users.

### **Validation Features**

- **🔍 Combination Validation** - Validates app + build type + IDF version combinations
- **🚫 Invalid Build Prevention** - Blocks builds with unsupported combinations
- **💡 Smart Error Messages** - Clear guidance on what combinations are allowed
- **🧠 Smart Defaults** - Automatic ESP-IDF version selection when not specified

### **Validation Flow**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           BUILD REQUEST                                     │
│  app: gpio_test, build_type: Release, idf_version: (unspecified)            │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        BASIC VALIDATION FIRST                               │
│  • Validate app type exists                                                 │
│  • Validate build type is supported                                         │
│  • Fail fast if basic validation fails                                      │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        SMART DEFAULT SELECTION                              │
│  • Only if basic validation passes                                          │
│  • Check app-specific IDF versions                                          │
│  • Find first version supporting requested build type                       │
│  • Fallback to global defaults if needed                                    │
│  • Result: release/v5.5                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        FINAL COMBINATION VALIDATION                         │  
│  • Single comprehensive check (no redundant individual validations)         │
│  • Functions remain standalone-safe for independent sourcing                │
│  • Check combination constraints                                            │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           VALIDATION RESULT                                 │
│  ✅ VALID: gpio_test + Release + release/v5.5                               │
│  → Proceed with build                                                       │
│                                                                             │
│  ❌ INVALID: gpio_test + Release + release/v5.4                             │
│  → Show error with valid combinations                                       │
│  → Provide helpful next steps                                               │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Key Optimization Points:**
- **Early Exit**: Basic validation happens first, failing fast on invalid inputs
- **Smart Defaults**: IDF version selection only occurs after basic validation passes
- **Function Safety**: Individual validation functions remain standalone-safe for independent sourcing
- **No Redundancy**: Combination validation doesn't repeat basic checks already performed

---

## 📁 **Script Reference**

### **File Structure**

```
scripts/
├── 📄 app_config.yml           # Centralized configuration
├── 📄 generate_matrix.py       # CI matrix generator
├── 📄 config_loader.sh         # Configuration utilities
├── 📄 setup_common.sh          # Shared setup functions for all environments
├── 📄 setup_ci.sh              # CI environment setup (minimal dependencies)
├── 📄 setup_repo.sh            # Local development setup (full environment)
├── 📄 build_app.sh             # Main build script
├── 📄 flash_app.sh             # Flashing and monitoring
└── 📄 README.md                # This documentation
```

### **Script Dependencies**

| Script | Dependencies | Purpose | Used By |
|--------|--------------|---------|---------|
| `app_config.yml` | None | Configuration source | All scripts |
| `generate_matrix.py` | `pyyaml` | CI matrix generation | CI/CD |
| `config_loader.sh` | `yq`, `sed` | Configuration parsing | Build scripts |
| `setup_common.sh` | System tools | Shared functions for all environments | Setup scripts |
| `setup_ci.sh` | `setup_common.sh` | CI environment (minimal dependencies) | CI/CD |
| `setup_repo.sh` | `setup_common.sh` | Local development (full environment) | Development |
| `build_app.sh` | `config_loader.sh` | Application building | All users |
| `flash_app.sh` | `config_loader.sh` | Device flashing | Development |

---

## 🔧 **Core Scripts**

### **`build_app.sh` - Main Build Script**

The primary build script that orchestrates the entire build process.

#### **Functionality**
- **ESP-IDF Management** - Auto-detection, installation, and environment setup
- **Configuration Loading** - Loads app configuration and build parameters
- **Enhanced Validation** - Validates build combinations before proceeding
- **Smart Defaults** - Automatic ESP-IDF version selection when not specified
- **Build Execution** - Runs ESP-IDF build with project-specific settings
- **Output Management** - Creates structured build directories and exports paths
- **Error Handling** - Comprehensive error checking and reporting

#### **Usage**
```bash
./scripts/build_app.sh [OPTIONS] <app_name> <build_type> [idf_version]

Options:
  -c, --clean          Clean build (remove existing build directory)
  -v, --verbose        Verbose output
  -h, --help           Show this help message

Arguments:
  app_name             Application name from app_config.yml
  build_type           Build type (Debug, Release)
  idf_version          ESP-IDF version (optional, uses default if not specified)

Examples:
  ./scripts/build_app.sh gpio_test Release
  ./scripts/build_app.sh adc_test Debug release/v5.4
  ./scripts/build_app.sh --clean wifi_test Release

New Enhanced Commands:
  ./scripts/build_app.sh info <app_name>        # Show app information
  ./scripts/build_app.sh combinations            # Show all valid combinations
  ./scripts/build_app.sh validate <app> <type> [idf]  # Validate combination
```

#### **Environment Variables**
```bash
# Exported variables
export ESP32_BUILD_APP_MOST_RECENT_DIRECTORY="$BUILD_DIR"

# Required environment variables
export IDF_TARGET="esp32c6"
export BUILD_TYPE="Release"
export APP_TYPE="gpio_test"
export IDF_VERSION="release/v5.5"
```

#### **Build Process**
1. **Configuration Loading** - Parse `app_config.yml` for build parameters
2. **ESP-IDF Setup** - Auto-detect or install required ESP-IDF version
3. **Environment Preparation** - Source ESP-IDF and set build environment
4. **Build Execution** - Run `idf.py` with project-specific settings
5. **Output Generation** - Create structured build directory
6. **Path Export** - Export build directory path for CI/CD integration

### **`flash_app.sh` - Flashing and Monitoring**

Handles device flashing, monitoring, and related operations.

#### **Functionality**
- **Device Flashing** - Flash firmware to ESP32 devices
- **Serial Monitoring** - Monitor device output and logs
- **Port Detection** - Auto-detect and validate serial ports
- **Error Handling** - Comprehensive error checking and recovery

#### **Usage**
```bash
./scripts/flash_app.sh <action> [app_name] [build_type]

Actions:
  flash                Flash firmware only
  monitor              Monitor serial output only
  flash_monitor        Flash and then monitor
  flash_erase          Erase flash and flash firmware

Examples:
  ./scripts/flash_app.sh flash_monitor gpio_test Release
  ./scripts/flash_app.sh monitor
  ./scripts/flash_app.sh flash_erase adc_test Debug
```

---

## ⚙️ **Setup Scripts**

### **`setup_common.sh` - Shared Setup Functions**

Contains common functions used by all setup scripts.

#### **Key Functions**
```bash
# ESP-IDF Management
install_esp_idf()           # Install ESP-IDF version
setup_esp_idf_env()         # Setup ESP-IDF environment
get_esp_idf_path()          # Get ESP-IDF installation path

# Development Tools
install_dev_tools()          # Install development tools
install_clang_tools()        # Install Clang toolchain
verify_tools()               # Verify tool installation

# System Utilities
check_system_requirements()  # Check system compatibility
setup_environment()          # Setup common environment
```

#### **ESP-IDF Management**
```bash
# Auto-installation process
install_esp_idf() {
    local version="$1"
    local install_dir="$HOME/esp/esp-idf-${version//\//_}"
    
    # Clone ESP-IDF repository
    git clone --recursive https://github.com/espressif/esp-idf.git "$install_dir"
    
    # Install tools and dependencies
    cd "$install_dir"
    ./install.sh
    
    # Setup environment
    source export.sh
}
```

### **`setup_repo.sh` - Local Development Setup**

Sets up the local development environment.

#### **Functionality**
- **Development Tools** - Install Clang, formatting tools, and analysis tools
- **ESP-IDF Setup** - Configure ESP-IDF environment for local development
- **Environment Variables** - Set development-specific environment variables
- **Tool Verification** - Verify all tools are properly installed
- **Complete Development Environment** - Full toolchain and user guidance

#### **Usage**
```bash
# Setup local development environment
source scripts/setup_repo.sh

# This will:
# 1. Install development tools (clang, clang-format, clang-tidy)
# 2. Setup ESP-IDF environment
# 3. Configure build tools
# 4. Export necessary environment variables
# 5. Provide complete development environment
```

#### **Installed Tools**
- **Clang Toolchain** - C/C++ compiler and tools
- **Clang-Format** - Code formatting tool
- **Clang-Tidy** - Static analysis tool
- **ESP-IDF Tools** - ESP32 development tools
- **Build Tools** - CMake, Ninja, and related tools

### **`setup_ci.sh` - 🆕 CI/CD Environment Setup**

Sets up the CI/CD environment for automated builds.

#### **Functionality**
- **CI Tools** - Install CI-specific minimal tools and dependencies
- **ESP-IDF Integration** - ESP-IDF handled by ESP-IDF CI action
- **Environment Variables** - Set CI-specific environment variables
- **Tool Verification** - Verify CI environment is properly configured
- **Build Directory Preparation** - Prepare build directory structure
- **Cache Optimization** - CI-specific cache management

#### **Usage**
```bash
# Setup CI environment
source scripts/setup_ci.sh

# This will:
# 1. Install minimal CI dependencies (clang-20, yq, PyYAML)
# 2. Prepare build directory structure
# 3. Configure for automated builds
# 4. Export CI-specific variables
# 5. Optimize for CI environment
```

#### **CI Environment Features**
- **Minimal Dependencies** - Only essential tools for CI builds
- **Automated Setup** - No user interaction required
- **Error Handling** - Comprehensive error checking and reporting
- **Logging** - Detailed setup logs for debugging
- **Cache Optimization** - CI-specific cache management
- **Build Directory Preparation** - Ready for CI builds

---

## 🔄 **Utility Scripts**

### **`config_loader.sh` - Configuration Utilities**

Provides functions for loading and parsing configuration files.

#### **Key Functions**
```bash
# Configuration Loading
load_config()                # Load app_config.yml
get_build_types()            # Get available build types
get_idf_versions()           # Get available IDF versions
get_target()                 # Get target MCU
get_idf_version()            # Get IDF version

# Build Directory Management
get_build_directory()        # Generate build directory name
parse_build_directory()      # Parse build directory components
get_build_component()        # Extract specific component
is_valid_build_directory()   # Validate directory format
list_build_directories()     # List all build directories

# Application Management
get_app_types()              # Get available application types
get_app_description()        # Get application description
is_valid_app_type()          # Validate application type
is_valid_build_type()        # Validate build type
get_project_name()           # Get project name

# 🆕 Enhanced Validation Functions
is_valid_combination()       # Validate app + build type + IDF version combination
get_version_index()          # Get version index for nested array access
get_app_idf_versions()       # Get app-specific IDF versions
get_app_build_types()        # Get app-specific build types
show_valid_combinations()    # Show valid combinations for specific app
get_idf_version_smart()      # Smart IDF version selection with build type matching
```

#### **Configuration Parsing**
```bash
# Load configuration with fallback paths
load_config() {
    local possible_paths=(
        "examples/esp32/app_config.yml"
        "app_config.yml"
        "../app_config.yml"
        "../../examples/esp32/app_config.yml"
    )
    
    for path in "${possible_paths[@]}"; do
        if [[ -f "$path" ]]; then
            echo "$path"
            return 0
        fi
    done
    
    return 1
}
```

#### **Build Directory Generation**
```bash
# Generate structured build directory name
get_build_directory() {
    local app_type="$1"
    local build_type="$2"
    local target="${3:-esp32c6}"
    local idf_version="${4:-release/v5.5}"
    
    # Sanitize IDF version for file system compatibility
    local sanitized_idf_version=$(echo "$idf_version" | sed 's/[^a-zA-Z0-9._-]/_/g')
    
    # Generate directory name
    echo "build-app-${app_type}-type-${build_type}-target-${target}-idf-${sanitized_idf_version}"
}
```

#### **Directory Parsing**
```bash
# Parse build directory components
parse_build_directory() {
    local build_dir="$1"
    
    # Extract components using regex
    local app_type=$(echo "$build_dir" | sed -n 's/.*app-\(.*\)-type.*/\1/p')
    local build_type=$(echo "$build_dir" | sed -n 's/.*type-\([^-]*\)-target.*/\1/p')
    local target=$(echo "$build_dir" | sed -n 's/.*target-\([^-]*\)-idf.*/\1/p')
    local idf_version=$(echo "$build_dir" | sed -n 's/.*idf-\([^-]*\)$/\1/p')
    
    echo "app_type:$app_type"
    echo "build_type:$build_type"
    echo "target:$target"
    echo "idf_version:$idf_version"
}
```

#### **🆕 Enhanced Validation System**
```bash
# Validate complete build combination
is_valid_combination() {
    local app_type="$1"
    local build_type="$2"
    local idf_version="$3"
    
    # Check if app type is valid
    if ! is_valid_app_type "$app_type"; then
        return 1
    fi
    
    # Check if build type is valid
    if ! is_valid_build_type "$build_type"; then
        return 1
    fi
    
    # Check if app supports this IDF version
    local app_idf_versions=$(get_app_idf_versions "$app_type")
    if ! echo "$app_idf_versions" | grep -q "$idf_version"; then
        return 1
    fi
    
    # Check if app supports this build type for this IDF version
    local app_build_types=$(get_app_build_types "$app_type")
    local clean_build_types=$(echo "$app_build_types" | sed 's/\[//g' | sed 's/\]//g' | sed 's/"//g' | tr ',' ' ')
    
    if [[ "$clean_build_types" == *"$build_type"* ]]; then
        return 0
    fi
    
    return 1
}

# Smart IDF version selection
get_idf_version_smart() {
    local app_type="$1"
    local build_type="$2"
    
    # Check for app-specific override first
    if check_yq; then
        local app_idf_versions=$(run_yq ".apps.${app_type}.idf_versions" -r 2>/dev/null)
        if [ "$app_idf_versions" != "null" ] && [ -n "$app_idf_versions" ]; then
            # Extract first version from array and clean it
            local first_version=$(echo "$app_idf_versions" | sed 's/\[//g' | sed 's/\]//g' | sed 's/"//g' | tr ',' '\n' | head -n1 | tr -d ' ')
            
            # For now, just return the first version
            # TODO: Implement build type matching logic
            echo "$first_version"
            return 0
        fi
    fi
    
    # Fallback to global default
    if check_yq; then
        local global_idf_versions=$(run_yq '.metadata.idf_versions' -r)
        if [ "$global_idf_versions" != "null" ] && [ -n "$global_idf_versions" ]; then
            local first_version=$(echo "$global_idf_versions" | sed 's/\[//g' | sed 's/\]//g' | sed 's/"//g' | tr ',' '\n' | head -n1 | tr -d ' ')
            echo "$first_version"
            return 0
        fi
    fi
    
    # Final fallback
    echo "release/v5.5"
}
```

---

## 📊 **Configuration Scripts**

### **`generate_matrix.py` - CI Matrix Generator**

Python script that generates CI/CD build matrices from centralized configuration.

#### **Functionality**
- **Configuration Loading** - Load and parse `app_config.yml` from multiple possible locations
- **Matrix Generation** - Generate GitHub Actions matrix configuration with hierarchical overrides
- **Configuration Validation** - Validate configuration structure and content before processing
- **Flexible Output** - Multiple output formats (JSON, YAML) and destinations (stdout, file)
- **App Filtering** - Filter matrix output for specific applications
- **Verbose Processing** - Detailed processing information and statistics

#### **Command Line Options**
```bash
--help, -h                  # Show comprehensive help message
--output <file>             # Output to file instead of stdout
--format <format>           # Output format: json, yaml (default: json)
--filter <app>              # Filter output for specific app only
--verbose                   # Show detailed processing information
--validate                  # Validate configuration before generating matrix
```

#### **Usage Examples**
```bash
# Basic usage (output to stdout)
python3 scripts/generate_matrix.py

# YAML format output
python3 scripts/generate_matrix.py --format yaml

# Filter for specific app
python3 scripts/generate_matrix.py --filter gpio_test

# Validate configuration
python3 scripts/generate_matrix.py --validate

# Verbose output with validation
python3 scripts/generate_matrix.py --verbose --validate

# Output to file
python3 scripts/generate_matrix.py --output matrix.json

# Complex combination
python3 scripts/generate_matrix.py --filter wifi_test --validate --verbose --format yaml --output wifi_matrix.yaml
```

#### **Output Structure**

**Standard Matrix Output**
```json
{
  "include": [
    {
      "idf_version": "release/v5.5",
      "idf_version_docker": "release-v5.5",
      "build_type": "Debug",
      "app_name": "gpio_test",
      "target": "esp32c6",
      "config_source": "app"
    }
  ]
}
```

**Matrix Entry Fields**
- `idf_version`: Git format for ESP-IDF cloning (e.g., "release/v5.5")
- `idf_version_docker`: Docker-safe format for artifacts (e.g., "release-v5.5")
- `build_type`: Build configuration ("Debug" or "Release")
- `app_name`: Application identifier
- `target`: Target MCU architecture (e.g., "esp32c6")
- `config_source`: Configuration source ("app" for per-app overrides, "global" for defaults)

#### **Configuration Validation**

The script validates:
- **Required Sections**: `metadata` and `apps` sections must exist
- **Data Types**: All configuration values must have correct types
- **Structure**: Nested arrays and object structures are validated
- **Content**: App descriptions, source files, and dependencies are checked

**Validation Output**
```bash
✅ Configuration validation passed
⚠️  3 warnings found

Configuration warnings:
  ⚠️  App 'gpio_test' missing description
  ⚠️  App 'adc_test' missing source_file
  ⚠️  No 'target' specified in metadata, using default
```

#### **Verbose Processing**

With `--verbose` flag, the script shows:
```bash
🔍 Loading configuration...
  📁 Config file: /path/to/app_config.yml
  📊 Apps found: 16
  🎯 Target: esp32c6
  🔧 IDF versions: ['release/v5.5', 'release/v5.4']

🔧 Generating CI matrix...
  📋 Matrix entries: 32

🔍 Filtered matrix for app: gpio_test (2 entries)
```

#### **Configuration Overrides**

**Global Configuration**
```yaml
metadata:
  idf_versions: ["release/v5.5", "release/v5.4"]
  build_types: [["Debug", "Release"], ["Debug", "Release"]]
  target: "esp32c6"
```

**Per-App Overrides**
```yaml
apps:
  gpio_test:
    ci_enabled: true
    description: "GPIO peripheral comprehensive testing"
    source_file: "main/GpioComprehensiveTest.cpp"
    idf_versions: ["release/v5.5"]  # Override global
    build_types: [["Debug", "Release"]]
    
  wifi_test:
    ci_enabled: false  # Exclude from CI
    idf_versions: ["release/v5.4"]  # Specific version only
    build_types: [["Debug"]]  # Limited build types
```

**CI Exclusions**
```yaml
ci_config:
  exclude_combinations:
    - app_name: "bluetooth_test"
      build_type: "Debug"
      idf_version: "release/v5.4"
```

#### **Path Detection**

The script automatically detects configuration files from:
- Workspace root: `examples/esp32/app_config.yml`
- Examples directory: `app_config.yml`
- Scripts directory: `../app_config.yml`
- GitHub workflows: `../../examples/esp32/app_config.yml`
- Absolute path calculation from script location

#### **CI Integration**

**GitHub Actions**
```yaml
- name: Generate Build Matrix
  run: |
    MATRIX=$(python3 scripts/generate_matrix.py)
    echo "matrix=$MATRIX" >> $GITHUB_OUTPUT

- name: Build Applications
  strategy:
    matrix: ${{ fromJson(steps.matrix.outputs.matrix) }}
  run: |
    python3 scripts/build_app.sh ${{ matrix.app_name }} ${{ matrix.build_type }}
```

**GitLab CI**
```yaml
generate_matrix:
  script:
    - python3 scripts/generate_matrix.py --format yaml > matrix.yml
  artifacts:
    paths:
      - matrix.yml
```

**Jenkins Pipeline**
```groovy
pipeline {
  agent any
  stages {
    stage('Generate Matrix') {
      steps {
        script {
          def matrix = sh(
            script: 'python3 scripts/generate_matrix.py --format json',
            returnStdout: true
          ).trim()
          def matrixData = readJSON text: matrix
          env.MATRIX = matrix
        }
      }
    }
  }
}
```

#### **Error Handling**

- **Configuration Not Found**: Clear error messages with attempted paths
- **Invalid YAML**: Syntax validation with helpful error details
- **Validation Failures**: Structured error reporting with exit codes
- **File I/O Errors**: Safe file operations with proper error handling

#### **Performance Features**

- **Smart Path Detection**: Multiple fallback paths for configuration
- **Efficient Parsing**: YAML parsing with error handling
- **Memory Optimization**: Stream processing for large configurations
- **Cache Friendly**: Minimal file I/O operations

### **`app_config.yml` - Configuration File**

YAML configuration file that defines all build parameters.

#### **Structure**
```yaml
# Global metadata
metadata:
  idf_versions: ["release/v5.5", "release/v5.4", "release/v5.3"]
  build_types: [["Debug", "Release"], ["Debug", "Release"], ["Debug"]]
  target: "esp32c6"
  build_directory_pattern: "build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}"

# Application configurations
apps:
  gpio_test:
    ci_enabled: true
    description: "GPIO peripheral comprehensive testing"
    idf_versions: ["release/v5.5"]  # Override global
    build_types: [["Debug", "Release"]]
    
  adc_test:
    ci_enabled: true
    description: "ADC peripheral testing"
    # Uses global configuration
    
  wifi_test:
    ci_enabled: false  # Exclude from CI
    description: "WiFi functionality testing"
    idf_versions: ["release/v5.4"]
    build_types: [["Release"]]

# CI configuration
ci_config:
  exclude_combinations:
    - app_name: "wifi_test"
      idf_version: "release/v5.3"
      build_type: "Release"
```

#### **Configuration Features**
- **Hierarchical Overrides** - Per-app configuration overrides global settings
- **CI Control** - Enable/disable applications in CI builds
- **Build Type Mapping** - Different build types per IDF version
- **Exclusion Rules** - Exclude specific combinations from CI
- **Flexible Patterns** - Customizable build directory naming

---

## 🚀 **Usage Examples**

### **Environment Setup Workflows**

#### **Local Development Setup**
```bash
# Complete local development environment
./setup_repo.sh

# What it provides:
# • Full development toolchain (clang, clang-format, clang-tidy)
# • ESP-IDF auto-installation and configuration
# • Development aliases and environment variables
# • Interactive setup with user guidance
# • Complete dependency installation
```

#### **CI/CD Environment Setup**
```bash
# CI-optimized environment setup
./setup_ci.sh

# What it provides:
# • Minimal CI dependencies (clang-20, yq, PyYAML)
# • Build directory structure preparation
# • Cache-aware installation
# • Non-interactive operation
# • ESP-IDF handled by ESP-IDF CI action
```

### **New Enhanced Commands**

The build system now includes several new commands for better user experience and validation:

#### **📋 Information Commands**
```bash
# Show detailed information for a specific app
./scripts/build_app.sh info gpio_test

# Show all valid build combinations across all apps
./scripts/build_app.sh combinations

# Validate a specific build combination
./scripts/build_app.sh validate gpio_test Release
./scripts/build_app.sh validate gpio_test Release release/v5.4
```

#### **🛡️ Validation Examples**
```bash
# Valid combination - proceeds with build
./scripts/build_app.sh validate gpio_test Release
# Output: ✅ VALID: This combination is allowed!

# Invalid combination - shows error with guidance
./scripts/build_app.sh validate gpio_test Release release/v5.4
# Output: ❌ INVALID: This combination is not allowed!
#        Valid combinations for 'gpio_test':
#        • release/v5.5: Debug Release
```

#### **🧠 Smart Default Examples**
```bash
# No IDF version specified - uses smart default
./scripts/build_app.sh gpio_test Release
# Output: No IDF version specified, using smart default: release/v5.5

# IDF version explicitly specified
./scripts/build_app.sh gpio_test Release release/v5.5
# Output: Uses specified version directly
```

### **Local Development Workflow**

```bash
# 1. Setup development environment (choose one)
./setup_repo.sh          # Local development
./setup_ci.sh            # CI environment

# 2. Build application
./scripts/build_app.sh gpio_test Release

# 3. Flash and monitor
./scripts/flash_app.sh flash_monitor gpio_test Release

# 4. Monitor only
./scripts/flash_app.sh monitor
```

### **CI/CD Integration**

```yaml
# GitHub Actions workflow
- name: ESP-IDF Build
  run: |
    # Source CI setup
    source ${{ env.ESP32_PROJECT_PATH }}/scripts/setup_ci.sh
    
    # Build application
    ./scripts/build_app.sh "${{ matrix.app_name }}" "${{ matrix.build_type }}" "${{ matrix.idf_version }}"
    
    # Capture build directory for artifacts
    echo "build_dir=$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" >> $GITHUB_OUTPUT
```

### **Advanced Configuration**

```bash
# Custom ESP-IDF version
./scripts/build_app.sh gpio_test Release release/v5.4

# Clean build
CLEAN=1 ./scripts/build_app.sh gpio_test Release

# Verbose output
./scripts/build_app.sh --verbose gpio_test Release

# Multiple applications
for app in gpio_test adc_test uart_test; do
    ./scripts/build_app.sh "$app" Release
done
```

### **Configuration Management**

```bash
# List available applications
source scripts/config_loader.sh
get_app_types

# Get application description
get_app_description "gpio_test"

# List build directories
list_build_directories

# Parse build directory
parse_build_directory "build-app-gpio_test-type-Release-target-esp32c6-idf-release_v5_5"
```

---

## 🔍 **Troubleshooting**

### **Common Issues**

#### **ESP-IDF Not Found**
```bash
# Error: ESP-IDF environment not found
# Solution: The build system will auto-install ESP-IDF

# Manual installation if needed:
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git esp-idf-release_v5_5
cd esp-idf-release_v5_5
./install.sh
source export.sh
```

#### **Permission Issues**
```bash
# Error: Permission denied
# Solution: Make scripts executable
chmod +x scripts/*.sh
chmod +x scripts/*.py
```

#### **Python Dependencies**
```bash
# Error: Module not found
# Solution: Install required packages
pip install pyyaml
pip install esptool
```

#### **Build Directory Issues**
```bash
# Error: Invalid build directory name
# Solution: Check app_config.yml build_directory_pattern

# Ensure pattern follows format:
build_directory_pattern: "build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}"
```

#### **🆕 Validation System Issues**
```bash
# Error: Invalid build combination
# Solution: Use validation commands to see what's allowed

# Check valid combinations for specific app:
./scripts/build_app.sh info gpio_test

# See all valid combinations:
./scripts/build_app.sh combinations

# Validate specific combination:
./scripts/build_app.sh validate gpio_test Release release/v5.4

# Common validation errors:
# • App doesn't support requested build type
# • App doesn't support requested IDF version
# • Combination constraint violation
```

#### **Smart Default Issues**
```bash
# Error: Smart default not working
# Solution: Check app_config.yml configuration

# Ensure app has idf_versions defined:
apps:
  gpio_test:
    idf_versions: ["release/v5.5"]
    build_types: ["Debug", "Release"]

# Or check global defaults:
metadata:
  idf_versions: ["release/v5.5", "release/v5.4"]
  build_types: [["Debug", "Release"], ["Debug"]]
```

#### **🆕 Environment Setup Issues**
**Problem**: Setup scripts fail or don't work as expected
**Symptoms**: Setup errors or missing dependencies
**Solutions**:
```bash
# For local development (complete environment)
./setup_repo.sh

# For CI/CD (minimal environment)
./setup_ci.sh

# Check script permissions
chmod +x scripts/*.sh

# Verify system requirements and dependencies
# Check environment variables and paths
```

### **Debug Mode**

Enable verbose output for debugging:

```bash
# Verbose build
./scripts/build_app.sh --verbose gpio_test Release

# Clean rebuild
CLEAN=1 ./scripts/build_app.sh gpio_test Release

# Check environment
source scripts/setup_repo.sh
echo $IDF_PATH
echo $IDF_TARGET
```

### **Log Files**

Build logs are available in:

```bash
# Build log
cat build-*/log/build.log

# CMake log
cat build-*/CMakeFiles/CMakeOutput.log

# Ninja log
cat build-*/.ninja_log
```

---

## 🤝 **Contributing**

### **Adding New Scripts**

1. **Create Script File**
```bash
   # Create new script
   touch scripts/new_script.sh
   chmod +x scripts/new_script.sh
   ```

2. **Add Documentation**
   - Update this README
   - Add usage examples
   - Document dependencies

3. **Test Integration**
   - Test with existing scripts
   - Verify CI compatibility
   - Check error handling

### **Modifying Existing Scripts**

1. **Backup Original**
   ```bash
   cp scripts/script_name.sh scripts/script_name.sh.backup
   ```

2. **Make Changes**
   - Implement new functionality
   - Maintain backward compatibility
   - Update error handling

3. **Test Changes**
   - Test locally
   - Verify CI compatibility
   - Check all use cases

### **Script Standards**

- **Error Handling** - Comprehensive error checking and reporting
- **Logging** - Detailed logs for debugging
- **Documentation** - Clear usage instructions and examples
- **Testing** - Test all functionality and edge cases
- **CI Compatibility** - Ensure scripts work in CI environment
- **🆕 Environment Separation** - Maintain clear separation between local and CI concerns

---

## 📄 **License**

This project is licensed under the GPL-3.0 License - see the [LICENSE](../../LICENSE) file for details.

---

## 🔗 **Related Documentation**

- [ESP32 Examples README](../README.md) - Examples overview and usage
- [Main Project README](../../README.md) - Project overview and architecture
- [CI/CD Workflows](../../.github/workflows/) - GitHub Actions workflows
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/) - ESP-IDF reference

---

<div align="center">

**🚀 Built for ESP32 Development**

*Build system with automated CI/CD integration*

</div>
