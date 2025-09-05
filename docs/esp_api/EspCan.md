# EspCan API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspUart](EspUart.md) | [Back to ESP API Index](README.md) | [Next: EspWifi
→](EspWifi.md)

</div>

---

## Overview

`EspCan` is the ESP32-C6 implementation of the `BaseCan` interface,
providing comprehensive CAN (Controller Area Network) functionality specifically optimized for
ESP32-C6 microcontrollers running ESP-IDF v5.5+.
It offers both basic and advanced CAN features with hardware-specific optimizations.

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
```text

## Class Definition

```cpp
class EspCan : public BaseCan {
public:
    // Constructor with full configuration
    explicit EspCan(
        hf*can*port*t port = hf*can*port*t::HF*CAN*PORT*0,
        hf*pin*num*t tx*pin = GPIO*NUM*5,
        hf*pin*num*t rx*pin = GPIO*NUM*4,
        hf*can*speed*t speed = hf*can*speed*t::HF*CAN*SPEED*500K,
        hf*can*mode*t mode = hf*can*mode*t::HF*CAN*MODE*NORMAL
    ) noexcept;

    // Destructor
    ~EspCan() override;

    // BaseCan implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // CAN operations
    hf*can*err*t SendMessage(const hf*can*message*t& message) noexcept override;
    hf*can*err*t ReceiveMessage(hf*can*message*t& message, hf*u32*t timeout*ms = 0) noexcept override;
    hf*can*err*t GetMessageCount(hf*size*t* count) const noexcept override;

    // Advanced features
    hf*can*err*t SetSpeed(hf*can*speed*t speed) noexcept override;
    hf*can*err*t GetSpeed(hf*can*speed*t* speed) const noexcept override;
    hf*can*err*t SetFilter(const hf*can*filter*t& filter) noexcept override;
    hf*can*err*t ClearFilters() noexcept override;
    hf*can*err*t GetErrorCounters(hf*can*error*counters*t& counters) noexcept override;
};
```text

## Usage Examples

### Basic CAN Communication

```cpp
#include "inc/mcu/esp32/EspCan.h"

// Create CAN instance
EspCan can(HF*CAN*PORT*0, GPIO*NUM*5, GPIO*NUM*4, HF*CAN*SPEED*500K);

// Initialize
if (!can.Initialize()) {
    printf("Failed to initialize CAN\n");
    return;
}

// Send a message
hf*can*message*t message;
message.id = 0x123;
message.flags = HF*CAN*FLAG*STANDARD;
message.data*length = 8;
message.data[0] = 0x01;
message.data[1] = 0x02;
// ... fill remaining data

hf*can*err*t err = can.SendMessage(message);
if (err != HF*CAN*ERR*OK) {
    printf("CAN send failed: %d\n", err);
}

// Receive a message
hf*can*message*t received*message;
err = can.ReceiveMessage(received*message, 1000); // 1 second timeout
if (err == HF*CAN*ERR*OK) {
    printf("Received message ID: 0x%X, Data: ", received*message.id);
    for (int i = 0; i < received*message.data*length; i++) {
        printf("%02X ", received*message.data[i]);
    }
    printf("\n");
}
```text

### Message Filtering

```cpp
// Set up a filter to only receive messages with specific ID
hf*can*filter*t filter;
filter.id = 0x123;
filter.mask = 0x7FF; // Standard 11-bit ID mask
filter.flags = HF*CAN*FLAG*STANDARD;

hf*can*err*t err = can.SetFilter(filter);
if (err != HF*CAN*ERR*OK) {
    printf("Failed to set filter: %d\n", err);
}
```text

### Error Monitoring

```cpp
// Get error counters
hf*can*error*counters*t counters;
hf*can*err*t err = can.GetErrorCounters(counters);

if (err == HF*CAN*ERR*OK) {
    printf("Error counters:\n");
    printf("  TX Error Count: %u\n", counters.tx*error*count);
    printf("  RX Error Count: %u\n", counters.rx*error*count);
    printf("  Bus Off Count: %u\n", counters.bus*off*count);
}
```text

## ESP32-C6 Specific Features

### TWAI Controller

The ESP32-C6 uses the TWAI (Two-Wire Automotive Interface) controller,
which is fully compatible with CAN 2.0A and CAN 2.0B standards.
**Note: CAN-FD is not supported by the ESP32-C6 TWAI controller.**

### Hardware Filtering

Up to 32 hardware filters for efficient message filtering without CPU overhead.

### DMA Support

High-performance DMA transfers for large message buffers.

## Error Handling

The `EspCan` class provides comprehensive error handling with specific error codes:

- `HF*CAN*ERR*OK` - Operation successful
- `HF*CAN*ERR*INVALID*ARG` - Invalid parameter
- `HF*CAN*ERR*NOT*INITIALIZED` - CAN not initialized
- `HF*CAN*ERR*TIMEOUT` - Operation timeout
- `HF*CAN*ERR*BUS*OFF` - CAN controller in bus-off state
- `HF*CAN*ERR*TX*FULL` - Transmit buffer full
- `HF*CAN*ERR*RX*EMPTY` - Receive buffer empty
- `HF*CAN*ERR*FILTER*FULL` - No more filters available

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

[← Previous: EspUart](EspUart.md) | [Back to ESP API Index](README.md) | [Next: EspWifi
→](EspWifi.md)

</div>