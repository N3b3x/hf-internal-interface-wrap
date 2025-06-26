/**
 * @file McuI2c.h
 * @brief Advanced MCU-integrated I2C controller implementation with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This header provides a comprehensive I2C implementation that utilizes all the advanced
 * features available in ESP-IDF v5.5+ for ESP32C6, including the new bus-device model,
 * asynchronous operations, multi-buffer transactions, custom sequences, glitch filtering,
 * power management, and comprehensive error handling.
 *
 * This is the unified I2C implementation for MCUs with integrated I2C controllers,
 * including both basic and advanced features in a single class.
 */

#ifndef MCU_I2C_H
#define MCU_I2C_H

#include "BaseI2c.h"
#include "McuTypes.h"
#include <mutex>
#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>

#ifdef ESP_PLATFORM
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "driver/i2c_types.h"
#include "hal/i2c_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#endif

//--------------------------------------
//  Advanced I2C Configuration
//--------------------------------------

/**
 * @brief Clock source selection for I2C bus
 */
enum class HfI2cClockSource : uint8_t {
    DEFAULT = 0,        ///< Default I2C source clock
    XTAL = 1,          ///< External crystal (lower power)
    RC_FAST = 2        ///< Internal 20MHz RC oscillator
};

/**
 * @brief I2C bus mode selection
 */
enum class HfI2cBusMode : uint8_t {
    MASTER = 0,        ///< Master mode
    SLAVE = 1          ///< Slave mode
};

/**
 * @brief I2C address bit width
 */
enum class HfI2cAddressBits : uint8_t {
    SEVEN_BIT = 7,     ///< 7-bit addressing (standard)
    TEN_BIT = 10       ///< 10-bit addressing (extended)
};

/**
 * @brief I2C power modes for energy efficiency
 */
enum class HfI2cPowerMode : uint8_t {
    FULL_POWER = 0,    ///< Maximum performance, highest power
    LOW_POWER = 1,     ///< Reduced power consumption
    SLEEP = 2          ///< Minimal power, bus suspended
};

/**
 * @brief Advanced I2C configuration structure
 */
struct I2cAdvancedConfig {
    // Basic configuration
    uint8_t busNumber;                          ///< I2C bus number (0, 1, etc.)
    uint32_t clockSpeed;                        ///< Clock speed in Hz (100000, 400000, 1000000)
    int sclPin;                                 ///< SCL pin number
    int sdaPin;                                 ///< SDA pin number
    bool pullupResistors;                       ///< Enable internal pullup resistors
    uint32_t timeoutMs;                         ///< Default timeout in milliseconds
    
    // Advanced configuration
    HfI2cClockSource clockSource;               ///< Clock source selection
    HfI2cBusMode busMode;                       ///< Bus mode (master/slave)
    bool clockStretchingEnabled;                ///< Enable clock stretching
    uint32_t clockStretchingTimeout;            ///< Clock stretching timeout (us)
    bool digitalFilterEnabled;                  ///< Enable digital glitch filter
    bool analogFilterEnabled;                   ///< Enable analog glitch filter
    uint8_t digitalFilterLength;                ///< Digital filter length (cycles)
    
    // Asynchronous operations
    bool asyncOperationsEnabled;                ///< Enable asynchronous operations
    uint8_t maxConcurrentOperations;            ///< Max concurrent async operations
    bool eventCallbacksEnabled;                ///< Enable event callbacks
    
    // Power management
    HfI2cPowerMode powerMode;                   ///< Power mode setting
    bool autoSuspendEnabled;                    ///< Auto-suspend when idle
    uint32_t autoSuspendDelayMs;                ///< Delay before auto-suspend
    
    // Statistics and diagnostics
    bool statisticsEnabled;                     ///< Enable operation statistics
    bool diagnosticsEnabled;                    ///< Enable diagnostic features
    
    // Default constructor
    I2cAdvancedConfig() 
        : busNumber(0), clockSpeed(100000), sclPin(-1), sdaPin(-1)
        , pullupResistors(true), timeoutMs(1000)
        , clockSource(HfI2cClockSource::DEFAULT), busMode(HfI2cBusMode::MASTER)
        , clockStretchingEnabled(true), clockStretchingTimeout(1000)
        , digitalFilterEnabled(true), analogFilterEnabled(true), digitalFilterLength(7)
        , asyncOperationsEnabled(false), maxConcurrentOperations(4), eventCallbacksEnabled(false)
        , powerMode(HfI2cPowerMode::FULL_POWER), autoSuspendEnabled(false), autoSuspendDelayMs(5000)
        , statisticsEnabled(false), diagnosticsEnabled(false) {}
};

/**
 * @brief I2C device-specific configuration
 */
struct I2cDeviceConfig {
    uint16_t deviceAddress;                     ///< Device address
    HfI2cAddressBits addressBits;               ///< Address bit width
    uint32_t timeoutMs;                         ///< Device-specific timeout
    uint8_t retryCount;                         ///< Number of retry attempts
    uint32_t clockStretchingTimeout;            ///< Device clock stretching timeout
    bool requiresSpecialHandling;               ///< Device needs special handling
    
    I2cDeviceConfig() 
        : deviceAddress(0), addressBits(HfI2cAddressBits::SEVEN_BIT)
        , timeoutMs(1000), retryCount(3), clockStretchingTimeout(1000)
        , requiresSpecialHandling(false) {}
};

/**
 * @brief Multi-buffer transaction descriptor
 */
struct I2cMultiBufferTransaction {
    struct Buffer {
        const uint8_t* data;                    ///< Buffer data pointer
        size_t length;                          ///< Buffer length
        bool isWrite;                           ///< true for write, false for read
        
        Buffer(const uint8_t* d, size_t l, bool w) : data(d), length(l), isWrite(w) {}
    };
    
    std::vector<Buffer> buffers;                ///< Transaction buffers
    uint16_t deviceAddress;                     ///< Target device address
    uint32_t timeoutMs;                         ///< Transaction timeout
    bool stopCondition;                         ///< Generate stop condition at end
    
    I2cMultiBufferTransaction() 
        : deviceAddress(0), timeoutMs(1000), stopCondition(true) {}
};

/**
 * @brief Custom command sequence operation
 */
struct I2cCustomCommand {
    enum class Type {
        WRITE,                                  ///< Write operation
        READ,                                   ///< Read operation
        DELAY,                                  ///< Delay operation
        START,                                  ///< Generate start condition
        STOP,                                   ///< Generate stop condition
        RESTART,                                ///< Generate restart condition
        CONDITIONAL                             ///< Conditional operation
    };
    
    Type type;                                  ///< Command type
    std::vector<uint8_t> data;                  ///< Command data
    uint32_t parameter;                         ///< Generic parameter (delay, condition, etc.)
    std::function<bool()> condition;            ///< Condition function for CONDITIONAL type
    
    I2cCustomCommand(Type t) : type(t), parameter(0) {}
};

/**
 * @brief I2C operation statistics
 */
struct I2cStatistics {
    uint64_t totalOperations;                   ///< Total operations performed
    uint64_t successfulOperations;              ///< Successful operations
    uint64_t failedOperations;                  ///< Failed operations
    uint64_t timeoutOperations;                 ///< Operations that timed out
    uint64_t bytesTransmitted;                  ///< Total bytes transmitted
    uint64_t bytesReceived;                     ///< Total bytes received
    uint64_t averageOperationTimeUs;            ///< Average operation time (microseconds)
    uint64_t maxOperationTimeUs;                ///< Maximum operation time
    uint64_t minOperationTimeUs;                ///< Minimum operation time
    uint32_t busErrorCount;                     ///< Bus error occurrences
    uint32_t arbitrationLossCount;              ///< Arbitration loss count
    uint32_t clockStretchingEvents;             ///< Clock stretching events
    
    I2cStatistics() : totalOperations(0), successfulOperations(0), failedOperations(0)
        , timeoutOperations(0), bytesTransmitted(0), bytesReceived(0)
        , averageOperationTimeUs(0), maxOperationTimeUs(0), minOperationTimeUs(UINT64_MAX)
        , busErrorCount(0), arbitrationLossCount(0), clockStretchingEvents(0) {}
};

/**
 * @brief I2C diagnostic information
 */
struct I2cDiagnostics {
    bool busHealthy;                            ///< Overall bus health status
    bool sclLineState;                          ///< SCL line state (high/low)
    bool sdaLineState;                          ///< SDA line state (high/low)
    uint32_t lastErrorCode;                     ///< Last error code
    uint64_t lastErrorTimestamp;                ///< Last error timestamp
    uint32_t consecutiveErrors;                 ///< Consecutive error count
    uint32_t busRecoveryCount;                  ///< Bus recovery attempts
    double busUtilizationPercent;               ///< Bus utilization percentage
    
    I2cDiagnostics() : busHealthy(true), sclLineState(true), sdaLineState(true)
        , lastErrorCode(0), lastErrorTimestamp(0), consecutiveErrors(0)
        , busRecoveryCount(0), busUtilizationPercent(0.0) {}
};

// Callback function types
using I2cAsyncCallback = std::function<void(HfI2cErr result, size_t bytesTransferred, void* userData)>;
using I2cEventCallback = std::function<void(int eventType, void* eventData, void* userData)>;

/**
 * @class McuI2c
 * @brief Advanced I2C bus implementation for microcontrollers with integrated I2C peripherals.
 *
 * This class provides comprehensive I2C communication using the microcontroller's built-in
 * I2C peripheral with support for both basic and advanced features. On ESP32C6, it utilizes
 * the latest ESP-IDF v5.5+ I2C master driver features including asynchronous operations,
 * multi-buffer transactions, and advanced configuration options.
 *
 * Features:
 * - High-performance I2C communication using MCU's integrated controller
 * - Support for standard (100kHz), fast (400kHz), and fast+ (1MHz) modes
 * - Advanced ESP32C6/ESP-IDF v5.5+ features:
 *   - Bus-device model with separate master bus and device handles
 *   - Asynchronous I2C operations with event callbacks
 *   - Multi-buffer transactions for complex protocols
 *   - Custom transaction sequences for non-standard devices
 *   - Advanced glitch filtering and signal conditioning
 *   - Multiple clock sources for power optimization
 *   - Low-power I2C support for sleep modes
 *   - Comprehensive error handling and bus recovery
 *   - Performance monitoring and statistics
 * - Thread-safe operation with mutex protection
 * - Device scanning and presence detection
 * - Register-based communication utilities
 * - Internal pull-up resistor configuration
 * - Lazy initialization support
 *
 * @note This implementation is thread-safe when used with multiple threads.
 * @note Advanced features require ESP-IDF v5.5+ for full functionality.
 */
class McuI2c : public BaseI2c {
public:
    /**
     * @brief Constructor with basic configuration.
     * @param config I2C bus configuration parameters
     */
    explicit McuI2c(const I2cBusConfig& config) noexcept;
    
    /**
     * @brief Constructor with advanced configuration.
     * @param config Advanced I2C bus configuration parameters
     */
    explicit McuI2c(const I2cAdvancedConfig& config) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup.
     */
    ~McuI2c() noexcept override;

    //==============================================//
    // BASIC I2C OPERATIONS (BaseI2c Interface)    //
    //==============================================//

    /**
     * @brief Initialize the I2C bus.
     * @return true if successful, false otherwise
     */
    bool Initialize() noexcept override;

    /**
     * @brief Deinitialize the I2C bus.
     * @return true if successful, false otherwise
     */
    bool Deinitialize() noexcept override;

    /**
     * @brief Write data to a slave device.
     * @param device_addr 7-bit device address
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr Write(uint8_t device_addr, const uint8_t* data,
                   uint16_t length, uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Read data from a slave device.
     * @param device_addr 7-bit device address
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr Read(uint8_t device_addr, uint8_t* data,
                  uint16_t length, uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Write then read from a slave device without releasing the bus.
     * @param device_addr 7-bit device address
     * @param tx_data Data buffer to send
     * @param tx_length Number of bytes to send
     * @param rx_data Buffer to store received data
     * @param rx_length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr WriteRead(uint8_t device_addr, const uint8_t* tx_data, uint16_t tx_length,
                       uint8_t* rx_data, uint16_t rx_length, uint32_t timeout_ms = 0) noexcept override;

    //==============================================//
    // ADVANCED I2C OPERATIONS                     //
    //==============================================//

    /**
     * @brief Initialize with advanced configuration.
     * @param config Advanced configuration parameters
     * @return HfI2cErr result code
     */
    HfI2cErr initializeAdvanced(const I2cAdvancedConfig& config) noexcept;

    /**
     * @brief Reconfigure the bus with new settings.
     * @param config New configuration parameters
     * @return HfI2cErr result code
     */
    HfI2cErr reconfigure(const I2cAdvancedConfig& config) noexcept;

    /**
     * @brief Configure a specific device.
     * @param deviceConfig Device-specific configuration
     * @return HfI2cErr result code
     */
    HfI2cErr configureDevice(const I2cDeviceConfig& deviceConfig) noexcept;

    /**
     * @brief Get current bus configuration.
     * @return Current configuration
     */
    I2cAdvancedConfig getCurrentConfiguration() const noexcept;

    /**
     * @brief Reset the I2C bus.
     * @return HfI2cErr result code
     */
    HfI2cErr resetBus() noexcept;

    /**
     * @brief Validate device presence.
     * @param deviceAddress Device address to check
     * @return true if device responds, false otherwise
     */
    bool validateDevice(uint16_t deviceAddress) noexcept;

    //==============================================//
    // REGISTER-BASED OPERATIONS                   //
    //==============================================//

    /**
     * @brief Write to a device register.
     * @param deviceAddr Device address
     * @param regAddr Register address
     * @param value Value to write
     * @return HfI2cErr result code
     */
    HfI2cErr writeRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t value) noexcept;

    /**
     * @brief Read from a device register.
     * @param deviceAddr Device address
     * @param regAddr Register address
     * @param value Reference to store read value
     * @return HfI2cErr result code
     */
    HfI2cErr readRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t& value) noexcept;

    /**
     * @brief Write multiple registers.
     * @param deviceAddr Device address
     * @param startRegAddr Starting register address
     * @param data Data to write
     * @return HfI2cErr result code
     */
    HfI2cErr writeMultipleRegisters(uint16_t deviceAddr, uint8_t startRegAddr, const std::vector<uint8_t>& data) noexcept;

    /**
     * @brief Read multiple registers.
     * @param deviceAddr Device address
     * @param startRegAddr Starting register address
     * @param data Reference to store read data
     * @param count Number of registers to read
     * @return HfI2cErr result code
     */
    HfI2cErr readMultipleRegisters(uint16_t deviceAddr, uint8_t startRegAddr, std::vector<uint8_t>& data, size_t count) noexcept;

    //==============================================//
    // ASYNCHRONOUS OPERATIONS                     //
    //==============================================//

    /**
     * @brief Asynchronous write operation.
     * @param deviceAddr Device address
     * @param data Data to write
     * @param callback Completion callback
     * @param userData User data for callback
     * @return HfI2cErr result code
     */
    HfI2cErr writeAsync(uint16_t deviceAddr, const std::vector<uint8_t>& data, I2cAsyncCallback callback, void* userData = nullptr) noexcept;

    /**
     * @brief Asynchronous read operation.
     * @param deviceAddr Device address
     * @param length Number of bytes to read
     * @param callback Completion callback
     * @param userData User data for callback
     * @return HfI2cErr result code
     */
    HfI2cErr readAsync(uint16_t deviceAddr, size_t length, I2cAsyncCallback callback, void* userData = nullptr) noexcept;

    /**
     * @brief Cancel pending asynchronous operation.
     * @param operationId Operation ID to cancel
     * @return HfI2cErr result code
     */
    HfI2cErr cancelAsyncOperation(uint32_t operationId) noexcept;

    /**
     * @brief Set event callback for bus events.
     * @param callback Event callback function
     * @param userData User data for callback
     */
    void setEventCallback(I2cEventCallback callback, void* userData = nullptr) noexcept;

    //==============================================//
    // MULTI-BUFFER TRANSACTIONS                   //
    //==============================================//

    /**
     * @brief Execute multi-buffer transaction.
     * @param transaction Transaction descriptor
     * @return HfI2cErr result code
     */
    HfI2cErr executeMultiBufferTransaction(const I2cMultiBufferTransaction& transaction) noexcept;

    /**
     * @brief Execute multi-buffer transaction asynchronously.
     * @param transaction Transaction descriptor
     * @param callback Completion callback
     * @param userData User data for callback
     * @return HfI2cErr result code
     */
    HfI2cErr executeMultiBufferTransactionAsync(const I2cMultiBufferTransaction& transaction, I2cAsyncCallback callback, void* userData = nullptr) noexcept;

    //==============================================//
    // CUSTOM COMMAND SEQUENCES                    //
    //==============================================//

    /**
     * @brief Execute custom command sequence.
     * @param commands Vector of custom commands
     * @return HfI2cErr result code
     */
    HfI2cErr executeCustomSequence(const std::vector<I2cCustomCommand>& commands) noexcept;

    /**
     * @brief Execute custom command sequence asynchronously.
     * @param commands Vector of custom commands
     * @param callback Completion callback
     * @param userData User data for callback
     * @return HfI2cErr result code
     */
    HfI2cErr executeCustomSequenceAsync(const std::vector<I2cCustomCommand>& commands, I2cAsyncCallback callback, void* userData = nullptr) noexcept;

    //==============================================//
    // POWER MANAGEMENT                            //
    //==============================================//

    /**
     * @brief Set power mode.
     * @param mode Power mode to set
     * @return HfI2cErr result code
     */
    HfI2cErr setPowerMode(HfI2cPowerMode mode) noexcept;

    /**
     * @brief Get current power mode.
     * @return Current power mode
     */
    HfI2cPowerMode getPowerMode() const noexcept;

    /**
     * @brief Suspend bus operation for power saving.
     * @return HfI2cErr result code
     */
    HfI2cErr suspendBus() noexcept;

    /**
     * @brief Resume bus operation from suspended state.
     * @return HfI2cErr result code
     */
    HfI2cErr resumeBus() noexcept;

    //==============================================//
    // STATISTICS AND DIAGNOSTICS                  //
    //==============================================//

    /**
     * @brief Get operation statistics.
     * @return Current statistics
     */
    I2cStatistics getStatistics() const noexcept;

    /**
     * @brief Reset operation statistics.
     */
    void resetStatistics() noexcept;

    /**
     * @brief Get diagnostic information.
     * @return Current diagnostics
     */
    I2cDiagnostics getDiagnostics() noexcept;

    /**
     * @brief Check bus health status.
     * @return true if bus is healthy, false otherwise
     */
    bool isBusHealthy() noexcept;

    //==============================================//
    // DEVICE MANAGEMENT                           //
    //==============================================//

    /**
     * @brief Scan for devices on the bus.
     * @param foundDevices Vector to store found device addresses
     * @param startAddr Starting address for scan (default: 0x08)
     * @param endAddr Ending address for scan (default: 0x77)
     * @return Number of devices found
     */
    size_t scanDevices(std::vector<uint16_t>& foundDevices, uint16_t startAddr = 0x08, uint16_t endAddr = 0x77) noexcept;

    /**
     * @brief Add device configuration.
     * @param deviceConfig Device configuration to add
     * @return HfI2cErr result code
     */
    HfI2cErr addDevice(const I2cDeviceConfig& deviceConfig) noexcept;

    /**
     * @brief Remove device configuration.
     * @param deviceAddress Device address to remove
     * @return HfI2cErr result code
     */
    HfI2cErr removeDevice(uint16_t deviceAddress) noexcept;

    //==============================================//
    // ENHANCED METHODS                             //
    //==============================================//

    /**
     * @brief Check if the I2C bus is busy.
     * @return true if busy, false if available
     */
    bool IsBusy() noexcept;

    /**
     * @brief Reset the I2C bus in case of errors.
     * @return true if reset successful, false otherwise
     */
    bool ResetBus() noexcept;

    /**
     * @brief Get the last error that occurred.
     * @return Last error code
     */
    HfI2cErr GetLastError() const noexcept {
        return last_error_;
    }

    /**
     * @brief Set a new clock speed (requires reinitialization).
     * @param clock_speed_hz New clock speed in Hz
     * @return true if successful, false otherwise
     */
    bool SetClockSpeed(uint32_t clock_speed_hz) noexcept;

    /**
     * @brief Enable or disable internal pull-up resistors.
     * @param enable True to enable pull-ups, false to disable
     * @return true if successful, false otherwise
     */
    bool SetPullUps(bool enable) noexcept;

    /**
     * @brief Get detailed bus status information.
     * @return Platform-specific status information
     */
    uint32_t GetBusStatus() noexcept;

    /**
     * @brief Perform a bus recovery sequence.
     * @return true if recovery successful, false otherwise
     */
    bool RecoverBus() noexcept;

private:
    //==============================================//
    // PRIVATE METHODS                              //
    //==============================================//

    /**
     * @brief Convert platform-specific error to HfI2cErr.
     * @param platform_error Platform-specific error code
     * @return Corresponding HfI2cErr
     */
    HfI2cErr ConvertPlatformError(int32_t platform_error) noexcept;

    /**
     * @brief Validate device address.
     * @param device_addr Device address to validate
     * @return true if valid, false otherwise
     */
    bool IsValidDeviceAddress(uint8_t device_addr) const noexcept {
        // Valid 7-bit I2C addresses are 0x08-0x77 (avoiding reserved addresses)
        return (device_addr >= 0x08 && device_addr <= 0x77);
    }

    /**
     * @brief Get timeout value (use default if timeout_ms is 0).
     * @param timeout_ms Requested timeout
     * @return Actual timeout to use
     */
    uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept {
        return (timeout_ms == 0) ? config_.timeout_ms : timeout_ms;
    }

    /**
     * @brief Perform platform-specific initialization.
     * @return true if successful, false otherwise
     */
    bool PlatformInitialize() noexcept;

    /**
     * @brief Perform platform-specific deinitialization.
     * @return true if successful, false otherwise
     */
    bool PlatformDeinitialize() noexcept;

    /**
     * @brief Update operation statistics.
     * @param success Operation success status
     * @param bytesTransferred Number of bytes transferred
     * @param operationTimeUs Operation time in microseconds
     */
    void UpdateStatistics(bool success, size_t bytesTransferred, uint64_t operationTimeUs) noexcept;

    /**
     * @brief Handle platform-specific error.
     * @param error Platform error code
     */
    void HandlePlatformError(int32_t error) noexcept;

    /**
     * @brief Initialize ESP32 I2C master bus.
     * @return HfI2cErr result code
     */
    HfI2cErr InitializeEsp32Master() noexcept;

    /**
     * @brief Create device handle for ESP32.
     * @param deviceAddr Device address
     * @return Device handle or nullptr on failure
     */
    void* CreateEsp32DeviceHandle(uint16_t deviceAddr) noexcept;

    //==============================================//
    // PRIVATE MEMBER VARIABLES                    //
    //==============================================//

    // Configuration
    I2cAdvancedConfig advanced_config_;         ///< Advanced configuration
    I2cBusConfig config_;                       ///< Basic configuration (for compatibility)
    bool use_advanced_config_;                  ///< Flag indicating advanced config usage

    // Platform-specific handles
    void* platform_handle_;                    ///< Platform-specific I2C handle
    void* master_bus_handle_;                  ///< ESP32 master bus handle
    std::unordered_map<uint16_t, void*> device_handles_; ///< Device handles map

    // State management
    mutable std::mutex mutex_;                  ///< Thread synchronization mutex
    HfI2cErr last_error_;                      ///< Last error that occurred
    uint64_t transaction_count_;               ///< Transaction counter
    bool bus_locked_;                          ///< Bus lock status
    bool advanced_initialized_;                ///< Advanced features initialized flag

    // Device configurations
    std::unordered_map<uint16_t, I2cDeviceConfig> device_configs_; ///< Device-specific configurations

    // Asynchronous operation support
    std::unordered_map<uint32_t, void*> async_operations_; ///< Active async operations
    uint32_t next_operation_id_;               ///< Next operation ID
    I2cEventCallback event_callback_;          ///< Event callback function
    void* event_callback_userdata_;           ///< Event callback user data

    // Statistics and diagnostics
    mutable I2cStatistics statistics_;         ///< Operation statistics
    I2cDiagnostics diagnostics_;               ///< Diagnostic information
    uint64_t last_operation_time_;             ///< Last operation timestamp

    // Power management
    HfI2cPowerMode current_power_mode_;        ///< Current power mode
    bool bus_suspended_;                       ///< Bus suspension state

    // Platform-specific constants
    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
    static constexpr uint32_t MAX_TRANSFER_SIZE = 4092;  // ESP32 limitation
    static constexpr uint8_t MAX_DEVICES = 127;         // Maximum I2C devices
};

#endif // MCU_I2C_H
