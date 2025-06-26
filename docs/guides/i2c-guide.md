# 🔄 I2C Guide

The I2C bus wrapper simplifies communication with sensors and peripheral chips.

## Initialization

```cpp
#include "McuI2cBus.h"

McuI2cBus i2c(HF_I2C_NUM_0, HF_GPIO_NUM_21, HF_GPIO_NUM_22, 400000);
i2c.Open();
```

The constructor arguments are: bus index, SDA pin, SCL pin and bus speed.

## Basic Operations

```cpp
// Write a register
hf_u8_t data = 0x55;
i2c.Write(0x50, &data, 1);

// Read a register
hf_u8_t rx;
i2c.Read(0x50, &rx, 1);
```

## Device Scanning

```cpp
std::vector<hf_u8_t> addresses;
i2c.Scan(addresses);
```

Use `Scan()` to detect all devices on the bus.

## Thread Safety

Wrap the instance with `SfI2cBus` when shared across tasks.

```cpp
#include "SfI2cBus.h"
#include <mutex>

std::mutex i2c_mutex;
SfI2cBus safe_i2c(HF_I2C_NUM_0, HF_GPIO_NUM_21, HF_GPIO_NUM_22, 400000, i2c_mutex);
```

Refer to the [Porting Guide](porting-guide.md) for implementing a new I2C backend.

## 🚀 DMA Transfers
Some MCUs offer DMA for I2C. Use `EnableDma()` when transferring large buffers.

## 🛠️ Troubleshooting
- Check pull-up resistors if communication fails
- Validate clock speed to avoid bus errors
- Use a logic analyzer to inspect signal integrity

## 🔗 Related Examples
- [🔄 Basic I2C Example](../examples/basic-i2c.md)
- [🔒 Sensor Hub](../examples/sensor-hub.md)
