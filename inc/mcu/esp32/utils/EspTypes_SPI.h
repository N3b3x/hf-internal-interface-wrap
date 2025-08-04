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

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"

#ifdef __cplusplus
}
#endif

// Forward declarations for new SPI bus/device classes
class EspSpiBus;
class EspSpiDevice;

//==============================================================================
// ESP32 SPI TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspSpi implementation
using hf_spi_device_handle_t = spi_device_handle_t;
using hf_spi_device_interface_config_t = spi_device_interface_config_t;
using hf_spi_transaction_t = spi_transaction_t;
using hf_spi_clock_source_t = spi_clock_source_t;
using hf_spi_sampling_point_t = spi_sampling_point_t;

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

// /**
//  * @brief ESP32 SPI host device enumeration.
//  * @details ESP32 SPI controller mapping aligned with ESP-IDF.
//  *          SPI1 is reserved for flash and not exposed to users.
//  */
// enum class hf_spi_host_device_t : uint8_t {
//   HF_SPI2_HOST =
//       static_cast<uint8_t>(spi_host_device_t::SPI2_HOST), ///< SPI2 host (general purpose) -
//       ESP-IDF SPI2_HOST
// #ifdef HF_MCU_ESP32C6
//   // ESP32-C6 only has SPI2_HOST available for general purpose use
//   HF_SPI_HOST_MAX =
//       static_cast<uint8_t>(spi_host_device_t::SPI2_HOST + 1), ///< Maximum number of SPI hosts
//       for ESP32-C6
// #else
//   HF_SPI3_HOST =
//       static_cast<uint8_t>(spi_host_device_t::SPI3_HOST), ///< SPI3 host (general purpose) -
//       ESP-IDF SPI3_HOST
//   HF_SPI_HOST_MAX = static_cast<uint8_t>(spi_host_device_t::SPI_HOST_MAX), ///< Maximum number of
//   SPI hosts
// #endif
// };

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
  ((host) < static_cast<uint8_t>(spi_host_device_t::HF_SPI_HOST_MAX))
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)

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
 * @var hf_spi_bus_config_t::dma_channel
 *   DMA channel (0=auto, 1/2=specific, 0xFF=disabled)
 * @var hf_spi_bus_config_t::timeout_ms
 *   Default timeout for operations (ms)
 * @var hf_spi_bus_config_t::use_iomux
 *   Use IOMUX for better performance
 */
struct hf_spi_bus_config_t {
  hf_host_id_t host;          ///< SPI host/controller (e.g., HF_SPI2_HOST)
  hf_pin_num_t mosi_pin;      ///< MOSI pin
  hf_pin_num_t miso_pin;      ///< MISO pin
  hf_pin_num_t sclk_pin;      ///< SCLK pin
  hf_u32_t clock_speed_hz;    ///< Default clock speed in Hz
  hf_u8_t dma_channel;        ///< DMA channel (0=auto, 1/2=specific, 0xFF=disabled)
  hf_timeout_ms_t timeout_ms; ///< Default timeout for operations (ms)
  bool use_iomux;             ///< Use IOMUX for better performance

  hf_spi_bus_config_t() noexcept
      : host(HF_INVALID_HOST), mosi_pin(HF_INVALID_PIN), miso_pin(HF_INVALID_PIN),
        sclk_pin(HF_INVALID_PIN), clock_speed_hz(1000000), dma_channel(0), timeout_ms(1000),
        use_iomux(true) {}
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
 * @var hf_spi_device_config_t::clock_source
 *   Clock source selection for ESP32C6 (0=default, see spi_clock_source_t)
 */
struct hf_spi_device_config_t {
  hf_u32_t clock_speed_hz;                ///< Device clock speed (Hz)
  hf_spi_mode_t mode;                     ///< SPI mode (0-3)
  hf_pin_num_t cs_pin;                    ///< CS pin (or -1 for software CS)
  hf_u8_t queue_size;                     ///< Transaction queue size
  hf_u8_t command_bits;                   ///< Command phase bits (0-16)
  hf_u8_t address_bits;                   ///< Address phase bits (0-64)
  hf_u8_t dummy_bits;                     ///< Dummy bits between address and data
  hf_u16_t duty_cycle_pos;                ///< Duty cycle of positive clock (1/256th, 128=50%)
  hf_u16_t cs_ena_pretrans;               ///< CS active before transmission (bit-cycles)
  hf_u8_t cs_ena_posttrans;               ///< CS active after transmission (bit-cycles)
  hf_u32_t flags;                         ///< Bitwise OR of SPI_DEVICE_* flags
  hf_u32_t input_delay_ns;                ///< Input delay (ns)
  void (*pre_cb)(void*);                  ///< Pre-transfer callback (optional)
  void (*post_cb)(void*);                 ///< Post-transfer callback (optional)
  void* user_ctx;                         ///< User context for callbacks
  hf_spi_clock_source_t clock_source;     ///< Clock source selection (0=default, ESP32C6 specific)
  hf_spi_sampling_point_t sampling_point; ///< Sampling point for data (ESP32C6 specific)

  hf_spi_device_config_t() noexcept
      : clock_speed_hz(1000000), mode(hf_spi_mode_t::HF_SPI_MODE_0), cs_pin(HF_INVALID_PIN),
        queue_size(7), command_bits(0), address_bits(0), dummy_bits(0), duty_cycle_pos(128),
        cs_ena_pretrans(0), cs_ena_posttrans(0), flags(0), input_delay_ns(0), pre_cb(nullptr),
        post_cb(nullptr), user_ctx(nullptr),
        clock_source(static_cast<hf_spi_clock_source_t>(SPI_CLK_SRC_DEFAULT)),
#ifdef HF_MCU_ESP32C6
        sampling_point(static_cast<hf_spi_sampling_point_t>(SPI_SAMPLING_POINT_PHASE_1)) {
  }
#else
        sampling_point(static_cast<hf_spi_sampling_point_t>(SPI_SAMPLING_POINT_DEFAULT)) {
  }
#endif
};

//==============================================================================
// END OF ESPSPI TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
