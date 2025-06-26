# 🎯 Custom Protocol Example

Implement a simple protocol over PIO to drive WS2812 LEDs.

## 💡 Setup
- Chain of WS2812 LEDs connected to `GPIO4`
- RMT peripheral used for precise timing

## 🚀 Code
```cpp
#include "McuPio.h"
#include <vector>

McuPio pio(HF_GPIO_NUM_4);

void SendLedData(const std::vector<uint8_t>& colors) {
    pio.WriteBuffer(colors.data(), colors.size());
}
```

## 📝 Notes
- The [WS2812 example](ws2812-pio.md) demonstrates a full application
- Timing is critical; keep interrupts disabled during transfer if possible
