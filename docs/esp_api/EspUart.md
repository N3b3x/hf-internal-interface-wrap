# EspUart API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspSpi](EspSpi.md) | [Back to ESP API Index](README.md) | [Next: EspCan →](EspCan.md)

</div>

---

## Overview

`EspUart` is the ESP32-C6 implementation of the `BaseUart` interface,
providing comprehensive UART (Universal Asynchronous Receiver-Transmitter) functionality
specifically optimized for ESP32-C6 microcontrollers running ESP-IDF v5.5+.
It offers both basic and advanced UART features with hardware-specific optimizations.

## Features

- **ESP32-C6 UART Controller** - Full support for ESP32-C6 UART capabilities
- **Multiple Ports** - Support for multiple UART ports
- **Hardware Flow Control** - RTS/CTS hardware flow control
- **DMA Integration** - High-performance DMA transfers
- **Interrupt Support** - Configurable interrupt handling
- **Baud Rate Control** - Wide range of baud rates
- **Power Management** - Deep sleep compatibility
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspUart.h"
```text

## Class Definition

```cpp
class EspUart : public BaseUart {
public:
    // Constructor with full configuration
    explicit EspUart(
        hf*uart*port*t port = hf*uart*port*t::HF*UART*PORT*1,
        hf*pin*num*t tx*pin = GPIO*NUM*1,
        hf*pin*num*t rx*pin = GPIO*NUM*3,
        hf*uart*baud*t baud*rate = 115200,
        hf*uart*data*bits*t data*bits = hf*uart*data*bits*t::HF*UART*DATA*BITS*8,
        hf*uart*parity*t parity = hf*uart*parity*t::HF*UART*PARITY*NONE,
        hf*uart*stop*bits*t stop*bits = hf*uart*stop*bits*t::HF*UART*STOP*BITS*1
    ) noexcept;

    // Destructor
    ~EspUart() override;

    // BaseUart implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // UART operations
    hf*uart*err*t WriteBytes(const hf*u8*t* data, hf*size*t length) noexcept override;
    hf*uart*err*t ReadBytes(hf*u8*t* data, hf*size*t length, hf*u32*t timeout*ms = 0) noexcept override;
    hf*uart*err*t WriteString(const char* str) noexcept override;
    hf*uart*err*t ReadString(char* str, hf*size*t max*length, hf*u32*t timeout*ms = 0) noexcept override;
    hf*uart*err*t GetBytesAvailable(hf*size*t* count) const noexcept override;
    hf*uart*err*t Flush() noexcept override;

    // Advanced features
    hf*uart*err*t SetBaudRate(hf*uart*baud*t baud*rate) noexcept override;
    hf*uart*err*t GetBaudRate(hf*uart*baud*t* baud*rate) const noexcept override;
    hf*uart*err*t SetFlowControl(hf*uart*flow*control*t flow*control) noexcept override;
    hf*uart*err*t GetFlowControl(hf*uart*flow*control*t* flow*control) const noexcept override;
    hf*uart*err*t SetInterruptCallback(hf*uart*interrupt*callback*t callback, void* user*data) noexcept override;
    hf*uart*err*t ClearInterruptCallback() noexcept override;
};
```text

## Usage Examples

### Basic UART Communication

```cpp
#include "inc/mcu/esp32/EspUart.h"

// Create UART instance
EspUart uart(HF*UART*PORT*1, GPIO*NUM*1, GPIO*NUM*3, 115200);

// Initialize
if (!uart.Initialize()) {
    printf("Failed to initialize UART\n");
    return;
}

// Write data
const char* message = "Hello, UART!\n";
hf*uart*err*t err = uart.WriteString(message);
if (err != HF*UART*ERR*OK) {
    printf("Failed to write string: %d\n", err);
    return;
}

// Read data
char buffer[256];
err = uart.ReadString(buffer, sizeof(buffer), 1000); // 1 second timeout
if (err == HF*UART*ERR*OK) {
    printf("Received: %s\n", buffer);
} else if (err == HF*UART*ERR*TIMEOUT) {
    printf("Read timeout\n");
}
```text

### Binary Data Transfer

```cpp
// Write binary data
hf*u8*t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
hf*uart*err*t err = uart.WriteBytes(data, sizeof(data));
if (err != HF*UART*ERR*OK) {
    printf("Failed to write bytes: %d\n", err);
    return;
}

// Read binary data
hf*u8*t read*buffer[10];
hf*size*t bytes*read;
err = uart.ReadBytes(read*buffer, sizeof(read*buffer), 1000);
if (err == HF*UART*ERR*OK) {
    printf("Read %zu bytes: ", bytes*read);
    for (hf*size*t i = 0; i < bytes*read; i++) {
        printf("%02X ", read*buffer[i]);
    }
    printf("\n");
}
```text

### Flow Control

```cpp
// Enable hardware flow control
hf*uart*err*t err = uart.SetFlowControl(HF*UART*FLOW*CONTROL*HARDWARE);
if (err != HF*UART*ERR*OK) {
    printf("Failed to set flow control: %d\n", err);
    return;
}

// Check available bytes before reading
hf*size*t available;
err = uart.GetBytesAvailable(&available);
if (err == HF*UART*ERR*OK) {
    printf("Bytes available: %zu\n", available);
    
    if (available > 0) {
        char buffer[256];
        err = uart.ReadString(buffer, sizeof(buffer), 0); // Non-blocking
        if (err == HF*UART*ERR*OK) {
            printf("Received: %s\n", buffer);
        }
    }
}
```text

### Interrupt-based Communication

```cpp
// Interrupt callback function
void uart*interrupt*callback(hf*uart*event*t event, void* user*data) {
    switch (event) {
        case HF*UART*EVENT*RX*DATA:
            printf("UART RX data available\n");
            break;
        case HF*UART*EVENT*TX*DONE:
            printf("UART TX completed\n");
            break;
        case HF*UART*EVENT*ERROR:
            printf("UART error occurred\n");
            break;
        default:
            break;
    }
}

// Set interrupt callback
hf*uart*err*t err = uart.SetInterruptCallback(uart*interrupt*callback, nullptr);
if (err != HF*UART*ERR*OK) {
    printf("Failed to set interrupt callback: %d\n", err);
    return;
}

// Enable interrupt-based communication
// The callback will be called when data is available or transmission is complete
```text

### Multiple UART Ports

```cpp
// Create multiple UART instances
EspUart uart1(HF*UART*PORT*1, GPIO*NUM*1, GPIO*NUM*3, 115200);
EspUart uart2(HF*UART*PORT*2, GPIO*NUM*17, GPIO*NUM*16, 9600);

// Initialize both
if (!uart1.Initialize() || !uart2.Initialize()) {
    printf("Failed to initialize UART ports\n");
    return;
}

// Use different baud rates
uart1.SetBaudRate(115200);
uart2.SetBaudRate(9600);

// Send data on both ports
uart1.WriteString("Port 1 message\n");
uart2.WriteString("Port 2 message\n");
```text

## ESP32-C6 Specific Features

### Multiple UART Ports

Support for multiple UART ports with independent configuration.

### Hardware Flow Control

RTS/CTS hardware flow control for reliable data transmission.

### DMA Integration

High-performance DMA transfers for large data blocks.

### Interrupt Support

Configurable interrupt handling for efficient data processing.

## Error Handling

The `EspUart` class provides comprehensive error handling with specific error codes:

- `HF*UART*ERR*OK` - Operation successful
- `HF*UART*ERR*INVALID*ARG` - Invalid parameter
- `HF*UART*ERR*NOT*INITIALIZED` - UART not initialized
- `HF*UART*ERR*TIMEOUT` - Operation timeout
- `HF*UART*ERR*BUFFER*FULL` - Buffer full
- `HF*UART*ERR*BUFFER*EMPTY` - Buffer empty
- `HF*UART*ERR*PARITY*ERROR` - Parity error
- `HF*UART*ERR*FRAMING*ERROR` - Framing error

## Performance Considerations

- **Baud Rate**: Choose appropriate baud rate for your application
- **Buffer Size**: Use appropriate buffer sizes for your data
- **Flow Control**: Enable flow control for reliable transmission
- **Interrupts**: Use interrupts for efficient data handling

## Related Documentation

- [BaseUart API Reference](../api/BaseUart.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF UART Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/uart.html) - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspSpi](EspSpi.md) | [Back to ESP API Index](README.md) | [Next: EspCan →](EspCan.md)

</div>