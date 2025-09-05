# 💾 BaseNvs API Reference

<div align="center">

![BaseNvs](https://img.shields.io/badge/BaseNvs-Abstract%20Base%20Class-blue?style=for-the-badge&logo=harddrive)

**🎯 Unified Non-Volatile Storage abstraction for all persistent data operations**

**📋 Navigation**

[← Previous: BaseBluetooth](BaseBluetooth.md) | [Back to API Index](README.md) | [Next: BaseLogger
→](BaseLogger.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BaseNvs` class provides a comprehensive non-volatile storage abstraction that serves as the
unified interface for all persistent data operations in the HardFOC system.
It supports key-value storage, multiple data types, namespaces,
and works across different storage implementations.

### ✨ **Key Features**

- 💾 **Key-Value Storage** - Simple and efficient key-value pairs
- 📝 **Multiple Data Types** - uint32*t, strings, binary blobs
- 🗂️ **Namespace Support** - Organized storage with namespaces
- 🔒 **Atomic Operations** - Safe concurrent access
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🔌 **Platform Agnostic** - Works with flash, EEPROM, and other storage
- 📊 **Statistics & Diagnostics** - Built-in monitoring and health reporting
- 🧵 **Thread Safe** - Designed for multi-threaded applications

### 📊 **Supported Hardware**

| Implementation | Hardware Type | Capacity | Features | Use Cases |

|----------------|---------------|----------|----------|-----------|

| `EspNvs` | ESP32-C6 Flash | Up to 1MB | Encryption, wear leveling | Configuration, logs |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseNvs {
        <<abstract>>
        +Initialize() hf*nvs*err*t
        +Deinitialize() hf*nvs*err*t
        +SetU32(key, value) hf*nvs*err*t
        +GetU32(key, value) hf*nvs*err*t
        +SetString(key, value) hf*nvs*err*t
        +GetString(key, buffer, size) hf*nvs*err*t
        +SetBlob(key, data, size) hf*nvs*err*t
        +GetBlob(key, buffer, size) hf*nvs*err*t
        +EraseKey(key) hf*nvs*err*t
        +EraseAll() hf*nvs*err*t
        +GetSize(key, size) hf*nvs*err*t
        +GetMaxKeyLength() size*t
        +GetMaxValueSize() size*t
        +GetStatistics(statistics) hf*nvs*err*t
        +GetDiagnostics(diagnostics) hf*nvs*err*t
    }
    
    class EspNvs {
        +EspNvs(namespace)
        +GetNamespace() const char*
        +SetEncryption(enabled) hf*nvs*err*t
    }
    
    BaseNvs <|-- EspNvs
```text

---

## 📋 **Error Codes**

The NVS system uses comprehensive error codes for robust error handling:

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `NVS*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `NVS*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `NVS*ERR*NOT*INITIALIZED` | 2 | ⚠️ NVS not initialized | Call Initialize() first |

| `NVS*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ NVS already initialized | Check initialization state |

| `NVS*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `NVS*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `NVS*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### 🔑 **Storage Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `NVS*ERR*KEY*NOT*FOUND` | 7 | 🔍 Key not found | Check key name or create key first |

| `NVS*ERR*KEY*TOO*LONG` | 8 | 📏 Key too long | Use shorter key name |

| `NVS*ERR*VALUE*TOO*LARGE` | 9 | 📊 Value too large | Check storage capacity |

| `NVS*ERR*NAMESPACE*NOT*FOUND` | 10 | 🗂️ Namespace not found | Create namespace first |

| `NVS*ERR*STORAGE*FULL` | 11 | 📦 Storage full | Free space or use larger storage |

| `NVS*ERR*INVALID*DATA` | 12 | ❌ Invalid data | Check data format |

| `NVS*ERR*READ*ONLY` | 13 | 📖 Read only mode | Check write permissions |

| `NVS*ERR*CORRUPTED` | 14 | 💥 Data corrupted | Re-initialize storage |

### 🔐 **Encryption Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `NVS*ERR*ENCRYPTION*FAILED` | 15 | 🔐 Encryption operation failed | Check encryption keys |

| `NVS*ERR*DECRYPTION*FAILED` | 16 | 🔓 Decryption operation failed | Check encryption keys |

| `NVS*ERR*ENCRYPTION*NOT*CONFIGURED` | 17 | ⚙️ Encryption not configured | Configure encryption first |

| `NVS*ERR*ENCRYPTION*NOT*SUPPORTED` | 18 | 🚫 Encryption not supported | Use different storage type |

| `NVS*ERR*KEY*PARTITION*CORRUPTED` | 19 | 💥 Key partition corrupted | Re-initialize encryption |

| `NVS*ERR*WRONG*ENCRYPTION*SCHEME` | 20 | 🔐 Wrong encryption scheme | Use correct encryption method |

### 🔧 **System Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `NVS*ERR*VERSION*MISMATCH` | 21 | 📊 Version mismatch | Update storage format |

| `NVS*ERR*NO*FREE*PAGES` | 22 | 📄 No free pages | Free space or reinitialize |

| `NVS*ERR*PARTITION*NOT*FOUND` | 23 | 🗂️ Partition not found | Check partition configuration |

| `NVS*ERR*ITERATOR*INVALID` | 24 | 🔄 Iterator invalid | Restart iteration |

| `NVS*ERR*SECURITY*VIOLATION` | 25 | 🚫 Security policy violation | Check access permissions |

| `NVS*ERR*UNSUPPORTED*OPERATION` | 26 | 🚫 Unsupported operation | Check hardware capabilities |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the NVS storage system
 * @return hf*nvs*err*t error code
 * 
 * 📝 Sets up storage hardware, opens namespace, and prepares for operations.
 * Must be called before any storage operations.
 * 
 * @example
 * EspNvs nvs("config");
 * if (nvs.Initialize() == hf*nvs*err*t::NVS*SUCCESS) {
 *     // NVS ready for use
 * }
 */
virtual hf*nvs*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the NVS storage system
 * @return hf*nvs*err*t error code
 * 
 * 🧹 Cleanly shuts down storage and closes namespace.
 */
virtual hf*nvs*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if NVS is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
bool IsInitialized() const noexcept;

/**
 * @brief Ensure NVS is initialized (lazy initialization)
 * @return true if initialized successfully, false otherwise
 * 
 * 🔄 Automatically initializes NVS if not already initialized.
 */
bool EnsureInitialized();

/**
 * @brief Ensure NVS is deinitialized (lazy deinitialization)
 * @return true if deinitialized successfully, false otherwise
 * 
 * 🔄 Automatically deinitializes NVS if currently initialized.
 */
bool EnsureDeinitialized();
```text

### 🔢 **Integer Storage Methods**

```cpp
/**
 * @brief Store 32-bit unsigned integer
 * @param key Storage key (null-terminated string)
 * @param value Value to store
 * @return hf*nvs*err*t error code
 * 
 * 💾 Stores a 32-bit unsigned integer value.
 * 
 * @example
 * hf*nvs*err*t result = nvs.SetU32("boot*count", 42);
 * if (result != hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Store failed: %s\n", HfNvsErrToString(result));
 * }
 */
virtual hf*nvs*err*t SetU32(const char *key, uint32*t value) noexcept = 0;

/**
 * @brief Retrieve 32-bit unsigned integer
 * @param key Storage key (null-terminated string)
 * @param value Reference to store retrieved value
 * @return hf*nvs*err*t error code
 * 
 * 📖 Retrieves a 32-bit unsigned integer value.
 * 
 * @example
 * uint32*t boot*count;
 * hf*nvs*err*t result = nvs.GetU32("boot*count", boot*count);
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Boot count: %u\n", boot*count);
 * } else if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
 *     printf("Boot count not found, using default\n");
 *     boot*count = 0;
 * }
 */
virtual hf*nvs*err*t GetU32(const char *key, uint32*t &value) noexcept = 0;
```text

### 📝 **String Storage Methods**

```cpp
/**
 * @brief Store string value
 * @param key Storage key (null-terminated string)
 * @param value String value to store
 * @return hf*nvs*err*t error code
 * 
 * 💾 Stores a null-terminated string value.
 * 
 * @example
 * hf*nvs*err*t result = nvs.SetString("device*name", "MyDevice");
 * if (result != hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("String store failed: %s\n", HfNvsErrToString(result));
 * }
 */
virtual hf*nvs*err*t SetString(const char *key, const char *value) noexcept = 0;

/**
 * @brief Retrieve string value
 * @param key Storage key (null-terminated string)
 * @param buffer Buffer to store retrieved string
 * @param buffer*size Size of the buffer in bytes
 * @param actual*size Actual size of the string (optional)
 * @return hf*nvs*err*t error code
 * 
 * 📖 Retrieves a string value.
 * 
 * @example
 * char device*name[32];
 * size*t actual*size;
 * hf*nvs*err*t result = nvs.GetString("device*name", device*name, sizeof(device*name), &actual*size);
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Device name: %s (length: %zu)\n", device*name, actual*size);
 * }
 */
virtual hf*nvs*err*t GetString(const char *key, char *buffer, size*t buffer*size,
                             size*t *actual*size = nullptr) noexcept = 0;
```text

### 📦 **Binary Blob Storage Methods**

```cpp
/**
 * @brief Store binary data (blob)
 * @param key Storage key (null-terminated string)
 * @param data Pointer to data to store
 * @param data*size Size of data in bytes
 * @return hf*nvs*err*t error code
 * 
 * 💾 Stores binary data of any size.
 * 
 * @example
 * uint8*t config*data[] = {0x01, 0x02, 0x03, 0x04};
 * hf*nvs*err*t result = nvs.SetBlob("config", config*data, sizeof(config*data));
 * if (result != hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Blob store failed: %s\n", HfNvsErrToString(result));
 * }
 */
virtual hf*nvs*err*t SetBlob(const char *key, const void *data, size*t data*size) noexcept = 0;

/**
 * @brief Retrieve binary data (blob)
 * @param key Storage key (null-terminated string)
 * @param buffer Buffer to store retrieved data
 * @param buffer*size Size of the buffer in bytes
 * @param actual*size Actual size of the data (optional)
 * @return hf*nvs*err*t error code
 * 
 * 📖 Retrieves binary data.
 * 
 * @example
 * uint8*t config*data[64];
 * size*t actual*size;
 * hf*nvs*err*t result = nvs.GetBlob("config", config*data, sizeof(config*data), &actual*size);
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Config data size: %zu bytes\n", actual*size);
 *     for (size*t i = 0; i < actual*size; i++) {
 *         printf("%02X ", config*data[i]);
 *     }
 *     printf("\n");
 * }
 */
virtual hf*nvs*err*t GetBlob(const char *key, void *buffer, size*t buffer*size,
                           size*t *actual*size = nullptr) noexcept = 0;
```text

### 🗑️ **Data Management Methods**

```cpp
/**
 * @brief Erase specific key
 * @param key Storage key to erase
 * @return hf*nvs*err*t error code
 * 
 * 🗑️ Removes a specific key-value pair from storage.
 * 
 * @example
 * hf*nvs*err*t result = nvs.EraseKey("obsolete*config");
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Key erased successfully\n");
 * }
 */
virtual hf*nvs*err*t EraseKey(const char *key) noexcept = 0;

/**
 * @brief Erase all data in namespace
 * @return hf*nvs*err*t error code
 * 
 * 🗑️ Removes all key-value pairs in the current namespace.
 * 
 * @example
 * hf*nvs*err*t result = nvs.EraseAll();
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("All data erased successfully\n");
 * }
 */
virtual hf*nvs*err*t EraseAll() noexcept = 0;

/**
 * @brief Get size of stored value
 * @param key Storage key
 * @param size Reference to store size
 * @return hf*nvs*err*t error code
 * 
 * 📊 Gets the size of a stored value without reading it.
 * 
 * @example
 * size*t value*size;
 * hf*nvs*err*t result = nvs.GetSize("config", value*size);
 * if (result == hf*nvs*err*t::NVS*SUCCESS) {
 *     printf("Config size: %zu bytes\n", value*size);
 * }
 */
virtual hf*nvs*err*t GetSize(const char *key, size*t &size) noexcept = 0;
```text

### 📊 **Information Methods**

```cpp
/**
 * @brief Get maximum key length
 * @return Maximum key length in characters
 * 
 * 📊 Returns the maximum allowed key length for this storage.
 */
virtual size*t GetMaxKeyLength() const noexcept = 0;

/**
 * @brief Get maximum value size
 * @return Maximum value size in bytes
 * 
 * 📊 Returns the maximum allowed value size for this storage.
 */
virtual size*t GetMaxValueSize() const noexcept = 0;
```text

### 📈 **Statistics and Diagnostics**

```cpp
/**
 * @brief Reset NVS operation statistics
 * @return hf*nvs*err*t error code
 * 
 * 🔄 Clears all accumulated statistics counters.
 */
virtual hf*nvs*err*t ResetStatistics() noexcept;

/**
 * @brief Reset NVS diagnostic information
 * @return hf*nvs*err*t error code
 * 
 * 🔄 Clears diagnostic information and error counters.
 */
virtual hf*nvs*err*t ResetDiagnostics() noexcept;

/**
 * @brief Get NVS operation statistics
 * @param statistics Reference to store statistics data
 * @return hf*nvs*err*t error code
 * 
 * 📊 Retrieves comprehensive statistics about NVS operations.
 */
virtual hf*nvs*err*t GetStatistics(hf*nvs*statistics*t &statistics) const noexcept;

/**
 * @brief Get NVS diagnostic information
 * @param diagnostics Reference to store diagnostics data
 * @return hf*nvs*err*t error code
 * 
 * 🔍 Retrieves diagnostic information about NVS health and status.
 */
virtual hf*nvs*err*t GetDiagnostics(hf*nvs*diagnostics*t &diagnostics) const noexcept;
```text

---

## 📊 **Data Structures**

### 📈 **NVS Statistics Structure**

```cpp
struct hf*nvs*statistics*t {
    uint32*t total*operations;      ///< Total operations performed
    uint32*t total*errors;          ///< Total errors encountered
    uint32*t total*reads;           ///< Total read operations
    uint32*t total*writes;          ///< Total write operations
    uint32*t total*commits;         ///< Total commit operations
    uint32*t total*erases;          ///< Total erase operations
    hf*nvs*err*t last*error;        ///< Last error encountered
    uint32*t last*operation*time*us; ///< Time of last operation
    uint32*t successful*ops;        ///< Successful operations
    uint32*t failed*ops;            ///< Failed operations
    uint32*t bytes*written;         ///< Total bytes written
    uint32*t bytes*read;            ///< Total bytes read
};
```text

### 🔍 **NVS Diagnostics Structure**

```cpp
struct hf*nvs*diagnostics*t {
    hf*nvs*err*t last*error;        ///< Last error encountered
    uint32*t consecutive*errors;    ///< Consecutive error count
    bool storage*healthy;           ///< Storage health status
    uint32*t system*uptime*ms;      ///< System uptime in milliseconds
};
```text

---

## 📊 **Usage Examples**

### 🔧 **Configuration Storage**

```cpp
#include "mcu/esp32/EspNvs.h"

class ConfigurationManager {
private:
    EspNvs config*nvs*;
    
public:
    ConfigurationManager() : config*nvs*("config") {}
    
    bool initialize() {
        return config*nvs*.EnsureInitialized();
    }
    
    bool save*device*config(const DeviceConfig& config) {
        // Store individual values
        hf*nvs*err*t result = config*nvs*.SetU32("device*id", config.device*id);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to save device*id: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        result = config*nvs*.SetString("device*name", config.device*name.c*str());
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to save device*name: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        result = config*nvs*.SetU32("baud*rate", config.baud*rate);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to save baud*rate: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ Device configuration saved successfully\n");
        return true;
    }
    
    bool load*device*config(DeviceConfig& config) {
        // Load individual values with defaults
        uint32*t device*id;
        hf*nvs*err*t result = config*nvs*.GetU32("device*id", device*id);
        if (result == hf*nvs*err*t::NVS*SUCCESS) {
            config.device*id = device*id;
        } else if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
            config.device*id = 1;  // Default value
            printf("⚠️ Using default device*id: %u\n", config.device*id);
        } else {
            printf("❌ Failed to load device*id: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        char device*name[32];
        size*t name*size;
        result = config*nvs*.GetString("device*name", device*name, sizeof(device*name), &name*size);
        if (result == hf*nvs*err*t::NVS*SUCCESS) {
            config.device*name = std::string(device*name, name*size);
        } else if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
            config.device*name = "DefaultDevice";  // Default value
            printf("⚠️ Using default device*name: %s\n", config.device*name.c*str());
        } else {
            printf("❌ Failed to load device*name: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        uint32*t baud*rate;
        result = config*nvs*.GetU32("baud*rate", baud*rate);
        if (result == hf*nvs*err*t::NVS*SUCCESS) {
            config.baud*rate = baud*rate;
        } else if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
            config.baud*rate = 115200;  // Default value
            printf("⚠️ Using default baud*rate: %u\n", config.baud*rate);
        } else {
            printf("❌ Failed to load baud*rate: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ Device configuration loaded successfully\n");
        return true;
    }
    
    void print*config*info() {
        printf("📊 Configuration Storage Info:\n");
        printf("  Max key length: %zu characters\n", config*nvs*.GetMaxKeyLength());
        printf("  Max value size: %zu bytes\n", config*nvs*.GetMaxValueSize());
        
        // Print statistics
        hf*nvs*statistics*t stats;
        if (config*nvs*.GetStatistics(stats) == hf*nvs*err*t::NVS*SUCCESS) {
            printf("  Total operations: %u\n", stats.total*operations);
            printf("  Successful operations: %u\n", stats.successful*ops);
            printf("  Failed operations: %u\n", stats.failed*ops);
            printf("  Bytes written: %u\n", stats.bytes*written);
            printf("  Bytes read: %u\n", stats.bytes*read);
        }
    }
};

struct DeviceConfig {
    uint32*t device*id;
    std::string device*name;
    uint32*t baud*rate;
};
```text

### 📊 **Calibration Data Storage**

```cpp
#include "mcu/esp32/EspNvs.h"

class CalibrationManager {
private:
    EspNvs calib*nvs*;
    
public:
    CalibrationManager() : calib*nvs*("calibration") {}
    
    bool initialize() {
        return calib*nvs*.EnsureInitialized();
    }
    
    bool save*adc*calibration(const AdcCalibration& calib) {
        // Store calibration data as blob
        hf*nvs*err*t result = calib*nvs*.SetBlob("adc*calib", &calib, sizeof(calib));
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to save ADC calibration: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        // Store calibration timestamp
        uint32*t timestamp = static*cast<uint32*t>(time(nullptr));
        result = calib*nvs*.SetU32("adc*calib*time", timestamp);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to save calibration timestamp: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ ADC calibration saved successfully\n");
        return true;
    }
    
    bool load*adc*calibration(AdcCalibration& calib) {
        // Check if calibration exists
        size*t calib*size;
        hf*nvs*err*t result = calib*nvs*.GetSize("adc*calib", calib*size);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Calibration not found\n");
            return false;
        }
        
        if (calib*size != sizeof(AdcCalibration)) {
            printf("❌ Calibration size mismatch: expected %zu, got %zu\n", 
                   sizeof(AdcCalibration), calib*size);
            return false;
        }
        
        // Load calibration data
        result = calib*nvs*.GetBlob("adc*calib", &calib, sizeof(calib));
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to load ADC calibration: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        // Load and check timestamp
        uint32*t timestamp;
        result = calib*nvs*.GetU32("adc*calib*time", timestamp);
        if (result == hf*nvs*err*t::NVS*SUCCESS) {
            uint32*t current*time = static*cast<uint32*t>(time(nullptr));
            uint32*t age*days = (current*time - timestamp) / (24 * 3600);
            printf("✅ ADC calibration loaded (age: %u days)\n", age*days);
            
            if (age*days > 30) {
                printf("⚠️ Calibration is old (%u days), consider re-calibration\n", age*days);
            }
        }
        
        return true;
    }
    
    bool is*calibration*valid() {
        size*t calib*size;
        hf*nvs*err*t result = calib*nvs*.GetSize("adc*calib", calib*size);
        return (result == hf*nvs*err*t::NVS*SUCCESS && calib*size == sizeof(AdcCalibration));
    }
    
    void clear*calibration() {
        calib*nvs*.EraseKey("adc*calib");
        calib*nvs*.EraseKey("adc*calib*time");
        printf("🗑️ Calibration data cleared\n");
    }
};

struct AdcCalibration {
    float gain*coefficients[8];
    float offset*coefficients[8];
    float temperature*coefficient;
    uint32*t calibration*date;
    uint16*t checksum;
};
```text

### 📝 **Log Storage**

```cpp
#include "mcu/esp32/EspNvs.h"

class LogManager {
private:
    EspNvs log*nvs*;
    uint32*t log*index*;
    
public:
    LogManager() : log*nvs*("logs"), log*index*(0) {}
    
    bool initialize() {
        if (!log*nvs*.EnsureInitialized()) {
            return false;
        }
        
        // Load current log index
        hf*nvs*err*t result = log*nvs*.GetU32("log*index", log*index*);
        if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
            log*index* = 0;  // Start from beginning
        } else if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to load log index: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        return true;
    }
    
    bool add*log*entry(const char* message) {
        char key[16];
        snprintf(key, sizeof(key), "log*%u", log*index*);
        
        // Store log message
        hf*nvs*err*t result = log*nvs*.SetString(key, message);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to store log entry: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        // Increment and save log index
        log*index*++;
        result = log*nvs*.SetU32("log*index", log*index*);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to update log index: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ Log entry %u stored: %s\n", log*index* - 1, message);
        return true;
    }
    
    void print*recent*logs(uint32*t count = 10) {
        printf("📝 Recent Log Entries:\n");
        printf("=====================\n");
        
        uint32*t start*index = (log*index* > count) ? (log*index* - count) : 0;
        
        for (uint32*t i = start*index; i < log*index*; i++) {
            char key[16];
            snprintf(key, sizeof(key), "log*%u", i);
            
            char message[128];
            hf*nvs*err*t result = log*nvs*.GetString(key, message, sizeof(message));
            if (result == hf*nvs*err*t::NVS*SUCCESS) {
                printf("[%u] %s\n", i, message);
            } else {
                printf("[%u] <log entry not found>\n", i);
            }
        }
    }
    
    void clear*logs() {
        // Erase all log entries
        for (uint32*t i = 0; i < log*index*; i++) {
            char key[16];
            snprintf(key, sizeof(key), "log*%u", i);
            log*nvs*.EraseKey(key);
        }
        
        // Reset log index
        log*index* = 0;
        log*nvs*.SetU32("log*index", log*index*);
        
        printf("🗑️ All logs cleared\n");
    }
    
    uint32*t get*log*count() const {
        return log*index*;
    }
};
```text

### 🔐 **Encrypted Storage (ESP32)**

```cpp
#include "mcu/esp32/EspNvs.h"

class SecureStorage {
private:
    EspNvs secure*nvs*;
    
public:
    SecureStorage() : secure*nvs*("secure") {}
    
    bool initialize() {
        if (!secure*nvs*.EnsureInitialized()) {
            return false;
        }
        
        // Enable encryption if supported
        hf*nvs*err*t result = secure*nvs*.SetEncryption(true);
        if (result == hf*nvs*err*t::NVS*SUCCESS) {
            printf("✅ Encryption enabled\n");
        } else if (result == hf*nvs*err*t::NVS*ERR*ENCRYPTION*NOT*SUPPORTED) {
            printf("⚠️ Encryption not supported on this storage\n");
        } else {
            printf("❌ Failed to enable encryption: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        return true;
    }
    
    bool store*credentials(const char* username, const char* password) {
        // Store username
        hf*nvs*err*t result = secure*nvs*.SetString("username", username);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to store username: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        // Store password
        result = secure*nvs*.SetString("password", password);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to store password: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ Credentials stored securely\n");
        return true;
    }
    
    bool load*credentials(char* username, size*t username*size, 
                         char* password, size*t password*size) {
        // Load username
        hf*nvs*err*t result = secure*nvs*.GetString("username", username, username*size);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to load username: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        // Load password
        result = secure*nvs*.GetString("password", password, password*size);
        if (result != hf*nvs*err*t::NVS*SUCCESS) {
            printf("❌ Failed to load password: %s\n", HfNvsErrToString(result));
            return false;
        }
        
        printf("✅ Credentials loaded successfully\n");
        return true;
    }
    
    void clear*credentials() {
        secure*nvs*.EraseKey("username");
        secure*nvs*.EraseKey("password");
        printf("🗑️ Credentials cleared\n");
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (!nvs.EnsureInitialized()) {
    printf("❌ NVS initialization failed\n");
    return false;
}

// ✅ Use appropriate error handling
uint32*t value;
hf*nvs*err*t result = nvs.GetU32("key", value);
if (result == hf*nvs*err*t::NVS*SUCCESS) {
    // Use the value
} else if (result == hf*nvs*err*t::NVS*ERR*KEY*NOT*FOUND) {
    // Key doesn't exist, use default
    value = default*value;
} else {
    printf("❌ NVS Error: %s\n", HfNvsErrToString(result));
    return false;
}

// ✅ Check data sizes before operations
size*t required*size;
if (nvs.GetSize("key", required*size) == hf*nvs*err*t::NVS*SUCCESS) {
    if (required*size > buffer*size) {
        printf("❌ Buffer too small, need %zu bytes\n", required*size);
        return false;
    }
}

// ✅ Use namespaces for organization
EspNvs config*nvs("config");
EspNvs calib*nvs("calibration");
EspNvs logs*nvs("logs");

// ✅ Validate data integrity
uint16*t stored*checksum;
if (nvs.GetU32("checksum", stored*checksum) == hf*nvs*err*t::NVS*SUCCESS) {
    uint16*t calculated*checksum = calculate*checksum(data, size);
    if (stored*checksum != calculated*checksum) {
        printf("❌ Data integrity check failed\n");
        return false;
    }
}

// ✅ Monitor storage health
hf*nvs*statistics*t stats;
if (nvs.GetStatistics(stats) == hf*nvs*err*t::NVS*SUCCESS) {
    if (stats.failed*ops > 10) {
        printf("⚠️ High NVS failure rate detected\n");
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore initialization
nvs.SetU32("key", value);  // May fail silently

// ❌ Don't ignore error codes
nvs.GetString("key", buffer, size);  // Error handling missing

// ❌ Don't assume key exists
uint32*t value = nvs.GetU32("key");  // May return garbage

// ❌ Don't use without checking buffer sizes
char buffer[16];
nvs.GetString("key", buffer, sizeof(buffer));  // May truncate

// ❌ Don't store sensitive data unencrypted
nvs.SetString("password", "secret");  // Use encrypted storage

// ❌ Don't ignore storage capacity
// Check available space before large writes
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use appropriate data types
// Use uint32*t for small integers
// Use blobs for large data structures
// Use strings for text data

// 🚀 Minimize write operations
// Batch related data together
// Use atomic operations where possible

// 🚀 Use appropriate key names
// Keep keys short but descriptive
// Use consistent naming conventions

// 🚀 Monitor storage usage
hf*nvs*statistics*t stats;
nvs.GetStatistics(stats);
if (stats.bytes*written > max*storage*bytes) {
    printf("⚠️ Storage usage high: %u bytes\n", stats.bytes*written);
}

// 🚀 Use encryption for sensitive data
// Enable encryption when available
// Store encryption keys securely

// 🚀 Implement data validation
// Use checksums for data integrity
// Validate data ranges and formats
```text

---

## 🔗 **Related Documentation**

- [⚙️ **EspNvs**](../esp_api/EspNvs.md) - ESP32-C6 implementation
- [🎯 **Hardware Types**](HardwareTypes.md) - Platform-agnostic types

---

<div align="center">

**📋 Navigation**

[← Previous: BaseBluetooth](BaseBluetooth.md) | [Back to API Index](README.md) | [Next: BaseLogger
→](BaseLogger.md)

</div>

---

<div align="center">

**💾 BaseNvs - The Foundation of Persistent Storage in HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 