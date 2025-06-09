# DigitalOutputGuard Class Guide

RAII helper that activates a `DigitalOutput` on creation and deactivates it on destruction.

Useful for toggling chip select lines or other temporary enables.

## Example
```cpp
{
    DigitalOutputGuard guard(myOutput);
    // myOutput is active within this scope
}
// automatically inactive here
```

---

[← Previous](DigitalOutput.md) | [Documentation Index](index.md) | [Next →](DigitalExternalIRQ.md)
