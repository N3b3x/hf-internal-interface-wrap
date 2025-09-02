# EepromNvs API Reference

## Overview

`EepromNvs` is an EEPROM-based implementation of the `BaseNvs` interface, providing NVS (Non-Volatile Storage) functionality through external EEPROM memory. This implementation is useful when internal flash-based NVS is not available or when external EEPROM storage is preferred.

## Features

- **EEPROM Storage** - Uses external EEPROM for persistent storage
- **I2C Interface** - Communicates with EEPROM via I2C
- **Configurable** - Flexible configuration for different EEPROM sizes
- **Wear Leveling** - Basic wear leveling support
- **Power Management** - Low power consumption

## Header File

```cpp
#include "inc/mcu/eeprom/EepromNvs.h"
```

## Class Definition

```cpp
class EepromNvs : public BaseNvs {
public:
    // Constructor with full configuration
    explicit EepromNvs(
        const char* namespace_name = "default",
        hf_nvs_open_mode_t open_mode = hf_nvs_open_mode_t::HF_NVS_OPEN_MODE_READWRITE,
        hf_i2c_port_t i2c_port = hf_i2c_port_t::HF_I2C_PORT_0,
        hf_u8_t eeprom_address = 0x50
    ) noexcept;

    // Destructor
    ~EepromNvs() override;

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

### Basic EEPROM NVS Usage

```cpp
#include "inc/mcu/eeprom/EepromNvs.h"

// Create EEPROM NVS instance
EepromNvs nvs("my_app", HF_NVS_OPEN_MODE_READWRITE, HF_I2C_PORT_0, 0x50);

// Initialize
if (!nvs.Initialize()) {
    printf("Failed to initialize EEPROM NVS\n");
    return;
}

// Store a string
hf_nvs_err_t err = nvs.SetString("device_name", "EEPROM_Device");
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