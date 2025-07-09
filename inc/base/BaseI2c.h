/**
 * @file BaseI2c.h
 * @brief Abstract base class for I2C bus implementations in the HardFOC system.
 * 
 * This header-only file defines the abstract base class for I2C bus communication
 * that provides a consistent API across different I2C controller implementations.
 * Concrete implementations for various microcontrollers inherit from this class.
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//--------------------------------------
//  HardFOC I2C Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC I2C error codes
 * @details Comprehensive error enumeration for all I2C operations in the system.
 *          This enumeration is used across all I2C-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_I2C_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(I2C_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(I2C_ERR_FAILURE, 1, "General failure")                                                         \
  X(I2C_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(I2C_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(I2C_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(I2C_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(I2C_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Bus errors */                                                                                 \
  X(I2C_ERR_BUS_BUSY, 7, "Bus busy")                                                               \
  X(I2C_ERR_BUS_ERROR, 8, "Bus error")                                                             \
  X(I2C_ERR_BUS_ARBITRATION_LOST, 9, "Arbitration lost")                                           \
  X(I2C_ERR_BUS_NOT_AVAILABLE, 10, "Bus not available")                                            \
  X(I2C_ERR_BUS_TIMEOUT, 11, "Bus timeout")                                                        \
  /* Device errors */                                                                              \
  X(I2C_ERR_DEVICE_NOT_FOUND, 12, "Device not found")                                              \
  X(I2C_ERR_DEVICE_NACK, 13, "Device NACK")                                                        \
  X(I2C_ERR_DEVICE_NOT_RESPONDING, 14, "Device not responding")                                    \
  X(I2C_ERR_INVALID_ADDRESS, 15, "Invalid device address")                                         \
  /* Data errors */                                                                                \
  X(I2C_ERR_DATA_TOO_LONG, 16, "Data too long")                                                    \
  X(I2C_ERR_READ_FAILURE, 17, "Read failure")                                                      \
  X(I2C_ERR_WRITE_FAILURE, 18, "Write failure")                                                    \
  X(I2C_ERR_TIMEOUT, 19, "Operation timeout")                                                      \
  /* Hardware errors */                                                                            \
  X(I2C_ERR_HARDWARE_FAULT, 20, "Hardware fault")                                                  \
  X(I2C_ERR_COMMUNICATION_FAILURE, 21, "Communication failure")                                    \
  X(I2C_ERR_VOLTAGE_OUT_OF_RANGE, 22, "Voltage out of range")                                      \
  X(I2C_ERR_CLOCK_STRETCH_TIMEOUT, 23, "Clock stretch timeout")                                    \
  /* Configuration errors */                                                                       \
  X(I2C_ERR_INVALID_CONFIGURATION, 24, "Invalid configuration")                                    \
  X(I2C_ERR_UNSUPPORTED_OPERATION, 25, "Unsupported operation")                                    \
  X(I2C_ERR_INVALID_CLOCK_SPEED, 26, "Invalid clock speed")                                        \
  X(I2C_ERR_PIN_CONFIGURATION_ERROR, 27, "Pin configuration error")                                \
  /* System errors */                                                                              \
  X(I2C_ERR_SYSTEM_ERROR, 28, "System error")                                                      \
  X(I2C_ERR_PERMISSION_DENIED, 29, "Permission denied")                                            \
  X(I2C_ERR_OPERATION_ABORTED, 30, "Operation aborted")

enum class hf_i2c_err_t : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_I2C_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_i2c_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfI2CErrToString(hf_i2c_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_i2c_err_t::NAME:                                                                        \
    return DESC;
    HF_I2C_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @brief I2C operation statistics.
 */
struct hf_i2c_statistics_t {
  uint64_t total_transactions;     ///< Total transactions attempted
  uint64_t successful_transactions; ///< Successful transactions
  uint64_t failed_transactions;    ///< Failed transactions
  uint64_t timeout_count;          ///< Transaction timeouts
  uint64_t bytes_written;          ///< Total bytes written
  uint64_t bytes_read;             ///< Total bytes read
  uint64_t total_transaction_time_us; ///< Total transaction time
  uint32_t max_transaction_time_us;   ///< Longest transaction time
  uint32_t min_transaction_time_us;   ///< Shortest transaction time
  uint32_t nack_errors;            ///< NACK error count
  uint32_t bus_errors;             ///< Bus error count
  uint32_t arbitration_lost_count; ///< Arbitration lost count
  uint32_t clock_stretch_timeouts; ///< Clock stretch timeouts
  uint32_t devices_added;          ///< Devices added to bus
  uint32_t devices_removed;        ///< Devices removed from bus

  hf_i2c_statistics_t() noexcept
      : total_transactions(0), successful_transactions(0), failed_transactions(0),
        timeout_count(0), bytes_written(0), bytes_read(0), total_transaction_time_us(0),
        max_transaction_time_us(0), min_transaction_time_us(UINT32_MAX), nack_errors(0),
        bus_errors(0), arbitration_lost_count(0), clock_stretch_timeouts(0),
        devices_added(0), devices_removed(0) {}
};

/**
 * @brief I2C diagnostic information.
 */
struct hf_i2c_diagnostics_t {
  bool bus_healthy;                           ///< Overall bus health status
  bool sda_line_state;                        ///< Current SDA line state
  bool scl_line_state;                        ///< Current SCL line state
  bool bus_locked;                           ///< Bus lock status
  hf_i2c_err_t last_error_code;                  ///< Last error code encountered
  uint64_t last_error_timestamp_us;          ///< Timestamp of last error
  uint32_t consecutive_errors;               ///< Consecutive error count
  uint32_t error_recovery_attempts;          ///< Bus recovery attempts
  float bus_utilization_percent;             ///< Bus utilization percentage
  uint32_t average_response_time_us;         ///< Average device response time
  uint32_t clock_stretching_events;          ///< Clock stretching event count
  uint32_t active_device_count;              ///< Number of active devices on bus
  uint32_t total_device_scans;               ///< Total device scan operations
  uint32_t devices_found_last_scan;          ///< Devices found in last scan

  hf_i2c_diagnostics_t() noexcept
      : bus_healthy(true), sda_line_state(true), scl_line_state(true), bus_locked(false),
        last_error_code(hf_i2c_err_t::I2C_SUCCESS), last_error_timestamp_us(0), consecutive_errors(0),
        error_recovery_attempts(0), bus_utilization_percent(0.0f), average_response_time_us(0),
        clock_stretching_events(0), active_device_count(0), total_device_scans(0), devices_found_last_scan(0) {}
};

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BaseI2c
 * @brief Abstract base class for I2C bus implementations.
 * @details This class provides a comprehensive I2C bus abstraction that serves as the base
 *          for all I2C implementations in the HardFOC system. It supports:
 *          - Master mode I2C communication
 *          - Standard (100kHz) and Fast (400kHz) modes
 *          - Read, write, and write-then-read operations
 *          - Configurable timeouts and error handling
 *          - Device scanning and presence detection
 *          - Register-based communication utilities
 *          - Lazy initialization pattern
 *
 *          Derived classes implement platform-specific details such as:
 *          - On-chip I2C controllers
 *          - Bit-banged I2C implementations
 *          - I2C bridge or adapter hardware
 *
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 */
class BaseI2c {
public:
  /**
   * @brief Virtual destructor ensures proper cleanup in derived classes.
   */
  virtual ~BaseI2c() noexcept = default;

  // Non-copyable, non-movable (can be changed in derived classes if needed)
  BaseI2c(const BaseI2c &) = delete;
  BaseI2c &operator=(const BaseI2c &) = delete;
  BaseI2c(BaseI2c &&) = delete;
  BaseI2c &operator=(BaseI2c &&) = delete;

  /**
   * @brief Ensures that the I2C bus is initialized (lazy initialization).
   * @return true if the I2C bus is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Ensures that the I2C bus is deinitialized (lazy deinitialization).
   * @return true if the I2C bus is deinitialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
      return !initialized_;
    }
    return true;
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
   * @brief Initialize the I2C bus.
   * @return true if successful, false otherwise
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the I2C bus.
   * @return true if successful, false otherwise
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Write data to an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param data Pointer to data buffer to write
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_i2c_err_t result code
   */
  virtual hf_i2c_err_t Write(uint8_t device_addr, const uint8_t *data, uint16_t length,
                          uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Read data from an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param data Pointer to buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_i2c_err_t result code
   */
  virtual hf_i2c_err_t Read(uint8_t device_addr, uint8_t *data, uint16_t length,
                         uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Write then read data from an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param tx_data Pointer to data buffer to write
   * @param tx_length Number of bytes to write
   * @param rx_data Pointer to buffer to store received data
   * @param rx_length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_i2c_err_t result code
   */
  virtual hf_i2c_err_t WriteRead(uint8_t device_addr, const uint8_t *tx_data, uint16_t tx_length,
                              uint8_t *rx_data, uint16_t rx_length,
                              uint32_t timeout_ms = 0) noexcept = 0;

  //==============================================//
  // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS
  //==============================================//

  /**
   * @brief Open the I2C bus (alias for Initialize).
   * @return true if successful, false otherwise
   */
  virtual bool Open() noexcept {
    return Initialize();
  }

  /**
   * @brief Close the I2C bus (alias for Deinitialize).
   * @return true if successful, false otherwise
   */
  virtual bool Close() noexcept {
    return Deinitialize();
  }

  /**
   * @brief Write data to an I2C device (simplified interface).
   * @param addr 7-bit I2C device address
   * @param data Pointer to data buffer to write
   * @param sizeBytes Number of bytes to write
   * @param timeoutMsec Timeout in milliseconds
   * @return true if successful, false otherwise
   */
  virtual bool Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
                     uint32_t timeoutMsec = 1000) noexcept {
    return Write(addr, data, sizeBytes, timeoutMsec) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Read data from an I2C device (simplified interface).
   * @param addr 7-bit I2C device address
   * @param data Pointer to buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if successful, false otherwise
   */
  virtual bool Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes,
                    uint32_t timeoutMsec = 1000) noexcept {
    return Read(addr, data, sizeBytes, timeoutMsec) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Write then read data from an I2C device (simplified interface).
   * @param addr 7-bit I2C device address
   * @param txData Pointer to data buffer to write
   * @param txSizeBytes Number of bytes to write
   * @param rxData Pointer to buffer to store received data
   * @param rxSizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if successful, false otherwise
   */
  virtual bool WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                         uint16_t rxSizeBytes, uint32_t timeoutMsec = 1000) noexcept {
    return WriteRead(addr, txData, txSizeBytes, rxData, rxSizeBytes, timeoutMsec) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Check if a device is present on the bus.
   * @param device_addr 7-bit I2C device address
   * @return true if device responds, false otherwise
   */
  virtual bool IsDevicePresent(uint8_t device_addr) noexcept {
    // Try to read 1 byte from the device
    uint8_t dummy;
    return Read(device_addr, &dummy, 1, 100) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Scan the I2C bus for devices.
   * @param addresses Array to store found device addresses
   * @param max_addresses Maximum number of addresses to store
   * @return Number of devices found
   */
  virtual uint8_t ScanBus(uint8_t *addresses, uint8_t max_addresses) noexcept {
    uint8_t found_count = 0;
    
    // Scan addresses 0x08 to 0x77 (valid 7-bit addresses)
    for (uint8_t addr = 0x08; addr <= 0x77 && found_count < max_addresses; addr++) {
      if (IsDevicePresent(addr)) {
        addresses[found_count++] = addr;
      }
    }
    
    return found_count;
  }

  /**
   * @brief Write a single byte to an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param data Byte to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteByte(uint8_t device_addr, uint8_t data) noexcept {
    return Write(device_addr, &data, 1) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Read a single byte from an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param data Reference to store the read byte
   * @return true if successful, false otherwise
   */
  virtual bool ReadByte(uint8_t device_addr, uint8_t &data) noexcept {
    return Read(device_addr, &data, 1) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Write to a register on an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param reg_addr Register address
   * @param data Data to write to register
   * @return true if successful, false otherwise
   */
  virtual bool WriteRegister(uint8_t device_addr, uint8_t reg_addr, uint8_t data) noexcept {
    uint8_t buffer[2] = {reg_addr, data};
    return Write(device_addr, buffer, 2) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Read from a register on an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param reg_addr Register address
   * @param data Reference to store the read data
   * @return true if successful, false otherwise
   */
  virtual bool ReadRegister(uint8_t device_addr, uint8_t reg_addr, uint8_t &data) noexcept {
    return WriteRead(device_addr, &reg_addr, 1, &data, 1) == hf_i2c_err_t::I2C_SUCCESS;
  }

  /**
   * @brief Read multiple registers from an I2C device.
   * @param device_addr 7-bit I2C device address
   * @param reg_addr Starting register address
   * @param data Pointer to buffer to store read data
   * @param length Number of registers to read
   * @return true if successful, false otherwise
   */
  virtual bool ReadRegisters(uint8_t device_addr, uint8_t reg_addr, uint8_t *data,
                            uint16_t length) noexcept {
    return WriteRead(device_addr, &reg_addr, 1, data, length) == hf_i2c_err_t::I2C_SUCCESS;
  }

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset I2C operation statistics.
   * @return hf_i2c_err_t::I2C_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_i2c_err_t ResetStatistics() noexcept {
    statistics_ = hf_i2c_statistics_t{}; // Reset statistics to default values
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset I2C diagnostic information.
   * @return hf_i2c_err_t::I2C_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_i2c_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_i2c_diagnostics_t{}; // Reset diagnostics to default values
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get I2C operation statistics
   * @param statistics Reference to store statistics data
   * @return hf_i2c_err_t::I2C_SUCCESS if successful, I2C_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_i2c_err_t GetStatistics(hf_i2c_statistics_t &statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get I2C diagnostic information
   * @param diagnostics Reference to store diagnostics data
   * @return hf_i2c_err_t::I2C_SUCCESS if successful, I2C_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_i2c_err_t GetDiagnostics(hf_i2c_diagnostics_t &diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  
  /**
   * @brief Protected constructor with configuration.
   * @param config I2C bus configuration parameters
   */
  explicit BaseI2c() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}

  bool initialized_;            ///< Initialization status
  hf_i2c_statistics_t statistics_; ///< I2C operation statistics
  hf_i2c_diagnostics_t diagnostics_; ///< I2C diagnostic information
};
