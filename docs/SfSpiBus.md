# SfSpiBus Class Guide

Thread‑safe SPI master with software controlled CS. Wraps ESP‑IDF SPI driver and guards transfers with a mutex.

## Highlights
- Software CS pin toggling
- Mutex protected transactions
- Same API as `SpiBus` plus `LockBus()` and `UnlockBus()`

## Example
```cpp
SemaphoreHandle_t m = xSemaphoreCreateMutex();
SfSpiBus bus(SPI2_HOST, busCfg, devCfg, m);
bus.Open();
bus.Write(txBuf, 4, 100);
```

---

[← Previous](SpiBus.md) | [Documentation Index](index.md) | [Next →](FlexCan.md)
