# HardwareTypes API Reference

## Overview

`HardwareTypes.h` defines platform-agnostic hardware type definitions for the HardFOC system. These types provide a consistent API across different hardware platforms without exposing MCU-specific implementation details.

## Design Philosophy

All base interface classes use these common types to ensure:
- **Platform Portability** - Code works across different microcontrollers
- **Consistent Naming** - Unified type names throughout the system
- **Future Extensibility** - Easy to modify underlying types if needed
- **Type Safety** - Strong typing to prevent common errors

## Header File

```cpp
#include "inc/base/HardwareTypes.h"
```

## Core Integer Types

### Unsigned Integer Types

```cpp
using hf_u8_t = uint8_t;    // 8-bit unsigned (0 to 255)
using hf_u16_t = uint16_t;  // 16-bit unsigned (0 to 65,535)
using hf_u32_t = uint32_t;  // 32-bit unsigned (0 to 4,294,967,295)
using hf_u64_t = uint64_t;  // 64-bit unsigned (0 to 18,446,744,073,709,551,615)
```

### Signed Integer Types

```cpp
using hf_i8_t = int8_t;     // 8-bit signed (-128 to 127)
using hf_i16_t = int16_t;   // 16-bit signed (-32,768 to 32,767)
using hf_i32_t = int32_t;   // 32-bit signed (-2,147,483,648 to 2,147,483,647)
using hf_i64_t = int64_t;   // 64-bit signed (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
```

### Boolean Type

```cpp
using hf_bool_t = bool;     // Platform-agnostic boolean type
```

## Hardware-Specific Types

### GPIO Pin Types

```cpp
using hf_pin_num_t = hf_i32_t;  // GPIO pin number type
```

**Constants:**
```cpp
constexpr hf_pin_num_t HF_INVALID_PIN = -1;        // Invalid/unassigned pin
constexpr hf_pin_num_t HF_MAX_PIN_NUMBER = 255;    // Maximum supported pin number
```

**Validation:**
```cpp
constexpr bool IsValidPin(hf_pin_num_t pin) noexcept;
```

### Port and Controller Types

```cpp
using hf_port_num_t = hf_u32_t;  // Communication port identifier
using hf_host_id_t = hf_u32_t;   // Host/controller identifier
```

**Constants:**
```cpp
constexpr hf_port_num_t HF_INVALID_PORT = std::numeric_limits<hf_port_num_t>::max();
constexpr hf_host_id_t HF_INVALID_HOST = std::numeric_limits<hf_host_id_t>::max();
```

**Validation:**
```cpp
constexpr bool IsValidPort(hf_port_num_t port) noexcept;
constexpr bool IsValidHost(hf_host_id_t host) noexcept;
```

### Channel Types

```cpp
using hf_channel_id_t = hf_u32_t;  // ADC/PWM/DMA channel identifier
```

**Constants:**
```cpp
constexpr hf_channel_id_t HF_INVALID_CHANNEL = std::numeric_limits<hf_channel_id_t>::max();
```

**Validation:**
```cpp
constexpr bool IsValidChannel(hf_channel_id_t channel) noexcept;
```

## Communication Types

### Frequency Types

```cpp
using hf_frequency_hz_t = hf_u32_t;  // Frequency in Hz
using hf_frequency_t = hf_frequency_hz_t;  // Backward compatibility alias
using hf_baud_rate_t = hf_u32_t;     // UART baud rate
```

### Timing Types

```cpp
using hf_time_t = hf_u32_t;          // Time in milliseconds
using hf_timeout_ms_t = hf_time_t;   // Timeout value in milliseconds
```

**Timeout Constants:**
```cpp
constexpr hf_time_t HF_TIMEOUT_DEFAULT_MS = 1000;  // Default 1 second timeout
constexpr hf_time_t HF_TIMEOUT_NONE = 0;           // No timeout (wait indefinitely)
constexpr hf_time_t HF_TIMEOUT_MAX = std::numeric_limits<hf_time_t>::max();  // Maximum timeout
```

## Usage Examples

### GPIO Pin Validation

```cpp
#include "inc/base/HardwareTypes.h"

bool configure_gpio_pin(hf_pin_num_t pin) {
    // Validate pin number before use
    if (!IsValidPin(pin)) {
        printf("Invalid pin number: %d\n", pin);
        return false;
    }
    
    if (pin == HF_INVALID_PIN) {
        printf("Pin not assigned\n");
        return false;
    }
    
    // Pin is valid, proceed with configuration
    printf("Configuring GPIO pin %d\n", pin);
    return true;
}

void test_pin_validation() {
    configure_gpio_pin(2);    // Valid: true
    configure_gpio_pin(-1);   // Invalid: HF_INVALID_PIN
    configure_gpio_pin(300);  // Invalid: exceeds HF_MAX_PIN_NUMBER
}
```

### Communication Port Configuration

```cpp
bool setup_i2c_port(hf_port_num_t port, hf_frequency_hz_t frequency) {
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

void test_i2c_setup() {
    setup_i2c_port(0, 400000);           // Valid: I2C port 0 at 400kHz
    setup_i2c_port(HF_INVALID_PORT, 400000);  // Invalid port
    setup_i2c_port(1, 50000);            // Invalid frequency (too low)
}
```

### ADC Channel Management

```cpp
class SensorManager {
private:
    static constexpr hf_u8_t MAX_SENSORS = 8;
    hf_channel_id_t sensor_channels_[MAX_SENSORS];
    
public:
    SensorManager() {
        // Initialize all channels as invalid
        for (hf_u8_t i = 0; i < MAX_SENSORS; i++) {
            sensor_channels_[i] = HF_INVALID_CHANNEL;
        }
    }
    
    bool add_sensor(hf_u8_t sensor_index, hf_channel_id_t channel) {
        if (sensor_index >= MAX_SENSORS) {
            return false;
        }
        
        if (!IsValidChannel(channel)) {
            printf("Invalid ADC channel: %u\n", channel);
            return false;
        }
        
        sensor_channels_[sensor_index] = channel;
        printf("Sensor %u assigned to ADC channel %u\n", sensor_index, channel);
        return true;
    }
    
    hf_channel_id_t get_sensor_channel(hf_u8_t sensor_index) const {
        if (sensor_index >= MAX_SENSORS) {
            return HF_INVALID_CHANNEL;
        }
        return sensor_channels_[sensor_index];
    }
    
    bool is_sensor_configured(hf_u8_t sensor_index) const {
        hf_channel_id_t channel = get_sensor_channel(sensor_index);
        return IsValidChannel(channel);
    }
};
```

### Timeout Handling

```cpp
enum class OperationResult {
    SUCCESS,
    TIMEOUT,
    ERROR
};

OperationResult wait_for_data(hf_timeout_ms_t timeout) {
    hf_time_t start_time = get_current_time_ms();
    
    while (true) {
        if (data_available()) {
            return OperationResult::SUCCESS;
        }
        
        if (timeout != HF_TIMEOUT_NONE) {  // Check for timeout
            hf_time_t elapsed = get_current_time_ms() - start_time;
            if (elapsed >= timeout) {
                printf("Operation timed out after %u ms\n", timeout);
                return OperationResult::TIMEOUT;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));  // Small delay
    }
}

void test_timeout_handling() {
    // Wait with default timeout
    OperationResult result1 = wait_for_data(HF_TIMEOUT_DEFAULT_MS);
    
    // Wait indefinitely
    OperationResult result2 = wait_for_data(HF_TIMEOUT_NONE);
    
    // Wait with custom timeout
    OperationResult result3 = wait_for_data(500);  // 500ms timeout
}
```

### Type-Safe Configuration Structures

```cpp
struct GpioConfig {
    hf_pin_num_t pin;
    hf_gpio_direction_t direction;
    hf_gpio_active_state_t active_state;
    hf_gpio_pull_mode_t pull_mode;
    
    // Constructor with validation
    GpioConfig(hf_pin_num_t p, hf_gpio_direction_t dir, 
               hf_gpio_active_state_t active = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
               hf_gpio_pull_mode_t pull = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING)
        : pin(p), direction(dir), active_state(active), pull_mode(pull) {
        
        if (!IsValidPin(pin)) {
            throw std::invalid_argument("Invalid GPIO pin number");
        }
    }
    
    bool is_valid() const {
        return IsValidPin(pin);
    }
};

struct I2cConfig {
    hf_port_num_t port;
    hf_frequency_hz_t frequency;
    hf_timeout_ms_t timeout;
    
    I2cConfig(hf_port_num_t p, hf_frequency_hz_t freq, 
              hf_timeout_ms_t to = HF_TIMEOUT_DEFAULT_MS)
        : port(p), frequency(freq), timeout(to) {
        
        if (!IsValidPort(port)) {
            throw std::invalid_argument("Invalid I2C port");
        }
    }
};
```

## Type Conversion Utilities

```cpp
// Safe conversion with bounds checking
template<typename T, typename U>
constexpr bool safe_cast(U value, T& result) noexcept {
    if (value < std::numeric_limits<T>::min() || 
        value > std::numeric_limits<T>::max()) {
        return false;
    }
    result = static_cast<T>(value);
    return true;
}

// Example usage
bool convert_pin_number(int input_pin, hf_pin_num_t& output_pin) {
    return safe_cast(input_pin, output_pin) && IsValidPin(output_pin);
}
```

## Best Practices

### Type Usage Guidelines

1. **Always use HardFOC types** instead of raw integer types in public APIs
2. **Validate inputs** using the provided validation functions
3. **Use constants** instead of magic numbers (e.g., `HF_INVALID_PIN` vs `-1`)
4. **Check for invalid values** before performing operations
5. **Use appropriate sized types** for the data range (e.g., `hf_u8_t` for small counts)

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
- [EspGpio API Reference](EspGpio.md) - ESP32-specific type mappings