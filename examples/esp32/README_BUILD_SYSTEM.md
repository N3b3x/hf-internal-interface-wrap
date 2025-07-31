# ESP32 HardFOC Interface Wrapper - Build Configuration Guide

This document explains how to build and use different examples in the ESP32 HardFOC Interface Wrapper project.

## Overview

The project contains example applications that demonstrate different aspects of the HardFOC system:

1. **Comprehensive Test** - Main integration test of all interfaces
2. **ASCII Art Generator** - Advanced text formatting and display

## Quick Start

### Using the Build Scripts

#### Windows (PowerShell)
```powershell
# Build comprehensive test (default)
.\build_example.ps1

# Build specific examples
.\build_example.ps1 ascii_art Debug
.\build_example.ps1 ascii_art Release
```

#### Linux/macOS (Bash)
```bash
# Build comprehensive test (default)
./build_example.sh

# Build specific examples
./build_example.sh ascii_art Debug
./build_example.sh ascii_art Release
```

## Detailed Examples

### 1. Comprehensive Test (`comprehensive`)
**File:** `main.cpp`
**Description:** Complete integration test of all HardFOC interfaces
**Features:**
- All interface initialization testing
- GPIO, SPI, I2C, UART, PWM, ADC testing
- Error handling verification
- Real-time diagnostics

**Expected Output:**
- ASCII art banners and boxes
- Initialization status for all interfaces
- Continuous status updates

### 2. ASCII Art Generator (`ascii_art`)
**File:** `AsciiArtExample.cpp`
**Description:** Advanced text formatting and display
**Features:**
- Banner creation
- Box drawing with borders
- Color formatting (ANSI codes)
- Text styling (bold, italic, underline)
- Custom character sets

**Build:**
```bash
./build_example.sh ascii_art Release
```

## Build System Architecture

### Flexible CMakeLists.txt

The project uses a flexible CMakeLists.txt system that allows switching between different main files:

```cmake
# Available example types
set(EXAMPLE_SOURCES
    comprehensive "main.cpp"
    ascii_art "AsciiArtExample.cpp"
)
```

### Command Line Usage

```bash
# Manual build with idf.py
idf.py build -DEXAMPLE_TYPE=comprehensive -DBUILD_TYPE=Release
idf.py build -DEXAMPLE_TYPE=ascii_art -DBUILD_TYPE=Debug

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
