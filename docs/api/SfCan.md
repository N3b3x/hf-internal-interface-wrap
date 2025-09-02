# SfCan API Reference

## Overview

`SfCan` is a software-based implementation of the `BaseCan` interface, providing CAN (Controller Area Network) functionality through software emulation. This implementation is useful for testing, simulation, or when hardware CAN controllers are not available.

## Features

- **Software Emulation** - Pure software implementation of CAN protocol
- **Testing Support** - Ideal for unit testing and simulation
- **Cross-Platform** - Works on any platform with basic I/O capabilities
- **Configurable** - Flexible configuration for different use cases
- **Debug Support** - Enhanced debugging and logging capabilities

## Header File

```cpp
#include "inc/mcu/software/SfCan.h"
```

## Class Definition

```cpp
class SfCan : public BaseCan {
public:
    // Constructor with full configuration
    explicit SfCan(
        hf_can_port_t port = hf_can_port_t::HF_CAN_PORT_0,
        hf_can_speed_t speed = hf_can_speed_t::HF_CAN_SPEED_500K,
        hf_can_mode_t mode = hf_can_mode_t::HF_CAN_MODE_NORMAL
    ) noexcept;

    // Destructor
    ~SfCan() override;

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

### Basic CAN Communication

```cpp
#include "inc/mcu/software/SfCan.h"

// Create software CAN instance
SfCan can(HF_CAN_PORT_0, HF_CAN_SPEED_500K);

// Initialize
if (!can.Initialize()) {
    printf("Failed to initialize software CAN\n");
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
    printf("Software CAN send failed: %d\n", err);
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