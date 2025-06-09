# Esp32C6Adc Class Guide

Concrete ADC driver built on ESP‑IDF for the ESP32‑C6. Implements `BaseAdc` and exposes simple channel reads in counts or volts.

## Features ✨
- Configurable ADC unit, attenuation and width
- Lazy initialization on first use
- Helper methods to read channels with optional averaging
- Returns raw counts or converted voltages

## Example
```cpp
Esp32C6Adc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);
float v;
if (adc.ReadChannelV(ADC_CHANNEL_0, v) == BaseAdc::AdcErr::ADC_SUCCESS) {
    // use voltage value
}
```

---

[← Previous](BaseAdc.md) | [Documentation Index](index.md) | [Next →](BaseGpio.md)
