/**
 * @file EspTypes.h
 * @brief Consolidated MCU-specific type definitions for hardware abstraction (hf_* types).
 *
 * This header consolidates all MCU-specific types and constants by including
 * the specialized peripheral headers. This maintains backward compatibility
 * while providing a cleaner, modular organization.
 *
 * @details ESP32C6/ESP-IDF v5.5+ Features Supported:
 * - Dual TWAI/CAN controllers with v2 API support
 * - GPIO hardware glitch filters (pin + 8 flexible filters)
 * - RTC GPIO with sleep retention and wake-up sources
 * - LP (Low Power) GPIO for ultra-low-power operation
 * - Advanced sleep modes with state retention
 * - Enhanced interrupt handling and error recovery
 * - Hardware-accelerated symbol encoding/decoding
 * - Comprehensive power management integration
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note All interface classes (CAN, UART, I2C, SPI, GPIO, ADC, PWM, RMT) must use only these types.
 * @note This implementation is verified against ESP-IDF v5.5+ documentation and supports all latest
 * features.
 */

#pragma once

// Include the base common types first
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)

// Include all peripheral-specific type definitions
#include "EspTypes_ADC.h"
#include "EspTypes_Base.h"
#include "EspTypes_CAN.h"
#include "EspTypes_GPIO.h"
#include "EspTypes_I2C.h"
#include "EspTypes_PIO.h"
#include "EspTypes_PWM.h"
#include "EspTypes_SPI.h"
#include "EspTypes_Timer.h"
#include "EspTypes_UART.h"

//==============================================================================
// BACKWARD COMPATIBILITY AND CONVENIENCE TYPES
//==============================================================================

// Ensure backward compatibility by providing any missing aliases
// that might have been used in the original consolidated file

// Communication types use platform-agnostic base types and hf_* MCU types defined above
// (This section can be expanded if any specific communication types were missed)

//==============================================================================
// CONSOLIDATED VALIDATION MACROS
//==============================================================================

// All validation macros are now defined in their respective peripheral headers
// This section can include any cross-peripheral validation macros if needed

//==============================================================================
// CONSOLIDATED CONSTANTS
//==============================================================================

// All constants are now defined in their respective peripheral headers
// This section can include any cross-peripheral constants if needed
