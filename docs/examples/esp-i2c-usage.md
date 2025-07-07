# ESP32 EspI2c Usage Guide

This guide demonstrates how to use the portable `EspI2c` class across ESP32 variants with ESP-IDF v5.5+ features.

## Overview

The `EspI2c` class provides a modern, thread-safe, and feature-rich I2C interface for ESP32 MCUs, supporting:
- Bus-device model (multiple devices per bus)
- Synchronous and asynchronous operations
- Multi-buffer transactions
- Power management and advanced signal conditioning
- Comprehensive error handling and diagnostics

## Basic Usage

```cpp
#include "EspI2c.h"

// Configure the I2C master bus
I2cMasterBusConfig bus_config;
bus_config.i2c_port = 0;
bus_config.sda_io_num = static_cast<GpioNum>(21);
bus_config.scl_io_num = static_cast<GpioNum>(22);
bus_config.enable_internal_pullup = true;

EspI2c i2c(bus_config);
if (!i2c.Initialize()) {
    // Handle initialization error
}

// Add a device
I2cDeviceConfig device;
device.device_address = 0x48;
device.scl_speed_hz = 400000;
i2c.AddDevice(device);

// Write data
uint8_t data[] = {0x10, 0x20, 0x30};
HfI2cErr result = i2c.Write(0x48, data, sizeof(data));

// Read data
uint8_t rx[4];
result = i2c.Read(0x48, rx, sizeof(rx));
```

## Asynchronous Operations

```cpp
// Async write
std::vector<uint8_t> tx_data = {0x01, 0x02};
i2c.WriteAsync(0x48, tx_data, [](HfI2cErr err, void* user_data) {
    if (err == HfI2cErr::I2C_SUCCESS) {
        // Write completed
    }
}, nullptr);

// Async read
size_t length = 4;
i2c.ReadAsync(0x48, length, [](HfI2cErr err, void* user_data) {
    if (err == HfI2cErr::I2C_SUCCESS) {
        // Read completed
    }
}, nullptr);
```

## Multi-Buffer Transactions

```cpp
I2cMultiBufferTransaction txn;
txn.device_addr = 0x48;
txn.buffers = {
    {I2cBufferType::Write, {0x01, 0x02}},
    {I2cBufferType::Read, std::vector<uint8_t>(4)}
};
HfI2cErr result = i2c.ExecuteMultiBufferTransaction(txn);
```

## Error Handling

```cpp
HfI2cErr result = i2c.Write(0x48, data, sizeof(data));
if (result != HfI2cErr::I2C_SUCCESS) {
    // Handle error
}
```

## Power Management

```cpp
i2c.SetPowerMode(I2cPowerMode::LOW_POWER);
// ...
i2c.SetPowerMode(I2cPowerMode::FULL_POWER);
```

## Device Management

```cpp
// Probe a device
if (i2c.ProbeDevice(0x48)) {
    // Device is present
}

// Scan for devices
std::vector<uint16_t> found;
i2c.ScanDevices(found);
for (auto addr : found) {
    printf("Found device at 0x%02X\n", addr);
}
```

## Register Access

```cpp
// Write a register
i2c.WriteRegister(0x48, 0x01, 0xFF);

// Read a register
uint8_t value;
i2c.ReadRegister(0x48, 0x01, value);
```

## Diagnostics

```cpp
I2cStatistics stats = i2c.GetStatistics();
I2cDiagnostics diag = i2c.GetDiagnostics();
```

## Best Practices
- Always check return values for errors.
- Use async operations for non-blocking transfers.
- Use multi-buffer transactions for complex protocols.
- Use power management features for energy efficiency.
- Use diagnostics for monitoring and debugging.

## Notes
- All public methods are thread-safe.
- Asynchronous callbacks execute in interrupt context—keep them minimal.
- Advanced features require ESP-IDF v5.5+ and are optimized for ESP32C6 and newer. 