# DacOutput Class Guide

Tiny helper around the ESP‑IDF DAC functions for generating analog voltages.

## Highlights
- Enable/disable a DAC channel
- Write 8‑bit values with `SetValue()`
- Based on ESP‑IDF's `dac_oneshot` driver (v5.5+)

## Example
```cpp
DacOutput dac(DAC_CHANNEL_1);
dac.Enable();
dac.SetValue(128); // mid‑scale output
```

---

[← Previous](SfUartDriver.md) | [Documentation Index](index.md) | [Next →](RMT.md)
