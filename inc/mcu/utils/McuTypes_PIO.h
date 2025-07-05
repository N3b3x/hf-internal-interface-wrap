/**
 * @file McuTypes_RMT.h
 * @brief MCU-specific RMT (Remote Control) type definitions for hardware abstraction.
 *
 * This header defines all RMT-specific types and constants that are used
 * throughout the internal interface wrap layer for RMT operations. This includes
 * ESP32C6 RMT controller support with ESP-IDF v5.5+ features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"    // Central MCU platform selection (includes all ESP-IDF)
#include "McuTypes_Base.h"
#include "BasePio.h" // For HfRmtErr

//==============================================================================
// ESP32C6 RMT (REMOTE CONTROL) PERIPHERAL TYPES - ESP-IDF v5.5+ FEATURES
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6

// Include RMT-specific headers for ESP32C6
#include "driver/rmt_encoder.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "hal/rmt_types.h"
#include "soc/soc_caps.h"

/**
 * @brief ESP32C6 RMT controller specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 4 RMT channels (0-3) with advanced features:
 * - TX/RX channels can be independently configured
 * - Hardware symbol encoding with configurable timing
 * - DMA support for large transfers (>64 symbols) 
 * - Multiple clock sources (APB, XTAL, RC_FAST)
 * - Power management with light sleep support
 * - Flexible memory allocation (48-1024 symbols per channel)
 * - Interrupt priority configuration
 * - Carrier modulation for IR protocols
 */
static constexpr uint8_t HF_RMT_MAX_CHANNELS = SOC_RMT_CHANNELS_PER_GROUP;  // 4 for ESP32C6
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = SOC_RMT_TX_CANDIDATES_PER_GROUP; // 2 for ESP32C6
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = SOC_RMT_RX_CANDIDATES_PER_GROUP; // 2 for ESP32C6
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;   // Minimum memory block size
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024; // Maximum for DMA mode
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64; // Default allocation
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000; // 80MHz APB clock
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;     // 1kHz minimum
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000; // 1MHz default
static constexpr uint8_t HF_RMT_MAX_QUEUE_DEPTH = 32;         // Maximum TX queue depth
static constexpr uint8_t HF_RMT_MAX_INTERRUPT_PRIORITY = 7;    // Maximum interrupt priority

/**
 * @brief RMT clock source selection for ESP32C6.
 * @details Maps to ESP-IDF v5.5+ rmt_clock_source_t enum.
 */
enum class hf_rmt_clock_source_t : uint8_t {
  HF_RMT_CLK_SRC_DEFAULT = 0,    ///< Default clock source (APB)
  HF_RMT_CLK_SRC_APB = 1,        ///< APB clock (80MHz)
  HF_RMT_CLK_SRC_XTAL = 2,       ///< Crystal clock (40MHz) 
  HF_RMT_CLK_SRC_RC_FAST = 3,    ///< RC fast clock (~8MHz)
};

/**
 * @brief RMT channel direction for ESP32C6.
 */
enum class hf_rmt_channel_direction_t : uint8_t {
  HF_RMT_CHANNEL_DIRECTION_TX = 0,  ///< Transmit direction
  HF_RMT_CHANNEL_DIRECTION_RX = 1,  ///< Receive direction
};

/**
 * @brief RMT symbol word structure - platform-specific.
 */
using hf_rmt_symbol_word_t = rmt_symbol_word_t;
using hf_rmt_channel_handle_t = rmt_channel_handle_t;
using hf_rmt_encoder_handle_t = rmt_encoder_handle_t;
using hf_rmt_tx_channel_config_t = rmt_tx_channel_config_t;
using hf_rmt_rx_channel_config_t = rmt_rx_channel_config_t;

/**
 * @brief RMT transmission configuration with ESP32C6 advanced features.
 */
struct hf_rmt_transmit_config_t {
  uint32_t loop_count;          ///< Loop count (0 = no loop)
  bool invert_signal;           ///< Invert output signal
  bool with_dma;                ///< Enable DMA mode for large transfers
  uint32_t queue_depth;         ///< TX queue depth (1-32)
  uint8_t intr_priority;        ///< Interrupt priority (0-7)
  bool allow_pd;                ///< Allow power down in sleep modes
  
  hf_rmt_transmit_config_t() noexcept
      : loop_count(0), invert_signal(false), with_dma(false),
        queue_depth(4), intr_priority(0), allow_pd(false) {}
};

/**
 * @brief RMT reception configuration with ESP32C6 advanced features.
 */
struct hf_rmt_receive_config_t {
  uint32_t signal_range_min_ns; ///< Minimum signal range in nanoseconds
  uint32_t signal_range_max_ns; ///< Maximum signal range in nanoseconds
  bool with_dma;                ///< Enable DMA mode for large transfers
  uint8_t intr_priority;        ///< Interrupt priority (0-7)
  bool allow_pd;                ///< Allow power down in sleep modes
  
  hf_rmt_receive_config_t() noexcept
      : signal_range_min_ns(1000), signal_range_max_ns(1000000),
        with_dma(false), intr_priority(0), allow_pd(false) {}
};

/**
 * @brief RMT carrier configuration for IR protocols.
 */
struct hf_rmt_carrier_config_t {
  uint32_t frequency_hz;    ///< Carrier frequency in Hz
  float duty_cycle;         ///< Duty cycle (0.0 to 1.0)
  uint8_t polarity_active_low; ///< Carrier polarity (0=high, 1=low)
  bool always_on;           ///< Always on carrier mode
  
  hf_rmt_carrier_config_t() noexcept
      : frequency_hz(38000), duty_cycle(0.5f),
        polarity_active_low(0), always_on(false) {}
};

#else
// Non-ESP32C6 platforms - use generic types
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024;
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64;
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000;
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000;
static constexpr uint8_t HF_RMT_MAX_QUEUE_DEPTH = 32;
static constexpr uint8_t HF_RMT_MAX_INTERRUPT_PRIORITY = 7;

enum class hf_rmt_clock_source_t : uint8_t {
  HF_RMT_CLK_SRC_DEFAULT = 0,
  HF_RMT_CLK_SRC_APB = 1,
  HF_RMT_CLK_SRC_XTAL = 2,
  HF_RMT_CLK_SRC_RC_FAST = 3,
};

enum class hf_rmt_channel_direction_t : uint8_t {
  HF_RMT_CHANNEL_DIRECTION_TX = 0,
  HF_RMT_CHANNEL_DIRECTION_RX = 1,
};

// Generic structures for non-ESP32C6 platforms
struct hf_rmt_symbol_word_t {
  uint32_t level0 : 1;
  uint32_t duration0 : 15;
  uint32_t level1 : 1;
  uint32_t duration1 : 15;
};

using hf_rmt_channel_handle_t = void*;
using hf_rmt_encoder_handle_t = void*;

struct hf_rmt_tx_channel_config_t {
  int dummy;
};

struct hf_rmt_rx_channel_config_t {
  int dummy;
};

struct hf_rmt_transmit_config_t {
  uint32_t loop_count;
  bool invert_signal;
  bool with_dma;
  uint32_t queue_depth;
  uint8_t intr_priority;
  bool allow_pd;
  
  hf_rmt_transmit_config_t() noexcept
      : loop_count(0), invert_signal(false), with_dma(false),
        queue_depth(4), intr_priority(0), allow_pd(false) {}
};

struct hf_rmt_receive_config_t {
  uint32_t signal_range_min_ns;
  uint32_t signal_range_max_ns;
  bool with_dma;
  uint8_t intr_priority;
  bool allow_pd;
  
  hf_rmt_receive_config_t() noexcept
      : signal_range_min_ns(1000), signal_range_max_ns(1000000),
        with_dma(false), intr_priority(0), allow_pd(false) {}
};

struct hf_rmt_carrier_config_t {
  uint32_t frequency_hz;
  float duty_cycle;
  uint8_t polarity_active_low;
  bool always_on;
  
  hf_rmt_carrier_config_t() noexcept
      : frequency_hz(38000), duty_cycle(0.5f),
        polarity_active_low(0), always_on(false) {}
};
#endif

//==============================================================================
// RMT VALIDATION MACROS
//==============================================================================

/**
 * @brief RMT validation macros for ESP32C6.
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
