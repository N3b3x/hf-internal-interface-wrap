/**
 * @file EspCan.cpp
 * @brief ESP32 CAN (TWAI) implementation for the HardFOC system.
 *
 * This file provides a clean, minimal, and robust CAN bus implementation for the ESP32
 * microcontroller family using modern ESP-IDF v5.4+ TWAI (Two-Wire Automotive Interface) APIs.
 * The implementation follows the same clean, minimal, and robust architectural pattern as EspAdc.
 *
 * Key Features Implemented:
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
 * @note This implementation follows the EspAdc architectural pattern
 * @note Requires ESP-IDF v5.4 or later for full feature support
 */

#include "EspCan.h"

#ifdef HF_MCU_FAMILY_ESP32

#include <algorithm>
#include <cstring>

// ESP32-specific includes via centralized McuSelect.h (included in EspCan.h)
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "EspCan";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR - Following EspAdc Pattern  
//==============================================================================

EspCan::EspCan(const EspCanConfig& config) noexcept
    : config_(config)
    , is_initialized_(false)
    , is_started_(false)
    , config_mutex_()
    , stats_mutex_()
    , twai_handle_(nullptr)
    , receive_callback_(nullptr)
    , statistics_{}
    , diagnostics_{}
{
    // **LAZY INITIALIZATION** - Store configuration but do NOT initialize hardware
    // This follows the exact same pattern as EspAdc
    ESP_LOGD(TAG, "Creating EspCan for controller %d - LAZY INIT", 
             static_cast<int>(config_.controller_id));
}

EspCan::~EspCan() noexcept {
    // Ensure proper cleanup following EspAdc pattern
    if (is_initialized_.load()) {
        ESP_LOGW(TAG, "EspCan destructor called on initialized instance - performing cleanup");
        Deinitialize();
    }
    ESP_LOGD(TAG, "EspCan controller %d destroyed", static_cast<int>(config_.controller_id));
}

//==============================================//
// INITIALIZATION AND CONFIGURATION
//==============================================//

HfCanErr EspCan::Initialize() noexcept
{
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    if (is_initialized_.load()) {
        ESP_LOGD(TAG, "TWAI controller %u already initialized", 
                 static_cast<unsigned>(config_.controller_id));
        return HfCanErr::CAN_SUCCESS;
    }
    
    ESP_LOGD(TAG, "Initializing TWAI controller %u", 
             static_cast<unsigned>(config_.controller_id));

    // Create TWAI node configuration using modern ESP-IDF v5.4+ API
    twai_onchip_node_config_t node_config = {
        .io_cfg = {
            .tx = static_cast<gpio_num_t>(config_.tx_pin),
            .rx = static_cast<gpio_num_t>(config_.rx_pin),
        },
        .bit_timing = {
            .bitrate = config_.baud_rate,
        },
        .tx_queue_depth = config_.tx_queue_len,
        .flags = {
            .enable_self_test = (config_.mode == hf_can_mode_t::HF_CAN_MODE_NO_ACK),
            .enable_listen_only = (config_.mode == hf_can_mode_t::HF_CAN_MODE_LISTEN_ONLY),
        }
    };

    // Create TWAI node using ESP-IDF v5.4+ handle-based API
    esp_err_t esp_err = twai_new_node_onchip(&node_config, &twai_handle_);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create TWAI node: %s", esp_err_to_name(esp_err));
        return ConvertEspError(esp_err);
    }

    // Enable the TWAI node
    esp_err = twai_node_enable(twai_handle_);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable TWAI node: %s", esp_err_to_name(esp_err));
        twai_node_delete(twai_handle_);
        twai_handle_ = nullptr;
        return ConvertEspError(esp_err);
    }

    // Initialize statistics
    {
        std::lock_guard<RtosMutex> stats_lock(stats_mutex_);
        statistics_ = BaseCan::CanStatistics{};
        diagnostics_ = BaseCan::CanDiagnostics{};
    }

    is_initialized_.store(true);
    is_started_.store(true);  // Node is automatically started when enabled
    ESP_LOGI(TAG, "TWAI controller %u initialized successfully", 
             static_cast<unsigned>(config_.controller_id));
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::Deinitialize() noexcept
{
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    if (!is_initialized_.load()) {
        ESP_LOGD(TAG, "TWAI controller %u already deinitialized", 
                 static_cast<unsigned>(config_.controller_id));
        return HfCanErr::CAN_SUCCESS;
    }
    
    ESP_LOGD(TAG, "Deinitializing TWAI controller %u", 
             static_cast<unsigned>(config_.controller_id));

    // Clear callbacks
    receive_callback_ = nullptr;

    // Stop and delete TWAI node
    if (twai_handle_ != nullptr) {
        if (is_started_.load()) {
            twai_node_disable(twai_handle_);
            is_started_.store(false);
        }
        
        esp_err_t esp_err = twai_node_delete(twai_handle_);
        if (esp_err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to delete TWAI node: %s", esp_err_to_name(esp_err));
        }
        twai_handle_ = nullptr;
    }

    is_initialized_.store(false);
    ESP_LOGI(TAG, "TWAI controller %u deinitialized successfully", 
             static_cast<unsigned>(config_.controller_id));
    return HfCanErr::CAN_SUCCESS;
}

//==============================================//
// CORE CAN OPERATIONS (From BaseCan interface)
//==============================================//

HfCanErr EspCan::SendMessage(const CanMessage& message, uint32_t timeout_ms) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }

    // Convert to native TWAI message
    twai_frame_t native_frame;
    HfCanErr convert_result = ConvertToNativeMessage(message, native_frame);
    if (convert_result != HfCanErr::CAN_SUCCESS) {
        UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_SEND, false);
        return convert_result;
    }

    // Send using ESP-IDF TWAI API
    esp_err_t esp_err = twai_node_transmit(twai_handle_, &native_frame, timeout_ms);
    
    bool success = (esp_err == ESP_OK);
    UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_SEND, success);
    
    if (!success) {
        return ConvertEspError(esp_err);
    }

    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::ReceiveMessage(CanMessage& message, uint32_t timeout_ms) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }

    // Note: With modern ESP-IDF v5.4+ API, message reception is typically handled via callbacks
    // This method is provided for compatibility but may not be the preferred approach
    ESP_LOGW(TAG, "Polling receive not recommended with modern TWAI API - use callbacks instead");
    
    UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_RECEIVE, false);
    return HfCanErr::CAN_ERR_UNSUPPORTED_OPERATION;
}

HfCanErr EspCan::SetReceiveCallback(CanReceiveCallback callback) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    receive_callback_ = callback;
    
    // Register ESP-IDF callback if we have a user callback
    if (callback) {
        twai_event_callbacks_t event_cbs = {
            .on_rx_done = [](twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx) -> bool {
                EspCan* esp_can = static_cast<EspCan*>(user_ctx);
                if (esp_can && esp_can->receive_callback_) {
                    // Receive the message from the callback
                    twai_frame_t rx_frame;
                    
                    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
                        CanMessage hf_message;
                        if (esp_can->ConvertFromNativeMessage(rx_frame, hf_message) == HfCanErr::CAN_SUCCESS) {
                            esp_can->receive_callback_(hf_message);
                            esp_can->UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_RECEIVE, true);
                        }
                    }
                }
                return false;
            }
        };
        
        esp_err_t esp_err = twai_node_register_event_callbacks(twai_handle_, &event_cbs, this);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register TWAI callbacks: %s", esp_err_to_name(esp_err));
            return ConvertEspError(esp_err);
        }
    }
    
    ESP_LOGI(TAG, "Receive callback %s for TWAI controller %u", 
             callback ? "set" : "cleared", 
             static_cast<unsigned>(config_.controller_id));
    
    return HfCanErr::CAN_SUCCESS;
}

void EspCan::ClearReceiveCallback() noexcept
{
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    receive_callback_ = nullptr;
    
    // Clear ESP-IDF callbacks by registering empty callback structure
    if (twai_handle_ != nullptr) {
        twai_event_callbacks_t empty_cbs = {};
        twai_node_register_event_callbacks(twai_handle_, &empty_cbs, nullptr);
    }
    
    ESP_LOGI(TAG, "Receive callback cleared for TWAI controller %u", 
             static_cast<unsigned>(config_.controller_id));
}

HfCanErr EspCan::GetStatus(CanBusStatus& status) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    // Get TWAI status using modern API
    twai_node_status_t node_status;
    twai_node_record_t node_record;
    esp_err_t esp_err = twai_node_get_info(twai_handle_, &node_status, &node_record);
    if (esp_err != ESP_OK) {
        return ConvertEspError(esp_err);
    }
    
    // Convert to CAN bus status - match BaseCan.h structure
    status.tx_error_count = node_status.tx_error_count;
    status.rx_error_count = node_status.rx_error_count;
    status.tx_failed_count = 0;  // Not directly available in new API
    status.rx_missed_count = 0;  // Not directly available in new API
    
    // Set state flags based on TWAI state
    switch (node_status.state) {
        case TWAI_ERROR_BUS_OFF:
            status.bus_off = true;
            status.error_warning = false;
            status.error_passive = false;
            break;
        case TWAI_ERROR_ACTIVE:
            status.bus_off = false;
            status.error_warning = false;
            status.error_passive = false;
            break;
        case TWAI_ERROR_WARNING:
            status.bus_off = false;
            status.error_warning = true;
            status.error_passive = false;
            break;
        case TWAI_ERROR_PASSIVE:
            status.bus_off = false;
            status.error_warning = true;
            status.error_passive = true;
            break;
        default:
            status.bus_off = false;
            status.error_warning = false;
            status.error_passive = false;
            break;
    }
    
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::Reset() noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    ESP_LOGI(TAG, "Resetting TWAI controller %u", 
             static_cast<unsigned>(config_.controller_id));
    
    // Reset statistics
    {
        std::lock_guard<RtosMutex> stats_lock(stats_mutex_);
        statistics_ = BaseCan::CanStatistics{};
        diagnostics_ = BaseCan::CanDiagnostics{};
    }
    
    // Reset driver using recovery function
    esp_err_t esp_err = twai_node_recover(twai_handle_);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "TWAI recovery failed: %s", esp_err_to_name(esp_err));
        return ConvertEspError(esp_err);
    }
    
    ESP_LOGI(TAG, "TWAI controller %u reset successfully", 
             static_cast<unsigned>(config_.controller_id));
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    std::lock_guard<RtosMutex> lock(config_mutex_);
    
    // Configure mask filter using modern API
    twai_mask_filter_config_t filter_cfg = {
        .id = id,
        .mask = mask,
        .is_ext = extended ? 1U : 0U,
    };
    
    esp_err_t esp_err = twai_node_config_mask_filter(twai_handle_, 0, &filter_cfg);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure filter: %s", esp_err_to_name(esp_err));
        return ConvertEspError(esp_err);
    }
    
    ESP_LOGI(TAG, "TWAI controller %u filter configured (ID: 0x%X, Mask: 0x%X, Extended: %s)", 
             static_cast<unsigned>(config_.controller_id), id, mask, extended ? "yes" : "no");
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::ClearAcceptanceFilter() noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    // Set filter to accept all messages (ID=0, Mask=0)
    return SetAcceptanceFilter(0, 0, false);
}

//==============================================//
// STATISTICS AND DIAGNOSTICS
//==============================================//

HfCanErr EspCan::GetStatistics(BaseCan::CanStatistics& stats) noexcept
{
    std::lock_guard<RtosMutex> lock(stats_mutex_);
    stats = statistics_;
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::ResetStatistics() noexcept
{
    std::lock_guard<RtosMutex> lock(stats_mutex_);
    statistics_ = BaseCan::CanStatistics{};
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::GetDiagnostics(BaseCan::CanDiagnostics& diagnostics) noexcept
{
    if (!is_initialized_.load()) {
        return HfCanErr::CAN_ERR_NOT_INITIALIZED;
    }
    
    std::lock_guard<RtosMutex> lock(stats_mutex_);
    
    // Update diagnostics with current TWAI status
    twai_node_status_t node_status;
    twai_node_record_t node_record;
    esp_err_t esp_err = twai_node_get_info(twai_handle_, &node_status, &node_record);
    if (esp_err == ESP_OK) {
        diagnostics_.tx_error_count = node_status.tx_error_count;
        diagnostics_.rx_error_count = node_status.rx_error_count;
        diagnostics_.last_error_timestamp = esp_timer_get_time() / 1000;  // Convert to milliseconds
    }
    
    diagnostics = diagnostics_;
    return HfCanErr::CAN_SUCCESS;
}

//==============================================//
// INTERNAL HELPER METHODS
//==============================================//

HfCanErr EspCan::ConvertToNativeMessage(const CanMessage& message, twai_frame_t& native_message) noexcept
{
    // Validate message
    if (message.dlc > 8) {
        return HfCanErr::CAN_ERR_MESSAGE_INVALID_DLC;
    }
    
    // Clear native message
    std::memset(&native_message, 0, sizeof(native_message));
    
    // Set identifier
    native_message.id = message.id;
    
    // Set frame format
    if (message.is_extended) {
        native_message.flags |= TWAI_MSG_FLAG_EXTD;
    }
    
    // Set remote frame flag
    if (message.is_rtr) {
        native_message.flags |= TWAI_MSG_FLAG_RTR;
    }
    
    // Set data length
    native_message.data_length_code = message.dlc;
    
    // Copy data
    if (!message.is_rtr && message.dlc > 0) {
        std::memcpy(native_message.data, message.data, message.dlc);
    }
    
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::ConvertFromNativeMessage(const twai_frame_t& native_message, CanMessage& message) noexcept
{
    // Clear message
    message = CanMessage{};
    
    // Copy identifier
    message.id = native_message.id;
    
    // Set frame format
    message.is_extended = (native_message.flags & TWAI_MSG_FLAG_EXTD) != 0;
    
    // Set remote frame flag
    message.is_rtr = (native_message.flags & TWAI_MSG_FLAG_RTR) != 0;
    
    // Set data length
    message.dlc = native_message.data_length_code;
    
    // Copy data
    if (!message.is_rtr && message.dlc > 0) {
        std::memcpy(message.data, native_message.data, message.dlc);
    }
    
    return HfCanErr::CAN_SUCCESS;
}

HfCanErr EspCan::ConvertEspError(esp_err_t esp_err) noexcept
{
    switch (esp_err) {
        case ESP_OK:
            return HfCanErr::CAN_SUCCESS;
        case ESP_ERR_INVALID_ARG:
            return HfCanErr::CAN_ERR_INVALID_PARAMETER;
        case ESP_ERR_INVALID_STATE:
            return HfCanErr::CAN_ERR_INVALID_STATE;
        case ESP_ERR_TIMEOUT:
            return HfCanErr::CAN_ERR_MESSAGE_TIMEOUT;
        case ESP_ERR_NO_MEM:
            return HfCanErr::CAN_ERR_OUT_OF_MEMORY;
        case ESP_ERR_NOT_FOUND:
            return HfCanErr::CAN_ERR_DEVICE_NOT_RESPONDING;
        case ESP_FAIL:
            return HfCanErr::CAN_ERR_FAILURE;
        default:
            return HfCanErr::CAN_ERR_SYSTEM_ERROR;
    }
}

void EspCan::UpdateStatistics(hf_can_operation_type_t operation_type, bool success) noexcept
{
    std::lock_guard<RtosMutex> lock(stats_mutex_);
    
    switch (operation_type) {
        case hf_can_operation_type_t::HF_CAN_OP_SEND:
            if (success) {
                statistics_.messages_sent++;
            } else {
                statistics_.send_failures++;
            }
            break;
        case hf_can_operation_type_t::HF_CAN_OP_RECEIVE:
            if (success) {
                statistics_.messages_received++;
            } else {
                statistics_.receive_failures++;
            }
            break;
        default:
            break;
    }
    
    statistics_.last_activity_timestamp = esp_timer_get_time() / 1000;  // Convert to milliseconds
}
#endif // HF_MCU_FAMILY_ESP32
