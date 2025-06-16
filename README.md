# HF-IID-ESPIDF

Internal Interface Drivers – handy wrappers for ESP‑IDF used by the HardFOC controller. 🏎️ These abstractions keep your code tidy and portable across ESP32 variants.

> **Note:** This component requires **ESP-IDF v5.5 or newer**.

For detailed API guides see [docs/index.md](docs/index.md).

## IID‑ESPIDF Overview

This component bundles a growing collection of interface drivers and utilities. They hide verbose ESP‑IDF boilerplate while staying small and reusable. Alongside the hardware helpers you will also find base thread frameworks and RTOS utilities used across the HardFOC ecosystem.

Each abstraction is intentionally tiny and header only where possible. Create an object, call `Open()` or `Start()` and off you go. All the ESP‑IDF ceremony is performed behind the scenes so your code stays compact and portable between boards – and even other MCU families that reuse the same interface layer.


### Contents
- `BaseGpio`, `DigitalInput`, `DigitalOutput` ⚙️
- Bus drivers `SpiBus` and `I2cBus` 🚌
- Thread‑safe variants `SfSpiBus`, `SfI2cBus` and `SfUartDriver` 🧵
- `FlexCan` for CAN peripherals 🚐
- `SfFlexCan` thread-safe CAN driver 🛡️
- `PwmOutput` abstraction for LEDC PWM generation 🎛️
- `PeriodicTimer` helper built on `esp_timer` ⏲️
- `UartDriver` and `SfUartDriver` serial helpers 📡
- `DacOutput` for analog voltages 🎚️
- `RmtOutput` wrapper for the RMT peripheral 📡
- `NvsStorage` for saving settings 💾
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

### SfUartDriver example
```cpp
SemaphoreHandle_t m = xSemaphoreCreateMutex();
uart_config_t cfg = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};
SfUartDriver serial(UART_NUM_1, cfg, GPIO_NUM_1, GPIO_NUM_3, m);
serial.Open();
serial.Write(reinterpret_cast<const uint8_t*>("hi"), 2);
```


### RmtOutput example
```cpp
RmtOutput rmt(RMT_CHANNEL_0, GPIO_NUM_18, 80);
rmt.Open();
rmt_item32_t item = {};
item.level0 = 1;
item.duration0 = 500;
item.level1 = 0;
item.duration1 = 500;
rmt.Write(&item, 1);
```

### NvsStorage example
```cpp
NvsStorage storage("app");
storage.Open();
storage.SetU32("count", 42);
uint32_t v;
storage.GetU32("count", v);
```

### SfFlexCan example
```cpp
SemaphoreHandle_t cm = xSemaphoreCreateMutex();
SfFlexCan can(0, 500000, cm);
can.Open();
FlexCan::Frame f{0x100, {0x01}, 1, false, false};
can.Write(f);
```

### License

This project is licensed under the GNU General Public License v3.0 or later.
