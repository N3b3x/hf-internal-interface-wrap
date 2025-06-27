# 🚀 HardFOC Internal Interface Wrapper - Developer Guide

Welcome to the comprehensive developer guide for the HardFOC Internal Interface Wrapper! This guide will help you understand the architecture, use the APIs effectively, and implement robust embedded applications.

## 📋 Table of Contents

1. [Getting Started](#-getting-started)
2. [Architecture Overview](#-architecture-overview)
3. [Layer-by-Layer Guide](#-layer-by-layer-guide)
4. [Common Patterns](#-common-patterns)
5. [Best Practices](#-best-practices)
6. [Performance Considerations](#-performance-considerations)
7. [Error Handling](#-error-handling)
8. [Testing Strategies](#-testing-strategies)

## 🏁 Getting Started

### Quick Setup

```cpp
#include "hf-internal-interface-wrap/All.h"

// Initialize the system
void setup() {
    // 1. Initialize base layer components
    McuDigitalGpio led(2);
    led.Initialize();
    
    // 2. Wrap with thread-safe layer if needed
    auto safe_led = std::make_shared<SfGpio>(
        std::make_shared<McuDigitalGpio>(2)
    );
    
    // 3. Use utility classes for RAII
    DigitalOutputGuard led_guard(led);
}
```

### Project Structure

```
your_project/
├── main/
│   ├── main.cpp
│   └── CMakeLists.txt
├── components/
│   └── hf-internal-interface-wrap/  # This library
└── CMakeLists.txt
```

### CMakeLists.txt Integration

```cmake
# In your main CMakeLists.txt
set(EXTRA_COMPONENT_DIRS "path/to/hf-internal-interface-wrap")

# In your component CMakeLists.txt
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES hf-internal-interface-wrap
)
```

## 🏗️ Architecture Overview

The HardFOC Internal Interface Wrapper follows a layered architecture:

```
┌─────────────────────────────────────┐
│         Application Layer           │
├─────────────────────────────────────┤
│         Utils Layer                 │
│    (Guards, Timers, Storage)       │
├─────────────────────────────────────┤
│       Thread-Safe Layer            │
│    (Sf* classes with mutexes)      │
├─────────────────────────────────────┤
│         MCU Layer                   │
│  (Platform-specific implementations)│
├─────────────────────────────────────┤
│         Base Layer                  │
│   (Abstract interfaces & types)    │
└─────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Purpose | Examples |
|-------|---------|----------|
| **Base** | Abstract interfaces | `BaseGpio`, `BaseAdc`, `BaseCan` |
| **MCU** | Platform implementations | `McuDigitalGpio`, `McuAdc`, `McuCan` |
| **Thread-Safe** | Mutex-protected wrappers | `SfGpio`, `SfAdc`, `SfCan` |
| **Utils** | Helper classes | `DigitalOutputGuard`, `PeriodicTimer` |

## 🔄 Layer-by-Layer Guide

### Base Layer: Abstract Interfaces

The base layer defines the contracts for all peripheral interfaces:

```cpp
// Example: GPIO abstraction
class BaseGpio {
public:
    virtual bool Initialize() noexcept = 0;
    virtual HfGpioErr SetDirection(Direction direction) noexcept = 0;
    virtual HfGpioErr SetActive() noexcept = 0;
    virtual HfGpioErr SetInactive() noexcept = 0;
    virtual bool IsActive() const noexcept = 0;
    // ... more methods
};
```

**When to use directly:** Never! Always use concrete implementations.

### MCU Layer: Platform Implementations

The MCU layer provides concrete implementations for specific microcontrollers:

```cpp
// Example: ESP32-C6 GPIO implementation
McuDigitalGpio status_led(2, 
    McuDigitalGpio::Direction::Output,
    McuDigitalGpio::ActiveState::High
);

if (status_led.Initialize()) {
    status_led.SetActive();   // Turn on LED
    status_led.SetInactive(); // Turn off LED
}
```

**When to use:** Single-threaded applications, direct hardware control.

### Thread-Safe Layer: Multi-Threading Support

The thread-safe layer adds mutex protection for concurrent access:

```cpp
// Example: Thread-safe GPIO
auto gpio_impl = std::make_shared<McuDigitalGpio>(2);
SfGpio safe_gpio(gpio_impl);

// Safe to call from multiple threads
safe_gpio.digitalWrite(2, true);
```

**When to use:** Multi-threaded applications, FreeRTOS tasks.

### Utils Layer: Helper Classes

The utils layer provides convenient helper classes:

```cpp
// Example: RAII GPIO management
void signal_activity() {
    static McuDigitalGpio activity_led(13);
    static bool initialized = false;
    
    if (!initialized) {
        activity_led.Initialize();
        initialized = true;
    }
    
    // LED automatically turns on/off
    DigitalOutputGuard guard(activity_led);
    
    if (guard.IsValid()) {
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms pulse
    }
    // LED automatically turns off when guard destructs
}
```

**When to use:** Resource management, timing, storage operations.

## 🎯 Common Patterns

### 1. Factory Pattern for Platform Independence

```cpp
class GpioFactory {
public:
    static std::unique_ptr<BaseGpio> CreateGpio(int pin) {
        #ifdef ESP32_C6
            return std::make_unique<McuDigitalGpio>(pin);
        #elif defined(STM32F4)
            return std::make_unique<Stm32Gpio>(pin);
        #else
            #error "Unsupported platform"
        #endif
    }
};

// Usage
auto gpio = GpioFactory::CreateGpio(2);
gpio->Initialize();
```

### 2. Composition with Thread Safety

```cpp
class ThreadSafeLedController {
private:
    std::unique_ptr<SfGpio> led_gpio_;
    int pin_;
    
public:
    ThreadSafeLedController(int pin) : pin_(pin) {
        auto gpio_impl = std::make_shared<McuDigitalGpio>(pin);
        led_gpio_ = std::make_unique<SfGpio>(gpio_impl);
        led_gpio_->setPinMode(pin, HF_GPIO_MODE_OUTPUT);
    }
    
    void TurnOn() {
        led_gpio_->digitalWrite(pin_, true);
    }

    void TurnOff() {
        led_gpio_->digitalWrite(pin_, false);
    }
    
    void Blink(int duration_ms) {
        TurnOn();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        TurnOff();
    }
};
```

### 3. RAII Resource Management

```cpp
class SafeOperationController {
private:
    McuDigitalGpio safety_output_;
    McuDigitalGpio status_led_;
    
public:
    SafeOperationController(int safety_pin, int status_pin) 
        : safety_output_(safety_pin), status_led_(status_pin) {
        safety_output_.Initialize();
        status_led_.Initialize();
    }
    
    bool PerformOperation() {
        // Status LED shows operation in progress
        DigitalOutputGuard status_guard(status_led_);
        
        if (!status_guard.IsValid()) {
            return false;
        }
        
        // Safety output ensures safe operation
        DigitalOutputGuard safety_guard(safety_output_);
        
        if (!safety_guard.IsValid()) {
            return false;
        }
        
        try {
            // Perform the actual operation
            DoActualWork();
            return true;
            
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Operation failed: %s", e.what());
            return false;
        }
        
        // Both guards automatically clean up
    }
};
```

### 4. Observer Pattern for Interrupts

```cpp
class ButtonObserver {
public:
    virtual ~ButtonObserver() = default;
    virtual void OnButtonPressed() = 0;
    virtual void OnButtonReleased() = 0;
};

class SmartButton {
private:
    McuDigitalGpio button_gpio_;
    std::vector<ButtonObserver*> observers_;
    
    static void button_isr_handler(void* arg) {
        auto* button = static_cast<SmartButton*>(arg);
        button->HandleInterrupt();
    }
    
    void HandleInterrupt() {
        if (button_gpio_.IsActive()) {
            for (auto* observer : observers_) {
                observer->OnButtonPressed();
            }
        } else {
            for (auto* observer : observers_) {
                observer->OnButtonReleased();
            }
        }
    }
    
public:
    SmartButton(int pin) : button_gpio_(pin, 
        McuDigitalGpio::Direction::Input,
        McuDigitalGpio::ActiveState::Low,
        McuDigitalGpio::OutputMode::PushPull,
        McuDigitalGpio::PullMode::PullUp) {
        
        button_gpio_.Initialize();
        button_gpio_.ConfigureInterrupt(
            McuDigitalGpio::InterruptTrigger::Both,
            button_isr_handler,
            this
        );
        button_gpio_.EnableInterrupt();
    }
    
    void AddObserver(ButtonObserver* observer) {
        observers_.push_back(observer);
    }
    
    void RemoveObserver(ButtonObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }
};
```

## 💡 Best Practices

### 1. Initialization Patterns

```cpp
// Good: Check initialization success
bool InitializeHardware() {
    static bool initialized = false;
    if (initialized) return true;
    
    // Initialize all hardware components
    if (!led_gpio_.Initialize()) {
        ESP_LOGE(TAG, "LED GPIO initialization failed");
        return false;
    }
    
    if (!button_gpio_.Initialize()) {
        ESP_LOGE(TAG, "Button GPIO initialization failed");
        return false;
    }
    
    if (!adc_.Initialize()) {
        ESP_LOGE(TAG, "ADC initialization failed");
        return false;
    }
    
    initialized = true;
    return true;
}
```

### 2. Error Handling

```cpp
// Good: Comprehensive error handling
HfGpioErr ConfigureGpio(BaseGpio& gpio) {
    HfGpioErr result = gpio.SetDirection(BaseGpio::Direction::Output);
    if (result != HfGpioErr::GPIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set GPIO direction: %d", static_cast<int>(result));
        return result;
    }
    
    result = gpio.SetOutputMode(BaseGpio::OutputMode::PushPull);
    if (result != HfGpioErr::GPIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set GPIO output mode: %d", static_cast<int>(result));
        return result;
    }
    
    return HfGpioErr::GPIO_SUCCESS;
}
```

### 3. Resource Management

```cpp
// Good: RAII for automatic cleanup
class HardwareManager {
private:
    std::vector<std::unique_ptr<BaseGpio>> gpios_;
    std::vector<std::unique_ptr<BaseAdc>> adcs_;
    
public:
    ~HardwareManager() {
        // Automatic cleanup of all resources
        for (auto& gpio : gpios_) {
            if (gpio) {
                gpio->Deinitialize();
            }
        }
        
        for (auto& adc : adcs_) {
            if (adc) {
                adc->Deinitialize();
            }
        }
    }
    
    template<typename T, typename... Args>
    T* AddGpio(Args&&... args) {
        auto gpio = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = gpio.get();
        gpios_.push_back(std::move(gpio));
        return ptr;
    }
};
```

### 4. Thread Safety

```cpp
// Good: Explicit thread safety where needed
class ThreadSafeHardwareManager {
private:
    mutable std::mutex hardware_mutex_;
    std::map<int, std::shared_ptr<SfGpio>> gpio_map_;
    
public:
    std::shared_ptr<SfGpio> GetGpio(int pin) {
        std::lock_guard<std::mutex> lock(hardware_mutex_);
        
        auto it = gpio_map_.find(pin);
        if (it != gpio_map_.end()) {
            return it->second;
        }
        
        // Create new thread-safe GPIO
        auto gpio_impl = std::make_shared<McuDigitalGpio>(pin);
        auto safe_gpio = std::make_shared<SfGpio>(gpio_impl);
        gpio_map_[pin] = safe_gpio;
        
        return safe_gpio;
    }
};
```

## ⚡ Performance Considerations

### 1. Initialization Cost

| Component | Init Time | Notes |
|-----------|-----------|-------|
| GPIO | ~50µs | Pin configuration |
| ADC | ~200µs | Calibration included |
| CAN | ~500µs | Driver setup |
| I2C | ~100µs | Bus configuration |

### 2. Runtime Performance

```cpp
// Fast: Direct MCU layer access
McuDigitalGpio fast_gpio(2);
fast_gpio.SetActive();     // ~1µs

// Slower: Thread-safe wrapper
SfGpio safe_gpio(gpio_impl);
safe_gpio.digitalWrite(2, true); // ~3µs (includes mutex)
```

### 3. Memory Usage

| Class | Size | Notes |
|-------|------|-------|
| `McuDigitalGpio` | ~32 bytes | Base implementation |
| `SfGpio` | ~48 bytes | Adds mutex overhead |
| `DigitalOutputGuard` | ~8 bytes | Minimal RAII overhead |

### 4. Optimization Tips

```cpp
// Good: Pre-allocate and reuse
class OptimizedController {
private:
    static constexpr size_t MAX_GPIOS = 10;
    std::array<std::unique_ptr<McuDigitalGpio>, MAX_GPIOS> gpios_;
    
public:
    // Pre-allocate all GPIOs during initialization
    void Initialize() {
        for (size_t i = 0; i < MAX_GPIOS; ++i) {
            gpios_[i] = std::make_unique<McuDigitalGpio>(i);
            gpios_[i]->Initialize();
        }
    }
    
    // Fast access during runtime
    void SetGpio(size_t index, bool active) {
        if (index < MAX_GPIOS && gpios_[index]) {
            if (active) {
                gpios_[index]->SetActive();
            } else {
                gpios_[index]->SetInactive();
            }
        }
    }
};
```

## 🛡️ Error Handling

### Error Code Hierarchy

```cpp
// Consistent error handling across all interfaces
template<typename ErrorType>
bool HandleError(ErrorType error, const char* operation) {
    if (error == ErrorType::SUCCESS || error == ErrorType::OK) {
        return true;
    }
    
    ESP_LOGE(TAG, "%s failed with error: %d", operation, static_cast<int>(error));
    
    // Log specific error details
    switch (error) {
        case ErrorType::INVALID_PIN:
        case ErrorType::INVALID_CHANNEL:
            ESP_LOGE(TAG, "Invalid hardware resource specified");
            break;
            
        case ErrorType::NOT_INITIALIZED:
            ESP_LOGE(TAG, "Hardware not initialized");
            break;
            
        case ErrorType::HARDWARE_FAILURE:
            ESP_LOGE(TAG, "Hardware failure detected");
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown error occurred");
            break;
    }
    
    return false;
}

// Usage
HfGpioErr result = gpio.SetDirection(BaseGpio::Direction::Output);
if (!HandleError(result, "GPIO direction set")) {
    return false;
}
```

### Exception Safety

```cpp
// Exception-safe hardware operations
class ExceptionSafeController {
private:
    McuDigitalGpio safety_gpio_;
    
public:
    void SafeOperation() {
        // RAII ensures cleanup even on exceptions
        DigitalOutputGuard safety_guard(safety_gpio_);
        
        if (!safety_guard.IsValid()) {
            throw std::runtime_error("Failed to activate safety system");
        }
        
        try {
            // Risky operations
            PerformRiskyOperation1();
            PerformRiskyOperation2();
            
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Operation failed: %s", e.what());
            // Safety GPIO automatically deactivated by guard
            throw; // Re-throw for caller to handle
        }
        
        // Safety GPIO automatically deactivated on normal exit
    }
};
```

## 🧪 Testing Strategies

### 1. Unit Testing with Mocks

```cpp
// Mock implementation for testing
class MockGpio : public BaseGpio {
private:
    bool is_active_ = false;
    Direction direction_ = Direction::Input;
    
public:
    bool Initialize() noexcept override { return true; }
    bool Deinitialize() noexcept override { return true; }
    
    HfGpioErr SetDirection(Direction direction) noexcept override {
        direction_ = direction;
        return HfGpioErr::GPIO_SUCCESS;
    }
    
    HfGpioErr SetActive() noexcept override {
        if (direction_ == Direction::Output) {
            is_active_ = true;
            return HfGpioErr::GPIO_SUCCESS;
        }
        return HfGpioErr::GPIO_ERR_INVALID_CONFIG;
    }
    
    bool IsActive() const noexcept override {
        return is_active_;
    }
    
    // ... implement other methods
};

// Test case
void TestGpioGuard() {
    MockGpio mock_gpio;
    mock_gpio.Initialize();
    mock_gpio.SetDirection(BaseGpio::Direction::Output);
    
    {
        DigitalOutputGuard guard(mock_gpio);
        assert(guard.IsValid());
        assert(mock_gpio.IsActive()); // Should be active
    }
    
    assert(!mock_gpio.IsActive()); // Should be inactive after guard destructs
}
```

### 2. Integration Testing

```cpp
// Integration test with real hardware
void TestRealHardware() {
    McuDigitalGpio test_gpio(2);
    
    // Test initialization
    assert(test_gpio.Initialize());
    
    // Test configuration
    assert(test_gpio.SetDirection(BaseGpio::Direction::Output) == HfGpioErr::GPIO_SUCCESS);
    
    // Test operations
    assert(test_gpio.SetActive() == HfGpioErr::GPIO_SUCCESS);
    assert(test_gpio.IsActive());
    
    assert(test_gpio.SetInactive() == HfGpioErr::GPIO_SUCCESS);
    assert(!test_gpio.IsActive());
    
    // Test cleanup
    assert(test_gpio.Deinitialize());
}
```

### 3. Performance Testing

```cpp
void BenchmarkGpioOperations() {
    McuDigitalGpio gpio(2);
    gpio.Initialize();
    gpio.SetDirection(BaseGpio::Direction::Output);
    
    // Benchmark direct operations
    auto start = esp_timer_get_time();
    for (int i = 0; i < 1000; ++i) {
        gpio.SetActive();
        gpio.SetInactive();
    }
    auto end = esp_timer_get_time();
    
    printf("Direct GPIO operations: %lld µs per toggle\n", 
           (end - start) / 2000);
           
    // Benchmark thread-safe operations
    auto safe_gpio = std::make_shared<SfGpio>(
        std::make_shared<McuDigitalGpio>(3)
    );
    
    start = esp_timer_get_time();
    for (int i = 0; i < 1000; ++i) {
        safe_gpio->digitalWrite(3, true);
        safe_gpio->digitalWrite(3, false);
    }
    end = esp_timer_get_time();
    
    printf("Thread-safe GPIO operations: %lld µs per toggle\n", 
           (end - start) / 2000);
}
```

## 🔧 Advanced Topics

### Custom Platform Support

To add support for a new platform:

1. **Create platform-specific types** in `McuTypes.h`
2. **Implement base interfaces** (e.g., `CustomGpio : public BaseGpio`)
3. **Add platform detection** in build system
4. **Test thoroughly** with both unit and integration tests

### Performance Optimization

```cpp
// High-performance GPIO operations
class HighPerformanceGpio {
private:
    volatile uint32_t* set_register_;
    volatile uint32_t* clear_register_;
    uint32_t pin_mask_;
    
public:
    // Direct register access for maximum speed
    inline void SetHigh() {
        *set_register_ = pin_mask_;
    }
    
    inline void SetLow() {
        *clear_register_ = pin_mask_;
    }
};
```

### Memory-Constrained Environments

```cpp
// Minimize memory usage for resource-constrained systems
class CompactGpioManager {
private:
    // Use bit fields to minimize memory
    struct GpioState {
        uint8_t pin : 6;           // Up to 64 pins
        uint8_t is_output : 1;     // Direction
        uint8_t is_active : 1;     // State
    };
    
    static constexpr size_t MAX_GPIOS = 8;
    std::array<GpioState, MAX_GPIOS> gpio_states_;
    uint8_t gpio_count_ = 0;
    
public:
    bool AddGpio(uint8_t pin) {
        if (gpio_count_ >= MAX_GPIOS) return false;
        
        gpio_states_[gpio_count_] = {pin, 0, 0};
        gpio_count_++;
        return true;
    }
};
```

---

*This guide covers the essential patterns and practices for working with the HardFOC Internal Interface Wrapper. For specific API details, refer to the [API Documentation](../api/), and for practical examples, see the [Examples](../examples/).*
