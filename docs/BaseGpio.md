# BaseGpio Class Guide

Abstract helper for GPIO based classes. Provides lazy initialization and pin configuration retrieval.

## Key Points
- Stores the target pin number
- `EnsureInitialized()` calls protected `Initialize()` implemented by subclasses
- `GetPin()` returns the ESP‑IDF `gpio_num_t`
- Used by `DigitalInput`, `DigitalOutput` and `PwmOutput`

## Example Subclass
```cpp
class SimpleGpio : public BaseGpio {
    bool Initialize() noexcept override { /* configure pin */ return true; }
};
```

---

[← Previous](Esp32C6Adc.md) | [Documentation Index](index.md) | [Next →](DigitalGpio.md)
