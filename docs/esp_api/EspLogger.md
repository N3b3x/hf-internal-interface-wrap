# 📝 EspLogger API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspTemperature](EspTemperature.md) | [Back to ESP API Index](README.md) | [Next: ESP
API Index](README.md)

</div>

---

## Overview

`EspLogger` provides ESP32-specific logging functionality using ESP-IDF's esp_log system.
It implements the `BaseLogger` interface with support for both ESP-IDF Log V1 and Log V2 systems,
offering comprehensive logging capabilities with performance monitoring and multi-output support.

## Features

- **ESP-IDF Integration** - Full support for ESP-IDF Log V1 and Log V2 systems
- **Multi-Output Support** - Console, file, and network logging
- **Performance Monitoring** - Built-in statistics and performance tracking
- **Thread Safety** - Mutex-protected operations for multi-threaded access
- **Configurable Levels** - Dynamic log level configuration
- **Tag-Based Logging** - Organized logging with custom tags
- **Memory Efficient** - Optimized for embedded systems
- **Real-Time Logging** - Low-latency logging for real-time applications

## Header File

```cpp
#include "mcu/esp32/EspLogger.h"
```text

## Class Definition

```cpp
class EspLogger : public BaseLogger {
public:
    // Constructor with configuration
    explicit EspLogger(const hf_logger_config_t& config) noexcept;
    
    // Destructor with proper cleanup
    ~EspLogger() noexcept override;
    
    // BaseLogger interface implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    
    // Logging operations
    hf_logger_err_t Log(hf_log_level_t level, const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t LogV(hf_log_level_t level, const char* tag, const char* format, va_list args) noexcept override;
    
    // Level management
    hf_logger_err_t SetLevel(hf_log_level_t level) noexcept override;
    hf_log_level_t GetLevel() const noexcept override;
    
    // Tag management
    hf_logger_err_t SetTagLevel(const char* tag, hf_log_level_t level) noexcept override;
    hf_log_level_t GetTagLevel(const char* tag) const noexcept override;
    
    // Output management
    hf_logger_err_t AddOutput(hf_logger_output_t output_type, const char* output_config) noexcept override;
    hf_logger_err_t RemoveOutput(hf_logger_output_t output_type) noexcept override;
    
    // Performance and diagnostics
    hf_logger_err_t GetStatistics(hf_logger_statistics_t& statistics) noexcept override;
    hf_logger_err_t GetDiagnostics(hf_logger_diagnostics_t& diagnostics) noexcept override;
    hf_logger_err_t ResetStatistics() noexcept override;
    
    // ESP-IDF specific features
    hf_logger_err_t SetEspLogLevel(esp_log_level_t level) noexcept;
    esp_log_level_t GetEspLogLevel() const noexcept;
    hf_logger_err_t EnableEspLogV2(bool enable) noexcept;
    bool IsEspLogV2Enabled() const noexcept;
};
```text

## Configuration Structure

### Logger Configuration

```cpp
struct hf_logger_config_t {
    hf_log_level_t default_level;                // Default log level
    bool enable_esp_log_v2;                      // Enable ESP-IDF Log V2
    bool enable_performance_monitoring;          // Enable performance tracking
    bool enable_thread_safety;                   // Enable mutex protection
    hf_logger_output_t output_types;             // Output type flags
    char console_config[64];                     // Console output configuration
    char file_config[128];                       // File output configuration
    char network_config[128];                    // Network output configuration
};
```text

## Usage Examples

### Basic Logging

```cpp
#include "mcu/esp32/EspLogger.h"

// Configure logger
hf_logger_config_t config = {};
config.default_level = hf_log_level_t::LOG_INFO;
config.enable_esp_log_v2 = true;
config.output_types = hf_logger_output_t::OUTPUT_CONSOLE;

// Create and initialize logger
EspLogger logger(config);
if (!logger.EnsureInitialized()) {
    printf("Failed to initialize logger\n");
    return;
}

// Basic logging
logger.Log(hf_log_level_t::LOG_INFO, "APP", "Application started");
logger.Log(hf_log_level_t::LOG_ERROR, "APP", "Error occurred: %d", error_code);
logger.Log(hf_log_level_t::LOG_DEBUG, "APP", "Debug info: %s", debug_string);
```text

### Tag-Based Logging

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Set different levels for different tags
logger.SetTagLevel("APP", hf_log_level_t::LOG_INFO);
logger.SetTagLevel("SENSOR", hf_log_level_t::LOG_DEBUG);
logger.SetTagLevel("NETWORK", hf_log_level_t::LOG_WARN);

// Log with different tags
logger.Log(hf_log_level_t::LOG_INFO, "APP", "Application initialized");
logger.Log(hf_log_level_t::LOG_DEBUG, "SENSOR", "Sensor reading: %.2f", sensor_value);
logger.Log(hf_log_level_t::LOG_WARN, "NETWORK", "Network timeout");
```text

### Multi-Output Logging

```cpp
// Configure multi-output logger
hf_logger_config_t config = {};
config.default_level = hf_log_level_t::LOG_INFO;
config.output_types = hf_logger_output_t::OUTPUT_CONSOLE | 
                     hf_logger_output_t::OUTPUT_FILE;
strcpy(config.file_config, "/spiffs/logs/app.log");

EspLogger logger(config);
logger.EnsureInitialized();

// Add network output
logger.AddOutput(hf_logger_output_t::OUTPUT_NETWORK, "udp://192.168.1.100:514");

// Log to all outputs
logger.Log(hf_log_level_t::LOG_INFO, "APP", "Multi-output logging active");
```text

### Performance Monitoring

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Enable performance monitoring
hf_logger_config_t perf_config = config;
perf_config.enable_performance_monitoring = true;
logger.Initialize();

// Perform logging operations
for (int i = 0; i < 1000; i++) {
    logger.Log(hf_log_level_t::LOG_DEBUG, "PERF", "Performance test %d", i);
}

// Get statistics
hf_logger_statistics_t stats;
logger.GetStatistics(stats);
printf("Total logs: %u\n", stats.total_logs);
printf("Successful logs: %u\n", stats.successful_logs);
printf("Average log time: %.2f µs\n", stats.average_log_time_us);
printf("Max log time: %.2f µs\n", stats.max_log_time_us);

// Get diagnostics
hf_logger_diagnostics_t diagnostics;
logger.GetDiagnostics(diagnostics);
printf("Last error: %d\n", diagnostics.last_error_code);
printf("Memory usage: %u bytes\n", diagnostics.memory_usage_bytes);
```text

### ESP-IDF Integration

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Set ESP-IDF log level
logger.SetEspLogLevel(ESP_LOG_INFO);

// Enable ESP-IDF Log V2
logger.EnableEspLogV2(true);

// Use ESP-IDF logging macros (they will use our logger)
ESP_LOGI("APP", "ESP-IDF integration working");
ESP_LOGE("APP", "Error with code: %d", error_code);
ESP_LOGD("APP", "Debug info: %s", debug_string);

// Check ESP-IDF Log V2 status
if (logger.IsEspLogV2Enabled()) {
    printf("ESP-IDF Log V2 is enabled\n");
}
```text

### Dynamic Level Control

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Set global level
logger.SetLevel(hf_log_level_t::LOG_WARN);

// Set tag-specific levels
logger.SetTagLevel("CRITICAL", hf_log_level_t::LOG_ERROR);
logger.SetTagLevel("DEBUG", hf_log_level_t::LOG_DEBUG);

// Log with different levels
logger.Log(hf_log_level_t::LOG_INFO, "APP", "This won't be logged (level too low)");
logger.Log(hf_log_level_t::LOG_WARN, "APP", "This will be logged");
logger.Log(hf_log_level_t::LOG_ERROR, "CRITICAL", "Critical error");
logger.Log(hf_log_level_t::LOG_DEBUG, "DEBUG", "Debug info");

// Get current levels
hf_log_level_t global_level = logger.GetLevel();
hf_log_level_t critical_level = logger.GetTagLevel("CRITICAL");
printf("Global level: %d, Critical level: %d\n", global_level, critical_level);
```text

## Log Levels

| Level | Value | Description |

|-------|-------|-------------|

| `LOG_NONE` | 0 | No logging |

| `LOG_ERROR` | 1 | Error messages only |

| `LOG_WARN` | 2 | Warning and error messages |

| `LOG_INFO` | 3 | Informational, warning, and error messages |

| `LOG_DEBUG` | 4 | Debug, info, warning, and error messages |

| `LOG_VERBOSE` | 5 | All messages including verbose |

## Output Types

| Output Type | Description |

|-------------|-------------|

| `OUTPUT_CONSOLE` | Standard console output |

| `OUTPUT_FILE` | File-based logging |

| `OUTPUT_NETWORK` | Network-based logging (UDP/TCP) |

## Performance Characteristics

- **Log Latency**: <10µs per log entry
- **Memory Usage**: ~2KB base + 1KB per output
- **Throughput**: >10,000 logs/second
- **Thread Safety**: Full mutex protection
- **ESP-IDF Integration**: Native performance

## Error Handling

The `EspLogger` class provides comprehensive error reporting through the `hf_logger_err_t`
enumeration:

- `LOGGER_SUCCESS` - Operation completed successfully
- `LOGGER_ERR_NOT_INITIALIZED` - Logger not initialized
- `LOGGER_ERR_INVALID_LEVEL` - Invalid log level
- `LOGGER_ERR_INVALID_TAG` - Invalid tag
- `LOGGER_ERR_OUTPUT_FAILED` - Output operation failed
- `LOGGER_ERR_MEMORY` - Memory allocation error

## Thread Safety

The `EspLogger` class uses mutex protection for thread-safe operation.
Multiple threads can safely call logging methods simultaneously.

## Related Documentation

- **[BaseLogger API Reference](../api/BaseLogger.md)** - Base class interface
- **[ESP-IDF Logging](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/system/log.html)** - ESP-IDF documentation
- **[DigitalOutputGuard Documentation](../utils/DigitalOutputGuard.md)** - Utility class documentation
## # <<<<<<< Current (Your changes)
<div align="center">

**📋 Navigation**

[← Previous: EspTemperature](EspTemperature.md) | [Back to ESP API Index](README.md) | [Next: ESP
API Index](README.md)

</div>
>>>>>>> Incoming (Background Agent changes)
