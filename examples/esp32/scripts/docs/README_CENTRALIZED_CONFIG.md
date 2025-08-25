# ESP32 Apps - Centralized Configuration System

This document explains the centralized configuration system for ESP32 apps, which provides a single source of truth for all app types, their metadata, and build configurations.

## Overview

The centralized configuration system eliminates the need to maintain app type lists in multiple files across the codebase. Instead of having app types scattered across:
- Build scripts (`build_app.sh`)
- Flash scripts (`flash_app.sh`) 
- CI workflows (`.github/workflows/esp32-component-ci.yml`)
- CMake files (`main/CMakeLists.txt`)

All app definitions are now centralized in a single YAML configuration file: **`app_config.yml`**

Tip: See the docs index at `examples/esp32/docs/README.md`.

## Configuration File Structure

### `app_config.yml`

```yaml
version: "1.0"
metadata:
  project: "ESP32 HardFOC Interface Wrapper"
  default_app: "ascii_art"
  default_build_type: "Release"
  target: "esp32c6"
  idf_versions: ["release/v5.5"]

apps:
  app_name:
    description: "Human-readable description"
    source_file: "SourceFile.cpp"
    category: "peripheral|utility|connectivity|sensor|storage|system"
    build_types: ["Debug", "Release"]
    ci_enabled: true|false
    featured: true|false
    documentation: "optional/path/to/docs.md"

build_config:
  build_types:
    Debug:
      description: "Debug symbols, verbose logging, assertions enabled"
      cmake_build_type: "Debug"
    Release:
      description: "Optimized for performance and size" 
      cmake_build_type: "Release"
  
  build_directory_pattern: "build_{app_type}_{build_type}"
  project_name_pattern: "esp32_iid_{app_type}_app"

ci_config:
  timeout_minutes: 30
  fail_fast: false
  exclude_combinations: []
```

## Helper Scripts and Libraries

### Bash Configuration Loader (`scripts/config_loader.sh`)

Provides functions for bash scripts to access configuration:

```bash
# Source the config loader
source "scripts/config_loader.sh"

# Available functions:
get_app_types                    # Get all app types
get_build_types                     # Get all build types
get_app_description "app"   # Get description
get_app_source_file "app"   # Get source file
is_valid_app_type "app"     # Validate app
is_valid_build_type "type"          # Validate build type
get_build_directory "app" "type"     # Get build directory
get_project_name "app"          # Get project name
get_featured_app_types          # Get featured apps
get_ci_app_types               # Get CI-enabled apps
```

### Python Configuration Scripts

#### `scripts/get_app_info.py`
Used by CMake to get configuration information:

```bash
# Get source file for an app
python3 scripts/get_app_info.py source_file gpio_test

# List all apps
python3 scripts/get_app_info.py list

# Validate app type
python3 scripts/get_app_info.py validate gpio_test
```

#### `.github/workflows/generate_matrix.py`
Generates CI build matrix from configuration:

```bash
# Generate full CI matrix
python3 .github/workflows/generate_matrix.py

# Get only example types
python3 .github/workflows/generate_matrix.py --examples-only

# Get only build types
python3 .github/workflows/generate_matrix.py --build-types-only
```

## Usage Examples

### Adding a New App

1. **Add to configuration file** (`app_config.yml`):
```yaml
apps:
  my_new_test:
    description: "My new peripheral test"
    source_file: "MyNewTest.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: false
```

2. **Create the source file** (`main/MyNewTest.cpp`)

3. **That's it!** The app is now automatically available in:
   - Build scripts: `./scripts/build_app.sh my_new_test`
   - Flash scripts: `./scripts/flash_app.sh my_new_test`
   - CI workflows (will build automatically)
   - CMake (will find source file automatically)

### Building Apps

```bash
# List available apps
./scripts/build_app.sh list
./scripts/flash_app.sh list

# Build specific app
./scripts/build_app.sh gpio_test Release
./scripts/flash_app.sh bluetooth_test Debug flash_monitor
```

### Disabling CI for an App

Simply set `ci_enabled: false` in the configuration:

```yaml
apps:
  experimental_test:
    description: "Experimental test (not ready for CI)"
    ci_enabled: false
    # ... other properties
```

### Making an App Featured

Featured apps appear first in help listings:

```yaml
apps:
  important_test:
    description: "Important test everyone should see"
    featured: true
    # ... other properties
```

## Integration Points

### Build Scripts
- `scripts/build_app.sh` uses config for validation and defaults
- Support `list` command to show all available apps with descriptions

### Flash Scripts  
- `scripts/flash_app.sh` uses config for validation
- Auto-build functionality respects configuration settings

### CI Workflows
- Matrix generation happens dynamically from configuration
- Only `ci_enabled: true` examples are included in CI builds
- Exclude combinations can be specified in config

### CMake
- `main/CMakeLists.txt` queries configuration for source file mapping
- Validation happens at CMake configure time
- Better error messages with available app lists

## Available Apps

The following apps are currently available in the system:

### Featured Apps (displayed first in listings)
- **ascii_art** - ASCII art generator app
- **gpio_test** - GPIO peripheral comprehensive testing
- **adc_test** - ADC peripheral comprehensive testing
- **pio_test** - Comprehensive PIO/RMT testing suite with WS2812 and logic analyzer
- **bluetooth_test** - Comprehensive Bluetooth testing suite
- **utils_test** - Utilities testing suite

### Additional Apps
- **uart_test** - UART peripheral comprehensive testing
- **spi_test** - SPI peripheral comprehensive testing
- **i2c_test** - I2C peripheral comprehensive testing
- **pwm_test** - PWM peripheral comprehensive testing
- **can_test** - CAN peripheral comprehensive testing
- **temperature_test** - Temperature sensor comprehensive testing
- **nvs_test** - NVS (Non-Volatile Storage) comprehensive testing
- **timer_test** - Timer peripheral comprehensive testing
- **logger_test** - Logger system comprehensive testing
- **wifi_test** - WiFi connectivity comprehensive testing

## Benefits

1. **Single Source of Truth**: All example metadata in one place
2. **Easy Maintenance**: Add/remove examples by editing one file
3. **Automatic Integration**: New examples work everywhere immediately
4. **Better Validation**: Consistent validation across all tools
5. **Rich Metadata**: Descriptions, categories, documentation links
6. **Flexible CI**: Easy to enable/disable examples in CI
7. **Backwards Compatibility**: Fallback parsing when tools unavailable

## Troubleshooting

### Missing `yq` Tool

The bash scripts fall back to basic parsing if `yq` is not available:
```bash
# Install yq for better parsing (optional)
sudo wget -qO /usr/local/bin/yq https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64
sudo chmod +x /usr/local/bin/yq
```

### Python Dependencies

For CI matrix generation:
```bash
pip install pyyaml
```

## Migration Guide

When migrating from the old scattered approach:

1. ✅ **Keep existing usage** - All existing build/flash commands work unchanged
2. ✅ **Configuration centralized** - Edit `app_config.yml` instead of multiple files  
3. ✅ **Enhanced features** - Get `list` command, better validation, descriptions
4. ✅ **CI improvements** - Dynamic matrix generation, easy enable/disable

No breaking changes to existing workflows!

## File Structure

The current file structure for the ESP32 examples is:

```
examples/esp32/
├── README_CENTRALIZED_CONFIG.md    # This file
├── app_config.yml             # Centralized configuration
├── CMakeLists.txt                  # ESP-IDF project file
├── sdkconfig                       # ESP-IDF configuration
├── requirements.txt                # Python dependencies
├── main/                           # Example source files
│   ├── CMakeLists.txt
│   ├── TestFramework.h
│   ├── AdcComprehensiveTest.cpp
│   ├── AsciiArtComprehensiveTest.cpp
│   ├── BluetoothComprehensiveTest.cpp
│   ├── CanComprehensiveTest.cpp
│   ├── GpioComprehensiveTest.cpp
│   ├── I2cComprehensiveTest.cpp
│   ├── LoggerComprehensiveTest.cpp
│   ├── NvsComprehensiveTest.cpp
│   ├── PioComprehensiveTest.cpp
│   ├── PwmComprehensiveTest.cpp
│   ├── SpiComprehensiveTest.cpp
│   ├── TemperatureComprehensiveTest.cpp
│   ├── TimerComprehensiveTest.cpp
│   ├── UartComprehensiveTest.cpp
│   ├── UtilsComprehensiveTest.cpp
│   └── WifiComprehensiveTest.cpp
├── scripts/                        # Build and utility scripts
│   ├── build_app.sh
│   ├── config_loader.sh
│   ├── flash_app.sh
│   ├── get_app_info.py
│   ├── setup_ci.sh
│   ├── setup_common.sh
│   └── setup_repo.sh
├── components/                     # ESP-IDF components
│   └── iid-espidf/
└── docs/                          # Documentation
    ├── README.md
    ├── README_BUILD_SYSTEM.md
    ├── README_PIO_TEST.md
    ├── README_TESTING_INFRASTRUCTURE.md
    ├── README_ADC_TESTING.md
    ├── README_NVS_TEST.md
    ├── README_GPIO_TEST.md
    ├── CI_CACHING_STRATEGY.md
    └── TIMER_TEST_REPORT.md
```

