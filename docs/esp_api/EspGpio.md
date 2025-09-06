# EspGpio API Reference

<div align="center">

**📋 Navigation**

[← Previous: ESP API Index](README.md) | [Back to ESP API Index](README.md) | [Next: EspAdc
→](EspAdc.md)

</div>

---

## Overview

`EspGpio` is the ESP32-C6 implementation of the `BaseGpio` interface,
providing comprehensive GPIO functionality specifically optimized for ESP32-C6 microcontrollers
running ESP-IDF v5.5+.
It offers both basic and advanced GPIO features with hardware-specific optimizations.

## Features

- **ESP32-C6 Optimized** - Full support for ESP32-C6 GPIO capabilities
- **Advanced Pin Configuration** - Drive strength, slew rate, schmitt trigger control
- **Hardware Interrupts** - Edge and level triggered interrupts with ISR handling
- **Power Management** - Deep sleep compatibility and RTC GPIO support
- **Glitch Filtering** - Hardware-based input glitch filtering
- **Open-Drain Support** - True open-drain output with configurable pull-ups
- **Performance Optimized** - Direct register access for critical operations

## Header File

```cpp
#include "inc/mcu/esp32/EspGpio.h"
```

## Class Definition

```cpp
class EspGpio : public BaseGpio {
public:
    // Constructor with full configuration
    explicit EspGpio(
        hf_pin_num_t pin_num,
        hf_gpio_direction_t direction = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
        hf_gpio_active_state_t active_state = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
        hf_gpio_output_mode_t output_mode = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
        hf_gpio_pull_mode_t pull_mode = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
        hf_gpio_drive_cap_t drive_capability = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM
    ) noexcept;

    // Destructor
    ~EspGpio() override;

    // BaseGpio implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    hf_u8_t GetMaxPins() const noexcept override;
    const char* GetDescription() const noexcept override;

    // Interrupt support
    hf_gpio_err_t SupportsInterrupts() const noexcept override;
    hf_gpio_err_t ConfigureInterrupt(hf_gpio_interrupt_trigger_t trigger,
                                    InterruptCallback callback = nullptr,
                                    void* user_data = nullptr) noexcept override;
    hf_gpio_err_t EnableInterrupt() noexcept override;
    hf_gpio_err_t DisableInterrupt() noexcept override;

    // ESP32-specific advanced features
    hf_gpio_err_t SetDriveCapability(hf_gpio_drive_cap_t capability) noexcept;
    hf_gpio_err_t GetDriveCapability(hf_gpio_drive_cap_t& capability) const noexcept;
    hf_gpio_err_t SetSlewRate(hf_gpio_slew_rate_t slew_rate) noexcept;
    hf_gpio_err_t GetSlewRate(hf_gpio_slew_rate_t& slew_rate) const noexcept;
    
    // GPIO pin mapping
    gpio_num_t GetEspGpioNum() const noexcept;
};
```

## ESP32-C6 Specific Types

### Drive Capability

```cpp
enum class hf_gpio_drive_cap_t : hf_u8_t {
    HF_GPIO_DRIVE_CAP_WEAK = 0,     // ~5mA drive strength
    HF_GPIO_DRIVE_CAP_MEDIUM = 1,   // ~10mA drive strength (default)
    HF_GPIO_DRIVE_CAP_STRONG = 2,   // ~20mA drive strength
    HF_GPIO_DRIVE_CAP_STRONGEST = 3 // ~40mA drive strength
};
```

### Slew Rate

```cpp
enum class hf_gpio_slew_rate_t : hf_u8_t {
    HF_GPIO_SLEW_RATE_SLOW = 0,     // Slower edge transitions, less EMI
    HF_GPIO_SLEW_RATE_FAST = 1      // Faster edge transitions, more EMI
};
```

## Constructor Parameters

### Basic Parameters

- **`pin_num`** - ESP32-C6 GPIO pin number (0-30, depending on package)
- **`direction`** - Initial pin direction (input/output)
- **`active_state`** - Active polarity (high/low)
- **`output_mode`** - Push-pull or open-drain output
- **`pull_mode`** - Pull resistor configuration
- **`drive_capability`** - Output drive strength

## Usage Examples

### Basic LED Control

```cpp
#include "inc/mcu/esp32/EspGpio.h"

// Create output pin for LED (active low, strong drive)
EspGpio led_pin(GPIO_NUM_2, 
               hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
               hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
               hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
               hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
               hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONG);

void setup_led() {
    if (!led_pin.EnsureInitialized()) {
        printf("Failed to initialize LED pin\n");
        return;
    }
    
    // LED starts off
    led_pin.SetInactive();
}

void blink_led() {
    led_pin.SetActive();    // Turn on
    vTaskDelay(pdMS_TO_TICKS(500));
    led_pin.SetInactive();  // Turn off
    vTaskDelay(pdMS_TO_TICKS(500));
}
```

### Button Input with Interrupt

```cpp
// Global flag for button state
volatile bool button_pressed = false;

// Interrupt callback function
void IRAM_ATTR button_isr_handler(BaseGpio* gpio, hf_gpio_interrupt_trigger_t trigger, void*
user_data) {
    button_pressed = true;  // Set flag for main loop
}

// Create input pin for button
EspGpio button_pin(GPIO_NUM_0,
                  hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                  hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
                  hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                  hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP);

void setup_button() {
    // Initialize pin
    if (!button_pin.EnsureInitialized()) {
        printf("Failed to initialize button pin\n");
        return;
    }
    
    // Configure interrupt
    hf_gpio_err_t result = button_pin.ConfigureInterrupt(
        hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE,
        button_isr_handler,
        nullptr
    );
    
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
        button_pin.EnableInterrupt();
        printf("Button interrupt configured\n");
    } else {
        printf("Failed to configure button interrupt: %s\n", HfGpioErrToString(result));
    }
}

void check_button() {
    if (button_pressed) {
        button_pressed = false;  // Clear flag
        printf("Button was pressed!\n");
        
        // Debouncing - check if still pressed after delay
        vTaskDelay(pdMS_TO_TICKS(50));
        if (button_pin.IsActive()) {
            printf("Button press confirmed\n");
            // Handle button press action
        }
    }
}
```

### High-Performance Digital Output

```cpp
// High-speed digital output for motor control
EspGpio motor_step_pin(GPIO_NUM_4,
                      hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST);

void setup_motor_control() {
    motor_step_pin.EnsureInitialized();
    
    // Configure for fastest switching
    motor_step_pin.SetSlewRate(hf_gpio_slew_rate_t::HF_GPIO_SLEW_RATE_FAST);
    motor_step_pin.SetDriveCapability(hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST);
}

void generate_step_pulses(int num_steps, int delay_us) {
    for (int i = 0; i < num_steps; i++) {
        motor_step_pin.SetActive();
        esp_rom_delay_us(delay_us);
        motor_step_pin.SetInactive();
        esp_rom_delay_us(delay_us);
    }
}
```

### Open-Drain Communication Bus

```cpp
// I2C-like open-drain communication
EspGpio sda_pin(GPIO_NUM_21,
               hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
               hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
               hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN,
               hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP);

EspGpio scl_pin(GPIO_NUM_22,
               hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
               hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
               hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN,
               hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP);

void setup_open_drain_bus() {
    sda_pin.EnsureInitialized();
    scl_pin.EnsureInitialized();
    
    // Set both lines high (pulled up externally)
    sda_pin.SetInactive();  // Release SDA (high via pull-up)
    scl_pin.SetInactive();  // Release SCL (high via pull-up)
}

void send_start_condition() {
    // I2C start condition: SDA low while SCL high
    scl_pin.SetInactive();  // Ensure SCL high
    vTaskDelay(pdMS_TO_TICKS(1));
    sda_pin.SetActive();    // Pull SDA low
    vTaskDelay(pdMS_TO_TICKS(1));
    scl_pin.SetActive();    // Pull SCL low
}
```

### Multi-Pin Control System

```cpp
class MotorControlSystem {
private:
    EspGpio enable_pin*;
    EspGpio direction_pin*;
    EspGpio step_pin*;
    EspGpio limit_switch*;
    
public:
    MotorControlSystem() 
        : enable_pin*(GPIO_NUM_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT)
        , direction_pin*(GPIO_NUM_3, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT)
        , step_pin*(GPIO_NUM_4, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT, 
                   hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                   hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                   hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                   hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST)
        , limit_switch*(GPIO_NUM_5, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                       hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
                       hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                       hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP)
    {}
    
    bool initialize() {
        bool success = true;
        success &= enable_pin*.EnsureInitialized();
        success &= direction_pin*.EnsureInitialized();
        success &= step_pin*.EnsureInitialized();
        success &= limit_switch*.EnsureInitialized();
        
        if (success) {
            // Set safe initial state
            enable_pin*.SetInactive();    // Motor disabled
            direction_pin*.SetInactive(); // Direction = forward
            step_pin*.SetInactive();      // No step
        }
        
        return success;
    }
    
    void move_motor(int steps, bool forward, int speed_hz) {
        if (limit_switch*.IsActive()) {
            printf("Limit switch activated - stopping\n");
            return;
        }
        
        direction_pin*.SetState(forward ? hf_gpio_state_t::HF_GPIO_STATE_INACTIVE 
                                       : hf_gpio_state_t::HF_GPIO_STATE_ACTIVE);
        enable_pin*.SetActive();  // Enable motor
        
        int delay_us = 500000 / speed_hz;  // Convert Hz to microseconds
        
        for (int i = 0; i < steps && !limit_switch*.IsActive(); i++) {
            step_pin*.SetActive();
            esp_rom_delay_us(delay_us);
            step_pin*.SetInactive();
            esp_rom_delay_us(delay_us);
        }
        
        enable_pin*.SetInactive();  // Disable motor
    }
    
    void emergency_stop() {
        enable_pin*.SetInactive();  // Immediately disable motor
        printf("Emergency stop activated\n");
    }
};
```

## ESP32-C6 Pin Mapping

### Available GPIO Pins

| Pin Number | Special Functions | Notes |

|------------|------------------|-------|

| GPIO0 | Boot mode, UART download | Pull-up recommended for normal operation |

| GPIO1 | ADC1_CH0, UART0_TXD | Can be used as GPIO after UART disable |

| GPIO2 | ADC1_CH1, FSPIQ | Available for general GPIO |

| GPIO3 | ADC1_CH2, FSPIHD | Available for general GPIO |

| GPIO4 | ADC1_CH3, FSPICS0 | Available for general GPIO |

| GPIO5 | ADC1_CH4, FSPIWP | Available for general GPIO |

| GPIO6 | ADC1_CH5, FSPICLK | Available for general GPIO |

| GPIO7 | ADC1_CH6, FSPID | Available for general GPIO |

| GPIO8-19 | General GPIO | Fully available for GPIO operations |

| GPIO20 | UART0_RXD | Can be used as GPIO after UART disable |

| GPIO21 | UART1_TXD | Available for general GPIO |

| GPIO22 | UART1_RXD | Available for general GPIO |

| GPIO23 | USB_D+ | Reserved for USB functionality |

| GPIO24 | USB_D- | Reserved for USB functionality |

| GPIO25-30 | General GPIO | Available (package dependent) |

### Drive Strength Guidelines

| Application | Recommended Drive Strength | Notes |

|-------------|---------------------------|-------|

| LEDs | STRONG or STRONGEST | Higher current needed |

| Logic Signals | MEDIUM | Balanced performance/EMI |

| I2C/SPI | MEDIUM | Standard digital communication |

| High-Speed Signals | STRONGEST + FAST slew | Maximum performance |

| Low-EMI Applications | WEAK + SLOW slew | Minimized electromagnetic interference |

## Advanced Features

### Power Management

```cpp
// Configure pin for deep sleep compatibility
EspGpio wake_pin(GPIO_NUM_0, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);
wake_pin.EnsureInitialized();

// Configure as wake source
esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // Wake on low level
```

### Performance Optimization

```cpp
// For time-critical applications, cache the ESP GPIO number
gpio_num_t esp_pin = gpio_pin.GetEspGpioNum();

// Use direct ESP-IDF calls for maximum speed (if needed)
gpio_set_level(esp_pin, 1);  // Direct register access
```

## Error Handling

All `EspGpio` methods return appropriate error codes from the `hf_gpio_err_t` enumeration.
Common ESP32-specific errors include:

- `GPIO_ERR_INVALID_PIN` - Pin number not available on ESP32-C6
- `GPIO_ERR_PIN_NOT_AVAILABLE` - Pin reserved for special functions
- `GPIO_ERR_HARDWARE_FAILURE` - ESP-IDF driver error
- `GPIO_ERR_INTERRUPT_NOT_SUPPORTED` - Interrupt configuration failed

## Thread Safety

`EspGpio` is **not thread-safe**.
Use appropriate synchronization when accessing from multiple tasks.

## Implementation Notes

- **Lazy Initialization**: Hardware configuration occurs only when `Initialize()` or `EnsureInitialized()` is called, `(empty constructors)
- **ISR Compatibility**: Interrupt callbacks must be marked with `IRAM_ATTR` for proper execution
- **Memory Efficiency**: Minimal RAM usage with static configuration
- **Performance**: Optimized for real-time motor control applications

## Related Documentation

- [BaseGpio API Reference](../api/BaseGpio.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF GPIO Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/gpio.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: ESP API Index](README.md) | [Back to ESP API Index](README.md) | [Next: EspAdc
→](EspAdc.md)

</div>