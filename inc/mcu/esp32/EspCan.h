/**
 * @file EspCan.h
 * @brief ESP32 CAN (TWAI) implementation for the HardFOC system - ESP-IDF v5.5 Compatible.
 *
 * This file contains the ESP32 CAN (TWAI) implementation that extends the BaseCan
 * abstract class. It provides a clean, minimal, and robust CAN interface using
 * the modern ESP-IDF v5.5+ handle-based TWAI node API with comprehensive
 * testing support for ESP32-C6 with external SN65 transceiver.
 *
 * Key Features:
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
 * @note Each EspCan instance represents a single TWAI node
 * @note Higher-level applications should instantiate multiple EspCan objects for multi-controller
 * boards
 */

#pragma once

#include "McuSelect.h"

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// ESP-IDF v5.5 TWAI node-based API
#include "esp_twai.h"
#include "esp_twai_onchip.h"

#ifdef __cplusplus
}
#endif

#include "BaseCan.h"
#include "RtosMutex.h"
#include "utils/EspTypes_CAN.h"

#include <atomic>
#include <functional>
#include <memory>

//==============================================================================
// ESP32 TWAI NODE CONFIGURATION (ESP-IDF v5.5)
//==============================================================================

/**
 * @brief ESP32 TWAI node configuration structure for ESP-IDF v5.5.
 * @details Comprehensive configuration following ESP-IDF v5.5 node-based API.
 */
struct hf_esp_can_config_t {
  // Core GPIO configuration
  hf_pin_num_t tx_pin; ///< TX GPIO pin number
  hf_pin_num_t rx_pin; ///< RX GPIO pin number

  // Timing configuration
  uint32_t baud_rate;              ///< Target baud rate in bps
  uint32_t sample_point_permill;   ///< Sample point in permille (750 = 75%)
  uint32_t secondary_sample_point; ///< Secondary sample point for enhanced timing

  // Queue configuration
  uint32_t tx_queue_depth; ///< Transmit queue depth
  uint32_t rx_queue_depth; ///< Receive queue depth (internal)

  // Node behavior configuration
  hf_can_controller_id_t controller_id; ///< Controller ID (0 for ESP32-C6)
  hf_can_mode_t mode;                   ///< Operating mode
  int8_t fail_retry_cnt;                ///< Retry count (-1 = infinite, 0 = single shot)
  uint8_t intr_priority;                ///< Interrupt priority (0-3)

  // Advanced features
  bool enable_self_test;   ///< Enable self-test mode (no ACK required)
  bool enable_loopback;    ///< Enable loopback mode
  bool enable_listen_only; ///< Enable listen-only mode
  bool no_receive_rtr;     ///< Filter out remote frames when using filters
  bool enable_alerts;      ///< Enable alert monitoring

  // Clock source configuration
  uint32_t clk_flags; ///< Clock source flags for specific requirements

  hf_esp_can_config_t() noexcept
      : tx_pin(4), rx_pin(5), baud_rate(500000), sample_point_permill(750),
        secondary_sample_point(0), tx_queue_depth(10), rx_queue_depth(20),
        controller_id(hf_can_controller_id_t::HF_CAN_CONTROLLER_0),
        mode(hf_can_mode_t::HF_CAN_MODE_NORMAL), fail_retry_cnt(-1), intr_priority(1),
        enable_self_test(false), enable_loopback(false), enable_listen_only(false),
        no_receive_rtr(false), enable_alerts(true), clk_flags(0) {}
};

/**
 * @brief Advanced bit timing configuration for fine-tuning.
 */
struct hf_esp_can_timing_config_t {
  uint32_t brp;        ///< Baud rate prescaler
  uint32_t prop_seg;   ///< Propagation segment
  uint32_t tseg_1;     ///< Time segment 1 (before sample point)
  uint32_t tseg_2;     ///< Time segment 2 (after sample point)
  uint32_t sjw;        ///< Synchronization jump width
  uint32_t ssp_offset; ///< Secondary sample point offset

  hf_esp_can_timing_config_t() noexcept
      : brp(8), prop_seg(10), tseg_1(4), tseg_2(5), sjw(3), ssp_offset(0) {}
};

/**
 * @brief CAN message filter configuration for hardware filtering.
 */
struct hf_esp_can_filter_config_t {
  uint32_t id;         ///< Filter ID
  uint32_t mask;       ///< Filter mask (0 = don't care, 1 = must match)
  bool is_extended;    ///< true for 29-bit extended ID, false for 11-bit standard
  bool is_dual_filter; ///< Enable dual filter mode

  // Dual filter configuration (when is_dual_filter = true)
  uint32_t id2;   ///< Second filter ID (for dual mode)
  uint32_t mask2; ///< Second filter mask (for dual mode)

  hf_esp_can_filter_config_t() noexcept
      : id(0), mask(0), is_extended(false), is_dual_filter(false), id2(0), mask2(0) {}
};

//==============================================================================
// ENHANCED USER CALLBACK SYSTEM
//==============================================================================

/**
 * @brief Comprehensive error information structure for error callbacks
 */
struct hf_esp_can_error_info_t {
  uint32_t error_type;     ///< ESP-IDF error type
  uint32_t tx_error_count; ///< Transmit error counter
  uint32_t rx_error_count; ///< Receive error counter
  bool bus_off_state;      ///< Bus-off state
  bool error_warning;      ///< Error warning state
  bool error_passive;      ///< Error passive state
  uint64_t timestamp_us;   ///< Error timestamp in microseconds
};

/**
 * @brief Bus state information for state change callbacks
 */
struct hf_esp_can_state_info_t {
  uint32_t previous_state; ///< Previous bus state
  uint32_t current_state;  ///< Current bus state
  uint64_t timestamp_us;   ///< State change timestamp in microseconds
};

/**
 * @brief Transmission completion information
 */
struct hf_esp_can_tx_info_t {
  hf_can_message_t message; ///< Message that was transmitted
  bool success;             ///< Transmission success status
  uint64_t timestamp_us;    ///< Transmission timestamp in microseconds
};

/**
 * @brief User callback function types with user data support
 */
using hf_esp_can_receive_callback_t =
    std::function<void(const hf_can_message_t& message, void* user_data)>;
using hf_esp_can_error_callback_t =
    std::function<void(const hf_esp_can_error_info_t& error_info, void* user_data)>;
using hf_esp_can_state_callback_t =
    std::function<void(const hf_esp_can_state_info_t& state_info, void* user_data)>;
using hf_esp_can_tx_callback_t =
    std::function<void(const hf_esp_can_tx_info_t& tx_info, void* user_data)>;

/**
 * @class EspCan
 * @brief ESP32 CAN (TWAI) implementation using ESP-IDF v5.5+ node-based API.
 *
 * This class provides clean, comprehensive CAN communication using the ESP32's TWAI
 * (Two-Wire Automotive Interface) controllers with modern ESP-IDF v5.5+ node-based APIs.
 * The implementation supports ESP32-C6 and external SN65 CAN transceivers.
 *
 * Key Features:
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
 * @note This implementation requires ESP-IDF v5.5 or later
 * @note Each EspCan instance represents a single TWAI node
 */
class EspCan : public BaseCan {
public:
  //==============================================//
  // CONSTRUCTOR AND DESTRUCTOR
  //==============================================//

  /**
   * @brief Constructor with TWAI node configuration.
   * @param config TWAI node configuration parameters
   * @details **LAZY INITIALIZATION**: The TWAI node is NOT physically configured
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
   * @brief Initialize the TWAI node and allocate resources.
   * @return hf_can_err_t error code
   * @note This method configures the TWAI hardware and starts the node
   */
  hf_can_err_t Initialize() noexcept override;

  /**
   * @brief Deinitialize the TWAI node and free resources.
   * @return hf_can_err_t error code
   * @note This method stops the TWAI node and releases all resources
   */
  hf_can_err_t Deinitialize() noexcept override;

  /**
   * @brief Send a CAN message.
   * @param message CAN message to send
   * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
   * @return hf_can_err_t error code
   */
  hf_can_err_t SendMessage(const hf_can_message_t& message,
                           uint32_t timeout_ms = 1000) noexcept override;

  /**
   * @brief Receive a CAN message (legacy polling interface).
   * @param message Reference to store received message
   * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
   * @return hf_can_err_t error code
   * @note Callback-based reception is preferred for ESP-IDF v5.5+
   */
  hf_can_err_t ReceiveMessage(hf_can_message_t& message, uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Set callback for received messages (legacy support removed - use
   * RegisterReceiveCallback).
   * @param callback Callback function to handle received messages
   * @return hf_can_err_t error code
   */
  hf_can_err_t SetReceiveCallback(hf_can_receive_callback_t callback) noexcept override;

  /**
   * @brief Clear the receive callback (legacy support removed - use UnregisterCallback).
   */
  void ClearReceiveCallback() noexcept override;

  //==============================================//
  // SINGLE-CALLBACK PER EVENT MANAGEMENT
  //==============================================//

  // Receive
  hf_can_err_t SetReceiveCallbackEx(hf_esp_can_receive_callback_t cb,
                                    void* user_data = nullptr) noexcept;
  void ClearReceiveCallbackEx() noexcept;

  // Error
  hf_can_err_t SetErrorCallback(hf_esp_can_error_callback_t cb, void* user_data = nullptr) noexcept;
  void ClearErrorCallback() noexcept;

  // State change
  hf_can_err_t SetStateChangeCallback(hf_esp_can_state_callback_t cb,
                                      void* user_data = nullptr) noexcept;
  void ClearStateChangeCallback() noexcept;

  // TX complete
  hf_can_err_t SetTxCompleteCallback(hf_esp_can_tx_callback_t cb,
                                     void* user_data = nullptr) noexcept;
  void ClearTxCompleteCallback() noexcept;

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
  hf_can_err_t SetAcceptanceFilter(uint32_t id, uint32_t mask,
                                   bool extended = false) noexcept override;

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

  //==============================================//
  // ESP-IDF v5.5 SPECIFIC ADVANCED FEATURES
  //==============================================//

  /**
   * @brief Configure advanced bit timing parameters.
   * @param timing_config Advanced timing configuration
   * @return hf_can_err_t error code
   */
  hf_can_err_t ConfigureAdvancedTiming(const hf_esp_can_timing_config_t& timing_config) noexcept;

  /**
   * @brief Configure hardware acceptance filter with advanced options.
   * @param filter_config Filter configuration including dual filter support
   * @return hf_can_err_t error code
   */
  hf_can_err_t ConfigureAdvancedFilter(const hf_esp_can_filter_config_t& filter_config) noexcept;

  /**
   * @brief Initiate bus recovery from bus-off state.
   * @return hf_can_err_t error code
   */
  hf_can_err_t InitiateBusRecovery() noexcept;

  /**
   * @brief Get detailed node information including error states.
   * @param node_info Reference to store node information
   * @return hf_can_err_t error code
   */
  hf_can_err_t GetNodeInfo(twai_node_record_t& node_info) noexcept;

  /**
   * @brief Send multiple messages in a batch for improved performance.
   * @param messages Array of messages to send
   * @param count Number of messages
   * @param timeout_ms Timeout for each message
   * @return Number of messages successfully sent
   */
  uint32_t SendMessageBatch(const hf_can_message_t* messages, uint32_t count,
                            uint32_t timeout_ms = 1000) noexcept;

private:
  //==============================================//
  // INTERNAL EVENT CALLBACKS (ESP-IDF v5.5)
  //==============================================//

  /**
   * @brief Internal receive event callback for ESP-IDF v5.5 event system.
   * @param handle TWAI node handle
   * @param event_data Event data
   * @param user_ctx User context
   * @return true to yield to higher priority task
   */
  static bool InternalReceiveCallback(twai_node_handle_t handle,
                                      const twai_rx_done_event_data_t* event_data,
                                      void* user_ctx) noexcept;

  /**
   * @brief Internal error event callback for comprehensive error handling.
   * @param handle TWAI node handle
   * @param event_data Error event data
   * @param user_ctx User context
   * @return true to yield to higher priority task
   */
  static bool InternalErrorCallback(twai_node_handle_t handle,
                                    const twai_error_event_data_t* event_data,
                                    void* user_ctx) noexcept;

  /**
   * @brief Internal state change callback for bus recovery monitoring.
   * @param handle TWAI node handle
   * @param event_data State change event data
   * @param user_ctx User context
   * @return true to yield to higher priority task
   */
  static bool InternalStateChangeCallback(twai_node_handle_t handle,
                                          const twai_state_change_event_data_t* event_data,
                                          void* user_ctx) noexcept;

  //==============================================//
  // INTERNAL HELPER METHODS
  //==============================================//

  /**
   * @brief Convert HF CAN message to ESP-IDF v5.5 TWAI frame.
   * @param hf_message Source HF message
   * @param twai_frame Destination TWAI frame
   * @return hf_can_err_t error code
   */
  hf_can_err_t ConvertToTwaiFrame(const hf_can_message_t& hf_message,
                                  twai_frame_t& twai_frame) noexcept;

  /**
   * @brief Convert ESP-IDF v5.5 TWAI frame to HF CAN message.
   * @param twai_frame Source TWAI frame
   * @param hf_message Destination HF message
   * @return hf_can_err_t error code
   */
  hf_can_err_t ConvertFromTwaiFrame(const twai_frame_t& twai_frame,
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

  /**
   * @brief Process received message through callback system.
   * @param frame Received TWAI frame
   */
  void ProcessReceivedMessage(const twai_frame_t& frame) noexcept;

  /**
   * @brief Update error statistics from error event.
   * @param error_type Error type from ESP-IDF
   */
  void UpdateErrorStatistics(uint32_t error_type) noexcept;

  //==============================================//
  // CALLBACK DISPATCH HELPERS
  //==============================================//

  /**
   * @brief Dispatch message to all registered receive callbacks.
   * @param frame Received TWAI frame
   */
  void DispatchReceiveCallbacks(const twai_frame_t& frame) noexcept;

  /**
   * @brief Dispatch error event to all registered error callbacks.
   * @param error_info Error information
   */
  void DispatchErrorCallbacks(const hf_esp_can_error_info_t& error_info) noexcept;

  /**
   * @brief Dispatch state change to all registered state callbacks.
   * @param state_info State change information
   */
  void DispatchStateChangeCallbacks(const hf_esp_can_state_info_t& state_info) noexcept;

  /**
   * @brief Dispatch transmission complete to all registered TX callbacks.
   * @param tx_info Transmission information
   */
  void DispatchTxCompleteCallbacks(const hf_esp_can_tx_info_t& tx_info) noexcept;

  // (no registry helpers in single-callback design)

  //==============================================//
  // MEMBER VARIABLES (ESP-IDF v5.5 Compatible)
  //==============================================//

  // Configuration (centralized)
  const hf_esp_can_config_t config_; ///< TWAI node configuration

  // State flags (atomic)
  std::atomic<bool> is_enabled_;    ///< Node enabled state
  std::atomic<bool> is_recovering_; ///< Bus recovery state

  // Thread safety (RtosMutex)
  mutable RtosMutex config_mutex_;   ///< Configuration mutex
  mutable RtosMutex stats_mutex_;    ///< Statistics mutex
  mutable RtosMutex callback_mutex_; ///< Callback registry mutex

  // ESP-IDF v5.5 TWAI node handle
  twai_node_handle_t twai_node_handle_; ///< Native TWAI node handle

  // Single callbacks + user data per event type
  hf_esp_can_receive_callback_t receive_cb_{};
  void* receive_ud_ = nullptr;

  hf_esp_can_error_callback_t error_cb_{};
  void* error_ud_ = nullptr;

  hf_esp_can_state_callback_t state_cb_{};
  void* state_ud_ = nullptr;

  hf_esp_can_tx_callback_t tx_cb_{};
  void* tx_ud_ = nullptr;

  // Statistics and diagnostics
  hf_can_statistics_t statistics_;   ///< Performance statistics
  hf_can_diagnostics_t diagnostics_; ///< Diagnostic information

  // Advanced features
  hf_esp_can_timing_config_t advanced_timing_; ///< Advanced timing configuration
  hf_esp_can_filter_config_t current_filter_;  ///< Current filter configuration
  bool filter_configured_;                     ///< Filter configuration state
};

//==============================================//
