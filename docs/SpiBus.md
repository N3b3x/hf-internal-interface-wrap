# SpiBus Class Guide

Basic SPI master driver using ESP‑IDF. No thread safety is provided.

## Capabilities
- Blocking read, write and write‑read operations
- Host/device configuration supplied by the user
- Query the configured clock frequency

## Example
```cpp
SpiBus bus(SPI2_HOST, busCfg, devCfg);
if (bus.Open()) {
    bus.Write(txBuf, 4);
    bus.Read(rxBuf, 4);
}
```

---

[← Previous](SfI2cBus.md) | [Documentation Index](index.md) | [Next →](SfSpiBus.md)
