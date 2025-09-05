# 🔌 BaseGpio API Reference

<div align="center">

**📋 Navigation**

[← Previous: HardwareTypes](HardwareTypes.md) | [Back to API Index](README.md) | [Next: BaseAdc
→](BaseAdc.md)

</div>

---

## 🌟 Overview

`BaseGpio` is the unified GPIO base class for all digital GPIO implementations in the HardFOC
system.
It provides a comprehensive digital GPIO abstraction that serves as the foundation for all GPIO
hardware implementations.

## ✨ Features

- **🔄 Dynamic Mode Switching** - Runtime switching between input and output modes
- **⚡ Active State Polarity** - Configurable active-high/active-low polarity
- **🔧 Pull Resistor Control** - Internal pull-up, pull-down, and floating modes
- **🚀 Output Drive Modes** - Push-pull and open-drain output configurations
- **⚡ Interrupt Support** - Edge and level triggered interrupts with callbacks
- **🔧 Lazy Initialization** - Resources allocated only when needed
- **🛡️ Comprehensive Error Handling** - 38 detailed error codes with descriptions

## Header File

```cpp
#include "inc/base/BaseGpio.h"
```text

## Type Definitions

### Error Codes

```cpp
enum class hf*gpio*err*t : hf*u8*t {
    GPIO*SUCCESS = 0,                    // ✅ Success
    GPIO*ERR*FAILURE = 1,                // ❌ General failure
    GPIO*ERR*NOT*INITIALIZED = 2,        // ⚠️ Not initialized
    GPIO*ERR*ALREADY*INITIALIZED = 3,    // ⚠️ Already initialized
    GPIO*ERR*INVALID*PARAMETER = 4,      // 🚫 Invalid parameter
    GPIO*ERR*NULL*POINTER = 5,           // 🚫 Null pointer
    GPIO*ERR*OUT*OF*MEMORY = 6,          // 💾 Out of memory
    GPIO*ERR*INVALID*PIN = 7,            // 🔍 Invalid pin
    GPIO*ERR*PIN*NOT*FOUND = 8,          // 🔍 Pin not found
    GPIO*ERR*PIN*NOT*CONFIGURED = 9,     // ⚙️ Pin not configured
    GPIO*ERR*PIN*ALREADY*REGISTERED = 10, // 📝 Pin already registered
    GPIO*ERR*PIN*ACCESS*DENIED = 11,     // 🔒 Pin access denied
    GPIO*ERR*PIN*BUSY = 12,              // 🔄 Pin busy
    GPIO*ERR*HARDWARE*FAULT = 13,        // 💥 Hardware fault
    GPIO*ERR*COMMUNICATION*FAILURE = 14, // 📡 Communication failure
    GPIO*ERR*DEVICE*NOT*RESPONDING = 15, // 🔇 Device not responding
    GPIO*ERR*TIMEOUT = 16,               // ⏰ Timeout
    GPIO*ERR*VOLTAGE*OUT*OF*RANGE = 17,  // ⚡ Voltage out of range
    GPIO*ERR*INVALID*CONFIGURATION = 18, // ⚙️ Invalid configuration
    GPIO*ERR*UNSUPPORTED*OPERATION = 19, // 🚫 Unsupported operation
    GPIO*ERR*RESOURCE*BUSY = 20,         // 🔄 Resource busy
    GPIO*ERR*RESOURCE*UNAVAILABLE = 21,  // 🚫 Resource unavailable
    GPIO*ERR*READ*FAILURE = 22,          // 📖 Read failure
    GPIO*ERR*WRITE*FAILURE = 23,         // ✍️ Write failure
    GPIO*ERR*DIRECTION*MISMATCH = 24,    // 🔄 Direction mismatch
    GPIO*ERR*PULL*RESISTOR*FAILURE = 25, // 🔧 Pull resistor failure
    GPIO*ERR*INTERRUPT*NOT*SUPPORTED = 26, // ⚡ Interrupt not supported
    GPIO*ERR*INTERRUPT*ALREADY*ENABLED = 27, // ⚡ Interrupt already enabled
    GPIO*ERR*INTERRUPT*NOT*ENABLED = 28, // ⚡ Interrupt not enabled
    GPIO*ERR*INTERRUPT*HANDLER*FAILED = 29, // ⚡ Interrupt handler failed
    GPIO*ERR*SYSTEM*ERROR = 30,          // 💻 System error
    GPIO*ERR*PERMISSION*DENIED = 31,     // 🔒 Permission denied
    GPIO*ERR*OPERATION*ABORTED = 32,     // 🛑 Operation aborted
    GPIO*ERR*NOT*SUPPORTED = 33,         // 🚫 Operation not supported
    GPIO*ERR*DRIVER*ERROR = 34,          // 🔧 Driver error
    GPIO*ERR*INVALID*STATE = 35,         // ⚠️ Invalid state
    GPIO*ERR*INVALID*ARG = 36,           // 🚫 Invalid argument
    GPIO*ERR*CALIBRATION*FAILURE = 37    // 🔧 Calibration failure
};
```text

### State and Level Types

```cpp
enum class hf*gpio*state*t : hf*u8*t {
    HF*GPIO*STATE*INACTIVE = 0,   // Logical inactive state
    HF*GPIO*STATE*ACTIVE = 1      // Logical active state
};

enum class hf*gpio*level*t : hf*u8*t {
    HF*GPIO*LEVEL*LOW = 0,        // Electrical low level (0V)
    HF*GPIO*LEVEL*HIGH = 1        // Electrical high level (VCC)
};

enum class hf*gpio*active*state*t : hf*u8*t {
    HF*GPIO*ACTIVE*LOW = 0,       // Active state is electrical low
    HF*GPIO*ACTIVE*HIGH = 1       // Active state is electrical high
};
```text

### Direction and Configuration Types

```cpp
enum class hf*gpio*direction*t : hf*u8*t {
    HF*GPIO*DIRECTION*INPUT = 0,  // Pin configured as input
    HF*GPIO*DIRECTION*OUTPUT = 1  // Pin configured as output
};

enum class hf*gpio*output*mode*t : hf*u8*t {
    HF*GPIO*OUTPUT*MODE*PUSH*PULL = 0,   // Push-pull output
    HF*GPIO*OUTPUT*MODE*OPEN*DRAIN = 1   // Open-drain output
};

enum class hf*gpio*pull*mode*t : hf*u8*t {
    HF*GPIO*PULL*MODE*FLOATING = 0,      // No pull resistor
    HF*GPIO*PULL*MODE*UP = 1,            // Internal pull-up resistor
    HF*GPIO*PULL*MODE*DOWN = 2,          // Internal pull-down resistor
    HF*GPIO*PULL*MODE*UP*DOWN = 3        // Both pull resistors
};
```text

### Interrupt Types

```cpp
enum class hf*gpio*interrupt*trigger*t : hf*u8*t {
    HF*GPIO*INTERRUPT*TRIGGER*NONE = 0,        // No interrupt
    HF*GPIO*INTERRUPT*TRIGGER*RISING*EDGE = 1, // Rising edge
    HF*GPIO*INTERRUPT*TRIGGER*FALLING*EDGE = 2,// Falling edge
    HF*GPIO*INTERRUPT*TRIGGER*BOTH*EDGES = 3,  // Both edges
    HF*GPIO*INTERRUPT*TRIGGER*LOW*LEVEL = 4,   // Low level
    HF*GPIO*INTERRUPT*TRIGGER*HIGH*LEVEL = 5   // High level
};
```text

## Class Interface

```cpp
class BaseGpio {
public:
    // Construction and destruction
    BaseGpio(const BaseGpio& copy) = delete;
    BaseGpio& operator=(const BaseGpio& copy) = delete;
    virtual ~BaseGpio() = default;
    
    // Initialization and status
    bool IsInitialized() const noexcept;
    bool EnsureInitialized() noexcept;
    bool EnsureDeinitialized() noexcept;
    hf*pin*num*t GetPin() const noexcept;
    
    // Pure virtual methods (implemented by derived classes)
    virtual bool Initialize() noexcept = 0;
    virtual bool Deinitialize() noexcept = 0;
    
    // Direction and mode control
    virtual hf*gpio*err*t SetDirection(hf*gpio*direction*t direction) noexcept = 0;
    virtual hf*gpio*err*t GetDirection(hf*gpio*direction*t& direction) const noexcept = 0;
    
    // State and level operations
    virtual hf*gpio*err*t SetState(hf*gpio*state*t state) noexcept = 0;
    virtual hf*gpio*err*t GetState(hf*gpio*state*t& state) const noexcept = 0;
    virtual hf*gpio*err*t SetLevel(hf*gpio*level*t level) noexcept = 0;
    virtual hf*gpio*err*t GetLevel(hf*gpio*level*t& level) const noexcept = 0;
    
    // Polarity configuration
    virtual hf*gpio*err*t SetActiveState(hf*gpio*active*state*t active*state) noexcept = 0;
    virtual hf*gpio*err*t GetActiveState(hf*gpio*active*state*t& active*state) const noexcept = 0;
    
    // Pull resistor configuration
    virtual hf*gpio*err*t SetPullMode(hf*gpio*pull*mode*t pull*mode) noexcept = 0;
    virtual hf*gpio*err*t GetPullMode(hf*gpio*pull*mode*t& pull*mode) const noexcept = 0;
    
    // Output mode configuration
    virtual hf*gpio*err*t SetOutputMode(hf*gpio*output*mode*t output*mode) noexcept = 0;
    virtual hf*gpio*err*t GetOutputMode(hf*gpio*output*mode*t& output*mode) const noexcept = 0;
    
    // Interrupt configuration
    virtual hf*gpio*err*t EnableInterrupt(hf*gpio*interrupt*trigger*t trigger,
                                         std::function<void()> callback) noexcept = 0;
    virtual hf*gpio*err*t DisableInterrupt() noexcept = 0;
    virtual hf*gpio*err*t IsInterruptEnabled(bool& enabled) const noexcept = 0;
};
```text

## Convenience Methods

The BaseGpio class provides high-level convenience methods that build on the core API:

```cpp
// High-level state control
bool SetActive() noexcept;      // Set pin to logical active state
bool SetInactive() noexcept;    // Set pin to logical inactive state
bool IsActive() const noexcept; // Check if pin is in active state
bool Toggle() noexcept;         // Toggle pin state

// High-level level control
bool SetHigh() noexcept;        // Set pin to electrical high
bool SetLow() noexcept;         // Set pin to electrical low
bool IsHigh() const noexcept;   // Check if pin is electrical high
bool IsLow() const noexcept;    // Check if pin is electrical low
```text

## Usage Examples

### Basic GPIO Operations

```cpp
#include "inc/mcu/esp32/EspGpio.h"

// Create output pin for LED control
EspGpio led*pin(GPIO*NUM*2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);

// Initialize the pin
if (!led*pin.EnsureInitialized()) {
    printf("Failed to initialize LED pin\n");
    return;
}

// Turn LED on and off
led*pin.SetActive();    // Turn on LED
vTaskDelay(1000);
led*pin.SetInactive();  // Turn off LED

// Toggle LED state
led*pin.Toggle();
```text

### Input Pin with Pull-up

```cpp
// Create input pin for button
EspGpio button*pin(GPIO*NUM*0, hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT);

// Initialize and configure pull-up
button*pin.EnsureInitialized();
button*pin.SetPullMode(hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);
button*pin.SetActiveState(hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW);

// Read button state
if (button*pin.IsActive()) {
    printf("Button is pressed\n");
}
```text

### Interrupt Handling

```cpp
// Interrupt callback function
void button*interrupt*handler() {
    printf("Button interrupt triggered!\n");
}

// Create input pin with interrupt
EspGpio interrupt*pin(GPIO*NUM*4, hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT);

// Configure interrupt
interrupt*pin.EnsureInitialized();
interrupt*pin.SetPullMode(hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);
interrupt*pin.EnableInterrupt(
    hf*gpio*interrupt*trigger*t::HF*GPIO*INTERRUPT*TRIGGER*FALLING*EDGE,
    button*interrupt*handler
);
```text

### Error Handling

```cpp
hf*gpio*err*t result = gpio*pin.SetDirection(hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);
if (result != hf*gpio*err*t::GPIO*SUCCESS) {
    printf("GPIO Error: %s\n", HfGpioErrToString(result));
    // Handle error appropriately
}
```text

## Utility Functions

```cpp
// Convert error code to string
const char* HfGpioErrToString(hf*gpio*err*t err) noexcept;
```text

## Thread Safety

The BaseGpio class is **not thread-safe**.
If you need to access GPIO from multiple threads,
you must provide your own synchronization mechanisms (e.g., mutexes, semaphores).

## Implementation Notes

- **Lazy Initialization**: Hardware resources are only allocated when `EnsureInitialized()` is called
- **Error Recovery**: Most operations return detailed error codes for robust error handling
- **Platform Abstraction**: The base class hides platform-specific implementation details
- **Memory Management**: No dynamic memory allocation in the base interface

## Derived Classes

The following concrete implementations are available:

- **EspGpio** - ESP32-C6 on-chip GPIO implementation
- **I2cGpioExpander** - I2C-based GPIO expander support
- **SpiGpioExpander** - SPI-based GPIO expander support

## Related Documentation

- [EspGpio API Reference](../esp_api/EspGpio.md) - ESP32-C6 implementation
<<<<<<< Current (Your changes)
## - [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions 
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions

---

<div align="center">

**📋 Navigation**

[← Previous: HardwareTypes](HardwareTypes.md) | [Back to API Index](README.md) | [Next: BaseAdc
→](BaseAdc.md)

</div> 
>>>>>>> Incoming (Background Agent changes)
