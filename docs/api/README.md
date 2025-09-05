# 🚀 HardFOC Interface Wrapper API Reference

<div align="center">

![HardFOC
Interface](https://img.shields.io/badge/HardFOC-Interface%20Wrapper-blue?style=for-the-badge&logo=hardware)

**🎯 Hardware Abstraction Layer for Embedded Systems**

*A platform-agnostic interface wrapper for hardware peripherals*

</div>

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
```text

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

| **[EspGpio](../esp_api/EspGpio.md)** | BaseGpio | Drive strength, slew rate, interrupts | ✅ Complete |

| **EspAdc** | BaseAdc | 12-bit resolution, multiple units | ✅ Available |

| **EspPwm** | BasePwm | LEDC controller, fade effects | 📝 In Progress |

| **EspI2c** | BaseI2c | Clock stretching, multi-master | 📝 In Progress |

| **[EspSpi](../esp_api/EspSpi.md)** | BaseSpi | Full-duplex, DMA support, IOMUX optimization | ✅ Complete |

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
using hf*u8*t = uint8*t;
using hf*u16*t = uint16*t;
using hf*u32*t = uint32*t;
using hf*u64*t = uint64*t;

// Hardware types
using hf*pin*num*t = hf*i32*t;
using hf*channel*id*t = hf*u32*t;
using hf*frequency*hz*t = hf*u32*t;
using hf*time*t = hf*u32*t;
```text

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
```text

### **2. Create Hardware Instances**

```cpp
// Use platform-specific implementations
EspAdc adc(ADC*UNIT*1, ADC*ATTEN*DB*11);
EspGpio led*pin(2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);
```text

### **3. Initialize and Use**

```cpp
// Lazy initialization (automatic on first use)
adc.EnsureInitialized();
led*pin.EnsureInitialized();

// Use the hardware
float voltage;
if (adc.ReadChannelV(0, voltage) == hf*adc*err*t::ADC*SUCCESS) {
    printf("Voltage: %.3f V\n", voltage);
}

if (voltage > 3.0f) {
    led*pin.SetActive();
}
```text

### **4. Error Handling**

```cpp
hf*adc*err*t result = adc.ReadChannelV(0, voltage);
if (result != hf*adc*err*t::ADC*SUCCESS) {
    printf("ADC Error: %s\n", HfAdcErrToString(result));
    // Handle error appropriately
}
```text

---

## 🧪 **Examples**

### **Motor Control System**

```cpp
#include "inc/mcu/esp32/EspAdc.h"
#include "inc/mcu/esp32/EspPwm.h"
#include "inc/mcu/esp32/EspGpio.h"

class MotorController {
private:
    EspAdc current*sensor*;
    EspPwm motor*driver*;
    EspGpio enable*pin*;
    
public:
    MotorController() 
        : current*sensor*(ADC*UNIT*1, ADC*ATTEN*DB*11)
        , motor*driver*()
        , enable*pin*(5, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT) {}
    
    bool Initialize() {
        current*sensor*.EnsureInitialized();
        motor*driver*.EnsureInitialized();
        enable*pin*.EnsureInitialized();
        
        // Configure motor driver
        motor*driver*.EnableChannel(0);
        motor*driver*.SetFrequency(0, 20000);  // 20kHz PWM
        return true;
    }
    
    void SetSpeed(float speed*percent) {
        motor*driver*.SetDutyCycle(0, speed*percent);
    }
    
    float GetCurrent() {
        float voltage;
        current*sensor*.ReadChannelV(0, voltage);
        return (voltage - 2.5f) / 0.1f;  // Convert to current (A)
    }
};
```text

### **Sensor Network**

```cpp
#include "inc/mcu/esp32/EspI2c.h"
#include "inc/mcu/esp32/EspAdc.h"

class SensorNetwork {
private:
    EspI2c i2c*bus*;
    EspAdc analog*sensors*;
    
public:
    bool ScanSensors() {
        hf*u8*t addresses[16];
        hf*u8*t count = i2c*bus*.ScanBus(addresses, 16);
        
        printf("Found %u I2C devices:\n", count);
        for (hf*u8*t i = 0; i < count; i++) {
            printf("  Address: 0x%02X\n", addresses[i]);
        }
        return count > 0;
    }
    
    float ReadTemperature() {
        // Read from I2C temperature sensor
        hf*u8*t data[2];
        if (i2c*bus*.ReadRegisters(0x48, 0x00, data, 2)) {
            hf*u16*t raw = (data[0] << 8) | data[1];
            return (raw >> 4) * 0.0625f;  // Convert to Celsius
        }
        return -999.0f;  // Error value
    }
    
    float ReadPressure() {
        float voltage;
        analog*sensors*.ReadChannelV(1, voltage);
        return voltage * 100.0f;  // Convert to PSI
    }
};
```text

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

| **[BaseCan](BaseCan.md)** | CAN Bus Communication | ✅ Complete |

| **[BaseWifi](BaseWifi.md)** | WiFi Communication | ✅ Complete |

| **[BaseBluetooth](BaseBluetooth.md)** | Bluetooth Communication | ✅ Complete |

| **[BaseTemperature](BaseTemperature.md)** | Temperature Sensing | ✅ Complete |

| **[BaseLogger](BaseLogger.md)** | Logging System | ✅ Complete |

### **Related Resources**

- **[Contributing Guidelines](../../CONTRIBUTING.md)** - How to contribute
- **[Hardware Types](HardwareTypes.md)** - Type definitions and validation

---

<div align="center">

**🚀 HardFOC Interface Wrapper**

*Part of the HardFOC Ecosystem*

</div> 