# ESP32 Project Configuration System Documentation

This document provides complete documentation for the configuration system, which includes the `config_loader.sh` script and the `app_config.yml` configuration file. This system makes the scripts directory completely portable between ESP32 projects.

---

**Navigation**: [← Previous: Logging System](README_LOGGING_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Utility Scripts →](README_UTILITY_SCRIPTS.md)

---

## Overview

The configuration system provides centralized management of all ESP32 app configurations, making the scripts directory completely portable:

- **Portable configuration** - Works in any ESP32 project without modification
- **Centralized management** - Single source of truth for all apps
- **Validation and error checking** - Ensures configuration integrity
- **Flexible app definitions** - Easy to add new apps
- **Build type management** - Debug/Release configurations
- **Target configuration** - MCU-specific settings
- **Project independence** - No hardcoded paths or project-specific references

## Configuration Architecture

### Components
1. **`app_config.yml`** - Main configuration file
2. **`config_loader.sh`** - Configuration loading and validation
3. **Script integration** - All scripts use centralized configuration
4. **Environment variables** - Runtime configuration overrides

### Configuration File Structure
```
app_config.yml
├── metadata                 # Global project metadata
│   ├── default_example     # Default example to build
│   ├── default_build_type  # Default build configuration
│   ├── target              # MCU target configuration
│   └── idf_versions        # Supported ESP-IDF versions
├── examples                # Example definitions
│   ├── featured            # Featured examples (shown first)
│   └── all                 # All available examples
└── build_config            # Build configuration
    ├── build_types         # Debug/Release configurations
    ├── patterns            # Directory and naming patterns
    └── optimization        # Compiler optimization settings
```

## Configuration File (`app_config.yml`)

### File Location
```
your_esp32_project/
├── app_config.yml         # Place this in your project root
├── scripts/                    # Scripts directory
├── main/                       # Source code directory
└── CMakeLists.txt             # Project CMake file
```

**Important**: The `app_config.yml` file must be placed in your **project root** (same level as the `scripts/` directory) for the scripts to work properly.

### Basic Structure
```yaml
# ESP32 Project Configuration
metadata:
  default_app: "ascii_art"
  default_build_type: "Release"
  target: "esp32c6"
  idf_versions: ["release/v5.5"]

# Apps section - define your project's apps
apps:
  ascii_art:
    description: "Main application app"
    source_file: "main/MainExample.cpp"
    category: "utility"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true
    
  gpio_test:
    description: "GPIO interface app"
    source_file: "main/GpioExample.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true
    
  adc_test:
    description: "Sensor interface app"
    source_file: "main/SensorExample.cpp"
    category: "sensor"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: false
    
  bluetooth_example:
    description: "Bluetooth connectivity app"
    source_file: "main/BluetoothExample.cpp"
    category: "connectivity"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true

# Build configuration section
build_config:
  build_types:
    Debug:
      description: "Debug build with symbols"
      cmake_build_type: "Debug"
      optimization: "-O0"
      debug_level: "-g3"
      defines: ["DEBUG"]
    Release:
      description: "Release build optimized"
      cmake_build_type: "Release"
      optimization: "-O2"
      debug_level: "-g"
      defines: ["NDEBUG"]
  
  build_directory_pattern: "build_{app_type}_{build_type}"
  project_name_pattern: "esp32_project_{app_type}_app"
```
    source_file: "main/PwmExample.cpp"
    featured: false
    
  can_test:
    description: "CAN peripheral testing"
    source_file: "main/CanExample.cpp"
    featured: false
    
  pio_test:
    description: "Enhanced PIO/RMT testing suite with channel-specific callbacks, ESP32 variant validation, WS2812, and ASCII art decoration"
    source_file: "main/PioExample.cpp"
    featured: true
    
  temperature_test:
    description: "Temperature sensor testing"
    source_file: "main/TemperatureExample.cpp"
    featured: false
    
  nvs_test:
    description: "NVS (Non-Volatile Storage) testing"
    source_file: "main/NvsExample.cpp"
    featured: false
    
  timer_test:
    description: "Timer peripheral testing"
    source_file: "main/TimerExample.cpp"
    featured: false
    
  logger_test:
    description: "Logger system testing"
    source_file: "main/LoggerExample.cpp"
    featured: false
    
  wifi_test:
    description: "WiFi connectivity testing"
    source_file: "main/WifiExample.cpp"
    featured: false
    
  bluetooth_test:
    description: "Bluetooth testing suite"
    source_file: "main/BluetoothExample.cpp"
    featured: true
    
  utils_test:
    description: "Utilities testing suite"
    source_file: "main/UtilsExample.cpp"
    featured: true
```

### Configuration Sections

#### 1. Target Configuration
```yaml
target: "esp32c6"
```
- **Purpose**: Specifies the target MCU
- **Supported values**: `esp32c6`, `esp32`, `esp32s3`
- **Usage**: Sets `IDF_TARGET` environment variable

#### 2. Build Types
```yaml
build_types: ["Debug", "Release"]
```
- **Purpose**: Defines available build configurations
- **Debug**: Includes debug symbols, verbose logging, assertions
- **Release**: Optimized for performance and size

#### 3. Default Values
```yaml
defaults:
  example: "gpio_test"
  build_type: "Release"
  operation: "flash_monitor"
```
- **Purpose**: Sets default values when parameters are not specified
- **Usage**: Scripts use these values when called without parameters

#### 4. Featured Examples
```yaml
featured_examples:
  - "ascii_art"
  - "gpio_test"
  - "adc_test"
```
- **Purpose**: Examples shown first in help and list commands
- **Usage**: Highlights most commonly used examples

#### 5. Example Definitions
```yaml
examples:
  gpio_test:
    description: "GPIO peripheral testing"
    source_file: "main/GpioExample.cpp"
    featured: true
```
- **description**: Human-readable description of the example
- **source_file**: Main source file for the example
- **featured**: Whether this example appears in featured list

## Configuration Loader (`config_loader.sh`)

### Purpose
The `config_loader.sh` script provides:
- Configuration file parsing and validation
- Helper functions for other scripts
- Configuration querying and validation
- Error handling and reporting

### Usage
```bash
# Source the configuration loader
source ./scripts/config_loader.sh

# Use configuration functions
get_app_types
get_build_types
get_app_description "gpio_test"
```

### Available Functions

#### 1. Configuration Loading
```bash
# Load configuration (called automatically when sourced)
load_configuration

# Check if configuration is valid
validate_configuration
```

#### 2. Example Information
```bash
# Get all example types
get_app_types

# Get featured example types
get_featured_example_types

# Get example description
get_app_description "gpio_test"

# Check if example type is valid
is_valid_example_type "gpio_test"
```

#### 3. Build Information
```bash
# Get available build types
get_build_types

# Check if build type is valid
is_valid_build_type "Release"

# Get default build type
get_default_build_type
```

#### 4. Configuration Values
```bash
# Get target MCU
get_target

# Get default example
get_default_example

# Get default operation
get_default_operation
```

#### 5. Build Directory Management
```bash
# Get build directory for app and build type
get_build_directory "gpio_test" "Release"

# Get project name for app
get_project_name "gpio_test"
```

### Function Examples

#### Getting App Information
```bash
#!/bin/bash
source ./scripts/config_loader.sh

echo "Available apps:"
for app in $(get_app_types); do
    description=$(get_app_description "$app")
    echo "  $app - $description"
done
```

#### Validating Configuration
```bash
#!/bin/bash
source ./scripts/config_loader.sh

if ! validate_configuration; then
    echo "Configuration validation failed"
    exit 1
fi

echo "Configuration is valid"
```

#### Getting Build Information
```bash
#!/bin/bash
source ./scripts/config_loader.sh

echo "Available build types: $(get_build_types)"
echo "Default build type: $(get_default_build_type)"
echo "Target MCU: $(get_target)"
```

## Configuration Integration

### Script Integration
All scripts automatically source and use the configuration:

```bash
# In any script
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$(dirname "${BASH_SOURCE[0]}")/config_loader.sh"

# Configuration is now available
APP_TYPE=${1:-$CONFIG_DEFAULT_APP}
BUILD_TYPE=${2:-$CONFIG_DEFAULT_BUILD_TYPE}
```

### Environment Variables
The configuration system respects environment variables:

```bash
# Override target
export IDF_TARGET="esp32c6"

# Override default app
export CONFIG_DEFAULT_APP="i2c_test"

# Override default build type
export CONFIG_DEFAULT_BUILD_TYPE="Debug"
```

### Configuration Overrides
Runtime configuration can override file-based configuration:

```bash
# Override in script
APP_TYPE=${1:-$CONFIG_DEFAULT_APP}
BUILD_TYPE=${2:-$CONFIG_DEFAULT_BUILD_TYPE}

# Use overridden values
if [ -n "$APP_TYPE" ]; then
    CONFIG_DEFAULT_APP="$APP_TYPE"
fi
```

## Adding New Apps

### 1. Update Configuration File
Add the new app to `app_config.yml`:

```yaml
apps:
  new_app:
    description: "Description of the new app"
    source_file: "NewExample.cpp"
    featured: false  # Set to true if it should be featured
```

### 2. Create Source File
Create the source file in `examples/esp32/main/`:

```cpp
// NewExample.cpp
#include "TestFramework.h"

int main() {
    // Example implementation
    return 0;
}
```

### 3. Update CMakeLists.txt
Ensure the source file is included in the CMake configuration:

```cmake
# In main/CMakeLists.txt
if(APP_TYPE STREQUAL "new_app")
    set(MAIN_SOURCE "NewExample.cpp")
endif()
```

### 4. Test Configuration
Verify the new example is recognized:

```bash
# Check if app appears in list
./scripts/flash_app.sh list

# Try building the app
./scripts/build_app.sh new_app Release
```

## Configuration Validation

### Automatic Validation
The configuration loader automatically validates:

- **File existence** - Configuration file exists and is readable
- **YAML syntax** - Configuration file has valid YAML syntax
- **Required fields** - All required configuration fields are present
- **Data types** - Configuration values have correct types
- **Cross-references** - App references are valid

### Validation Errors
Common validation errors and solutions:

#### 1. YAML Syntax Error
```bash
ERROR: Invalid YAML syntax in app_config.yml
```

**Solution**: Check YAML syntax, ensure proper indentation

#### 2. Missing Required Field
```bash
ERROR: Missing required field 'target' in configuration
```

**Solution**: Add missing field to configuration file

#### 3. Invalid Example Reference
```bash
ERROR: Invalid app type 'invalid_app'
```

**Solution**: Check app name spelling in configuration

### Manual Validation
Validate configuration manually:

```bash
# Source configuration loader
source ./scripts/config_loader.sh

# Validate configuration
if validate_configuration; then
    echo "Configuration is valid"
else
    echo "Configuration validation failed"
    exit 1
fi
```

## Configuration Best Practices

### 1. Use Descriptive Names
```yaml
# Good: Clear, descriptive names
apps:
  gpio_test:
  description: "GPIO testing suite"
    
# Bad: Unclear names
apps:
  test1:
    description: "Test"
```

### 2. Organize Examples Logically
```yaml
# Group related apps
featured_apps:
  - "gpio_test"      # Basic peripheral
  - "adc_test"       # Basic peripheral
  - "i2c_test"       # Communication
  - "spi_test"       # Communication
  - "bluetooth_test" # Advanced features
```

### 3. Keep Descriptions Concise
```yaml
# Good: Clear, concise descriptions
examples:
  gpio_test:
    description: "GPIO peripheral comprehensive testing"
    
# Bad: Too verbose
examples:
  gpio_test:
    description: "This example provides testing of the GPIO peripheral including input/output operations, interrupt handling, and various configuration options for the ESP32 platform"
```

### 4. Use Consistent Naming
```yaml
# Good: Consistent naming convention
examples:
  gpio_test:
  adc_test:
  i2c_test:
  
# Bad: Inconsistent naming
examples:
  gpio_test:
  adc_example:
  i2c_demo:
```

## Advanced Configuration Features

### Conditional Configuration
Use environment variables for conditional configuration:

```bash
#!/bin/bash
# Conditional configuration based on environment
if [ "$CI" = "true" ]; then
    CONFIG_DEFAULT_BUILD_TYPE="Release"
    CONFIG_DEFAULT_EXAMPLE="all"
else
    CONFIG_DEFAULT_BUILD_TYPE="Debug"
    CONFIG_DEFAULT_EXAMPLE="gpio_test"
fi
```

### Dynamic Configuration
Generate configuration dynamically:

```bash
#!/bin/bash
# Generate configuration from available source files
echo "examples:" > app_config.yml
for file in main/*.cpp; do
    example_name=$(basename "$file" .cpp | tr '[:upper:]' '[:lower:]')
    echo "  $example_name:" >> app_config.yml
    echo "    description: \"$(basename "$file" .cpp) example\"" >> app_config.yml
    echo "    source_file: \"$(basename "$file")\"" >> app_config.yml
done
```

### Configuration Inheritance
Use base configurations with overrides:

```yaml
# Base configuration
base_config:
  target: "esp32c6"
  build_types: ["Debug", "Release"]

# Development overrides
development:
  extends: base_config
  defaults:
    build_type: "Debug"
    example: "gpio_test"

# Production overrides
production:
  extends: base_config
  defaults:
    build_type: "Release"
    example: "all"
```

## Troubleshooting

### Common Issues

#### 1. Configuration Not Loading
**Symptoms**: Scripts fail with configuration errors

**Solutions:**
```bash
# Check configuration file exists
ls -la examples/esp32/app_config.yml

# Check file permissions
chmod 644 examples/esp32/app_config.yml

# Validate YAML syntax
python3 -c "import yaml; yaml.safe_load(open('app_config.yml'))"
```

#### 2. Examples Not Recognized
**Symptoms**: New examples don't appear in lists

**Solutions:**
```bash
# Check configuration file syntax
source ./scripts/config_loader.sh
validate_configuration

# Verify example definition
get_app_description "new_example"

# Check source file exists
ls -la examples/esp32/main/NewExample.cpp
```

#### 3. Build Type Errors
**Symptoms**: Invalid build type errors

**Solutions:**
```bash
# Check available build types
get_build_types

# Verify build type in configuration
grep -A 5 "build_types:" app_config.yml

# Check environment variables
echo $CONFIG_DEFAULT_BUILD_TYPE
```

### Debug Mode
Enable debug output for troubleshooting:

```bash
# Enable debug mode
export DEBUG=1

# Source configuration with debug output
source ./scripts/config_loader.sh

# Check configuration loading
load_configuration
```

## Performance Considerations

### Configuration Loading
- **Single load**: Configuration is loaded once when sourced
- **Cached values**: Configuration values are cached in memory
- **Minimal overhead**: Configuration loading adds minimal overhead

### Memory Usage
- **Small footprint**: Configuration data uses minimal memory
- **Efficient storage**: YAML parsing is memory-efficient
- **No leaks**: Configuration data is properly managed

### Startup Time
- **Fast loading**: Configuration loads in milliseconds
- **Lazy evaluation**: Values are evaluated when needed
- **Minimal delay**: No noticeable delay in script execution

## Security Considerations

### Configuration File Security
- **Local storage**: Configuration remains on local machine
- **Permission control**: Configuration file has appropriate permissions
- **No secrets**: Configuration file should not contain sensitive information

### Validation Security
- **Input validation**: All configuration values are validated
- **Type checking**: Configuration values are type-checked
- **Error handling**: Invalid configurations are handled gracefully

### Best Practices
```bash
# Set appropriate permissions
chmod 644 app_config.yml

# Don't include sensitive information
# Use environment variables for secrets

# Validate configuration before use
validate_configuration
```

## Support and Maintenance

### Getting Help
1. **Check configuration file syntax**
2. **Review this documentation**
3. **Check script source code** for detailed comments
4. **Use debug mode**: `export DEBUG=1`

### Reporting Issues
1. **Include configuration file content**
2. **Provide error messages and output**
3. **Specify platform and environment**
4. **Include relevant script output**

### Contributing
1. **Follow existing configuration style**
2. **Add clear descriptions**
3. **Test configuration validation**
4. **Update documentation**

## Version Information

- **Configuration System Version**: 2.0.0
- **YAML Support**: Full YAML 1.2 support
- **Platform Support**: Linux, macOS
- **Last Updated**: August 2025

For additional information, see:
- [Scripts Overview](README_SCRIPTS_OVERVIEW.md)
- [Flash System](README_FLASH_SYSTEM.md)
- [Build System](README_BUILD_SYSTEM.md)
- [Logging System](README_LOGGING_SYSTEM.md)

---

**Navigation**: [← Previous: Logging System](README_LOGGING_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Utility Scripts →](README_UTILITY_SCRIPTS.md)
