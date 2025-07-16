/**
 * @file EspTypes_SPI.h
 * @brief ESP32 SPI type definitions for hardware abstraction.
 *
 * This header defines only the essential SPI-specific types used by
 * the EspSpi implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseSpi.h" // For hf_spi_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)

// Forward declarations for new SPI bus/device classes
class EspSpiBus;
class EspSpiDevice;

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"

#ifdef __cplusplus
}
#endif

//==============================================================================
// ESP32 SPI TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspSpi implementation
using hf_spi_device_handle_t = spi_device_handle_t;
using hf_spi_device_interface_config_t = spi_device_interface_config_t;
using hf_spi_transaction_t = spi_transaction_t;

//==============================================================================
// ESP32 SPI BUS CONFIG STRUCT
//==============================================================================
/**
 * @struct hf_spi_bus_config_t
 * @brief Platform-agnostic SPI bus configuration structure for ESP32.
 *
 * This structure provides all configuration options for initializing an SPI bus
 * on ESP32 platforms, including DMA, IOMUX, and advanced timing options. All
 * fields use project types for portability.
 *
 * @var hf_spi_bus_config_t::host
 *   SPI host/controller (e.g., HF_SPI2_HOST)
 * @var hf_spi_bus_config_t::mosi_pin
 *   MOSI pin number
 * @var hf_spi_bus_config_t::miso_pin
 *   MISO pin number
 * @var hf_spi_bus_config_t::sclk_pin
 *   SCLK pin number
 * @var hf_spi_bus_config_t::clock_speed_hz
 *   Default clock speed in Hz
 * @var hf_spi_bus_config_t::dma_channel
 *   DMA channel (0=auto, 1/2=specific, 0xFF=disabled)
 * @var hf_spi_bus_config_t::bits_per_word
 *   Bits per transfer (typically 8 or 16)
 * @var hf_spi_bus_config_t::timeout_ms
 *   Default timeout for operations (ms)
 * @var hf_spi_bus_config_t::use_iomux
 *   Use IOMUX for better performance
 */
struct hf_spi_bus_config_t {
  hf_host_id_t host;                ///< SPI host/controller (e.g., HF_SPI2_HOST)
  hf_pin_num_t mosi_pin;            ///< MOSI pin
  hf_pin_num_t miso_pin;            ///< MISO pin
  hf_pin_num_t sclk_pin;            ///< SCLK pin
  hf_u32_t clock_speed_hz;          ///< Default clock speed (Hz)
  hf_u8_t dma_channel;              ///< DMA channel (0=auto, 1/2=specific, 0xFF=disabled)
  hf_u8_t bits_per_word;            ///< Bits per transfer (typically 8 or 16)
  hf_timeout_ms_t timeout_ms;       ///< Default timeout for operations (ms)
  bool use_iomux;                   ///< Use IOMUX for better performance

  hf_spi_bus_config_t() noexcept
      : host(HF_INVALID_HOST), mosi_pin(HF_INVALID_PIN), miso_pin(HF_INVALID_PIN),
        sclk_pin(HF_INVALID_PIN), clock_speed_hz(1000000), dma_channel(0),
        bits_per_word(8), timeout_ms(1000), use_iomux(true) {}
};

//==============================================================================
// ESP32 SPI DEVICE CONFIG STRUCT
//==============================================================================
/**
 * @struct hf_spi_device_config_t
 * @brief Platform-agnostic SPI device configuration structure for ESP32.
 *
 * This structure provides all configuration options for registering a device on
 * an SPI bus, including clock, mode, CS, queue, DMA, callbacks, and advanced
 * ESP-IDF v5.5+ features. All fields use project types for portability.
 *
 * @var hf_spi_device_config_t::clock_speed_hz
 *   Device clock speed in Hz
 * @var hf_spi_device_config_t::mode
 *   SPI mode (0-3)
 * @var hf_spi_device_config_t::cs_pin
 *   CS pin number (or -1 for software CS)
 * @var hf_spi_device_config_t::queue_size
 *   Transaction queue size
 * @var hf_spi_device_config_t::command_bits
 *   Command phase bits (0-16)
 * @var hf_spi_device_config_t::address_bits
 *   Address phase bits (0-64)
 * @var hf_spi_device_config_t::dummy_bits
 *   Dummy bits between address and data
 * @var hf_spi_device_config_t::duty_cycle_pos
 *   Duty cycle of positive clock (1/256th, 128=50%)
 * @var hf_spi_device_config_t::cs_ena_pretrans
 *   CS active before transmission (bit-cycles)
 * @var hf_spi_device_config_t::cs_ena_posttrans
 *   CS active after transmission (bit-cycles)
 * @var hf_spi_device_config_t::flags
 *   Bitwise OR of SPI_DEVICE_* flags
 * @var hf_spi_device_config_t::input_delay_ns
 *   Input delay in nanoseconds
 * @var hf_spi_device_config_t::pre_cb
 *   Pre-transfer callback (optional)
 * @var hf_spi_device_config_t::post_cb
 *   Post-transfer callback (optional)
 * @var hf_spi_device_config_t::user_ctx
 *   User context for callbacks
 */
struct hf_spi_device_config_t {
  hf_u32_t clock_speed_hz;      ///< Device clock speed (Hz)
  hf_u8_t mode;                 ///< SPI mode (0-3)
  hf_pin_num_t cs_pin;          ///< CS pin (or -1 for software CS)
  hf_u8_t queue_size;           ///< Transaction queue size
  hf_u8_t command_bits;         ///< Command phase bits (0-16)
  hf_u8_t address_bits;         ///< Address phase bits (0-64)
  hf_u8_t dummy_bits;           ///< Dummy bits between address and data
  hf_u16_t duty_cycle_pos;      ///< Duty cycle of positive clock (1/256th, 128=50%)
  hf_u16_t cs_ena_pretrans;     ///< CS active before transmission (bit-cycles)
  hf_u8_t cs_ena_posttrans;     ///< CS active after transmission (bit-cycles)
  hf_u32_t flags;               ///< Bitwise OR of SPI_DEVICE_* flags
  hf_u32_t input_delay_ns;      ///< Input delay (ns)
  void (*pre_cb)(void*);        ///< Pre-transfer callback (optional)
  void (*post_cb)(void*);       ///< Post-transfer callback (optional)
  void* user_ctx;               ///< User context for callbacks

  hf_spi_device_config_t() noexcept
      : clock_speed_hz(1000000), mode(0), cs_pin(HF_INVALID_PIN), queue_size(7),
        command_bits(0), address_bits(0), dummy_bits(0), duty_cycle_pos(128),
        cs_ena_pretrans(0), cs_ena_posttrans(0), flags(0), input_delay_ns(0),
        pre_cb(nullptr), post_cb(nullptr), user_ctx(nullptr) {}
};

//==============================================================================
// ESP32 SPI ENUMS
//==============================================================================

/**
 * @brief ESP32 SPI mode configuration.
 */
enum class hf_spi_mode_t : uint8_t {
  HF_SPI_MODE_0 = 0, ///< CPOL=0, CPHA=0
  HF_SPI_MODE_1 = 1, ///< CPOL=0, CPHA=1
  HF_SPI_MODE_2 = 2, ///< CPOL=1, CPHA=0
  HF_SPI_MODE_3 = 3, ///< CPOL=1, CPHA=1
};

/**
 * @brief ESP32 SPI host device enumeration.
 * @details ESP32 SPI controller mapping aligned with ESP-IDF.
 *          SPI1 is reserved for flash and not exposed to users.
 */
enum class hf_spi_host_device_t : uint8_t {
  HF_SPI2_HOST = 1,    ///< SPI2 host (general purpose) - ESP-IDF SPI2_HOST
  HF_SPI3_HOST = 2,    ///< SPI3 host (general purpose) - ESP-IDF SPI3_HOST
  HF_SPI_HOST_MAX = 3, ///< Maximum number of SPI hosts
};

/**
 * @brief SPI transfer modes for ESP32.
 * @details Advanced transfer modes including octal SPI support.
 */
enum class hf_spi_transfer_mode_t : uint8_t {
  HF_SPI_TRANSFER_MODE_SINGLE = 0, ///< Standard SPI (1-bit MOSI/MISO)
  HF_SPI_TRANSFER_MODE_DUAL = 1,   ///< Dual SPI (2-bit data lines)
  HF_SPI_TRANSFER_MODE_QUAD = 2,   ///< Quad SPI (4-bit data lines)
  HF_SPI_TRANSFER_MODE_OCTAL = 3   ///< Octal SPI (8-bit data lines) - ESP32 specific
};

/**
 * @brief SPI clock source selection for ESP32.
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

//==============================================================================
// ESP32 SPI CONSTANTS AND VALIDATION MACROS
//==============================================================================

static constexpr uint32_t HF_SPI_MIN_CLOCK_SPEED = 1000;     ///< Minimum SPI clock speed (Hz)
static constexpr uint32_t HF_SPI_MAX_CLOCK_SPEED = 80000000; ///< Maximum SPI clock speed (Hz)
static constexpr uint32_t HF_SPI_MAX_TRANSFER_SIZE = 4092;   ///< Maximum transfer size (bytes)
static constexpr uint8_t HF_SPI_MAX_HOSTS = 3;               ///< Maximum SPI hosts

#define HF_SPI_IS_VALID_HOST(host) \
  ((host) < static_cast<uint8_t>(hf_spi_host_device_t::HF_SPI_HOST_MAX))
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)

//==============================================================================
// END OF ESPSPI TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
