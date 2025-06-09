# DigitalExternalIRQ Class Guide

Interrupt capable input pin built on `DigitalInput`. Handles enabling, disabling and waiting for a GPIO interrupt using ESP‑IDF.

## Highlights
- Configurable interrupt type (rising, falling, etc.)
- Wait for an event with optional timeout
- Ideal for motion sensors or other GPIO IRQ sources

## Example
```cpp
DigitalExternalIRQ irq(GPIO_NUM_13, GPIO_INTR_NEGEDGE);
irq.Enable();
if (irq.Wait(1000)) {
    // interrupt received
}
```

---

[← Previous](DigitalOutputGuard.md) | [Documentation Index](index.md) | [Next →](I2cBus.md)
