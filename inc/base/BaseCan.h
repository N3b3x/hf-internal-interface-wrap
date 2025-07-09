/**
 * @file BaseCan.h
 * @brief Abstract base class for CAN bus implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for CAN bus communication
 * that provides a consistent API across different CAN controller implementations.
 * Concrete implementations for various microcontrollers inherit from this class.
 * 
 * ERROR HANDLING:
 * - All CAN error codes are defined in this file using the hf_can_err_t enumeration
 * - Lower-level types (McuTypes_CAN.h) maintain minimal error constants for compatibility
 * - All virtual methods return hf_can_err_t for comprehensive error reporting
 * - No legacy compatibility code - use hf_can_message_t structure for all operations
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseAdc.
 * @note Users should program against this interface, not specific implementations.
 * @note All legacy camelCase types and bool-returning methods have been removed.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC CAN Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC CAN error codes
 * @details Comprehensive error enumeration for all CAN operations in the system.
 *          This enumeration is used across all CAN-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_CAN_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(CAN_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(CAN_ERR_FAILURE, 1, "General failure")                                                         \
  X(CAN_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(CAN_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(CAN_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(CAN_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(CAN_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Bus errors */                                                                                 \
  X(CAN_ERR_BUS_OFF, 7, "Bus off state")                                                           \
  X(CAN_ERR_BUS_ERROR, 8, "Bus error")                                                             \
  X(CAN_ERR_BUS_BUSY, 9, "Bus busy")                                                               \
  X(CAN_ERR_BUS_NOT_AVAILABLE, 10, "Bus not available")                                            \
  X(CAN_ERR_BUS_RECOVERY_FAILED, 11, "Bus recovery failed")                                        \
  X(CAN_ERR_BUS_ARBITRATION_LOST, 12, "Bus arbitration lost")                                      \
  /* Message errors */                                                                             \
  X(CAN_ERR_MESSAGE_TIMEOUT, 13, "Message timeout")                                                \
  X(CAN_ERR_MESSAGE_LOST, 14, "Message lost")                                                      \
  X(CAN_ERR_MESSAGE_INVALID, 15, "Invalid message")                                                \
  X(CAN_ERR_MESSAGE_TOO_LONG, 16, "Message too long")                                              \
  X(CAN_ERR_MESSAGE_INVALID_ID, 17, "Invalid message ID")                                          \
  X(CAN_ERR_MESSAGE_INVALID_DLC, 18, "Invalid data length code")                                   \
  X(CAN_ERR_QUEUE_FULL, 19, "Queue full")                                                          \
  X(CAN_ERR_QUEUE_EMPTY, 20, "Queue empty")                                                        \
  /* Transmission errors */                                                                        \
  X(CAN_ERR_TX_FAILED, 21, "Transmission failed")                                                  \
  X(CAN_ERR_TX_ABORTED, 22, "Transmission aborted")                                                \
  X(CAN_ERR_TX_ERROR_PASSIVE, 23, "Transmit error passive")                                        \
  X(CAN_ERR_TX_ERROR_WARNING, 24, "Transmit error warning")                                        \
  /* Reception errors */                                                                           \
  X(CAN_ERR_RX_OVERRUN, 25, "Receive overrun")                                                     \
  X(CAN_ERR_RX_ERROR_PASSIVE, 26, "Receive error passive")                                         \
  X(CAN_ERR_RX_ERROR_WARNING, 27, "Receive error warning")                                         \
  X(CAN_ERR_RX_FIFO_FULL, 28, "Receive FIFO full")                                                 \
  /* Hardware errors */                                                                            \
  X(CAN_ERR_HARDWARE_FAULT, 29, "Hardware fault")                                                  \
  X(CAN_ERR_COMMUNICATION_FAILURE, 30, "Communication failure")                                    \
  X(CAN_ERR_DEVICE_NOT_RESPONDING, 31, "Device not responding")                                    \
  X(CAN_ERR_VOLTAGE_OUT_OF_RANGE, 32, "Voltage out of range")                                      \
  X(CAN_ERR_CLOCK_ERROR, 33, "Clock error")                                                        \
  X(CAN_ERR_TRANSCEIVER_ERROR, 34, "Transceiver error")                                            \
  /* Configuration errors */                                                                       \
  X(CAN_ERR_INVALID_CONFIGURATION, 35, "Invalid configuration")                                    \
  X(CAN_ERR_UNSUPPORTED_OPERATION, 36, "Unsupported operation")                                    \
  X(CAN_ERR_INVALID_BAUD_RATE, 37, "Invalid baud rate")                                            \
  X(CAN_ERR_INVALID_CONTROLLER_ID, 38, "Invalid controller ID")                                    \
  X(CAN_ERR_FILTER_ERROR, 39, "Filter error")                                                      \
  X(CAN_ERR_FILTER_FULL, 40, "Filter table full")                                                  \
  /* Protocol errors */                                                                            \
  X(CAN_ERR_STUFF_ERROR, 41, "Bit stuffing error")                                                 \
  X(CAN_ERR_FORM_ERROR, 42, "Frame format error")                                                  \
  X(CAN_ERR_CRC_ERROR, 43, "CRC error")                                                            \
  X(CAN_ERR_ACK_ERROR, 44, "Acknowledgment error")                                                 \
  X(CAN_ERR_BIT_ERROR, 45, "Bit error")                                                            \
  /* System errors */                                                                              \
  X(CAN_ERR_SYSTEM_ERROR, 46, "System error")                                                      \
  X(CAN_ERR_PERMISSION_DENIED, 47, "Permission denied")                                            \
  X(CAN_ERR_OPERATION_ABORTED, 48, "Operation aborted")                                            \
                                                                                                   \
  /* Extended CAN errors (for ESP32 compatibility) */                                             \
  X(CAN_ERR_FAIL, 49, "Generic failure")                                                           \
  X(CAN_ERR_RESOURCE_BUSY, 50, "Resource busy")                                                    \
  X(CAN_ERR_INVALID_STATE, 51, "Invalid state")                                                    \
  X(CAN_ERR_NOT_SUPPORTED, 52, "Not supported")                                                    \
  X(CAN_ERR_TIMEOUT_ALT, 53, "Operation timeout")

enum class hf_can_err_t : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_CAN_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_can_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfCanErrToString(hf_can_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_can_err_t::NAME:                                                                         \
    return DESC;
    HF_CAN_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @brief Platform-agnostic CAN bus configuration.
 * @details Configuration structure that works across different CAN implementations
 *          without exposing MCU-specific types.
 */
struct hf_can_config_t {
  hf_pin_num_t tx_pin;     ///< CAN TX pin
  hf_pin_num_t rx_pin;     ///< CAN RX pin
  hf_baud_rate_t baudrate; ///< CAN baudrate (bps)
  bool loopback_mode;      ///< Enable loopback mode for testing
  bool silent_mode;        ///< Enable silent mode (listen-only)
  uint16_t tx_queue_size;  ///< TX queue size (implementation-dependent)
  uint16_t rx_queue_size;  ///< RX queue size (implementation-dependent)

  hf_can_config_t() noexcept
      : tx_pin(HF_INVALID_PIN), rx_pin(HF_INVALID_PIN), baudrate(500000), loopback_mode(false),
        silent_mode(false), tx_queue_size(10), rx_queue_size(10) {}
};

/**
 * @brief Platform-agnostic CAN message structure.
 * @details Comprehensive CAN message format with standard flags and metadata.
 *          Supports both standard (11-bit) and extended (29-bit) identifiers,
 *          with complete transmission control and diagnostic information.
 */
struct hf_can_message_t {
  // === Core CAN Message Fields ===
  uint32_t id;     ///< Message ID (11 or 29-bit)
  uint8_t dlc;     ///< Data length code (0-8 for classic CAN)
  uint8_t data[8]; ///< Message data (max 8 bytes for classic CAN)

  // === Standard CAN Flags ===
  bool is_extended;  ///< Extended ID flag (29-bit vs 11-bit)
  bool is_rtr;       ///< Remote transmission request flag
  bool is_ss;        ///< Single shot flag (no retransmission)
  bool is_self;      ///< Self reception request flag
  bool dlc_non_comp; ///< DLC is non-compliant (> 8 for classic CAN)

  // === Metadata and Diagnostics ===
  uint64_t timestamp_us;    ///< Precise timestamp in microseconds
  uint32_t sequence_number; ///< Message sequence number
  uint8_t controller_id;    ///< Originating controller ID
  uint8_t retry_count;      ///< Number of transmission retries
  uint8_t error_count;      ///< Associated error count

  // === CAN-FD Extended Fields (for future compatibility) ===
  bool is_canfd;       ///< CAN-FD frame flag
  bool is_brs;         ///< Bit Rate Switching flag (CAN-FD)
  bool is_esi;         ///< Error State Indicator flag (CAN-FD)
  uint8_t canfd_dlc;   ///< CAN-FD DLC (can be > 8)

  hf_can_message_t() noexcept
      : id(0), dlc(0), data{}, is_extended(false), is_rtr(false), is_ss(false), 
        is_self(false), dlc_non_comp(false), timestamp_us(0), sequence_number(0),
        controller_id(0), retry_count(0), error_count(0), is_canfd(false), 
        is_brs(false), is_esi(false), canfd_dlc(0) {}

  /**
   * @brief Get maximum data length for current frame type
   * @return Maximum allowed data length (8 for classic CAN, up to 64 for CAN-FD)
   */
  constexpr uint8_t GetMaxDataLength() const noexcept { 
    return is_canfd ? 64 : 8; 
  }

  /**
   * @brief Validate DLC for current frame type
   * @param dlc Data length code to validate
   * @return true if valid for the frame type, false otherwise
   */
  bool IsValidDLC(uint8_t dlc) const noexcept { 
    return is_canfd ? (dlc <= 64) : (dlc <= 8); 
  }

  /**
   * @brief Get effective DLC for the current frame type
   * @return DLC value to use (canfd_dlc for CAN-FD, dlc for classic)
   */
  uint8_t GetEffectiveDLC() const noexcept {
    return is_canfd ? canfd_dlc : dlc;
  }

  /**
   * @brief Set data length code for current frame type
   * @param dlc Data length code to set
   * @return true if valid and set, false otherwise
   */
  bool SetDLC(uint8_t dlc) noexcept {
    if (!IsValidDLC(dlc)) {
      return false;
    }
    if (is_canfd) {
      canfd_dlc = dlc;
    } else {
      this->dlc = dlc;
    }
    return true;
  }

  /**
   * @brief Set standard frame format (11-bit ID)
   */
  void SetStandardFrame() noexcept { is_extended = false; }

  /**
   * @brief Set extended frame format (29-bit ID)
   */
  void SetExtendedFrame() noexcept { is_extended = true; }

  /**
   * @brief Set data frame (not remote)
   */
  void SetDataFrame() noexcept { is_rtr = false; }

  /**
   * @brief Set remote frame
   */
  void SetRemoteFrame() noexcept { is_rtr = true; }

  /**
   * @brief Set single shot transmission
   */
  void SetSingleShot() noexcept { is_ss = true; }

  /**
   * @brief Set self reception request
   */
  void SetSelfReception() noexcept { is_self = true; }

  /**
   * @brief Validate message ID for current frame format
   * @return true if valid, false otherwise
   */
  bool IsValidId() const noexcept {
    if (is_extended) {
      return id <= 0x1FFFFFFF; // 29-bit max
    } else {
      return id <= 0x7FF; // 11-bit max
    }
  }
};

/**
 * @brief CAN bus status information structure.
 * @details Comprehensive status information for CAN bus monitoring and diagnostics.
 */
struct hf_can_status_t {
  uint32_t tx_error_count;  ///< Transmit error counter
  uint32_t rx_error_count;  ///< Receive error counter
  uint32_t tx_failed_count; ///< Failed transmission count
  uint32_t rx_missed_count; ///< Missed reception count
  bool bus_off;             ///< Bus-off state
  bool error_warning;       ///< Error warning state
  bool error_passive;       ///< Error passive state

  // CAN-FD specific status
  bool canfd_enabled;        ///< CAN-FD mode is active
  bool canfd_brs_enabled;    ///< Bit Rate Switching is enabled
  uint32_t nominal_baudrate; ///< Nominal bit rate (arbitration phase)
  uint32_t data_baudrate;    ///< Data bit rate (data phase for CAN-FD)
  uint32_t canfd_tx_count;   ///< Number of CAN-FD frames transmitted
  uint32_t canfd_rx_count;   ///< Number of CAN-FD frames received
  uint32_t brs_tx_count;     ///< Number of BRS frames transmitted
  uint32_t brs_rx_count;     ///< Number of BRS frames received
  uint32_t form_errors;      ///< CAN-FD form errors
  uint32_t stuff_errors;     ///< Stuff errors
  uint32_t crc_errors;       ///< CRC errors
  uint32_t bit_errors;       ///< Bit errors
  uint32_t ack_errors;       ///< Acknowledgment errors

  hf_can_status_t() noexcept
      : tx_error_count(0), rx_error_count(0), tx_failed_count(0), rx_missed_count(0),
        bus_off(false), error_warning(false), error_passive(false), canfd_enabled(false),
        canfd_brs_enabled(false), nominal_baudrate(0), data_baudrate(0), canfd_tx_count(0),
        canfd_rx_count(0), brs_tx_count(0), brs_rx_count(0), form_errors(0), stuff_errors(0),
        crc_errors(0), bit_errors(0), ack_errors(0) {}
};

/**
 * @brief CAN message receive callback function type.
 * @note Updated to use new hf_can_message_t structure
 */
using hf_can_receive_callback_t = std::function<void(const hf_can_message_t &message)>;

/**
 * @brief CAN-FD specific receive callback with enhanced information
 * @param message Received CAN/CAN-FD message
 * @param reception_info Additional reception information (timing, errors, etc.)
 */
struct hf_can_reception_info_t {
  uint32_t timestamp_us;      ///< Reception timestamp in microseconds
  uint8_t rx_fifo_level;      ///< RX FIFO level when received
  bool data_phase_error;      ///< Error occurred in data phase
  bool arbitration_lost;      ///< Arbitration was lost during transmission
  float bit_timing_tolerance; ///< Measured bit timing tolerance
};
using hf_can_fd_receive_callback_t =
    std::function<void(const hf_can_message_t &message, const hf_can_reception_info_t &info)>;


/**
 * @brief CAN bus statistics structure for performance monitoring.
 * @details Provides comprehensive statistics for monitoring CAN bus performance
 *          and identifying potential issues.
 */
struct hf_can_statistics_t {
  // Message counters
  uint64_t messages_sent;          ///< Total messages successfully sent
  uint64_t messages_received;      ///< Total messages successfully received
  uint64_t bytes_transmitted;      ///< Total bytes transmitted
  uint64_t bytes_received;         ///< Total bytes received
  
  // Error counters
  uint32_t send_failures;          ///< Failed send operations
  uint32_t receive_failures;       ///< Failed receive operations
  uint32_t bus_error_count;        ///< Total bus errors
  uint32_t arbitration_lost_count; ///< Arbitration lost events
  uint32_t tx_failed_count;        ///< Transmission failures
  uint32_t bus_off_events;         ///< Bus-off occurrences
  uint32_t error_warning_events;   ///< Error warning events
  
  // Performance metrics
  uint64_t uptime_seconds;         ///< Total uptime in seconds
  uint32_t last_activity_timestamp;///< Last activity timestamp
  hf_can_err_t last_error;         ///< Last error encountered
  
  // Queue statistics
  uint32_t tx_queue_peak;          ///< Peak TX queue usage
  uint32_t rx_queue_peak;          ///< Peak RX queue usage
  uint32_t tx_queue_overflows;     ///< TX queue overflow count
  uint32_t rx_queue_overflows;     ///< RX queue overflow count

  hf_can_statistics_t() noexcept
      : messages_sent(0), messages_received(0), bytes_transmitted(0), bytes_received(0),
        send_failures(0), receive_failures(0), bus_error_count(0), arbitration_lost_count(0),
        tx_failed_count(0), bus_off_events(0), error_warning_events(0), uptime_seconds(0),
        last_activity_timestamp(0), last_error(hf_can_err_t::CAN_SUCCESS), tx_queue_peak(0),
        rx_queue_peak(0), tx_queue_overflows(0), rx_queue_overflows(0) {}
};

/**
 * @brief CAN diagnostics structure for detailed error analysis.
 * @details Provides detailed diagnostic information for troubleshooting
 *          and monitoring CAN bus health and performance.
 */
struct hf_can_diagnostics_t {
  uint32_t tx_error_count;         ///< Transmit error counter
  uint32_t rx_error_count;         ///< Receive error counter
  uint32_t tx_queue_peak;          ///< Peak TX queue usage
  uint32_t rx_queue_peak;          ///< Peak RX queue usage
  uint32_t last_error_timestamp;   ///< Timestamp of last error
  uint32_t controller_resets;      ///< Number of controller resets
  uint32_t bus_load_percentage;    ///< Current bus load percentage
  float bit_error_rate;            ///< Bit error rate (errors/bits)
  
  hf_can_diagnostics_t() noexcept
      : tx_error_count(0), rx_error_count(0), tx_queue_peak(0), rx_queue_peak(0),
        last_error_timestamp(0), controller_resets(0), bus_load_percentage(0), bit_error_rate(0.0f) {}
};

  
/**
 * @class BaseCan
 * @brief Abstract base class defining the unified CAN bus API.
 *
 * This abstract class defines the interface that all CAN controller implementations
 * must provide. It ensures a consistent API across different platforms and CAN
 * controller types, making the system extensible and maintainable.
 *
 * Concrete implementation examples:
 * - Microcontrollers with integrated CAN peripherals
 * - External CAN controllers via SPI (e.g. MCP2515)
 *
 * Features:
 * - Lazy initialization support (initialize on first use)
 * - Comprehensive error handling with detailed error codes
 * - Thread-safe operations (implementation dependent)
 * - Consistent API across different CAN hardware
 *
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 */
class BaseCan {
public:
  /**
   * @brief Virtual destructor ensures proper cleanup in derived classes.
   */
  virtual ~BaseCan() noexcept = default;

  // Non-copyable, non-movable (can be changed in derived classes if needed)
  BaseCan(const BaseCan &) = delete;
  BaseCan &operator=(const BaseCan &) = delete;
  BaseCan(BaseCan &&) = delete;
  BaseCan &operator=(BaseCan &&) = delete;

  /**
   * @brief Ensures that the CAN bus is initialized (lazy initialization).
   * @return true if the CAN bus is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = (Initialize() == hf_can_err_t::CAN_SUCCESS);
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the CAN bus is deinitialized.
   * @return true if the CAN bus is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !(Deinitialize() == hf_can_err_t::CAN_SUCCESS);
    }
    return !initialized_;
  }

  /**
   * @brief Checks if the class is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS [MUST BE OVERRIDDEN]  //
  //==============================================//

  /**
   * @brief Initialize the CAN controller (must be implemented by derived classes).
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the CAN controller (must be implemented by derived classes).
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Send a CAN message.
   * @param message CAN message to send
   * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t SendMessage(const hf_can_message_t &message, uint32_t timeout_ms = 1000) noexcept = 0;

  /**
   * @brief Receive a CAN message.
   * @param message Reference to store received message
   * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t ReceiveMessage(hf_can_message_t &message, uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Set callback for received messages.
   * @param callback Callback function to handle received messages
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t SetReceiveCallback(hf_can_receive_callback_t callback) noexcept = 0;

  /**
   * @brief Set acceptance filter for incoming messages.
   * @param id CAN ID to accept
   * @param mask Acceptance mask (0 = don't care bits)
   * @param extended true for extended frames, false for standard
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept = 0;

  /**
   * @brief Get current CAN bus status.
   * @param status Reference to store status information
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t GetStatus(hf_can_status_t &status) noexcept = 0;

  /**
   * @brief Reset the CAN controller.
   * @return hf_can_err_t error code
   */
  virtual hf_can_err_t Reset() noexcept = 0;

  //==============================================//
  // (OPTIONAL IMPLEMENTATIONS)                   //
  //==============================================//

  /**
   * @brief Clear the receive callback.
   * @note Default implementation does nothing
   */
  virtual void ClearReceiveCallback() noexcept {}

  /**
   * @brief Clear all acceptance filters (accept all messages).
   * @return hf_can_err_t error code
   * @note Default implementation sets filter to accept all (ID=0, Mask=0)
   */
  virtual hf_can_err_t ClearAcceptanceFilter() noexcept {
    return SetAcceptanceFilter(0, 0, false);
  }

  /**
   * @brief Set CAN-FD receive callback with enhanced information.
   * @param callback CAN-FD callback function
   * @return hf_can_err_t error code
   * @note Default implementation returns CAN_ERR_UNSUPPORTED_OPERATION
   */
  virtual hf_can_err_t SetReceiveCallbackFD(hf_can_fd_receive_callback_t callback) noexcept {
    (void)callback; // Suppress unused parameter warning
    return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Check if CAN-FD is supported by this controller.
   * @return true if CAN-FD is supported, false otherwise
   * @note Default implementation returns false
   */
  virtual bool SupportsCanFD() const noexcept {
    return false;
  }

  /**
   * @brief Enable or disable CAN-FD mode.
   * @param enable true to enable CAN-FD, false to disable
   * @param data_baudrate Data phase baudrate for CAN-FD (default: 2Mbps)
   * @return true if successful, false otherwise
   * @note Default implementation returns false (not supported)
   */
  virtual bool SetCanFDMode(bool enable, uint32_t data_baudrate = 2000000,
                           uint32_t timeout_ms = 1000) noexcept {
    (void)enable;
    (void)data_baudrate;
    (void)timeout_ms;
    return false;
  }

  /**
   * @brief Configure CAN-FD timing parameters.
   * @param nominal_prescaler Nominal phase prescaler
   * @param nominal_tseg1 Nominal phase TSEG1
   * @param nominal_tseg2 Nominal phase TSEG2
   * @param data_prescaler Data phase prescaler
   * @param data_tseg1 Data phase TSEG1
   * @param data_tseg2 Data phase TSEG2
   * @param sjw Synchronization jump width
   * @return true if successful, false otherwise
   * @note Default implementation returns false (not supported)
   */
  virtual bool ConfigureCanFDTiming(uint16_t nominal_prescaler, uint8_t nominal_tseg1,
                                   uint8_t nominal_tseg2, uint16_t data_prescaler,
                                   uint8_t data_tseg1, uint8_t data_tseg2,
                                   uint8_t sjw = 1) noexcept {
    (void)nominal_prescaler;
    (void)nominal_tseg1;
    (void)nominal_tseg2;
    (void)data_prescaler;
    (void)data_tseg1;
    (void)data_tseg2;
    (void)sjw;
    return false;
  }

  /**
   * @brief Set transmitter delay compensation (CAN-FD feature).
   * @param tdc_offset TDC offset value
   * @param tdc_filter TDC filter value
   * @return true if successful, false otherwise
   * @note Default implementation returns false (not supported)
   */
  virtual bool SetTransmitterDelayCompensation(uint8_t tdc_offset, uint8_t tdc_filter) noexcept {
    (void)tdc_offset;
    (void)tdc_filter;
    return false;
  }

  /**
   * @brief Send multiple messages in a batch.
   * @param messages Array of messages to send
   * @param count Number of messages to send
   * @param timeout_ms Timeout in milliseconds
   * @return Number of messages successfully sent
   * @note Default implementation sends messages sequentially
   */
  virtual uint32_t SendMessageBatch(const hf_can_message_t *messages, uint32_t count,
                                   uint32_t timeout_ms = 1000) noexcept {
    if (!messages || count == 0) {
      return 0;
    }

    uint32_t sent_count = 0;
    for (uint32_t i = 0; i < count; ++i) {
      if (SendMessage(messages[i], timeout_ms) == hf_can_err_t::CAN_SUCCESS) {
        sent_count++;
      }
    }
    return sent_count;
  }

  /**
   * @brief Receive multiple messages in a batch.
   * @param messages Array to store received messages
   * @param max_count Maximum number of messages to receive
   * @param timeout_ms Timeout in milliseconds
   * @return Number of messages actually received
   * @note Default implementation receives messages one by one
   */
  virtual uint32_t ReceiveMessageBatch(hf_can_message_t *messages, uint32_t max_count,
                                      uint32_t timeout_ms = 100) noexcept {
    if (!messages || max_count == 0) {
      return 0;
    }

    uint32_t received_count = 0;
    for (uint32_t i = 0; i < max_count; ++i) {
      if (ReceiveMessage(messages[i], timeout_ms) == hf_can_err_t::CAN_SUCCESS) {
        received_count++;
      } else {
        break; // No more messages available
      }
    }
    return received_count;
  }

//==============================================//
// STATISTICS AND DIAGNOSTICS STRUCTURES        //
//==============================================//

  /**
   * @brief Reset CAN operation statistics.
   * @return hf_can_err_t::CAN_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_can_err_t ResetStatistics() noexcept {
    statistics_ = hf_can_statistics_t{}; // Reset statistics to default values
    return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset CAN diagnostic information.
   * @return hf_can_err_t::CAN_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_can_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_can_diagnostics_t{}; // Reset diagnostics to default values
    return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get CAN operation statistics.
   * @param statistics Reference to statistics structure to fill
   * @return hf_can_err_t::CAN_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics
   */
  virtual hf_can_err_t GetStatistics(hf_can_statistics_t &statistics) noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get CAN diagnostic information.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return hf_can_err_t::CAN_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics
   */
  virtual hf_can_err_t GetDiagnostics(hf_can_diagnostics_t &diagnostics) noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
  }

protected:

  /**
   * @brief Protected constructor
   */
  BaseCan() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}

  //==============================================//
  // VARIABLES                                    //
  //==============================================//

  bool initialized_; ///< Initialization status
  hf_can_statistics_t statistics_; ///< CAN operation statistics
  hf_can_diagnostics_t diagnostics_; ///< CAN diagnostic information
};

//==============================================//
// END OF BASECAN - CLEAN AND MODERN API        //
//==============================================//
