# 🔒 SfGpio Class (Thread-Safe GPIO)

[![Thread Safe Layer](https://img.shields.io/badge/Layer-ThreadSafe-success?style=flat-square)](../index.md#thread-safe-layer)
[![GPIO Interface](https://img.shields.io/badge/Interface-GPIO-green?style=flat-square)](BaseGpio.md)
[![Mutex Protected](https://img.shields.io/badge/Mutex-Protected-red?style=flat-square)](#thread-safety)

## 📋 Overview

The `SfGpio` class provides a thread-safe wrapper around any `BaseGpio` implementation, ensuring atomic operations and thread safety for multi-threaded applications. It uses the composition pattern with mutex protection to make any GPIO implementation safe for concurrent access.

### 🎯 Key Features

- 🔒 **Thread-Safe Operations**: All GPIO operations are mutex-protected
- 🔄 **Composition Pattern**: Wraps any BaseGpio implementation
- ⚡ **Zero-Copy Design**: Minimal overhead wrapper
- 🛡️ **Atomic Operations**: All GPIO state changes are atomic
- 🚀 **FreeRTOS Integration**: Native FreeRTOS mutex support
- 🎛️ **Full API Coverage**: Complete wrapping of BaseGpio interface
- 💼 **Resource Management**: Automatic mutex cleanup

## 🏗️ Class Hierarchy

```
SfGpio (Thread-Safe Wrapper)
    └── BaseGpio Implementation (McuDigitalGpio, etc.)
```

## 🚀 Quick Start

### Basic Thread-Safe GPIO

```cpp
#include "SfGpio.h"
#include "McuDigitalGpio.h"

// Create underlying GPIO implementation
auto gpio_impl = std::make_shared<McuDigitalGpio>(2);

// Wrap with thread-safe interface
HardFOC::ThreadSafe::SfGpio safe_gpio(gpio_impl);

// Now safe to use from multiple threads
safe_gpio.setPinMode(2, HF_GPIO_MODE_OUTPUT);
safe_gpio.setPinLevel(2, HF_GPIO_LEVEL_HIGH);
```

### Multi-Threaded LED Control

```cpp
class ThreadSafeLedController {
private:
    HardFOC::ThreadSafe::SfGpio led_gpio_;
    
public:
    ThreadSafeLedController(int pin) 
        : led_gpio_(std::make_shared<McuDigitalGpio>(pin)) {
        led_gpio_.setPinMode(pin, HF_GPIO_MODE_OUTPUT);
    }
    
    void TurnOn() {
        led_gpio_.setPinLevel(gpio_pin_, HF_GPIO_LEVEL_HIGH);
    }
    
    void TurnOff() {
        led_gpio_.setPinLevel(gpio_pin_, HF_GPIO_LEVEL_LOW);
    }
    
    void Toggle() {
        hf_gpio_level_t current_level;
        if (led_gpio_.getPinLevel(gpio_pin_, current_level) == HF_OK) {
            hf_gpio_level_t new_level = (current_level == HF_GPIO_LEVEL_HIGH) 
                ? HF_GPIO_LEVEL_LOW : HF_GPIO_LEVEL_HIGH;
            led_gpio_.setPinLevel(gpio_pin_, new_level);
        }
    }
};

// Safe to call from multiple threads
ThreadSafeLedController led(2);

// Thread 1
void blinking_task(void* param) {
    auto* controller = static_cast<ThreadSafeLedController*>(param);
    while (true) {
        controller->Toggle();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Thread 2
void control_task(void* param) {
    auto* controller = static_cast<ThreadSafeLedController*>(param);
    while (true) {
        controller->TurnOn();
        vTaskDelay(pdMS_TO_TICKS(100));
        controller->TurnOff();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

## 📚 Constructor

### SfGpio Constructor

```cpp
explicit SfGpio(std::shared_ptr<BaseGpio> gpio_impl);
```

#### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `gpio_impl` | `std::shared_ptr<BaseGpio>` | Shared pointer to GPIO implementation to wrap |

#### Notes
- Takes ownership through shared_ptr
- Automatic mutex creation and initialization
- Move semantics supported, copy disabled

## 🔧 Thread-Safe GPIO Operations

### Pin Configuration

#### setPinMode()
```cpp
hf_return_code_t setPinMode(hf_gpio_num_t pin, hf_gpio_mode_t mode);
```
- **Purpose**: Set pin mode (input/output) in a thread-safe manner
- **Parameters**: 
  - `pin` - GPIO pin number
  - `mode` - Pin mode (HF_GPIO_MODE_INPUT, HF_GPIO_MODE_OUTPUT, etc.)
- **Returns**: `HF_OK` on success, error code on failure
- **Thread Safety**: ✅ Mutex protected

#### setPinPull()
```cpp
hf_return_code_t setPinPull(hf_gpio_num_t pin, hf_gpio_pull_t pull);
```
- **Purpose**: Configure pull resistors thread-safely
- **Parameters**:
  - `pin` - GPIO pin number
  - `pull` - Pull configuration (HF_GPIO_PULL_NONE, HF_GPIO_PULL_UP, HF_GPIO_PULL_DOWN)
- **Thread Safety**: ✅ Mutex protected

#### setPinDriveCapability()
```cpp
hf_return_code_t setPinDriveCapability(hf_gpio_num_t pin, hf_gpio_drive_cap_t drive_cap);
```
- **Purpose**: Set pin drive capability thread-safely
- **Parameters**:
  - `pin` - GPIO pin number
  - `drive_cap` - Drive capability level
- **Thread Safety**: ✅ Mutex protected

### Digital I/O Operations

#### setPinLevel()
```cpp
hf_return_code_t setPinLevel(hf_gpio_num_t pin, hf_gpio_level_t level);
```
- **Purpose**: Set output pin level atomically
- **Parameters**:
  - `pin` - GPIO pin number
  - `level` - Output level (HF_GPIO_LEVEL_LOW, HF_GPIO_LEVEL_HIGH)
- **Thread Safety**: ✅ Mutex protected

#### getPinLevel()
```cpp
hf_return_code_t getPinLevel(hf_gpio_num_t pin, hf_gpio_level_t& level);
```
- **Purpose**: Read pin level atomically
- **Parameters**:
  - `pin` - GPIO pin number
  - `level` - Reference to store pin level
- **Thread Safety**: ✅ Mutex protected

### Interrupt Operations

#### enableInterrupt()
```cpp
hf_return_code_t enableInterrupt(hf_gpio_num_t pin);
```

#### disableInterrupt()
```cpp
hf_return_code_t disableInterrupt(hf_gpio_num_t pin);
```

#### setInterruptType()
```cpp
hf_return_code_t setInterruptType(hf_gpio_num_t pin, hf_gpio_int_type_t type);
```

All interrupt operations are mutex-protected for thread safety.

## 🔒 Thread Safety

### Mutex Protection

The `SfGpio` class ensures thread safety through:

- **FreeRTOS Mutex**: Each instance has its own mutex
- **Atomic Operations**: All public methods are mutex-protected
- **Lock Ordering**: Consistent lock ordering prevents deadlocks
- **Exception Safety**: RAII ensures proper mutex cleanup

### Thread Safety Guarantees

```cpp
// These operations are all thread-safe:
SfGpio gpio(std::make_shared<McuDigitalGpio>(2));

// Thread 1
gpio.setPinLevel(2, HF_GPIO_LEVEL_HIGH);

// Thread 2 (concurrent access is safe)
hf_gpio_level_t level;
gpio.getPinLevel(2, level);

// Thread 3 (also safe)
gpio.setPinMode(2, HF_GPIO_MODE_INPUT);
```

### Performance Considerations

| Operation | Overhead | Impact |
|-----------|----------|--------|
| Mutex Lock/Unlock | ~1-2µs | Minimal |
| GPIO Operation | Original cost | No change |
| Memory | +16 bytes | Minimal |

## 🎯 Usage Patterns

### Producer-Consumer Pattern

```cpp
class ThreadSafeButton {
private:
    HardFOC::ThreadSafe::SfGpio button_gpio_;
    QueueHandle_t event_queue_;
    
public:
    ThreadSafeButton(int pin) 
        : button_gpio_(std::make_shared<McuDigitalGpio>(pin)) {
        
        event_queue_ = xQueueCreate(10, sizeof(bool));
        
        // Configure as input with pull-up
        button_gpio_.setPinMode(pin, HF_GPIO_MODE_INPUT);
        button_gpio_.setPinPull(pin, HF_GPIO_PULL_UP);
        
        // Enable interrupt
        button_gpio_.setInterruptType(pin, HF_GPIO_INTR_NEGEDGE);
        button_gpio_.enableInterrupt(pin);
    }
    
    // Safe to call from ISR or any thread
    bool IsPressed() {
        hf_gpio_level_t level;
        if (button_gpio_.getPinLevel(button_pin_, level) == HF_OK) {
            return (level == HF_GPIO_LEVEL_LOW); // Active low
        }
        return false;
    }
};
```

### Resource Sharing

```cpp
class SharedGpioResource {
private:
    HardFOC::ThreadSafe::SfGpio shared_gpio_;
    
public:
    SharedGpioResource(int pin) 
        : shared_gpio_(std::make_shared<McuDigitalGpio>(pin)) {}
    
    // Multiple threads can safely access
    void SetHigh() { shared_gpio_.setPinLevel(pin_, HF_GPIO_LEVEL_HIGH); }
    void SetLow() { shared_gpio_.setPinLevel(pin_, HF_GPIO_LEVEL_LOW); }
    bool IsHigh() {
        hf_gpio_level_t level;
        return (shared_gpio_.getPinLevel(pin_, level) == HF_OK && 
                level == HF_GPIO_LEVEL_HIGH);
    }
};
```

## 🛡️ Error Handling

All methods return `hf_return_code_t` values:

| Return Code | Description |
|-------------|-------------|
| `HF_OK` | Operation successful |
| `HF_ERR_INVALID_ARG` | Invalid pin or parameter |
| `HF_ERR_NOT_INITIALIZED` | GPIO not initialized |
| `HF_ERR_TIMEOUT` | Mutex timeout |
| `HF_ERR_NO_MEM` | Memory allocation failed |

### Error Handling Example

```cpp
hf_return_code_t result = gpio.setPinLevel(2, HF_GPIO_LEVEL_HIGH);
switch (result) {
    case HF_OK:
        printf("Pin set successfully\n");
        break;
    case HF_ERR_INVALID_ARG:
        printf("Invalid pin number\n");
        break;
    case HF_ERR_NOT_INITIALIZED:
        printf("GPIO not initialized\n");
        break;
    case HF_ERR_TIMEOUT:
        printf("Mutex timeout - possible deadlock\n");
        break;
    default:
        printf("Unexpected error: %d\n", result);
        break;
}
```

## 💡 Best Practices

### 1. **Use Shared Pointers**
```cpp
// Correct: Use shared_ptr for automatic memory management
auto gpio_impl = std::make_shared<McuDigitalGpio>(2);
SfGpio safe_gpio(gpio_impl);
```

### 2. **Avoid Long Critical Sections**
```cpp
// Good: Keep mutex-protected sections short
void ProcessGpio() {
    hf_gpio_level_t level;
    gpio.getPinLevel(2, level);  // Short critical section
    
    // Process data outside critical section
    ProcessData(level);
    
    // Another short critical section
    gpio.setPinLevel(3, processed_level);
}
```

### 3. **Proper Resource Management**
```cpp
class GpioManager {
private:
    std::unique_ptr<SfGpio> gpio_;
    
public:
    GpioManager(int pin) {
        auto impl = std::make_shared<McuDigitalGpio>(pin);
        gpio_ = std::make_unique<SfGpio>(impl);
    }
    
    // Automatic cleanup in destructor
    ~GpioManager() = default;
};
```

### 4. **Avoid Nested Locks**
```cpp
// Avoid: Don't call thread-safe methods from within callbacks
// that might already hold locks
```

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - Base GPIO interface
- [`McuDigitalGpio`](McuDigitalGpio.md) - MCU GPIO implementation
- [`SfAdc`](SfAdc.md) - Thread-safe ADC wrapper
- [`SfCan`](SfCan.md) - Thread-safe CAN wrapper

## 🎚️ Configuration Options

The thread-safe wrapper supports all configuration options of the underlying GPIO implementation:

```cpp
// All these operations are thread-safe
gpio.setPinMode(2, HF_GPIO_MODE_OUTPUT);
gpio.setPinPull(2, HF_GPIO_PULL_UP);
gpio.setPinDriveCapability(2, HF_GPIO_DRIVE_CAP_3);
gpio.setInterruptType(2, HF_GPIO_INTR_POSEDGE);
```

---

*For more information about the HardFOC Internal Interface Wrapper, see the [main documentation](../index.md).*
