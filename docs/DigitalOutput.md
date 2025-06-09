# DigitalOutput Class Guide

Configurable GPIO output pin. Supports push‑pull or open‑drain modes and provides simple state helpers.

## Capabilities
- Lazy initialization and optional initial state
- `SetActive()`, `SetInactive()` and `Toggle()` helpers
- Query the configured mode with `OutputMode()`
- Use `IsActive()` to check the logical level

## Example
```cpp
DigitalOutput led(GPIO_NUM_2, DigitalGpio::ActiveState::High);
led.SetActive();       // LED on
led.Toggle();          // LED off
```

---

[← Previous](DigitalInput.md) | [Documentation Index](index.md) | [Next →](DigitalOutputGuard.md)
