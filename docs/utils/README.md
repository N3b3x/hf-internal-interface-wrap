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
```text

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
EspGpio led*pin(2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);

// Use utility for safe GPIO management
{
    DigitalOutputGuard guard(led*pin);
    if (guard.IsValid()) {
        // GPIO is automatically active
        // ... perform operations ...
    }
    // GPIO automatically set inactive when guard goes out of scope
}
```text

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
    EspGpio enable*pin*;
    EspPwm motor*pwm*;
    
public:
    void EnableMotor() {
        // Safe GPIO control with automatic cleanup
        DigitalOutputGuard guard(enable*pin*);
        if (guard.IsValid()) {
            // Motor is safely enabled
            motor*pwm*.SetDutyCycle(0, 50.0f);
        }
        // Motor automatically disabled when guard goes out of scope
    }
};
```text

---

## 📊 **Getting Started**

### **1. Include Utility Headers**

```cpp
// Include utility classes
#include "inc/utils/DigitalOutputGuard.h"

// Include platform implementations
#include "inc/mcu/esp32/EspGpio.h"
```text

### **2. Create Hardware Instances**

```cpp
// Create GPIO instance
EspGpio led*pin(2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);
led*pin.EnsureInitialized();
```text

### **3. Use Utility Classes**

```cpp
// Use DigitalOutputGuard for safe GPIO management
{
    DigitalOutputGuard guard(led*pin);
    if (!guard.IsValid()) {
        // Handle initialization error
        return;
    }
    
    // GPIO is automatically active
    // ... perform operations ...
    
} // GPIO automatically set inactive when guard goes out of scope
```text

### **4. Error Handling**

```cpp
DigitalOutputGuard guard(led*pin);
if (!guard.IsValid()) {
    hf*gpio*err*t error = guard.GetLastError();
    switch (error) {
        case hf*gpio*err*t::GPIO*ERR*NULL*POINTER:
            ESP*LOGE(TAG, "Null pointer provided");
            break;
        case hf*gpio*err*t::GPIO*ERR*NOT*INITIALIZED:
            ESP*LOGE(TAG, "GPIO not initialized");
            break;
        default:
            ESP*LOGE(TAG, "Unknown error: %d", static*cast<int>(error));
            break;
    }
    return;
}
```text

---

## 🧪 **Examples**

### **Safe GPIO Control**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"

class StatusIndicator {
private:
    EspGpio status*led*;
    
public:
    StatusIndicator(hf*pin*num*t pin) 
        : status*led*(pin, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT,
                      hf*gpio*active*state*t::HF*GPIO*ACTIVE*HIGH,
                      hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                      hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*DOWN) {
        status*led*.EnsureInitialized();
    }
    
    void ShowStatus(bool is*ok) {
        // Safe GPIO control with automatic cleanup
        DigitalOutputGuard guard(status*led*);
        if (!guard.IsValid()) {
            return;
        }
        
        if (is*ok) {
            guard.SetActive();  // LED on
        } else {
            guard.SetInactive(); // LED off
        }
        
        // LED automatically turned off when guard goes out of scope
    }
};
```text

### **Motor Control with Safety**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"
#include "inc/mcu/esp32/EspPwm.h"

class SafeMotorController {
private:
    EspGpio enable*pin*;
    EspPwm motor*pwm*;
    
public:
    SafeMotorController(hf*pin*num*t enable*pin, hf*pin*num*t pwm*pin) 
        : enable*pin*(enable*pin, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT),
          motor*pwm*() {
        enable*pin*.EnsureInitialized();
        motor*pwm*.EnsureInitialized();
        motor*pwm*.EnableChannel(0);
    }
    
    void SetSpeed(float speed*percent) {
        // Safe motor control with automatic disable
        DigitalOutputGuard guard(enable*pin*);
        if (!guard.IsValid()) {
            return;
        }
        
        // Motor is safely enabled
        motor*pwm*.SetDutyCycle(0, speed*percent);
        
        // Motor automatically disabled when guard goes out of scope
    }
    
    void EmergencyStop() {
        // Immediate motor disable
        enable*pin*.SetInactive();
        motor*pwm*.SetDutyCycle(0, 0.0f);
    }
};
```text

### **Multi-Threaded Safety**

```cpp
#include "inc/utils/DigitalOutputGuard.h"
#include "inc/mcu/esp32/EspGpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Global GPIO for shared access
EspGpio shared*led*(2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);

void led*task(void* parameter) {
    while (true) {
        // Thread-safe GPIO control
        DigitalOutputGuard guard(shared*led*);
        if (guard.IsValid()) {
            guard.SetActive();
            vTaskDelay(pdMS*TO*TICKS(100));
            guard.SetInactive();
            vTaskDelay(pdMS*TO*TICKS(100));
        }
    }
}

// Create multiple tasks safely accessing the same GPIO
void setup*led*tasks() {
    xTaskCreate(led*task, "led*task*1", 2048, NULL, 1, NULL);
    xTaskCreate(led*task, "led*task*2", 2048, NULL, 1, NULL);
}
```text

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
