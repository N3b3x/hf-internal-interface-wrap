/**
 * @file BaseUart.h
 * @brief Abstract base class for UART driver implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for UART communication
 * that provides a consistent API across different UART controller implementations.
 * Concrete implementations for various microcontrollers inherit from this class
 * to provide serial communication, flow control, and data transmission features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 *
 * @example EspUart.h
 * This example demonstrates the ESP32 UART implementation that shows how to use
 * the base UART API with ESP32-specific features and hardware capabilities.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC UART Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC UART error codes
 * @details Comprehensive error enumeration for all UART operations in the system.
 *          This enumeration is used across all UART-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_UART_ERR_LIST(X)                                                                        \
  /* Success codes */                                                                              \
  X(UART_SUCCESS, 0, "Success")                                                                    \
  /* General errors */                                                                             \
  X(UART_ERR_FAILURE, 1, "General failure")                                                        \
  X(UART_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                \
  X(UART_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                        \
  X(UART_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                            \
  X(UART_ERR_NULL_POINTER, 5, "Null pointer")                                                      \
  X(UART_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                    \
  /* Communication errors */                                                                       \
  X(UART_ERR_TIMEOUT, 7, "Operation timeout")                                                      \
  X(UART_ERR_BUFFER_FULL, 8, "Buffer full")                                                        \
  X(UART_ERR_BUFFER_EMPTY, 9, "Buffer empty")                                                      \
  X(UART_ERR_TRANSMISSION_FAILED, 10, "Transmission failed")                                       \
  X(UART_ERR_RECEPTION_FAILED, 11, "Reception failed")                                             \
  /* Frame errors */                                                                               \
  X(UART_ERR_FRAME_ERROR, 12, "Frame error")                                                       \
  X(UART_ERR_PARITY_ERROR, 13, "Parity error")                                                     \
  X(UART_ERR_OVERRUN_ERROR, 14, "Overrun error")                                                   \
  X(UART_ERR_NOISE_ERROR, 15, "Noise error")                                                       \
  X(UART_ERR_BREAK_DETECTED, 16, "Break condition detected")                                       \
  /* Hardware errors */                                                                            \
  X(UART_ERR_HARDWARE_FAULT, 17, "Hardware fault")                                                 \
  X(UART_ERR_COMMUNICATION_FAILURE, 18, "Communication failure")                                   \
  X(UART_ERR_DEVICE_NOT_RESPONDING, 19, "Device not responding")                                   \
  X(UART_ERR_VOLTAGE_OUT_OF_RANGE, 20, "Voltage out of range")                                     \
  /* Configuration errors */                                                                       \
  X(UART_ERR_INVALID_CONFIGURATION, 21, "Invalid configuration")                                   \
  X(UART_ERR_UNSUPPORTED_OPERATION, 22, "Unsupported operation")                                   \
  X(UART_ERR_INVALID_BAUD_RATE, 23, "Invalid baud rate")                                           \
  X(UART_ERR_INVALID_DATA_BITS, 24, "Invalid data bits")                                           \
  X(UART_ERR_INVALID_PARITY, 25, "Invalid parity")                                                 \
  X(UART_ERR_INVALID_STOP_BITS, 26, "Invalid stop bits")                                           \
  X(UART_ERR_PIN_CONFIGURATION_ERROR, 27, "Pin configuration error")                               \
  X(UART_ERR_FLOW_CONTROL_ERROR, 28, "Flow control error")                                         \
  /* System errors */                                                                              \
  X(UART_ERR_SYSTEM_ERROR, 29, "System error")                                                     \
  X(UART_ERR_PERMISSION_DENIED, 30, "Permission denied")                                           \
  X(UART_ERR_OPERATION_ABORTED, 31, "Operation aborted")                                           \
  X(UART_ERR_UNKNOWN, 32, "Unknown error")

enum class hf_uart_err_t : hf_u8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_UART_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_uart_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfUartErrToString(hf_uart_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_uart_err_t::NAME:                                                                        \
    return DESC;
    HF_UART_ERR_LIST(X)
#undef X
  default:
    return HfUartErrToString(hf_uart_err_t::UART_ERR_UNKNOWN);
  }
}

//--------------------------------------
//  UART Configuration Structure
//--------------------------------------

/**
 * @brief UART operation statistics.
 */
struct hf_uart_statistics_t {
  hf_u32_t tx_byte_count;            ///< Total bytes transmitted
  hf_u32_t rx_byte_count;            ///< Total bytes received
  hf_u32_t tx_error_count;           ///< Transmission error count
  hf_u32_t rx_error_count;           ///< Reception error count
  hf_u32_t frame_error_count;        ///< Frame error count
  hf_u32_t parity_error_count;       ///< Parity error count
  hf_u32_t overrun_error_count;      ///< Overrun error count
  hf_u32_t noise_error_count;        ///< Noise error count
  hf_u32_t break_count;              ///< Break condition count
  hf_u32_t timeout_count;            ///< Timeout occurrence count
  hf_u32_t pattern_detect_count;     ///< Pattern detection count
  hf_u32_t wakeup_count;             ///< Wakeup event count
  hf_u64_t last_activity_timestamp;  ///< Last activity timestamp (microseconds)
  hf_u64_t initialization_timestamp; ///< Initialization timestamp (microseconds)

  hf_uart_statistics_t() noexcept
      : tx_byte_count(0), rx_byte_count(0), tx_error_count(0), rx_error_count(0),
        frame_error_count(0), parity_error_count(0), overrun_error_count(0), noise_error_count(0),
        break_count(0), timeout_count(0), pattern_detect_count(0), wakeup_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

/**
 * @brief UART diagnostic information.
 */
struct hf_uart_diagnostics_t {
  hf_uart_err_t last_error;      ///< Last error that occurred
  hf_u32_t consecutive_errors;   ///< Number of consecutive errors
  hf_u32_t error_reset_count;    ///< Number of times error state was reset
  hf_u64_t last_error_timestamp; ///< Timestamp of last error (microseconds)
  bool is_initialized;           ///< Initialization status
  bool is_transmitting;          ///< Transmission status
  bool is_receiving;             ///< Reception status
  bool flow_control_active;      ///< Flow control status
  bool pattern_detection_active; ///< Pattern detection status
  bool wakeup_enabled;           ///< Wakeup status
  hf_u32_t tx_buffer_usage;      ///< TX buffer usage percentage
  hf_u32_t rx_buffer_usage;      ///< RX buffer usage percentage
  hf_u32_t event_queue_usage;    ///< Event queue usage percentage

  hf_uart_diagnostics_t() noexcept
      : last_error(hf_uart_err_t::UART_SUCCESS), consecutive_errors(0), error_reset_count(0),
        last_error_timestamp(0), is_initialized(false), is_transmitting(false), is_receiving(false),
        flow_control_active(false), pattern_detection_active(false), wakeup_enabled(false),
        tx_buffer_usage(0), rx_buffer_usage(0), event_queue_usage(0) {}
};

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BaseUart
 * @brief Abstract base class for UART driver implementations.
 * @details This class provides a comprehensive UART driver abstraction that serves as the base
 *          for all UART implementations in the HardFOC system. It supports:
 *          - Asynchronous serial communication
 *          - Configurable baud rates, data bits, parity, and stop bits
 *          - Hardware flow control (RTS/CTS)
 *          - Buffered TX/RX with configurable buffer sizes
 *          - Blocking and non-blocking I/O operations
 *          - Comprehensive error handling and status reporting
 *          - Printf-style formatted output
 *          - Lazy initialization pattern
 *
 *          Derived classes implement platform-specific details such as:
 *          - On-chip UART controllers
 *          - USB-to-serial adapters
 *          - Wireless serial bridges
 *          - Software UART implementations
 *
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 */
class BaseUart {
public:
  /**
   * @brief Virtual destructor ensures proper cleanup in derived classes.
   */
  virtual ~BaseUart() noexcept = default;

  // Non-copyable, non-movable (can be changed in derived classes if needed)
  BaseUart(const BaseUart&) = delete;
  BaseUart& operator=(const BaseUart&) = delete;
  BaseUart(BaseUart&&) = delete;
  BaseUart& operator=(BaseUart&&) = delete;

  /**
   * @brief Ensures that the UART is initialized (lazy initialization).
   * @return true if the UART is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the UART is deinitialized (lazy deinitialization).
   * @return true if the UART is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
    }
    return !initialized_;
  }

  /**
   * @brief Checks if the driver is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Get the UART port number.
   * @return Port number
   */
  [[nodiscard]] hf_port_num_t GetPort() const noexcept {
    return port_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
  //==============================================//

  /**
   * @brief Initialize the UART driver.
   * @return true if successful, false otherwise
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the UART driver.
   * @return true if successful, false otherwise
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Write data to the UART.
   * @param data Data buffer to transmit
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_uart_err_t result code
   */
  virtual hf_uart_err_t Write(const hf_u8_t* data, hf_u16_t length,
                              hf_u32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Read data from the UART.
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_uart_err_t result code
   */
  virtual hf_uart_err_t Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Get the number of bytes available to read.
   * @return Number of bytes available in the receive buffer
   */
  virtual hf_u16_t BytesAvailable() noexcept = 0;

  /**
   * @brief Flush the transmit buffer.
   * @return hf_uart_err_t result code
   */
  virtual hf_uart_err_t FlushTx() noexcept = 0;

  /**
   * @brief Flush the receive buffer.
   * @return hf_uart_err_t result code
   */
  virtual hf_uart_err_t FlushRx() noexcept = 0;

  /**
   * @brief Printf-style formatted output.
   * @param format Format string
   * @param ... Format arguments
   * @return Number of characters written, or -1 on error
   */
  virtual int Printf(const char* format, ...) noexcept = 0;

  //==============================================//
  // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS
  //==============================================//

  /**
   * @brief Open the UART (alias for Initialize).
   * @return true if successful, false otherwise
   */
  virtual bool Open() noexcept {
    return Initialize();
  }

  /**
   * @brief Close the UART (alias for Deinitialize).
   * @return true if successful, false otherwise
   */
  virtual bool Close() noexcept {
    return Deinitialize();
  }

  // Removed duplicate Write method to avoid overload conflicts

  // Removed duplicate Read method to avoid overload conflicts

  /**
   * @brief Write a string to the UART.
   * @param str String to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteString(const char* str) noexcept {
    if (!str)
      return false;
    return Write(reinterpret_cast<const hf_u8_t*>(str), static_cast<hf_u16_t>(strlen(str))) ==
           hf_uart_err_t::UART_SUCCESS;
  }

  /**
   * @brief Write a single byte to the UART.
   * @param byte Byte to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteByte(hf_u8_t byte) noexcept {
    return Write(&byte, 1) == hf_uart_err_t::UART_SUCCESS;
  }

  // Removed duplicate FlushTx and FlushRx methods to avoid overload conflicts

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset UART operation statistics.
   * @return hf_uart_err_t::UART_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_uart_err_t ResetStatistics() noexcept {
    statistics_ = hf_uart_statistics_t{}; // Reset statistics to default values
    return hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset UART diagnostic information.
   * @return hf_uart_err_t::UART_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_uart_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_uart_diagnostics_t{}; // Reset diagnostics to default values
    return hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get UART operation statistics
   * @param statistics Reference to store statistics data
   * @return hf_uart_err_t::UART_SUCCESS if successful, UART_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_uart_err_t GetStatistics(hf_uart_statistics_t& statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get UART diagnostic information
   * @param diagnostics Reference to store diagnostics data
   * @return hf_uart_err_t::UART_SUCCESS if successful, UART_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_uart_err_t GetDiagnostics(hf_uart_diagnostics_t& diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  /**
   * @brief Protected constructor with port.
   * @param port UART port number
   */
  BaseUart(hf_port_num_t port) noexcept
      : port_(port), initialized_(false), statistics_{}, diagnostics_{} {}

  hf_port_num_t port_;                ///< UART port number
  bool initialized_;                  ///< Initialization status
  hf_uart_statistics_t statistics_;   ///< UART operation statistics
  hf_uart_diagnostics_t diagnostics_; ///< UART diagnostic information
};
