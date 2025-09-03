# EspCan API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspUart](EspUart.md) | [Back to ESP API Index](README.md) | [Next: EspWifi →](EspWifi.md)

</div>

---

## Overview

`EspCan` is the ESP32-C6 implementation of the `BaseCan` interface, providing comprehensive CAN (Controller Area Network) functionality specifically optimized for ESP32-C6 microcontrollers running ESP-IDF v5.5+. It offers both basic and advanced CAN features with hardware-specific optimizations.

## Features

- **ESP32-C6 TWAI Controller** - Full support for ESP32-C6 TWAI (Two-Wire Automotive Interface) capabilities
- **CAN 2.0A/2.0B Support** - Standard and extended frame formats (no CAN-FD support)
- **High-Speed Operation** - Up to 1 Mbps CAN bus speeds
- **DMA Integration** - High-performance DMA transfers
- **Filter Support** - Hardware message filtering
- **Error Detection** - Comprehensive error detection and reporting
- **Power Management** - Deep sleep compatibility
- **Performance Optimized** - Direct register access for critical operations

## Header File

```cpp
#include "inc/mcu/esp32/EspCan.h"
```

## Class Definition

```cpp
class EspCan : public BaseCan {
public:
    // Constructor with full configuration
    explicit EspCan(
        hf_can_port_t port = hf_can_port_t::HF_CAN_PORT_0,
        hf_pin_num_t tx_pin = GPIO_NUM_5,
        hf_pin_num_t rx_pin = GPIO_NUM_4,
        hf_can_speed_t speed = hf_can_speed_t::HF_CAN_SPEED_500K,
        hf_can_mode_t mode = hf_can_mode_t::HF_CAN_MODE_NORMAL
    ) noexcept;

    // Destructor
    ~EspCan() override;

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
#include "inc/mcu/esp32/EspCan.h"

// Create CAN instance
EspCan can(HF_CAN_PORT_0, GPIO_NUM_5, GPIO_NUM_4, HF_CAN_SPEED_500K);

// Initialize
if (!can.Initialize()) {
    printf("Failed to initialize CAN\n");
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
    printf("CAN send failed: %d\n", err);
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

### Message Filtering

```cpp
// Set up a filter to only receive messages with specific ID
hf_can_filter_t filter;
filter.id = 0x123;
filter.mask = 0x7FF; // Standard 11-bit ID mask
filter.flags = HF_CAN_FLAG_STANDARD;

hf_can_err_t err = can.SetFilter(filter);
if (err != HF_CAN_ERR_OK) {
    printf("Failed to set filter: %d\n", err);
}
```

### Error Monitoring

```cpp
// Get error counters
hf_can_error_counters_t counters;
hf_can_err_t err = can.GetErrorCounters(counters);

if (err == HF_CAN_ERR_OK) {
    printf("Error counters:\n");
    printf("  TX Error Count: %u\n", counters.tx_error_count);
    printf("  RX Error Count: %u\n", counters.rx_error_count);
    printf("  Bus Off Count: %u\n", counters.bus_off_count);
}
```

## ESP32-C6 Specific Features

### TWAI Controller

The ESP32-C6 uses the TWAI (Two-Wire Automotive Interface) controller, which is fully compatible with CAN 2.0A and CAN 2.0B standards. **Note: CAN-FD is not supported by the ESP32-C6 TWAI controller.**

### Hardware Filtering

Up to 32 hardware filters for efficient message filtering without CPU overhead.

### DMA Support

High-performance DMA transfers for large message buffers.

## Error Handling

The `EspCan` class provides comprehensive error handling with specific error codes:

- `HF_CAN_ERR_OK` - Operation successful
- `HF_CAN_ERR_INVALID_ARG` - Invalid parameter
- `HF_CAN_ERR_NOT_INITIALIZED` - CAN not initialized
- `HF_CAN_ERR_TIMEOUT` - Operation timeout
- `HF_CAN_ERR_BUS_OFF` - CAN controller in bus-off state
- `HF_CAN_ERR_TX_FULL` - Transmit buffer full
- `HF_CAN_ERR_RX_EMPTY` - Receive buffer empty
- `HF_CAN_ERR_FILTER_FULL` - No more filters available

## Performance Considerations

- **Bus Speed**: Choose appropriate speed for your application (125k, 250k, 500k, 1M bps)
- **Termination**: Ensure proper 120Ω termination resistors on CAN bus
- **Cable Length**: Consider signal integrity for longer cables
- **Filter Usage**: Use hardware filters to reduce CPU load

## Related Documentation

- [BaseCan API Reference](../api/BaseCan.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF TWAI Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/twai.html) - ESP-IDF documentation
- [CAN Comprehensive Tests](../../examples/esp32/docs/README_CAN_TEST.md) - Complete CAN validation suite

---

<div align="center">

**📋 Navigation**

[← Previous: EspUart](EspUart.md) | [Back to ESP API Index](README.md) | [Next: EspWifi →](EspWifi.md)

</div>