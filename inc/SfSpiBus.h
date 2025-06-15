/**
 * @file SfSpiBus.h
 * @brief Declaration of the SfSpiBus class for ESP32 (ESP-IDF), providing
 * thread-safe SPI master communication with software-controlled CS.
 *
 * This class abstracts the ESP-IDF SPI master driver and provides thread-safe
 * SPI transactions using FreeRTOS mutexes. All configuration (bus, device,
 * pins, etc.) must be passed in by the caller.
 *
 * @author Nebula Tech Corporation
 * @copyright Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 */

#ifndef SFSPIBUS_H_
#define SFSPIBUS_H_

#include <cstdint>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @class SfSpiBus
 * @brief Thread-safe SPI master bus abstraction for ESP32 (ESP-IDF) with
 * software-controlled CS.
 *
 * This class provides blocking SPI read/write operations with FreeRTOS mutex
 * protection. The CS pin is managed in software (not by the ESP-IDF SPI
 * driver). All configuration (bus, device, pins, etc.) must be passed in by the
 * caller.
 */
class SfSpiBus {
public:
  /**
   * @brief Constructor for SfSpiBus.
   * @param host SPI host device (e.g., SPI2_HOST)
   * @param buscfg SPI bus configuration (pins, etc.)
   * @param devcfg SPI device configuration (CS, mode, etc.)
   * @param mutexHandle FreeRTOS mutex handle for thread safety
   */
  SfSpiBus(spi_host_device_t host, const spi_bus_config_t &buscfg,
           const spi_device_interface_config_t &devcfg,
           SemaphoreHandle_t mutexHandle) noexcept;

  /**
   * @brief Destructor. Closes the SPI bus if initialized.
   */
  ~SfSpiBus() noexcept;

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
   * @param timeoutMsec Timeout in milliseconds for acquiring the mutex
   * @return true if the data was sent successfully, false otherwise
   */
  bool Write(const uint8_t *data, uint16_t sizeBytes,
             uint32_t timeoutMsec) noexcept;

  /**
   * @brief Read a block of data over the SPI bus (blocking, software CS).
   * @param data Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to read
   * @param timeoutMsec Timeout in milliseconds for acquiring the mutex
   * @return true if the data was read successfully, false otherwise
   */
  bool Read(uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec) noexcept;

  /**
   * @brief Write and read a block of data over the SPI bus (full-duplex,
   * blocking, software CS).
   * @param write_data Pointer to the data buffer to send
   * @param read_data Pointer to the buffer to store received data
   * @param sizeBytes Number of bytes to transfer
   * @param timeoutMsec Timeout in milliseconds for acquiring the mutex
   * @return true if the transaction was successful, false otherwise
   */
  bool WriteRead(const uint8_t *write_data, uint8_t *read_data,
                 uint16_t sizeBytes, uint32_t timeoutMsec) noexcept;

  /**
   * @brief Lock the SPI bus for exclusive access.
   * @param timeoutMsec Timeout in milliseconds for acquiring the mutex
   * @return true if the mutex was acquired, false otherwise.
   */
  bool LockBus(uint32_t timeoutMsec = portMAX_DELAY) noexcept;

  /**
   * @brief Unlock the SPI bus.
   * @return true if the mutex was released, false otherwise.
   */
  bool UnlockBus() noexcept;

  /**
   * @brief Get the configured SPI clock frequency in Hz.
   * @return SPI clock frequency in Hz
   */
  uint32_t GetClockHz() const noexcept;

  /**
   * @brief Check if the class is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept { return initialized; }

private:
  bool Initialize() noexcept;
  bool SelectDevice() noexcept;
  bool DeselectDevice() noexcept;

  spi_host_device_t spiHost;               ///< ESP-IDF SPI host
  spi_device_handle_t spiHandle;           ///< ESP-IDF SPI device handle
  spi_bus_config_t busConfig;              ///< SPI bus configuration
  spi_device_interface_config_t devConfig; ///< SPI device configuration
  SemaphoreHandle_t busMutex;              ///< FreeRTOS mutex for thread safety
  bool initialized;                        ///< Initialization state
  gpio_num_t csPin; ///< Chip select GPIO pin (software-controlled)
};

#endif // SFSPIBUS_H_
