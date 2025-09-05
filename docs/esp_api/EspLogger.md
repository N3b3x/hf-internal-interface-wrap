# 📝 EspLogger API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspTemperature](EspTemperature.md) | [Back to ESP API Index](README.md) | [Next: ESP
API Index](README.md)

</div>

---

## Overview

`EspLogger` provides ESP32-specific logging functionality using ESP-IDF's esp*log system.
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
    explicit EspLogger(const hf*logger*config*t& config) noexcept;
    
    // Destructor with proper cleanup
    ~EspLogger() noexcept override;
    
    // BaseLogger interface implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    
    // Logging operations
    hf*logger*err*t Log(hf*log*level*t level, const char* tag, const char* format, ...) noexcept override;
    hf*logger*err*t LogV(hf*log*level*t level, const char* tag, const char* format, va*list args) noexcept override;
    
    // Level management
    hf*logger*err*t SetLevel(hf*log*level*t level) noexcept override;
    hf*log*level*t GetLevel() const noexcept override;
    
    // Tag management
    hf*logger*err*t SetTagLevel(const char* tag, hf*log*level*t level) noexcept override;
    hf*log*level*t GetTagLevel(const char* tag) const noexcept override;
    
    // Output management
    hf*logger*err*t AddOutput(hf*logger*output*t output*type, const char* output*config) noexcept override;
    hf*logger*err*t RemoveOutput(hf*logger*output*t output*type) noexcept override;
    
    // Performance and diagnostics
    hf*logger*err*t GetStatistics(hf*logger*statistics*t& statistics) noexcept override;
    hf*logger*err*t GetDiagnostics(hf*logger*diagnostics*t& diagnostics) noexcept override;
    hf*logger*err*t ResetStatistics() noexcept override;
    
    // ESP-IDF specific features
    hf*logger*err*t SetEspLogLevel(esp*log*level*t level) noexcept;
    esp*log*level*t GetEspLogLevel() const noexcept;
    hf*logger*err*t EnableEspLogV2(bool enable) noexcept;
    bool IsEspLogV2Enabled() const noexcept;
};
```text

## Configuration Structure

### Logger Configuration

```cpp
struct hf*logger*config*t {
    hf*log*level*t default*level;                // Default log level
    bool enable*esp*log*v2;                      // Enable ESP-IDF Log V2
    bool enable*performance*monitoring;          // Enable performance tracking
    bool enable*thread*safety;                   // Enable mutex protection
    hf*logger*output*t output*types;             // Output type flags
    char console*config[64];                     // Console output configuration
    char file*config[128];                       // File output configuration
    char network*config[128];                    // Network output configuration
};
```text

## Usage Examples

### Basic Logging

```cpp
#include "mcu/esp32/EspLogger.h"

// Configure logger
hf*logger*config*t config = {};
config.default*level = hf*log*level*t::LOG*INFO;
config.enable*esp*log*v2 = true;
config.output*types = hf*logger*output*t::OUTPUT*CONSOLE;

// Create and initialize logger
EspLogger logger(config);
if (!logger.EnsureInitialized()) {
    printf("Failed to initialize logger\n");
    return;
}

// Basic logging
logger.Log(hf*log*level*t::LOG*INFO, "APP", "Application started");
logger.Log(hf*log*level*t::LOG*ERROR, "APP", "Error occurred: %d", error*code);
logger.Log(hf*log*level*t::LOG*DEBUG, "APP", "Debug info: %s", debug*string);
```text

### Tag-Based Logging

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Set different levels for different tags
logger.SetTagLevel("APP", hf*log*level*t::LOG*INFO);
logger.SetTagLevel("SENSOR", hf*log*level*t::LOG*DEBUG);
logger.SetTagLevel("NETWORK", hf*log*level*t::LOG*WARN);

// Log with different tags
logger.Log(hf*log*level*t::LOG*INFO, "APP", "Application initialized");
logger.Log(hf*log*level*t::LOG*DEBUG, "SENSOR", "Sensor reading: %.2f", sensor*value);
logger.Log(hf*log*level*t::LOG*WARN, "NETWORK", "Network timeout");
```text

### Multi-Output Logging

```cpp
// Configure multi-output logger
hf*logger*config*t config = {};
config.default*level = hf*log*level*t::LOG*INFO;
config.output*types = hf*logger*output*t::OUTPUT*CONSOLE | 
                     hf*logger*output*t::OUTPUT*FILE;
strcpy(config.file*config, "/spiffs/logs/app.log");

EspLogger logger(config);
logger.EnsureInitialized();

// Add network output
logger.AddOutput(hf*logger*output*t::OUTPUT*NETWORK, "udp://192.168.1.100:514");

// Log to all outputs
logger.Log(hf*log*level*t::LOG*INFO, "APP", "Multi-output logging active");
```text

### Performance Monitoring

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Enable performance monitoring
hf*logger*config*t perf*config = config;
perf*config.enable*performance*monitoring = true;
logger.Initialize();

// Perform logging operations
for (int i = 0; i < 1000; i++) {
    logger.Log(hf*log*level*t::LOG*DEBUG, "PERF", "Performance test %d", i);
}

// Get statistics
hf*logger*statistics*t stats;
logger.GetStatistics(stats);
printf("Total logs: %u\n", stats.total*logs);
printf("Successful logs: %u\n", stats.successful*logs);
printf("Average log time: %.2f µs\n", stats.average*log*time*us);
printf("Max log time: %.2f µs\n", stats.max*log*time*us);

// Get diagnostics
hf*logger*diagnostics*t diagnostics;
logger.GetDiagnostics(diagnostics);
printf("Last error: %d\n", diagnostics.last*error*code);
printf("Memory usage: %u bytes\n", diagnostics.memory*usage*bytes);
```text

### ESP-IDF Integration

```cpp
EspLogger logger(config);
logger.EnsureInitialized();

// Set ESP-IDF log level
logger.SetEspLogLevel(ESP*LOG*INFO);

// Enable ESP-IDF Log V2
logger.EnableEspLogV2(true);

// Use ESP-IDF logging macros (they will use our logger)
ESP*LOGI("APP", "ESP-IDF integration working");
ESP*LOGE("APP", "Error with code: %d", error*code);
ESP*LOGD("APP", "Debug info: %s", debug*string);

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
logger.SetLevel(hf*log*level*t::LOG*WARN);

// Set tag-specific levels
logger.SetTagLevel("CRITICAL", hf*log*level*t::LOG*ERROR);
logger.SetTagLevel("DEBUG", hf*log*level*t::LOG*DEBUG);

// Log with different levels
logger.Log(hf*log*level*t::LOG*INFO, "APP", "This won't be logged (level too low)");
logger.Log(hf*log*level*t::LOG*WARN, "APP", "This will be logged");
logger.Log(hf*log*level*t::LOG*ERROR, "CRITICAL", "Critical error");
logger.Log(hf*log*level*t::LOG*DEBUG, "DEBUG", "Debug info");

// Get current levels
hf*log*level*t global*level = logger.GetLevel();
hf*log*level*t critical*level = logger.GetTagLevel("CRITICAL");
printf("Global level: %d, Critical level: %d\n", global*level, critical*level);
```text

## Log Levels

| Level | Value | Description |

|-------|-------|-------------|

| `LOG*NONE` | 0 | No logging |

| `LOG*ERROR` | 1 | Error messages only |

| `LOG*WARN` | 2 | Warning and error messages |

| `LOG*INFO` | 3 | Informational, warning, and error messages |

| `LOG*DEBUG` | 4 | Debug, info, warning, and error messages |

| `LOG*VERBOSE` | 5 | All messages including verbose |

## Output Types

| Output Type | Description |

|-------------|-------------|

| `OUTPUT*CONSOLE` | Standard console output |

| `OUTPUT*FILE` | File-based logging |

| `OUTPUT*NETWORK` | Network-based logging (UDP/TCP) |

## Performance Characteristics

- **Log Latency**: <10µs per log entry
- **Memory Usage**: ~2KB base + 1KB per output
- **Throughput**: >10,000 logs/second
- **Thread Safety**: Full mutex protection
- **ESP-IDF Integration**: Native performance

## Error Handling

The `EspLogger` class provides comprehensive error reporting through the `hf*logger*err*t`
enumeration:

- `LOGGER*SUCCESS` - Operation completed successfully
- `LOGGER*ERR*NOT*INITIALIZED` - Logger not initialized
- `LOGGER*ERR*INVALID*LEVEL` - Invalid log level
- `LOGGER*ERR*INVALID*TAG` - Invalid tag
- `LOGGER*ERR*OUTPUT*FAILED` - Output operation failed
- `LOGGER*ERR*MEMORY` - Memory allocation error

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
