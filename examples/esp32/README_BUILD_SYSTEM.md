# ESP32 HardFOC Interface Wrapper - Build Configuration Guide

This document explains how to build and use different examples in the ESP32 HardFOC Interface Wrapper project.

## Overview

The project contains example applications that demonstrate different aspects of the HardFOC system:

1. **ASCII Art Generator** - Advanced text formatting and display
2. **Peripheral Test Suites** - Individual tests for each hardware interface (GPIO, ADC, UART, etc.)
3. **Bluetooth Test Suite** - Comprehensive Bluetooth/NimBLE testing
4. **Utilities Test Suite** - Testing of utility functions, memory utilities, and hardware types

## Quick Start

### Using the Build Scripts

#### Windows (PowerShell)
```powershell
# Build ASCII art example (default)
.\build_example.ps1

# Build specific examples
.\build_example.ps1 ascii_art Debug
.\build_example.ps1 gpio_test Release
.\build_example.ps1 bluetooth_test Release
.\build_example.ps1 utils_test Release
```

#### Linux/macOS (Bash)
```bash
# Build ASCII art example (default)
./build_example.sh

# Build specific examples
./build_example.sh ascii_art Debug
./build_example.sh gpio_test Release
./build_example.sh bluetooth_test Debug
./build_example.sh utils_test Release
```

## Detailed Examples

### 1. ASCII Art Generator (`ascii_art`)
**File:** `AsciiArtComprehensiveTest.cpp`
**Description:** Comprehensive testing suite for the AsciiArtGenerator class
**Features:**
- Complete functionality testing of AsciiArtGenerator
- Basic text generation validation
- Uppercase conversion testing
- Special characters and symbols testing
- Numbers and punctuation testing
- Empty and edge case handling
- Custom character management (add/remove/clear)
- Character support validation
- Supported characters list testing
- Complex text generation testing
- Performance and stability testing

**Test Coverage:**
- 10 comprehensive test functions
- All public methods of AsciiArtGenerator
- Edge cases and error conditions
- Memory stability and performance
- Custom character lifecycle management

**Expected Output:**
- Detailed test results for each functionality
- ASCII art examples for various text types
- Performance metrics and timing information
- Success/failure summary with statistics

### 2. Peripheral Test Suites
Individual comprehensive test suites for each hardware interface:

- **GPIO Test** (`gpio_test`) - Digital I/O testing
- **ADC Test** (`adc_test`) - Analog-to-digital conversion
- **UART Test** (`uart_test`) - Serial communication
- **SPI Test** (`spi_test`) - SPI bus communication
- **I2C Test** (`i2c_test`) - I2C bus communication
- **PWM Test** (`pwm_test`) - Pulse-width modulation
- **CAN Test** (`can_test`) - CAN bus communication
- **Temperature Test** (`temperature_test`) - Temperature sensor interface
- **NVS Test** (`nvs_test`) - Non-volatile storage
- **Timer Test** (`timer_test`) - Hardware timer functionality
- **WiFi Test** (`wifi_test`) - WiFi connectivity and features

### 3. Bluetooth Test Suite (`bluetooth_test`)
**File:** `BluetoothComprehensiveTest.cpp`
**Description:** Comprehensive Bluetooth/NimBLE testing
**Features:**
- Device name configuration
- BLE scanning and discovery
- Callback event handling
- Connection state management

### 4. Utilities Test Suite (`utils_test`)
**File:** `UtilsComprehensiveTest.cpp`
**Description:** Testing of utility functions and helper classes
**Features:**
- ASCII art generation testing
- Memory utility function verification
- Hardware type definitions testing

**Build Examples:**
```bash
./build_example.sh ascii_art Release
./build_example.sh gpio_test Release
./build_example.sh bluetooth_test Debug
./build_example.sh utils_test Release
```

## Build System Architecture

### Flexible CMakeLists.txt

The project uses a flexible CMakeLists.txt system that allows switching between different main files:

```cmake
# Available example types
set(EXAMPLE_SOURCES
    comprehensive "main.cpp"
    ascii_art "AsciiArtExample.cpp"
    nimble_test "Esp32c6NimbleTest.cpp"
)
```

### Command Line Usage

```bash
# Manual build with idf.py
idf.py build -DEXAMPLE_TYPE=comprehensive -DBUILD_TYPE=Release
idf.py build -DEXAMPLE_TYPE=ascii_art -DBUILD_TYPE=Debug
idf.py build -DEXAMPLE_TYPE=nimble_test -DBUILD_TYPE=Debug

# Flash and monitor
idf.py flash monitor
```

## Build Types

- **Release** (default): Optimized for performance and size
- **Debug**: Debug symbols, verbose logging, assertions enabled

## Target Configuration

- **Primary Target**: ESP32-C6
- **ESP-IDF Version**: v5.5+
- **Toolchain**: Modern C++17 with ESP-IDF compatibility

## Troubleshooting

### Common Issues

1. **Build fails with missing headers**
   - Ensure ESP-IDF is properly installed and sourced
   - Check that the target is set to ESP32-C6: `idf.py set-target esp32c6`

2. **Flash memory issues**
   - Use `idf.py erase-flash` to clear the device
   - Check partition table configuration

3. **Compilation errors**
   - Verify ESP-IDF version is v5.5 or later
   - Ensure all submodules are initialized: `git submodule update --init --recursive`

### Getting Help

- Check the main project README.md for setup instructions
- Review the component documentation in `docs/`
- Use `idf.py --help` for ESP-IDF specific options

## Advanced Usage

### Custom Builds

You can create custom build configurations by modifying the CMakeLists.txt files or using environment variables:

```bash
# Custom compiler flags
export EXTRA_CXXFLAGS="-DCUSTOM_DEBUG_LEVEL=3"
idf.py build
```

### CI/CD Integration

The project includes GitHub Actions workflows for automated testing:
- Matrix builds across different example types
- Static analysis with cppcheck and clang-tidy  
- Build artifact generation and size analysis

For local CI testing, use the same build commands as the GitHub Actions workflow in `.github/workflows/esp32-component-ci.yml`.
