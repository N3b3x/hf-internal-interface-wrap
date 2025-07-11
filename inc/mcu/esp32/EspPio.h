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

#include "BasePio.h"
#include "EspTypes.h"
#include "RtosMutex.h"
#include <array>
// Include ESP-IDF RMT driver headers for direct type usage
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"

#ifdef __cplusplus
}
#endif

//==============================================================================
// TYPE ALIASES FOR PLATFORM COMPATIBILITY (from McuTypes.h)
//==============================================================================

// Forward declarations for PIO types
struct hf_pio_channel_statistics_t {
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
 * @brief ESP32C6 RMT-based Programmable IO Channel implementation with advanced ESP-IDF v5.5+
 * features.
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
  hf_pio_err_t Initialize() noexcept override;
  hf_pio_err_t Deinitialize() noexcept override;

  hf_pio_err_t ConfigureChannel(uint8_t channel_id,
                                const hf_pio_channel_config_t &config) noexcept override;

  hf_pio_err_t Transmit(uint8_t channel_id, const hf_pio_symbol_t *symbols, size_t symbol_count,
                        bool wait_completion = false) noexcept override;

  hf_pio_err_t StartReceive(uint8_t channel_id, hf_pio_symbol_t *buffer, size_t buffer_size,
                            uint32_t timeout_us = 0) noexcept override;
  hf_pio_err_t StopReceive(uint8_t channel_id, size_t &symbols_received) noexcept override;

  bool IsChannelBusy(uint8_t channel_id) const noexcept override;
  hf_pio_err_t GetChannelStatus(uint8_t channel_id,
                                hf_pio_channel_status_t &status) const noexcept override;
  hf_pio_err_t GetCapabilities(hf_pio_capabilities_t &capabilities) const noexcept override;

  void SetTransmitCallback(hf_pio_transmit_callback_t callback,
                           void *user_data = nullptr) noexcept override;
  void SetReceiveCallback(hf_pio_receive_callback_t callback,
                          void *user_data = nullptr) noexcept override;
  void SetErrorCallback(hf_pio_error_callback_t callback,
                        void *user_data = nullptr) noexcept override;
  void ClearCallbacks() noexcept override;

  /**
   * @brief Get PIO operation statistics.
   * @param statistics Reference to statistics structure to fill
   * @return hf_pio_err_t::PIO_SUCCESS if successful, error code otherwise
   */
  hf_pio_err_t GetStatistics(hf_pio_statistics_t &statistics) const noexcept override;

  /**
   * @brief Get PIO diagnostic information.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return hf_pio_err_t::PIO_SUCCESS if successful, error code otherwise
   */
  hf_pio_err_t GetDiagnostics(hf_pio_diagnostics_t &diagnostics) const noexcept override;

  //==============================================//
  // Lazy Initialization Support
  // Advanced Low-Level RMT Control Methods
  //==============================================//

  /**
   * @brief Transmit raw RMT symbols directly (bypassing hf_pio_symbol_t conversion)
   * @param channel_id Channel identifier
   * @param rmt_symbols Array of raw RMT symbols
   * @param symbol_count Number of RMT symbols
   * @param wait_completion If true, block until transmission is complete
   * @return Error code indicating success or failure
   * @note This provides direct RMT access similar to rmt_wrapper.hpp
   */
  hf_pio_err_t TransmitRawRmtSymbols(uint8_t channel_id, const rmt_symbol_word_t *rmt_symbols,
                                     size_t symbol_count, bool wait_completion = false) noexcept;

  /**
   * @brief Receive raw RMT symbols directly (bypassing hf_pio_symbol_t conversion)
   * @param channel_id Channel identifier
   * @param rmt_buffer Buffer to store raw RMT symbols
   * @param buffer_size Size of buffer in RMT symbols
   * @param symbols_received [out] Number of symbols actually received
   * @param timeout_us Timeout in microseconds
   * @return Error code indicating success or failure
   * @note This provides direct RMT access similar to rmt_wrapper.hpp
   */
  hf_pio_err_t ReceiveRawRmtSymbols(uint8_t channel_id, rmt_symbol_word_t *rmt_buffer,
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
  hf_pio_err_t ConfigureAdvancedRmt(uint8_t channel_id, size_t memory_blocks = 64,
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
  hf_pio_err_t ConfigureCarrier(uint8_t channel_id, uint32_t carrier_freq_hz,
                                float duty_cycle) noexcept;

  /**
   * @brief Enable/disable loopback mode for testing
   * @param channel_id Channel identifier
   * @param enable true to enable loopback, false to disable
   * @return Error code indicating success or failure
   */
  hf_pio_err_t EnableLoopback(uint8_t channel_id, bool enable) noexcept;

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
  hf_pio_err_t ConfigureEncoder(uint8_t channel_id, const hf_pio_symbol_t &bit0_config,
                                const hf_pio_symbol_t &bit1_config) noexcept;

  /**
   * @brief Set RMT channel idle output level
   * @param channel_id Channel identifier
   * @param idle_level true for high, false for low
   * @return Error code indicating success or failure
   */
  hf_pio_err_t SetIdleLevel(uint8_t channel_id, bool idle_level) noexcept;

  /**
   * @brief Get current RMT channel statistics
   * @param channel_id Channel identifier
   * @param stats [out] Channel statistics structure
   * @return Error code indicating success or failure
   */
  hf_pio_err_t GetChannelStatistics(uint8_t channel_id,
                                    hf_pio_channel_statistics_t &stats) const noexcept;

  /**
   * @brief Reset channel statistics counters
   * @param channel_id Channel identifier
   * @return Error code indicating success or failure
   */
  hf_pio_err_t ResetChannelStatistics(uint8_t channel_id) noexcept;

private:
  //==============================================//
  // Internal Structures
  //==============================================//

  struct ChannelState {
    bool configured;
    bool busy;
    hf_pio_channel_config_t config;
    hf_pio_channel_status_t status;

    // Use original ESP-IDF RMT types directly
    rmt_channel_handle_t tx_channel;
    rmt_channel_handle_t rx_channel;
    rmt_encoder_handle_t encoder;
    rmt_encoder_handle_t bytes_encoder; // For byte-level protocols

    // Buffers
    hf_pio_symbol_t *rx_buffer;
    size_t rx_buffer_size;
    size_t rx_symbols_received;

    // Timing
    uint64_t last_operation_time;
    
    // Idle level configuration
    bool idle_level;

    ChannelState() noexcept
        : configured(false), busy(false), config(), status(), tx_channel(nullptr),
          rx_channel(nullptr), encoder(nullptr), bytes_encoder(nullptr), rx_buffer(nullptr),
          rx_buffer_size(0), rx_symbols_received(0), last_operation_time(0), idle_level(false) {}
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
  hf_pio_transmit_callback_t transmit_callback_;
  hf_pio_receive_callback_t receive_callback_;
  hf_pio_error_callback_t error_callback_;
  void *callback_user_data_;

  //==============================================//
  // Internal Helper Methods
  //==============================================//

  /**
   * @brief Validate channel ID
   */
  bool IsValidChannelId(uint8_t channel_id) const noexcept;
  /**
   * @brief Convert hf_pio_symbol_t array to RMT symbol format
   */
  hf_pio_err_t ConvertToRmtSymbols(const hf_pio_symbol_t *symbols, size_t symbol_count,
                                   rmt_symbol_word_t *rmt_symbols,
                                   size_t &rmt_symbol_count) noexcept;

  /**
   * @brief Convert RMT symbols back to hf_pio_symbol_t format
   */
  hf_pio_err_t ConvertFromRmtSymbols(const rmt_symbol_word_t *rmt_symbols,
                                     size_t rmt_symbol_count, hf_pio_symbol_t *symbols,
                                     size_t &symbol_count) noexcept;

#ifdef HF_MCU_FAMILY_ESP32
  // Update callback signatures to match ESP-IDF v5.5 API
  static bool OnTransmitComplete(rmt_channel_handle_t channel,
                                const rmt_tx_done_event_data_t *edata, void *user_ctx);
  static bool OnReceiveComplete(rmt_channel_handle_t channel,
                               const rmt_rx_done_event_data_t *edata, void *user_ctx);
#endif

  /**
   * @brief Initialize a specific channel
   */
  hf_pio_err_t InitializeChannel(uint8_t channel_id) noexcept;

  /**
   * @brief Deinitialize a specific channel
   */
  hf_pio_err_t DeinitializeChannel(uint8_t channel_id) noexcept;

  /**
   * @brief Validate symbol array
   */
  hf_pio_err_t ValidateSymbols(const hf_pio_symbol_t *symbols, size_t symbol_count) const noexcept;

  /**
   * @brief Update channel status
   */
  void UpdateChannelStatus(uint8_t channel_id) noexcept;

  /**
   * @brief Invoke error callback if set
   */
  void InvokeErrorCallback(uint8_t channel_id, hf_pio_err_t error) noexcept;

  /**
   * @brief Calculate RMT clock divider for desired resolution
   */
  uint32_t CalculateClockDivider(uint32_t resolution_ns) const noexcept;

private:
  static constexpr const char *TAG = "McuPio"; ///< Logging tag
};
