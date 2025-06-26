# 🏎️ Motor Control Example

Drive a brushless motor using PWM outputs.

## 💡 Setup
- Connect the motor driver inputs to `GPIO18`, `GPIO19`, `GPIO21`
- Configure PWM frequency at 20 kHz

## 🚀 Code
```cpp
#include "PwmOutput.h"

PwmOutput phase_u(HF_GPIO_NUM_18, HF_LEDC_CHANNEL_0, HF_LEDC_TIMER_0, 20000);
PwmOutput phase_v(HF_GPIO_NUM_19, HF_LEDC_CHANNEL_1, HF_LEDC_TIMER_0, 20000);
PwmOutput phase_w(HF_GPIO_NUM_21, HF_LEDC_CHANNEL_2, HF_LEDC_TIMER_0, 20000);

void app_main() {
    phase_u.Start();
    phase_v.Start();
    phase_w.Start();
    phase_u.SetDuty(0.5f);
    phase_v.SetDuty(0.5f);
    phase_w.SetDuty(0.5f);
}
```

## 📝 Notes
- Adjust the duty cycle based on FOC or other control algorithms
- The wrapper supports multiple timers for advanced modulation schemes
