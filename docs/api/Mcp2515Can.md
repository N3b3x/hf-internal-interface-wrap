# Mcp2515Can API Reference

## Overview

`Mcp2515Can` is an MCP2515-based implementation of the `BaseCan` interface, providing CAN (Controller Area Network) functionality through the MCP2515 CAN controller IC. This implementation is useful when the microcontroller doesn't have built-in CAN support.

## Features

- **MCP2515 Controller** - Uses MCP2515 CAN controller IC
- **SPI Interface** - Communicates with MCP2515 via SPI
- **CAN 2.0A/2.0B Support** - Standard and extended frame formats
- **High-Speed Operation** - Up to 1 Mbps CAN bus speeds
- **Filter Support** - Hardware message filtering
- **Error Detection** - Comprehensive error detection and reporting

## Header File

```cpp
#include "inc/mcu/mcp2515/Mcp2515Can.h"
```

## Class Definition

```cpp
class Mcp2515Can : public BaseCan {
public:
    // Constructor with full configuration
    explicit Mcp2515Can(
        hf_can_port_t port = hf_can_port_t::HF_CAN_PORT_0,
        hf_spi_port_t spi_port = hf_spi_port_t::HF_SPI_PORT_0,
        hf_pin_num_t cs_pin = GPIO_NUM_5,
        hf_pin_num_t int_pin = GPIO_NUM_4,
        hf_can_speed_t speed = hf_can_speed_t::HF_CAN_SPEED_500K,
        hf_can_mode_t mode = hf_can_mode_t::HF_CAN_MODE_NORMAL
    ) noexcept;

    // Destructor
    ~Mcp2515Can() override;

    // BaseCan implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // CAN operations
    hf_can_err_t SendMessage(const hf_can_message_t& message) noexcept override;
    hf_can_err_t ReceiveMessage(hf_can_message_t& message, hf_u32_t timeout_ms = 0) noexcept override;
    hf_can_err_t GetMessageCount(hf_size_t* count) const noexcept override;

    // Advanced features
    hf_can_err_t SetSpeed(hf_can_speed_t speed) noexcept override;
    hf_can_err_t GetSpeed(hf_can_speed_t* speed) const noexcept override;
    hf_can_err_t SetFilter(const hf_can_filter_t& filter) noexcept override;
    hf_can_err_t ClearFilters() noexcept override;
    hf_can_err_t GetErrorCounters(hf_can_error_counters_t& counters) noexcept override;
};
```

## Usage Examples

### Basic MCP2515 CAN Usage

```cpp
#include "inc/mcu/mcp2515/Mcp2515Can.h"

// Create MCP2515 CAN instance
Mcp2515Can can(HF_CAN_PORT_0, HF_SPI_PORT_0, GPIO_NUM_5, GPIO_NUM_4, HF_CAN_SPEED_500K);

// Initialize
if (!can.Initialize()) {
    printf("Failed to initialize MCP2515 CAN\n");
    return;
}

// Send a message
hf_can_message_t message;
message.id = 0x123;
message.flags = HF_CAN_FLAG_STANDARD;
message.data_length = 8;
message.data[0] = 0x01;
message.data[1] = 0x02;
// ... fill remaining data

hf_can_err_t err = can.SendMessage(message);
if (err != HF_CAN_ERR_OK) {
    printf("MCP2515 CAN send failed: %d\n", err);
}

// Receive a message
hf_can_message_t received_message;
err = can.ReceiveMessage(received_message, 1000); // 1 second timeout
if (err == HF_CAN_ERR_OK) {
    printf("Received message ID: 0x%X, Data: ", received_message.id);
    for (int i = 0; i < received_message.data_length; i++) {
        printf("%02X ", received_message.data[i]);
    }
    printf("\n");
}
```

## Related Documentation

- [BaseCan API Reference](BaseCan.md) - Base class interface
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions