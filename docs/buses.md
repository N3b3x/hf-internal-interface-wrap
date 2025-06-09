[⬅️ Prev](gpio.md) | [🔙 Index](index.md) | [Next ➡️](utilities.md)

# 📡 Bus Interfaces

HF-IID-ESPIDF contains drivers for common communication buses on ESP32 targets.

## SPI Bus

`SpiBus` provides master mode communication. A thread-safe version `SfSpiBus` is available when used with the HF-RTOSW mutex APIs.

## I2C Bus

The `I2cBus` class exposes standard master transactions. Use `SfI2cBus` for thread-safe access in multi-tasking environments.

```cpp
#include "SfI2cBus.h"
SfI2cBus bus(I2C_NUM_0);
bus.init();
```

[⬅️ Prev](gpio.md) | [🔙 Index](index.md) | [Next ➡️](utilities.md)
