# PeriodicTimer Class Guide

Simple wrapper around the `esp_timer` API that repeatedly invokes a callback.
Useful for scheduling periodic tasks without dealing with the raw driver.

## Features ⏲️
- Create timers with any period in microseconds
- `Start()` and `Stop()` helpers
- Works from tasks or ISRs via the ESP-IDF timer service

## Example
```cpp
#include "PeriodicTimer.h"

static void Blink(void*) {
    // toggle an LED
}

PeriodicTimer timer(&Blink);

timer.Start(1000000); // call every second
```

---

[← Previous](PwmOutput.md) | [Documentation Index](index.md)
