/**
 * @file EspTypes_CAN.h
 * @brief ESP32 CAN type definitions for hardware abstraction.
 *
 * This header defines only the essential CAN-specific types and constants used by
 * the EspCan implementation. It follows the same clean, minimal pattern as EspTypes_ADC.h,
 * providing only necessary types without redundant or duplicate definitions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h"
#include "McuSelect.h"      // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"
#include "BaseCan.h"        // For hf_can_err_t and hf_can_message_t

//==============================================================================
// ESSENTIAL CAN TYPES (ESP32)
//==============================================================================

//==============================================================================
// ESSENTIAL CAN ENUMS (ESP32 SPECIFIC)
//==============================================================================

/**
 * @brief CAN controller ID for ESP32 family.
 */
enum class hf_can_controller_id_t : uint8_t {
  HF_CAN_CONTROLLER_0 = 0,   ///< Primary CAN controller
  HF_CAN_CONTROLLER_1 = 1,   ///< Secondary CAN controller (ESP32 only)
};

/**
 * @brief CAN operating mode mapping from ESP-IDF TWAI modes.
 */
enum class hf_can_mode_t : uint8_t {
  HF_CAN_MODE_NORMAL = 0,      ///< Normal operating mode
  HF_CAN_MODE_NO_ACK = 1,      ///< No acknowledgment mode (self-test)
  HF_CAN_MODE_LISTEN_ONLY = 2, ///< Listen-only mode (bus monitor)
};

/**
 * @brief CAN operation types for statistics tracking.
 */
enum class hf_can_operation_type_t : uint8_t {
    HF_CAN_OP_SEND = 0,      ///< Send operation
    HF_CAN_OP_RECEIVE = 1,   ///< Receive operation
    HF_CAN_OP_FILTER = 2,    ///< Filter operation
    HF_CAN_OP_ALERT = 3,     ///< Alert operation
    HF_CAN_OP_INIT = 4,      ///< Initialization operation
    HF_CAN_OP_DEINIT = 5,    ///< Deinitialization operation
    HF_CAN_OP_RESET = 6,     ///< Reset operation
    HF_CAN_OP_RECOVER = 7,   ///< Bus recovery operation
};  

//==============================================================================
// ESSENTIAL CAN CONFIGURATION STRUCTURES (MINIMAL)
//==============================================================================

/**
 * @brief CAN filter configuration structure.
 */
struct hf_can_filter_config_t {
    uint32_t acceptance_code;        ///< Acceptance code
    uint32_t acceptance_mask;        ///< Acceptance mask  
    bool single_filter;              ///< Use single filter mode
    
    hf_can_filter_config_t() noexcept
        : acceptance_code(0), acceptance_mask(0xFFFFFFFF), single_filter(true) {}
};

/**
 * @brief CAN alert callback function type.
 */
using hf_can_alert_callback_t = void (*)(uint32_t alerts, void* user_data);

//==============================================================================
// END OF ESPCAN TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
