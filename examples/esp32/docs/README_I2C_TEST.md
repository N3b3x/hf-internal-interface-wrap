# ESP32 I2C Comprehensive Test Suite

## Overview

This directory contains the comprehensive I2C test suite (`I2cComprehensiveTest.cpp`) for the ESP32
implementation.
For complete documentation including test details, API reference, and usage examples, see:

**📖 Complete I2C Documentation**

## Quick Test Information

### Test File Location
- **Main Test File**: `examples/esp32/main/I2cComprehensiveTest.cpp`
- **Test Framework**: `examples/esp32/main/TestFramework.h`

### Running the Tests

```bash
## From project root
cd examples/esp32

## Build the I2C test suite
./scripts/build_example.sh i2c_test Release

## Flash and monitor
./scripts/flash_example.sh i2c_test Release flash_monitor
```text

### Test Categories (24 Total)

The comprehensive test suite includes:

#### Core Functionality (10 tests)
1. **Bus initialization** - Basic setup and configuration
2. **Bus deinitialization** - Cleanup and state management  
3. **Configuration validation** - Clock sources and settings
4. **Device creation** - 7-bit and 10-bit addressing
5. **Device management** - Multi-device operations
6. **Device probing** - Device detection
7. **Bus scanning** - Device discovery
8. **Write operations** - Data transmission
9. **Read operations** - Data reception
10. **Write-read operations** - Register access patterns

#### Advanced Features (8 tests)
11. **Error handling** - Fault conditions and recovery
12. **Timeout handling** - Timing validation
13. **Multi-device operations** - Concurrent device access
14. **Clock speeds** - Standard/Fast/Fast+ mode testing
15. **Address modes** - 7-bit vs 10-bit addressing
16. **ESP-specific features** - Clock sources, power management
17. **Thread safety** - Concurrent access verification
18. **Performance** - Timing and throughput measurement

#### New Features (6 tests)
19. **Edge cases** - Boundary conditions and limits
20. **Power management** - Sleep mode compatibility
21. **Async operations** - Non-blocking I2C operations
22. **Async timeout handling** - Async slot management
23. **Async multiple operations** - Sequential async operations  
24. **Index-based access** - Device iteration and access methods

### Expected Output

```text
╔══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                    ║
║                         HardFOC Internal Interface                          ║
╚══════════════════════════════════════════════════════════════════════════════╝

═══════════════════════════════════════════════════════════════════
  I2C Bus Initialization
═══════════════════════════════════════════════════════════════════
[SUCCESS] Bus initialization tests passed

// ... 24 test categories ...

## Test Summary: I2C
Tests Run: 24
Passed: 24  
Failed: 0
Success Rate: 100.00%
```text

### Hardware Requirements

- **ESP32-C6 DevKit-M-1** (or compatible)
- **GPIO Configuration**:
  - SDA: GPIO21
  - SCL: GPIO22
- **Optional**: I2C devices for comprehensive testing

### Test Configuration

The test suite uses these default configurations:

```cpp
// GPIO pins
static constexpr hf_pin_num_t TEST_SDA_PIN = 21;
static constexpr hf_pin_num_t TEST_SCL_PIN = 22;

// Test device addresses
static constexpr uint16_t TEST_DEVICE_ADDR_1 = 0x48; // Common device
static constexpr uint16_t TEST_DEVICE_ADDR_2 = 0x50; // EEPROM
static constexpr uint16_t NONEXISTENT_ADDR = 0x7E;   // Non-existent

// Clock frequencies  
static constexpr uint32_t STANDARD_FREQ = 100000;    // 100kHz
static constexpr uint32_t FAST_FREQ = 400000;        // 400kHz
static constexpr uint32_t FAST_PLUS_FREQ = 1000000;  // 1MHz
```text

## For Complete Documentation

**📖 See Complete I2C Documentation for:**

- Complete API reference
- Detailed test descriptions  
- Usage examples
- Asynchronous operation details
- Index-based access methods
- Performance characteristics
- Best practices and considerations
