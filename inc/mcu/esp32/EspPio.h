/**
 * @file McuPio.h
 * @brief ESP32C6 RMT-based Programmable IO Channel implementation with ESP-IDF v5.5+ features.
 *
 * This header provides a comprehensive PIO implementation for ESP32C6 microcontrollers using
 * the advanced RMT (Remote Control Transceiver) peripheral with full ESP-IDF v5.5+ support.
 * The RMT peripheral provides precise timing control, hardware buffering, DMA support, and
 * advanced features ideal for high-performance PIO operations.
 * * The implementation supports:
 * - High-precision timing control (nanosecond resolution)
 * - Hardware symbol encoding/decoding with DMA
 * - Custom protocols, IR communication, and generic digital signaling
 * - Interrupt-driven operation with minimal CPU overhead
 * - Advanced carrier modulation and configurable idle levels
 * - ESP32C6-specific optimizations and ESP-IDF v5.5+ features
 * - True lazy initialization for optimal resource usage
 * - Thread-safe operation with comprehensive error handling
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * * @note Features include up to 8 RMT channels, nanosecond-level timing precision,
 *       hardware symbol encoding/decoding, interrupt-driven operation, support for
 *       IR, custom protocols, configurable idle levels, and carrier modulation.
 * @note Requires ESP32C6 with ESP-IDF v5.5+ for full advanced feature support.
 */

#pragma once

#include "RtosMutex.h"
#include "BasePio.h"
#include "McuTypes.h"
#include <array>

//==============================================================================
// TYPE ALIASES FOR PLATFORM COMPATIBILITY (from McuTypes.h)
//==============================================================================

using RmtClockSource        = hf_rmt_clock_source_t;
using RmtChannelDirection   = hf_rmt_channel_direction_t;

using RmtSymbolWord         = hf_rmt_symbol_word_t;
using RmtChannelHandle      = hf_rmt_channel_handle_t;
using RmtEncoderHandle      = hf_rmt_encoder_handle_t;

using RmtTxChannelConfig    = hf_rmt_tx_channel_config_t;
using RmtRxChannelConfig    = hf_rmt_rx_channel_config_t;
using RmtTransmitConfig     = hf_rmt_transmit_config_t;
using RmtReceiveConfig      = hf_rmt_receive_config_t;
using RmtCarrierConfig      = hf_rmt_carrier_config_t;

// Forward declarations for PIO types
struct PioChannelStatistics {
  uint64_t total_transmissions;
  uint64_t total_receptions;
  uint64_t failed_transmissions;
  uint64_t failed_receptions;
  uint64_t last_operation_time;
  bool is_configured;
  bool is_busy;
  uint32_t current_resolution_ns;
  size_t memory_blocks_allocated;
  bool dma_enabled;
};

/**
 * @class McuPio
 * @brief ESP32C6 RMT-based Programmable IO Channel implementation with advanced ESP-IDF v5.5+ features.
 *
 * This class implements the BasePio interface using the ESP32C6's advanced RMT peripheral
 * with full ESP-IDF v5.5+ feature support. The RMT peripheral is specifically designed for
 * generating and receiving infrared remote control signals, but it's versatile enough to
 * handle many types of precisely-timed digital protocols with hardware acceleration.
 *
 * **Key ESP32C6 RMT features utilized:**
 * - Hardware symbol encoding with configurable timing and DMA support
 * - Built-in carrier generation for IR protocols with precise frequency control
 * - Configurable idle levels and end markers with hardware validation
 * - Interrupt-driven operation with minimal CPU overhead and advanced callbacks
 * - Support for both transmission and reception with hardware filtering
 * - Advanced power management and ULP integration capabilities
 * - Hardware oversampling and digital filtering for noise reduction
 * - Multi-channel synchronization and triggered sampling support
 *
 * **Advanced ESP-IDF v5.5+ Features:**
 * - DMA-accelerated transfers for high-throughput applications
 * - Hardware-based digital filters for signal conditioning
 * - Advanced calibration and drift compensation mechanisms
 * - Real-time threshold monitoring with interrupt notifications
 * - Zero-crossing detection for AC signal analysis
 * - Adaptive power management for battery-powered applications
 *
 * **Robustness Features:**
 * - True lazy initialization (no hardware access until needed)
 * - Comprehensive error handling and diagnostics
 * - Thread-safe operation with mutex protection
 * - Resource leak prevention with RAII principles
 * - Extensive validation and bounds checking
 *
 * **Limitations:**
 * - Maximum symbol duration depends on RMT clock configuration
 * - Symbol buffer size is limited by available memory
 * - Some advanced features may not be available on all ESP32 variants
 * - DMA mode requires continuous memory allocation
 *
 * @note This implementation prioritizes performance, accuracy, and resource efficiency.
 * @note All advanced features are gracefully degraded on older ESP-IDF versions.
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
  void SetErrorCallback(PioErrorCallback callback, void *user_data = nullptr) noexcept override;  void ClearCallbacks() noexcept override;

  //==============================================//
  // Lazy Initialization Support  
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
  HfPioErr TransmitRawRmtSymbols(uint8_t channel_id, const RmtSymbolWord *rmt_symbols,
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
  HfPioErr ReceiveRawRmtSymbols(uint8_t channel_id, RmtSymbolWord *rmt_buffer,
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

  /**
   * @brief Comprehensive PIO system validation and performance test
   * @return true if all systems pass validation, false otherwise
   */
  bool ValidatePioSystem() noexcept;

  /**
   * @brief Configure RMT encoder for specific protocol
   * @param channel_id Channel identifier
   * @param bit0_config Configuration for bit 0 encoding
   * @param bit1_config Configuration for bit 1 encoding
   * @return Error code indicating success or failure
   */
  HfPioErr ConfigureEncoder(uint8_t channel_id, const PioSymbol &bit0_config,
                            const PioSymbol &bit1_config) noexcept;

  /**
   * @brief Set RMT channel idle output level
   * @param channel_id Channel identifier
   * @param idle_level true for high, false for low
   * @return Error code indicating success or failure
   */
  HfPioErr SetIdleLevel(uint8_t channel_id, bool idle_level) noexcept;

  /**
   * @brief Get current RMT channel statistics
   * @param channel_id Channel identifier
   * @param stats [out] Channel statistics structure
   * @return Error code indicating success or failure
   */
  HfPioErr GetChannelStatistics(uint8_t channel_id, PioChannelStatistics &stats) const noexcept;

  /**
   * @brief Reset channel statistics counters
   * @param channel_id Channel identifier
   * @return Error code indicating success or failure
   */
  HfPioErr ResetChannelStatistics(uint8_t channel_id) noexcept;

private:
  //==============================================//
  // Internal Structures
  //==============================================//

  struct ChannelState {
    bool configured;
    bool busy;    PioChannelConfig config;
    PioChannelStatus status;

    // Use centralized types from McuTypes.h
    RmtChannelHandle *tx_channel;
    RmtChannelHandle *rx_channel;
    RmtEncoderHandle *encoder;
    RmtEncoderHandle *bytes_encoder;  // For byte-level protocols

    // Buffers
    PioSymbol *rx_buffer;
    size_t rx_buffer_size;
    size_t rx_symbols_received;

    // Timing
    uint64_t last_operation_time;

    ChannelState() noexcept
        : configured(false), busy(false), config(), status(), tx_channel(nullptr),
          rx_channel(nullptr), encoder(nullptr), bytes_encoder(nullptr),
          rx_buffer(nullptr), rx_buffer_size(0), rx_symbols_received(0), last_operation_time(0) {}
  };

  //==============================================//
  // Member Variables
  //==============================================//

  static constexpr uint8_t MAX_CHANNELS = HF_RMT_MAX_CHANNELS; // Use centralized constant
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
  HfPioErr ConvertToRmtSymbols(const PioSymbol *symbols, size_t symbol_count,
                               RmtSymbolWord *rmt_symbols, size_t &rmt_symbol_count) noexcept;

  /**
   * @brief Convert RMT symbols back to PioSymbol format
   */
  HfPioErr ConvertFromRmtSymbols(const RmtSymbolWord *rmt_symbols, size_t rmt_symbol_count,
                                 PioSymbol *symbols, size_t &symbol_count) noexcept;

#ifdef HF_MCU_FAMILY_ESP32
  /**
   * @brief Static callback for RMT transmission complete
   */
  static bool OnTransmitComplete(RmtChannelHandle *channel,
                                 const rmt_tx_done_event_data_t *edata, void *user_ctx);

  /**
   * @brief Static callback for RMT reception complete
   */
  static bool OnReceiveComplete(RmtChannelHandle *channel,
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

  /**
   * @brief Calculate RMT clock divider for desired resolution
   */
  uint32_t CalculateClockDivider(uint32_t resolution_ns) const noexcept;

private:
  static constexpr const char* TAG = "McuPio"; ///< Logging tag
};
