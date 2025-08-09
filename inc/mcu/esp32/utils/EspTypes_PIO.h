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

// ESP32-C6 specific RMT channel limits
#if defined(CONFIG_IDF_TARGET_ESP32C6)
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 2;      // ESP32-C6 has 2 RMT channels
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;   // Both can be used for TX
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;   // Both can be used for RX
#else
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;      // Other ESP32 variants have 4 channels
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;   // 2 TX channels
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;   // 2 RX channels
#endif
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024;
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64;
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000;
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000;
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
 * @brief RMT validation macros for ESP32.
 */
#define HF_RMT_IS_VALID_CHANNEL(ch) ((ch) < HF_RMT_MAX_CHANNELS)
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) < HF_RMT_MAX_TX_CHANNELS)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) < HF_RMT_MAX_RX_CHANNELS)
#define HF_RMT_IS_VALID_RESOLUTION(res) \
  ((res) >= HF_RMT_MIN_RESOLUTION_HZ && (res) <= HF_RMT_MAX_RESOLUTION_HZ)
#define HF_RMT_IS_VALID_MEM_BLOCK_SIZE(size) \
  ((size) >= HF_RMT_MIN_MEM_BLOCK_SYMBOLS && (size) <= HF_RMT_MAX_MEM_BLOCK_SYMBOLS)
#define HF_RMT_IS_VALID_QUEUE_DEPTH(depth) ((depth) >= 1 && (depth) <= HF_RMT_MAX_QUEUE_DEPTH)
#define HF_RMT_IS_VALID_INTR_PRIORITY(prio) ((prio) <= HF_RMT_MAX_INTERRUPT_PRIORITY)

//==============================================================================
// END OF ESPPIO TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
