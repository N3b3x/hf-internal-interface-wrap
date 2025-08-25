# ESP32 HardFOC Interface Wrapper - Utility Scripts Documentation

This document provides comprehensive documentation for all utility scripts in the ESP32 scripts directory, including port detection, setup scripts, and helper tools.

---

**Navigation**: [← Previous: Configuration System](README_CONFIG_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Port Detection →](README_PORT_DETECTION.md)

---

## Overview

The utility scripts provide essential support functions for the ESP32 development environment:
- **Port detection and troubleshooting** - Find and validate ESP32 devices
- **Environment setup** - Configure development environment
- **Repository management** - Initialize and maintain the project
- **Information extraction** - Get example and project details

## Port Detection Script (`detect_ports.sh`)

### Purpose
The `detect_ports.sh` script provides comprehensive port detection and troubleshooting for ESP32 devices across different platforms.

### Usage
```bash
./scripts/detect_ports.sh [options]
```

### Available Options

#### Basic Commands
```bash
# List available ports
./scripts/detect_ports.sh

# Verbose output with detailed information
./scripts/detect_ports.sh --verbose

# Test port connectivity
./scripts/detect_ports.sh --test-connection

# Show help
./scripts/detect_ports.sh --help
```

#### Advanced Options
```bash
# Test specific port
./scripts/detect_ports.sh --port /dev/ttyACM0

# Filter by device type
./scripts/detect_ports.sh --filter esp32

# Show USB device information
./scripts/detect_ports.sh --usb-info

# Generate port report
./scripts/detect_ports.sh --report
```

### Output Examples

#### Basic Port Detection
```bash
$ ./scripts/detect_ports.sh
Found 2 ESP32 device(s):

Port: /dev/ttyACM0
  Device: Silicon Labs CP210x USB to UART Bridge
  Status: Available
  Permissions: Readable

Port: /dev/ttyUSB0
  Device: CH340 USB Serial
  Status: Available
  Permissions: Readable
```

#### Verbose Output
```bash
$ ./scripts/detect_ports.sh --verbose
=== ESP32 Port Detection Report ===
Date: 2025-08-25 15:30:00
Platform: Linux (Ubuntu 22.04)
Kernel: 5.15.0-88-generic

USB Devices:
  Bus 001 Device 003: Silicon Labs CP210x USB to UART Bridge
    ID: 10c4:ea60
    Driver: cp210x
    Status: Active

  Bus 001 Device 004: CH340 USB Serial
    ID: 1a86:7523
    Driver: ch341
    Status: Active

Serial Ports:
  /dev/ttyACM0 (CP210x)
    Major: 166, Minor: 0
    Permissions: 666
    Owner: root, Group: dialout
    Status: Readable

  /dev/ttyUSB0 (CH340)
    Major: 188, Minor: 0
    Permissions: 666
    Owner: root, Group: dialout
    Status: Readable

Recommendations:
  - Both ports are available and accessible
  - /dev/ttyACM0 is recommended for ESP32-C6
  - /dev/ttyUSB0 is suitable for older ESP32
```

#### Port Testing
```bash
$ ./scripts/detect_ports.sh --test-connection
Testing port connectivity...

Port: /dev/ttyACM0
  Basic access: ✓ Available
  Read permissions: ✓ Readable
  Write permissions: ✓ Writable
  Serial communication: ✓ Testable
  Status: READY

Port: /dev/ttyUSB0
  Basic access: ✓ Available
  Read permissions: ✓ Readable
  Write permissions: ✓ Writable
  Serial communication: ✓ Testable
  Status: READY

All ports are ready for use!
```

### Platform-Specific Features

#### Linux
- **Device detection**: Uses `lsusb` and `/sys/class/tty`
- **Permission handling**: Automatically detects and reports permission issues
- **Driver information**: Shows loaded USB drivers
- **Kernel messages**: Displays relevant kernel messages

#### macOS
- **Device detection**: Uses `system_profiler` and `/dev` directory
- **Permission handling**: Reports System Preferences requirements
- **Driver status**: Shows USB device enumeration
- **Port validation**: Validates callout vs. terminal devices

### Troubleshooting Features

#### Permission Issues
```bash
$ ./scripts/detect_ports.sh --verbose
Port: /dev/ttyACM0
  Status: Permission denied
  Solution: Run: sudo chmod 666 /dev/ttyACM0
  Alternative: Add user to dialout group: sudo usermod -a -G dialout $USER
```

#### Driver Issues
```bash
$ ./scripts/detect_ports.sh --verbose
USB Device: Silicon Labs CP210x
  Status: No driver loaded
  Solution: Install CP210x driver or update kernel
  Alternative: Use different USB cable/port
```

#### Connection Issues
```bash
$ ./scripts/detect_ports.sh --test-connection
Port: /dev/ttyACM0
  Serial communication: ✗ Failed
  Error: Device not responding
  Solutions:
    1. Check if ESP32 is in download mode
    2. Try different USB cable
    3. Check USB port power
    4. Restart ESP32 device
```

## Setup Common Script (`setup_common.sh`)

### Purpose
The `setup_common.sh` script provides common environment setup for ESP32 development, including ESP-IDF configuration and system preparation.

### Usage
```bash
# Source the script
source ./scripts/setup_common.sh

# Or run directly
./scripts/setup_common.sh
```

### Features

#### ESP-IDF Environment Setup
```bash
# Automatically sources ESP-IDF
source ./scripts/setup_common.sh

# Check if ESP-IDF is available
if command -v idf.py &> /dev/null; then
    echo "ESP-IDF is ready"
else
    echo "ESP-IDF not found"
fi
```

#### Environment Variables
```bash
# Sets common environment variables
export IDF_TARGET="esp32c6"
export IDF_CCACHE_ENABLE=1
export ESPPORT="/dev/ttyACM0"

# Shows current configuration
echo "Target: $IDF_TARGET"
echo "Port: $ESPPORT"
echo "ccache: $IDF_CCACHE_ENABLE"
```

#### System Preparation
```bash
# Checks system requirements
./scripts/setup_common.sh --check-system

# Installs dependencies (Linux)
./scripts/setup_common.sh --install-deps

# Configures user groups
./scripts/setup_common.sh --configure-user
```

### Configuration Options

#### ESP-IDF Paths
```bash
# Custom ESP-IDF path
export ESP_IDF_PATH="/custom/path/to/esp-idf"
source ./scripts/setup_common.sh

# Multiple ESP-IDF versions
export ESP_IDF_PATH_ESP32="/path/to/esp-idf-esp32"
export ESP_IDF_PATH_ESP32C6="/path/to/esp-idf-esp32c6"
source ./scripts/setup_common.sh
```

#### Target Configuration
```bash
# Set specific target
export ESP32_TARGET="esp32c6"
source ./scripts/setup_common.sh

# Multiple targets
export ESP32_TARGETS="esp32 esp32c6 esp32s3"
source ./scripts/setup_common.sh
```

## Setup CI Script (`setup_ci.sh`)

### Purpose
The `setup_ci.sh` script configures the environment for continuous integration and automated testing.

### Usage
```bash
# Setup CI environment
./scripts/setup_ci.sh

# Setup with specific options
./scripts/setup_ci.sh --target esp32c6 --build-type Release
```

### Features

#### Automated Setup
```bash
# Complete CI environment setup
./scripts/setup_ci.sh --auto

# This includes:
# - ESP-IDF installation
# - Toolchain setup
# - Build configuration
# - Test environment
# - Logging setup
```

#### Build Configuration
```bash
# Configure for specific build type
./scripts/setup_ci.sh --build-type Release

# Configure for multiple targets
./scripts/setup_ci.sh --targets esp32 esp32c6

# Configure with specific ESP-IDF version
./scripts/setup_ci.sh --esp-idf-version v5.5.0
```

#### Test Environment
```bash
# Setup test environment
./scripts/setup_ci.sh --test-env

# Configure test parameters
./scripts/setup_ci.sh --test-timeout 300 --test-retries 3

# Setup logging for CI
./scripts/setup_ci.sh --enable-logging
```

### CI-Specific Features

#### Environment Variables
```bash
# Sets CI-specific environment variables
export CI=true
export CI_BUILD_TYPE="Release"
export CI_TARGET="esp32c6"
export CI_LOG_LEVEL="INFO"
export CI_TEST_TIMEOUT=300
```

#### Build Optimization
```bash
# Optimize for CI builds
export IDF_CCACHE_ENABLE=1
export IDF_CCACHE_DIR="/tmp/ccache"
export IDF_CCACHE_SIZE="2G"

# Parallel builds
export IDF_PARALLEL_BUILD=4
export IDF_VERBOSE_BUILD=0
```

#### Logging Configuration
```bash
# CI logging setup
export CI_LOG_DIR="/tmp/ci_logs"
export CI_LOG_LEVEL="INFO"
export CI_LOG_FORMAT="json"

# Archive logs
export CI_LOG_ARCHIVE="/tmp/ci_archives"
export CI_LOG_RETENTION=7
```

## Setup Repository Script (`setup_repo.sh`)

### Purpose
The `setup_repo.sh` script initializes and configures the repository for development.

### Usage
```bash
# Initialize repository
./scripts/setup_repo.sh

# Setup with specific options
./scripts/setup_repo.sh --init --deps --hooks
```

### Features

#### Repository Initialization
```bash
# Complete repository setup
./scripts/setup_repo.sh --init

# This includes:
# - Git configuration
# - Submodule initialization
# - Hook installation
# - Directory structure
# - Configuration files
```

#### Dependency Management
```bash
# Install dependencies
./scripts/setup_repo.sh --deps

# Update dependencies
./scripts/setup_repo.sh --update-deps

# Check dependency status
./scripts/setup_repo.sh --check-deps
```

#### Git Hooks
```bash
# Install Git hooks
./scripts/setup_repo.sh --hooks

# Available hooks:
# - pre-commit: Code formatting and validation
# - pre-push: Build and test validation
# - post-merge: Dependency updates
```

#### Configuration Setup
```bash
# Setup configuration files
./scripts/setup_repo.sh --config

# This creates:
# - app_config.yml (if missing)
# - .gitignore updates
# - Build configuration
# - Environment files
```

### Repository Structure
```bash
# Creates standard directory structure
./scripts/setup_repo.sh --structure

# Directory structure:
examples/esp32/
├── scripts/
│   ├── docs/
│   ├── flash_app.sh
│   ├── build_app.sh
│   └── ...
├── main/
├── components/
├── logs/
└── build_*/
```

## App Information Script (`get_app_info.py`)

### Purpose
The `get_app_info.py` Python script extracts and displays information about available apps and their configurations.

### Usage
```bash
# Show all examples
python3 ./scripts/get_app_info.py

# Show specific example
python3 ./scripts/get_app_info.py gpio_test

# Show with specific format
python3 ./scripts/get_app_info.py --format json
python3 ./scripts/get_app_info.py --format yaml
python3 ./scripts/get_app_info.py --format table
```

### Features

#### Example Information Display
```bash
$ python3 ./scripts/get_app_info.py
ESP32 HardFOC Interface Wrapper - Example Information
====================================================

Featured Examples:
  ascii_art
    Description: ASCII art generator example
    Source: AsciiArtComprehensiveTest.cpp
    Featured: Yes
    
  gpio_test
    Description: GPIO peripheral comprehensive testing
    Source: GpioComprehensiveTest.cpp
    Featured: Yes
    
  adc_test
    Description: ADC peripheral comprehensive testing
    Source: AdcComprehensiveTest.cpp
    Featured: Yes

All Examples (17 total):
  ascii_art, gpio_test, adc_test, uart_test, spi_test, i2c_test,
  pwm_test, can_test, pio_test, temperature_test, nvs_test,
  timer_test, logger_test, wifi_test, bluetooth_test, utils_test
```

#### Detailed Example Information
```bash
$ python3 ./scripts/get_app_info.py gpio_test
Example: gpio_test
==================

Description: GPIO peripheral comprehensive testing
Source File: GpioComprehensiveTest.cpp
Featured: Yes
Build Types: Debug, Release
Target: esp32c6

Source Code Location: examples/esp32/main/GpioComprehensiveTest.cpp
Build Directory: build_gpio_test_{build_type}

Available Operations:
  - flash: Flash firmware to device
  - monitor: Monitor serial output
  - flash_monitor: Flash and monitor (default)

Dependencies:
  - ESP-IDF v5.5+
  - ESP32-C6 target
  - GPIO hardware interface
```

#### Configuration Analysis
```bash
$ python3 ./scripts/get_app_info.py --analyze
Configuration Analysis:
======================

Target Configuration:
  Primary Target: esp32c6
  Supported Targets: esp32, esp32c6, esp32s3
  
Build Configuration:
  Default Build Type: Release
  Available Build Types: Debug, Release
  Build Directory Pattern: build_{example}_{build_type}
  
Example Distribution:
  Featured Examples: 6
  Total Examples: 17
  Coverage: 35.3%
  
Configuration Health:
  ✓ All examples have descriptions
  ✓ All examples have source files
  ✓ Featured examples are properly configured
  ✓ Build types are consistent
```

### Output Formats

#### JSON Format
```bash
$ python3 ./scripts/get_app_info.py --format json
{
  "target": "esp32c6",
  "build_types": ["Debug", "Release"],
  "defaults": {
    "example": "gpio_test",
    "build_type": "Release"
  },
  "examples": {
    "gpio_test": {
      "description": "GPIO peripheral comprehensive testing",
      "source_file": "GpioComprehensiveTest.cpp",
      "featured": true
    }
  }
}
```

#### YAML Format
```bash
$ python3 ./scripts/get_app_info.py --format yaml
target: esp32c6
build_types:
  - Debug
  - Release
defaults:
  example: gpio_test
  build_type: Release
examples:
  gpio_test:
    description: GPIO peripheral comprehensive testing
    source_file: GpioComprehensiveTest.cpp
    featured: true
```

#### Table Format
```bash
$ python3 ./scripts/get_app_info.py --format table
┌─────────────┬─────────────────────────────────────┬─────────────────────┬──────────┐
│ Example     │ Description                         │ Source File         │ Featured │
├─────────────┼─────────────────────────────────────┼─────────────────────┼──────────┤
│ ascii_art   │ ASCII art generator example        │ AsciiArtCompreh... │ Yes      │
│ gpio_test   │ GPIO peripheral comprehensive t... │ GpioComprehensi... │ Yes      │
│ adc_test    │ ADC peripheral comprehensive t...  │ AdcComprehensiv... │ Yes      │
└─────────────┴─────────────────────────────────────┴─────────────────────┴──────────┘
```

## Integration Between Scripts

### Script Dependencies
```bash
# All scripts depend on config_loader.sh
source ./scripts/config_loader.sh

# Port detection is used by flash_app.sh
./scripts/detect_ports.sh --verbose

# Setup scripts prepare environment for others
source ./scripts/setup_common.sh
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### Common Workflows

#### Development Environment Setup
```bash
# 1. Setup repository
./scripts/setup_repo.sh --init --deps

# 2. Setup common environment
source ./scripts/setup_common.sh

# 3. Verify setup
./scripts/detect_ports.sh --verbose
./scripts/get_app_info.py

# 4. Start development
./scripts/flash_app.sh gpio_test Release flash_monitor --log
```

#### CI/CD Environment Setup
```bash
# 1. Setup CI environment
./scripts/setup_ci.sh --auto

# 2. Verify configuration
./scripts/get_app_info.py --analyze

# 3. Run automated tests
./scripts/flash_app.sh all Release flash --log ci_$(date +%Y%m%d_%H%M%S)

# 4. Analyze results
./scripts/manage_logs.sh stats
./scripts/manage_logs.sh search "ERROR"
```

#### Troubleshooting Workflow
```bash
# 1. Check system status
./scripts/detect_ports.sh --verbose --test-connection

# 2. Verify configuration
./scripts/get_app_info.py --analyze

# 3. Check environment
source ./scripts/setup_common.sh
echo "IDF_PATH: $IDF_PATH"
echo "IDF_TARGET: $IDF_TARGET"

# 4. Test with logging
./scripts/flash_app.sh gpio_test Release monitor --log debug_$(date +%H%M)
```

## Advanced Usage

### Custom Port Detection
```bash
#!/bin/bash
# Custom port detection script

# Source the port detection functions
source ./scripts/detect_ports.sh

# Custom port filtering
custom_ports=$(find_esp32_devices | grep -E "(CP210|CH340)")

# Custom port testing
for port in $custom_ports; do
    if test_port_connectivity "$port"; then
        echo "Port $port is ready for use"
    else
        echo "Port $port has issues"
    fi
done
```

### Environment Customization
```bash
#!/bin/bash
# Custom environment setup

# Override default settings
export ESP32_CUSTOM_TARGET="esp32s3"
export ESP32_CUSTOM_BUILD_TYPE="Debug"
export ESP32_CUSTOM_PORT="/dev/ttyUSB0"

# Source setup with custom values
source ./scripts/setup_common.sh

# Verify custom configuration
echo "Custom target: $IDF_TARGET"
echo "Custom build type: $CONFIG_DEFAULT_BUILD_TYPE"
echo "Custom port: $ESPPORT"
```

### Automated Testing
```bash
#!/bin/bash
# Automated testing script

# Setup environment
source ./scripts/setup_common.sh

# Get all examples
examples=$(python3 ./scripts/get_app_info.py --list-only)

# Test each example
for example in $examples; do
    echo "Testing $example..."
    
    # Build and flash with logging
    ./scripts/flash_app.sh "$example" Release flash --log "auto_test_$(date +%Y%m%d_%H%M)"
    
    # Check for errors
    if ./scripts/manage_logs.sh search "ERROR" | grep -q "$example"; then
        echo "ERROR: $example test failed"
        exit 1
    fi
    
    echo "✓ $example test passed"
done

echo "All tests completed successfully!"
```

## Troubleshooting

### Common Issues

#### Port Detection Problems
```bash
# No ports found
./scripts/detect_ports.sh --verbose

# Solutions:
# 1. Check USB connections
# 2. Install drivers
# 3. Check permissions
# 4. Try different USB ports
```

#### Setup Script Failures
```bash
# Setup script fails
./scripts/setup_common.sh --debug

# Solutions:
# 1. Check ESP-IDF installation
# 2. Verify environment variables
# 3. Check system requirements
# 4. Review error messages
```

#### Python Script Issues
```bash
# Python script fails
python3 ./scripts/get_app_info.py --help

# Solutions:
# 1. Check Python version (3.6+)
# 2. Install required packages
# 3. Check file permissions
# 4. Verify configuration file
```

### Debug Mode
Enable debug output for troubleshooting:

```bash
# Enable debug mode
export DEBUG=1

# Run scripts with debug output
./scripts/detect_ports.sh --verbose
./scripts/setup_common.sh
python3 ./scripts/get_app_info.py
```

## Best Practices

### 1. Always Use Port Detection
```bash
# Good: Check ports before flashing
./scripts/detect_ports.sh --verbose
./scripts/flash_app.sh gpio_test Release flash_monitor

# Bad: Assume port availability
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### 2. Setup Environment Properly
```bash
# Good: Source setup scripts
source ./scripts/setup_common.sh
./scripts/flash_app.sh gpio_test Release flash_monitor

# Bad: Skip environment setup
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### 3. Use Information Scripts
```bash
# Good: Check configuration before use
python3 ./scripts/get_app_info.py --analyze
./scripts/flash_app.sh gpio_test Release flash_monitor

# Bad: Assume configuration is correct
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### 4. Regular Maintenance
```bash
# Weekly: Check system status
./scripts/detect_ports.sh --verbose
./scripts/get_app_info.py --analyze

# Monthly: Update dependencies
./scripts/setup_repo.sh --update-deps

# As needed: Clean and rebuild
./scripts/setup_repo.sh --clean
./scripts/setup_repo.sh --init
```

## Support and Maintenance

### Getting Help
1. **Check script help**: `./script_name.sh --help`
2. **Review this documentation**
3. **Check script source code** for detailed comments
4. **Use debug mode**: `export DEBUG=1`

### Reporting Issues
1. **Include script name and version**
2. **Provide error messages and output**
3. **Specify platform and environment**
4. **Include relevant configuration**

### Contributing
1. **Follow existing code style**
2. **Add comprehensive documentation**
3. **Include error handling**
4. **Test across platforms**

## Version Information

- **Utility Scripts Version**: 2.0.0
- **Platform Support**: Linux, macOS
- **Dependencies**: Bash 4.0+, Python 3.6+
- **Last Updated**: August 2025

For additional information, see:
- [Scripts Overview](README_SCRIPTS_OVERVIEW.md)
- [Flash System](README_FLASH_SYSTEM.md)
- [Build System](README_BUILD_SYSTEM.md)
- [Logging System](README_LOGGING_SYSTEM.md)
- [Configuration System](README_CONFIG_SYSTEM.md)

---

**Navigation**: [← Previous: Configuration System](README_CONFIG_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Port Detection →](README_PORT_DETECTION.md)
