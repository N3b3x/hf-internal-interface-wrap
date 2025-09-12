---
layout: default
title: "💾 EspNvs"
description: "ESP32-C6 NVS implementation with encryption and wear leveling support"
nav_order: 10
parent: "🔧 ESP32 Implementations"
permalink: /docs/esp_api/EspNvs/
---

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
```

## Class Definition

```cpp
class EspNvs : public BaseNvs {
public:
    // Constructor with full configuration
    explicit EspNvs(
        const char* namespace_name = "default",
        hf_nvs_open_mode_t open_mode = hf_nvs_open_mode_t::HF_NVS_OPEN_MODE_READWRITE
    ) noexcept;

    // Destructor
    ~EspNvs() override;

    // BaseNvs implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // NVS operations
    hf_nvs_err_t SetString(const char* key, const char* value) noexcept override;
    hf_nvs_err_t GetString(const char* key, char* value, hf_size_t max_length) noexcept override;
    hf_nvs_err_t SetBlob(const char* key, const void* data, hf_size_t length) noexcept override;
    hf_nvs_err_t GetBlob(const char* key, void* data, hf_size_t* length) noexcept override;
    hf_nvs_err_t SetU8(const char* key, hf_u8_t value) noexcept override;
    hf_nvs_err_t GetU8(const char* key, hf_u8_t* value) noexcept override;
    hf_nvs_err_t SetU16(const char* key, hf_u16_t value) noexcept override;
    hf_nvs_err_t GetU16(const char* key, hf_u16_t* value) noexcept override;
    hf_nvs_err_t SetU32(const char* key, hf_u32_t value) noexcept override;
    hf_nvs_err_t GetU32(const char* key, hf_u32_t* value) noexcept override;
    hf_nvs_err_t SetU64(const char* key, hf_u64_t value) noexcept override;
    hf_nvs_err_t GetU64(const char* key, hf_u64_t* value) noexcept override;

    // Advanced features
    hf_nvs_err_t EraseKey(const char* key) noexcept override;
    hf_nvs_err_t EraseAll() noexcept override;
    hf_nvs_err_t Commit() noexcept override;
    hf_nvs_err_t GetUsedEntries(hf_size_t* used_entries) const noexcept override;
    hf_nvs_err_t GetFreeEntries(hf_size_t* free_entries) const noexcept override;
};
```

## Usage Examples

### Basic String Storage

```cpp
#include "inc/mcu/esp32/EspNvs.h"

// Create NVS instance
EspNvs nvs("my_app");

// Initialize
if (!nvs.Initialize()) {
    printf("Failed to initialize NVS\n");
    return;
}

// Store a string
hf_nvs_err_t err = nvs.SetString("device_name", "ESP32-C6_Device");
if (err != HF_NVS_ERR_OK) {
    printf("Failed to set string: %d\n", err);
    return;
}

// Retrieve the string
char device_name[64];
err = nvs.GetString("device_name", device_name, sizeof(device_name));
if (err == HF_NVS_ERR_OK) {
    printf("Device name: %s\n", device_name);
} else if (err == HF_NVS_ERR_NOT_FOUND) {
    printf("Device name not found\n");
}
```

### Numeric Data Storage

```cpp
// Store different numeric types
nvs.SetU8("sensor_count", 5);
nvs.SetU16("max_connections", 100);
nvs.SetU32("total_runtime", 12345);
nvs.SetU64("unique_id", 0x123456789ABCDEF0);

// Retrieve numeric values
hf_u8_t sensor_count;
hf_u16_t max_connections;
hf_u32_t total_runtime;
hf_u64_t unique_id;

if (nvs.GetU8("sensor_count", &sensor_count) == HF_NVS_ERR_OK) {
    printf("Sensor count: %d\n", sensor_count);
}

if (nvs.GetU16("max_connections", &max_connections) == HF_NVS_ERR_OK) {
    printf("Max connections: %d\n", max_connections);
}

if (nvs.GetU32("total_runtime", &total_runtime) == HF_NVS_ERR_OK) {
    printf("Total runtime: %u seconds\n", total_runtime);
}

if (nvs.GetU64("unique_id", &unique_id) == HF_NVS_ERR_OK) {
    printf("Unique ID: 0x%016llX\n", unique_id);
}
```

### Binary Data Storage

```cpp
// Store binary data
struct sensor_config {
    hf_u8_t sensor_id;
    hf_u16_t sample_rate;
    hf_u32_t calibration_factor;
    hf_f32_t offset;
};

sensor_config config = {1, 100, 12345, 0.5f};

hf_nvs_err_t err = nvs.SetBlob("sensor_config", &config, sizeof(config));
if (err != HF_NVS_ERR_OK) {
    printf("Failed to store sensor config: %d\n", err);
    return;
}

// Retrieve binary data
sensor_config retrieved_config;
hf_size_t blob_length = sizeof(retrieved_config);
err = nvs.GetBlob("sensor_config", &retrieved_config, &blob_length);
if (err == HF_NVS_ERR_OK) {
    printf("Retrieved sensor config:\n");
    printf("  ID: %d\n", retrieved_config.sensor_id);
    printf("  Sample rate: %d\n", retrieved_config.sample_rate);
    printf("  Calibration factor: %u\n", retrieved_config.calibration_factor);
    printf("  Offset: %.2f\n", retrieved_config.offset);
}
```

### Configuration Management

```cpp
// Store application configuration
struct app_config {
    char wifi_ssid[32];
    char wifi_password[64];
    hf_u8_t brightness;
    hf_u16_t update_interval;
    bool auto_start;
};

app_config config;
strcpy(config.wifi_ssid, "MyNetwork");
strcpy(config.wifi_password, "MyPassword");
config.brightness = 80;
config.update_interval = 300;
config.auto_start = true;

// Store configuration
hf_nvs_err_t err = nvs.SetBlob("app_config", &config, sizeof(config));
if (err != HF_NVS_ERR_OK) {
    printf("Failed to store app config: %d\n", err);
    return;
}

// Commit changes
err = nvs.Commit();
if (err != HF_NVS_ERR_OK) {
    printf("Failed to commit changes: %d\n", err);
    return;
}

printf("Configuration saved successfully\n");
```

### Data Management

```cpp
// Check storage usage
hf_size_t used_entries, free_entries;
if (nvs.GetUsedEntries(&used_entries) == HF_NVS_ERR_OK &&
    nvs.GetFreeEntries(&free_entries) == HF_NVS_ERR_OK) {
    printf("NVS usage: %zu used, %zu free entries\n", used_entries, free_entries);
}

// Erase specific key
hf_nvs_err_t err = nvs.EraseKey("old_setting");
if (err == HF_NVS_ERR_OK) {
    printf("Key erased successfully\n");
} else if (err == HF_NVS_ERR_NOT_FOUND) {
    printf("Key not found\n");
}

// Erase all data (use with caution!)
// err = nvs.EraseAll();
// if (err == HF_NVS_ERR_OK) {
//     printf("All data erased\n");
// }
```

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

- `HF_NVS_ERR_OK` - Operation successful
- `HF_NVS_ERR_INVALID_ARG` - Invalid parameter
- `HF_NVS_ERR_NOT_INITIALIZED` - NVS not initialized
- `HF_NVS_ERR_NOT_FOUND` - Key not found
- `HF_NVS_ERR_INVALID_LENGTH` - Invalid data length
- `HF_NVS_ERR_NO_FREE_PAGES` - No free pages available
- `HF_NVS_ERR_READ_ONLY` - Read-only namespace
- `HF_NVS_ERR_COMMIT_FAILED` - Commit operation failed

## Performance Considerations

- **Commit Frequency**: Commit changes periodically, not after every write
- **Key Names**: Use short, descriptive key names
- **Data Size**: Keep individual values reasonably sized
- **Namespace Usage**: Use namespaces to organize related data

## Related Documentation

- [BaseNvs API Reference](../api/BaseNvs.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
<!-- markdownlint-disable-next-line MD013 -->
- [ESP-IDF NVS Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/storage/nvs_flash.html) - ESP-IDF docs

---

<div align="center">

**📋 Navigation**

[← Previous: EspBluetooth](EspBluetooth.md) | [Back to ESP API Index](README.md) | [Next:
EspPeriodicTimer →](EspPeriodicTimer.md)

</div>