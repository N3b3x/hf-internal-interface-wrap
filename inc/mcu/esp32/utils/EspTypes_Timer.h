/**
 * @file EspTypes_Timer.h
 * @brief ESP32 timer type definitions for hardware abstraction.
 *
 * This header defines only the essential timer-specific types used by
 * the EspPeriodicTimer implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "EspTypes_Base.h"
#include "HardwareTypes.h"
#include "McuSelect.h"

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_timer.h"

#ifdef __cplusplus
}
#endif

//==============================================================================
// ESP32 TIMER TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspPeriodicTimer implementation

// Timer handle type
typedef esp_timer_handle_t hf_timer_handle_t;

// Timestamp type
typedef uint64_t hf_timestamp_us_t;

//==============================================================================
// END OF ESPTIMER TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
