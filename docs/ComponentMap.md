# 🗺️ Component Map

This document lists the main header and source files in the project and provides quick links to the related documentation pages. Use it as a directory when exploring the codebase.

## 📂 Directory Overview

| Folder | Purpose |
|-------|---------|
| `include/base` | Abstract base interfaces |
| `include/mcu` | MCU-specific implementations |
| `include/thread_safe` | Thread-safe wrappers |
| `include/utils` | Utility classes |
| `src/mcu` | Implementation of MCU classes |
| `src/thread_safe` | Implementation of thread-safe classes |
| `src/utils` | Utility implementations |

## 🔗 File Reference

| Header | Source | Documentation |
|-------|-------|--------------|
| [`BaseAdc.h`](../include/base/BaseAdc.h) | *(header only)* | [BaseAdc](api/BaseAdc.md) |
| [`BaseCan.h`](../include/base/BaseCan.h) | *(header only)* | [BaseCan](api/BaseCan.md) |
| [`BaseGpio.h`](../include/base/BaseGpio.h) | *(header only)* | [BaseGpio](api/BaseGpio.md) |
| [`BaseI2c.h`](../include/base/BaseI2c.h) | *(header only)* | [BaseI2c](api/BaseI2c.md) |
| [`BasePio.h`](../include/base/BasePio.h) | *(header only)* | [BasePio](api/BasePio.md) |
| [`BasePwm.h`](../include/base/BasePwm.h) | *(header only)* | [BasePwm](api/BasePwm.md) |
| [`BaseSpi.h`](../include/base/BaseSpi.h) | *(header only)* | [BaseSpi](api/BaseSpi.md) |
| [`BaseUart.h`](../include/base/BaseUart.h) | *(header only)* | [BaseUart](api/BaseUart.md) |
| [`McuDigitalGpio.h`](../include/mcu/McuDigitalGpio.h) | [`src/mcu/McuDigitalGpio.cpp`](../src/mcu/McuDigitalGpio.cpp) | [McuDigitalGpio](api/McuDigitalGpio.md) |
| [`McuAdc.h`](../include/mcu/McuAdc.h) | [`src/mcu/McuAdc.cpp`](../src/mcu/McuAdc.cpp) | [McuAdc](api/McuAdc.md) |
| [`McuI2c.h`](../include/mcu/McuI2c.h) | [`src/mcu/McuI2c.cpp`](../src/mcu/McuI2c.cpp) | API pending |
| [`McuSpi.h`](../include/mcu/McuSpi.h) | [`src/mcu/McuSpi.cpp`](../src/mcu/McuSpi.cpp) | API pending |
| [`McuUart.h`](../include/mcu/McuUart.h) | [`src/mcu/McuUart.cpp`](../src/mcu/McuUart.cpp) | API pending |
| [`McuCan.h`](../include/mcu/McuCan.h) | [`src/mcu/McuCan.cpp`](../src/mcu/McuCan.cpp) | API pending |
| [`McuPwm.h`](../include/mcu/McuPwm.h) | [`src/mcu/McuPwm.cpp`](../src/mcu/McuPwm.cpp) | API pending |
| [`McuPio.h`](../include/mcu/McuPio.h) | [`src/mcu/McuPio.cpp`](../src/mcu/McuPio.cpp) | API pending |
| [`SfGpio.h`](../include/thread_safe/SfGpio.h) | [`src/thread_safe/SfGpio.cpp`](../src/thread_safe/SfGpio.cpp) | API pending |
| [`SfAdc.h`](../include/thread_safe/SfAdc.h) | [`src/thread_safe/SfAdc.cpp`](../src/thread_safe/SfAdc.cpp) | API pending |
| [`SfI2cBus.h`](../include/thread_safe/SfI2cBus.h) | [`src/thread_safe/SfI2cBus.cpp`](../src/thread_safe/SfI2cBus.cpp) | API pending |
| [`SfSpiBus.h`](../include/thread_safe/SfSpiBus.h) | [`src/thread_safe/SfSpiBus.cpp`](../src/thread_safe/SfSpiBus.cpp) | API pending |
| [`SfUartDriver.h`](../include/thread_safe/SfUartDriver.h) | [`src/thread_safe/SfUartDriver.cpp`](../src/thread_safe/SfUartDriver.cpp) | API pending |
| [`SfCan.h`](../include/thread_safe/SfCan.h) | [`src/thread_safe/SfCan.cpp`](../src/thread_safe/SfCan.cpp) | API pending |
| [`SfPwm.h`](../include/thread_safe/SfPwm.h) | [`src/thread_safe/SfPwm.cpp`](../src/thread_safe/SfPwm.cpp) | API pending |
| [`DigitalOutputGuard.h`](../include/utils/DigitalOutputGuard.h) | [`src/utils/DigitalOutputGuard.cpp`](../src/utils/DigitalOutputGuard.cpp) | [DigitalOutputGuard](api/DigitalOutputGuard.md) |
| [`NvsStorage.h`](../include/utils/NvsStorage.h) | [`src/utils/NvsStorage.cpp`](../src/utils/NvsStorage.cpp) | API pending |
| [`PeriodicTimer.h`](../include/utils/PeriodicTimer.h) | [`src/utils/PeriodicTimer.cpp`](../src/utils/PeriodicTimer.cpp) | API pending |

Use the links above to quickly locate files and read the corresponding documentation.

[⬆️ Back to Index](index.md)
