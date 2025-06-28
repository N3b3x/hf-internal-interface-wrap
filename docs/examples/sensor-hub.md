# 🔒 Sensor Hub Example

Collect data from multiple sensors using thread-safe wrappers.

## 💡 Setup
- Two I2C sensors on bus 0
- UART device for debug output

## 🚀 Code
```cpp
#include "SfI2cBus.h"
#include "SfUartDriver.h"
#include "RtosMutex.h"

RtosMutex i2c_mutex;
RtosMutex uart_mutex;
SfI2cBus i2c(0, 400000, i2c_mutex);
SfUartDriver uart(1, {115200, HF_UART_DATA_8_BITS, HF_UART_PARITY_DISABLE, HF_UART_STOP_BITS_1, HF_UART_HW_FLOWCTRL_DISABLE}, HF_GPIO_NUM_1, HF_GPIO_NUM_3, uart_mutex);

void app_main() {
    i2c.Open();
    uart.Open();

    uint8_t data[2];
    while (true) {
        i2c.ReadFrom(0x40, data, 2);
        uart.Write(data, 2);
    }
}
```

## 📝 Notes
- Protecting both the I2C bus and UART ensures safe concurrent access
- Use FreeRTOS queues for sensor data if multiple tasks need it
