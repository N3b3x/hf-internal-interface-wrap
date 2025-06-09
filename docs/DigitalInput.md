# DigitalInput Class Guide

GPIO input wrapper. Extends `DigitalGpio` and configures the pin as an input on demand.

## Features
- Lazy initialization when first read
- `IsActive()` helper to check the logical state
- Optional `GetState()` method to read without translating to active level
- Ideal for buttons or external interrupt sources

## Example
```cpp
DigitalInput button(GPIO_NUM_0, DigitalGpio::ActiveState::Low);
if (button.IsActive()) {
    // button pressed
}
```

---

[← Previous](DigitalGpio.md) | [Documentation Index](index.md) | [Next →](DigitalOutput.md)
