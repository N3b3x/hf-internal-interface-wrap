# HardwareTypes API Reference

<div align="center">

**📋 Navigation**

[← Previous: API Index](README.md) | [Back to API Index](README.md) | [Next: BaseGpio
→](BaseGpio.md)

</div>

---

## Overview

`HardwareTypes.h` defines platform-agnostic hardware type definitions for the HardFOC system.
These types provide a consistent API across different hardware platforms without exposing
MCU-specific implementation details.

## Design Philosophy

All base interface classes use these common types to ensure:
- **Platform Portability** - Code works across different microcontrollers
- **Consistent Naming** - Unified type names throughout the system
- **Future Extensibility** - Easy to modify underlying types if needed
- **Type Safety** - Strong typing to prevent common errors

## Header File

```cpp
#include "inc/base/HardwareTypes.h"
```text

## Core Integer Types

### Unsigned Integer Types

```cpp
using hf*u8*t = uint8*t;    // 8-bit unsigned (0 to 255)
using hf*u16*t = uint16*t;  // 16-bit unsigned (0 to 65,535)
using hf*u32*t = uint32*t;  // 32-bit unsigned (0 to 4,294,967,295)
using hf*u64*t = uint64*t;  // 64-bit unsigned (0 to 18,446,744,073,709,551,615)
```text

### Signed Integer Types

```cpp
using hf*i8*t = int8*t;     // 8-bit signed (-128 to 127)
using hf*i16*t = int16*t;   // 16-bit signed (-32,768 to 32,767)
using hf*i32*t = int32*t;   // 32-bit signed (-2,147,483,648 to 2,147,483,647)
using hf*i64*t = int64*t; // 64-bit signed (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
```text

### Boolean Type

```cpp
using hf*bool*t = bool;     // Platform-agnostic boolean type
```text

## Hardware-Specific Types

### GPIO Pin Types

```cpp
using hf*pin*num*t = hf*i32*t;  // GPIO pin number type
```text

**Constants:**
```cpp
constexpr hf*pin*num*t HF*INVALID*PIN = -1;        // Invalid/unassigned pin
constexpr hf*pin*num*t HF*MAX*PIN*NUMBER = 255;    // Maximum supported pin number
```text

**Validation:**
```cpp
constexpr bool IsValidPin(hf*pin*num*t pin) noexcept;
```text

### Port and Controller Types

```cpp
using hf*port*num*t = hf*u32*t;  // Communication port identifier
using hf*host*id*t = hf*u32*t;   // Host/controller identifier
```text

**Constants:**
```cpp
constexpr hf*port*num*t HF*INVALID*PORT = std::numeric*limits<hf*port*num*t>::max();
constexpr hf*host*id*t HF*INVALID*HOST = std::numeric*limits<hf*host*id*t>::max();
```text

**Validation:**
```cpp
constexpr bool IsValidPort(hf*port*num*t port) noexcept;
constexpr bool IsValidHost(hf*host*id*t host) noexcept;
```text

### Channel Types

```cpp
using hf*channel*id*t = hf*u32*t;  // ADC/PWM/DMA channel identifier
```text

**Constants:**
```cpp
constexpr hf*channel*id*t HF*INVALID*CHANNEL = std::numeric*limits<hf*channel*id*t>::max();
```text

**Validation:**
```cpp
constexpr bool IsValidChannel(hf*channel*id*t channel) noexcept;
```text

## Communication Types

### Frequency Types

```cpp
using hf*frequency*hz*t = hf*u32*t;  // Frequency in Hz
using hf*frequency*t = hf*frequency*hz*t;  // Backward compatibility alias
using hf*baud*rate*t = hf*u32*t;     // UART baud rate
```text

### Timing Types

```cpp
using hf*time*t = hf*u32*t;          // Time in milliseconds
using hf*timeout*ms*t = hf*time*t;   // Timeout value in milliseconds
```text

**Timeout Constants:**
```cpp
constexpr hf*time*t HF*TIMEOUT*DEFAULT*MS = 1000;  // Default 1 second timeout
constexpr hf*time*t HF*TIMEOUT*NONE = 0;           // No timeout (wait indefinitely)
constexpr hf*time*t HF*TIMEOUT*MAX = std::numeric*limits<hf*time*t>::max();  // Maximum timeout
```text

## Usage Examples

### GPIO Pin Validation

```cpp
#include "inc/base/HardwareTypes.h"

bool configure*gpio*pin(hf*pin*num*t pin) {
    // Validate pin number before use
    if (!IsValidPin(pin)) {
        printf("Invalid pin number: %d\n", pin);
        return false;
    }
    
    if (pin == HF*INVALID*PIN) {
        printf("Pin not assigned\n");
        return false;
    }
    
    // Pin is valid, proceed with configuration
    printf("Configuring GPIO pin %d\n", pin);
    return true;
}

void test*pin*validation() {
    configure*gpio*pin(2);    // Valid: true
    configure*gpio*pin(-1);   // Invalid: HF*INVALID*PIN
    configure*gpio*pin(300);  // Invalid: exceeds HF*MAX*PIN*NUMBER
}
```text

### Communication Port Configuration

```cpp
bool setup*i2c*port(hf*port*num*t port, hf*frequency*hz*t frequency) {
    // Validate port
    if (!IsValidPort(port)) {
        printf("Invalid I2C port: %u\n", port);
        return false;
    }
    
    // Validate frequency range (typical I2C: 100kHz to 1MHz)
    if (frequency < 100000 || frequency > 1000000) {
        printf("Invalid I2C frequency: %u Hz\n", frequency);
        return false;
    }
    
    printf("Setting up I2C port %u at %u Hz\n", port, frequency);
    return true;
}

void test*i2c*setup() {
    setup*i2c*port(0, 400000);           // Valid: I2C port 0 at 400kHz
    setup*i2c*port(HF*INVALID*PORT, 400000);  // Invalid port
    setup*i2c*port(1, 50000);            // Invalid frequency (too low)
}
```text

### ADC Channel Management

```cpp
class SensorManager {
private:
    static constexpr hf*u8*t MAX*SENSORS = 8;
    hf*channel*id*t sensor*channels*[MAX*SENSORS];
    
public:
    SensorManager() {
        // Initialize all channels as invalid
        for (hf*u8*t i = 0; i < MAX*SENSORS; i++) {
            sensor*channels*[i] = HF*INVALID*CHANNEL;
        }
    }
    
    bool add*sensor(hf*u8*t sensor*index, hf*channel*id*t channel) {
        if (sensor*index >= MAX*SENSORS) {
            return false;
        }
        
        if (!IsValidChannel(channel)) {
            printf("Invalid ADC channel: %u\n", channel);
            return false;
        }
        
        sensor*channels*[sensor*index] = channel;
        printf("Sensor %u assigned to ADC channel %u\n", sensor*index, channel);
        return true;
    }
    
    hf*channel*id*t get*sensor*channel(hf*u8*t sensor*index) const {
        if (sensor*index >= MAX*SENSORS) {
            return HF*INVALID*CHANNEL;
        }
        return sensor*channels*[sensor*index];
    }
    
    bool is*sensor*configured(hf*u8*t sensor*index) const {
        hf*channel*id*t channel = get*sensor*channel(sensor*index);
        return IsValidChannel(channel);
    }
};
```text

### Timeout Handling

```cpp
enum class OperationResult {
    SUCCESS,
    TIMEOUT,
    ERROR
};

OperationResult wait*for*data(hf*timeout*ms*t timeout) {
    hf*time*t start*time = get*current*time*ms();
    
    while (true) {
        if (data*available()) {
            return OperationResult::SUCCESS;
        }
        
        if (timeout != HF*TIMEOUT*NONE) {  // Check for timeout
            hf*time*t elapsed = get*current*time*ms() - start*time;
            if (elapsed >= timeout) {
                printf("Operation timed out after %u ms\n", timeout);
                return OperationResult::TIMEOUT;
            }
        }
        
        vTaskDelay(pdMS*TO*TICKS(1));  // Small delay
    }
}

void test*timeout*handling() {
    // Wait with default timeout
    OperationResult result1 = wait*for*data(HF*TIMEOUT*DEFAULT*MS);
    
    // Wait indefinitely
    OperationResult result2 = wait*for*data(HF*TIMEOUT*NONE);
    
    // Wait with custom timeout
    OperationResult result3 = wait*for*data(500);  // 500ms timeout
}
```text

### Type-Safe Configuration Structures

```cpp
struct GpioConfig {
    hf*pin*num*t pin;
    hf*gpio*direction*t direction;
    hf*gpio*active*state*t active*state;
    hf*gpio*pull*mode*t pull*mode;
    
    // Constructor with validation
    GpioConfig(hf*pin*num*t p, hf*gpio*direction*t dir, 
               hf*gpio*active*state*t active = hf*gpio*active*state*t::HF*GPIO*ACTIVE*HIGH,
               hf*gpio*pull*mode*t pull = hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*FLOATING)
        : pin(p), direction(dir), active*state(active), pull*mode(pull) {
        
        if (!IsValidPin(pin)) {
            throw std::invalid*argument("Invalid GPIO pin number");
        }
    }
    
    bool is*valid() const {
        return IsValidPin(pin);
    }
};

struct I2cConfig {
    hf*port*num*t port;
    hf*frequency*hz*t frequency;
    hf*timeout*ms*t timeout;
    
    I2cConfig(hf*port*num*t p, hf*frequency*hz*t freq, 
              hf*timeout*ms*t to = HF*TIMEOUT*DEFAULT*MS)
        : port(p), frequency(freq), timeout(to) {
        
        if (!IsValidPort(port)) {
            throw std::invalid*argument("Invalid I2C port");
        }
    }
};
```text

## Type Conversion Utilities

```cpp
// Safe conversion with bounds checking
template<typename T, typename U>
constexpr bool safe*cast(U value, T& result) noexcept {
    if (value < std::numeric*limits<T>::min() || 
        value > std::numeric*limits<T>::max()) {
        return false;
    }
    result = static*cast<T>(value);
    return true;
}

// Example usage
bool convert*pin*number(int input*pin, hf*pin*num*t& output*pin) {
    return safe*cast(input*pin, output*pin) && IsValidPin(output*pin);
}
```text

## Best Practices

### Type Usage Guidelines

1. **Always use HardFOC types** instead of raw integer types in public APIs
2. **Validate inputs** using the provided validation functions
3. **Use constants** instead of magic numbers (e.g., `HF*INVALID*PIN` vs `-1`)
4. **Check for invalid values** before performing operations
5. **Use appropriate sized types** for the data range (e.g., `hf*u8*t` for small counts)

### Performance Considerations

- All types are compile-time aliases with zero runtime overhead
- Validation functions are `constexpr` and can be evaluated at compile time
- Constants are compile-time evaluated and don't consume memory

### Platform Portability

- Types automatically adapt to the underlying platform's integer sizes
- Code using these types will compile and run on any supported platform
- No platform-specific `#ifdef` blocks needed in application code

## Compilation Requirements

- **C++11 or later** - Required for `constexpr` and type aliases
- **Standard Library** - Uses `<cstdint>` and `<limits>`
- **Header-Only** - No separate compilation required

## Related Documentation

- [BaseGpio API Reference](BaseGpio.md) - GPIO type usage examples
- [BaseAdc API Reference](BaseAdc.md) - ADC channel type usage
- [EspGpio API Reference](../esp_api/EspGpio.md) - ESP32-specific type mappings

---

<div align="center">

**📋 Navigation**

[← Previous: API Index](README.md) | [Back to API Index](README.md) | [Next: BaseGpio
→](BaseGpio.md)

</div>