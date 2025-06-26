# 🚗 CAN Gateway Example

Forward CAN messages from one port to another while filtering IDs.

## 💡 Setup
- Connect two CAN transceivers to `CAN_PORT_0` and `CAN_PORT_1`
- Ensure termination resistors are present on both buses

## 🚀 Code
```cpp
#include "McuCan.h"

McuCan can0(HF_CAN_PORT_0, 500000);
McuCan can1(HF_CAN_PORT_1, 500000);

void app_main() {
    can0.Open();
    can1.Open();

    hf_can_message_t msg;
    while (can0.Receive(msg, 100)) {
        if (msg.identifier >= 0x100 && msg.identifier <= 0x1FF) {
            can1.Transmit(msg);
        }
    }
}
```

## 📝 Notes
- Use hardware filters if available for better performance
- Consider the [Thread Safety Guide](../guides/thread-safety.md) for multi-core systems
