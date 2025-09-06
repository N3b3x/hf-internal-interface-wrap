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

| **SPI Hosts** | SPI2_HOST (GP-SPI2) | General-purpose SPI host |

| **Clock Sources** | PLL_F80M, XTAL, RC_FAST | Configurable clock sources for power optimization |

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
│ + RemoveDevice(device_id) : bool                                │
│ + GetDevice(device_id) : BaseSpi*                              │
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
│ + Transfer(tx_data, rx_data, length, timeout) : hf_spi_err_t   │
│ + Transmit(data, length, timeout) : hf_spi_err_t               │
│ + Receive(data, length, timeout) : hf_spi_err_t                │
│ + GetDeviceStatus(status) : hf_spi_err_t                       │
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
│ + Transfer() : hf_spi_err_t                                    │
└─────────────────────────────────────────────────────────────────┘
```

### **Design Principles**

1. **Device Lifecycle Management**: `CreateDevice()` creates the C++ wrapper, `Initialize()` creates ESP-IDF device
2. **Resource Ownership**: Bus owns devices, devices are managed through RAII
3. **Thread Safety**: All operations protected by RTOS mutex
4. **Error Handling**: Comprehensive validation and ESP-IDF error translation

---

## 🔧 **Core Classes**

### **EspSpiBus**

The main SPI bus controller that manages the ESP-IDF SPI host and multiple devices.

#### **Constructor**
```cpp
EspSpiBus(const hf_spi_bus_config_t& config) noexcept
```

#### **Key Methods**
```cpp
bool Initialize() noexcept;                    // Initialize ESP-IDF SPI bus
bool Deinitialize() noexcept;                  // Clean up ESP-IDF resources
int CreateDevice(const hf_spi_device_config_t& config);  // Create device wrapper
bool RemoveDevice(int device_id);              // Remove device from bus
BaseSpi* GetDevice(int device_id);            // Get device by ID
bool IsInitialized() const noexcept;          // Check initialization status
```

### **EspSpiDevice**

Represents a single SPI device on the bus, implementing the `BaseSpi` interface.

#### **Constructor**
```cpp
EspSpiDevice(EspSpiBus* parent, const hf_spi_device_config_t& config)
```

#### **Key Methods**
```cpp
bool Initialize() noexcept override;           // Create ESP-IDF device
bool Deinitialize() noexcept override;        // Remove ESP-IDF device
hf_spi_err_t Transfer(const uint8_t* tx_data, uint8_t* rx_data, 
                      size_t length, uint32_t timeout_ms) override;
hf_spi_err_t Transmit(const uint8_t* data, size_t length, 
                      uint32_t timeout_ms) override;
hf_spi_err_t Receive(uint8_t* data, size_t length, 
                     uint32_t timeout_ms) override;
```

---

## 📋 **Configuration**

### **Bus Configuration (`hf_spi_bus_config_t`)**

```cpp
typedef struct {
    hf_pin_num_t mosi_pin;           // MOSI pin number
    hf_pin_num_t miso_pin;           // MISO pin number  
    hf_pin_num_t sclk_pin;           // SCLK pin number
    hf_host_id_t host;               // SPI host ID (SPI2_HOST = 1)
    uint32_t clock_speed_hz;         // Bus clock frequency
    uint8_t dma_channel;             // DMA channel (0xFF = auto, 0 = disabled)
    bool use_iomux;                  // Use IOMUX for maximum performance
    uint32_t timeout_ms;             // Bus timeout in milliseconds
} hf_spi_bus_config_t;
```

### **Device Configuration (`hf_spi_device_config_t`)**

```cpp
typedef struct {
    uint32_t clock_speed_hz;         // Device-specific clock frequency
    hf_spi_mode_t mode;              // SPI mode (0, 1, 2, 3)
    hf_pin_num_t cs_pin;            // Chip select pin
    uint8_t queue_size;              // Transaction queue size
    uint8_t command_bits;            // Command bits (0 = disabled)
    uint8_t address_bits;            // Address bits (0 = disabled)
    uint8_t dummy_bits;              // Dummy bits between phases
    uint32_t cs_ena_pretrans;        // CS setup time (in SPI clock cycles)
    uint32_t cs_ena_posttrans;       // CS hold time (in SPI clock cycles)
    hf_spi_clock_source_t clock_source; // Clock source selection
} hf_spi_device_config_t;
```

### **Clock Sources**

| Enum Value | Description | Frequency | Use Case |

|------------|-------------|-----------|----------|

| `PLL_F80M_CLK` | PLL clock | 80 MHz | High-speed operations |

| `XTAL_CLK` | Crystal oscillator | 40 MHz | Stable, power-efficient |

| `RC_FAST_CLK` | RC oscillator | ~17.5 MHz | Low-power, approximate |

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
hf_spi_bus_config_t bus_config = {};
bus_config.mosi_pin = 7;           // GPIO7
bus_config.miso_pin = 2;           // GPIO2  
bus_config.sclk_pin = 6;           // GPIO6
bus_config.host = static_cast<hf_host_id_t>(1);  // SPI2_HOST
bus_config.clock_speed_hz = 10000000;  // 10 MHz
bus_config.dma_channel = 0;        // Use DMA channel 0
bus_config.use_iomux = true;       // Maximum performance

// 2. Create and initialize bus
auto spi_bus = std::make_unique<EspSpiBus>(bus_config);
if (!spi_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return;
}

// 3. Create device configuration
hf_spi_device_config_t device_config = {};
device_config.clock_speed_hz = 10000000;  // 10 MHz
device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
device_config.cs_pin = 21;         // GPIO21
device_config.queue_size = 7;      // Transaction queue

// 4. Create and initialize device
int device_id = spi_bus->CreateDevice(device_config);
if (device_id < 0) {
    ESP_LOGE(TAG, "Failed to create SPI device");
    return;
}

BaseSpi* device = spi_bus->GetDevice(device_id);
if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI device");
    return;
}

// 5. Perform data transfer
uint8_t tx_data[] = {0xAA, 0x55, 0x12, 0x34};
uint8_t rx_data[4] = {0};

hf_spi_err_t result = device->Transfer(tx_data, rx_data, 4, 1000);
if (result == hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGI(TAG, "Transfer successful");
} else {
    ESP_LOGE(TAG, "Transfer failed: %s", HfSpiErrToString(result).data());
}
```

### **Multi-Device Setup**

```cpp
// Create multiple devices on the same bus
hf_spi_device_config_t device1_config = {};
device1_config.clock_speed_hz = 10000000;
device1_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
device1_config.cs_pin = 21;

hf_spi_device_config_t device2_config = {};
device2_config.clock_speed_hz = 5000000;
device2_config.mode = hf_spi_mode_t::HF_SPI_MODE_1;
device2_config.cs_pin = 22;

int device1_id = spi_bus->CreateDevice(device1_config);
int device2_id = spi_bus->CreateDevice(device2_config);

BaseSpi* device1 = spi_bus->GetDevice(device1_id);
BaseSpi* device2 = spi_bus->GetDevice(device2_id);

device1->Initialize();
device2->Initialize();

// Use devices independently
device1->Transfer(tx_data1, rx_data1, 4, 1000);
device2->Transfer(tx_data2, rx_data2, 8, 1000);
```

### **Advanced Configuration**

```cpp
// High-speed device with custom timing
hf_spi_device_config_t fast_device_config = {};
fast_device_config.clock_speed_hz = 80000000;  // 80 MHz
fast_device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
fast_device_config.cs_pin = 21;
fast_device_config.queue_size = 15;
fast_device_config.command_bits = 8;           // 8-bit command phase
fast_device_config.address_bits = 24;          // 24-bit address phase
fast_device_config.dummy_bits = 8;             // 8 dummy bits
fast_device_config.cs_ena_pretrans = 2;        // 2 clock cycles setup
fast_device_config.cs_ena_posttrans = 2;       // 2 clock cycles hold
fast_device_config.clock_source = hf_spi_clock_source_t::PLL_F80M_CLK;
```

---

## 🧪 **Best Practices**

### **Performance Optimization**

1. **Use IOMUX Pins**: Enable `use_iomux = true` for maximum performance
2. **DMA Configuration**: Use dedicated DMA channels for large transfers (>64 bytes)
3. **Clock Source Selection**: Use `PLL_F80M_CLK` for high-speed, `XTAL_CLK` for stability
4. **Queue Size**: Set appropriate queue size based on application needs

### **Memory Management**

1. **RAII Pattern**: Use `std::unique_ptr` for automatic cleanup
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

#### **"invalid host_id" Error**
- **Cause**: Incorrect host ID for ESP32-C6
- **Solution**: Use `static_cast<hf_host_id_t>(1)` for `SPI2_HOST`

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
uint64_t start_time = esp_timer_get_time();
hf_spi_err_t result = device->Transfer(tx_data, rx_data, length, timeout);
uint64_t end_time = esp_timer_get_time();
uint64_t transfer_time = end_time - start_time;

ESP_LOGI(TAG, "Transfer %zu bytes in %llu μs", length, transfer_time);
```

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
- **[ESP-IDF SPI Master Driver](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/spi_master.html)** - Official ESP-IDF docs

---

<div align="center">

**📋 Navigation**

[← Previous: EspI2c](EspI2c.md) | [Back to ESP API Index](README.md) | [Next: EspUart →](EspUart.md)

</div>

- **Current**: ESP-IDF v5.5+ compatible implementation
- **Features**: Full BaseSpi compliance, DMA support, IOMUX optimization
- **Architecture**: Two-tier design with bus and device management
- **Thread Safety**: RTOS mutex protection for all operations
