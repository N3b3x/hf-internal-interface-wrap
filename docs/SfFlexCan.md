# SfFlexCan Class Guide

Thread‑safe CAN controller built on `FlexCan` and a FreeRTOS mutex. 🛡️

## Highlights
- Same API as `FlexCan` but guarded by a mutex
- `Lock()`/`Unlock()` helpers for manual control

## Example
```cpp
SemaphoreHandle_t m = xSemaphoreCreateMutex();
SfFlexCan can(0, 500000, m);
can.Open();
FlexCan::Frame f{0x123, {0x01}, 1, false, false};
can.Write(f);
```

---

[← Previous](FlexCan.md) | [Documentation Index](index.md) | [Next →](PwmOutput.md)
