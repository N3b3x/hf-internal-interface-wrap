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
- [🔧 **ESP32 Implementations**](#-esp32-implementations)
- [🎯 **Type System**](#-type-system)
- [📊 **Getting Started**](#-getting-started)
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
| **[BaseGpio](BaseGpio.md)** | Digital I/O Control | Dynamic direction, interrupts, polarity control | Status LEDs, switches, relay control, digital sensors |
| **BaseI2c** | I2C Bus Communication | Device scanning, register access, clock stretching | EEPROM, sensors, display controllers, RTC modules |
| **BaseNvs** | Non-Volatile Storage | Key-value storage, namespaces, encryption | Configuration storage, calibration data, logs |
| **BasePeriodicTimer** | High-Precision Timing | Microsecond resolution, callbacks, period control | Control loops, sampling, event timing |
| **BasePio** | Programmable I/O | Precise timing, symbol transmission, custom protocols | WS2812 LEDs, IR communication, stepper control |
| **BasePwm** | Pulse Width Modulation | Multi-channel, frequency control, dead-time | Motor control, LED dimming, power conversion |
| **BaseSpi** | SPI Bus Communication | Full-duplex transfers, chip select, DMA support | Flash memory, ADCs, display controllers |
| **BaseUart** | Serial Communication | Flow control, buffering, printf support | Debug output, GPS modules, wireless modules |
| **BaseCan** | CAN Bus Communication | Message filtering, error handling, CAN-FD support | Motor control, vehicle systems, industrial networks |
| **BaseWifi** | WiFi Communication | Station/AP modes, security, mesh networking | IoT connectivity, remote monitoring |
| **BaseBluetooth** | Bluetooth Communication | Classic & BLE, pairing, service discovery | Mobile apps, wireless sensors |
| **BaseTemperature** | Temperature Sensing | Multi-sensor support, calibration, thermal protection | System monitoring, safety protection |
| **BaseLogger** | System Logging | Multi-level logging, multiple outputs | Debugging, diagnostics, system monitoring |

---

## 🔧 **ESP32 Implementations**

ESP32-C6 specific implementations with optimized features:

| Implementation | Base Class | ESP32-C6 Features | Documentation |
|----------------|------------|-------------------|---------------|
| **[EspGpio](EspGpio.md)** | BaseGpio | Drive strength, slew rate, interrupts | ✅ Complete |
| **EspAdc** | BaseAdc | 12-bit resolution, multiple units | ✅ Available |
| **EspPwm** | BasePwm | LEDC controller, fade effects | 📝 In Progress |
| **EspI2c** | BaseI2c | Clock stretching, multi-master | 📝 In Progress |
| **EspSpi** | BaseSpi | Full-duplex, DMA support | 📝 In Progress |
| **EspUart** | BaseUart | Hardware flow control | 📝 In Progress |
| **EspCan** | BaseCan | TWAI controller | 📝 In Progress |
| **EspWifi** | BaseWifi | 802.11n, WPA3, mesh | 📝 In Progress |
| **EspBluetooth** | BaseBluetooth | Classic & BLE support | 📝 In Progress |
| **EspTemperature** | BaseTemperature | Internal sensor, calibration | 📝 In Progress |
| **EspLogger** | BaseLogger | UART, network, file output | 📝 In Progress |

---

## 🎯 **Type System**

Platform-agnostic type definitions for consistent APIs:

| Documentation | Description | Status |
|---------------|-------------|--------|
| **[HardwareTypes](HardwareTypes.md)** | Core type definitions, validation functions | ✅ Complete |

### **Core Types**

```cpp
// Integer types
using hf_u8_t = uint8_t;
using hf_u16_t = uint16_t;
using hf_u32_t = uint32_t;
using hf_u64_t = uint64_t;

// Hardware types
using hf_pin_num_t = hf_i32_t;
using hf_channel_id_t = hf_u32_t;
using hf_frequency_hz_t = hf_u32_t;
using hf_time_t = hf_u32_t;
```

---

## 📊 **Getting Started**

### **1. Include the Headers**

```cpp
// Base classes (abstract interfaces)
#include "inc/base/BaseAdc.h"
#include "inc/base/BaseGpio.h"

// Platform implementations
#include "inc/mcu/esp32/EspAdc.h"
#include "inc/mcu/esp32/EspGpio.h"
```

### **2. Create Hardware Instances**

```cpp
// Use platform-specific implementations
EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);
EspGpio led_pin(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
```

### **3. Initialize and Use**

```cpp
// Lazy initialization (automatic on first use)
adc.EnsureInitialized();
led_pin.EnsureInitialized();

// Use the hardware
float voltage;
if (adc.ReadChannelV(0, voltage) == hf_adc_err_t::ADC_SUCCESS) {
    printf("Voltage: %.3f V\n", voltage);
}

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
#include "inc/mcu/esp32/EspAdc.h"
#include "inc/mcu/esp32/EspPwm.h"
#include "inc/mcu/esp32/EspGpio.h"

class MotorController {
private:
    EspAdc current_sensor_;
    EspPwm motor_driver_;
    EspGpio enable_pin_;
    
public:
    MotorController() 
        : current_sensor_(ADC_UNIT_1, ADC_ATTEN_DB_11)
        , motor_driver_()
        , enable_pin_(5, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT) {}
    
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
        float voltage;
        current_sensor_.ReadChannelV(0, voltage);
        return (voltage - 2.5f) / 0.1f;  // Convert to current (A)
    }
};
```

### **Sensor Network**

```cpp
#include "inc/mcu/esp32/EspI2c.h"
#include "inc/mcu/esp32/EspAdc.h"

class SensorNetwork {
private:
    EspI2c i2c_bus_;
    EspAdc analog_sensors_;
    
public:
    bool ScanSensors() {
        hf_u8_t addresses[16];
        hf_u8_t count = i2c_bus_.ScanBus(addresses, 16);
        
        printf("Found %u I2C devices:\n", count);
        for (hf_u8_t i = 0; i < count; i++) {
            printf("  Address: 0x%02X\n", addresses[i]);
        }
        return count > 0;
    }
    
    float ReadTemperature() {
        // Read from I2C temperature sensor
        hf_u8_t data[2];
        if (i2c_bus_.ReadRegisters(0x48, 0x00, data, 2)) {
            hf_u16_t raw = (data[0] << 8) | data[1];
            return (raw >> 4) * 0.0625f;  // Convert to Celsius
        }
        return -999.0f;  // Error value
    }
    
    float ReadPressure() {
        float voltage;
        analog_sensors_.ReadChannelV(1, voltage);
        return voltage * 100.0f;  // Convert to PSI
    }
};
```

---

## 🔗 **Related Documentation**

- **[Main Documentation](../index.md)** - Complete system overview
- **[Contributing Guidelines](../../CONTRIBUTING.md)** - How to contribute

---

<div align="center">

**🚀 HardFOC Interface Wrapper - Powering the Future of Embedded Systems**

*Part of the HardFOC Ecosystem*

</div> 