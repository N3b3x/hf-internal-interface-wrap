# 🔌 GPIO Guide

This guide explains how to configure and use General Purpose Input/Output (GPIO) pins through the wrapper.

## Digital Output

```cpp
#include "McuDigitalGpio.h"

McuDigitalGpio led(HF_GPIO_NUM_2);
led.SetAsOutput();
led.SetHigh();
```

Use `SetHigh()`, `SetLow()` and `Toggle()` to control the pin.

## Digital Input

```cpp
McuDigitalGpio button(HF_GPIO_NUM_0);
button.SetAsInput(HF_GPIO_PULL_UP);
bool pressed = button.IsActive();
```

Configure pull resistors via `HF_GPIO_PULL_UP` or `HF_GPIO_PULL_DOWN`.

## Interrupts

```cpp
button.EnableInterrupt(HF_GPIO_INT_FALLING, [](){
    // handle button press
});
```

Interrupt callbacks run in an ISR context, so keep them short.

## Thread-Safe Wrapper

```cpp
#include "SfGpio.h"

SfGpio safe_led(std::make_shared<McuDigitalGpio>(HF_GPIO_NUM_2));
```

## Best Practices

- Avoid changing pin modes after initialization.
- Use `DigitalOutputGuard` to automatically restore pin state.
- Debounce inputs either in software or using hardware filters.

## 🌟 Advanced Usage
- Use open-drain mode for multi-device bus lines
- Change drive strength if high-current loads require it

## 🛠️ Troubleshooting
- If interrupts do not fire, check the CPU interrupt matrix configuration
- On some MCUs certain pins are input-only; verify with the datasheet

## 🔗 Related Examples
- [🔌 Basic GPIO Example](../examples/basic-gpio.md)
- [🏭 Industrial I/O](../examples/industrial-io.md)
