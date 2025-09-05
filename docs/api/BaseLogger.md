# 📝 BaseLogger API Reference

<div align="center">

![BaseLogger](https://img.shields.io/badge/BaseLogger-Abstract%20Base%20Class-yellow?style=for-the-badge&logo=file-text)

**🎯 Unified logging abstraction for comprehensive system monitoring and debugging**

**📋 Navigation**

[← Previous: BaseNvs](BaseNvs.md) | [Back to API Index](README.md) | [Next: BaseTemperature
→](BaseTemperature.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [📝 **Log Levels**](#-log-levels)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BaseLogger` class provides a comprehensive logging abstraction that serves as the unified
interface for all logging operations in the HardFOC system.
It supports multiple log levels, configurable output destinations, thread-safe operations,
performance monitoring, and works across different hardware implementations with minimal overhead.

### ✨ **Key Features**

- 📝 **Multi-Level Logging** - ERROR, WARN, INFO, DEBUG, VERBOSE levels
- 🔒 **Thread-Safe Operations** - Concurrent logging from multiple tasks
- 📊 **Multiple Output Destinations** - UART, file, memory buffer, network
- ⚡ **High Performance** - Minimal overhead with efficient buffering
- 🎯 **Configurable Filtering** - Runtime level control and tag filtering
- 📈 **Performance Monitoring** - Built-in logging statistics and profiling
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Memory Efficient** - Optimized memory usage with circular buffers
- 🔌 **Platform Agnostic** - Works across different MCU platforms

### 📊 **Supported Hardware**

| Implementation | UART | File | Memory | Network | Performance |

|----------------|------|------|--------|---------|-------------|

| `EspLogger` | ✅ | ✅ | ✅ | ✅ | High |

| `Esp8266Logger` | ✅ | ❌ | ✅ | ✅ | Medium |

| `ArmLogger` | ✅ | ✅ | ✅ | ❌ | High |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseLogger {
        <<abstract>>
        +EnsureInitialized() hf*logger*err*t
        +SetLogLevel(hf*log*level*t) hf*logger*err*t
        +LogMessage(level, tag, format, ...) hf*logger*err*t
        +LogError(tag, format, ...) hf*logger*err*t
        +LogWarn(tag, format, ...) hf*logger*err*t
        +LogInfo(tag, format, ...) hf*logger*err*t
        +LogDebug(tag, format, ...) hf*logger*err*t
        +LogVerbose(tag, format, ...) hf*logger*err*t
        +AddOutput(hf*log*output*t*) hf*logger*err*t
        +RemoveOutput(hf*log*output*t*) hf*logger*err*t
        +FlushBuffers() hf*logger*err*t
        +GetStatistics(hf*logger*statistics*t&) hf*logger*err*t
        +IsInitialized() bool
        #DoInitialize() hf*logger*err*t*
        #DoLogMessage(level, tag, message) hf*logger*err*t*
    }

    class EspLogger {
        +EspLogger()
        +EnableUartOutput(uart*num) hf*logger*err*t
        +EnableFileOutput(path) hf*logger*err*t
        +EnableNetworkOutput(host, port) hf*logger*err*t
        +SetBufferSize(size) hf*logger*err*t
    }

    class ConsoleLogger {
        +ConsoleLogger()
        +SetColorOutput(enable) hf*logger*err*t
        +SetTimestampFormat(format) hf*logger*err*t
    }

    class FileLogger {
        +FileLogger(filepath)
        +SetRotationSize(size) hf*logger*err*t
        +SetMaxFiles(count) hf*logger*err*t
        +CompressOldLogs(enable) hf*logger*err*t
    }

    BaseLogger <|-- EspLogger
    BaseLogger <|-- ConsoleLogger  
    BaseLogger <|-- FileLogger
```text

---

## 📋 **Error Codes**

### 🚨 **Logger Error Enumeration**

```cpp
enum class hf*logger*err*t : hf*u32*t {
    // Success codes
    LOGGER*SUCCESS = 0,
    
    // General errors
    LOGGER*ERR*FAILURE = 1,
    LOGGER*ERR*NOT*INITIALIZED = 2,
    LOGGER*ERR*ALREADY*INITIALIZED = 3,
    LOGGER*ERR*INVALID*PARAMETER = 4,
    LOGGER*ERR*NULL*POINTER = 5,
    LOGGER*ERR*OUT*OF*MEMORY = 6,
    
    // Output errors
    LOGGER*ERR*OUTPUT*NOT*FOUND = 7,
    LOGGER*ERR*OUTPUT*ALREADY*ADDED = 8,
    LOGGER*ERR*OUTPUT*FAILURE = 9,
    LOGGER*ERR*OUTPUT*FULL = 10,
    
    // Buffer errors
    LOGGER*ERR*BUFFER*OVERFLOW = 11,
    LOGGER*ERR*BUFFER*UNDERFLOW = 12,
    LOGGER*ERR*BUFFER*FULL = 13,
    LOGGER*ERR*BUFFER*EMPTY = 14,
    
    // Format errors
    LOGGER*ERR*FORMAT*ERROR = 15,
    LOGGER*ERR*MESSAGE*TOO*LONG = 16,
    LOGGER*ERR*INVALID*TAG = 17,
    LOGGER*ERR*INVALID*LEVEL = 18,
    
    // File errors
    LOGGER*ERR*FILE*NOT*FOUND = 19,
    LOGGER*ERR*FILE*PERMISSION = 20,
    LOGGER*ERR*FILE*WRITE*ERROR = 21,
    LOGGER*ERR*DISK*FULL = 22,
    
    // Network errors
    LOGGER*ERR*NETWORK*UNAVAILABLE = 23,
    LOGGER*ERR*NETWORK*TIMEOUT = 24,
    LOGGER*ERR*NETWORK*ERROR = 25,
    
    // System errors
    LOGGER*ERR*SYSTEM*ERROR = 26,
    LOGGER*ERR*PERMISSION*DENIED = 27,
    LOGGER*ERR*OPERATION*ABORTED = 28
};
```text

### 📊 **Error Code Categories**

| Category | Range | Description |

|----------|-------|-------------|

| **Success** | 0 | Successful operation |

| **General** | 1-6 | Basic initialization and parameter errors |

| **Output** | 7-10 | Output destination errors |

| **Buffer** | 11-14 | Buffer management errors |

| **Format** | 15-18 | Message formatting errors |

| **File** | 19-22 | File system errors |

| **Network** | 23-25 | Network logging errors |

| **System** | 26-28 | System-level errors |

---

## 🔧 **Core API**

### 🎯 **Essential Methods**

#### **Initialization & Configuration**
```cpp
/**
 * @brief Ensure the logger is initialized
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t EnsureInitialized() = 0;

/**
 * @brief Set the global log level
 * @param level Minimum log level to output
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t SetLogLevel(hf*log*level*t level) = 0;

/**
 * @brief Check if logger is initialized
 * @return bool True if initialized
 */
virtual bool IsInitialized() const = 0;
```text

#### **Logging Methods**
```cpp
/**
 * @brief Log a message with specified level
 * @param level Log level
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogMessage(hf*log*level*t level, 
                                 const char* tag,
                                 const char* format, ...) = 0;

/**
 * @brief Log error message
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogError(const char* tag, const char* format, ...) = 0;

/**
 * @brief Log warning message
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogWarn(const char* tag, const char* format, ...) = 0;

/**
 * @brief Log info message
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogInfo(const char* tag, const char* format, ...) = 0;

/**
 * @brief Log debug message
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogDebug(const char* tag, const char* format, ...) = 0;

/**
 * @brief Log verbose message
 * @param tag Message tag/category
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t LogVerbose(const char* tag, const char* format, ...) = 0;
```text

#### **Output Management**
```cpp
/**
 * @brief Add log output destination
 * @param output Pointer to output handler
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t AddOutput(hf*log*output*t* output) = 0;

/**
 * @brief Remove log output destination
 * @param output Pointer to output handler to remove
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t RemoveOutput(hf*log*output*t* output) = 0;

/**
 * @brief Flush all output buffers
 * @return hf*logger*err*t Error code
 */
virtual hf*logger*err*t FlushBuffers() = 0;
```text

---

## 📊 **Data Structures**

### 📝 **Log Level Types**

```cpp
enum class hf*log*level*t : hf*u8*t {
    LOG*LEVEL*NONE = 0,     ///< No logging
    LOG*LEVEL*ERROR = 1,    ///< Error conditions only
    LOG*LEVEL*WARN = 2,     ///< Warning and error conditions
    LOG*LEVEL*INFO = 3,     ///< Informational messages
    LOG*LEVEL*DEBUG = 4,    ///< Debug information
    LOG*LEVEL*VERBOSE = 5   ///< Detailed trace information
};
```text

### 📤 **Output Types**

```cpp
enum class hf*log*output*type*t : hf*u8*t {
    LOG*OUTPUT*UART = 0,        ///< UART/Serial output
    LOG*OUTPUT*FILE = 1,        ///< File system output
    LOG*OUTPUT*MEMORY = 2,      ///< Memory buffer output
    LOG*OUTPUT*NETWORK = 3,     ///< Network/UDP output
    LOG*OUTPUT*CUSTOM = 4       ///< Custom user-defined output
};
```text

### 📋 **Log Message Structure**

```cpp
struct hf*log*message*t {
    hf*u64*t timestamp*us;              ///< Timestamp in microseconds
    hf*log*level*t level;               ///< Log level
    char tag[16];                       ///< Message tag/category
    char message[256];                  ///< Formatted message
    hf*u32*t task*id;                   ///< Task/thread ID
    const char* file;                   ///< Source file name
    hf*u32*t line;                      ///< Source line number
};
```text

### 📤 **Output Handler Interface**

```cpp
struct hf*log*output*t {
    hf*log*output*type*t type;          ///< Output type
    hf*log*level*t min*level;           ///< Minimum level for this output
    
    /**
     * @brief Write log message to output
     * @param message Log message to write
     * @return hf*logger*err*t Error code
     */
    hf*logger*err*t (*write)(const hf*log*message*t* message);
    
    /**
     * @brief Flush output buffer
     * @return hf*logger*err*t Error code
     */
    hf*logger*err*t (*flush)(void);
    
    void* user*data;                    ///< User-defined data
};
```text

### 📈 **Logger Statistics**

```cpp
struct hf*logger*statistics*t {
    hf*u64*t total*messages;            ///< Total messages logged
    hf*u64*t messages*by*level[6];      ///< Messages per log level
    hf*u64*t dropped*messages;          ///< Messages dropped due to buffer full
    hf*u64*t format*errors;             ///< Format string errors
    hf*u64*t output*errors;             ///< Output write errors
    hf*u32*t buffer*high*water*mark;    ///< Maximum buffer usage
    hf*u32*t average*message*size;      ///< Average message size in bytes
    hf*u64*t total*bytes*logged;        ///< Total bytes written
    hf*u32*t active*outputs;            ///< Number of active outputs
    hf*u64*t uptime*ms;                 ///< Logger uptime in milliseconds
};
```text

---

## 📝 **Log Levels**

### 🚨 **ERROR Level**
Critical errors that require immediate attention:
```cpp
logger.LogError("MOTOR", "Motor controller fault detected: %s", fault*description);
logger.LogError("COMM", "CAN bus communication timeout after %d ms", timeout*ms);
```text

### ⚠️ **WARN Level**
Warning conditions that should be monitored:
```cpp
logger.LogWarn("TEMP", "Temperature high: %.1f°C (limit: %.1f°C)", temp, limit);
logger.LogWarn("MEMORY", "Low memory warning: %d bytes remaining", free*bytes);
```text

### ℹ️ **INFO Level**
General informational messages:
```cpp
logger.LogInfo("SYSTEM", "Motor controller initialized successfully");
logger.LogInfo("NETWORK", "Connected to WiFi: %s (IP: %s)", ssid, ip*address);
```text

### 🔧 **DEBUG Level**
Debug information for troubleshooting:
```cpp
logger.LogDebug("ADC", "Reading channel %d: raw=%u, voltage=%.3fV", channel, raw, voltage);
logger.LogDebug("GPIO", "Pin %d state changed: %s", pin, state ? "HIGH" : "LOW");
```text

### 📊 **VERBOSE Level**
Detailed trace information:
```cpp
logger.LogVerbose("I2C", "Transaction: addr=0x%02X, write=%d bytes, read=%d bytes", 
                  address, write*len, read*len);
logger.LogVerbose("TIMER", "Callback executed: task=%s, duration=%lu us", 
                  task*name, duration);
```text

---

## 📊 **Usage Examples**

### 🔧 **Basic System Logger**

```cpp
#include "inc/mcu/esp32/EspLogger.h"

class SystemLogger {
private:
    EspLogger logger*;
    bool is*initialized*;
    
public:
    SystemLogger() : is*initialized*(false) {}
    
    bool initialize() {
        if (logger*.EnsureInitialized() != hf*logger*err*t::LOGGER*SUCCESS) {
            return false;
        }
        
        // Set log level based on build configuration
        #ifdef DEBUG
            logger*.SetLogLevel(hf*log*level*t::LOG*LEVEL*VERBOSE);
        #else
            logger*.SetLogLevel(hf*log*level*t::LOG*LEVEL*INFO);
        #endif
        
        // Enable UART output for development
        if (logger*.EnableUartOutput(UART*NUM*0) != hf*logger*err*t::LOGGER*SUCCESS) {
            return false;
        }
        
        // Enable file output for production logging
        if (logger*.EnableFileOutput("/spiffs/system.log") != hf*logger*err*t::LOGGER*SUCCESS) {
            printf("Warning: File logging not available\n");
        }
        
        is*initialized* = true;
        logger*.LogInfo("SYSTEM", "System logger initialized");
        
        return true;
    }
    
    void log*system*startup() {
        if (!is*initialized*) return;
        
        logger*.LogInfo("BOOT", "=== HardFOC Motor Controller Starting ===");
        logger*.LogInfo("BOOT", "Firmware version: %s", get*firmware*version());
        logger*.LogInfo("BOOT", "Build date: %s %s", **DATE**, **TIME**);
        logger*.LogInfo("BOOT", "Free heap: %d bytes", esp*get*free*heap*size());
        logger*.LogInfo("BOOT", "CPU frequency: %d MHz", esp*clk*cpu*freq() / 1000000);
    }
    
    void log*motor*operation(float speed, float current, float temperature) {
        if (!is*initialized*) return;
        
        logger*.LogDebug("MOTOR", "Speed: %.2f RPM, Current: %.2f A, Temp: %.1f°C", 
                        speed, current, temperature);
        
        // Log warnings for abnormal conditions
        if (current > 10.0f) {
            logger*.LogWarn("MOTOR", "High current detected: %.2f A", current);
        }
        
        if (temperature > 80.0f) {
            logger*.LogWarn("MOTOR", "High temperature detected: %.1f°C", temperature);
        }
        
        // Log errors for fault conditions
        if (temperature > 100.0f) {
            logger*.LogError("MOTOR", "CRITICAL: Temperature overload: %.1f°C", temperature);
        }
    }
    
    void log*communication*event(const char* interface, bool success, 
                                const char* details) {
        if (!is*initialized*) return;
        
        if (success) {
            logger*.LogDebug("COMM", "%s: %s", interface, details);
        } else {
            logger*.LogError("COMM", "%s error: %s", interface, details);
        }
    }
    
    void show*logger*statistics() {
        if (!is*initialized*) return;
        
        hf*logger*statistics*t stats;
        if (logger*.GetStatistics(stats) == hf*logger*err*t::LOGGER*SUCCESS) {
            logger*.LogInfo("STATS", "=== Logger Statistics ===");
            logger*.LogInfo("STATS", "Total messages: %llu", stats.total*messages);
            logger*.LogInfo("STATS", "Errors: %llu, Warnings: %llu, Info: %llu",
                           stats.messages*by*level[1], 
                           stats.messages*by*level[2],
                           stats.messages*by*level[3]);
            logger*.LogInfo("STATS", "Debug: %llu, Verbose: %llu",
                           stats.messages*by*level[4],
                           stats.messages*by*level[5]);
            logger*.LogInfo("STATS", "Dropped messages: %llu", stats.dropped*messages);
            logger*.LogInfo("STATS", "Total bytes: %llu", stats.total*bytes*logged);
            
            if (stats.dropped*messages > 0) {
                logger*.LogWarn("STATS", "Performance issue: %llu messages dropped", 
                               stats.dropped*messages);
            }
        }
    }
    
private:
    const char* get*firmware*version() {
        return "1.2.3";  // This would come from build system
    }
};
```text

### 📊 **Performance Monitoring Logger**

```cpp
#include "inc/mcu/esp32/EspLogger.h"

class PerformanceLogger {
private:
    EspLogger logger*;
    hf*u64*t last*memory*check*;
    hf*u64*t last*performance*log*;
    
public:
    bool initialize() {
        if (logger*.EnsureInitialized() != hf*logger*err*t::LOGGER*SUCCESS) {
            return false;
        }
        
        logger*.SetLogLevel(hf*log*level*t::LOG*LEVEL*DEBUG);
        logger*.EnableUartOutput(UART*NUM*0);
        
        last*memory*check* = esp*timer*get*time();
        last*performance*log* = esp*timer*get*time();
        
        return true;
    }
    
    void log*function*performance(const char* function*name, 
                                hf*u64*t start*time*us,
                                hf*u64*t end*time*us) {
        hf*u64*t duration*us = end*time*us - start*time*us;
        
        if (duration*us > 1000) {  // Log if > 1ms
            logger*.LogWarn("PERF", "%s took %llu us (> 1ms)", function*name, duration*us);
        } else if (duration*us > 100) {  // Log if > 100us
            logger*.LogDebug("PERF", "%s took %llu us", function*name, duration*us);
        } else {
            logger*.LogVerbose("PERF", "%s took %llu us", function*name, duration*us);
        }
    }
    
    void log*memory*usage() {
        hf*u64*t now = esp*timer*get*time();
        
        // Log memory usage every 5 seconds
        if (now - last*memory*check* >= 5000000) {
            size*t free*heap = esp*get*free*heap*size();
            size*t min*free*heap = esp*get*minimum*free*heap*size();
            
            logger*.LogInfo("MEMORY", "Free heap: %u bytes (minimum: %u bytes)", 
                           free*heap, min*free*heap);
            
            if (free*heap < 10000) {
                logger*.LogError("MEMORY", "CRITICAL: Low memory condition");
            } else if (free*heap < 50000) {
                logger*.LogWarn("MEMORY", "Low memory warning");
            }
            
            last*memory*check* = now;
        }
    }
    
    void log*task*performance() {
        hf*u64*t now = esp*timer*get*time();
        
        // Log task statistics every 10 seconds
        if (now - last*performance*log* >= 10000000) {
            TaskStatus*t* task*array;
            UBaseType*t task*count = uxTaskGetNumberOfTasks();
            
            task*array = (TaskStatus*t*)pvPortMalloc(task*count * sizeof(TaskStatus*t));
            if (task*array != nullptr) {
                task*count = uxTaskGetSystemState(task*array, task*count, nullptr);
                
                logger*.LogInfo("TASKS", "=== Task Performance ===");
                for (UBaseType*t i = 0; i < task*count; i++) {
                    logger*.LogInfo("TASKS", "%s: Priority=%u, Stack=%u", 
                                   task*array[i].pcTaskName,
                                   task*array[i].uxCurrentPriority,
                                   task*array[i].usStackHighWaterMark);
                    
                    if (task*array[i].usStackHighWaterMark < 512) {
                        logger*.LogWarn("TASKS", "Low stack warning for task: %s", 
                                       task*array[i].pcTaskName);
                    }
                }
                
                vPortFree(task*array);
            }
            
            last*performance*log* = now;
        }
    }
    
    // RAII class for automatic function timing
    class FunctionTimer {
    private:
        PerformanceLogger* logger*;
        const char* function*name*;
        hf*u64*t start*time*;
        
    public:
        FunctionTimer(PerformanceLogger* logger, const char* function*name)
            : logger*(logger), function*name*(function*name) {
            start*time* = esp*timer*get*time();
        }
        
        ~FunctionTimer() {
            hf*u64*t end*time = esp*timer*get*time();
            logger*->log*function*performance(function*name*, start*time*, end*time);
        }
    };
};

// Macro for easy function timing
#define PERF*TIME*FUNCTION(logger) \
    PerformanceLogger::FunctionTimer *timer(logger, **FUNCTION**)
```text

### 📤 **Multi-Output Logger System**

```cpp
#include "inc/mcu/esp32/EspLogger.h"

class MultiOutputLogger {
private:
    EspLogger logger*;
    hf*log*output*t uart*output*;
    hf*log*output*t file*output*;
    hf*log*output*t network*output*;
    
public:
    bool initialize() {
        if (logger*.EnsureInitialized() != hf*logger*err*t::LOGGER*SUCCESS) {
            return false;
        }
        
        // Setup UART output for immediate feedback
        setup*uart*output();
        
        // Setup file output for persistent logging
        setup*file*output();
        
        // Setup network output for remote monitoring
        setup*network*output();
        
        logger*.LogInfo("LOGGER", "Multi-output logger system initialized");
        return true;
    }
    
private:
    void setup*uart*output() {
        uart*output*.type = hf*log*output*type*t::LOG*OUTPUT*UART;
        uart*output*.min*level = hf*log*level*t::LOG*LEVEL*DEBUG;
        uart*output*.write = uart*write*callback;
        uart*output*.flush = uart*flush*callback;
        uart*output*.user*data = this;
        
        logger*.AddOutput(&uart*output*);
    }
    
    void setup*file*output() {
        file*output*.type = hf*log*output*type*t::LOG*OUTPUT*FILE;
        file*output*.min*level = hf*log*level*t::LOG*LEVEL*INFO;
        file*output*.write = file*write*callback;
        file*output*.flush = file*flush*callback;
        file*output*.user*data = this;
        
        logger*.AddOutput(&file*output*);
    }
    
    void setup*network*output() {
        network*output*.type = hf*log*output*type*t::LOG*OUTPUT*NETWORK;
        network*output*.min*level = hf*log*level*t::LOG*LEVEL*ERROR;  // Only errors
        network*output*.write = network*write*callback;
        network*output*.flush = network*flush*callback;
        network*output*.user*data = this;
        
        logger*.AddOutput(&network*output*);
    }
    
    static hf*logger*err*t uart*write*callback(const hf*log*message*t* message) {
        // Format timestamp
        char timestamp[32];
        format*timestamp(message->timestamp*us, timestamp, sizeof(timestamp));
        
        // Add color coding based on level
        const char* color = get*color*code(message->level);
        const char* level*str = get*level*string(message->level);
        
        printf("%s[%s] %s (%s:%lu) %s: %s\033[0m\n",
               color,
               timestamp,
               level*str,
               message->file,
               message->line,
               message->tag,
               message->message);
        
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static hf*logger*err*t file*write*callback(const hf*log*message*t* message) {
        FILE* log*file = fopen("/spiffs/system.log", "a");
        if (log*file == nullptr) {
            return hf*logger*err*t::LOGGER*ERR*FILE*WRITE*ERROR;
        }
        
        char timestamp[32];
        format*timestamp(message->timestamp*us, timestamp, sizeof(timestamp));
        
        fprintf(log*file, "[%s] %s %s: %s\n",
                timestamp,
                get*level*string(message->level),
                message->tag,
                message->message);
        
        fclose(log*file);
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static hf*logger*err*t network*write*callback(const hf*log*message*t* message) {
        // Send critical errors to monitoring server
        if (message->level == hf*log*level*t::LOG*LEVEL*ERROR) {
            // Create JSON payload
            char json*payload[512];
            snprintf(json*payload, sizeof(json*payload),
                    "{"
                    "\"timestamp\":%llu,"
                    "\"level\":\"ERROR\","
                    "\"tag\":\"%s\","
                    "\"message\":\"%s\","
                    "\"file\":\"%s\","
                    "\"line\":%lu"
                    "}",
                    message->timestamp*us,
                    message->tag,
                    message->message,
                    message->file,
                    message->line);
            
            // Send via UDP (implementation depends on network stack)
            send*udp*message("log.server.com", 5140, json*payload);
        }
        
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static hf*logger*err*t uart*flush*callback(void) {
        fflush(stdout);
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static hf*logger*err*t file*flush*callback(void) {
        // File is closed after each write, so no flush needed
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static hf*logger*err*t network*flush*callback(void) {
        // UDP is connectionless, no flush needed
        return hf*logger*err*t::LOGGER*SUCCESS;
    }
    
    static void format*timestamp(hf*u64*t timestamp*us, char* buffer, size*t buffer*size) {
        hf*u64*t timestamp*ms = timestamp*us / 1000;
        hf*u32*t seconds = timestamp*ms / 1000;
        hf*u32*t milliseconds = timestamp*ms % 1000;
        
        snprintf(buffer, buffer*size, "%lu.%03lu", seconds, milliseconds);
    }
    
    static const char* get*level*string(hf*log*level*t level) {
        switch (level) {
            case hf*log*level*t::LOG*LEVEL*ERROR: return "ERROR";
            case hf*log*level*t::LOG*LEVEL*WARN: return "WARN ";
            case hf*log*level*t::LOG*LEVEL*INFO: return "INFO ";
            case hf*log*level*t::LOG*LEVEL*DEBUG: return "DEBUG";
            case hf*log*level*t::LOG*LEVEL*VERBOSE: return "VERB ";
            default: return "UNKN ";
        }
    }
    
    static const char* get*color*code(hf*log*level*t level) {
        switch (level) {
            case hf*log*level*t::LOG*LEVEL*ERROR: return "\033[31m";    // Red
            case hf*log*level*t::LOG*LEVEL*WARN: return "\033[33m";     // Yellow
            case hf*log*level*t::LOG*LEVEL*INFO: return "\033[32m";     // Green
            case hf*log*level*t::LOG*LEVEL*DEBUG: return "\033[36m";    // Cyan
            case hf*log*level*t::LOG*LEVEL*VERBOSE: return "\033[37m";  // White
            default: return "\033[0m";                                  // Reset
        }
    }
    
    static void send*udp*message(const char* host, int port, const char* message) {
        // UDP implementation would go here
        // This is a placeholder for actual network implementation
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Practices**

1. **🎯 Use Appropriate Log Levels**
   ```cpp
   // Use ERROR for critical issues
   logger.LogError("MOTOR", "Controller fault: emergency stop engaged");
   
   // Use WARN for concerning but non-critical issues
   logger.LogWarn("TEMP", "Temperature approaching limit: %.1f°C", temp);
   
   // Use INFO for important operational messages
   logger.LogInfo("SYSTEM", "Motor controller initialized successfully");
   
   // Use DEBUG for troubleshooting information
   logger.LogDebug("ADC", "Channel %d reading: %u", channel, raw*value);
   
   // Use VERBOSE for detailed tracing
   logger.LogVerbose("I2C", "Write transaction complete: %d bytes", count);
   ```

1. **🏷️ Use Meaningful Tags**
   ```cpp
   // GOOD: Descriptive, hierarchical tags
   logger.LogInfo("MOTOR.CTRL", "Speed set to %.2f RPM", speed);
   logger.LogDebug("COMM.CAN", "Message received: ID=0x%03X", msg*id);
   logger.LogError("SENSOR.TEMP", "Temperature sensor not responding");
   
   // BAD: Vague or inconsistent tags
   logger.LogInfo("", "Something happened");
   logger.LogError("error", "Bad thing");
   ```

1. **📊 Monitor Performance**
   ```cpp
   // Regular statistics monitoring
   hf*logger*statistics*t stats;
   logger.GetStatistics(stats);
   
   if (stats.dropped*messages > 0) {
       logger.LogWarn("LOGGER", "Performance issue: %llu messages dropped", 
                     stats.dropped*messages);
   }
   
   // Check buffer usage
   if (stats.buffer*high*water*mark > 80) {  // 80% usage
       logger.LogWarn("LOGGER", "High buffer usage: %u%%", 
                     stats.buffer*high*water*mark);
   }
   ```

1. **🔄 Implement Log Rotation**
   ```cpp
   // For file logging, implement rotation
   class LogRotator {
   public:
       void check*rotation() {
           struct stat st;
           if (stat("/spiffs/system.log", &st) == 0) {
               if (st.st*size > MAX*LOG*SIZE) {
                   rotate*logs();
               }
           }
       }
       
   private:
       void rotate*logs() {
           rename("/spiffs/system.log", "/spiffs/system.log.old");
           // Create new log file
       }
   };
   ```

### ❌ **Common Pitfalls**

1. **🚫 Logging in ISRs or Critical Sections**
   ```cpp
   // BAD: Logging from ISR
   void IRAM*ATTR gpio*isr*handler(void* arg) {
       logger.LogDebug("ISR", "GPIO interrupt");  // Don't do this!
   }
   
   // GOOD: Defer logging to task context
   void IRAM*ATTR gpio*isr*handler(void* arg) {
       BaseType*t xHigherPriorityTaskWoken = pdFALSE;
       xSemaphoreGiveFromISR(gpio*semaphore, &xHigherPriorityTaskWoken);
       portYIELD*FROM*ISR(xHigherPriorityTaskWoken);
   }
   
   void gpio*task(void* params) {
       while (true) {
           if (xSemaphoreTake(gpio*semaphore, portMAX*DELAY)) {
               logger.LogDebug("GPIO", "Interrupt processed");
           }
       }
   }
   ```

1. **🚫 Excessive Verbose Logging**
   ```cpp
   // BAD: Too much verbose logging
   for (int i = 0; i < 1000; i++) {
       logger.LogVerbose("LOOP", "Iteration %d", i);  // Floods log
   }
   
   // GOOD: Sample verbose logging
   for (int i = 0; i < 1000; i++) {
       if (i % 100 == 0) {  // Log every 100 iterations
           logger.LogVerbose("LOOP", "Progress: %d/1000", i);
       }
   }
   ```

1. **🚫 Not Checking Logger Errors**
   ```cpp
   // BAD: Ignoring logger errors
   logger.LogError("CRITICAL", "System failure");
   
   // GOOD: Handle logger failures
   if (logger.LogError("CRITICAL", "System failure") != LOGGER*SUCCESS) {
       // Fallback logging method
       printf("CRITICAL ERROR: System failure\n");
   }
   ```

### 🎯 **Performance Tips**

1. **⚡ Use Appropriate Buffer Sizes**
   ```cpp
   // Configure buffer size based on log volume
   logger.SetBufferSize(8192);  // 8KB buffer for high-volume logging
   ```

1. **📊 Batch Flush Operations**
   ```cpp
   // Flush periodically rather than after each message
   void periodic*flush*task(void* params) {
       while (true) {
           vTaskDelay(pdMS*TO*TICKS(1000));  // Flush every second
           logger.FlushBuffers();
       }
   }
   ```

1. **🔍 Use Conditional Compilation**
   ```cpp
   // Remove verbose logging in production builds
   #ifdef DEBUG*VERBOSE
   #define LOG*VERBOSE(tag, format, ...) logger.LogVerbose(tag, format, ##**VA*ARGS_*)
   #else
   #define LOG*VERBOSE(tag, format, ...) do {} while(0)
   #endif
   ```

---

<div align="center">

**📋 Navigation**

[← Previous: BaseNvs](BaseNvs.md) | [Back to API Index](README.md) | [Next: BaseTemperature
→](BaseTemperature.md)

</div>

---

<div align="center">

**📝 Professional Logging for Critical System Monitoring**

*Enabling comprehensive system observability with optimal performance and reliability*

</div>