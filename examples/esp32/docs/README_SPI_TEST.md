# ESP32-C6 SPI Comprehensive Test Suite Documentation

## Overview

The SPI Comprehensive Test Suite provides extensive validation of the `EspSpi` class for ESP32-C6
platforms using ESP-IDF v5.5+ SPI master driver.
This comprehensive test suite demonstrates complete SPI functionality including bus management,
device lifecycle, data transfer operations, DMA support, clock configuration,
ESP-specific features,
and advanced capabilities with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware with comprehensive validation**
**🎯 Focus: Full-duplex communication, DMA operations, and ESP32-C6 specific optimizations**
**🔧 Updated: Comprehensive SPI documentation with logic analyzer requirements**

## Features Tested

### Core SPI Functionality
- **Bus Management**: Initialization, configuration, and cleanup
- **Device Lifecycle**: Creation, initialization, and removal
- **Data Transfer**: Full-duplex, half-duplex, and read-only operations
- **Transfer Sizes**: 1, 4, 16, 64, 256, and 1024 byte transfers
- **Pattern Generation**: Sequential and alternating data patterns
- **Error Handling**: Comprehensive error condition testing and recovery

### Advanced SPI Features
- **Clock Configuration**: Multiple clock sources and frequency settings
- **SPI Modes**: CPOL/CPHA configuration (Mode 0-3)
- **DMA Operations**: Hardware-accelerated data transfer
- **Multi-Device Support**: Concurrent device management
- **Thread Safety**: RTOS mutex protection and concurrent access
- **Performance Optimization**: Timing and throughput measurement

### ESP32-C6 Specific Features
- **IOMUX Integration**: Direct pin-to-peripheral connections
- **Clock Sources**: PLL*F80M, XTAL, and RC*FAST clock options
- **Host Management**: SPI2*HOST (only general-purpose host on ESP32-C6)
- **Power Management**: Sleep mode compatibility and wake-up
- **Hardware Constraints**: PHASE*0 sampling point limitation

### Direct API Comparison
- **ESP-IDF Direct API**: Pure C-based SPI testing for baseline validation
- **C++ Wrapper Replica**: C++ wrapper comparison for wrapper validation
- **Performance Comparison**: Direct vs wrapper performance analysis

## Hardware Requirements

### ESP32-C6 DevKit-M-1 Pin Layout

The test suite uses the following safe pins on ESP32-C6 DevKit-M-1:

```text
Safe Test Pins:
┌─────────────────────────────────────┐
│ Function            │ GPIO Pin      │
├─────────────────────┼───────────────┤
│ MOSI (Master Out)   │ GPIO 7        │
│ MISO (Master In)    │ GPIO 2        │
│ SCLK (Clock)        │ GPIO 6        │
│ CS (Chip Select)    │ GPIO 21       │
│ Test Progress LED    │ GPIO 14       │
└─────────────────────┴───────────────┘
```text

### Optional Hardware
- **Logic Analyzer**: For signal analysis and timing validation
- **SPI Device**: For real device communication testing
- **Oscilloscope**: For detailed signal analysis

## Test Configuration

### Default Test Settings

```cpp
// GPIO pin configuration
static constexpr hf*pin*num*t TEST*MOSI*PIN = 7;
static constexpr hf*pin*num*t TEST*MISO*PIN = 2;
static constexpr hf*pin*num*t TEST*SCLK*PIN = 6;
static constexpr hf*pin*num*t TEST*CS*PIN = 21;

// SPI host configuration
static constexpr hf*host*id*t SPI*HOST*NUM = static*cast<hf*host*id*t>(1); // SPI2*HOST

// Clock configuration
static constexpr uint32*t SPI*CLOCK*SPEED = 1000000; // 1MHz
static constexpr hf*spi*clock*source*t CLOCK*SOURCE = hf*spi*clock*source*t::PLL*F80M*CLK;

// SPI mode configuration
static constexpr hf*spi*mode*t SPI*MODE = hf*spi*mode*t::MODE*0; // CPOL=0, CPHA=0
```text

### Test Section Configuration

The test suite includes configurable test sections:

```cpp
// Core SPI functionality tests
static constexpr bool ENABLE*CORE*TESTS = true;           // Bus management, device lifecycle
static constexpr bool ENABLE*TRANSFER*TESTS = true;       // Data transfer operations
static constexpr bool ENABLE*PERFORMANCE*TESTS = true;    // Clock speeds, performance
static constexpr bool ENABLE*ADVANCED*TESTS = true;       // ESP-specific features
static constexpr bool ENABLE*STRESS*TESTS = true;         // Error handling, edge cases

// Direct API comparison tests
static constexpr bool ENABLE*ESPIDF*DIRECT*TEST = true;   // Pure ESP-IDF API testing
static constexpr bool ENABLE*ESPIDF*WRAPPER*REPLICA = true; // C++ wrapper comparison
```text

## Test Categories

### Core Functionality (8 tests)
1. **Bus initialization** - Basic setup and configuration validation
2. **Bus deinitialization** - Cleanup and state management verification
3. **Configuration validation** - Clock sources and settings verification
4. **Device creation** - Device lifecycle management
5. **Device management** - Multi-device operations and cleanup
6. **Write operations** - Data transmission validation
7. **Read operations** - Data reception validation
8. **Write-read operations** - Full-duplex communication testing

### Advanced Features (8 tests)
9. **Error handling** - Fault conditions and recovery mechanisms
10. **Timeout handling** - Timing validation and error recovery
11. **Multi-device operations** - Concurrent device access testing
12. **Clock speeds** - Various frequency configuration testing
13. **SPI modes** - CPOL/CPHA configuration validation
14. **ESP-specific features** - Clock sources, power management
15. **Thread safety** - Concurrent access verification
16. **Performance** - Timing and throughput measurement

### Transfer Size Testing (4 tests)
17. **Small transfers** - 1, 4, 16 byte transfer validation
18. **Medium transfers** - 64, 256 byte transfer validation
19. **Large transfers** - 1024 byte transfer validation
20. **Pattern generation** - Sequential and alternating pattern validation

### Direct API Comparison (2 tests)
21. **ESP-IDF direct API** - Pure C-based SPI testing for baseline
22. **C++ wrapper replica** - C++ wrapper comparison for validation

## Expected Output

### Test Suite Header

```text
╔══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-C6 SPI COMPREHENSIVE TEST SUITE                    ║
║                         HardFOC Internal Interface                          ║
╚══════════════════════════════════════════════════════════════════════════════╝

═══════════════════════════════════════════════════════════════════
  ESP-IDF Direct API Test
═══════════════════════════════════════════════════════════════════
[SUCCESS] ESP-IDF direct API test passed

═══════════════════════════════════════════════════════════════════
  C++ Wrapper Replica Test
═══════════════════════════════════════════════════════════════════
[SUCCESS] C++ wrapper replica test passed

═══════════════════════════════════════════════════════════════════
  SPI Core Functionality Tests
═══════════════════════════════════════════════════════════════════
[SUCCESS] Bus initialization tests passed
[SUCCESS] Bus deinitialization tests passed
[SUCCESS] Configuration validation tests passed
[SUCCESS] Device creation tests passed
[SUCCESS] Device management tests passed
[SUCCESS] Write operations tests passed
[SUCCESS] Read operations tests passed
[SUCCESS] Write-read operations tests passed

═══════════════════════════════════════════════════════════════════
  SPI Advanced Features Tests
═══════════════════════════════════════════════════════════════════
[SUCCESS] Error handling tests passed
[SUCCESS] Timeout handling tests passed
[SUCCESS] Multi-device operations tests passed
[SUCCESS] Clock speeds tests passed
[SUCCESS] SPI modes tests passed
[SUCCESS] ESP-specific features tests passed
[SUCCESS] Thread safety tests passed
[SUCCESS] Performance tests passed

═══════════════════════════════════════════════════════════════════
  SPI Transfer Size Tests
═══════════════════════════════════════════════════════════════════
[SUCCESS] Small transfers tests passed
[SUCCESS] Medium transfers tests passed
[SUCCESS] Large transfers tests passed
[SUCCESS] Pattern generation tests passed

## Test Summary: SPI
Tests Run: 22
Passed: 22  
Failed: 0
Success Rate: 100.00%
```text

## Logic Analyzer Requirements

### Sampling Rate Requirements

For proper SPI signal analysis, ensure your logic analyzer meets these specifications:

#### Minimum Requirements
- **Sampling Rate**: 4x SPI clock frequency
- **Example**: For 1MHz SPI clock → 4MS/s minimum

#### Recommended Requirements
- **Sampling Rate**: 10x SPI clock frequency
- **Example**: For 1MHz SPI clock → 10MS/s minimum, 40MS/s recommended

### Mathematical Justification

Based on Nyquist-Shannon theorem and digital signal analysis:

#### Nyquist Rate
- **Fundamental**: 2x clock frequency (minimum required)
- **Digital signals**: Contain harmonics up to 5x-10x fundamental frequency
- **Jitter tolerance**: Higher sampling rates reduce timing uncertainty

#### Oversampling Benefits
- **Signal reconstruction**: 4x oversampling provides reliable digital signal capture
- **Noise immunity**: Higher sampling rates improve signal-to-noise ratio
- **Timing accuracy**: Better resolution for edge detection and timing analysis

### Key Considerations

#### Hardware Requirements
- **Memory depth**: Ensure sufficient capture length for complete transfers
- **Probe quality**: High-quality probes reduce signal degradation
- **Ground connection**: Proper grounding essential for signal integrity
- **Bandwidth**: Logic analyzer bandwidth should exceed sampling rate

#### Signal Analysis
- **Clock signal**: Verify clean clock edges and proper frequency
- **Data signals**: Check for proper data timing and levels
- **CS signal**: Validate chip select timing and behavior
- **Timing relationships**: Ensure proper setup and hold times

## Running the Tests

### Build and Flash

```bash
## From project root
cd examples/esp32

## Build the SPI test suite
./scripts/build*example.sh main Release

## Flash and monitor
./scripts/flash*example.sh main Release flash*monitor
```text

### Test Execution

The test suite runs automatically with the following sequence:

1. **ESP-IDF Direct API Test** - Pure C-based validation
2. **C++ Wrapper Replica Test** - C++ wrapper comparison
3. **Core Functionality Tests** - Basic SPI operations
4. **Advanced Features Tests** - ESP-specific capabilities
5. **Transfer Size Tests** - Various data transfer sizes
6. **Test Summary** - Complete results and statistics

### Monitoring Output

```bash
## Monitor test execution
./scripts/flash*example.sh main Release monitor

## View detailed logs
./scripts/flash*example.sh main Release monitor --log
```text

## Troubleshooting

### Common Issues

#### Build Errors
- **ESP-IDF not sourced**: Ensure `export.sh` is sourced
- **Missing dependencies**: Check component requirements
- **Version mismatch**: Verify ESP-IDF v5.5+ compatibility

#### Runtime Errors
- **GPIO conflicts**: Verify pin assignments don't conflict
- **Memory issues**: Check available heap and stack sizes
- **Clock configuration**: Validate clock source and frequency settings

#### Logic Analyzer Issues
- **Sampling rate too low**: Increase to 4x-10x clock frequency
- **Signal quality**: Check probe connections and grounding
- **Memory depth**: Ensure sufficient capture length

### Debug Information

The test suite provides comprehensive debug output:

- **Test section headers**: Clear separation between test categories
- **Individual test results**: Success/failure status for each test
- **Performance metrics**: Timing and throughput measurements
- **Error details**: Specific error codes and descriptions

## Performance Characteristics

### Transfer Performance

| **Transfer Size** | **Expected Performance** | **Notes** |

|-------------------|--------------------------|-----------|

| **1-16 bytes** | < 100 μs | Small transfer optimization |

| **64-256 bytes** | < 500 μs | Medium transfer with DMA |

| **1024 bytes** | < 2 ms | Large transfer with DMA |

### Clock Performance

| **Clock Source** | **Maximum Frequency** | **Notes** |

|------------------|----------------------|-----------|

| **PLL*F80M** | 80 MHz | Maximum performance |

| **XTAL** | 40 MHz | Stable, crystal-based |

| **RC*FAST** | ~17.5 MHz | Low-power, approximate |

## For Complete Documentation

**📖 See [EspSpi.md](../../../docs/esp_api/EspSpi.md) for:**

- Complete API reference and implementation details
- Detailed test descriptions and validation procedures
- Usage examples and best practices
- Clock configuration and optimization details
- SPI mode explanations and timing requirements
- Performance characteristics and benchmarking
- Advanced features and ESP-specific capabilities
- Troubleshooting guide and common solutions

## Navigation

### **Documentation Structure**

- **[🏠 Main Documentation](../../../docs/README.md)** - Complete system overview
- **[📋 API Interfaces](../../../docs/api/README.md)** - Base classes and interfaces
- **[🔧 ESP32 Implementations](../../../docs/esp_api/README.md)** - Hardware-specific implementations
- **[🧪 Test Suites](README.md)** - Testing and validation overview

### **Related Documentation**

- **[EspSpi Implementation](../../../docs/esp_api/EspSpi.md)** - ESP32-C6 SPI implementation
- **[BaseSpi API Reference](../../../docs/api/BaseSpi.md)** - Abstract SPI interface
- **[Hardware Types](../../../docs/api/HardwareTypes.md)** - Type definitions
- **[ESP-IDF SPI Master Driver](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/spi_master.html)** - Official ESP-IDF documentation

### **Navigation Links**

- **[⬅️ Previous: I2C Tests](README_I2C_TEST.md)** - I2C functionality testing
- **[➡️ Next: UART Tests](README_UART_TESTING.md)** - Serial communication testing
- **[🔙 Back to Test Suites](README.md)** - Test suites overview
