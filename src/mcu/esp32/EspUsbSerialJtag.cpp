/**
 * @file EspUsbSerialJtag.cpp
 * @brief ESP32-family USB Serial/JTAG console wrapper implementation.
 *
 * Bridges `BaseUsbSerialJtag` to the ESP-IDF `esp_driver_usb_serial_jtag`
 * component. See `EspUsbSerialJtag.h` for design rationale.
 *
 * @author HardFOC
 * @date 2026
 * @copyright HardFOC
 */

#include "EspUsbSerialJtag.h"

#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

static constexpr const char* TAG = "EspUsbSerialJtag";

namespace {

/// Convert milliseconds to FreeRTOS ticks, rounding 0 ms straight to 0.
inline TickType_t MsToTicks(hf_u32_t ms) noexcept {
  if (ms == 0) {
    return 0;
  }
  // pdMS_TO_TICKS rounds down; round up by one tick when there's a remainder
  // so callers that pass a small non-zero timeout never silently get 0 ticks.
  const TickType_t t = pdMS_TO_TICKS(ms);
  return (t == 0) ? 1 : t;
}

}  // namespace

//==============================================================================
// Construction / destruction
//==============================================================================

EspUsbSerialJtag::EspUsbSerialJtag(const hf_usb_serial_jtag_config_t& cfg) noexcept
    : BaseUsbSerialJtag(cfg) {
  statistics_.initialization_timestamp = static_cast<hf_u64_t>(esp_timer_get_time());
}

EspUsbSerialJtag::EspUsbSerialJtag() noexcept : EspUsbSerialJtag(hf_usb_serial_jtag_config_t{}) {}

EspUsbSerialJtag::~EspUsbSerialJtag() noexcept {
  // Best-effort cleanup; if we don't own the IDF-installed driver, leave it.
  if (initialized_ && owns_driver_) {
    (void)Deinitialize();
  }
}

//==============================================================================
// Lifecycle
//==============================================================================

bool EspUsbSerialJtag::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  // If `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` (or any other component already
  // brought the driver up) we MUST NOT call `usb_serial_jtag_driver_install`
  // a second time — it returns ESP_ERR_INVALID_STATE. Detect, latch, share.
  if (usb_serial_jtag_is_driver_installed()) {
    owns_driver_ = false;
    initialized_ = true;
    ESP_LOGI(TAG, "Sharing pre-installed USB Serial/JTAG driver (rx=%lu, tx=%lu cfg ignored)",
             static_cast<unsigned long>(config_.rx_buffer_size),
             static_cast<unsigned long>(config_.tx_buffer_size));
    return true;
  }

  if (config_.tx_buffer_size == 0 || config_.rx_buffer_size == 0) {
    ESP_LOGE(TAG, "Refusing install: tx/rx buffer sizes must be > 0");
    return false;
  }

  usb_serial_jtag_driver_config_t idf_cfg{};
  idf_cfg.tx_buffer_size = config_.tx_buffer_size;
  idf_cfg.rx_buffer_size = config_.rx_buffer_size;

  const esp_err_t e = usb_serial_jtag_driver_install(&idf_cfg);
  if (e != ESP_OK) {
    ESP_LOGE(TAG, "usb_serial_jtag_driver_install failed: %s", esp_err_to_name(e));
    return false;
  }

  owns_driver_ = true;
  initialized_ = true;
  ESP_LOGI(TAG, "Installed USB Serial/JTAG driver (tx=%lu, rx=%lu)",
           static_cast<unsigned long>(config_.tx_buffer_size),
           static_cast<unsigned long>(config_.rx_buffer_size));
  return true;
}

bool EspUsbSerialJtag::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }
  if (!owns_driver_) {
    // Don't touch a driver someone else (e.g. IDF console) is using.
    initialized_ = false;
    return true;
  }
  const esp_err_t e = usb_serial_jtag_driver_uninstall();
  if (e != ESP_OK) {
    ESP_LOGW(TAG, "usb_serial_jtag_driver_uninstall: %s", esp_err_to_name(e));
    return false;
  }
  initialized_ = false;
  owns_driver_ = false;
  return true;
}

//==============================================================================
// I/O
//==============================================================================

hf_uart_err_t EspUsbSerialJtag::Write(const hf_u8_t* data, hf_u32_t length,
                                      hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }
  if (!data) {
    return hf_uart_err_t::UART_ERR_NULL_POINTER;
  }
  if (length == 0) {
    return hf_uart_err_t::UART_SUCCESS;
  }

  const TickType_t ticks = MsToTicks(timeout_ms);
  const int written =
      usb_serial_jtag_write_bytes(static_cast<const void*>(data), static_cast<size_t>(length), ticks);

  if (written < 0) {
    return hf_uart_err_t::UART_ERR_TRANSMISSION_FAILED;
  }
  statistics_.tx_byte_count += static_cast<hf_u32_t>(written);
  statistics_.last_activity_timestamp = static_cast<hf_u64_t>(esp_timer_get_time());

  if (static_cast<hf_u32_t>(written) != length) {
    return hf_uart_err_t::UART_ERR_TIMEOUT;
  }
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUsbSerialJtag::Read(hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms,
                                     hf_u32_t* out_read_count) noexcept {
  if (out_read_count) {
    *out_read_count = 0;
  }
  if (!initialized_) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }
  if (!data) {
    return hf_uart_err_t::UART_ERR_NULL_POINTER;
  }
  if (length == 0) {
    return hf_uart_err_t::UART_SUCCESS;
  }

  const TickType_t ticks = MsToTicks(timeout_ms);
  const int got = usb_serial_jtag_read_bytes(static_cast<void*>(data), length, ticks);
  if (got < 0) {
    return hf_uart_err_t::UART_ERR_RECEPTION_FAILED;
  }
  if (out_read_count) {
    *out_read_count = static_cast<hf_u32_t>(got);
  }
  if (got == 0) {
    return hf_uart_err_t::UART_ERR_TIMEOUT;
  }
  statistics_.rx_byte_count += static_cast<hf_u32_t>(got);
  statistics_.last_activity_timestamp = static_cast<hf_u64_t>(esp_timer_get_time());
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUsbSerialJtag::FlushTx(hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }
  const esp_err_t e = usb_serial_jtag_wait_tx_done(MsToTicks(timeout_ms));
  if (e == ESP_ERR_TIMEOUT) {
    return hf_uart_err_t::UART_ERR_TIMEOUT;
  }
  return (e == ESP_OK) ? hf_uart_err_t::UART_SUCCESS : hf_uart_err_t::UART_ERR_FAILURE;
}

bool EspUsbSerialJtag::IsHostConnected() const noexcept {
  if (!initialized_) {
    return false;
  }
  return usb_serial_jtag_is_connected();
}

//==============================================================================
// Singleton helper
//==============================================================================

EspUsbSerialJtag& EspUsbSerialJtag::Default(const hf_usb_serial_jtag_config_t& cfg) noexcept {
  // Magic-static ensures one-time, thread-safe initialisation per process.
  static EspUsbSerialJtag instance{cfg};
  return instance;
}
