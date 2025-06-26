# 🔌 McuDigitalGpio Class

[![MCU Layer](https://img.shields.io/badge/Layer-MCU-blue?style=flat-square)](../index.md#mcu-layer)
[![GPIO Interface](https://img.shields.io/badge/Interface-GPIO-green?style=flat-square)](BaseGpio.md)
[![Thread Safe](https://img.shields.io/badge/Thread-Safe-success?style=flat-square)](#thread-safety)

## 📋 Overview

The `McuDigitalGpio` class provides a concrete MCU-specific implementation of the unified `BaseGpio` interface. It enables direct control of microcontroller GPIO pins with advanced features including dynamic mode switching, interrupt handling, and comprehensive configuration options.

### 🎯 Key Features

- ✅ **Dynamic Mode Switching**: Runtime switching between input and output modes
- ✅ **Interrupt Support**: Full interrupt handling with callbacks and statistics
- ✅ **Active State Configuration**: Support for active-high and active-low logic
- ✅ **Pull Resistor Control**: Floating, pull-up, and pull-down configurations
- ✅ **Drive Mode Selection**: Push-pull and open-drain output modes
- ✅ **Thread-Safe Operations**: Built-in synchronization for multi-threaded environments
- ✅ **Platform Agnostic**: Unified interface across different MCU platforms

## 🏗️ Class Hierarchy

```
BaseGpio (Abstract Base)
    └── McuDigitalGpio (MCU Implementation)
```

## 🚀 Quick Start

### Basic Usage

```cpp
#include "McuDigitalGpio.h"

// Create GPIO for pin 2 as output, active-high
McuDigitalGpio led(2, McuDigitalGpio::Direction::Output, 
                   McuDigitalGpio::ActiveState::High);

// Initialize and use
if (led.Initialize()) {
    led.SetActive();      // Turn on LED
    led.SetInactive();    // Turn off LED
}
```

### Advanced Configuration

```cpp
// Create GPIO with full configuration
McuDigitalGpio button(4, 
    McuDigitalGpio::Direction::Input,     // Input mode
    McuDigitalGpio::ActiveState::Low,     // Active-low (pressed = LOW)
    McuDigitalGpio::OutputMode::PushPull, // Not used for input
    McuDigitalGpio::PullMode::PullUp      // Enable pull-up resistor
);

// Initialize and configure interrupt
if (button.Initialize()) {
    button.ConfigureInterrupt(
        McuDigitalGpio::InterruptTrigger::FallingEdge,
        [](void* user_data) {
            // Button pressed callback
            static_cast<MyClass*>(user_data)->OnButtonPressed();
        },
        &myInstance
    );
    button.EnableInterrupt();
}
```

## 📚 Constructor

### McuDigitalGpio Constructor

```cpp
explicit McuDigitalGpio(
    hf_gpio_num_t pin_num,
    Direction direction = Direction::Input,
    ActiveState active_state = ActiveState::High,
    OutputMode output_mode = OutputMode::PushPull,
    PullMode pull_mode = PullMode::Floating
) noexcept;
```

#### Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `pin_num` | `hf_gpio_num_t` | - | MCU GPIO pin number |
| `direction` | `Direction` | `Input` | Initial pin direction |
| `active_state` | `ActiveState` | `High` | Active state polarity |
| `output_mode` | `OutputMode` | `PushPull` | Output drive mode |
| `pull_mode` | `PullMode` | `Floating` | Pull resistor configuration |

## 🔧 Core Methods

### Initialization

#### Initialize()
```cpp
bool Initialize() noexcept override;
```
- **Purpose**: Configure the physical MCU pin with current settings
- **Returns**: `true` if successful, `false` otherwise
- **Thread Safety**: ✅ Safe

#### Deinitialize()
```cpp
bool Deinitialize() noexcept override;
```
- **Purpose**: Reset pin to safe default state
- **Returns**: `true` if successful, `false` otherwise
- **Thread Safety**: ✅ Safe

### Pin Information

#### IsPinAvailable()
```cpp
bool IsPinAvailable() const noexcept override;
```
- **Purpose**: Check if pin is available for GPIO operations
- **Returns**: `true` if available, `false` if reserved
- **Thread Safety**: ✅ Safe

#### GetMaxPins()
```cpp
uint8_t GetMaxPins() const noexcept override;
```
- **Purpose**: Get maximum number of pins supported by MCU
- **Returns**: Maximum pin count
- **Thread Safety**: ✅ Safe

## ⚡ Interrupt Handling

### Interrupt Support

#### SupportsInterrupts()
```cpp
bool SupportsInterrupts() const noexcept override;
```
- **Purpose**: Check if interrupts are supported
- **Returns**: Always `true` for MCU GPIOs
- **Thread Safety**: ✅ Safe

### Interrupt Configuration

#### ConfigureInterrupt()
```cpp
HfGpioErr ConfigureInterrupt(
    InterruptTrigger trigger,
    InterruptCallback callback = nullptr,
    void* user_data = nullptr
) noexcept override;
```

**Parameters:**
- `trigger`: Interrupt trigger type (Rising, Falling, Both, Level)
- `callback`: Optional callback function
- `user_data`: Optional user data for callback

**Returns:** `HfGpioErr` status code

### Interrupt Control

#### EnableInterrupt() / DisableInterrupt()
```cpp
HfGpioErr EnableInterrupt() noexcept override;
HfGpioErr DisableInterrupt() noexcept override;
```

#### WaitForInterrupt()
```cpp
HfGpioErr WaitForInterrupt(uint32_t timeout_ms = 0) noexcept override;
```
- **Parameters**: `timeout_ms` - Timeout in milliseconds (0 = no timeout)
- **Returns**: `GPIO_SUCCESS` if interrupt occurred, error code otherwise

### Interrupt Status

#### GetInterruptStatus()
```cpp
HfGpioErr GetInterruptStatus(InterruptStatus& status) noexcept override;
```

#### ClearInterruptStats()
```cpp
HfGpioErr ClearInterruptStats() noexcept override;
```

## 🔄 Dynamic Mode Switching

The `McuDigitalGpio` class supports runtime switching between input and output modes:

```cpp
McuDigitalGpio pin(5);
pin.Initialize();

// Start as input
pin.SetDirection(McuDigitalGpio::Direction::Input);
pin.SetPullMode(McuDigitalGpio::PullMode::PullUp);

// Read value
bool isHigh = pin.IsActive();

// Switch to output
pin.SetDirection(McuDigitalGpio::Direction::Output);
pin.SetOutputMode(McuDigitalGpio::OutputMode::PushPull);

// Drive output
pin.SetActive();
```

## 🛡️ Error Handling

All methods return appropriate error codes from the `HfGpioErr` enumeration:

| Error Code | Description |
|------------|-------------|
| `GPIO_SUCCESS` | Operation completed successfully |
| `GPIO_ERR_INVALID_PIN` | Invalid pin number |
| `GPIO_ERR_NOT_INITIALIZED` | Pin not initialized |
| `GPIO_ERR_INVALID_CONFIG` | Invalid configuration |
| `GPIO_ERR_HARDWARE_FAILURE` | Hardware operation failed |
| `GPIO_ERR_INTERRUPT_FAILED` | Interrupt operation failed |

## 🔒 Thread Safety

The `McuDigitalGpio` class is designed to be thread-safe:

- ✅ All public methods are thread-safe
- ✅ Internal state is protected by synchronization primitives
- ✅ Safe for concurrent access from multiple threads
- ✅ Interrupt callbacks are properly synchronized

## 💡 Best Practices

### 1. **Always Initialize**
```cpp
McuDigitalGpio gpio(2);
if (!gpio.Initialize()) {
    // Handle initialization failure
    return false;
}
```

### 2. **Check Pin Availability**
```cpp
if (!gpio.IsPinAvailable()) {
    // Pin is reserved for other functions
    return false;
}
```

### 3. **Proper Resource Management**
```cpp
class GpioManager {
private:
    McuDigitalGpio gpio_;
    
public:
    GpioManager(int pin) : gpio_(pin) {}
    
    ~GpioManager() {
        gpio_.DisableInterrupt();
        gpio_.Deinitialize();
    }
};
```

### 4. **Interrupt Safety**
```cpp
// Keep interrupt handlers lightweight
gpio.ConfigureInterrupt(
    McuDigitalGpio::InterruptTrigger::FallingEdge,
    [](void* data) {
        // Minimal processing in ISR
        auto* flag = static_cast<volatile bool*>(data);
        *flag = true;
    },
    &interrupt_flag
);
```

## 🎛️ Configuration Examples

### LED Control
```cpp
McuDigitalGpio led(2, McuDigitalGpio::Direction::Output);
led.Initialize();
led.SetActive();  // Turn on
```

### Button Input with Debouncing
```cpp
McuDigitalGpio button(4, 
    McuDigitalGpio::Direction::Input,
    McuDigitalGpio::ActiveState::Low,
    McuDigitalGpio::OutputMode::PushPull,
    McuDigitalGpio::PullMode::PullUp
);

button.Initialize();
// Add debouncing logic as needed
```

### Open-Drain Output
```cpp
McuDigitalGpio i2c_sda(21, 
    McuDigitalGpio::Direction::Output,
    McuDigitalGpio::ActiveState::High,
    McuDigitalGpio::OutputMode::OpenDrain
);
```

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - Base GPIO interface
- [`SfGpio`](SfGpio.md) - Thread-safe GPIO wrapper
- [`DigitalOutputGuard`](DigitalOutputGuard.md) - Output safety management

---

*For more information about the HardFOC Internal Interface Wrapper, see the [main documentation](../index.md).*
