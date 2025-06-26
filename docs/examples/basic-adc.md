# 📊 Basic ADC Example

Read an analog sensor and print the voltage value.

## 💡 Setup
- Connect the sensor to `ADC_CHANNEL_0`
- Ensure the reference voltage is configured correctly

## 🚀 Code
```cpp
#include "McuAdc.h"
#include <stdio.h>

McuAdc adc(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);

void app_main() {
    adc.Open();
    while (true) {
        float voltage = adc.ReadVoltage();
        printf("Voltage: %.3fV\n", voltage);
    }
}
```

## 📝 Notes
- Use `ReadAveraged()` to smooth noisy sensors
- On some MCUs call `Calibrate()` before reading for best accuracy
