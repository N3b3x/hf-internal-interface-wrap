# PwmOutput Class Guide

Wrapper around the ESP‑IDF LEDC driver for generating PWM on a single pin.

## Features
- Configurable channel, timer, frequency and resolution
- `Start()`, `Stop()` and `SetDuty()` helpers
- Integrates with `DigitalGpio` for active state handling

## Example
```cpp
PwmOutput pwm(GPIO_NUM_4, LEDC_CHANNEL_0, LEDC_TIMER_0, 5000, LEDC_TIMER_13_BIT);
pwm.Start();
pwm.SetDuty(0.5f);
```

---

[← Previous](SfFlexCan.md) | [Documentation Index](index.md) | [Next →](PeriodicTimer.md)
