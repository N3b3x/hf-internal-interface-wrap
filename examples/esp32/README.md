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

### Prerequisites
```bash
# ESP-IDF v5.5+ installation required
. $IDF_PATH/export.sh

# Set target platform
export IDF_TARGET=esp32c6
```

### Build and Flash Any Test
```bash
# Navigate to examples directory
cd examples/esp32

# Build specific test (replace 'gpio_test' with desired test)
idf.py build -DEXAMPLE_TYPE=gpio_test -DBUILD_TYPE=Release

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Using Build Scripts (Recommended)
```bash
# Build with optimization
./scripts/build_example.sh gpio_test Release

# Flash to device
idf.py -B build_gpio_test_Release flash monitor
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

### Available Scripts
- `build_example.sh` - Build examples with centralized configuration
- `flash_example.sh` - Flash and monitor with auto-build capability  
- `config_loader.sh` - Configuration helper functions
- `get_example_info.py` - Python script for CMake integration
- `setup_ci.sh` - CI environment setup

## 🚀 Usage Examples

### Build Specific Test
```bash
# GPIO test
./scripts/build_example.sh gpio_test Release
idf.py -B build_gpio_test_Release flash monitor

# PIO test with debug
./scripts/build_example.sh pio_test Debug
idf.py -B build_pio_test_Debug flash monitor

# NVS test
./scripts/build_example.sh nvs_test Release
idf.py -B build_nvs_test_Release flash monitor
```

### List Available Examples
```bash
./scripts/build_example.sh list
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

## 🤝 Contributing

When adding new tests or modifying existing ones:

1. **Follow Test Patterns**: Use the established test framework in `TestFramework.h`
2. **Add Configuration**: Update `examples_config.yml` with test metadata
3. **Create Documentation**: Add detailed README in `docs/` directory
4. **Update This Guide**: Add test information to the matrix and checklists
5. **CI Integration**: Ensure test works in CI pipeline

## 📄 License

Copyright HardFOC - Internal Interface Testing Suite


