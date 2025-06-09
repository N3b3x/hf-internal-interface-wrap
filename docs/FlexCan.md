# FlexCan Class Guide

Minimal CAN controller abstraction built around the ESP‑IDF TWAI driver.

## Features
- Simple `Frame` struct for TX/RX
- Open, close, read and write helpers
- Suitable for basic CAN communication

## Example
```cpp
FlexCan can(0, 500000); // CAN0 at 500 kbit/s
can.Open();
FlexCan::Frame f{0x123, {0x01,0x02}, 2, false, false};
can.Write(f);
```

---

[← Previous](SfSpiBus.md) | [Documentation Index](index.md) | [Next →](PwmOutput.md)
