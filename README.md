# 🔧 Hardware Abstraction Layer - Multi-MCU Peripheral Interface

<div align="center">

![HAL](https://img.shields.io/badge/HAL-Hardware%20Abstraction%20Layer-blue?style=for-the-badge&logo=microchip)
![C++17](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![Multi-MCU](https://img.shields.io/badge/Multi--MCU-Support-green?style=for-the-badge&logo=espressif)
![License](https://img.shields.io/badge/License-GPL--3.0-green?style=for-the-badge&logo=opensourceinitiative)

**🎯 Universal Hardware Interface for Multi-MCU Development**

*A professional hardware abstraction layer enabling seamless MCU portability through unified peripheral APIs - designed for the HardFOC board ecosystem*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#️-architecture)
- [🔌 **Peripheral Interfaces**](#-peripheral-interfaces)
- [🖥️ **MCU Support**](#️-mcu-support)
- [🚀 **Quick Start**](#-quick-start)
- [📖 **API Documentation**](#-api-documentation)
- [🔧 **Building**](#-building)
- [📊 **Examples**](#-examples)
- [🤝 **Contributing**](#-contributing)
- [📄 **License**](#-license)

---

## 🎯 **Overview**

This **Internal Interface Wrap (IID)** provides a unified interface for common MCU peripherals, enabling seamless portability between different microcontroller platforms. Originally designed for the **HardFOC board** which needs to support multiple MCU types, this abstraction layer allows developers to write hardware-agnostic code while maintaining optimal performance.

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
BaseGpio* led = GpioFactory::Create(GPIO_PIN_2, GPIO_OUTPUT);
led->SetHigh();

// Same code works on ESP32, STM32, or any supported MCU
// The factory handles MCU-specific implementation selection
```

---

## 🏗️ **Architecture**

### **Two-Layer Design**

```
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
```

### **Abstraction Benefits**

#### **1. MCU Independence**
```cpp
// Application code remains the same across MCUs
class MotorController {
    BaseGpio* enable_pin;
    BasePwm* speed_control;
    BaseAdc* current_sensor;
    
public:
    void Initialize() {
        enable_pin = GpioFactory::Create(MOTOR_ENABLE_PIN, GPIO_OUTPUT);
        speed_control = PwmFactory::Create(PWM_CHANNEL_1, 1000); // 1kHz
        current_sensor = AdcFactory::Create(ADC_CHANNEL_1);
    }
    
    void SetSpeed(hf_u16_t speed_percent) {
        speed_control->SetDutyCycle(speed_percent);
    }
};
```

#### **2. External Driver Support**
The base classes can be extended for external chips:
```cpp
// External motor driver chip
class DRV8302_Driver : public BasePwm {
    BaseI2c* i2c_bus;
    BaseSpi* spi_bus;
    
public:
    // Implement BasePwm interface using external chip
    void SetDutyCycle(hf_u16_t duty) override {
        // Send PWM command to DRV8302 via SPI/I2C
    }
};
```

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

1. **Create MCU directory**: `inc/mcu/your_mcu/` and `src/mcu/your_mcu/`
2. **Implement base interfaces**: Inherit from base classes
3. **Add factory support**: Register your implementations
4. **Update build system**: Add MCU-specific compilation flags

```cpp
// Example: Adding STM32 GPIO support
class Stm32Gpio : public BaseGpio {
    GPIO_TypeDef* gpio_port;
    hf_u16_t gpio_pin;
    
public:
    void SetHigh() override {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    }
    
    void SetLow() override {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    }
};
```

---

## 🚀 **Quick Start**

### **1. Clone Repository**
```bash
git clone https://github.com/your-repo/hf-internal-interface-wrap.git
cd hf-internal-interface-wrap
```

### **2. Setup Development Environment**
```bash
# For ESP32 development
cd examples/esp32
./scripts/setup_repo.sh
```

### **3. Build Example**
```bash
# Build GPIO test for ESP32
./scripts/build_app.sh gpio_test Release esp32
```

### **4. Flash and Monitor**
```bash
# Flash to connected ESP32
./scripts/flash_app.sh gpio_test Release flash

# Monitor serial output
./scripts/flash_app.sh gpio_test Release monitor
```

### **5. Basic Usage**
```cpp
#include "base/BaseGpio.h"
#include "mcu/esp32/EspGpio.h"

void setup() {
    // Create GPIO instance for built-in LED
    BaseGpio* led = new EspGpio(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    
    // Blink LED
    while(true) {
        led->SetHigh();
        vTaskDelay(pdMS_TO_TICKS(500));
        led->SetLow();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

---

## 📖 **API Documentation**

### **Generated Documentation**
- **[API Reference](docs/api/)** - Complete interface documentation
- **[ESP32 Implementation](docs/esp_api/)** - ESP32-specific details

### **Key Concepts**

#### **Type System**
```cpp
// Project uses consistent type definitions
typedef uint8_t  hf_u8_t;
typedef uint16_t hf_u16_t;
typedef uint32_t hf_u32_t;

// Enums use snake_case with _t suffix
enum class hf_gpio_state_t {
    LOW = 0,
    HIGH = 1
};
```

#### **Error Handling**
```cpp
// All operations return status codes
enum class hf_gpio_err_t {
    SUCCESS = 0,
    INVALID_PIN,
    ALREADY_CONFIGURED,
    HARDWARE_ERROR
};

hf_gpio_err_t result = gpio->Configure(GPIO_MODE_OUTPUT);
if (result != hf_gpio_err_t::SUCCESS) {
    Logger::GetInstance().LogError("GPIO configuration failed");
}
```

#### **Factory Pattern**

The factory pattern enables completely MCU-agnostic code by automatically selecting the correct implementation at compile time. Factories support both **dynamic allocation** (heap-based) and **static allocation** (stack-based) patterns:

```cpp
// inc/utils/GpioFactory.h - MCU-agnostic factory interface
class GpioFactory {
public:
    static BaseGpio* Create(hf_u8_t pin, gpio_mode_t mode);
    static BaseGpio* CreateWithInterrupt(hf_u8_t pin, gpio_isr_t callback);
    static void Destroy(BaseGpio* gpio);
};

// src/utils/GpioFactory.cpp - Automatic MCU selection
BaseGpio* GpioFactory::Create(hf_u8_t pin, gpio_mode_t mode) {
#ifdef MCU_ESP32
    return new EspGpio(static_cast<gpio_num_t>(pin), mode);
#elif defined(MCU_STM32)
    return new Stm32Gpio(pin, mode);
#elif defined(MCU_NRF)
    return new NrfGpio(pin, mode);
#else
    #error "Unsupported MCU platform"
#endif
}

// Application code - same across all MCUs
BaseGpio* led = GpioFactory::Create(GPIO_PIN_2, GPIO_OUTPUT);
BaseGpio* button = GpioFactory::CreateWithInterrupt(GPIO_PIN_0, button_callback);
```

**Static Allocation Alternative:**

For systems requiring deterministic memory usage or avoiding heap allocation:

```cpp
// inc/utils/StaticGpioFactory.h - Stack-based allocation (C++23 compatible)
template<size_t MAX_GPIOS = 16>
class StaticGpioFactory {
private:
    // Modern C++23 approach: alignas + std::byte array instead of deprecated std::aligned_storage
    static std::array<alignas(BaseGpio) std::byte[sizeof(EspGpio)], MAX_GPIOS> gpio_storage;
    static std::array<bool, MAX_GPIOS> gpio_used;
    static size_t next_index;
    
public:
    static BaseGpio* Create(hf_u8_t pin, gpio_mode_t mode) {
        if (next_index >= MAX_GPIOS) return nullptr;
        
        // Construct in-place in pre-allocated storage using placement new
        void* storage = &gpio_storage[next_index];
        BaseGpio* gpio = nullptr;
        
#ifdef MCU_ESP32
        gpio = new(storage) EspGpio(static_cast<gpio_num_t>(pin), mode);
#elif defined(MCU_STM32)
        gpio = new(storage) Stm32Gpio(pin, mode);
#elif defined(MCU_NRF)
        gpio = new(storage) NrfGpio(pin, mode);
#endif
        
        gpio_used[next_index] = true;
        next_index++;
        return gpio;
    }
    
    static void DestroyAll() {
        for (size_t i = 0; i < next_index; ++i) {
            if (gpio_used[i]) {
                // Use std::launder for safe pointer conversion (C++17+)
                BaseGpio* gpio = std::launder(reinterpret_cast<BaseGpio*>(&gpio_storage[i]));
                gpio->~BaseGpio();  // Call destructor explicitly
                gpio_used[i] = false;
            }
        }
        next_index = 0;
    }
};

// Pre-allocated object pool for known hardware configuration
class HardwarePool {
private:
    // Modern C++23 approach: alignas + std::byte arrays instead of deprecated std::aligned_storage
    alignas(BaseGpio) std::byte motor_enable_storage[sizeof(EspGpio)];
    alignas(BaseGpio) std::byte fault_pin_storage[sizeof(EspGpio)];
    alignas(BasePwm) std::byte motor_pwm_storage[sizeof(EspPwm)];
    alignas(BaseAdc) std::byte current_adc_storage[sizeof(EspAdc)];
    
public:
    BaseGpio* motor_enable;
    BaseGpio* fault_pin;
    BasePwm* motor_pwm;
    BaseAdc* current_adc;
    
    // Constructor creates all objects in pre-allocated storage
    HardwarePool() {
#ifdef MCU_ESP32
        motor_enable = new(&motor_enable_storage) EspGpio(GPIO_NUM_5, GPIO_MODE_OUTPUT);
        fault_pin = new(&fault_pin_storage) EspGpio(GPIO_NUM_4, GPIO_MODE_INPUT);
        motor_pwm = new(&motor_pwm_storage) EspPwm(LEDC_CHANNEL_0, 20000, 12, GPIO_NUM_18);
        current_adc = new(&current_adc_storage) EspAdc(ADC1_CHANNEL_0);
#elif defined(MCU_STM32)
        motor_enable = new(&motor_enable_storage) Stm32Gpio(5, GPIO_MODE_OUTPUT);
        fault_pin = new(&fault_pin_storage) Stm32Gpio(4, GPIO_MODE_INPUT);
        motor_pwm = new(&motor_pwm_storage) Stm32Pwm(TIM1, 20000, 5);
        current_adc = new(&current_adc_storage) Stm32Adc(ADC1, 0);
#endif
    }
    
    // Destructor calls destructors explicitly
    ~HardwarePool() {
        if (motor_enable) motor_enable->~BaseGpio();
        if (fault_pin) fault_pin->~BaseGpio();
        if (motor_pwm) motor_pwm->~BasePwm();
        if (current_adc) current_adc->~BaseAdc();
    }
};
```

**Usage Comparison:**

```cpp
// Dynamic allocation (heap-based)
class DynamicController {
    BaseGpio* motor_enable;
    BasePwm* motor_speed;
    
public:
    void Initialize() {
        motor_enable = GpioFactory::Create(GPIO_PIN_5, GPIO_OUTPUT);
        motor_speed = PwmFactory::CreateMotorControl(PWM_CH_0, GPIO_PIN_18);
    }
    
    ~DynamicController() {
        GpioFactory::Destroy(motor_enable);
        PwmFactory::Destroy(motor_speed);
    }
};

// Static allocation (stack-based, deterministic memory)
class StaticController {
    HardwarePool hardware;  // All objects created in constructor
    
public:
    void Initialize() {
        // Objects already created in hardware pool constructor
        // Just configure them
        hardware.motor_enable->SetHigh();
        hardware.motor_pwm->SetDutyCycle(0);
    }
    
    void RunMotor(hf_u16_t speed) {
        hardware.motor_pwm->SetDutyCycle(speed);
        hf_u16_t current = hardware.current_adc->ReadRaw();
        Logger::GetInstance().LogInfo("Motor speed: %d%%, Current: %d", speed, current);
    }
    
    // Destructor automatically called, no manual cleanup needed
};

// Real-time system with pre-allocated pool
void RealTimeTask() {
    static StaticGpioFactory<8> gpio_pool;  // Max 8 GPIOs, stack allocated
    
    BaseGpio* led1 = gpio_pool.Create(GPIO_PIN_2, GPIO_OUTPUT);
    BaseGpio* led2 = gpio_pool.Create(GPIO_PIN_3, GPIO_OUTPUT);
    BaseGpio* button = gpio_pool.Create(GPIO_PIN_0, GPIO_INPUT);
    
    // No heap allocation, deterministic timing
    while(true) {
        if (button->Read() == GPIO_HIGH) {
            led1->SetHigh();
            led2->SetLow();
        } else {
            led1->SetLow();
            led2->SetHigh();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

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
The examples above use modern C++23 syntax with `alignas()` and `std::byte` arrays instead of the deprecated `std::aligned_storage`. As noted in [P1413R3](https://stackoverflow.com/questions/71828288/why-is-stdaligned-storage-to-be-deprecated-in-c23-and-what-to-use-instead), `std::aligned_storage` is deprecated due to API issues and potential undefined behavior. The replacement pattern `alignas(T) std::byte[sizeof(T)]` provides the same functionality with better type safety and constexpr compatibility.

**Advanced Factory Examples:**

```cpp
// PWM Factory with motor control optimization
class PwmFactory {
public:
    static BasePwm* CreateMotorControl(hf_u8_t channel, hf_u8_t gpio_pin) {
        // Automatically configures optimal settings for motor control
        // ESP32: 20kHz, 12-bit resolution
        // STM32: 20kHz, 16-bit resolution  
        // nRF: 20kHz, 10-bit resolution
    }
    
    static BasePwm* CreateServoControl(hf_u8_t channel, hf_u8_t gpio_pin) {
        // Automatically configures for servo control (50Hz, precise timing)
    }
};

// Communication Factory with bus management
class CommFactory {
public:
    static BaseI2c* CreateSensorBus(hf_u8_t bus_num) {
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
```

---

## 🔧 **Building**

### **Build System Features**
- **Multi-MCU Support** - Single build system for all platforms
- **Automated Testing** - Comprehensive test suites
- **CI/CD Integration** - Automated builds and validation

### **Build Commands**
```bash
# Build specific application
./scripts/build_app.sh <app_name> <build_type> <target>

# Examples:
./scripts/build_app.sh gpio_test Debug esp32
./scripts/build_app.sh pwm_test Release esp32c6
./scripts/build_app.sh uart_test Debug esp32s3
```

### **Configuration**
Applications are configured in `examples/esp32/app_config.yml`:
```yaml
applications:
  gpio_test:
    source_file: "GpioComprehensiveTest.cpp"
    description: "GPIO interface testing"
    enabled: true
    
  pwm_test:
    source_file: "PwmComprehensiveTest.cpp" 
    description: "PWM interface testing"
    enabled: true
```

---

## 📊 **Examples**

### **Available Test Applications**

| **Application** | **Tests** | **Purpose** |
|-----------------|-----------|-------------|
| **gpio_test** | Digital I/O, interrupts | GPIO interface validation |
| **adc_test** | Multi-channel sampling | ADC accuracy and performance |
| **pwm_test** | Frequency/duty control | Motor control applications |
| **uart_test** | Serial communication | Data transmission testing |
| **i2c_test** | Sensor communication | I2C bus operations |
| **spi_test** | High-speed data | SPI protocol testing |
| **wifi_test** | Network connectivity | WiFi stack validation |
| **bluetooth_test** | Wireless pairing | Bluetooth functionality |

### **Factory Usage Examples**

#### **Basic Factory Usage**
```cpp
// Simple GPIO control - works on any MCU
void BlinkLED() {
    BaseGpio* led = GpioFactory::Create(GPIO_PIN_2, GPIO_OUTPUT);
    
    while(true) {
        led->SetHigh();
        vTaskDelay(pdMS_TO_TICKS(500));
        led->SetLow(); 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    GpioFactory::Destroy(led);
}

// PWM motor control - MCU-optimized automatically
void ControlMotor() {
    BasePwm* motor = PwmFactory::CreateMotorControl(PWM_CH_0, GPIO_PIN_5);
    BaseAdc* current = AdcFactory::Create(ADC_CHANNEL_1);
    
    motor->SetDutyCycle(75);  // 75% speed
    hf_u16_t current_ma = current->ReadMillivolts() / 10;  // Convert to mA
    
    Logger::GetInstance().LogInfo("Motor current: %d mA", current_ma);
}
```

#### **Multi-Peripheral Application**
```cpp
class HardFOCController {
    // Hardware interfaces - MCU agnostic
    BaseGpio* motor_enable;
    BaseGpio* fault_pin;
    BasePwm* motor_speed;
    BaseAdc* current_sensor;
    BaseAdc* voltage_sensor;
    BaseUart* debug_port;
    BaseI2c* sensor_bus;
    BaseWifi* telemetry;
    
public:
    hf_gpio_err_t Initialize() {
        // Factory creates MCU-specific implementations automatically
        motor_enable = GpioFactory::Create(MOTOR_EN_PIN, GPIO_OUTPUT);
        fault_pin = GpioFactory::CreateWithInterrupt(FAULT_PIN, fault_callback);
        motor_speed = PwmFactory::CreateMotorControl(PWM_CH_0, MOTOR_PWM_PIN);
        current_sensor = AdcFactory::Create(CURRENT_ADC_CH);
        voltage_sensor = AdcFactory::Create(VOLTAGE_ADC_CH);
        debug_port = CommFactory::CreateDebugPort();
        sensor_bus = CommFactory::CreateSensorBus(I2C_BUS_0);
        telemetry = WifiFactory::Create();
        
        // Validate all interfaces created successfully
        if (!motor_enable || !motor_speed || !current_sensor) {
            return hf_gpio_err_t::HARDWARE_ERROR;
        }
        
        debug_port->Printf("HardFOC Controller initialized on %s\n", MCU_NAME);
        return hf_gpio_err_t::SUCCESS;
    }
    
    void RunMotor(hf_u16_t speed_percent) {
        // Enable motor driver
        motor_enable->SetHigh();
        
        // Set motor speed
        motor_speed->SetDutyCycle(speed_percent);
        
        // Read diagnostics
        hf_u16_t current_ma = current_sensor->ReadMillivolts() / 10;
        hf_u16_t voltage_mv = voltage_sensor->ReadMillivolts();
        
        // Log locally
        debug_port->Printf("Speed: %d%%, Current: %dmA, Voltage: %dmV\n", 
                          speed_percent, current_ma, voltage_mv);
        
        // Send telemetry if connected
        if (telemetry && telemetry->IsConnected()) {
            telemetry->SendData("motor_speed", speed_percent);
            telemetry->SendData("motor_current", current_ma);
            telemetry->SendData("bus_voltage", voltage_mv);
        }
    }
    
    void EmergencyStop() {
        motor_enable->SetLow();
        motor_speed->SetDutyCycle(0);
        debug_port->Printf("EMERGENCY STOP - Motor disabled\n");
    }
    
    ~HardFOCController() {
        // Clean up all resources
        EmergencyStop();
        GpioFactory::Destroy(motor_enable);
        GpioFactory::Destroy(fault_pin);
        PwmFactory::Destroy(motor_speed);
        AdcFactory::Destroy(current_sensor);
        AdcFactory::Destroy(voltage_sensor);
        CommFactory::DestroyComm(debug_port);
        CommFactory::DestroyComm(sensor_bus);
        WifiFactory::Destroy(telemetry);
    }
};

// Same code compiles and runs on ESP32, STM32, nRF!
HardFOCController controller;
controller.Initialize();
controller.RunMotor(50);  // 50% speed
```

#### **External Sensor Integration**
```cpp
// Factories can create drivers for external chips too
class EnvironmentalMonitor {
    BaseTemperature* internal_temp;    // MCU internal sensor
    BaseTemperature* external_temp;    // DS18B20 external sensor
    BaseAdc* external_adc;             // MCP3208 external ADC
    
public:
    void Initialize() {
        // Internal MCU sensors
        internal_temp = SensorFactory::CreateInternalTemp();
        
        // External sensors using base interfaces
        BaseGpio* ds18b20_pin = GpioFactory::Create(GPIO_PIN_4, GPIO_INPUT_OUTPUT);
        external_temp = SensorFactory::CreateDS18B20(ds18b20_pin);
        
        BaseSpi* spi_bus = CommFactory::CreateSpi(SPI_BUS_1, 1000000);
        BaseGpio* cs_pin = GpioFactory::Create(GPIO_PIN_10, GPIO_OUTPUT);
        external_adc = SensorFactory::CreateMCP3208(spi_bus, cs_pin);
    }
    
    void ReadAllSensors() {
        float internal_celsius = internal_temp->ReadCelsius();
        float external_celsius = external_temp->ReadCelsius();
        hf_u16_t external_raw = external_adc->ReadRaw();
        
        Logger::GetInstance().LogInfo("Temps: Internal=%.1f°C, External=%.1f°C, ADC=%d", 
                                     internal_celsius, external_celsius, external_raw);
    }
};
```

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
- **Types**: snake_case with `_t` suffix (`hf_gpio_state_t`)
- **Enums**: snake_case enum class (`hf_adc_err_t`)
- **Logging**: Use `Logger::GetInstance()` for all output

---

## 📄 **License**

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for full details.

---

## 🔗 **Quick Links**

### **Documentation**
- 📚 [API Reference](docs/api/) - Complete interface documentation
- 🔧 [Build System](examples/esp32/scripts/docs/) - Build and deployment guides
- 🛡️ [CI/CD Pipeline](.github/workflows/README.md) - Automated workflows and testing

### **Development**
- 🚀 [Examples](examples/esp32/) - Test applications and usage examples
- 🔧 [Scripts](examples/esp32/scripts/) - Build, flash, and development tools
- 📊 [Configuration](examples/esp32/app_config.yml) - Application and build settings

### **Community**
- 🐛 [Issues](https://github.com/your-repo/hf-internal-interface-wrap/issues) - Bug reports and feature requests
- 💬 [Discussions](https://github.com/your-repo/hf-internal-interface-wrap/discussions) - Community support
- 🤝 [Contributing](CONTRIBUTING.md) - Development guidelines

---

<div align="center">

**Built for the HardFOC ecosystem - Enabling seamless MCU portability**

*Hardware abstraction that just works™*

</div>