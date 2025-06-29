/**
 * @file McuPio.h
 * @brief ESP32 RMT-based Programmable IO Channel implementation.
 *
 * This header provides a PIO implementation for ESP32 microcontrollers using
 * the RMT (Remote Control Transceiver) peripheral. The RMT peripheral provides
 * precise timing control and hardware buffering ideal for PIO operations.
 * The implementation supports custom protocols, LED strips, IR communication,
 * and other timing-critical applications with nanosecond-level precision.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Features include up to 8 RMT channels, nanosecond-level timing precision,
 *       hardware symbol encoding/decoding, interrupt-driven operation, support for
 *       WS2812, IR, and custom protocols, and configurable idle levels with carrier modulation.
 */

#pragma once

#include "../utils/RtosMutex.h"
#include "BasePio.h"
#include "McuTypes.h"
#include <array>

// Forward declarations for ESP32 RMT types
#ifdef HF_MCU_FAMILY_ESP32
#include "driver/rmt_encoder.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#else
// For non-ESP32 platforms, we'll provide stub types
struct rmt_channel_handle_t;
struct rmt_encoder_handle_t;
struct rmt_symbol_word_t;
#endif

/**
 * @class McuPio
 * @brief ESP32 RMT-based Programmable IO Channel implementation.
 *
 * This class implements the BasePio interface using the ESP32's RMT peripheral.
 * The RMT peripheral is specifically designed for generating and receiving
 * infrared remote control signals, but it's versatile enough to handle many
 * types of precisely-timed digital protocols.
 *
 * Key ESP32 RMT features utilized:
 * - Hardware symbol encoding with configurable timing
 * - Built-in carrier generation for IR protocols
 * - Configurable idle levels and end markers
 * - Interrupt-driven operation with minimal CPU overhead
 * - Support for both transmission and reception
 *
 * Limitations:
 * - Maximum symbol duration depends on RMT clock configuration
 * - Symbol buffer size is limited by available memory
 * - Some advanced features may not be available on all ESP32 variants
 */
class McuPio : public BasePio {
public:
  /**
   * @brief Constructor
   */
  McuPio() noexcept;

  /**
   * @brief Destructor
   */
  ~McuPio() noexcept override;

  // Disable copy constructor and assignment operator
  McuPio(const McuPio &) = delete;
  McuPio &operator=(const McuPio &) = delete;

  // Allow move operations
  McuPio(McuPio &&) noexcept = default;
  McuPio &operator=(McuPio &&) noexcept = default;

  //==============================================//
  // BasePio Interface Implementation
  //==============================================//

  HfPioErr Initialize() noexcept override;
  HfPioErr Deinitialize() noexcept override;
  bool IsInitialized() const noexcept override;

  HfPioErr ConfigureChannel(uint8_t channel_id, const PioChannelConfig &config) noexcept override;

  HfPioErr Transmit(uint8_t channel_id, const PioSymbol *symbols, size_t symbol_count,
                    bool wait_completion = false) noexcept override;

  HfPioErr StartReceive(uint8_t channel_id, PioSymbol *buffer, size_t buffer_size,
                        uint32_t timeout_us = 0) noexcept override;
  HfPioErr StopReceive(uint8_t channel_id, size_t &symbols_received) noexcept override;

  bool IsChannelBusy(uint8_t channel_id) const noexcept override;
  HfPioErr GetChannelStatus(uint8_t channel_id, PioChannelStatus &status) const noexcept override;
  HfPioErr GetCapabilities(PioCapabilities &capabilities) const noexcept override;

  void SetTransmitCallback(PioTransmitCallback callback,
                           void *user_data = nullptr) noexcept override;
  void SetReceiveCallback(PioReceiveCallback callback, void *user_data = nullptr) noexcept override;
  void SetErrorCallback(PioErrorCallback callback, void *user_data = nullptr) noexcept override;
  void ClearCallbacks() noexcept override;

  //==============================================//
  // Advanced Low-Level RMT Control Methods
  //==============================================//

  /**
   * @brief Transmit raw RMT symbols directly (bypassing PioSymbol conversion)
   * @param channel_id Channel identifier
   * @param rmt_symbols Array of raw RMT symbols
   * @param symbol_count Number of RMT symbols
   * @param wait_completion If true, block until transmission is complete
   * @return Error code indicating success or failure
   * @note This provides direct RMT access similar to rmt_wrapper.hpp
   */
  HfPioErr TransmitRawRmtSymbols(uint8_t channel_id, const rmt_symbol_word_t *rmt_symbols,
                                 size_t symbol_count, bool wait_completion = false) noexcept;

  /**
   * @brief Receive raw RMT symbols directly (bypassing PioSymbol conversion)
   * @param channel_id Channel identifier
   * @param rmt_buffer Buffer to store raw RMT symbols
   * @param buffer_size Size of buffer in RMT symbols
   * @param symbols_received [out] Number of symbols actually received
   * @param timeout_us Timeout in microseconds
   * @return Error code indicating success or failure
   * @note This provides direct RMT access similar to rmt_wrapper.hpp
   */
  HfPioErr ReceiveRawRmtSymbols(uint8_t channel_id, rmt_symbol_word_t *rmt_buffer,
                                size_t buffer_size, size_t &symbols_received,
                                uint32_t timeout_us = 10000) noexcept;

  /**
   * @brief Configure advanced RMT channel settings
   * @param channel_id Channel identifier
   * @param memory_blocks Number of memory blocks (symbols) to allocate
   * @param enable_dma Enable DMA mode for large transfers
   * @param queue_depth Transmit queue depth
   * @return Error code indicating success or failure
   */
  HfPioErr ConfigureAdvancedRmt(uint8_t channel_id, size_t memory_blocks = 64,
                                bool enable_dma = false, uint32_t queue_depth = 4) noexcept;

  /**
   * @brief Create WS2812-optimized encoder with configurable timing
   * @param channel_id Channel identifier
   * @param resolution_hz RMT resolution for timing calculations
   * @param t0h_ns High time for '0' bit in nanoseconds (default: 400ns)
   * @param t0l_ns Low time for '0' bit in nanoseconds (default: 850ns)
   * @param t1h_ns High time for '1' bit in nanoseconds (default: 800ns)
   * @param t1l_ns Low time for '1' bit in nanoseconds (default: 450ns)
   * @return Error code indicating success or failure
   */
  HfPioErr CreateWS2812Encoder(uint8_t channel_id, uint32_t resolution_hz = 10000000,
                               uint32_t t0h_ns = 400, uint32_t t0l_ns = 850, uint32_t t1h_ns = 800,
                               uint32_t t1l_ns = 450) noexcept;

  /**
   * @brief Transmit WS2812/NeoPixel data using optimized encoder
   * @param channel_id Channel identifier
   * @param grb_data GRB pixel data (Green, Red, Blue format)
   * @param length Number of bytes
   * @param wait_completion If true, block until transmission is complete
   * @return Error code indicating success or failure
   */
  HfPioErr TransmitWS2812(uint8_t channel_id, const uint8_t *grb_data, size_t length,
                          bool wait_completion = false) noexcept;

  //==============================================//
  // ESP32-Specific Methods (continued)
  //==============================================//

  /**
   * @brief Configure carrier modulation for IR protocols
   * @param channel_id Channel identifier
   * @param carrier_freq_hz Carrier frequency in Hz (0 to disable)
   * @param duty_cycle Carrier duty cycle (0.0 to 1.0)
   * @return Error code indicating success or failure
   */
  HfPioErr ConfigureCarrier(uint8_t channel_id, uint32_t carrier_freq_hz,
                            float duty_cycle) noexcept;

  /**
   * @brief Enable/disable loopback mode for testing
   * @param channel_id Channel identifier
   * @param enable true to enable loopback, false to disable
   * @return Error code indicating success or failure
   */
  HfPioErr EnableLoopback(uint8_t channel_id, bool enable) noexcept;

  /**
   * @brief Get the maximum number of symbols that can be transmitted in one operation
   * @return Maximum symbol count
   */
  size_t GetMaxSymbolCount() const noexcept;

private:
  //==============================================//
  // Internal Structures
  //==============================================//

  struct ChannelState {
    bool configured;
    bool busy;
    PioChannelConfig config;
    PioChannelStatus status;

#ifdef HF_MCU_FAMILY_ESP32
    rmt_channel_handle_t *tx_channel;
    rmt_channel_handle_t *rx_channel;
    rmt_encoder_handle_t *encoder;
    rmt_encoder_handle_t *bytes_encoder;  // For byte-level protocols
    rmt_encoder_handle_t *ws2812_encoder; // For WS2812/NeoPixel
#else
    void *tx_channel;
    void *rx_channel;
    void *encoder;
    void *bytes_encoder;
    void *ws2812_encoder;
#endif

    // Buffers
    PioSymbol *rx_buffer;
    size_t rx_buffer_size;
    size_t rx_symbols_received;

    // Timing
    uint64_t last_operation_time;

    ChannelState() noexcept
        : configured(false), busy(false), config(), status(), tx_channel(nullptr),
          rx_channel(nullptr), encoder(nullptr), bytes_encoder(nullptr), ws2812_encoder(nullptr),
          rx_buffer(nullptr), rx_buffer_size(0), rx_symbols_received(0), last_operation_time(0) {}
  };

  //==============================================//
  // Member Variables
  //==============================================//

  static constexpr uint8_t MAX_CHANNELS = 8; // ESP32 RMT has up to 8 channels
  static constexpr size_t MAX_SYMBOLS_PER_TRANSMISSION = 64;
  static constexpr uint32_t DEFAULT_RESOLUTION_NS = 1000; // 1 microsecond
  static constexpr uint32_t RMT_CLK_SRC_FREQ = 80000000;  // 80 MHz APB clock

  bool initialized_;
  std::array<ChannelState, MAX_CHANNELS> channels_;
  mutable RtosMutex state_mutex_;

  // Callbacks
  PioTransmitCallback transmit_callback_;
  PioReceiveCallback receive_callback_;
  PioErrorCallback error_callback_;
  void *callback_user_data_;

  //==============================================//
  // Internal Helper Methods
  //==============================================//

  /**
   * @brief Validate channel ID
   */
  bool IsValidChannelId(uint8_t channel_id) const noexcept;

  /**
   * @brief Convert PioSymbol array to RMT symbol format
   */
#ifdef HF_MCU_FAMILY_ESP32
  HfPioErr ConvertToRmtSymbols(const PioSymbol *symbols, size_t symbol_count,
                               rmt_symbol_word_t *rmt_symbols, size_t &rmt_symbol_count) noexcept;

  /**
   * @brief Convert RMT symbols back to PioSymbol format
   */
  HfPioErr ConvertFromRmtSymbols(const rmt_symbol_word_t *rmt_symbols, size_t rmt_symbol_count,
                                 PioSymbol *symbols, size_t &symbol_count) noexcept;

  /**
   * @brief Calculate RMT clock divider for desired resolution
   */
  uint32_t CalculateClockDivider(uint32_t resolution_ns) const noexcept;

  /**
   * @brief Static callback for RMT transmission complete
   */
  static bool OnTransmitComplete(rmt_channel_handle_t *channel,
                                 const rmt_tx_done_event_data_t *edata, void *user_ctx);

  /**
   * @brief Static callback for RMT reception complete
   */
  static bool OnReceiveComplete(rmt_channel_handle_t *channel,
                                const rmt_rx_done_event_data_t *edata, void *user_ctx);
#endif

  /**
   * @brief Initialize a specific channel
   */
  HfPioErr InitializeChannel(uint8_t channel_id) noexcept;

  /**
   * @brief Deinitialize a specific channel
   */
  HfPioErr DeinitializeChannel(uint8_t channel_id) noexcept;

  /**
   * @brief Validate symbol array
   */
  HfPioErr ValidateSymbols(const PioSymbol *symbols, size_t symbol_count) const noexcept;

  /**
   * @brief Update channel status
   */
  void UpdateChannelStatus(uint8_t channel_id) noexcept;

  /**
   * @brief Invoke error callback if set
   */
  void InvokeErrorCallback(uint8_t channel_id, HfPioErr error) noexcept;
};
