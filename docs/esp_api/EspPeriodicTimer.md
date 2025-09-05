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
```text

## Class Definition

```cpp
class EspPeriodicTimer : public BasePeriodicTimer {
public:
    // Constructor with full configuration
    explicit EspPeriodicTimer(
        hf*timer*group*t group = hf*timer*group*t::HF*TIMER*GROUP*0,
        hf*timer*t timer = hf*timer*t::HF*TIMER*0,
        hf*timer*scale*t scale = hf*timer*scale*t::HF*TIMER*SCALE*MS,
        bool auto*reload = true
    ) noexcept;

    // Destructor
    ~EspPeriodicTimer() override;

    // BasePeriodicTimer implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // Timer operations
    hf*timer*err*t Start(hf*timer*period*t period) noexcept override;
    hf*timer*err*t Stop() noexcept override;
    hf*timer*err*t IsRunning(bool* running) const noexcept override;
    hf*timer*err*t SetPeriod(hf*timer*period*t period) noexcept override;
    hf*timer*err*t GetPeriod(hf*timer*period*t* period) const noexcept override;
    hf*timer*err*t GetCount(hf*timer*count*t* count) const noexcept override;
    hf*timer*err*t Reset() noexcept override;

    // Advanced features
    hf*timer*err*t SetCallback(hf*timer*callback*t callback, void* user*data) noexcept override;
    hf*timer*err*t ClearCallback() noexcept override;
    hf*timer*err*t SetScale(hf*timer*scale*t scale) noexcept override;
    hf*timer*err*t GetScale(hf*timer*scale*t* scale) const noexcept override;
    hf*timer*err*t SetAutoReload(bool auto*reload) noexcept override;
    hf*timer*err*t GetAutoReload(bool* auto*reload) const noexcept override;
};
```text

## Usage Examples

### Basic Timer Usage

```cpp
#include "inc/mcu/esp32/EspPeriodicTimer.h"

// Create timer instance
EspPeriodicTimer timer(HF*TIMER*GROUP*0, HF*TIMER*0, HF*TIMER*SCALE*MS);

// Initialize
if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Start timer with 1 second period
hf*timer*err*t err = timer.Start(1000); // 1000ms = 1 second
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Check if timer is running
bool running;
err = timer.IsRunning(&running);
if (err == HF*TIMER*ERR*OK && running) {
    printf("Timer is running\n");
}

// Stop timer
err = timer.Stop();
if (err == HF*TIMER*ERR*OK) {
    printf("Timer stopped\n");
}
```text

### Timer with Callback

```cpp
// Timer callback function
void timer*callback(void* user*data) {
    static int count = 0;
    count++;
    printf("Timer callback called %d times\n", count);
    
    // Access user data if needed
    if (user*data) {
        int* data = static*cast<int*>(user*data);
        (*data)++;
    }
}

// Create timer with callback
int callback*data = 0;
EspPeriodicTimer timer(HF*TIMER*GROUP*0, HF*TIMER*0, HF*TIMER*SCALE*MS);

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Set callback
hf*timer*err*t err = timer.SetCallback(timer*callback, &callback*data);
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to set callback: %d\n", err);
    return;
}

// Start timer with 500ms period
err = timer.Start(500);
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Let timer run for a while
vTaskDelay(pdMS*TO*TICKS(5000)); // 5 seconds

// Stop timer
timer.Stop();
printf("Callback data: %d\n", callback*data);
```text

### Multiple Timers

```cpp
// Create multiple timers
EspPeriodicTimer timer1(HF*TIMER*GROUP*0, HF*TIMER*0, HF*TIMER*SCALE*MS);
EspPeriodicTimer timer2(HF*TIMER*GROUP*0, HF*TIMER*1, HF*TIMER*SCALE*MS);
EspPeriodicTimer timer3(HF*TIMER*GROUP*1, HF*TIMER*0, HF*TIMER*SCALE*US); // Microsecond precision

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
vTaskDelay(pdMS*TO*TICKS(3000)); // 3 seconds

// Stop all timers
timer1.Stop();
timer2.Stop();
timer3.Stop();
```text

### High Precision Timing

```cpp
// Create high-precision timer
EspPeriodicTimer timer(HF*TIMER*GROUP*0, HF*TIMER*0, HF*TIMER*SCALE*US);

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Set microsecond precision
hf*timer*err*t err = timer.SetScale(HF*TIMER*SCALE*US);
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to set scale: %d\n", err);
    return;
}

// Start with 1000 microsecond period (1ms)
err = timer.Start(1000);
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Get current count
hf*timer*count*t count;
err = timer.GetCount(&count);
if (err == HF*TIMER*ERR*OK) {
    printf("Current timer count: %llu\n", count);
}
```text

### Timer Configuration

```cpp
// Create timer with specific configuration
EspPeriodicTimer timer(HF*TIMER*GROUP*0, HF*TIMER*0, HF*TIMER*SCALE*MS, false); // No auto-reload

if (!timer.Initialize()) {
    printf("Failed to initialize timer\n");
    return;
}

// Configure timer properties
timer.SetAutoReload(true);  // Enable auto-reload
timer.SetScale(HF*TIMER*SCALE*MS);  // Millisecond scale

// Start timer
hf*timer*err*t err = timer.Start(2000); // 2 second period
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to start timer: %d\n", err);
    return;
}

// Change period while running
err = timer.SetPeriod(1000); // Change to 1 second
if (err != HF*TIMER*ERR*OK) {
    printf("Failed to set period: %d\n", err);
    return;
}

// Reset timer
err = timer.Reset();
if (err == HF*TIMER*ERR*OK) {
    printf("Timer reset\n");
}
```text

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

- `HF*TIMER*ERR*OK` - Operation successful
- `HF*TIMER*ERR*INVALID*ARG` - Invalid parameter
- `HF*TIMER*ERR*NOT*INITIALIZED` - Timer not initialized
- `HF*TIMER*ERR*INVALID*GROUP` - Invalid timer group
- `HF*TIMER*ERR*INVALID*TIMER` - Invalid timer
- `HF*TIMER*ERR*ALREADY*RUNNING` - Timer already running
- `HF*TIMER*ERR*NOT*RUNNING` - Timer not running
- `HF*TIMER*ERR*CALLBACK*FAILED` - Callback execution failed

## Performance Considerations

- **Timer Groups**: Use different timer groups for independent timers
- **Precision**: Choose appropriate scale for your timing requirements
- **Callbacks**: Keep callback functions short and efficient
- **Power**: Consider power consumption for battery-powered applications

## Related Documentation

- [BasePeriodicTimer API Reference](../api/BasePeriodicTimer.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF Timer Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/gptimer.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspNvs](EspNvs.md) | [Back to ESP API Index](README.md) | [Next: EspTemperature
→](EspTemperature.md)

</div>