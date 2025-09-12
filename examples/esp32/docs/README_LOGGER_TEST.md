---
layout: default
title: "🧪 Logger Test"
description: "Logger Test Suite - Logging system and debug output testing"
nav_order: 12
parent: "🧪 Test Documentation"
permalink: /examples/esp32/docs/logger_test/
---

# ESP32-C6 Logger Comprehensive Test Suite

## Overview

The Logger Comprehensive Test Suite provides extensive validation of the `EspLogger` class for
ESP32-C6 platforms using ESP-IDF v5.5+.
This test suite demonstrates complete logging functionality, level management, buffer operations,
statistics, diagnostics,
and ESP-IDF Log V2 features with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware**

## Features Tested

### Core Logging Functionality
- **Basic Logging Operations**: Debug, Info, Warning, Error, and Verbose levels
- **Formatted Logging**: Printf-style formatted output with variable arguments
- **Level Management**: Dynamic log level configuration and filtering
- **Message Length Validation**: Configurable maximum message lengths

### Advanced Features
- **ESP-IDF Log V2 Integration**: Native ESP-IDF logging system compatibility
- **Buffer Logging**: Circular buffer for log message storage and retrieval
- **Location Logging**: File name, line number, and function name tracking
- **Thread Safety**: Multi-threaded logging support with proper synchronization

### Monitoring & Diagnostics
- **Statistics Tracking**: Message counts, error rates, and performance metrics
- **Health Monitoring**: System health checks and diagnostic information
- **Performance Testing**: Logging throughput and latency measurements
- **Error Handling**: Comprehensive error condition testing

### Configuration & Management
- **Dynamic Configuration**: Runtime configuration changes
- **Output Destinations**: UART, file system, and custom output targets
- **Format Options**: Customizable log message formatting
- **Flush Control**: Automatic and manual buffer flushing

## Hardware Requirements

### Supported Platforms
- **Primary Target**: ESP32-C6-DevKitM-1
- **ESP-IDF Version**: v5.5 or later
- **Minimum Flash**: 4MB
- **Minimum RAM**: 256KB

### Connections
- **USB**: For flashing and serial monitoring (built-in USB-JTAG)
- **No External Hardware Required**: All tests use internal peripherals

## Building and Running

### Prerequisites
```bash
## ESP-IDF v5.5+ installation required
. $IDF_PATH/export.sh

## Set target platform
export IDF_TARGET=esp32c6
```text

### Quick Start
```bash
## Navigate to examples directory
cd examples/esp32

## Build Logger test
idf.py build -DEXAMPLE_TYPE=logger_test -DBUILD_TYPE=Release

## Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```text

### Alternative Build Methods

#### Using Build Scripts (Recommended)
```bash
## Source ESP-IDF environment
source /path/to/esp-idf/export.sh

## Build with optimization
./build_example.sh logger_test Release

## Flash to device
idf.py -B build_logger_test_Release flash monitor
```text

#### Debug Build for Development
```bash
## Build with debug symbols and verbose output
idf.py build -DEXAMPLE_TYPE=logger_test -DBUILD_TYPE=Debug

## Run with detailed logging
idf.py -p /dev/ttyUSB0 flash monitor
```text

## Test Categories

### 1. Construction and Initialization Tests
```cpp
bool test_logger_construction() noexcept;
bool test_logger_initialization() noexcept;
```text
- **Validates**: Object creation, memory allocation, initial state
- **Tests**: Default constructor, configuration application, error handling
- **Expected Results**: Clean initialization with proper default values

### 2. Basic Logging Operations
```cpp
bool test_logger_basic_logging() noexcept;
```text
- **Validates**: Core logging functions across all levels
- **Tests**: Debug, Info, Warning, Error, Verbose message output
- **Expected Results**: Proper message formatting and level filtering

### 3. Level Management
```cpp
bool test_logger_level_management() noexcept;
```text
- **Validates**: Dynamic log level configuration
- **Tests**: Level setting, filtering, runtime changes
- **Expected Results**: Messages filtered according to configured levels

### 4. Formatted Logging
```cpp
bool test_logger_formatted_logging() noexcept;
```text
- **Validates**: Printf-style formatted output
- **Tests**: Variable arguments, format specifiers, buffer management
- **Expected Results**: Correctly formatted messages with proper parameter substitution

### 5. ESP-IDF Log V2 Features
```cpp
bool test_logger_log_v2_features() noexcept;
```text
- **Validates**: Integration with ESP-IDF native logging
- **Tests**: ESP-IDF compatibility, performance, feature parity
- **Expected Results**: Seamless integration with existing ESP-IDF logging

### 6. Buffer Logging
```cpp
bool test_logger_buffer_logging() noexcept;
```text
- **Validates**: Circular buffer implementation
- **Tests**: Buffer storage, retrieval, overflow handling
- **Expected Results**: Efficient buffering with proper memory management

### 7. Location Logging
```cpp
bool test_logger_location_logging() noexcept;
```text
- **Validates**: Source code location tracking
- **Tests**: File names, line numbers, function names
- **Expected Results**: Accurate location information in log messages

### 8. Statistics and Diagnostics
```cpp
bool test_logger_statistics_diagnostics() noexcept;
bool test_logger_health_monitoring() noexcept;
```text
- **Validates**: Performance metrics and system health
- **Tests**: Message counting, error tracking, resource monitoring
- **Expected Results**: Accurate statistics and health indicators

### 9. Error Handling
```cpp
bool test_logger_error_handling() noexcept;
```text
- **Validates**: Robust error condition handling
- **Tests**: Invalid parameters, resource exhaustion, recovery
- **Expected Results**: Graceful error handling without crashes

### 10. Performance Testing
```cpp
bool test_logger_performance_testing() noexcept;
```text
- **Validates**: Logging system performance
- **Tests**: Throughput, latency, resource usage
- **Expected Results**: Optimal performance within embedded constraints

### 11. Utility Functions
```cpp
bool test_logger_utility_functions() noexcept;
```text
- **Validates**: Supporting functionality
- **Tests**: Helper functions, configuration utilities, status queries
- **Expected Results**: Complete utility function coverage

### 12. Cleanup Operations
```cpp
bool test_logger_cleanup() noexcept;
```text
- **Validates**: Proper resource deallocation
- **Tests**: Destructor behavior, memory cleanup, state reset
- **Expected Results**: Clean shutdown with no resource leaks

## Configuration Options

### Logger Configuration Structure
```cpp
hf_logger_config_t create_test_config() noexcept {
  hf_logger_config_t config = {};
  config.default_level = hf_log_level_t::LOG_LEVEL_INFO;
  config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
  config.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
  config.max_message_length = 512;
  config.buffer_size = 1024;
  config.flush_interval_ms = 100;
  config.enable_thread_safety = true;
  config.enable_performance_monitoring = true;
  return config;
}
```text

### Key Configuration Parameters
- **Max Message Length**: 512 bytes (configurable)
- **Buffer Size**: 1024 bytes (circular buffer)
- **Flush Interval**: 100ms (automatic flushing)
- **Thread Safety**: Enabled for multi-threaded environments
- **Performance Monitoring**: Enabled for statistics collection

### Log Levels

| Level | Description | Usage |

|-------|-------------|-------|

| `LOG_LEVEL_NONE` | No logging | Production with minimal overhead |

| `LOG_LEVEL_ERROR` | Error messages only | Critical error reporting |

| `LOG_LEVEL_WARN` | Warnings and errors | Important system events |

| `LOG_LEVEL_INFO` | Informational messages | General application flow |

| `LOG_LEVEL_DEBUG` | Debug information | Development and troubleshooting |

| `LOG_LEVEL_VERBOSE` | Detailed tracing | Deep debugging |

### Output Destinations
- **UART**: Serial port output (default)
- **FILE**: File system logging
- **BUFFER**: In-memory circular buffer
- **CUSTOM**: User-defined output handlers

## Expected Test Results

### Successful Execution Output
```text
╔══════════════════════════════════════════════════════════════════════════════╗
║                 ESP32-C6 ESPLOGGER COMPREHENSIVE TEST SUITE                 ║
║                         HardFOC Internal Interface                          ║
╚══════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_logger_construction                                           ║
╚══════════════════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED: test_logger_construction (0.12 ms)

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_logger_initialization                                         ║
╚══════════════════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED: test_logger_initialization (0.85 ms)

... (additional tests) ...

=== ESPLOGGER TEST SUMMARY ===
Total: 14, Passed: 14, Failed: 0, Success: 100.00%, Time: 156.32 ms
[SUCCESS] ALL ESPLOGGER TESTS PASSED!
```text

### Performance Metrics
Typical performance on ESP32-C6 @ 160MHz:
- **Initialization**: <1ms
- **Basic Log Message**: ~10µs
- **Formatted Message**: ~25µs
- **Buffer Operations**: ~5µs
- **Statistics Update**: ~2µs

### Memory Usage
- **Static Memory**: ~1KB for logger instance
- **Dynamic Buffer**: Configurable (default 1KB)
- **Flash Usage**: ~8KB for test code
- **Stack Usage**: ~256 bytes per logging call

## Troubleshooting

### Common Issues

#### Build Failures
```bash
## Missing ESP-IDF environment
source $IDF_PATH/export.sh

## Wrong target platform
idf.py set-target esp32c6

## Dependency issues
idf.py clean
idf.py build
```text

#### Runtime Issues
- **Initialization Failures**: Check UART configuration and permissions
- **Buffer Overflows**: Increase buffer size in configuration
- **Performance Issues**: Reduce log level or disable verbose logging
- **Memory Issues**: Check available heap and stack space

#### Serial Monitor Issues
```bash
## Check port permissions
sudo chmod 666 /dev/ttyUSB0

## Alternative port detection
ls /dev/tty*

## Monitor with specific baudrate
idf.py monitor -p /dev/ttyUSB0 -b 115200
```text

### Debug Mode Configuration
Enable enhanced debugging:
```bash
## Build with debug configuration
idf.py build -DEXAMPLE_TYPE=logger_test -DBUILD_TYPE=Debug -DCONFIG_LOG_LEVEL=5

## Enable verbose ESP-IDF logging
idf.py menuconfig
## Component config → Log output → Default log verbosity → Verbose
```text

## Integration Examples

### Basic Logger Usage
```cpp
#include "mcu/esp32/EspLogger.h"

// Create logger instance
EspLogger logger;

// Initialize with configuration
hf_logger_config_t config = {};
config.default_level = hf_log_level_t::LOG_LEVEL_INFO;
config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;

if (logger.Initialize(config) == hf_logger_err_t::LOGGER_SUCCESS) {
    // Basic logging
    logger.LogInfo("System initialized successfully");
    logger.LogError("Error code: %d", error_code);
    
    // Formatted logging
    logger.LogDebug("Temperature: %.2f°C, Pressure: %d hPa", temp, pressure);
}
```text

### Advanced Features
```cpp
// Buffer logging
std::vector<std::string> buffered_logs;
logger.GetBufferedLogs(buffered_logs);

// Statistics
auto stats = logger.GetStatistics();
ESP_LOGI("LOGGER", "Messages logged: %u, Errors: %u", 
         stats.total_messages, stats.error_count);

// Health monitoring
auto health = logger.GetHealthStatus();
if (health.status == HF_HEALTH_STATUS_OK) {
    ESP_LOGI("LOGGER", "Logger health: OK");
}
```text

## API Reference

### Core Functions
```cpp
// Lifecycle management
hf_logger_err_t Initialize(const hf_logger_config_t& config) noexcept;
hf_logger_err_t Deinitialize() noexcept;
bool IsInitialized() const noexcept;

// Basic logging
void LogDebug(const char* message) noexcept;
void LogInfo(const char* message) noexcept;
void LogWarning(const char* message) noexcept;
void LogError(const char* message) noexcept;
void LogVerbose(const char* message) noexcept;

// Formatted logging
void LogInfo(const char* format, ...) noexcept;
void LogError(const char* format, ...) noexcept;
// ... (similar for other levels)

// Configuration
hf_logger_err_t SetLogLevel(hf_log_level_t level) noexcept;
hf_log_level_t GetLogLevel() const noexcept;
```text

### Advanced Functions
```cpp
// Buffer management
hf_logger_err_t GetBufferedLogs(std::vector<std::string>& logs) noexcept;
hf_logger_err_t FlushBuffers() noexcept;

// Statistics and monitoring
hf_logger_stats_t GetStatistics() const noexcept;
hf_health_status_t GetHealthStatus() const noexcept;

// Utility functions
const char* GetDescription() const noexcept;
hf_logger_err_t ValidateConfiguration(const hf_logger_config_t& config) noexcept;
```text

## Embedded Development Best Practices

### Performance Optimization
- Use appropriate log levels for production
- Configure buffer sizes based on available memory
- Enable thread safety only when needed
- Monitor statistics for performance tuning

### Memory Management
- All functions are `noexcept` - no exception handling overhead
- Circular buffers prevent memory leaks
- Configurable memory allocation
- Stack usage optimization

### Real-time Considerations
- Non-blocking logging operations
- Configurable flush intervals
- Priority-based message handling
- Minimal interrupt latency impact

## CI/CD Integration

The logger test is automatically included in the continuous integration pipeline:

```yaml
matrix:
  example_type: [logger_test, ...]
  build_type: [Release, Debug]
```text

### Automated Testing
- **Build Verification**: Compile-time validation
- **Runtime Testing**: Automated test execution
- **Performance Benchmarking**: Performance regression detection
- **Memory Analysis**: Memory usage validation

## References

- [ESP-IDF Logging Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/system/log.html)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/index.html)
