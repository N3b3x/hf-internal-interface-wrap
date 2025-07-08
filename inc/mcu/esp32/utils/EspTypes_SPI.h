/**
 * @file McuTypes_SPI.h
 * @brief MCU-specific SPI type definitions for hardware abstraction.
 *
 * This header defines all SPI-specific types and constants that are used
 * throughout the internal interface wrap layer for SPI operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"    // Central MCU platform selection (includes all ESP-IDF)
#include "McuTypes_Base.h"
#include "BaseSpi.h" // For hf_spi_err_t

//==============================================================================
// PLATFORM-SPECIFIC SPI TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 SPI specific mappings
using hf_spi_host_native_t = spi_host_device_t;
using hf_spi_bus_config_native_t = spi_bus_config_t;
using hf_spi_device_config_native_t = spi_device_interface_config_t;
using hf_spi_transaction_native_t = spi_transaction_t;
using hf_spi_device_handle_native_t = spi_device_handle_t;
#else
// Non-ESP32 platforms - use generic types
using hf_spi_host_native_t = uint8_t;
using hf_spi_bus_config_native_t = struct {
  int dummy;
};
using hf_spi_device_config_native_t = struct {
  int dummy;
};
using hf_spi_transaction_native_t = struct {
  int dummy;
};
using hf_spi_device_handle_native_t = void *;
#endif


/**
 * @brief SPI bus configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_bus_config_t {
  int mosi_io_num;        ///< MOSI GPIO pin
  int miso_io_num;        ///< MISO GPIO pin
  int sclk_io_num;        ///< SCLK GPIO pin
  int quadwp_io_num;      ///< WP pin for quad/octal mode
  int quadhd_io_num;      ///< HD pin for quad/octal mode
  int data4_io_num;       ///< DATA4 pin for octal mode
  int data5_io_num;       ///< DATA5 pin for octal mode
  int data6_io_num;       ///< DATA6 pin for octal mode
  int data7_io_num;       ///< DATA7 pin for octal mode
  int max_transfer_sz;    ///< Maximum transfer size
  uint32_t flags;         ///< Bus configuration flags
  uint32_t intr_flags;    ///< Interrupt allocation flags
  
  hf_spi_bus_config_t() noexcept
      : mosi_io_num(-1), miso_io_num(-1), sclk_io_num(-1),
        quadwp_io_num(-1), quadhd_io_num(-1), data4_io_num(-1),
        data5_io_num(-1), data6_io_num(-1), data7_io_num(-1),
        max_transfer_sz(4092), flags(0), intr_flags(0) {}
};

/**
 * @brief SPI device configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_device_interface_config_t {
  uint8_t command_bits;        ///< Command phase bit length (0-16)
  uint8_t address_bits;        ///< Address phase bit length (0-64)
  uint8_t dummy_bits;          ///< Dummy phase bit length
  uint8_t mode;                ///< SPI mode (0-3)
  uint8_t duty_cycle_pos;      ///< Duty cycle of positive clock
  uint8_t cs_ena_pretrans;     ///< CS setup time
  uint8_t cs_ena_posttrans;    ///< CS hold time
  int clock_speed_hz;          ///< Clock speed in Hz
  int input_delay_ns;          ///< Input delay in nanoseconds
  int spics_io_num;            ///< CS GPIO pin (-1 = not used)
  uint32_t flags;              ///< Device configuration flags
  int queue_size;              ///< Transaction queue size
  void (*pre_cb)(void *trans); ///< Pre-transaction callback
  void (*post_cb)(void *trans);///< Post-transaction callback
  
  hf_spi_device_interface_config_t() noexcept
      : command_bits(0), address_bits(0), dummy_bits(0), mode(0),
        duty_cycle_pos(128), cs_ena_pretrans(0), cs_ena_posttrans(0),
        clock_speed_hz(1000000), input_delay_ns(0), spics_io_num(-1),
        flags(0), queue_size(7), pre_cb(nullptr), post_cb(nullptr) {}
};

/**
 * @brief SPI transaction structure for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_transaction_t {
  uint32_t flags;              ///< Transaction flags
  uint16_t cmd;                ///< Command data
  uint64_t addr;               ///< Address data
  size_t length;               ///< Data length in bits
  size_t rxlength;             ///< RX data length in bits
  void *user;                  ///< User data pointer
  union {
    const void *tx_buffer;     ///< TX data buffer
    uint8_t tx_data[4];        ///< TX data for ≤32 bits
  };
  union {
    void *rx_buffer;           ///< RX data buffer
    uint8_t rx_data[4];        ///< RX data for ≤32 bits
  };
  
  hf_spi_transaction_t() noexcept
      : flags(0), cmd(0), addr(0), length(0), rxlength(0),
        user(nullptr), tx_buffer(nullptr), rx_buffer(nullptr) {}
};

/**
 * @brief SPI diagnostics information structure.
 */
struct hf_spi_diagnostics_t {
  bool is_initialized;        ///< Initialization state
  bool is_bus_suspended;      ///< Bus suspension state
  bool dma_enabled;           ///< DMA enabled state
  uint32_t current_clock_speed; ///< Current clock speed in Hz
  uint8_t current_mode;       ///< Current SPI mode
  uint16_t max_transfer_size; ///< Maximum transfer size
  uint8_t device_count;       ///< Number of registered devices
  uint32_t last_error;        ///< Last error code
  uint64_t total_transactions; ///< Total transactions performed
  uint64_t failed_transactions; ///< Failed transactions count
  
  hf_spi_diagnostics_t() noexcept
      : is_initialized(false), is_bus_suspended(false), dma_enabled(false),
        current_clock_speed(0), current_mode(0), max_transfer_size(0),
        device_count(0), last_error(0), total_transactions(0), failed_transactions(0) {}
};

// Type alias for SPI diagnostics (following naming convention)
using hf_spi_diagnostics_alias_t = hf_spi_diagnostics_t;

/**
 * @brief SPI device handle type.
 */
using hf_spi_device_handle_t = hf_spi_device_handle_native_t;
using hf_spi_host_device_id_t = hf_spi_host_native_t;


//==============================================================================
// MCU-SPECIFIC SPI TYPES
//==============================================================================

/**
 * @brief MCU-specific SPI mode configuration.
 */
enum class hf_spi_mode_t : uint8_t {
  HF_SPI_MODE_0 = 0, ///< CPOL=0, CPHA=0
  HF_SPI_MODE_1 = 1, ///< CPOL=0, CPHA=1
  HF_SPI_MODE_2 = 2, ///< CPOL=1, CPHA=0
  HF_SPI_MODE_3 = 3, ///< CPOL=1, CPHA=1
};

/**
 * @brief ESP32C6 SPI host device enumeration.
 * @details ESP32C6 SPI controller mapping aligned with ESP-IDF v5.4.2+.
 *          SPI1 is reserved for flash and not exposed to users.
 */
enum class hf_spi_host_device_t : uint8_t {
  HF_SPI2_HOST = 1,      ///< SPI2 host (general purpose) - ESP-IDF SPI2_HOST
  HF_SPI3_HOST = 2,      ///< SPI3 host (general purpose) - ESP-IDF SPI3_HOST  
  HF_SPI_HOST_MAX = 3,   ///< Maximum number of SPI hosts
};

/**
 * @brief SPI transfer modes for ESP32C6.
 * @details Advanced transfer modes including octal SPI support.
 */
enum class hf_spi_transfer_mode_t : uint8_t {
  HF_SPI_TRANSFER_MODE_SINGLE = 0, ///< Standard SPI (1-bit MOSI/MISO)
  HF_SPI_TRANSFER_MODE_DUAL = 1,   ///< Dual SPI (2-bit data lines)
  HF_SPI_TRANSFER_MODE_QUAD = 2,   ///< Quad SPI (4-bit data lines)
  HF_SPI_TRANSFER_MODE_OCTAL = 3   ///< Octal SPI (8-bit data lines) - ESP32C6 specific
};

/**
 * @brief SPI clock source selection for ESP32C6.
 * @details Clock source options for power optimization.
 */
enum class hf_spi_clock_source_t : uint8_t {
  HF_SPI_CLK_SRC_DEFAULT = 0, ///< Default SPI clock source
  HF_SPI_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_SPI_CLK_SRC_XTAL = 2     ///< Crystal oscillator (40MHz, lower power)
};

/**
 * @brief SPI event types for callback notifications.
 * @details Event types reported via SPI event callbacks.
 */
enum class hf_spi_event_type_t : int {
  HF_SPI_EVENT_TRANSACTION_COMPLETE = 0, ///< Transaction completed
  HF_SPI_EVENT_TRANSACTION_ERROR = 1,    ///< Transaction error occurred
  HF_SPI_EVENT_BUS_SUSPENDED = 2,        ///< Bus suspended for power saving
  HF_SPI_EVENT_BUS_RESUMED = 3,          ///< Bus resumed from suspension
  HF_SPI_EVENT_DMA_ERROR = 4             ///< DMA error occurred
};

/**
 * @brief SPI device handle type for MCU SPI driver.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_spi_handle_t = spi_device_handle_t;
#else
using hf_spi_handle_t = void *;
#endif

/**
 * @brief SPI transaction result and diagnostics.
 * @details Performance monitoring and error tracking for SPI operations.
 */
struct hf_spi_transaction_diagnostics_t {
  uint32_t total_transactions;     ///< Total number of transactions
  uint32_t successful_transactions; ///< Number of successful transactions
  uint32_t failed_transactions;    ///< Number of failed transactions
  uint32_t timeout_transactions;   ///< Number of timed-out transactions
  uint32_t total_bytes_sent;       ///< Total bytes transmitted
  uint32_t total_bytes_received;   ///< Total bytes received
  uint32_t max_transaction_time_us; ///< Maximum transaction time (microseconds)
  uint32_t min_transaction_time_us; ///< Minimum transaction time (microseconds)
  uint64_t last_activity_timestamp; ///< Last activity timestamp
  uint64_t initialization_timestamp; ///< Initialization timestamp

  hf_spi_transaction_diagnostics_t() noexcept
      : total_transactions(0), successful_transactions(0), failed_transactions(0),
        timeout_transactions(0), total_bytes_sent(0), total_bytes_received(0),
        max_transaction_time_us(0), min_transaction_time_us(0xFFFFFFFF),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

//==============================================================================
// ESP32C6 SPI CONSTANTS AND VALIDATION MACROS
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6
static constexpr uint32_t HF_SPI_MIN_CLOCK_SPEED = 1000;      ///< Minimum SPI clock speed (Hz)
static constexpr uint32_t HF_SPI_MAX_CLOCK_SPEED = 80000000;  ///< Maximum SPI clock speed (Hz)
static constexpr uint32_t HF_SPI_MAX_TRANSFER_SIZE = 4092;    ///< Maximum transfer size (bytes)
static constexpr uint8_t HF_SPI_MAX_HOSTS = 3;               ///< Maximum SPI hosts

#define HF_SPI_IS_VALID_HOST(host) ((host) < static_cast<uint8_t>(hf_spi_host_device_t::HF_SPI_HOST_MAX))
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)
#else
// Generic constants for non-ESP32C6 platforms
static constexpr uint32_t HF_SPI_MIN_CLOCK_SPEED = 1000;
static constexpr uint32_t HF_SPI_MAX_CLOCK_SPEED = 50000000;
static constexpr uint32_t HF_SPI_MAX_TRANSFER_SIZE = 4096;
static constexpr uint8_t HF_SPI_MAX_HOSTS = 2;

#define HF_SPI_IS_VALID_HOST(host) ((host) < HF_SPI_MAX_HOSTS)
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)
#endif

// SPI diagnostics alias for convenience (following naming convention)
using hf_spi_diagnostics_alias_t = hf_spi_diagnostics_t;
