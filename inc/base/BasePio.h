/**
 * @file BasePio.h
 * @ingroup pio
 * @brief Abstract base class for Programmable IO Channel implementations in the HardFOC system.
 *
 * This header defines the abstract base class for precise, buffered digital signal I/O
 * that provides a consistent API across different PIO implementations.
 * Concrete implementations for various microcontrollers inherit from this class.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as
 * BaseCan/BaseAdc/BasePwm.
 * @note Users should program against this interface, not specific implementations.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

/**
 * @defgroup pio PIO Module
 * @brief All PIO-related types, enums, and functions for Programmable I/O operations.
 */

//--------------------------------------
//  HardFOC PIO Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC PIO error codes
 * @ingroup pio
 * @details Comprehensive error enumeration for all PIO operations in the system.
 *          This enumeration is used across all PIO-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_PIO_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(PIO_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(PIO_ERR_FAILURE, 1, "General failure")                                                         \
  X(PIO_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(PIO_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(PIO_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(PIO_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(PIO_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Channel errors */                                                                             \
  X(PIO_ERR_INVALID_CHANNEL, 7, "Invalid PIO channel")                                             \
  X(PIO_ERR_CHANNEL_BUSY, 8, "Channel already in use")                                             \
  X(PIO_ERR_CHANNEL_NOT_AVAILABLE, 9, "Channel not available")                                     \
  X(PIO_ERR_INSUFFICIENT_CHANNELS, 10, "Insufficient channels available")                          \
  /* Timing errors */                                                                              \
  X(PIO_ERR_INVALID_RESOLUTION, 11, "Invalid time resolution")                                     \
  X(PIO_ERR_RESOLUTION_TOO_HIGH, 12, "Time resolution too high")                                   \
  X(PIO_ERR_RESOLUTION_TOO_LOW, 13, "Time resolution too low")                                     \
  X(PIO_ERR_DURATION_TOO_LONG, 14, "Duration too long")                                            \
  X(PIO_ERR_DURATION_TOO_SHORT, 15, "Duration too short")                                          \
  /* Buffer errors */                                                                              \
  X(PIO_ERR_BUFFER_OVERFLOW, 16, "Buffer overflow")                                                \
  X(PIO_ERR_BUFFER_UNDERFLOW, 17, "Buffer underflow")                                              \
  X(PIO_ERR_BUFFER_TOO_SMALL, 18, "Buffer too small")                                              \
  X(PIO_ERR_BUFFER_TOO_LARGE, 19, "Buffer too large")                                              \
  /* Hardware errors */                                                                            \
  X(PIO_ERR_HARDWARE_FAULT, 20, "Hardware fault")                                                  \
  X(PIO_ERR_COMMUNICATION_TIMEOUT, 21, "Communication timeout")                                    \
  X(PIO_ERR_COMMUNICATION_FAILURE, 22, "Communication failure")                                    \
  X(PIO_ERR_DEVICE_NOT_RESPONDING, 23, "Device not responding")                                    \
  /* Configuration errors */                                                                       \
  X(PIO_ERR_INVALID_CONFIGURATION, 24, "Invalid configuration")                                    \
  X(PIO_ERR_UNSUPPORTED_OPERATION, 25, "Unsupported operation")                                    \
  X(PIO_ERR_PIN_CONFLICT, 26, "Pin already in use")                                                \
  X(PIO_ERR_RESOURCE_BUSY, 27, "Resource busy")                                                    \
  /* System errors */                                                                              \
  X(PIO_ERR_SYSTEM_ERROR, 28, "System error")                                                      \
  X(PIO_ERR_PERMISSION_DENIED, 29, "Permission denied")                                            \
  X(PIO_ERR_OPERATION_ABORTED, 30, "Operation aborted")                                            \
  X(PIO_ERR_UNKNOWN, 31, "Unknown error")

enum class hf_pio_err_t : hf_u8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_PIO_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_pio_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfPioErrToString(hf_pio_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_pio_err_t::NAME:                                                                         \
    return DESC;
    HF_PIO_ERR_LIST(X)
#undef X
  default:
    return HfPioErrToString(hf_pio_err_t::PIO_ERR_UNKNOWN);
  }
}

//--------------------------------------
//  PIO Configuration Structures
//--------------------------------------

/**
 * @brief PIO channel direction
 */
enum class hf_pio_direction_t : hf_u8_t {
  Transmit = 0,     ///< Transmit mode (output)
  Receive = 1,      ///< Receive mode (input)
  Bidirectional = 2 ///< Bidirectional mode (if supported)
};

/**
 * @brief PIO signal polarity
 */
enum class hf_pio_polarity_t : hf_u8_t {
  Normal = 0,  ///< Normal polarity (idle low, active high)
  Inverted = 1 ///< Inverted polarity (idle high, active low)
};

/**
 * @brief PIO idle state
 */
enum class hf_pio_idle_state_t : hf_u8_t {
  Low = 0, ///< Idle state is low
  High = 1 ///< Idle state is high
};

/**
 * @brief PIO channel configuration structure
 */
struct hf_pio_channel_config_t {
  hf_pin_num_t gpio_pin;          ///< GPIO pin for PIO signal
  hf_pio_direction_t direction;   ///< Channel direction
  hf_u32_t resolution_ns;         ///< Time resolution in nanoseconds (user-friendly interface)
  hf_pio_polarity_t polarity;     ///< Signal polarity
  hf_pio_idle_state_t idle_state; ///< Idle state
  hf_u32_t timeout_us;            ///< Operation timeout in microseconds
  size_t buffer_size;             ///< Buffer size for symbols/durations

  hf_pio_channel_config_t() noexcept
      : gpio_pin(-1), direction(hf_pio_direction_t::Transmit), resolution_ns(1000), // 1µs default
        polarity(hf_pio_polarity_t::Normal), idle_state(hf_pio_idle_state_t::Low),
        timeout_us(10000), buffer_size(64) {}
};

/**
 * @brief PIO symbol structure for precise timing
 */
struct hf_pio_symbol_t {
  hf_u32_t duration; ///< Duration in resolution units
  bool level;        ///< Signal level (true = high, false = low)

  hf_pio_symbol_t() noexcept : duration(0), level(false) {}
  hf_pio_symbol_t(hf_u32_t dur, bool lvl) noexcept : duration(dur), level(lvl) {}
};

/**
 * @brief PIO channel status information
 */
struct hf_pio_channel_status_t {
  bool is_initialized;      ///< Channel is initialized
  bool is_busy;             ///< Channel is currently busy
  bool is_transmitting;     ///< Channel is transmitting
  bool is_receiving;        ///< Channel is receiving
  size_t symbols_queued;    ///< Number of symbols in queue
  size_t symbols_processed; ///< Number of symbols processed
  hf_pio_err_t last_error;  ///< Last error that occurred
  hf_u32_t timestamp_us;    ///< Timestamp of last operation
};

/**
 * @brief PIO capability information
 */
struct hf_pio_capabilities_t {
  hf_u8_t max_channels;        ///< Maximum number of channels
  hf_u32_t min_resolution_ns;  ///< Minimum time resolution
  hf_u32_t max_resolution_ns;  ///< Maximum time resolution
  hf_u32_t max_duration;       ///< Maximum single duration
  size_t max_buffer_size;      ///< Maximum buffer size
  bool supports_bidirectional; ///< Supports bidirectional mode
  bool supports_loopback;      ///< Supports loopback mode
  bool supports_carrier;       ///< Supports carrier modulation
};

/**
 * @brief PIO operation statistics.
 */
struct hf_pio_statistics_t {
  hf_u32_t totalTransmissions;        ///< Total transmissions performed
  hf_u32_t successfulTransmissions;   ///< Successful transmissions
  hf_u32_t failedTransmissions;       ///< Failed transmissions
  hf_u32_t totalReceptions;           ///< Total receptions performed
  hf_u32_t successfulReceptions;      ///< Successful receptions
  hf_u32_t failedReceptions;          ///< Failed receptions
  hf_u32_t symbolsTransmitted;        ///< Total symbols transmitted
  hf_u32_t symbolsReceived;           ///< Total symbols received
  hf_u32_t averageTransmissionTimeUs; ///< Average transmission time (microseconds)
  hf_u32_t maxTransmissionTimeUs;     ///< Maximum transmission time
  hf_u32_t minTransmissionTimeUs;     ///< Minimum transmission time
  hf_u32_t timingErrors;              ///< Number of timing errors
  hf_u32_t bufferOverflows;           ///< Number of buffer overflows

  hf_pio_statistics_t()
      : totalTransmissions(0), successfulTransmissions(0), failedTransmissions(0),
        totalReceptions(0), successfulReceptions(0), failedReceptions(0), symbolsTransmitted(0),
        symbolsReceived(0), averageTransmissionTimeUs(0), maxTransmissionTimeUs(0),
        minTransmissionTimeUs(UINT32_MAX), timingErrors(0), bufferOverflows(0) {}
};

/**
 * @brief PIO diagnostic information.
 */
struct hf_pio_diagnostics_t {
  bool pioHealthy;              ///< Overall PIO health status
  hf_pio_err_t lastErrorCode;   ///< Last error code
  hf_u32_t lastErrorTimestamp;  ///< Last error timestamp
  hf_u32_t consecutiveErrors;   ///< Consecutive error count
  bool pioInitialized;          ///< PIO initialization status
  hf_u8_t activeChannels;       ///< Number of active channels
  hf_u32_t currentResolutionNs; ///< Current time resolution
  bool bidirectionalSupported;  ///< Bidirectional mode support

  hf_pio_diagnostics_t()
      : pioHealthy(true), lastErrorCode(hf_pio_err_t::PIO_SUCCESS), lastErrorTimestamp(0),
        consecutiveErrors(0), pioInitialized(false), activeChannels(0), currentResolutionNs(0),
        bidirectionalSupported(false) {}
};

//--------------------------------------
//  Callback Types
//--------------------------------------

/**
 * @brief Callback for PIO transmission complete events
 * @param channel_id Channel that completed transmission
 * @param symbols_sent Number of symbols transmitted
 * @param user_data User-provided data
 */
using hf_pio_transmit_callback_t =
    std::function<void(hf_u8_t channel_id, size_t symbols_sent, void* user_data)>;

/**
 * @brief Callback for PIO reception complete events
 * @param channel_id Channel that received data
 * @param symbols Received symbols
 * @param symbol_count Number of symbols received
 * @param user_data User-provided data
 */
using hf_pio_receive_callback_t = std::function<void(
    hf_u8_t channel_id, const hf_pio_symbol_t* symbols, size_t symbol_count, void* user_data)>;

/**
 * @brief Callback for PIO error events
 * @param channel_id Channel that encountered error
 * @param error Error that occurred
 * @param user_data User-provided data
 */
using hf_pio_error_callback_t =
    std::function<void(hf_u8_t channel_id, hf_pio_err_t error, void* user_data)>;

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BasePio
 * @ingroup pio
 * @brief Abstract base class for Programmable IO Channel implementations.
 *
 * This class defines the interface for precise, buffered digital signal I/O
 * that can handle timing-critical operations like WS2812 LED driving,
 * IR communication, stepper motor control, and custom protocols.
 *
 * The abstraction is designed to work with various hardware backends:
 * - Dedicated peripherals (e.g. RMT or PIO engines)
 * - Timer + DMA + GPIO combinations
 * - Fully software-based implementations
 *
 * Key features:
 * - Precise timing control down to nanosecond resolution
 * - Buffered symbol transmission and reception
 * - Asynchronous operation with callbacks
 * - Platform-agnostic interface
 * - Support for complex waveform generation and decoding
 */
class BasePio {
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~BasePio() noexcept = default;

  // Disable copy constructor and assignment operator for safety
  BasePio(const BasePio&) = delete;
  BasePio& operator=(const BasePio&) = delete;

  // Allow move operations
  BasePio(BasePio&&) noexcept = default;
  BasePio& operator=(BasePio&&) noexcept = default;

  //==============================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
  //==============================================//

  /**
   * @brief Initialize the PIO peripheral
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the PIO peripheral
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Check if the PIO is initialized
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Ensures that the PIO is initialized (lazy initialization).
   * @return true if the PIO is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = (Initialize() == hf_pio_err_t::PIO_SUCCESS);
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the PIO is deinitialized (lazy deinitialization).
   * @return true if the PIO is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !(Deinitialize() == hf_pio_err_t::PIO_SUCCESS);
      return !initialized_;
    }
    return true;
  }

  /**
   * @brief Configure a PIO channel
   * @param channel_id Channel identifier
   * @param config Channel configuration
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t ConfigureChannel(hf_u8_t channel_id,
                                        const hf_pio_channel_config_t& config) noexcept = 0;

  /**
   * @brief Transmit a sequence of symbols
   * @param channel_id Channel identifier
   * @param symbols Array of symbols to transmit
   * @param symbol_count Number of symbols in the array
   * @param wait_completion If true, block until transmission is complete
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t Transmit(hf_u8_t channel_id, const hf_pio_symbol_t* symbols,
                                size_t symbol_count, bool wait_completion = false) noexcept = 0;

  /**
   * @brief Start receiving symbols
   * @param channel_id Channel identifier
   * @param buffer Buffer to store received symbols
   * @param buffer_size Size of the buffer
   * @param timeout_us Timeout in microseconds (0 = no timeout)
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t StartReceive(hf_u8_t channel_id, hf_pio_symbol_t* buffer, size_t buffer_size,
                                    hf_u32_t timeout_us = 0) noexcept = 0;

  /**
   * @brief Stop receiving and get the number of symbols received
   * @param channel_id Channel identifier
   * @param symbols_received [out] Number of symbols actually received
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t StopReceive(hf_u8_t channel_id, size_t& symbols_received) noexcept = 0;

  /**
   * @brief Check if a channel is currently busy
   * @param channel_id Channel identifier
   * @return true if channel is busy, false otherwise
   */
  virtual bool IsChannelBusy(hf_u8_t channel_id) const noexcept = 0;

  /**
   * @brief Get channel status information
   * @param channel_id Channel identifier
   * @param status [out] Status information
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t GetChannelStatus(hf_u8_t channel_id,
                                        hf_pio_channel_status_t& status) const noexcept = 0;

  /**
   * @brief Get PIO capabilities
   * @param capabilities [out] Capability information
   * @return Error code indicating success or failure
   */
  virtual hf_pio_err_t GetCapabilities(hf_pio_capabilities_t& capabilities) const noexcept = 0;

  /**
   * @brief Set callback for transmission complete events
   * @param channel_id Channel identifier
   * @param callback Callback function
   * @param user_data User data to pass to callback
   */
  virtual void SetTransmitCallback(hf_u8_t channel_id, hf_pio_transmit_callback_t callback,
                                   void* user_data = nullptr) noexcept = 0;

  /**
   * @brief Set callback for reception complete events
   * @param channel_id Channel identifier
   * @param callback Callback function
   * @param user_data User data to pass to callback
   */
  virtual void SetReceiveCallback(hf_u8_t channel_id, hf_pio_receive_callback_t callback,
                                  void* user_data = nullptr) noexcept = 0;

  /**
   * @brief Set callback for error events
   * @param channel_id Channel identifier
   * @param callback Callback function
   * @param user_data User data to pass to callback
   */
  virtual void SetErrorCallback(hf_u8_t channel_id, hf_pio_error_callback_t callback,
                                void* user_data = nullptr) noexcept = 0;

  /**
   * @brief Clear all callbacks for a specific channel
   * @param channel_id Channel identifier
   */
  virtual void ClearChannelCallbacks(hf_u8_t channel_id) noexcept = 0;

  /**
   * @brief Clear all callbacks
   */
  virtual void ClearCallbacks() noexcept = 0;

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset PIO operation statistics.
   * @return hf_pio_err_t::PIO_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_pio_err_t ResetStatistics() noexcept {
    statistics_ = hf_pio_statistics_t{}; // Reset statistics to default values
    return hf_pio_err_t::PIO_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset PIO diagnostic information.
   * @return hf_pio_err_t::PIO_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_pio_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_pio_diagnostics_t{}; // Reset diagnostics to default values
    return hf_pio_err_t::PIO_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get PIO operation statistics
   * @param channel_id Channel identifier
   * @param statistics Reference to store statistics data
   * @return hf_pio_err_t::PIO_SUCCESS if successful, PIO_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_pio_err_t GetStatistics(hf_u8_t channel_id,
                                     hf_pio_statistics_t& statistics) const noexcept {
    // This method needs to be implemented to return platform-specific statistics
    return hf_pio_err_t::PIO_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get PIO diagnostic information
   * @param channel_id Channel identifier
   * @param diagnostics Reference to store diagnostics data
   * @return hf_pio_err_t::PIO_SUCCESS if successful, PIO_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_pio_err_t GetDiagnostics(hf_u8_t channel_id,
                                      hf_pio_diagnostics_t& diagnostics) const noexcept {
    // This method needs to be implemented to return platform-specific diagnostics
    return hf_pio_err_t::PIO_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  /**
   * @brief Protected constructor
   */
  BasePio() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}

  /**
   * @brief Initialization state tracking
   */
  bool initialized_;
  hf_pio_statistics_t statistics_;   ///< PIO operation statistics
  hf_pio_diagnostics_t diagnostics_; ///< PIO diagnostic information
};
