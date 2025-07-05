/**
 * @file McuTypes_I2C.h
 * @brief Complete I2C type definitions for ESP-IDF v5.5+ hardware abstraction.
 *
 * This header provides the definitive collection of all I2C-related types, structures,
 * enums, and constants for the HardFOC system. Designed specifically for ESP-IDF v5.5+
 * and ESP32C6, this abstraction layer provides clean, platform-agnostic types while
 * exposing all the advanced features of the modern ESP-IDF I2C driver.
 *
 * @section features ESP32C6/ESP-IDF v5.5+ Feature Support
 * - **New Bus-Device Model**: Separate bus and device handle management
 * - **Asynchronous Operations**: Non-blocking I2C with event callbacks
 * - **Advanced Transactions**: Multi-buffer and custom command sequences
 * - **Signal Conditioning**: Digital/analog glitch filtering and clock stretching
 * - **Power Management**: Multiple clock sources and low-power modes
 * - **Comprehensive Monitoring**: Real-time statistics and bus health diagnostics
 * - **Thread Safety**: Full RTOS integration with proper synchronization
 * - **Hardware Acceleration**: DMA transfers and interrupt-driven operation
 * - **Error Recovery**: Automatic bus recovery and comprehensive error handling
 *
 * @section organization Code Organization
 * - Platform-specific type mappings and includes
 * - Core I2C enumerations (clock sources, power modes, etc.)
 * - Configuration structures (bus, device, slave configurations)
 * - Transaction structures (multi-buffer, custom commands)
 * - Monitoring structures (statistics, diagnostics)
 * - Callback function signatures
 * - Hardware specifications and validation macros
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * @version 2.0.0 - Complete ESP-IDF v5.5+ rewrite
 *
 * @note All types are compatible with ESP-IDF v5.5+ and ESP32C6 hardware.
 * @note Legacy API support has been completely removed for cleaner abstraction.
 * @note All structs and enums are defined here for clean separation of concerns.
 */

#pragma once

#include "McuTypes_Base.h"  // Base types and platform selection
#include "BaseI2c.h"  // For HfI2cErr

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <chrono>
#include <cstdint>

//==============================================================================
// PLATFORM-SPECIFIC I2C TYPE MAPPINGS (ESP-IDF v5.5+)
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 I2C v5.5+ specific includes - New bus-device model
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "driver/i2c_types.h"
#include "hal/i2c_types.h"
#include "esp_err.h"

// Direct platform type mappings for ESP-IDF v5.5+
using I2cPort = i2c_port_t;
using I2cMasterBusHandle = i2c_master_bus_handle_t;
using I2cMasterDevHandle = i2c_master_dev_handle_t;
using I2cSlaveDevHandle = i2c_slave_dev_handle_t;
using GpioNum = gpio_num_t;
using EspErr = esp_err_t;

#else
// Generic/mock types for non-ESP32 platforms
using I2cPort = int;
using I2cMasterBusHandle = void*;
using I2cMasterDevHandle = void*;
using I2cSlaveDevHandle = void*;
using GpioNum = int;
using EspErr = int;
#endif

//==============================================================================
// I2C CORE ENUMERATIONS (ESP-IDF v5.5+ ALIGNED)
//==============================================================================

/**
 * @brief I2C clock source options for ESP32C6.
 * @details Clock source selection affects power consumption and performance.
 *          APB_CLK provides the best precision, XTAL_CLK enables low-power modes,
 *          RC_FAST_CLK provides lowest power consumption but less precision.
 */
enum class I2cClockSource : uint8_t {
    DEFAULT = 0,    ///< Use default clock source (typically APB)
    APB_CLK = 1,    ///< APB clock (most precise, highest power)
    XTAL_CLK = 2,   ///< Crystal oscillator (good precision, lower power)
    RC_FAST_CLK = 3 ///< RC fast clock (lowest power, least precise)
};

/**
 * @brief I2C address bit length configuration.
 * @details Determines whether to use 7-bit or 10-bit addressing mode.
 */
enum class I2cAddressBits : uint8_t {
    ADDR_7_BIT = 0,  ///< 7-bit addressing (standard)
    ADDR_10_BIT = 1  ///< 10-bit addressing (extended)
};

/**
 * @brief I2C power mode configuration for energy optimization.
 * @details Different power modes balance performance with energy consumption.
 */
enum class I2cPowerMode : uint8_t {
    FULL_POWER = 0,     ///< Full performance mode
    LOW_POWER = 1,      ///< Reduced power consumption
    SLEEP_MODE = 2,     ///< Minimum power for sleep-compatible operation
    DEEP_SLEEP = 3      ///< Deepest sleep mode (may require reinitialization)
};

/**
 * @brief I2C transaction types for operation classification.
 * @details Used internally to track and optimize different transaction patterns.
 */
enum class I2cTransactionType : uint8_t {
    WRITE_ONLY = 0,        ///< Pure write transaction
    READ_ONLY = 1,         ///< Pure read transaction
    WRITE_READ = 2,        ///< Combined write-then-read transaction
    MULTI_BUFFER = 3,      ///< Multiple buffer transaction
    CUSTOM_SEQUENCE = 4,   ///< Custom command sequence
    REGISTER_ACCESS = 5    ///< Register-based access
};

/**
 * @brief I2C event types for callback notifications.
 * @details Events that can be reported through the event callback system.
 */
enum class I2cEventType : int {
    TRANSACTION_COMPLETE = 0,    ///< Transaction completed successfully
    TRANSACTION_ERROR = 1,       ///< Transaction failed
    BUS_ERROR = 2,              ///< Bus-level error occurred
    DEVICE_NACK = 3,            ///< Device sent NACK
    ARBITRATION_LOST = 4,       ///< Lost arbitration in multi-master
    TIMEOUT = 5,                ///< Operation timed out
    CLOCK_STRETCH_TIMEOUT = 6,  ///< Clock stretching timeout
    BUS_RECOVERED = 7,          ///< Bus recovery completed
    POWER_MODE_CHANGED = 8,     ///< Power mode changed
    DEVICE_ADDED = 9,           ///< Device added to bus
    DEVICE_REMOVED = 10         ///< Device removed from bus
};

/**
 * @brief I2C glitch filter configuration.
 * @details Controls the digital glitch filtering capability.
 */
enum class I2cGlitchFilter : uint8_t {
    DISABLED = 0,        ///< No glitch filtering
    FILTER_1_CYCLE = 1,  ///< Filter glitches <= 1 APB cycle
    FILTER_2_CYCLES = 2, ///< Filter glitches <= 2 APB cycles
    FILTER_3_CYCLES = 3, ///< Filter glitches <= 3 APB cycles
    FILTER_4_CYCLES = 4, ///< Filter glitches <= 4 APB cycles
    FILTER_5_CYCLES = 5, ///< Filter glitches <= 5 APB cycles
    FILTER_6_CYCLES = 6, ///< Filter glitches <= 6 APB cycles
    FILTER_7_CYCLES = 7  ///< Filter glitches <= 7 APB cycles (maximum)
};

/**
 * @brief I2C command types for custom sequence operations.
 * @details Defines the types of commands that can be executed in custom sequences.
 */
enum class I2cCommandType : uint8_t {
    WRITE = 0,       ///< Write data to device
    READ = 1,        ///< Read data from device
    WRITE_READ = 2,  ///< Write then read in single transaction
    DELAY = 3,       ///< Insert delay between operations
    START = 4,       ///< Generate start condition
    STOP = 5,        ///< Generate stop condition
    RESTART = 6      ///< Generate restart condition
};

//==============================================================================
// I2C CALLBACK FUNCTION SIGNATURES
//==============================================================================

// Forward declarations for callback structures
struct I2cAsyncResult;

/**
 * @brief Callback function signature for asynchronous I2C operations.
 * @param result Operation result (success/error code)
 * @param bytes_transferred Number of bytes successfully transferred
 * @param user_data User-provided data pointer
 * 
 * @note This callback is executed in interrupt context - keep it minimal and fast!
 * @note Avoid blocking operations, heap allocation, or complex computations.
 * @note Use FreeRTOS queue/semaphore mechanisms to communicate with tasks.
 */
using I2cAsyncCallback = std::function<void(HfI2cErr result, size_t bytes_transferred, void* user_data)>;

/**
 * @brief Callback function signature for I2C event notifications.
 * @param event_type Type of event that occurred
 * @param event_data Event-specific data (may be nullptr)
 * @param user_data User-provided data pointer
 * 
 * @note This callback is executed in interrupt context - keep it minimal and fast!
 * @note Event data lifetime is only valid during the callback execution.
 * @note Use FreeRTOS primitives to safely communicate with application tasks.
 */
using I2cEventCallback = std::function<void(I2cEventType event_type, void* event_data, void* user_data)>;

/**
 * @brief Detailed result structure for async operations.
 * @details Provides comprehensive information about async operation completion.
 */
struct I2cAsyncResult {
    HfI2cErr error_code;        ///< Operation result code
    size_t bytes_transferred;   ///< Number of bytes successfully transferred
    uint32_t operation_id;      ///< Unique operation identifier
    uint64_t completion_time_us; ///< Completion timestamp in microseconds
    I2cTransactionType transaction_type; ///< Type of transaction completed
};

//==============================================================================
// I2C CONFIGURATION STRUCTURES (ESP-IDF v5.5+ ALIGNED)
//==============================================================================

/**
 * @brief Master I2C bus configuration for ESP-IDF v5.5+ bus-device model.
 * @details Comprehensive bus-level configuration supporting all ESP32C6 features.
 *          This structure configures the master bus which can support multiple devices.
 */
struct I2cMasterBusConfig {
    // Basic bus configuration
    I2cPort i2c_port;                  ///< I2C port number (0 or 1 for ESP32C6)
    GpioNum sda_io_num;                 ///< SDA GPIO pin number
    GpioNum scl_io_num;                 ///< SCL GPIO pin number
    bool enable_internal_pullup;        ///< Enable internal pull-up resistors

    // Clock and timing
    I2cClockSource clk_source;          ///< Clock source selection
    uint32_t clk_flags;                 ///< Additional clock configuration flags
    I2cGlitchFilter glitch_ignore_cnt;  ///< Digital glitch filter length

    // Advanced features
    uint32_t intr_priority;             ///< Interrupt priority (0-7, 0=lowest)
    uint32_t trans_queue_depth;         ///< Transaction queue depth for async ops
    uint32_t flags;                     ///< Additional configuration flags

    // Power management
    bool allow_pd;                      ///< Allow power down in sleep modes

    /**
     * @brief Default constructor with ESP32C6-optimized settings.
     */
    I2cMasterBusConfig() noexcept
        : i2c_port(static_cast<I2cPort>(0)), sda_io_num(static_cast<GpioNum>(HF_INVALID_PIN)), 
          scl_io_num(static_cast<GpioNum>(HF_INVALID_PIN)),
          enable_internal_pullup(true), clk_source(I2cClockSource::DEFAULT),
          clk_flags(0), glitch_ignore_cnt(I2cGlitchFilter::FILTER_7_CYCLES),
          intr_priority(0), trans_queue_depth(16), flags(0), allow_pd(false) {}
};

/**
 * @brief I2C device configuration for individual devices on the bus.
 * @details Device-specific configuration that works with the master bus to provide
 *          per-device customization of timing, addressing, and behavior.
 */
struct I2cDeviceConfig {
    uint16_t device_address;             ///< 7-bit or 10-bit device address
    I2cAddressBits dev_addr_length;      ///< Address bit length (7 or 10 bit)
    uint32_t scl_speed_hz;              ///< SCL clock frequency for this device
    uint32_t scl_wait_us;               ///< SCL wait time in microseconds
    uint32_t flags;                     ///< Device-specific configuration flags

    /**
     * @brief Default constructor with standard I2C device settings.
     */
    I2cDeviceConfig() noexcept
        : device_address(0), dev_addr_length(I2cAddressBits::ADDR_7_BIT),
          scl_speed_hz(100000), scl_wait_us(0), flags(0) {}
};

/**
 * @brief I2C slave device configuration for slave mode operation.
 * @details Configuration for I2C slave mode with callback support and buffering.
 */
struct I2cSlaveConfig {
    // Basic slave configuration
    I2cPort i2c_port;                   ///< I2C port number
    I2cClockSource clk_source;          ///< Clock source selection
    GpioNum scl_io_num;                 ///< SCL GPIO pin
    GpioNum sda_io_num;                 ///< SDA GPIO pin
    uint16_t slave_addr;                ///< Slave address
    I2cAddressBits addr_bit_len;        ///< Address bit length

    // Buffer configuration
    uint32_t send_buf_depth;            ///< Send buffer depth
    uint32_t receive_buf_depth;         ///< Receive buffer depth

    // Advanced slave features
    bool enable_internal_pullup;        ///< Enable internal pull-ups
    bool broadcast_en;                  ///< Enable general call address (0x00) response
    bool allow_pd;                      ///< Allow power down in sleep modes
    uint32_t intr_priority;             ///< Interrupt priority

    /**
     * @brief Default constructor with standard slave settings.
     */
    I2cSlaveConfig() noexcept
        : i2c_port(static_cast<I2cPort>(0)), clk_source(I2cClockSource::DEFAULT),
          scl_io_num(static_cast<GpioNum>(HF_INVALID_PIN)), 
          sda_io_num(static_cast<GpioNum>(HF_INVALID_PIN)),
          slave_addr(0), addr_bit_len(I2cAddressBits::ADDR_7_BIT),
          send_buf_depth(256), receive_buf_depth(256), enable_internal_pullup(true),
          broadcast_en(false), allow_pd(false), intr_priority(0) {}
};

//==============================================================================
// I2C TRANSACTION STRUCTURES
//==============================================================================

/**
 * @brief Buffer descriptor for multi-buffer transactions.
 * @details Describes a single buffer in a complex transaction sequence.
 */
struct I2cTransactionBuffer {
    const uint8_t* data;        ///< Pointer to buffer data
    size_t length;              ///< Buffer length in bytes
    bool is_write;              ///< true = write buffer, false = read buffer

    /**
     * @brief Constructor for write buffer.
     */
    I2cTransactionBuffer(const uint8_t* buf, size_t len) noexcept
        : data(buf), length(len), is_write(true) {}

    /**
     * @brief Constructor with explicit read/write flag.
     */
    I2cTransactionBuffer(const uint8_t* buf, size_t len, bool write) noexcept
        : data(buf), length(len), is_write(write) {}
};

/**
 * @brief Multi-buffer transaction for complex I2C protocols.
 * @details Supports protocols requiring multiple read/write operations in sequence
 *          without releasing the bus between operations.
 */
struct I2cMultiBufferTransaction {
    uint16_t device_address;                    ///< Target device address
    std::vector<I2cTransactionBuffer> buffers;  ///< Buffer sequence
    uint32_t timeout_ms;                        ///< Transaction timeout
    uint32_t flags;                            ///< Transaction flags

    /**
     * @brief Default constructor.
     */
    I2cMultiBufferTransaction() noexcept
        : device_address(0), timeout_ms(1000), flags(0) {}
};

/**
 * @brief Custom I2C command for non-standard protocols.
 * @details Provides flexibility for implementing custom I2C sequences.
 */
struct I2cCustomCommand {
    /**
     * @brief Command type enumeration.
     */
    enum class Type : uint8_t {
        WRITE = 0,          ///< Write command
        READ = 1,           ///< Read command
        START = 2,          ///< Generate start condition
        STOP = 3,           ///< Generate stop condition
        RESTART = 4,        ///< Generate restart condition
        DELAY = 5,          ///< Insert delay
        PROBE = 6           ///< Probe for device presence
    };

    Type command_type;              ///< Command type
    std::vector<uint8_t> data;      ///< Command data (if applicable)
    uint32_t delay_us;              ///< Delay in microseconds (for DELAY command)
    uint32_t flags;                 ///< Command-specific flags

    /**
     * @brief Constructor for basic command.
     */
    explicit I2cCustomCommand(Type type) noexcept
        : command_type(type), delay_us(0), flags(0) {}

    /**
     * @brief Constructor for delay command.
     */
    explicit I2cCustomCommand(uint32_t delay_microseconds) noexcept
        : command_type(Type::DELAY), delay_us(delay_microseconds), flags(0) {}
};

//==============================================================================
// I2C MONITORING AND DIAGNOSTICS STRUCTURES
//==============================================================================

/**
 * @brief I2C operation statistics for performance monitoring.
 * @details Comprehensive statistics for analyzing I2C bus performance and health.
 */
struct I2cStatistics {
    // Transaction counters
    std::atomic<uint64_t> total_transactions{0};     ///< Total transactions attempted
    std::atomic<uint64_t> successful_transactions{0}; ///< Successful transactions
    std::atomic<uint64_t> failed_transactions{0};    ///< Failed transactions
    std::atomic<uint64_t> timeout_count{0};          ///< Transaction timeouts

    // Byte counters
    std::atomic<uint64_t> bytes_written{0};          ///< Total bytes written
    std::atomic<uint64_t> bytes_read{0};             ///< Total bytes read
    
    // Timing statistics
    std::atomic<uint64_t> total_transaction_time_us{0}; ///< Total transaction time
    std::atomic<uint32_t> max_transaction_time_us{0};   ///< Longest transaction time
    std::atomic<uint32_t> min_transaction_time_us{UINT32_MAX}; ///< Shortest transaction time

    // Error counters
    std::atomic<uint32_t> nack_errors{0};            ///< NACK error count
    std::atomic<uint32_t> bus_errors{0};             ///< Bus error count
    std::atomic<uint32_t> arbitration_lost_count{0}; ///< Arbitration lost count
    std::atomic<uint32_t> clock_stretch_timeouts{0}; ///< Clock stretch timeouts

    // Device management
    std::atomic<uint32_t> devices_added{0};          ///< Devices added to bus
    std::atomic<uint32_t> devices_removed{0};        ///< Devices removed from bus

    /**
     * @brief Get average transaction time in microseconds.
     */
    uint32_t GetAverageTransactionTime() const noexcept {
        auto total = total_transactions.load();
        return total > 0 ? static_cast<uint32_t>(total_transaction_time_us.load() / total) : 0;
    }

    /**
     * @brief Get success rate as percentage (0-100).
     */
    float GetSuccessRate() const noexcept {
        auto total = total_transactions.load();
        return total > 0 ? (static_cast<float>(successful_transactions.load()) / total) * 100.0f : 0.0f;
    }

    /**
     * @brief Reset all statistics.
     */
    void Reset() noexcept {
        total_transactions = 0;
        successful_transactions = 0;
        failed_transactions = 0;
        timeout_count = 0;
        bytes_written = 0;
        bytes_read = 0;
        total_transaction_time_us = 0;
        max_transaction_time_us = 0;
        min_transaction_time_us = UINT32_MAX;
        nack_errors = 0;
        bus_errors = 0;
        arbitration_lost_count = 0;
        clock_stretch_timeouts = 0;
        devices_added = 0;
        devices_removed = 0;
    }
};

/**
 * @brief I2C bus diagnostics for health monitoring.
 * @details Real-time diagnostics information for troubleshooting and monitoring.
 */
struct I2cDiagnostics {
    // Bus health indicators
    bool bus_healthy;                           ///< Overall bus health status
    bool sda_line_state;                        ///< Current SDA line state
    bool scl_line_state;                        ///< Current SCL line state
    bool bus_locked;                           ///< Bus lock status

    // Error information
    HfI2cErr last_error_code;                  ///< Last error code encountered
    uint64_t last_error_timestamp_us;          ///< Timestamp of last error
    uint32_t consecutive_errors;               ///< Consecutive error count
    uint32_t error_recovery_attempts;          ///< Bus recovery attempts

    // Performance metrics
    float bus_utilization_percent;             ///< Bus utilization percentage
    uint32_t average_response_time_us;         ///< Average device response time
    uint32_t clock_stretching_events;          ///< Clock stretching event count

    // Power management status
    I2cPowerMode current_power_mode;           ///< Current power mode
    bool auto_suspend_enabled;                 ///< Auto-suspend feature status
    uint64_t last_activity_timestamp_us;       ///< Last bus activity timestamp

    // Device management
    uint32_t active_device_count;              ///< Number of active devices on bus
    uint32_t total_device_scans;               ///< Total device scan operations
    uint32_t devices_found_last_scan;          ///< Devices found in last scan

    /**
     * @brief Default constructor with healthy defaults.
     */
    I2cDiagnostics() noexcept
        : bus_healthy(true), sda_line_state(true), scl_line_state(true), bus_locked(false),
          last_error_code(HfI2cErr::I2C_SUCCESS), last_error_timestamp_us(0), consecutive_errors(0),
          error_recovery_attempts(0), bus_utilization_percent(0.0f), average_response_time_us(0),
          clock_stretching_events(0), current_power_mode(I2cPowerMode::FULL_POWER),
          auto_suspend_enabled(false), last_activity_timestamp_us(0), active_device_count(0),
          total_device_scans(0), devices_found_last_scan(0) {}
};

//==============================================================================
// ESP32C6 I2C HARDWARE SPECIFICATIONS AND VALIDATION
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6
// ESP32C6-specific I2C hardware specifications (ESP-IDF v5.5+)
static constexpr uint8_t I2C_MAX_CONTROLLERS = 2;             ///< ESP32C6 has 2 I2C controllers
static constexpr uint32_t I2C_MIN_CLOCK_SPEED = 1000;         ///< Minimum I2C clock speed (1kHz)
static constexpr uint32_t I2C_STD_CLOCK_SPEED = 100000;       ///< Standard mode: 100kHz
static constexpr uint32_t I2C_FAST_CLOCK_SPEED = 400000;      ///< Fast mode: 400kHz
static constexpr uint32_t I2C_FAST_PLUS_CLOCK_SPEED = 1000000; ///< Fast mode plus: 1MHz (ESP32C6)
static constexpr uint32_t I2C_MAX_CLOCK_SPEED = 1000000;      ///< Maximum I2C clock speed (1MHz)
static constexpr uint32_t I2C_MAX_TRANSFER_SIZE = 4092;       ///< Maximum single transfer size (bytes)
static constexpr uint32_t I2C_FIFO_SIZE = 32;                 ///< Hardware FIFO size (bytes)
static constexpr uint8_t I2C_MAX_GLITCH_FILTER = 7;           ///< Maximum glitch filter length
static constexpr uint16_t I2C_MIN_DEVICE_ADDR = 0x08;         ///< Minimum valid device address
static constexpr uint16_t I2C_MAX_DEVICE_ADDR_7BIT = 0x77;    ///< Maximum 7-bit device address
static constexpr uint16_t I2C_MAX_DEVICE_ADDR_10BIT = 0x3FF;  ///< Maximum 10-bit device address
static constexpr uint32_t I2C_DEFAULT_TIMEOUT_MS = 1000;      ///< Default operation timeout
static constexpr uint32_t I2C_MAX_TIMEOUT_MS = 60000;         ///< Maximum operation timeout
static constexpr uint8_t I2C_MAX_RETRY_COUNT = 10;            ///< Maximum retry attempts
static constexpr uint32_t I2C_DEFAULT_QUEUE_DEPTH = 8;        ///< Default transaction queue depth
static constexpr uint32_t I2C_MAX_QUEUE_DEPTH = 64;           ///< Maximum transaction queue depth
static constexpr uint32_t I2C_CLOCK_STRETCH_TIMEOUT_US = 10000; ///< Default clock stretch timeout

#else
// Generic constants for non-ESP32C6 platforms
static constexpr uint8_t I2C_MAX_CONTROLLERS = 2;
static constexpr uint32_t I2C_MIN_CLOCK_SPEED = 1000;
static constexpr uint32_t I2C_STD_CLOCK_SPEED = 100000;
static constexpr uint32_t I2C_FAST_CLOCK_SPEED = 400000;
static constexpr uint32_t I2C_MAX_CLOCK_SPEED = 400000;
static constexpr uint32_t I2C_MAX_TRANSFER_SIZE = 4096;
static constexpr uint32_t I2C_FIFO_SIZE = 32;
static constexpr uint8_t I2C_MAX_GLITCH_FILTER = 7;
static constexpr uint16_t I2C_MIN_DEVICE_ADDR = 0x08;
static constexpr uint16_t I2C_MAX_DEVICE_ADDR_7BIT = 0x77;
static constexpr uint16_t I2C_MAX_DEVICE_ADDR_10BIT = 0x3FF;
static constexpr uint32_t I2C_DEFAULT_TIMEOUT_MS = 1000;
static constexpr uint32_t I2C_MAX_TIMEOUT_MS = 60000;
static constexpr uint8_t I2C_MAX_RETRY_COUNT = 10;
static constexpr uint32_t I2C_DEFAULT_QUEUE_DEPTH = 8;
static constexpr uint32_t I2C_MAX_QUEUE_DEPTH = 64;
static constexpr uint32_t I2C_CLOCK_STRETCH_TIMEOUT_US = 10000;
#endif

//==============================================================================
// I2C VALIDATION MACROS
//==============================================================================

/// @brief Validate I2C port number
#define I2C_IS_VALID_PORT(port) ((port) < I2C_MAX_CONTROLLERS)

/// @brief Validate I2C clock speed
#define I2C_IS_VALID_CLOCK_SPEED(speed) \
    ((speed) >= I2C_MIN_CLOCK_SPEED && (speed) <= I2C_MAX_CLOCK_SPEED)

/// @brief Validate 7-bit I2C device address
#define I2C_IS_VALID_DEVICE_ADDR_7BIT(addr) \
    ((addr) >= I2C_MIN_DEVICE_ADDR && (addr) <= I2C_MAX_DEVICE_ADDR_7BIT)

/// @brief Validate 10-bit I2C device address
#define I2C_IS_VALID_DEVICE_ADDR_10BIT(addr) \
    ((addr) >= I2C_MIN_DEVICE_ADDR && (addr) <= I2C_MAX_DEVICE_ADDR_10BIT)

/// @brief Validate any I2C device address (7-bit or 10-bit)
#define I2C_IS_VALID_DEVICE_ADDR(addr) \
    ((addr) >= I2C_MIN_DEVICE_ADDR && (addr) <= I2C_MAX_DEVICE_ADDR_10BIT)

/// @brief Validate I2C transfer size
#define I2C_IS_VALID_TRANSFER_SIZE(size) \
    ((size) > 0 && (size) <= I2C_MAX_TRANSFER_SIZE)

/// @brief Validate glitch filter setting
#define I2C_IS_VALID_GLITCH_FILTER(filter) \
    ((filter) <= I2C_MAX_GLITCH_FILTER)

/// @brief Validate timeout value
#define I2C_IS_VALID_TIMEOUT(timeout) \
    ((timeout) == 0 || ((timeout) > 0 && (timeout) <= I2C_MAX_TIMEOUT_MS))

/// @brief Validate retry count
#define I2C_IS_VALID_RETRY_COUNT(count) \
    ((count) <= I2C_MAX_RETRY_COUNT)

/// @brief Validate transaction queue depth
#define I2C_IS_VALID_QUEUE_DEPTH(depth) \
    ((depth) > 0 && (depth) <= I2C_MAX_QUEUE_DEPTH)
