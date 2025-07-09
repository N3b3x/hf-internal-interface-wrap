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
#include "esp_timer.h"

//==============================================================================
// ESP32 TIMER TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspPeriodicTimer implementation

//==============================================================================
// END OF ESPTIMER TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
