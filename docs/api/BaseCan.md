# 🚌 BaseCan API Reference

<div align="center">

![BaseCan](https://img.shields.io/badge/BaseCan-Abstract%20Base%20Class-blue?style=for-the-badge&logo=bus)

**🎯 Unified CAN bus abstraction for all Controller Area Network operations**

**📋 Navigation**

[← Previous: BaseUart](BaseUart.md) | [Back to API Index](README.md) | [Next: BaseWifi
→](BaseWifi.md)

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

The `BaseCan` class provides a comprehensive CAN bus abstraction that serves as the
unified interface for all Controller Area Network operations in the HardFOC system.
It supports both classic CAN and CAN-FD protocols, message filtering, error
handling, and works across different CAN controller implementations.

### ✨ **Key Features**

- 🚌 **CAN 2.0A/2.0B Support** - Classic CAN protocols (CAN-FD support varies by hardware)
- 📨 **Message Filtering** - Hardware-based acceptance filtering
- 🔄 **Error Recovery** - Automatic bus recovery and error handling
- 📊 **Statistics & Diagnostics** - Comprehensive monitoring and reporting
- ⚡ **High Performance** - Optimized for real-time applications
- 🛡️ **Robust Error Handling** - Detailed error codes and recovery mechanisms
- 🔌 **Platform Agnostic** - Works with internal and external CAN controllers
- 🧵 **Thread Safe** - Designed for multi-threaded applications

### 📊 **Supported Hardware**

| Implementation | Hardware Type | Protocol | Speed | Features |

|----------------|---------------|----------|-------|----------|

| `EspCan` | ESP32-C6 Internal | CAN 2.0A/B | Up to 1 Mbps | Built-in error handling, no CAN-FD |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseCan {
        <<abstract>>
        +Initialize() hf*can*err*t
        +Deinitialize() hf*can*err*t
        +SendMessage(message, timeout) hf*can*err*t
        +ReceiveMessage(message, timeout) hf*can*err*t
        +SetReceiveCallback(callback) hf*can*err*t
        +SetAcceptanceFilter(id, mask, extended) hf*can*err*t
        +GetStatus(status) hf*can*err*t
        +ResetStatistics() hf*can*err*t
        +ResetDiagnostics() hf*can*err*t
    }
    
    class EspCan {
        +EspCan(config)
        +GetControllerId() uint8*t
        +SetBaudRate(baudrate) hf*can*err*t
    }
    
    BaseCan <|-- EspCan
```text

---

## 📋 **Error Codes**

The CAN system uses comprehensive error codes for robust error handling:

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `CAN*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `CAN*ERR*NOT*INITIALIZED` | 2 | ⚠️ CAN not initialized | Call Initialize() first |

| `CAN*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ CAN already initialized | Check initialization state |

| `CAN*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `CAN*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `CAN*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### 🚌 **Bus Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*BUS*OFF` | 7 | 🚫 Bus off state | Restart CAN controller |

| `CAN*ERR*BUS*ERROR` | 8 | ❌ Bus error | Check bus wiring and termination |

| `CAN*ERR*BUS*BUSY` | 9 | 🔄 Bus busy | Wait for bus availability |

| `CAN*ERR*BUS*NOT*AVAILABLE` | 10 | 🚫 Bus not available | Check bus configuration |

| `CAN*ERR*BUS*RECOVERY*FAILED` | 11 | ❌ Bus recovery failed | Restart CAN controller |

| `CAN*ERR*BUS*ARBITRATION*LOST` | 12 | 🔄 Bus arbitration lost | Normal in multi-node systems |

### 📨 **Message Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*MESSAGE*TIMEOUT` | 13 | ⏰ Message timeout | Check bus load and timing |

| `CAN*ERR*MESSAGE*LOST` | 14 | 📤 Message lost | Check buffer sizes |

| `CAN*ERR*MESSAGE*INVALID` | 15 | ❌ Invalid message | Check message format |

| `CAN*ERR*MESSAGE*TOO*LONG` | 16 | 📏 Message too long | Check DLC value |

| `CAN*ERR*MESSAGE*INVALID*ID` | 17 | 🆔 Invalid message ID | Check ID range |

| `CAN*ERR*MESSAGE*INVALID*DLC` | 18 | 📊 Invalid DLC | Check data length |

| `CAN*ERR*QUEUE*FULL` | 19 | 📦 Queue full | Increase queue size or process faster |

| `CAN*ERR*QUEUE*EMPTY` | 20 | 📭 Queue empty | Check message reception |

### 📤 **Transmission Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*TX*FAILED` | 21 | ❌ Transmission failed | Check bus state and wiring |

| `CAN*ERR*TX*ABORTED` | 22 | 🚫 Transmission aborted | Check bus errors |

| `CAN*ERR*TX*ERROR*PASSIVE` | 23 | ⚠️ Transmit error passive | Check error counters |

| `CAN*ERR*TX*ERROR*WARNING` | 24 | ⚠️ Transmit error warning | Monitor error counters |

### 📥 **Reception Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*RX*OVERRUN` | 25 | 📈 Receive overrun | Process messages faster |

| `CAN*ERR*RX*ERROR*PASSIVE` | 26 | ⚠️ Receive error passive | Check error counters |

| `CAN*ERR*RX*ERROR*WARNING` | 27 | ⚠️ Receive error warning | Monitor error counters |

| `CAN*ERR*RX*FIFO*FULL` | 28 | 📦 Receive FIFO full | Process messages faster |

### 🌐 **Hardware Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*HARDWARE*FAULT` | 29 | 💥 Hardware fault | Check power and connections |

| `CAN*ERR*COMMUNICATION*FAILURE` | 30 | 📡 Communication failure | Check interface connections |

| `CAN*ERR*DEVICE*NOT*RESPONDING` | 31 | 🔇 Device not responding | Check device power and address |

| `CAN*ERR*VOLTAGE*OUT*OF*RANGE` | 32 | ⚡ Voltage out of range | Check power supply |

| `CAN*ERR*CLOCK*ERROR` | 33 | ⏰ Clock error | Check clock configuration |

| `CAN*ERR*TRANSCEIVER*ERROR` | 34 | 🔌 Transceiver error | Check transceiver connections |

### ⚙️ **Configuration Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*INVALID*CONFIGURATION` | 35 | ⚙️ Invalid configuration | Check configuration parameters |

| `CAN*ERR*UNSUPPORTED*OPERATION` | 36 | 🚫 Unsupported operation | Check hardware capabilities |

| `CAN*ERR*INVALID*BAUD*RATE` | 37 | 📊 Invalid baud rate | Use supported baud rate |

| `CAN*ERR*INVALID*CONTROLLER*ID` | 38 | 🆔 Invalid controller ID | Use valid controller ID |

| `CAN*ERR*FILTER*ERROR` | 39 | 🔍 Filter error | Check filter configuration |

| `CAN*ERR*FILTER*FULL` | 40 | 📦 Filter table full | Reduce number of filters |

### 🔧 **Protocol Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `CAN*ERR*STUFF*ERROR` | 41 | 🔧 Bit stuffing error | Check bus quality |

| `CAN*ERR*FORM*ERROR` | 42 | 📋 Frame format error | Check message format |

| `CAN*ERR*CRC*ERROR` | 43 | 🔢 CRC error | Check bus integrity |

| `CAN*ERR*ACK*ERROR` | 44 | ✅ Acknowledgment error | Check bus termination |

| `CAN*ERR*BIT*ERROR` | 45 | 🔌 Bit error | Check bus quality |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the CAN controller
 * @return hf*can*err*t error code
 * 
 * 📝 Sets up CAN hardware, configures pins, and prepares for communication.
 * Must be called before any CAN operations.
 * 
 * @example
 * EspCan can(config);
 * if (can.Initialize() == hf*can*err*t::CAN*SUCCESS) {
 *     // CAN ready for use
 * }
 */
virtual hf*can*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the CAN controller
 * @return hf*can*err*t error code
 * 
 * 🧹 Cleanly shuts down CAN hardware and releases resources.
 */
virtual hf*can*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if CAN is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
[[nodiscard]] bool IsInitialized() const noexcept;

/**
 * @brief Ensure CAN is initialized (lazy initialization)
 * @return true if initialized successfully, false otherwise
 * 
 * 🔄 Automatically initializes CAN if not already initialized.
 */
bool EnsureInitialized() noexcept;

/**
 * @brief Ensure CAN is deinitialized (lazy deinitialization)
 * @return true if deinitialized successfully, false otherwise
 * 
 * 🔄 Automatically deinitializes CAN if currently initialized.
 */
bool EnsureDeinitialized() noexcept;
```text

### 📨 **Message Transmission**

```cpp
/**
 * @brief Send a CAN message
 * @param message CAN message to send
 * @param timeout*ms Timeout in milliseconds (0 = non-blocking)
 * @return hf*can*err*t error code
 * 
 * 📤 Transmits a CAN message with optional timeout.
 * Supports both classic CAN and CAN-FD messages.
 * 
 * @example
 * hf*can*message*t msg;
 * msg.id = 0x123;
 * msg.dlc = 8;
 * msg.data[0] = 0x01;
 * msg.data[1] = 0x02;
 * // ... set other data bytes
 * 
 * hf*can*err*t result = can.SendMessage(msg, 1000);
 * if (result != hf*can*err*t::CAN*SUCCESS) {
 *     printf("Send failed: %s\n", HfCanErrToString(result));
 * }
 */
virtual hf*can*err*t SendMessage(const hf*can*message*t &message, uint32*t timeout*ms = 1000)
noexcept = 0;

/**
 * @brief Send multiple messages in batch
 * @param messages Array of messages to send
 * @param count Number of messages
 * @param timeout*ms Timeout in milliseconds
 * @return Number of messages successfully sent
 * 
 * 📦 Transmits multiple messages efficiently.
 * Returns the number of messages actually sent.
 * 
 * @example
 * hf*can*message*t messages[3];
 * // ... populate messages
 * uint32*t sent = can.SendMessageBatch(messages, 3, 1000);
 * printf("Sent %u of 3 messages\n", sent);
 */
virtual uint32*t SendMessageBatch(const hf*can*message*t *messages, uint32*t count,
                                uint32*t timeout*ms = 1000) noexcept;
```text

### 📥 **Message Reception**

```cpp
/**
 * @brief Receive a CAN message
 * @param message Reference to store received message
 * @param timeout*ms Timeout in milliseconds (0 = non-blocking)
 * @return hf*can*err*t error code
 * 
 * 📥 Receives a CAN message with optional timeout.
 * Returns CAN*ERR*QUEUE*EMPTY if no message available.
 * 
 * @example
 * hf*can*message*t received*msg;
 * hf*can*err*t result = can.ReceiveMessage(received*msg, 100);
 * if (result == hf*can*err*t::CAN*SUCCESS) {
 *     printf("Received ID: 0x%03X, Data: ", received*msg.id);
 *     for (int i = 0; i < received*msg.dlc; i++) {
 *         printf("%02X ", received*msg.data[i]);
 *     }
 *     printf("\n");
 * }
 */
virtual hf*can*err*t ReceiveMessage(hf*can*message*t &message, uint32*t timeout*ms = 0) noexcept =
0;

/**
 * @brief Receive multiple messages in batch
 * @param messages Array to store received messages
 * @param max*count Maximum number of messages to receive
 * @param timeout*ms Timeout in milliseconds
 * @return Number of messages received
 * 
 * 📦 Receives multiple messages efficiently.
 * Returns the number of messages actually received.
 * 
 * @example
 * hf*can*message*t messages[10];
 * uint32*t received = can.ReceiveMessageBatch(messages, 10, 100);
 * printf("Received %u messages\n", received);
 */
virtual uint32*t ReceiveMessageBatch(hf*can*message*t *messages, uint32*t max*count,
                                   uint32*t timeout*ms = 100) noexcept;
```text

### 🔍 **Message Filtering**

```cpp
/**
 * @brief Set acceptance filter
 * @param id Filter ID
 * @param mask Filter mask
 * @param extended Extended frame flag
 * @return hf*can*err*t error code
 * 
 * 🔍 Configures hardware-based message filtering.
 * Only messages matching the filter will be received.
 * 
 * @example
 * // Accept only messages with ID 0x100-0x1FF
 * can.SetAcceptanceFilter(0x100, 0x700, false);
 * 
 * // Accept only extended frame with ID 0x18FF0000
 * can.SetAcceptanceFilter(0x18FF0000, 0x1FFFFFFF, true);
 */
virtual hf*can*err*t SetAcceptanceFilter(uint32*t id, uint32*t mask, bool extended = false) noexcept
= 0;

/**
 * @brief Clear acceptance filter
 * @return hf*can*err*t error code
 * 
 * 🔄 Removes all acceptance filters.
 * All messages will be received.
 */
virtual hf*can*err*t ClearAcceptanceFilter() noexcept;
```text

### 📞 **Callback Management**

```cpp
/**
 * @brief Set receive callback function
 * @param callback Callback function
 * @return hf*can*err*t error code
 * 
 * 📞 Sets callback function for asynchronous message reception.
 * Callback is invoked when messages are received.
 * 
 * @example
 * void on*can*message(const hf*can*message*t &msg) {
 *     printf("Async received ID: 0x%03X\n", msg.id);
 * }
 * 
 * can.SetReceiveCallback(on*can*message);
 */
virtual hf*can*err*t SetReceiveCallback(hf*can*receive*callback*t callback) noexcept = 0;

/**
 * @brief Clear receive callback
 * 
 * 🔄 Removes the receive callback function.
 */
virtual void ClearReceiveCallback() noexcept;

/**
 * @brief Set CAN-FD receive callback
 * @param callback CAN-FD callback function
 * @return hf*can*err*t error code
 * 
 * 📞 Sets callback for CAN-FD messages with enhanced information.
 * Only available if CAN-FD is supported by the hardware.
 * ESP32-C6 TWAI controller does not support CAN-FD.
 */
virtual hf*can*err*t SetReceiveCallbackFD(hf*can*fd*receive*callback*t callback) noexcept;
```text

### 🎛️ **Configuration and Status**

```cpp
/**
 * @brief Check if CAN-FD is supported
 * @return true if supported, false otherwise
 * 
 * ✅ Checks if the hardware supports CAN-FD protocol.
 * ESP32-C6 TWAI controller returns false (no CAN-FD support).
 */
virtual bool SupportsCanFD() const noexcept;

/**
 * @brief Enable/disable CAN-FD mode
 * @param enable Enable CAN-FD mode
 * @param data*baudrate Data phase baudrate (for CAN-FD)
 * @return true if successful, false otherwise
 * 
 * 🚀 Configures CAN-FD mode if supported.
 * ESP32-C6 TWAI controller does not support CAN-FD - returns false.
 * Requires re-initialization to take effect.
 */
virtual bool SetCanFDMode(bool enable, uint32*t data*baudrate = 2000000,
                         uint32*t timeout*ms = 1000) noexcept;

/**
 * @brief Get CAN bus status
 * @param status Reference to store status information
 * @return hf*can*err*t error code
 * 
 * 📊 Retrieves comprehensive CAN bus status information.
 * 
 * @example
 * hf*can*status*t status;
 * if (can.GetStatus(status) == hf*can*err*t::CAN*SUCCESS) {
 *     printf("TX errors: %u, RX errors: %u\n", 
 *            status.tx*error*count, status.rx*error*count);
 *     printf("Bus off: %s\n", status.bus*off ? "Yes" : "No");
 * }
 */
virtual hf*can*err*t GetStatus(hf*can*status*t &status) const noexcept = 0;
```text

### 📈 **Statistics and Diagnostics**

```cpp
/**
 * @brief Reset CAN operation statistics
 * @return hf*can*err*t error code
 * 
 * 🔄 Clears all accumulated statistics counters.
 */
virtual hf*can*err*t ResetStatistics() noexcept;

/**
 * @brief Reset CAN diagnostic information
 * @return hf*can*err*t error code
 * 
 * 🔄 Clears diagnostic information and error counters.
 */
virtual hf*can*err*t ResetDiagnostics() noexcept;

/**
 * @brief Get CAN operation statistics
 * @param statistics Reference to store statistics data
 * @return hf*can*err*t error code
 * 
 * 📊 Retrieves comprehensive statistics about CAN operations.
 */
virtual hf*can*err*t GetStatistics(hf*can*statistics*t &statistics) const noexcept;

/**
 * @brief Get CAN diagnostic information
 * @param diagnostics Reference to store diagnostics data
 * @return hf*can*err*t error code
 * 
 * 🔍 Retrieves diagnostic information about CAN health and status.
 */
virtual hf*can*err*t GetDiagnostics(hf*can*diagnostics*t &diagnostics) const noexcept;
```text

---

## 📊 **Data Structures**

### 📨 **CAN Message Structure**

```cpp
struct hf*can*message*t {
    // === Core CAN Message Fields ===
    uint32*t id;     ///< Message ID (11 or 29-bit)
    uint8*t dlc;     ///< Data length code (0-8 for classic CAN)
    uint8*t data[8]; ///< Message data (max 8 bytes for classic CAN)

    // === Standard CAN Flags ===
    bool is*extended;  ///< Extended ID flag (29-bit vs 11-bit)
    bool is*rtr;       ///< Remote transmission request flag
    bool is*ss;        ///< Single shot flag (no retransmission)
    bool is*self;      ///< Self reception request flag
    bool dlc*non*comp; ///< DLC is non-compliant (> 8 for classic CAN)

    // === Metadata and Diagnostics ===
    uint64*t timestamp*us;    ///< Precise timestamp in microseconds
    uint32*t sequence*number; ///< Message sequence number
    uint8*t controller*id;    ///< Originating controller ID
    uint8*t retry*count;      ///< Number of transmission retries
    uint8*t error*count;      ///< Associated error count

    // === CAN-FD Extended Fields ===
    bool is*canfd;       ///< CAN-FD frame flag
    bool is*brs;         ///< Bit Rate Switching flag (CAN-FD)
    bool is*esi;         ///< Error State Indicator flag (CAN-FD)
    uint8*t canfd*dlc;   ///< CAN-FD DLC (can be > 8)

    // === Helper Methods ===
    uint8*t GetMaxDataLength() const noexcept;  ///< Get max data length for frame type
    bool IsValidDLC(uint8*t dlc) const noexcept;  ///< Validate DLC for frame type
    uint8*t GetEffectiveDLC() const noexcept;  ///< Get effective DLC value
    bool SetDLC(uint8*t dlc) noexcept;  ///< Set DLC for current frame type
    void SetStandardFrame() noexcept;  ///< Set standard frame format
    void SetExtendedFrame() noexcept;  ///< Set extended frame format
    void SetDataFrame() noexcept;  ///< Set data frame (not remote)
    void SetRemoteFrame() noexcept;  ///< Set remote frame
    void SetSingleShot() noexcept;  ///< Set single shot transmission
    void SetSelfReception() noexcept;  ///< Enable self reception
    bool IsValidId() const noexcept;  ///< Validate message ID
};
```text

### ⚙️ **CAN Configuration Structure**

```cpp
struct hf*can*config*t {
    hf*pin*num*t tx*pin;     ///< CAN TX pin
    hf*pin*num*t rx*pin;     ///< CAN RX pin
    hf*baud*rate*t baudrate; ///< CAN baudrate (bps)
    bool loopback*mode;      ///< Enable loopback mode for testing
    bool silent*mode;        ///< Enable silent mode (listen-only)
    uint16*t tx*queue*size;  ///< TX queue size (implementation-dependent)
    uint16*t rx*queue*size;  ///< RX queue size (implementation-dependent)
};
```text

### 📊 **CAN Status Structure**

```cpp
struct hf*can*status*t {
    uint32*t tx*error*count;  ///< Transmit error counter
    uint32*t rx*error*count;  ///< Receive error counter
    uint32*t tx*failed*count; ///< Failed transmission count
    uint32*t rx*missed*count; ///< Missed reception count
    bool bus*off;             ///< Bus-off state
    bool error*warning;       ///< Error warning state
    bool error*passive;       ///< Error passive state

    // CAN-FD specific status
    bool canfd*enabled;        ///< CAN-FD mode is active
    bool canfd*brs*enabled;    ///< Bit Rate Switching is enabled
    uint32*t nominal*baudrate; ///< Nominal bit rate (arbitration phase)
    uint32*t data*baudrate;    ///< Data bit rate (data phase for CAN-FD)
    uint32*t canfd*tx*count;   ///< Number of CAN-FD frames transmitted
    uint32*t canfd*rx*count;   ///< Number of CAN-FD frames received
    uint32*t brs*tx*count;     ///< Number of BRS frames transmitted
    uint32*t brs*rx*count;     ///< Number of BRS frames received
    uint32*t form*errors;      ///< CAN-FD form errors
    uint32*t stuff*errors;     ///< Stuff errors
    uint32*t crc*errors;       ///< CRC errors
    uint32*t bit*errors;       ///< Bit errors
    uint32*t ack*errors;       ///< Acknowledgment errors
};
```text

### 📈 **CAN Statistics Structure**

```cpp
struct hf*can*statistics*t {
    // Message counters
    uint64*t messages*sent;          ///< Total messages successfully sent
    uint64*t messages*received;      ///< Total messages successfully received
    uint64*t bytes*transmitted;      ///< Total bytes transmitted
    uint64*t bytes*received;         ///< Total bytes received
    
    // Error counters
    uint32*t send*failures;          ///< Failed send operations
    uint32*t receive*failures;       ///< Failed receive operations
    uint32*t bus*error*count;        ///< Total bus errors
    uint32*t arbitration*lost*count; ///< Arbitration lost events
    uint32*t tx*failed*count;        ///< Transmission failures
    uint32*t bus*off*events;         ///< Bus-off occurrences
    uint32*t error*warning*events;   ///< Error warning events
    
    // Performance metrics
    uint64*t uptime*seconds;         ///< Total uptime in seconds
    uint32*t last*activity*timestamp;///< Last activity timestamp
    hf*can*err*t last*error;         ///< Last error encountered
    
    // Queue statistics
    uint32*t tx*queue*peak;          ///< Peak TX queue usage
    uint32*t rx*queue*peak;          ///< Peak RX queue usage
    uint32*t tx*queue*overflows;     ///< TX queue overflow count
    uint32*t rx*queue*overflows;     ///< RX queue overflow count
};
```text

### 🔍 **CAN Diagnostics Structure**

```cpp
struct hf*can*diagnostics*t {
    uint32*t tx*error*count;         ///< Transmit error counter
    uint32*t rx*error*count;         ///< Receive error counter
    uint32*t tx*queue*peak;          ///< Peak TX queue usage
    uint32*t rx*queue*peak;          ///< Peak RX queue usage
    uint32*t last*error*timestamp;   ///< Timestamp of last error
    uint32*t controller*resets;      ///< Number of controller resets
    uint32*t bus*load*percentage;    ///< Current bus load percentage
    float bit*error*rate;            ///< Bit error rate (errors/bits)
};
```text

---

## 📊 **Usage Examples**

### 📨 **Basic Message Transmission**

```cpp
#include "mcu/esp32/EspCan.h"

// Create CAN instance
hf*can*config*t config = {
    .tx*pin = 5,
    .rx*pin = 4,
    .baudrate = 500000,
    .loopback*mode = false,
    .silent*mode = false,
    .tx*queue*size = 10,
    .rx*queue*size = 10
};

EspCan can(config);

void setup() {
    // Initialize CAN
    if (can.Initialize() == hf*can*err*t::CAN*SUCCESS) {
        printf("✅ CAN initialized successfully\n");
    }
}

void send*status*message() {
    hf*can*message*t msg;
    msg.id = 0x100;  // Status message ID
    msg.dlc = 8;     // 8 bytes of data
    msg.is*extended = false;
    msg.is*rtr = false;
    
    // Pack status data
    msg.data[0] = 0x01;  // Status byte
    msg.data[1] = 0x02;  // Temperature
    msg.data[2] = 0x03;  // Voltage
    msg.data[3] = 0x04;  // Current
    msg.data[4] = 0x05;  // Speed
    msg.data[5] = 0x06;  // Position
    msg.data[6] = 0x07;  // Error flags
    msg.data[7] = 0x08;  // Checksum
    
    hf*can*err*t result = can.SendMessage(msg, 1000);
    if (result != hf*can*err*t::CAN*SUCCESS) {
        printf("❌ Send failed: %s\n", HfCanErrToString(result));
    } else {
        printf("✅ Message sent successfully\n");
    }
}
```text

### 📥 **Message Reception**

```cpp
#include "mcu/esp32/EspCan.h"

EspCan can(config);

void receive*messages() {
    hf*can*message*t msg;
    
    while (true) {
        hf*can*err*t result = can.ReceiveMessage(msg, 100);
        
        if (result == hf*can*err*t::CAN*SUCCESS) {
            printf("📥 Received ID: 0x%03X, DLC: %u, Data: ", msg.id, msg.dlc);
            for (int i = 0; i < msg.dlc; i++) {
                printf("%02X ", msg.data[i]);
            }
            printf("\n");
            
            // Process message based on ID
            switch (msg.id) {
                case 0x100:
                    process*status*message(msg);
                    break;
                case 0x200:
                    process*command*message(msg);
                    break;
                default:
                    printf("⚠️ Unknown message ID: 0x%03X\n", msg.id);
                    break;
            }
        } else if (result == hf*can*err*t::CAN*ERR*QUEUE*EMPTY) {
            // No message available, continue
            continue;
        } else {
            printf("❌ Receive error: %s\n", HfCanErrToString(result));
        }
    }
}

void process*status*message(const hf*can*message*t &msg) {
    if (msg.dlc >= 8) {
        uint8*t status = msg.data[0];
        uint8*t temperature = msg.data[1];
        uint8*t voltage = msg.data[2];
        uint8*t current = msg.data[3];
        
        printf("📊 Status - Temp: %u°C, V: %uV, I: %uA\n", 
               temperature, voltage, current);
    }
}
```text

### 🔍 **Message Filtering**

```cpp
#include "mcu/esp32/EspCan.h"

EspCan can(config);

void setup*filtering() {
    // Initialize CAN
    can.Initialize();
    
    // Accept only status messages (0x100-0x1FF)
    can.SetAcceptanceFilter(0x100, 0x700, false);
    
    // Accept only command messages (0x200-0x2FF)
    can.SetAcceptanceFilter(0x200, 0x700, false);
    
    // Accept only diagnostic messages (0x7DF-0x7FF)
    can.SetAcceptanceFilter(0x7DF, 0x7E0, false);
    
    printf("✅ Message filtering configured\n");
}

void receive*filtered*messages() {
    hf*can*message*t msg;
    
    while (true) {
        if (can.ReceiveMessage(msg, 100) == hf*can*err*t::CAN*SUCCESS) {
            // Only filtered messages will be received
            printf("📥 Filtered message ID: 0x%03X\n", msg.id);
        }
    }
}
```text

### 📞 **Asynchronous Reception with Callbacks**

```cpp
#include "mcu/esp32/EspCan.h"

EspCan can(config);

// Callback function for received messages
void on*can*message(const hf*can*message*t &msg) {
    printf("📞 Async received ID: 0x%03X\n", msg.id);
    
    // Process message in callback context
    switch (msg.id) {
        case 0x100:
            // Handle status message
            break;
        case 0x200:
            // Handle command message
            break;
    }
}

void setup*async*reception() {
    // Initialize CAN
    can.Initialize();
    
    // Set receive callback
    can.SetReceiveCallback(on*can*message);
    
    printf("✅ Asynchronous reception enabled\n");
}

void main*loop() {
    while (true) {
        // Main application logic
        // Messages will be handled automatically by callback
        
        vTaskDelay(pdMS*TO*TICKS(100));
    }
}
```text

### 🚌 **Motor Control System**

```cpp
#include "mcu/esp32/EspCan.h"

class MotorController {
private:
    EspCan can*;
    uint32*t motor*id*;
    
public:
    MotorController(const hf*can*config*t &config, uint32*t motor*id) 
        : can*(config), motor*id*(motor*id) {}
    
    bool initialize() {
        return can*.Initialize() == hf*can*err*t::CAN*SUCCESS;
    }
    
    void set*speed(float speed*rpm) {
        hf*can*message*t msg;
        msg.id = 0x200 + motor*id*;  // Command message for this motor
        msg.dlc = 4;
        msg.is*extended = false;
        msg.is*rtr = false;
        
        // Pack speed command
        uint16*t speed*raw = static*cast<uint16*t>(speed*rpm);
        msg.data[0] = 0x01;  // Command type: set speed
        msg.data[1] = speed*raw & 0xFF;
        msg.data[2] = (speed*raw >> 8) & 0xFF;
        msg.data[3] = calculate*checksum(msg.data, 3);
        
        hf*can*err*t result = can*.SendMessage(msg, 1000);
        if (result != hf*can*err*t::CAN*SUCCESS) {
            printf("❌ Speed command failed: %s\n", HfCanErrToString(result));
        }
    }
    
    void request*status() {
        hf*can*message*t msg;
        msg.id = 0x100 + motor*id*;  // Status request for this motor
        msg.dlc = 0;
        msg.is*extended = false;
        msg.is*rtr = true;  // Remote frame
        
        can*.SendMessage(msg, 1000);
    }
    
    void monitor*status() {
        hf*can*status*t status;
        if (can*.GetStatus(status) == hf*can*err*t::CAN*SUCCESS) {
            printf("📊 CAN Status - TX errors: %u, RX errors: %u, Bus off: %s\n",
                   status.tx*error*count, status.rx*error*count,
                   status.bus*off ? "Yes" : "No");
        }
    }
    
private:
    uint8*t calculate*checksum(const uint8*t *data, uint8*t length) {
        uint8*t checksum = 0;
        for (uint8*t i = 0; i < length; i++) {
            checksum ^= data[i];
        }
        return checksum;
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (can.Initialize() != hf*can*err*t::CAN*SUCCESS) {
    printf("❌ CAN initialization failed\n");
    return false;
}

// ✅ Use appropriate timeouts
can.SendMessage(msg, 1000);  // 1 second timeout for critical messages
can.ReceiveMessage(msg, 100);  // 100ms timeout for non-blocking receive

// ✅ Handle all error codes
hf*can*err*t result = can.SendMessage(msg, timeout);
if (result != hf*can*err*t::CAN*SUCCESS) {
    printf("⚠️ Send error: %s\n", HfCanErrToString(result));
    // Handle specific error types
    if (result == hf*can*err*t::CAN*ERR*BUS*OFF) {
        // Bus off - restart controller
        can.Deinitialize();
        can.Initialize();
    }
}

// ✅ Use message filtering for efficiency
can.SetAcceptanceFilter(0x100, 0x700, false);  // Only accept status messages

// ✅ Monitor bus health
hf*can*status*t status;
if (can.GetStatus(status) == hf*can*err*t::CAN*SUCCESS) {
    if (status.bus*off) {
        printf("🚨 Bus off detected!\n");
    }
    if (status.tx*error*count > 100) {
        printf("⚠️ High TX error count: %u\n", status.tx*error*count);
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore initialization
can.SendMessage(msg);  // May fail silently

// ❌ Don't use infinite timeouts in real-time systems
can.ReceiveMessage(msg, UINT32*MAX);  // May block forever

// ❌ Don't ignore error codes
can.SendMessage(msg);  // Error handling missing

// ❌ Don't assume message reception
// Always check return values for receive operations

// ❌ Don't use without proper bus termination
// CAN bus requires proper termination resistors

// ❌ Don't ignore bus-off state
// Bus-off requires controller restart
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use batch operations for multiple messages
hf*can*message*t messages[10];
// ... populate messages
uint32*t sent = can.SendMessageBatch(messages, 10, 1000);

// 🚀 Use callbacks for high-frequency reception
can.SetReceiveCallback(on*message);  // Non-blocking reception

// 🚀 Use appropriate queue sizes
hf*can*config*t config = {
    .tx*queue*size = 20,  // Larger for high-frequency transmission
    .rx*queue*size = 50   // Larger for high-frequency reception
};

// 🚀 Use message filtering to reduce CPU load
can.SetAcceptanceFilter(target*id, mask, extended);

// 🚀 Monitor statistics for performance tuning
hf*can*statistics*t stats;
can.GetStatistics(stats);
if (stats.tx*queue*overflows > 0) {
    printf("⚠️ TX queue overflow - increase queue size\n");
}
```text

---

## 🔗 **Related Documentation**

- [⚙️ **EspCan**](../esp_api/EspCan.md) - ESP32-C6 implementation
- [🎯 **Hardware Types**](HardwareTypes.md) - Platform-agnostic types

---

<div align="center">

**📋 Navigation**

[← Previous: BaseUart](BaseUart.md) | [Back to API Index](README.md) | [Next: BaseWifi
→](BaseWifi.md)

</div>

---

<div align="center">

**🚌 BaseCan - The Foundation of CAN Communication in HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 