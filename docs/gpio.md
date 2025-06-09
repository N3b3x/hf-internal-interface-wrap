[⬅️ Back to Index](index.md) | [Next ➡️](buses.md)

# 🔌 GPIO Basics

This section covers the general purpose I/O abstractions provided by HF-IID-ESPIDF.

## Overview

The drivers wrap ESP-IDF GPIO functionality, providing a clean object-oriented interface for digital inputs and outputs. Lazy initialization keeps resource usage minimal until the pin is needed.

- **BaseGpio** – common functionality for GPIO pins.
- **DigitalInput / DigitalOutput** – simple input/output helpers.
- **DigitalExternalIRQ** – configure interrupts on GPIO pins.

## Usage Tips

```cpp
#include "DigitalOutput.h"

DigitalOutput led(GPIO_NUM_2);
led.init();
led.set(true);
```

[⬅️ Back to Index](index.md) | [Next ➡️](buses.md)
