/**
 * @file EspCan.cpp
 * @brief ESP32 CAN (TWAI) implementation for the HardFOC system - ESP-IDF v5.5 Compatible.
 *
 * This file provides a comprehensive CAN bus implementation for the ESP32
 * microcontroller family using modern ESP-IDF v5.5+ TWAI node-based APIs.
 * The implementation provides advanced features for ESP32-C6 with external SN65 transceivers.
 *
 * Key Features Implemented:
 * - ESP-IDF v5.5+ handle-based TWAI node API
 * - ESP32-C6 compatible TWAI controller support
 * - Event-driven callback-based message reception
 * - Advanced acceptance filtering (single/dual mask modes)
 * - Comprehensive error detection and bus recovery
 * - Advanced bit timing configuration for various baud rates
 * - Thread-safe operations with proper resource management
 * - Support for external SN65 CAN transceivers
 * - Comprehensive diagnostics and performance monitoring
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation requires ESP-IDF v5.5 or later
 * @note Requires ESP-IDF v5.5 or later for full feature support
 */

#include "EspCan.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>

#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// ESP32-specific includes via centralized McuSelect.h (included in EspCan.h)
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

static const char* TAG = "EspCan";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR - ESP-IDF v5.5 Compatible
//==============================================================================

EspCan::EspCan(const hf_esp_can_config_t& config) noexcept
    : BaseCan(), config_(config), is_initialized_(false), is_enabled_(false), 
      is_recovering_(false), config_mutex_(), stats_mutex_(), callback_mutex_(),
      twai_node_handle_(nullptr),
      receive_cb_{}, receive_ud_(nullptr),
      error_cb_{}, error_ud_(nullptr),
      state_cb_{}, state_ud_(nullptr),
      tx_cb_{}, tx_ud_(nullptr),
      statistics_{}, diagnostics_{}, advanced_timing_{}, 
      current_filter_{}, filter_configured_(false) {
  // **LAZY INITIALIZATION** - Store configuration but do NOT initialize hardware
  // This follows the same pattern as EspAdc
  ESP_LOGD(TAG, "Creating EspCan for controller %d - LAZY INIT",
           static_cast<int>(config_.controller_id));
}

EspCan::~EspCan() noexcept {
  // Ensure proper cleanup following EspAdc pattern
  if (is_initialized_.load()) {
    ESP_LOGW(TAG, "EspCan destructor called on initialized instance - performing cleanup");
    Deinitialize();
  }
  
  // Clear callbacks (single-callback design)
  ClearReceiveCallbackEx();
  ClearErrorCallback();
  ClearStateChangeCallback();
  ClearTxCompleteCallback();
  
  ESP_LOGD(TAG, "EspCan controller %d destroyed", static_cast<int>(config_.controller_id));
}

//==============================================//
// INITIALIZATION AND CONFIGURATION
//==============================================//

hf_can_err_t EspCan::Initialize() noexcept {
  MutexLockGuard lock(config_mutex_);

  if (is_initialized_.load()) {
    ESP_LOGD(TAG, "TWAI node %u already initialized",
             static_cast<unsigned>(config_.controller_id));
    return hf_can_err_t::CAN_SUCCESS;
  }

  ESP_LOGD(TAG, "Initializing TWAI node %u", static_cast<unsigned>(config_.controller_id));

  // Create TWAI node configuration using ESP-IDF v5.5 API
  twai_onchip_node_config_t node_config = {
    .io_cfg = {
      .tx = static_cast<gpio_num_t>(config_.tx_pin),
      .rx = static_cast<gpio_num_t>(config_.rx_pin),
    },
    .bit_timing = {
      .bitrate = config_.baud_rate,
      .sp_permill = config_.sample_point_permill,
      .ssp_permill = config_.secondary_sample_point,
    },
    .tx_queue_depth = config_.tx_queue_depth,
    .fail_retry_cnt = config_.fail_retry_cnt,
    .intr_priority = config_.intr_priority,
    .flags = {
      .enable_self_test = config_.enable_self_test,
      .enable_loopback = config_.enable_loopback,
      .enable_listen_only = config_.enable_listen_only,
      .no_receive_rtr = config_.no_receive_rtr,
    },
  };

  // Create TWAI node using ESP-IDF v5.5 API
  esp_err_t esp_err = twai_new_node_onchip(&node_config, &twai_node_handle_);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create TWAI node: %s", esp_err_to_name(esp_err));
    return ConvertEspError(esp_err);
  }

  // Register event callbacks for comprehensive event handling
  twai_event_callbacks_t callbacks = {
    .on_rx_done = InternalReceiveCallback,
    .on_error = InternalErrorCallback,
    .on_state_change = InternalStateChangeCallback,
  };

  esp_err = twai_node_register_event_callbacks(twai_node_handle_, &callbacks, this);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register event callbacks: %s", esp_err_to_name(esp_err));
    twai_node_delete(twai_node_handle_);
    twai_node_handle_ = nullptr;
    return ConvertEspError(esp_err);
  }

  // Enable the TWAI node
  esp_err = twai_node_enable(twai_node_handle_);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable TWAI node: %s", esp_err_to_name(esp_err));
    twai_node_delete(twai_node_handle_);
    twai_node_handle_ = nullptr;
    return ConvertEspError(esp_err);
  }

  // Initialize statistics
  {
    MutexLockGuard stats_lock(stats_mutex_);
    statistics_ = hf_can_statistics_t{};
    diagnostics_ = hf_can_diagnostics_t{};
  }

  is_initialized_.store(true);
  is_enabled_.store(true);
  ESP_LOGI(TAG, "TWAI node %u initialized successfully",
           static_cast<unsigned>(config_.controller_id));
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::Deinitialize() noexcept {
  MutexLockGuard lock(config_mutex_);

  if (!is_initialized_.load()) {
    ESP_LOGD(TAG, "TWAI node %u already deinitialized",
             static_cast<unsigned>(config_.controller_id));
    return hf_can_err_t::CAN_SUCCESS;
  }

  ESP_LOGD(TAG, "Deinitializing TWAI node %u", static_cast<unsigned>(config_.controller_id));

  // Clear callbacks
  ClearReceiveCallbackEx();
  ClearErrorCallback();
  ClearStateChangeCallback();
  ClearTxCompleteCallback();

  // Disable and delete TWAI node
  if (twai_node_handle_ != nullptr) {
    if (is_enabled_.load()) {
      twai_node_disable(twai_node_handle_);
      is_enabled_.store(false);
    }

    esp_err_t esp_err = twai_node_delete(twai_node_handle_);
    if (esp_err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to delete TWAI node: %s", esp_err_to_name(esp_err));
    }
    twai_node_handle_ = nullptr;
  }

  is_initialized_.store(false);
  filter_configured_ = false;
  ESP_LOGI(TAG, "TWAI node %u deinitialized successfully",
           static_cast<unsigned>(config_.controller_id));
  return hf_can_err_t::CAN_SUCCESS;
}

//==============================================//
// CORE CAN OPERATIONS (From BaseCan interface)
//==============================================//

hf_can_err_t EspCan::SendMessage(const hf_can_message_t& message, hf_u32_t timeout_ms) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  // Convert to ESP-IDF v5.5 TWAI frame
  twai_frame_t frame;
  hf_can_err_t convert_result = ConvertToTwaiFrame(message, frame);
  if (convert_result != hf_can_err_t::CAN_SUCCESS) {
    UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_SEND, false);
    return convert_result;
  }

  // Send using ESP-IDF v5.5 TWAI node API
  esp_err_t esp_err = twai_node_transmit(twai_node_handle_, &frame, timeout_ms);

  bool success = (esp_err == ESP_OK);
  UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_SEND, success);

  // Dispatch TX complete callback to user callbacks
  hf_esp_can_tx_info_t tx_info;
  tx_info.message = message;
  tx_info.success = success;
  tx_info.timestamp_us = esp_timer_get_time();
  DispatchTxCompleteCallbacks(tx_info);

  if (!success) {
    return ConvertEspError(esp_err);
  }

  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::ReceiveMessage(hf_can_message_t& message, hf_u32_t timeout_ms) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  // Note: With ESP-IDF v5.5 node API, message reception is handled via callbacks
  // This method is provided for legacy compatibility but may not be the preferred approach
  ESP_LOGW(TAG, "Polling receive not recommended with ESP-IDF v5.5 node API - use callbacks instead");

  UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_RECEIVE, false);
  return hf_can_err_t::CAN_ERR_UNSUPPORTED_OPERATION;
}

hf_can_err_t EspCan::SetReceiveCallback(hf_can_receive_callback_t callback) noexcept {
  ESP_LOGW(TAG, "SetReceiveCallback (BaseCan) is deprecated - use SetReceiveCallbackEx instead");
  return hf_can_err_t::CAN_ERR_NOT_SUPPORTED;
}

void EspCan::ClearReceiveCallback() noexcept {
  ESP_LOGW(TAG, "ClearReceiveCallback (BaseCan) is deprecated - use ClearReceiveCallbackEx instead");
}

//==============================================================================
// SINGLE-CALLBACK PER EVENT MANAGEMENT
//==============================================================================

hf_can_err_t EspCan::SetReceiveCallbackEx(hf_esp_can_receive_callback_t cb, void* user_data) noexcept {
  if (!cb) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
  MutexLockGuard lock(callback_mutex_);
  receive_cb_ = std::move(cb);
  receive_ud_ = user_data;
  return hf_can_err_t::CAN_SUCCESS;
}
void EspCan::ClearReceiveCallbackEx() noexcept {
  MutexLockGuard lock(callback_mutex_);
  receive_cb_ = {};
  receive_ud_ = nullptr;
}

hf_can_err_t EspCan::SetErrorCallback(hf_esp_can_error_callback_t cb, void* user_data) noexcept {
  if (!cb) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
  MutexLockGuard lock(callback_mutex_);
  error_cb_ = std::move(cb);
  error_ud_ = user_data;
  return hf_can_err_t::CAN_SUCCESS;
}
void EspCan::ClearErrorCallback() noexcept {
  MutexLockGuard lock(callback_mutex_);
  error_cb_ = {};
  error_ud_ = nullptr;
}

hf_can_err_t EspCan::SetStateChangeCallback(hf_esp_can_state_callback_t cb, void* user_data) noexcept {
  if (!cb) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
  MutexLockGuard lock(callback_mutex_);
  state_cb_ = std::move(cb);
  state_ud_ = user_data;
  return hf_can_err_t::CAN_SUCCESS;
}
void EspCan::ClearStateChangeCallback() noexcept {
  MutexLockGuard lock(callback_mutex_);
  state_cb_ = {};
  state_ud_ = nullptr;
}

hf_can_err_t EspCan::SetTxCompleteCallback(hf_esp_can_tx_callback_t cb, void* user_data) noexcept {
  if (!cb) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
  MutexLockGuard lock(callback_mutex_);
  tx_cb_ = std::move(cb);
  tx_ud_ = user_data;
  return hf_can_err_t::CAN_SUCCESS;
}
void EspCan::ClearTxCompleteCallback() noexcept {
  MutexLockGuard lock(callback_mutex_);
  tx_cb_ = {};
  tx_ud_ = nullptr;
}

hf_can_err_t EspCan::GetStatus(hf_can_status_t& status) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  // Get TWAI node info using ESP-IDF v5.5 API
  twai_node_info_t node_info;
  esp_err_t esp_err = twai_node_get_info(twai_node_handle_, &node_info);
  if (esp_err != ESP_OK) {
    return ConvertEspError(esp_err);
  }

  // Convert to CAN bus status - match BaseCan.h structure
  status.tx_error_count = node_info.tx_error_counter;
  status.rx_error_count = node_info.rx_error_counter;
  status.tx_failed_count = 0; // Not directly available in new API
  status.rx_missed_count = 0; // Not directly available in new API

  // Set state flags based on TWAI state
  switch (node_info.state) {
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

  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::Reset() noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  MutexLockGuard lock(config_mutex_);

  ESP_LOGI(TAG, "Resetting TWAI node %u", static_cast<unsigned>(config_.controller_id));

  // Reset statistics
  {
    MutexLockGuard stats_lock(stats_mutex_);
    statistics_ = hf_can_statistics_t{};
    diagnostics_ = hf_can_diagnostics_t{};
  }

  // Reset node using recovery function
  esp_err_t esp_err = twai_node_recover(twai_node_handle_);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "TWAI node recovery failed: %s", esp_err_to_name(esp_err));
    return ConvertEspError(esp_err);
  }

  ESP_LOGI(TAG, "TWAI node %u reset successfully",
           static_cast<unsigned>(config_.controller_id));
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::SetAcceptanceFilter(hf_u32_t id, hf_u32_t mask, bool extended) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  hf_esp_can_filter_config_t filter_config;
  filter_config.id = id;
  filter_config.mask = mask;
  filter_config.is_extended = extended;
  filter_config.is_dual_filter = false;

  return ConfigureAdvancedFilter(filter_config);
}

hf_can_err_t EspCan::ClearAcceptanceFilter() noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  // Set filter to accept all messages (ID=0, Mask=0)
  return SetAcceptanceFilter(0, 0, false);
}

//==============================================//
// STATISTICS AND DIAGNOSTICS
//==============================================//

hf_can_err_t EspCan::GetStatistics(hf_can_statistics_t& stats) noexcept {
  MutexLockGuard lock(stats_mutex_);
  stats = statistics_;
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::ResetStatistics() noexcept {
  MutexLockGuard lock(stats_mutex_);
  statistics_ = hf_can_statistics_t{};
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::GetDiagnostics(hf_can_diagnostics_t& diagnostics) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  MutexLockGuard lock(stats_mutex_);

  // Update diagnostics with current TWAI node status
  twai_node_info_t node_info;
  esp_err_t esp_err = twai_node_get_info(twai_node_handle_, &node_info);
  if (esp_err == ESP_OK) {
    diagnostics_.tx_error_count = node_info.tx_error_counter;
    diagnostics_.rx_error_count = node_info.rx_error_counter;
    diagnostics_.last_error_timestamp = esp_timer_get_time() / 1000; // Convert to milliseconds
  }

  diagnostics = diagnostics_;
  return hf_can_err_t::CAN_SUCCESS;
}

//==============================================//
// ESP-IDF v5.5 SPECIFIC ADVANCED FEATURES
//==============================================//

hf_can_err_t EspCan::ConfigureAdvancedTiming(const hf_esp_can_timing_config_t& timing_config) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  MutexLockGuard lock(config_mutex_);

  // Configure advanced timing using ESP-IDF v5.5 API
  twai_timing_advanced_config_t advanced_timing = {
    .brp = timing_config.brp,
    .prop_seg = timing_config.prop_seg,
    .tseg_1 = timing_config.tseg_1,
    .tseg_2 = timing_config.tseg_2,
    .sjw = timing_config.sjw,
    .ssp_offset = timing_config.ssp_offset,
  };

  esp_err_t esp_err = twai_node_reconfig_timing(twai_node_handle_, &advanced_timing, nullptr);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure advanced timing: %s", esp_err_to_name(esp_err));
    return ConvertEspError(esp_err);
  }

  // Store configuration
  advanced_timing_ = timing_config;

  ESP_LOGI(TAG, "Advanced timing configured for TWAI node %u",
           static_cast<unsigned>(config_.controller_id));
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::ConfigureAdvancedFilter(const hf_esp_can_filter_config_t& filter_config) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  MutexLockGuard lock(config_mutex_);

  twai_mask_filter_config_t mask_filter;

  if (filter_config.is_dual_filter) {
    // Configure dual filter mode
    mask_filter = twai_make_dual_filter(
      filter_config.id, filter_config.mask,
      filter_config.id2, filter_config.mask2,
      filter_config.is_extended
    );
  } else {
    // Configure single filter mode
    mask_filter.id = filter_config.id;
    mask_filter.mask = filter_config.mask;
    mask_filter.is_ext = filter_config.is_extended;
  }

  esp_err_t esp_err = twai_node_config_mask_filter(twai_node_handle_, 0, &mask_filter);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure filter: %s", esp_err_to_name(esp_err));
    return ConvertEspError(esp_err);
  }

  // Store configuration
  current_filter_ = filter_config;
  filter_configured_ = true;

  ESP_LOGI(TAG, "Filter configured for TWAI node %u (ID: 0x%X, Mask: 0x%X, Extended: %s, Dual: %s)",
           static_cast<unsigned>(config_.controller_id), filter_config.id, filter_config.mask,
           filter_config.is_extended ? "yes" : "no", filter_config.is_dual_filter ? "yes" : "no");
  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::InitiateBusRecovery() noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  ESP_LOGI(TAG, "Initiating bus recovery for TWAI node %u",
           static_cast<unsigned>(config_.controller_id));

  is_recovering_.store(true);
  
  esp_err_t esp_err = twai_node_recover(twai_node_handle_);
  if (esp_err != ESP_OK) {
    is_recovering_.store(false);
    ESP_LOGE(TAG, "Failed to initiate bus recovery: %s", esp_err_to_name(esp_err));
    return ConvertEspError(esp_err);
  }

  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::GetNodeInfo(twai_node_info_t& node_info) noexcept {
  if (!is_initialized_.load()) {
    return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;
  }

  esp_err_t esp_err = twai_node_get_info(twai_node_handle_, &node_info);
  if (esp_err != ESP_OK) {
    return ConvertEspError(esp_err);
  }

  return hf_can_err_t::CAN_SUCCESS;
}

uint32_t EspCan::SendMessageBatch(const hf_can_message_t* messages, uint32_t count,
                                 uint32_t timeout_ms) noexcept {
  if (!messages || count == 0 || !is_initialized_.load()) {
    return 0;
  }

  uint32_t sent_count = 0;
  for (uint32_t i = 0; i < count; ++i) {
    if (SendMessage(messages[i], timeout_ms) == hf_can_err_t::CAN_SUCCESS) {
      sent_count++;
    } else {
      break; // Stop on first failure to maintain order
    }
  }

  return sent_count;
}

//==============================================//
// INTERNAL EVENT CALLBACKS (ESP-IDF v5.5)
//==============================================//

bool EspCan::InternalReceiveCallback(twai_node_handle_t handle,
                                   const twai_rx_done_event_data_t* event_data,
                                   void* user_ctx) noexcept {
  EspCan* esp_can = static_cast<EspCan*>(user_ctx);
  if (!esp_can) {
    return false;
  }

  // Receive message from ISR
  uint8_t recv_buffer[8];
  twai_frame_t rx_frame = {
    .buffer = recv_buffer,
    .buffer_len = sizeof(recv_buffer),
  };

  if (twai_node_receive_from_isr(handle, &rx_frame) == ESP_OK) {
    esp_can->ProcessReceivedMessage(rx_frame);
    esp_can->UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_RECEIVE, true);
  } else {
    esp_can->UpdateStatistics(hf_can_operation_type_t::HF_CAN_OP_RECEIVE, false);
  }

  return false; // Don't yield to higher priority task
}

bool EspCan::InternalErrorCallback(twai_node_handle_t handle,
                                 const twai_error_event_data_t* event_data,
                                 void* user_ctx) noexcept {
  EspCan* esp_can = static_cast<EspCan*>(user_ctx);
  if (!esp_can || !event_data) {
    return false;
  }

  esp_can->UpdateErrorStatistics(event_data->error_type);
  
  ESP_LOGW(TAG, "TWAI error occurred: type=0x%X", event_data->error_type);

  return false;
}

bool EspCan::InternalStateChangeCallback(twai_node_handle_t handle,
                                       const twai_state_change_event_data_t* event_data,
                                       void* user_ctx) noexcept {
  EspCan* esp_can = static_cast<EspCan*>(user_ctx);
  if (!esp_can || !event_data) {
    return false;
  }

  ESP_LOGI(TAG, "TWAI state changed");

  // Handle bus recovery completion
  if (esp_can->is_recovering_.load()) {
    esp_can->is_recovering_.store(false);
    ESP_LOGI(TAG, "Bus recovery completed successfully");
  }

  // Create state change info and dispatch to user callbacks
  hf_esp_can_state_info_t state_info;
  state_info.previous_state = 0; // Would need actual previous state from ESP-IDF
  state_info.current_state = 1;  // Would need actual current state from ESP-IDF
  state_info.timestamp_us = esp_timer_get_time();
  
  esp_can->DispatchStateChangeCallbacks(state_info);

  return false;
}

//==============================================//
// INTERNAL HELPER METHODS
//==============================================//

hf_can_err_t EspCan::ConvertToTwaiFrame(const hf_can_message_t& hf_message,
                                       twai_frame_t& twai_frame) noexcept {
  // Validate message
  if (hf_message.dlc > 8) {
    return hf_can_err_t::CAN_ERR_MESSAGE_INVALID_DLC;
  }

  // Clear frame
  std::memset(&twai_frame, 0, sizeof(twai_frame));

  // Set header fields
  twai_frame.header.id = hf_message.id;
  twai_frame.header.ide = hf_message.is_extended;
  twai_frame.header.rtr = hf_message.is_rtr;
  twai_frame.header.dlc = hf_message.dlc;

  // Set buffer and data length
  twai_frame.buffer_len = hf_message.dlc;
  
  // Copy data if not a remote frame
  if (!hf_message.is_rtr && hf_message.dlc > 0) {
    std::memcpy(const_cast<uint8_t*>(twai_frame.buffer), hf_message.data, hf_message.dlc);
  }

  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::ConvertFromTwaiFrame(const twai_frame_t& twai_frame,
                                         hf_can_message_t& hf_message) noexcept {
  // Clear message
  hf_message = hf_can_message_t{};

  // Copy header fields
  hf_message.id = twai_frame.header.id;
  hf_message.is_extended = twai_frame.header.ide;
  hf_message.is_rtr = twai_frame.header.rtr;
  hf_message.dlc = twai_frame.header.dlc;

  // Set timestamp
  hf_message.timestamp_us = esp_timer_get_time();

  // Copy data if not a remote frame
  if (!hf_message.is_rtr && hf_message.dlc > 0) {
    std::memcpy(hf_message.data, twai_frame.buffer, hf_message.dlc);
  }

  return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t EspCan::ConvertEspError(esp_err_t esp_err) noexcept {
  switch (esp_err) {
    case ESP_OK:
      return hf_can_err_t::CAN_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
    case ESP_ERR_INVALID_STATE:
      return hf_can_err_t::CAN_ERR_INVALID_STATE;
    case ESP_ERR_TIMEOUT:
      return hf_can_err_t::CAN_ERR_MESSAGE_TIMEOUT;
    case ESP_ERR_NO_MEM:
      return hf_can_err_t::CAN_ERR_OUT_OF_MEMORY;
    case ESP_ERR_NOT_FOUND:
      return hf_can_err_t::CAN_ERR_DEVICE_NOT_RESPONDING;
    case ESP_FAIL:
      return hf_can_err_t::CAN_ERR_FAILURE;
    default:
      return hf_can_err_t::CAN_ERR_SYSTEM_ERROR;
  }
}

void EspCan::UpdateStatistics(hf_can_operation_type_t operation_type, bool success) noexcept {
  MutexLockGuard lock(stats_mutex_);

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

  statistics_.last_activity_timestamp = esp_timer_get_time() / 1000; // Convert to milliseconds
}

//==============================================================================
// ENHANCED CALLBACK DISPATCH METHODS
//==============================================================================

void EspCan::ProcessReceivedMessage(const twai_frame_t& frame) noexcept {
  // Dispatch to callback system
  DispatchReceiveCallbacks(frame);
}

void EspCan::DispatchReceiveCallbacks(const twai_frame_t& frame) noexcept {
  // Convert to HF message format
  hf_can_message_t hf_message;
  if (ConvertFromTwaiFrame(frame, hf_message) != hf_can_err_t::CAN_SUCCESS) {
    return;
  }

  MutexLockGuard lock(callback_mutex_);
  if (receive_cb_) {
    receive_cb_(hf_message, receive_ud_);
  }
}

void EspCan::DispatchErrorCallbacks(const hf_esp_can_error_info_t& error_info) noexcept {
  MutexLockGuard lock(callback_mutex_);
  if (error_cb_) {
    error_cb_(error_info, error_ud_);
  }
}

void EspCan::DispatchStateChangeCallbacks(const hf_esp_can_state_info_t& state_info) noexcept {
  MutexLockGuard lock(callback_mutex_);
  if (state_cb_) {
    state_cb_(state_info, state_ud_);
  }
}

void EspCan::DispatchTxCompleteCallbacks(const hf_esp_can_tx_info_t& tx_info) noexcept {
  MutexLockGuard lock(callback_mutex_);
  if (tx_cb_) {
    tx_cb_(tx_info, tx_ud_);
  }
}

// (No registry helpers in single-callback design)

void EspCan::UpdateErrorStatistics(uint32_t error_type) noexcept {
  MutexLockGuard lock(stats_mutex_);
  
  // Update error counters
  switch (error_type) {
    case 0x01: // TX error
      statistics_.tx_error_count++;
      break;
    case 0x02: // RX error
      statistics_.rx_error_count++;
      break;
    case 0x04: // Bus error
      statistics_.bus_error_count++;
      break;
    default:
      statistics_.other_error_count++;
      break;
  }
  
  statistics_.last_error_timestamp = esp_timer_get_time() / 1000;
  
  // Create error info and dispatch to callbacks
  hf_esp_can_error_info_t error_info;
  error_info.error_type = error_type;
  error_info.tx_error_count = statistics_.tx_error_count;
  error_info.rx_error_count = statistics_.rx_error_count;
  error_info.bus_off_state = false; // Would need to query actual state
  error_info.error_warning = false; // Would need to query actual state
  error_info.error_passive = false; // Would need to query actual state
  error_info.timestamp_us = esp_timer_get_time();
  
  DispatchErrorCallbacks(error_info);
}

#endif // HF_MCU_FAMILY_ESP32
