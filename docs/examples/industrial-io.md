# 🏭 Industrial I/O Example

Combine GPIO, ADC and CAN to create a simple industrial controller.

## 💡 Setup
- Digital outputs for actuators on `GPIO2` and `GPIO4`
- Analog input for sensor feedback
- CAN bus for remote monitoring

## 🚀 Code
```cpp
#include "McuDigitalGpio.h"
#include "McuAdc.h"
#include "McuCan.h"

McuDigitalGpio out1(HF_GPIO_NUM_2);
McuDigitalGpio out2(HF_GPIO_NUM_4);
McuAdc feedback(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);
McuCan can(HF_CAN_PORT_0, 250000);

void app_main() {
    out1.SetAsOutput();
    out2.SetAsOutput();
    feedback.Open();
    can.Open();

    hf_can_message_t tx = {};
    while (true) {
        float v = feedback.ReadVoltage();
        tx.identifier = 0x200;
        memcpy(tx.data, &v, sizeof(v));
        tx.data_length_code = 4;
        can.Transmit(tx);
    }
}
```

## 📝 Notes
- Add debouncing for any input signals
- Scale the voltage reading according to your sensor
