/**
 * @file SfI2cBus.h
 * @brief Thread-safe I2C master driver with platform-agnostic interface.
 *
 * This class provides thread-safe I2C operations using a platform-agnostic
 * interface. It wraps the I2cBus class and adds mutex protection for
 * multi-threaded environments.
 */
#ifndef SFI2CBUS_H_
#define SFI2CBUS_H_

#include "McuI2c.h"
#include "McuTypes.h"
#include <cstdint>
#include <memory>
#include <mutex>

/**
 * @class SfI2cBus
 * @brief Thread-safe I2C master bus abstraction.
 */
class SfI2cBus {
public:
  /**
   * @brief Constructor for thread-safe I2C bus.
   * @param config I2C configuration
   */
  explicit SfI2cBus(const I2cBusConfig &config) noexcept;

  ~SfI2cBus() noexcept;

  SfI2cBus(const SfI2cBus &) = delete;
  SfI2cBus &operator=(const SfI2cBus &) = delete;

  /**
   * @brief Open and initialize the I2C port.
   */
  bool Open() noexcept;

  /**
   * @brief Close and de-initialize the I2C port.
   */
  bool Close() noexcept;

  /**
   * @brief Write to a device in a thread-safe manner.
   */
  bool Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
             uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Read from a device in a thread-safe manner.
   */
  bool Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Combined write then read operation.
   */
  bool WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                 uint16_t rxSizeBytes, uint32_t timeoutMsec = 1000) noexcept;

  /**
   * @brief Lock the bus for exclusive access.
   */
  bool Lock(uint32_t timeoutMsec = UINT32_MAX) noexcept;

  /**
   * @brief Unlock the bus.
   */
  bool Unlock() noexcept;

  /**
   * @brief Get the clock speed in Hz.
   */
  uint32_t GetClockHz() const noexcept;

  /**
   * @brief Check initialization state.
   */
  bool IsInitialized() const noexcept {
    return initialized_;
  }

private:
  std::unique_ptr<I2cBus> i2c_bus_;
  mutable std::mutex mutex_;
  bool initialized_;
};

#endif // SFI2CBUS_H_
