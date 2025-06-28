# 🔒 Thread Safety Guide

Multi-threaded applications must coordinate access to hardware. The wrapper provides dedicated `Sf*` classes that encapsulate mutexes while preserving the base API.

## Wrapping Drivers

```cpp
#include "SfGpio.h"

auto base = std::make_shared<McuDigitalGpio>(HF_GPIO_NUM_2);
SfGpio safe_gpio(base); // SfGpio creates its own FreeRTOS mutex
```

All member functions of `SfGpio` automatically acquire an internal mutex before delegating to `McuDigitalGpio`.

## RAII Helpers

Classes like `DigitalOutputGuard` temporarily change pin state and restore it when leaving scope. Combine these with the thread‑safe wrappers for robust behavior.

## Guidelines

- Keep critical sections short to reduce contention.
- Avoid performing blocking I/O while holding a lock.
- Document which layer owns each mutex to prevent deadlocks.

Refer to the [Developer Guide](DeveloperGuide.md) for more advanced multi-threading patterns.

## 🧰 Choosing a Mutex Type
- Use `RtosMutex` for standard mutex protection
- `RtosSharedMutex` allows shared reader access when needed

## 🛠️ Troubleshooting
- Deadlocks often indicate inconsistent lock order
- Use timeouts when waiting on locks in real-time tasks

## 🔗 Related Examples
- [🔒 Sensor Hub](../examples/sensor-hub.md)
