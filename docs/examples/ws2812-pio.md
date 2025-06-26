# 🌈 WS2812 PIO Example

Drive addressable LEDs using the PIO wrapper.

## 💡 Setup
- Data line connected to `GPIO4`
- Use external power supply for long LED strips

## 🚀 Code
```cpp
#include "McuPio.h"

McuPio ws2812(HF_GPIO_NUM_4);

void app_main() {
    std::array<uint8_t, 3> red = {0xFF, 0x00, 0x00};
    ws2812.WriteBuffer(red.data(), red.size());
}
```

## 📝 Notes
- See [Custom Protocol Example](custom-protocol.md) for a reusable driver
- Timing is automatically handled by the wrapper
