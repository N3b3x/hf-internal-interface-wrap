# ESP32-C6 Comprehensive Test Suites

This directory contains comprehensive test suites for the ESP32-C6 HardFOC library components, specifically focusing on `EspSpi` and `EspI2C` implementations.

## Overview

The test suites provide thorough validation of all ESP32-C6 peripheral functionality using ESP-IDF v5.5+ features. Each test suite follows a standardized framework with enhanced reporting, memory validation, and performance measurement capabilities.

## Test Suites

### 1. SPI Comprehensive Test Suite (`SpiComprehensiveTest.cpp`)

**Coverage:**
- **Bus Management**: Initialization, configuration validation, multiple bus support
- **Device Operations**: Creation, configuration variations, multiple devices per bus
- **Transfer Modes**: Write-only, read-only, full-duplex transfers
- **Data Transfer**: Various data sizes (1-4092 bytes), different transfer patterns
- **Bus Acquisition**: Exclusive bus access, concurrent device operations
- **Error Handling**: Invalid parameters, timeout scenarios, resource exhaustion
- **Performance Testing**: Throughput measurement, latency analysis, clock frequency testing
- **Resource Management**: Device removal, bus cleanup, memory leak detection

**Key Features Tested:**
- ESP-IDF v5.5+ SPI Master driver API
- DMA-accelerated transfers
- IOMUX optimization for high-frequency operations
- Transaction queuing with interrupt handling
- Clock source configuration (APB, XTAL)
- Advanced timing control with input delay compensation
- Multi-device management with different configurations

### 2. I2C Comprehensive Test Suite (`I2cComprehensiveTest.cpp`)

**Coverage:**
- **Bus Management**: Initialization, configuration validation, multiple bus support
- **Device Operations**: Creation, configuration variations, multiple devices per bus
- **Communication**: Write, read, and write-read combined operations
- **Device Discovery**: Bus scanning, device probing, address validation
- **Error Handling**: Invalid parameters, timeout scenarios, NACK handling
- **Statistics Tracking**: Operation counters, timing measurements, error diagnostics
- **Performance Testing**: Throughput at different clock speeds, latency analysis
- **Resource Management**: Device removal, bus reset, memory validation

**Key Features Tested:**
- ESP-IDF v5.5+ I2C Master driver API
- Bus-device architecture with proper resource management
- Multiple clock speeds (Standard, Fast, Fast+)
- 7-bit and 10-bit addressing support
- Clock stretching and glitch filtering
- Per-device statistics and diagnostics
- Multi-master operation capability

## Test Framework Features

### Enhanced TestFramework.h

The shared test framework provides:

**Core Features:**
- Standardized test execution with `RUN_TEST()` macro
- Automatic timing measurement and result tracking
- Memory leak detection and heap integrity validation
- Comprehensive error reporting with enhanced formatting

**Advanced Features:**
- **Test Grouping**: Organize related tests with `START_TEST_GROUP()`, `RUN_GROUP_TEST()`, `END_TEST_GROUP()`
- **Performance Measurement**: Throughput and latency analysis with `PerformanceMeasurement` class
- **Memory Validation**: Heap integrity checks and leak detection with `MemoryValidator` class
- **Stress Testing**: Resource exhaustion testing with `StressTestHelper` class
- **System Information**: Detailed system info logging for debugging

### Memory Management

- **Heap Tracking**: Monitor memory usage before/after each test
- **Leak Detection**: Automatic detection of memory leaks with configurable thresholds
- **Integrity Validation**: ESP32 heap integrity checks to detect corruption
- **Resource Cleanup**: Ensure proper cleanup of all allocated resources

## Running the Tests

### Prerequisites

- ESP32-C6 development board (DevKit-M-1 recommended)
- ESP-IDF v5.5+ installed and configured
- Proper pin connections for SPI/I2C peripherals

### Hardware Setup

#### SPI Test Setup
```
ESP32-C6 Pin | SPI Signal | Connection
-------------|------------|------------
GPIO 10      | MOSI       | Connect to SPI device DI/MOSI
GPIO 9       | MISO       | Connect to SPI device DO/MISO  
GPIO 11      | SCLK       | Connect to SPI device CLK/SCLK
GPIO 12-15   | CS         | Connect to SPI device CS/SS
```

#### I2C Test Setup
```
ESP32-C6 Pin | I2C Signal | Connection
-------------|------------|------------
GPIO 21      | SDA        | I2C SDA with pull-up resistor (4.7kΩ)
GPIO 22      | SCL        | I2C SCL with pull-up resistor (4.7kΩ)
```

### Building and Running

1. **Configure the project:**
   ```bash
   cd examples/esp32
   idf.py menuconfig
   ```

2. **Select the test to run:**
   - Modify the main CMakeLists.txt to include the desired test file
   - Comment out other test main files

3. **Build and flash:**
   ```bash
   idf.py build
   idf.py flash monitor
   ```

### Example Output

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-C6 SPI COMPREHENSIVE TEST SUITE                    ║
║                         HardFOC Internal Interface                          ║
╚══════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_spi_bus_initialization                                        ║
╚══════════════════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED: test_spi_bus_initialization (1.23 ms)

...

╔══════════════════════════════════════════════════════════════════════════════╗
║                           SPI TEST SUMMARY                                  ║
╠══════════════════════════════════════════════════════════════════════════════╣
║ Tests:        14 total,  14 passed,   0 failed                             ║
║ Success:     100.0%                                                         ║
║ Time:        145.67 ms total, 10.40 ms average                             ║
║ Memory:      0 bytes leaked, 245760 bytes minimum free                     ║
╚══════════════════════════════════════════════════════════════════════════════╝
🎉 [SUCCESS] ALL SPI TESTS PASSED!
```

## Test Categories

### 1. Initialization and Configuration Tests
- Basic bus initialization
- Configuration parameter validation
- Multiple bus support
- Error handling for invalid configurations

### 2. Device Management Tests
- Device creation and configuration
- Multiple devices on single bus
- Device removal and cleanup
- Resource management validation

### 3. Communication Tests
- Various transfer modes and sizes
- Different communication patterns
- Timeout and error handling
- Concurrent operations

### 4. Performance and Stress Tests
- Throughput measurement at different speeds
- Latency analysis
- Resource exhaustion testing
- Memory usage optimization

### 5. Robustness Tests
- Error injection and recovery
- Edge case handling
- Hardware fault simulation
- Resource cleanup validation

## Integration with CI/CD

The test suites are designed for automated testing:

- **Return Codes**: Tests return appropriate exit codes for CI systems
- **Structured Output**: Machine-readable test results
- **Timeout Handling**: Configurable test timeouts
- **Memory Validation**: Automatic memory leak detection
- **Performance Regression**: Threshold-based performance validation

## Troubleshooting

### Common Issues

1. **Hardware Connection Errors**
   - Verify pin connections match the defined GPIO assignments
   - Check pull-up resistors for I2C (4.7kΩ recommended)
   - Ensure proper power supply to connected devices

2. **Test Failures**
   - Review detailed error logs in the test output
   - Check memory usage and potential leaks
   - Verify ESP-IDF version compatibility (v5.5+ required)

3. **Performance Issues**
   - Monitor system load and memory usage
   - Adjust test iteration counts for slower systems
   - Verify clock source configuration

### Debug Features

- **Verbose Logging**: Enable detailed ESP_LOG output
- **Memory Tracking**: Monitor heap usage throughout tests
- **System Information**: Detailed chip and system info logging
- **Performance Metrics**: Comprehensive timing and throughput data

## Contributing

When adding new tests:

1. Follow the existing test naming convention
2. Use the standardized `RUN_TEST()` macro
3. Include proper error handling and cleanup
4. Add memory validation for resource-intensive tests
5. Document test coverage and expected behavior
6. Update this README with new test descriptions

## License

Copyright (c) 2025 HardFOC. All rights reserved.