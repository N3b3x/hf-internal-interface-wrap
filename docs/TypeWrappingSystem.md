# 🔧 Type Wrapping System

<div align="center">

![Type System](https://img.shields.io/badge/Type%20System-Wrapped%20Types-blue?style=for-the-badge&logo=typescript)

**🎯 Comprehensive type wrapping system for platform-agnostic hardware interfaces**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Type Definitions**](#️-type-definitions)
- [🔧 **Implementation Details**](#-implementation-details)
- [INFO: **Benefits**](#-benefits)
- [INFO: **Usage Examples**](#-usage-examples)
- [🔄 **Migration Guide**](#-migration-guide)

---

## 🎯 **Overview**

The HardFOC Internal Interface Wrapper implements a comprehensive type wrapping system that provides platform-agnostic type definitions across all hardware interfaces. This system ensures consistency, portability, and type safety throughout the entire codebase.

### ✨ **Key Principles**

- **🔒 Type Consistency**: All interfaces use the same wrapped type system
- **🔄 Portability**: Easy to adapt to different platforms and architectures
- **🎯 Clarity**: Clear distinction between platform types and wrapped types
- **🛡️ Safety**: Prevents type mismatches and improves compile-time checking

---

## 🏗️ **Type Definitions**

### INFO: **Integer Type Wrappers**

All standard integer types are wrapped with consistent naming:

```cpp
// Unsigned integer types
using hf_u8_t = uint8_t;    // 8-bit unsigned integer
using hf_u16_t = uint16_t;  // 16-bit unsigned integer
using hf_u32_t = uint32_t;  // 32-bit unsigned integer
using hf_u64_t = uint64_t;  // 64-bit unsigned integer

// Signed integer types
using hf_i8_t = int8_t;     // 8-bit signed integer
using hf_i16_t = int16_t;   // 16-bit signed integer
using hf_i32_t = int32_t;   // 32-bit signed integer
using hf_i64_t = int64_t;   // 64-bit signed integer
```

### 🔌 **Hardware-Specific Types**

Platform-agnostic types for hardware interfaces:

```cpp
// GPIO and pin types
using hf_pin_num_t = hf_i32_t;      // GPIO pin numbers (supports -1 for invalid)
using hf_channel_id_t = hf_u32_t;   // ADC/PWM channel identifiers
using hf_port_num_t = hf_u32_t;     // Communication port numbers
using hf_host_id_t = hf_u32_t;      // Host/controller identifiers

// Timing and frequency types
using hf_time_t = hf_u32_t;         // Time in milliseconds
using hf_timeout_ms_t = hf_time_t;  // Timeout values
using hf_frequency_hz_t = hf_u32_t; // Frequency in Hz
using hf_baud_rate_t = hf_u32_t;    // Baud rate for communication

// Timestamp types
using hf_timestamp_us_t = hf_u64_t; // Microsecond timestamps
```

### 🚫 **Invalid Value Constants**

Consistent invalid value definitions:

```cpp
// Invalid pin constant
constexpr hf_pin_num_t HF_INVALID_PIN = -1;

// Invalid port/host constants
constexpr hf_port_num_t HF_INVALID_PORT = std::numeric_limits<hf_port_num_t>::max();
constexpr hf_host_id_t HF_INVALID_HOST = std::numeric_limits<hf_host_id_t>::max();

// Invalid channel constant
constexpr hf_channel_id_t HF_INVALID_CHANNEL = std::numeric_limits<hf_channel_id_t>::max();

// Timeout constants
constexpr hf_time_t HF_TIMEOUT_DEFAULT_MS = 1000;
constexpr hf_time_t HF_TIMEOUT_NONE = 0;
constexpr hf_time_t HF_TIMEOUT_MAX = std::numeric_limits<hf_time_t>::max();
```

---

## 🔧 **Implementation Details**

### 📁 **File Structure**

The type wrapping system is implemented across multiple files:

```
inc/base/
├── HardwareTypes.h          # Core type definitions
├── BaseAdc.h               # ADC interface with wrapped types
├── BaseCan.h               # CAN interface with wrapped types
├── BaseGpio.h              # GPIO interface with wrapped types
├── BaseI2c.h               # I2C interface with wrapped types
├── BaseSpi.h               # SPI interface with wrapped types
├── BaseUart.h              # UART interface with wrapped types
├── BasePwm.h               # PWM interface with wrapped types
├── BasePio.h               # PIO interface with wrapped types
├── BaseNvs.h               # NVS interface with wrapped types
└── BasePeriodicTimer.h     # Timer interface with wrapped types
```

### 🔄 **Error Code Enums**

All error enums use wrapped types for consistency:

```cpp
// ADC error codes
enum class hf_adc_err_t : hf_u8_t {
    ADC_SUCCESS = 0,
    ADC_ERR_FAILURE = 1,
    // ... more error codes
};

// GPIO error codes
enum class hf_gpio_err_t : hf_u8_t {
    GPIO_SUCCESS = 0,
    GPIO_ERR_FAILURE = 1,
    // ... more error codes
};

// CAN error codes
enum class hf_can_err_t : hf_u8_t {
    CAN_SUCCESS = 0,
    CAN_ERR_FAILURE = 1,
    // ... more error codes
};

// Timer error codes
enum class hf_timer_err_t : hf_i32_t {
    TIMER_SUCCESS = 0,
    TIMER_ERR_FAILURE = 1,
    // ... more error codes
};
```

### INFO: **Data Structures**

All data structures use wrapped types:

```cpp
// ADC statistics
struct hf_adc_statistics_t {
    hf_u32_t totalConversions;        // Total conversions performed
    hf_u32_t successfulConversions;   // Successful conversions
    hf_u32_t failedConversions;       // Failed conversions
    hf_u32_t averageConversionTimeUs; // Average conversion time
    hf_u32_t maxConversionTimeUs;     // Maximum conversion time
    hf_u32_t minConversionTimeUs;     // Minimum conversion time
    hf_u32_t calibrationCount;        // Number of calibrations
    hf_u32_t thresholdViolations;     // Threshold monitor violations
    hf_u32_t calibration_errors;      // Calibration errors
};

// GPIO diagnostics
struct hf_gpio_diagnostics_t {
    bool gpioHealthy;                 // Overall GPIO health status
    hf_gpio_err_t lastErrorCode;      // Last error code
    hf_u32_t lastErrorTimestamp;      // Last error timestamp
    hf_u32_t consecutiveErrors;       // Consecutive error count
    float temperatureC;               // GPIO temperature (if available)
    hf_u32_t enabled_pins;            // Bit mask of enabled pins
};
```

---

## INFO: **Benefits**

### 🔒 **Type Safety**

- **Consistent Types**: All interfaces use the same type system
- **Compile-Time Checking**: Prevents type mismatches at compile time
- **Clear Interfaces**: Method signatures are unambiguous

### 🔄 **Portability**

- **Platform Agnostic**: Easy to adapt to different architectures
- **Future Proof**: Can change underlying types without breaking API
- **Consistent API**: Same interface across all platforms

### 🎯 **Clarity**

- **Self-Documenting**: Type names clearly indicate their purpose
- **Namespace Separation**: Clear distinction between platform and wrapped types
- **Consistent Naming**: All wrapped types follow the same naming convention

### 🛡️ **Maintainability**

- **Centralized Definitions**: All types defined in one place
- **Easy Updates**: Change types in one location affects entire system
- **Backward Compatibility**: Can maintain compatibility while evolving types

---

## INFO: **Usage Examples**

### 🔌 **GPIO Operations**

```cpp
#include "inc/base/BaseGpio.h"
#include "mcu/esp32/EspGpio.h"

// Create GPIO instance with wrapped types
EspGpio led_pin(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

void setup() {
    // Initialize with wrapped types
    if (led_pin.EnsureInitialized()) {
        printf("SUCCESS: GPIO initialized\n");
    }
}

void control_led() {
    // Use wrapped types for pin operations
    hf_pin_num_t pin = 2;
    hf_gpio_state_t state = hf_gpio_state_t::HF_GPIO_STATE_HIGH;
    
    led_pin.SetState(state);
}
```

### INFO: **ADC Reading**

```cpp
#include "inc/base/BaseAdc.h"
#include "mcu/esp32/EspAdc.h"

EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);

void read_sensors() {
    // Use wrapped types for channel operations
    hf_channel_id_t channel = 0;
    hf_u8_t samples = 4;  // 4-sample average
    hf_time_t delay_ms = 1;  // 1ms between samples
    
    float voltage;
    hf_adc_err_t result = adc.ReadChannelV(channel, voltage, samples, delay_ms);
    
    if (result == hf_adc_err_t::ADC_SUCCESS) {
        printf("Voltage: %.3f V\n", voltage);
    }
}
```

### 🔄 **Multi-Channel Operations**

```cpp
#include "inc/base/BaseAdc.h"

void read_multiple_channels() {
    // Use wrapped types for multi-channel operations
    hf_channel_id_t channels[] = {0, 1, 2};
    hf_u8_t num_channels = 3;
    hf_u32_t raw_readings[3];
    float voltages[3];
    
    hf_adc_err_t result = adc.ReadMultipleChannels(
        channels, num_channels, raw_readings, voltages
    );
    
    if (result == hf_adc_err_t::ADC_SUCCESS) {
        for (hf_u8_t i = 0; i < num_channels; i++) {
            printf("Channel %u: Raw=%u, Voltage=%.3f V\n", 
                   channels[i], raw_readings[i], voltages[i]);
        }
    }
}
```

### ⏱️ **Timer Operations**

```cpp
#include "inc/base/BasePeriodicTimer.h"

void configure_timer() {
    // Use wrapped types for timer operations
    hf_u64_t period_us = 1000000;  // 1 second period
    hf_u64_t min_period = timer.GetMinPeriod();
    hf_u64_t max_period = timer.GetMaxPeriod();
    
    if (period_us >= min_period && period_us <= max_period) {
        hf_timer_err_t result = timer.Start(period_us);
        if (result == hf_timer_err_t::TIMER_SUCCESS) {
            printf("SUCCESS: Timer started with %llu μs period\n", period_us);
        }
    }
}
```

---

## 🔄 **Migration Guide**

### 📋 **Before (Standard Types)**

```cpp
// Old code using standard types
uint8_t channel = 0;
uint32_t timeout = 1000;
uint64_t period = 1000000;

float voltage;
uint32_t raw_count;
adc.ReadChannelV(channel, voltage);
adc.ReadChannelCount(channel, raw_count);
timer.Start(period);
```

### SUCCESS: **After (Wrapped Types)**

```cpp
// New code using wrapped types
hf_channel_id_t channel = 0;
hf_time_t timeout = 1000;
hf_u64_t period = 1000000;

float voltage;
hf_u32_t raw_count;
adc.ReadChannelV(channel, voltage);
adc.ReadChannelCount(channel, raw_count);
timer.Start(period);
```

### 🔧 **Migration Steps**

1. **Update Variable Declarations**:
   ```cpp
   // Replace standard types with wrapped types
   uint8_t → hf_u8_t
   uint16_t → hf_u16_t
   uint32_t → hf_u32_t
   uint64_t → hf_u64_t
   int8_t → hf_i8_t
   int16_t → hf_i16_t
   int32_t → hf_i32_t
   int64_t → hf_i64_t
   ```

2. **Update Function Parameters**:
   ```cpp
   // Update method signatures to use wrapped types
   void ReadChannel(uint8_t channel, uint32_t& value);
   // becomes
   void ReadChannel(hf_channel_id_t channel, hf_u32_t& value);
   ```

3. **Update Return Types**:
   ```cpp
   // Update return types to use wrapped types
   uint32_t GetMaxChannels() const;
   // becomes
   hf_u8_t GetMaxChannels() const;
   ```

4. **Update Constants**:
   ```cpp
   // Use wrapped type constants
   const uint32_t INVALID_PIN = 0xFFFFFFFF;
   // becomes
   constexpr hf_pin_num_t HF_INVALID_PIN = -1;
   ```

### 🚨 **Common Pitfalls**

- **Mixed Types**: Don't mix wrapped and standard types in the same interface
- **Type Casting**: Be careful when casting between wrapped types
- **Constants**: Use the provided constants instead of magic numbers
- **Validation**: Use the provided validation functions

---

## 🔗 **Related Documentation**

- [INFO: **BaseAdc API**](api/BaseAdc.md) - ADC interface with wrapped types
- [🔌 **BaseGpio API**](api/BaseGpio.md) - GPIO interface with wrapped types
- [🚗 **BaseCan API**](api/BaseCan.md) - CAN interface with wrapped types
- [📡 **BaseI2c API**](api/BaseI2c.md) - I2C interface with wrapped types
- [🔄 **BaseSpi API**](api/BaseSpi.md) - SPI interface with wrapped types
- [📺 **BaseUart API**](api/BaseUart.md) - UART interface with wrapped types
- [⚡ **BasePwm API**](api/BasePwm.md) - PWM interface with wrapped types
- [🎛️ **BasePio API**](api/BasePio.md) - PIO interface with wrapped types
- [💾 **BaseNvs API**](api/BaseNvs.md) - NVS interface with wrapped types
- [⏱️ **BasePeriodicTimer API**](api/BasePeriodicTimer.md) - Timer interface with wrapped types

---

<div align="center">

**🔧 Type Wrapping System - Ensuring Consistency Across the HardFOC Platform**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 