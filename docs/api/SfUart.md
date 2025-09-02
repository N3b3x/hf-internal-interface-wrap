# SfUart API Reference

## Overview

`SfUart` is a software-based implementation of the `BaseUart` interface, providing UART (Universal Asynchronous Receiver-Transmitter) functionality through software emulation. This implementation is useful for testing, simulation, or when hardware UART controllers are not available.

## Features

- **Software Emulation** - Pure software implementation of UART functionality
- **Testing Support** - Ideal for unit testing and simulation
- **Cross-Platform** - Works on any platform with basic I/O capabilities
- **Configurable** - Flexible configuration for different use cases
- **Debug Support** - Enhanced debugging and logging capabilities

## Header File

```cpp
#include "inc/mcu/software/SfUart.h"
```

## Class Definition

```cpp
class SfUart : public BaseUart {
public:
    // Constructor with full configuration
    explicit SfUart(
        hf_uart_port_t port = hf_uart_port_t::HF_UART_PORT_1,
        hf_pin_num_t tx_pin = GPIO_NUM_1,
        hf_pin_num_t rx_pin = GPIO_NUM_3,
        hf_uart_baud_t baud_rate = 115200,
        hf_uart_data_bits_t data_bits = hf_uart_data_bits_t::HF_UART_DATA_BITS_8,
        hf_uart_parity_t parity = hf_uart_parity_t::HF_UART_PARITY_NONE,
        hf_uart_stop_bits_t stop_bits = hf_uart_stop_bits_t::HF_UART_STOP_BITS_1
    ) noexcept;

    // Destructor
    ~SfUart() override;

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
```

## Usage Examples

### Basic UART Usage

```cpp
#include "inc/mcu/software/SfUart.h"

// Create software UART instance
SfUart uart(HF_UART_PORT_1, GPIO_NUM_1, GPIO_NUM_3, 115200);

// Initialize
if (!uart.Initialize()) {
    printf("Failed to initialize software UART\n");
    return;
}

// Write data
const char* message = "Hello, Software UART!\n";
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
```

## Related Documentation

- [BaseUart API Reference](BaseUart.md) - Base class interface
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions