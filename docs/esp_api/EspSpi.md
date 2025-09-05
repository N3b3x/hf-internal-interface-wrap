# 🔌 EspSpi API Reference

<div align="center">

![EspSpi](https://img.shields.io/badge/EspSpi-ESP32C6%20Implementation-blue?style=for-the-badge&logo=espressif)

**🔄 ESP32-C6 SPI implementation with ESP-IDF v5.5+ features**

**📋 Navigation**

[← Previous: EspI2c](EspI2c.md) | [Back to ESP API Index](README.md) | [Next: EspUart →](EspUart.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Architecture**](#-architecture)
- [🔧 **Core Classes**](#-core-classes)
- [📋 **Configuration**](#-configuration)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)
- [🔍 **Troubleshooting**](#-troubleshooting)

---

## 🎯 **Overview**

The `EspSpi` library provides a comprehensive SPI implementation for ESP32-C6 using ESP-IDF v5.5+.
It implements the `BaseSpi` interface and provides advanced features including DMA acceleration,
IOMUX optimization, multi-device management, and comprehensive error handling.

### ✨ **Key Features**

- 🔄 **ESP-IDF v5.5+ Integration** - Full compliance with latest ESP-IDF SPI Master driver
- ⚡ **High-Speed Transfer** - Up to 80 MHz with proper DMA configuration
- 🎛️ **Flexible Modes** - Support for all SPI modes (0, 1, 2, 3)
- 📊 **DMA Support** - Hardware-accelerated data transfer with configurable channels
- 🏎️ **IOMUX Optimization** - Direct pin-to-peripheral connections for maximum performance
- 🛡️ **Thread-Safe Operations** - RTOS mutex protection for multi-device management
- 🔌 **Multi-Device Support** - Single bus with multiple device management
- 📈 **Advanced Timing Control** - Configurable CS setup/hold times and input delay compensation

### 🔌 **Hardware Support**

| Feature | ESP32-C6 Support | Description |

|---------|------------------|-------------|

| **SPI Hosts** | SPI2*HOST (GP-SPI2) | General-purpose SPI host |

| **Clock Sources** | PLL*F80M, XTAL, RC*FAST | Configurable clock sources for power optimization |

| **DMA Channels** | 0-3 | Hardware DMA acceleration |

| **IOMUX Pins** | Direct connection | Up to 80 MHz operation |

| **GPIO Matrix** | Flexible routing | Up to 40 MHz operation |

---

## 🏗️ **Architecture**

The EspSpi library follows a two-tier architecture:

```text
┌─────────────────────────────────────────────────────────────────┐
│                        EspSpiBus                                │
├─────────────────────────────────────────────────────────────────┤
│ + Initialize() : bool                                           │
│ + Deinitialize() : bool                                         │
│ + CreateDevice(config) : int                                    │
│ + RemoveDevice(device*id) : bool                                │
│ + GetDevice(device*id) : BaseSpi*                              │
│ + IsInitialized() : bool                                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ manages
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                       EspSpiDevice                              │
├─────────────────────────────────────────────────────────────────┤
│ + Initialize() : bool                                           │
│ + Deinitialize() : bool                                         │
│ + Transfer(tx*data, rx*data, length, timeout) : hf*spi*err*t   │
│ + Transmit(data, length, timeout) : hf*spi*err*t               │
│ + Receive(data, length, timeout) : hf*spi*err*t                │
│ + GetDeviceStatus(status) : hf*spi*err*t                       │
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ implements
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                         BaseSpi                                 │
│                        (abstract)                               │
├─────────────────────────────────────────────────────────────────┤
│ + Initialize() : bool                                           │
│ + Deinitialize() : bool                                         │
│ + Transfer() : hf*spi*err*t                                    │
└─────────────────────────────────────────────────────────────────┘
```text

### **Design Principles**

1. **Device Lifecycle Management**: `CreateDevice()` creates the C++ wrapper, `Initialize()` creates the ESP-IDF device
2. **Resource Ownership**: Bus owns devices, devices are managed through RAII
3. **Thread Safety**: All operations protected by RTOS mutex
4. **Error Handling**: Comprehensive validation and ESP-IDF error translation

---

## 🔧 **Core Classes**

### **EspSpiBus**

The main SPI bus controller that manages the ESP-IDF SPI host and multiple devices.

#### **Constructor**
```cpp
EspSpiBus(const hf*spi*bus*config*t& config) noexcept
```text

#### **Key Methods**
```cpp
bool Initialize() noexcept;                    // Initialize ESP-IDF SPI bus
bool Deinitialize() noexcept;                  // Clean up ESP-IDF resources
int CreateDevice(const hf*spi*device*config*t& config);  // Create device wrapper
bool RemoveDevice(int device*id);              // Remove device from bus
BaseSpi* GetDevice(int device*id);            // Get device by ID
bool IsInitialized() const noexcept;          // Check initialization status
```text

### **EspSpiDevice**

Represents a single SPI device on the bus, implementing the `BaseSpi` interface.

#### **Constructor**
```cpp
EspSpiDevice(EspSpiBus* parent, const hf*spi*device*config*t& config)
```text

#### **Key Methods**
```cpp
bool Initialize() noexcept override;           // Create ESP-IDF device
bool Deinitialize() noexcept override;        // Remove ESP-IDF device
hf*spi*err*t Transfer(const uint8*t* tx*data, uint8*t* rx*data, 
                      size*t length, uint32*t timeout*ms) override;
hf*spi*err*t Transmit(const uint8*t* data, size*t length, 
                      uint32*t timeout*ms) override;
hf*spi*err*t Receive(uint8*t* data, size*t length, 
                     uint32*t timeout*ms) override;
```text

---

## 📋 **Configuration**

### **Bus Configuration (`hf*spi*bus*config*t`)**

```cpp
typedef struct {
    hf*pin*num*t mosi*pin;           // MOSI pin number
    hf*pin*num*t miso*pin;           // MISO pin number  
    hf*pin*num*t sclk*pin;           // SCLK pin number
    hf*host*id*t host;               // SPI host ID (SPI2*HOST = 1)
    uint32*t clock*speed*hz;         // Bus clock frequency
    uint8*t dma*channel;             // DMA channel (0xFF = auto, 0 = disabled)
    bool use*iomux;                  // Use IOMUX for maximum performance
    uint32*t timeout*ms;             // Bus timeout in milliseconds
} hf*spi*bus*config*t;
```text

### **Device Configuration (`hf*spi*device*config*t`)**

```cpp
typedef struct {
    uint32*t clock*speed*hz;         // Device-specific clock frequency
    hf*spi*mode*t mode;              // SPI mode (0, 1, 2, 3)
    hf*pin*num*t cs*pin;            // Chip select pin
    uint8*t queue*size;              // Transaction queue size
    uint8*t command*bits;            // Command bits (0 = disabled)
    uint8*t address*bits;            // Address bits (0 = disabled)
    uint8*t dummy*bits;              // Dummy bits between phases
    uint32*t cs*ena*pretrans;        // CS setup time (in SPI clock cycles)
    uint32*t cs*ena*posttrans;       // CS hold time (in SPI clock cycles)
    hf*spi*clock*source*t clock*source; // Clock source selection
} hf*spi*device*config*t;
```text

### **Clock Sources**

| Enum Value | Description | Frequency | Use Case |

|------------|-------------|-----------|----------|

| `PLL*F80M*CLK` | PLL clock | 80 MHz | High-speed operations |

| `XTAL*CLK` | Crystal oscillator | 40 MHz | Stable, power-efficient |

| `RC*FAST*CLK` | RC oscillator | ~17.5 MHz | Low-power, approximate |

### **SPI Modes**

| Mode | CPOL | CPHA | Description |

|------|------|------|-------------|

| 0 | 0 | 0 | Clock idle low, data sampled on rising edge |

| 1 | 0 | 1 | Clock idle low, data sampled on falling edge |

| 2 | 1 | 0 | Clock idle high, data sampled on falling edge |

| 3 | 1 | 1 | Clock idle high, data sampled on rising edge |

---

## 📊 **Usage Examples**

### **Basic Setup and Usage**

```cpp
#include "mcu/esp32/EspSpi.h"

// 1. Create bus configuration
hf*spi*bus*config*t bus*config = {};
bus*config.mosi*pin = 7;           // GPIO7
bus*config.miso*pin = 2;           // GPIO2  
bus*config.sclk*pin = 6;           // GPIO6
bus*config.host = static*cast<hf*host*id*t>(1);  // SPI2*HOST
bus*config.clock*speed*hz = 10000000;  // 10 MHz
bus*config.dma*channel = 0;        // Use DMA channel 0
bus*config.use*iomux = true;       // Maximum performance

// 2. Create and initialize bus
auto spi*bus = std::make*unique<EspSpiBus>(bus*config);
if (!spi*bus->Initialize()) {
    ESP*LOGE(TAG, "Failed to initialize SPI bus");
    return;
}

// 3. Create device configuration
hf*spi*device*config*t device*config = {};
device*config.clock*speed*hz = 10000000;  // 10 MHz
device*config.mode = hf*spi*mode*t::HF*SPI*MODE*0;
device*config.cs*pin = 21;         // GPIO21
device*config.queue*size = 7;      // Transaction queue

// 4. Create and initialize device
int device*id = spi*bus->CreateDevice(device*config);
if (device*id < 0) {
    ESP*LOGE(TAG, "Failed to create SPI device");
    return;
}

BaseSpi* device = spi*bus->GetDevice(device*id);
if (!device->Initialize()) {
    ESP*LOGE(TAG, "Failed to initialize SPI device");
    return;
}

// 5. Perform data transfer
uint8*t tx*data[] = {0xAA, 0x55, 0x12, 0x34};
uint8*t rx*data[4] = {0};

hf*spi*err*t result = device->Transfer(tx*data, rx*data, 4, 1000);
if (result == hf*spi*err*t::SPI*SUCCESS) {
    ESP*LOGI(TAG, "Transfer successful");
} else {
    ESP*LOGE(TAG, "Transfer failed: %s", HfSpiErrToString(result).data());
}
```text

### **Multi-Device Setup**

```cpp
// Create multiple devices on the same bus
hf*spi*device*config*t device1*config = {};
device1*config.clock*speed*hz = 10000000;
device1*config.mode = hf*spi*mode*t::HF*SPI*MODE*0;
device1*config.cs*pin = 21;

hf*spi*device*config*t device2*config = {};
device2*config.clock*speed*hz = 5000000;
device2*config.mode = hf*spi*mode*t::HF*SPI*MODE*1;
device2*config.cs*pin = 22;

int device1*id = spi*bus->CreateDevice(device1*config);
int device2*id = spi*bus->CreateDevice(device2*config);

BaseSpi* device1 = spi*bus->GetDevice(device1*id);
BaseSpi* device2 = spi*bus->GetDevice(device2*id);

device1->Initialize();
device2->Initialize();

// Use devices independently
device1->Transfer(tx*data1, rx*data1, 4, 1000);
device2->Transfer(tx*data2, rx*data2, 8, 1000);
```text

### **Advanced Configuration**

```cpp
// High-speed device with custom timing
hf*spi*device*config*t fast*device*config = {};
fast*device*config.clock*speed*hz = 80000000;  // 80 MHz
fast*device*config.mode = hf*spi*mode*t::HF*SPI*MODE*0;
fast*device*config.cs*pin = 21;
fast*device*config.queue*size = 15;
fast*device*config.command*bits = 8;           // 8-bit command phase
fast*device*config.address*bits = 24;          // 24-bit address phase
fast*device*config.dummy*bits = 8;             // 8 dummy bits
fast*device*config.cs*ena*pretrans = 2;        // 2 clock cycles setup
fast*device*config.cs*ena*posttrans = 2;       // 2 clock cycles hold
fast*device*config.clock*source = hf*spi*clock*source*t::PLL*F80M*CLK;
```text

---

## 🧪 **Best Practices**

### **Performance Optimization**

1. **Use IOMUX Pins**: Enable `use*iomux = true` for maximum performance
2. **DMA Configuration**: Use dedicated DMA channels for large transfers (>64 bytes)
3. **Clock Source Selection**: Use `PLL*F80M*CLK` for high-speed, `XTAL*CLK` for stability
4. **Queue Size**: Set appropriate queue size based on application needs

### **Memory Management**

1. **RAII Pattern**: Use `std::unique*ptr` for automatic cleanup
2. **Device Lifecycle**: Always call `Initialize()` after `CreateDevice()`
3. **Resource Cleanup**: Let destructors handle cleanup automatically

### **Error Handling**

1. **Check Return Values**: Always verify `Initialize()` and transfer results
2. **Timeout Configuration**: Set appropriate timeouts for your application
3. **Error Logging**: Use ESP-IDF logging for debugging

### **Thread Safety**

1. **Single Bus Access**: The library is thread-safe, but avoid concurrent bus operations
2. **Device Independence**: Multiple devices can be used concurrently
3. **Mutex Protection**: All operations are internally protected

---

## 🔍 **Troubleshooting**

### **Common Issues**

#### **"invalid host*id" Error**
- **Cause**: Incorrect host ID for ESP32-C6
- **Solution**: Use `static*cast<hf*host*id*t>(1)` for `SPI2*HOST`

#### **Large Transfer Failures (>256 bytes)**
- **Cause**: DMA configuration issues or memory constraints
- **Solution**: Verify DMA channel configuration and reduce transfer size

#### **Clock Glitches**
- **Cause**: Improper clock source or divider configuration
- **Solution**: Use stable clock sources and verify TRM-compliant settings

#### **Data Corruption**
- **Cause**: Small data optimization issues or timing problems
- **Solution**: Ensure proper CS timing and verify data buffer management

### **Debugging Tips**

1. **Enable ESP-IDF Logging**: Set log level to DEBUG for detailed information
2. **Verify Pin Configuration**: Check pin assignments and IOMUX usage
3. **Monitor Clock Signals**: Use logic analyzer to verify timing
4. **Check DMA Status**: Verify DMA channel availability and configuration

### **Performance Monitoring**

```cpp
// Measure transfer performance
uint64*t start*time = esp*timer*get*time();
hf*spi*err*t result = device->Transfer(tx*data, rx*data, length, timeout);
uint64*t end*time = esp*timer*get*time();
uint64*t transfer*time = end*time - start*time;

ESP*LOGI(TAG, "Transfer %zu bytes in %llu μs", length, transfer*time);
```text

---

## 🔗 **Navigation**

### **Documentation Structure**

- **[🏠 Main Documentation](../README.md)** - Complete system overview
- **[📋 API Interfaces](../api/README.md)** - Base classes and interfaces
- **[🔧 ESP32 Implementations](README.md)** - Hardware-specific implementations
- **[🧪 Test Suites](../../examples/esp32/docs/README.md)** - Testing and validation

### **Related Documentation**

- **[BaseSpi API Reference](../api/BaseSpi.md)** - Abstract SPI interface
- **[Hardware Types](../api/HardwareTypes.md)** - Type definitions
- **[SPI Comprehensive Tests](../../examples/esp32/docs/README_SPI_TEST.md)** - Complete SPI validation
- **[ESP-IDF SPI Master Driver](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/spi*master.html)** - Official ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspI2c](EspI2c.md) | [Back to ESP API Index](README.md) | [Next: EspUart →](EspUart.md)

</div>

- **Current**: ESP-IDF v5.5+ compatible implementation
- **Features**: Full BaseSpi compliance, DMA support, IOMUX optimization
- **Architecture**: Two-tier design with bus and device management
- **Thread Safety**: RTOS mutex protection for all operations
