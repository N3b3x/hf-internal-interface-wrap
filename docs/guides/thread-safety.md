# 🔒 Thread Safety Guide

Multi-threaded applications must coordinate access to hardware. The wrapper provides dedicated `Sf*` classes that encapsulate mutexes while preserving the base API.

## Wrapping Drivers

```cpp
#include "SfGpio.h"
#include <mutex>

std::mutex gpio_mutex;
auto base = std::make_shared<McuDigitalGpio>(HF_GPIO_NUM_2);
SfGpio safe_gpio(base, gpio_mutex);
```

All member functions of `SfGpio` automatically acquire the provided mutex before delegating to `McuDigitalGpio`.

## RAII Helpers

Classes like `DigitalOutputGuard` temporarily change pin state and restore it when leaving scope. Combine these with the thread‑safe wrappers for robust behavior.

## Guidelines

- Keep critical sections short to reduce contention.
- Avoid performing blocking I/O while holding a lock.
- Document which layer owns each mutex to prevent deadlocks.

Refer to the [Developer Guide](DeveloperGuide.md) for more advanced multi-threading patterns.

## 🧰 Choosing a Mutex Type
- Use `std::mutex` for standard use cases
- `std::recursive_mutex` may be necessary for nested locking

## 🛠️ Troubleshooting
- Deadlocks often indicate inconsistent lock order
- Use timeouts when waiting on locks in real-time tasks

## 🔗 Related Examples
- [🔒 Sensor Hub](../examples/sensor-hub.md)
