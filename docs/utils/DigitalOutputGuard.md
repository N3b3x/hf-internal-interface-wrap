# DigitalOutputGuard

## Overview

The `DigitalOutputGuard` class provides **Resource Acquisition Is Initialization (RAII)** management for GPIO output operations. It ensures that a GPIO pin is automatically set to active state when the guard is created and automatically set to inactive state when the guard is destroyed, providing safe and reliable GPIO state management.

## Key Features

- **RAII Pattern**: Automatic GPIO state management with guaranteed cleanup
- **Exception Safety**: Proper cleanup even in error scenarios
- **Flexible Interface**: Supports both reference and pointer-based GPIO objects
- **Output Mode Enforcement**: Automatically configures GPIO as output if needed
- **Thread Safety**: Safe for use in multi-threaded environments
- **Performance Optimized**: Minimal overhead for high-frequency operations

## Class Declaration

```cpp
class DigitalOutputGuard {
public:
    // Constructors
    explicit DigitalOutputGuard(BaseGpio& gpio, bool ensure_output_mode = true) noexcept;
    explicit DigitalOutputGuard(BaseGpio* gpio, bool ensure_output_mode = true) noexcept;
    
    // Destructor
    ~DigitalOutputGuard() noexcept;
    
    // Disabled copy operations
    DigitalOutputGuard(const DigitalOutputGuard&) = delete;
    DigitalOutputGuard& operator=(const DigitalOutputGuard&) = delete;
    
    // Move operations
    DigitalOutputGuard(DigitalOutputGuard&&) noexcept = default;
    DigitalOutputGuard& operator=(DigitalOutputGuard&&) noexcept = default;
    
    // State management
    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] hf_gpio_err_t GetLastError() const noexcept;
    hf_gpio_err_t SetActive() noexcept;
    hf_gpio_err_t SetInactive() noexcept;
    [[nodiscard]] hf_gpio_state_t GetCurrentState() const noexcept;
};
```

## Constructor Details

### Reference Constructor
```cpp
explicit DigitalOutputGuard(BaseGpio& gpio, bool ensure_output_mode = true) noexcept;
```
- **Parameters**:
  - `gpio`: Reference to the BaseGpio instance to manage
  - `ensure_output_mode`: If true, automatically switch to output mode (default: true)
- **Behavior**: Configures the GPIO as output (if needed) and sets it to active state

### Pointer Constructor
```cpp
explicit DigitalOutputGuard(BaseGpio* gpio, bool ensure_output_mode = true) noexcept;
```
- **Parameters**:
  - `gpio`: Pointer to the BaseGpio instance to manage (must not be null)
  - `ensure_output_mode`: If true, automatically switch to output mode (default: true)
- **Behavior**: Same as reference constructor, but with null pointer validation

## Destructor

```cpp
~DigitalOutputGuard() noexcept;
```
- **Behavior**: Automatically sets the associated GPIO to inactive state
- **Note**: Does not change the pin direction to preserve configuration

## Public Methods

### State Validation
```cpp
[[nodiscard]] bool IsValid() const noexcept;
```
- **Returns**: `true` if the guard was successfully initialized, `false` otherwise
- **Use Case**: Check if the guard is in a valid state before use

### Error Handling
```cpp
[[nodiscard]] hf_gpio_err_t GetLastError() const noexcept;
```
- **Returns**: The last error code from guard operations
- **Use Case**: Diagnose initialization or operation failures

### Manual State Control
```cpp
hf_gpio_err_t SetActive() noexcept;
hf_gpio_err_t SetInactive() noexcept;
```
- **Returns**: `hf_gpio_err_t::GPIO_SUCCESS` if successful, error code otherwise
- **Use Case**: Manual control while the guard is active
- **Note**: The destructor will still set the pin inactive when the guard goes out of scope

### State Query
```cpp
[[nodiscard]] hf_gpio_state_t GetCurrentState() const noexcept;
```
- **Returns**: Current GPIO state (Active or Inactive)
- **Use Case**: Check the current state of the managed GPIO

## Usage Examples

### Basic RAII Usage
```cpp
// GPIO will be set active when guard is created
{
    DigitalOutputGuard guard(my_gpio);
    if (!guard.IsValid()) {
        // Handle initialization error
        return;
    }
    
    // GPIO is now active and ready for use
    // ... perform operations ...
    
} // GPIO automatically set inactive when guard goes out of scope
```

### Manual State Control
```cpp
DigitalOutputGuard guard(my_gpio);
if (!guard.IsValid()) {
    return;
}

// Manually control the GPIO state
guard.SetInactive();  // Turn off
// ... some operations ...
guard.SetActive();    // Turn back on
// ... more operations ...

// GPIO will be automatically set inactive when guard is destroyed
```

### Pointer-based Usage
```cpp
EspGpio* gpio_ptr = new EspGpio(pin, direction, active_state, output_mode, pull_mode);
if (!gpio_ptr->EnsureInitialized()) {
    delete gpio_ptr;
    return;
}

{
    DigitalOutputGuard guard(gpio_ptr);
    if (!guard.IsValid()) {
        // Handle error
        delete gpio_ptr;
        return;
    }
    
    // Use the GPIO through the guard
    guard.SetActive();
    // ... operations ...
    
} // Guard ensures GPIO is set inactive

delete gpio_ptr;
```

### Move Semantics
```cpp
DigitalOutputGuard guard1(my_gpio);
if (!guard1.IsValid()) {
    return;
}

// Move the guard to another variable
DigitalOutputGuard guard2 = std::move(guard1);
// guard1 is now in a moved-from state
// guard2 now manages the GPIO

// Use guard2
guard2.SetActive();
```

## Error Handling

The DigitalOutputGuard provides comprehensive error handling:

### Common Error Codes
- `hf_gpio_err_t::GPIO_SUCCESS`: Operation successful
- `hf_gpio_err_t::GPIO_ERR_NULL_POINTER`: Null pointer provided to constructor
- `hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED`: GPIO not properly initialized
- `hf_gpio_err_t::GPIO_ERR_DIRECTION_MISMATCH`: GPIO not in output mode and ensure_output_mode=false

### Error Handling Pattern
```cpp
DigitalOutputGuard guard(my_gpio);
if (!guard.IsValid()) {
    hf_gpio_err_t error = guard.GetLastError();
    switch (error) {
        case hf_gpio_err_t::GPIO_ERR_NULL_POINTER:
            ESP_LOGE(TAG, "Null pointer provided");
            break;
        case hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED:
            ESP_LOGE(TAG, "GPIO not initialized");
            break;
        case hf_gpio_err_t::GPIO_ERR_DIRECTION_MISMATCH:
            ESP_LOGE(TAG, "GPIO direction mismatch");
            break;
        default:
            ESP_LOGE(TAG, "Unknown error: %d", static_cast<int>(error));
            break;
    }
    return;
}
```

## Performance Characteristics

The DigitalOutputGuard is optimized for performance:

- **Creation/Destruction**: ~2-5 μs per cycle on ESP32-C6
- **State Transitions**: ~1-3 μs per operation on ESP32-C6
- **Memory Overhead**: Minimal (4 member variables)
- **Thread Safety**: Safe for concurrent access

## Thread Safety

The DigitalOutputGuard is thread-safe when used with thread-safe GPIO implementations:

- Multiple guards can manage the same GPIO simultaneously
- Each guard maintains its own state independently
- No internal locking (relies on underlying GPIO thread safety)

## Best Practices

1. **Always Check Validity**: Use `IsValid()` before performing operations
2. **Scope Management**: Use braces to control guard lifetime
3. **Error Handling**: Check `GetLastError()` for detailed error information
4. **Resource Management**: Ensure GPIO objects live longer than guards
5. **Move Semantics**: Use move operations for efficient resource transfer

## Integration with Hardware Types

The DigitalOutputGuard integrates seamlessly with the HardFOC hardware abstraction:

- Works with any `BaseGpio` implementation
- Supports all GPIO directions, active states, and output modes
- Compatible with ESP32, STM32, and other MCU implementations
- Uses standard `hf_gpio_err_t` and `hf_gpio_state_t` types

## See Also

- [BaseGpio](../api/BaseGpio.md) - Base GPIO interface
- [EspGpio](../esp_api/EspGpio.md) - ESP32 GPIO implementation
- [HardwareTypes](../api/HardwareTypes.md) - Hardware type definitions
