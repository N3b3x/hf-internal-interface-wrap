# 🎛️ PWM Guide

Pulse Width Modulation (PWM) is used for controlling motors, LEDs and other devices. This guide demonstrates how to configure and drive PWM outputs.

## Basic Setup

```cpp
#include "McuPwm.h"

McuPwm pwm(HF_GPIO_NUM_4, HF_LEDC_CHANNEL_0, HF_LEDC_TIMER_0, 20000);
pwm.Open();
```

The last parameter is the frequency in Hz.

## Changing Duty Cycle

```cpp
pwm.SetDuty(0.5f);  // 50%
pwm.SetDutyPercent(75); // 75%
```

Use floating point or percent-based setters to adjust duty cycle.

## Servo Example

```cpp
pwm.ConfigureForServo();
pwm.SetDutyPercent(5);   // full left
pwm.SetDutyPercent(10);  // center
pwm.SetDutyPercent(15);  // full right
```

## Tips

- Use `SfPwm` when controlling channels from multiple tasks.
- For high frequency signals ensure the timer resolution supports your desired accuracy.

## 🛠️ Troubleshooting
- Verify timer clock source when frequencies appear incorrect
- Some channels share timers; ensure you allocate them properly

## 🔗 Related Examples
- [🏎️ Motor Control](../examples/motor-control.md)
