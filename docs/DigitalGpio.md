# DigitalGpio Class Guide

Abstract GPIO helper inherited by most pin classes. Defines enums for mode, state and resistance and converts them to strings.

## Highlights
- Supports active-high or active-low pins
- Enumeration helpers for mode, state and pull resistors
- Designed for extension by `DigitalInput`, `DigitalOutput` and similar classes

## Example
```cpp
DigitalOutput led(GPIO_NUM_5, DigitalGpio::ActiveState::High);
led.SetActive();
```

---

[← Previous](BaseGpio.md) | [Documentation Index](index.md) | [Next →](DigitalInput.md)
