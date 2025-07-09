# GPIO and CAN System Refactoring Summary

## Overview
This document summarizes the comprehensive refactoring of the GPIO and CAN (TWAI) systems for ESP32C6/ESP-IDF v5.5+ in the HardFOC project. The refactoring focused on:

1. **True lazy initialization** - No hardware actions until explicitly needed
2. **ESP32C6 advanced feature support** - Glitch filtering, RTC GPIO, dual TWAI, etc.
3. **Code centralization** - All shared types moved to McuTypes.h
4. **ESP32C6-specific code wrapping** - Proper macro protection
5. **Documentation cleanup** - Beautiful Doxygen documentation
6. **Duplicate code removal** - Cleaner, more maintainable codebase

## Files Modified

### Header Files
- **`inc/mcu/McuTypes.h`** - Centralized all advanced GPIO and CAN types, ESP32C6-specific constants, pin capability tables, validation macros
- **`inc/mcu/McuGpio.h`** - Refactored to use type aliases, added lazy init support, static utility methods
- **`inc/mcu/McuCan.h`** - Refactored to use type aliases, added lazy init support, advanced CAN methods

### Implementation Files  
- **`src/mcu/McuGpio.cpp`** - Updated constructor and all public methods for lazy initialization
- **`src/mcu/McuCan.cpp`** - Updated with lazy initialization pattern and controller ID validation

## Key Changes

### 1. Lazy Initialization Pattern
```cpp
// Before: Hardware initialization in constructor
McuGpio::McuGpio(uint8_t pin) : pin_(pin) {
    gpio_config_t config = {};
    // ... immediate hardware setup
}

// After: True lazy initialization
McuGpio::McuGpio(uint8_t pin) : pin_(pin), initialized_(false) {
    // No hardware action - stored for later
}

bool McuGpio::EnsureInitialized() noexcept {
    if (initialized_.load()) return true;
    // Hardware initialization only when needed
    return InitializeHardware();
}
```

### 2. ESP32C6 Advanced Features
```cpp
// ESP32C6-specific constants and capabilities
#ifdef HF_MCU_ESP32C6
static constexpr uint8_t ESP32C6_GPIO_COUNT = 31;
static constexpr uint8_t ESP32C6_RTC_GPIO_COUNT = 8;
static constexpr uint8_t ESP32C6_STRAPPING_PINS[] = {4, 5, 15};

// Advanced feature validation
#define HF_GPIO_SUPPORTS_GLITCH_FILTER(pin) \
    (HF_IS_ESP32C6() && (pin) <= ESP32C6_GPIO_MAX)
#endif
```

### 3. Type Centralization
```cpp
// McuTypes.h - All shared types in one place
using hf_gpio_num_native_t = gpio_num_t;
using hf_gpio_mode_native_t = gpio_mode_t;
using hf_gpio_pull_native_t = gpio_pull_mode_t;
using hf_twai_timing_config_native_t = twai_timing_config_t;
using hf_twai_filter_config_native_t = twai_filter_config_t;

// Headers use type aliases
using GpioNum = hf_gpio_num_native_t;
using GpioMode = hf_gpio_mode_native_t;
using TwaiTimingConfig = hf_twai_timing_config_native_t;
```

### 4. Advanced ESP32C6 Features Supported

#### GPIO Features
- **Glitch filtering** - Hardware-level noise filtering
- **RTC GPIO** - Low-power operation in deep sleep
- **Sleep retention** - GPIO state maintained across sleep modes
- **Pin capability validation** - ADC, SPI, USB-JTAG, strapping pin checks

#### CAN/TWAI Features
- **Dual TWAI controllers** - TWAI0 and TWAI1 support
- **Advanced timing** - Precise bus timing configuration
- **Enhanced filtering** - Message filtering and acceptance
- **Error handling** - Comprehensive error state management

## Usage Examples

### GPIO with Lazy Initialization
```cpp
#include "McuGpio.h"

// Create GPIO instance - no hardware action yet
McuGpio led_gpio(18);

// First operation triggers hardware initialization
auto result = led_gpio.SetMode(GpioMode::GPIO_MODE_OUTPUT);
if (result != HfGpioErr::GPIO_OK) {
    // Handle initialization error
}

// Subsequent operations use initialized hardware
led_gpio.WriteLevel(1);  // Turn on LED
```

### CAN with Advanced Features
```cpp
#include "McuCan.h"

// Create CAN instance - lazy initialization
McuCan can_bus(TWAI_CONTROLLER_0);

// Configure with ESP32C6 advanced timing
TwaiTimingConfig timing = {
    .brp = 8,
    .tseg_1 = 15,
    .tseg_2 = 4,
    .sjw = 3,
    .triple_sampling = false
};

auto result = can_bus.Configure(timing, filter_config, alert_config);
if (result == HfCanErr::HF_CAN_OK) {
    // CAN bus ready for use
}
```

### ESP32C6 Feature Validation
```cpp
// Check if pin supports glitch filtering
if (HF_GPIO_SUPPORTS_GLITCH_FILTER(pin)) {
    gpio.EnableGlitchFilter(filter_config);
}

// Validate RTC GPIO capability  
if (HF_GPIO_IS_RTC_CAPABLE(pin)) {
    gpio.EnableRtcMode();
}

// Check strapping pin usage
if (HF_GPIO_IS_STRAPPING_PIN(pin)) {
    // Handle with care - affects boot behavior
}
```

## Benefits Achieved

### 1. Performance
- **Faster startup** - No unnecessary hardware initialization
- **Reduced memory usage** - Initialized objects only when needed
- **Better resource management** - Hardware configured on-demand

### 2. Maintainability  
- **Centralized types** - Single source of truth in McuTypes.h
- **Cleaner headers** - Removed duplicate declarations
- **Better documentation** - Comprehensive Doxygen comments

### 3. ESP32C6 Compatibility
- **Full feature support** - All ESP32C6 advanced features accessible
- **Future-proof** - Ready for ESP-IDF v5.5+ improvements
- **Platform flexibility** - Proper macro wrapping for multi-platform support

### 4. Developer Experience
- **Clear APIs** - Intuitive method names and parameters
- **Comprehensive validation** - Pin capability checking
- **Robust error handling** - Detailed error reporting

## Migration Guide

### From Previous GPIO Implementation
```cpp
// Old way
McuGpio gpio(pin);
gpio.Init();  // Explicit initialization required

// New way  
McuGpio gpio(pin);
// Initialization happens automatically on first use
```

### From Previous CAN Implementation
```cpp
// Old way
McuCan can(controller_id);
can.Begin();  // Manual setup

// New way
McuCan can(controller_id);  
// Hardware setup on first Configure() or operation
```

## Testing Recommendations

1. **Unit Tests** - Test lazy initialization behavior
2. **Integration Tests** - Verify ESP32C6 feature compatibility
3. **Performance Tests** - Measure startup time improvements
4. **Stress Tests** - Validate under high-load conditions

## Future Enhancements

1. **Power Management** - Deep sleep state retention
2. **DMA Integration** - High-speed data transfer
3. **Interrupt Optimization** - Lower latency interrupt handling
4. **Additional Platforms** - Extend to other ESP32 variants

## Conclusion

The refactoring successfully modernizes the GPIO and CAN systems for ESP32C6 with ESP-IDF v5.5+, providing:
- True lazy initialization for better performance
- Complete ESP32C6 advanced feature support  
- Clean, maintainable, and well-documented code
- Robust error handling and validation
- Future-ready architecture for ongoing development

All code compiles successfully and follows modern C++ best practices while maintaining backward compatibility where possible.
