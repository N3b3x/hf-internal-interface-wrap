/**
 * @file EspTypes_PIO.h
 * @brief ESP32 PIO/RMT type definitions for hardware abstraction.
 *
 * This header defines only the essential PIO/RMT-specific types used by
 * the EspPio implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BasePio.h" // For hf_pio_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)

//==============================================================================
// ESP32 PIO/RMT TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspPio implementation

//==============================================================================
// ESP32 PIO/RMT CONSTANTS
//==============================================================================

// ESP32 Variant-Specific RMT Channel Allocation
// Based on ESP-IDF v5.5 specifications for different ESP32 models

#if defined(CONFIG_IDF_TARGET_ESP32)
// ESP32: 8 channels, each configurable as TX or RX
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 8;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 8;  // All channels can be TX
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 8;  // All channels can be RX
static constexpr uint8_t HF_RMT_TX_CHANNEL_START = 0; // TX channels: 0-7
static constexpr uint8_t HF_RMT_RX_CHANNEL_START = 0; // RX channels: 0-7

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
// ESP32-S2: 4 channels, each configurable as TX or RX
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 4;  // All channels can be TX
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 4;  // All channels can be RX
static constexpr uint8_t HF_RMT_TX_CHANNEL_START = 0; // TX channels: 0-3
static constexpr uint8_t HF_RMT_RX_CHANNEL_START = 0; // RX channels: 0-3

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// ESP32-S3: 8 channels, hardcoded TX/RX allocation
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 8;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 4;  // Channels 0-3 are hardcoded for TX
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 4;  // Channels 4-7 are hardcoded for RX
static constexpr uint8_t HF_RMT_TX_CHANNEL_START = 0; // TX channels: 0-3
static constexpr uint8_t HF_RMT_RX_CHANNEL_START = 4; // RX channels: 4-7

#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || \
    defined(CONFIG_IDF_TARGET_ESP32H2)
// ESP32-C3/C6/H2: 4 channels, hardcoded TX/RX allocation
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;  // Channels 0-1 are hardcoded for TX
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;  // Channels 2-3 are hardcoded for RX
static constexpr uint8_t HF_RMT_TX_CHANNEL_START = 0; // TX channels: 0-1
static constexpr uint8_t HF_RMT_RX_CHANNEL_START = 2; // RX channels: 2-3

#else
// Default fallback for unknown ESP32 variants
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;
static constexpr uint8_t HF_RMT_TX_CHANNEL_START = 0;
static constexpr uint8_t HF_RMT_RX_CHANNEL_START = 2;
#endif

// Common RMT constants for all ESP32 variants
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024;
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64;
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000;    // 80 MHz max
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;        // 1 kHz min
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000; // 1 MHz default
static constexpr uint8_t HF_RMT_MAX_QUEUE_DEPTH = 32;
static constexpr uint8_t HF_RMT_MAX_INTERRUPT_PRIORITY = 7;

//==============================================================================
// ESP32 PIO/RMT ENUMS
//==============================================================================

/**
 * @brief ESP32 RMT clock source selection.
 */
enum class hf_rmt_clock_source_t : uint8_t {
  HF_RMT_CLK_SRC_DEFAULT = 0, ///< Default clock source (APB)
  HF_RMT_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_RMT_CLK_SRC_XTAL = 2,    ///< Crystal clock (40MHz)
  HF_RMT_CLK_SRC_RC_FAST = 3, ///< RC fast clock (~8MHz)
};

/**
 * @brief ESP32 RMT channel direction.
 */
enum class hf_rmt_channel_direction_t : uint8_t {
  HF_RMT_CHANNEL_DIRECTION_TX = 0, ///< Transmit direction
  HF_RMT_CHANNEL_DIRECTION_RX = 1, ///< Receive direction
};

//==============================================================================
// ESP32 PIO/RMT CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief ESP32 RMT transmission configuration.
 */
struct hf_rmt_transmit_config_t {
  uint32_t loop_count;   ///< Loop count (0 = no loop)
  bool invert_signal;    ///< Invert output signal
  bool with_dma;         ///< Enable DMA mode for large transfers
  uint32_t queue_depth;  ///< TX queue depth (1-32)
  uint8_t intr_priority; ///< Interrupt priority (0-7)
  bool allow_pd;         ///< Allow power down in sleep modes

  hf_rmt_transmit_config_t() noexcept
      : loop_count(0), invert_signal(false), with_dma(false), queue_depth(4), intr_priority(0),
        allow_pd(false) {}
};

/**
 * @brief ESP32 RMT reception configuration.
 */
struct hf_rmt_receive_config_t {
  uint32_t signal_range_min_ns; ///< Minimum signal range in nanoseconds
  uint32_t signal_range_max_ns; ///< Maximum signal range in nanoseconds
  bool with_dma;                ///< Enable DMA mode for large transfers
  uint8_t intr_priority;        ///< Interrupt priority (0-7)
  bool allow_pd;                ///< Allow power down in sleep modes

  hf_rmt_receive_config_t() noexcept
      : signal_range_min_ns(1000), signal_range_max_ns(1000000), with_dma(false), intr_priority(0),
        allow_pd(false) {}
};

/**
 * @brief ESP32 RMT carrier configuration for IR protocols.
 */
struct hf_rmt_carrier_config_t {
  uint32_t frequency_hz;       ///< Carrier frequency in Hz
  float duty_cycle;            ///< Duty cycle (0.0 to 1.0)
  uint8_t polarity_active_low; ///< Carrier polarity (0=high, 1=low)
  bool always_on;              ///< Always on carrier mode

  hf_rmt_carrier_config_t() noexcept
      : frequency_hz(38000), duty_cycle(0.5f), polarity_active_low(0), always_on(false) {}
};

//==============================================================================
// ESP32 PIO/RMT VALIDATION MACROS
//==============================================================================

/**
 * @brief RMT validation macros for ESP32 variants.
 */
#define HF_RMT_IS_VALID_CHANNEL(ch) ((ch) < HF_RMT_MAX_CHANNELS)

// ESP32 variant-specific TX channel validation
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
// ESP32/ESP32-S2: Any channel can be TX or RX
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) < HF_RMT_MAX_CHANNELS)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) < HF_RMT_MAX_CHANNELS)
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// ESP32-S3: Channels 0-3 for TX, 4-7 for RX
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) >= 0 && (ch) < 4)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) >= 4 && (ch) < 8)
#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || \
    defined(CONFIG_IDF_TARGET_ESP32H2)
// ESP32-C3/C6/H2: Channels 0-1 for TX, 2-3 for RX
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) >= 0 && (ch) < 2)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) >= 2 && (ch) < 4)
#else
// Default fallback
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) >= 0 && (ch) < 2)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) >= 2 && (ch) < 4)
#endif

#define HF_RMT_IS_VALID_RESOLUTION(res) \
  ((res) >= HF_RMT_MIN_RESOLUTION_HZ && (res) <= HF_RMT_MAX_RESOLUTION_HZ)
#define HF_RMT_IS_VALID_MEM_BLOCK_SIZE(size) \
  ((size) >= HF_RMT_MIN_MEM_BLOCK_SYMBOLS && (size) <= HF_RMT_MAX_MEM_BLOCK_SYMBOLS)
#define HF_RMT_IS_VALID_QUEUE_DEPTH(depth) ((depth) >= 1 && (depth) <= HF_RMT_MAX_QUEUE_DEPTH)
#define HF_RMT_IS_VALID_INTR_PRIORITY(prio) ((prio) <= HF_RMT_MAX_INTERRUPT_PRIORITY)

/**
 * @brief Get the recommended TX channel for the current ESP32 variant
 * @param index Channel index (0-based within available TX channels)
 * @return Actual channel number, or -1 if invalid
 */
inline constexpr int8_t HfRmtGetTxChannel(uint8_t index) noexcept {
  if (index >= HF_RMT_MAX_TX_CHANNELS) {
    return -1;
  }
  return static_cast<int8_t>(HF_RMT_TX_CHANNEL_START + index);
}

/**
 * @brief Get the recommended RX channel for the current ESP32 variant
 * @param index Channel index (0-based within available RX channels)
 * @return Actual channel number, or -1 if invalid
 */
inline constexpr int8_t HfRmtGetRxChannel(uint8_t index) noexcept {
  if (index >= HF_RMT_MAX_RX_CHANNELS) {
    return -1;
  }
  return static_cast<int8_t>(HF_RMT_RX_CHANNEL_START + index);
}

/**
 * @brief Validate channel for specific direction on current ESP32 variant
 * @param channel_id Channel number to validate
 * @param direction Direction (TX or RX)
 * @return true if channel is valid for the direction, false otherwise
 */
inline constexpr bool HfRmtIsChannelValidForDirection(uint8_t channel_id,
                                                      hf_pio_direction_t direction) noexcept {
  if (!HF_RMT_IS_VALID_CHANNEL(channel_id)) {
    return false;
  }

  switch (direction) {
    case hf_pio_direction_t::Transmit:
      return HF_RMT_IS_VALID_TX_CHANNEL(channel_id);
    case hf_pio_direction_t::Receive:
      return HF_RMT_IS_VALID_RX_CHANNEL(channel_id);
    case hf_pio_direction_t::Bidirectional:
      // Bidirectional requires both TX and RX capability
      return HF_RMT_IS_VALID_TX_CHANNEL(channel_id) && HF_RMT_IS_VALID_RX_CHANNEL(channel_id);
    default:
      return false;
  }
}

/**
 * @brief Get ESP32 variant name for debugging
 * @return String describing the current ESP32 variant
 */
inline constexpr const char* HfRmtGetVariantName() noexcept {
#if defined(CONFIG_IDF_TARGET_ESP32)
  return "ESP32";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
  return "ESP32-S2";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  return "ESP32-S3";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  return "ESP32-C3";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  return "ESP32-C6";
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
  return "ESP32-H2";
#else
  return "Unknown ESP32";
#endif
}

//==============================================================================
// END OF ESPPIO TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
