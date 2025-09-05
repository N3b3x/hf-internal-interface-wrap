# EspNvs API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspBluetooth](EspBluetooth.md) | [Back to ESP API Index](README.md) | [Next:
EspPeriodicTimer →](EspPeriodicTimer.md)

</div>

---

## Overview

`EspNvs` is the ESP32-C6 implementation of the `BaseNvs` interface,
providing comprehensive NVS (Non-Volatile Storage) functionality specifically optimized for ESP32-C6
microcontrollers running ESP-IDF v5.5+.
It offers both basic and advanced NVS features with hardware-specific optimizations.

## Features

- **ESP32-C6 NVS** - Full support for ESP32-C6 NVS capabilities
- **Persistent Storage** - Data survives power cycles and deep sleep
- **Multiple Namespaces** - Organized storage with namespaces
- **Type Safety** - Type-safe storage for different data types
- **Atomic Operations** - Safe concurrent access
- **Power Management** - Deep sleep compatibility
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspNvs.h"
```text

## Class Definition

```cpp
class EspNvs : public BaseNvs {
public:
    // Constructor with full configuration
    explicit EspNvs(
        const char* namespace*name = "default",
        hf*nvs*open*mode*t open*mode = hf*nvs*open*mode*t::HF*NVS*OPEN*MODE*READWRITE
    ) noexcept;

    // Destructor
    ~EspNvs() override;

    // BaseNvs implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // NVS operations
    hf*nvs*err*t SetString(const char* key, const char* value) noexcept override;
    hf*nvs*err*t GetString(const char* key, char* value, hf*size*t max*length) noexcept override;
    hf*nvs*err*t SetBlob(const char* key, const void* data, hf*size*t length) noexcept override;
    hf*nvs*err*t GetBlob(const char* key, void* data, hf*size*t* length) noexcept override;
    hf*nvs*err*t SetU8(const char* key, hf*u8*t value) noexcept override;
    hf*nvs*err*t GetU8(const char* key, hf*u8*t* value) noexcept override;
    hf*nvs*err*t SetU16(const char* key, hf*u16*t value) noexcept override;
    hf*nvs*err*t GetU16(const char* key, hf*u16*t* value) noexcept override;
    hf*nvs*err*t SetU32(const char* key, hf*u32*t value) noexcept override;
    hf*nvs*err*t GetU32(const char* key, hf*u32*t* value) noexcept override;
    hf*nvs*err*t SetU64(const char* key, hf*u64*t value) noexcept override;
    hf*nvs*err*t GetU64(const char* key, hf*u64*t* value) noexcept override;

    // Advanced features
    hf*nvs*err*t EraseKey(const char* key) noexcept override;
    hf*nvs*err*t EraseAll() noexcept override;
    hf*nvs*err*t Commit() noexcept override;
    hf*nvs*err*t GetUsedEntries(hf*size*t* used*entries) const noexcept override;
    hf*nvs*err*t GetFreeEntries(hf*size*t* free*entries) const noexcept override;
};
```text

## Usage Examples

### Basic String Storage

```cpp
#include "inc/mcu/esp32/EspNvs.h"

// Create NVS instance
EspNvs nvs("my*app");

// Initialize
if (!nvs.Initialize()) {
    printf("Failed to initialize NVS\n");
    return;
}

// Store a string
hf*nvs*err*t err = nvs.SetString("device*name", "ESP32-C6*Device");
if (err != HF*NVS*ERR*OK) {
    printf("Failed to set string: %d\n", err);
    return;
}

// Retrieve the string
char device*name[64];
err = nvs.GetString("device*name", device*name, sizeof(device*name));
if (err == HF*NVS*ERR*OK) {
    printf("Device name: %s\n", device*name);
} else if (err == HF*NVS*ERR*NOT*FOUND) {
    printf("Device name not found\n");
}
```text

### Numeric Data Storage

```cpp
// Store different numeric types
nvs.SetU8("sensor*count", 5);
nvs.SetU16("max*connections", 100);
nvs.SetU32("total*runtime", 12345);
nvs.SetU64("unique*id", 0x123456789ABCDEF0);

// Retrieve numeric values
hf*u8*t sensor*count;
hf*u16*t max*connections;
hf*u32*t total*runtime;
hf*u64*t unique*id;

if (nvs.GetU8("sensor*count", &sensor*count) == HF*NVS*ERR*OK) {
    printf("Sensor count: %d\n", sensor*count);
}

if (nvs.GetU16("max*connections", &max*connections) == HF*NVS*ERR*OK) {
    printf("Max connections: %d\n", max*connections);
}

if (nvs.GetU32("total*runtime", &total*runtime) == HF*NVS*ERR*OK) {
    printf("Total runtime: %u seconds\n", total*runtime);
}

if (nvs.GetU64("unique*id", &unique*id) == HF*NVS*ERR*OK) {
    printf("Unique ID: 0x%016llX\n", unique*id);
}
```text

### Binary Data Storage

```cpp
// Store binary data
struct sensor*config {
    hf*u8*t sensor*id;
    hf*u16*t sample*rate;
    hf*u32*t calibration*factor;
    hf*f32*t offset;
};

sensor*config config = {1, 100, 12345, 0.5f};

hf*nvs*err*t err = nvs.SetBlob("sensor*config", &config, sizeof(config));
if (err != HF*NVS*ERR*OK) {
    printf("Failed to store sensor config: %d\n", err);
    return;
}

// Retrieve binary data
sensor*config retrieved*config;
hf*size*t blob*length = sizeof(retrieved*config);
err = nvs.GetBlob("sensor*config", &retrieved*config, &blob*length);
if (err == HF*NVS*ERR*OK) {
    printf("Retrieved sensor config:\n");
    printf("  ID: %d\n", retrieved*config.sensor*id);
    printf("  Sample rate: %d\n", retrieved*config.sample*rate);
    printf("  Calibration factor: %u\n", retrieved*config.calibration*factor);
    printf("  Offset: %.2f\n", retrieved*config.offset);
}
```text

### Configuration Management

```cpp
// Store application configuration
struct app*config {
    char wifi*ssid[32];
    char wifi*password[64];
    hf*u8*t brightness;
    hf*u16*t update*interval;
    bool auto*start;
};

app*config config;
strcpy(config.wifi*ssid, "MyNetwork");
strcpy(config.wifi*password, "MyPassword");
config.brightness = 80;
config.update*interval = 300;
config.auto*start = true;

// Store configuration
hf*nvs*err*t err = nvs.SetBlob("app*config", &config, sizeof(config));
if (err != HF*NVS*ERR*OK) {
    printf("Failed to store app config: %d\n", err);
    return;
}

// Commit changes
err = nvs.Commit();
if (err != HF*NVS*ERR*OK) {
    printf("Failed to commit changes: %d\n", err);
    return;
}

printf("Configuration saved successfully\n");
```text

### Data Management

```cpp
// Check storage usage
hf*size*t used*entries, free*entries;
if (nvs.GetUsedEntries(&used*entries) == HF*NVS*ERR*OK &&
    nvs.GetFreeEntries(&free*entries) == HF*NVS*ERR*OK) {
    printf("NVS usage: %zu used, %zu free entries\n", used*entries, free*entries);
}

// Erase specific key
hf*nvs*err*t err = nvs.EraseKey("old*setting");
if (err == HF*NVS*ERR*OK) {
    printf("Key erased successfully\n");
} else if (err == HF*NVS*ERR*NOT*FOUND) {
    printf("Key not found\n");
}

// Erase all data (use with caution!)
// err = nvs.EraseAll();
// if (err == HF*NVS*ERR*OK) {
//     printf("All data erased\n");
// }
```text

## ESP32-C6 Specific Features

### Flash-based Storage

NVS uses flash memory for persistent storage, ensuring data survives power cycles.

### Namespace Organization

Organize data using namespaces for better structure and access control.

### Type Safety

Type-safe storage with specific functions for different data types.

### Atomic Operations

Safe concurrent access with atomic operations.

## Error Handling

The `EspNvs` class provides comprehensive error handling with specific error codes:

- `HF*NVS*ERR*OK` - Operation successful
- `HF*NVS*ERR*INVALID*ARG` - Invalid parameter
- `HF*NVS*ERR*NOT*INITIALIZED` - NVS not initialized
- `HF*NVS*ERR*NOT*FOUND` - Key not found
- `HF*NVS*ERR*INVALID*LENGTH` - Invalid data length
- `HF*NVS*ERR*NO*FREE*PAGES` - No free pages available
- `HF*NVS*ERR*READ*ONLY` - Read-only namespace
- `HF*NVS*ERR*COMMIT*FAILED` - Commit operation failed

## Performance Considerations

- **Commit Frequency**: Commit changes periodically, not after every write
- **Key Names**: Use short, descriptive key names
- **Data Size**: Keep individual values reasonably sized
- **Namespace Usage**: Use namespaces to organize related data

## Related Documentation

- [BaseNvs API Reference](../api/BaseNvs.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF NVS Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/storage/nvs_flash.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspBluetooth](EspBluetooth.md) | [Back to ESP API Index](README.md) | [Next:
EspPeriodicTimer →](EspPeriodicTimer.md)

</div>