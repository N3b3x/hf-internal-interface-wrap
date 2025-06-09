# HF-IID-ESPIDF

**HF-IID-ESPIDF** provides internal interface drivers and utilities for ESP-IDF targets used by the HardFOC controller.

## ✨ Features

- GPIO, SPI and I2C abstractions
- Thread-safe bus drivers
- Platform utilities including base threading and mutex helpers

## 📦 Installation

Add `iid-espidf` as a dependency in your component configuration:

```cmake
idf_component_register(
    REQUIRES iid-espidf
)
```

The component exports the required include directories and depends on FreeRTOS.

## 🚀 Usage

Include the headers you need and initialize the drivers as shown in the documentation. Example:

```cpp
#include "DigitalOutput.h"
DigitalOutput led(GPIO_NUM_2);
led.init();
led.set(true);
```

## 📖 Documentation

Extensive documentation is available in the [docs](docs/index.md) folder.

## 📝 License

This project is licensed under the GNU General Public License v3.0 or later.
