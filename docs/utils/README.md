# 🛠️ HardFOC Interface Wrapper - Utilities

<div align="center">

![HardFOC
Utils](https://img.shields.io/badge/HardFOC-Utilities-green?style=for-the-badge&logo=tools)

**🔧 Utility Classes and Helper Components**

*Advanced utility classes that enhance the HardFOC Interface Wrapper ecosystem*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#-architecture)
- [📋 **Utility Classes**](#-utility-classes)
- [🔗 **Integration**](#-integration)
- [📊 **Getting Started**](#-getting-started)
- [🧪 **Examples**](#-examples)
- [🔗 **Navigation**](#-navigation)

---

## 🎯 **Overview**

The **HardFOC Interface Wrapper Utilities** provide advanced utility classes and helper components
that enhance the core hardware abstraction layer.
These utilities implement common design patterns, provide safety mechanisms,
and offer convenient abstractions for complex hardware operations.

### ✨ **Key Benefits**

- 🛡️ **Safety First** - RAII patterns and automatic resource management
- ⚡ **Performance Optimized** - Minimal overhead with maximum efficiency
- 🔧 **Design Patterns** - Common patterns like RAII, guards, and smart pointers
- 📊 **Error Handling** - Comprehensive error management and recovery
- 🧵 **Thread Safe** - Designed for multi-threaded embedded applications
- 🔄 **Platform Agnostic** - Works across all supported hardware platforms

### 🎯 **Target Use Cases**

- **Resource Management** - Automatic cleanup and state management
- **Safety Critical Systems** - Guaranteed resource cleanup and error handling
- **High-Performance Applications** - Optimized utility classes for real-time systems
- **Complex Hardware Control** - Advanced abstractions for sophisticated hardware operations
- **Multi-Threaded Systems** - Thread-safe utility components

---

## 🏗️ **Architecture**

### **Design Philosophy**

The HardFOC Utilities follow a **utility-first design pattern**:

```text
┌─────────────────────────────────────┐
│         Application Layer           │
├─────────────────────────────────────┤
│         Utility Classes             │ ← RAII, Guards, Helpers
├─────────────────────────────────────┤
│         Interface Wrapper           │
├─────────────────────────────────────┤
│        Platform Implementation      │
├─────────────────────────────────────┤
│           Hardware Layer            │
└─────────────────────────────────────┘
```

### **Core Components**

1. **Resource Management Utilities** - RAII patterns and automatic cleanup
2. **Safety Utilities** - Guards and protection mechanisms
3. **Helper Classes** - Convenience wrappers and abstractions
4. **Error Handling Utilities** - Advanced error management patterns

### **Integration Strategy**

- **Seamless Integration** - Works with all HardFOC Interface components
- **Zero Dependencies** - Self-contained utility classes
- **Consistent API** - Follows HardFOC Interface design patterns
- **Performance Focused** - Optimized for embedded systems

---

## 📋 **Utility Classes**

The HardFOC Interface Wrapper provides utility classes for common patterns and safety mechanisms:

| Class | Purpose | Key Features | Typical Use Cases |

|-------|---------|--------------|-------------------|

| **[DigitalOutputGuard](DigitalOutputGuard.md)** | RAII GPIO Management | Automatic state management, exception safety | Safe GPIO control, resource management |

| **[AsciiArtGenerator](AsciiArtGenerator.md)** | ASCII Art Generation | Predefined patterns, custom banners, decorative elements | Console output, logging enhancement, visual presentation |

### **Utility Categories**

#### **🛡️ Resource Management**
- **RAII Patterns** - Automatic resource acquisition and cleanup
- **Guard Classes** - Scope-based resource protection
- **Smart Pointers** - Automatic memory management
- **State Management** - Automatic state transitions and cleanup

#### **🔧 Safety Utilities**
- **Exception Safety** - Guaranteed cleanup in error scenarios
- **Thread Safety** - Multi-threaded access protection
- **Resource Protection** - Automatic resource state management
- **Error Recovery** - Graceful error handling and recovery

#### **⚡ Performance Utilities**
- **Zero-Copy Operations** - Efficient data handling
- **Move Semantics** - Efficient resource transfer
- **Optimized Algorithms** - High-performance utility functions
- **Memory Management** - Efficient memory usage patterns

---

## 🔗 **Integration**

### **With Core Interface**

The utility classes integrate seamlessly with the HardFOC Interface:

```cpp
// Utility classes work with any BaseGpio implementation
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"

// Create GPIO instance
EspGpio led_pin(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

// Use utility for safe GPIO management
{
    DigitalOutputGuard guard(led_pin);
    if (guard.IsValid()) {
        // GPIO is automatically active
        // ... perform operations ...
    }
    // GPIO automatically set inactive when guard goes out of scope
}
```

### **With Platform Implementations**

Utilities work with all platform implementations:

- **ESP32** - EspGpio, EspAdc, EspPwm, etc.
- **STM32** - Stm32Gpio, Stm32Adc, Stm32Pwm, etc.
- **Future Platforms** - Any BaseGpio implementation

### **With Application Code**

Utilities provide convenient abstractions for application development:

```cpp
// Application code using utilities
class MotorController {
private:
    EspGpio enable_pin*;
    EspPwm motor_pwm*;
    
public:
    void EnableMotor() {
        // Safe GPIO control with automatic cleanup
        DigitalOutputGuard guard(enable_pin*);
        if (guard.IsValid()) {
            // Motor is safely enabled
            motor_pwm*.SetDutyCycle(0, 50.0f);
        }
        // Motor automatically disabled when guard goes out of scope
    }
};
```

---

## 📊 **Getting Started**

### **1. Include Utility Headers**

```cpp
// Include utility classes
#include "inc/utils/DigitalOutputGuard.h"

// Include platform implementations
#include "inc/mcu/esp32/EspGpio.h"
```

### **2. Create Hardware Instances**

```cpp
// Create GPIO instance
EspGpio led_pin(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
led_pin.EnsureInitialized();
```

### **3. Use Utility Classes**

```cpp
// Use DigitalOutputGuard for safe GPIO management
{
    DigitalOutputGuard guard(led_pin);
    if (!guard.IsValid()) {
        // Handle initialization error
        return;
    }
    
    // GPIO is automatically active
    // ... perform operations ...
    
} // GPIO automatically set inactive when guard goes out of scope
```

### **4. Error Handling**

```cpp
DigitalOutputGuard guard(led_pin);
if (!guard.IsValid()) {
    hf_gpio_err_t error = guard.GetLastError();
    switch (error) {
        case hf_gpio_err_t::GPIO_ERR_NULL_POINTER:
            ESP_LOGE(TAG, "Null pointer provided");
            break;
        case hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED:
            ESP_LOGE(TAG, "GPIO not initialized");
            break;
        default:
            ESP_LOGE(TAG, "Unknown error: %d", static_cast<int>(error));
            break;
    }
    return;
}
```

---

## 🧪 **Examples**

### **Safe GPIO Control**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"

class StatusIndicator {
private:
    EspGpio status_led*;
    
public:
    StatusIndicator(hf_pin_num_t pin) 
        : status_led*(pin, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN) {
        status_led*.EnsureInitialized();
    }
    
    void ShowStatus(bool is_ok) {
        // Safe GPIO control with automatic cleanup
        DigitalOutputGuard guard(status_led*);
        if (!guard.IsValid()) {
            return;
        }
        
        if (is_ok) {
            guard.SetActive();  // LED on
        } else {
            guard.SetInactive(); // LED off
        }
        
        // LED automatically turned off when guard goes out of scope
    }
};
```

### **Motor Control with Safety**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"
#include "inc/mcu/esp32/EspPwm.h"

class SafeMotorController {
private:
    EspGpio enable_pin*;
    EspPwm motor_pwm*;
    
public:
    SafeMotorController(hf_pin_num_t enable_pin, hf_pin_num_t pwm_pin) 
        : enable_pin*(enable_pin, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT),
          motor_pwm*() {
        enable_pin*.EnsureInitialized();
        motor_pwm*.EnsureInitialized();
        motor_pwm*.EnableChannel(0);
    }
    
    void SetSpeed(float speed_percent) {
        // Safe motor control with automatic disable
        DigitalOutputGuard guard(enable_pin*);
        if (!guard.IsValid()) {
            return;
        }
        
        // Motor is safely enabled
        motor_pwm*.SetDutyCycle(0, speed_percent);
        
        // Motor automatically disabled when guard goes out of scope
    }
    
    void EmergencyStop() {
        // Immediate motor disable
        enable_pin*.SetInactive();
        motor_pwm*.SetDutyCycle(0, 0.0f);
    }
};
```

### **Multi-Threaded Safety**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Global GPIO for shared access
EspGpio shared_led*(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

void led_task(void* parameter) {
    while (true) {
        // Thread-safe GPIO control
        DigitalOutputGuard guard(shared_led*);
        if (guard.IsValid()) {
            guard.SetActive();
            vTaskDelay(pdMS_TO_TICKS(100));
            guard.SetInactive();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Create multiple tasks safely accessing the same GPIO
void setup_led_tasks() {
    xTaskCreate(led_task, "led_task_1", 2048, NULL, 1, NULL);
    xTaskCreate(led_task, "led_task_2", 2048, NULL, 1, NULL);
}
```

---

## 🔗 **Navigation**

### **Documentation Structure**

- **[🏠 Main Documentation](../README.md)** - Complete system overview
- **[📋 API Reference](../api/README.md)** - Core interface documentation
- **[🔧 ESP32 Implementations](../esp_api/README.md)** - Hardware-specific implementations
- **[🧪 Test Suites](../../examples/esp32/docs/README.md)** - Testing and validation

### **Utility Class Documentation**

| **Utility Class** | **Documentation** | **Status** |

|-------------------|-------------------|------------|

| **[DigitalOutputGuard](DigitalOutputGuard.md)** | RAII GPIO Management | ✅ Complete |

| **[AsciiArtGenerator](AsciiArtGenerator.md)** | ASCII Art Generation | ✅ Complete |

### **Related Resources**

- **[Contributing Guidelines](../../CONTRIBUTING.md)** - How to contribute
- **[Hardware Types](../api/HardwareTypes.md)** - Type definitions and validation
- **[Base Classes](../api/README.md)** - Core interface documentation

### **Quick Links**

- **[DigitalOutputGuard API](DigitalOutputGuard.md)** - Complete RAII GPIO management
- **[AsciiArtGenerator API](AsciiArtGenerator.md)** - ASCII art generation utilities
- **[DOG Test Documentation](../../examples/esp32/docs/README_DOG_TEST.md)** - Comprehensive test suite
- **[HardFOC Interface API](../api/README.md)** - Core hardware abstraction

---

## 🚀 **Future Utilities**

The HardFOC Utilities ecosystem is designed to grow with common patterns:

### **Planned Utilities**

- **AnalogInputGuard** - RAII analog input management
- **PwmOutputGuard** - Safe PWM output control
- **CommunicationGuard** - Safe communication protocol management
- **TimerGuard** - Automatic timer cleanup and management
- **MemoryGuard** - Safe memory allocation and cleanup

### **Contribution Guidelines**

To contribute new utilities:

1. **Follow RAII Patterns** - Automatic resource management
2. **Ensure Thread Safety** - Multi-threaded compatibility
3. **Provide Comprehensive Tests** - Full test coverage
4. **Document Performance** - Timing and memory characteristics
5. **Use Consistent APIs** - Follow HardFOC Interface patterns

---

<div align="center">

**🛠️ HardFOC Interface Wrapper - Utilities**

*Enhancing the HardFOC Ecosystem with Advanced Utility Classes*

</div>
