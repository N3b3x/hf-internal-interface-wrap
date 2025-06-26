# 🛡️ DigitalOutputGuard Class

[![Utils Layer](https://img.shields.io/badge/Layer-Utils-purple?style=flat-square)](../index.md#utils-layer)
[![RAII Pattern](https://img.shields.io/badge/Pattern-RAII-orange?style=flat-square)](#raii-pattern)
[![Exception Safe](https://img.shields.io/badge/Exception-Safe-success?style=flat-square)](#exception-safety)

## 📋 Overview

The `DigitalOutputGuard` class provides a RAII (Resource Acquisition Is Initialization) guard for managing the state of a `BaseGpio` instance as an output. It ensures automatic activation in the constructor and deactivation in the destructor, providing safe and reliable GPIO output state management.

### 🎯 Key Features

- 🔒 **RAII Pattern**: Automatic resource management with constructor/destructor
- ⚡ **Automatic Configuration**: Optional automatic output mode switching
- 🛡️ **Exception Safety**: Safe cleanup even during exceptions
- 🎛️ **Flexible Interface**: Support for both reference and pointer interfaces
- 📊 **Error Management**: Built-in error tracking and reporting
- 🚀 **Move Semantics**: Efficient move operations supported
- 🔧 **Configurable Behavior**: Optional output mode enforcement

## 🏗️ RAII Pattern

```
Constructor:    GPIO → Output Mode → Active State
Destructor:     GPIO → Inactive State (preserves mode)
```

## 🚀 Quick Start

### Basic Output Control

```cpp
#include "DigitalOutputGuard.h"
#include "McuDigitalGpio.h"

McuDigitalGpio led(2);
led.Initialize();

{
    // LED automatically turns ON when guard is created
    DigitalOutputGuard led_guard(led);
    
    if (led_guard.IsValid()) {
        // LED is now active (ON)
        vTaskDelay(pdMS_TO_TICKS(1000)); // Keep LED on for 1 second
    }
    // LED automatically turns OFF when guard goes out of scope
}
```

### Temporary Output Activation

```cpp
void SignalActivity() {
    static McuDigitalGpio status_led(13);
    static bool initialized = false;
    
    if (!initialized) {
        status_led.Initialize();
        initialized = true;
    }
    
    // LED flashes briefly to indicate activity
    DigitalOutputGuard flash(status_led);
    if (flash.IsValid()) {
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms flash
    }
    // LED automatically turns off when function exits
}
```

### Safe Relay Control

```cpp
class SafeRelayController {
private:
    McuDigitalGpio relay_pin_;
    
public:
    SafeRelayController(int pin) : relay_pin_(pin) {
        relay_pin_.Initialize();
    }
    
    void PerformSafeOperation() {
        // Relay automatically activated and deactivated
        DigitalOutputGuard relay_guard(relay_pin_);
        
        if (!relay_guard.IsValid()) {
            ESP_LOGE(TAG, "Failed to activate relay");
            return;
        }
        
        // Perform operation with relay active
        DoOperation();
        
        // Relay automatically deactivated when guard destructs
        // Even if DoOperation() throws an exception!
    }
};
```

## 📚 Constructors

### Reference Constructor

```cpp
explicit DigitalOutputGuard(
    BaseGpio& gpio,
    bool ensure_output_mode = true
) noexcept;
```

### Pointer Constructor

```cpp
explicit DigitalOutputGuard(
    BaseGpio* gpio,
    bool ensure_output_mode = true
) noexcept;
```

#### Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `gpio` | `BaseGpio&` or `BaseGpio*` | - | GPIO instance to manage |
| `ensure_output_mode` | `bool` | `true` | Automatically switch to output mode |

#### Behavior

| Phase | Action | Notes |
|-------|--------|-------|
| Constructor | 1. Set to output mode (if requested)<br>2. Set to active state | Validates GPIO and handles errors |
| Destructor | Set to inactive state | Preserves pin mode configuration |

## 🔧 Core Methods

### Validation

#### IsValid()
```cpp
[[nodiscard]] bool IsValid() const noexcept;
```
- **Purpose**: Check if guard was successfully initialized
- **Returns**: `true` if GPIO is properly managed, `false` otherwise
- **Thread Safety**: ✅ Safe

#### GetLastError()
```cpp
[[nodiscard]] HfGpioErr GetLastError() const noexcept;
```
- **Purpose**: Get the last error that occurred during operations
- **Returns**: `HfGpioErr` error code
- **Thread Safety**: ✅ Safe

## 🎯 Usage Patterns

### Scoped Resource Management

```cpp
void ControlLED() {
    McuDigitalGpio led(2);
    led.Initialize();
    
    if (condition) {
        DigitalOutputGuard led_on(led);
        if (led_on.IsValid()) {
            // LED is on for this block
            ProcessWithLED();
        }
        // LED automatically off when leaving block
    }
    
    // LED remains off after the if block
}
```

### Exception-Safe Operations

```cpp
void CriticalOperation(BaseGpio& safety_output) {
    // Safety output activated immediately
    DigitalOutputGuard safety_guard(safety_output);
    
    if (!safety_guard.IsValid()) {
        throw std::runtime_error("Failed to activate safety output");
    }
    
    try {
        // Perform critical operation
        PerformCriticalTask();
        
        // If this throws, safety output is still deactivated properly
        PerformAnotherTask();
        
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Critical operation failed: %s", e.what());
        // Safety output automatically deactivated by guard destructor
        throw; // Re-throw
    }
    
    // Safety output automatically deactivated on normal exit too
}
```

### Conditional Activation

```cpp
class SmartOutputController {
private:
    McuDigitalGpio output_pin_;
    
public:
    void ConditionalActivation(bool activate_output) {
        if (activate_output) {
            DigitalOutputGuard guard(output_pin_);
            
            if (guard.IsValid()) {
                // Output is active
                DoWorkWithOutput();
            } else {
                ESP_LOGE(TAG, "Output activation failed: %d", 
                         static_cast<int>(guard.GetLastError()));
            }
            // Output automatically deactivated
        } else {
            // Do work without activating output
            DoWorkWithoutOutput();
        }
    }
};
```

### Function-Scoped Control

```cpp
// RAII ensures proper cleanup even with early returns
bool ProcessWithHeartbeat(BaseGpio& heartbeat_led) {
    DigitalOutputGuard heartbeat(heartbeat_led);
    
    if (!heartbeat.IsValid()) {
        return false; // LED properly cleaned up
    }
    
    if (!CheckPreconditions()) {
        return false; // LED automatically deactivated
    }
    
    if (!PerformMainTask()) {
        return false; // LED automatically deactivated
    }
    
    return true; // LED automatically deactivated on success too
}
```

## 🛡️ Error Handling

The guard tracks and reports errors through its validation methods:

```cpp
DigitalOutputGuard guard(gpio);

if (!guard.IsValid()) {
    HfGpioErr error = guard.GetLastError();
    
    switch (error) {
        case HfGpioErr::GPIO_SUCCESS:
            // Should not happen if IsValid() returned false
            break;
            
        case HfGpioErr::GPIO_ERR_INVALID_PIN:
            ESP_LOGE(TAG, "Invalid GPIO pin");
            break;
            
        case HfGpioErr::GPIO_ERR_NOT_INITIALIZED:
            ESP_LOGE(TAG, "GPIO not initialized");
            break;
            
        case HfGpioErr::GPIO_ERR_INVALID_CONFIG:
            ESP_LOGE(TAG, "Invalid GPIO configuration");
            break;
            
        default:
            ESP_LOGE(TAG, "GPIO error: %d", static_cast<int>(error));
            break;
    }
}
```

## 🔄 Move Semantics

The class supports efficient move operations:

```cpp
class MoveExample {
private:
    std::unique_ptr<DigitalOutputGuard> guard_;
    
public:
    void CreateGuard(BaseGpio& gpio) {
        // Move construction
        guard_ = std::make_unique<DigitalOutputGuard>(std::move(
            DigitalOutputGuard(gpio)
        ));
    }
    
    void TransferGuard(std::unique_ptr<DigitalOutputGuard> other_guard) {
        // Move assignment
        guard_ = std::move(other_guard);
    }
};
```

## 💡 Best Practices

### 1. **Always Check Validity**
```cpp
DigitalOutputGuard guard(gpio);
if (!guard.IsValid()) {
    // Handle error appropriately
    ESP_LOGE(TAG, "Guard initialization failed");
    return;
}
```

### 2. **Use Appropriate Scope**
```cpp
void GoodScoping() {
    McuDigitalGpio led(2);
    led.Initialize();
    
    // Good: Guard lifetime matches LED usage
    {
        DigitalOutputGuard led_guard(led);
        if (led_guard.IsValid()) {
            DoWorkWithLED();
        }
    } // LED automatically turns off here
    
    // LED is now off for subsequent code
}
```

### 3. **Consider Output Mode Management**
```cpp
// For GPIOs already configured as output
DigitalOutputGuard guard(configured_output, false); // Don't change mode

// For GPIOs that might need mode switching
DigitalOutputGuard guard(unknown_gpio, true); // Ensure output mode
```

### 4. **Exception Safety**
```cpp
void ExceptionSafeFunction(BaseGpio& gpio) {
    DigitalOutputGuard guard(gpio);
    
    if (!guard.IsValid()) {
        throw std::runtime_error("GPIO activation failed");
    }
    
    // Any exception here will still properly deactivate GPIO
    RiskyOperation();
    AnotherRiskyOperation();
    
    // GPIO deactivated automatically on any exit path
}
```

## 🔒 Thread Safety

- ❌ **Not Thread-Safe**: The guard itself is not thread-safe
- ✅ **Underlying GPIO**: Thread safety depends on the wrapped GPIO implementation
- 🔧 **Best Practice**: Use one guard per thread, or external synchronization

```cpp
// Good: Each thread has its own guard
void thread_function(void* param) {
    auto* gpio = static_cast<BaseGpio*>(param);
    
    DigitalOutputGuard local_guard(*gpio);
    // Use local_guard safely in this thread
}

// Requires external synchronization for shared access
```

## ⚡ Performance Considerations

| Aspect | Impact | Notes |
|--------|--------|-------|
| **Construction** | ~2-5µs | Includes GPIO mode setting |
| **Destruction** | ~1-2µs | Single GPIO state change |
| **Memory** | ~8 bytes | Minimal overhead |
| **Exception Overhead** | None | RAII pattern has no exception overhead |

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - Base GPIO interface
- [`McuDigitalGpio`](McuDigitalGpio.md) - MCU GPIO implementation
- [`SfGpio`](SfGpio.md) - Thread-safe GPIO wrapper
- [`PeriodicTimer`](#) - Timing utilities

## 🎛️ Configuration Examples

### Emergency Stop System
```cpp
class EmergencyStop {
private:
    McuDigitalGpio stop_output_;
    
public:
    void TriggerEmergencyStop() {
        // Emergency output activated immediately
        DigitalOutputGuard emergency(stop_output_);
        
        if (emergency.IsValid()) {
            // Notify all systems
            NotifyAllSystems();
            
            // Wait for acknowledgment
            WaitForSystemsToStop();
            
            // Log emergency event
            LogEmergencyEvent();
        }
        
        // Emergency output automatically deactivated
        // when function exits (systems can resume)
    }
};
```

---

*For more information about the HardFOC Internal Interface Wrapper, see the [main documentation](../index.md).*
