/**
 * @file McuCan.h
 * @brief MCU-integrated CAN controller implementation.
 *
 * This header provides a CAN bus implementation for microcontrollers with
 * built-in CAN peripherals. On ESP32, this wraps TWAI (Two-Wire Automotive Interface),
 * on STM32 it would wrap CAN peripheral, etc. The implementation supports standard
 * and extended CAN frames, filtering, error handling, and interrupt-driven operation.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the primary CAN implementation for the ESP32C6 and similar MCUs
 *       that have integrated CAN controllers with external transceivers.
 */

#pragma once

#include "RtosMutex.h"
#include "BaseCan.h"
#include "McuTypes.h"

// Type aliases to centralized types in McuTypes.h (no duplicate type declarations)
using CanControllerId = hf_can_controller_id_t;
using CanMode = hf_can_mode_t;
using CanErrorState = hf_can_error_state_t;
using CanAlert = hf_can_alert_t;
using CanTimingConfig = hf_can_timing_config_t;
using CanGeneralConfig = hf_can_general_config_t;
using CanFilterConfig = hf_can_filter_config_t;
using CanStatusInfo = hf_can_status_info_t;
using CanCapabilities = hf_can_capabilities_t;

/**
 * @class McuCan
 * @brief Advanced CAN bus implementation for ESP32C6 with ESP-IDF v5.4.2+ TWAI support.
 *
 * This class provides comprehensive CAN communication using the ESP32C6's dual TWAI 
 * (Two-Wire Automotive Interface) controllers with modern ESP-IDF v5.4.2+ APIs.
 * The implementation leverages all advanced features including dual controller support,
 * sleep retention, comprehensive error handling, alert monitoring, and interrupt-driven
 * operation with robust error recovery mechanisms.
 *
 * Key Features:
 * - Dual TWAI controller support (ESP32C6 has 2 independent controllers)
 * - Modern ESP-IDF v5.4.2+ handle-based API with full thread safety
 * - Comprehensive error detection and bus recovery mechanisms
 * - Sleep retention for power-efficient operation
 * - Advanced filtering with runtime reconfiguration support
 * - Interrupt-driven callbacks with configurable alerts
 * - High-performance batch operations for improved throughput
 * - Extensive diagnostics and monitoring capabilities
 * - Production-ready error handling with automatic recovery
 *
 * Hardware Requirements:
 * - ESP32C6 microcontroller with 2 TWAI controllers
 * - External CAN transceiver (e.g., SN65HVD23x for ISO 11898-2)
 * - Proper bus termination and electrical isolation
 *
 * @note This implementation is optimized for ESP32C6 and requires an external 
 *       CAN transceiver for physical layer communication.
 * @note ESP32C6 TWAI controllers support classic CAN only (no CAN-FD).
 */
class McuCan : public BaseCan {
public:
  /**
   * @brief Constructor with configuration and optional controller selection.
   * @param config CAN bus configuration parameters
   * @param controller_id TWAI controller ID (0 or 1 for ESP32C6, default: 0)
   * @details **LAZY INITIALIZATION**: The CAN controller is NOT physically configured
   *          until the first call to EnsureInitialized(), Initialize(), or any CAN operation.
   *          This allows creating CAN objects without immediate hardware access.
   *          ESP32C6 automatically uses the modern node-based TWAI API.
   */  explicit McuCan(const CanBusConfig &config, 
                  CanControllerId controller_id = CanControllerId::HF_CAN_CONTROLLER_0) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup and resource deallocation.
   */
  ~McuCan() noexcept override;

  //==============================================================//
  // LAZY INITIALIZATION SUPPORT  
  //==============================================================//

  /**
   * @brief Ensure the CAN controller is initialized before use.
   * @details This method implements lazy initialization - the CAN controller is only
   *          physically configured when first accessed. This allows creating
   *          CAN objects without immediate hardware configuration.
   * @return true if initialization successful or already initialized, false otherwise
   */
  bool EnsureInitialized() noexcept;

  /**
   * @brief Check if the CAN controller has been initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

  // === Core BaseCan Interface Implementation ===
  bool Initialize() noexcept override;
  bool Deinitialize() noexcept override;
  bool SendMessage(const CanMessage &message, uint32_t timeout_ms = 1000) noexcept override;
  bool ReceiveMessage(CanMessage &message, uint32_t timeout_ms = 0) noexcept override;
  bool SetReceiveCallback(CanReceiveCallback callback) noexcept override;
  void ClearReceiveCallback() noexcept override;
  bool GetStatus(CanBusStatus &status) noexcept override;
  bool Reset() noexcept override;
  bool SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept override;
  bool ClearAcceptanceFilter() noexcept override;

  // === Advanced ESP32C6-Specific Operations ===

  /**
   * @brief Start the CAN controller with comprehensive error checking.
   * @return true if started successfully, false otherwise
   */
  bool Start() noexcept;

  /**
   * @brief Stop the CAN controller gracefully.
   * @return true if stopped successfully, false otherwise
   */
  bool Stop() noexcept;

  /**
   * @brief Enable/disable sleep retention for power management.
   * @param enable true to enable sleep retention, false to disable
   * @return true if configured successfully, false otherwise
   * @note Requires ESP-IDF v5.4.2+ and ESP32C6 sleep retention support
   */
  bool ConfigureSleepRetention(bool enable) noexcept;

  /**
   * @brief Configure alert monitoring for comprehensive error detection.
   * @param alerts Bitmask of alerts to enable (see hf_can_alert_t)
   * @return true if alerts configured successfully, false otherwise
   */
  bool ConfigureAlerts(uint32_t alerts) noexcept;

  /**
   * @brief Read and clear triggered alerts.
   * @param alerts_out Pointer to store triggered alert flags
   * @param timeout_ms Timeout for alert reading (0 = non-blocking)
   * @return true if alerts read successfully, false on timeout or error
   */
  bool ReadAlerts(uint32_t *alerts_out, uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Send multiple messages in batch for improved performance.
   * @param messages Array of messages to send
   * @param count Number of messages in array
   * @param timeout_ms Timeout for the entire batch operation
   * @return Number of messages successfully sent (0 to count)
   */
  uint32_t SendMessageBatch(const CanMessage *messages, uint32_t count,
                           uint32_t timeout_ms = 1000) noexcept override;

  /**
   * @brief Receive multiple messages in batch.
   * @param messages Array to store received messages
   * @param max_count Maximum number of messages to receive
   * @param timeout_ms Timeout for the batch operation
   * @return Number of messages actually received (0 to max_count)
   */
  uint32_t ReceiveMessageBatch(CanMessage *messages, uint32_t max_count,
                              uint32_t timeout_ms = 100) noexcept override;

  /**
   * @brief Reconfigure acceptance filters at runtime.
   * @param id CAN ID to accept
   * @param mask Acceptance mask (0 = don't care bits)
   * @param extended true for extended frames, false for standard
   * @param single_filter Use single filter mode (true) or dual filter mode (false)
   * @return true if filter reconfigured successfully, false otherwise
   * @note This operation temporarily stops and restarts the controller
   */
  bool ReconfigureAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false,
                                  bool single_filter = true) noexcept;

  /**
   * @brief Perform comprehensive bus recovery from error states.
   * @param force_reset Force a hard reset even if not in bus-off state
   * @return true if recovery successful, false otherwise
   */
  bool RecoverFromBusOff(bool force_reset = false) noexcept;

  // === Status and Diagnostics ===

  /**
   * @brief Get the current configuration.
   * @return Reference to the configuration structure
   */
  const CanBusConfig &GetConfig() const noexcept {
    return config_;
  }

  /**
   * @brief Get the TWAI controller ID being used.
   * @return Controller ID (0 or 1 for ESP32C6)
   */
  uint8_t GetControllerId() const noexcept {
    return controller_id_;
  }

  // Remove this method since ESP32C6 always uses modern node-based TWAI API

  /**
   * @brief Get comprehensive controller statistics.
   * @param stats_out Pointer to store statistics
   * @return true if statistics retrieved successfully
   */
  bool GetStatistics(CanControllerStats *stats_out) const noexcept;

  /**
   * @brief Reset controller statistics counters.
   */
  void ResetStatistics() noexcept;

  /**
   * @brief Check if the transmit queue is full.
   * @return true if queue is full, false otherwise
   */
  bool IsTransmitQueueFull() const noexcept;

  /**
   * @brief Check if the receive queue is empty.
   * @return true if queue is empty, false otherwise
   */
  bool IsReceiveQueueEmpty() const noexcept;

  /**
   * @brief Get the current transmit error count.
   * @return Number of transmit errors
   */
  uint32_t GetTransmitErrorCount() const noexcept;

  /**
   * @brief Get the current receive error count.
   * @return Number of receive errors
   */
  uint32_t GetReceiveErrorCount() const noexcept;

  /**
   * @brief Get the current queue levels.
   * @param tx_level_out Pointer to store TX queue level
   * @param rx_level_out Pointer to store RX queue level
   * @return true if levels retrieved successfully
   */
  bool GetQueueLevels(uint32_t *tx_level_out, uint32_t *rx_level_out) const noexcept;

  // === CAN-FD Support (ESP32C6 limitation - classic CAN only) ===
  bool SupportsCanFD() const noexcept override;
  bool SetCanFDMode(bool enable, uint32_t data_baudrate = 2000000,
                    bool enable_brs = true) noexcept override;
  bool ConfigureCanFDTiming(uint16_t nominal_prescaler, uint8_t nominal_tseg1,
                            uint8_t nominal_tseg2, uint16_t data_prescaler, uint8_t data_tseg1,
                            uint8_t data_tseg2, uint8_t sjw = 1) noexcept override;
  bool SetTransmitterDelayCompensation(uint8_t tdc_offset, uint8_t tdc_filter) noexcept override;
  bool GetCanFDCapabilities(uint8_t &max_data_bytes, uint32_t &max_nominal_baudrate,
                            uint32_t &max_data_baudrate, bool &supports_brs,
                            bool &supports_esi) noexcept override;

private:
  // === Configuration and State ===
  CanBusConfig config_;                         ///< CAN bus configuration
  CanControllerId controller_id_;               ///< TWAI controller ID (0 or 1 for ESP32C6)
  bool initialized_;                            ///< Lazy initialization flag
  CanReceiveCallback receive_callback_;         ///< User receive callback
  mutable RtosMutex mutex_;                     ///< Thread safety mutex
  hf_can_statistics_t stats_;                   ///< Performance statistics (thread-safe)
  uint64_t init_timestamp_;                     ///< Initialization timestamp

  // === ESP-IDF v5.4.2+ Handle Management ===
  hf_can_handle_t twai_handle_;                 ///< ESP-IDF v5.4.2+ TWAI handle
  bool handle_valid_;                           ///< Handle validity flag

  // === Runtime State Tracking ===
  volatile bool is_started_;                    ///< Controller started state
  volatile uint32_t current_alerts_;            ///< Currently configured alerts
  volatile hf_can_err_t last_error_code_;       ///< Last platform error code
  
  // === Internal Configuration Structures ===
  hf_can_general_config_t general_config_;      ///< Native general configuration
  hf_can_timing_config_t timing_config_;        ///< Native timing configuration
  hf_can_filter_config_t filter_config_;        ///< Native filter configuration
  hf_can_alert_config_t alert_config_;          ///< Alert configuration
  hf_can_power_config_t power_config_;          ///< Power management configuration

  // === Platform-Specific Implementation Methods ===
  
  // Core driver operations
  bool PlatformInitialize() noexcept;
  bool PlatformDeinitialize() noexcept;
  bool PlatformStart() noexcept;
  bool PlatformStop() noexcept;
  
  // Message operations
  bool PlatformSendMessage(const CanMessage &message, uint32_t timeout_ms) noexcept;
  bool PlatformReceiveMessage(CanMessage &message, uint32_t timeout_ms) noexcept;
  uint32_t PlatformSendMessageBatch(const CanMessage *messages, uint32_t count,
                                   uint32_t timeout_ms) noexcept;
  uint32_t PlatformReceiveMessageBatch(CanMessage *messages, uint32_t max_count,
                                      uint32_t timeout_ms) noexcept;
  
  // Status and diagnostics
  bool PlatformGetStatus(CanBusStatus &status) noexcept;
  bool PlatformReset() noexcept;
  bool PlatformGetNativeStatus(hf_can_status_info_t &native_status) noexcept;
  
  // Filter management
  bool PlatformSetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended) noexcept;
  bool PlatformClearAcceptanceFilter() noexcept;
  bool PlatformReconfigureFilter(uint32_t id, uint32_t mask, bool extended, bool single_filter) noexcept;
  
  // Alert and error handling
  bool PlatformConfigureAlerts(uint32_t alerts) noexcept;
  bool PlatformReadAlerts(uint32_t *alerts_out, uint32_t timeout_ms) noexcept;
  bool PlatformRecoverFromError() noexcept;

  // === Configuration and Conversion Helpers ===
  
  // Configuration building and validation
  bool BuildNativeGeneralConfig() noexcept;
  bool BuildNativeTimingConfig() noexcept;
  bool BuildNativeFilterConfig() noexcept;
  bool ValidateConfiguration() const noexcept;
  
  // Message format conversion
  bool ConvertToNativeMessage(const CanMessage &src, hf_can_message_native_t &dst) const noexcept;
  bool ConvertFromNativeMessage(const hf_can_message_native_t &src, CanMessage &dst) const noexcept;
  bool ConvertNativeStatus(const hf_can_status_info_t &native_status, CanBusStatus &status) const noexcept;
  
  // Timing calculations and validation
  bool CalculateTimingConfig(uint32_t baud_rate, hf_can_timing_config_t &timing_config) const noexcept;
  bool ValidateTimingParameters(const hf_can_timing_config_t &config) const noexcept;
  bool OptimizeTimingForBusLength(uint32_t bus_length_meters, hf_can_timing_config_t &config) const noexcept;

  // === Queue and State Management ===
  
  // Queue level monitoring
  bool PlatformIsTransmitQueueFull() const noexcept;
  bool PlatformIsReceiveQueueEmpty() const noexcept;
  bool PlatformGetQueueLevels(uint32_t *tx_level, uint32_t *rx_level) const noexcept;
  
  // Error counter access
  uint32_t PlatformGetTransmitErrorCount() const noexcept;
  uint32_t PlatformGetReceiveErrorCount() const noexcept;
  uint32_t PlatformGetArbitrationLostCount() const noexcept;
  uint32_t PlatformGetBusErrorCount() const noexcept;

  // === Statistics and Monitoring ===
  
  // Statistics tracking
  void UpdateSendStatistics(bool success) noexcept;
  void UpdateReceiveStatistics(bool success) noexcept;
  void UpdateErrorStatistics(hf_can_error_state_t error_state) noexcept;
  void UpdateQueueStatistics(uint32_t tx_level, uint32_t rx_level) noexcept;
  void IncrementEventCounter(uint64_t &counter) noexcept;
  
  // Performance monitoring
  bool MonitorPerformance() const noexcept;
  void LogPerformanceMetrics() const noexcept;
  
  // === Error Handling and Recovery ===
  
  // Error analysis and recovery
  bool AnalyzeErrorCondition() const noexcept;
  bool AttemptAutomaticRecovery() noexcept;
  bool PerformBusOffRecovery(bool force_reset) noexcept;
  void LogError(const char* operation, hf_can_err_t error_code) const noexcept;
  
  // === Interrupt and Callback Handling ===
  
  // Static callback handlers for platform interrupts
  static void StaticReceiveHandler(void *arg);
  static void StaticAlertHandler(void *arg);
  static void StaticErrorHandler(void *arg);
  
  // Instance-specific interrupt handlers
  void HandleReceiveInterrupt() noexcept;
  void HandleAlertInterrupt() noexcept;
  void HandleErrorInterrupt() noexcept;
  void ProcessIncomingMessage(const hf_can_message_native_t &native_message) noexcept;
  
  // === Utility and Helper Methods ===
  
  // Validation helpers
  bool IsValidCanId(uint32_t id, bool extended) const noexcept;
  bool IsValidDataLength(uint8_t dlc) const noexcept;
  bool IsValidBaudRate(uint32_t baud_rate) const noexcept;
  bool IsValidControllerState() const noexcept;
  
  // Resource management
  void CleanupResources() noexcept;
  void ResetInternalState() noexcept;
  uint64_t GetCurrentTimestamp() const noexcept;
  
  // Logging and debugging
  void LogConfigurationDetails() const noexcept;
  void LogStatusInformation() const noexcept;
  const char* GetControllerStateString() const noexcept;
  const char* GetErrorStateString(hf_can_error_state_t state) const noexcept;
};
