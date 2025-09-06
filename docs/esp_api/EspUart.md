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
        hf_uart_port_t port = hf_uart_port_t::HF_UART_PORT_1,
        hf_pin_num_t tx_pin = GPIO_NUM_1,
        hf_pin_num_t rx_pin = GPIO_NUM_3,
        hf_uart_baud_t baud_rate = 115200,
        hf_uart_data_bits_t data_bits = hf_uart_data_bits_t::HF_UART_DATA_BITS_8,
        hf_uart_parity_t parity = hf_uart_parity_t::HF_UART_PARITY_NONE,
        hf_uart_stop_bits_t stop_bits = hf_uart_stop_bits_t::HF_UART_STOP_BITS_1
    ) noexcept;

    // Destructor
    ~EspUart() override;

    // BaseUart implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // UART operations
    hf_uart_err_t WriteBytes(const hf_u8_t* data, hf_size_t length) noexcept override;
    hf_uart_err_t ReadBytes(hf_u8_t* data, hf_size_t length, hf_u32_t timeout_ms = 0) noexcept override;
    hf_uart_err_t WriteString(const char* str) noexcept override;
    hf_uart_err_t ReadString(char* str, hf_size_t max_length, hf_u32_t timeout_ms = 0) noexcept override;
    hf_uart_err_t GetBytesAvailable(hf_size_t* count) const noexcept override;
    hf_uart_err_t Flush() noexcept override;

    // Advanced features
    hf_uart_err_t SetBaudRate(hf_uart_baud_t baud_rate) noexcept override;
    hf_uart_err_t GetBaudRate(hf_uart_baud_t* baud_rate) const noexcept override;
    hf_uart_err_t SetFlowControl(hf_uart_flow_control_t flow_control) noexcept override;
    hf_uart_err_t GetFlowControl(hf_uart_flow_control_t* flow_control) const noexcept override;
    hf_uart_err_t SetInterruptCallback(hf_uart_interrupt_callback_t callback, void* user_data) noexcept override;
    hf_uart_err_t ClearInterruptCallback() noexcept override;
};
```text

## Usage Examples

### Basic UART Communication

```cpp
#include "inc/mcu/esp32/EspUart.h"

// Create UART instance
EspUart uart(HF_UART_PORT_1, GPIO_NUM_1, GPIO_NUM_3, 115200);

// Initialize
if (!uart.Initialize()) {
    printf("Failed to initialize UART\n");
    return;
}

// Write data
const char* message = "Hello, UART!\n";
hf_uart_err_t err = uart.WriteString(message);
if (err != HF_UART_ERR_OK) {
    printf("Failed to write string: %d\n", err);
    return;
}

// Read data
char buffer[256];
err = uart.ReadString(buffer, sizeof(buffer), 1000); // 1 second timeout
if (err == HF_UART_ERR_OK) {
    printf("Received: %s\n", buffer);
} else if (err == HF_UART_ERR_TIMEOUT) {
    printf("Read timeout\n");
}
```text

### Binary Data Transfer

```cpp
// Write binary data
hf_u8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
hf_uart_err_t err = uart.WriteBytes(data, sizeof(data));
if (err != HF_UART_ERR_OK) {
    printf("Failed to write bytes: %d\n", err);
    return;
}

// Read binary data
hf_u8_t read_buffer[10];
hf_size_t bytes_read;
err = uart.ReadBytes(read_buffer, sizeof(read_buffer), 1000);
if (err == HF_UART_ERR_OK) {
    printf("Read %zu bytes: ", bytes_read);
    for (hf_size_t i = 0; i < bytes_read; i++) {
        printf("%02X ", read_buffer[i]);
    }
    printf("\n");
}
```text

### Flow Control

```cpp
// Enable hardware flow control
hf_uart_err_t err = uart.SetFlowControl(HF_UART_FLOW_CONTROL_HARDWARE);
if (err != HF_UART_ERR_OK) {
    printf("Failed to set flow control: %d\n", err);
    return;
}

// Check available bytes before reading
hf_size_t available;
err = uart.GetBytesAvailable(&available);
if (err == HF_UART_ERR_OK) {
    printf("Bytes available: %zu\n", available);
    
    if (available > 0) {
        char buffer[256];
        err = uart.ReadString(buffer, sizeof(buffer), 0); // Non-blocking
        if (err == HF_UART_ERR_OK) {
            printf("Received: %s\n", buffer);
        }
    }
}
```text

### Interrupt-based Communication

```cpp
// Interrupt callback function
void uart_interrupt_callback(hf_uart_event_t event, void* user_data) {
    switch (event) {
        case HF_UART_EVENT_RX_DATA:
            printf("UART RX data available\n");
            break;
        case HF_UART_EVENT_TX_DONE:
            printf("UART TX completed\n");
            break;
        case HF_UART_EVENT_ERROR:
            printf("UART error occurred\n");
            break;
        default:
            break;
    }
}

// Set interrupt callback
hf_uart_err_t err = uart.SetInterruptCallback(uart_interrupt_callback, nullptr);
if (err != HF_UART_ERR_OK) {
    printf("Failed to set interrupt callback: %d\n", err);
    return;
}

// Enable interrupt-based communication
// The callback will be called when data is available or transmission is complete
```text

### Multiple UART Ports

```cpp
// Create multiple UART instances
EspUart uart1(HF_UART_PORT_1, GPIO_NUM_1, GPIO_NUM_3, 115200);
EspUart uart2(HF_UART_PORT_2, GPIO_NUM_17, GPIO_NUM_16, 9600);

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

- `HF_UART_ERR_OK` - Operation successful
- `HF_UART_ERR_INVALID_ARG` - Invalid parameter
- `HF_UART_ERR_NOT_INITIALIZED` - UART not initialized
- `HF_UART_ERR_TIMEOUT` - Operation timeout
- `HF_UART_ERR_BUFFER_FULL` - Buffer full
- `HF_UART_ERR_BUFFER_EMPTY` - Buffer empty
- `HF_UART_ERR_PARITY_ERROR` - Parity error
- `HF_UART_ERR_FRAMING_ERROR` - Framing error

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