#ifndef ESP_I2C_H_
#define ESP_I2C_H_

/**
 * @file EspI2c.h
 * @brief Advanced ESP32-integrated I2C controller for ESP-IDF v5.5+ with proper bus-device
 * architecture.
 *
 * This header provides a comprehensive I2C implementation that properly utilizes the ESP-IDF v5.5+
 * bus-device model, following the same pattern as the SPI implementation. The architecture
 * separates bus management from device operations, providing clean abstraction and optimal resource
 * management.
 *
 * @section features ESP32C6/ESP-IDF v5.5+ Features Supported:
 * - **Bus-Device Architecture**: Separate EspI2cBus and EspI2cDevice classes
 * - **Modern API**: Uses i2c_new_master_bus() and i2c_master_bus_add_device()
 * - **Per-Device Configuration**: Each device has its own clock speed and settings
 * - **Thread Safety**: Full RTOS integration with proper synchronization
 * - **Device Management**: Dynamic device addition/removal with proper cleanup
 * - **BaseI2c Integration**: EspI2cDevice inherits from BaseI2c for portability
 * - **Comprehensive Error Handling**: Proper error conversion and reporting
 * - **Resource Management**: Automatic cleanup and proper resource lifecycle
 *
 * @section performance Performance Characteristics:
 * - Standard Mode: 100 kHz
 * - Fast Mode: 400 kHz
 * - Fast Mode Plus: 1 MHz (ESP32C6)
 * - 7-bit and 10-bit addressing support
 * - Clock stretching with configurable timeout
 * - Hardware FIFO utilization
 * - Multi-master operation capability
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * @version 3.0.0 - Complete bus-device architecture rewrite
 *
 * @note This implementation requires ESP-IDF v5.5+ and is optimized for ESP32C6.
 * @note Thread-safe operation is guaranteed for all public methods.
 * @note Follows the same architectural pattern as EspSpi for consistency.
 *
 * @example Basic Usage:
 * @code
 * // Create bus configuration
 * hf_i2c_master_bus_config_t bus_config = {};
 * bus_config.i2c_port = 0;
 * bus_config.sda_io_num = 21;
 * bus_config.scl_io_num = 22;
 * bus_config.enable_internal_pullup = true;
 *
 * // Create I2C bus
 * EspI2cBus i2c_bus(bus_config);
 * if (!i2c_bus.Initialize()) {
 *     // Handle initialization error
 * }
 *
 * // Create device configuration
 * hf_i2c_device_config_t device_config = {};
 * device_config.device_address = 0x48;
 * device_config.scl_speed_hz = 400000;
 * device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
 *
 * // Add device to bus
 * int device_index = i2c_bus.CreateDevice(device_config);
 * BaseI2c* device = i2c_bus.GetDevice(device_index);
 *
 * // Use device for I2C operations
 * uint8_t data[] = {0x10, 0x20, 0x30};
 * hf_i2c_err_t result = device->Write(data, sizeof(data));
 * @endcode
 */

#pragma once

#include "BaseI2c.h"
#include "utils/EspTypes.h"
#include "utils/RtosMutex.h"
#include <memory>
#include <vector>

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif
#include "driver/i2c_master.h"
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

// Forward declarations
class EspI2cBus;
class EspI2cDevice;

/**
 * @class EspI2cDevice
 * @brief Represents a single I2C device on a bus.
 *
 * Inherits from BaseI2c and delegates operations to the parent bus. Provides full
 * configuration and control for a single I2C device, including per-device clock speed,
 * addressing mode, and ESP-IDF v5.5+ features.
 *
 * @note Thread-safe. All operations are protected by RtosMutex.
 * @note Each device maintains its own handle and configuration.
 */
class EspI2cDevice : public BaseI2c {
public:
  /**
   * @brief Construct a new EspI2cDevice.
   * @param parent Pointer to the parent EspI2cBus
   * @param handle ESP-IDF device handle
   * @param config Device configuration
   */
  EspI2cDevice(EspI2cBus* parent, i2c_master_dev_handle_t handle,
               const hf_i2c_device_config_t& config);

  /**
   * @brief Destructor. Automatically removes device from bus if needed.
   */
  ~EspI2cDevice() noexcept override;

  /**
   * @brief Initialize the I2C device (no-op if already initialized).
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the I2C device and free resources.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Write data to the I2C device.
   * @param data Pointer to data buffer to write
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = default)
   * @return I2C operation result
   * @note Device address is configured during device creation
   */
  hf_i2c_err_t Write(const hf_u8_t* data, hf_u16_t length,
                     hf_u32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Read data from the I2C device.
   * @param data Pointer to buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = default)
   * @return I2C operation result
   * @note Device address is configured during device creation
   */
  hf_i2c_err_t Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Write then read data from the I2C device.
   * @param tx_data Pointer to data buffer to write
   * @param tx_length Number of bytes to write
   * @param rx_data Pointer to buffer to store received data
   * @param rx_length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = default)
   * @return I2C operation result
   * @note Device address is configured during device creation
   */
  hf_i2c_err_t WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length, hf_u8_t* rx_data,
                         hf_u16_t rx_length, hf_u32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Get I2C bus statistics.
   * @param statistics Reference to statistics structure to fill
   * @return Operation result
   */
  hf_i2c_err_t GetStatistics(hf_i2c_statistics_t& statistics) const noexcept override;

  /**
   * @brief Get I2C bus diagnostics.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return Operation result
   */
  hf_i2c_err_t GetDiagnostics(hf_i2c_diagnostics_t& diagnostics) const noexcept override;

  /**
   * @brief Reset I2C statistics.
   * @return Operation result
   */
  hf_i2c_err_t ResetStatistics() noexcept override;

  /**
   * @brief Get the ESP-IDF device handle.
   * @return i2c_master_dev_handle_t
   */
  i2c_master_dev_handle_t GetHandle() const noexcept;

  /**
   * @brief Get the device configuration.
   * @return const hf_i2c_device_config_t&
   */
  const hf_i2c_device_config_t& GetConfig() const noexcept;

  /**
   * @brief Get the device address.
   * @return Device address
   */
  hf_u16_t GetDeviceAddress() const noexcept;

  /**
   * @brief Get the actual clock frequency for this device.
   * @param actual_freq_hz Reference to store actual frequency
   * @return Operation result
   */
  hf_i2c_err_t GetActualClockFrequency(hf_u32_t& actual_freq_hz) const noexcept;

  /**
   * @brief Probe if the device is present on the bus.
   * @return true if device responds, false otherwise
   */
  bool ProbeDevice() noexcept;

private:
  EspI2cBus* parent_bus_;                    ///< Parent bus pointer
  i2c_master_dev_handle_t handle_;           ///< ESP-IDF device handle
  hf_i2c_device_config_t config_;            ///< Device configuration
  bool initialized_;                         ///< Initialization status
  mutable hf_i2c_statistics_t statistics_;   ///< Per-device statistics
  mutable hf_i2c_diagnostics_t diagnostics_; ///< Per-device diagnostics
  mutable RtosMutex mutex_;                  ///< Device mutex for thread safety

  /**
   * @brief Update statistics with operation result.
   * @param success Operation success status
   * @param bytes_transferred Number of bytes transferred
   * @param operation_time_us Operation time in microseconds
   */
  void UpdateStatistics(bool success, size_t bytes_transferred,
                        hf_u64_t operation_time_us) noexcept;

  /**
   * @brief Convert ESP-IDF error to HardFOC error.
   * @param esp_error ESP-IDF error code
   * @return HardFOC I2C error code
   */
  hf_i2c_err_t ConvertEspError(esp_err_t esp_error) const noexcept;
};

/**
 * @class EspI2cBus
 * @brief Manages a single I2C bus. Handles bus initialization and device creation.
 *
 * Provides full configuration and control for the I2C bus, including device management,
 * bus initialization, and ESP-IDF v5.5+ features. Thread-safe device management with
 * proper resource cleanup.
 */
class EspI2cBus {
public:
  /**
   * @brief Construct a new EspI2cBus.
   * @param config Bus configuration
   */
  explicit EspI2cBus(const hf_i2c_master_bus_config_t& config) noexcept;

  /**
   * @brief Destructor. Automatically deinitializes the bus if needed.
   */
  ~EspI2cBus() noexcept;

  // Non-copyable, non-movable
  EspI2cBus(const EspI2cBus&) = delete;
  EspI2cBus& operator=(const EspI2cBus&) = delete;
  EspI2cBus(EspI2cBus&&) = delete;
  EspI2cBus& operator=(EspI2cBus&&) = delete;

  /**
   * @brief Initialize the I2C bus.
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept;

  /**
   * @brief Deinitialize the I2C bus and remove all devices.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept;

  /**
   * @brief Create and add a device to the I2C bus.
   * @param device_config Device configuration
   * @return Device index (>= 0) if successful, -1 if failed
   */
  int CreateDevice(const hf_i2c_device_config_t& device_config) noexcept;

  /**
   * @brief Get device by index (BaseI2c interface).
   * @param device_index Index of the device
   * @return Pointer to BaseI2c device, or nullptr if invalid
   */
  BaseI2c* GetDevice(int device_index) noexcept;

  /**
   * @brief Get device by index (const version).
   * @param device_index Index of the device
   * @return Pointer to const BaseI2c device, or nullptr if invalid
   */
  const BaseI2c* GetDevice(int device_index) const noexcept;

  /**
   * @brief Get ESP-specific device by index.
   * @param device_index Index of the device
   * @return Pointer to EspI2cDevice, or nullptr if invalid
   */
  EspI2cDevice* GetEspDevice(int device_index) noexcept;

  /**
   * @brief Get ESP-specific device by index (const version).
   * @param device_index Index of the device
   * @return Pointer to const EspI2cDevice, or nullptr if invalid
   */
  const EspI2cDevice* GetEspDevice(int device_index) const noexcept;

  /**
   * @brief Get device by address.
   * @param device_address Device address to find
   * @return Pointer to BaseI2c device, or nullptr if not found
   */
  BaseI2c* GetDeviceByAddress(hf_u16_t device_address) noexcept;

  /**
   * @brief Get number of devices on the bus.
   * @return Number of devices
   */
  std::size_t GetDeviceCount() const noexcept;

  /**
   * @brief Remove a device from the bus.
   * @param device_index Index of the device to remove
   * @return true if successful, false otherwise
   */
  bool RemoveDevice(int device_index) noexcept;

  /**
   * @brief Remove a device from the bus by address.
   * @param device_address Address of the device to remove
   * @return true if successful, false otherwise
   */
  bool RemoveDeviceByAddress(hf_u16_t device_address) noexcept;

  /**
   * @brief Get the bus configuration.
   * @return const hf_i2c_master_bus_config_t&
   */
  const hf_i2c_master_bus_config_t& GetConfig() const noexcept;

  /**
   * @brief Get the ESP-IDF bus handle.
   * @return i2c_master_bus_handle_t
   */
  i2c_master_bus_handle_t GetHandle() const noexcept;

  /**
   * @brief Get the I2C port number.
   * @return I2C port number
   */
  int GetPort() const noexcept;

  /**
   * @brief Check if the bus is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept;

  /**
   * @brief Scan the I2C bus for devices.
   * @param found_devices Vector to store found device addresses
   * @param start_addr Starting address for scan (default: 0x08)
   * @param end_addr Ending address for scan (default: 0x77)
   * @return Number of devices found
   */
  size_t ScanDevices(std::vector<hf_u16_t>& found_devices, hf_u16_t start_addr = 0x08,
                     hf_u16_t end_addr = 0x77) noexcept;

  /**
   * @brief Probe for device presence on the bus.
   * @param device_addr Device address to probe
   * @return true if device responds, false otherwise
   */
  bool ProbeDevice(hf_u16_t device_addr) noexcept;

  /**
   * @brief Reset the I2C bus.
   * @return true if successful, false otherwise
   */
  bool ResetBus() noexcept;

private:
  hf_i2c_master_bus_config_t config_;                  ///< Bus configuration
  i2c_master_bus_handle_t bus_handle_;                 ///< ESP-IDF bus handle
  bool initialized_;                                   ///< Initialization status
  mutable RtosMutex mutex_;                            ///< Bus mutex for thread safety
  std::vector<std::unique_ptr<EspI2cDevice>> devices_; ///< Device instances

  /**
   * @brief Find device index by address.
   * @param device_address Device address to find
   * @return Device index if found, -1 otherwise
   */
  int FindDeviceIndexByAddress(hf_u16_t device_address) const noexcept;

  /**
   * @brief Convert ESP-IDF error to HardFOC error.
   * @param esp_error ESP-IDF error code
   * @return HardFOC I2C error code
   */
  hf_i2c_err_t ConvertEspError(esp_err_t esp_error) const noexcept;
};

#endif // ESP_I2C_H_
