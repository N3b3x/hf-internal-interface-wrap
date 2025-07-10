/**
 * @file EspTypes_I2C.h
 * @brief ESP32 I2C type definitions for hardware abstraction.
 *
 * This header defines only the essential I2C-specific types used by
 * the EspI2c implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseI2c.h"         // For hf_i2c_err_t
#include "HardwareTypes.h"   // For basic hardware types
#include "McuSelect.h"       // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"
#include <functional>
#include <vector>

//==============================================================================
// ESP32 I2C TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspI2c implementation

//==============================================================================
// ESP32 I2C CONSTANTS
//==============================================================================

static constexpr uint8_t HF_I2C_MAX_PORTS = 2;
static constexpr uint32_t HF_I2C_MAX_FREQ_HZ = 1000000;
static constexpr uint32_t HF_I2C_MIN_FREQ_HZ = 1000;
static constexpr uint32_t HF_I2C_DEFAULT_FREQ_HZ = 100000;
static constexpr uint32_t HF_I2C_FAST_FREQ_HZ = 400000;
static constexpr uint32_t HF_I2C_FAST_PLUS_FREQ_HZ = 1000000;
static constexpr uint32_t HF_I2C_MAX_TRANSFER_BYTES = 1024;
static constexpr uint32_t HF_I2C_FIFO_SIZE = 32;
static constexpr uint32_t HF_I2C_DEFAULT_TIMEOUT_MS = 1000;
static constexpr uint32_t HF_I2C_MAX_TIMEOUT_MS = 10000;
static constexpr uint8_t HF_I2C_MAX_RETRY_COUNT = 3;

//==============================================================================
// ESP32 I2C ENUMS
//==============================================================================

/**
 * @brief ESP32 I2C clock source selection.
 */
enum class hf_i2c_clock_source_t : uint8_t {
  HF_I2C_CLK_SRC_DEFAULT = 0, ///< Default clock source
  HF_I2C_CLK_SRC_APB = 1,     ///< APB clock
  HF_I2C_CLK_SRC_XTAL = 2     ///< Crystal oscillator
};

/**
 * @brief ESP32 I2C address bit length.
 */
enum class hf_i2c_address_bits_t : uint8_t {
  HF_I2C_ADDR_7_BIT = 7,   ///< 7-bit address
  HF_I2C_ADDR_10_BIT = 10  ///< 10-bit address
};

/**
 * @brief ESP32 I2C power mode configuration.
 */
enum class hf_i2c_power_mode_t : uint8_t {
  HF_I2C_POWER_MODE_NORMAL = 0, ///< Normal power mode
  HF_I2C_POWER_MODE_LOW = 1,    ///< Low power mode
  HF_I2C_POWER_MODE_SLEEP = 2   ///< Sleep mode
};

/**
 * @brief ESP32 I2C transaction types.
 */
enum class hf_i2c_transaction_type_t : uint8_t {
  HF_I2C_TRANS_WRITE = 0,  ///< Write transaction
  HF_I2C_TRANS_READ = 1,   ///< Read transaction
  HF_I2C_TRANS_WRITE_READ = 2 ///< Write-then-read transaction
};

/**
 * @brief ESP32 I2C event types.
 */
enum class hf_i2c_event_type_t : int {
  HF_I2C_EVENT_MASTER_START = 0,      ///< Master start event
  HF_I2C_EVENT_MASTER_STOP = 1,       ///< Master stop event
  HF_I2C_EVENT_MASTER_WRITE = 2,      ///< Master write event
  HF_I2C_EVENT_MASTER_READ = 3,       ///< Master read event
  HF_I2C_EVENT_SLAVE_START = 4,       ///< Slave start event
  HF_I2C_EVENT_SLAVE_STOP = 5,        ///< Slave stop event
  HF_I2C_EVENT_SLAVE_WRITE = 6,       ///< Slave write event
  HF_I2C_EVENT_SLAVE_READ = 7,        ///< Slave read event
  HF_I2C_EVENT_ERROR = 8              ///< Error event
};

/**
 * @brief ESP32 I2C glitch filter configuration.
 */
enum class hf_i2c_glitch_filter_t : uint8_t {
  HF_I2C_GLITCH_FILTER_0_CYCLES = 0,  ///< No glitch filter
  HF_I2C_GLITCH_FILTER_1_CYCLES = 1,  ///< 1 cycle filter
  HF_I2C_GLITCH_FILTER_2_CYCLES = 2,  ///< 2 cycle filter
  HF_I2C_GLITCH_FILTER_3_CYCLES = 3,  ///< 3 cycle filter
  HF_I2C_GLITCH_FILTER_4_CYCLES = 4,  ///< 4 cycle filter
  HF_I2C_GLITCH_FILTER_5_CYCLES = 5,  ///< 5 cycle filter
  HF_I2C_GLITCH_FILTER_6_CYCLES = 6,  ///< 6 cycle filter
  HF_I2C_GLITCH_FILTER_7_CYCLES = 7   ///< 7 cycle filter
};

/**
 * @brief ESP32 I2C custom command types.
 */
enum class hf_i2c_command_type_t : uint8_t {
  HF_I2C_CMD_START = 0,     ///< Start condition
  HF_I2C_CMD_STOP = 1,      ///< Stop condition
  HF_I2C_CMD_WRITE = 2,     ///< Write data
  HF_I2C_CMD_READ = 3,      ///< Read data
  HF_I2C_CMD_WRITE_READ = 4, ///< Write then read in one transaction
  HF_I2C_CMD_DELAY = 5      ///< Delay
};

//==============================================================================
// ESP32 I2C CALLBACK TYPES
//==============================================================================

/**
 * @brief Callback function signature for asynchronous I2C operations.
 * @param result Operation result code
 * @param bytes_transferred Number of bytes successfully transferred
 * @param user_data User-provided data pointer
 * 
 * @note This callback is executed in interrupt context - keep it minimal and fast!
 * @note Avoid blocking operations, heap allocation, or complex computations.
 * @note Use FreeRTOS queue/semaphore mechanisms to communicate with tasks.
 */
using hf_i2c_async_callback_t = std::function<void(hf_i2c_err_t result, size_t bytes_transferred, void* user_data)>;

/**
 * @brief Callback function signature for I2C event notifications.
 * @param event_type Type of event that occurred
 * @param event_data Event-specific data (if any)
 * @param user_data User-provided data pointer
 * 
 * @note This callback is executed in interrupt context - keep it minimal and fast!
 * @note Avoid blocking operations, heap allocation, or complex computations.
 * @note Use FreeRTOS queue/semaphore mechanisms to communicate with tasks.
 */
using hf_i2c_event_callback_t = std::function<void(hf_i2c_event_type_t event_type, void* event_data, void* user_data)>;

//==============================================================================
// ESP32 I2C CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief Asynchronous operation result structure.
 * @details Contains the result of an asynchronous I2C operation.
 */
struct hf_i2c_async_result_t {
    hf_i2c_err_t error_code;        ///< Operation result code
    size_t bytes_transferred;   ///< Number of bytes successfully transferred
    uint32_t operation_id;      ///< Unique operation identifier
    uint64_t completion_time_us; ///< Completion timestamp in microseconds
    hf_i2c_transaction_type_t transaction_type; ///< Type of transaction completed
};

/**
 * @brief I2C master bus configuration structure.
 * @details Configuration for creating an I2C master bus.
 */
struct hf_i2c_master_bus_config_t {
    i2c_port_t i2c_port;                     ///< I2C port number (0 to MAX_PORTS-1)
    hf_pin_num_t sda_io_num;                 ///< SDA GPIO pin number
    hf_pin_num_t scl_io_num;                 ///< SCL GPIO pin number
    bool enable_internal_pullup;        ///< Enable internal pull-up resistors
    hf_i2c_clock_source_t clk_source;          ///< Clock source selection
    uint32_t clk_flags;                 ///< Additional clock configuration flags
    hf_i2c_glitch_filter_t glitch_ignore_cnt;  ///< Digital glitch filter length
    uint32_t trans_queue_depth;         ///< Transaction queue depth for async ops
    uint32_t intr_priority;             ///< Interrupt priority (0-7, 0=lowest)
    uint32_t flags;                     ///< Additional configuration flags
    bool allow_pd;                      ///< Allow power down in sleep modes

    /**
     * @brief Default constructor with sensible defaults.
     */
    hf_i2c_master_bus_config_t() noexcept
        : i2c_port(I2C_NUM_0), sda_io_num(static_cast<hf_pin_num_t>(HF_INVALID_PIN)), 
          scl_io_num(static_cast<hf_pin_num_t>(HF_INVALID_PIN)),
          enable_internal_pullup(true), clk_source(hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT),
          clk_flags(0), glitch_ignore_cnt(hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES),
          trans_queue_depth(8), intr_priority(5), flags(0), allow_pd(false) {}
};

/**
 * @brief I2C device configuration structure.
 * @details Configuration for adding a device to an I2C master bus.
 */
struct hf_i2c_device_config_t {
    uint16_t device_address;             ///< 7-bit or 10-bit device address
    hf_i2c_address_bits_t dev_addr_length;      ///< Address bit length (7 or 10 bit)
    uint32_t scl_speed_hz;              ///< SCL clock frequency for this device
    uint32_t scl_wait_us;               ///< SCL wait time in microseconds
    uint32_t flags;                     ///< Device-specific configuration flags

    /**
     * @brief Default constructor with sensible defaults.
     */
    hf_i2c_device_config_t() noexcept
        : device_address(0), dev_addr_length(hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT),
          scl_speed_hz(100000), scl_wait_us(0), flags(0) {}
};

/**
 * @brief I2C slave configuration structure.
 * @details Configuration for creating an I2C slave device.
 */
struct hf_i2c_slave_config_t {
    i2c_port_t i2c_port;                   ///< I2C port number
    hf_i2c_clock_source_t clk_source;          ///< Clock source selection
    hf_pin_num_t scl_io_num;                 ///< SCL GPIO pin
    hf_pin_num_t sda_io_num;                 ///< SDA GPIO pin
    uint16_t slave_addr;                ///< Slave address
    hf_i2c_address_bits_t addr_bit_len;        ///< Address bit length
    uint32_t clk_speed_hz;              ///< SCL clock frequency
    uint32_t send_buf_depth;            ///< Send buffer depth
    uint32_t receive_buf_depth;         ///< Receive buffer depth
    uint32_t intr_priority;             ///< Interrupt priority
    bool enable_internal_pullup;        ///< Enable internal pull-ups
    bool broadcast_en;                  ///< Enable general call address (0x00) response
    bool allow_pd;                      ///< Allow power down in sleep modes

    /**
     * @brief Default constructor with sensible defaults.
     */
    hf_i2c_slave_config_t() noexcept
        : i2c_port(I2C_NUM_0), clk_source(hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT),
          scl_io_num(static_cast<hf_pin_num_t>(HF_INVALID_PIN)), 
          sda_io_num(static_cast<hf_pin_num_t>(HF_INVALID_PIN)),
          slave_addr(0), addr_bit_len(hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT),
          clk_speed_hz(100000), send_buf_depth(32), receive_buf_depth(32),
          intr_priority(5), enable_internal_pullup(true), broadcast_en(false), allow_pd(false) {}
};

//==============================================================================
// ESP32 I2C TRANSACTION STRUCTURES
//==============================================================================

/**
 * @brief I2C transaction buffer structure.
 * @details Represents a single buffer in a multi-buffer transaction.
 */
struct hf_i2c_transaction_buffer_t {
    const uint8_t* buffer;      ///< Buffer pointer
    size_t length;              ///< Buffer length in bytes
    bool is_write;              ///< true = write buffer, false = read buffer

    /**
     * @brief Constructor for read buffer.
     */
    hf_i2c_transaction_buffer_t(const uint8_t* buf, size_t len) noexcept
        : buffer(buf), length(len), is_write(true) {}

    /**
     * @brief Constructor with explicit write/read flag.
     */
    hf_i2c_transaction_buffer_t(const uint8_t* buf, size_t len, bool write) noexcept
        : buffer(buf), length(len), is_write(write) {}
};

/**
 * @brief Multi-buffer I2C transaction structure.
 * @details Allows complex I2C protocols with multiple read/write sequences.
 */
struct hf_i2c_multi_buffer_transaction_t {
    uint16_t device_address;                    ///< Target device address
    std::vector<hf_i2c_transaction_buffer_t> buffers;  ///< Buffer sequence
    uint32_t timeout_ms;                        ///< Transaction timeout
    uint32_t flags;                            ///< Transaction flags

    /**
     * @brief Default constructor.
     */
    hf_i2c_multi_buffer_transaction_t() noexcept
        : device_address(0), timeout_ms(1000), flags(0) {}
};

/**
 * @brief Custom I2C command for non-standard protocols.
 * @details Provides flexibility for implementing custom I2C sequences.
 */
struct hf_i2c_custom_command_t {
    hf_i2c_command_type_t command_type;              ///< Command type
    uint16_t device_addr;                           ///< Device address for the command
    std::vector<uint8_t> data;      ///< Command data (if applicable)
    uint32_t delay_us;              ///< Delay in microseconds (for DELAY command)
    uint32_t timeout_ms;            ///< Timeout for the command
    size_t expected_read_size;      ///< Expected read size for READ commands
    uint32_t flags;                 ///< Command-specific flags

    /**
     * @brief Constructor for basic command.
     */
    explicit hf_i2c_custom_command_t(hf_i2c_command_type_t type, uint16_t addr = 0) noexcept
        : command_type(type), device_addr(addr), delay_us(0), timeout_ms(1000), 
          expected_read_size(0), flags(0) {}

    /**
     * @brief Constructor for delay command.
     */
    explicit hf_i2c_custom_command_t(uint32_t delay_microseconds) noexcept
        : command_type(hf_i2c_command_type_t::HF_I2C_CMD_DELAY), device_addr(0), 
          delay_us(delay_microseconds), timeout_ms(0), expected_read_size(0), flags(0) {}
};

//==============================================================================
// END OF ESPI2C TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
