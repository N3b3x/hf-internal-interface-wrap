/**
 * @file McuTypes_CAN.h
 * @brief MCU-specific CAN/TWAI type definitions for hardware abstraction.
 *
 * This header defines all CAN/TWAI-specific types and constants that are used
 * throughout the internal interface wrap layer for CAN operations. This includes
 * ESP32C6 dual TWAI controller support and ESP-IDF v5.5+ features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"

//==============================================================================
// PLATFORM-SPECIFIC CAN/TWAI TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 CAN/TWAI specific mappings for v5.5+
using hf_twai_handle_native_t = twai_handle_t;
using hf_twai_general_config_native_t = twai_general_config_t;
using hf_twai_timing_config_native_t = twai_timing_config_t;
using hf_twai_filter_config_native_t = twai_filter_config_t;
using hf_twai_message_native_t = twai_message_t;
using hf_twai_status_info_native_t = twai_status_info_t;
using hf_twai_mode_native_t = twai_mode_t;
using hf_twai_error_state_native_t = twai_error_state_t;

// ESP32 Timer specific mappings
using hf_esp_timer_handle_native_t = esp_timer_handle_t;
#else
// Non-ESP32 platforms - use generic types
using hf_twai_handle_native_t = void *;
using hf_twai_general_config_native_t = struct {
  int dummy;
};
using hf_twai_timing_config_native_t = struct {
  int dummy;
};
using hf_twai_filter_config_native_t = struct {
  int dummy;
};
using hf_twai_message_native_t = struct {
  int dummy;
};
using hf_twai_status_info_native_t = struct {
  int dummy;
};
using hf_twai_mode_native_t = uint8_t;
using hf_twai_error_state_native_t = uint8_t;
using hf_esp_timer_handle_native_t = void *;
#endif

//==============================================================================
// MCU-SPECIFIC CAN/TWAI TYPES
//==============================================================================

/**
 * @brief MCU-specific CAN statistics for performance monitoring.
 * @details Thread-safe statistics collection for production diagnostics.
 */
struct hf_can_statistics_t {
  std::atomic<uint32_t> tx_message_count;       ///< Total messages transmitted
  std::atomic<uint32_t> rx_message_count;       ///< Total messages received
  std::atomic<uint32_t> tx_error_count;         ///< Transmission errors
  std::atomic<uint32_t> rx_error_count;         ///< Reception errors
  std::atomic<uint32_t> arbitration_lost_count; ///< Arbitration lost events
  std::atomic<uint32_t> bus_error_count;        ///< Bus error events
  std::atomic<uint32_t> total_error_count;      ///< Total error count
  std::atomic<uint32_t> bus_off_count;          ///< Bus-off events
  std::atomic<uint32_t> recovery_count;         ///< Recovery events
  std::atomic<float> bus_load_percentage;       ///< Estimated bus load %
  uint64_t last_activity_timestamp;             ///< Last activity timestamp
  uint64_t initialization_timestamp;            ///< Initialization timestamp

  hf_can_statistics_t() noexcept
      : tx_message_count(0), rx_message_count(0), tx_error_count(0), rx_error_count(0),
        arbitration_lost_count(0), bus_error_count(0), total_error_count(0), bus_off_count(0),
        recovery_count(0), bus_load_percentage(0.0f), last_activity_timestamp(0),
        initialization_timestamp(0) {}
};

/**
 * @brief MCU-specific CAN alert configuration for comprehensive monitoring.
 * @details Configuration for alert system with callback support.
 */
struct hf_can_alert_config_t {
  uint32_t alerts_enabled;         ///< Bitfield of enabled alerts
  bool enable_bus_error;           ///< Enable bus error alerts
  bool enable_arbitration_lost;    ///< Enable arbitration lost alerts
  bool enable_tx_idle;             ///< Enable TX idle alerts
  bool enable_rx_data;             ///< Enable RX data alerts
  bool enable_rx_queue_full;       ///< Enable RX queue full alerts
  bool enable_tx_queue_empty;      ///< Enable TX queue empty alerts
  bool enable_above_error_warning; ///< Enable above error warning alerts
  bool enable_below_error_warning; ///< Enable below error warning alerts
  bool enable_error_passive;       ///< Enable error passive alerts
  bool enable_bus_off;             ///< Enable bus-off alerts
  bool enable_recovery;            ///< Enable recovery alerts
  uint32_t alert_queue_depth;      ///< Alert queue depth

  hf_can_alert_config_t() noexcept
      : alerts_enabled(0xFFFFFFFF), // Enable all alerts by default
        enable_bus_error(true), enable_arbitration_lost(true), enable_tx_idle(false),
        enable_rx_data(true), enable_rx_queue_full(true), enable_tx_queue_empty(false),
        enable_above_error_warning(true), enable_below_error_warning(true),
        enable_error_passive(true), enable_bus_off(true), enable_recovery(true),
        alert_queue_depth(10) {}
};

/**
 * @brief MCU-specific CAN power management configuration.
 * @details Power management settings for sleep modes and retention.
 */
struct hf_can_power_config_t {
  bool sleep_retention_enable;  ///< Enable sleep retention
  bool allow_pd_in_light_sleep; ///< Allow power down in light sleep
  bool allow_pd_in_deep_sleep;  ///< Allow power down in deep sleep
  uint32_t wakeup_filter_count; ///< Wakeup filter frame count
  uint32_t wakeup_filter_id;    ///< Wakeup filter ID
  uint32_t wakeup_filter_mask;  ///< Wakeup filter mask

  hf_can_power_config_t() noexcept
      : sleep_retention_enable(false), allow_pd_in_light_sleep(false),
        allow_pd_in_deep_sleep(false), wakeup_filter_count(1), wakeup_filter_id(0),
        wakeup_filter_mask(0xFFFFFFFF) {}
};

/**
 * @brief ESP32C6 enhanced TWAI timing configuration with optimization support.
 * @details Platform-specific timing parameters optimized for ESP32C6 40MHz APB clock.
 */
struct hf_twai_timing_config_t {
  uint32_t brp;                  ///< Baud rate prescaler (1-16384)
  uint8_t tseg_1;                ///< Time segment 1 (1-16)
  uint8_t tseg_2;                ///< Time segment 2 (1-8)
  uint8_t sjw;                   ///< Synchronization jump width (1-4)
  bool triple_sampling;          ///< Enable triple sampling for noise immunity
  uint32_t quanta_resolution_hz; ///< Time quantum resolution (0 = auto)

  // ESP-IDF v5.5+ enhanced timing features
  uint8_t sync_seg;              ///< Synchronization segment (always 1)
  uint32_t nominal_baudrate;     ///< Calculated nominal baudrate
  uint32_t actual_baudrate;      ///< Actual achieved baudrate
  float baudrate_accuracy;       ///< Baudrate accuracy percentage
  uint32_t bit_time_ns;          ///< Total bit time in nanoseconds
  uint32_t sample_point_percent; ///< Sample point percentage (60-90%)

  hf_twai_timing_config_t() noexcept
      : brp(8), tseg_1(15), tseg_2(4), sjw(3), triple_sampling(false), quanta_resolution_hz(0),
        sync_seg(1), nominal_baudrate(0), actual_baudrate(0), baudrate_accuracy(0.0f),
        bit_time_ns(0), sample_point_percent(87) {}
};

/**
 * @brief ESP32C6 enhanced TWAI general configuration with dual controller support.
 * @details Comprehensive configuration for ESP-IDF v5.5+ TWAI controller features.
 */
struct hf_twai_general_config_t {
  hf_twai_mode_t mode;      ///< Operating mode
  hf_gpio_num_t tx_io;      ///< TX GPIO pin number
  hf_gpio_num_t rx_io;      ///< RX GPIO pin number
  hf_gpio_num_t clkout_io;  ///< Clock output GPIO (optional)
  hf_gpio_num_t bus_off_io; ///< Bus-off indicator GPIO (optional)

  // Queue configuration
  uint32_t tx_queue_len; ///< TX queue length (1-64)
  uint32_t rx_queue_len; ///< RX queue length (1-64)

  // Alert and monitoring configuration
  uint32_t alerts_enabled; ///< Enabled alert flags bitmask
  uint32_t clkout_divider; ///< Clock output divider (0 = disabled)
  uint32_t intr_flags;     ///< Interrupt allocation flags

  // ESP32C6 specific features
  hf_twai_controller_id_t controller_id; ///< Controller ID (0 or 1)
  bool sleep_retention_enable;           ///< Enable sleep retention (ESP-IDF v5.5+)
  bool auto_recovery_enable;             ///< Enable automatic bus-off recovery
  uint32_t recovery_timeout_ms;          ///< Recovery timeout in milliseconds

  // Power management (ESP-IDF v5.5+)
  bool power_management_enable; ///< Enable power management
  bool clock_gating_enable;     ///< Enable clock gating for power saving
  uint32_t idle_timeout_ms;     ///< Idle timeout before power saving

  // Performance and reliability features
  uint32_t error_warning_limit;      ///< Error warning threshold (default: 96)
  uint32_t error_passive_limit;      ///< Error passive threshold (default: 128)
  uint32_t bus_off_recovery_time_ms; ///< Bus-off recovery time
  bool enable_advanced_diagnostics;  ///< Enable advanced error diagnostics

  hf_twai_general_config_t() noexcept
      : mode(hf_twai_mode_t::HF_TWAI_MODE_NORMAL), tx_io(HF_INVALID_PIN), rx_io(HF_INVALID_PIN),
        clkout_io(HF_INVALID_PIN), bus_off_io(HF_INVALID_PIN), tx_queue_len(10), rx_queue_len(10),
        alerts_enabled(static_cast<uint32_t>(hf_twai_alert_t::HF_TWAI_ALERT_ALL_ERRORS)),
        clkout_divider(0), intr_flags(0),
        controller_id(hf_twai_controller_id_t::HF_TWAI_CONTROLLER_0), sleep_retention_enable(false),
        auto_recovery_enable(true), recovery_timeout_ms(2000), power_management_enable(false),
        clock_gating_enable(false), idle_timeout_ms(5000), error_warning_limit(96),
        error_passive_limit(128), bus_off_recovery_time_ms(1000),
        enable_advanced_diagnostics(true) {}
};

/**
 * @brief Enhanced TWAI filter configuration with multi-filter support.
 * @details Hardware acceptance filter configuration with ESP-IDF v5.5+ enhancements.
 */
struct hf_twai_filter_config_t {
  uint32_t acceptance_code; ///< Acceptance code for filtering
  uint32_t acceptance_mask; ///< Acceptance mask (0 = don't care)
  bool single_filter;       ///< Single vs dual filter mode

  // ESP-IDF v5.5+ enhanced filtering features
  uint32_t acceptance_code_ext; ///< Extended acceptance code (29-bit)
  uint32_t acceptance_mask_ext; ///< Extended acceptance mask (29-bit)
  bool enable_std_filter;       ///< Enable standard frame filtering
  bool enable_ext_filter;       ///< Enable extended frame filtering
  bool enable_rtr_filter;       ///< Enable RTR frame filtering

  // Advanced filter configuration
  uint8_t filter_priority;      ///< Filter priority (0-7)
  bool filter_invert;           ///< Invert filter logic
  uint32_t filter_hit_counter;  ///< Filter hit counter (read-only)
  uint32_t filter_miss_counter; ///< Filter miss counter (read-only)

  hf_twai_filter_config_t() noexcept
      : acceptance_code(0), acceptance_mask(0xFFFFFFFF), single_filter(true),
        acceptance_code_ext(0), acceptance_mask_ext(0x1FFFFFFF), enable_std_filter(true),
        enable_ext_filter(true), enable_rtr_filter(true), filter_priority(0), filter_invert(false),
        filter_hit_counter(0), filter_miss_counter(0) {}
};

/**
 * @brief Enhanced TWAI message structure with ESP-IDF v5.5+ metadata.
 * @details Native message format with comprehensive timing and diagnostic information.
 */
struct hf_twai_message_t {
  uint32_t id;     ///< Message ID (11 or 29-bit)
  uint8_t dlc;     ///< Data length code (0-8)
  uint8_t data[8]; ///< Message data

  // Standard CAN flags
  bool is_extended;  ///< Extended ID flag (29-bit vs 11-bit)
  bool is_rtr;       ///< Remote transmission request flag
  bool is_ss;        ///< Single shot flag (no retransmission)
  bool is_self;      ///< Self reception request flag
  bool dlc_non_comp; ///< DLC is non-compliant (< 8)

  // ESP-IDF v5.5+ enhanced metadata
  uint64_t timestamp_us;    ///< Precise timestamp in microseconds
  uint32_t sequence_number; ///< Message sequence number
  uint8_t controller_id;    ///< Originating controller ID
  uint8_t queue_position;   ///< Position in queue when received

  // Reception and transmission metadata
  uint8_t retry_count;        ///< Number of transmission retries
  uint8_t error_count;        ///< Associated error count
  uint16_t bit_timing_errors; ///< Bit timing error flags
  uint16_t reception_flags;   ///< Reception condition flags

  // Advanced diagnostics (ESP-IDF v5.5+)
  float signal_quality;        ///< Signal quality indicator (0.0-1.0)
  uint8_t bus_load_percent;    ///< Bus load percentage when transmitted
  uint16_t inter_frame_gap_us; ///< Inter-frame gap in microseconds
  uint32_t crc_calculated;     ///< Calculated CRC for validation

  hf_twai_message_t() noexcept
      : id(0), dlc(0), data{}, is_extended(false), is_rtr(false), is_ss(false), is_self(false),
        dlc_non_comp(false), timestamp_us(0), sequence_number(0), controller_id(0),
        queue_position(0), retry_count(0), error_count(0), bit_timing_errors(0), reception_flags(0),
        signal_quality(1.0f), bus_load_percent(0), inter_frame_gap_us(0), crc_calculated(0) {}
};

/**
 * @brief Comprehensive TWAI status information for ESP32C6 with ESP-IDF v5.5+.
 * @details Enhanced status structure with detailed diagnostics and performance metrics.
 */
struct hf_twai_status_info_t {
  hf_twai_error_state_t state; ///< Current error state

  // Error counters
  uint32_t tx_error_counter; ///< Transmit error counter
  uint32_t rx_error_counter; ///< Receive error counter
  uint32_t tx_failed_count;  ///< Failed transmission count
  uint32_t rx_missed_count;  ///< Missed reception count

  // Queue status
  uint32_t rx_queue_len;  ///< Current RX queue length
  uint32_t tx_queue_len;  ///< Current TX queue length
  uint32_t rx_queue_peak; ///< Peak RX queue usage
  uint32_t tx_queue_peak; ///< Peak TX queue usage

  // Advanced error statistics (ESP-IDF v5.5+)
  uint32_t arbitration_lost_count; ///< Arbitration lost count
  uint32_t bus_error_count;        ///< Bus error count
  uint32_t stuff_error_count;      ///< Bit stuffing error count
  uint32_t form_error_count;       ///< Frame format error count
  uint32_t crc_error_count;        ///< CRC error count
  uint32_t ack_error_count;        ///< Acknowledgment error count
  uint32_t bit_error_count;        ///< Bit error count

  // Performance metrics
  uint32_t messages_transmitted; ///< Total messages transmitted
  uint32_t messages_received;    ///< Total messages received
  uint32_t bytes_transmitted;    ///< Total bytes transmitted
  uint32_t bytes_received;       ///< Total bytes received
  uint64_t bus_uptime_us;        ///< Bus uptime in microseconds

  // Real-time bus conditions
  uint8_t bus_load_percent;     ///< Current bus load percentage
  uint32_t dominant_bit_count;  ///< Dominant bit count in last frame
  uint32_t recessive_bit_count; ///< Recessive bit count in last frame
  float bit_error_rate;         ///< Bit error rate (errors/bits)

  // Power and clock status (ESP-IDF v5.5+)
  bool clock_stable;             ///< Clock source is stable
  bool power_domain_active;      ///< Power domain is active
  uint32_t sleep_wakeup_count;   ///< Number of sleep/wake cycles
  uint32_t clock_recovery_count; ///< Clock recovery events

  hf_twai_status_info_t() noexcept
      : state(hf_twai_error_state_t::HF_TWAI_ERROR_ACTIVE), tx_error_counter(0),
        rx_error_counter(0), tx_failed_count(0), rx_missed_count(0), rx_queue_len(0),
        tx_queue_len(0), rx_queue_peak(0), tx_queue_peak(0), arbitration_lost_count(0),
        bus_error_count(0), stuff_error_count(0), form_error_count(0), crc_error_count(0),
        ack_error_count(0), bit_error_count(0), messages_transmitted(0), messages_received(0),
        bytes_transmitted(0), bytes_received(0), bus_uptime_us(0), bus_load_percent(0),
        dominant_bit_count(0), recessive_bit_count(0), bit_error_rate(0.0f), clock_stable(true),
        power_domain_active(true), sleep_wakeup_count(0), clock_recovery_count(0) {}
};

/**
 * @brief ESP32C6 TWAI controller capabilities and limitations.
 * @details Static capability information for runtime feature detection.
 */
struct hf_twai_capabilities_t {
  uint8_t num_controllers;        ///< Number of TWAI controllers (2 for ESP32C6)
  uint8_t max_tx_queue_size;      ///< Maximum TX queue size
  uint8_t max_rx_queue_size;      ///< Maximum RX queue size
  uint32_t max_baudrate;          ///< Maximum supported baudrate
  uint32_t min_baudrate;          ///< Minimum supported baudrate
  bool supports_canfd;            ///< CAN-FD support (false for ESP32C6)
  bool supports_sleep_retention;  ///< Sleep retention support
  bool supports_dual_controllers; ///< Dual controller support
  bool supports_advanced_filters; ///< Advanced filtering support
  bool supports_power_management; ///< Power management support
  uint8_t num_hardware_filters;   ///< Number of hardware filters
  uint32_t min_bit_time_ns;       ///< Minimum bit time in nanoseconds
  uint32_t max_bit_time_ns;       ///< Maximum bit time in nanoseconds

  // Constructor with ESP32C6 defaults
  hf_twai_capabilities_t() noexcept
      : num_controllers(2), max_tx_queue_size(64), max_rx_queue_size(64), max_baudrate(1000000),
        min_baudrate(10000), supports_canfd(false), supports_sleep_retention(true),
        supports_dual_controllers(true), supports_advanced_filters(true),
        supports_power_management(true), num_hardware_filters(2), min_bit_time_ns(1000),
        max_bit_time_ns(100000000) {}
};

#else
// TWAI types for non-ESP32 platforms (simplified versions)
// Controller ID and other basic types

struct hf_twai_timing_config_t {
  uint32_t brp;
  uint8_t tseg_1;
  uint8_t tseg_2;
  uint8_t sjw;
  bool triple_sampling;
  uint32_t quanta_resolution_hz;

  hf_twai_timing_config_t() noexcept
      : brp(8), tseg_1(15), tseg_2(4), sjw(3), triple_sampling(false), quanta_resolution_hz(0) {}
};

struct hf_twai_general_config_t {
  hf_twai_mode_t mode;
  uint32_t tx_io;
  uint32_t rx_io;
  uint32_t tx_queue_len;
  uint32_t rx_queue_len;
  uint32_t alerts_enabled;
  hf_twai_controller_id_t controller_id;

  hf_twai_general_config_t() noexcept
      : mode(hf_twai_mode_t::HF_TWAI_MODE_NORMAL), tx_io(HF_INVALID_PIN), rx_io(HF_INVALID_PIN),
        tx_queue_len(10), rx_queue_len(10), alerts_enabled(0),
        controller_id(hf_twai_controller_id_t::HF_TWAI_CONTROLLER_0) {}
};

struct hf_twai_filter_config_t {
  uint32_t acceptance_code;
  uint32_t acceptance_mask;
  bool single_filter;

  hf_twai_filter_config_t() noexcept
      : acceptance_code(0), acceptance_mask(0xFFFFFFFF), single_filter(true) {}
};

struct hf_twai_message_t {
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
  bool is_extended;
  bool is_rtr;
  uint64_t timestamp_us;

  hf_twai_message_t() noexcept
      : id(0), dlc(0), data{}, is_extended(false), is_rtr(false), timestamp_us(0) {}
};

struct hf_twai_status_info_t {
  hf_twai_error_state_t state;
  uint32_t tx_error_counter;
  uint32_t rx_error_counter;
  uint32_t tx_failed_count;
  uint32_t rx_missed_count;
  uint32_t rx_queue_len;
  uint32_t tx_queue_len;

  hf_twai_status_info_t() noexcept
      : state(hf_twai_error_state_t::HF_TWAI_ERROR_ACTIVE), tx_error_counter(0),
        rx_error_counter(0), tx_failed_count(0), rx_missed_count(0), rx_queue_len(0),
        tx_queue_len(0) {}
};

struct hf_twai_capabilities_t {
  uint8_t num_controllers;
  uint8_t max_tx_queue_size;
  uint8_t max_rx_queue_size;
  uint32_t max_baudrate;
  uint32_t min_baudrate;
  bool supports_canfd;

  hf_twai_capabilities_t() noexcept
      : num_controllers(1), max_tx_queue_size(32), max_rx_queue_size(32), max_baudrate(1000000),
        min_baudrate(10000), supports_canfd(false) {}
};

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// ENHANCED ESP-IDF v5.5+ TWAI DRIVER FUNCTION MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// Enhanced ESP-IDF v5.5+ TWAI driver function mappings with comprehensive error handling

// Core driver lifecycle operations
#define HF_TWAI_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle)                               \
  twai_driver_install_v2(gconfig, tconfig, fconfig, handle)
#define HF_TWAI_DRIVER_UNINSTALL_V2(handle) twai_driver_uninstall_v2(handle)
#define HF_TWAI_START_V2(handle) twai_start_v2(handle)
#define HF_TWAI_STOP_V2(handle) twai_stop_v2(handle)

// Message transmission and reception operations
#define HF_TWAI_TRANSMIT_V2(handle, message, ticks) twai_transmit_v2(handle, message, ticks)
#define HF_TWAI_RECEIVE_V2(handle, message, ticks) twai_receive_v2(handle, message, ticks)

// Status and diagnostics operations
#define HF_TWAI_GET_STATUS_INFO_V2(handle, status) twai_get_status_info_v2(handle, status)
#define HF_TWAI_CLEAR_TRANSMIT_QUEUE_V2(handle) twai_clear_transmit_queue_v2(handle)
#define HF_TWAI_CLEAR_RECEIVE_QUEUE_V2(handle) twai_clear_receive_queue_v2(handle)

// Alert and monitoring operations
#define HF_TWAI_RECONFIGURE_ALERTS_V2(handle, alerts, prev)                                        \
  twai_reconfigure_alerts_v2(handle, alerts, prev)
#define HF_TWAI_READ_ALERTS_V2(handle, alerts, ticks) twai_read_alerts_v2(handle, alerts, ticks)

// Error recovery operations
#define HF_TWAI_INITIATE_RECOVERY_V2(handle) twai_initiate_recovery_v2(handle)
#define HF_TWAI_GET_ERROR_COUNTERS_V2(handle, tx_err, rx_err)                                      \
  twai_get_error_counters_v2(handle, tx_err, rx_err)

// Legacy v1 API for single controller backwards compatibility
#define HF_TWAI_DRIVER_INSTALL(gconfig, tconfig, fconfig)                                          \
  twai_driver_install(gconfig, tconfig, fconfig)
#define HF_TWAI_DRIVER_UNINSTALL() twai_driver_uninstall()
#define HF_TWAI_START() twai_start()
#define HF_TWAI_STOP() twai_stop()
#define HF_TWAI_TRANSMIT(message, ticks) twai_transmit(message, ticks)
#define HF_TWAI_RECEIVE(message, ticks) twai_receive(message, ticks)
#define HF_TWAI_GET_STATUS_INFO(status) twai_get_status_info(status)
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable)                                       \
  twai_configure_sleep_retention_v2(handle, enable)
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) twai_set_power_management_v2(handle, enable)
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) twai_get_capabilities_v2(handle, caps)


#else
// Non-ESP32 platforms - placeholder definitions that return not supported
#define HF_TWAI_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_DRIVER_UNINSTALL_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_START_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_STOP_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_TRANSMIT_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_RECEIVE_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_STATUS_INFO_V2(handle, status) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_CLEAR_TRANSMIT_QUEUE_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_CLEAR_RECEIVE_QUEUE_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_RECONFIGURE_ALERTS_V2(handle, alerts, prev) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_READ_ALERTS_V2(handle, alerts, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_INITIATE_RECOVERY_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_ERROR_COUNTERS_V2(handle, tx_err, rx_err) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) HF_CAN_ERR_NOT_SUPPORTED

#define HF_TWAI_DRIVER_INSTALL(gconfig, tconfig, fconfig) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_DRIVER_UNINSTALL() HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_START() HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_STOP() HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_TRANSMIT(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_RECEIVE(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_STATUS_INFO(status) HF_CAN_ERR_NOT_SUPPORTED

// Default to not supported if version detection fails
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) ESP_ERR_NOT_SUPPORTED
#endif

//==============================================================================
// TWAI UTILITY MACROS AND CONSTANTS
//==============================================================================

/**
 * @brief ESP32C6 TWAI controller physical limits and capabilities.
 */
#ifdef HF_MCU_ESP32C6
static constexpr uint8_t HF_TWAI_MAX_CONTROLLERS = 2;      ///< ESP32C6 has 2 TWAI controllers
static constexpr uint32_t HF_TWAI_APB_CLOCK_HZ = 40000000; ///< ESP32C6 APB clock frequency
static constexpr uint32_t HF_TWAI_MAX_BRP = 16384;         ///< Maximum baud rate prescaler
static constexpr uint8_t HF_TWAI_MAX_TSEG1 = 16;           ///< Maximum time segment 1
static constexpr uint8_t HF_TWAI_MAX_TSEG2 = 8;            ///< Maximum time segment 2
static constexpr uint8_t HF_TWAI_MAX_SJW = 4;              ///< Maximum synchronization jump width
static constexpr uint32_t HF_TWAI_MIN_BAUDRATE = 1000;     ///< Minimum supported baudrate
static constexpr uint32_t HF_TWAI_MAX_BAUDRATE = 1000000;  ///< Maximum supported baudrate
static constexpr uint8_t HF_TWAI_MAX_QUEUE_SIZE = 64;      ///< Maximum queue size
static constexpr uint8_t HF_TWAI_MIN_QUEUE_SIZE = 1;       ///< Minimum queue size
#else
static constexpr uint8_t HF_TWAI_MAX_CONTROLLERS = 1;
static constexpr uint32_t HF_TWAI_APB_CLOCK_HZ = 80000000;
static constexpr uint32_t HF_TWAI_MAX_BRP = 16384;
static constexpr uint8_t HF_TWAI_MAX_TSEG1 = 16;
static constexpr uint8_t HF_TWAI_MAX_TSEG2 = 8;
static constexpr uint8_t HF_TWAI_MAX_SJW = 4;
static constexpr uint32_t HF_TWAI_MIN_BAUDRATE = 1000;
static constexpr uint32_t HF_TWAI_MAX_BAUDRATE = 1000000;
static constexpr uint8_t HF_TWAI_MAX_QUEUE_SIZE = 32;
static constexpr uint8_t HF_TWAI_MIN_QUEUE_SIZE = 1;
#endif

/**
 * @brief TWAI message and protocol constants.
 */
static constexpr uint8_t HF_TWAI_MAX_DATA_LEN = 8;          ///< Classic CAN max data length
static constexpr uint32_t HF_TWAI_STD_ID_MASK = 0x7FF;      ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_TWAI_EXT_ID_MASK = 0x1FFFFFFF; ///< Extended ID mask (29-bit)
static constexpr uint32_t HF_TWAI_MAX_STD_ID = 0x7FF;       ///< Maximum standard ID
static constexpr uint32_t HF_TWAI_MAX_EXT_ID = 0x1FFFFFFF;  ///< Maximum extended ID

/**
 * @brief TWAI timing calculation macros for ESP32C6.
 */
#define HF_TWAI_CALCULATE_BIT_TIME_NS(brp, tseg1, tseg2)                                           \
  (((brp) * ((tseg1) + (tseg2) + 1) * 1000000000ULL) / HF_TWAI_APB_CLOCK_HZ)

#define HF_TWAI_CALCULATE_BAUDRATE(brp, tseg1, tseg2)                                              \
  (HF_TWAI_APB_CLOCK_HZ / ((brp) * ((tseg1) + (tseg2) + 1)))

#define HF_TWAI_CALCULATE_SAMPLE_POINT_PERCENT(tseg1, tseg2)                                       \
  (((tseg1) + 1) * 100 / ((tseg1) + (tseg2) + 1))

/**
 * @brief TWAI validation macros.
 */
#define HF_TWAI_IS_VALID_CONTROLLER_ID(id) ((id) < HF_TWAI_MAX_CONTROLLERS)

#define HF_TWAI_IS_VALID_BAUDRATE(rate)                                                            \
  ((rate) >= HF_TWAI_MIN_BAUDRATE && (rate) <= HF_TWAI_MAX_BAUDRATE)

#define HF_TWAI_IS_VALID_QUEUE_SIZE(size)                                                          \
  ((size) >= HF_TWAI_MIN_QUEUE_SIZE && (size) <= HF_TWAI_MAX_QUEUE_SIZE)

#define HF_TWAI_IS_VALID_STD_ID(id) ((id) <= HF_TWAI_MAX_STD_ID)

#define HF_TWAI_IS_VALID_EXT_ID(id) ((id) <= HF_TWAI_MAX_EXT_ID)

#define HF_TWAI_IS_VALID_DLC(dlc) ((dlc) <= HF_TWAI_MAX_DATA_LEN)


/**
 * @brief ESP32C6 CAN handle types for v5.5+ API.
 * @details Platform-specific handle types for ESP-IDF v5.5+ TWAI driver.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_can_handle_t = void *;        ///< TWAI driver handle (twai_handle_t in ESP-IDF)
using hf_can_obj_config_t = void *;    ///< TWAI object config (twai_obj_config_t in ESP-IDF)
using hf_can_filter_handle_t = void *; ///< TWAI filter handle
#else
using hf_can_handle_t = void *;        ///< Generic handle for other platforms
using hf_can_obj_config_t = void *;    ///< Generic config for other platforms
using hf_can_filter_handle_t = void *; ///< Generic filter handle
#endif

//==============================================================================
// ESP32C6 ENHANCED TWAI/CAN TYPES FOR ESP-IDF v5.5+ FEATURES
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief Enhanced ESP32C6 TWAI controller IDs with dual controller support.
 * @details ESP32C6 has 2 independent TWAI controllers that can operate simultaneously.
 */
enum class hf_twai_controller_id_t : uint8_t {
  HF_TWAI_CONTROLLER_0 = 0,   ///< Primary TWAI controller (default)
  HF_TWAI_CONTROLLER_1 = 1,   ///< Secondary TWAI controller (ESP32C6 specific)
  HF_TWAI_CONTROLLER_MAX = 2, ///< Maximum number of controllers
};

/**
 * @brief Enhanced TWAI operating modes for ESP-IDF v5.5+ with sleep support.
 * @details Extended mode configuration with power management features.
 */
enum class hf_twai_mode_t : uint8_t {
  HF_TWAI_MODE_NORMAL = 0,      ///< Normal mode with acknowledgment
  HF_TWAI_MODE_NO_ACK = 1,      ///< No acknowledgment mode (self-test)
  HF_TWAI_MODE_LISTEN_ONLY = 2, ///< Listen-only mode (monitoring)
  HF_TWAI_MODE_LOOPBACK = 3,    ///< Internal loopback for testing
  HF_TWAI_MODE_SLEEP = 4,       ///< Sleep mode (ESP-IDF v5.5+)
  HF_TWAI_MODE_RECOVERY = 5,    ///< Bus recovery mode (ESP-IDF v5.5+)
};

/**
 * @brief Enhanced TWAI error states with detailed recovery information.
 * @details Maps to ESP-IDF v5.5+ twai_error_state_t with additional states.
 */
enum class hf_twai_error_state_t : uint8_t {
  HF_TWAI_ERROR_ACTIVE = 0,     ///< Error active: TEC/REC < 96
  HF_TWAI_ERROR_WARNING = 1,    ///< Error warning: TEC/REC >= 96 and < 128
  HF_TWAI_ERROR_PASSIVE = 2,    ///< Error passive: TEC/REC >= 128 and < 256
  HF_TWAI_ERROR_BUS_OFF = 3,    ///< Bus-off: TEC >= 256 (node offline)
  HF_TWAI_ERROR_RECOVERING = 4, ///< Recovery in progress (ESP-IDF v5.5+)
};

/**
 * @brief Comprehensive TWAI alert flags for ESP-IDF v5.5+ monitoring.
 * @details Enhanced alert system for comprehensive error detection and diagnostics.
 */
enum class hf_twai_alert_t : uint32_t {
  HF_TWAI_ALERT_NONE = 0x00000000, ///< No alerts
  
  // Basic operation alerts
  HF_TWAI_ALERT_TX_IDLE = 0x00000001,    ///< TX queue empty
  HF_TWAI_ALERT_TX_SUCCESS = 0x00000002, ///< TX successful
  HF_TWAI_ALERT_RX_DATA = 0x00000004,    ///< RX data available
  HF_TWAI_ALERT_TX_FAILED = 0x00000008,  ///< TX failed
  
  // Error state alerts
  HF_TWAI_ALERT_ERR_ACTIVE = 0x00000010,  ///< Error active state
  HF_TWAI_ALERT_ERR_WARNING = 0x00000020, ///< Error warning state
  HF_TWAI_ALERT_ERR_PASSIVE = 0x00000040, ///< Error passive state
  HF_TWAI_ALERT_BUS_OFF = 0x00000080,     ///< Bus-off state
  
  // Queue and buffer alerts
  HF_TWAI_ALERT_RX_QUEUE_FULL = 0x00000100,    ///< RX queue full
  HF_TWAI_ALERT_TX_QUEUE_FULL = 0x00000200,    ///< TX queue full
  HF_TWAI_ALERT_RX_FIFO_OVERRUN = 0x00000400,  ///< RX FIFO overrun
  HF_TWAI_ALERT_TX_FIFO_UNDERRUN = 0x00000800, ///< TX FIFO underrun
  
  // Bus condition alerts
  HF_TWAI_ALERT_ARBITRATION_LOST = 0x00001000, ///< Arbitration lost
  HF_TWAI_ALERT_BUS_ERROR = 0x00002000,        ///< Bus error detected
  HF_TWAI_ALERT_STUFF_ERROR = 0x00004000,      ///< Bit stuffing error
  HF_TWAI_ALERT_FORM_ERROR = 0x00008000,       ///< Frame format error
  
  // Advanced ESP-IDF v5.5+ alerts
  HF_TWAI_ALERT_CRC_ERROR = 0x00010000,     ///< CRC error
  HF_TWAI_ALERT_ACK_ERROR = 0x00020000,     ///< Acknowledgment error
  HF_TWAI_ALERT_RECOVERY_COMPLETE = 0x00040000, ///< Recovery completed
  HF_TWAI_ALERT_SLEEP_WAKEUP = 0x00080000,  ///< Wake from sleep
  
  // Convenience flags
  HF_TWAI_ALERT_ALL = 0xFFFFFFFF,           ///< All alerts enabled
};

// CAN controller ID is an alias to TWAI controller ID for consistency
using hf_can_controller_id_t = hf_twai_controller_id_t;
using hf_can_mode_t = hf_twai_mode_t;
using hf_can_error_state_t = hf_twai_error_state_t;
using hf_can_alert_t = hf_twai_alert_t;

// CAN constants as aliases to TWAI constants
constexpr hf_can_controller_id_t HF_CAN_CONTROLLER_0 = hf_twai_controller_id_t::HF_TWAI_CONTROLLER_0;
constexpr hf_can_controller_id_t HF_CAN_CONTROLLER_1 = hf_twai_controller_id_t::HF_TWAI_CONTROLLER_1;
constexpr hf_can_controller_id_t HF_CAN_CONTROLLER_MAX = hf_twai_controller_id_t::HF_TWAI_CONTROLLER_MAX;

#else
// Non-ESP32 platforms - simplified types
enum class hf_twai_controller_id_t : uint8_t {
  HF_TWAI_CONTROLLER_0 = 0,
  HF_TWAI_CONTROLLER_MAX = 1,
};

enum class hf_twai_mode_t : uint8_t {
  HF_TWAI_MODE_NORMAL = 0,
  HF_TWAI_MODE_LISTEN_ONLY = 1,
};

enum class hf_twai_error_state_t : uint8_t {
  HF_TWAI_ERROR_ACTIVE = 0,
  HF_TWAI_ERROR_PASSIVE = 1,
  HF_TWAI_ERROR_BUS_OFF = 2,
};

enum class hf_twai_alert_t : uint32_t {
  HF_TWAI_ALERT_NONE = 0x00000000,
  HF_TWAI_ALERT_RX_DATA = 0x00000004,
  HF_TWAI_ALERT_BUS_ERROR = 0x00002000,
  HF_TWAI_ALERT_ALL = 0xFFFFFFFF,
};

using hf_can_controller_id_t = hf_twai_controller_id_t;
using hf_can_mode_t = hf_twai_mode_t;
using hf_can_error_state_t = hf_twai_error_state_t;
using hf_can_alert_t = hf_twai_alert_t;

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// MCU-SPECIFIC CAN CONSTANTS
//==============================================================================

static constexpr hf_gpio_num_t HF_CAN_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_CAN_MAX_DATA_LEN = 8;         ///< Classic CAN max data length
static constexpr uint32_t HF_CAN_STD_ID_MASK = 0x7FF;      ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_CAN_EXT_ID_MASK = 0x1FFFFFFF; ///< Extended ID mask (29-bit)

#ifdef HF_MCU_FAMILY_ESP32
static constexpr uint8_t HF_TWAI_MAX_CONTROLLERS = 2;      ///< ESP32C6 dual controllers
static constexpr uint32_t HF_TWAI_APB_CLOCK_HZ = 80000000; ///< ESP32C6 APB clock
static constexpr uint32_t HF_TWAI_MIN_BAUDRATE = 1000;     ///< Minimum baud rate
static constexpr uint32_t HF_TWAI_MAX_BAUDRATE = 1000000;  ///< Maximum baud rate
static constexpr uint32_t HF_TWAI_MIN_QUEUE_SIZE = 1;      ///< Minimum queue size
static constexpr uint32_t HF_TWAI_MAX_QUEUE_SIZE = 64;     ///< Maximum queue size
static constexpr uint32_t HF_TWAI_MAX_STD_ID = 0x7FF;      ///< Max standard ID
static constexpr uint32_t HF_TWAI_MAX_EXT_ID = 0x1FFFFFFF; ///< Max extended ID
#else
static constexpr uint8_t HF_TWAI_MAX_CONTROLLERS = 1;
static constexpr uint32_t HF_TWAI_APB_CLOCK_HZ = 80000000;
static constexpr uint32_t HF_TWAI_MIN_BAUDRATE = 1000;
static constexpr uint32_t HF_TWAI_MAX_BAUDRATE = 1000000;
static constexpr uint32_t HF_TWAI_MIN_QUEUE_SIZE = 1;
static constexpr uint32_t HF_TWAI_MAX_QUEUE_SIZE = 64;
static constexpr uint32_t HF_TWAI_MAX_STD_ID = 0x7FF;
static constexpr uint32_t HF_TWAI_MAX_EXT_ID = 0x1FFFFFFF;
#endif

//==============================================================================
// CAN/TWAI FUNCTION MACROS
//==============================================================================

/**
 * @brief MCU-specific CAN driver function pointers for ESP-IDF v5.5+ abstraction.
 * @details Function-like macros that map to the modern ESP-IDF TWAI node-based API.
 *          ESP32-C6 uses the new handle-based API introduced in ESP-IDF v5.5+.
 */
#ifdef HF_MCU_FAMILY_ESP32
// Modern ESP-IDF v5.5+ TWAI Node API function mappings 
#define HF_TWAI_NEW_NODE_ONCHIP(node_config, handle) twai_new_node_onchip(node_config, handle)
#define HF_TWAI_DEL_NODE(handle) twai_del_node(handle)
#define HF_TWAI_NODE_ENABLE(handle) twai_node_enable(handle)
#define HF_TWAI_NODE_DISABLE(handle) twai_node_disable(handle)
#define HF_TWAI_NODE_TRANSMIT(handle, message, timeout) twai_node_transmit(handle, message, timeout)
#define HF_TWAI_NODE_RECEIVE(handle, message, timeout) twai_node_receive(handle, message, timeout)
#define HF_TWAI_NODE_GET_STATUS(handle, status) twai_node_get_status(handle, status)

#else
// Non-ESP32 platforms - placeholder definitions
#define HF_TWAI_NEW_NODE_ONCHIP(node_config, handle) (-1)
#define HF_TWAI_DEL_NODE(handle) (-1)
#define HF_TWAI_NODE_ENABLE(handle) (-1)
#define HF_TWAI_NODE_DISABLE(handle) (-1)
#define HF_TWAI_NODE_TRANSMIT(handle, message, timeout) (-1)
#define HF_TWAI_NODE_RECEIVE(handle, message, timeout) (-1)
#define HF_TWAI_NODE_GET_STATUS(handle, status) (-1)

#endif

//==============================================================================
// CAN/TWAI VALIDATION MACROS
//==============================================================================

#define HF_TWAI_IS_VALID_CONTROLLER_ID(id) ((id) < HF_TWAI_MAX_CONTROLLERS)
#define HF_TWAI_IS_VALID_BAUDRATE(rate) \
  ((rate) >= HF_TWAI_MIN_BAUDRATE && (rate) <= HF_TWAI_MAX_BAUDRATE)
#define HF_TWAI_IS_VALID_QUEUE_SIZE(size) \
  ((size) >= HF_TWAI_MIN_QUEUE_SIZE && (size) <= HF_TWAI_MAX_QUEUE_SIZE)
#define HF_TWAI_IS_VALID_STD_ID(id) ((id) <= HF_TWAI_MAX_STD_ID)
#define HF_TWAI_IS_VALID_EXT_ID(id) ((id) <= HF_TWAI_MAX_EXT_ID)
#define HF_TWAI_IS_VALID_DLC(dlc) ((dlc) <= HF_CAN_MAX_DATA_LEN)

#define HF_TWAI_CALCULATE_BIT_TIME_NS(brp, tseg1, tseg2) \
  (((brp) * ((tseg1) + (tseg2) + 1) * 1000000000ULL) / HF_TWAI_APB_CLOCK_HZ)

#define HF_TWAI_CALCULATE_BAUDRATE(brp, tseg1, tseg2) \
  (HF_TWAI_APB_CLOCK_HZ / ((brp) * ((tseg1) + (tseg2) + 1)))

#define HF_TWAI_CALCULATE_SAMPLE_POINT_PERCENT(tseg1, tseg2) \
  (((tseg1) + 1) * 100 / ((tseg1) + (tseg2) + 1))
