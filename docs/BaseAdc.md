# BaseAdc Class Guide

Lightweight abstract base class for ADC peripherals. Derived drivers implement `Initialize()` and the channel read functions.

## Features ✨
- Lazy initialization via `EnsureInitialized()`
- Standardized `AdcErr` codes for error handling
- Pure virtual methods to read channels in volts or raw counts
- Designed for easy extension by MCU specific drivers

## Usage Example
```cpp
class MyAdc : public BaseAdc {
    bool Initialize() noexcept override { /* configure hardware */ return true; }
    AdcErr ReadChannelV(uint8_t ch, float& v, uint8_t n=1, uint32_t t=0) noexcept override { /* read */ }
    AdcErr ReadChannelCount(uint8_t ch, uint32_t& c, uint8_t n=1, uint32_t t=0) noexcept override { /* read */ }
    AdcErr ReadChannel(uint8_t ch, uint32_t& c, float& v, uint8_t n=1, uint32_t t=0) noexcept override { /* read */ }
};
```

---

[Documentation Index](index.md) | [Next →](Esp32C6Adc.md)
