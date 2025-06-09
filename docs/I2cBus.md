# I2cBus Class Guide

Non‑thread‑safe I²C master wrapper around ESP‑IDF APIs.

## Features
- Configurable port and bus parameters
- Blocking read, write and write‑read helpers
- Simple `Open()` / `Close()` lifecycle

## Example
```cpp
i2c_config_t cfg = {};
cfg.sda_io_num = 21;
cfg.scl_io_num = 22;
I2cBus bus(I2C_NUM_0, cfg);
bus.Open();
uint8_t id;
bus.WriteRead(0x50, &reg, 1, &id, 1);
```

---

[← Previous](DigitalExternalIRQ.md) | [Documentation Index](index.md) | [Next →](SfI2cBus.md)
