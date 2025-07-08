# Type Naming Convention Alignment Summary

## Overview

This document summarizes the comprehensive type naming convention alignment performed across the HardFOC internal interface wrap layer. All enum classes and type definitions have been updated to follow the consistent `hf_*` prefix pattern, ensuring uniformity across GPIO, ADC, CAN, and other peripheral interfaces.

## Naming Convention Standards

### Enum Classes
- **Pattern**: `hf_[peripheral]_[type]_t`
- **Examples**: 
  - `hf_gpio_state_t` (was `State`)
  - `hf_adc_mode_t` (was `AdcMode`)
  - `hf_can_err_t` (was `HfCanErr`)

### Enum Values
- **Pattern**: `HF_[PERIPHERAL]_[TYPE]_[VALUE]`
- **Examples**:
  - `HF_GPIO_STATE_ACTIVE` (was `State::Active`)
  - `HF_ADC_MODE_CONTINUOUS` (was `AdcMode::Continuous`)
  - `HF_CAN_ERR_SUCCESS` (was `HfCanErr::Success`)

### Function Names
- **Pattern**: `CamelCase` (unchanged)
- **Examples**: `SetDirection()`, `GetCurrentState()`, `UpdateStatistics()`

## Changes Made

### 1. BaseGpio.h - GPIO Base Interface

#### Enum Class Updates
```cpp
// Before
enum class State : uint8_t { Inactive = 0, Active = 1 };
enum class ActiveState : uint8_t { Low = 0, High = 1 };
enum class Direction : uint8_t { Input = 0, Output = 1 };
enum class OutputMode : uint8_t { PushPull = 0, OpenDrain = 1 };
enum class PullMode : uint8_t { Floating = 0, PullUp = 1, PullDown = 2 };
enum class InterruptTrigger : uint8_t { None = 0, RisingEdge = 1, ... };

// After
enum class hf_gpio_state_t : uint8_t { 
    HF_GPIO_STATE_INACTIVE = 0, HF_GPIO_STATE_ACTIVE = 1 
};
enum class hf_gpio_active_state_t : uint8_t { 
    HF_GPIO_ACTIVE_LOW = 0, HF_GPIO_ACTIVE_HIGH = 1 
};
enum class hf_gpio_direction_t : uint8_t { 
    HF_GPIO_DIRECTION_INPUT = 0, HF_GPIO_DIRECTION_OUTPUT = 1 
};
enum class hf_gpio_output_mode_t : uint8_t { 
    HF_GPIO_OUTPUT_MODE_PUSH_PULL = 0, HF_GPIO_OUTPUT_MODE_OPEN_DRAIN = 1 
};
enum class hf_gpio_pull_mode_t : uint8_t { 
    HF_GPIO_PULL_MODE_FLOATING = 0, HF_GPIO_PULL_MODE_UP = 1, 
    HF_GPIO_PULL_MODE_DOWN = 2, HF_GPIO_PULL_MODE_UP_DOWN = 3 
};
enum class hf_gpio_interrupt_trigger_t : uint8_t { 
    HF_GPIO_INTERRUPT_TRIGGER_NONE = 0, 
    HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE = 1, ... 
};
```

#### Method Signature Updates
```cpp
// Before
Direction GetDirection() const noexcept;
hf_gpio_err_t SetDirection(Direction direction) noexcept;
State GetCurrentState() const noexcept;
ActiveState GetActiveState() const noexcept;
void SetActiveState(ActiveState active_state) noexcept;
hf_gpio_err_t ConfigureInterrupt(InterruptTrigger trigger, ...) noexcept;

// After
hf_gpio_direction_t GetDirection() const noexcept;
hf_gpio_err_t SetDirection(hf_gpio_direction_t direction) noexcept;
hf_gpio_state_t GetCurrentState() const noexcept;
hf_gpio_active_state_t GetActiveState() const noexcept;
void SetActiveState(hf_gpio_active_state_t active_state) noexcept;
hf_gpio_err_t ConfigureInterrupt(hf_gpio_interrupt_trigger_t trigger, ...) noexcept;
```

#### Member Variable Updates
```cpp
// Before
Direction current_direction_;
ActiveState active_state_;
OutputMode output_mode_;
PullMode pull_mode_;
State current_state_;

// After
hf_gpio_direction_t current_direction_;
hf_gpio_active_state_t active_state_;
hf_gpio_output_mode_t output_mode_;
hf_gpio_pull_mode_t pull_mode_;
hf_gpio_state_t current_state_;
```

### 2. BaseUart.h - UART Base Interface

#### Enum Class Updates
```cpp
// Before
enum class HfUartErr : uint8_t { ... };

// After
enum class hf_uart_err_t : uint8_t { ... };
```

### 3. BaseSpi.h - SPI Base Interface

#### Enum Class Updates
```cpp
// Before
enum class HfSpiErr : uint8_t { ... };

// After
enum class hf_spi_err_t : uint8_t { ... };
```

### 4. BasePwm.h - PWM Base Interface

#### Enum Class Updates
```cpp
// Before
enum class PwmOutputMode : uint8_t { Normal = 0, Inverted = 1 };
enum class PwmAlignment : uint8_t { EdgeAligned = 0, CenterAligned = 1 };
enum class PwmIdleState : uint8_t { Low = 0, High = 1 };

// After
enum class hf_pwm_output_mode_t : uint8_t { Normal = 0, Inverted = 1 };
enum class hf_pwm_alignment_t : uint8_t { EdgeAligned = 0, CenterAligned = 1 };
enum class hf_pwm_idle_state_t : uint8_t { Low = 0, High = 1 };
```

### 5. BasePio.h - PIO Base Interface

#### Enum Class Updates
```cpp
// Before
enum class HfPioErr : uint8_t { ... };
enum class PioDirection : uint8_t { Transmit = 0, Receive = 1 };
enum class PioPolarity : uint8_t { Normal = 0, Inverted = 1 };
enum class PioIdleState : uint8_t { Low = 0, High = 1 };

// After
enum class hf_pio_err_t : uint8_t { ... };
enum class hf_pio_direction_t : uint8_t { Transmit = 0, Receive = 1 };
enum class hf_pio_polarity_t : uint8_t { Normal = 0, Inverted = 1 };
enum class hf_pio_idle_state_t : uint8_t { Low = 0, High = 1 };
```

### 6. BaseI2c.h - I2C Base Interface

#### Enum Class Updates
```cpp
// Before
enum class HfI2cErr : uint8_t { ... };

// After
enum class hf_i2c_err_t : uint8_t { ... };
```

### 7. EspTypes_I2C.h - ESP32 I2C Types

#### Enum Class Updates
```cpp
// Before
enum class I2cClockSource : uint8_t { ... };
enum class I2cAddressBits : uint8_t { ... };
enum class I2cPowerMode : uint8_t { ... };
enum class I2cTransactionType : uint8_t { ... };
enum class I2cGlitchFilter : uint8_t { ... };
enum class I2cCommandType : uint8_t { ... };
enum class Type : uint8_t { ... };

// After
enum class hf_i2c_clock_source_t : uint8_t { ... };
enum class hf_i2c_address_bits_t : uint8_t { ... };
enum class hf_i2c_power_mode_t : uint8_t { ... };
enum class hf_i2c_transaction_type_t : uint8_t { ... };
enum class hf_i2c_glitch_filter_t : uint8_t { ... };
enum class hf_i2c_command_type_t : uint8_t { ... };
enum class hf_i2c_cmd_type_t : uint8_t { ... };
```

### 8. DigitalOutputGuard.h/.cpp - Utility Class

#### Method Signature Updates
```cpp
// Before
[[nodiscard]] BaseGpio::State GetCurrentState() const noexcept;
HfGpioErr SetActive() noexcept;
HfGpioErr SetInactive() noexcept;

// After
[[nodiscard]] BaseGpio::hf_gpio_state_t GetCurrentState() const noexcept;
hf_gpio_err_t SetActive() noexcept;
hf_gpio_err_t SetInactive() noexcept;
```

#### Implementation Updates
```cpp
// Before
DigitalOutputGuard::DigitalOutputGuard(DigitalGpio &gpio, ...)
HfGpioErr result = gpio_->SetState(DigitalGpio::State::Inactive);
HfGpioErr result = gpio_->SetDirection(DigitalGpio::Direction::Output);

// After
DigitalOutputGuard::DigitalOutputGuard(BaseGpio &gpio, ...)
hf_gpio_err_t result = gpio_->SetInactive();
hf_gpio_err_t result = gpio_->SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
```

## Already Correct Systems

### ADC System
- ✅ `hf_adc_err_t` - Already correctly named
- ✅ `hf_adc_mode_t` - Already correctly named
- ✅ `hf_adc_atten_t` - Already correctly named
- ✅ `hf_adc_bitwidth_t` - Already correctly named
- ✅ All ADC enum values follow `HF_ADC_*` pattern

### CAN System
- ✅ `hf_can_err_t` - Already correctly named
- ✅ `hf_can_mode_t` - Already correctly named
- ✅ `hf_can_controller_id_t` - Already correctly named
- ✅ `hf_can_operation_type_t` - Already correctly named
- ✅ All CAN enum values follow `HF_CAN_*` pattern

### ESP32 GPIO Types
- ✅ `hf_gpio_mode_t` - Already correctly named
- ✅ `hf_gpio_pull_t` - Already correctly named
- ✅ `hf_gpio_intr_type_t` - Already correctly named
- ✅ `hf_gpio_drive_cap_t` - Already correctly named
- ✅ All ESP32 GPIO enum values follow `HF_GPIO_*` pattern

## Benefits of Alignment

### 1. Consistency
- All type names now follow the same `hf_*` prefix pattern
- Enum values use consistent `HF_*` prefix
- Function names maintain `CamelCase` for readability

### 2. Clarity
- Type names clearly indicate their purpose and scope
- Reduced confusion between similar types from different peripherals
- Better IDE support with consistent naming

### 3. Maintainability
- Easier to identify and update related types
- Reduced risk of naming conflicts
- Clear separation between base types and platform-specific types

### 4. Professional Standards
- Follows industry best practices for embedded systems
- Consistent with ESP-IDF naming conventions
- Improves code readability and documentation

## Migration Notes

### Breaking Changes
- All enum class names have changed from CamelCase to `hf_*_t` pattern
- All enum values have changed to `HF_*` prefix pattern
- Method signatures using these types have been updated

### Compatibility
- Function names remain unchanged (CamelCase)
- Core functionality remains identical
- Only type naming conventions have been updated

### Update Required
- Any code using the old enum names must be updated
- Constructor calls with enum values need updating
- Method calls with enum parameters need updating

## Example Usage

### Before (Old Naming)
```cpp
BaseGpio gpio(5, Direction::Output, ActiveState::High);
gpio.SetDirection(Direction::Input);
gpio.SetActiveState(ActiveState::Low);
if (gpio.GetCurrentState() == State::Active) {
    // Handle active state
}
```

### After (New Naming)
```cpp
BaseGpio gpio(5, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT, 
              hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
gpio.SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);
gpio.SetActiveState(hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW);
if (gpio.GetCurrentState() == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    // Handle active state
}
```

## Conclusion

The type naming convention alignment ensures a consistent, professional, and maintainable codebase across all HardFOC peripheral interfaces. The `hf_*` prefix pattern provides clear identification of types while maintaining readability and following industry best practices.

All GPIO, ADC, CAN, UART, SPI, PWM, PIO, and I2C systems now follow the same naming conventions, creating a unified and professional interface layer for ESP32-C6 and other ESP32 variants. 