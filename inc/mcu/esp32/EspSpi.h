/**
 * @file EspSpi.h
 * @ingroup spi
 * @brief Advanced MCU-integrated SPI controller implementation with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This header provides a comprehensive SPI implementation that utilizes all the advanced
 * features available in ESP-IDF v5.5+ for ESP32C6, including DMA acceleration, octal/quad modes,
 * advanced timing control, multi-device management, power optimization, and comprehensive
 * error handling. The implementation supports both master and slave modes with extensive
 * configuration options for high-performance and low-power applications.
 *
 * Key ESP32C6/ESP-IDF v5.5+ Features Supported:
 * - High-speed SPI Master with DMA support (up to 80MHz)
 * - Multiple clock sources (APB, XTAL) for power optimization
 * - IOMUX optimization for high-frequency operations
 * - Transaction queuing with interrupt and polling modes
 * - Comprehensive error handling and status reporting
 * - Thread-safe multi-device management on single bus
 * - Advanced timing control with input delay compensation
 * - Transaction callbacks for custom handling
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation fully complies with ESP-IDF v5.5 SPI Master driver API
 * @note Supports ESP32C6 hardware features including dual/quad/octal SPI modes
 * @note Thread-safe operations using RTOS mutex protection
 */

#pragma once

#include "BaseSpi.h"
#include "RtosMutex.h"
#include "utils/EspTypes.h"
#include <memory>
#include <vector>

// Suppress pedantic warnings for ESP-IDF headers
#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

// Restore warnings after ESP-IDF headers
#ifdef __cplusplus
#pragma GCC diagnostic pop
#endif

class EspSpiBus;
class EspSpiDevice;

/**
 * @class EspSpiDevice
 * @brief Represents a single SPI device on a bus (CS/config/handle).
 *
 * Inherits from BaseSpi and delegates transfers to the parent bus. Provides full
 * configuration and control for a single SPI device, including DMA, queueing,
 * and advanced ESP-IDF v5.5+ features.
 *
 * @note Thread-safe. All operations are protected by RtosMutex.
 */
class EspSpiDevice : public BaseSpi {
public:
  /**
   * @brief Construct a new EspSpiDevice.
   * @param parent Pointer to the parent EspSpiBus
   * @param config Device configuration (hf_spi_device_config_t)
   */
  EspSpiDevice(EspSpiBus* parent, const hf_spi_device_config_t& config);
  /**
   * @brief Destructor. Automatically deinitializes the device if needed.
   */
  ~EspSpiDevice() noexcept override;

  /**
   * @brief Initialize the SPI device (no-op if already initialized).
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept override;
  /**
   * @brief Deinitialize the SPI device and free resources.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Mark the device as deinitialized without ESP-IDF cleanup.
   * @return true if successful, false otherwise
   * @note ESP-IDF cleanup is handled by the parent bus
   */
  bool MarkAsDeinitialized() noexcept;

  /**
   * @brief Perform a full-duplex SPI transfer.
   * @param tx_data Pointer to transmit buffer (can be nullptr)
   * @param rx_data Pointer to receive buffer (can be nullptr)
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds (0 = default)
   * @return SPI operation result (hf_spi_err_t)
   */
  hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data, hf_u16_t length,
                        hf_u32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Get the device configuration for this SPI device.
   * @return Pointer to device configuration
   */
  const void* GetDeviceConfig() const noexcept override;

  /**
   * @brief Acquire the SPI bus for exclusive use by this device.
   * @param timeout_ms Timeout in milliseconds for acquiring the bus lock (0 = wait indefinitely)
   * @return hf_spi_err_t result code
   * @note Use with ReleaseBus() for back-to-back transactions
   */
  hf_spi_err_t AcquireBus(hf_u32_t timeout_ms) noexcept;

  /**
   * @brief Release the bus lock after operations.
   * @return hf_spi_err_t
   */
  hf_spi_err_t ReleaseBus() noexcept;

  /**
   * @brief Get the ESP-IDF device handle.
   * @return spi_device_handle_t
   */
  spi_device_handle_t GetHandle() const noexcept;
  /**
   * @brief Get the device configuration.
   * @return const hf_spi_device_config_t&
   */
  const hf_spi_device_config_t& GetConfig() const noexcept;

  /**
   * @brief Get the actual clock frequency used by this device.
   * @param actual_freq_hz Reference to store the actual frequency (Hz)
   * @return hf_spi_err_t
   */
  hf_spi_err_t GetActualClockFrequency(hf_u32_t& actual_freq_hz) const noexcept;

private:
  EspSpiBus* parent_bus_;         ///< Parent SPI bus
  spi_device_handle_t handle_;    ///< ESP-IDF device handle
  hf_spi_device_config_t config_; ///< Device configuration
  bool initialized_;              ///< Initialization state
  RtosMutex mutex_;               ///< Thread safety
};

/**
 * @class EspSpiBus
 * @brief Manages a single SPI bus (host). Handles bus init/deinit and device creation.
 *
 * Provides full configuration and control for the SPI bus, including DMA, IOMUX,
 * and advanced ESP-IDF v5.5+ features. Thread-safe device management.
 */
class EspSpiBus {
public:
  /**
   * @brief Construct a new EspSpiBus.
   * @param config Bus configuration (hf_spi_bus_config_t)
   */
  explicit EspSpiBus(const hf_spi_bus_config_t& config) noexcept;
  /**
   * @brief Destructor. Automatically deinitializes the bus if needed.
   */
  ~EspSpiBus() noexcept;

  /**
   * @brief Initialize the SPI bus.
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept;

  /**
   * @brief Check if the bus is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept;

  /**
   * @brief Deinitialize the SPI bus and free resources.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept;

  /**
   * @brief Create a new SPI device on this bus and store it internally.
   * @param device_config Device configuration (hf_spi_device_config_t)
   * @return Index of the created device (use with GetDevice), or -1 on failure
   */
  int CreateDevice(const hf_spi_device_config_t& device_config) noexcept;

  /**
   * @brief Get a device by index.
   * @param device_index Index returned by CreateDevice()
   * @return Pointer to BaseSpi device, or nullptr if invalid index
   */
  BaseSpi* GetDevice(int device_index) noexcept;

  /**
   * @brief Get a device by index (const version).
   * @param device_index Index returned by CreateDevice()
   * @return Const pointer to BaseSpi device, or nullptr if invalid index
   */
  const BaseSpi* GetDevice(int device_index) const noexcept;

  /**
   * @brief Get an ESP-specific device by index.
   * @param device_index Index returned by CreateDevice()
   * @return Pointer to EspSpiDevice, or nullptr if invalid index
   */
  EspSpiDevice* GetEspDevice(int device_index) noexcept;

  /**
   * @brief Get an ESP-specific device by index (const version).
   * @param device_index Index returned by CreateDevice()
   * @return Const pointer to EspSpiDevice, or nullptr if invalid index
   */
  const EspSpiDevice* GetEspDevice(int device_index) const noexcept;

  /**
   * @brief Get number of devices on this bus.
   * @return Number of devices
   */
  std::size_t GetDeviceCount() const noexcept;

  /**
   * @brief Remove a device from the bus.
   * @param device_index Index of device to remove
   * @return true if successful, false otherwise
   */
  bool RemoveDevice(int device_index) noexcept;

  /**
   * @brief Get the bus configuration.
   * @return const hf_spi_bus_config_t&
   */
  const hf_spi_bus_config_t& GetConfig() const noexcept;
  /**
   * @brief Get the ESP-IDF host ID for this bus.
   * @return spi_host_device_t
   */
  spi_host_device_t GetHost() const noexcept;

private:
  hf_spi_bus_config_t config_; ///< Bus configuration
  bool initialized_;           ///< Initialization state
  mutable RtosMutex mutex_;    ///< Thread safety (mutable for const operations)
  std::vector<std::unique_ptr<EspSpiDevice>> devices_; ///< Managed devices on this bus
};
