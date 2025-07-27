# HardFOC Logger System Architecture

## Overview

The HardFOC Logger System provides a comprehensive, platform-agnostic logging solution that integrates seamlessly with the HardFOC hardware abstraction layer. It replaces the previous `ConsolePort` implementation with a more robust, feature-rich logging system that supports multiple log levels, tag-based filtering, performance monitoring, and thread-safe operations.

## Architecture Components

### 1. BaseLogger (Abstract Interface)

**Location**: `inc/base/BaseLogger.h`

The `BaseLogger` class provides the abstract interface for all logging implementations. It defines:

- **Log Levels**: ERROR, WARN, INFO, DEBUG, VERBOSE
- **Output Destinations**: UART, USB, File, Network, Custom
- **Format Options**: Timestamp, Level, Tag, File/Line, Function, Thread ID, Colors
- **Statistics & Diagnostics**: Performance monitoring and health checks
- **Thread Safety**: Thread-safe operations with mutex protection

### 2. EspLogger (ESP32 Implementation)

**Location**: `inc/mcu/esp32/EspLogger.h` and `src/mcu/esp32/EspLogger.cpp`

The `EspLogger` class provides the ESP32-specific implementation that:

- **Integrates with ESP-IDF**: Direct integration with `esp_log` system
- **Supports All ESP32 Variants**: C6, Classic, S2, S3, C3, C2, H2
- **Tag-based Filtering**: Runtime log level control per tag
- **Performance Optimized**: Efficient message formatting and buffering
- **Health Monitoring**: Comprehensive health checks and diagnostics

### 3. LoggerManager (Singleton Interface)

**Location**: `inc/utils/LoggerManager.h` and `src/utils/LoggerManager.cpp`

The `LoggerManager` provides a singleton interface that:

- **Centralizes Access**: Single point of access for all logging operations
- **Automatic Initialization**: Lazy initialization with default configuration
- **Static Methods**: Convenient static methods for easy logging access
- **Configuration Management**: Centralized logger configuration
- **Error Handling**: Comprehensive error handling and reporting

### 4. LoggerUtils (Utility Functions)

**Location**: `src/utils/LoggerUtils.cpp`

Utility functions for:

- **Error Code Conversion**: Convert error codes to human-readable strings
- **Log Level Conversion**: Convert between log level formats
- **Timestamp Utilities**: Platform-specific timestamp functions
- **Thread ID Utilities**: Platform-specific thread ID functions

## Key Features

### 1. Multiple Log Levels

```cpp
LoggerManager::Error("TAG", "Critical error message");
LoggerManager::Warn("TAG", "Warning message");
LoggerManager::Info("TAG", "Information message");
LoggerManager::Debug("TAG", "Debug message");
LoggerManager::Verbose("TAG", "Verbose message");
```

### 2. Tag-based Filtering

```cpp
auto& logger = LoggerManager::GetInstance();

// Set different log levels for different tags
logger.SetLogLevel("SENSOR", hf_log_level_t::LOG_LEVEL_VERBOSE);
logger.SetLogLevel("COMM", hf_log_level_t::LOG_LEVEL_WARN);
logger.SetLogLevel("MOTOR", hf_log_level_t::LOG_LEVEL_ERROR);
```

### 3. Custom Configuration

```cpp
hf_logger_config_t config = {};
config.default_level = hf_log_level_t::LOG_LEVEL_DEBUG;
config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
config.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
config.max_message_length = 512;
config.buffer_size = 1024;
config.enable_thread_safety = true;
config.enable_performance_monitoring = true;

LoggerManager::Initialize(config);
```

### 4. Convenience Macros

```cpp
// Direct logging macros
HF_LOG_ERROR("TAG", "Error message");
HF_LOG_WARN("TAG", "Warning message");
HF_LOG_INFO("TAG", "Info message");
HF_LOG_DEBUG("TAG", "Debug message");
HF_LOG_VERBOSE("TAG", "Verbose message");

// Conditional logging
HF_LOG_IF(condition, level, tag, format, ...);
```

### 5. Statistics and Diagnostics

```cpp
auto& logger = LoggerManager::GetInstance();

// Get statistics
hf_logger_statistics_t stats;
if (logger.GetStatistics(stats) == hf_logger_err_t::LOGGER_SUCCESS) {
    printf("Total messages: %llu\n", stats.total_messages);
    printf("Total bytes written: %llu\n", stats.total_bytes_written);
}

// Get diagnostics
hf_logger_diagnostics_t diagnostics;
if (logger.GetDiagnostics(diagnostics) == hf_logger_err_t::LOGGER_SUCCESS) {
    printf("Healthy: %s\n", diagnostics.is_healthy ? "Yes" : "No");
    printf("Uptime: %llu seconds\n", diagnostics.uptime_seconds);
}
```

## Usage Examples

### Basic Usage

```cpp
#include "utils/LoggerManager.h"

// Initialize with default configuration
LoggerManager::Initialize();

// Log messages
LoggerManager::Info("MAIN", "System starting up");
LoggerManager::Debug("SENSOR", "Temperature: %.2f°C", temperature);
LoggerManager::Error("COMM", "Communication timeout");
```

### Advanced Usage

```cpp
#include "utils/LoggerManager.h"
#include "base/BaseLogger.h"

// Custom configuration
hf_logger_config_t config = {};
config.default_level = hf_log_level_t::LOG_LEVEL_DEBUG;
config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
config.format_options = hf_log_format_t::LOG_FORMAT_TIMESTAMP | 
                       hf_log_format_t::LOG_FORMAT_LEVEL | 
                       hf_log_format_t::LOG_FORMAT_TAG;
config.max_message_length = 256;
config.buffer_size = 512;
config.enable_thread_safety = true;
config.enable_performance_monitoring = true;

// Initialize with custom configuration
if (LoggerManager::Initialize(config) == hf_logger_err_t::LOGGER_SUCCESS) {
    auto& logger = LoggerManager::GetInstance();
    
    // Set tag-specific levels
    logger.SetLogLevel("SENSOR", hf_log_level_t::LOG_LEVEL_VERBOSE);
    logger.SetLogLevel("COMM", hf_log_level_t::LOG_LEVEL_WARN);
    
    // Log messages
    LoggerManager::Info("MAIN", "System initialized with custom config");
    LoggerManager::Debug("SENSOR", "Raw ADC value: %d", adc_value);
    LoggerManager::Verbose("SENSOR", "Calibration factor: %.6f", cal_factor);
}
```

### Integration with Existing Code

To replace existing `ESP_LOGI`, `ESP_LOGE`, etc. calls:

```cpp
// Old way
ESP_LOGI("TAG", "Message");
ESP_LOGE("TAG", "Error: %s", error_msg);

// New way
LoggerManager::Info("TAG", "Message");
LoggerManager::Error("TAG", "Error: %s", error_msg);

// Or using macros
HF_LOG_INFO("TAG", "Message");
HF_LOG_ERROR("TAG", "Error: %s", error_msg);
```

## Migration from ConsolePort

The new logger system replaces the old `ConsolePort` implementation. Key differences:

### Old ConsolePort
```cpp
#include "ConsolePort.h"

ConsolePort& console = ConsolePort::GetInstance();
console.Info("TAG", "Message");
console.Error("TAG", "Error: %s", error_msg);
```

### New Logger System
```cpp
#include "utils/LoggerManager.h"

// Static methods (recommended)
LoggerManager::Info("TAG", "Message");
LoggerManager::Error("TAG", "Error: %s", error_msg);

// Or singleton instance
auto& logger = LoggerManager::GetInstance();
logger.Info("TAG", "Message");
logger.Error("TAG", "Error: %s", error_msg);
```

## Performance Considerations

### 1. Lazy Initialization
The logger system uses lazy initialization, so it only initializes when first used.

### 2. Thread Safety
All operations are thread-safe with proper mutex protection.

### 3. Message Buffering
Efficient message buffering reduces memory allocations.

### 4. Level Filtering
Messages are filtered at the source, reducing unnecessary processing.

### 5. ESP-IDF Integration
Direct integration with ESP-IDF logging for optimal performance.

## Error Handling

The logger system provides comprehensive error handling:

```cpp
hf_logger_err_t result = LoggerManager::Info("TAG", "Message");
if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    printf("Logging failed: %s\n", HfLoggerErrToString(result));
}

// Get last error
auto& logger = LoggerManager::GetInstance();
hf_logger_err_t last_error = logger.GetLastError();
char error_msg[256];
logger.GetLastErrorMessage(error_msg, sizeof(error_msg));
```

## Configuration Options

### Log Levels
- `LOG_LEVEL_NONE`: No logging
- `LOG_LEVEL_ERROR`: Error messages only
- `LOG_LEVEL_WARN`: Warning and error messages
- `LOG_LEVEL_INFO`: Info, warning, and error messages
- `LOG_LEVEL_DEBUG`: Debug, info, warning, and error messages
- `LOG_LEVEL_VERBOSE`: All messages

### Output Destinations
- `LOG_OUTPUT_NONE`: No output
- `LOG_OUTPUT_UART`: UART serial output
- `LOG_OUTPUT_USB`: USB CDC output
- `LOG_OUTPUT_FILE`: File system output
- `LOG_OUTPUT_NETWORK`: Network output
- `LOG_OUTPUT_CUSTOM`: Custom output callback

### Format Options
- `LOG_FORMAT_TIMESTAMP`: Include timestamp
- `LOG_FORMAT_LEVEL`: Include log level
- `LOG_FORMAT_TAG`: Include tag
- `LOG_FORMAT_FILE_LINE`: Include file and line
- `LOG_FORMAT_FUNCTION`: Include function name
- `LOG_FORMAT_THREAD_ID`: Include thread ID
- `LOG_FORMAT_COLORS`: Include ANSI colors

## Platform Support

### Currently Supported
- **ESP32**: Full support with ESP-IDF integration
  - ESP32-C6, ESP32 Classic, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2

### Future Support
- **STM32**: Planned implementation
- **Other MCUs**: Extensible architecture for additional platforms

## Building and Integration

The logger system is integrated into the HardFOC internal interface wrapper and is automatically available when including the appropriate headers.

### Required Headers
```cpp
#include "utils/LoggerManager.h"  // For basic logging
#include "base/BaseLogger.h"      // For advanced features
```

### Dependencies
- HardFOC Hardware Types (`HardwareTypes.h`)
- RTOS Mutex (`RtosMutex.h`)
- ESP-IDF (for ESP32 implementation)

## Example Implementation

See `examples/esp32/main/LoggerExample.cpp` for a comprehensive example demonstrating all features of the logger system.

## Conclusion

The new HardFOC Logger System provides a robust, feature-rich logging solution that integrates seamlessly with the existing HardFOC architecture. It offers significant improvements over the previous `ConsolePort` implementation while maintaining backward compatibility through the `LoggerManager` singleton interface.

The system is designed to be:
- **Platform Agnostic**: Works across different MCU platforms
- **Performance Optimized**: Efficient and lightweight
- **Thread Safe**: Safe for concurrent access
- **Feature Rich**: Comprehensive logging capabilities
- **Easy to Use**: Simple interface with convenience macros
- **Extensible**: Easy to add new platforms and features 