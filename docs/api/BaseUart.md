# 📡 BaseUart API Reference

<div align="center">

![BaseUart](https://img.shields.io/badge/BaseUart-Abstract%20Base%20Class-blue?style=for-the-badge&logo=broadcast)

**📡 Unified UART abstraction for serial communication**

**📋 Navigation**

[← Previous: BaseSpi](BaseSpi.md) | [Back to API Index](README.md) | [Next: BaseCan →](BaseCan.md)

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

The `BaseUart` class provides a comprehensive UART abstraction that serves as the unified interface
for all Universal Asynchronous Receiver-Transmitter operations in the HardFOC system.
It supports configurable baud rates, data formats, and flow control.

### ✨ **Key Features**

- 📡 **Configurable Baud Rates** - Support for standard and custom baud rates
- 🔧 **Flexible Data Formats** - Configurable data bits, stop bits, and parity
- 🔄 **Flow Control** - Hardware and software flow control support
- 📊 **DMA Support** - Hardware-accelerated data transfer
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Performance Optimized** - Minimal overhead for critical applications
- 🔌 **Platform Agnostic** - Works with various UART hardware implementations
- 📈 **Real-time Control** - Low-latency communication for time-critical applications

### 📡 **Supported Applications**

| Application | Baud Rate | Description |

|-------------|-----------|-------------|

| **Debug Output** | 115200 | Serial console and debugging |

| **GPS Communication** | 9600 | GPS module communication |

| **Bluetooth** | 115200 | Bluetooth module communication |

| **Modbus RTU** | 9600-115200 | Industrial protocol communication |

| **Sensor Communication** | 9600-115200 | Sensor data exchange |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseUart {
        <<abstract>>
        +Initialize() hf*uart*err*t
        +Deinitialize() hf*uart*err*t
        +Configure(config) hf*uart*err*t
        +Transmit(data, length) hf*uart*err*t
        +Receive(data, length) hf*uart*err*t
        +GetStatus(status) hf*uart*err*t
        +GetCapabilities(capabilities) hf*uart*err*t
    }
    
    class EspUart {
        +EspUart(port)
        +GetPort() uart*port*t
        +GetTxPin() hf*pin*num*t
        +GetRxPin() hf*pin*num*t
    }
    
    BaseUart <|-- EspUart
```text

---

## 📋 **Error Codes**

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `UART*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `UART*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `UART*ERR*NOT*INITIALIZED` | 2 | ⚠️ UART not initialized | Call Initialize() first |

| `UART*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ UART already initialized | Check initialization state |

| `UART*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `UART*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `UART*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### 📡 **Communication Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `UART*ERR*TRANSMIT*TIMEOUT` | 7 | ⏰ Transmit timeout | Check baud rate and flow control |

| `UART*ERR*RECEIVE*TIMEOUT` | 8 | ⏰ Receive timeout | Check data source and timing |

| `UART*ERR*TRANSMIT*FAILURE` | 9 | ❌ Transmit failed | Check connections and device state |

| `UART*ERR*RECEIVE*FAILURE` | 10 | ❌ Receive failed | Check connections and device state |

| `UART*ERR*FRAME*ERROR` | 11 | 📊 Frame error | Check baud rate and data format |

| `UART*ERR*PARITY*ERROR` | 12 | 🔍 Parity error | Check parity settings |

### ⚙️ **Configuration Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `UART*ERR*INVALID*CONFIGURATION` | 13 | ⚙️ Invalid configuration | Check configuration parameters |

| `UART*ERR*UNSUPPORTED*BAUD*RATE` | 14 | 🚫 Unsupported baud rate | Use supported baud rate |

| `UART*ERR*UNSUPPORTED*DATA*FORMAT` | 15 | 🚫 Unsupported data format | Use supported format |

| `UART*ERR*PIN*CONFLICT` | 16 | 🔌 Pin already in use | Use different pins |

| `UART*ERR*RESOURCE*BUSY` | 17 | 🔄 Resource busy | Wait for resource availability |

### 🌐 **Hardware Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `UART*ERR*HARDWARE*FAULT` | 18 | 💥 Hardware fault | Check power and connections |

| `UART*ERR*COMMUNICATION*FAILURE` | 19 | 📡 Communication failure | Check bus connections |

| `UART*ERR*DMA*ERROR` | 20 | 💾 DMA error | Check DMA configuration |

| `UART*ERR*BUFFER*OVERFLOW` | 21 | 📈 Buffer overflow | Increase buffer size |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the UART peripheral
 * @return hf*uart*err*t error code
 * 
 * 📝 Sets up UART hardware, configures pins, and prepares for communication.
 * Must be called before any UART operations.
 * 
 * @example
 * EspUart uart(UART*NUM*0);
 * hf*uart*err*t result = uart.Initialize();
 * if (result == hf*uart*err*t::UART*SUCCESS) {
 *     // UART ready for use
 * }
 */
virtual hf*uart*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the UART peripheral
 * @return hf*uart*err*t error code
 * 
 * 🧹 Cleanly shuts down UART hardware and releases resources.
 */
virtual hf*uart*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if UART is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
[[nodiscard]] bool IsInitialized() const noexcept;

/**
 * @brief Ensure UART is initialized (lazy initialization)
 * @return true if initialized successfully, false otherwise
 * 
 * 🔄 Automatically initializes UART if not already initialized.
 */
bool EnsureInitialized() noexcept;
```text

### ⚙️ **Configuration Methods**

```cpp
/**
 * @brief Configure UART parameters
 * @param config UART configuration structure
 * @return hf*uart*err*t error code
 * 
 * ⚙️ Configures UART parameters including baud rate, data format,
 * flow control, and pin assignments.
 * 
 * @example
 * hf*uart*config*t config;
 * config.baud*rate = 115200;
 * config.data*bits = hf*uart*data*bits*t::DATA*8BIT;
 * config.stop*bits = hf*uart*stop*bits*t::STOP*1BIT;
 * config.parity = hf*uart*parity*t::PARITY*NONE;
 * config.flow*control = hf*uart*flow*control*t::FLOW*NONE;
 * config.tx*pin = 1;
 * config.rx*pin = 3;
 * 
 * hf*uart*err*t result = uart.Configure(config);
 */
virtual hf*uart*err*t Configure(const hf*uart*config*t &config) noexcept = 0;

/**
 * @brief Get current UART configuration
 * @param config [out] Current configuration structure
 * @return hf*uart*err*t error code
 * 
 * 📊 Retrieves the current UART configuration.
 */
virtual hf*uart*err*t GetConfiguration(hf*uart*config*t &config) const noexcept = 0;
```text

### 📤 **Transmission Methods**

```cpp
/**
 * @brief Transmit data
 * @param data Data buffer to transmit
 * @param length Number of bytes to transmit
 * @return hf*uart*err*t error code
 * 
 * 📤 Transmits data over UART. Blocks until transmission is complete
 * or timeout occurs.
 * 
 * @example
 * const char* message = "Hello, World!\r\n";
 * hf*uart*err*t result = uart.Transmit(
 *     reinterpret*cast<const uint8*t*>(message), strlen(message));
 * if (result == hf*uart*err*t::UART*SUCCESS) {
 *     printf("✅ Message transmitted\n");
 * }
 */
virtual hf*uart*err*t Transmit(const uint8*t *data, size*t length) noexcept = 0;

/**
 * @brief Transmit data with timeout
 * @param data Data buffer to transmit
 * @param length Number of bytes to transmit
 * @param timeout*ms Timeout in milliseconds
 * @return hf*uart*err*t error code
 * 
 * 📤 Transmits data with specified timeout.
 */
virtual hf*uart*err*t Transmit(const uint8*t *data, size*t length, 
                              uint32*t timeout*ms) noexcept = 0;

/**
 * @brief Get number of bytes available for transmission
 * @return Number of bytes that can be transmitted
 * 
 * 📊 Returns the number of bytes that can be transmitted without blocking.
 */
virtual size*t GetTransmitSpace() const noexcept = 0;
```text

### 📥 **Reception Methods**

```cpp
/**
 * @brief Receive data
 * @param data Buffer to store received data
 * @param length Number of bytes to receive
 * @return hf*uart*err*t error code
 * 
 * 📥 Receives data from UART. Blocks until requested number of bytes
 * is received or timeout occurs.
 * 
 * @example
 * uint8*t buffer[64];
 * hf*uart*err*t result = uart.Receive(buffer, 64);
 * if (result == hf*uart*err*t::UART*SUCCESS) {
 *     printf("📥 Received %zu bytes\n", 64);
 * }
 */
virtual hf*uart*err*t Receive(uint8*t *data, size*t length) noexcept = 0;

/**
 * @brief Receive data with timeout
 * @param data Buffer to store received data
 * @param length Number of bytes to receive
 * @param timeout*ms Timeout in milliseconds
 * @return hf*uart*err*t error code
 * 
 * 📥 Receives data with specified timeout.
 */
virtual hf*uart*err*t Receive(uint8*t *data, size*t length, 
                             uint32*t timeout*ms) noexcept = 0;

/**
 * @brief Get number of bytes available for reception
 * @return Number of bytes available to receive
 * 
 * 📊 Returns the number of bytes available to receive without blocking.
 */
virtual size*t GetReceiveSpace() const noexcept = 0;

/**
 * @brief Flush receive buffer
 * @return hf*uart*err*t error code
 * 
 * 🧹 Clears all data in the receive buffer.
 */
virtual hf*uart*err*t FlushReceive() noexcept = 0;
```text

### 📊 **Status and Capabilities**

```cpp
/**
 * @brief Get UART status information
 * @param status [out] Status information structure
 * @return hf*uart*err*t error code
 * 
 * 📊 Retrieves comprehensive status information about UART.
 */
virtual hf*uart*err*t GetStatus(hf*uart*status*t &status) const noexcept = 0;

/**
 * @brief Get UART capabilities
 * @param capabilities [out] Capability information structure
 * @return hf*uart*err*t error code
 * 
 * 📋 Retrieves hardware capabilities and limitations.
 */
virtual hf*uart*err*t GetCapabilities(hf*uart*capabilities*t &capabilities) const noexcept = 0;
```text

---

## 📊 **Data Structures**

### ⚙️ **UART Configuration**

```cpp
struct hf*uart*config*t {
    uint32*t baud*rate;                    ///< Baud rate in bits per second
    hf*uart*data*bits*t data*bits;         ///< Number of data bits
    hf*uart*stop*bits*t stop*bits;         ///< Number of stop bits
    hf*uart*parity*t parity;               ///< Parity setting
    hf*uart*flow*control*t flow*control;   ///< Flow control setting
    hf*pin*num*t tx*pin;                   ///< Transmit pin
    hf*pin*num*t rx*pin;                   ///< Receive pin
    hf*pin*num*t rts*pin;                  ///< RTS pin (-1 if not used)
    hf*pin*num*t cts*pin;                  ///< CTS pin (-1 if not used)
    uint32*t timeout*ms;                   ///< Default timeout in milliseconds
    bool use*dma;                          ///< Use DMA for transfers
    size*t rx*buffer*size;                 ///< Receive buffer size
    size*t tx*buffer*size;                 ///< Transmit buffer size
};
```text

### 📊 **UART Status**

```cpp
struct hf*uart*status*t {
    bool is*initialized;        ///< UART is initialized
    bool is*configured;         ///< UART is configured
    uint32*t current*baud*rate; ///< Current baud rate
    size*t rx*bytes*available;  ///< Bytes available to receive
    size*t tx*space*available;  ///< Space available for transmission
    uint32*t rx*errors;         ///< Number of receive errors
    uint32*t tx*errors;         ///< Number of transmit errors
    hf*uart*err*t last*error;   ///< Last error that occurred
    uint32*t timestamp*us;      ///< Timestamp of last operation
};
```text

### 📋 **UART Capabilities**

```cpp
struct hf*uart*capabilities*t {
    uint8*t max*ports;              ///< Maximum number of UART ports
    uint32*t min*baud*rate;         ///< Minimum baud rate
    uint32*t max*baud*rate;         ///< Maximum baud rate
    uint8*t supported*data*bits;    ///< Bit mask of supported data bits
    uint8*t supported*stop*bits;    ///< Bit mask of supported stop bits
    uint8*t supported*parity;       ///< Bit mask of supported parity
    bool supports*flow*control;     ///< Supports hardware flow control
    bool supports*dma;              ///< Supports DMA transfers
    size*t max*buffer*size;         ///< Maximum buffer size
    uint32*t max*timeout*ms;        ///< Maximum timeout value
};
```text

### 📈 **UART Statistics**

```cpp
struct hf*uart*statistics*t {
    uint32*t total*transmissions;   ///< Total transmissions performed
    uint32*t successful*transmissions; ///< Successful transmissions
    uint32*t failed*transmissions;  ///< Failed transmissions
    uint32*t total*receptions;      ///< Total receptions performed
    uint32*t successful*receptions; ///< Successful receptions
    uint32*t failed*receptions;     ///< Failed receptions
    uint32*t bytes*transmitted;     ///< Total bytes transmitted
    uint32*t bytes*received;        ///< Total bytes received
    uint32*t frame*errors;          ///< Number of frame errors
    uint32*t parity*errors;         ///< Number of parity errors
    uint32*t buffer*overflows;      ///< Number of buffer overflows
    uint32*t average*transmit*time*us; ///< Average transmit time
    uint32*t max*transmit*time*us;  ///< Maximum transmit time
    uint32*t min*transmit*time*us;  ///< Minimum transmit time
};
```text

---

## 📊 **Usage Examples**

### 📡 **Basic Serial Communication**

```cpp
#include "mcu/esp32/EspUart.h"

class SerialCommunicator {
private:
    EspUart uart*;
    
public:
    bool initialize() {
        uart* = EspUart(UART*NUM*0);
        
        if (!uart*.EnsureInitialized()) {
            printf("❌ UART initialization failed\n");
            return false;
        }
        
        // Configure for standard serial communication
        hf*uart*config*t config;
        config.baud*rate = 115200;
        config.data*bits = hf*uart*data*bits*t::DATA*8BIT;
        config.stop*bits = hf*uart*stop*bits*t::STOP*1BIT;
        config.parity = hf*uart*parity*t::PARITY*NONE;
        config.flow*control = hf*uart*flow*control*t::FLOW*NONE;
        config.tx*pin = 1;  // GPIO 1
        config.rx*pin = 3;  // GPIO 3
        config.rts*pin = -1;  // Not used
        config.cts*pin = -1;  // Not used
        config.timeout*ms = 1000;
        config.use*dma = false;
        config.rx*buffer*size = 1024;
        config.tx*buffer*size = 1024;
        
        hf*uart*err*t result = uart*.Configure(config);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ UART configuration failed: %s\n", HfUartErrToString(result));
            return false;
        }
        
        printf("✅ Serial communicator initialized\n");
        return true;
    }
    
    void send*message(const char* message) {
        size*t length = strlen(message);
        hf*uart*err*t result = uart*.Transmit(
            reinterpret*cast<const uint8*t*>(message), length);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            printf("📤 Sent: %s", message);
        } else {
            printf("❌ Send failed: %s\n", HfUartErrToString(result));
        }
    }
    
    void receive*message(char* buffer, size*t max*length) {
        hf*uart*err*t result = uart*.Receive(
            reinterpret*cast<uint8*t*>(buffer), max*length);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            buffer[max*length] = '\0';  // Null terminate
            printf("📥 Received: %s", buffer);
        } else {
            printf("❌ Receive failed: %s\n", HfUartErrToString(result));
        }
    }
    
    void echo*loop() {
        char buffer[256];
        printf("🔄 Starting echo loop...\n");
        
        while (true) {
            // Check if data is available
            if (uart*.GetReceiveSpace() > 0) {
                hf*uart*err*t result = uart*.Receive(
                    reinterpret*cast<uint8*t*>(buffer), 255, 100);
                
                if (result == hf*uart*err*t::UART*SUCCESS) {
                    buffer[255] = '\0';
                    printf("📥 Echo: %s", buffer);
                    
                    // Echo back
                    uart*.Transmit(reinterpret*cast<const uint8*t*>(buffer), 
                                 strlen(buffer));
                }
            }
            
            vTaskDelay(pdMS*TO*TICKS(10));
        }
    }
};
```text

### 🗺️ **GPS Communication**

```cpp
#include "mcu/esp32/EspUart.h"

class GpsController {
private:
    EspUart uart*;
    static constexpr size*t GPS*BUFFER*SIZE = 512;
    char gps*buffer*[GPS*BUFFER*SIZE];
    
public:
    bool initialize() {
        uart* = EspUart(UART*NUM*1);
        
        if (!uart*.EnsureInitialized()) {
            return false;
        }
        
        // Configure for GPS communication
        hf*uart*config*t config;
        config.baud*rate = 9600;  // Standard GPS baud rate
        config.data*bits = hf*uart*data*bits*t::DATA*8BIT;
        config.stop*bits = hf*uart*stop*bits*t::STOP*1BIT;
        config.parity = hf*uart*parity*t::PARITY*NONE;
        config.flow*control = hf*uart*flow*control*t::FLOW*NONE;
        config.tx*pin = 17;  // GPS TX
        config.rx*pin = 16;  // GPS RX
        config.rts*pin = -1;
        config.cts*pin = -1;
        config.timeout*ms = 5000;  // 5 second timeout
        config.use*dma = false;
        config.rx*buffer*size = GPS*BUFFER*SIZE;
        config.tx*buffer*size = 256;
        
        hf*uart*err*t result = uart*.Configure(config);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ GPS configuration failed\n");
            return false;
        }
        
        printf("✅ GPS controller initialized\n");
        return true;
    }
    
    bool read*gps*data() {
        // Read GPS data with timeout
        hf*uart*err*t result = uart*.Receive(
            reinterpret*cast<uint8*t*>(gps*buffer*), GPS*BUFFER*SIZE - 1, 1000);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            gps*buffer*[GPS*BUFFER*SIZE - 1] = '\0';
            
            // Parse NMEA sentences
            parse*nmea*data(gps*buffer*);
            return true;
        } else if (result == hf*uart*err*t::UART*ERR*RECEIVE*TIMEOUT) {
            printf("⏰ GPS timeout - no data received\n");
            return false;
        } else {
            printf("❌ GPS read failed: %s\n", HfUartErrToString(result));
            return false;
        }
    }
    
private:
    void parse*nmea*data(const char* data) {
        // Simple NMEA parser - look for GPRMC sentences
        char* line = strtok(const*cast<char*>(data), "\r\n");
        while (line != nullptr) {
            if (strncmp(line, "$GPRMC", 6) == 0) {
                parse*gprmc(line);
            } else if (strncmp(line, "$GPGGA", 6) == 0) {
                parse*gpgga(line);
            }
            line = strtok(nullptr, "\r\n");
        }
    }
    
    void parse*gprmc(const char* sentence) {
        // Parse GPRMC sentence for time, date, position, speed
        printf("📍 GPRMC: %s\n", sentence);
        // Add actual parsing logic here
    }
    
    void parse*gpgga(const char* sentence) {
        // Parse GPGGA sentence for position and altitude
        printf("🌍 GPGGA: %s\n", sentence);
        // Add actual parsing logic here
    }
    
public:
    void send*gps*command(const char* command) {
        // Send command to GPS module
        size*t length = strlen(command);
        hf*uart*err*t result = uart*.Transmit(
            reinterpret*cast<const uint8*t*>(command), length);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            printf("📤 GPS Command: %s", command);
        } else {
            printf("❌ GPS command failed: %s\n", HfUartErrToString(result));
        }
    }
    
    void gps*monitoring*task() {
        printf("🗺️ Starting GPS monitoring...\n");
        
        while (true) {
            if (read*gps*data()) {
                // Data received and parsed
                vTaskDelay(pdMS*TO*TICKS(100));
            } else {
                // No data or error
                vTaskDelay(pdMS*TO*TICKS(1000));
            }
        }
    }
};
```text

### 🔵 **Bluetooth Communication**

```cpp
#include "mcu/esp32/EspUart.h"

class BluetoothController {
private:
    EspUart uart*;
    static constexpr size*t BT*BUFFER*SIZE = 1024;
    char bt*buffer*[BT*BUFFER*SIZE];
    
public:
    bool initialize() {
        uart* = EspUart(UART*NUM*2);
        
        if (!uart*.EnsureInitialized()) {
            return false;
        }
        
        // Configure for Bluetooth communication
        hf*uart*config*t config;
        config.baud*rate = 115200;  // Standard BT baud rate
        config.data*bits = hf*uart*data*bits*t::DATA*8BIT;
        config.stop*bits = hf*uart*stop*bits*t::STOP*1BIT;
        config.parity = hf*uart*parity*t::PARITY*NONE;
        config.flow*control = hf*uart*flow*control*t::FLOW*NONE;
        config.tx*pin = 25;  // BT TX
        config.rx*pin = 26;  // BT RX
        config.rts*pin = -1;
        config.cts*pin = -1;
        config.timeout*ms = 1000;
        config.use*dma = true;  // Use DMA for BT
        config.rx*buffer*size = BT*BUFFER*SIZE;
        config.tx*buffer*size = BT*BUFFER*SIZE;
        
        hf*uart*err*t result = uart*.Configure(config);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ Bluetooth configuration failed\n");
            return false;
        }
        
        printf("✅ Bluetooth controller initialized\n");
        return true;
    }
    
    void send*data(const char* data) {
        size*t length = strlen(data);
        hf*uart*err*t result = uart*.Transmit(
            reinterpret*cast<const uint8*t*>(data), length);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            printf("📤 BT Sent: %s", data);
        } else {
            printf("❌ BT send failed: %s\n", HfUartErrToString(result));
        }
    }
    
    bool receive*data(char* buffer, size*t max*length) {
        hf*uart*err*t result = uart*.Receive(
            reinterpret*cast<uint8*t*>(buffer), max*length - 1, 100);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            buffer[max*length - 1] = '\0';
            printf("📥 BT Received: %s", buffer);
            return true;
        } else if (result == hf*uart*err*t::UART*ERR*RECEIVE*TIMEOUT) {
            return false;  // No data available
        } else {
            printf("❌ BT receive failed: %s\n", HfUartErrToString(result));
            return false;
        }
    }
    
    void bluetooth*chat*task() {
        printf("🔵 Starting Bluetooth chat...\n");
        
        char input*buffer[256];
        char output*buffer[256];
        
        while (true) {
            // Check for incoming data
            if (receive*data(input*buffer, sizeof(input*buffer))) {
                // Process received data
                snprintf(output*buffer, sizeof(output*buffer), 
                        "Echo: %s", input*buffer);
                send*data(output*buffer);
            }
            
            // Check for local input (simulated)
            if (/* local input available */) {
                snprintf(output*buffer, sizeof(output*buffer), 
                        "Local: Hello from ESP32!\n");
                send*data(output*buffer);
            }
            
            vTaskDelay(pdMS*TO*TICKS(100));
        }
    }
};
```text

### 🏭 **Modbus RTU Communication**

```cpp
#include "mcu/esp32/EspUart.h"

class ModbusController {
private:
    EspUart uart*;
    static constexpr uint8*t MODBUS*SLAVE*ADDRESS = 1;
    static constexpr size*t MODBUS*BUFFER*SIZE = 256;
    uint8*t modbus*buffer*[MODBUS*BUFFER*SIZE];
    
public:
    bool initialize() {
        uart* = EspUart(UART*NUM*1);
        
        if (!uart*.EnsureInitialized()) {
            return false;
        }
        
        // Configure for Modbus RTU
        hf*uart*config*t config;
        config.baud*rate = 9600;  // Standard Modbus baud rate
        config.data*bits = hf*uart*data*bits*t::DATA*8BIT;
        config.stop*bits = hf*uart*stop*bits*t::STOP*1BIT;
        config.parity = hf*uart*parity*t::PARITY*EVEN;  // Modbus standard
        config.flow*control = hf*uart*flow*control*t::FLOW*NONE;
        config.tx*pin = 17;
        config.rx*pin = 16;
        config.rts*pin = -1;
        config.cts*pin = -1;
        config.timeout*ms = 1000;
        config.use*dma = false;
        config.rx*buffer*size = MODBUS*BUFFER*SIZE;
        config.tx*buffer*size = MODBUS*BUFFER*SIZE;
        
        hf*uart*err*t result = uart*.Configure(config);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ Modbus configuration failed\n");
            return false;
        }
        
        printf("✅ Modbus controller initialized\n");
        return true;
    }
    
    bool read*holding*registers(uint8*t slave*addr, uint16*t start*addr, 
                               uint16*t count, uint16*t* data) {
        // Build Modbus RTU read holding registers request
        uint8*t request[8];
        request[0] = slave*addr;           // Slave address
        request[1] = 0x03;                 // Function code (read holding registers)
        request[2] = (start*addr >> 8) & 0xFF;  // Starting address high
        request[3] = start*addr & 0xFF;         // Starting address low
        request[4] = (count >> 8) & 0xFF;       // Quantity high
        request[5] = count & 0xFF;              // Quantity low
        
        // Calculate CRC
        uint16*t crc = calculate*crc16(request, 6);
        request[6] = crc & 0xFF;           // CRC low
        request[7] = (crc >> 8) & 0xFF;    // CRC high
        
        // Send request
        hf*uart*err*t result = uart*.Transmit(request, 8);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ Modbus request failed: %s\n", HfUartErrToString(result));
            return false;
        }
        
        // Receive response
        size*t response*length = 5 + count * 2;  // Header + data + CRC
        result = uart*.Receive(modbus*buffer*, response*length, 1000);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            // Verify response
            if (modbus*buffer*[0] == slave*addr && modbus*buffer*[1] == 0x03) {
                // Extract data
                for (int i = 0; i < count; i++) {
                    data[i] = (modbus*buffer*[3 + i * 2] << 8) | 
                              modbus*buffer*[4 + i * 2];
                }
                printf("✅ Read %d holding registers\n", count);
                return true;
            } else {
                printf("❌ Invalid Modbus response\n");
                return false;
            }
        } else {
            printf("❌ Modbus response failed: %s\n", HfUartErrToString(result));
            return false;
        }
    }
    
    bool write*single*register(uint8*t slave*addr, uint16*t addr, uint16*t value) {
        // Build Modbus RTU write single register request
        uint8*t request[8];
        request[0] = slave*addr;           // Slave address
        request[1] = 0x06;                 // Function code (write single register)
        request[2] = (addr >> 8) & 0xFF;   // Register address high
        request[3] = addr & 0xFF;          // Register address low
        request[4] = (value >> 8) & 0xFF;  // Value high
        request[5] = value & 0xFF;         // Value low
        
        // Calculate CRC
        uint16*t crc = calculate*crc16(request, 6);
        request[6] = crc & 0xFF;           // CRC low
        request[7] = (crc >> 8) & 0xFF;    // CRC high
        
        // Send request
        hf*uart*err*t result = uart*.Transmit(request, 8);
        if (result != hf*uart*err*t::UART*SUCCESS) {
            printf("❌ Modbus write request failed: %s\n", HfUartErrToString(result));
            return false;
        }
        
        // Receive response (should be echo of request)
        result = uart*.Receive(modbus*buffer*, 8, 1000);
        
        if (result == hf*uart*err*t::UART*SUCCESS) {
            if (memcmp(request, modbus*buffer*, 8) == 0) {
                printf("✅ Wrote register 0x%04X = 0x%04X\n", addr, value);
                return true;
            } else {
                printf("❌ Invalid Modbus write response\n");
                return false;
            }
        } else {
            printf("❌ Modbus write response failed: %s\n", HfUartErrToString(result));
            return false;
        }
    }
    
private:
    uint16*t calculate*crc16(const uint8*t* data, size*t length) {
        uint16*t crc = 0xFFFF;
        
        for (size*t i = 0; i < length; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc = crc >> 1;
                }
            }
        }
        
        return crc;
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (!uart.EnsureInitialized()) {
    printf("❌ UART initialization failed\n");
    return false;
}

// ✅ Use appropriate baud rates
// Debug: 115200
// GPS: 9600
// Bluetooth: 115200
// Modbus: 9600-115200

// ✅ Handle timeouts gracefully
hf*uart*err*t result = uart.Receive(buffer, length, 1000);
if (result == hf*uart*err*t::UART*ERR*RECEIVE*TIMEOUT) {
    printf("⏰ No data received within timeout\n");
    return false;
}

// ✅ Check buffer space before operations
if (uart.GetReceiveSpace() > 0) {
    // Data available to receive
}

if (uart.GetTransmitSpace() >= length) {
    // Space available to transmit
}

// ✅ Use appropriate data formats
// Most applications: 8N1 (8 data bits, no parity, 1 stop bit)
// Modbus: 8E1 (8 data bits, even parity, 1 stop bit)

// ✅ Monitor statistics for system health
hf*uart*statistics*t stats;
if (uart.GetStatistics(stats) == hf*uart*err*t::UART*SUCCESS) {
    if (stats.frame*errors > 10) {
        printf("⚠️ High frame error rate detected\n");
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore initialization
uart.Transmit(data, length);  // May fail silently

// ❌ Don't use mismatched baud rates
// Both devices must use the same baud rate

// ❌ Don't ignore buffer overflows
// Check buffer space before large transfers

// ❌ Don't use without error checking in critical applications
// Always check return values in safety-critical systems

// ❌ Don't forget to handle flow control
// Some devices require hardware flow control

// ❌ Don't assume all data formats are supported
// Check capabilities before using specific formats
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use DMA for large transfers
config.use*dma = (transfer*size > 64);  // Use DMA for transfers > 64 bytes

// 🚀 Use appropriate buffer sizes
// Small buffers: Lower memory usage, more frequent interrupts
// Large buffers: Higher memory usage, fewer interrupts

// 🚀 Minimize timeout values
// Short timeouts for fast devices
// Longer timeouts for slow devices

// 🚀 Use appropriate baud rates
// Higher baud rate = faster communication but may cause errors

// 🚀 Batch operations when possible
// Send multiple commands in one transfer

// 🚀 Use flow control when needed
// Prevents buffer overflows in high-speed communication
```text

---

## 🔗 **Related Documentation**

- [⚙️ **EspUart**](../esp_api/EspUart.md) - ESP32-C6 implementation
- [🎛️ **Hardware Types**](HardwareTypes.md) - Platform-agnostic types

---

<div align="center">

**📋 Navigation**

[← Previous: BaseSpi](BaseSpi.md) | [Back to API Index](README.md) | [Next: BaseCan →](BaseCan.md)

</div>

---

<div align="center">

**📡 BaseUart - Reliable Serial Communication for HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 