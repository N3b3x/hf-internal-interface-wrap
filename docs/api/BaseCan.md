# 🚌 BaseCan API Documentation

## 📋 Overview

The `BaseCan` class is an abstract base class that provides a unified, platform-agnostic interface for CAN (Controller Area Network) bus communication in the HardFOC system. This class follows the same architectural pattern as other Base* classes, providing consistent APIs across different CAN controller implementations.

## 🏗️ Class Hierarchy

```
BaseCan (Abstract Base Class)
    ├── McuCan (ESP32 TWAI implementation)
    ├── ExtCanController (External CAN via SPI)
    └── SfCan (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **Platform Agnostic**: Works with different CAN controllers
- ✅ **Classic CAN & CAN-FD Ready**: Supports both standard and extended CAN protocols
- ✅ **Interrupt-Driven**: Callback-based message reception
- ✅ **Comprehensive Error Handling**: 27+ specific error codes
- ✅ **Lazy Initialization**: Initialize on first use
- ✅ **Thread-Safe Design**: Safe for concurrent access (implementation dependent)
- ✅ **Status Monitoring**: Detailed bus status with CAN-FD metrics

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `CAN_SUCCESS` | 0 | Operation successful |
| `CAN_ERR_NOT_INITIALIZED` | 2 | CAN bus not initialized |
| `CAN_ERR_INVALID_PARAMETER` | 4 | Invalid parameter provided |
| `CAN_ERR_BUS_OFF` | 7 | CAN bus in off state |
| `CAN_ERR_MESSAGE_TIMEOUT` | 11 | Message transmission timeout |
| `CAN_ERR_QUEUE_FULL` | 15 | TX/RX queue is full |
| `CAN_ERR_HARDWARE_FAULT` | 17 | Hardware malfunction |
| `CAN_ERR_INVALID_BAUD_RATE` | 23 | Unsupported baud rate |

*See header file for complete list of 27 error codes*

## 🏗️ Data Structures

### CanBusConfig
Configuration structure for CAN bus initialization:

```cpp
struct CanBusConfig {
    hf_gpio_num_t tx_pin;               // CAN TX pin
    hf_gpio_num_t rx_pin;               // CAN RX pin
    uint32_t baudrate;                  // CAN baudrate (bps)
    bool loopback_mode;                 // Enable loopback mode
    bool silent_mode;                   // Enable silent mode (listen-only)
    uint16_t tx_queue_size;             // TX queue size
    uint16_t rx_queue_size;             // RX queue size
};
```

### CanMessage
CAN message structure supporting both classic CAN and CAN-FD:

```cpp
struct CanMessage {
    uint32_t id;                        // CAN identifier (11 or 29-bit)
    bool extended_id;                   // Extended (29-bit) ID flag
    bool remote_frame;                  // Remote transmission request
    uint8_t dlc;                        // Data length code (0-8 for classic)
    uint8_t data[8];                    // Message data
    uint32_t timestamp;                 // Reception timestamp
};
```

### CanBusStatus
Comprehensive bus status including CAN-FD metrics:

```cpp
struct CanBusStatus {
    uint32_t tx_error_count;            // Transmit error counter
    uint32_t rx_error_count;            // Receive error counter
    bool bus_off;                       // Bus-off state
    bool error_warning;                 // Error warning state
    
    // CAN-FD specific
    bool canfd_enabled;                 // CAN-FD mode active
    bool canfd_brs_enabled;             // Bit Rate Switching enabled
    uint32_t nominal_baudrate;          // Arbitration phase rate
    uint32_t data_baudrate;             // Data phase rate
    uint32_t canfd_tx_count;            // CAN-FD frames transmitted
    uint32_t canfd_rx_count;            // CAN-FD frames received
};
```

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes CAN bus on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Yes  

#### `IsInitialized()`
```cpp
bool IsInitialized() const noexcept
```
**Description**: Check if CAN bus is initialized  
**Returns**: `true` if initialized, `false` otherwise  
**Thread-Safe**: Yes  

### Message Transmission

#### `SendMessage()`
```cpp
bool SendMessage(const CanMessage& message, uint32_t timeout_ms = 1000) noexcept
```
**Description**: Send a CAN message  
**Parameters**:
- `message`: CAN message to transmit
- `timeout_ms`: Timeout in milliseconds (0 = no wait)

**Returns**: `true` if sent successfully, `false` on failure  
**Thread-Safe**: Implementation dependent  

### Message Reception

#### `ReceiveMessage()`
```cpp
bool ReceiveMessage(CanMessage& message, uint32_t timeout_ms = 0) noexcept
```
**Description**: Receive a CAN message (polling mode)  
**Parameters**:
- `message`: Buffer to store received message
- `timeout_ms`: Timeout in milliseconds (0 = no wait)

**Returns**: `true` if received successfully, `false` on timeout/error  
**Thread-Safe**: Implementation dependent  

#### `SetReceiveCallback()`
```cpp
bool SetReceiveCallback(CanReceiveCallback callback) noexcept
```
**Description**: Set callback for interrupt-driven message reception  
**Parameters**:
- `callback`: Function to call when message received

**Returns**: `true` if callback set successfully  
**Thread-Safe**: Implementation dependent  

#### `SetReceiveCallbackFD()`
```cpp
bool SetReceiveCallbackFD(CanFdReceiveCallback callback) noexcept
```
**Description**: Set enhanced CAN-FD callback with additional reception info  
**Parameters**:
- `callback`: Enhanced callback with timing and error information

**Returns**: `true` if callback set successfully  
**Thread-Safe**: Implementation dependent  

### Status and Control

#### `GetStatus()`
```cpp
bool GetStatus(CanBusStatus& status) noexcept
```
**Description**: Get comprehensive bus status including CAN-FD metrics  
**Parameters**:
- `status`: Buffer to store status information

**Returns**: `true` if status retrieved successfully  
**Thread-Safe**: Yes  

#### `Reset()`
```cpp
virtual bool Reset() noexcept = 0
```
**Description**: Reset CAN controller, clear errors  
**Returns**: `true` if reset successful  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### Basic CAN Communication
```cpp
#include "mcu/McuCan.h"

// Create CAN instance
auto can = McuCan::Create();

// Configure CAN bus
CanBusConfig config;
config.tx_pin = 21;
config.rx_pin = 22;
config.baudrate = 500000;  // 500 kbps

// Initialize
if (!can->Configure(config) || !can->EnsureInitialized()) {
    // Handle initialization error
    return;
}

// Send message
CanMessage msg;
msg.id = 0x123;
msg.extended_id = false;
msg.dlc = 8;
std::memcpy(msg.data, "HARDFOC!", 8);

if (can->SendMessage(msg)) {
    printf("Message sent successfully\n");
}

// Receive message (polling)
CanMessage received;
if (can->ReceiveMessage(received, 1000)) {
    printf("Received message ID: 0x%X\n", received.id);
}
```

### Interrupt-Driven Reception
```cpp
// Set up callback for received messages
can->SetReceiveCallback([](const CanMessage& msg) {
    printf("Received CAN ID: 0x%X, DLC: %d\n", msg.id, msg.dlc);
    
    // Process message data
    for (int i = 0; i < msg.dlc; ++i) {
        printf("Data[%d]: 0x%02X\n", i, msg.data[i]);
    }
});
```

### CAN-FD Enhanced Reception
```cpp
// Set up enhanced CAN-FD callback
can->SetReceiveCallbackFD([](const CanMessage& msg, const CanReceptionInfo& info) {
    printf("CAN-FD Message ID: 0x%X\n", msg.id);
    printf("Timestamp: %u us\n", info.timestamp_us);
    printf("FIFO Level: %d\n", info.rx_fifo_level);
    
    if (info.data_phase_error) {
        printf("Warning: Data phase error detected\n");
    }
});
```

### Status Monitoring
```cpp
CanBusStatus status;
if (can->GetStatus(status)) {
    printf("TX Errors: %u, RX Errors: %u\n", 
           status.tx_error_count, status.rx_error_count);
    
    if (status.canfd_enabled) {
        printf("CAN-FD Mode: Active\n");
        printf("Nominal Rate: %u bps\n", status.nominal_baudrate);
        printf("Data Rate: %u bps\n", status.data_baudrate);
        printf("CAN-FD Messages TX: %u, RX: %u\n", 
               status.canfd_tx_count, status.canfd_rx_count);
    }
    
    if (status.bus_off) {
        printf("Warning: Bus is in OFF state\n");
        can->Reset();  // Attempt recovery
    }
}
```

### Error Handling
```cpp
// Check for specific errors
if (!can->SendMessage(msg)) {
    CanBusStatus status;
    can->GetStatus(status);
    
    if (status.bus_off) {
        printf("Error: CAN bus is off - attempting reset\n");
        can->Reset();
    } else if (status.tx_error_count > 100) {
        printf("Warning: High TX error count: %u\n", status.tx_error_count);
    }
}
```

## 🧪 Testing

The BaseCan class can be tested using:

```cpp
#include "tests/CommBusTests.h"

// Run comprehensive CAN tests
bool success = TestCanCommunication();
```

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **Platform Specific**: Actual implementation depends on target hardware
3. **CAN-FD Support**: Not all implementations support CAN-FD features
4. **Thread Safety**: Depends on concrete implementation
5. **Error Recovery**: Always check status and implement error recovery logic
6. **Queue Management**: Monitor TX/RX queue levels to prevent overflows

## 🔗 Related Classes

- [`BaseAdc`](BaseAdc.md) - ADC interface following same pattern
- [`BaseGpio`](BaseGpio.md) - GPIO interface with similar architecture
- [`McuCan`](../mcu/McuCan.md) - ESP32 TWAI implementation
- [`SfCan`](../thread_safe/SfCan.md) - Thread-safe wrapper

## 📝 See Also

- [CAN Guide](../guides/can-guide.md)
- [Error Handling Best Practices](../guides/error-handling.md)
