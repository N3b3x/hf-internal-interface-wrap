# 🚀 HardFOC Internal Interface Wrapper

<div align="center">

![Multi-MCU
Interface](https://img.shields.io/badge/Multi--MCU-Interface%20Wrapper-blue?style=for-the-badge&logo=microchip)
![C++17](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![ESP32-C6](https://img.shields.io/badge/ESP32--C6-First%20MCU-green?style=for-the-badge&logo=espressif)
![STM32](https://img.shields.io/badge/STM32-Coming%20Soon-orange?style=for-the-badge&logo=stmicroelectronics)
![License](https://img.shields.io/badge/License-GPL--3.0-green?style=for-the-badge&logo=opensourceinitiative)

**🎯 Multi-MCU Hardware Abstraction Layer**

*Universal interface wrapper supporting multiple MCU platforms - ESP32 first implementation*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#-multi-mcu-architecture)
- [🔧 **Type System**](#-type-system)
- [✨ **Key Features**](#-key-features)
- [🔌 **Supported Hardware**](#-supported-hardware)
- [🏛️ **Design Principles**](#-design-principles)
- [📋 **API Reference**](#-api-reference)
- [🚀 **Quick Start**](#-quick-start)
- [📊 **Examples**](#-examples)
- [🤝 **Contributing**](#-contributing)
- [📄 **License**](#-license)

---

## 🎯 **Overview**

The **HardFOC Internal Interface Wrapper** is a multi-MCU hardware abstraction layer designed to
provide unified APIs across different microcontroller platforms.
Currently supporting the **ESP32 family** (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6,
ESP32-H2) as the first implementation,
with **STM32** and additional MCUs planned for future releases.

### 🎯 **Multi-MCU Architecture Goals**

- **🔄 MCU Portability** - Write once, run on multiple MCU platforms
- **🎯 Unified APIs** - Consistent interface across all peripheral types
- **⚡ Performance** - Zero-cost abstractions with compile-time optimization
- **🛡️ Type Safety** - Strong typing with project-specific type system
- **📈 Extensible** - Easy to add new MCUs and peripheral drivers
- **🔌 Complete Coverage** - 14+ peripheral interfaces for comprehensive hardware control

---

## 🏗️ **Multi-MCU Architecture**

The wrapper follows a multi-layered architecture supporting multiple MCU platforms:

```text
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           🎯 Application Layer                                  │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ Motor       │  │ IoT         │  │ Industrial  │  │ Custom      │             │
│  │ Control     │  │ Integration │  │ Systems     │  │ Applications│             │
│  │ Apps        │  │ Apps        │  │ Apps        │  │ Apps        │             │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘             │
│         │                 │                 │                 │                 │
│         └─────────────────┼─────────────────┼─────────────────┘                 │
│                           │                 │                                   │
│  ┌────────────────────────────────────────────────────────────────────────────┐ │
│  │                    🔒 Thread-Safe Layer (Optional)                         │ │
│  │                                                                            │ │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │ │
│  │  │ Thread-Safe │  │ Concurrent  │  │ Mutex       │  │ Lock-Free   │        │ │
│  │  │ Wrappers    │  │ Access      │  │ Protection  │  │ Operations  │        │ │
│  │  │             │  │ Control     │  │             │  │             │        │ │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘        │ │
│  └────────────────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                        🏛️ Base Interface Layer (MCU-Agnostic)                   │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────┐  ┌─────────────┐            │
│  │ Core        │  │ Communication│  │ Wireless    │  │ System      │            │
│  │ Interfaces  │  │ Interfaces   │  │ Interfaces  │  │ Interfaces  │            │
│  │             │  │              │  │             │  │             │            │
│  │ GPIO        │  │ I2C          │  │ WiFi        │  │ NVS         │            │
│  │ ADC         │  │ SPI          │  │ Bluetooth   │  │ Timer       │            │
│  │ PWM         │  │ UART         │  │             │  │ Temperature │            │
│  │ PIO         │  │ CAN          │  │             │  │ Logger      │            │
│  └─────────────┘  └──────────────┘  └─────────────┘  └─────────────┘            │
└─────────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                      ⚙️ MCU Implementation Layer                                │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ ESP32-C6    │  │ STM32       │  │ Future      │  │ External    │             │
│  │ (Current)   │  │ (Planned)   │  │ MCUs        │  │ Hardware    │             │
│  │             │  │             │  │ (Planned)   │  │ Drivers     │             │
│  │ EspGpio     │  │ StmGpio     │  │             │  │ I2C Devices │             │
│  │ EspAdc      │  │ StmAdc      │  │             │  │ SPI Devices │             │
│  │ EspPwm      │  │ StmPwm      │  │             │  │ UART Devices│             │
│  │ EspI2c      │  │ StmI2c      │  │             │  │ CAN Devices │             │
│  │ EspSpi      │  │ StmSpi      │  │             │  │             │             │
│  │ EspUart     │  │ StmUart     │  │             │  │             │             │
│  │ EspCan      │  │ StmCan      │  │             │  │             │             │
│  │ EspWifi     │  │ StmWifi     │  │             │  │             │             │
│  │ EspBluetooth│  │ StmBluetooth│  │             │  │             │             │
│  │ EspNvs      │  │ StmNvs      │  │             │  │             │             │
│  │ EspTimer    │  │ StmTimer    │  │             │  │             │             │
│  │ EspTemp     │  │ StmTemp     │  │             │  │             │             │
│  │ EspLogger   │  │ StmLogger   │  │             │  │             │             │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘             │
└─────────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           🔧 Hardware Layer                                     │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ ESP32-C6    │  │ STM32       │  │ Future      │  │ External    │             │
│  │ Hardware    │  │ Hardware    │  │ MCU         │  │ Components  │             │
│  │ (Current)   │  │ (Planned)   │  │ Hardware    │  │             │             │
│  │             │  │             │  │ (Planned)   │  │ Sensors     │             │
│  │ GPIO Pins   │  │ GPIO Pins   │  │             │  │ Actuators   │             │
│  │ ADC Units   │  │ ADC Units   │  │             │  │ Displays    │             │
│  │ PWM Timers  │  │ PWM Timers  │  │             │  │ Memory      │             │
│  │ I2C Buses   │  │ I2C Buses   │  │             │  │ Storage     │             │
│  │ SPI Buses   │  │ SPI Buses   │  │             │  │             │             │
│  │ UART Ports  │  │ UART Ports  │  │             │  │             │             │
│  │ CAN Controllers│ CAN Controllers│             │  │             │             │
│  │ WiFi Radio  │  │ WiFi Radio  │  │             │  │             │             │
│  │ BT Radio    │  │ BT Radio    │  │             │  │             │             │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘             │
└─────────────────────────────────────────────────────────────────────────────────┘
```text

### 🔄 **Interface Inheritance Pattern**

All interfaces follow a consistent inheritance pattern across MCU platforms:

```text
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           🏛️ Base Interface (Abstract)                          │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌────────────────────────────────────────────────────────────────────────────┐ │
│  │                    BaseInterface (Pure Virtual)                            │ │
│  │                                                                            │ │
│  │  + EnsureInitialized() → error*t                                           │ │
│  │  + IsInitialized() → bool                                                  │ │
│  │  + GetCapabilities() → capabilities*t                                      │ │
│  │  + Reset() → error*t                                                       │ │
│  │  + GetLastError() → error*t                                                │ │
│  │  + GetStatistics() → statistics*t                                          │ │
│  └────────────────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                        🔧 MCU-Specific Implementations                          │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                              │
│  │ ESP32       │  │ STM32       │  │ Future      │                              │
│  │ (Current)   │  │ (Planned)   │  │             │                              │
│  │             │  │             │  │ (Planned)   │                              │
│  │ EspGpio     │  │ StmGpio     │  │             │                              │
│  │ EspAdc      │  │ StmAdc      │  │             │                              │
│  │ EspPwm      │  │ StmPwm      │  │             │                              │
│  │ ...         │  │ ...         │  │ ...         │                              │
│  │             │  │             │  │             │                              │                 
│  └─────────────┘  └─────────────┘  └─────────────┘                              │                 
└─────────────────────────────────────────────────────────────────────────────────┘
```text

---

## 🔧 **Type System**

The wrapper uses a comprehensive type system designed for maximum portability and consistency across
multiple MCU platforms:

### 🎯 **Core Types for Multi-MCU Applications**

```cpp
// Platform-agnostic integer types for all MCU platforms
using hf*u8*t = uint8*t;    // 8-bit unsigned
using hf*u16*t = uint16*t;  // 16-bit unsigned  
using hf*u32*t = uint32*t;  // 32-bit unsigned
using hf*u64*t = uint64*t;  // 64-bit unsigned
using hf*i8*t = int8*t;     // 8-bit signed
using hf*i16*t = int16*t;   // 16-bit signed
using hf*i32*t = int32*t;   // 32-bit signed
using hf*i64*t = int64*t;   // 64-bit signed
```text

### 🏭 **Hardware Abstraction Types**

```cpp
// Hardware abstraction types for all MCU platforms
using hf*pin*num*t = hf*i32*t;          // GPIO pin numbers
using hf*channel*id*t = hf*u32*t;       // ADC/PWM/DMA channels
using hf*time*t = hf*u64*t;             // Time values in microseconds
using hf*frequency*hz*t = hf*u32*t;     // Frequency values in Hz

// Application-specific semantic types
using hf*voltage*mv*t = hf*u32*t;       // Voltage in millivolts
using hf*current*ma*t = hf*u32*t;       // Current in milliamps
using hf*temperature*c*t = hf*i32*t;    // Temperature in Celsius (scaled by 100)
using hf*speed*rpm*t = hf*u32*t;        // Motor speed in RPM
using hf*torque*nm*t = hf*u32*t;        // Torque in Newton-meters (scaled)
```text

📖 **Complete Documentation**: [HardwareTypes API Reference](api/HardwareTypes.md)

---

## ✨ **Key Features**

### 🔌 **Comprehensive Multi-MCU Hardware Support**
- **14 Complete Base Interfaces** - From GPIO to wireless communication across all MCU platforms
- **ESP32 Family Implementation** - Full support for all ESP32 variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2) (current)
- **STM32 Implementation** - Planned support for STM32 peripherals (future)
- **External Hardware Integration** - I2C/SPI device support for expansion boards

### ⚡ **Performance & Reliability**
- **Real-Time Optimized** - Designed for critical timing requirements
- **Lazy Initialization** - Resources allocated only when needed
- **Thread-Safe Options** - Optional concurrent access support
- **Comprehensive Error Handling** - Detailed error reporting for reliability

### 🌐 **Modern Connectivity**
- **WiFi Station/AP Modes** - Complete networking for IoT integration
- **Bluetooth Classic & BLE** - Mobile and IoT connectivity
- **Cloud Integration Ready** - Built-in features for cloud connectivity
- **Remote Monitoring** - Advanced logging and diagnostics

### 📊 **Professional Features**
- **Rich Diagnostics** - Performance monitoring and system health
- **Configuration Management** - Non-volatile settings storage
- **Advanced Logging** - Multi-level, multi-output logging
- **Thermal Management** - Temperature monitoring and protection

---

## 🔌 **Supported Hardware**

### 📊 **Multi-MCU Platform Support Matrix**

| **MCU Platform** | **GPIO** | **ADC** | **PWM** | **I2C** | **SPI** | **UART** | **CAN** | **WiFi** | **BT** | **Temp** | **NVS** | **Timer** | **PIO** | **Logger** | **Status** |

|------------------|----------|---------|---------|---------|---------|----------|---------|----------|--------|----------|---------|-----------|---------|------------|------------|

| **ESP32 Family** | ✅       | ✅      | ✅      | ✅      | ✅      | ✅       | ✅      | ✅       | ✅     | ✅       | ✅      | ✅        | ✅      | ✅         | **Current** |

| **STM32**        | 🔄       | 🔄      | 🔄      | 🔄      | 🔄      | 🔄       | 🔄      | 🔄       | 🔄     | 🔄       | 🔄      | 🔄        | 🔄      | 🔄         | **Planned** |

| **Future MCUs**  | 🔄       | 🔄      | 🔄      | 🔄      | 🔄      | 🔄       | 🔄      | 🔄       | 🔄     | 🔄       | 🔄      | 🔄        | 🔄      | 🔄         | **Planned** |

---

## 🏛️ **Design Principles**

### 🎯 **Multi-MCU Design Goals**

1. **🔌 Consistency** - Uniform APIs across all MCU platforms and peripheral interfaces
2. **⚡ Performance** - Optimized for real-time requirements across all MCU platforms
3. **🛡️ Reliability** - Comprehensive error handling for critical applications
4. **📈 Scalability** - From simple prototypes to complex industrial systems
5. **🔧 Simplicity** - Easy-to-use APIs for rapid development
6. **🌐 Modern** - Built-in IoT connectivity for next-generation applications

### 🏗️ **Architectural Patterns**

- **Abstract Base Classes** - Define consistent interfaces for all peripheral types
- **Platform Implementations** - Hardware-specific optimizations for each MCU platform
- **Optional Thread Safety** - Concurrent access support for complex applications
- **Lazy Resource Management** - Efficient memory usage on resource-constrained MCUs
- **Comprehensive Error Handling** - Detailed error reporting for reliability
- **Semantic Type System** - Application domain-specific types for clarity

---

## 📋 **API Reference**

### 📚 **Documentation Structure**

Our comprehensive documentation is organized into logical sections for easy navigation:

| **Section** | **Description** | **Documentation** |

|-------------|-----------------|-------------------|

| **[📋 API Interfaces](api/README.md)** | Base classes and abstract interfaces | Complete API reference with examples |

| **[🔧 ESP32 Implementations](esp_api/README.md)** | ESP32-C6 specific implementations | Hardware-specific optimizations and features |

| **[🛠️ Utility Classes](utils/README.md)** | Advanced utility classes and helpers | RAII patterns, safety mechanisms, and convenience wrappers |

| **[🧪 Test Suites](../examples/esp32/docs/README.md)** | Test documentation and examples | Test suites and examples |

### 🏛️ **Core Interfaces (MCU-Agnostic)**

| **Interface** | **Key Features** | **Use Cases** | **Status** |

|---------------|------------------|---------------|------------|

| [**`BaseGpio`**](api/BaseGpio.md) | Digital I/O, interrupts, pull resistors | Enable pins, limit switches, indicators | ✅ Complete |

| [**`BaseAdc`**](api/BaseAdc.md) | Multi-channel, calibration, voltage conversion | Current sensing, position feedback | ✅ Complete |

| [**`BasePwm`**](api/BasePwm.md) | Multi-channel, frequency control, duty cycle | Motor speed control, servo control | ✅ Complete |

| [**`BasePio`**](api/BasePio.md) | Custom protocols, precise timing, encoding | Encoder reading, custom protocols | ✅ Complete |

### 📡 **Communication Interfaces (MCU-Agnostic)**

| **Interface** | **Key Features** | **Use Cases** | **Status** |

|---------------|------------------|---------------|------------|

| [**`BaseI2c`**](api/BaseI2c.md) | Master mode, device scanning, error recovery | Sensor communication, display control | ✅ Complete |

| [**`BaseSpi`**](api/BaseSpi.md) | Full-duplex, configurable modes, DMA support | High-speed data, SD cards | ✅ Complete |

| [**`BaseUart`**](api/BaseUart.md) | Async I/O, flow control, configurable parameters | Debug output, external communication | ✅ Complete |

| [**`BaseCan`**](api/BaseCan.md) | Standard/Extended frames, filtering, error handling | Industrial networking, multi-device coordination | ✅ Complete |

### 🌐 **Wireless Interfaces (MCU-Agnostic)**

| **Interface** | **Key Features** | **Use Cases** | **Status** |

|---------------|------------------|---------------|------------|

| [**`BaseWifi`**](api/BaseWifi.md) | Station/AP modes, WPA3 security, mesh networking | Cloud connectivity, remote monitoring | ✅ Complete |

| [**`BaseBluetooth`**](api/BaseBluetooth.md) | Classic & BLE, pairing, service discovery | Mobile apps, wireless configuration | ✅ Complete |

### 🛠️ **System Interfaces (MCU-Agnostic)**

| **Interface** | **Key Features** | **Use Cases** | **Status** |

|---------------|------------------|---------------|------------|

| [**`BaseNvs`**](api/BaseNvs.md) | Key-value storage, encryption, wear leveling | Configuration storage, calibration data | ✅ Complete |

| [**`BasePeriodicTimer`**](api/BasePeriodicTimer.md) | Callback scheduling, high precision, multi-timer | Control loops, sensor sampling | ✅ Complete |

| [**`BaseTemperature`**](api/BaseTemperature.md) | Multi-sensor support, calibration, thermal protection | Thermal monitoring, safety protection | ✅ Complete |

| [**`BaseLogger`**](api/BaseLogger.md) | Multi-level logging, thread-safe, network output | System diagnostics, performance monitoring | ✅ Complete |

### 🔧 **MCU-Specific Implementations**

| **MCU Platform** | **Implementation** | **Base Class** | **MCU-Specific Features** | **Documentation** | **Status** |

|------------------|-------------------|----------------|---------------------------|-------------------|------------|

| **ESP32 Family** | [**`EspGpio`**](esp_api/EspGpio.md) | BaseGpio | Drive strength, slew rate control | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspAdc`**](esp_api/EspAdc.md) | BaseAdc | 12-bit resolution, multiple units | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspPwm`**](esp_api/EspPwm.md) | BasePwm | LEDC controller, fade effects | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspI2c`**](esp_api/EspI2c.md) | BaseI2c | Clock stretching, multi-master | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspSpi`**](esp_api/EspSpi.md) | BaseSpi | Full-duplex, DMA support | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspUart`**](esp_api/EspUart.md) | BaseUart | Hardware flow control | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspCan`**](esp_api/EspCan.md) | BaseCan | TWAI controller, SN65 transceiver | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspWifi`**](esp_api/EspWifi.md) | BaseWifi | 802.11n, WPA3, mesh | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspBluetooth`**](esp_api/EspBluetooth.md) | BaseBluetooth | BLE/Classic, NimBLE optimized | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspNvs`**](esp_api/EspNvs.md) | BaseNvs | Encrypted storage, wear leveling | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspPeriodicTimer`**](esp_api/EspPeriodicTimer.md) | BasePeriodicTimer | High precision, multi-timer | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspTemperature`**](esp_api/EspTemperature.md) | BaseTemperature | Internal sensor, I2C/1-Wire | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspPio`**](esp_api/EspPio.md) | BasePio | RMT peripheral, custom protocols | ✅ Complete | ✅ Complete |

| **ESP32 Family** | [**`EspLogger`**](esp_api/EspLogger.md) | BaseLogger | Multi-output, network logging | ✅ Complete | ✅ Complete |

| **STM32** | **`StmGpio`** | BaseGpio | STM32-specific GPIO features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmAdc`** | BaseAdc | STM32-specific ADC features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmPwm`** | BasePwm | STM32-specific PWM features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmI2c`** | BaseI2c | STM32-specific I2C features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmSpi`** | BaseSpi | STM32-specific SPI features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmUart`** | BaseUart | STM32-specific UART features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmCan`** | BaseCan | STM32-specific CAN features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmWifi`** | BaseWifi | STM32-specific WiFi features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmBluetooth`** | BaseBluetooth | STM32-specific Bluetooth features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmNvs`** | BaseNvs | STM32-specific NVS features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmPeriodicTimer`** | BasePeriodicTimer | STM32-specific timer features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmTemperature`** | BaseTemperature | STM32-specific temperature features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmPio`** | BasePio | STM32-specific PIO features | 🔄 Planned | 🔄 Planned |

| **STM32** | **`StmLogger`** | BaseLogger | STM32-specific logger features | 🔄 Planned | 🔄 Planned |

### 📋 **ESP32 Family Support Details**

The ESP32 implementations support multiple ESP32 variants with conditional compilation:

| **ESP32 Variant** | **GPIO** | **ADC** | **PWM** | **I2C** | **SPI** | **UART** | **CAN** | **WiFi** | **BT** | **Temp** | **NVS** | **Timer** | **PIO** | **Logger** |

|-------------------|----------|---------|---------|---------|---------|----------|---------|----------|--------|----------|---------|-----------|---------|------------|

| **ESP32** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

| **ESP32-S2** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |

| **ESP32-S3** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

| **ESP32-C3** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

| **ESP32-C6** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

| **ESP32-H2** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

### 🎯 **Type System Reference**

| **Documentation** | **Description** | **Status** |

|------------------|-----------------|------------|

| [**`HardwareTypes`**](api/HardwareTypes.md) | Platform-agnostic type definitions | ✅ Complete |

---

## 🚀 **Quick Start**

### 📋 **Prerequisites for Multi-MCU Development**

- **ESP-IDF v5.0+** for ESP32 family development (current)
- **STM32CubeIDE** for STM32 development (planned)
- **C++17** compatible compiler (GCC 8+ or Clang 7+)
- **CMake 3.16+** for project management
- **Target MCU Development Board** (ESP32 family, STM32, etc.)

### ⚙️ **Installation for Multi-MCU Projects**

```bash
## Clone the multi-MCU wrapper repository
git clone https://github.com/hardfoc/hf-internal-interface-wrap.git
cd hf-internal-interface-wrap

## For ESP32 projects, add to your CMakeLists.txt
idf*component*register(
    SRCS "main.cpp"
    INCLUDE*DIRS "."
    REQUIRES hf*internal*interface*wrap
)

## For STM32 projects (future)
## Add to your CMakeLists.txt or project configuration
```text

### 🎯 **Basic Multi-MCU GPIO Example**

```cpp
// ESP32 Family Implementation (Current)
#include "inc/mcu/esp32/EspGpio.h"

// Create output pin for LED control
EspGpio led*pin(GPIO*NUM*2, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);

// Create input pin for button
EspGpio button*pin(GPIO*NUM*0, hf*gpio*direction*t::HF*GPIO*DIRECTION*INPUT,
                  hf*gpio*active*state*t::HF*GPIO*ACTIVE*LOW,
                  hf*gpio*output*mode*t::HF*GPIO*OUTPUT*MODE*PUSH*PULL,
                  hf*gpio*pull*mode*t::HF*GPIO*PULL*MODE*UP);

void app*main() {
    // Initialize pins
    led*pin.EnsureInitialized();
    button*pin.EnsureInitialized();
    
    while (true) {
        if (button*pin.IsActive()) {
            led*pin.SetActive();    // Turn on LED when button pressed
        } else {
            led*pin.SetInactive();  // Turn off LED when button released
        }
        vTaskDelay(pdMS*TO*TICKS(10));
    }
}

// STM32 Implementation (Future)
// #include "inc/mcu/stm32/StmGpio.h"
// StmGpio led*pin(GPIO*PIN*5, hf*gpio*direction*t::HF*GPIO*DIRECTION*OUTPUT);
// ... same API, different implementation
```text

### 📊 **Basic Multi-MCU ADC Example**

```cpp
// ESP32 Family Implementation (Current)
#include "inc/mcu/esp32/EspAdc.h"

void read*sensors() {
    EspAdc adc(ADC*UNIT*1, ADC*ATTEN*DB*11);
    
    // Initialize ADC
    if (!adc.EnsureInitialized()) {
        printf("Failed to initialize ADC\n");
        return;
    }
    
    // Read current sensor (channel 0)
    float current*voltage;
    if (adc.ReadChannelV(0, current*voltage) == hf*adc*err*t::ADC*SUCCESS) {
        float current*amps = (current*voltage - 2.5f) / 0.1f;  // ACS712 conversion
        printf("Current: %.2f A\n", current*amps);
    }
    
    // Read position sensor (channel 1)
    float position*voltage;
    if (adc.ReadChannelV(1, position*voltage) == hf*adc*err*t::ADC*SUCCESS) {
        float position*degrees = (position*voltage / 3.3f) * 360.0f;
        printf("Position: %.1f degrees\n", position*degrees);
    }
}

// STM32 Implementation (Future)
// #include "inc/mcu/stm32/StmAdc.h"
// StmAdc adc(ADC1, ADC*CHANNEL*0);
// ... same API, different implementation
```text

---

## 📊 **Examples**

### 🎯 **Basic Interface Examples (Multi-MCU)**
- **GPIO Control** - LED and button control across MCU platforms
- **ADC Monitoring** - Sensor data acquisition for all MCUs
- **PWM Generation** - Motor speed control for all MCUs
- **Temperature Sensing** - Thermal monitoring across platforms

### 🌐 **Wireless Examples (Multi-MCU)**
- **WiFi Station** - Internet connectivity for IoT applications
- **WiFi Access Point** - Local network creation for all MCUs
- **Bluetooth BLE** - Mobile app integration across platforms
- **Bluetooth Classic** - Serial communication for all MCUs

### 🚀 **Advanced Integration Examples (Multi-MCU)**
- **Complete Motor Controller** - Full-featured motor control with TMC-style controllers
- **IoT Gateway** - WiFi bridge with monitoring across MCUs
- **Multi-Sensor Logger** - Data collection system for all platforms
- **Secure Communication** - Encrypted data transfer across MCUs

### 🧪 **Production-Ready Examples (Multi-MCU)**
- **Industrial Control System** - Complete industrial solution
- **Automotive Interface** - CAN bus integration across platforms
- **Remote Monitoring** - Cloud-connected system for all MCUs
- **Diagnostic System** - Advanced diagnostics across platforms

---

### ⚙️ **Multi-MCU Project Configuration**

Configure specific features for your target MCU platform:

- **Interface Selection** - Enable only the interfaces your MCU uses
- **Performance Tuning** - Optimize for real-time requirements
- **Memory Configuration** - Configure buffers for your application
- **Wireless Settings** - WiFi and Bluetooth configuration for IoT
- **Debug Options** - Logging levels for development

---

## 🤝 **Contributing**

We welcome contributions to improve multi-MCU support!
Please see our [Contributing Guidelines](../CONTRIBUTING.md).

### 🎯 **Areas for Multi-MCU Development**
- **New MCU Support** - Additional MCU platform implementations (STM32, etc.)
- **Performance Optimization** - Real-time improvements for all MCU platforms
- **Example Applications** - More use case demonstrations across MCUs
- **Documentation** - Enhanced guides for multi-MCU development
- **Testing** - Hardware validation across all supported MCUs

---

## 📄 **License**

This project is licensed under the **GNU General Public License v3.0** - see the
[LICENSE](../LICENSE) file for details.

The GPL-3.0 license ensures that improvements to the multi-MCU wrapper remain open source and
benefit the entire community.

---

<div align="center">

**🚀 Multi-MCU Interface Wrapper**

*Universal hardware abstraction layer supporting multiple MCU platforms*

---

**🔗 Quick Links**

[🚀 Quick Start](#-quick-start) | [📋 API Reference](#-api-reference) | [📊 Examples](#-examples) | [🤝
Contributing](#-contributing)

**📚 Documentation Navigation**

[📋 API Interfaces](api/README.md) | [🔧 ESP32 Implementations](esp_api/README.md) | [🛠️ Utility
Classes](utils/README.md) | [🧪 Test Suites](../examples/esp32/docs/README.md)

**📞 Support**

[💬 GitHub Discussions](https://github.com/N3b3x/hf-internal-interface-wrap/discussions) | [🐛 Issue
Tracker](https://github.com/N3b3x/hf-internal-interface-wrap/issues) | [📧 Multi-MCU
Support](mailto:support@hardfoc.com)

</div>
