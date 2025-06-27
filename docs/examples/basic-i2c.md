# 🔄 Basic I2C Example

Communicate with a simple I2C temperature sensor.

## 💡 Setup
- Connect the sensor SDA to `GPIO21` and SCL to `GPIO22`
- Use pull-up resistors on both lines

## 🚀 Code
```cpp
#include "McuI2c.h"

McuI2c i2c(HF_I2C_NUM_0, HF_GPIO_NUM_21, HF_GPIO_NUM_22, 400000);

void app_main() {
    i2c.Open();
    uint8_t temp;
    i2c.ReadRegister(0x48, 0x00, temp);
}
```

## 📝 Notes
- Use `WriteRegister()` to configure the sensor
- Check the return values for error handling
