# ESP32 HardFOC Interface Wrapper - Build Configuration Guide

This document explains how to build and use different apps in the ESP32 HardFOC Interface Wrapper project.

---

**Navigation**: [← Previous: Scripts Overview](README_SCRIPTS_OVERVIEW.md) | [Back to Scripts](../README.md) | [Next: Flash System →](README_FLASH_SYSTEM.md)

---

## Overview

The build system is a configuration-driven system that automatically extracts build information from `app_config.yml` and provides flexible build options for ESP32 apps. It's designed to be independent of specific test implementations and focuses on the build process itself.

### Key Features
- **Configuration-Driven**: All build information extracted from `app_config.yml`
- **Flexible Build Types**: Debug and Release configurations with customizable parameters
- **Smart Defaults**: Automatic fallbacks when configuration is incomplete
- **Build Optimization**: ccache support and incremental build options
- **Cross-Platform**: Works on Linux, macOS, and Windows (WSL)
- **ESP-IDF Integration**: Direct integration with ESP-IDF build system

## Configuration Structure

### Configuration File: `app_config.yml`

The build system reads from a centralized YAML configuration file located at:
```
examples/esp32/app_config.yml
```

#### Configuration Structure
```yaml
# Metadata section - global defaults
metadata:
  default_app: "ascii_art"            # Default app to build
  default_build_type: "Release"       # Default build configuration
  target: "esp32c6"                   # Target MCU
  idf_versions: ["release/v5.5"]      # Supported ESP-IDF versions

# Apps section - defines all available apps
apps:
  ascii_art:
    description: "ASCII art generator app"
    source_file: "AsciiArtComprehensiveTest.cpp"     # Source file to compile
    category: "utility"
    build_types: ["Debug", "Release"]  # Supported build types
    ci_enabled: true                   # Include in CI builds
    featured: true                     # Show in featured list

  gpio_test:
    description: "GPIO peripheral testing app"
    source_file: "GpioComprehensiveTest.cpp"     # Source file to compile
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true

  adc_test:
    description: "ADC peripheral testing app"
    source_file: "AdcComprehensiveTest.cpp"   # Source file to compile
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: false

# Build configuration section
build_config:
  build_types:
    Debug:
      description: "Debug symbols, verbose logging, assertions enabled"
      cmake_build_type: "Debug"
      optimization: "-O0"
      debug_level: "-g3"
      defines: ["DEBUG"]
    Release:
      description: "Optimized for performance and size"
      cmake_build_type: "Release"
      optimization: "-O2"
      debug_level: "-g"
      defines: ["NDEBUG"]
  
  build_directory_pattern: "build_{app_type}_{build_type}"
  project_name_pattern: "esp32_project_{app_type}_app"
```

### Configuration Loading Process

The build system uses `config_loader.sh` to extract information:

1. **Primary Method**: Uses `yq` for reliable YAML parsing
2. **Fallback Method**: Basic text parsing with `grep`/`sed` if `yq` unavailable
3. **Environment Variables**: Exports configuration as shell variables
4. **Validation**: Checks configuration integrity and provides defaults

#### Available Configuration Functions
```bash
# Load and initialize configuration
init_config

# Get app information
get_app_types                    # All available apps
get_featured_app_types          # Featured apps only
get_app_description "gpio_test" # Description for specific app
get_app_source_file "gpio_test" # Source file for app

# Get build information
get_build_types                     # Available build types
is_valid_app_type "gpio_test"  # Validate app type
is_valid_build_type "Release"      # Validate build type

# Get build configuration
get_build_directory "gpio_test" "Release"  # Build directory path
get_project_name "gpio_test"               # Project name
```

### **How app_config.yml Drives the Build System**

#### **Automatic Configuration Detection**
The build system automatically finds and loads your configuration:

```bash
# Scripts automatically detect project root
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONFIG_FILE="$PROJECT_DIR/app_config.yml"

# Configuration is loaded automatically
source "$PROJECT_DIR/scripts/config_loader.sh"
init_config
```

#### **Configuration Validation**
Before building, the system validates your configuration:

```bash
# Check if configuration file exists
if ! [ -f "$CONFIG_FILE" ]; then
    echo "ERROR: app_config.yml not found in project root"
echo "Please create app_config.yml in: $PROJECT_DIR"
    exit 1
fi

# Validate app type
if ! is_valid_app_type "$APP_TYPE"; then
    echo "ERROR: Invalid app type: $APP_TYPE"
    echo "Available types: $(get_app_types)"
    echo "Check your app_config.yml file"
    exit 1
fi

# Validate build type
if ! is_valid_build_type "$BUILD_TYPE"; then
    echo "ERROR: Invalid build type: $BUILD_TYPE"
    echo "Available types: $(get_build_types)"
    echo "Check your app_config.yml file"
    exit 1
fi
```

#### **Dynamic Build Directory Creation**
Build directories are created based on your configuration patterns:

```bash
# Get build directory from configuration
BUILD_DIR=$(get_build_directory "$APP_TYPE" "$BUILD_TYPE")

# For gpio_test Release, this creates:
# build_gpio_test_Release/

# The pattern is configurable in app_config.yml:
# build_directory_pattern: "build_{app_type}_{build_type}"
```

#### **Project Name Generation**
Binary files are named using your configured patterns:

```bash
# Get project name from configuration
PROJECT_NAME=$(get_project_name "$APP_TYPE")

# For gpio_test, this generates:
# esp32_iid_gpio_test_app

# The pattern is configurable in app_config.yml:
# project_name_pattern: "esp32_project_{app_type}_app"
```

#### **CMake Integration**
Your configuration drives CMake variables automatically:

```bash
# Configure project with your configuration
idf.py -B "$BUILD_DIR" \
       -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
       -D APP_TYPE="$APP_TYPE" \
       -D IDF_CCACHE_ENABLE="$USE_CCACHE" \
       reconfigure

# These variables come from your app_config.yml:
# - CMAKE_BUILD_TYPE: "Debug" or "Release"
# - APP_TYPE: "gpio_test", "adc_test", etc.
# - IDF_CCACHE_ENABLE: 1 or 0 (from --use-cache flag)
```

#### **Source File Resolution**
The system automatically finds your source files:

```bash
# Get source file from configuration
SOURCE_FILE=$(get_app_source_file "$APP_TYPE")

# For gpio_test, this returns:
# "main/GpioComprehensiveTest.cpp"

# The system validates this file exists before building
if [ ! -f "$SOURCE_FILE" ]; then
    echo "ERROR: Source file not found: $SOURCE_FILE"
    echo "Check your app_config.yml source_file path"
    exit 1
fi
```

## Build System Script: `build_app.sh`

### Command Syntax
```bash
./scripts/build_app.sh [app_type] [build_type] [flags]
```

### Parameters
- **`app_type`** - Type of app to build (e.g., `gpio_test`, `adc_test`)
- **`build_type`** - Build configuration (`Debug` or `Release`)
- **`flags`** - Build control flags (see below)

### Build Control Flags

#### Clean Build Options
```bash
--clean          # Remove existing build directory before building
--no-clean       # Preserve existing build directory (default)
```

#### Cache Control Options
```bash
--use-cache     # Enable ccache for faster builds (default)
--no-cache      # Disable ccache for clean builds
```

#### Environment Variable Overrides
```bash
export CLEAN=1              # Force clean builds
export USE_CCACHE=0         # Disable ccache
export APP_TYPE="gpio_test"  # Override app type
export BUILD_TYPE="Debug"   # Override build type
```

### Default Values
- **`app_type`**: Loaded from `app_config.yml` → `metadata.default_app` (e.g., `ascii_art`)
- **`build_type`**: Loaded from `app_config.yml` → `metadata.default_build_type` (e.g., `Release`)
- **`CLEAN`**: `0` (preserve existing builds)
- **`USE_CCACHE`**: `1` (enable compiler cache)

## Build Process

### 1. Configuration Loading
```bash
# Source configuration loader
source ./scripts/config_loader.sh

# Initialize configuration
init_config

# Configuration is now available as environment variables:
echo $CONFIG_DEFAULT_APP          # DEFAULT: ascii_art
echo $CONFIG_DEFAULT_BUILD_TYPE   # DEFAULT: Release
echo $CONFIG_TARGET               # DEFAULT: esp32c6
```

### 2. App and Build Type Validation
```bash
# Validate app type
if is_valid_app_type "$APP_TYPE"; then
    echo "Valid app: $APP_TYPE"
    description=$(get_app_description "$APP_TYPE")
    echo "Description: $description"
else
    echo "ERROR: Invalid app type: $APP_TYPE"
    echo "Available: $(get_app_types)"
    exit 1
fi

# Validate build type
if is_valid_build_type "$BUILD_TYPE"; then
    echo "Valid build type: $BUILD_TYPE"
else
    echo "ERROR: Invalid build type: $BUILD_TYPE"
    echo "Available: $(get_build_types)"
    exit 1
fi
```

### 3. Build Directory Configuration
```bash
# Get build directory using configuration
BUILD_DIR=$(get_build_directory "$APP_TYPE" "$BUILD_TYPE")
# Result: "build_gpio_test_Release"

# Get project name using configuration
PROJECT_NAME=$(get_project_name "$APP_TYPE")
# Result: "esp32_iid_gpio_test_app"
```

### 4. ESP-IDF Integration
```bash
# Set ESP-IDF target
export IDF_TARGET=$CONFIG_TARGET

# Configure project with CMake variables
idf.py -B "$BUILD_DIR" \
       -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
       -D APP_TYPE="$APP_TYPE" \
       -D IDF_CCACHE_ENABLE="$USE_CCACHE" \
       reconfigure

# Build project
idf.py -B "$BUILD_DIR" build
```

## Build Directory Structure

### Directory Naming Convention
The build system uses configurable patterns from `app_config.yml`:

```yaml
build_directory_pattern: "build_{example_type}_{build_type}"
project_name_pattern: "esp32_iid_{example_type}_example"
```

### Example Directory Structure
```
examples/esp32/
├── build_gpio_test_Release/       # GPIO test, Release build
├── build_gpio_test_Debug/         # GPIO test, Debug build
├── build_adc_test_Release/        # ADC test, Release build
├── build_adc_test_Debug/          # ADC test, Debug build
└── build_ascii_art_Release/       # ASCII art, Release build
```

### Build Artifacts
Each build directory contains:
- **CMake files**: `CMakeCache.txt`, `CMakeFiles/`
- **Build files**: `build.ninja`, `compile_commands.json`
- **Output files**: `.bin`, `.elf`, `.map` files
- **Component files**: Compiled object files and libraries

## Build Features

### Compiler Cache (ccache)
The build system automatically enables ccache for faster incremental builds:

```bash
# Enable ccache (default)
export USE_CCACHE=1
./scripts/build_app.sh gpio_test Release

# Disable ccache
export USE_CCACHE=0
./scripts/build_app.sh gpio_test Release

# Or use flags
./scripts/build_app.sh gpio_test Release --use-cache
./scripts/build_app.sh gpio_test Release --no-cache
```

### Clean Builds
Force complete rebuilds when needed:

```bash
# Clean build (remove existing build directory)
./scripts/build_app.sh gpio_test Release --clean

# Preserve existing build (default)
./scripts/build_app.sh gpio_test Release --no-clean

# Environment variable override
export CLEAN=1
./scripts/build_app.sh gpio_test Release
```

### Build Validation
The build system validates builds and provides detailed output:

```bash
# Build with validation
./scripts/build_app.sh gpio_test Release

# Output includes:
# - Configuration validation
# - Build directory setup
# - ESP-IDF integration
# - Build completion status
# - Binary file information
# - Size analysis
```

## Build Output and Analysis

### Binary Files
After successful build, the following files are generated:

```bash
# Main binary file
$BUILD_DIR/$PROJECT_NAME.bin          # Flashable binary
$BUILD_DIR/$PROJECT_NAME.elf          # ELF file with symbols
$BUILD_DIR/$PROJECT_NAME.map          # Memory map

# Bootloader files
$BUILD_DIR/bootloader/bootloader.bin  # Bootloader binary
$BUILD_DIR/partition_table/partition-table.bin  # Partition table
```

### Size Analysis
The build system automatically provides size information:

```bash
# Size analysis output
======================================================
BUILD SIZE INFORMATION
======================================================
   text    data     bss     dec     hex filename
  12345    1234     567   14146    3742 esp32_iid_gpio_test_example.elf
```

### Build Statistics
```bash
# Build completion summary
======================================================
BUILD COMPLETED SUCCESSFULLY
======================================================
App Type: gpio_test
Build Type: Release
Target: esp32c6
Build Directory: build_gpio_test_Release
Project Name: esp32_iid_gpio_test_app
Binary: build_gpio_test_Release/esp32_iid_gpio_test_app.bin
```

## Integration with Other Scripts

### Flash System Integration
The build system integrates with the flash system:

```bash
# Build then flash
./scripts/build_app.sh gpio_test Release
./scripts/flash_app.sh flash gpio_test Release

# Or use flash system's auto-build feature
./scripts/flash_app.sh flash gpio_test Release  # Builds if needed
```

### CI/CD Integration
The build system supports CI/CD workflows:

```bash
# Get CI-enabled examples
ci_examples=$(get_ci_example_types)

# Build all CI examples
for example in $ci_examples; do
    ./scripts/build_app.sh "$example" Release
done
```

### Configuration Validation
```bash
# Validate configuration before building
if ! ./scripts/config_loader.sh --validate; then
    echo "Configuration validation failed"
    exit 1
fi

# Check specific app configuration
if is_valid_app_type "gpio_test"; then
    echo "GPIO test is properly configured"
fi
```

## Error Handling and Troubleshooting

### Common Build Issues

#### 1. Configuration Errors
```bash
# Error: Invalid example type
ERROR: Invalid example type: invalid_example
Available types: ascii_art gpio_test adc_test ...

# Solution: Check app_config.yml or use list command
./scripts/build_app.sh list
```

#### 2. ESP-IDF Environment Issues
```bash
# Error: ESP-IDF environment not found
ERROR: ESP-IDF export.sh not found at $HOME/esp/esp-idf/export.sh

# Solution: Source ESP-IDF environment
source $HOME/esp/esp-idf/export.sh
# Or use setup scripts
./scripts/setup_repo.sh
```

#### 3. Build Directory Issues
```bash
# Error: Build directory creation failed
ERROR: Could not create build directory

# Solution: Check permissions and disk space
ls -la examples/esp32/
df -h examples/esp32/
```

#### 4. CMake Configuration Issues
```bash
# Error: CMake configuration failed
ERROR: Configuration failed

# Solution: Check CMake variables and dependencies
idf.py -B build_dir -D APP_TYPE=gpio_test reconfigure
```

### Debug Mode
Enable verbose output for troubleshooting:

```bash
# Enable debug output
export DEBUG=1
./scripts/build_app.sh gpio_test Release

# Check configuration loading
source ./scripts/config_loader.sh
init_config
echo "Default example: $CONFIG_DEFAULT_EXAMPLE"
echo "Default build type: $CONFIG_DEFAULT_BUILD_TYPE"
```

## Best Practices

### 1. Use Configuration-Driven Approach
```bash
# Good: Let configuration drive the build
./scripts/build_app.sh gpio_test Release

# Bad: Hardcode build parameters
idf.py build -D APP_TYPE=gpio_test -D BUILD_TYPE=Release
```

### 2. Leverage Build Types Appropriately
```bash
# Development: Use Debug for detailed logging
./scripts/build_app.sh gpio_test Debug

# Production: Use Release for performance
./scripts/build_app.sh gpio_test Release
```

### 3. Use Clean Builds When Needed
```bash
# Good: Clean build after configuration changes
./scripts/build_app.sh gpio_test Release --clean

# Good: Incremental build for development
./scripts/build_app.sh gpio_test Release --no-clean
```

### 4. Monitor Build Output
```bash
# Check build completion
./scripts/build_app.sh gpio_test Release

# Verify binary generation
ls -la build_gpio_test_Release/*.bin

# Analyze build size
idf.py -B build_gpio_test_Release size
```

### 5. Validate Configuration
```bash
# Check configuration before building
./scripts/build_app.sh list

# Verify specific examples
./scripts/config_loader.sh --validate
```

## Performance Optimization

### Build Speed Optimization
```bash
# Enable ccache for faster incremental builds
export USE_CCACHE=1
./scripts/build_app.sh gpio_test Release

# Use parallel builds (ESP-IDF handles this automatically)
export MAKEFLAGS="-j$(nproc)"
```

### Disk Space Management
```bash
# Clean old builds to save space
find examples/esp32/ -name "build_*" -type d -exec du -sh {} \;

# Remove specific build directories
rm -rf build_gpio_test_Debug build_gpio_test_Release
```

### Memory Optimization
```bash
# Monitor build memory usage
./scripts/build_app.sh gpio_test Release 2>&1 | grep -i memory

# Use appropriate build types for memory constraints
./scripts/build_app.sh gpio_test Release  # Smaller binary
```

## Support and Maintenance

### Getting Help
```bash
# Script help
./scripts/build_app.sh --help

# List available examples
./scripts/build_app.sh list

# Configuration help
./scripts/config_loader.sh --help
```

### Reporting Issues
1. **Include configuration file content**
2. **Provide build output and error messages**
3. **Specify platform and ESP-IDF version**
4. **Include relevant environment variables**

### Contributing
1. **Follow existing configuration style**
2. **Add clear descriptions**
3. **Test configuration validation**
4. **Update documentation**

## Version Information

- **Build System Version**: 2.0.0
- **Configuration Format**: YAML 1.2
- **ESP-IDF Compatibility**: v5.5+
- **Platform Support**: Linux, macOS, Windows (WSL)
- **Last Updated**: August 2025

For additional information, see:
- [Scripts Overview](README_SCRIPTS_OVERVIEW.md)
- [Flash System](README_FLASH_SYSTEM.md)
- [Configuration System](README_CONFIG_SYSTEM.md)
- [Port Detection](README_PORT_DETECTION.md)

## Quick Start

### Basic Build Commands

```bash
# Build with defaults (ascii_art, Release)
./scripts/build_app.sh

# Build specific example and build type
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh adc_test Debug

# List all available examples
./scripts/build_app.sh list

# Get help
./scripts/build_app.sh --help
```

### Build Control Options

```bash
# Clean build (remove existing build directory)
./scripts/build_app.sh gpio_test Release --clean

# Disable ccache for clean builds
./scripts/build_app.sh gpio_test Release --no-cache

# Combine options
./scripts/build_app.sh gpio_test Release --clean --no-cache
```

### Environment Variable Overrides

```bash
# Override defaults
export APP_TYPE="gpio_test"
export BUILD_TYPE="Debug"
export CLEAN=1
./scripts/build_app.sh

# Or set specific values
export USE_CCACHE=0
./scripts/build_app.sh gpio_test Release
```

## Build Examples

### Basic Build Examples

```bash
# Build with defaults (ascii_art, Release)
./scripts/build_app.sh

# Build specific examples
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh adc_test Debug
./scripts/build_app.sh bluetooth_example Release
./scripts/build_app.sh utility_example Debug
```

### Build Examples

```bash
# Clean build for development
./scripts/build_app.sh gpio_test Debug --clean

# Production build with cache disabled
./scripts/build_app.sh gpio_test Release --no-cache

# Debug build with custom environment
export CLEAN=1
export USE_CCACHE=0
./scripts/build_app.sh adc_test Debug
```

### CI/CD Build Examples

```bash
# Build all CI-enabled examples
for example in $(./scripts/config_loader.sh --get-ci-examples); do
    ./scripts/build_app.sh "$example" Release
done

# Build featured examples only
for example in $(./scripts/config_loader.sh --get-featured-examples); do
    ./scripts/build_app.sh "$example" Release
done
```

### Build Validation Examples

```bash
# Validate configuration before building
./scripts/config_loader.sh --validate

# Check specific example configuration
./scripts/config_loader.sh --check-example gpio_test

# Verify build types for example
./scripts/config_loader.sh --get-build-types
```

## Build System Structure

### Configuration-Driven Design

The build system is designed around a centralized configuration approach:

```yaml
# app_config.yml drives everything
metadata:
  default_example: "ascii_art"
  default_build_type: "Release"
  target: "esp32c6"

examples:
  gpio_test:
    source_file: "main/GpioExample.cpp"
    build_types: ["Debug", "Release"]
    category: "peripheral"
```

### ESP-IDF Integration

The build system integrates with ESP-IDF:

```bash
# Automatic ESP-IDF environment detection
if [ -z "$IDF_PATH" ]; then
    source "$HOME/esp/esp-idf/export.sh"
fi

# Target configuration
export IDF_TARGET=$CONFIG_TARGET

# CMake integration
idf.py -B "$BUILD_DIR" \
       -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
       -D APP_TYPE="$APP_TYPE" \
       reconfigure
```

### Build Directory Management

```bash
# Configurable directory patterns
build_directory_pattern: "build_{example_type}_{build_type}"
project_name_pattern: "esp32_iid_{example_type}_example"

# Results in:
# build_gpio_test_Release/
# build_adc_test_Debug/
# build_ascii_art_Release/
```

## Build Types and Optimization

### Debug Configuration
- **CMake Build Type**: `Debug`
- **Optimization**: `-O0` (no optimization)
- **Debug Symbols**: `-g3` (maximum debug information)
- **Defines**: `DEBUG` (enable debug features)

### Release Configuration
- **CMake Build Type**: `Release`
- **Optimization**: `-O2` (high optimization)
- **Debug Symbols**: `-g` (minimal debug information)
- **Defines**: `NDEBUG` (disable debug features)

## Target Configuration

- **Primary Target**: ESP32-C6
- **ESP-IDF Version**: v5.5+
- **Toolchain**: Modern C++17 with ESP-IDF compatibility
- **Build System**: Ninja-based build system

## Troubleshooting

### Common Build Issues

#### 1. Configuration Errors
```bash
# Error: Invalid example type
ERROR: Invalid example type: invalid_example
Available types: ascii_art gpio_test adc_test ...

# Solution: Check app_config.yml or use list command
./scripts/build_app.sh list
```

#### 2. ESP-IDF Environment Issues
```bash
# Error: ESP-IDF environment not found
ERROR: ESP-IDF export.sh not found at $HOME/esp/esp-idf/export.sh

# Solution: Source ESP-IDF environment
source $HOME/esp/esp-idf/export.sh
# Or use setup scripts
./scripts/setup_repo.sh
```

#### 3. Build Directory Issues
```bash
# Error: Build directory creation failed
ERROR: Could not create build directory

# Solution: Check permissions and disk space
ls -la examples/esp32/
df -h examples/esp32/
```

#### 4. CMake Configuration Issues
```bash
# Error: CMake configuration failed
ERROR: Configuration failed

# Solution: Check CMake variables and dependencies
idf.py -B build_dir -D APP_TYPE=gpio_test reconfigure
```

### Debug Mode
Enable verbose output for troubleshooting:

```bash
# Enable debug output
export DEBUG=1
./scripts/build_app.sh gpio_test Release

# Check configuration loading
source ./scripts/config_loader.sh
init_config
echo "Default example: $CONFIG_DEFAULT_EXAMPLE"
echo "Default build type: $CONFIG_DEFAULT_BUILD_TYPE"
```

### Getting Help

- **Script help**: `./scripts/build_app.sh --help`
- **List examples**: `./scripts/build_app.sh list`
- **Configuration help**: `./scripts/config_loader.sh --help`
- **Check main project README.md** for setup instructions
- **Review component documentation** in `docs/`

## Usage

### Environment Variable Overrides

```bash
# Override build behavior
export CLEAN=1              # Force clean builds
export USE_CCACHE=0         # Disable ccache
export APP_TYPE="gpio"  # Override example type
export BUILD_TYPE="Debug"   # Override build type

# Run with overrides
./scripts/build_app.sh
```

### Custom Build Configurations

```bash
# Custom compiler flags
export EXTRA_CXXFLAGS="-DCUSTOM_DEBUG_LEVEL=3"
export EXTRA_CFLAGS="-DCUSTOM_FEATURE=1"

# Build with custom flags
./scripts/build_app.sh gpio_test Release
```

### CI/CD Integration

The build system supports automated CI/CD workflows:

```bash
# Build all CI-enabled examples
ci_examples=$(get_ci_example_types)
for example in $ci_examples; do
    ./scripts/build_app.sh "$example" Release
done

# Build featured examples only
featured_examples=$(get_featured_example_types)
for example in $featured_examples; do
    ./scripts/build_app.sh "$example" Release
done
```

### Performance Optimization

```bash
# Enable parallel builds
export MAKEFLAGS="-j$(nproc)"

# Optimize ccache
export CCACHE_DIR="$HOME/.ccache"
export CCACHE_MAXSIZE=10G

# Build with optimizations
./scripts/build_app.sh gpio_test Release
```

---

**Navigation**: [← Previous: Scripts Overview](README_SCRIPTS_OVERVIEW.md) | [Back to Scripts](../README.md) | [Next: Flash System →](README_FLASH_SYSTEM.md)


