# 🚗 CAN Guide

The **Controller Area Network (CAN)** stack allows robust real-time communication between nodes. This guide introduces the CAN classes and shows how to transmit and receive messages.

## Opening the Bus

```cpp
#include "McuCan.h"

McuCan can(HF_CAN_PORT_0, 500000); // 500 kbit/s
can.Open();
```

Use `HF_CAN_PORT_0` (or `1` if available) and specify the desired bitrate.

## Sending Messages

```cpp
hf_can_message_t msg = {};
msg.identifier = 0x123;
msg.data_length_code = 2;
msg.data[0] = 0xAB;
msg.data[1] = 0xCD;

can.Transmit(msg);
```

The message structure uses `hf_*` types for portability.

## Receiving Messages

```cpp
hf_can_message_t rx;
if (can.Receive(rx, 100)) { // timeout in ms
    // Process message
}
```

Use the optional timeout parameter to wait for a message. Zero performs a non-blocking check.

## Thread-Safe Wrapper

For multi-threaded systems use `SfCan` which protects all operations with a mutex.

```cpp
#include "SfCan.h"
#include <mutex>

std::mutex can_mutex;
SfCan safe_can(HF_CAN_PORT_0, 500000, can_mutex);
```

## Tips

- Configure message filters in the MCU implementation for best performance.
- Use the [Testing Guide](testing-guide.md) to simulate CAN traffic during development.

## 🧰 Advanced Features
- **Listen Only Mode**: Enable this to monitor traffic without affecting the bus
- **Error Counters**: Read error counters to diagnose wiring problems
- **Remote Frames**: Support remote transmission requests if your MCU allows it

## 🛠️ Troubleshooting
- Check termination resistors if the bus won't start
- Monitor the error warning flag to catch intermittent disconnections

## 🔗 Related Examples
- [🚗 CAN Gateway Example](../examples/can-gateway.md)
