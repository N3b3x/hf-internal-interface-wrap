# NvsStorage Class Guide

Small helper for non-volatile key-value pairs using the ESP‑IDF NVS API. 💾

## Highlights
- Initializes NVS flash on first use
- Opens a namespace given at construction
- Get/Set 32‑bit values with commit on write

## Example
```cpp
NvsStorage store("app");
store.Open();
store.SetU32("boot_count", 1);
uint32_t val;
store.GetU32("boot_count", val);
```

---

[← Previous](RMT.md) | [Documentation Index](index.md)
