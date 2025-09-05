# 🔌 BaseSpi API Reference

<div align="center">

![BaseSpi](https://img.shields.io/badge/BaseSpi-Abstract%20Base%20Class-blue?style=for-the-badge&logo=plug)

**🔄 Unified SPI abstraction for high-speed serial communication**

**📋 Navigation**

[← Previous: BaseI2c](BaseI2c.md) | [Back to API Index](README.md) | [Next: BaseUart →](BaseUart.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BaseSpi` class provides a comprehensive SPI abstraction that serves as the unified interface
for all Serial Peripheral Interface operations in the HardFOC system.
It supports multi-device communication, configurable modes, and high-speed data transfer.

### ✨ **Key Features**

- 🔄 **Multi-Device Support** - Simultaneous communication with multiple SPI devices
- ⚡ **High-Speed Transfer** - Configurable clock frequencies up to 80 MHz
- 🎛️ **Flexible Modes** - Support for all SPI modes (0, 1, 2, 3)
- 📊 **DMA Support** - Hardware-accelerated data transfer
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Performance Optimized** - Minimal overhead for critical applications
- 🔌 **Platform Agnostic** - Works with various SPI hardware implementations
- 📈 **Real-time Control** - Low-latency communication for time-critical applications

### 🔌 **Supported Applications**

| Application | Speed | Description |

|-------------|-------|-------------|

| **Sensor Communication** | 1-10 MHz | Temperature, pressure, IMU sensors |

| **Display Control** | 10-40 MHz | LCD, OLED, TFT displays |

| **Memory Access** | 20-80 MHz | Flash, EEPROM, FRAM |

| **Motor Control** | 1-20 MHz | Motor driver ICs |

| **Audio Codecs** | 1-50 MHz | Digital audio interfaces |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseSpi {
        <<abstract>>
        +Initialize() hf*spi*err*t
        +Deinitialize() hf*spi*err*t
        +ConfigureDevice(device*id, config) hf*spi*err*t
        +TransmitReceive(device*id, tx*data, rx*data, length) hf*spi*err*t
        +Transmit(device*id, data, length) hf*spi*err*t
        +Receive(device*id, data, length) hf*spi*err*t
        +GetDeviceStatus(device*id, status) hf*spi*err*t
        +GetCapabilities(capabilities) hf*spi*err*t
    }
    
    class EspSpi {
        +EspSpi(host, cs*pin)
        +GetHost() spi*host*device*t
        +GetCsPin() hf*pin*num*t
    }
    
    BaseSpi <|-- EspSpi
```text

---

## 📋 **Error Codes**

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `SPI*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `SPI*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `SPI*ERR*NOT*INITIALIZED` | 2 | ⚠️ SPI not initialized | Call Initialize() first |

| `SPI*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ SPI already initialized | Check initialization state |

| `SPI*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `SPI*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `SPI*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### 🔧 **Device Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `SPI*ERR*INVALID*DEVICE` | 7 | 🚫 Invalid SPI device | Use valid device numbers |

| `SPI*ERR*DEVICE*BUSY` | 8 | 🔄 Device already in use | Wait or use different device |

| `SPI*ERR*DEVICE*NOT*AVAILABLE` | 9 | ⚠️ Device not available | Check device availability |

| `SPI*ERR*DEVICE*NOT*CONFIGURED` | 10 | ⚙️ Device not configured | Configure device first |

| `SPI*ERR*DEVICE*NOT*RESPONDING` | 11 | 🔇 Device not responding | Check device power and connections |

### ⚡ **Transfer Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `SPI*ERR*TRANSFER*TIMEOUT` | 12 | ⏰ Transfer timeout | Check clock frequency and device |

| `SPI*ERR*TRANSFER*FAILURE` | 13 | ❌ Transfer failed | Check connections and device state |

| `SPI*ERR*TRANSFER*INCOMPLETE` | 14 | 📊 Transfer incomplete | Check data length and buffer size |

| `SPI*ERR*TRANSFER*ABORTED` | 15 | ⏹️ Transfer aborted | Check abort conditions |

### 🎛️ **Configuration Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `SPI*ERR*INVALID*CONFIGURATION` | 16 | ⚙️ Invalid configuration | Check configuration parameters |

| `SPI*ERR*UNSUPPORTED*MODE` | 17 | 🚫 Unsupported SPI mode | Use supported mode |

| `SPI*ERR*UNSUPPORTED*FREQUENCY` | 18 | 🚫 Unsupported frequency | Use supported frequency range |

| `SPI*ERR*UNSUPPORTED*DATA*SIZE` | 19 | 🚫 Unsupported data size | Use supported data size |

| `SPI*ERR*PIN*CONFLICT` | 20 | 🔌 Pin already in use | Use different pins |

### 🌐 **Hardware Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `SPI*ERR*HARDWARE*FAULT` | 21 | 💥 Hardware fault | Check power and connections |

| `SPI*ERR*COMMUNICATION*FAILURE` | 22 | 📡 Communication failure | Check bus connections |

| `SPI*ERR*DMA*ERROR` | 23 | 💾 DMA error | Check DMA configuration |

| `SPI*ERR*RESOURCE*BUSY` | 24 | 🔄 Resource busy | Wait for resource availability |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the SPI peripheral
 * @return hf*spi*err*t error code
 * 
 * 📝 Sets up SPI hardware, configures devices, and prepares for communication.
 * Must be called before any SPI operations.
 * 
 * @example
 * EspSpi spi(SPI2*HOST, 5);  // SPI2, CS on pin 5
 * hf*spi*err*t result = spi.Initialize();
 * if (result == hf*spi*err*t::SPI*SUCCESS) {
 *     // SPI ready for use
 * }
 */
virtual hf*spi*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the SPI peripheral
 * @return hf*spi*err*t error code
 * 
 * 🧹 Cleanly shuts down SPI hardware and releases resources.
 */
virtual hf*spi*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if SPI is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
[[nodiscard]] bool IsInitialized() const noexcept;

/**
 * @brief Ensure SPI is initialized (lazy initialization)
 * @return true if initialized successfully, false otherwise
 * 
 * 🔄 Automatically initializes SPI if not already initialized.
 */
bool EnsureInitialized() noexcept;
```text

### ⚙️ **Device Configuration**

```cpp
/**
 * @brief Configure an SPI device
 * @param device*id Device identifier
 * @param config Device configuration structure
 * @return hf*spi*err*t error code
 * 
 * ⚙️ Configures device parameters including mode, frequency, data size,
 * and pin assignments.
 * 
 * @example
 * hf*spi*device*config*t config;
 * config.mode = hf*spi*mode*t::MODE*0;
 * config.frequency*hz = 1000000;  // 1 MHz
 * config.data*size = hf*spi*data*size*t::DATA*8BIT;
 * config.cs*pin = 5;
 * config.cs*active*low = true;
 * 
 * hf*spi*err*t result = spi.ConfigureDevice(0, config);
 */
virtual hf*spi*err*t ConfigureDevice(uint8*t device*id,
                                   const hf*spi*device*config*t &config) noexcept = 0;
```text

### 🔄 **Data Transfer Methods**

```cpp
/**
 * @brief Transmit and receive data simultaneously
 * @param device*id Device identifier
 * @param tx*data Transmit data buffer
 * @param rx*data Receive data buffer
 * @param length Number of bytes to transfer
 * @return hf*spi*err*t error code
 * 
 * 🔄 Performs full-duplex SPI transfer. Both transmit and receive
 * buffers must be at least 'length' bytes.
 * 
 * @example
 * uint8*t tx*data[] = {0x01, 0x02, 0x03};
 * uint8*t rx*data[3];
 * hf*spi*err*t result = spi.TransmitReceive(0, tx*data, rx*data, 3);
 * if (result == hf*spi*err*t::SPI*SUCCESS) {
 *     printf("Received: %02X %02X %02X\n", rx*data[0], rx*data[1], rx*data[2]);
 * }
 */
virtual hf*spi*err*t TransmitReceive(uint8*t device*id, const uint8*t *tx*data,
                                   uint8*t *rx*data, size*t length) noexcept = 0;

/**
 * @brief Transmit data only
 * @param device*id Device identifier
 * @param data Transmit data buffer
 * @param length Number of bytes to transmit
 * @return hf*spi*err*t error code
 * 
 * 📤 Performs SPI transmit operation. Receive data is discarded.
 * 
 * @example
 * uint8*t command[] = {0xAA, 0x55, 0x01};
 * hf*spi*err*t result = spi.Transmit(0, command, 3);
 */
virtual hf*spi*err*t Transmit(uint8*t device*id, const uint8*t *data,
                            size*t length) noexcept = 0;

/**
 * @brief Receive data only
 * @param device*id Device identifier
 * @param data Receive data buffer
 * @param length Number of bytes to receive
 * @return hf*spi*err*t error code
 * 
 * 📥 Performs SPI receive operation. Transmit data is zeros.
 * 
 * @example
 * uint8*t response[4];
 * hf*spi*err*t result = spi.Receive(0, response, 4);
 */
virtual hf*spi*err*t Receive(uint8*t device*id, uint8*t *data,
                           size*t length) noexcept = 0;
```text

### 📊 **Status and Capabilities**

```cpp
/**
 * @brief Get device status information
 * @param device*id Device identifier
 * @param status [out] Status information structure
 * @return hf*spi*err*t error code
 * 
 * 📊 Retrieves comprehensive status information about a device.
 */
virtual hf*spi*err*t GetDeviceStatus(uint8*t device*id,
                                   hf*spi*device*status*t &status) const noexcept = 0;

/**
 * @brief Get SPI capabilities
 * @param capabilities [out] Capability information structure
 * @return hf*spi*err*t error code
 * 
 * 📋 Retrieves hardware capabilities and limitations.
 */
virtual hf*spi*err*t GetCapabilities(hf*spi*capabilities*t &capabilities) const noexcept = 0;
```text

---

## 📊 **Data Structures**

### ⚙️ **Device Configuration**

```cpp
struct hf*spi*device*config*t {
    hf*spi*mode*t mode;              ///< SPI mode (0-3)
    uint32*t frequency*hz;           ///< Clock frequency in Hz
    hf*spi*data*size*t data*size;    ///< Data size (8, 16, 32 bit)
    hf*pin*num*t cs*pin;             ///< Chip select pin
    bool cs*active*low;              ///< CS active low (true) or high (false)
    hf*spi*bit*order*t bit*order;    ///< Bit order (MSB or LSB first)
    uint32*t timeout*ms;             ///< Transfer timeout in milliseconds
    bool use*dma;                    ///< Use DMA for transfers
};
```text

### 📊 **Device Status**

```cpp
struct hf*spi*device*status*t {
    bool is*configured;        ///< Device is configured
    bool is*busy;              ///< Device is currently busy
    hf*spi*mode*t current*mode; ///< Current SPI mode
    uint32*t current*frequency; ///< Current frequency in Hz
    uint32*t bytes*transferred; ///< Total bytes transferred
    uint32*t transfer*errors;   ///< Number of transfer errors
    hf*spi*err*t last*error;    ///< Last error that occurred
    uint32*t timestamp*us;      ///< Timestamp of last operation
};
```text

### 📋 **SPI Capabilities**

```cpp
struct hf*spi*capabilities*t {
    uint8*t max*devices;           ///< Maximum number of devices
    uint32*t min*frequency*hz;     ///< Minimum frequency
    uint32*t max*frequency*hz;     ///< Maximum frequency
    uint8*t supported*modes;       ///< Bit mask of supported modes
    uint8*t supported*data*sizes;  ///< Bit mask of supported data sizes
    bool supports*dma;             ///< Supports DMA transfers
    bool supports*quad*spi;        ///< Supports quad SPI
    uint32*t max*transfer*size;    ///< Maximum transfer size in bytes
};
```text

### 📈 **SPI Statistics**

```cpp
struct hf*spi*statistics*t {
    uint32*t total*transfers;      ///< Total transfers performed
    uint32*t successful*transfers; ///< Successful transfers
    uint32*t failed*transfers;     ///< Failed transfers
    uint32*t bytes*transmitted;    ///< Total bytes transmitted
    uint32*t bytes*received;       ///< Total bytes received
    uint32*t average*transfer*time*us; ///< Average transfer time
    uint32*t max*transfer*time*us; ///< Maximum transfer time
    uint32*t min*transfer*time*us; ///< Minimum transfer time
    uint32*t timeout*errors;       ///< Timeout errors
    uint32*t communication*errors; ///< Communication errors
};
```text

---

## 📊 **Usage Examples**

### 📡 **Sensor Communication**

```cpp
#include "mcu/esp32/EspSpi.h"
#include "utils/memory*utils.h"

class SensorController {
private:
    EspSpi spi*;
    static constexpr uint8*t SENSOR*DEVICE = 0;
    
public:
    bool initialize() {
        spi* = EspSpi(SPI2*HOST, 5);  // SPI2, CS on pin 5
        
        if (!spi*.EnsureInitialized()) {
            printf("❌ SPI initialization failed\n");
            return false;
        }
        
        // Configure for sensor communication
        hf*spi*device*config*t config;
        config.mode = hf*spi*mode*t::MODE*0;
        config.frequency*hz = 1000000;  // 1 MHz
        config.data*size = hf*spi*data*size*t::DATA*8BIT;
        config.cs*pin = 5;
        config.cs*active*low = true;
        config.bit*order = hf*spi*bit*order*t::MSB*FIRST;
        config.timeout*ms = 100;
        config.use*dma = false;
        
        hf*spi*err*t result = spi*.ConfigureDevice(SENSOR*DEVICE, config);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Sensor configuration failed: %s\n", HfSpiErrToString(result));
            return false;
        }
        
        printf("✅ Sensor controller initialized\n");
        return true;
    }
    
    uint16*t read*temperature() {
        // Send temperature read command
        uint8*t tx*cmd[] = {0x03, 0x00, 0x00};  // Read temperature command
        uint8*t rx*data[3];
        
        hf*spi*err*t result = spi*.TransmitReceive(SENSOR*DEVICE, tx*cmd, rx*data, 3);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Temperature read failed: %s\n", HfSpiErrToString(result));
            return 0xFFFF;  // Error value
        }
        
        // Convert response to temperature (example conversion)
        uint16*t raw*temp = (rx*data[1] << 8) | rx*data[2];
        float temperature = (raw*temp * 175.0f / 65535.0f) - 45.0f;
        
        printf("🌡️ Temperature: %.1f°C\n", temperature);
        return raw*temp;
    }
    
    void write*config(uint8*t config*register, uint8*t value) {
        // Send configuration write command
        uint8*t tx*data[] = {0x02, config*register, value};  // Write command
        uint8*t rx*data[3];
        
        hf*spi*err*t result = spi*.TransmitReceive(SENSOR*DEVICE, tx*data, rx*data, 3);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            printf("✅ Config written: 0x%02X = 0x%02X\n", config*register, value);
        } else {
            printf("❌ Config write failed: %s\n", HfSpiErrToString(result));
        }
    }
    
    void read*sensor*data(uint8*t* data, size*t length) {
        // Read sensor data using nothrow allocation
        uint8*t tx*cmd[] = {0x04, 0x00};  // Read data command
        auto rx*data = hf::utils::make*unique*array*nothrow<uint8*t>(length + 2);
        if (!rx*data) {
            printf("❌ Failed to allocate memory for receive buffer\n");
            return;
        }
        
        hf*spi*err*t result = spi*.TransmitReceive(SENSOR*DEVICE, tx*cmd, rx*data.get(), length + 2);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            // Copy data (skip command bytes)
            memcpy(data, rx*data.get() + 2, length);
            printf("📊 Read %zu bytes of sensor data\n", length);
        } else {
            printf("❌ Sensor data read failed: %s\n", HfSpiErrToString(result));
        }
        
        // rx*data automatically cleaned up when going out of scope
    }
};
```text

### 🖥️ **Display Control**

```cpp
#include "mcu/esp32/EspSpi.h"
#include "utils/memory*utils.h"

class DisplayController {
private:
    EspSpi spi*;
    static constexpr uint8*t DISPLAY*DEVICE = 0;
    static constexpr uint16*t DISPLAY*WIDTH = 240;
    static constexpr uint16*t DISPLAY*HEIGHT = 320;
    
public:
    bool initialize() {
        spi* = EspSpi(SPI2*HOST, 15);  // SPI2, CS on pin 15
        
        if (!spi*.EnsureInitialized()) {
            return false;
        }
        
        // Configure for display communication
        hf*spi*device*config*t config;
        config.mode = hf*spi*mode*t::MODE*0;
        config.frequency*hz = 40000000;  // 40 MHz for display
        config.data*size = hf*spi*data*size*t::DATA*8BIT;
        config.cs*pin = 15;
        config.cs*active*low = true;
        config.bit*order = hf*spi*bit*order*t::MSB*FIRST;
        config.timeout*ms = 1000;
        config.use*dma = true;  // Use DMA for large transfers
        
        hf*spi*err*t result = spi*.ConfigureDevice(DISPLAY*DEVICE, config);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Display configuration failed\n");
            return false;
        }
        
        // Initialize display
        init*display();
        printf("✅ Display controller initialized\n");
        return true;
    }
    
private:
    void init*display() {
        // Display initialization sequence
        uint8*t init*commands[] = {
            0x01, 0x00,  // Software reset
            0x11, 0x00,  // Sleep out
            0x29, 0x00   // Display on
        };
        
        for (size*t i = 0; i < sizeof(init*commands); i += 2) {
            send*command(init*commands[i]);
            if (init*commands[i + 1] != 0) {
                send*data(&init*commands[i + 1], 1);
            }
            vTaskDelay(pdMS*TO*TICKS(10));
        }
    }
    
    void send*command(uint8*t command) {
        // Set DC pin low for command
        gpio*set*level(16, 0);  // DC pin on GPIO 16
        
        uint8*t rx*data;
        hf*spi*err*t result = spi*.TransmitReceive(DISPLAY*DEVICE, &command, &rx*data, 1);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Command send failed: %s\n", HfSpiErrToString(result));
        }
    }
    
    void send*data(const uint8*t* data, size*t length) {
        // Set DC pin high for data
        gpio*set*level(16, 1);  // DC pin on GPIO 16
        
        auto rx*data = hf::utils::make*unique*array*nothrow<uint8*t>(length);
        if (!rx*data) {
            printf("❌ Failed to allocate memory for receive buffer\n");
            return;
        }
        
        hf*spi*err*t result = spi*.TransmitReceive(DISPLAY*DEVICE, data, rx*data.get(), length);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Data send failed: %s\n", HfSpiErrToString(result));
        }
        
        // rx*data automatically cleaned up when going out of scope
    }
    
public:
    void set*window(uint16*t x*start, uint16*t y*start, uint16*t x*end, uint16*t y*end) {
        // Set display window for drawing
        uint8*t caset*cmd[] = {0x2A, 0x00, (x*start >> 8) & 0xFF, x*start & 0xFF,
                              0x00, (x*end >> 8) & 0xFF, x*end & 0xFF};
        uint8*t raset*cmd[] = {0x2B, 0x00, (y*start >> 8) & 0xFF, y*start & 0xFF,
                              0x00, (y*end >> 8) & 0xFF, y*end & 0xFF};
        
        send*command(0x2A);  // Column address set
        send*data(caset*cmd + 1, 6);
        
        send*command(0x2B);  // Row address set
        send*data(raset*cmd + 1, 6);
        
        send*command(0x2C);  // Memory write
    }
    
    void fill*screen(uint16*t color) {
        set*window(0, 0, DISPLAY*WIDTH - 1, DISPLAY*HEIGHT - 1);
        
        // Prepare color data
        uint8*t color*data[2] = {(color >> 8) & 0xFF, color & 0xFF};
        
        // Fill screen with color
        for (int i = 0; i < DISPLAY*WIDTH * DISPLAY*HEIGHT; i++) {
            send*data(color*data, 2);
        }
    }
    
    void draw*pixel(uint16*t x, uint16*t y, uint16*t color) {
        set*window(x, y, x, y);
        
        uint8*t color*data[2] = {(color >> 8) & 0xFF, color & 0xFF};
        send*data(color*data, 2);
    }
};
```text

### 💾 **Memory Access**

```cpp
#include "mcu/esp32/EspSpi.h"
#include "utils/memory*utils.h"

class MemoryController {
private:
    EspSpi spi*;
    static constexpr uint8*t MEMORY*DEVICE = 0;
    static constexpr uint32*t MEMORY*SIZE = 1024 * 1024;  // 1MB
    
public:
    bool initialize() {
        spi* = EspSpi(SPI2*HOST, 5);  // SPI2, CS on pin 5
        
        if (!spi*.EnsureInitialized()) {
            return false;
        }
        
        // Configure for memory access
        hf*spi*device*config*t config;
        config.mode = hf*spi*mode*t::MODE*0;
        config.frequency*hz = 80000000;  // 80 MHz for fast memory access
        config.data*size = hf*spi*data*size*t::DATA*8BIT;
        config.cs*pin = 5;
        config.cs*active*low = true;
        config.bit*order = hf*spi*bit*order*t::MSB*FIRST;
        config.timeout*ms = 100;
        config.use*dma = true;
        
        hf*spi*err*t result = spi*.ConfigureDevice(MEMORY*DEVICE, config);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Memory configuration failed\n");
            return false;
        }
        
        printf("✅ Memory controller initialized\n");
        return true;
    }
    
    bool read*memory(uint32*t address, uint8*t* data, size*t length) {
        // Send read command using nothrow allocation
        uint8*t tx*cmd[] = {0x03, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
        auto rx*data = hf::utils::make*unique*array*nothrow<uint8*t>(length + 4);
        if (!rx*data) {
            printf("❌ Failed to allocate memory for receive buffer\n");
            return false;
        }
        
        hf*spi*err*t result = spi*.TransmitReceive(MEMORY*DEVICE, tx*cmd, rx*data.get(), length + 4);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            // Copy data (skip command bytes)
            memcpy(data, rx*data.get() + 4, length);
            printf("📖 Read %zu bytes from address 0x%06X\n", length, address);
            return true;
        } else {
            printf("❌ Memory read failed: %s\n", HfSpiErrToString(result));
            return false;
        }
        // rx*data automatically cleaned up when going out of scope
    }
    
    bool write*memory(uint32*t address, const uint8*t* data, size*t length) {
        // Send write enable command
        uint8*t write*enable = 0x06;
        uint8*t rx*data;
        hf*spi*err*t result = spi*.TransmitReceive(MEMORY*DEVICE, &write*enable, &rx*data, 1);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Write enable failed\n");
            return false;
        }
        
        // Send write command using nothrow allocation
        auto tx*data = hf::utils::make*unique*array*nothrow<uint8*t>(length + 4);
        if (!tx*data) {
            printf("❌ Failed to allocate memory for transmit buffer\n");
            return false;
        }
        
        tx*data[0] = 0x02;  // Page program command
        tx*data[1] = (address >> 16) & 0xFF;
        tx*data[2] = (address >> 8) & 0xFF;
        tx*data[3] = address & 0xFF;
        memcpy(tx*data.get() + 4, data, length);
        
        result = spi*.Transmit(MEMORY*DEVICE, tx*data.get(), length + 4);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            printf("✍️ Wrote %zu bytes to address 0x%06X\n", length, address);
            return true;
        } else {
            printf("❌ Memory write failed: %s\n", HfSpiErrToString(result));
            return false;
        }
        // tx*data automatically cleaned up when going out of scope
    }
    
    bool erase*sector(uint32*t address) {
        // Send write enable command
        uint8*t write*enable = 0x06;
        uint8*t rx*data;
        hf*spi*err*t result = spi*.TransmitReceive(MEMORY*DEVICE, &write*enable, &rx*data, 1);
        if (result != hf*spi*err*t::SPI*SUCCESS) {
            printf("❌ Write enable failed\n");
            return false;
        }
        
        // Send sector erase command
        uint8*t erase*cmd[] = {0x20, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
        result = spi*.Transmit(MEMORY*DEVICE, erase*cmd, 4);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            printf("🗑️ Erased sector at address 0x%06X\n", address);
            return true;
        } else {
            printf("❌ Sector erase failed: %s\n", HfSpiErrToString(result));
            return false;
        }
    }
    
    uint32*t read*device*id() {
        uint8*t tx*cmd[] = {0x90, 0x00, 0x00, 0x00};  // Read ID command
        uint8*t rx*data[4];
        
        hf*spi*err*t result = spi*.TransmitReceive(MEMORY*DEVICE, tx*cmd, rx*data, 4);
        if (result == hf*spi*err*t::SPI*SUCCESS) {
            uint32*t device*id = (rx*data[1] << 16) | (rx*data[2] << 8) | rx*data[3];
            printf("🆔 Device ID: 0x%06X\n", device*id);
            return device*id;
        } else {
            printf("❌ Device ID read failed: %s\n", HfSpiErrToString(result));
            return 0;
        }
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (!spi.EnsureInitialized()) {
    printf("❌ SPI initialization failed\n");
    return false;
}

// ✅ Validate device configuration
hf*spi*capabilities*t caps;
if (spi.GetCapabilities(caps) == hf*spi*err*t::SPI*SUCCESS) {
    if (device*id >= caps.max*devices) {
        printf("❌ Device %u exceeds maximum (%u)\n", device*id, caps.max*devices);
        return;
    }
}

// ✅ Use appropriate frequency for your application
// Sensors: 1-10 MHz
// Displays: 10-40 MHz
// Memory: 20-80 MHz

// ✅ Handle transfer errors gracefully
hf*spi*err*t result = spi.TransmitReceive(device*id, tx*data, rx*data, length);
if (result != hf*spi*err*t::SPI*SUCCESS) {
    printf("⚠️ SPI Error: %s\n", HfSpiErrToString(result));
    // Implement retry logic or error recovery
}

// ✅ Use DMA for large transfers
config.use*dma = (length > 32);  // Use DMA for transfers > 32 bytes

// ✅ Check device status before operations
hf*spi*device*status*t status;
if (spi.GetDeviceStatus(device*id, status) == hf*spi*err*t::SPI*SUCCESS) {
    if (status.is*busy) {
        printf("⏳ Device %u is busy\n", device*id);
        return;
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore initialization
spi.TransmitReceive(0, tx*data, rx*data, length);  // May fail silently

// ❌ Don't use invalid frequencies
spi.ConfigureDevice(0, {mode: MODE*0, frequency*hz: 100000000});  // Too high

// ❌ Don't use invalid device numbers
spi.ConfigureDevice(99, config);  // Invalid device

// ❌ Don't ignore transfer timeouts
// Large transfers may timeout - check return values

// ❌ Don't assume all modes are supported
// Check capabilities before using specific modes

// ❌ Don't forget to handle CS pin manually when needed
// Some devices require manual CS control
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use appropriate frequency for your application
// Higher frequency = faster transfers but may cause errors

// 🚀 Use DMA for large transfers
// DMA reduces CPU overhead for transfers > 32 bytes

// 🚀 Minimize CS toggling
// Keep CS low for multiple transfers to the same device

// 🚀 Use appropriate data size
// 8-bit: Most common, good compatibility
// 16-bit: Faster for 16-bit data
// 32-bit: Fastest for 32-bit data

// 🚀 Batch operations when possible
// Configure all devices before starting transfers

// 🚀 Use appropriate timeout values
// Short timeouts for fast devices
// Longer timeouts for slow devices
```text

---

## 🔗 **Navigation**

### **Documentation Structure**

- **[🏠 Main Documentation](../README.md)** - Complete system overview
- **[📋 API Interfaces](README.md)** - Base classes and interfaces overview
- **[🔧 ESP32 Implementations](../esp_api/README.md)** - Hardware-specific implementations
- **[🧪 Test Suites](../../examples/esp32/docs/README.md)** - Testing and validation

### **Related Documentation**

- **[EspSpi Implementation](../esp_api/EspSpi.md)** - ESP32-C6 SPI implementation
- **[SPI Comprehensive Tests](../../examples/esp32/docs/README_SPI_TEST.md)** - Complete SPI validation
- **[Hardware Types](HardwareTypes.md)** - Type definitions and validation
- **[ESP-IDF SPI Master Driver](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/spi*master.html)** - Official ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: BaseI2c](BaseI2c.md) | [Back to API Index](README.md) | [Next: BaseUart →](BaseUart.md)

</div>

**🔌 BaseSpi - High-Speed Serial Communication for HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 