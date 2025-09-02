# SoftwarePeriodicTimer API Reference

## Overview

`SoftwarePeriodicTimer` is an alternative software-based implementation of the `BasePeriodicTimer` interface, providing periodic timer functionality through software emulation with enhanced features for development and testing.

## Features

- **Software Emulation** - Pure software implementation of periodic timers
- **Enhanced Testing** - Advanced testing and simulation capabilities
- **Cross-Platform** - Works on any platform with basic timing capabilities
- **Configurable** - Flexible configuration for different use cases
- **Debug Support** - Enhanced debugging and logging capabilities
- **Performance Monitoring** - Built-in performance monitoring

## Header File

```cpp
#include "inc/mcu/software/SoftwarePeriodicTimer.h"
```

## Class Definition

```cpp
class SoftwarePeriodicTimer : public BasePeriodicTimer {
public:
    // Constructor with full configuration
    explicit SoftwarePeriodicTimer(
        hf_timer_group_t group = hf_timer_group_t::HF_TIMER_GROUP_0,
        hf_timer_t timer = hf_timer_t::HF_TIMER_0,
        hf_timer_scale_t scale = hf_timer_scale_t::HF_TIMER_SCALE_MS,
        bool auto_reload = true
    ) noexcept;

    // Destructor
    ~SoftwarePeriodicTimer() override;

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
#include "inc/mcu/software/SoftwarePeriodicTimer.h"

// Create software timer instance
SoftwarePeriodicTimer timer(HF_TIMER_GROUP_0, HF_TIMER_0, HF_TIMER_SCALE_MS);

// Initialize
if (!timer.Initialize()) {
    printf("Failed to initialize software timer\n");
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
    printf("Software timer is running\n");
}

// Stop timer
err = timer.Stop();
if (err == HF_TIMER_ERR_OK) {
    printf("Timer stopped\n");
}
```

## Related Documentation

- [BasePeriodicTimer API Reference](BasePeriodicTimer.md) - Base class interface
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions