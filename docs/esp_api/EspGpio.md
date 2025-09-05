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
```text

## Class Definition

```cpp
class EspGpio : public BaseGpio {
public:
    // Constructor with full configuration
    explicit EspGpio(
        hf*pin*num*t pin*num,
        hf*gpio*direction*t direction = hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT,
        hf*gpio*active*state*t active*state = hf*gpio*active*state*t::HF*GPIO*ACTIVE*HIGH,
        hf*gpio*output*mode*t output*mode = hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
        hf*gpio*pull*mode*t pull*mode = hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*FLOATING,
        hf*gpio*drive*cap*t drive*capability = hf*gpio*drive*cap*t::HF*GPIO*DRIVE*CAP*MEDIUM
    ) noexcept;

    // Destructor
    ~EspGpio() override;

    // BaseGpio implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    hf*u8*t GetMaxPins() const noexcept override;
    const char* GetDescription() const noexcept override;

    // Interrupt support
    hf*gpio*err*t SupportsInterrupts() const noexcept override;
    hf*gpio*err*t ConfigureInterrupt(hf*gpio*interrupt*trigger*t trigger,
                                    InterruptCallback callback = nullptr,
                                    void* user*data = nullptr) noexcept override;
    hf*gpio*err*t EnableInterrupt() noexcept override;
    hf*gpio*err*t DisableInterrupt() noexcept override;

    // ESP32-specific advanced features
    hf*gpio*err*t SetDriveCapability(hf*gpio*drive*cap*t capability) noexcept;
    hf*gpio*err*t GetDriveCapability(hf*gpio*drive*cap*t& capability) const noexcept;
    hf*gpio*err*t SetSlewRate(hf*gpio*slew*rate*t slew*rate) noexcept;
    hf*gpio*err*t GetSlewRate(hf*gpio*slew*rate*t& slew*rate) const noexcept;
    
    // GPIO pin mapping
    gpio*num*t GetEspGpioNum() const noexcept;
};
```text

## ESP32-C6 Specific Types

### Drive Capability

```cpp
enum class hf*gpio*drive*cap*t : hf*u8*t {
    HF*GPIO*DRIVE*CAP*WEAK = 0,     // ~5mA drive strength
    HF*GPIO*DRIVE*CAP*MEDIUM = 1,   // ~10mA drive strength (default)
    HF*GPIO*DRIVE*CAP*STRONG = 2,   // ~20mA drive strength
    HF*GPIO*DRIVE*CAP*STRONGEST = 3 // ~40mA drive strength
};
```text

### Slew Rate

```cpp
enum class hf*gpio*slew*rate*t : hf*u8*t {
    HF*GPIO*SLEW*RATE*SLOW = 0,     // Slower edge transitions, less EMI
    HF*GPIO*SLEW*RATE*FAST = 1      // Faster edge transitions, more EMI
};
```text

## Constructor Parameters

### Basic Parameters

- **`pin*num`** - ESP32-C6 GPIO pin number (0-30, depending on package)
- **`direction`** - Initial pin direction (input/output)
- **`active*state`** - Active polarity (high/low)
- **`output*mode`** - Push-pull or open-drain output
- **`pull*mode`** - Pull resistor configuration
- **`drive*capability`** - Output drive strength

## Usage Examples

### Basic LED Control

```cpp
#include "inc/mcu/esp32/EspGpio.h"

// Create output pin for LED (active low, strong drive)
EspGpio led*pin(GPIO*NUM*2, 
               hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT,
               hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
               hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
               hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*FLOATING,
               hf*gpio*drive*cap*t::HF*GPIO*DRIVE*CAP*STRONG);

void setup*led() {
    if (!led*pin.EnsureInitialized()) {
        printf("Failed to initialize LED pin\n");
        return;
    }
    
    // LED starts off
    led*pin.SetInactive();
}

void blink*led() {
    led*pin.SetActive();    // Turn on
    vTaskDelay(pdMS*TO*TICKS(500));
    led*pin.SetInactive();  // Turn off
    vTaskDelay(pdMS*TO*TICKS(500));
}
```text

### Button Input with Interrupt

```cpp
// Global flag for button state
volatile bool button*pressed = false;

// Interrupt callback function
void IRAM*ATTR button*isr*handler(BaseGpio* gpio, hf*gpio*interrupt*trigger*t trigger, void*
user*data) {
    button*pressed = true;  // Set flag for main loop
}

// Create input pin for button
EspGpio button*pin(GPIO*NUM*0,
                  hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT,
                  hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
                  hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                  hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);

void setup*button() {
    // Initialize pin
    if (!button*pin.EnsureInitialized()) {
        printf("Failed to initialize button pin\n");
        return;
    }
    
    // Configure interrupt
    hf*gpio*err*t result = button*pin.ConfigureInterrupt(
        hf*gpio*interrupt*trigger*t::HF*GPIO*INTERRUPT*TRIGGER*FALLING*EDGE,
        button*isr*handler,
        nullptr
    );
    
    if (result == hf*gpio*err*t::GPIO*SUCCESS) {
        button*pin.EnableInterrupt();
        printf("Button interrupt configured\n");
    } else {
        printf("Failed to configure button interrupt: %s\n", HfGpioErrToString(result));
    }
}

void check*button() {
    if (button*pressed) {
        button*pressed = false;  // Clear flag
        printf("Button was pressed!\n");
        
        // Debouncing - check if still pressed after delay
        vTaskDelay(pdMS*TO*TICKS(50));
        if (button*pin.IsActive()) {
            printf("Button press confirmed\n");
            // Handle button press action
        }
    }
}
```text

### High-Performance Digital Output

```cpp
// High-speed digital output for motor control
EspGpio motor*step*pin(GPIO*NUM*4,
                      hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT,
                      hf*gpio*active*state*t::HF*GPIO*ACTIVE*HIGH,
                      hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                      hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*FLOATING,
                      hf*gpio*drive*cap*t::HF*GPIO*DRIVE*CAP*STRONGEST);

void setup*motor*control() {
    motor*step*pin.EnsureInitialized();
    
    // Configure for fastest switching
    motor*step*pin.SetSlewRate(hf*gpio*slew*rate*t::HF*GPIO*SLEW*RATE*FAST);
    motor*step*pin.SetDriveCapability(hf*gpio*drive*cap*t::HF*GPIO*DRIVE*CAP*STRONGEST);
}

void generate*step*pulses(int num*steps, int delay*us) {
    for (int i = 0; i < num*steps; i++) {
        motor*step*pin.SetActive();
        esp*rom*delay*us(delay*us);
        motor*step*pin.SetInactive();
        esp*rom*delay*us(delay*us);
    }
}
```text

### Open-Drain Communication Bus

```cpp
// I2C-like open-drain communication
EspGpio sda*pin(GPIO*NUM*21,
               hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT,
               hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
               hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*OPEN*DRAIN,
               hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);

EspGpio scl*pin(GPIO*NUM*22,
               hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT,
               hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
               hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*OPEN*DRAIN,
               hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);

void setup*open*drain*bus() {
    sda*pin.EnsureInitialized();
    scl*pin.EnsureInitialized();
    
    // Set both lines high (pulled up externally)
    sda*pin.SetInactive();  // Release SDA (high via pull-up)
    scl*pin.SetInactive();  // Release SCL (high via pull-up)
}

void send*start*condition() {
    // I2C start condition: SDA low while SCL high
    scl*pin.SetInactive();  // Ensure SCL high
    vTaskDelay(pdMS*TO*TICKS(1));
    sda*pin.SetActive();    // Pull SDA low
    vTaskDelay(pdMS*TO*TICKS(1));
    scl*pin.SetActive();    // Pull SCL low
}
```text

### Multi-Pin Control System

```cpp
class MotorControlSystem {
private:
    EspGpio enable*pin*;
    EspGpio direction*pin*;
    EspGpio step*pin*;
    EspGpio limit*switch*;
    
public:
    MotorControlSystem() 
        : enable*pin*(GPIO*NUM*2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT)
        , direction*pin*(GPIO*NUM*3, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT)
        , step*pin*(GPIO*NUM*4, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT, 
                   hf*gpio*active*state*t::HF*GPIO*ACTIVE*HIGH,
                   hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                   hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*FLOATING,
                   hf*gpio*drive*cap*t::HF*GPIO*DRIVE*CAP*STRONGEST)
        , limit*switch*(GPIO*NUM*5, hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT,
                       hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
                       hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                       hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP)
    {}
    
    bool initialize() {
        bool success = true;
        success &= enable*pin*.EnsureInitialized();
        success &= direction*pin*.EnsureInitialized();
        success &= step*pin*.EnsureInitialized();
        success &= limit*switch*.EnsureInitialized();
        
        if (success) {
            // Set safe initial state
            enable*pin*.SetInactive();    // Motor disabled
            direction*pin*.SetInactive(); // Direction = forward
            step*pin*.SetInactive();      // No step
        }
        
        return success;
    }
    
    void move*motor(int steps, bool forward, int speed*hz) {
        if (limit*switch*.IsActive()) {
            printf("Limit switch activated - stopping\n");
            return;
        }
        
        direction*pin*.SetState(forward ? hf*gpio*state*t::HF*GPIO*STATE*INACTIVE 
                                       : hf*gpio*state*t::HF*GPIO*STATE*ACTIVE);
        enable*pin*.SetActive();  // Enable motor
        
        int delay*us = 500000 / speed*hz;  // Convert Hz to microseconds
        
        for (int i = 0; i < steps && !limit*switch*.IsActive(); i++) {
            step*pin*.SetActive();
            esp*rom*delay*us(delay*us);
            step*pin*.SetInactive();
            esp*rom*delay*us(delay*us);
        }
        
        enable*pin*.SetInactive();  // Disable motor
    }
    
    void emergency*stop() {
        enable*pin*.SetInactive();  // Immediately disable motor
        printf("Emergency stop activated\n");
    }
};
```text

## ESP32-C6 Pin Mapping

### Available GPIO Pins

| Pin Number | Special Functions | Notes |

|------------|------------------|-------|

| GPIO0 | Boot mode, UART download | Pull-up recommended for normal operation |

| GPIO1 | ADC1*CH0, UART0*TXD | Can be used as GPIO after UART disable |

| GPIO2 | ADC1*CH1, FSPIQ | Available for general GPIO |

| GPIO3 | ADC1*CH2, FSPIHD | Available for general GPIO |

| GPIO4 | ADC1*CH3, FSPICS0 | Available for general GPIO |

| GPIO5 | ADC1*CH4, FSPIWP | Available for general GPIO |

| GPIO6 | ADC1*CH5, FSPICLK | Available for general GPIO |

| GPIO7 | ADC1*CH6, FSPID | Available for general GPIO |

| GPIO8-19 | General GPIO | Fully available for GPIO operations |

| GPIO20 | UART0*RXD | Can be used as GPIO after UART disable |

| GPIO21 | UART1*TXD | Available for general GPIO |

| GPIO22 | UART1*RXD | Available for general GPIO |

| GPIO23 | USB*D+ | Reserved for USB functionality |

| GPIO24 | USB*D- | Reserved for USB functionality |

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
EspGpio wake*pin(GPIO*NUM*0, hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT);
wake*pin.EnsureInitialized();

// Configure as wake source
esp*sleep*enable*ext0*wakeup(GPIO*NUM*0, 0);  // Wake on low level
```text

### Performance Optimization

```cpp
// For time-critical applications, cache the ESP GPIO number
gpio*num*t esp*pin = gpio*pin.GetEspGpioNum();

// Use direct ESP-IDF calls for maximum speed (if needed)
gpio*set*level(esp*pin, 1);  // Direct register access
```text

## Error Handling

All `EspGpio` methods return appropriate error codes from the `hf*gpio*err*t` enumeration.
Common ESP32-specific errors include:

- `GPIO*ERR*INVALID*PIN` - Pin number not available on ESP32-C6
- `GPIO*ERR*PIN*NOT*AVAILABLE` - Pin reserved for special functions
- `GPIO*ERR*HARDWARE*FAILURE` - ESP-IDF driver error
- `GPIO*ERR*INTERRUPT*NOT*SUPPORTED` - Interrupt configuration failed

## Thread Safety

`EspGpio` is **not thread-safe**.
Use appropriate synchronization when accessing from multiple tasks.

## Implementation Notes

- **Lazy Initialization**: Hardware configuration occurs only when `Initialize()` or `EnsureInitialized()` is called
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