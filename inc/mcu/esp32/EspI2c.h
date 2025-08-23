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
 * - **Mode-Aware Operations**: Single mode variable determines sync vs async capabilities
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
 * @version 4.0.0 - Mode-aware architecture with DRY implementation
 *
 * @note This implementation requires ESP-IDF v5.5+ and is optimized for ESP32C6.
 * @note Thread-safe operation is guaranteed for all public methods.
 * @note Follows the same architectural pattern as EspSpi for consistency.
 * @note ESP-IDF v5.5+ enforces strict separation between sync/async modes.
 *
 * @example Basic Usage:
 * @code
 * // Create sync mode bus configuration
 * hf_i2c_master_bus_config_t bus_config = {};
 * bus_config.i2c_port = 0;
 * bus_config.sda_io_num = 21;
 * bus_config.scl_io_num = 22;
 * bus_config.mode = hf_i2c_mode_t::HF_I2C_MODE_SYNC;  // Sync mode only
 * bus_config.trans_queue_depth = 0;  // No queue for sync mode
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
 * // Use device for I2C operations (sync mode)
 * uint8_t data[] = {0x10, 0x20, 0x30};
 * hf_i2c_err_t result = device->Write(data, sizeof(data));
 * @endcode
 *
 * @example Async Mode Usage:
 * @code
 * // Create async mode bus configuration
 * hf_i2c_master_bus_config_t bus_config = {};
 * bus_config.i2c_port = 0;
 * bus_config.sda_io_num = 21;
 * bus_config.scl_io_num = 22;
 * bus_config.mode = hf_i2c_mode_t::HF_I2C_MODE_ASYNC;  // Async mode only
 * bus_config.trans_queue_depth = 10;  // Queue required for async mode
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
 * // Use device for I2C operations (async mode)
 * uint8_t data[] = {0x10, 0x20, 0x30};
 * hf_i2c_err_t result = device->WriteAsync(data, sizeof(data), 
 *     [](hf_i2c_err_t err, void* user_data) {
 *         ESP_LOGI("Async write completed: %s", HfI2CErrToString(err).data());
 *     });
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

// Forward declarations
class EspI2cBus;
class EspI2cDevice;

/**
 * @enum hf_i2c_operation_t
 * @brief Enumeration for I2C operation types used in logging and validation
 */
enum class hf_i2c_operation_t : uint8_t {
  HF_I2C_OP_WRITE = 0,           ///< Write operation
  HF_I2C_OP_READ = 1,            ///< Read operation
  HF_I2C_OP_WRITE_READ = 2,      ///< Write-then-read operation
  HF_I2C_OP_WRITE_ASYNC = 3,     ///< Asynchronous write operation
  HF_I2C_OP_READ_ASYNC = 4,      ///< Asynchronous read operation
  HF_I2C_OP_WRITE_READ_ASYNC = 5 ///< Asynchronous write-then-read operation
};

/**
 * @brief Convert operation type to string for logging
 * @param op Operation type
 * @return String representation of operation
 */
const char* HfI2COperationToString(hf_i2c_operation_t op) noexcept;

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
 * @note Supports both synchronous and asynchronous operations.
 * @warning ESP-IDF v5.5 constraint: Only one device per bus can use async mode
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
   * @brief Mark device as deinitialized without ESP-IDF cleanup
   * @return true if successful
   * 
   * This method is called by the bus when it handles ESP-IDF cleanup.
   * The device should not attempt to remove itself from the ESP-IDF bus.
   */
  bool MarkAsDeinitialized() noexcept;

  /**
   * @brief Deinitialize the device (internal cleanup only)
   * @return true if successful
   * 
   * This method only handles internal device cleanup.
   * ESP-IDF cleanup is handled by the parent bus.
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

  //==============================================//
  // ASYNC OPERATIONS - ESP-IDF v5.5+ FEATURES   //
  //==============================================//

  /**
   * @brief Write data to the I2C device asynchronously.
   * @param data Pointer to data buffer to write
   * @param length Number of bytes to write
   * @param callback Callback function for operation completion
   * @param user_data User data passed to callback
   * @param timeout_ms Timeout to wait for async slot availability (0 = no wait)
   * @return I2C operation result
   * @note Returns immediately once async operation starts
   * @note Completion is reported via callback
   * @warning Only one device per bus can use async mode at a time
   * @note Will wait for timeout_ms if another async operation is in progress
   */
  hf_i2c_err_t WriteAsync(const hf_u8_t* data, hf_u16_t length,
                          hf_i2c_async_callback_t callback,
                          void* user_data = nullptr,
                          hf_u32_t timeout_ms = 1000) noexcept;

  /**
   * @brief Read data from the I2C device asynchronously.
   * @param data Pointer to buffer to store received data
   * @param length Number of bytes to read
   * @param callback Callback function for operation completion
   * @param user_data User data passed to callback
   * @param timeout_ms Timeout to wait for async slot availability (0 = no wait)
   * @return I2C operation result
   * @note Returns immediately once async operation starts
   * @note Completion is reported via callback
   * @warning Only one device per bus can use async mode at a time
   * @note Will wait for timeout_ms if another async operation is in progress
   */
  hf_i2c_err_t ReadAsync(hf_u8_t* data, hf_u16_t length,
                         hf_i2c_async_callback_t callback,
                         void* user_data = nullptr,
                         hf_u32_t timeout_ms = 1000) noexcept;

  /**
   * @brief Write then read data from the I2C device asynchronously.
   * @param tx_data Pointer to data buffer to write
   * @param tx_length Number of bytes to write
   * @param rx_data Pointer to buffer to store received data
   * @param rx_length Number of bytes to read
   * @param callback Callback function for operation completion
   * @param user_data User data passed to callback
   * @param timeout_ms Timeout to wait for async slot availability (0 = no wait)
   * @return I2C operation result
   * @note Returns immediately once async operation starts
   * @note Completion is reported via callback
   * @warning Only one device per bus can use async mode at a time
   * @note Will wait for timeout_ms if another async operation is in progress
   */
  hf_i2c_err_t WriteReadAsync(const hf_u8_t* tx_data, hf_u16_t tx_length,
                              hf_u8_t* rx_data, hf_u16_t rx_length,
                              hf_i2c_async_callback_t callback,
                              void* user_data = nullptr,
                              hf_u32_t timeout_ms = 1000) noexcept;

  /**
   * @brief Check if async mode is supported on this device.
   * @return true if async mode is available, false otherwise
   */
  bool IsAsyncModeSupported() const noexcept;

  /**
   * @brief Check if an async operation is currently in progress.
   * @return true if async operation is active, false otherwise
   */
  bool IsAsyncOperationInProgress() const noexcept;

  /**
   * @brief Check if a sync operation is currently in progress.
   * @return true if sync operation is active, false otherwise
   */
  bool IsSyncOperationInProgress() const noexcept;

  /**
   * @brief Wait for current async operation to complete.
   * @param timeout_ms Timeout in milliseconds (0 = wait indefinitely)
   * @return true if operation completed, false on timeout
   */
  bool WaitAsyncOperationComplete(hf_u32_t timeout_ms = 0) noexcept;

  /**
   * @brief Wait for async operation completion using FreeRTOS task notification.
   * @param timeout_ms Timeout in milliseconds (0 = wait indefinitely)
   * @return true if operation completed, false on timeout
   * @note This method properly handles ESP-IDF v5.5 async completion events
   */
  bool WaitAsyncOperationCompleteWithNotification(hf_u32_t timeout_ms = 0) noexcept;

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

  /**
   * @brief Get the ESP-IDF device handle for internal operations.
   * @return ESP-IDF device handle
   */
  i2c_master_dev_handle_t GetDeviceHandle() const noexcept { return handle_; }

  /**
   * @brief Get the FreeRTOS task handle for async operation completion.
   * @return Task handle of the task that initiated the async operation
   */
  TaskHandle_t GetTaskHandle() const noexcept { return current_task_handle_; }

  //==============================================//
  // MODE-AWARE OPERATION METHODS                //
  //==============================================//

  /**
   * @brief Get the current I2C operation mode for this device
   * @return Current operation mode (sync/async)
   */
  hf_i2c_mode_t GetMode() const noexcept;

  /**
   * @brief Check if this device is in async mode
   * @return true if in async mode, false if in sync mode
   */
  bool IsAsyncMode() const noexcept;

  /**
   * @brief Check if this device is in sync mode
   * @return true if in sync mode, false if in async mode
   */
  bool IsSyncMode() const noexcept;

private:
  EspI2cBus* parent_bus_;                    ///< Parent bus pointer
  i2c_master_dev_handle_t handle_;           ///< ESP-IDF device handle
  hf_i2c_device_config_t config_;            ///< Device configuration
  bool initialized_;                         ///< Initialization status
  mutable hf_i2c_statistics_t statistics_;   ///< Per-device statistics
  mutable hf_i2c_diagnostics_t diagnostics_; ///< Per-device diagnostics
  mutable RtosMutex mutex_;                  ///< Device mutex for thread safety
  hf_i2c_mode_t device_mode_;               ///< Device operation mode (inherited from bus)

  //==============================================//
  // ASYNC OPERATION SUPPORT                     //
  //==============================================//
  bool async_operation_in_progress_;           ///< Is async operation active
  bool sync_operation_in_progress_;            ///< Is sync operation active (mutual exclusion)
  hf_i2c_async_callback_t current_callback_;  ///< Current user callback
  void* current_user_data_;                   ///< Current user data
  hf_u64_t async_start_time_;                 ///< Async operation start time
  hf_i2c_transaction_type_t current_op_type_; ///< Current operation type
  TaskHandle_t current_task_handle_;          ///< Task handle for the current async operation

  //==============================================//
  // ASYNC COMPLETION RESULT STORAGE             //
  //==============================================//
  hf_i2c_err_t pending_completion_result_;   ///< Pending completion result from ISR
  size_t pending_completion_bytes_;           ///< Pending completion bytes from ISR
  bool completion_result_ready_;              ///< Flag indicating completion result is ready

  //==============================================//
  // ESP-IDF v5.5 ASYNC CALLBACK BRIDGE          //
  //==============================================//
  
  /**
   * @brief Internal C callback bridge for ESP-IDF callbacks.
   * @param i2c_dev ESP-IDF device handle
   * @param evt_data Event data from ESP-IDF (i2c_master_event_data_t)
   * @param arg User data (pointer to EspI2cDevice)
   * @return false (no high priority wake needed)
   * @note This single callback handles ALL operation types (write/read/write-read)
   */
  static bool InternalAsyncCallback(i2c_master_dev_handle_t i2c_dev,
                                   const i2c_master_event_data_t* evt_data,
                                   void* arg);

  /**
   * @brief Handle completion of async operation.
   * @param result Operation result
   */
  void HandleAsyncCompletion(hf_i2c_err_t result) noexcept;

  /**
   * @brief Handle async completion in task context (called from task notification).
   * @param result Operation result
   * @param bytes_transferred Number of bytes transferred
   */
  void HandleAsyncCompletionInTask(hf_i2c_err_t result, size_t bytes_transferred) noexcept;

  /**
   * @brief Register temporary async callback for single operation.
   * @param callback User callback to register
   * @param user_data User data for callback
   * @param timeout_ms Timeout to wait for slot availability
   * @return true if successful, false otherwise
   */
  bool RegisterTemporaryCallback(hf_i2c_async_callback_t callback,
                                void* user_data,
                                hf_u32_t timeout_ms) noexcept;

  /**
   * @brief Unregister temporary async callback after operation.
   */
  void UnregisterTemporaryCallback() noexcept;

  /**
   * @brief Start async operation tracking after I2C operation is successfully started.
   */
  inline void StartAsyncOperationTracking() noexcept;

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

  /**
   * @brief Common validation for all I2C operations.
   * @param data Data buffer pointer
   * @param length Data length
   * @param operation_type Operation type for logging
   * @return true if validation passes, false otherwise
   */
  bool ValidateOperation(const void* data, hf_u16_t length, hf_i2c_operation_t operation_type) noexcept;

  /**
   * @brief Common sync operation setup and cleanup.
   * @param operation_type Operation type for logging
   * @return true if setup successful, false otherwise
   */
  bool SetupSyncOperation(hf_i2c_operation_t operation_type) noexcept;

  /**
   * @brief Common sync operation cleanup.
   */
  void CleanupSyncOperation() noexcept;

  /**
   * @brief Common async operation setup.
   * @param callback User callback
   * @param user_data User data
   * @param timeout_ms Timeout for slot availability
   * @return true if setup successful, false otherwise
   */
  bool SetupAsyncOperation(hf_i2c_async_callback_t callback, void* user_data, hf_u32_t timeout_ms) noexcept;

  /**
   * @brief Store async completion result for task context retrieval.
   * @param result Operation result
   * @param bytes_transferred Number of bytes transferred
   * @note This method is called from ISR context - keep it minimal!
   */
  void StoreAsyncCompletionResult(hf_i2c_err_t result, size_t bytes_transferred) noexcept;
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
   * @brief Destructor. Automatically deinitializes the bus and devices if needed.
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

  //==============================================//
  // INDEX-BASED ACCESS AND ITERATION METHODS    //
  //==============================================//

  /**
   * @brief Get device by index with bounds checking.
   * @param device_index Index of the device (0-based)
   * @return Pointer to BaseI2c device, or nullptr if index out of bounds
   * @note Thread-safe access to devices
   */
  BaseI2c* operator[](int device_index) noexcept;

  /**
   * @brief Get device by index with bounds checking (const version).
   * @param device_index Index of the device (0-based)
   * @return Pointer to const BaseI2c device, or nullptr if index out of bounds
   * @note Thread-safe access to devices
   */
  const BaseI2c* operator[](int device_index) const noexcept;

  /**
   * @brief Get ESP-specific device by index with bounds checking.
   * @param device_index Index of the device (0-based)
   * @return Pointer to EspI2cDevice, or nullptr if index out of bounds
   * @note Thread-safe access to devices
   */
  EspI2cDevice* At(int device_index) noexcept;

  /**
   * @brief Get ESP-specific device by index with bounds checking (const version).
   * @param device_index Index of the device (0-based)
   * @return Pointer to const EspI2cDevice, or nullptr if index out of bounds
   * @note Thread-safe access to devices
   */
  const EspI2cDevice* At(int device_index) const noexcept;

  /**
   * @brief Check if device index is valid.
   * @param device_index Index to check
   * @return true if index is valid, false otherwise
   */
  bool IsValidIndex(int device_index) const noexcept;

  /**
   * @brief Get first device on the bus.
   * @return Pointer to first BaseI2c device, or nullptr if no devices
   */
  BaseI2c* GetFirstDevice() noexcept;

  /**
   * @brief Get first device on the bus (const version).
   * @return Pointer to first const BaseI2c device, or nullptr if no devices
   */
  const BaseI2c* GetFirstDevice() const noexcept;

  /**
   * @brief Get last device on the bus.
   * @return Pointer to last BaseI2c device, or nullptr if no devices
   */
  BaseI2c* GetLastDevice() noexcept;

  /**
   * @brief Get last device on the bus (const version).
   * @return Pointer to last const BaseI2c device, or nullptr if no devices
   */
  const BaseI2c* GetLastDevice() const noexcept;

  /**
   * @brief Get all devices as a vector of BaseI2c pointers.
   * @return Vector of BaseI2c pointers (copy of internal device list)
   * @note Thread-safe, returns a copy of the current device list
   */
  std::vector<BaseI2c*> GetAllDevices() noexcept;

  /**
   * @brief Get all devices as a vector of const BaseI2c pointers.
   * @return Vector of const BaseI2c pointers (copy of internal device list)
   * @note Thread-safe, returns a copy of the current device list
   */
  std::vector<const BaseI2c*> GetAllDevices() const noexcept;

  /**
   * @brief Get all ESP-specific devices as a vector.
   * @return Vector of EspI2cDevice pointers (copy of internal device list)
   * @note Thread-safe, returns a copy of the current device list
   */
  std::vector<EspI2cDevice*> GetAllEspDevices() noexcept;

  /**
   * @brief Get all ESP-specific devices as a vector (const version).
   * @return Vector of const EspI2cDevice pointers (copy of internal device list)
   * @note Thread-safe, returns a copy of the current device list
   */
  std::vector<const EspI2cDevice*> GetAllEspDevices() const noexcept;

  /**
   * @brief Find device index by address with bounds checking.
   * @param device_address Device address to find
   * @return Device index if found, -1 if not found
   */
  int FindDeviceIndex(hf_u16_t device_address) const noexcept;

  /**
   * @brief Get device addresses as a vector.
   * @return Vector of device addresses
   * @note Thread-safe, returns a copy of current device addresses
   */
  std::vector<hf_u16_t> GetDeviceAddresses() const noexcept;

  /**
   * @brief Check if bus has any devices.
   * @return true if bus has devices, false if empty
   */
  bool HasDevices() const noexcept;

  /**
   * @brief Check if bus is empty (no devices).
   * @return true if bus is empty, false if it has devices
   */
  bool IsEmpty() const noexcept;

  /**
   * @brief Clear all devices from the bus.
   * @return true if successful, false otherwise
   * @note All devices will be properly deinitialized and removed
   */
  bool ClearAllDevices() noexcept;

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
   * @param scan_timeout_ms Timeout for each probe operation (0 = use fast 10ms timeout)
   * @return Number of devices found
   */
  size_t ScanDevices(std::vector<hf_u16_t>& found_devices, hf_u16_t start_addr = 0x08,
                     hf_u16_t end_addr = 0x77, hf_u32_t scan_timeout_ms = 0) noexcept;

  /**
   * @brief Probe for device presence on the bus.
   * @param device_addr Device address to probe
   * @param timeout_ms Timeout in milliseconds (0 = use default 1000ms)
   * @return true if device responds, false otherwise
   */
  bool ProbeDevice(hf_u16_t device_addr, hf_u32_t timeout_ms = 10) noexcept;

  /**
   * @brief Reset the I2C bus.
   * @return true if successful, false otherwise
   */
  bool ResetBus() noexcept;

  //==============================================//
  // MODE MANAGEMENT METHODS                      //
  //==============================================//

  /**
   * @brief Get the current I2C operation mode
   * @return Current operation mode (sync/async)
   */
  hf_i2c_mode_t GetMode() const noexcept;

  /**
   * @brief Check if async mode is enabled
   * @return true if in async mode, false if in sync mode
   */
  bool IsAsyncMode() const noexcept;

  /**
   * @brief Check if sync mode is enabled
   * @return true if in sync mode, false if in async mode
   */
  bool IsSyncMode() const noexcept;

  /**
   * @brief Switch operation mode (recreates bus)
   * @param new_mode New operation mode to switch to
   * @param queue_depth Queue depth for async mode (ignored for sync mode)
   * @return true if successful, false otherwise
   * @note This will deinitialize and reinitialize the bus
   */
  bool SwitchMode(hf_i2c_mode_t new_mode, uint8_t queue_depth = 10) noexcept;

private:
  hf_i2c_master_bus_config_t config_;                  ///< Bus configuration
  i2c_master_bus_handle_t bus_handle_;                 ///< ESP-IDF bus handle
  bool initialized_;                                   ///< Initialization status
  mutable RtosMutex mutex_;                            ///< Bus mutex for thread safety
  std::vector<std::unique_ptr<EspI2cDevice>> devices_; ///< Device instances
  hf_i2c_mode_t current_mode_;                        ///< Current operation mode

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

  /**
   * @brief Custom fast I2C probe that works around ESP-IDF's broken probe function
   * @param device_addr Device address to probe
   * @param timeout_ms Timeout in milliseconds
   * @return true if device responds, false otherwise
   * @note This function creates a temporary device and performs a real I2C transaction
   * @note Much faster and more reliable than ESP-IDF's broken probe function
   */
  bool CustomFastProbe(hf_u16_t device_addr, hf_u32_t timeout_ms) noexcept;
};

#endif // ESP_I2C_H_
