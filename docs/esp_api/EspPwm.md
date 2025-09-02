# EspPwm API Reference

## Overview

`EspPwm` is the ESP32-C6 implementation of the `BasePwm` interface, providing comprehensive PWM (Pulse Width Modulation) functionality specifically optimized for ESP32-C6 microcontrollers running ESP-IDF v5.5+. It offers both basic and advanced PWM features with hardware-specific optimizations.

## Features

- **ESP32-C6 LEDC Controller** - Full support for ESP32-C6 LEDC (LED Control) capabilities
- **Multiple Channels** - Up to 8 PWM channels per timer
- **High Resolution** - Up to 20-bit resolution
- **Fade Effects** - Hardware-accelerated fade in/out effects
- **Frequency Control** - Wide frequency range from Hz to MHz
- **Duty Cycle Control** - Precise duty cycle control
- **Power Management** - Deep sleep compatibility
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspPwm.h"
```

## Class Definition

```cpp
class EspPwm : public BasePwm {
public:
    // Constructor with full configuration
    explicit EspPwm(
        hf_pin_num_t pin,
        hf_pwm_channel_t channel = hf_pwm_channel_t::HF_PWM_CHANNEL_0,
        hf_pwm_timer_t timer = hf_pwm_timer_t::HF_PWM_TIMER_0,
        hf_pwm_freq_t frequency = 1000,
        hf_pwm_resolution_t resolution = hf_pwm_resolution_t::HF_PWM_RESOLUTION_8_BIT
    ) noexcept;

    // Destructor
    ~EspPwm() override;

    // BasePwm implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // PWM operations
    hf_pwm_err_t SetFrequency(hf_pwm_freq_t frequency) noexcept override;
    hf_pwm_err_t GetFrequency(hf_pwm_freq_t* frequency) const noexcept override;
    hf_pwm_err_t SetDutyCycle(hf_pwm_duty_t duty_cycle) noexcept override;
    hf_pwm_err_t GetDutyCycle(hf_pwm_duty_t* duty_cycle) const noexcept override;
    hf_pwm_err_t Start() noexcept override;
    hf_pwm_err_t Stop() noexcept override;
    hf_pwm_err_t IsRunning(bool* running) const noexcept override;

    // Advanced features
    hf_pwm_err_t SetResolution(hf_pwm_resolution_t resolution) noexcept override;
    hf_pwm_err_t GetResolution(hf_pwm_resolution_t* resolution) const noexcept override;
    hf_pwm_err_t FadeStart(hf_pwm_duty_t target_duty, hf_u32_t fade_time_ms) noexcept override;
    hf_pwm_err_t FadeStop() noexcept override;
    hf_pwm_err_t IsFading(bool* fading) const noexcept override;
};
```

## Usage Examples

### Basic PWM Control

```cpp
#include "inc/mcu/esp32/EspPwm.h"

// Create PWM instance
EspPwm pwm(GPIO_NUM_2, HF_PWM_CHANNEL_0, HF_PWM_TIMER_0, 1000, HF_PWM_RESOLUTION_8_BIT);

// Initialize
if (!pwm.Initialize()) {
    printf("Failed to initialize PWM\n");
    return;
}

// Set duty cycle (0-100%)
hf_pwm_err_t err = pwm.SetDutyCycle(50.0f); // 50% duty cycle
if (err != HF_PWM_ERR_OK) {
    printf("Failed to set duty cycle: %d\n", err);
    return;
}

// Start PWM
err = pwm.Start();
if (err != HF_PWM_ERR_OK) {
    printf("Failed to start PWM: %d\n", err);
    return;
}

printf("PWM started with 50%% duty cycle\n");
```

### Frequency Control

```cpp
// Set different frequencies
hf_pwm_freq_t frequencies[] = {100, 500, 1000, 5000, 10000};
for (int i = 0; i < 5; i++) {
    hf_pwm_err_t err = pwm.SetFrequency(frequencies[i]);
    if (err == HF_PWM_ERR_OK) {
        printf("PWM frequency set to %d Hz\n", frequencies[i]);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
    }
}
```

### Fade Effects

```cpp
// Fade in from 0% to 100% over 2 seconds
hf_pwm_err_t err = pwm.FadeStart(100.0f, 2000);
if (err != HF_PWM_ERR_OK) {
    printf("Failed to start fade: %d\n", err);
    return;
}

// Wait for fade to complete
bool fading = true;
while (fading) {
    err = pwm.IsFading(&fading);
    if (err != HF_PWM_ERR_OK) {
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

printf("Fade completed\n");

// Fade out from 100% to 0% over 1 second
err = pwm.FadeStart(0.0f, 1000);
```

### Multiple PWM Channels

```cpp
// Create multiple PWM channels
EspPwm pwm1(GPIO_NUM_2, HF_PWM_CHANNEL_0, HF_PWM_TIMER_0, 1000);
EspPwm pwm2(GPIO_NUM_3, HF_PWM_CHANNEL_1, HF_PWM_TIMER_0, 1000);
EspPwm pwm3(GPIO_NUM_4, HF_PWM_CHANNEL_2, HF_PWM_TIMER_1, 2000);

// Initialize all
if (!pwm1.Initialize() || !pwm2.Initialize() || !pwm3.Initialize()) {
    printf("Failed to initialize PWM channels\n");
    return;
}

// Set different duty cycles
pwm1.SetDutyCycle(25.0f);  // 25%
pwm2.SetDutyCycle(50.0f);  // 50%
pwm3.SetDutyCycle(75.0f);  // 75%

// Start all channels
pwm1.Start();
pwm2.Start();
pwm3.Start();
```

## ESP32-C6 Specific Features

### LEDC Controller

The ESP32-C6 uses the LEDC (LED Control) controller, which provides high-resolution PWM with hardware-accelerated fade effects.

### High Resolution

Support for up to 20-bit resolution for precise control.

### Hardware Fade

Hardware-accelerated fade effects with configurable timing.

### Multiple Timers

Up to 4 independent timers with different frequencies.

## Error Handling

The `EspPwm` class provides comprehensive error handling with specific error codes:

- `HF_PWM_ERR_OK` - Operation successful
- `HF_PWM_ERR_INVALID_ARG` - Invalid parameter
- `HF_PWM_ERR_NOT_INITIALIZED` - PWM not initialized
- `HF_PWM_ERR_INVALID_FREQUENCY` - Invalid frequency
- `HF_PWM_ERR_INVALID_DUTY` - Invalid duty cycle
- `HF_PWM_ERR_CHANNEL_IN_USE` - Channel already in use
- `HF_PWM_ERR_TIMER_IN_USE` - Timer already in use
- `HF_PWM_ERR_FADE_IN_PROGRESS` - Fade operation in progress

## Performance Considerations

- **Resolution vs Frequency**: Higher resolution reduces maximum frequency
- **Timer Sharing**: Multiple channels can share the same timer
- **Fade Timing**: Hardware fade is more efficient than software fade
- **Pin Selection**: Use appropriate pins for PWM output

## Related Documentation

- [BasePwm API Reference](../api/BasePwm.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF LEDC Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/ledc.html) - ESP-IDF documentation