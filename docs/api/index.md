# 🚀 HardFOC Interface Wrapper API Reference

<div align="center">

![HardFOC Interface](https://img.shields.io/badge/HardFOC-Interface%20Wrapper-blue?style=for-the-badge&logo=hardware)

**🎯 Unified Hardware Abstraction Layer for Embedded Systems**

*A comprehensive, platform-agnostic interface wrapper for all hardware peripherals*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#️-architecture)
- [📋 **Base Classes**](#-base-classes)
- [🔧 **Core Principles**](#-core-principles)
- [💡 **Getting Started**](#-getting-started)
- [🧪 **Examples**](#-examples)

---

## 🎯 **Overview**

The **HardFOC Interface Wrapper** provides a unified, platform-agnostic abstraction layer for embedded hardware peripherals. It enables developers to write portable, maintainable code that works across different microcontrollers and hardware platforms without modification.

### ✨ **Key Benefits**

- 🔄 **Platform Portability** - Write once, run anywhere
- 🛡️ **Type Safety** - Strongly typed interfaces with comprehensive error handling
- ⚡ **Performance Optimized** - Minimal overhead with direct hardware access
- 🔧 **Extensible** - Easy to add new hardware platforms and peripherals
- 📊 **Observable** - Built-in statistics, diagnostics, and monitoring
- 🧵 **Thread Safe** - Designed for multi-threaded applications

### 🎯 **Target Applications**

- **Motor Control Systems** - FOC, BLDC, stepper motor control
- **Sensor Networks** - Multi-sensor data acquisition and processing
- **Communication Systems** - CAN, UART, I2C, SPI protocols
- **Industrial Automation** - PLC-like control systems
- **IoT Devices** - Connected embedded systems
- **Robotics** - Real-time control and sensing

---

## 🏗️ **Architecture**

### **Design Philosophy**

The HardFOC Interface follows a **layered abstraction pattern**:

```
┌─────────────────────────────────────┐
│           Application Layer         │
├─────────────────────────────────────┤
│         Interface Wrapper           │
├─────────────────────────────────────┤
│        Platform Implementation      │
├─────────────────────────────────────┤
│           Hardware Layer            │
└─────────────────────────────────────┘
```

### **Core Components**

1. **Base Classes** - Abstract interfaces defining the API contract
2. **Platform Implementations** - Concrete implementations for specific hardware
3. **Utility Classes** - Helper classes for common patterns
4. **Type System** - Platform-agnostic type definitions

### **Error Handling Strategy**

- **Comprehensive Error Codes** - Detailed error enumeration for each peripheral
- **String Conversion** - Human-readable error messages
- **Error Recovery** - Built-in recovery mechanisms where possible
- **Diagnostics** - Runtime error tracking and statistics

---

## 📋 **Base Classes**

The HardFOC Interface provides abstract base classes for all major hardware peripherals:

| Class | Purpose | Key Features | Typical Use Cases |
|-------|---------|--------------|-------------------|
| **[BaseAdc](BaseAdc.md)** | Analog-to-Digital Conversion | Multi-channel, calibration, voltage conversion | Sensor reading, current sensing, voltage monitoring |
| **[BaseCan](BaseCan.md)** | CAN Bus Communication | Message filtering, error handling, CAN-FD support | Motor control, vehicle systems, industrial networks |
| **[BaseGpio](BaseGpio.md)** | Digital I/O Control | Dynamic direction, interrupts, polarity control | Status LEDs, switches, relay control, digital sensors |
| **[BaseI2c](BaseI2c.md)** | I2C Bus Communication | Device scanning, register access, clock stretching | EEPROM, sensors, display controllers, RTC modules |
| **[BaseNvs](BaseNvs.md)** | Non-Volatile Storage | Key-value storage, namespaces, encryption | Configuration storage, calibration data, logs |
| **[BasePeriodicTimer](BasePeriodicTimer.md)** | High-Precision Timing | Microsecond resolution, callbacks, period control | Control loops, sampling, event timing |
| **[BasePio](BasePio.md)** | Programmable I/O | Precise timing, symbol transmission, custom protocols | WS2812 LEDs, IR communication, stepper control |
| **[BasePwm](BasePwm.md)** | Pulse Width Modulation | Multi-channel, frequency control, dead-time | Motor control, LED dimming, power conversion |
| **[BaseSpi](BaseSpi.md)** | SPI Bus Communication | Full-duplex transfers, chip select, DMA support | Flash memory, ADCs, display controllers |
| **[BaseUart](BaseUart.md)** | Serial Communication | Flow control, buffering, printf support | Debug output, GPS modules, wireless modules |

### **Quick Start Guide**

```cpp
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/EspGpio.h"

// Create hardware instances
EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);
EspGpio led_pin(2, HF_GPIO_DIRECTION_OUTPUT);

// Initialize (lazy initialization)
adc.EnsureInitialized();
led_pin.EnsureInitialized();

// Use the hardware
float voltage = adc.ReadChannelV(0);
led_pin.SetActive();
```

---

## 🔧 **Core Principles**

### **1. Lazy Initialization**

All peripherals support lazy initialization - hardware is only configured when first used:

```cpp
// Hardware is not initialized until first use
BaseAdc* adc = new EspAdc(ADC_UNIT_1, ADC_ATTEN_DB_11);

// First call automatically initializes the hardware
float voltage = adc->ReadChannelV(0);  // Initialize() called internally
```

### **2. Comprehensive Error Handling**

Every operation returns detailed error codes with string conversion:

```cpp
hf_adc_err_t result = adc->ReadChannelV(0, voltage);
if (result != hf_adc_err_t::ADC_SUCCESS) {
    printf("Error: %s\n", HfAdcErrToString(result));
}
```

### **3. Platform-Agnostic Types**

All types are defined to work across different platforms:

```cpp
// Platform-agnostic types
hf_pin_num_t pin = 2;
hf_frequency_hz_t freq = 1000000;
hf_time_t timeout = 1000;
```

### **4. Statistics and Diagnostics**

Built-in monitoring for all peripherals:

```cpp
hf_adc_statistics_t stats;
adc->GetStatistics(stats);
printf("Total conversions: %u\n", stats.totalConversions);
```

---

## 💡 **Getting Started**

### **1. Include the Headers**

```cpp
// Base classes (abstract interfaces)
#include "base/BaseAdc.h"
#include "base/BaseGpio.h"

// Platform implementations
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/EspGpio.h"
```

### **2. Create Hardware Instances**

```cpp
// Use platform-specific implementations
EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);
EspGpio led_pin(2, HF_GPIO_DIRECTION_OUTPUT);
```

### **3. Initialize and Use**

```cpp
// Lazy initialization (automatic on first use)
adc.EnsureInitialized();
led_pin.EnsureInitialized();

// Use the hardware
float voltage = adc.ReadChannelV(0);
if (voltage > 3.0f) {
    led_pin.SetActive();
}
```

### **4. Error Handling**

```cpp
hf_adc_err_t result = adc.ReadChannelV(0, voltage);
if (result != hf_adc_err_t::ADC_SUCCESS) {
    printf("ADC Error: %s\n", HfAdcErrToString(result));
    // Handle error appropriately
}
```

---

## 🧪 **Examples**

### **Motor Control System**

```cpp
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/EspPwm.h"
#include "mcu/esp32/EspGpio.h"

class MotorController {
private:
    EspAdc current_sensor_;
    EspPwm motor_driver_;
    EspGpio enable_pin_;
    
public:
    MotorController() 
        : current_sensor_(ADC_UNIT_1, ADC_ATTEN_DB_11)
        , motor_driver_()
        , enable_pin_(5, HF_GPIO_DIRECTION_OUTPUT) {}
    
    bool Initialize() {
        current_sensor_.EnsureInitialized();
        motor_driver_.EnsureInitialized();
        enable_pin_.EnsureInitialized();
        
        // Configure motor driver
        motor_driver_.EnableChannel(0);
        motor_driver_.SetFrequency(0, 20000);  // 20kHz PWM
        return true;
    }
    
    void SetSpeed(float speed_percent) {
        motor_driver_.SetDutyCycle(0, speed_percent);
    }
    
    float GetCurrent() {
        float voltage = current_sensor_.ReadChannelV(0);
        return (voltage - 2.5f) / 0.1f;  // Convert to current (A)
    }
};
```

### **Sensor Network**

```cpp
#include "mcu/esp32/EspI2c.h"
#include "mcu/esp32/EspAdc.h"

class SensorNetwork {
private:
    EspI2c i2c_bus_;
    EspAdc analog_sensors_;
    
public:
    bool ScanSensors() {
        uint8_t addresses[16];
        uint8_t count = i2c_bus_.ScanBus(addresses, 16);
        
        printf("Found %u I2C devices:\n", count);
        for (uint8_t i = 0; i < count; i++) {
            printf("  Address: 0x%02X\n", addresses[i]);
        }
        return count > 0;
    }
    
    float ReadTemperature() {
        // Read from I2C temperature sensor
        uint8_t data[2];
        if (i2c_bus_.ReadRegisters(0x48, 0x00, data, 2)) {
            uint16_t raw = (data[0] << 8) | data[1];
            return (raw >> 4) * 0.0625f;  // Convert to Celsius
        }
        return -999.0f;  // Error value
    }
    
    float ReadPressure() {
        return analog_sensors_.ReadChannelV(1) * 100.0f;  // Convert to PSI
    }
};
```

---

## 🔗 **Related Documentation**

- **[Hardware Types](HardwareTypes.md)** - Platform-agnostic type definitions
- **[ESP32 Implementation](../mcu/esp32/)** - ESP32-specific implementations
- **[Examples](../examples/)** - Complete working examples
- **[Contributing Guide](../../CONTRIBUTING.md)** - How to add new platforms

---

<div align="center">

**🚀 HardFOC Interface Wrapper - Powering the Future of Embedded Systems**

*Part of the HardFOC Ecosystem*

</div> 