/**
 * @file BaseUsbSerialJtag.h
 * @ingroup uart
 * @brief Abstract base class for USB Serial/JTAG controller "console-style"
 *        serial transports in the HardFOC system.
 *
 * @details
 * Some MCUs (notably the ESP32-S3 / ESP32-C3 / ESP32-C6 family) ship a built-in
 * **USB Serial/JTAG controller**: a fixed-function silicon block that, via the
 * native USB jack, presents a CDC-ACM virtual serial port AND a JTAG endpoint
 * over a single cable. From the host PC it appears as `/dev/ttyACM*`, `COM*`,
 * or `/dev/cu.usbmodem*` and supports:
 *  - Bidirectional serial console (logs out + REPL/CLI in)
 *  - `idf.py flash` (auto download-mode via DTR/RTS)
 *  - Concurrent OpenOCD JTAG debugging
 *
 * It is **not** a UART:
 *  - No baud rate, no parity, no stop bits, no pin assignment.
 *  - No RS-485, no flow control, no break detection.
 *  - The block is hard-wired to the chip's USB D+/D- pads (ESP32-S3: GPIO19/20).
 *
 * Forcing this peripheral into the `BaseUart` contract would mean implementing
 * 80% of that API as `UART_ERR_UNSUPPORTED_OPERATION` no-ops. Instead, this
 * file defines a peer abstraction with the **same `hf_uart_err_t` error
 * vocabulary** so application code (consoles, simple text protocols) can swap
 * between `BaseUart*` and `BaseUsbSerialJtag*` backends with minimal glue.
 *
 *
 * @par Consumer pattern
 * @code
 * BaseUsbSerialJtag* serial = comms.AcquireUsbSerialJtag();  // lazy install
 * if (serial && serial->EnsureInitialized()) {
 *   serial->WriteLine("hello USB-VCP");
 *   uint8_t b{};
 *   if (serial->Read(&b, 1, 50) == hf_uart_err_t::UART_SUCCESS) { ... }
 * }
 * @endcode
 *
 * @author HardFOC
 * @date 2026
 * @copyright HardFOC
 *
 * @note Header-only abstract base, parallel to `BaseUart` / `BaseI2c` / etc.
 * @note Implementations are typically singletons because the underlying USB
 *       Serial/JTAG controller exposes a single host-facing endpoint pair.
 * @note Reuses `hf_uart_err_t` (declared in `BaseUart.h`) for return codes so
 *       upper layers don't need a third error enum.
 */

#pragma once

#include "BaseUart.h"  // for hf_uart_err_t + hf_uart_statistics_t/diagnostics_t reuse

#include <cstdarg>
#include <cstdint>
#include <cstring>

//==============================================================================
// USB Serial/JTAG configuration
//==============================================================================

/**
 * @brief Configuration for a USB Serial/JTAG controller instance.
 *
 * @details Intentionally minimal: the controller has no electrical knobs.
 * Only the host-side ring-buffer sizes matter, plus a couple of policy bits.
 */
struct hf_usb_serial_jtag_config_t {
  /// Bytes for the driver's internal TX ring buffer (must be > 0).
  /// Larger values absorb log bursts when the host PC is slow to drain.
  hf_u32_t tx_buffer_size{256};

  /// Bytes for the driver's internal RX ring buffer (must be > 0).
  hf_u32_t rx_buffer_size{256};

  /// When true, calls to `Read()` will not block longer than `timeout_ms`
  /// even if the host is not connected. When false the implementation may
  /// busy-poll connection state. Default true matches typical console use.
  bool non_blocking_when_disconnected{true};
};

//==============================================================================
// Abstract base class
//==============================================================================

/**
 * @class BaseUsbSerialJtag
 * @ingroup uart
 * @brief Abstract base class for built-in USB Serial/JTAG controller wrappers.
 *
 * @details Mirrors the lazy-initialise / Read / Write / Flush shape of
 * `BaseUart` so a `ConsoleSerialTransport` can hold either a `BaseUart*` or
 * a `BaseUsbSerialJtag*` behind a tiny adapter.
 *
 * @note Not inherently thread-safe. The underlying ESP-IDF driver is
 *       safe for concurrent reader/writer callers, but if you wrap it
 *       with stateful helpers (e.g. printf) protect them with a mutex.
 */
class BaseUsbSerialJtag {
public:
  virtual ~BaseUsbSerialJtag() noexcept = default;

  BaseUsbSerialJtag(const BaseUsbSerialJtag&)            = delete;
  BaseUsbSerialJtag& operator=(const BaseUsbSerialJtag&) = delete;
  BaseUsbSerialJtag(BaseUsbSerialJtag&&)                 = delete;
  BaseUsbSerialJtag& operator=(BaseUsbSerialJtag&&)      = delete;

  //----------------------------------------------------------------------//
  // Lazy lifecycle (matches BaseUart pattern)
  //----------------------------------------------------------------------//

  /// Install the underlying driver if not already installed.
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /// Uninstall the underlying driver if previously installed.
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
    }
    return !initialized_;
  }

  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  //----------------------------------------------------------------------//
  // Pure virtual contract
  //----------------------------------------------------------------------//

  /// Install the platform driver according to `config_`. Returns true on success.
  virtual bool Initialize() noexcept = 0;

  /// Tear down the platform driver. Returns true on success.
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Push `length` bytes to the host PC.
   * @param data        Pointer to source buffer.
   * @param length      Number of bytes to push.
   * @param timeout_ms  Maximum time to wait if the TX ring buffer is full
   *                    (0 = use the driver's default).
   * @return `UART_SUCCESS` if all bytes were accepted into the TX ring,
   *         `UART_ERR_TIMEOUT` if only some were accepted, or one of the
   *         configuration / not-initialised errors otherwise.
   */
  virtual hf_uart_err_t Write(const hf_u8_t* data, hf_u32_t length,
                              hf_u32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Pull up to `length` bytes from the host PC.
   * @param data        Destination buffer.
   * @param length      Capacity of destination buffer.
   * @param timeout_ms  Maximum time to wait for the first byte
   *                    (0 = poll, no wait).
   * @param[out] out_read_count  Optional: number of bytes actually read.
   * @return `UART_SUCCESS` when at least one byte was read,
   *         `UART_ERR_TIMEOUT` when no byte arrived in time, or another
   *         error code on failure.
   */
  virtual hf_uart_err_t Read(hf_u8_t* data, hf_u32_t length, hf_u32_t timeout_ms = 0,
                             hf_u32_t* out_read_count = nullptr) noexcept = 0;

  /// Block until the TX ring is fully drained to the host (or `timeout_ms`).
  virtual hf_uart_err_t FlushTx(hf_u32_t timeout_ms = 0) noexcept = 0;

  /// True iff the host PC is currently enumerating / receiving SOF packets.
  /// Returns false when only powered (e.g. plugged into a power bank).
  [[nodiscard]] virtual bool IsHostConnected() const noexcept = 0;

  //----------------------------------------------------------------------//
  // Convenience helpers (non-virtual)
  //----------------------------------------------------------------------//

  /// Write a NUL-terminated string (no automatic newline).
  hf_uart_err_t WriteString(const char* s, hf_u32_t timeout_ms = 0) noexcept {
    if (!s) {
      return hf_uart_err_t::UART_ERR_NULL_POINTER;
    }
    return Write(reinterpret_cast<const hf_u8_t*>(s), static_cast<hf_u32_t>(std::strlen(s)),
                 timeout_ms);
  }

  /// Write a NUL-terminated string then "\r\n".
  hf_uart_err_t WriteLine(const char* s, hf_u32_t timeout_ms = 0) noexcept {
    auto e = WriteString(s, timeout_ms);
    if (e != hf_uart_err_t::UART_SUCCESS) {
      return e;
    }
    static constexpr hf_u8_t crlf[2] = {'\r', '\n'};
    return Write(crlf, 2, timeout_ms);
  }

  /// Read exactly one byte. Returns `UART_ERR_TIMEOUT` when nothing arrives.
  hf_uart_err_t ReadByte(hf_u8_t& out, hf_u32_t timeout_ms = 0) noexcept {
    return Read(&out, 1, timeout_ms);
  }

  //----------------------------------------------------------------------//
  // Optional telemetry
  //----------------------------------------------------------------------//

  /// Snapshot byte counters (default: zeroed). Implementations may override.
  virtual hf_uart_statistics_t GetStatistics() const noexcept {
    return hf_uart_statistics_t{};
  }

  /// Snapshot diagnostic state (default: minimally populated).
  virtual hf_uart_diagnostics_t GetDiagnostics() const noexcept {
    hf_uart_diagnostics_t d{};
    d.is_initialized = initialized_;
    return d;
  }

protected:
  explicit BaseUsbSerialJtag(const hf_usb_serial_jtag_config_t& cfg) noexcept : config_(cfg) {}

  /// Subclasses may consult / mutate the buffered config before Initialize().
  hf_usb_serial_jtag_config_t config_{};

  /// Lifecycle latch maintained by `EnsureInitialized()` / `EnsureDeinitialized()`.
  bool initialized_{false};
};
