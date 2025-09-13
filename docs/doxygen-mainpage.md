# HardFOC Interface Wrapper

A comprehensive multi-MCU hardware abstraction layer for HardFOC systems with unified API across multiple MCU platforms.

## Supported MCUs

- **ESP32 Family**: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2
- **Planned**: STM32F4, STM32H7, STM32G4, nRF52840, nRF5340

## Core Interfaces

### Base Classes
- [BaseAdc](@ref BaseAdc) - Analog-to-Digital Converter interface
- [BaseBluetooth](@ref BaseBluetooth) - Bluetooth communication interface
- [BaseCan](@ref BaseCan) - Controller Area Network interface
- [BaseGpio](@ref BaseGpio) - General Purpose Input/Output interface
- [BaseI2c](@ref BaseI2c) - Inter-Integrated Circuit interface
- [BaseLogger](@ref BaseLogger) - Logging system interface
- [BaseNvs](@ref BaseNvs) - Non-Volatile Storage interface
- [BasePeriodicTimer](@ref BasePeriodicTimer) - Periodic timer interface
- [BasePio](@ref BasePio) - Programmable Input/Output interface
- [BasePwm](@ref BasePwm) - Pulse Width Modulation interface
- [BaseSpi](@ref BaseSpi) - Serial Peripheral Interface
- [BaseTemperature](@ref BaseTemperature) - Temperature sensor interface
- [BaseUart](@ref BaseUart) - Universal Asynchronous Receiver-Transmitter
- [BaseWifi](@ref BaseWifi) - WiFi communication interface

### ESP32 Implementations
- [EspAdc](@ref EspAdc) - ESP32 ADC implementation
- [EspBluetooth](@ref EspBluetooth) - ESP32 Bluetooth implementation
- [EspCan](@ref EspCan) - ESP32 CAN implementation
- [EspGpio](@ref EspGpio) - ESP32 GPIO implementation
- [EspI2c](@ref EspI2c) - ESP32 I2C implementation
- [EspLogger](@ref EspLogger) - ESP32 Logger implementation
- [EspNvs](@ref EspNvs) - ESP32 NVS implementation
- [EspPeriodicTimer](@ref EspPeriodicTimer) - ESP32 Periodic Timer implementation
- [EspPio](@ref EspPio) - ESP32 PIO implementation
- [EspPwm](@ref EspPwm) - ESP32 PWM implementation
- [EspSpi](@ref EspSpi) - ESP32 SPI implementation
- [EspTemperature](@ref EspTemperature) - ESP32 Temperature implementation
- [EspUart](@ref EspUart) - ESP32 UART implementation
- [EspWifi](@ref EspWifi) - ESP32 WiFi implementation

### Utility Classes
- [AsciiArtGenerator](@ref AsciiArtGenerator) - ASCII art generation utility
- [DigitalOutputGuard](@ref DigitalOutputGuard) - Digital output protection utility
- [McuSelect](@ref McuSelect) - MCU platform selection utilities
- [RtosMutex](@ref RtosMutex) - RTOS mutex wrapper utilities

## Hardware Types

- [HardwareTypes](@ref HardwareTypes) - Common hardware type definitions

## Quick Start

```cpp
#include "base/BaseGpio.h"
#include "mcu/esp32/EspGpio.h"

// Initialize GPIO
EspGpio gpio;
if (gpio.EnsureInitialized()) {
    // Configure pin as output
    gpio.SetMode(2, HF_GPIO_MODE_OUTPUT);
    gpio.Write(2, HF_GPIO_STATE_HIGH);
}
```

## Features

- **Unified API** - Consistent interface across different MCU platforms
- **Type Safety** - Strong typing with project-specific typedefs
- **Error Handling** - Comprehensive error reporting and handling
- **Thread Safety** - RTOS-aware implementations
- **Memory Management** - Safe memory operations and guards
- **Logging** - Integrated logging system
- **Testing** - Comprehensive test suites for all modules

## Architecture

The HardFOC Interface Wrapper follows a layered architecture:

1. **Base Layer** - Abstract interfaces defining the API contract
2. **Implementation Layer** - MCU-specific implementations
3. **Utility Layer** - Common utilities and helpers
4. **Application Layer** - Example applications and tests

## License

This project is licensed under the GPL v3 License. See the LICENSE file for details.

## Contributing

Please see the main project repository for contributing guidelines and code of conduct.
