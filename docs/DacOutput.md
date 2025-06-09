# DacOutput Class Guide

Tiny helper around the ESP‑IDF DAC functions for generating analog voltages.

## Highlights
- Enable/disable a DAC channel
- Write 8‑bit values with `SetValue()`

## Example
```cpp
DacOutput dac(DAC_CHANNEL_1);
dac.Enable();
dac.SetValue(128); // mid‑scale output
```

---

[← Previous](UartDriver.md) | [Documentation Index](index.md)
