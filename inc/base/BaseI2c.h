/**
 * @file BaseI2c.h
 * @brief Abstract base class for I2C bus implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for I2C bus communication
 * that provides a consistent API across different I2C controller implementations.
 * Concrete implementations (like McuI2cBus for ESP32 I2C) inherit from this class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BaseI2c_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BaseI2c_H_

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

enum class HfI2cErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_I2C_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfI2cErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfI2cErrToString(HfI2cErr err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case HfI2cErr::NAME:                                                                             \
    return DESC;
    HF_I2C_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

//--------------------------------------
//  I2C Configuration Structure
//--------------------------------------

/**
 * @brief Platform-agnostic I2C bus configuration structure.
 * @details Comprehensive configuration for I2C bus initialization,
 *          supporting various platforms and I2C modes without MCU-specific types.
 */
struct I2cBusConfig {
  HfPortNumber port;            ///< I2C port number
  HfPinNumber sda_pin;          ///< SDA pin number
  HfPinNumber scl_pin;          ///< SCL pin number
  HfFrequencyHz clock_speed_hz; ///< Clock speed in Hz (typically 100kHz or 400kHz)
  bool enable_pullups;          ///< Enable internal pull-up resistors
  HfTimeoutMs timeout_ms;       ///< Default timeout for operations in milliseconds
  uint16_t tx_buffer_size;      ///< Transmit buffer size (0 = no buffer/blocking mode)
  uint16_t rx_buffer_size;      ///< Receive buffer size (0 = no buffer/blocking mode)

  /**
   * @brief Default constructor with sensible defaults.
   */
  I2cBusConfig() noexcept
      : port(HF_INVALID_PORT), sda_pin(HF_INVALID_PIN), scl_pin(HF_INVALID_PIN),
        clock_speed_hz(100000),                                    // 100kHz standard mode
        enable_pullups(true), timeout_ms(1000), tx_buffer_size(0), // Blocking mode by default
        rx_buffer_size(0) {}
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
 *          Derived classes implement platform-specific details for:
 *          - MCU I2C controllers (ESP32, STM32, etc.)
 *          - Bit-banged I2C implementations
 *          - I2C bridge/adapter hardware
 *
 * @note This is a header-only abstract base class - instantiate concrete implementations instead.
 * @note This class is not inherently thread-safe. Use appropriate synchronization if
 *       accessed from multiple contexts.
 */
class BaseI2c {
public:
  /**
   * @brief Constructor with configuration.
   * @param config I2C bus configuration parameters
   */
  explicit BaseI2c(const I2cBusConfig &config) noexcept : config_(config), initialized_(false) {}

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
   * @brief Checks if the bus is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Get the bus configuration.
   * @return Reference to the current configuration
   */
  [[nodiscard]] const I2cBusConfig &GetConfig() const noexcept {
    return config_;
  }

  //==============================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
  //==============================================//

  /**
   * @brief Initialize the I2C bus.
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the I2C bus.
   * @return true if successful, false otherwise
   * @note Must be implemented by concrete classes.
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Write data to a slave device.
   * @param device_addr 7-bit device address
   * @param data Data buffer to transmit
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return HfI2cErr result code
   * @note Must be implemented by concrete classes.
   */
  virtual HfI2cErr Write(uint8_t device_addr, const uint8_t *data, uint16_t length,
                         uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Read data from a slave device.
   * @param device_addr 7-bit device address
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return HfI2cErr result code
   * @note Must be implemented by concrete classes.
   */
  virtual HfI2cErr Read(uint8_t device_addr, uint8_t *data, uint16_t length,
                        uint32_t timeout_ms = 0) noexcept = 0;

  /**
   * @brief Write then read from a slave device without releasing the bus.
   * @param device_addr 7-bit device address
   * @param tx_data Data buffer to send
   * @param tx_length Number of bytes to send
   * @param rx_data Buffer to store received data
   * @param rx_length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return HfI2cErr result code
   * @note Must be implemented by concrete classes.
   */
  virtual HfI2cErr WriteRead(uint8_t device_addr, const uint8_t *tx_data, uint16_t tx_length,
                             uint8_t *rx_data, uint16_t rx_length,
                             uint32_t timeout_ms = 0) noexcept = 0;

  //==============================================//
  // CONVENIENCE METHODS WITH DEFAULT IMPLEMENTATIONS //
  //==============================================//

  /**
   * @brief Legacy compatibility: Open and initialize the I2C port.
   * @return true if open, false otherwise.
   */
  virtual bool Open() noexcept {
    return EnsureInitialized();
  }

  /**
   * @brief Legacy compatibility: Close and de-initialize the I2C port.
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
   * @brief Legacy compatibility: Write with boolean return.
   * @param addr 7-bit device address
   * @param data Data buffer to transmit
   * @param sizeBytes Number of bytes to write
   * @param timeoutMsec Timeout in milliseconds
   * @return true if transmission succeeded
   */
  virtual bool Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
                     uint32_t timeoutMsec = 1000) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return Write(addr, data, sizeBytes, timeoutMsec) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Legacy compatibility: Read with boolean return.
   * @param addr 7-bit device address
   * @param data Buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if read succeeded
   */
  virtual bool Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes,
                    uint32_t timeoutMsec = 1000) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return Read(addr, data, sizeBytes, timeoutMsec) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Legacy compatibility: WriteRead with boolean return.
   * @param addr 7-bit device address
   * @param txData Data buffer to send
   * @param txSizeBytes Number of bytes to send
   * @param rxData Buffer to store received data
   * @param rxSizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if transaction succeeded
   */
  virtual bool WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                         uint16_t rxSizeBytes, uint32_t timeoutMsec = 1000) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    return WriteRead(addr, txData, txSizeBytes, rxData, rxSizeBytes, timeoutMsec) ==
           HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Get the configured clock speed.
   * @return Clock speed in Hz
   */
  [[nodiscard]] virtual uint32_t GetClockHz() const noexcept {
    return config_.clock_speed_hz;
  }

  /**
   * @brief Check if a device is present at given address.
   * @param device_addr 7-bit device address
   * @return true if device responds, false otherwise
   */
  virtual bool IsDevicePresent(uint8_t device_addr) noexcept {
    if (!EnsureInitialized()) {
      return false;
    }
    // Try to write 0 bytes to the device - this will generate only the address
    return Write(device_addr, nullptr, 0) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Scan for devices on the I2C bus.
   * @param addresses Buffer to store found device addresses
   * @param max_addresses Maximum number of addresses to find
   * @return Number of devices found
   */
  virtual uint8_t ScanBus(uint8_t *addresses, uint8_t max_addresses) noexcept {
    if (!EnsureInitialized() || !addresses || max_addresses == 0) {
      return 0;
    }

    uint8_t found = 0;
    // Scan 7-bit addresses from 0x08 to 0x77 (avoiding reserved addresses)
    for (uint8_t addr = 0x08; addr <= 0x77 && found < max_addresses; ++addr) {
      if (IsDevicePresent(addr)) {
        addresses[found++] = addr;
      }
    }
    return found;
  }

  /**
   * @brief Write single byte to device.
   * @param device_addr 7-bit device address
   * @param data Byte to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteByte(uint8_t device_addr, uint8_t data) noexcept {
    return Write(device_addr, &data, 1) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Read single byte from device.
   * @param device_addr 7-bit device address
   * @param data Output: byte read
   * @return true if successful, false otherwise
   */
  virtual bool ReadByte(uint8_t device_addr, uint8_t &data) noexcept {
    return Read(device_addr, &data, 1) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Write to register (register address + data).
   * @param device_addr 7-bit device address
   * @param reg_addr Register address
   * @param data Data to write
   * @return true if successful, false otherwise
   */
  virtual bool WriteRegister(uint8_t device_addr, uint8_t reg_addr, uint8_t data) noexcept {
    uint8_t buffer[2] = {reg_addr, data};
    return Write(device_addr, buffer, 2) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Read from register.
   * @param device_addr 7-bit device address
   * @param reg_addr Register address
   * @param data Output: data read
   * @return true if successful, false otherwise
   */
  virtual bool ReadRegister(uint8_t device_addr, uint8_t reg_addr, uint8_t &data) noexcept {
    return WriteRead(device_addr, &reg_addr, 1, &data, 1) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Read multiple bytes from register.
   * @param device_addr 7-bit device address
   * @param reg_addr Register address
   * @param data Buffer to store data
   * @param length Number of bytes to read
   * @return true if successful, false otherwise
   */
  virtual bool ReadRegisters(uint8_t device_addr, uint8_t reg_addr, uint8_t *data,
                             uint16_t length) noexcept {
    return WriteRead(device_addr, &reg_addr, 1, data, length) == HfI2cErr::I2C_SUCCESS;
  }

  /**
   * @brief Get port number.
   * @return I2C port number
   */
  [[nodiscard]] virtual HfPortNumber GetPort() const noexcept {
    return config_.port;
  }

protected:
  I2cBusConfig config_; ///< Bus configuration
  bool initialized_;    ///< Initialization state
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BaseI2c_H_
