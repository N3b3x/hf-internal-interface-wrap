# 📝 Data Logger Example

Store sensor data periodically using `NvsStorage`.

## 💡 Setup
- Configure flash storage partition
- Sample data from an ADC channel

## 🚀 Code
```cpp
#include "NvsStorage.h"
#include "McuAdc.h"

NvsStorage storage("logger");
McuAdc adc(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);

void app_main() {
    storage.Open();
    adc.Open();
    uint32_t index = 0;
    while (true) {
        float v = adc.ReadVoltage();
        storage.SetFloat("v" + std::to_string(index++), v);
    }
}
```

## 📝 Notes
- Consider batching writes to reduce flash wear
- Use the timestamp API from the platform layer for precise logging
