# ⚡ BasePwm API Documentation

## 📋 Overview

The `BasePwm` class is an abstract base class that provides a unified, platform-agnostic interface for PWM (Pulse Width Modulation) generation in the HardFOC system. This class enables precise control of motors, LEDs, servos, and other devices requiring variable power control through consistent timing signals.

## 🏗️ Class Hierarchy

```
BasePwm (Abstract Base Class)
    ├── McuPwm (ESP32 LEDC implementation)
    ├── Pca9685Pwm (External 16-channel PWM IC)
    ├── Tlc5940Pwm (LED driver with PWM)
    └── SfPwm (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **Multi-Channel Support**: Up to 16+ PWM channels depending on implementation
- ✅ **Variable Resolution**: 8-16 bit resolution for precise control
- ✅ **Wide Frequency Range**: 1Hz to 100kHz+ depending on hardware
- ✅ **Motor Control Ready**: Center-aligned PWM and complementary outputs
- ✅ **Hardware Acceleration**: Utilizes dedicated PWM timers when available
- ✅ **Real-time Updates**: Change duty cycle without disrupting the signal
- ✅ **Comprehensive Error Handling**: 24 specific error codes
- ✅ **Callback Support**: Period and duty cycle complete notifications

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `PWM_SUCCESS` | 0 | Operation successful |
| `PWM_ERR_NOT_INITIALIZED` | 2 | PWM not initialized |
| `PWM_ERR_INVALID_CHANNEL` | 7 | Invalid PWM channel number |
| `PWM_ERR_CHANNEL_BUSY` | 8 | Channel already in use |
| `PWM_ERR_INVALID_FREQUENCY` | 11 | Unsupported frequency |
| `PWM_ERR_FREQUENCY_TOO_HIGH` | 12 | Frequency exceeds hardware limits |
| `PWM_ERR_INVALID_DUTY_CYCLE` | 15 | Duty cycle out of range |
| `PWM_ERR_TIMER_CONFLICT` | 18 | Timer resource conflict |
| `PWM_ERR_PIN_CONFLICT` | 19 | GPIO pin already in use |

*See header file for complete list of 24 error codes*

## 🏗️ Data Structures

### hf_pwm_channel_config_t
Configuration structure for PWM channel setup:

```cpp
struct hf_pwm_channel_config_t {
    hf_gpio_num_t output_pin;           // GPIO pin for PWM output
    uint32_t frequency_hz;              // PWM frequency in Hz
    uint8_t resolution_bits;            // PWM resolution (8-16 bits)
    PwmOutputMode output_mode;          // Normal/Inverted/Complementary
    PwmAlignment alignment;             // Edge/Center aligned
    PwmIdleState idle_state;            // Low/High when idle
    float initial_duty_cycle;           // Initial duty cycle (0.0-1.0)
    bool invert_output;                 // Invert the output signal
};
```

### PwmOutputMode
PWM output mode enumeration:

```cpp
enum class PwmOutputMode : uint8_t {
    Normal = 0,        // Normal PWM output
    Inverted = 1,      // Inverted PWM output
    Complementary = 2, // Complementary output (for motor control)
    Differential = 3   // Differential output
};
```

### PwmAlignment
PWM alignment mode for motor control applications:

```cpp
enum class PwmAlignment : uint8_t {
    EdgeAligned = 0,   // Edge-aligned PWM (standard)
    CenterAligned = 1  // Center-aligned PWM (better for motor control)
};
```

### hf_pwm_channel_status_t
Real-time channel status information:

```cpp
struct hf_pwm_channel_status_t {
    bool is_enabled;                    // Channel is enabled
    bool is_running;                    // Channel is actively generating PWM
    uint32_t current_frequency_hz;      // Current frequency
    float current_duty_cycle;           // Current duty cycle (0.0-1.0)
    uint32_t raw_duty_value;            // Raw duty register value
    hf_pwm_err_t last_error;                // Last error encountered
};
```

### hf_pwm_capabilities_t
Hardware capability information:

```cpp
struct hf_pwm_capabilities_t {
    uint8_t max_channels;               // Maximum number of channels
    uint8_t max_timers;                 // Maximum number of timers
    uint32_t min_frequency_hz;          // Minimum supported frequency
    uint32_t max_frequency_hz;          // Maximum supported frequency
    uint8_t min_resolution_bits;        // Minimum resolution
    uint8_t max_resolution_bits;        // Maximum resolution
    bool supports_complementary;        // Supports complementary outputs
    bool supports_center_aligned;      // Supports center-aligned PWM
    bool supports_deadtime;             // Supports deadtime insertion
    bool supports_phase_shift;          // Supports phase shifting
};
```

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes PWM controller on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Yes  

#### `IsInitialized()`
```cpp
bool IsInitialized() const noexcept
```
**Description**: Check if PWM controller is initialized  
**Returns**: `true` if initialized, `false` otherwise  
**Thread-Safe**: Yes  

### Channel Management

#### `ConfigureChannel()`
```cpp
virtual hf_pwm_err_t ConfigureChannel(uint8_t channel, const hf_pwm_channel_config_t& config) noexcept = 0
```
**Description**: Configure a PWM channel with specified settings  
**Parameters**:
- `channel`: Channel number (0-based)
- `config`: Channel configuration structure

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

#### `EnableChannel()`
```cpp
virtual hf_pwm_err_t EnableChannel(uint8_t channel) noexcept = 0
```
**Description**: Enable PWM output on specified channel  
**Parameters**:
- `channel`: Channel number to enable

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

#### `DisableChannel()`
```cpp
virtual hf_pwm_err_t DisableChannel(uint8_t channel) noexcept = 0
```
**Description**: Disable PWM output on specified channel  
**Parameters**:
- `channel`: Channel number to disable

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

### Duty Cycle Control

#### `SetDutyCycle()`
```cpp
virtual hf_pwm_err_t SetDutyCycle(uint8_t channel, float duty_cycle) noexcept = 0
```
**Description**: Set duty cycle for a PWM channel  
**Parameters**:
- `channel`: Channel number
- `duty_cycle`: Duty cycle (0.0 = 0%, 1.0 = 100%)

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

#### `SetDutyCycleRaw()`
```cpp
virtual hf_pwm_err_t SetDutyCycleRaw(uint8_t channel, uint32_t raw_value) noexcept = 0
```
**Description**: Set duty cycle using raw timer value  
**Parameters**:
- `channel`: Channel number
- `raw_value`: Raw duty cycle value (0 to max resolution)

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

#### `GetDutyCycle()`
```cpp
virtual hf_pwm_err_t GetDutyCycle(uint8_t channel, float& duty_cycle) noexcept = 0
```
**Description**: Get current duty cycle for a channel  
**Parameters**:
- `channel`: Channel number
- `duty_cycle`: Reference to store duty cycle value

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Yes  

### Frequency Control

#### `SetFrequency()`
```cpp
virtual hf_pwm_err_t SetFrequency(uint8_t channel, uint32_t frequency_hz) noexcept = 0
```
**Description**: Set PWM frequency for a channel  
**Parameters**:
- `channel`: Channel number
- `frequency_hz`: Frequency in Hz

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

#### `GetFrequency()`
```cpp
virtual hf_pwm_err_t GetFrequency(uint8_t channel, uint32_t& frequency_hz) noexcept = 0
```
**Description**: Get current frequency for a channel  
**Parameters**:
- `channel`: Channel number
- `frequency_hz`: Reference to store frequency value

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Yes  

### Status and Information

#### `GetChannelStatus()`
```cpp
virtual hf_pwm_err_t GetChannelStatus(uint8_t channel, hf_pwm_channel_status_t& status) noexcept = 0
```
**Description**: Get comprehensive channel status information  
**Parameters**:
- `channel`: Channel number
- `status`: Reference to store status information

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Yes  

#### `GetCapabilities()`
```cpp
virtual hf_pwm_err_t GetCapabilities(hf_pwm_capabilities_t& capabilities) noexcept = 0
```
**Description**: Get hardware capability information  
**Parameters**:
- `capabilities`: Reference to store capability information

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Yes  

### Callback Management

#### `SetPeriodCallback()`
```cpp
virtual hf_pwm_err_t SetPeriodCallback(uint8_t channel, hf_pwm_period_callback_t callback) noexcept = 0
```
**Description**: Set callback for PWM period complete events  
**Parameters**:
- `channel`: Channel number
- `callback`: Function to call on period complete

**Returns**: `hf_pwm_err_t` result code  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### Basic PWM Control
```cpp
#include "mcu/McuPwm.h"

// Create PWM instance
auto pwm = McuPwm::Create();

// Configure channel for LED brightness control
hf_pwm_channel_config_t config;
config.output_pin = 18;          // GPIO 18
config.frequency_hz = 1000;      // 1kHz
config.resolution_bits = 12;     // 12-bit resolution (0-4095)
config.initial_duty_cycle = 0.5f; // 50% brightness

// Initialize and configure
if (pwm->EnsureInitialized()) {
    hf_pwm_err_t result = pwm->ConfigureChannel(0, config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
        pwm->EnableChannel(0);
        printf("PWM channel 0 configured successfully\n");
    }
}
```

### Motor Control with Complementary Outputs
```cpp
// Configure for 3-phase motor control
hf_pwm_channel_config_t motor_config;
motor_config.frequency_hz = 20000;           // 20kHz for motor control
motor_config.resolution_bits = 14;           // 14-bit for precise control
motor_config.alignment = PwmAlignment::CenterAligned;
motor_config.output_mode = PwmOutputMode::Complementary;

// Configure 3 phases
for (uint8_t phase = 0; phase < 3; ++phase) {
    motor_config.output_pin = 18 + phase;    // GPIO 18, 19, 20
    pwm->ConfigureChannel(phase, motor_config);
    pwm->EnableChannel(phase);
}

// Set phase duties for rotation
pwm->SetDutyCycle(0, 0.3f);  // Phase A: 30%
pwm->SetDutyCycle(1, 0.6f);  // Phase B: 60%
pwm->SetDutyCycle(2, 0.1f);  // Phase C: 10%
```

### Servo Control
```cpp
// Configure for servo control (50Hz, 1-2ms pulse width)
hf_pwm_channel_config_t servo_config;
servo_config.output_pin = 21;
servo_config.frequency_hz = 50;              // 50Hz (20ms period)
servo_config.resolution_bits = 16;           // High resolution for precise timing
servo_config.initial_duty_cycle = 0.075f;   // 1.5ms pulse (neutral position)

pwm->ConfigureChannel(1, servo_config);
pwm->EnableChannel(1);

// Move servo to different positions
pwm->SetDutyCycle(1, 0.05f);   // 1ms pulse (0 degrees)
vTaskDelay(pdMS_TO_TICKS(1000));
pwm->SetDutyCycle(1, 0.1f);    // 2ms pulse (180 degrees)
```

### Dynamic Frequency and Duty Cycle
```cpp
// Sweep frequency and duty cycle
for (uint32_t freq = 1000; freq <= 10000; freq += 1000) {
    pwm->SetFrequency(0, freq);
    
    for (float duty = 0.1f; duty <= 0.9f; duty += 0.1f) {
        pwm->SetDutyCycle(0, duty);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Monitor status
        hf_pwm_channel_status_t status;
        if (pwm->GetChannelStatus(0, status) == hf_pwm_err_t::PWM_SUCCESS) {
            printf("Freq: %u Hz, Duty: %.1f%%, Raw: %u\n", 
                   status.current_frequency_hz, 
                   status.current_duty_cycle * 100.0f,
                   status.raw_duty_value);
        }
    }
}
```

### Callback-Based Control
```cpp
// Set up period complete callback for precise timing
pwm->SetPeriodCallback(0, [](uint8_t channel, uint32_t period_count) {
    static float duty = 0.0f;
    static bool increasing = true;
    
    // Create breathing effect
    if (increasing) {
        duty += 0.01f;
        if (duty >= 1.0f) {
            duty = 1.0f;
            increasing = false;
        }
    } else {
        duty -= 0.01f;
        if (duty <= 0.0f) {
            duty = 0.0f;
            increasing = true;
        }
    }
    
    // Update duty cycle from callback
    pwm->SetDutyCycle(channel, duty);
});
```

### Hardware Capability Discovery
```cpp
// Discover what the PWM implementation supports
hf_pwm_capabilities_t caps;
if (pwm->GetCapabilities(caps) == hf_pwm_err_t::PWM_SUCCESS) {
    printf("PWM Capabilities:\n");
    printf("  Max Channels: %d\n", caps.max_channels);
    printf("  Max Timers: %d\n", caps.max_timers);
    printf("  Frequency Range: %u - %u Hz\n", caps.min_frequency_hz, caps.max_frequency_hz);
    printf("  Resolution Range: %d - %d bits\n", caps.min_resolution_bits, caps.max_resolution_bits);
    printf("  Supports Complementary: %s\n", caps.supports_complementary ? "Yes" : "No");
    printf("  Supports Center Aligned: %s\n", caps.supports_center_aligned ? "Yes" : "No");
    printf("  Supports Deadtime: %s\n", caps.supports_deadtime ? "Yes" : "No");
}
```

### Error Handling
```cpp
// Comprehensive error handling
hf_pwm_err_t result = pwm->SetDutyCycle(0, 1.5f);  // Invalid duty cycle
if (result != hf_pwm_err_t::PWM_SUCCESS) {
    printf("Error setting duty cycle: %s\n", PwmErrorToString(result));
    
    switch (result) {
        case hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL:
            printf("Channel does not exist\n");
            break;
        case hf_pwm_err_t::PWM_ERR_DUTY_OUT_OF_RANGE:
            printf("Duty cycle must be between 0.0 and 1.0\n");
            break;
        case hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED:
            printf("PWM not initialized - calling EnsureInitialized()\n");
            pwm->EnsureInitialized();
            break;
        default:
            printf("Unexpected error occurred\n");
            break;
    }
}
```

## 🧪 Testing

Unit tests are not provided in this repository.

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **Thread Safety**: Depends on concrete implementation (use SfPwm for thread safety)
3. **Hardware Limitations**: Frequency and resolution are hardware-dependent
4. **Timer Sharing**: Multiple channels may share timers, affecting frequency independence
5. **Pin Conflicts**: Ensure GPIO pins are not used by other peripherals
6. **Real-time Updates**: Duty cycle changes take effect on next PWM period
7. **Motor Control**: Use center-aligned PWM for better motor control performance

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - GPIO interface for PWM pin control
- [`BaseAdc`](BaseAdc.md) - ADC interface for feedback control
- [`McuPwm`](../mcu/McuPwm.md) - ESP32 LEDC implementation
- [`SfPwm`](../thread_safe/SfPwm.md) - Thread-safe wrapper

## 📝 See Also

- [PWM Guide](../guides/pwm-guide.md)
- [Motor Control Example](../examples/motor-control.md)
