/**
 * @file EspTypes_Base.h
 * @brief ESP32 base type definitions for hardware abstraction.
 *
 * This header defines the common base types and includes that are shared across
 * all ESP32 peripheral type definitions. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This file should be included by all EspTypes_*.h files.
 * @note All interface classes must use only these types.
 */

#pragma once

#include "../../utils/McuSelect.h" // Central MCU platform selection (includes all ESP-IDF headers)
#include "HardwareTypes.h"
#include <atomic>
#include <cstdint>

//==============================================================================
// BASIC TYPE DEFINITIONS
//==============================================================================

/// @brief Timeout value in milliseconds
using hf_timeout_ms_t = uint32_t;

//==============================================================================
// ESP32-SPECIFIC CONSTANTS
//==============================================================================

static constexpr hf_timeout_ms_t HF_TIMEOUT_NEVER = 0xFFFFFFFF;
static constexpr hf_timeout_ms_t HF_TIMEOUT_IMMEDIATE = 0;
static constexpr hf_timeout_ms_t HF_TIMEOUT_DEFAULT = 1000;

// Missing ADC constants
static constexpr uint32_t HF_ADC_DEFAULT_SAMPLING_FREQ = 1000U;
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_DEFAULT = 1024U;

//==============================================================================
// ESP32 TIMING CONVERSION MACROS
//==============================================================================

/**
 * @brief Convert milliseconds to RTOS ticks for operations.
 * @param ms Milliseconds to convert
 * @return RTOS ticks (implementation-specific)
 */
#define HF_TICKS_FROM_MS(ms) (pdMS_TO_TICKS(ms))
#define HF_MS_FROM_TICKS(ticks) ((ticks) * portTICK_PERIOD_MS)
#define HF_US_TO_TICKS(us) ((us) / (portTICK_PERIOD_MS * 1000))
#define HF_TICKS_TO_US(ticks) ((ticks) * portTICK_PERIOD_MS * 1000)

//==============================================================================
// ESP32 POWER MANAGEMENT AND TIMING TYPES
//==============================================================================

/**
 * @brief ESP32 power domain configuration for operations.
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
 * @brief ESP32 sleep mode types.
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