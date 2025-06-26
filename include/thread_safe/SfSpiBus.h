/**
 * @file SfSpiBus.h
 * @brief Declaration of the SfSpiBus class, providing thread-safe SPI master
 * communication with software-controlled CS.
 *
 * This class abstracts the SPI master driver and provides thread-safe SPI
 * transactions using a standard C++ mutex. All configuration (bus, device,
 * pins, etc.) must be passed in by the caller.
 *
 * @author Nebula Tech Corporation
 * @copyright Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 */

#ifndef SFSPIBUS_H_
#define SFSPIBUS_H_

#include "../mcu/McuSpiBus.h"
#include "../mcu/McuTypes.h"
#include <cstdint>
#include <memory>
#include <mutex>

/**
 * @class SfSpiBus
 * @brief Thread-safe SPI master bus abstraction.
 *
 * This class provides blocking SPI read/write operations with mutex protection.
 * The CS pin is managed in software (not by the SPI driver). All configuration
 * (bus, device, pins, etc.) must be passed in by the caller.
 */
class SfSpiBus {
public:
  /**
   * @brief Constructor for thread-safe SPI bus.
   * @param config SPI bus configuration
   */
  explicit SfSpiBus(const SpiBusConfig &config) noexcept;

  /**
   * @brief Destructor. Closes the SPI bus if initialized.
   */
  ~SfSpiBus() noexcept;

  SfSpiBus(const SfSpiBus &) = delete;
  SfSpiBus &operator=(const SfSpiBus &) = delete;

  /**
   * @brief Open and initialize the SPI bus and device.
   * @return true if open, false otherwise.
   */
  bool Open() noexcept;

  /**
   * @brief Close and de-initialize the SPI bus and device.
   * @return true if closed, false otherwise.
   */
  bool Close() noexcept;

  /**
   * @brief Write a block of data over the SPI bus (blocking, software CS).
   * @param data Pointer to the data buffer to send
   * @param sizeBytes Number of bytes to send
   * @return true if the data was sent successfully, false otherwise
   */
  bool Write(const uint8_t *data, uint16_t sizeBytes) noexcept;

  /**
   * @brief Read a block of data over the SPI bus (blocking, software CS).
   * @param data Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @return true if the data was read successfully, false otherwise
   */
  bool Read(uint8_t *data, uint16_t sizeBytes) noexcept;

  /**
   * @brief Write and read a block of data over the SPI bus (full-duplex,
   * blocking, software CS).
   * @param write_data Pointer to the data buffer to send
   * @param read_data Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to transfer
   * @return true if the transaction was successful, false otherwise
   */
  bool WriteRead(const uint8_t *write_data, uint8_t *read_data, uint16_t sizeBytes) noexcept;

  /**
   * @brief Lock the SPI bus for exclusive access.
   * @param timeoutMsec Timeout in milliseconds for acquiring the mutex
   * @return true if the mutex was acquired, false otherwise.
   */
  bool Lock(uint32_t timeoutMsec = UINT32_MAX) noexcept;

  /**
   * @brief Unlock the SPI bus.
   * @return true if the mutex was released, false otherwise.
   */
  bool Unlock() noexcept;

  /**
   * @brief Get the configured SPI clock frequency in Hz.
   * @return SPI clock frequency in Hz
   */
  uint32_t GetClockHz() const noexcept;

  /**
   * @brief Check if the class is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Get the SPI bus configuration.
   * @return const reference to the SPI bus configuration
   */
  const SpiBusConfig &GetConfig() const noexcept {
    return *spi_bus_;
  }

private:
  std::unique_ptr<SpiBus> spi_bus_;        ///< Underlying SPI bus implementation
  mutable std::mutex mutex_;              ///< Mutex for thread safety
  bool initialized_;                      ///< Initialization state
};

#endif // SFSPIBUS_H_
