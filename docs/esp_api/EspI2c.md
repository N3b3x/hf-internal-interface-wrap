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
```text

## Class Definition

```cpp
class EspI2c : public BaseI2c {
public:
    // Constructor with full configuration
    explicit EspI2c(
        hf*i2c*port*t port = hf*i2c*port*t::HF*I2C*PORT*0,
        hf*pin*num*t sda*pin = GPIO*NUM*21,
        hf*pin*num*t scl*pin = GPIO*NUM*22,
        hf*i2c*freq*t frequency = hf*i2c*freq*t::HF*I2C*FREQ*100K,
        hf*i2c*mode*t mode = hf*i2c*mode*t::HF*I2C*MODE*MASTER
    ) noexcept;

    // Destructor
    ~EspI2c() override;

    // BaseI2c implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // I2C operations
    hf*i2c*err*t WriteBytes(hf*u8*t device*addr, const hf*u8*t* data, hf*size*t length) noexcept override;
    hf*i2c*err*t ReadBytes(hf*u8*t device*addr, hf*u8*t* data, hf*size*t length) noexcept override;
    hf*i2c*err*t WriteReadBytes(hf*u8*t device*addr, const hf*u8*t* write*data, hf*size*t write*length,
                                hf*u8*t* read*data, hf*size*t read*length) noexcept override;

    // Advanced features
    hf*i2c*err*t SetFrequency(hf*i2c*freq*t frequency) noexcept override;
    hf*i2c*err*t GetFrequency(hf*i2c*freq*t* frequency) const noexcept override;
    hf*i2c*err*t ScanBus(hf*u8*t* device*addresses, hf*size*t max*devices, hf*size*t* found*count) noexcept override;
};
```text

## Usage Examples

### Basic I2C Communication

```cpp
#include "inc/mcu/esp32/EspI2c.h"

// Create I2C instance
EspI2c i2c(HF*I2C*PORT*0, GPIO*NUM*21, GPIO*NUM*22, HF*I2C*FREQ*400K);

// Initialize
if (!i2c.Initialize()) {
    printf("Failed to initialize I2C\n");
    return;
}

// Write data to device
const hf*u8*t data[] = {0x01, 0x02, 0x03};
hf*i2c*err*t err = i2c.WriteBytes(0x48, data, sizeof(data));
if (err != HF*I2C*ERR*OK) {
    printf("I2C write failed: %d\n", err);
}

// Read data from device
hf*u8*t read*data[4];
err = i2c.ReadBytes(0x48, read*data, sizeof(read*data));
if (err == HF*I2C*ERR*OK) {
    printf("Read data: %02X %02X %02X %02X\n", 
           read*data[0], read*data[1], read*data[2], read*data[3]);
}
```text

### Bus Scanning

```cpp
// Scan for devices on the I2C bus
hf*u8*t devices[16];
hf*size*t found*count;
hf*i2c*err*t err = i2c.ScanBus(devices, 16, &found*count);

if (err == HF*I2C*ERR*OK) {
    printf("Found %zu devices:\n", found*count);
    for (hf*size*t i = 0; i < found*count; i++) {
        printf("  Device at address 0x%02X\n", devices[i]);
    }
}
```text

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

- `HF*I2C*ERR*OK` - Operation successful
- `HF*I2C*ERR*INVALID*ARG` - Invalid parameter
- `HF*I2C*ERR*NOT*INITIALIZED` - I2C not initialized
- `HF*I2C*ERR*TIMEOUT` - Operation timeout
- `HF*I2C*ERR*NACK` - Device not acknowledging
- `HF*I2C*ERR*BUS*BUSY` - Bus busy or locked

## Performance Considerations

- **DMA Usage**: Enable DMA for transfers larger than 32 bytes
- **Clock Frequency**: Use appropriate frequency for your application (100kHz, 400kHz, 1MHz)
- **Pull-up Resistors**: Ensure proper pull-up resistors on SDA/SCL lines
- **Cable Length**: Consider signal integrity for longer cables

## Related Documentation

- [BaseI2c API Reference](../api/BaseI2c.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF I2C Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/i2c.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspPwm](EspPwm.md) | [Back to ESP API Index](README.md) | [Next: EspSpi →](EspSpi.md)

</div>