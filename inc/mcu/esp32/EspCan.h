/**
 * @file EspCan.h
 * @brief ESP32 CAN (TWAI) implementation for the HardFOC system.
 *
 * This file contains the ESP32 CAN (TWAI) implementation that extends the BaseCan
 * abstract class. It provides a clean, minimal, and robust CAN interface using
 * the modern ESP-IDF v5.4+ handle-based TWAI API.
 *
 * Key Features:
 * - Clean architectural pattern following EspAdc design
 * - Lazy initialization for efficient resource management
 * - Thread-safe operations with proper resource management
 * - Modern ESP-IDF v5.4+ handle-based TWAI API
 * - Support for all ESP32 family members
 * - Comprehensive error handling and diagnostics
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation follows the same clean architectural pattern as EspAdc
 * @note Each EspCan instance represents a single TWAI controller
 * @note Higher-level applications should instantiate multiple EspCan objects for multi-controller boards
 */

#pragma once

#include "McuSelect.h"

// Only compile for ESP32 family
#ifdef HF_MCU_FAMILY_ESP32

#include "BaseCan.h"
#include "RtosMutex.h"
#include "utils/EspTypes_CAN.h"

#include <memory>
#include <atomic>

// ESP-IDF TWAI functionality is included via McuSelect.h

//==============================================================================
// ESP32 TWAI CONFIGURATION (Minimal like EspAdc)
//==============================================================================

/**
 * @brief ESP32 TWAI controller configuration structure.
 * @details Minimal configuration following EspAdc pattern - essential parameters only.
 */
struct hf_esp_can_config_t {
    hf_can_controller_id_t controller_id;       ///< Controller ID (0 or 1 for ESP32C6) 
    hf_can_mode_t mode;                         ///< Operating mode (normal, listen-only, no-ack)
    hf_pin_num_t tx_pin;                        ///< TX GPIO pin number
    hf_pin_num_t rx_pin;                        ///< RX GPIO pin number
    uint32_t baud_rate;                         ///< Target baud rate in bps
    uint32_t tx_queue_len;                      ///< Transmit queue length
    uint32_t rx_queue_len;                      ///< Receive queue length
    bool enable_alerts;                         ///< Enable alert monitoring
    
    hf_esp_can_config_t() noexcept
        : controller_id(hf_can_controller_id_t::HF_CAN_CONTROLLER_0)
        , mode(hf_can_mode_t::HF_CAN_MODE_NORMAL)
        , tx_pin(4)
        , rx_pin(5)
        , baud_rate(500000)
        , tx_queue_len(10)
        , rx_queue_len(20)
        , enable_alerts(false) {}
};

/**
 * @class EspCan
 * @brief ESP32 CAN (TWAI) implementation following EspAdc architectural pattern.
 *
 * This class provides clean, minimal CAN communication using the ESP32's TWAI 
 * (Two-Wire Automotive Interface) controllers with modern ESP-IDF v5.4+ APIs.
 * The implementation follows the same clean, minimal, and robust pattern as EspAdc.
 *
 * Key Features:
 * - Clean architectural pattern following EspAdc design
 * - Lazy initialization for efficient resource management
 * - Thread-safe operations with proper resource management
 * - Modern ESP-IDF v5.4+ handle-based TWAI API
 * - Support for all ESP32 family members
 * - Comprehensive error handling and diagnostics
 *
 * @note This implementation follows the same architectural pattern as EspAdc
 * @note Each EspCan instance represents a single TWAI controller
 */
class EspCan : public BaseCan {
public:
    //==============================================//
    // CONSTRUCTOR AND DESTRUCTOR
    //==============================================//

    /**
     * @brief Constructor with TWAI controller configuration.
     * @param config TWAI controller configuration parameters
     * @details **LAZY INITIALIZATION**: The TWAI controller is NOT physically configured
     *          until Initialize() is called. This follows the same pattern as EspAdc.
     */
    explicit EspCan(const hf_esp_can_config_t& config) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup and resource deallocation.
     */
    ~EspCan() noexcept override;

    //==============================================//
    // CORE CAN OPERATIONS (From BaseCan interface)
    //==============================================//

    /**
     * @brief Initialize the TWAI controller and allocate resources.
     * @return hf_can_err_t error code
     * @note This method configures the TWAI hardware and starts the driver
     */
    hf_can_err_t Initialize() noexcept override;

    /**
     * @brief Deinitialize the TWAI controller and free resources.
     * @return hf_can_err_t error code
     * @note This method stops the TWAI driver and releases all resources
     */
    hf_can_err_t Deinitialize() noexcept override;

    /**
     * @brief Send a CAN message.
     * @param message CAN message to send
     * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
     * @return hf_can_err_t error code
     */
    hf_can_err_t SendMessage(const hf_can_message_t& message, uint32_t timeout_ms = 1000) noexcept override;

    /**
     * @brief Receive a CAN message.
     * @param message Reference to store received message
     * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
     * @return hf_can_err_t error code
     */
    hf_can_err_t ReceiveMessage(hf_can_message_t& message, uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Set callback for received messages.
     * @param callback Callback function to handle received messages
     * @return hf_can_err_t error code
     */
    hf_can_err_t SetReceiveCallback(hf_can_receive_callback_t callback) noexcept override;

    /**
     * @brief Clear the receive callback.
     */
    void ClearReceiveCallback() noexcept override;

    /**
     * @brief Get current CAN bus status.
     * @param status Reference to store status information
     * @return hf_can_err_t error code
     */
    hf_can_err_t GetStatus(hf_can_status_t& status) noexcept override;

    /**
     * @brief Reset the CAN controller.
     * @return hf_can_err_t error code
     */
    hf_can_err_t Reset() noexcept override;

    /**
     * @brief Set acceptance filter for incoming messages.
     * @param id CAN ID to accept
     * @param mask Acceptance mask (0 = don't care bits)
     * @param extended true for extended frames, false for standard
     * @return hf_can_err_t error code
     */
    hf_can_err_t SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept override;

    /**
     * @brief Clear all acceptance filters (accept all messages).
     * @return hf_can_err_t error code
     */
    hf_can_err_t ClearAcceptanceFilter() noexcept override;

    //==============================================//
    // STATISTICS AND DIAGNOSTICS (From BaseCan)
    //==============================================//

    /**
     * @brief Get detailed statistics.
     * @param stats Reference to store statistics
     * @return hf_can_err_t error code
     */
    hf_can_err_t GetStatistics(hf_can_statistics_t& stats) noexcept override;

    /**
     * @brief Reset statistics counters.
     * @return hf_can_err_t error code
     */
    hf_can_err_t ResetStatistics() noexcept override;

    /**
     * @brief Get diagnostic information.
     * @param diagnostics Reference to store diagnostic data
     * @return hf_can_err_t error code
     */
    hf_can_err_t GetDiagnostics(hf_can_diagnostics_t& diagnostics) noexcept override;

private:
    //==============================================//
    // INTERNAL HELPER METHODS
    //==============================================//

    /**
     * @brief Convert HF CAN message to native TWAI message.
     * @param hf_message Source HF message
     * @param native_message Destination native message
     * @return hf_can_err_t error code
     */
    hf_can_err_t ConvertToNativeMessage(const hf_can_message_t& hf_message, 
                                       twai_frame_t& native_message) noexcept;

    /**
     * @brief Convert native TWAI message to HF CAN message.
     * @param native_message Source native message
     * @param hf_message Destination HF message
     * @return hf_can_err_t error code
     */
    hf_can_err_t ConvertFromNativeMessage(const twai_frame_t& native_message,
                                         hf_can_message_t& hf_message) noexcept;

    /**
     * @brief Convert ESP-IDF error to HF error code.
     * @param esp_err ESP-IDF error code
     * @return hf_can_err_t error code
     */
    hf_can_err_t ConvertEspError(esp_err_t esp_err) noexcept;

    /**
     * @brief Update statistics after an operation.
     * @param operation_type Type of operation
     * @param success Whether operation was successful
     */
    void UpdateStatistics(hf_can_operation_type_t operation_type, bool success) noexcept;

    //==============================================//
    // MEMBER VARIABLES (Following EspAdc pattern)
    //==============================================//

    // Configuration (centralized like EspAdc)
    const hf_esp_can_config_t config_;              ///< TWAI controller configuration

    // State flags (atomic like EspAdc)
    std::atomic<bool> is_initialized_;              ///< Initialization state
    std::atomic<bool> is_started_;                  ///< Started state

    // Thread safety (RtosMutex like EspAdc)
    mutable RtosMutex config_mutex_;                ///< Configuration mutex
    mutable RtosMutex stats_mutex_;                 ///< Statistics mutex

    // ESP-IDF TWAI handle (native handle like EspAdc)
    twai_node_handle_t twai_handle_;                ///< Native TWAI handle

    // Callbacks (similar to EspAdc callback management)
    hf_can_receive_callback_t receive_callback_;    ///< Receive message callback

    // Statistics and diagnostics (like EspAdc stats)
    hf_can_statistics_t statistics_;               ///< Performance statistics
    hf_can_diagnostics_t diagnostics_;             ///< Diagnostic information
};

//==============================================//


#endif // HF_MCU_FAMILY_ESP32
