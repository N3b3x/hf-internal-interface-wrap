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

#include "BaseSpi.h"       // For hf_spi_err_t
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"

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

#define HF_SPI_IS_VALID_HOST(host)                                                                 \
  ((host) < static_cast<uint8_t>(hf_spi_host_device_t::HF_SPI_HOST_MAX))
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed)                                                         \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)

//==============================================================================
// END OF ESPSPI TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
