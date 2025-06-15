/**
 * @file SpiBus.h
 * @brief Declaration of the SpiBus class for ESP32 (ESP-IDF), providing
 * non-thread-safe SPI master communication.
 *
 * This class abstracts the ESP-IDF SPI master driver and provides blocking SPI
 * transactions. All configuration (bus, device, pins, etc.) must be passed in
 * by the caller.
 *
 * @author Nebula Tech Corporation
 * @copyright Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 */

#ifndef SPIBUS_H_
#define SPIBUS_H_

#include <cstdint>
#include <driver/spi_master.h>

/**
 * @class SpiBus
 * @brief Non-thread-safe SPI master bus abstraction for ESP32 (ESP-IDF).
 *
 * This class provides blocking SPI read/write operations. The CS pin is managed
 * by the ESP-IDF SPI driver. All configuration (bus, device, pins, etc.) must
 * be passed in by the caller.
 */
class SpiBus {
public:
  /**
   * @brief Constructor for SpiBus.
   * @param host SPI host device (e.g., SPI2_HOST)
   * @param buscfg SPI bus configuration (pins, etc.)
   * @param devcfg SPI device configuration (CS, mode, etc.)
   */
  SpiBus(spi_host_device_t host, const spi_bus_config_t &buscfg,
         const spi_device_interface_config_t &devcfg) noexcept;

  /**
   * @brief Destructor. Closes the SPI bus if initialized.
   */
  ~SpiBus() noexcept;

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
   * @brief Write a block of data over the SPI bus (blocking).
   * @param data Pointer to the data buffer to send
   * @param sizeBytes Number of bytes to send
   * @param timeoutMsec Timeout in milliseconds (not used, for API
   * compatibility)
   * @return true if the data was sent successfully, false otherwise
   */
  bool Write(const uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec = 0) noexcept;

  /**
   * @brief Read a block of data over the SPI bus (blocking).
   * @param data Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds (not used, for API
   * compatibility)
   * @return true if the data was read successfully, false otherwise
   */
  bool Read(uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec = 0) noexcept;

  /**
   * @brief Write and read a block of data over the SPI bus (full-duplex,
   * blocking).
   * @param tx Pointer to the data buffer to send
   * @param rx Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to transfer
   * @param timeoutMsec Timeout in milliseconds (not used, for API
   * compatibility)
   * @return true if the transaction was successful, false otherwise
   */
  bool WriteRead(const uint8_t *tx, uint8_t *rx, uint16_t sizeBytes,
                 uint32_t timeoutMsec = 0) noexcept;

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
    return initialized;
  }

private:
  spi_host_device_t spiHost;               ///< ESP-IDF SPI host
  spi_device_handle_t spiHandle;           ///< ESP-IDF SPI device handle
  spi_bus_config_t busConfig;              ///< SPI bus configuration
  spi_device_interface_config_t devConfig; ///< SPI device configuration
  bool initialized;                        ///< Initialization state
};

#endif // SPIBUS_H_
