/**
 * @file RMT.hpp
 * @brief High level RAII wrapper for the ESP-IDF RMT driver.
 */

// rmt_wrapper.hpp — High-level C++ RAII wrapper for the ESP-IDF v5.3 RMT driver
// on ESP32-C6 Copyright (c) 2025 Nebiyu Tadesse SPDX-License-Identifier:
// GPL-3.0-or-later
#ifndef RMT_WRAPPER_HPP
#define RMT_WRAPPER_HPP

#if !defined(ESP_IDF_VERSION)
#error "This file requires ESP-IDF"
#elif ESP_IDF_VERSION_MAJOR < 5
#error "This file requires ESP-IDF v5.0 or later"
#endif

#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include <functional>
#include <memory>

namespace HardFOC {
namespace Utils {

/**
 * @brief RAII wrapper for ESP-IDF RMT (Remote Control Transceiver) driver
 *
 * This class provides a C++ wrapper around the ESP-IDF RMT driver for
 * generating and receiving infrared remote control signals, WS2812 LED
 * data, and other pulse-width modulated signals.
 */
class RMTWrapper {
public:
  using TxCallback = std::function<void()>;
  using RxCallback = std::function<void(const rmt_symbol_word_t *data, size_t num_symbols)>;

  /**
   * @brief Constructor for TX-only RMT channel
   * @param gpio_num GPIO pin number for RMT output
   * @param resolution_hz Resolution of RMT timer in Hz
   */
  RMTWrapper(int gpio_num, uint32_t resolution_hz = 1000000);

  ~RMTWrapper();

  // Non-copyable but movable
  RMTWrapper(const RMTWrapper &) = delete;
  RMTWrapper &operator=(const RMTWrapper &) = delete;
  RMTWrapper(RMTWrapper &&) = default;
  RMTWrapper &operator=(RMTWrapper &&) = default;

  /**
   * @brief Initialize the RMT channel
   * @return true if successful
   */
  bool Initialize();

  /**
   * @brief Transmit RMT symbols
   * @param symbols Array of RMT symbols to transmit
   * @param num_symbols Number of symbols in the array
   * @param wait_until_done If true, block until transmission is complete
   * @return true if successful
   */
  bool Transmit(const rmt_symbol_word_t *symbols, size_t num_symbols, bool wait_until_done = true);

  /**
   * @brief Enable/disable the RMT channel
   * @param enable true to enable, false to disable
   * @return true if successful
   */
  bool Enable(bool enable = true);

  /**
   * @brief Check if RMT channel is initialized
   */
  bool IsInitialized() const {
    return tx_channel_ != nullptr;
  }

private:
  rmt_channel_handle_t tx_channel_;
  rmt_encoder_handle_t encoder_;
  int gpio_num_;
  uint32_t resolution_hz_;
  bool initialized_;
};

} // namespace Utils
} // namespace HardFOC

#endif // RMT_WRAPPER_HPP
