# EspPeriodicTimer API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspNvs](EspNvs.md) | [Back to ESP API Index](README.md) | [Next: EspTemperature
→](EspTemperature.md)

</div>

---

## Overview

`EspPeriodicTimer` is the ESP32-C6 implementation of the `BasePeriodicTimer` interface,
providing comprehensive periodic timer functionality specifically optimized for ESP32-C6
microcontrollers running ESP-IDF v5.5+.
It offers both basic and advanced timer features with hardware-specific optimizations.

## Features

- **ESP32-C6 Timer** - Full support for ESP32-C6 timer capabilities
- **High Precision** - Microsecond-level precision
- **Multiple Timers** - Support for multiple independent timers
- **Interrupt Support** - Configurable interrupt handling
- **Power Management** - Deep sleep compatibility
- **Callback Support** - User-defined callback functions
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspPeriodicTimer.h"
```

## Class Definition

```cpp
class EspPeriodicTimer : public BasePeriodicTimer {
public:
    // Constructor with full configuration
    explicit EspPeriodicTimer(
        hf_timer_group_t group = hf_timer_group_t::HF_TIMER_GROUP_0,
        hf_timer_t timer = hf_timer_t::HF_TIMER_0,
        hf_timer_scale_t scale = hf_timer_scale_t::HF_TIMER_SCALE_MS,
        bool auto_reload = true
    ) noexcept;

    // Destructor
    ~EspPeriodicTimer() override;

    // BasePeriodicTimer implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // Timer operations
    hf_timer_err_t Start(hf_timer_period_t period) noexcept override;
    hf_timer_err_t Stop() noexcept override;
    hf_timer_err_t IsRunning(bool* running) const noexcept override;
    hf_timer_err_t SetPeriod(hf_timer_period_t period) noexcept override;
    hf_timer_err_t GetPeriod(hf_timer_period_t* period) const noexcept override;
    hf_timer_err_t GetCount(hf_timer_count_t* count) const noexcept override;
    hf_timer_err_t Reset() noexcept override;

    // Advanced features
    hf_timer_err_t SetCallback(hf_timer_callback_t callback, void* user_data) noexcept override;
    hf_timer_err_t ClearCallback() noexcept override;
    hf_timer_err_t SetScale(hf_timer_scale_t scale) noexcept override;
    hf_timer_err_t GetScale(hf_timer_scale_t* scale) const noexcept override;
    hf_timer_err_t SetAutoReload(bool auto_reload) noexcept override;
    hf_timer_err_t GetAutoReload(bool* auto_reload) const noexcept override;
};
```

## Usage Examples

### Basic Timer Usage

```cpp
#include "inc/mcu/esp32/EspPeriodicTimer.h"

// Create timer instance
EspPeriodicTimer timer(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_MS);

// Initialize
if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Start timer with 1 second period
hf_timer_err_t err = timer.Start(1000); // 1000ms = 1 second
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Check if timer is running
bool running;
err = timer.IsRunning(&running);
if (err == HF_TIMER_ERR_OK && running) {
    printf("Timer is running\n");
}

// Stop timer
err = timer.Stop();
if (err == HF_TIMER_ERR_OK) {
    printf("Timer stopped\n");
}
```

### Timer with Callback

```cpp
// Timer callback function
void timer_callback(void* user_data) {
    static int count = 0;
    count++;
    printf("Timer callback called %d times\n", count);
    
    // Access user data if needed
    if (user_data) {
        int* data = static_cast<int*>(user_data);
        (*data)++;
    }
}

// Create timer with callback
int callback_data = 0;
EspPeriodicTimer timer(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_MS);

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Set callback
hf_timer_err_t err = timer.SetCallback(timer_callback, &callback_data);
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to set callback: %d\n", err);
    return;
}

// Start timer with 500ms period
err = timer.Start(500);
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Let timer run for a while
vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds

// Stop timer
timer.Stop();
printf("Callback data: %d\n", callback_data);
```

### Multiple Timers

```cpp
// Create multiple timers
EspPeriodicTimer timer1(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_MS);
EspPeriodicTimer timer2(HF_TIMER_GROUP_0, HF_TIMER_1, HF_TIMER_SCALE_MS);
EspPeriodicTimer timer3(HF_TIMER_GROUP_1, HF_TIMER_0, HF_TIMER_SCALE_US); // Microsecond precision

// Initialize all timers
if (!timer1.Initialize() || !timer2.Initialize() || !timer3.Initialize()) {
    printf("Failed to initialize timers\n");
    return;
}

// Set different periods
timer1.Start(1000);  // 1 second
timer2.Start(500);   // 500ms
timer3.Start(10000); // 10ms (in microseconds)

// Set different callbacks
timer1.SetCallback([](void* data) { printf("Timer 1 tick\n"); }, nullptr);
timer2.SetCallback([](void* data) { printf("Timer 2 tick\n"); }, nullptr);
timer3.SetCallback([](void* data) { printf("Timer 3 tick\n"); }, nullptr);

// Let timers run
vTaskDelay(pdMS_TO_TICKS(3000)); // 3 seconds

// Stop all timers
timer1.Stop();
timer2.Stop();
timer3.Stop();
```

### High Precision Timing

```cpp
// Create high-precision timer
EspPeriodicTimer timer(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_US);

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Set microsecond precision
hf_timer_err_t err = timer.SetScale(HF_TIMER_SCALE_US);
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to set scale: %d\n", err);
    return;
}

// Start with 1000 microsecond period (1ms)
err = timer.Start(1000);
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Get current count
hf_timer_count_t count;
err = timer.GetCount(&count);
if (err == HF_TIMER_ERR_OK) {
    printf("Current timer count: %llu\n", count);
}
```

### Timer Configuration

```cpp
// Create timer with specific configuration
EspPeriodicTimer timer(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_MS, false); // No auto-reload

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Configure timer properties
timer.SetAutoReload(true);  // Enable auto-reload
timer.SetScale(HF_TIMER_SCALE_MS);  // Millisecond scale

// Start timer
hf_timer_err_t err = timer.Start(2000); // 2 second period
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Change period while running
err = timer.SetPeriod(1000); // Change to 1 second
if (err != HF_TIMER_ERR_OK) {
    printf("Failed to set period: %d\n", err);
    return;
}

// Reset timer
err = timer.Reset();
if (err == HF_TIMER_ERR_OK) {
    printf("Timer reset\n");
}
```

## ESP32-C6 Specific Features

### Hardware Timers

The ESP32-C6 provides multiple hardware timer groups with independent timers.

### High Precision

Support for microsecond-level precision timing.

### Interrupt Support

Hardware interrupt support for efficient timer handling.

### Power Management

Timers can be configured for deep sleep compatibility.

## Error Handling

The `EspPeriodicTimer` class provides comprehensive error handling with specific error codes:

- `HF_TIMER_ERR_OK` - Operation successful
- `HF_TIMER_ERR_INVALID_ARG` - Invalid parameter
- `HF_TIMER_ERR_NOT_INITIALIZED` - Timer not initialized
- `HF_TIMER_ERR_INVALID_GROUP` - Invalid timer group
- `HF_TIMER_ERR_INVALID_TIMER` - Invalid timer
- `HF_TIMER_ERR_ALREADY_RUNNING` - Timer already running
- `HF_TIMER_ERR_NOT_RUNNING` - Timer not running
- `HF_TIMER_ERR_CALLBACK_FAILED` - Callback execution failed

## Performance Considerations

- **Timer Groups**: Use different timer groups for independent timers
- **Precision**: Choose appropriate scale for your timing requirements
- **Callbacks**: Keep callback functions short and efficient
- **Power**: Consider power consumption for battery-powered applications

## Related Documentation

- [BasePeriodicTimer API Reference](../api/BasePeriodicTimer.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF Timer Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/gptimer.html) - ESP-IDF docs

---

<div align="center">

**📋 Navigation**

[← Previous: EspNvs](EspNvs.md) | [Back to ESP API Index](README.md) | [Next: EspTemperature
→](EspTemperature.md)

</div>