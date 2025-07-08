/**
 * @file EspI2c.h
 * @brief Advanced ESP32-integrated I2C controller for ESP-IDF v5.5+ with ESP32C6 support.
 *
 * This header provides a comprehensive I2C implementation that utilizes all the advanced
 * features available in ESP-IDF v5.5+ for ESP32C6. The implementation follows the new
 * bus-device model, supports asynchronous operations, advanced power management, and
 * comprehensive error handling while maintaining clean abstraction.
 *
 * @section features ESP32C6/ESP-IDF v5.5+ Features Supported:
 * - **New Bus-Device Model**: i2c_new_master_bus + i2c_master_bus_add_device
 * - **Asynchronous Operations**: Non-blocking I2C with event callbacks
 * - **Multi-Buffer Transactions**: Complex protocols with multiple sequences
 * - **Advanced Signal Conditioning**: Digital glitch filtering and clock stretching
 * - **Power Management**: Multiple clock sources and low-power modes
 * - **Comprehensive Monitoring**: Real-time statistics and bus health diagnostics
 * - **Thread Safety**: Full RTOS integration with proper synchronization
 * - **Hardware Acceleration**: DMA transfers and interrupt-driven operation
 * - **Error Recovery**: Automatic bus recovery and comprehensive error handling
 *
 * @section performance Performance Characteristics:
 * - Standard Mode: 100 kHz
 * - Fast Mode: 400 kHz
 * - Fast Mode Plus: 1 MHz (ESP32C6)
 * - 7-bit and 10-bit addressing support
 * - Clock stretching with configurable timeout
 * - Multi-master operation capability
 * - Hardware FIFO (32 bytes)
 * - DMA support for large transfers
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * @version 2.0.0 - Complete ESP-IDF v5.5+ rewrite
 *
 * @note This implementation requires ESP-IDF v5.5+ and is optimized for ESP32C6.
 * @note Thread-safe operation is guaranteed for all public methods.
 * @note All asynchronous callbacks are executed in interrupt context.
 *
 * @example Basic Usage:
 * @code
 * // Create bus configuration
 * hf_i2c_master_bus_config_t bus_config;
 * bus_config.i2c_port = 0;
 * bus_config.sda_io_num = static_cast<GpioNum>(21);
 * bus_config.scl_io_num = static_cast<GpioNum>(22);
 * bus_config.enable_internal_pullup = true;
 * 
 * // Create I2C instance
 * EspI2c i2c(bus_config);
 * if (!i2c.Initialize()) {
 *     // Handle initialization error
 * }
 * 
 * // Add device
 * hf_i2c_device_config_t device;
 * device.device_address = 0x48;
 * device.scl_speed_hz = 400000;
 * i2c.AddDevice(device);
 * 
 * // Simple write operation
 * uint8_t data[] = {0x10, 0x20, 0x30};
 * hf_i2c_err_t result = i2c.Write(0x48, data, sizeof(data));
 * @endcode
 */

#pragma once

#include "BaseI2c.h"
#include "McuTypes_I2C.h"
#include "RtosMutex.h"
#include <unordered_map>
#include <vector>
#include <atomic>
#include <memory>

/**
 * @class EspI2c
 * @brief Advanced ESP32-integrated I2C controller implementation for ESP-IDF v5.5+.
 *
 * This class provides comprehensive I2C communication using the microcontroller's built-in
 * I2C peripheral with full support for ESP-IDF v5.5+ advanced features. The implementation
 * utilizes the new bus-device model, asynchronous operations, and comprehensive error handling
 * to provide a robust, high-performance I2C solution for the HardFOC system.
 *
 * @details Architecture Overview:
 * - **Bus-Device Model**: Separate master bus and device handles for optimal resource management
 * - **Asynchronous Support**: Non-blocking operations with event callbacks and queueing
 * - **Advanced Transactions**: Multi-buffer and custom command sequences
 * - **Power Management**: Intelligent power modes with automatic suspend/resume
 * - **Comprehensive Monitoring**: Real-time statistics and diagnostics
 * - **Thread Safety**: Fine-grained mutex protection for multi-threaded access
 * - **Error Recovery**: Automatic bus recovery with configurable retry logic
 *
 * @note All public methods are thread-safe and can be called from multiple tasks.
 * @note Asynchronous callbacks execute in interrupt context - keep them minimal.
 * @note The implementation automatically manages ESP-IDF v5.5+ resources.
 */
class EspI2c : public BaseI2c {
public:
    //==========================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    //==========================================================================

    /**
     * @brief Constructor with I2C master bus configuration.
     * @param config Master bus configuration with ESP-IDF v5.5+ features
     */
    explicit EspI2c(const hf_i2c_master_bus_config_t& config) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup of all resources.
     * @details Automatically deinitializes the bus, removes all devices,
     *          cancels pending operations, and releases allocated resources.
     */
    ~EspI2c() noexcept override;

    // Non-copyable, non-movable for safety
    EspI2c(const EspI2c&) = delete;
    EspI2c& operator=(const EspI2c&) = delete;
    EspI2c(EspI2c&&) = delete;
    EspI2c& operator=(EspI2c&&) = delete;

    //==========================================================================
    // CORE I2C OPERATIONS (BaseI2c Interface)
    //==========================================================================

    /**
     * @brief Initialize the I2C bus with ESP-IDF v5.5+ new API.
     * @details Creates the master bus using i2c_new_master_bus() and configures
     *          all advanced features like glitch filtering, power management, etc.
     * @return true if successful, false otherwise
     * @note This method is thread-safe and idempotent
     */
    bool Initialize() noexcept override;

    /**
     * @brief Deinitialize the I2C bus and clean up all resources.
     * @details Removes all registered devices, deletes the master bus, cancels
     *          pending operations, and releases all allocated memory.
     * @return true if successful, false otherwise
     * @note This method is thread-safe
     */
    bool Deinitialize() noexcept override;

    /**
     * @brief Write data to an I2C device.
     * @param device_addr 7-bit I2C device address
     * @param data Pointer to data buffer to write
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return I2C operation result
     * @note Uses ESP-IDF v5.5+ i2c_master_transmit() internally
     */
    hf_i2c_err_t Write(uint8_t device_addr, const uint8_t* data, uint16_t length,
                   uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Read data from an I2C device.
     * @param device_addr 7-bit I2C device address
     * @param data Pointer to buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return I2C operation result
     * @note Uses ESP-IDF v5.5+ i2c_master_receive() internally
     */
    hf_i2c_err_t Read(uint8_t device_addr, uint8_t* data, uint16_t length,
                  uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Write then read data from an I2C device.
     * @param device_addr 7-bit I2C device address
     * @param tx_data Pointer to data buffer to write
     * @param tx_length Number of bytes to write
     * @param rx_data Pointer to buffer to store received data
     * @param rx_length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return I2C operation result
     * @note Uses ESP-IDF v5.5+ i2c_master_transmit_receive() internally
     */
    hf_i2c_err_t WriteRead(uint8_t device_addr, const uint8_t* tx_data, uint16_t tx_length,
                       uint8_t* rx_data, uint16_t rx_length, 
                       uint32_t timeout_ms = 0) noexcept override;

    //==========================================================================
    // DEVICE MANAGEMENT
    //==========================================================================

    /**
     * @brief Add a device to the I2C bus.
     * @param device_config Device configuration with ESP-IDF v5.5+ features
     * @return Operation result
     * @note Uses ESP-IDF v5.5+ i2c_master_bus_add_device() internally
     */
    hf_i2c_err_t AddDevice(const hf_i2c_device_config_t& device_config) noexcept;

    /**
     * @brief Remove a device from the I2C bus.
     * @param device_address Device address to remove
     * @return Operation result
     * @note Uses ESP-IDF v5.5+ i2c_master_bus_rm_device() internally
     */
    hf_i2c_err_t RemoveDevice(uint16_t device_address) noexcept;

    /**
     * @brief Check if a device is present at the given address.
     * @param device_addr Device address to probe
     * @return true if device responds, false otherwise
     * @note Uses ESP-IDF v5.5+ i2c_master_probe() internally
     */
    bool ProbeDevice(uint16_t device_addr) noexcept;

    /**
     * @brief Scan for devices on the I2C bus.
     * @param found_devices Vector to store found device addresses
     * @param start_addr Starting address for scan (default 0x08)
     * @param end_addr Ending address for scan (default 0x77)
     * @return Number of devices found
     */
    size_t ScanDevices(std::vector<uint16_t>& found_devices,
                       uint16_t start_addr = I2C_MIN_DEVICE_ADDR,
                       uint16_t end_addr = I2C_MAX_DEVICE_ADDR_7BIT) noexcept;

    //==========================================================================
    // ASYNCHRONOUS OPERATIONS
    //==========================================================================

    /**
     * @brief Perform asynchronous write operation.
     * @param device_addr Device address
     * @param data Data vector to write
     * @param callback Callback function to invoke on completion
     * @param user_data User data pointer for callback
     * @return Operation result with operation ID for tracking
     */
    hf_i2c_err_t WriteAsync(uint16_t device_addr, const std::vector<uint8_t>& data,
                        hf_i2c_async_callback_t callback, void* user_data = nullptr) noexcept;

    /**
     * @brief Perform asynchronous read operation.
     * @param device_addr Device address
     * @param length Number of bytes to read
     * @param callback Callback function to invoke on completion
     * @param user_data User data pointer for callback
     * @return Operation result with operation ID for tracking
     */
    hf_i2c_err_t ReadAsync(uint16_t device_addr, size_t length, hf_i2c_async_callback_t callback,
                       void* user_data = nullptr) noexcept;

    /**
     * @brief Cancel a pending asynchronous operation.
     * @param operation_id ID of the operation to cancel
     * @return Operation result
     */
    hf_i2c_err_t CancelAsyncOperation(uint32_t operation_id) noexcept;

    /**
     * @brief Set event callback for I2C operations.
     * @param callback Callback function pointer
     * @param user_data User data pointer for callback
     */
    void SetEventCallback(hf_i2c_event_callback_t callback, void* user_data = nullptr) noexcept;

    //==========================================================================
    // ADVANCED TRANSACTIONS
    //==========================================================================

    /**
     * @brief Execute a multi-buffer transaction.
     * @param transaction Multi-buffer transaction descriptor
     * @return Operation result
     */
    hf_i2c_err_t ExecuteMultiBufferTransaction(const hf_i2c_multi_buffer_transaction_t& transaction) noexcept;

    /**
     * @brief Execute a multi-buffer transaction asynchronously.
     * @param transaction Multi-buffer transaction descriptor
     * @param callback Callback function to invoke on completion
     * @param user_data User data pointer for callback
     * @return Operation result
     */
    hf_i2c_err_t ExecuteMultiBufferTransactionAsync(const hf_i2c_multi_buffer_transaction_t& transaction,
                                                hf_i2c_async_callback_t callback,
                                                void* user_data = nullptr) noexcept;

    /**
     * @brief Execute a sequence of custom I2C commands.
     * @param commands Vector of custom commands to execute
     * @return Operation result
     */
    hf_i2c_err_t ExecuteCustomSequence(const std::vector<hf_i2c_custom_command_t>& commands) noexcept;

    /**
     * @brief Execute a sequence of custom I2C commands asynchronously.
     * @param commands Vector of custom commands to execute
     * @param callback Callback function to invoke on completion
     * @param user_data User data pointer for callback
     * @return Operation result
     */
    hf_i2c_err_t ExecuteCustomSequenceAsync(const std::vector<hf_i2c_custom_command_t>& commands,
                                        hf_i2c_async_callback_t callback, void* user_data = nullptr) noexcept;

    //==========================================================================
    // REGISTER ACCESS UTILITIES
    //==========================================================================

    /**
     * @brief Write to a device register.
     * @param device_addr Device address
     * @param reg_addr Register address
     * @param value Value to write
     * @return Operation result
     */
    hf_i2c_err_t WriteRegister(uint16_t device_addr, uint8_t reg_addr, uint8_t value) noexcept;

    /**
     * @brief Read from a device register.
     * @param device_addr Device address
     * @param reg_addr Register address
     * @param value Reference to store read value
     * @return Operation result
     */
    hf_i2c_err_t ReadRegister(uint16_t device_addr, uint8_t reg_addr, uint8_t& value) noexcept;

    /**
     * @brief Write to multiple consecutive registers.
     * @param device_addr Device address
     * @param start_reg_addr Starting register address
     * @param data Vector of data to write
     * @return Operation result
     */
    hf_i2c_err_t WriteMultipleRegisters(uint16_t device_addr, uint8_t start_reg_addr,
                                    const std::vector<uint8_t>& data) noexcept;

    /**
     * @brief Read from multiple consecutive registers.
     * @param device_addr Device address
     * @param start_reg_addr Starting register address
     * @param data Vector to store read data
     * @param count Number of registers to read
     * @return Operation result
     */
    hf_i2c_err_t ReadMultipleRegisters(uint16_t device_addr, uint8_t start_reg_addr,
                                   std::vector<uint8_t>& data, size_t count) noexcept;

    //==========================================================================
    // POWER MANAGEMENT
    //==========================================================================

    /**
     * @brief Set I2C bus power mode.
     * @param mode Desired power mode
     * @return Operation result
     */
    hf_i2c_err_t SetPowerMode(hf_i2c_power_mode_t mode) noexcept;

    /**
     * @brief Get current I2C bus power mode.
     * @return Current power mode
     */
    [[nodiscard]] hf_i2c_power_mode_t GetPowerMode() const noexcept;

    /**
     * @brief Suspend I2C bus operations for power saving.
     * @return Operation result
     */
    hf_i2c_err_t SuspendBus() noexcept;

    /**
     * @brief Resume I2C bus operations from suspended state.
     * @return Operation result
     */
    hf_i2c_err_t ResumeBus() noexcept;

    //==========================================================================
    // MONITORING AND DIAGNOSTICS
    //==========================================================================

    /**
     * @brief Get I2C operation statistics.
     * @return Current statistics snapshot
     */
    [[nodiscard]] hf_i2c_statistics_t GetStatistics() const noexcept;

    /**
     * @brief Reset I2C operation statistics.
     */
    void ResetStatistics() noexcept;

    /**
     * @brief Get I2C diagnostics information.
     * @return Current diagnostics snapshot
     */
    [[nodiscard]] hf_i2c_diagnostics_t GetDiagnostics() noexcept;

    /**
     * @brief Check if I2C bus is healthy.
     * @return true if bus is healthy, false otherwise
     */
    [[nodiscard]] bool IsBusHealthy() noexcept;

    //==========================================================================
    // ERROR RECOVERY
    //==========================================================================

    /**
     * @brief Attempt to recover the I2C bus from error conditions.
     * @return Operation result
     */
    hf_i2c_err_t RecoverBus() noexcept;

    /**
     * @brief Reset the I2C bus hardware.
     * @return true if successful, false otherwise
     */
    bool ResetBus() noexcept;

    //==========================================================================
    // CONFIGURATION
    //==========================================================================

    /**
     * @brief Get the current bus configuration.
     * @return Reference to current configuration
     */
    [[nodiscard]] const hf_i2c_master_bus_config_t& GetConfig() const noexcept;

    /**
     * @brief Set clock speed for the bus.
     * @param clock_speed_hz Clock speed in Hz
     * @return true if successful, false otherwise
     */
    bool SetClockSpeed(uint32_t clock_speed_hz) noexcept;

    /**
     * @brief Enable or disable internal pull-up resistors.
     * @param enable true to enable pull-ups, false to disable
     * @return true if successful, false otherwise
     */
    bool SetPullUps(bool enable) noexcept;

private:
    //==========================================================================
    // PRIVATE MEMBERS
    //==========================================================================

    // Configuration and state
    hf_i2c_master_bus_config_t bus_config_;              ///< Bus configuration
    std::atomic<bool> initialized_{false};       ///< Initialization state
    std::atomic<hf_i2c_err_t> last_error_{hf_i2c_err_t::I2C_SUCCESS}; ///< Last error

    // ESP-IDF v5.5+ handles
    I2cMasterBusHandle master_bus_handle_{nullptr}; ///< Master bus handle
    std::unordered_map<uint16_t, I2cMasterDevHandle> device_handles_; ///< Device handles
    std::unordered_map<uint16_t, hf_i2c_device_config_t> device_configs_; ///< Device configurations

    // Thread safety
    mutable RtosMutex mutex_;                    ///< Mutex for thread safety
    std::atomic<bool> bus_locked_{false};        ///< Bus lock status

    // Asynchronous operations
    std::unordered_map<uint32_t, void*> async_operations_; ///< Active async operations
    std::atomic<uint32_t> next_operation_id_{1}; ///< Next operation ID
    hf_i2c_event_callback_t event_callback_{nullptr};   ///< Event callback
    void* event_callback_userdata_{nullptr};     ///< Event callback user data

    // Monitoring and diagnostics
    mutable hf_i2c_statistics_t statistics_;           ///< Operation statistics
    hf_i2c_diagnostics_t diagnostics_;                 ///< Bus diagnostics
    std::atomic<uint64_t> last_operation_time_us_{0}; ///< Last operation timestamp

    // Power management
    std::atomic<hf_i2c_power_mode_t> current_power_mode_{hf_i2c_power_mode_t::HF_I2C_POWER_FULL}; ///< Current power mode
    std::atomic<bool> bus_suspended_{false};     ///< Bus suspension state
    void* auto_suspend_timer_{nullptr};          ///< Auto-suspend timer

    //==========================================================================
    // PRIVATE METHODS
    //==========================================================================

    /**
     * @brief Convert ESP-IDF error codes to HardFOC I2C error codes.
     * @param esp_error ESP-IDF esp_err_t error code
     * @return Corresponding hf_i2c_err_t error code
     */
    [[nodiscard]] hf_i2c_err_t ConvertEspError(EspErr esp_error) const noexcept;

    /**
     * @brief Get or create device handle for a given address.
     * @param device_addr Device address
     * @return Device handle or nullptr on failure
     */
    I2cMasterDevHandle GetOrCreateDeviceHandle(uint16_t device_addr) noexcept;

    /**
     * @brief Update operation statistics.
     * @param success Whether the operation was successful
     * @param bytes_transferred Number of bytes transferred
     * @param operation_time_us Operation duration in microseconds
     */
    void UpdateStatistics(bool success, size_t bytes_transferred, uint64_t operation_time_us) noexcept;

    /**
     * @brief Update diagnostics information.
     */
    void UpdateDiagnostics() noexcept;

    /**
     * @brief Validate device address.
     * @param device_addr Device address to validate
     * @return true if valid, false otherwise
     */
    [[nodiscard]] bool IsValidDeviceAddress(uint16_t device_addr) const noexcept;

    /**
     * @brief Get effective timeout value.
     * @param timeout_ms Requested timeout (0 = use default)
     * @return Effective timeout value
     */
    [[nodiscard]] uint32_t GetEffectiveTimeout(uint32_t timeout_ms) const noexcept;

    /**
     * @brief Create auto-suspend timer for power management.
     * @return true if successful, false otherwise
     */
    bool CreateAutoSuspendTimer() noexcept;

    /**
     * @brief Destroy auto-suspend timer.
     */
    void DestroyAutoSuspendTimer() noexcept;

    /**
     * @brief Start or restart auto-suspend timer.
     */
    void StartAutoSuspendTimer() noexcept;

    //==========================================================================
    // CONSTANTS
    //==========================================================================

    static constexpr uint32_t AUTO_SUSPEND_DELAY_MS = 5000;  ///< Auto-suspend delay
    static constexpr uint8_t MAX_DEVICES = 127;              ///< Maximum devices on bus
};
