# CAN Comprehensive Test Documentation

## Overview

This document describes the comprehensive CAN testing suite for ESP32-C6 with ESP-IDF v5.5 TWAI API
and SN65 transceiver integration.
The test suite validates all EspCan functionality including hardware integration, error handling,
and performance characteristics.

## Hardware Requirements

### Required Components
- **ESP32-C6 DevKit** - Primary microcontroller
- **SN65HVD230/SN65HVD232** - CAN transceiver
- **120Ω termination resistors** - CAN bus termination
- **Jumper wires** - For connections

### Optional Components
- **Second CAN node** - For full bus testing
- **Oscilloscope** - For signal quality analysis

## Wiring Configuration

### ESP32-C6 + SN65 Transceiver
```text
ESP32-C6    SN65HVD230/232
GPIO4   →   CTX (TX)
GPIO5   →   CRX (RX)
3.3V    →   VCC
GND     →   GND
```text

### CAN Bus Connections
```text
SN65 CANH  →  CAN Bus High
SN65 CANL  →  CAN Bus Low
120Ω       →  Between CANH and CANL (termination)
```text

### External Loopback Testing
```text
SN65 CANH  →  120Ω resistor  →  SN65 CANL
```text

## Test Configuration

### Pin Configuration
- **TX Pin**: GPIO4 (configurable)
- **RX Pin**: GPIO5 (configurable)
- **Baud Rate**: 500 kbps (configurable)
- **Progress Indicator**: GPIO14 (toggles after each test)

### Test Sections
The test suite is organized into configurable sections:

```cpp
// Core functionality tests
static constexpr bool ENABLE*CORE*TESTS = true;

// Advanced feature tests  
static constexpr bool ENABLE*ADVANCED*TESTS = true;

// Error handling tests
static constexpr bool ENABLE*ERROR*TESTS = true;

// Performance tests
static constexpr bool ENABLE*PERFORMANCE*TESTS = true;

// SN65 transceiver tests
static constexpr bool ENABLE*TRANSCEIVER*TESTS = true;
```text

## Test Categories

### 1. Core Tests
- **Initialization Test**: Validates CAN controller startup
- **Self-Test Mode**: Tests internal loopback functionality
- **Message Transmission**: Basic send/receive operations

### 2. Advanced Tests
- **Acceptance Filtering**: Hardware message filtering
- **Advanced Timing**: Bit timing configuration

### 3. Error Tests
- **Error Handling**: Comprehensive error detection
- **Bus Recovery**: Recovery from bus-off state

### 4. Performance Tests
- **Batch Transmission**: Multiple message handling
- **High Throughput**: Maximum performance testing

### 5. Transceiver Tests
- **Loopback Comparison**: Internal vs external loopback
- **Physical Loopback**: Real CAN bus testing
- **SN65 Integration**: Transceiver-specific tests
- **Signal Quality**: Communication reliability

## Loopback Modes

### Internal Loopback
- **Configuration**: `enable*loopback = true`
- **Hardware**: TX and RX on same pin (GPIO4)
- **Use Case**: Basic functionality testing
- **Limitations**: No real CAN bus signaling

### External Loopback
- **Configuration**: `enable*loopback = false`
- **Hardware**: CANH → 120Ω → CANL (after transceiver)
- **Use Case**: Real CAN bus testing
- **Advantages**: Tests actual differential signaling

## Test Execution

### Running Tests
```bash
## Build and flash the test
idf.py build flash monitor

## Enable specific test sections in CanComprehensiveTest.cpp
## Modify the ENABLE***TESTS constants as needed
```text

### Monitoring Progress
- **Serial Output**: Detailed test results via UART
- **GPIO14 Indicator**: Toggles after each test completion
- **Test Sections**: 5 blinks indicate test section completion

### Expected Results
- **Success Rate**: >98% for signal quality tests
- **Error Handling**: Proper error detection and recovery
- **Performance**: Meets timing requirements
- **Hardware Integration**: SN65 transceiver functionality

## Troubleshooting

### Common Issues

#### Initialization Failures
- Check GPIO pin configuration
- Verify SN65 power supply (3.3V)
- Ensure proper grounding

#### Message Transmission Errors
- Verify CAN bus termination (120Ω)
- Check transceiver connections
- Monitor error counters

#### Signal Quality Issues
- Check bus termination
- Verify cable quality
- Monitor for electrical interference

### Error Codes
The test suite uses comprehensive error reporting:
- `CAN*SUCCESS`: Operation completed successfully
- `CAN*ERR*BUS*OFF`: Bus-off state detected
- `CAN*ERR*TIMEOUT`: Operation timeout
- `CAN*ERR*HARDWARE*FAULT`: Hardware issue detected

## Performance Metrics

### Expected Performance
- **Message Rate**: Up to 1000 messages/second
- **Latency**: <1ms for single messages
- **Success Rate**: >99% under normal conditions
- **Error Recovery**: <100ms from bus-off

### Monitoring
- **Statistics**: Message counts, error rates
- **Diagnostics**: Bus load, error counters
- **Timing**: Message timestamps, latencies

## Integration Notes

### ESP-IDF v5.5 Compatibility
- Uses modern TWAI node-based API
- Supports advanced timing configuration
- Implements proper error handling

### SN65 Transceiver Features
- 3.3V/5V compatible
- High-speed operation (up to 1 Mbps)
- Built-in protection features
- Low power consumption

## Related Documentation

- [EspCan API Reference](../../../docs/esp_api/EspCan.md)
- [BaseCan API Reference](../../../docs/api/BaseCan.md)
- [ESP-IDF TWAI Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/twai.html)
- [SN65HVD230 Datasheet](https://www.ti.com/lit/ds/symlink/sn65hvd230.pdf)

## Test Results Interpretation

### Success Criteria
- All test sections complete without critical failures
- Error rates within acceptable limits
- Performance metrics meet requirements
- Hardware integration functions correctly

### Failure Analysis
- Check hardware connections
- Verify configuration parameters
- Monitor error counters and statistics
- Review test logs for specific error codes

---

*This documentation reflects the current state of the CAN testing suite as of 2025.*