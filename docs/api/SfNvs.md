# SfNvs API Reference

## Overview

`SfNvs` is a software-based implementation of the `BaseNvs` interface, providing NVS (Non-Volatile Storage) functionality through software emulation. This implementation is useful for testing, simulation, or when hardware NVS is not available.

## Features

- **Software Emulation** - Pure software implementation of NVS functionality
- **Testing Support** - Ideal for unit testing and simulation
- **Cross-Platform** - Works on any platform with basic storage capabilities
- **Configurable** - Flexible configuration for different use cases
- **Debug Support** - Enhanced debugging and logging capabilities

## Header File

```cpp
#include "inc/mcu/software/SfNvs.h"
```

## Class Definition

```cpp
class SfNvs : public BaseNvs {
public:
    // Constructor with full configuration
    explicit SfNvs(
        const char* namespace_name = "default",
        hf_nvs_open_mode_t open_mode = hf_nvs_open_mode_t::HF_NVS_OPEN_MODE_READWRITE
    ) noexcept;

    // Destructor
    ~SfNvs() override;

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

### Basic NVS Usage

```cpp
#include "inc/mcu/software/SfNvs.h"

// Create software NVS instance
SfNvs nvs("my_app");

// Initialize
if (!nvs.Initialize()) {
    printf("Failed to initialize software NVS\n");
    return;
}

// Store a string
hf_nvs_err_t err = nvs.SetString("device_name", "Software_Device");
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

## Related Documentation

- [BaseNvs API Reference](BaseNvs.md) - Base class interface
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions