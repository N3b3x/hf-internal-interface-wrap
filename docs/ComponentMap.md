# 🗺️ HardFOC Internal Interface Wrapper - Component Map

<div align="center">

![Component Map](https://img.shields.io/badge/Component%20Map-System%20Architecture-blue?style=for-the-badge&logo=mindmeister)

**🏗️ Visual guide to all components and their relationships in the HardFOC system**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture Layers**](#️-architecture-layers)
- [📊 **Component Matrix**](#-component-matrix)
- [🔌 **Interface Implementations**](#-interface-implementations)
- [🔄 **Data Flow**](#-data-flow)
- [📁 **File Structure**](#-file-structure)

---

## 🎯 **Overview**

The HardFOC Internal Interface Wrapper is organized in a layered architecture with clear separation of concerns. This component map provides a comprehensive view of all components and their relationships.

### 🌟 **Key Principles**

- **🔌 Abstraction**: Base classes define common interfaces
- **🏗️ Implementation**: Platform-specific implementations
- **🔒 Thread Safety**: Optional thread-safe wrappers
- **🎯 Type Safety**: Consistent type wrapping system

---

## 🏗️ **Architecture Layers**

```mermaid
graph TB
    subgraph "🎯 Application Layer"
        APP[Your Application Code]
    end
    
    subgraph "🔒 Thread-Safe Layer"
        SF_GPIO[SfGpio]
        SF_ADC[SfAdc]
        SF_I2C[SfI2cBus]
        SF_SPI[SfSpiBus]
        SF_UART[SfUartDriver]
        SF_CAN[SfCan]
        SF_PWM[SfPwm]
    end
    
    subgraph "🏛️ Abstract Base Layer"
        BASE_GPIO[BaseGpio]
        BASE_ADC[BaseAdc]
        BASE_I2C[BaseI2c]
        BASE_SPI[BaseSpi]
        BASE_UART[BaseUart]
        BASE_CAN[BaseCan]
        BASE_PWM[BasePwm]
        BASE_PIO[BasePio]
        BASE_NVS[BaseNvs]
        BASE_TIMER[BasePeriodicTimer]
    end
    
    subgraph "⚙️ MCU Implementation Layer"
        ESP_GPIO[EspGpio]
        ESP_ADC[EspAdc]
        ESP_I2C[EspI2c]
        ESP_SPI[EspSpi]
        ESP_UART[EspUart]
        ESP_CAN[EspCan]
        ESP_PWM[EspPwm]
        ESP_PIO[EspPio]
        ESP_NVS[EspNvs]
        ESP_TIMER[EspPeriodicTimer]
    end
    
    subgraph "🔧 Hardware Layer"
        HW[ESP32-C6 Hardware]
    end
    
    APP --> SF_GPIO
    APP --> SF_ADC
    APP --> SF_I2C
    APP --> SF_SPI
    APP --> SF_UART
    APP --> SF_CAN
    APP --> SF_PWM
    
    SF_GPIO --> BASE_GPIO
    SF_ADC --> BASE_ADC
    SF_I2C --> BASE_I2C
    SF_SPI --> BASE_SPI
    SF_UART --> BASE_UART
    SF_CAN --> BASE_CAN
    SF_PWM --> BASE_PWM
    
    BASE_GPIO --> ESP_GPIO
    BASE_ADC --> ESP_ADC
    BASE_I2C --> ESP_I2C
    BASE_SPI --> ESP_SPI
    BASE_UART --> ESP_UART
    BASE_CAN --> ESP_CAN
    BASE_PWM --> ESP_PWM
    BASE_PIO --> ESP_PIO
    BASE_NVS --> ESP_NVS
    BASE_TIMER --> ESP_TIMER
    
    ESP_GPIO --> HW
    ESP_ADC --> HW
    ESP_I2C --> HW
    ESP_SPI --> HW
    ESP_UART --> HW
    ESP_CAN --> HW
    ESP_PWM --> HW
    ESP_PIO --> HW
    ESP_NVS --> HW
    ESP_TIMER --> HW
```

---

## 📊 **Component Matrix**

### 🏛️ **Base Classes** (Abstract Layer)

| Component | File | Purpose | Key Features |
|-----------|------|---------|--------------|
| **BaseGpio** | `inc/base/BaseGpio.h` | GPIO abstraction | Pin control, interrupts, pull resistors |
| **BaseAdc** | `inc/base/BaseAdc.h` | ADC abstraction | Multi-channel, calibration, voltage conversion |
| **BaseI2c** | `inc/base/BaseI2c.h` | I2C abstraction | Master mode, device scanning, error recovery |
| **BaseSpi** | `inc/base/BaseSpi.h` | SPI abstraction | Full-duplex, configurable modes, DMA support |
| **BaseUart** | `inc/base/BaseUart.h` | UART abstraction | Async I/O, flow control, configurable parameters |
| **BaseCan** | `inc/base/BaseCan.h` | CAN abstraction | Standard/Extended frames, filtering, error handling |
| **BasePwm** | `inc/base/BasePwm.h` | PWM abstraction | Multi-channel, frequency control, duty cycle |
| **BasePio** | `inc/base/BasePio.h` | PIO abstraction | Custom protocols, precise timing, hardware encoding |
| **BaseNvs** | `inc/base/BaseNvs.h` | NVS abstraction | Key-value storage, persistence, encryption |
| **BasePeriodicTimer** | `inc/base/BasePeriodicTimer.h` | Timer abstraction | Periodic callbacks, high precision, low latency |

### ⚙️ **ESP32 Implementations**

| Component | Header | Source | Purpose |
|-----------|---------|---------|---------|
| **EspGpio** | `inc/mcu/esp32/EspGpio.h` | `src/mcu/esp32/EspGpio.cpp` | ESP32-C6 GPIO driver |
| **EspAdc** | `inc/mcu/esp32/EspAdc.h` | `src/mcu/esp32/EspAdc.cpp` | ESP32-C6 ADC with calibration |
| **EspI2c** | `inc/mcu/esp32/EspI2c.h` | `src/mcu/esp32/EspI2c.cpp` | ESP32-C6 I2C master |
| **EspSpi** | `inc/mcu/esp32/EspSpi.h` | `src/mcu/esp32/EspSpi.cpp` | ESP32-C6 SPI master |
| **EspUart** | `inc/mcu/esp32/EspUart.h` | `src/mcu/esp32/EspUart.cpp` | ESP32-C6 UART driver |
| **EspCan** | `inc/mcu/esp32/EspCan.h` | `src/mcu/esp32/EspCan.cpp` | ESP32-C6 TWAI (CAN) |
| **EspPwm** | `inc/mcu/esp32/EspPwm.h` | `src/mcu/esp32/EspPwm.cpp` | ESP32-C6 LEDC PWM |
| **EspPio** | `inc/mcu/esp32/EspPio.h` | `src/mcu/esp32/EspPio.cpp` | ESP32-C6 RMT-based PIO |
| **EspNvs** | `inc/mcu/esp32/EspNvs.h` | `src/mcu/esp32/EspNvs.cpp` | ESP32-C6 NVS storage |
| **EspPeriodicTimer** | `inc/mcu/esp32/EspPeriodicTimer.h` | `src/mcu/esp32/EspPeriodicTimer.cpp` | ESP32-C6 timer callbacks |

### 🔒 **Thread-Safe Wrappers**

| Component | File | Base Class | Synchronization |
|-----------|------|------------|-----------------|
| **SfGpio** | `inc/thread_safe/SfGpio.h` | BaseGpio | Mutex protection |
| **SfAdc** | `inc/thread_safe/SfAdc.h` | BaseAdc | Lock-free reads, batch operations |
| **SfI2cBus** | `inc/thread_safe/SfI2cBus.h` | BaseI2c | Transaction-level locking |
| **SfSpiBus** | `inc/thread_safe/SfSpiBus.h` | BaseSpi | Transfer-level locking |
| **SfUartDriver** | `inc/thread_safe/SfUartDriver.h` | BaseUart | Buffer-level protection |
| **SfCan** | `inc/thread_safe/SfCan.h` | BaseCan | Message queue protection |
| **SfPwm** | `inc/thread_safe/SfPwm.h` | BasePwm | Channel-level locking |

### 🛠️ **Utility Components**

| Component | File | Purpose |
|-----------|------|---------|
| **HardwareTypes** | `inc/base/HardwareTypes.h` | Platform-agnostic type definitions |
| **DigitalOutputGuard** | `inc/utils/DigitalOutputGuard.h` | RAII GPIO management |

---

## 🔌 **Interface Implementations**

### 📊 **GPIO Interface**

```mermaid
classDiagram
    class BaseGpio {
        <<abstract>>
        +SetDirection(direction) hf_gpio_err_t
        +SetHigh() hf_gpio_err_t
        +SetLow() hf_gpio_err_t
        +Read() bool
        +SetPullResistor(mode) hf_gpio_err_t
        +EnableInterrupt(callback) hf_gpio_err_t
    }
    
    class EspGpio {
        -pin_number_: hf_pin_num_t
        -direction_: hf_gpio_direction_t
        -is_initialized_: bool
        +EspGpio(pin, direction)
        +Initialize() bool
    }
    
    class SfGpio {
        -mutex_: std::mutex
        -base_gpio_: std::unique_ptr<BaseGpio>
        +SfGpio(base_impl)
        +ThreadSafeSetHigh() hf_gpio_err_t
    }
    
    BaseGpio <|-- EspGpio
    BaseGpio <-- SfGpio
```

### 📡 **Communication Interfaces**

```mermaid
classDiagram
    class BaseI2c {
        <<abstract>>
        +Initialize(port, frequency) bool
        +Write(address, data, size) hf_i2c_err_t
        +Read(address, buffer, size) hf_i2c_err_t
        +Scan() std::vector<uint8_t>
    }
    
    class BaseSpi {
        <<abstract>>
        +Initialize(host, frequency) bool
        +Transfer(tx_data, rx_data, size) hf_spi_err_t
        +SetMode(mode) hf_spi_err_t
    }
    
    class BaseUart {
        <<abstract>>
        +Initialize(port, baudrate) bool
        +Write(data, size) hf_uart_err_t
        +Read(buffer, size) hf_uart_err_t
        +SetFlowControl(enable) hf_uart_err_t
    }
    
    class EspI2c {
        +EspI2c(port, sda, scl)
    }
    
    class EspSpi {
        +EspSpi(host, mosi, miso, sclk)
    }
    
    class EspUart {
        +EspUart(port, tx, rx)
    }
    
    BaseI2c <|-- EspI2c
    BaseSpi <|-- EspSpi
    BaseUart <|-- EspUart
```

---

## 🔄 **Data Flow**

### 📊 **ADC Data Flow**

```mermaid
flowchart TD
    A[Application] --> B[SfAdc Thread-Safe Wrapper]
    B --> C[BaseAdc Abstract Interface]
    C --> D[EspAdc Implementation]
    D --> E[ESP32-C6 ADC Hardware]
    
    E --> F[Raw Digital Value]
    F --> G[Calibration]
    G --> H[Voltage Conversion]
    H --> I[Return to Application]
    
    style A fill:#e1f5fe
    style E fill:#f3e5f5
    style I fill:#e8f5e8
```

### 🚗 **CAN Message Flow**

```mermaid
flowchart TD
    A[Application] --> B[Send CAN Message]
    B --> C[SfCan Thread-Safe Layer]
    C --> D[BaseCan Interface]
    D --> E[EspCan Implementation]
    E --> F[ESP32-C6 TWAI Controller]
    F --> G[CAN Bus]
    
    G --> H[Receive CAN Message]
    H --> I[TWAI Interrupt]
    I --> J[EspCan Handler]
    J --> K[Callback Function]
    K --> L[Application Handler]
    
    style A fill:#e1f5fe
    style G fill:#fff3e0
    style L fill:#e8f5e8
```

---

## 📁 **File Structure**

```
hf-internal-interface-wrap/
├── 📁 inc/                          # Header files
│   ├── 📁 base/                     # Abstract base classes
│   │   ├── 📄 BaseAdc.h
│   │   ├── 📄 BaseGpio.h
│   │   ├── 📄 BaseCan.h
│   │   ├── 📄 BaseI2c.h
│   │   ├── 📄 BaseSpi.h
│   │   ├── 📄 BaseUart.h
│   │   ├── 📄 BasePwm.h
│   │   ├── 📄 BasePio.h
│   │   ├── 📄 BaseNvs.h
│   │   ├── 📄 BasePeriodicTimer.h
│   │   └── 📄 HardwareTypes.h       # Type definitions
│   ├── 📁 mcu/                      # MCU implementations
│   │   └── 📁 esp32/               # ESP32-specific
│   │       ├── 📄 EspAdc.h
│   │       ├── 📄 EspGpio.h
│   │       ├── 📄 EspCan.h
│   │       ├── 📄 EspI2c.h
│   │       ├── 📄 EspSpi.h
│   │       ├── 📄 EspUart.h
│   │       ├── 📄 EspPwm.h
│   │       ├── 📄 EspPio.h
│   │       ├── 📄 EspNvs.h
│   │       └── 📄 EspPeriodicTimer.h
│   ├── 📁 thread_safe/              # Thread-safe wrappers
│   │   ├── 📄 SfGpio.h
│   │   ├── 📄 SfAdc.h
│   │   ├── 📄 SfI2cBus.h
│   │   ├── 📄 SfSpiBus.h
│   │   ├── 📄 SfUartDriver.h
│   │   ├── 📄 SfCan.h
│   │   └── 📄 SfPwm.h
│   └── 📁 utils/                    # Utility classes
│       └── 📄 DigitalOutputGuard.h
├── 📁 src/                          # Source files
│   ├── 📁 mcu/esp32/               # ESP32 implementations
│   │   ├── 📄 EspAdc.cpp
│   │   ├── 📄 EspGpio.cpp
│   │   ├── 📄 EspCan.cpp
│   │   ├── 📄 EspI2c.cpp
│   │   ├── 📄 EspSpi.cpp
│   │   ├── 📄 EspUart.cpp
│   │   ├── 📄 EspPwm.cpp
│   │   ├── 📄 EspPio.cpp
│   │   ├── 📄 EspNvs.cpp
│   │   └── 📄 EspPeriodicTimer.cpp
│   └── 📁 utils/                    # Utility implementations
│       └── 📄 DigitalOutputGuard.cpp
├── 📁 examples/                     # Example projects
│   ├── 📁 esp32/                   # ESP32 examples
│   └── 📁 common/                  # Common examples
├── 📁 docs/                         # Documentation
│   ├── 📁 api/                     # API documentation
│   ├── 📁 guides/                  # User guides
│   └── 📁 examples/                # Example documentation
└── 📁 .github/                      # GitHub workflows
    └── 📁 workflows/               # CI/CD pipelines
```

---

## 🎯 **Component Dependencies**

### 📊 **Dependency Graph**

```mermaid
graph TD
    subgraph "🏛️ Core Types"
        HT[HardwareTypes.h]
    end
    
    subgraph "🏛️ Base Classes"
        BA[BaseAdc] --> HT
        BG[BaseGpio] --> HT
        BC[BaseCan] --> HT
        BI[BaseI2c] --> HT
        BS[BaseSpi] --> HT
        BU[BaseUart] --> HT
        BP[BasePwm] --> HT
        BPio[BasePio] --> HT
        BN[BaseNvs] --> HT
        BT[BasePeriodicTimer] --> HT
    end
    
    subgraph "⚙️ ESP32 Implementations"
        EA[EspAdc] --> BA
        EG[EspGpio] --> BG
        EC[EspCan] --> BC
        EI[EspI2c] --> BI
        ES[EspSpi] --> BS
        EU[EspUart] --> BU
        EP[EspPwm] --> BP
        EPio[EspPio] --> BPio
        EN[EspNvs] --> BN
        ET[EspPeriodicTimer] --> BT
    end
    
    subgraph "🔒 Thread-Safe Wrappers"
        SFA[SfAdc] --> BA
        SFG[SfGpio] --> BG
        SFC[SfCan] --> BC
        SFI[SfI2cBus] --> BI
        SFS[SfSpiBus] --> BS
        SFU[SfUartDriver] --> BU
        SFP[SfPwm] --> BP
    end
    
    subgraph "🛠️ Utilities"
        DOG[DigitalOutputGuard] --> BG
    end
```

---

## 🚀 **Usage Patterns**

### 🎯 **Direct Usage Pattern**

```cpp
// Direct ESP32 implementation usage
EspGpio led_pin(GPIO_NUM_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
led_pin.Initialize();
led_pin.SetHigh();
```

### 🔒 **Thread-Safe Pattern**

```cpp
// Thread-safe wrapper usage
auto esp_gpio = std::make_unique<EspGpio>(GPIO_NUM_2, 
                                          hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
SfGpio safe_gpio(std::move(esp_gpio));
safe_gpio.Initialize();
safe_gpio.SetHigh();  // Thread-safe operation
```

### 🎯 **Polymorphic Pattern**

```cpp
// Using base class interface
std::unique_ptr<BaseGpio> gpio = std::make_unique<EspGpio>(GPIO_NUM_2, 
                                                           hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
gpio->Initialize();
gpio->SetHigh();  // Virtual function call
```

---

<div align="center">

**🗺️ This component map provides a comprehensive overview of the HardFOC Internal Interface Wrapper system architecture**

*Use this guide to understand component relationships and make informed implementation decisions*

</div>