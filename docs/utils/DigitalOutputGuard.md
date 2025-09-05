# DigitalOutputGuard

## Overview

The `DigitalOutputGuard` class provides **Resource Acquisition Is Initialization (RAII)** management
for GPIO output operations.
It ensures that a GPIO pin is automatically set to active state when the guard is created and
automatically set to inactive state when the guard is destroyed,
providing safe and reliable GPIO state management.

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
    explicit DigitalOutputGuard(BaseGpio& gpio, bool ensure*output*mode = true) noexcept;
    explicit DigitalOutputGuard(BaseGpio* gpio, bool ensure*output*mode = true) noexcept;
    
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
    [[nodiscard]] hf*gpio*err*t GetLastError() const noexcept;
    hf*gpio*err*t SetActive() noexcept;
    hf*gpio*err*t SetInactive() noexcept;
    [[nodiscard]] hf*gpio*state*t GetCurrentState() const noexcept;
};
```text

## Constructor Details

### Reference Constructor
```cpp
explicit DigitalOutputGuard(BaseGpio& gpio, bool ensure*output*mode = true) noexcept;
```text
- **Parameters**:
  - `gpio`: Reference to the BaseGpio instance to manage
  - `ensure*output*mode`: If true, automatically switch to output mode (default: true)
- **Behavior**: Configures the GPIO as output (if needed) and sets it to active state

### Pointer Constructor
```cpp
explicit DigitalOutputGuard(BaseGpio* gpio, bool ensure*output*mode = true) noexcept;
```text
- **Parameters**:
  - `gpio`: Pointer to the BaseGpio instance to manage (must not be null)
  - `ensure*output*mode`: If true, automatically switch to output mode (default: true)
- **Behavior**: Same as reference constructor, but with null pointer validation

## Destructor

```cpp
~DigitalOutputGuard() noexcept;
```text
- **Behavior**: Automatically sets the associated GPIO to inactive state
- **Note**: Does not change the pin direction to preserve configuration

## Public Methods

### State Validation
```cpp
[[nodiscard]] bool IsValid() const noexcept;
```text
- **Returns**: `true` if the guard was successfully initialized, `false` otherwise
- **Use Case**: Check if the guard is in a valid state before use

### Error Handling
```cpp
[[nodiscard]] hf*gpio*err*t GetLastError() const noexcept;
```text
- **Returns**: The last error code from guard operations
- **Use Case**: Diagnose initialization or operation failures

### Manual State Control
```cpp
hf*gpio*err*t SetActive() noexcept;
hf*gpio*err*t SetInactive() noexcept;
```text
- **Returns**: `hf*gpio*err*t::GPIO*SUCCESS` if successful, error code otherwise
- **Use Case**: Manual control while the guard is active
- **Note**: The destructor will still set the pin inactive when the guard goes out of scope

### State Query
```cpp
[[nodiscard]] hf*gpio*state*t GetCurrentState() const noexcept;
```text
- **Returns**: Current GPIO state (Active or Inactive)
- **Use Case**: Check the current state of the managed GPIO

## Usage Examples

### Basic RAII Usage
```cpp
// GPIO will be set active when guard is created
{
    DigitalOutputGuard guard(my*gpio);
    if (!guard.IsValid()) {
        // Handle initialization error
        return;
    }
    
    // GPIO is now active and ready for use
    // ... perform operations ...
    
} // GPIO automatically set inactive when guard goes out of scope
```text

### Manual State Control
```cpp
DigitalOutputGuard guard(my*gpio);
if (!guard.IsValid()) {
    return;
}

// Manually control the GPIO state
guard.SetInactive();  // Turn off
// ... some operations ...
guard.SetActive();    // Turn back on
// ... more operations ...

// GPIO will be automatically set inactive when guard is destroyed
```text

### Pointer-based Usage
```cpp
EspGpio* gpio*ptr = new EspGpio(pin, direction, active*state, output*mode, pull*mode);
if (!gpio*ptr->EnsureInitialized()) {
    delete gpio*ptr;
    return;
}

{
    DigitalOutputGuard guard(gpio*ptr);
    if (!guard.IsValid()) {
        // Handle error
        delete gpio*ptr;
        return;
    }
    
    // Use the GPIO through the guard
    guard.SetActive();
    // ... operations ...
    
} // Guard ensures GPIO is set inactive

delete gpio*ptr;
```text

### Move Semantics
```cpp
DigitalOutputGuard guard1(my*gpio);
if (!guard1.IsValid()) {
    return;
}

// Move the guard to another variable
DigitalOutputGuard guard2 = std::move(guard1);
// guard1 is now in a moved-from state
// guard2 now manages the GPIO

// Use guard2
guard2.SetActive();
```text

## Error Handling

The DigitalOutputGuard provides comprehensive error handling:

### Common Error Codes
- `hf*gpio*err*t::GPIO*SUCCESS`: Operation successful
- `hf*gpio*err*t::GPIO*ERR*NULL*POINTER`: Null pointer provided to constructor
- `hf*gpio*err*t::GPIO*ERR*NOT*INITIALIZED`: GPIO not properly initialized
- `hf*gpio*err*t::GPIO*ERR*DIRECTION*MISMATCH`: GPIO not in output mode and ensure*output*mode=false

### Error Handling Pattern
```cpp
DigitalOutputGuard guard(my*gpio);
if (!guard.IsValid()) {
    hf*gpio*err*t error = guard.GetLastError();
    switch (error) {
        case hf*gpio*err*t::GPIO*ERR*NULL*POINTER:
            ESP*LOGE(TAG, "Null pointer provided");
            break;
        case hf*gpio*err*t::GPIO*ERR*NOT*INITIALIZED:
            ESP*LOGE(TAG, "GPIO not initialized");
            break;
        case hf*gpio*err*t::GPIO*ERR*DIRECTION*MISMATCH:
            ESP*LOGE(TAG, "GPIO direction mismatch");
            break;
        default:
            ESP*LOGE(TAG, "Unknown error: %d", static*cast<int>(error));
            break;
    }
    return;
}
```text

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
- Uses standard `hf*gpio*err*t` and `hf*gpio*state*t` types

## See Also

- [BaseGpio](../api/BaseGpio.md) - Base GPIO interface
- [EspGpio](../esp_api/EspGpio.md) - ESP32 GPIO implementation
- [HardwareTypes](../api/HardwareTypes.md) - Hardware type definitions
