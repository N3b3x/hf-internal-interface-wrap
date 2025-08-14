# ESP32 HardFOC Interface Wrapper — Comprehensive Test Suite Documentation

Welcome to the ESP32 examples documentation hub. This comprehensive guide provides detailed information about all available tests, their functionality, completion status, and usage instructions.

## 📋 Test Status Overview

### ✅ **Fully Tested & Working** (5 tests)
- [**GPIO Test**](#gpio-comprehensive-test) - GPIO peripheral testing with interrupts, RTC, and advanced features
- [**PIO Test**](#pio-comprehensive-test) - Enhanced PIO/RMT testing with WS2812 LED support and callbacks  
- [**NVS Test**](#nvs-comprehensive-test) - Non-Volatile Storage comprehensive testing
- [**Logger Test**](#logger-comprehensive-test) - Logger system with ESP-IDF Log V2 integration
- [**ASCII Art Test**](#ascii-art-comprehensive-test) - ASCII art generator with full character support

### 🔧 **In Development/Debugging** (12 tests)
- **ADC Test** - Hardware-in-the-loop ADC testing with potentiometer control
- **UART Test** - Serial communication peripheral testing
- **SPI Test** - SPI bus communication testing
- **I2C Test** - I2C bus communication testing  
- **PWM Test** - Pulse Width Modulation testing
- **CAN Test** - Controller Area Network testing
- **Temperature Test** - Built-in temperature sensor testing
- **Timer Test** - Periodic timer functionality testing
- **WiFi Test** - WiFi connectivity testing
- **Bluetooth Test** - Bluetooth communication testing
- **Utils Test** - Utility functions testing
- **ASCII Art Test** - ASCII art generation utilities

## 🎯 Quick Start

### 🚀 **Recommended: Automated Script Workflow**

The ESP32 examples include powerful automation scripts that handle environment setup, dependency management, and streamlined building. **This is the recommended approach for most users.**

#### **Step 1: Repository Setup (One-time)**
```bash
# Navigate to ESP32 examples directory
cd examples/esp32

# Run repository setup (handles ESP-IDF environment, dependencies, target configuration)
./scripts/setup_repo.sh

# What setup_repo.sh does:
# ✅ Validates ESP-IDF installation and version (v5.5+)
# ✅ Sets up environment variables and target (esp32c6)
# ✅ Installs Python dependencies from requirements.txt
# ✅ Validates hardware target compatibility
# ✅ Sets up development environment for optimal performance
# ✅ Creates necessary directories and permissions
```

#### **Step 2: Build Examples**
```bash
# Build any test with automated configuration
./scripts/build_example.sh <test_name> <build_type>

# Examples:
./scripts/build_example.sh gpio_test Release      # GPIO test optimized
./scripts/build_example.sh pio_test Debug        # PIO test with debug symbols
./scripts/build_example.sh nvs_test Release      # NVS test optimized

# What build_example.sh provides:
# ✅ Automatic configuration loading from examples_config.yml
# ✅ Intelligent build directory management (build_<test>_<type>)
# ✅ Optimized compiler flags per build type
# ✅ Dependency validation and error checking
# ✅ Build artifact organization and cleanup
# ✅ Cross-platform compatibility (Linux, macOS, Windows WSL)
```

#### **Step 3: Flash and Monitor**
```bash
# Flash with intelligent device detection and monitoring
./scripts/flash_example.sh <test_name> <build_type> [options]

# Examples:
./scripts/flash_example.sh gpio_test Release              # Auto-detect port and flash
./scripts/flash_example.sh pio_test Debug --port /dev/ttyUSB0  # Specific port
./scripts/flash_example.sh nvs_test Release --monitor     # Flash and start monitoring

# What flash_example.sh provides:
# ✅ Automatic serial port detection (Linux: /dev/ttyUSB*, /dev/ttyACM*)
# ✅ Intelligent baud rate selection based on target
# ✅ Auto-build if binary doesn't exist or is outdated
# ✅ Flash verification and error recovery
# ✅ Integrated serial monitoring with proper terminal setup
# ✅ Support for multiple ESP32 variants and development boards
```

#### **Complete Workflow Example**
```bash
# One-time setup
cd examples/esp32
./scripts/setup_repo.sh

# Build and flash GPIO test
./scripts/build_example.sh gpio_test Release
./scripts/flash_example.sh gpio_test Release --monitor

# Build and flash PIO test with debug
./scripts/build_example.sh pio_test Debug  
./scripts/flash_example.sh pio_test Debug --port /dev/ttyUSB0 --monitor
```

### 📋 **Script Benefits & Features**

#### **setup_repo.sh - Repository Initialization**
- **Environment Validation**: Checks ESP-IDF installation, version compatibility
- **Automatic Configuration**: Sets up target, toolchain, and environment variables
- **Dependency Management**: Installs Python packages, validates tools
- **Cross-Platform**: Works on Linux, macOS, Windows WSL
- **Error Recovery**: Provides helpful error messages and solutions

#### **build_example.sh - Intelligent Building**
- **Centralized Configuration**: Uses `examples_config.yml` for all settings
- **Build Optimization**: Automatic compiler flags per build type
- **Dependency Tracking**: Only rebuilds when necessary
- **Artifact Management**: Organized build directories and cleanup
- **Validation**: Pre-build checks for configuration and dependencies

#### **flash_example.sh - Smart Flashing**
- **Auto-Detection**: Finds ESP32 devices automatically
- **Build Integration**: Builds if needed before flashing
- **Error Handling**: Comprehensive error recovery and reporting
- **Monitoring**: Integrated serial monitor with proper formatting
- **Multi-Device**: Handles multiple connected ESP32 devices

### 🛠️ **Alternative: Raw ESP-IDF Commands**

For users who prefer direct control or need custom configurations, raw `idf.py` commands are fully supported:

#### **Manual Environment Setup**
```bash
# Manual ESP-IDF environment setup
source $IDF_PATH/export.sh
export IDF_TARGET=esp32c6

# Navigate to examples directory
cd examples/esp32
```

#### **Direct Build Commands**
```bash
# Set target (required once per clean workspace)
idf.py set-target esp32c6

# Build specific test with custom configuration
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release

# Alternative: Configure then build
idf.py reconfigure -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Debug
idf.py build

# Clean and rebuild
idf.py clean
idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Release
```

#### **Direct Flash and Monitor**
```bash
# Flash with automatic port detection
idf.py flash

# Flash with specific port
idf.py -p /dev/ttyUSB0 flash

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor

# Monitor only (after flashing)
idf.py -p /dev/ttyUSB0 monitor

# Custom baud rate
idf.py -p /dev/ttyUSB0 -b 921600 flash monitor
```

#### **Advanced Raw ESP-IDF Usage**
```bash
# Build with custom optimization
idf.py build -DEXAMPLE_TYPE=gpio_test -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Menuconfig for detailed configuration
idf.py menuconfig

# Size analysis
idf.py size
idf.py size-components

# Erase flash completely
idf.py erase-flash

# Monitor with filters
idf.py monitor --print_filter="*:I"

# Partition table info
idf.py partition-table
```

### 🤔 **Which Approach to Choose?**

#### **✅ Use Scripts When:**
- You want the fastest, most reliable workflow
- You're new to ESP-IDF or this project
- You want automated error handling and recovery
- You need cross-platform compatibility
- You prefer the "just works" approach

#### **⚙️ Use Raw idf.py When:**
- You need fine-grained control over build process
- You're debugging build system issues
- You want to customize compiler flags or configurations
- You're integrating with external build systems
- You're an experienced ESP-IDF developer

### 📝 **Quick Reference Commands**

#### **🚀 RECOMMENDED: Script Workflow (Automated & Reliable)**
```bash
# === ONE-TIME SETUP ===
cd examples/esp32
./scripts/setup_repo.sh                                    # Complete environment setup

# === DAILY WORKFLOW ===
./scripts/build_example.sh list                            # List all available tests
./scripts/build_example.sh gpio_test Release               # Build GPIO test
./scripts/flash_example.sh gpio_test Release --monitor     # Flash and monitor GPIO test

# === WORKING TESTS (Ready to Use) ===
./scripts/build_example.sh gpio_test Release && ./scripts/flash_example.sh gpio_test Release --monitor
./scripts/build_example.sh pio_test Release && ./scripts/flash_example.sh pio_test Release --monitor  
./scripts/build_example.sh nvs_test Release && ./scripts/flash_example.sh nvs_test Release --monitor
./scripts/build_example.sh logger_test Release && ./scripts/flash_example.sh logger_test Release --monitor
./scripts/build_example.sh ascii_art Release && ./scripts/flash_example.sh ascii_art Release --monitor

# === DEBUG BUILDS ===
./scripts/build_example.sh gpio_test Debug && ./scripts/flash_example.sh gpio_test Debug --monitor
```

#### **⚙️ ALTERNATIVE: Raw ESP-IDF Workflow (Manual Control)**
```bash
# === ENVIRONMENT SETUP ===
source $IDF_PATH/export.sh && export IDF_TARGET=esp32c6    # Load ESP-IDF environment
cd examples/esp32                                          # Navigate to project

# === BUILD AND FLASH ===
idf.py set-target esp32c6                                  # Set target (once per workspace)
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release # Build specific test
idf.py -p /dev/ttyUSB0 flash monitor                      # Flash and monitor

# === WORKING TESTS (Raw Commands) ===
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release && idf.py flash monitor
idf.py build -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Release && idf.py flash monitor
idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Release && idf.py flash monitor
idf.py build -DEXAMPLE_TYPE=logger_test -DBUILD_TYPE=Release && idf.py flash monitor
idf.py build -DEXAMPLE_TYPE=ascii_art -DBUILD_TYPE=Release && idf.py flash monitor

# === ADVANCED COMMANDS ===
idf.py menuconfig                                          # Detailed configuration
idf.py clean && idf.py build -DEXAMPLE_TYPE=gpio_test     # Clean rebuild
idf.py size && idf.py size-components                     # Size analysis
idf.py monitor --print_filter="*:I"                       # Filtered monitoring
```

#### **🎯 Most Common Commands (Copy & Paste Ready)**
```bash
# Script approach (recommended for beginners)
cd examples/esp32 && ./scripts/setup_repo.sh
./scripts/build_example.sh gpio_test Release && ./scripts/flash_example.sh gpio_test Release --monitor

# Raw idf.py approach (for ESP-IDF experts)  
source $IDF_PATH/export.sh && export IDF_TARGET=esp32c6 && cd examples/esp32
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release && idf.py flash monitor
```

## 📊 Detailed Test Coverage Matrix

| Test Name | Category | Hardware Required | APIs Tested | Status | Documentation |
|-----------|----------|------------------|-------------|---------|---------------|
| **GPIO** | Peripheral | None (DevKit pins) | EspGpio, Interrupts, RTC GPIO | ✅ Complete | [GPIO Guide](docs/README_GPIO_TEST.md) |
| **PIO** | Peripheral | Jumper wire (GPIO8→18) | EspPio, RMT, WS2812, Callbacks | ✅ Complete | [PIO Guide](docs/README_PIO_TEST.md) |
| **NVS** | Storage | None | EspNvs, Flash storage, Persistence | ✅ Complete | [NVS Guide](docs/README_NVS_TEST.md) |
| **Logger** | Utility | None | EspLogger, ESP-IDF Log V2, Buffers | ✅ Complete | [Logger Guide](docs/README_LOGGER_TEST.md) |
| **ASCII Art** | Utility | None | AsciiArtGenerator, Character support | ✅ Complete | [ASCII Guide](docs/README_ASCII_ART_TEST.md) |
| **ADC** | Peripheral | 10kΩ pot, resistors | EspAdc, Calibration, Thresholds | 🔧 Debug | [ADC Guide](docs/README_ADC_TESTING.md) |
| **Temperature** | Sensor | None (built-in) | EspTemperature, Monitoring, Calibration | 🔧 Debug | [Temp Guide](docs/README_TEMPERATURE_TEST.md) |
| **Timer** | Peripheral | None | EspPeriodicTimer, ESP Timer API | 🔧 Debug | [Timer Report](docs/TIMER_TEST_REPORT.md) |
| **UART** | Peripheral | Loopback wire | EspUart, Serial comm, Protocols | 🔧 Debug | *In Development* |
| **SPI** | Peripheral | SPI device/loopback | EspSpi, Bus communication | 🔧 Debug | *In Development* |
| **I2C** | Peripheral | I2C device | EspI2c, Bus scanning, Devices | 🔧 Debug | *In Development* |
| **PWM** | Peripheral | LED/scope | EspPwm, Duty cycle, Frequency | 🔧 Debug | *In Development* |
| **CAN** | Peripheral | CAN transceiver | EspCan, Bus communication | 🔧 Debug | *In Development* |
| **WiFi** | Connectivity | WiFi network | EspWifi, Connection, Protocols | 🔧 Debug | *In Development* |
| **Bluetooth** | Connectivity | BLE device | EspBluetooth, Advertising, GATT | 🔧 Debug | *In Development* |
| **Utils** | Utility | None | Utility functions, Helpers | 🔧 Debug | *In Development* |

## 🔍 Working Tests - Detailed Analysis

### GPIO Comprehensive Test
**Status**: ✅ **Fully Working & Tested**

**Hardware**: ESP32-C6 DevKit-M-1 (no external components required)

**Features Tested**:
- ✅ Basic GPIO initialization and configuration
- ✅ Input/output operations and state management
- ✅ Pull resistor configuration (floating, pull-up, pull-down)
- ✅ Pin validation and error handling
- ✅ Interrupt functionality with multiple trigger types
- ✅ Drive capability testing (5mA to 40mA)
- ✅ RTC GPIO support for low-power operations
- ✅ Glitch filter configuration
- ✅ Sleep and wake-up functionality
- ✅ Hold functionality for state retention
- ✅ Stress testing with rapid state changes
- ✅ Concurrent GPIO operations

**Safe Test Pins**: GPIO 0,1,2,3,6,7,8,10,11,16,20,21,23

**API Coverage**: Complete EspGpio class implementation

---

### PIO Comprehensive Test  
**Status**: ✅ **Fully Working & Tested**

**Hardware**: ESP32-C6 DevKit-M-1 + jumper wire (GPIO8 → GPIO18)

**Features Tested**:
- ✅ Constructor/Destructor behavior and resource management
- ✅ Lifecycle management (Initialize/Deinitialize operations)
- ✅ Channel configuration and multi-channel setup
- ✅ Symbol transmission/reception with timing validation
- ✅ Channel-specific callbacks with user data handling
- ✅ Resolution control with nanosecond precision
- ✅ Carrier modulation (38kHz for IR protocols)
- ✅ WS2812 LED protocol testing (built-in RGB LED on GPIO8)
- ✅ Automated loopback testing (TX/RX verification)
- ✅ Logic analyzer patterns for signal analysis
- ✅ Frequency sweep testing
- ✅ ESP32 variant-specific channel validation
- ✅ DMA support and memory block management

**Protocol Support**: WS2812 timing validation, IR carrier modulation

**API Coverage**: Complete EspPio class with advanced RMT features

---

### NVS Comprehensive Test
**Status**: ✅ **Fully Working & Tested**

**Hardware**: ESP32-C6 DevKit-M-1 (no external components required)

**Features Tested**:
- ✅ Initialization and deinitialization (normal and edge cases)
- ✅ U32 operations (set/get, boundary values, overwriting)
- ✅ String operations (basic storage, empty strings, long strings)
- ✅ Blob operations (binary data, large blobs, null bytes)
- ✅ Key management (existence checking, size retrieval, erasure)
- ✅ Commit operations (normal, multiple, uninitialized handling)
- ✅ Statistics and diagnostics (operation counting, error tracking)
- ✅ Metadata operations (description, namespace info, limits)
- ✅ Edge cases (special characters, rapid operations, type overwriting)
- ✅ Stress testing (multiple namespaces, large key counts)

**Storage Types**: U32, String, Blob with full CRUD operations

**API Coverage**: Complete EspNvs class implementation

---

### Logger Comprehensive Test
**Status**: ✅ **Fully Working & Tested**

**Hardware**: ESP32-C6 DevKit-M-1 (no external components required)

**Features Tested**:
- ✅ Construction and initialization with configuration
- ✅ Basic logging operations (Debug, Info, Warning, Error, Verbose)
- ✅ Level management with dynamic configuration
- ✅ Formatted logging with printf-style output
- ✅ ESP-IDF Log V2 integration and compatibility
- ✅ Buffer logging with circular buffer implementation
- ✅ Location logging (file names, line numbers, functions)
- ✅ Statistics and diagnostics (message counts, performance)
- ✅ Health monitoring and system checks
- ✅ Error handling with robust error conditions
- ✅ Performance testing (throughput, latency, resources)
- ✅ Utility functions and configuration management

**Log Levels**: None, Error, Warning, Info, Debug, Verbose

**API Coverage**: Complete EspLogger class with ESP-IDF integration

---

### ASCII Art Comprehensive Test
**Status**: ✅ **Fully Working & Tested**

**Hardware**: ESP32-C6 DevKit-M-1 (no external components required)

**Features Tested**:
- ✅ Basic ASCII art generation (characters, words, phrases)
- ✅ Uppercase conversion for consistent output
- ✅ Special characters (punctuation, mathematical symbols)
- ✅ Numbers and symbols (0-9, operators, special chars)
- ✅ Empty and edge cases (null inputs, long strings)
- ✅ Custom character management (adding, overriding patterns)
- ✅ Character support validation and checking
- ✅ Supported characters list enumeration
- ✅ Complex text generation (multi-word, mixed types)
- ✅ Performance and stability testing

**Character Set**: A-Z, 0-9, punctuation, mathematical symbols, special characters

**API Coverage**: Complete AsciiArtGenerator class implementation

## 🔧 Tests Under Development

### ADC Test (Hardware-in-the-Loop)
**Status**: 🔧 **Debug Phase - API Integration Issues**

**Issues**: 
- ADC calibration API integration needs refinement
- Monitor threshold callback implementation
- Hardware validation circuit verification

**Hardware Required**: 10kΩ potentiometer, voltage divider resistors

**Progress**: ~70% - Core functionality working, refinement needed

---

### Temperature Test (Built-in Sensor)
**Status**: 🔧 **Debug Phase - Sensor Calibration**

**Issues**:
- Internal temperature sensor calibration
- Threshold monitoring callback integration
- Power management integration

**Hardware Required**: None (built-in sensor)

**Progress**: ~60% - Basic readings work, advanced features need debugging

---

### Timer Test (ESP Timer API)
**Status**: 🔧 **Debug Phase - Callback System**

**Issues**:
- Periodic timer callback wrapper implementation
- Period validation and adjustment
- Sleep mode integration

**Hardware Required**: None

**Progress**: ~80% - Most functionality working, edge cases remain

---

### Communication Tests (UART, SPI, I2C)
**Status**: 🔧 **Debug Phase - Protocol Implementation**

**Issues**:
- Bus communication protocol integration
- Device detection and enumeration
- Error handling and recovery

**Hardware Required**: Varies by test (loopback wires, devices)

**Progress**: ~40% - Basic setup working, protocol implementation needed

---

### Connectivity Tests (WiFi, Bluetooth)
**Status**: 🔧 **Debug Phase - Network Stack Integration**

**Issues**:
- Network stack integration
- Connection management
- Security and authentication

**Hardware Required**: WiFi network, BLE devices

**Progress**: ~30% - Framework setup, core implementation needed

## 📁 Project Structure

```
examples/esp32/
├── README.md                           # This comprehensive guide
├── CMakeLists.txt                      # Build configuration
├── examples_config.yml                 # Centralized test configuration
├── requirements.txt                    # Python dependencies
├── sdkconfig                          # ESP-IDF configuration
├── scripts/                           # Build and utility scripts
│   ├── build_example.sh              # Build script for examples
│   ├── flash_example.sh              # Flash and monitor script
│   ├── config_loader.sh              # Configuration helpers
│   └── get_example_info.py           # CMake integration
├── main/                              # Test source files
│   ├── [Test]ComprehensiveTest.cpp   # Individual test implementations
│   ├── TestFramework.h               # Common test framework
│   └── CMakeLists.txt                # Main build configuration
├── components/                        # Custom components
└── docs/                             # Detailed documentation
    ├── README_GPIO_TEST.md           # GPIO test guide
    ├── README_PIO_TEST.md            # PIO test guide
    ├── README_NVS_TEST.md            # NVS test guide
    ├── README_LOGGER_TEST.md         # Logger test guide
    ├── README_ASCII_ART_TEST.md      # ASCII art test guide
    ├── README_ADC_TESTING.md         # ADC test guide
    ├── README_TEMPERATURE_TEST.md    # Temperature test guide
    ├── TIMER_TEST_REPORT.md          # Timer test report
    ├── README_BUILD_SYSTEM.md        # Build system guide
    └── README_TESTING_INFRASTRUCTURE.md # Testing framework
```

## 🛠️ Build System

### Centralized Configuration
All examples are configured through `examples_config.yml`:
- Test definitions and metadata  
- Build configurations (Debug/Release)
- CI/CD integration settings
- Hardware target specifications

### Build Types
- **Release**: Optimized builds (`-O2`, performance focused)
- **Debug**: Debug builds (`-O0 -g3`, development focused)

### 📜 **Available Scripts (Detailed)**

#### **Core Workflow Scripts**
- **`setup_repo.sh`** - One-time repository initialization and environment setup
  - ESP-IDF environment validation and configuration
  - Python dependency installation and verification
  - Target platform setup (ESP32-C6) and toolchain validation
  - Development environment optimization

- **`build_example.sh`** - Intelligent build system with centralized configuration
  - Automated configuration loading from `examples_config.yml`
  - Optimized build flags per build type (Release/Debug)
  - Dependency validation and incremental builds
  - Cross-platform compatibility and error handling

- **`flash_example.sh`** - Smart flashing with device detection and monitoring
  - Automatic ESP32 device detection and port selection
  - Integrated build-on-demand if binaries are missing/outdated
  - Serial monitoring with proper terminal configuration
  - Multi-device support and error recovery

#### **Supporting Scripts** 
- **`config_loader.sh`** - Configuration helper functions for bash scripts
  - YAML parsing utilities for `examples_config.yml`
  - Environment variable management and validation
  - Cross-script configuration sharing

- **`get_example_info.py`** - Python integration script for CMake
  - Example metadata extraction for build system
  - Configuration validation and preprocessing
  - CMake variable generation from YAML configuration

- **`setup_ci.sh`** - Continuous Integration environment setup
  - CI-specific environment configuration and optimization
  - Automated testing pipeline setup
  - Matrix build configuration for multiple test combinations

#### **Script Usage Patterns**
```bash
# Complete workflow with all scripts
./scripts/setup_repo.sh                           # One-time setup
./scripts/build_example.sh gpio_test Release      # Build with config
./scripts/flash_example.sh gpio_test Release      # Flash with automation

# Direct script help
./scripts/build_example.sh --help                 # Show build options
./scripts/flash_example.sh --help                 # Show flash options
./scripts/setup_repo.sh --check                   # Validate environment
```

## 🚀 Usage Examples

### **Recommended Script-Based Workflow**

#### **List Available Tests**
```bash
./scripts/build_example.sh list
# Shows all available tests with descriptions and build types
```

#### **Working Tests (Ready to Use)**
```bash
# GPIO Test - Complete peripheral testing
./scripts/build_example.sh gpio_test Release
./scripts/flash_example.sh gpio_test Release --monitor

# PIO Test - RMT with WS2812 LED (requires GPIO8→GPIO18 jumper)
./scripts/build_example.sh pio_test Release  
./scripts/flash_example.sh pio_test Release --monitor

# NVS Test - Non-volatile storage
./scripts/build_example.sh nvs_test Release
./scripts/flash_example.sh nvs_test Release --monitor

# Logger Test - Logging system
./scripts/build_example.sh logger_test Release
./scripts/flash_example.sh logger_test Release --monitor

# ASCII Art Test - Character generation
./scripts/build_example.sh ascii_art Release
./scripts/flash_example.sh ascii_art Release --monitor
```

#### **Debug Builds for Development**
```bash
# Build with debug symbols and verbose logging
./scripts/build_example.sh gpio_test Debug
./scripts/flash_example.sh gpio_test Debug --monitor

# Multiple tests for development
./scripts/build_example.sh pio_test Debug
./scripts/build_example.sh nvs_test Debug  
./scripts/build_example.sh logger_test Debug
```

#### **Tests Under Development (May Require Debugging)**
```bash
# ADC Test - Hardware-in-the-loop (requires potentiometer setup)
./scripts/build_example.sh adc_test Debug
./scripts/flash_example.sh adc_test Debug --monitor

# Timer Test - Periodic timers (mostly working)
./scripts/build_example.sh timer_test Debug  
./scripts/flash_example.sh timer_test Debug --monitor

# Temperature Test - Built-in sensor
./scripts/build_example.sh temperature_test Debug
./scripts/flash_example.sh temperature_test Debug --monitor
```

### **Alternative: Raw ESP-IDF Commands**

#### **Working Tests with Direct idf.py**
```bash
# Setup environment
source $IDF_PATH/export.sh
export IDF_TARGET=esp32c6
cd examples/esp32

# GPIO Test
idf.py set-target esp32c6
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release
idf.py -p /dev/ttyUSB0 flash monitor

# PIO Test  
idf.py build -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Release
idf.py -p /dev/ttyUSB0 flash monitor

# NVS Test
idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Release
idf.py -p /dev/ttyUSB0 flash monitor
```

#### **Advanced Raw Commands**
```bash
# Clean build with size analysis
idf.py clean
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release  
idf.py size

# Custom build configurations
idf.py build -DEXAMPLE_TYPE=pio_test -DCMAKE_BUILD_TYPE=RelWithDebInfo
idf.py menuconfig  # For detailed configuration

# Monitor with specific filters
idf.py monitor --print_filter="GPIO_Test:*"
```

### CI/CD Integration
All working tests are integrated into CI pipeline:
```yaml
matrix:
  example_type: [gpio_test, pio_test, nvs_test, logger_test, ascii_art]
  build_type: [Release, Debug]
```

## 📋 Test Completion Checklist

### ✅ **Completed Tests (5/17)**
- [x] **GPIO Test** - Full peripheral testing with interrupts and RTC
- [x] **PIO Test** - Complete RMT testing with WS2812 and callbacks  
- [x] **NVS Test** - Comprehensive storage testing with all data types
- [x] **Logger Test** - Full logging system with ESP-IDF integration
- [x] **ASCII Art Test** - Complete character generation and validation

### 🔧 **In Progress Tests (12/17)**
- [ ] **ADC Test** - Hardware-in-the-loop testing (70% complete)
- [ ] **Temperature Test** - Built-in sensor testing (60% complete)
- [ ] **Timer Test** - Periodic timer functionality (80% complete)
- [ ] **UART Test** - Serial communication (40% complete)
- [ ] **SPI Test** - SPI bus communication (40% complete)
- [ ] **I2C Test** - I2C bus communication (40% complete)
- [ ] **PWM Test** - Pulse width modulation (40% complete)
- [ ] **CAN Test** - Controller area network (40% complete)
- [ ] **WiFi Test** - WiFi connectivity (30% complete)
- [ ] **Bluetooth Test** - Bluetooth communication (30% complete)
- [ ] **Utils Test** - Utility functions (50% complete)

## 🎯 Development Priorities

### Phase 1: Complete Hardware Peripheral Tests
1. **ADC Test** - Finalize calibration and monitoring
2. **Timer Test** - Complete callback system implementation
3. **Temperature Test** - Finish sensor integration

### Phase 2: Communication Protocols  
1. **UART Test** - Serial communication protocols
2. **SPI Test** - SPI bus device communication
3. **I2C Test** - I2C device scanning and communication

### Phase 3: Advanced Peripherals
1. **PWM Test** - PWM signal generation and control
2. **CAN Test** - CAN bus communication

### Phase 4: Connectivity
1. **WiFi Test** - WiFi connection and protocols
2. **Bluetooth Test** - BLE advertising and GATT

### Phase 5: Utilities
1. **Utils Test** - Helper functions and utilities

## 📖 Additional Resources

### Core Guides
- [Centralized Configuration System](README_CENTRALIZED_CONFIG.md)
- [Build & Flash Guide](docs/README_BUILD_SYSTEM.md)
- [Testing Infrastructure](docs/README_TESTING_INFRASTRUCTURE.md)

### Hardware References
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP32-C6-DevKitM-1 User Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/hw-reference/esp32c6/user-guide-devkitm-1.html)
- [ESP-IDF v5.5 Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)

### API Documentation
- [ESP-IDF GPIO API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/gpio.html)
- [ESP-IDF RMT API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/rmt.html)
- [ESP-IDF NVS API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/storage/nvs_flash.html)
- [ESP-IDF Logging API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/system/log.html)

## 🛠️ **Troubleshooting & Common Issues**

### **Script-Based Workflow Issues**

#### **setup_repo.sh Problems**
```bash
# ESP-IDF not found
export IDF_PATH=/path/to/esp-idf
./scripts/setup_repo.sh

# Permission issues
chmod +x scripts/*.sh
./scripts/setup_repo.sh

# Python dependency issues
pip install -r requirements.txt
./scripts/setup_repo.sh --force-deps
```

#### **build_example.sh Problems**
```bash
# Invalid test name
./scripts/build_example.sh list                    # See available tests

# Configuration issues  
./scripts/build_example.sh gpio_test Release --clean # Clean build
./scripts/build_example.sh gpio_test Release --verbose # Debug output

# Build type issues
./scripts/build_example.sh gpio_test Debug         # Try debug build
```

#### **flash_example.sh Problems**
```bash
# Port detection issues
./scripts/flash_example.sh gpio_test Release --port /dev/ttyUSB0

# Permission issues
sudo usermod -a -G dialout $USER                   # Add user to dialout group
sudo chmod 666 /dev/ttyUSB0                        # Temporary fix

# Device not found
lsusb | grep -i esp                                # Check USB connection
ls /dev/tty*                                       # List available ports
```

### **Raw ESP-IDF Workflow Issues**

#### **Environment Problems**
```bash
# ESP-IDF environment not loaded
source $IDF_PATH/export.sh
export IDF_TARGET=esp32c6

# Target not set
idf.py set-target esp32c6

# Clean environment reset
idf.py fullclean
idf.py set-target esp32c6
```

#### **Build Problems** 
```bash
# Configuration issues
idf.py clean
idf.py reconfigure -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release

# Dependency issues
idf.py menuconfig                                  # Check component config
idf.py build --verbose                             # Debug build process

# Cache issues
rm -rf build/ sdkconfig.old
idf.py set-target esp32c6
idf.py build -DEXAMPLE_TYPE=gpio_test
```

#### **Flash and Monitor Problems**
```bash
# Port issues
idf.py -p /dev/ttyUSB0 flash                      # Specific port
idf.py -p /dev/ttyACM0 flash                      # Alternative port

# Baud rate issues  
idf.py -p /dev/ttyUSB0 -b 115200 flash            # Slower baud rate
idf.py -p /dev/ttyUSB0 -b 921600 flash            # Faster baud rate

# Monitor issues
idf.py -p /dev/ttyUSB0 monitor                    # Monitor only
idf.py monitor --print_filter="*:V"               # Verbose output
```

### **When to Use Which Approach**

#### **✅ Use Scripts For:**
- **First-time users**: Automated setup and error handling
- **Production workflows**: Reliable, tested automation
- **Team development**: Consistent environment and processes
- **CI/CD integration**: Standardized build and test processes
- **Cross-platform development**: Windows WSL, Linux, macOS support

#### **⚙️ Use Raw idf.py For:**
- **Custom build configurations**: Non-standard compiler flags or options
- **Debugging build issues**: Direct access to ESP-IDF build system
- **Integration projects**: Embedding into existing build systems
- **Advanced development**: Fine-grained control over build process
- **ESP-IDF experts**: Leveraging deep ESP-IDF knowledge

### **Performance Comparison**

| Task | Scripts | Raw idf.py | Winner |
|------|---------|------------|---------|
| First-time setup | ~30 seconds | ~5-10 minutes | 🏆 Scripts |
| Regular builds | ~15 seconds | ~15 seconds | 🤝 Tie |
| Error recovery | Automatic | Manual debugging | 🏆 Scripts |
| Customization | Limited | Full control | 🏆 Raw idf.py |
| Learning curve | Minimal | Steep | 🏆 Scripts |
| Cross-platform | Excellent | Good | 🏆 Scripts |

## 🤝 Contributing

When adding new tests or modifying existing ones:

1. **Follow Test Patterns**: Use the established test framework in `TestFramework.h`
2. **Add Configuration**: Update `examples_config.yml` with test metadata
3. **Create Documentation**: Add detailed README in `docs/` directory  
4. **Update This Guide**: Add test information to the matrix and checklists
5. **Script Integration**: Ensure tests work with both script and raw workflows
6. **CI Integration**: Verify tests pass in automated CI pipeline

## 📄 License

Copyright HardFOC - Internal Interface Testing Suite


