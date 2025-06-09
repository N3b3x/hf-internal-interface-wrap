# HF-IID-ESPIDF

Internal Interface Drivers – wrappers for the ESP‑IDF to be used in the HardFOC controller. 🏎️

For detailed API guides see [docs/index.md](docs/index.md).

## IID‑ESPIDF Overview

This component bundles the internal interface drivers and platform specific utilities used by the HardFOC project. The drivers provide low level access to GPIO, ADC and bus interfaces for ESP‑IDF targets. Utility code such as the base thread framework and the ThreadX compatibility layer are also included here as they depend on the underlying RTOS implementation.

Each abstraction is intentionally tiny and header only where possible. You simply create the object and call `Open()` or `Start()` when ready. Behind the scenes the verbose ESP‑IDF calls are made for you. This means you can write compact applications that remain portable between boards and even other MCU families that reuse the same interface layer.


### Contents
- `BaseGpio`, `DigitalInput`, `DigitalOutput` ⚙️
- Bus drivers like `SpiBus`, `I2cBus` and their thread safe versions 🚌
- Thread aware versions `SfSpiBus` and `SfI2cBus` 🧵
- `FlexCan` for CAN peripherals 🚐
- `PwmOutput` abstraction for LEDC based PWM generation 🎛️
- `PeriodicTimer` helper built on `esp_timer` ⏲️
- `UartDriver` serial helper 📡
- `DacOutput` for analog voltages 🎚️
- Platform utilities from `UTILITIES/common` (timers, mutex helpers, base threads) 🧰

### Usage
Add `iid-espidf` to your component requirements. The component exports the include directories for both the driver headers and the utilities and depends on FreeRTOS.

```cmake
idf_component_register(
    SRCS ...
    REQUIRES iid-espidf
)
```

### PwmOutput example
```cpp
#include "PwmOutput.h"

PwmOutput pwm(GPIO_NUM_4, LEDC_CHANNEL_0, LEDC_TIMER_0, 5000,
              LEDC_TIMER_13_BIT);

void app_main() {
    pwm.Start();
    pwm.SetDuty(0.5f); // 50% duty cycle
}
```

### PeriodicTimer example
```cpp
#include "PeriodicTimer.h"

static void Blink(void*) {
    // toggle LED
}

PeriodicTimer timer(&Blink);

void app_main() {
    timer.Start(500000); // 0.5 second period
}
```

### License

This project is licensed under the GNU General Public License v3.0 or later.
