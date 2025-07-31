# BaseGpio API Reference

## Overview

`BaseGpio` is the unified GPIO base class for all digital GPIO implementations in the HardFOC system. It provides a comprehensive digital GPIO abstraction that serves as the foundation for all GPIO hardware implementations.

## Features

- **Dynamic Mode Switching** - Runtime switching between input and output modes
- **Active State Polarity** - Configurable active-high/active-low polarity
- **Pull Resistor Control** - Internal pull-up, pull-down, and floating modes
- **Output Drive Modes** - Push-pull and open-drain output configurations
- **Interrupt Support** - Edge and level triggered interrupts with callbacks
- **Lazy Initialization** - Resources allocated only when needed
- **Comprehensive Error Handling** - 38 detailed error codes with descriptions

## Header File

```cpp
#include "inc/base/BaseGpio.h"
```

## Type Definitions

### Error Codes

```cpp
enum class hf_gpio_err_t : hf_u8_t {
    GPIO_SUCCESS = 0,                    // Success
    GPIO_ERR_FAILURE = 1,                // General failure
    GPIO_ERR_NOT_INITIALIZED = 2,        // Not initialized
    GPIO_ERR_ALREADY_INITIALIZED = 3,    // Already initialized
    GPIO_ERR_INVALID_PARAMETER = 4,      // Invalid parameter
    GPIO_ERR_NULL_POINTER = 5,           // Null pointer
    GPIO_ERR_OUT_OF_MEMORY = 6,          // Out of memory
    GPIO_ERR_PIN_NOT_FOUND = 7,          // Pin not found
    GPIO_ERR_PIN_NOT_AVAILABLE = 8,      // Pin not available
    GPIO_ERR_PIN_ALREADY_CONFIGURED = 9, // Pin already configured
    GPIO_ERR_INVALID_PIN = 10,           // Invalid pin
    GPIO_ERR_INVALID_DIRECTION = 11,     // Invalid direction
    GPIO_ERR_INVALID_STATE = 12,         // Invalid state
    GPIO_ERR_INVALID_LEVEL = 13,         // Invalid level
    GPIO_ERR_INVALID_POLARITY = 14,      // Invalid polarity
    GPIO_ERR_INVALID_PULL_MODE = 15,     // Invalid pull mode
    GPIO_ERR_INVALID_OUTPUT_MODE = 16,   // Invalid output mode
    GPIO_ERR_INVALID_INTERRUPT_TYPE = 17,// Invalid interrupt type
    GPIO_ERR_INTERRUPT_NOT_SUPPORTED = 18,// Interrupt not supported
    GPIO_ERR_INTERRUPT_ALREADY_ENABLED = 19,// Interrupt already enabled
    GPIO_ERR_INTERRUPT_NOT_ENABLED = 20, // Interrupt not enabled
    GPIO_ERR_CALLBACK_NOT_SET = 21,      // Callback not set
    GPIO_ERR_HARDWARE_FAILURE = 22,      // Hardware failure
    GPIO_ERR_TIMEOUT = 23,               // Timeout
    GPIO_ERR_BUSY = 24,                  // Busy
    GPIO_ERR_NOT_SUPPORTED = 25,         // Not supported
    GPIO_ERR_CONFIGURATION_LOCKED = 26,  // Configuration locked
    GPIO_ERR_POWER_MANAGEMENT = 27,      // Power management
    GPIO_ERR_VOLTAGE_LEVEL = 28,         // Voltage level
    GPIO_ERR_DRIVE_STRENGTH = 29,        // Drive strength
    GPIO_ERR_SLEW_RATE = 30,            // Slew rate
    GPIO_ERR_SCHMITT_TRIGGER = 31,      // Schmitt trigger
    GPIO_ERR_OPEN_DRAIN_CONFLICT = 32,  // Open drain conflict
    GPIO_ERR_PULL_RESISTOR_CONFLICT = 33,// Pull resistor conflict
    GPIO_ERR_MULTI_DRIVE_CONFLICT = 34, // Multi drive conflict
    GPIO_ERR_INPUT_BUFFER_DISABLE = 35, // Input buffer disable
    GPIO_ERR_OUTPUT_BUFFER_DISABLE = 36,// Output buffer disable
    GPIO_ERR_CALIBRATION_FAILURE = 37   // Calibration failure
};
```

### State and Level Types

```cpp
enum class hf_gpio_state_t : hf_u8_t {
    HF_GPIO_STATE_INACTIVE = 0,   // Logical inactive state
    HF_GPIO_STATE_ACTIVE = 1      // Logical active state
};

enum class hf_gpio_level_t : hf_u8_t {
    HF_GPIO_LEVEL_LOW = 0,        // Electrical low level (0V)
    HF_GPIO_LEVEL_HIGH = 1        // Electrical high level (VCC)
};

enum class hf_gpio_active_state_t : hf_u8_t {
    HF_GPIO_ACTIVE_LOW = 0,       // Active state is electrical low
    HF_GPIO_ACTIVE_HIGH = 1       // Active state is electrical high
};
```

### Direction and Configuration Types

```cpp
enum class hf_gpio_direction_t : hf_u8_t {
    HF_GPIO_DIRECTION_INPUT = 0,  // Pin configured as input
    HF_GPIO_DIRECTION_OUTPUT = 1  // Pin configured as output
};

enum class hf_gpio_output_mode_t : hf_u8_t {
    HF_GPIO_OUTPUT_MODE_PUSH_PULL = 0,   // Push-pull output
    HF_GPIO_OUTPUT_MODE_OPEN_DRAIN = 1   // Open-drain output
};

enum class hf_gpio_pull_mode_t : hf_u8_t {
    HF_GPIO_PULL_MODE_FLOATING = 0,      // No pull resistor
    HF_GPIO_PULL_MODE_UP = 1,            // Internal pull-up resistor
    HF_GPIO_PULL_MODE_DOWN = 2,          // Internal pull-down resistor
    HF_GPIO_PULL_MODE_UP_DOWN = 3        // Both pull resistors
};
```

### Interrupt Types

```cpp
enum class hf_gpio_interrupt_trigger_t : hf_u8_t {
    HF_GPIO_INTERRUPT_TRIGGER_NONE = 0,        // No interrupt
    HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE = 1, // Rising edge
    HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE = 2,// Falling edge
    HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES = 3,  // Both edges
    HF_GPIO_INTERRUPT_TRIGGER_LOW_LEVEL = 4,   // Low level
    HF_GPIO_INTERRUPT_TRIGGER_HIGH_LEVEL = 5   // High level
};
```

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
    hf_pin_num_t GetPin() const noexcept;
    
    // Pure virtual methods (implemented by derived classes)
    virtual bool Initialize() noexcept = 0;
    virtual bool Deinitialize() noexcept = 0;
    
    // Direction and mode control
    virtual hf_gpio_err_t SetDirection(hf_gpio_direction_t direction) noexcept = 0;
    virtual hf_gpio_err_t GetDirection(hf_gpio_direction_t& direction) const noexcept = 0;
    
    // State and level operations
    virtual hf_gpio_err_t SetState(hf_gpio_state_t state) noexcept = 0;
    virtual hf_gpio_err_t GetState(hf_gpio_state_t& state) const noexcept = 0;
    virtual hf_gpio_err_t SetLevel(hf_gpio_level_t level) noexcept = 0;
    virtual hf_gpio_err_t GetLevel(hf_gpio_level_t& level) const noexcept = 0;
    
    // Polarity configuration
    virtual hf_gpio_err_t SetActiveState(hf_gpio_active_state_t active_state) noexcept = 0;
    virtual hf_gpio_err_t GetActiveState(hf_gpio_active_state_t& active_state) const noexcept = 0;
    
    // Pull resistor configuration
    virtual hf_gpio_err_t SetPullMode(hf_gpio_pull_mode_t pull_mode) noexcept = 0;
    virtual hf_gpio_err_t GetPullMode(hf_gpio_pull_mode_t& pull_mode) const noexcept = 0;
    
    // Output mode configuration
    virtual hf_gpio_err_t SetOutputMode(hf_gpio_output_mode_t output_mode) noexcept = 0;
    virtual hf_gpio_err_t GetOutputMode(hf_gpio_output_mode_t& output_mode) const noexcept = 0;
    
    // Interrupt configuration
    virtual hf_gpio_err_t EnableInterrupt(hf_gpio_interrupt_trigger_t trigger,
                                         std::function<void()> callback) noexcept = 0;
    virtual hf_gpio_err_t DisableInterrupt() noexcept = 0;
    virtual hf_gpio_err_t IsInterruptEnabled(bool& enabled) const noexcept = 0;
};
```

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
```

## Usage Examples

### Basic GPIO Operations

```cpp
#include "inc/mcu/esp32/EspGpio.h"

// Create output pin for LED control
EspGpio led_pin(GPIO_NUM_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

// Initialize the pin
if (!led_pin.EnsureInitialized()) {
    printf("Failed to initialize LED pin\n");
    return;
}

// Turn LED on and off
led_pin.SetActive();    // Turn on LED
vTaskDelay(1000);
led_pin.SetInactive();  // Turn off LED

// Toggle LED state
led_pin.Toggle();
```

### Input Pin with Pull-up

```cpp
// Create input pin for button
EspGpio button_pin(GPIO_NUM_0, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

// Initialize and configure pull-up
button_pin.EnsureInitialized();
button_pin.SetPullMode(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP);
button_pin.SetActiveState(hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW);

// Read button state
if (button_pin.IsActive()) {
    printf("Button is pressed\n");
}
```

### Interrupt Handling

```cpp
// Interrupt callback function
void button_interrupt_handler() {
    printf("Button interrupt triggered!\n");
}

// Create input pin with interrupt
EspGpio interrupt_pin(GPIO_NUM_4, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

// Configure interrupt
interrupt_pin.EnsureInitialized();
interrupt_pin.SetPullMode(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP);
interrupt_pin.EnableInterrupt(
    hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE,
    button_interrupt_handler
);
```

### Error Handling

```cpp
hf_gpio_err_t result = gpio_pin.SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    printf("GPIO Error: %s\n", HfGpioErrToString(result));
    // Handle error appropriately
}
```

## Utility Functions

```cpp
// Convert error code to string
const char* HfGpioErrToString(hf_gpio_err_t err) noexcept;
```

## Thread Safety

The BaseGpio class is **not thread-safe**. If you need to access GPIO from multiple threads, you must provide your own synchronization mechanisms (e.g., mutexes, semaphores).

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

- [EspGpio API Reference](EspGpio.md) - ESP32-C6 implementation
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions 