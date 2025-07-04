/**
 * @file BaseCan.h
 * @brief Abstract base class for CAN bus implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for CAN bus communication
 * that provides a consistent API across different CAN controller implementations.
 * Concrete implementations for various microcontrollers inherit from this class.
 * 
 * ERROR HANDLING:
 * - All CAN error codes are defined in this file using the HfCanErr enumeration
 * - Lower-level types (McuTypes_CAN.h) maintain minimal error constants for compatibility
 * - All virtual methods return HfCanErr for comprehensive error reporting
 * - No legacy compatibility code - use CanMessage structure for all operations
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseAdc.
 * @note Users should program against this interface, not specific implementations.
 * @note All legacy hf_can_message_t and bool-returning methods have been removed.
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
  X(CAN_ERR_RESOURCE_BUSY, 49, "Resource busy")                                                    \
  X(CAN_ERR_INVALID_STATE, 50, "Invalid state")

enum class HfCanErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_CAN_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfCanErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfCanErrToString(HfCanErr err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case HfCanErr::NAME:                                                                             \
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
struct CanBusConfig {
  HfPinNumber tx_pin;     ///< CAN TX pin
  HfPinNumber rx_pin;     ///< CAN RX pin
  HfBaudRate baudrate;    ///< CAN baudrate (bps)
  bool loopback_mode;     ///< Enable loopback mode for testing
  bool silent_mode;       ///< Enable silent mode (listen-only)
  uint16_t tx_queue_size; ///< TX queue size (implementation-dependent)
  uint16_t rx_queue_size; ///< RX queue size (implementation-dependent)

  CanBusConfig() noexcept
      : tx_pin(HF_INVALID_PIN), rx_pin(HF_INVALID_PIN), baudrate(500000), loopback_mode(false),
        silent_mode(false), tx_queue_size(10), rx_queue_size(10) {}
};

/**
 * @brief Platform-agnostic CAN message structure.
 * @details Comprehensive CAN message format with standard flags and metadata.
 *          Supports both standard (11-bit) and extended (29-bit) identifiers,
 *          with complete transmission control and diagnostic information.
 */
struct CanMessage {
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

  CanMessage() noexcept
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
   * @brief Set message as standard CAN (11-bit ID)
   */
  void SetStandardFrame() noexcept { is_extended = false; }

  /**
   * @brief Set message as extended CAN (29-bit ID)
   */
  void SetExtendedFrame() noexcept { is_extended = true; }

  /**
   * @brief Set message as data frame
   */
  void SetDataFrame() noexcept { is_rtr = false; }

  /**
   * @brief Set message as remote transmission request
   */
  void SetRemoteFrame() noexcept { is_rtr = true; }

  /**
   * @brief Set single shot transmission (no retries)
   */
  void SetSingleShot() noexcept { is_ss = true; }

  /**
   * @brief Enable self-reception
   */
  void SetSelfReception() noexcept { is_self = true; }

  /**
   * @brief Check if message ID is valid for the frame type
   * @return true if ID is within valid range
   */
  bool IsValidId() const noexcept {
    return is_extended ? (id <= 0x1FFFFFFF) : (id <= 0x7FF);
  }
};

/**
 * @brief Enhanced CAN bus status information including CAN-FD metrics
 */
struct CanBusStatus {
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

  CanBusStatus() noexcept
      : tx_error_count(0), rx_error_count(0), tx_failed_count(0), rx_missed_count(0),
        bus_off(false), error_warning(false), error_passive(false), canfd_enabled(false),
        canfd_brs_enabled(false), nominal_baudrate(0), data_baudrate(0), canfd_tx_count(0),
        canfd_rx_count(0), brs_tx_count(0), brs_rx_count(0), form_errors(0), stuff_errors(0),
        crc_errors(0), bit_errors(0), ack_errors(0) {}
};

/**
 * @brief CAN message receive callback function type.
 * @note Updated to use new CanMessage structure
 */
using CanReceiveCallback = std::function<void(const CanMessage &message)>;

/**
 * @brief CAN-FD specific receive callback with enhanced information
 * @param message Received CAN/CAN-FD message
 * @param reception_info Additional reception information (timing, errors, etc.)
 */
struct CanReceptionInfo {
  uint32_t timestamp_us;      ///< Reception timestamp in microseconds
  uint8_t rx_fifo_level;      ///< RX FIFO level when received
  bool data_phase_error;      ///< Error occurred in data phase
  bool arbitration_lost;      ///< Arbitration was lost during transmission
  float bit_timing_tolerance; ///< Measured bit timing tolerance
};
using CanFdReceiveCallback =
    std::function<void(const CanMessage &message, const CanReceptionInfo &info)>;

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
   * @brief Constructor
   */
  BaseCan() noexcept : initialized_(false) {}

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
      initialized_ = (Initialize() == HfCanErr::CAN_SUCCESS);
    }
    return initialized_;
  }

  /**
   * @brief Checks if the class is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
  //==============================================//

  /**
   * @brief Initialize the CAN bus.
   * @return HfCanErr::CAN_SUCCESS if successful, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the CAN bus.
   * @return HfCanErr::CAN_SUCCESS if successful, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr Deinitialize() noexcept = 0;

  /**
   * @brief Send a CAN/CAN-FD message.
   * @param message Message to send (supports both classic CAN and CAN-FD)
   * @param timeout_ms Timeout in milliseconds (0 = no wait)
   * @return HfCanErr::CAN_SUCCESS if sent successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr SendMessage(const CanMessage &message, uint32_t timeout_ms = 1000) noexcept = 0;

  /**
   * @brief Receive a CAN/CAN-FD message.
   * @param message Buffer to store received message
   * @param timeout_ms Timeout in milliseconds (0 = no wait)
   * @return HfCanErr::CAN_SUCCESS if received successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr ReceiveMessage(CanMessage &message, uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Set receive callback for interrupt-driven reception.
   * @param callback Callback function to call when message is received
   * @return HfCanErr::CAN_SUCCESS if callback set successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr SetReceiveCallback(CanReceiveCallback callback) noexcept = 0;

  /**
   * @brief Set enhanced CAN-FD receive callback with additional information.
   * @param callback Enhanced callback with reception information
   * @return HfCanErr::CAN_SUCCESS if callback set successfully, error code otherwise
   * @note Default implementation falls back to standard callback
   */
  virtual HfCanErr SetReceiveCallbackFD(CanFdReceiveCallback callback) noexcept {
    // Default implementation: convert to standard callback
    if (!callback) {
      return SetReceiveCallback(nullptr);
    }

    auto standard_callback = [callback](const CanMessage &msg) {
      CanReceptionInfo info{}; // Default/empty info
      callback(msg, info);
    };

    return SetReceiveCallback(standard_callback);
  }

  /**
   * @brief Clear the receive callback.
   * @note Must be implemented by concrete classes.
   */
  virtual void ClearReceiveCallback() noexcept = 0;

  /**
   * @brief Get current bus status including CAN-FD metrics.
   * @param status Buffer to store status information
   * @return HfCanErr::CAN_SUCCESS if status retrieved successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr GetStatus(CanBusStatus &status) noexcept = 0;

  /**
   * @brief Reset the CAN controller (clear errors, restart).
   * @return HfCanErr::CAN_SUCCESS if reset successful, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr Reset() noexcept = 0;

  /**
   * @brief Set acceptance filter for incoming messages.
   * @param id CAN ID to accept
   * @param mask Acceptance mask (0 = don't care bits)
   * @param extended true for extended frames, false for standard
   * @return HfCanErr::CAN_SUCCESS if filter set successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept = 0;

  /**
   * @brief Clear all acceptance filters (accept all messages).
   * @return HfCanErr::CAN_SUCCESS if cleared successfully, error code otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual HfCanErr ClearAcceptanceFilter() noexcept = 0;

  //==============================================//
  // CAN-FD SPECIFIC METHODS (OPTIONAL)          //
  //==============================================//

  /**
   * @brief Check if CAN-FD is supported by this implementation.
   * @return true if CAN-FD is supported, false otherwise
   * @note Default implementation returns false (classic CAN only)
   */
  virtual bool SupportsCanFD() const noexcept {
    return false;
  }

  /**
   * @brief Enable/disable CAN-FD mode.
   * @param enable true to enable CAN-FD, false for classic CAN
   * @param data_baudrate Data phase baudrate (for BRS)
   * @param enable_brs Enable Bit Rate Switching
   * @return true if mode set successfully, false otherwise
   * @note Default implementation returns false (not supported)
   */
  virtual bool SetCanFDMode(bool enable, uint32_t data_baudrate = 2000000,
                            bool enable_brs = true) noexcept {
    return false; // Not supported by default
  }

  /**
   * @brief Configure CAN-FD timing parameters.
   * @param nominal_prescaler Prescaler for nominal bit timing
   * @param nominal_tseg1 Time segment 1 for nominal bit timing
   * @param nominal_tseg2 Time segment 2 for nominal bit timing
   * @param data_prescaler Prescaler for data bit timing
   * @param data_tseg1 Time segment 1 for data bit timing
   * @param data_tseg2 Time segment 2 for data bit timing
   * @param sjw Synchronization jump width
   * @return true if configuration successful, false otherwise
   */
  virtual bool ConfigureCanFDTiming(uint16_t nominal_prescaler, uint8_t nominal_tseg1,
                                    uint8_t nominal_tseg2, uint16_t data_prescaler,
                                    uint8_t data_tseg1, uint8_t data_tseg2,
                                    uint8_t sjw = 1) noexcept {
    return false; // Not supported by default
  }

  /**
   * @brief Set Transmitter Delay Compensation for CAN-FD.
   * @param tdc_offset Transmitter Delay Compensation Offset
   * @param tdc_filter Transmitter Delay Compensation Filter
   * @return true if TDC set successfully, false otherwise
   */
  virtual bool SetTransmitterDelayCompensation(uint8_t tdc_offset, uint8_t tdc_filter) noexcept {
    return false; // Not supported by default
  }

  /**
   * @brief Get CAN-FD specific capabilities and limits.
   * @param max_data_bytes Maximum data bytes per frame
   * @param max_nominal_baudrate Maximum nominal baudrate
   * @param max_data_baudrate Maximum data baudrate
   * @param supports_brs Supports Bit Rate Switching
   * @param supports_esi Supports Error State Indicator
   * @return true if capabilities retrieved successfully
   */
  virtual bool GetCanFDCapabilities(uint8_t &max_data_bytes, uint32_t &max_nominal_baudrate,
                                    uint32_t &max_data_baudrate, bool &supports_brs,
                                    bool &supports_esi) noexcept {
    return false; // Not supported by default
  }

  /**
   * @brief Send multiple CAN-FD messages in a batch for improved performance.
   * @param messages Array of messages to send
   * @param count Number of messages in array
   * @param timeout_ms Timeout for the entire batch operation
   * @return Number of messages successfully sent
   */
  virtual uint32_t SendMessageBatch(const CanMessage *messages, uint32_t count,
                                    uint32_t timeout_ms = 1000) noexcept {
    uint32_t sent_count = 0;
    for (uint32_t i = 0; i < count; ++i) {
      if (SendMessage(messages[i], timeout_ms)) {
        sent_count++;
      } else {
        break; // Stop on first failure
      }
    }
    return sent_count;
  }

  /**
   * @brief Receive multiple CAN-FD messages in a batch.
   * @param messages Array to store received messages
   * @param max_count Maximum number of messages to receive
   * @param timeout_ms Timeout for the batch operation
   * @return Number of messages actually received
   */
  virtual uint32_t ReceiveMessageBatch(CanMessage *messages, uint32_t max_count,
                                       uint32_t timeout_ms = 100) noexcept {
    uint32_t received_count = 0;
    for (uint32_t i = 0; i < max_count; ++i) {
      if (ReceiveMessage(messages[i], timeout_ms)) {
        received_count++;
      } else {
        break; // Stop on timeout or no more messages
      }
    }
    return received_count;
  }

protected:
  bool initialized_; ///< Initialization state
};
