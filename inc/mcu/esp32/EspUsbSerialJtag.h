/**
 * @file EspUsbSerialJtag.h
 * @ingroup uart
 * @brief ESP32-S3 / -C3 / -C6 USB Serial/JTAG console wrapper.
 *
 * @details
 * Concrete implementation of `BaseUsbSerialJtag` for Espressif chips that
 * embed the **USB Serial/JTAG controller** (see ESP-IDF
 * `components/esp_driver_usb_serial_jtag/`). Internally calls
 * `usb_serial_jtag_driver_install()` once and forwards reads/writes to
 * `usb_serial_jtag_read_bytes()` / `usb_serial_jtag_write_bytes()`.
 *
 * @par Why a wrapper?
 * Application code in `main/` MUST NOT include `driver/usb_serial_jtag.h`
 * directly — that would leak `esp_err_t`, `TickType_t`, and FreeRTOS into
 * MCU-agnostic layers. By owning the driver behind `BaseUsbSerialJtag`,
 * upper layers stay portable to alternate transports (host-side mock,
 * future USB-CDC OTG implementation, etc.).
 *
 * @par Coexistence with the IDF console
 * If `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`, the IDF console driver is
 * already installed before `app_main` runs and `usb_serial_jtag_is_driver_installed()`
 * returns true. `Initialize()` detects this and treats the controller as
 * already-good without calling `usb_serial_jtag_driver_install()` a second
 * time (which would return `ESP_ERR_INVALID_STATE`). In that mode, the IDF
 * `printf` / `ESP_LOG` infrastructure shares the controller with this
 * wrapper; both paths push to the same TX ring without conflict.
 *
 * @par Thread safety
 * The Espressif driver supports concurrent reader and writer tasks. This
 * wrapper holds no internal state beyond `initialized_` after `Initialize()`,
 * so multiple writer tasks may call `Write()` concurrently. If you call
 * `Read()` from more than one task, hold an external mutex.
 *
 * @par Singleton helper
 * Most boards only ever need one instance (the controller is a single
 * silicon block). Use `EspUsbSerialJtag::Default()` to grab a process-wide
 * shared instance configured with the supplied `config` on first call.
 *
 * @author HardFOC
 * @date 2026
 * @copyright HardFOC
 */

#pragma once

#include "BaseUsbSerialJtag.h"
#include "McuSelect.h"  // brings esp32 toolchain knobs

#include <cstdint>

class EspUsbSerialJtag final : public BaseUsbSerialJtag {
public:
  /// Construct with explicit configuration (driver is NOT installed yet —
  /// call `EnsureInitialized()` or `Initialize()` to install).
  explicit EspUsbSerialJtag(const hf_usb_serial_jtag_config_t& cfg) noexcept;

  /// Construct with library defaults (256-byte TX/RX rings).
  EspUsbSerialJtag() noexcept;

  ~EspUsbSerialJtag() noexcept override;

  //----- BaseUsbSerialJtag contract --------------------------------------//
  bool          Initialize() noexcept override;
  bool          Deinitialize() noexcept override;
  hf_uart_err_t Write(const hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms = 0) noexcept override;
  hf_uart_err_t Read(hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms = 0,
                     hf_u32_t* out_read_count = nullptr) noexcept override;
  hf_uart_err_t FlushTx(hf_u32_t timeout_ms = 0) noexcept override;
  [[nodiscard]] bool IsHostConnected() const noexcept override;

  //----- Telemetry -------------------------------------------------------//
  hf_uart_statistics_t GetStatistics() const noexcept override {
    return statistics_;
  }

  /**
   * @brief Get-or-create a process-wide shared instance.
   * @details The first call configures the singleton with `cfg`. Subsequent
   *          calls ignore `cfg` and return the same instance. This matches
   *          the underlying hardware: the USB Serial/JTAG block is global.
   * @param cfg Configuration applied on first call only.
   * @return Reference to the shared instance.
   */
  static EspUsbSerialJtag& Default(const hf_usb_serial_jtag_config_t& cfg = {}) noexcept;

private:
  /// True when this wrapper called `usb_serial_jtag_driver_install()` itself
  /// (vs. discovering an already-installed IDF console driver). Controls
  /// whether `Deinitialize()` calls `usb_serial_jtag_driver_uninstall()`.
  bool owns_driver_{false};

  hf_uart_statistics_t statistics_{};
};
