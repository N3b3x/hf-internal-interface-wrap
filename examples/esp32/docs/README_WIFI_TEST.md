# WiFi Comprehensive Test Suite

## Overview

The WiFi Comprehensive Test Suite is a thorough validation system for the ESP32-C6 WiFi implementation. It provides both interface testing and real hardware functionality testing to ensure complete WiFi operation validation.

## Test Architecture

The test suite is organized into four main categories:

### 1. Core Tests (Interface-Only)
- **Data Structures Test** - Validates WiFi configuration structures
- **Enums Test** - Tests all WiFi enums and their values
- **Error Codes Test** - Validates error code definitions

### 2. Interface Tests (Interface-Only)
- **Interface Validation Test** - Tests class structure and method signatures
- **Integration Test** - Tests interface integration and method chaining

### 3. Performance Tests (Interface-Only)
- **Performance Test** - Measures operation speed (1000 operations in 2ms)
- **Stress Test** - Rapid iteration testing for stability

### 4. Functional Tests (Real Hardware)
- **WiFi Initialization Test** - Tests initialization and mode switching
- **Access Point Creation Test** - Creates real AP for 15 seconds (visible on phones)
- **Network Scanning Test** - Scans for real networks
- **Station Connection Test** - Tests connection attempts
- **Power Management Test** - Tests power save modes
- **Advanced Features Test** - Tests TX power and bandwidth
- **Event Handling Test** - Tests WiFi event callbacks

## Test Configuration

Tests can be enabled/disabled by modifying the configuration flags at the top of `WifiComprehensiveTest.cpp`:

```cpp
// Core WiFi interface tests
static constexpr bool ENABLE_CORE_TESTS = true;         // Data structures, enums, error codes
static constexpr bool ENABLE_INTERFACE_TESTS = true;    // Interface validation, integration
static constexpr bool ENABLE_PERFORMANCE_TESTS = true;  // Performance, stress testing
static constexpr bool ENABLE_FUNCTIONAL_TESTS = true;   // Real WiFi functionality tests
```

## Running the Tests

### Build and Flash
```bash
cd examples/esp32
./scripts/build_app.sh wifi_test Release
./scripts/flash_app.sh flash_monitor wifi_test Release
```

### Expected Output

The test suite provides comprehensive logging with ASCII art banners and detailed progress information:

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                ESP32-C6 WIFI COMPREHENSIVE TEST SUITE                        ║
║                       HardFOC Internal Interface                             ║
╚══════════════════════════════════════════════════════════════════════════════╝
```

## Test Results Summary

### Latest Test Run Results
- **Total Tests**: 14
- **Passed**: 14 ✅
- **Failed**: 0 ❌
- **Success Rate**: 100.00%
- **Test Duration**: ~35 seconds
- **Hardware**: ESP32-C6 DevKit-M-1
- **ESP-IDF Version**: v5.5-468-g02c5f2dbb9

### Individual Test Results

#### Core Tests
1. ✅ **Data Structures Test** - 10.44 ms
2. ✅ **Enums Test** - 72.68 ms
3. ✅ **Error Codes Test** - 9.67 ms

#### Interface Tests
4. ✅ **Interface Validation Test** - 11.29 ms
5. ✅ **Integration Test** - 11.47 ms

#### Performance Tests
6. ✅ **Performance Test** - 20.14 ms (1000 operations in 2ms)
7. ✅ **Stress Test** - 10.61 ms

#### Functional Tests
8. ✅ **WiFi Initialization Test** - 978.60 ms
9. ✅ **Access Point Creation Test** - 17.75 seconds (15s AP duration)
10. ✅ **Network Scanning Test** - 2.22 seconds
11. ✅ **Station Connection Test** - 10.70 seconds
12. ✅ **Power Management Test** - 715.11 ms
13. ✅ **Advanced Features Test** - 735.06 ms
14. ✅ **Event Handling Test** - 1.79 seconds

## Real Hardware Validation

### Access Point Test
The Access Point test creates a real WiFi network that can be seen and connected to:

- **Network Name**: "ESP32-C6_TestAP"
- **MAC Address**: **:**:**:**:**:**
- **IP Address**: 192.168.4.1 (default AP subnet)
- **Security**: WPA2-PSK
- **Duration**: 15 seconds (optimized for test framework)
- **Visibility**: ✅ **Confirmed visible on phones/computers**
- **Real Connection**: Ready for external device connection

### Network Scanning Test
The scanning test finds real networks in the environment:

1. **WifiName1** (RSSI: -74 dBm, Channel: 1, WPA2-PSK)
2. **WifiName2** (RSSI: -81 dBm, Channel: 11, WPA2-PSK)

**Note**: Network availability varies by environment and time of testing.

### Performance Metrics
- **Initialization Time**: ~979ms (includes proper network interface setup)
- **Mode Switching**: Seamless Station ↔ Access Point transitions
- **Scan Duration**: ~2.2 seconds for 2 networks
- **Memory Usage**: 30.55% DIRAM, 0.15% LP SRAM
- **Binary Size**: 881KB (43% free space)
- **Network Interface Management**: ✅ **Optimized** - No duplicate interface creation

## Test Framework Features

### Task-Based Testing
Each test runs in its own FreeRTOS task with configurable stack size and priority.

### Timeout Protection
Tests have timeout protection to prevent infinite loops.

### Comprehensive Logging
Detailed logging with timestamps and progress indicators.

### Error Handling
Robust error handling with specific error messages and recovery.

### Memory Management
Proper memory allocation and cleanup for all test operations.

## Network Interface Management

### Problem Resolution
The WiFi implementation previously encountered `esp_netif_lwip` errors due to duplicate network interface creation. This has been **completely resolved** through:

1. **Global Interface Management**: Default network interfaces (`WIFI_STA_DEF`, `WIFI_AP_DEF`) are created once and shared across all `EspWifi` instances
2. **Atomic Synchronization**: Thread-safe flags prevent race conditions during interface creation
3. **Proper Resource Sharing**: Multiple WiFi instances can coexist without conflicts
4. **Optimized Initialization**: Network interfaces are referenced rather than recreated

### Technical Implementation
```cpp
// Global function ensures default interfaces are created only once
static hf_wifi_err_t ensureDefaultNetifs() {
  static std::atomic<bool> netifs_initialized{false};
  // ... implementation details
}
```

## Troubleshooting

### Common Issues

1. **Test Timeout**: If tests timeout, check for hardware issues or increase timeout values
2. **WiFi Initialization Failure**: Ensure ESP32-C6 is properly connected and powered
3. **Network Scanning Issues**: Check for WiFi interference or antenna placement
4. **Access Point Not Visible**: Verify the AP is running and check device WiFi settings

### Debug Information

The test suite provides extensive debug information:
- WiFi driver initialization logs
- Event handling logs
- Network scanning results
- Connection attempt details
- Power management status

## Test Customization

### Adding New Tests
1. Create a new test function following the naming convention `test_wifi_*`
2. Add the test to the appropriate test section
3. Update the test configuration flags if needed

### Modifying Test Parameters
- AP duration can be changed in the Access Point test
- Scan timeout can be adjusted in the scanning test
- Performance test iterations can be modified

### Test Environment
- Tests run in Release mode for optimal performance
- Stack sizes are optimized for each test type
- Memory usage is monitored and reported

## Integration with CI/CD

The test suite is designed to be integrated into continuous integration pipelines:

- **Interface Tests**: Safe for CI environments (no hardware required)
- **Functional Tests**: Require ESP32-C6 hardware for full validation
- **Performance Tests**: Provide baseline metrics for regression testing

## Related Documentation

- [EspWifi API Reference](../../../docs/esp_api/EspWifi.md)
- [BaseWifi API Reference](../../../docs/api/BaseWifi.md)
- [ESP-IDF WiFi Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/network/esp_wifi.html)
