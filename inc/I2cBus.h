/**
 * @file I2cBus.h
 * @brief Declaration of the I2cBus class providing basic I2C master access on
 * ESP32-C6.
 *
 * This driver wraps the ESP-IDF I2C master APIs and does not implement thread
 * safety.  It follows the same design as SpiBus.
 */
#ifndef I2CBUS_H_
#define I2CBUS_H_

#include <cstdint>
#include <driver/i2c.h>

/**
 * @class I2cBus
 * @brief Non-thread-safe I2C master bus abstraction for ESP32-C6 (ESP-IDF).
 */
class I2cBus {
public:
  /**
   * @brief Construct an I2cBus instance.
   * @param port I2C port number (e.g. I2C_NUM_0)
   * @param cfg I2C configuration structure (pins, mode, clock speed)
   */
  I2cBus(i2c_port_t port, const i2c_config_t &cfg) noexcept;

  /**
   * @brief Destructor closes the bus if open.
   */
  ~I2cBus() noexcept;

  /**
   * @brief Open and initialize the I2C port.
   * @return true if open, false otherwise.
   */
  bool Open() noexcept;

  /**
   * @brief Close and de-initialize the I2C port.
   * @return true if closed, false otherwise.
   */
  bool Close() noexcept;

  /**
   * @brief Write data to a slave device.
   * @param addr 7-bit device address
   * @param data Data buffer to transmit
   * @param sizeBytes Number of bytes to write
   * @param timeoutMsec Timeout in milliseconds
   * @return true if transmission succeeded
   */
  bool Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
             uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Read data from a slave device.
   * @param addr 7-bit device address
   * @param data Buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if read succeeded
   */
  bool Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Write then read from a slave device without releasing the bus.
   * @param addr 7-bit device address
   * @param txData Data buffer to send
   * @param txSizeBytes Number of bytes to send
   * @param rxData Buffer to store received data
   * @param rxSizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds
   * @return true if transaction succeeded
   */
  bool WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                 uint16_t rxSizeBytes, uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Get the configured clock speed.
   * @return Clock speed in Hz
   */
  uint32_t GetClockHz() const noexcept;

  /**
   * @brief Check if the bus is initialized.
   */
  bool IsInitialized() const noexcept {
    return initialized;
  }

private:
  i2c_port_t i2cPort;  ///< I2C port
  i2c_config_t config; ///< Configuration copy
  bool initialized;    ///< Initialization flag
};

#endif // I2CBUS_H_
