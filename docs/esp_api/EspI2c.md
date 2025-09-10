---
layout: default
title: "🔗 EspI2c"
description: "ESP32-C6 I2C implementation with bus-device architecture and multi-master support"
nav_order: 4
parent: "🔧 ESP32 Implementations"
permalink: /docs/esp_api/EspI2c/
---

# EspI2c API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspPwm](EspPwm.md) | [Back to ESP API Index](README.md) | [Next: EspSpi →](EspSpi.md)

</div>

---

## Overview

`EspI2c` is the ESP32-C6 implementation of the `BaseI2c` interface,
providing comprehensive I2C functionality specifically optimized for ESP32-C6 microcontrollers
running ESP-IDF v5.5+.
It offers both basic and advanced I2C features with hardware-specific optimizations.

## Features

- **ESP32-C6 Optimized** - Full support for ESP32-C6 I2C capabilities
- **Clock Stretching** - Hardware support for clock stretching
- **Multi-Master Support** - True multi-master I2C bus support
- **DMA Integration** - High-performance DMA transfers
- **Power Management** - Deep sleep compatibility
- **Error Recovery** - Automatic bus recovery mechanisms
- **Performance Optimized** - Direct register access for critical operations

## Header File

```cpp
#include "inc/mcu/esp32/EspI2c.h"
```

## Class Definition

```cpp
class EspI2c : public BaseI2c {
public:
    // Constructor with full configuration
    explicit EspI2c(
        hf_i2c_port_t port = hf_i2c_port_t::HF_I2C_PORT_0,
        hf_pin_num_t sda_pin = GPIO_NUM_21,
        hf_pin_num_t scl_pin = GPIO_NUM_22,
        hf_i2c_freq_t frequency = hf_i2c_freq_t::HF_I2C_FREQ_100K,
        hf_i2c_mode_t mode = hf_i2c_mode_t::HF_I2C_MODE_MASTER
    ) noexcept;

    // Destructor
    ~EspI2c() override;

    // BaseI2c implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // I2C operations
    hf_i2c_err_t WriteBytes(hf_u8_t device_addr, const hf_u8_t* data, hf_size_t length) noexcept override;
    hf_i2c_err_t ReadBytes(hf_u8_t device_addr, hf_u8_t* data, hf_size_t length) noexcept override;
    hf_i2c_err_t WriteReadBytes(hf_u8_t device_addr, const hf_u8_t* write_data, hf_size_t write_length,
                                hf_u8_t* read_data, hf_size_t read_length) noexcept override;

    // Advanced features
    hf_i2c_err_t SetFrequency(hf_i2c_freq_t frequency) noexcept override;
    hf_i2c_err_t GetFrequency(hf_i2c_freq_t* frequency) const noexcept override;
    hf_i2c_err_t ScanBus(hf_u8_t* device_addresses, hf_size_t max_devices, hf_size_t* found_count) noexcept override;
};
```

## Usage Examples

### Basic I2C Communication

```cpp
#include "inc/mcu/esp32/EspI2c.h"

// Create I2C instance
EspI2c i2c(HF_I2C_PORT_0, GPIO_NUM_21, GPIO_NUM_22, HF_I2C_FREQ_400K);

// Initialize
if (!i2c.Initialize()) {
    printf("Failed to initialize I2C\n");
    return;
}

// Write data to device
const hf_u8_t data[] = {0x01, 0x02, 0x03};
hf_i2c_err_t err = i2c.WriteBytes(0x48, data, sizeof(data));
if (err != HF_I2C_ERR_OK) {
    printf("I2C write failed: %d\n", err);
}

// Read data from device
hf_u8_t read_data[4];
err = i2c.ReadBytes(0x48, read_data, sizeof(read_data));
if (err == HF_I2C_ERR_OK) {
    printf("Read data: %02X %02X %02X %02X\n", 
           read_data[0], read_data[1], read_data[2], read_data[3]);
}
```

### Bus Scanning

```cpp
// Scan for devices on the I2C bus
hf_u8_t devices[16];
hf_size_t found_count;
hf_i2c_err_t err = i2c.ScanBus(devices, 16, &found_count);

if (err == HF_I2C_ERR_OK) {
    printf("Found %zu devices:\n", found_count);
    for (hf_size_t i = 0; i < found_count; i++) {
        printf("  Device at address 0x%02X\n", devices[i]);
    }
}
```

## ESP32-C6 Specific Features

### Clock Stretching Support

The ESP32-C6 I2C controller supports hardware clock stretching, allowing slave devices to hold the
clock line low when they need more time to process data.

### Multi-Master Support

True multi-master support with automatic arbitration and collision detection.

### DMA Integration

High-performance DMA transfers for large data blocks with minimal CPU overhead.

## Error Handling

The `EspI2c` class provides comprehensive error handling with specific error codes for different
failure modes:

- `HF_I2C_ERR_OK` - Operation successful
- `HF_I2C_ERR_INVALID_ARG` - Invalid parameter
- `HF_I2C_ERR_NOT_INITIALIZED` - I2C not initialized
- `HF_I2C_ERR_TIMEOUT` - Operation timeout
- `HF_I2C_ERR_NACK` - Device not acknowledging
- `HF_I2C_ERR_BUS_BUSY` - Bus busy or locked

## Performance Considerations

- **DMA Usage**: Enable DMA for transfers larger than 32 bytes
- **Clock Frequency**: Use appropriate frequency for your application (100kHz, 400kHz, 1MHz)
- **Pull-up Resistors**: Ensure proper pull-up resistors on SDA/SCL lines
- **Cable Length**: Consider signal integrity for longer cables

## Related Documentation

- [BaseI2c API Reference](../api/BaseI2c.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
<!-- markdownlint-disable-next-line MD013 -->
- [ESP-IDF I2C Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/i2c.html) - ESP-IDF docs

---

<div align="center">

**📋 Navigation**

[← Previous: EspPwm](EspPwm.md) | [Back to ESP API Index](README.md) | [Next: EspSpi →](EspSpi.md)

</div>