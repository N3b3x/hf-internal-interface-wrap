/**
 * @file BaseCan.h
 * @brief Abstract base class for CAN bus implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for CAN bus communication
 * that provides a consistent API across different CAN controller implementations.
 * Concrete implementations (like McuCan for ESP32 TWAI) inherit from this class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseAdc.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASECAN_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASECAN_H_

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
  /* Message errors */                                                                             \
  X(CAN_ERR_MESSAGE_TIMEOUT, 11, "Message timeout")                                                \
  X(CAN_ERR_MESSAGE_LOST, 12, "Message lost")                                                      \
  X(CAN_ERR_MESSAGE_INVALID, 13, "Invalid message")                                                \
  X(CAN_ERR_MESSAGE_TOO_LONG, 14, "Message too long")                                              \
  X(CAN_ERR_QUEUE_FULL, 15, "Queue full")                                                          \
  X(CAN_ERR_QUEUE_EMPTY, 16, "Queue empty")                                                        \
  /* Hardware errors */                                                                            \
  X(CAN_ERR_HARDWARE_FAULT, 17, "Hardware fault")                                                  \
  X(CAN_ERR_COMMUNICATION_FAILURE, 18, "Communication failure")                                    \
  X(CAN_ERR_DEVICE_NOT_RESPONDING, 19, "Device not responding")                                    \
  X(CAN_ERR_VOLTAGE_OUT_OF_RANGE, 20, "Voltage out of range")                                      \
  /* Configuration errors */                                                                       \
  X(CAN_ERR_INVALID_CONFIGURATION, 21, "Invalid configuration")                                    \
  X(CAN_ERR_UNSUPPORTED_OPERATION, 22, "Unsupported operation")                                    \
  X(CAN_ERR_INVALID_BAUD_RATE, 23, "Invalid baud rate")                                            \
  X(CAN_ERR_FILTER_ERROR, 24, "Filter error")                                                      \
  /* System errors */                                                                              \
  X(CAN_ERR_SYSTEM_ERROR, 25, "System error")                                                      \
  X(CAN_ERR_PERMISSION_DENIED, 26, "Permission denied")                                            \
  X(CAN_ERR_OPERATION_ABORTED, 27, "Operation aborted")

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
 * @details Standard CAN message format compatible with classic CAN implementations.
 *          Supports both standard (11-bit) and extended (29-bit) identifiers.
 */
struct CanMessage {
  uint32_t id;        ///< CAN identifier (11 or 29-bit)
  bool extended_id;   ///< Extended (29-bit) ID flag
  bool remote_frame;  ///< Remote transmission request flag
  uint8_t dlc;        ///< Data length code (0-8)
  uint8_t data[8];    ///< Message data (max 8 bytes for classic CAN)
  uint32_t timestamp; ///< Reception timestamp (implementation-specific)

  CanMessage() noexcept
      : id(0), extended_id(false), remote_frame(false), dlc(0), data{}, timestamp(0) {}

  /**
   * @brief Get maximum data length for classic CAN
   * @return Maximum allowed data length (8 bytes)
   */
  constexpr uint8_t GetMaxDataLength() const noexcept {
    return 8;
  }

  /**
   * @brief Validate DLC for classic CAN
   * @param dlc Data length code to validate
   * @return true if valid (0-8), false otherwise
   */
  static constexpr bool IsValidDLC(uint8_t dlc) noexcept {
    return dlc <= 8;
  }
};

// For backward compatibility with existing code
using hf_can_message_t = CanMessage;

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
 * Concrete implementations Examples:
 * - McuCan: For microcontrollers with built-in CAN peripherals (ESP32 TWAI, STM32 CAN, etc.)
 * - ExtCanCan: For external CAN controllers via SPI (MCP2515, etc.)
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
      initialized_ = Initialize();
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
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the CAN bus.
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Send a CAN/CAN-FD message.
   * @param message Message to send (supports both classic CAN and CAN-FD)
   * @param timeout_ms Timeout in milliseconds (0 = no wait)
   * @return true if sent successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool SendMessage(const CanMessage &message, uint32_t timeout_ms = 1000) noexcept = 0;

  /**
   * @brief Receive a CAN/CAN-FD message.
   * @param message Buffer to store received message
   * @param timeout_ms Timeout in milliseconds (0 = no wait)
   * @return true if received successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool ReceiveMessage(CanMessage &message, uint32_t timeout_ms = 0) noexcept = 0;

  // Backward compatibility methods
  virtual bool SendMessage(const hf_can_message_t &message, uint32_t timeout_ms = 1000) noexcept {
    return SendMessage(static_cast<const CanMessage &>(message), timeout_ms);
  }

  virtual bool ReceiveMessage(hf_can_message_t &message, uint32_t timeout_ms = 0) noexcept {
    return ReceiveMessage(static_cast<CanMessage &>(message), timeout_ms);
  }

  /**
   * @brief Set receive callback for interrupt-driven reception.
   * @param callback Callback function to call when message is received
   * @return true if callback set successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool SetReceiveCallback(CanReceiveCallback callback) noexcept = 0;

  /**
   * @brief Set enhanced CAN-FD receive callback with additional information.
   * @param callback Enhanced callback with reception information
   * @return true if callback set successfully, false otherwise
   * @note Default implementation falls back to standard callback
   */
  virtual bool SetReceiveCallbackFD(CanFdReceiveCallback callback) noexcept {
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
   * @return true if status retrieved successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool GetStatus(CanBusStatus &status) noexcept = 0;

  /**
   * @brief Reset the CAN controller (clear errors, restart).
   * @return true if reset successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Reset() noexcept = 0;

  /**
   * @brief Set acceptance filter for incoming messages.
   * @param id CAN ID to accept
   * @param mask Acceptance mask (0 = don't care bits)
   * @param extended true for extended frames, false for standard
   * @return true if filter set successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept = 0;

  /**
   * @brief Clear all acceptance filters (accept all messages).
   * @return true if cleared successfully, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool ClearAcceptanceFilter() noexcept = 0;

  //==============================================//
  // CAN-FD SPECIFIC METHODS (OPTIONAL)          //
  //==============================================//

  /**
   * @brief Check if CAN-FD is supported by this implementation.
   * @return true if CAN-FD is supported, false otherwise
   * @note Default implementation returns false for backward compatibility
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

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASECAN_H_
