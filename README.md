# 🔧 HardFOC Internal Interface Layer: **Multi-MCU Peripheral Interface**

<div align="center">

![HAL](https://img.shields.io/badge/HAL-Hardware%20Abstraction%20Layer-blue?style=for-the-badge&logo=microchip)
![C++17](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![Multi-MCU](https://img.shields.io/badge/Multi--MCU-Support-green?style=for-the-badge&logo=espressif)
![License](https://img.shields.io/badge/License-GPL--3.0-green?style=for-the-badge&logo=opensourceinitiative)

**🎯 Universal Hardware Interface for Multi-MCU Development**

*A professional hardware abstraction layer enabling seamless MCU portability through unified
peripheral APIs - designed for the HardFOC board ecosystem*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#-architecture)
- [🔌 **Peripheral Interfaces**](#-peripheral-interfaces)
- [🖥️ **MCU Support**](#-mcu-support)
- [🚀 **Quick Start**](#-quick-start)
- [📖 **API Documentation**](#-api-documentation)
- [🔧 **Building**](#-building)
- [📊 **Examples**](#-examples)
- [🤝 **Contributing**](#-contributing)
- [📄 **License**](#-license)

---

## 🎯 **Overview**

This **Internal Interface Wrap (IID)** provides a unified interface for common MCU peripherals,
enabling seamless portability between different microcontroller platforms.
Originally designed for the **HardFOC board** which needs to support multiple MCU types,
this abstraction layer allows developers to write hardware-agnostic code while maintaining optimal
performance.

### 🏆 **Core Benefits**

- **🔄 MCU Portability** - Write once, run on multiple MCU platforms
- **🎯 Unified APIs** - Consistent interface across all peripheral types
- **⚡ Performance** - Zero-cost abstractions with compile-time optimization
- **🛡️ Type Safety** - Strong typing with project-specific type system
- **📈 Extensible** - Easy to add new MCUs and peripheral drivers
- **🔌 Complete Coverage** - 14+ peripheral interfaces for comprehensive hardware control

### 🎨 **Design Philosophy**

```cpp
// Write hardware-agnostic code
BaseGpio* led = GpioFactory::Create(GPIO*PIN*2, GPIO*OUTPUT);
led->SetHigh();

// Same code works on ESP32, STM32, or any supported MCU
// The factory handles MCU-specific implementation selection
```text

---

## 🏗️ **Architecture**

### **Two-Layer Design**

```text
📦 Hardware Abstraction Layer
├── 🎯 Base Layer (inc/base/)           # Abstract interfaces
│   ├── BaseGpio.h                      # GPIO operations
│   ├── BaseAdc.h                       # Analog-to-digital conversion
│   ├── BasePwm.h                       # Pulse width modulation
│   ├── BaseUart.h                      # Serial communication
│   ├── BaseI2c.h                       # I2C bus operations
│   ├── BaseSpi.h                       # SPI bus operations
│   ├── BaseCan.h                       # CAN bus communication
│   ├── BaseWifi.h                      # WiFi networking
│   ├── BaseBluetooth.h                 # Bluetooth connectivity
│   ├── BaseNvs.h                       # Non-volatile storage
│   ├── BaseLogger.h                    # Unified logging system
│   ├── BaseTemperature.h               # Temperature sensing
│   ├── BasePeriodicTimer.h             # Timer operations
│   └── BasePio.h                       # Programmable I/O (advanced GPIO)
│
└── 🔧 MCU Layer (inc/mcu/ & src/mcu/)  # Platform implementations
    ├── esp32/                          # ESP32 family support
    │   ├── EspGpio.h/.cpp             # ESP32 GPIO implementation
    │   ├── EspAdc.h/.cpp              # ESP32 ADC implementation
    │   ├── EspPwm.h/.cpp              # ESP32 PWM implementation
    │   └── ...                        # All other ESP32 peripherals
    │
    ├── stm32/                          # STM32 family (future)
    │   └── ...                        # STM32 implementations
    │
    └── nrf/                            # Nordic nRF (future)
        └── ...                        # nRF implementations
```text

### **Abstraction Benefits**

#### **1. MCU Independence**
```cpp
// Application code remains the same across MCUs
class MotorController {
    BaseGpio* enable*pin;
    BasePwm* speed*control;
    BaseAdc* current*sensor;
    
public:
    void Initialize() {
        enable*pin = GpioFactory::Create(MOTOR*ENABLE*PIN, GPIO*OUTPUT);
        speed*control = PwmFactory::Create(PWM*CHANNEL*1, 1000); // 1kHz
        current*sensor = AdcFactory::Create(ADC*CHANNEL*1);
    }
    
    void SetSpeed(hf*u16*t speed*percent) {
        speed*control->SetDutyCycle(speed*percent);
    }
};
```text

#### **2. External Driver Support**
The base classes can be extended for external chips:
```cpp
// External motor driver chip
class DRV8302*Driver : public BasePwm {
    BaseI2c* i2c*bus;
    BaseSpi* spi*bus;
    
public:
    // Implement BasePwm interface using external chip
    void SetDutyCycle(hf*u16*t duty) override {
        // Send PWM command to DRV8302 via SPI/I2C
    }
};
```text

---

## 🔌 **Peripheral Interfaces**

### **Core Peripherals**

| **Interface** | **Purpose** | **Key Features** |

|---------------|-------------|------------------|

| **BaseGpio** | Digital I/O control | Input/output, interrupts, pull-up/down |

| **BaseAdc** | Analog measurement | Multi-channel, calibration, DMA support |

| **BasePwm** | Motor/servo control | Frequency control, duty cycle, phase alignment |

| **BaseUart** | Serial communication | Async I/O, flow control, custom baud rates |

### **Communication Buses**

| **Interface** | **Purpose** | **Key Features** |

|---------------|-------------|------------------|

| **BaseI2c** | Sensor communication | Master/slave, clock stretching, error recovery |

| **BaseSpi** | High-speed data | Full/half duplex, DMA, chip select management |

| **BaseCan** | Automotive/industrial | Message filtering, error handling, bus monitoring |

### **Wireless Connectivity**

| **Interface** | **Purpose** | **Key Features** |

|---------------|-------------|------------------|

| **BaseWifi** | Network connectivity | STA/AP modes, WPA3 security, power management |

| **BaseBluetooth** | Short-range wireless | Classic/BLE, pairing, service discovery |

### **System Services**

| **Interface** | **Purpose** | **Key Features** |

|---------------|-------------|------------------|

| **BaseNvs** | Configuration storage | Key-value pairs, encryption, wear leveling |

| **BaseLogger** | Debug/monitoring | Multiple levels, async logging, filtering |

| **BaseTemperature** | Thermal monitoring | Internal/external sensors, calibration |

| **BasePeriodicTimer** | Task scheduling | Precise timing, ISR-safe callbacks |

| **BasePio** | Advanced GPIO | State machines, DMA, complex protocols |

---

## 🖥️ **MCU Support**

### **Currently Supported**

#### **ESP32 Family** ✅
- **ESP32** - Original dual-core WiFi/BT
- **ESP32-C6** - RISC-V with WiFi 6 + Zigbee
- **ESP32-S3** - Dual-core with AI acceleration
- **ESP32-C3** - Single-core RISC-V WiFi/BT

**Implementation Status**: All 14 peripheral interfaces fully implemented

### **Planned Support**

#### **STM32 Family** 🚧
- **STM32F4** - High-performance ARM Cortex-M4
- **STM32H7** - Dual-core ARM Cortex-M7
- **STM32G4** - Motor control optimized

#### **Nordic nRF** 🚧
- **nRF52840** - Bluetooth 5.0 + Thread/Zigbee
- **nRF5340** - Dual-core Bluetooth 5.2

### **Adding New MCUs**

1. **Create MCU directory**: `inc/mcu/your*mcu/` and `src/mcu/your*mcu/`
2. **Implement base interfaces**: Inherit from base classes
3. **Add factory support**: Register your implementations
4. **Update build system**: Add MCU-specific compilation flags

```cpp
// Example: Adding STM32 GPIO support
class Stm32Gpio : public BaseGpio {
    GPIO*TypeDef* gpio*port;
    hf*u16*t gpio*pin;
    
public:
    void SetHigh() override {
        HAL*GPIO*WritePin(gpio*port, gpio*pin, GPIO*PIN*SET);
    }
    
    void SetLow() override {
        HAL*GPIO*WritePin(gpio*port, gpio*pin, GPIO*PIN*RESET);
    }
};
```text

---

## 🚀 **Quick Start**

### **1. Clone Repository**
```bash
git clone <repository-url>
cd hf-internal-interface-wrap
```text

### **2. Setup Development Environment**
```bash
## For ESP32 development
cd examples/esp32
./scripts/setup*repo.sh
```text

### **3. Build Example**
```bash
## Build GPIO test for ESP32
./scripts/build*app.sh gpio*test Release esp32
```text

### **4. Flash and Monitor**
```bash
## Flash to connected ESP32
./scripts/flash*app.sh gpio*test Release flash

## Monitor serial output
./scripts/flash*app.sh gpio*test Release monitor
```text

### **5. Basic Usage**
```cpp
#include "base/BaseGpio.h"
#include "mcu/esp32/EspGpio.h"

void setup() {
    // Create GPIO instance for built-in LED
    BaseGpio* led = new EspGpio(GPIO*NUM*2, GPIO*MODE*OUTPUT);
    
    // Blink LED
    while(true) {
        led->SetHigh();
        vTaskDelay(pdMS*TO*TICKS(500));
        led->SetLow();
        vTaskDelay(pdMS*TO*TICKS(500));
    }
}
```text

---

## 📖 **API Documentation**

### **Generated Documentation**
- **[API Reference](docs/api/)** - Complete interface documentation
- **[ESP32 Implementation](docs/esp_api/)** - ESP32-specific details

### **Key Concepts**

#### **Type System**
```cpp
// Project uses consistent type definitions
typedef uint8*t  hf*u8*t;
typedef uint16*t hf*u16*t;
typedef uint32*t hf*u32*t;

// Enums use snake*case with *t suffix
enum class hf*gpio*state*t {
    LOW = 0,
    HIGH = 1
};
```text

#### **Error Handling**
```cpp
// All operations return status codes
enum class hf*gpio*err*t {
    SUCCESS = 0,
    INVALID*PIN,
    ALREADY*CONFIGURED,
    HARDWARE*ERROR
};

hf*gpio*err*t result = gpio->Configure(GPIO*MODE*OUTPUT);
if (result != hf*gpio*err*t::SUCCESS) {
    Logger::GetInstance().LogError("GPIO configuration failed");
}
```text

#### **Factory Pattern**

The factory pattern enables completely MCU-agnostic code by automatically selecting the correct
implementation at compile time.
Factories support both **dynamic allocation** (heap-based) and **static allocation** (stack-based)
patterns:

```cpp
// inc/utils/GpioFactory.h - MCU-agnostic factory interface
class GpioFactory {
public:
    static BaseGpio* Create(hf*u8*t pin, gpio*mode*t mode);
    static BaseGpio* CreateWithInterrupt(hf*u8*t pin, gpio*isr*t callback);
    static void Destroy(BaseGpio* gpio);
};

// src/utils/GpioFactory.cpp - Automatic MCU selection
BaseGpio* GpioFactory::Create(hf*u8*t pin, gpio*mode*t mode) {
#ifdef MCU*ESP32
    return new EspGpio(static*cast<gpio*num*t>(pin), mode);
#elif defined(MCU*STM32)
    return new Stm32Gpio(pin, mode);
#elif defined(MCU*NRF)
    return new NrfGpio(pin, mode);
#else
    #error "Unsupported MCU platform"
#endif
}

// Application code - same across all MCUs
BaseGpio* led = GpioFactory::Create(GPIO*PIN*2, GPIO*OUTPUT);
BaseGpio* button = GpioFactory::CreateWithInterrupt(GPIO*PIN*0, button*callback);
```text

**Static Allocation Alternative:**

For systems requiring deterministic memory usage or avoiding heap allocation:

```cpp
// inc/utils/StaticGpioFactory.h - Stack-based allocation (C++23 compatible)
template<size*t MAX*GPIOS = 16>
class StaticGpioFactory {
private:
    // Modern C++23 approach: alignas + std::byte array instead of deprecated std::aligned*storage
    static std::array<alignas(BaseGpio) std::byte[sizeof(EspGpio)], MAX*GPIOS> gpio*storage;
    static std::array<bool, MAX*GPIOS> gpio*used;
    static size*t next*index;
    
public:
    static BaseGpio* Create(hf*u8*t pin, gpio*mode*t mode) {
        if (next*index >= MAX*GPIOS) return nullptr;
        
        // Construct in-place in pre-allocated storage using placement new
        void* storage = &gpio*storage[next*index];
        BaseGpio* gpio = nullptr;
        
#ifdef MCU*ESP32
        gpio = new(storage) EspGpio(static*cast<gpio*num*t>(pin), mode);
#elif defined(MCU*STM32)
        gpio = new(storage) Stm32Gpio(pin, mode);
#elif defined(MCU*NRF)
        gpio = new(storage) NrfGpio(pin, mode);
#endif
        
        gpio*used[next*index] = true;
        next*index++;
        return gpio;
    }
    
    static void DestroyAll() {
        for (size*t i = 0; i < next*index; ++i) {
            if (gpio*used[i]) {
                // Use std::launder for safe pointer conversion (C++17+)
                BaseGpio* gpio = std::launder(reinterpret*cast<BaseGpio*>(&gpio*storage[i]));
                gpio->~BaseGpio();  // Call destructor explicitly
                gpio*used[i] = false;
            }
        }
        next*index = 0;
    }
};

// Pre-allocated object pool for known hardware configuration
class HardwarePool {
private:
    // Modern C++23 approach: alignas + std::byte arrays instead of deprecated std::aligned*storage
    alignas(BaseGpio) std::byte motor*enable*storage[sizeof(EspGpio)];
    alignas(BaseGpio) std::byte fault*pin*storage[sizeof(EspGpio)];
    alignas(BasePwm) std::byte motor*pwm*storage[sizeof(EspPwm)];
    alignas(BaseAdc) std::byte current*adc*storage[sizeof(EspAdc)];
    
public:
    BaseGpio* motor*enable;
    BaseGpio* fault*pin;
    BasePwm* motor*pwm;
    BaseAdc* current*adc;
    
    // Constructor creates all objects in pre-allocated storage
    HardwarePool() {
#ifdef MCU*ESP32
        motor*enable = new(&motor*enable*storage) EspGpio(GPIO*NUM*5, GPIO*MODE*OUTPUT);
        fault*pin = new(&fault*pin*storage) EspGpio(GPIO*NUM*4, GPIO*MODE*INPUT);
        motor*pwm = new(&motor*pwm*storage) EspPwm(LEDC*CHANNEL*0, 20000, 12, GPIO*NUM*18);
        current*adc = new(&current*adc*storage) EspAdc(ADC1*CHANNEL*0);
#elif defined(MCU*STM32)
        motor*enable = new(&motor*enable*storage) Stm32Gpio(5, GPIO*MODE*OUTPUT);
        fault*pin = new(&fault*pin*storage) Stm32Gpio(4, GPIO*MODE*INPUT);
        motor*pwm = new(&motor*pwm*storage) Stm32Pwm(TIM1, 20000, 5);
        current*adc = new(&current*adc*storage) Stm32Adc(ADC1, 0);
#endif
    }
    
    // Destructor calls destructors explicitly
    ~HardwarePool() {
        if (motor*enable) motor*enable->~BaseGpio();
        if (fault*pin) fault*pin->~BaseGpio();
        if (motor*pwm) motor*pwm->~BasePwm();
        if (current*adc) current*adc->~BaseAdc();
    }
};
```text

**Usage Comparison:**

```cpp
// Dynamic allocation (heap-based)
class DynamicController {
    BaseGpio* motor*enable;
    BasePwm* motor*speed;
    
public:
    void Initialize() {
        motor*enable = GpioFactory::Create(GPIO*PIN*5, GPIO*OUTPUT);
        motor*speed = PwmFactory::CreateMotorControl(PWM*CH*0, GPIO*PIN*18);
    }
    
    ~DynamicController() {
        GpioFactory::Destroy(motor*enable);
        PwmFactory::Destroy(motor*speed);
    }
};

// Static allocation (stack-based, deterministic memory)
class StaticController {
    HardwarePool hardware;  // All objects created in constructor
    
public:
    void Initialize() {
        // Objects already created in hardware pool constructor
        // Just configure them
        hardware.motor*enable->SetHigh();
        hardware.motor*pwm->SetDutyCycle(0);
    }
    
    void RunMotor(hf*u16*t speed) {
        hardware.motor*pwm->SetDutyCycle(speed);
        hf*u16*t current = hardware.current*adc->ReadRaw();
        Logger::GetInstance().LogInfo("Motor speed: %d%%, Current: %d", speed, current);
    }
    
    // Destructor automatically called, no manual cleanup needed
};

// Real-time system with pre-allocated pool
void RealTimeTask() {
    static StaticGpioFactory<8> gpio*pool;  // Max 8 GPIOs, stack allocated
    
    BaseGpio* led1 = gpio*pool.Create(GPIO*PIN*2, GPIO*OUTPUT);
    BaseGpio* led2 = gpio*pool.Create(GPIO*PIN*3, GPIO*OUTPUT);
    BaseGpio* button = gpio*pool.Create(GPIO*PIN*0, GPIO*INPUT);
    
    // No heap allocation, deterministic timing
    while(true) {
        if (button->Read() == GPIO*HIGH) {
            led1->SetHigh();
            led2->SetLow();
        } else {
            led1->SetLow();
            led2->SetHigh();
        }
        vTaskDelay(pdMS*TO*TICKS(10));
    }
}
```text

**Memory Allocation Benefits:**

| **Allocation Type** | **Use Case** | **Benefits** | **Trade-offs** |

|---------------------|--------------|--------------|----------------|

| **Dynamic (Heap)** | Flexible systems | Easy to use, unlimited objects | Runtime allocation, fragmentation risk |

| **Static Pool** | Known hardware count | Deterministic memory, no fragmentation | Fixed object count, more setup code |

| **Pre-allocated** | Real-time systems | Constructor-based, automatic cleanup | Compile-time hardware definition |

**When to Use Each:**
- **Dynamic**: Prototyping, flexible configurations, plenty of RAM
- **Static Pool**: Real-time systems, safety-critical applications  
- **Pre-allocated**: Known hardware layout, maximum determinism

**C++23 Compatibility Note:**
The examples above use modern C++23 syntax with `alignas()` and `std::byte` arrays instead of the
deprecated `std::aligned*storage`.
As noted in
[P1413R3](https://stackoverflow.com/questions/71828288/why-is-stdaligned-storage-to-be-deprecated-in-c23-and-what-to-use-instead),
`std::aligned*storage` is deprecated due to API issues and potential undefined behavior.
The replacement pattern `alignas(T) std::byte[sizeof(T)]` provides the same functionality with
better type safety and constexpr compatibility.

**Advanced Factory Examples:**

```cpp
// PWM Factory with motor control optimization
class PwmFactory {
public:
    static BasePwm* CreateMotorControl(hf*u8*t channel, hf*u8*t gpio*pin) {
        // Automatically configures optimal settings for motor control
        // ESP32: 20kHz, 12-bit resolution
        // STM32: 20kHz, 16-bit resolution  
        // nRF: 20kHz, 10-bit resolution
    }
    
    static BasePwm* CreateServoControl(hf*u8*t channel, hf*u8*t gpio*pin) {
        // Automatically configures for servo control (50Hz, precise timing)
    }
};

// Communication Factory with bus management
class CommFactory {
public:
    static BaseI2c* CreateSensorBus(hf*u8*t bus*num) {
        // Optimized I2C settings for sensor communication
        // Handles MCU-specific pin assignments automatically
    }
    
    static BaseUart* CreateDebugPort() {
        // Creates standard debug UART on each MCU's debug pins
        // ESP32: UART0 on GPIO1/3
        // STM32: USART2 on PA2/PA3
        // nRF: UART0 on P0.06/P0.08
    }
};
```text

---

## 🔧 **Building**

### **Build System Features**
- **Multi-MCU Support** - Single build system for all platforms
- **Automated Testing** - Comprehensive test suites
- **CI/CD Integration** - Automated builds and validation

### **Build Commands**
```bash
## Build specific application
./scripts/build*app.sh <app*name> <build*type> <target>

## Examples:
./scripts/build*app.sh gpio*test Debug esp32
./scripts/build*app.sh pwm*test Release esp32c6
./scripts/build*app.sh uart*test Debug esp32s3
```yaml

### **Configuration**
Applications are configured in `examples/esp32/app*config.yml`:
```yaml
applications:
  gpio*test:
    source*file: "GpioComprehensiveTest.cpp"
    description: "GPIO interface testing"
    enabled: true
    
  pwm*test:
    source*file: "PwmComprehensiveTest.cpp" 
    description: "PWM interface testing"
    enabled: true
```text

---

## 📊 **Examples**

### **Available Test Applications**

| **Application** | **Tests** | **Purpose** |

|-----------------|-----------|-------------|

| **gpio*test** | Digital I/O, interrupts | GPIO interface validation |

| **adc*test** | Multi-channel sampling | ADC accuracy and performance |

| **pwm*test** | Frequency/duty control | Motor control applications |

| **uart*test** | Serial communication | Data transmission testing |

| **i2c*test** | Sensor communication | I2C bus operations |

| **spi*test** | High-speed data | SPI protocol testing |

| **wifi*test** | Network connectivity | WiFi stack validation |

| **bluetooth*test** | Wireless pairing | Bluetooth functionality |

### **Factory Usage Examples**

#### **Basic Factory Usage**
```cpp
// Simple GPIO control - works on any MCU
void BlinkLED() {
    BaseGpio* led = GpioFactory::Create(GPIO*PIN*2, GPIO*OUTPUT);
    
    while(true) {
        led->SetHigh();
        vTaskDelay(pdMS*TO*TICKS(500));
        led->SetLow(); 
        vTaskDelay(pdMS*TO*TICKS(500));
    }
    
    GpioFactory::Destroy(led);
}

// PWM motor control - MCU-optimized automatically
void ControlMotor() {
    BasePwm* motor = PwmFactory::CreateMotorControl(PWM*CH*0, GPIO*PIN*5);
    BaseAdc* current = AdcFactory::Create(ADC*CHANNEL*1);
    
    motor->SetDutyCycle(75);  // 75% speed
    hf*u16*t current*ma = current->ReadMillivolts() / 10;  // Convert to mA
    
    Logger::GetInstance().LogInfo("Motor current: %d mA", current*ma);
}
```text

#### **Multi-Peripheral Application**
```cpp
class HardFOCController {
    // Hardware interfaces - MCU agnostic
    BaseGpio* motor*enable;
    BaseGpio* fault*pin;
    BasePwm* motor*speed;
    BaseAdc* current*sensor;
    BaseAdc* voltage*sensor;
    BaseUart* debug*port;
    BaseI2c* sensor*bus;
    BaseWifi* telemetry;
    
public:
    hf*gpio*err*t Initialize() {
        // Factory creates MCU-specific implementations automatically
        motor*enable = GpioFactory::Create(MOTOR*EN*PIN, GPIO*OUTPUT);
        fault*pin = GpioFactory::CreateWithInterrupt(FAULT*PIN, fault*callback);
        motor*speed = PwmFactory::CreateMotorControl(PWM*CH*0, MOTOR*PWM*PIN);
        current*sensor = AdcFactory::Create(CURRENT*ADC*CH);
        voltage*sensor = AdcFactory::Create(VOLTAGE*ADC*CH);
        debug*port = CommFactory::CreateDebugPort();
        sensor*bus = CommFactory::CreateSensorBus(I2C*BUS*0);
        telemetry = WifiFactory::Create();
        
        // Validate all interfaces created successfully
        if (!motor*enable || !motor*speed || !current*sensor) {
            return hf*gpio*err*t::HARDWARE*ERROR;
        }
        
        debug*port->Printf("HardFOC Controller initialized on %s\n", MCU*NAME);
        return hf*gpio*err*t::SUCCESS;
    }
    
    void RunMotor(hf*u16*t speed*percent) {
        // Enable motor driver
        motor*enable->SetHigh();
        
        // Set motor speed
        motor*speed->SetDutyCycle(speed*percent);
        
        // Read diagnostics
        hf*u16*t current*ma = current*sensor->ReadMillivolts() / 10;
        hf*u16*t voltage*mv = voltage*sensor->ReadMillivolts();
        
        // Log locally
        debug*port->Printf("Speed: %d%%, Current: %dmA, Voltage: %dmV\n", 
                          speed*percent, current*ma, voltage*mv);
        
        // Send telemetry if connected
        if (telemetry && telemetry->IsConnected()) {
            telemetry->SendData("motor*speed", speed*percent);
            telemetry->SendData("motor*current", current*ma);
            telemetry->SendData("bus*voltage", voltage*mv);
        }
    }
    
    void EmergencyStop() {
        motor*enable->SetLow();
        motor*speed->SetDutyCycle(0);
        debug*port->Printf("EMERGENCY STOP - Motor disabled\n");
    }
    
    ~HardFOCController() {
        // Clean up all resources
        EmergencyStop();
        GpioFactory::Destroy(motor*enable);
        GpioFactory::Destroy(fault*pin);
        PwmFactory::Destroy(motor*speed);
        AdcFactory::Destroy(current*sensor);
        AdcFactory::Destroy(voltage*sensor);
        CommFactory::DestroyComm(debug*port);
        CommFactory::DestroyComm(sensor*bus);
        WifiFactory::Destroy(telemetry);
    }
};

// Same code compiles and runs on ESP32, STM32, nRF!
HardFOCController controller;
controller.Initialize();
controller.RunMotor(50);  // 50% speed
```text

#### **External Sensor Integration**
```cpp
// Factories can create drivers for external chips too
class EnvironmentalMonitor {
    BaseTemperature* internal*temp;    // MCU internal sensor
    BaseTemperature* external*temp;    // DS18B20 external sensor
    BaseAdc* external*adc;             // MCP3208 external ADC
    
public:
    void Initialize() {
        // Internal MCU sensors
        internal*temp = SensorFactory::CreateInternalTemp();
        
        // External sensors using base interfaces
        BaseGpio* ds18b20*pin = GpioFactory::Create(GPIO*PIN*4, GPIO*INPUT*OUTPUT);
        external*temp = SensorFactory::CreateDS18B20(ds18b20*pin);
        
        BaseSpi* spi*bus = CommFactory::CreateSpi(SPI*BUS*1, 1000000);
        BaseGpio* cs*pin = GpioFactory::Create(GPIO*PIN*10, GPIO*OUTPUT);
        external*adc = SensorFactory::CreateMCP3208(spi*bus, cs*pin);
    }
    
    void ReadAllSensors() {
        float internal*celsius = internal*temp->ReadCelsius();
        float external*celsius = external*temp->ReadCelsius();
        hf*u16*t external*raw = external*adc->ReadRaw();
        
        Logger::GetInstance().LogInfo("Temps: Internal=%.1f°C, External=%.1f°C, ADC=%d", 
                                     internal*celsius, external*celsius, external*raw);
    }
};
```text

---

## 🤝 **Contributing**

### **Development Workflow**
1. **Fork** the repository
2. **Create** feature branch (`feature/new-mcu-support`)
3. **Implement** following coding standards
4. **Test** with existing applications
5. **Document** your changes
6. **Submit** pull request

### **Adding New Peripherals**
1. **Create base interface** in `inc/base/BaseYourPeripheral.h`
2. **Implement for ESP32** in `inc/mcu/esp32/EspYourPeripheral.h`
3. **Add comprehensive tests** in `examples/esp32/main/`
4. **Update documentation**

### **Coding Standards**
- **Functions**: CamelCase (`SetDutyCycle`, `ReadValue`)
- **Types**: snake*case with `*t` suffix (`hf*gpio*state*t`)
- **Enums**: snake*case enum class (`hf*adc*err*t`)
- **Logging**: Use `Logger::GetInstance()` for all output

---

## 📄 **License**

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for full details.

---

## 🔗 **Quick Links**

### **Documentation**
- 📚 [API Reference](docs/api/) - Complete interface documentation
- 🔧 [ESP32 Implementations](docs/esp_api/) - Hardware-specific implementations
- 🛠️ [Utility Classes](docs/utils/) - Advanced utility classes and helpers
- 🔧 [Build System](examples/esp32/) - Build and deployment guides
- 🛡️ [CI/CD Pipeline](.github/workflows/) - Advanced automated workflows and testing

### **Development**
- 🚀 [Examples](examples/esp32/) - Test applications and usage examples
- 🧪 [Test Documentation](examples/esp32/docs/README.md) - Comprehensive test documentation
- 🔧 [Scripts](examples/esp32/scripts/) - Build, flash, and development tools
- 📊 [Configuration](examples/esp32/app_config.yml) - Application and build settings

### **Community**
- 🤝 [Contributing](CONTRIBUTING.md) - Development guidelines

---

<div align="center">

**Built for the HardFOC ecosystem - Enabling seamless MCU portability**

*Hardware abstraction that just works™*

</div>
