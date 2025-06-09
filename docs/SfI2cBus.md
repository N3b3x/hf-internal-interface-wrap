# SfI2cBus Class Guide

Thread‑safe I²C master using a FreeRTOS mutex. Mirror of `I2cBus` with locking helpers.

## Benefits
- Safe access from multiple tasks
- `LockBus()` and `UnlockBus()` for manual control
- Identical API to `I2cBus`

## Example
```cpp
SemaphoreHandle_t m = xSemaphoreCreateMutex();
SfI2cBus bus(I2C_NUM_0, cfg, m);
bus.Open();
bus.LockBus();
bus.Write(addr, data, 2, 100);
bus.UnlockBus();
```

---

[← Previous](I2cBus.md) | [Documentation Index](index.md) | [Next →](SpiBus.md)
