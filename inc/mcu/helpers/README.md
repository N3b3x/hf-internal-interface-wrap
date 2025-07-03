# MCU Types Modular Organization

This directory contains the modularized MCU-specific type definitions for the HardFOC hardware abstraction layer. The original `McuTypes.h` file has been split into multiple specialized headers for better organization and maintainability.

## File Structure

### Base Header
- **`McuTypes_Base.h`** - Common base types, includes, constants, and macros shared across all peripherals

### Peripheral-Specific Headers
- **`McuTypes_ADC.h`** - ADC (Analog-to-Digital Converter) types and constants
- **`McuTypes_GPIO.h`** - GPIO (General Purpose Input/Output) types and constants
- **`McuTypes_CAN.h`** - CAN/TWAI (Controller Area Network) types and constants
- **`McuTypes_UART.h`** - UART (Universal Asynchronous Receiver-Transmitter) types and constants
- **`McuTypes_SPI.h`** - SPI (Serial Peripheral Interface) types and constants
- **`McuTypes_I2C.h`** - I2C (Inter-Integrated Circuit) types and constants
- **`McuTypes_PWM.h`** - PWM/LEDC (Pulse Width Modulation) types and constants
- **`McuTypes_RMT.h`** - RMT (Remote Control) types and constants

### Consolidated Header
- **`McuTypes_Consolidated.h`** - Includes all peripheral headers for backward compatibility

## Usage

### For New Code
Include only the specific peripheral headers you need:

```cpp
#include "McuTypes_Base.h"     // Always include for base types
#include "McuTypes_GPIO.h"     // For GPIO operations
#include "McuTypes_CAN.h"      // For CAN operations
```

### For Backward Compatibility
If you need all types (legacy usage), include the consolidated header:

```cpp
#include "McuTypes_Consolidated.h"  // Includes all peripheral types
```

### For the Original Interface
The original `McuTypes.h` file can be replaced with `McuTypes_Consolidated.h` or updated to include it.

## Benefits of Modular Organization

1. **Reduced Compilation Time** - Only include headers for peripherals you actually use
2. **Better Code Organization** - Related types are grouped together
3. **Easier Maintenance** - Changes to one peripheral don't affect others
4. **Cleaner Dependencies** - Clear separation of concerns
5. **Scalability** - Easy to add new peripheral types

## ESP32C6 Features Supported

Each peripheral header includes full support for ESP32C6/ESP-IDF v5.5+ features:

- **CAN/TWAI**: Dual controller support, modern node-based API
- **GPIO**: Advanced glitch filters, RTC GPIO, low-power modes
- **PWM/LEDC**: 14-bit resolution, hardware fade, multiple clock sources
- **ADC**: 12-bit SAR ADC, calibration, continuous mode with DMA
- **RMT**: 4 channels, DMA support, carrier modulation
- **UART**: 3 ports, flow control, power management
- **SPI**: 3 hosts, high-speed operation, DMA support
- **I2C**: 2 controllers, master mode, high-speed operation

## Platform Support

All headers support both ESP32 platforms and generic platforms:

- **ESP32 Family**: Full native ESP-IDF type mappings
- **Generic Platforms**: Simplified type definitions for portability

## Validation Macros

Each peripheral header includes validation macros for parameter checking:

```cpp
HF_GPIO_IS_VALID_GPIO(pin)
HF_CAN_IS_VALID_BAUDRATE(rate)
HF_PWM_IS_VALID_FREQUENCY(freq)
// ... and many more
```

## Migration Guide

To migrate from the original consolidated file:

1. **Identify Dependencies**: Determine which peripherals your code uses
2. **Include Specific Headers**: Replace the consolidated include with specific ones
3. **Update Build System**: Ensure new headers are in the include path
4. **Test Compilation**: Verify all types are still accessible

## Contributing

When adding new peripheral types:

1. Create a new `McuTypes_[PERIPHERAL].h` file
2. Follow the existing pattern with base include and platform-specific sections
3. Add the include to `McuTypes_Consolidated.h`
4. Update this README with the new peripheral information
