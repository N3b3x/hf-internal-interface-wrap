/**
 * @file BaseSpi.h
 * @brief Abstract base class for SPI device implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for SPI device communication
 * that provides a consistent API across different SPI controller implementations.
 * Concrete implementations for various microcontrollers inherit from this class
 * to provide high-speed serial communication and transfer management.
 * Each BaseSpi instance represents a single SPI device with pre-configured settings.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 * @note Each BaseSpi instance represents a specific SPI device, not the SPI bus itself.
 *
 * @example SpiComprehensiveTest.cpp
 * This example demonstrates comprehensive SPI testing including device communication,
 * transfer management, and hardware-specific capabilities for ESP32-C6.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC SPI Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC SPI error codes
 * @details Comprehensive error enumeration for all SPI operations in the system.
 *          This enumeration is used across all SPI-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_SPI_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(SPI_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(SPI_ERR_FAILURE, 1, "General failure")                                                         \
  X(SPI_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(SPI_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(SPI_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(SPI_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(SPI_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Bus errors */                                                                                 \
  X(SPI_ERR_BUS_BUSY, 7, "Bus busy")                                                               \
  X(SPI_ERR_BUS_ERROR, 8, "Bus error")                                                             \
  X(SPI_ERR_BUS_NOT_AVAILABLE, 9, "Bus not available")                                             \
  X(SPI_ERR_BUS_TIMEOUT, 10, "Bus timeout")                                                        \
  /* Transfer errors */                                                                            \
  X(SPI_ERR_TRANSFER_FAILED, 11, "Transfer failed")                                                \
  X(SPI_ERR_TRANSFER_TIMEOUT, 12, "Transfer timeout")                                              \
  X(SPI_ERR_TRANSFER_TOO_LONG, 13, "Transfer too long")                                            \
  X(SPI_ERR_TRANSFER_SIZE_MISMATCH, 14, "Transfer size mismatch")                                  \
  /* Device errors */                                                                              \
  X(SPI_ERR_DEVICE_NOT_FOUND, 15, "Device not found")                                              \
  X(SPI_ERR_DEVICE_NOT_RESPONDING, 16, "Device not responding")                                    \
  X(SPI_ERR_CS_CONTROL_FAILED, 17, "Chip select control failed")                                   \
  /* Hardware errors */                                                                            \
  X(SPI_ERR_HARDWARE_FAULT, 18, "Hardware fault")                                                  \
  X(SPI_ERR_COMMUNICATION_FAILURE, 19, "Communication failure")                                    \
  X(SPI_ERR_VOLTAGE_OUT_OF_RANGE, 20, "Voltage out of range")                                      \
  X(SPI_ERR_CLOCK_ERROR, 21, "Clock error")                                                        \
  /* Configuration errors */                                                                       \
  X(SPI_ERR_INVALID_CONFIGURATION, 22, "Invalid configuration")                                    \
  X(SPI_ERR_UNSUPPORTED_OPERATION, 23, "Unsupported operation")                                    \
  X(SPI_ERR_INVALID_CLOCK_SPEED, 24, "Invalid clock speed")                                        \
  X(SPI_ERR_INVALID_MODE, 25, "Invalid SPI mode")                                                  \
  X(SPI_ERR_PIN_CONFIGURATION_ERROR, 26, "Pin configuration error")                                \
  /* System errors */                                                                              \
  X(SPI_ERR_SYSTEM_ERROR, 27, "System error")                                                      \
  X(SPI_ERR_PERMISSION_DENIED, 28, "Permission denied")                                            \
  X(SPI_ERR_OPERATION_ABORTED, 29, "Operation aborted")                                            \
  X(SPI_ERR_UNKNOWN, 30, "Unknown error")

enum class hf_spi_err_t : hf_u8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_SPI_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_spi_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfSpiErrToString(hf_spi_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_spi_err_t::NAME:                                                                         \
    return DESC;
    HF_SPI_ERR_LIST(X)
#undef X
  default:
    return HfSpiErrToString(hf_spi_err_t::SPI_ERR_UNKNOWN);
  }
}

/**
 * @brief SPI operation statistics.
 */
struct hf_spi_statistics_t {
  hf_u32_t total_transactions;       ///< Total number of transactions
  hf_u32_t successful_transactions;  ///< Number of successful transactions
  hf_u32_t failed_transactions;      ///< Number of failed transactions
  hf_u32_t timeout_transactions;     ///< Number of timed-out transactions
  hf_u32_t total_bytes_sent;         ///< Total bytes transmitted
  hf_u32_t total_bytes_received;     ///< Total bytes received
  hf_u32_t max_transaction_time_us;  ///< Maximum transaction time (microseconds)
  hf_u32_t min_transaction_time_us;  ///< Minimum transaction time (microseconds)
  hf_u64_t last_activity_timestamp;  ///< Last activity timestamp
  hf_u64_t initialization_timestamp; ///< Initialization timestamp

  hf_spi_statistics_t() noexcept
      : total_transactions(0), successful_transactions(0), failed_transactions(0),
        timeout_transactions(0), total_bytes_sent(0), total_bytes_received(0),
        max_transaction_time_us(0), min_transaction_time_us(0xFFFFFFFF), last_activity_timestamp(0),
        initialization_timestamp(0) {}
};

/**
 * @brief SPI diagnostic information.
 */
struct hf_spi_diagnostics_t {
  bool is_initialized;          ///< Initialization state
  bool is_bus_suspended;        ///< Bus suspension state
  bool dma_enabled;             ///< DMA enabled state
  hf_u32_t current_clock_speed; ///< Current clock speed in Hz
  hf_u8_t current_mode;         ///< Current SPI mode
  hf_u16_t max_transfer_size;   ///< Maximum transfer size
  hf_u8_t device_count;         ///< Number of registered devices
  hf_u32_t last_error;          ///< Last error code
  hf_u64_t total_transactions;  ///< Total transactions performed
  hf_u64_t failed_transactions; ///< Failed transactions count

  hf_spi_diagnostics_t() noexcept
      : is_initialized(false), is_bus_suspended(false), dma_enabled(false), current_clock_speed(0),
        current_mode(0), max_transfer_size(0), device_count(0), last_error(0),
        total_transactions(0), failed_transactions(0) {}
};

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BaseSpi
 * @brief Abstract base class for SPI device implementations.
 * @details This class provides a comprehensive SPI device abstraction that serves as the base
 *          for all SPI device implementations in the HardFOC system. Each instance represents
 *          a single SPI device with pre-configured settings. It supports:
 *          - Master mode SPI communication
 *          - Configurable SPI modes (0-3)
 *          - Full-duplex, write-only, and read-only transfers
 *          - Configurable clock speeds and timing
 *          - Automatic chip select management
 *          - Configurable word sizes
 *          - Comprehensive error handling
 *          - Lazy initialization pattern
 *
 *          Device configuration (mode, speed, CS pin) is set during device creation
 *          and managed automatically, ensuring each device operates with its
 *          correct settings without manual configuration per transaction.
 *
 *          Derived classes implement platform-specific details such as:
 *          - On-chip SPI controllers with device handles
 *          - SPI bridge or adapter hardware
 *          - Device-specific configurations
 *
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 * @note Each BaseSpi instance represents a specific SPI device, not the SPI bus itself.
 */
class BaseSpi {
public:
  /**
   * @brief Virtual destructor ensures proper cleanup in derived classes.
   */
  virtual ~BaseSpi() noexcept = default;

  // Non-copyable, non-movable (can be changed in derived classes if needed)
  BaseSpi(const BaseSpi&) = delete;
  BaseSpi& operator=(const BaseSpi&) = delete;
  BaseSpi(BaseSpi&&) = delete;
  BaseSpi& operator=(BaseSpi&&) = delete;

  /**
   * @brief Ensures that the SPI bus is initialized (lazy initialization).
   * @return true if the SPI bus is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the SPI bus is deinitialized (lazy deinitialization).
   * @return true if the SPI bus is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
    }
    return !initialized_;
  }

  /**
   * @brief Checks if the bus is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
  //==============================================//

  /**
   * @brief Initialize the SPI bus.
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the SPI bus.
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Perform a full-duplex SPI transfer.
   * @param tx_data Transmit data buffer (can be nullptr for read-only)
   * @param rx_data Receive data buffer (can be nullptr for write-only)
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_spi_err_t result code
   * @note Must be implemented by concrete classes.
   * @note Chip select is managed automatically by the device implementation.
   */
  virtual hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data, hf_u16_t length,
                                hf_u32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Get the device configuration for this SPI device.
   * @return Device-specific configuration information
   */
  virtual const void* GetDeviceConfig() const noexcept = 0;

  //==============================================//
  // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS //
  //==============================================//

  /**
   * @brief Legacy compatibility: Open and initialize the SPI bus.
   * @return true if open, false otherwise.
   */
  virtual bool Open() noexcept {
    return EnsureInitialized();
  }

  /**
   * @brief Legacy compatibility: Close and de-initialize the SPI bus.
   * @return true if closed, false otherwise.
   */
  virtual bool Close() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
      return !initialized_;
    }
    return true;
  }

  /**
   * @brief Legacy compatibility: Transfer with boolean return.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @return true if transfer succeeded
   */
  virtual bool Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data, hf_u16_t length) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return Transfer(tx_data, rx_data, length, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  /**
   * @brief Write data to SPI bus.
   * @param data Data buffer to transmit
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_spi_err_t result code
   */
  virtual hf_spi_err_t Write(const hf_u8_t* data, hf_u16_t length,
                             hf_u32_t timeout_ms = 0) noexcept {
    return Transfer(data, nullptr, length, timeout_ms);
  }

  /**
   * @brief Read data from SPI bus.
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_spi_err_t result code
   */
  virtual hf_spi_err_t Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept {
    return Transfer(nullptr, data, length, timeout_ms);
  }

  /**
   * @brief Legacy compatibility: Write with boolean return.
   * @param data Data buffer to transmit
   * @param length Number of bytes to write
   * @return true if write succeeded
   */
  virtual bool Write(const hf_u8_t* data, hf_u16_t length) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return Write(data, length, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  /**
   * @brief Legacy compatibility: Read with boolean return.
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @return true if read succeeded
   */
  virtual bool Read(hf_u8_t* data, hf_u16_t length) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return Read(data, length, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  /**
   * @brief Write single byte to SPI bus.
   * @param data Byte to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteByte(hf_u8_t data) noexcept {
    return Write(&data, 1, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  /**
   * @brief Read single byte from SPI bus.
   * @param data Output: byte read
   * @return true if successful, false otherwise
   */
  virtual bool ReadByte(hf_u8_t& data) noexcept {
    return Read(&data, 1, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  /**
   * @brief Write single byte and read response.
   * @param tx_data Byte to write
   * @param rx_data Output: byte read
   * @return true if successful, false otherwise
   */
  virtual bool TransferByte(hf_u8_t tx_data, hf_u8_t& rx_data) noexcept {
    return Transfer(&tx_data, &rx_data, 1, 0) == hf_spi_err_t::SPI_SUCCESS;
  }

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset SPI operation statistics.
   * @return hf_spi_err_t::SPI_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_spi_err_t ResetStatistics() noexcept {
    statistics_ = hf_spi_statistics_t{}; // Reset statistics to default values
    return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset SPI diagnostic information.
   * @return hf_spi_err_t::SPI_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_spi_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_spi_diagnostics_t{}; // Reset diagnostics to default values
    return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get SPI operation statistics
   * @param statistics Reference to store statistics data
   * @return hf_spi_err_t::SPI_SUCCESS if successful, SPI_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_spi_err_t GetStatistics(hf_spi_statistics_t& statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get SPI diagnostic information
   * @param diagnostics Reference to store diagnostics data
   * @return hf_spi_err_t::SPI_SUCCESS if successful, SPI_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_spi_err_t GetDiagnostics(hf_spi_diagnostics_t& diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  /**
   * @brief Protected default constructor.
   * @note Configurations are handled by derived classes
   */
  BaseSpi() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}

  bool initialized_;                 ///< Initialization state
  hf_spi_statistics_t statistics_;   ///< SPI operation statistics
  hf_spi_diagnostics_t diagnostics_; ///< SPI diagnostic information
};
