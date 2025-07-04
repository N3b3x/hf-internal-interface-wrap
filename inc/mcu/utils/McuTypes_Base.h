/**
 * @file McuTypes_Base.h
 * @brief Base MCU-specific type definitions for hardware abstraction (hf_* types).
 *
 * This header defines the common base types and includes that are shared across
 * all MCU peripheral type definitions. It provides the foundation for platform-specific
 * configurations while maintaining interface compatibility.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This file should be included by all McuTypes_*.h files.
 * @note All interface classes must use only these types.
 */

#pragma once

#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF headers)
#include <atomic>
#include <cstdint>

//==============================================================================
// PLATFORM-SPECIFIC INCLUDES AND BASIC TYPE DEFINITIONS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32-specific type definitions - includes already handled by McuSelect.h
// Additional ESP-IDF includes for specific types only if needed
#ifdef __cplusplus
extern "C" {
#endif

// ESP32-specific includes and type definitions
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "driver/twai.h" // ESP-IDF v5.5+ TWAI driver
#include "driver/uart.h"
#include "driver/ledc.h"
#include "esp_timer.h" // For esp_timer_handle_t
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" // For QueueHandle_t
#include "freertos/semphr.h"

#ifdef __cplusplus
}
#endif

#else
// Non-ESP32 platforms - use generic types
// Generic RTOS handle types for non-ESP32 platforms
using SemaphoreHandle_t = void *;
using QueueHandle_t = void *;
using i2c_config_t = struct {
  int dummy;
};
using spi_bus_config_t = struct {
  int dummy;
};
using spi_device_interface_config_t = struct {
  int dummy;
};
using uart_config_t = struct {
  int dummy;
};
#endif

//==============================================================================
// MCU-SPECIFIC CONSTANTS
//==============================================================================

static constexpr hf_timeout_ms_t HF_TIMEOUT_NEVER = 0xFFFFFFFF;
static constexpr hf_timeout_ms_t HF_TIMEOUT_IMMEDIATE = 0;
static constexpr hf_timeout_ms_t HF_TIMEOUT_DEFAULT = 1000;

//==============================================================================
// MCU TIMING CONVERSION MACROS
//==============================================================================

/**
 * @brief Convert milliseconds to RTOS ticks for operations.
 * @param ms Milliseconds to convert
 * @return RTOS ticks (implementation-specific)
 */
#ifdef HF_MCU_FAMILY_ESP32
#define HF_TICKS_FROM_MS(ms) (pdMS_TO_TICKS(ms))
#define HF_MS_FROM_TICKS(ticks) ((ticks) * portTICK_PERIOD_MS)
#define HF_US_TO_TICKS(us) ((us) / (portTICK_PERIOD_MS * 1000))
#define HF_TICKS_TO_US(ticks) ((ticks) * portTICK_PERIOD_MS * 1000)
#else
#define HF_TICKS_FROM_MS(ms) (ms)
#define HF_MS_FROM_TICKS(ticks) (ticks)
#define HF_US_TO_TICKS(us) (us)
#define HF_TICKS_TO_US(ticks) (ticks)
#endif

//==============================================================================
// POWER MANAGEMENT AND TIMING TYPES
//==============================================================================

/**
 * @brief ESP32C6 power domain configuration for operations.
 */
enum class hf_power_domain_t : uint8_t {
  HF_POWER_DOMAIN_CPU = 0,    ///< CPU power domain
  HF_POWER_DOMAIN_RTC_PERIPH, ///< RTC peripherals power domain
  HF_POWER_DOMAIN_XTAL,       ///< Crystal oscillator domain
  HF_POWER_DOMAIN_MODEM,      ///< RF/WiFi/BT modem domain
  HF_POWER_DOMAIN_VDDSDIO,    ///< SDIO power domain
  HF_POWER_DOMAIN_TOP,        ///< SoC top domain
};

/**
 * @brief ESP32C6 sleep mode types.
 */
enum class hf_sleep_mode_t : uint8_t {
  HF_SLEEP_MODE_NONE = 0,    ///< No sleep mode
  HF_SLEEP_MODE_LIGHT,       ///< Light sleep mode
  HF_SLEEP_MODE_DEEP,        ///< Deep sleep mode
  HF_SLEEP_MODE_HIBERNATION, ///< Hibernation mode (lowest power)
};

/**
 * @brief High-resolution timing types for operations.
 */
using hf_timestamp_us_t = uint64_t; ///< Microsecond timestamp
using hf_timestamp_ns_t = uint64_t; ///< Nanosecond timestamp
using hf_duration_us_t = uint32_t;  ///< Duration in microseconds
using hf_duration_ns_t = uint32_t;  ///< Duration in nanoseconds
