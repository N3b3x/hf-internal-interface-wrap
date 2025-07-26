# 🔧 HardwareTypes API Reference

<div align="center">

![HardwareTypes](https://img.shields.io/badge/HardwareTypes-Type%20System-blue?style=for-the-badge&logo=typescript)

**🎯 Platform-agnostic type definitions for consistent hardware interfaces**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [📊 **Integer Types**](#-integer-types)
- [🔌 **Hardware-Specific Types**](#-hardware-specific-types)
- [⏰ **Timing Types**](#-timing-types)
- [🚫 **Invalid Value Constants**](#-invalid-value-constants)
- [🔧 **Utility Functions**](#-utility-functions)
- [💡 **Usage Examples**](#-usage-examples)

---

## 🎯 **Overview**

The `HardwareTypes.h` header provides platform-agnostic type definitions that ensure consistency across all hardware interfaces in the HardFOC Internal Interface Wrapper system. These types abstract away platform-specific differences and provide a unified type system.

### ✨ **Key Benefits**

- **🔒 Type Consistency**: All interfaces use the same wrapped type system
- **🔄 Portability**: Easy to adapt to different platforms and architectures
- **🎯 Clarity**: Clear distinction between platform types and wrapped types
- **🛡️ Safety**: Prevents type mismatches and improves compile-time checking

---

## 📊 **Integer Types**

### 🔢 **Unsigned Integer Types**

```cpp
/**
 * @brief 8-bit unsigned integer type
 * 
 * Platform-agnostic wrapper for uint8_t, used throughout the system
 * for byte-sized values, flags, and small counters.
 */
using hf_u8_t = uint8_t;

/**
 * @brief 16-bit unsigned integer type
 * 
 * Platform-agnostic wrapper for uint16_t, commonly used for
 * ADC values, small counts, and 16-bit register values.
 */
using hf_u16_t = uint16_t;

/**
 * @brief 32-bit unsigned integer type
 * 
 * Platform-agnostic wrapper for uint32_t, the most common
 * integer type for counts, IDs, and general-purpose values.
 */
using hf_u32_t = uint32_t;

/**
 * @brief 64-bit unsigned integer type
 * 
 * Platform-agnostic wrapper for uint64_t, used for
 * large counts, timestamps, and 64-bit operations.
 */
using hf_u64_t = uint64_t;
```

### 🔢 **Signed Integer Types**

```cpp
/**
 * @brief 8-bit signed integer type
 * 
 * Platform-agnostic wrapper for int8_t, used for
 * small signed values and offsets.
 */
using hf_i8_t = int8_t;

/**
 * @brief 16-bit signed integer type
 * 
 * Platform-agnostic wrapper for int16_t, used for
 * signed counts and medium-range values.
 */
using hf_i16_t = int16_t;

/**
 * @brief 32-bit signed integer type
 * 
 * Platform-agnostic wrapper for int32_t, commonly used
 * for pin numbers (supporting negative invalid values).
 */
using hf_i32_t = int32_t;

/**
 * @brief 64-bit signed integer type
 * 
 * Platform-agnostic wrapper for int64_t, used for
 * large signed values and extended precision.
 */
using hf_i64_t = int64_t;
```

---

## 🔌 **Hardware-Specific Types**

### 📍 **Pin and Channel Types**

```cpp
/**
 * @brief GPIO pin number type
 * 
 * Represents GPIO pin numbers across different platforms.
 * Uses signed integer to support invalid pin constant (-1).
 * 
 * @note Range varies by platform:
 *       - ESP32-C6: 0-30 (some pins reserved)
 *       - STM32: 0-15 per port
 *       - Invalid: -1 (HF_INVALID_PIN)
 */
using hf_pin_num_t = hf_i32_t;

/**
 * @brief Channel identifier type
 * 
 * Used for ADC channels, PWM channels, and other
 * hardware channels that need unique identification.
 * 
 * @note Common ranges:
 *       - ADC channels: 0-7 typical
 *       - PWM channels: 0-15 typical
 *       - Invalid: HF_INVALID_CHANNEL
 */
using hf_channel_id_t = hf_u32_t;

/**
 * @brief Communication port number type
 * 
 * Identifies communication ports (I2C, SPI, UART, CAN).
 * 
 * @note Platform examples:
 *       - ESP32-C6 I2C: 0-1
 *       - ESP32-C6 SPI: 0-3
 *       - ESP32-C6 UART: 0-5
 */
using hf_port_num_t = hf_u32_t;

/**
 * @brief Host controller identifier type
 * 
 * Used to identify host controllers for SPI, I2C,
 * and other multi-host interfaces.
 */
using hf_host_id_t = hf_u32_t;
```

### 🎛️ **Communication Types**

```cpp
/**
 * @brief Baud rate type for UART communication
 * 
 * Represents baud rates in bits per second.
 * 
 * @note Common values:
 *       - 9600, 19200, 38400, 57600, 115200
 *       - 230400, 460800, 921600
 */
using hf_baud_rate_t = hf_u32_t;

/**
 * @brief I2C device address type
 * 
 * 7-bit or 10-bit I2C device addresses.
 * 
 * @note Address ranges:
 *       - 7-bit: 0x08-0x77 (0x00-0x07 and 0x78-0x7F reserved)
 *       - 10-bit: 0x000-0x3FF
 */
using hf_i2c_address_t = hf_u16_t;

/**
 * @brief CAN message ID type
 * 
 * CAN message identifiers for standard and extended frames.
 * 
 * @note ID ranges:
 *       - Standard CAN: 11-bit (0x000-0x7FF)
 *       - Extended CAN: 29-bit (0x00000000-0x1FFFFFFF)
 */
using hf_can_id_t = hf_u32_t;
```

---

## ⏰ **Timing Types**

### 🕐 **Time and Duration Types**

```cpp
/**
 * @brief Time value in milliseconds
 * 
 * Used for delays, timeouts, and general timing operations.
 * Provides millisecond precision for most timing needs.
 */
using hf_time_t = hf_u32_t;

/**
 * @brief Timeout value in milliseconds
 * 
 * Specific type for timeout values to improve code clarity.
 * Same underlying type as hf_time_t but semantically different.
 */
using hf_timeout_ms_t = hf_time_t;

/**
 * @brief Microsecond timestamp type
 * 
 * High-precision timestamps in microseconds.
 * Used for performance measurement and precise timing.
 */
using hf_timestamp_us_t = hf_u64_t;

/**
 * @brief Frequency in Hertz
 * 
 * Represents frequencies for PWM, SPI, I2C, and other
 * interfaces that require frequency specification.
 * 
 * @note Common ranges:
 *       - I2C: 100,000 Hz (100 kHz), 400,000 Hz (400 kHz)
 *       - SPI: 1,000,000 Hz (1 MHz) to 80,000,000 Hz (80 MHz)
 *       - PWM: 1,000 Hz (1 kHz) to 40,000,000 Hz (40 MHz)
 */
using hf_frequency_hz_t = hf_u32_t;
```

### ⏱️ **Period and Interval Types**

```cpp
/**
 * @brief Period in milliseconds
 * 
 * Used for periodic timers and repeating operations.
 */
using hf_period_ms_t = hf_u32_t;

/**
 * @brief Interval in microseconds
 * 
 * High-precision intervals for precise timing operations.
 */
using hf_interval_us_t = hf_u32_t;
```

---

## 🚫 **Invalid Value Constants**

### 🚫 **Pin and Channel Constants**

```cpp
/**
 * @brief Invalid GPIO pin constant
 * 
 * Used to indicate an invalid or unassigned GPIO pin.
 * Value of -1 allows easy checking for validity.
 */
constexpr hf_pin_num_t HF_INVALID_PIN = -1;

/**
 * @brief Invalid channel constant
 * 
 * Indicates an invalid or unassigned channel.
 */
constexpr hf_channel_id_t HF_INVALID_CHANNEL = std::numeric_limits<hf_channel_id_t>::max();

/**
 * @brief Invalid port constant
 * 
 * Indicates an invalid communication port.
 */
constexpr hf_port_num_t HF_INVALID_PORT = std::numeric_limits<hf_port_num_t>::max();

/**
 * @brief Invalid host constant
 * 
 * Indicates an invalid host controller.
 */
constexpr hf_host_id_t HF_INVALID_HOST = std::numeric_limits<hf_host_id_t>::max();
```

### ⏰ **Timeout Constants**

```cpp
/**
 * @brief Default timeout in milliseconds
 * 
 * Standard timeout value for operations that need a reasonable default.
 */
constexpr hf_timeout_ms_t HF_TIMEOUT_DEFAULT_MS = 1000;

/**
 * @brief No timeout (immediate return)
 * 
 * Used for non-blocking operations.
 */
constexpr hf_timeout_ms_t HF_TIMEOUT_NONE = 0;

/**
 * @brief Infinite timeout
 * 
 * Operations will wait indefinitely.
 */
constexpr hf_timeout_ms_t HF_TIMEOUT_INFINITE = std::numeric_limits<hf_timeout_ms_t>::max();

/**
 * @brief Maximum timeout value
 * 
 * Largest possible timeout value.
 */
constexpr hf_timeout_ms_t HF_TIMEOUT_MAX = std::numeric_limits<hf_timeout_ms_t>::max() - 1;
```

---

## 🔧 **Utility Functions**

### ✅ **Validation Functions**

```cpp
/**
 * @brief Check if a pin number is valid
 * 
 * @param pin Pin number to validate
 * @return true if pin is valid, false otherwise
 */
inline bool IsValidPin(hf_pin_num_t pin) {
    return pin != HF_INVALID_PIN && pin >= 0;
}

/**
 * @brief Check if a channel ID is valid
 * 
 * @param channel Channel ID to validate
 * @return true if channel is valid, false otherwise
 */
inline bool IsValidChannel(hf_channel_id_t channel) {
    return channel != HF_INVALID_CHANNEL;
}

/**
 * @brief Check if a port number is valid
 * 
 * @param port Port number to validate
 * @return true if port is valid, false otherwise
 */
inline bool IsValidPort(hf_port_num_t port) {
    return port != HF_INVALID_PORT;
}

/**
 * @brief Check if a host ID is valid
 * 
 * @param host Host ID to validate
 * @return true if host is valid, false otherwise
 */
inline bool IsValidHost(hf_host_id_t host) {
    return host != HF_INVALID_HOST;
}
```

### 🔄 **Conversion Utilities**

```cpp
/**
 * @brief Convert milliseconds to microseconds
 * 
 * @param ms Time in milliseconds
 * @return Time in microseconds
 */
inline hf_timestamp_us_t MillisecondsToMicroseconds(hf_time_t ms) {
    return static_cast<hf_timestamp_us_t>(ms) * 1000ULL;
}

/**
 * @brief Convert microseconds to milliseconds
 * 
 * @param us Time in microseconds
 * @return Time in milliseconds
 */
inline hf_time_t MicrosecondsToMilliseconds(hf_timestamp_us_t us) {
    return static_cast<hf_time_t>(us / 1000ULL);
}

/**
 * @brief Convert frequency to period in microseconds
 * 
 * @param frequency_hz Frequency in Hz
 * @return Period in microseconds
 */
inline hf_interval_us_t FrequencyToPeriodUs(hf_frequency_hz_t frequency_hz) {
    return frequency_hz > 0 ? (1000000UL / frequency_hz) : 0;
}

/**
 * @brief Convert period to frequency
 * 
 * @param period_us Period in microseconds
 * @return Frequency in Hz
 */
inline hf_frequency_hz_t PeriodToFrequencyHz(hf_interval_us_t period_us) {
    return period_us > 0 ? (1000000UL / period_us) : 0;
}
```

---

## 💡 **Usage Examples**

### 🔌 **GPIO Pin Declaration**

```cpp
#include "base/HardwareTypes.h"

// Declare GPIO pins using wrapped types
constexpr hf_pin_num_t LED_PIN = 2;
constexpr hf_pin_num_t BUTTON_PIN = 0;
constexpr hf_pin_num_t UNUSED_PIN = HF_INVALID_PIN;

// Validate pins before use
if (IsValidPin(LED_PIN)) {
    // Safe to use LED_PIN
    printf("LED pin %d is valid\n", LED_PIN);
}

if (!IsValidPin(UNUSED_PIN)) {
    printf("Unused pin is invalid (value: %d)\n", UNUSED_PIN);
}
```

### 📊 **ADC Channel Configuration**

```cpp
#include "base/HardwareTypes.h"

// ADC channel setup
constexpr hf_channel_id_t VOLTAGE_CHANNEL = 0;
constexpr hf_channel_id_t CURRENT_CHANNEL = 1;
constexpr hf_channel_id_t TEMPERATURE_CHANNEL = 2;

// Channel validation
std::vector<hf_channel_id_t> channels = {
    VOLTAGE_CHANNEL,
    CURRENT_CHANNEL,
    TEMPERATURE_CHANNEL
};

for (const auto& channel : channels) {
    if (IsValidChannel(channel)) {
        printf("Channel %u is configured\n", channel);
    }
}
```

### 🔄 **Communication Port Setup**

```cpp
#include "base/HardwareTypes.h"

// Communication port configuration
constexpr hf_port_num_t I2C_PORT = 0;
constexpr hf_port_num_t SPI_PORT = 1;
constexpr hf_port_num_t UART_PORT = 0;

// Frequency and timing
constexpr hf_frequency_hz_t I2C_FREQUENCY = 400000;  // 400 kHz
constexpr hf_frequency_hz_t SPI_FREQUENCY = 1000000; // 1 MHz
constexpr hf_baud_rate_t UART_BAUDRATE = 115200;     // 115200 bps

// Timeout configuration
constexpr hf_timeout_ms_t SHORT_TIMEOUT = 100;
constexpr hf_timeout_ms_t LONG_TIMEOUT = 5000;

printf("I2C Port: %u, Frequency: %u Hz\n", I2C_PORT, I2C_FREQUENCY);
printf("SPI Port: %u, Frequency: %u Hz\n", SPI_PORT, SPI_FREQUENCY);
printf("UART Port: %u, Baud: %u bps\n", UART_PORT, UART_BAUDRATE);
```

### ⏰ **Timing Operations**

```cpp
#include "base/HardwareTypes.h"

// Timer configuration
constexpr hf_period_ms_t CONTROL_PERIOD = 10;    // 10 ms (100 Hz)
constexpr hf_period_ms_t SAFETY_PERIOD = 100;    // 100 ms (10 Hz)

// Frequency calculations
hf_frequency_hz_t control_freq = 1000 / CONTROL_PERIOD;  // 100 Hz
hf_frequency_hz_t safety_freq = 1000 / SAFETY_PERIOD;    // 10 Hz

printf("Control loop: %u ms (%u Hz)\n", CONTROL_PERIOD, control_freq);
printf("Safety check: %u ms (%u Hz)\n", SAFETY_PERIOD, safety_freq);

// Precision timing
hf_timestamp_us_t start_time = GetCurrentTimeMicros();
// ... perform operation ...
hf_timestamp_us_t end_time = GetCurrentTimeMicros();
hf_timestamp_us_t duration = end_time - start_time;

printf("Operation took %llu microseconds\n", duration);
```

### 🎯 **Type-Safe Function Parameters**

```cpp
#include "base/HardwareTypes.h"

/**
 * @brief Configure GPIO pin with type safety
 */
bool ConfigureGpio(hf_pin_num_t pin, bool output_mode) {
    if (!IsValidPin(pin)) {
        printf("Error: Invalid pin number %d\n", pin);
        return false;
    }
    
    printf("Configuring pin %d as %s\n", 
           pin, output_mode ? "output" : "input");
    return true;
}

/**
 * @brief Setup communication with proper types
 */
bool SetupCommunication(hf_port_num_t port, 
                       hf_frequency_hz_t frequency,
                       hf_timeout_ms_t timeout) {
    if (!IsValidPort(port)) {
        return false;
    }
    
    printf("Port %u: %u Hz, timeout %u ms\n", 
           port, frequency, timeout);
    return true;
}

// Usage with type safety
ConfigureGpio(2, true);  // Pin 2 as output
ConfigureGpio(HF_INVALID_PIN, false);  // Will fail validation

SetupCommunication(0, 400000, HF_TIMEOUT_DEFAULT_MS);
```

---

## 🎯 **Best Practices**

### ✅ **Recommended Practices**

1. **Always validate inputs** using provided utility functions
2. **Use appropriate invalid constants** for initialization
3. **Prefer explicit type conversion** when necessary
4. **Use descriptive names** for pin and channel constants
5. **Group related constants** in namespaces or classes

### ❌ **Things to Avoid**

1. **Don't use raw integer literals** for pin numbers
2. **Don't assume specific platform ranges** without validation
3. **Don't mix signed and unsigned types** without careful consideration
4. **Don't use magic numbers** for timeouts or frequencies

---

<div align="center">

**🔧 The HardwareTypes system provides a solid foundation for platform-agnostic hardware programming**

*Use these types consistently across all hardware interfaces for maximum portability and clarity*

</div>