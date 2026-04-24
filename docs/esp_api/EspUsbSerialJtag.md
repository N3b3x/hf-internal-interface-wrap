---
layout: default
title: "🔌 EspUsbSerialJtag"
description: "ESP32-family wrapper for the built-in USB Serial/JTAG controller (CDC-ACM virtual COM port over native USB)"
nav_order: 15
parent: "🔧 ESP32 Implementations"
permalink: /docs/esp_api/EspUsbSerialJtag/
---

# 🔌 EspUsbSerialJtag API Reference

## Overview

`EspUsbSerialJtag` is the ESP32-family implementation of
[`BaseUsbSerialJtag`](../api/BaseUsbSerialJtag.md). It wraps the ESP-IDF
`esp_driver_usb_serial_jtag` component (`driver/usb_serial_jtag.h`) so that
application code can use the chip's built-in USB Serial/JTAG controller
without leaking ESP-IDF / FreeRTOS types into MCU-agnostic layers.

### Supported targets

| Chip       | USB pads (TX/RX, fixed) | Notes                                                    |
|------------|-------------------------|----------------------------------------------------------|
| ESP32-S3   | GPIO19 (D−) / GPIO20 (D+) | Most common HardFOC target.                            |
| ESP32-C3   | GPIO18 / GPIO19         |                                                          |
| ESP32-C6   | GPIO12 / GPIO13         |                                                          |
| ESP32-H2   | GPIO26 / GPIO27         |                                                          |
| ESP32-P4   | GPIO24 / GPIO25         |                                                          |
| ESP32 / S2 | —                       | **No** USB Serial/JTAG controller; use a UART bridge instead. |

The pads are part of the chip's silicon USB block — they cannot be remapped.

### Features

- One-call install via `EnsureInitialized()` (lazy lifecycle).
- Auto-detect of an already-installed IDF console driver
  (`CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`) — shares the install instead of
  failing with `ESP_ERR_INVALID_STATE`.
- Process-wide singleton via `Default()` so multiple subsystems can use the
  same controller without coordination.
- Per-instance `hf_uart_statistics_t` byte counters.
- Cleanly tears down only its own driver install (never an IDF-installed one).

---

## Header File

```cpp
#include "mcu/esp32/EspUsbSerialJtag.h"
```

The wrapper relies on the
[`esp_driver_usb_serial_jtag`](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/usb-serial-jtag-console.html)
ESP-IDF component. Make sure your component CMakeLists lists it under
`PRIV_REQUIRES` (or `REQUIRES` if the header is exposed).

---

## Class Definition

```cpp
class EspUsbSerialJtag final : public BaseUsbSerialJtag {
public:
    explicit EspUsbSerialJtag(const hf_usb_serial_jtag_config_t& cfg) noexcept;
    EspUsbSerialJtag() noexcept;
    ~EspUsbSerialJtag() noexcept override;

    bool          Initialize() noexcept override;
    bool          Deinitialize() noexcept override;
    hf_uart_err_t Write(const hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms = 0) noexcept override;
    hf_uart_err_t Read(hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms = 0,
                       hf_u32_t* out_read_count = nullptr) noexcept override;
    hf_uart_err_t FlushTx(hf_u32_t timeout_ms = 0) noexcept override;
    bool          IsHostConnected() const noexcept override;

    hf_uart_statistics_t GetStatistics() const noexcept override;

    static EspUsbSerialJtag& Default(const hf_usb_serial_jtag_config_t& cfg = {}) noexcept;
};
```

### Construction

| Constructor                                            | Notes                                                      |
|--------------------------------------------------------|------------------------------------------------------------|
| `EspUsbSerialJtag()`                                   | Library defaults (256-byte TX/RX rings).                   |
| `EspUsbSerialJtag(cfg)`                                | Custom buffer sizes via `hf_usb_serial_jtag_config_t`.     |
| `EspUsbSerialJtag::Default(cfg)`                       | Magic-static singleton; `cfg` applied **only** on the first call. |

The constructors do **not** install the driver — call `EnsureInitialized()`
or `Initialize()` to do so. This matches `BaseUart` / `EspUart`.

### Coexistence with the IDF console

When `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` is set in `sdkconfig`, ESP-IDF
brings up the controller before `app_main()` runs and pipes `printf` /
`ESP_LOG*` through it. `Initialize()` detects this via
`usb_serial_jtag_is_driver_installed()`, latches `owns_driver_ = false`, and
returns success without calling `usb_serial_jtag_driver_install()` a second
time. Both paths (IDF console + this wrapper) then share the same TX ring;
writes interleave at byte granularity but are never lost.

When `Deinitialize()` is later invoked on a wrapper that does **not** own
the driver, only the local lifecycle latch is cleared — the underlying
driver is left running for the IDF console.

---

## Usage Examples

### Quick console echo

```cpp
#include "mcu/esp32/EspUsbSerialJtag.h"

extern "C" void app_main() {
    auto& serial = EspUsbSerialJtag::Default();
    if (!serial.EnsureInitialized()) {
        return;
    }

    serial.WriteLine("EspUsbSerialJtag: ready (type to echo)");

    uint8_t b{};
    while (true) {
        if (serial.ReadByte(b, /*timeout_ms=*/100) == hf_uart_err_t::UART_SUCCESS) {
            (void)serial.Write(&b, 1);
        }
    }
}
```

### Bursty logging with FlushTx

```cpp
auto& serial = EspUsbSerialJtag::Default();
serial.EnsureInitialized();

for (int i = 0; i < 32; ++i) {
    char line[64];
    int n = std::snprintf(line, sizeof(line), "tick %d\r\n", i);
    (void)serial.Write(reinterpret_cast<const hf_u8_t*>(line), n);
}

// Make sure all 32 lines are out before we power down.
(void)serial.FlushTx(/*timeout_ms=*/200);
```

### Custom buffer sizes

```cpp
hf_usb_serial_jtag_config_t cfg{};
cfg.tx_buffer_size = 4096;   // larger TX ring for verbose log bursts
cfg.rx_buffer_size = 256;

EspUsbSerialJtag serial{cfg};
serial.EnsureInitialized();
```

> Use either a custom local instance **or** `Default()` consistently —
> mixing the two means the singleton's first-call config wins for everyone
> using `Default()`.

### Skip noisy output when no host is enumerated

```cpp
auto& serial = EspUsbSerialJtag::Default();
serial.EnsureInitialized();

if (serial.IsHostConnected()) {
    serial.WriteLine("verbose status...");
}
```

---

## CMake Integration

If the parent project consumes this wrapper through `hf-core`, the relevant
flag is `HF_CORE_ENABLE_USB_SERIAL_JTAG` (default OFF). Turning it ON:

- Compiles `src/mcu/esp32/EspUsbSerialJtag.cpp` into the component.
- Adds `esp_driver_usb_serial_jtag` to the IDF requires list.
- Defines `HARDFOC_USB_SERIAL_JTAG_SUPPORT=1` for downstream code.

If you depend on the wrap repo directly (e.g. as a standalone IDF
component), simply make sure your component CMakeLists requires
`esp_driver_usb_serial_jtag`:

```cmake
idf_component_register(
    SRCS "${SOURCES}"
    INCLUDE_DIRS "${INC_DIRS}"
    REQUIRES driver esp_timer freertos esp_driver_usb_serial_jtag
)
```

---

## Thread Safety

| API                  | Multiple callers safe? | Notes                                                                  |
|----------------------|------------------------|------------------------------------------------------------------------|
| `Write` / `WriteString` / `WriteLine` | ✅ Yes              | The ESP-IDF driver serializes ring access internally.                  |
| `Read` / `ReadByte`  | ⚠️ Single reader recommended | Multiple readers compete for bytes; wrap with a mutex if order matters. |
| `FlushTx`            | ✅ Yes                 | Safe to call from any task; blocks until the ring is drained.          |
| `IsHostConnected`    | ✅ Yes                 | Cheap, lock-free.                                                      |
| `Initialize` / `Deinitialize` | ⚠️ Once             | Call from a single owner (typically the HAL bring-up).                 |

The `EnsureInitialized()` / `EnsureDeinitialized()` helpers are idempotent
but not internally locked — call them from a single bring-up task.

---

## Related Documents

- [`BaseUsbSerialJtag`](../api/BaseUsbSerialJtag.md) — abstract interface and rationale.
- [`EspUart`](EspUart.md) — classic UART implementation when you need baud / parity / pin control.
- [USB Serial/JTAG Console (ESP-IDF)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/usb-serial-jtag-console.html)
- [`usb_serial_jtag` driver header (ESP-IDF)](https://github.com/espressif/esp-idf/blob/master/components/esp_driver_usb_serial_jtag/include/driver/usb_serial_jtag.h)
- Example app: [`UsbSerialJtagComprehensiveTest.cpp`](../../examples/esp32/main/UsbSerialJtagComprehensiveTest.cpp)
  (`./scripts/build_app.sh usb_serial_jtag_test Release`).
