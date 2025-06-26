# 🔌 Basic GPIO Example

A quick demonstration of controlling an LED and reading a button using the wrapper APIs.

## 💡 Setup
- Connect an LED to `GPIO2`
- Connect a push button to `GPIO0` with a pull-up resistor

## 🚀 Code
```cpp
#include "McuDigitalGpio.h"

McuDigitalGpio led(HF_GPIO_NUM_2);
McuDigitalGpio button(HF_GPIO_NUM_0);

void app_main() {
    led.SetAsOutput();
    button.SetAsInput(HF_GPIO_PULL_UP);

    while (true) {
        if (button.IsActive()) {
            led.SetHigh();
        } else {
            led.SetLow();
        }
    }
}
```

## 📝 Notes
- `IsActive()` returns the debounced state if supported by the MCU layer
- Use `DigitalOutputGuard` for temporary pin control
