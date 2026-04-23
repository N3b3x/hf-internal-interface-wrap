---
layout: default
title: "📋 API Reference"
description: "Complete API reference for the HardFOC Interface Wrapper base classes and abstract interfaces"
nav_order: 0
parent: "📚 Documentation"
permalink: /docs/api/
has_children: true
---

# 🚀 HardFOC Interface Wrapper API Reference

![HardFOC Interface](https://img.shields.io/badge/HardFOC-Interface%20Wrapper-blue?style=for-the-badge&logo=hardware)

**🎯 Hardware Abstraction Layer for Embedded Systems**

*A platform-agnostic interface wrapper for hardware peripherals*

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#-architecture)
- [📋 **Base Classes**](#-base-classes)
- [🔧 **ESP32 Implementations**](#-esp32-implementations)
- [🎯 **Type System**](#-type-system)
- [📊 **Getting Started**](#-getting-started)
- [🧪 **Examples**](#-examples)
- [🧪 **Testing**](#-testing)

---

## 🎯 **Overview**

The **HardFOC Interface Wrapper** provides a unified,
platform-agnostic abstraction layer for embedded hardware peripherals.
It enables developers to write portable,
maintainable code that works across different microcontrollers and hardware platforms without
modification.

### ✨ **Key Benefits**

- 🔄 **Platform Portability** - Write once, run anywhere
- 🛡️ **Type Safety** - Strongly typed interfaces with error handling
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

```text
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
- **Error Recovery** - Recovery mechanisms where possible
- **Diagnostics** - Runtime error tracking and statistics

---

## 📋 **Base Classes**

The HardFOC Interface provides abstract base classes for all major hardware peripherals:

| Class | Purpose | Key Features | Typical Use Cases |

|-------|---------|--------------|-------------------|

<!-- markdownlint-disable-next-line MD013 -->
| **[BaseAdc](BaseAdc.md)** | Analog-to-Digital Conversion | Multi-channel, calibration | Sensor reading |

<!-- markdownlint-disable-next-line MD013 -->
| **[BaseGpio](BaseGpio.md)** | Digital I/O Control | Dynamic direction, interrupts | Status LEDs, switches |

| **BaseI2c** | I2C Bus Communication | Device scanning, register access | EEPROM, sensors |

| **BaseNvs** | Non-Volatile Storage | Key-value storage, namespaces | Configuration storage |

<!-- markdownlint-disable-next-line MD013 -->
| **BasePeriodicTimer** | High-Precision Timing | Microsecond resolution, callbacks | Control loops, sampling |

| **BasePio** | Programmable I/O | Precise timing, symbol transmission | WS2812 LEDs, IR |

| **BasePwm** | Pulse Width Modulation | Multi-channel, frequency control | Motor control, LED |

| **BaseSpi** | SPI Bus Communication | Full-duplex transfers, chip select | Flash memory, ADCs |

| **BaseUart** | Serial Communication | Flow control, buffering, printf support | Debug output, GPS |

| **BaseUsbSerialJtag** | Native-USB Console | CDC-ACM virtual COM port + JTAG over one cable | Operator console, log/REPL on dev kits |

| **BaseCan** | CAN Bus Communication | Message filtering, error handling | Motor control, vehicle |

| **BaseWifi** | WiFi Communication | Station/AP modes, security | IoT connectivity |

| **BaseBluetooth** | Bluetooth Communication | Classic & BLE, pairing | Mobile apps, wireless |

| **BaseTemperature** | Temperature Sensing | Multi-sensor support, calibration | System monitoring |

| **BaseLogger** | System Logging | Multi-level logging, multiple outputs | Debugging, diagnostics |

---

## 🔧 **ESP32 Implementations**

ESP32-C6 specific implementations with optimized features:

| Implementation | Base Class | ESP32-C6 Features | Documentation |

|----------------|------------|-------------------|---------------|

<!-- markdownlint-disable-next-line MD013 -->
| **[EspGpio](../esp_api/EspGpio.md)** | BaseGpio | Drive strength, slew rate, interrupts | ✅ Complete |

| **EspAdc** | BaseAdc | 12-bit resolution, multiple units | ✅ Available |

| **EspPwm** | BasePwm | LEDC controller, fade effects | 📝 In Progress |

| **EspI2c** | BaseI2c | Clock stretching, multi-master | 📝 In Progress |

| **[EspSpi](../esp_api/EspSpi.md)** | BaseSpi | Full-duplex, DMA support | ✅ Complete |

| **EspUart** | BaseUart | Hardware flow control | 📝 In Progress |

| **[EspUsbSerialJtag](../esp_api/EspUsbSerialJtag.md)** | BaseUsbSerialJtag | Built-in USB Serial/JTAG controller, IDF-console coexistence | ✅ Complete |

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
    EspAdc current_sensor*;
    EspPwm motor_driver*;
    EspGpio enable_pin*;
    
public:
    MotorController() 
        : current_sensor*(ADC_UNIT_1, ADC_ATTEN_DB_11)
        , motor_driver*()
        , enable_pin*(5, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT) {}
    
    bool Initialize() {
        current_sensor*.EnsureInitialized();
        motor_driver*.EnsureInitialized();
        enable_pin*.EnsureInitialized();
        
        // Configure motor driver
        motor_driver*.EnableChannel(0);
        motor_driver*.SetFrequency(0, 20000);  // 20kHz PWM
        return true;
    }
    
    void SetSpeed(float speed_percent) {
        motor_driver*.SetDutyCycle(0, speed_percent);
    }
    
    float GetCurrent() {
        float voltage;
        current_sensor*.ReadChannelV(0, voltage);
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
    EspI2c i2c_bus*;
    EspAdc analog_sensors*;
    
public:
    bool ScanSensors() {
        hf_u8_t addresses[16];
        hf_u8_t count = i2c_bus*.ScanBus(addresses, 16);
        
        printf("Found %u I2C devices:\n", count);
        for (hf_u8_t i = 0; i < count; i++) {
            printf("  Address: 0x%02X\n", addresses[i]);
        }
        return count > 0;
    }
    
    float ReadTemperature() {
        // Read from I2C temperature sensor
        hf_u8_t data[2];
        if (i2c_bus*.ReadRegisters(0x48, 0x00, data, 2)) {
            hf_u16_t raw = (data[0] << 8) | data[1];
            return (raw >> 4) * 0.0625f;  // Convert to Celsius
        }
        return -999.0f;  // Error value
    }
    
    float ReadPressure() {
        float voltage;
        analog_sensors*.ReadChannelV(1, voltage);
        return voltage * 100.0f;  // Convert to PSI
    }
};
```

---

## 🧪 **Testing**

Comprehensive test suites for validating hardware interface implementations:

- **[🧪 Test Suites](../../examples/esp32/docs/README.md)** - Testing and validation

### **Test Features**

- **Automated Testing** - Comprehensive validation of all functionality
- **Performance Testing** - Speed, DMA, and optimization validation
- **Error Handling** - Edge case and failure mode testing
- **Hardware Validation** - Real hardware signal verification
- **Pattern Testing** - Data integrity and timing validation

---

## 🔗 **Navigation**

### **Documentation Structure**

- **[🏠 Main Documentation](../README.md)** - Complete system overview
- **[🔧 ESP32 Implementations](../esp_api/README.md)** - Hardware-specific implementations
- **[🛠️ Utility Classes](../utils/README.md)** - Advanced utility classes and helpers
- **[🧪 Test Suites](../../examples/esp32/docs/README.md)** - Testing and validation

### **Base Class Documentation**

| **Interface** | **Documentation** | **Status** |

|---------------|-------------------|------------|

| **[BaseAdc](BaseAdc.md)** | Analog-to-Digital Conversion | ✅ Complete |

| **[BaseGpio](BaseGpio.md)** | Digital I/O Control | ✅ Complete |

| **[BaseI2c](BaseI2c.md)** | I2C Bus Communication | ✅ Complete |

| **[BaseNvs](BaseNvs.md)** | Non-Volatile Storage | ✅ Complete |

| **[BasePeriodicTimer](BasePeriodicTimer.md)** | High-Precision Timing | ✅ Complete |

| **[BasePio](BasePio.md)** | Programmable I/O | ✅ Complete |

| **[BasePwm](BasePwm.md)** | Pulse Width Modulation | ✅ Complete |

| **[BaseSpi](BaseSpi.md)** | SPI Bus Communication | ✅ Complete |

| **[BaseUart](BaseUart.md)** | Serial Communication | ✅ Complete |

| **[BaseUsbSerialJtag](BaseUsbSerialJtag.md)** | Native-USB Console (CDC-ACM + JTAG) | ✅ Complete |

| **[BaseCan](BaseCan.md)** | CAN Bus Communication | ✅ Complete |

| **[BaseWifi](BaseWifi.md)** | WiFi Communication | ✅ Complete |

| **[BaseBluetooth](BaseBluetooth.md)** | Bluetooth Communication | ✅ Complete |

| **[BaseTemperature](BaseTemperature.md)** | Temperature Sensing | ✅ Complete |

| **[BaseLogger](BaseLogger.md)** | Logging System | ✅ Complete |

### **Related Resources**

- **[Hardware Types](HardwareTypes.md)** - Type definitions and validation

---

**🚀 HardFOC Interface Wrapper**

*Part of the HardFOC Ecosystem*

 
