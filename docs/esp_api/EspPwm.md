# EspPwm API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspAdc](EspAdc.md) | [Back to ESP API Index](README.md) | [Next: EspI2c →](EspI2c.md)

</div>

---

## Overview

`EspPwm` is the ESP32-C6 implementation of the `BasePwm` interface,
providing comprehensive PWM (Pulse Width Modulation) functionality specifically optimized for
ESP32-C6 microcontrollers running ESP-IDF v5.5+.
It offers both basic and advanced PWM features with hardware-specific optimizations.

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
```text

## Class Definition

```cpp
class EspPwm : public BasePwm {
public:
    // Constructor with full configuration
    explicit EspPwm(
        hf*pin*num*t pin,
        hf*pwm*channel*t channel = hf*pwm*channel*t::HF*PWM*CHANNEL*0,
        hf*pwm*timer*t timer = hf*pwm*timer*t::HF*PWM*TIMER*0,
        hf*pwm*freq*t frequency = 1000,
        hf*pwm*resolution*t resolution = hf*pwm*resolution*t::HF*PWM*RESOLUTION*8*BIT
    ) noexcept;

    // Destructor
    ~EspPwm() override;

    // BasePwm implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // PWM operations
    hf*pwm*err*t SetFrequency(hf*pwm*freq*t frequency) noexcept override;
    hf*pwm*err*t GetFrequency(hf*pwm*freq*t* frequency) const noexcept override;
    hf*pwm*err*t SetDutyCycle(hf*pwm*duty*t duty*cycle) noexcept override;
    hf*pwm*err*t GetDutyCycle(hf*pwm*duty*t* duty*cycle) const noexcept override;
    hf*pwm*err*t Start() noexcept override;
    hf*pwm*err*t Stop() noexcept override;
    hf*pwm*err*t IsRunning(bool* running) const noexcept override;

    // Advanced features
    hf*pwm*err*t SetResolution(hf*pwm*resolution*t resolution) noexcept override;
    hf*pwm*err*t GetResolution(hf*pwm*resolution*t* resolution) const noexcept override;
    hf*pwm*err*t FadeStart(hf*pwm*duty*t target*duty, hf*u32*t fade*time*ms) noexcept override;
    hf*pwm*err*t FadeStop() noexcept override;
    hf*pwm*err*t IsFading(bool* fading) const noexcept override;
};
```text

## Usage Examples

### Basic PWM Control

```cpp
#include "inc/mcu/esp32/EspPwm.h"

// Create PWM instance
EspPwm pwm(GPIO*NUM*2, HF*PWM*CHANNEL*0, HF*PWM*TIMER*0, 1000, HF*PWM*RESOLUTION*8*BIT);

// Initialize
if (!pwm.Initialize()) {
    printf("Failed to initialize PWM\n");
    return;
}

// Set duty cycle (0-100%)
hf*pwm*err*t err = pwm.SetDutyCycle(50.0f); // 50% duty cycle
if (err != HF*PWM*ERR*OK) {
    printf("Failed to set duty cycle: %d\n", err);
    return;
}

// Start PWM
err = pwm.Start();
if (err != HF*PWM*ERR*OK) {
    printf("Failed to start PWM: %d\n", err);
    return;
}

printf("PWM started with 50%% duty cycle\n");
```text

### Frequency Control

```cpp
// Set different frequencies
hf*pwm*freq*t frequencies[] = {100, 500, 1000, 5000, 10000};
for (int i = 0; i < 5; i++) {
    hf*pwm*err*t err = pwm.SetFrequency(frequencies[i]);
    if (err == HF*PWM*ERR*OK) {
        printf("PWM frequency set to %d Hz\n", frequencies[i]);
        vTaskDelay(pdMS*TO*TICKS(1000)); // Wait 1 second
    }
}
```text

### Fade Effects

```cpp
// Fade in from 0% to 100% over 2 seconds
hf*pwm*err*t err = pwm.FadeStart(100.0f, 2000);
if (err != HF*PWM*ERR*OK) {
    printf("Failed to start fade: %d\n", err);
    return;
}

// Wait for fade to complete
bool fading = true;
while (fading) {
    err = pwm.IsFading(&fading);
    if (err != HF*PWM*ERR*OK) {
        break;
    }
    vTaskDelay(pdMS*TO*TICKS(100));
}

printf("Fade completed\n");

// Fade out from 100% to 0% over 1 second
err = pwm.FadeStart(0.0f, 1000);
```text

### Multiple PWM Channels

```cpp
// Create multiple PWM channels
EspPwm pwm1(GPIO*NUM*2, HF*PWM*CHANNEL*0, HF*PWM*TIMER*0, 1000);
EspPwm pwm2(GPIO*NUM*3, HF*PWM*CHANNEL*1, HF*PWM*TIMER*0, 1000);
EspPwm pwm3(GPIO*NUM*4, HF*PWM*CHANNEL*2, HF*PWM*TIMER*1, 2000);

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
```text

## ESP32-C6 Specific Features

### LEDC Controller

The ESP32-C6 uses the LEDC (LED Control) controller, which provides high-resolution PWM with
hardware-accelerated fade effects.

### High Resolution

Support for up to 20-bit resolution for precise control.

### Hardware Fade

Hardware-accelerated fade effects with configurable timing.

### Multiple Timers

Up to 4 independent timers with different frequencies.

## Error Handling

The `EspPwm` class provides comprehensive error handling with specific error codes:

- `HF*PWM*ERR*OK` - Operation successful
- `HF*PWM*ERR*INVALID*ARG` - Invalid parameter
- `HF*PWM*ERR*NOT*INITIALIZED` - PWM not initialized
- `HF*PWM*ERR*INVALID*FREQUENCY` - Invalid frequency
- `HF*PWM*ERR*INVALID*DUTY` - Invalid duty cycle
- `HF*PWM*ERR*CHANNEL*IN*USE` - Channel already in use
- `HF*PWM*ERR*TIMER*IN*USE` - Timer already in use
- `HF*PWM*ERR*FADE*IN_PROGRESS` - Fade operation in progress

## Performance Considerations

- **Resolution vs Frequency**: Higher resolution reduces maximum frequency
- **Timer Sharing**: Multiple channels can share the same timer
- **Fade Timing**: Hardware fade is more efficient than software fade
- **Pin Selection**: Use appropriate pins for PWM output

## Related Documentation

- [BasePwm API Reference](../api/BasePwm.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF LEDC Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/ledc.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspAdc](EspAdc.md) | [Back to ESP API Index](README.md) | [Next: EspI2c →](EspI2c.md)

</div>