/**
 * @file McuTypes_CAN.h
 * @brief MCU-specific CAN type definitions for hardware abstraction.
 *
 * This header defines all CAN-specific types and constants used throughout
 * the internal interface wrap layer for CAN operations. On ESP32 platforms,
 * this abstracts TWAI (Two-Wire Automotive Interface) to standard CAN naming 
 * for platform independence. Supports ESP32C6 dual controller and ESP-IDF v5.4+ features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"
#include "BaseCan.h" // For HfCanErr

//==============================================================================
// PLATFORM-SPECIFIC DRIVER IMPORTS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 uses TWAI (Two-Wire Automotive Interface) which is CAN compatible
// Import ESP-IDF TWAI driver headers
#include "driver/twai.h"
#include "hal/twai_types.h"
#include "soc/twai_periph.h"

// Clock source support
#include "soc/clk_tree_defs.h"

// For ESP32C6 with multiple controllers
#ifdef HF_MCU_ESP32C6
// ESP32C6 supports 2 TWAI controllers via standard driver/twai.h _v2 API
#endif

#else
// Non-ESP32 platforms - placeholder for future CAN controller support
// #include "platform_can_driver.h"
#endif

//==============================================================================
// HF CAN BASIC ERROR CONSTANTS
//==============================================================================

/**
 * @brief Basic CAN error constants for low-level compatibility.
 * @note Full error handling is provided by HfCanErr enum in BaseCan.h
 */
static constexpr int32_t HF_CAN_OK = 0;                 ///< Success
static constexpr int32_t HF_CAN_ERR_NOT_SUPPORTED = -1; ///< Operation not supported

//==============================================================================
// PLATFORM-SPECIFIC NATIVE TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 native TWAI type mappings (internal use only)
using hf_can_handle_native_t = twai_handle_t;
using hf_can_general_config_native_t = twai_general_config_t;
using hf_can_timing_config_native_t = twai_timing_config_t;
using hf_can_filter_config_native_t = twai_filter_config_t;
using hf_can_message_native_t = twai_message_t;
using hf_can_status_info_native_t = twai_status_info_t;
using hf_can_clock_source_native_t = twai_clock_source_t;

#else
// Non-ESP32 platforms - use generic types
using hf_can_handle_native_t = void *;
using hf_can_general_config_native_t = struct { int dummy; };
using hf_can_timing_config_native_t = struct { int dummy; };
using hf_can_filter_config_native_t = struct { int dummy; };
using hf_can_message_native_t = struct { int dummy; };
using hf_can_status_info_native_t = struct { int dummy; };
using hf_can_clock_source_native_t = uint32_t;
#endif

//==============================================================================
// HF NATIVE ENUM MAPPINGS FROM ESP-IDF TWAI TYPES
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief HF CAN controller ID mapping from ESP-IDF TWAI.
 * @details ESP32C6 has 2 TWAI controllers, ESP32 has 1 TWAI controller.
 */
enum class hf_can_controller_id_t : uint8_t {
  HF_CAN_CONTROLLER_0 = 0,   ///< Primary CAN controller (TWAI_CONTROLLER_0)
  HF_CAN_CONTROLLER_1 = 1,   ///< Secondary CAN controller (ESP32C6 specific)
  HF_CAN_CONTROLLER_MAX = 2, ///< Maximum number of controllers
};

/**
 * @brief HF CAN operating mode mapping from ESP-IDF twai_mode_t.
 * @details Maps directly to ESP-IDF TWAI controller operating modes.
 */
enum class hf_can_mode_t : uint8_t {
  HF_CAN_MODE_NORMAL = TWAI_MODE_NORMAL,           ///< Normal operating mode
  HF_CAN_MODE_NO_ACK = TWAI_MODE_NO_ACK,           ///< No acknowledgment mode (self-test)
  HF_CAN_MODE_LISTEN_ONLY = TWAI_MODE_LISTEN_ONLY, ///< Listen-only mode (bus monitor)
};

/**
 * @brief HF CAN driver state mapping from ESP-IDF twai_state_t.
 * @details Maps directly to ESP-IDF TWAI driver states for lifecycle management.
 */
enum class hf_can_state_t : uint8_t {
  HF_CAN_STATE_STOPPED = TWAI_STATE_STOPPED,       ///< Driver stopped
  HF_CAN_STATE_RUNNING = TWAI_STATE_RUNNING,       ///< Driver running, can TX/RX
  HF_CAN_STATE_BUS_OFF = TWAI_STATE_BUS_OFF,       ///< Bus-off state, needs recovery
  HF_CAN_STATE_RECOVERING = TWAI_STATE_RECOVERING, ///< Recovery in progress
};

/**
 * @brief HF CAN error state mapping based on error counters.
 * @details Error states based on TX/RX error counter values per CAN specification.
 */
enum class hf_can_error_state_t : uint8_t {
  HF_CAN_ERROR_ACTIVE = 0,     ///< Error active: TEC/REC < 96
  HF_CAN_ERROR_WARNING = 1,    ///< Error warning: TEC/REC >= 96 and < 128
  HF_CAN_ERROR_PASSIVE = 2,    ///< Error passive: TEC/REC >= 128 and < 256
  HF_CAN_ERROR_BUS_OFF = 3,    ///< Bus-off: TEC >= 256 (node offline)
  HF_CAN_ERROR_RECOVERING = 4, ///< Recovery in progress
};

/**
 * @brief HF CAN alert flags mapping from ESP-IDF TWAI alerts.
 * @details Comprehensive alert system mapping directly to ESP-IDF TWAI alert flags.
 */
enum class hf_can_alert_t : uint32_t {
  HF_CAN_ALERT_NONE = 0x00000000, ///< No alerts
  
  // Basic operation alerts (ESP-IDF native alerts)
  HF_CAN_ALERT_TX_IDLE = TWAI_ALERT_TX_IDLE,             ///< No more messages queued for TX
  HF_CAN_ALERT_TX_SUCCESS = TWAI_ALERT_TX_SUCCESS,       ///< Previous transmission successful
  HF_CAN_ALERT_RX_DATA = TWAI_ALERT_RX_DATA,             ///< Frame received and added to RX queue
  HF_CAN_ALERT_TX_FAILED = TWAI_ALERT_TX_FAILED,         ///< Previous transmission failed
  
  // Error state alerts (ESP-IDF native alerts)
  HF_CAN_ALERT_ERR_ACTIVE = TWAI_ALERT_ERR_ACTIVE,       ///< Controller became error-active
  HF_CAN_ALERT_ABOVE_ERR_WARN = TWAI_ALERT_ABOVE_ERR_WARN, ///< Error counter exceeded warning limit
  HF_CAN_ALERT_BELOW_ERR_WARN = TWAI_ALERT_BELOW_ERR_WARN, ///< Error counter below warning limit
  HF_CAN_ALERT_ERR_PASS = TWAI_ALERT_ERR_PASS,           ///< Controller became error-passive
  HF_CAN_ALERT_BUS_OFF = TWAI_ALERT_BUS_OFF,             ///< Bus-off condition occurred
  HF_CAN_ALERT_BUS_RECOVERED = TWAI_ALERT_BUS_RECOVERED, ///< Bus recovery completed
  
  // Hardware and bus error alerts (ESP-IDF native alerts)
  HF_CAN_ALERT_ARB_LOST = TWAI_ALERT_ARB_LOST,           ///< Previous transmission lost arbitration
  HF_CAN_ALERT_BUS_ERROR = TWAI_ALERT_BUS_ERROR,         ///< Bus error occurred
  HF_CAN_ALERT_RX_QUEUE_FULL = TWAI_ALERT_RX_QUEUE_FULL, ///< RX queue full, frame lost
  HF_CAN_ALERT_RECOVERY_IN_PROGRESS = TWAI_ALERT_RECOVERY_IN_PROGRESS, ///< Recovery in progress
  
  // Convenience alert combinations
  HF_CAN_ALERT_ALL_ERRORS = (TWAI_ALERT_TX_FAILED | TWAI_ALERT_ERR_PASS | 
                             TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_ERROR |
                             TWAI_ALERT_ARB_LOST | TWAI_ALERT_RX_QUEUE_FULL), ///< All error alerts
  HF_CAN_ALERT_ALL = 0xFFFFFFFF, ///< All alerts enabled
};

/**
 * @brief HF CAN clock source mapping from ESP-IDF twai_clock_source_t.
 * @details Maps to ESP-IDF TWAI clock sources. According to ESP-IDF v5.4.2 docs,
 *          ESP32C6 TWAI only supports XTAL clock source (40MHz).
 */
enum class hf_can_clock_source_t : uint8_t {
  HF_CAN_CLK_SRC_DEFAULT = 0, ///< Default clock source (XTAL for ESP32C6)
  HF_CAN_CLK_SRC_XTAL = 1,    ///< XTAL clock source (40MHz for ESP32C6)
};

#else
// Non-ESP32 platforms - simplified enums with generic values

enum class hf_can_controller_id_t : uint8_t {
  HF_CAN_CONTROLLER_0 = 0,
  HF_CAN_CONTROLLER_MAX = 1,
};

enum class hf_can_mode_t : uint8_t {
  HF_CAN_MODE_NORMAL = 0,
  HF_CAN_MODE_NO_ACK = 1,
  HF_CAN_MODE_LISTEN_ONLY = 2,
};

enum class hf_can_state_t : uint8_t {
  HF_CAN_STATE_STOPPED = 0,
  HF_CAN_STATE_RUNNING = 1,
  HF_CAN_STATE_BUS_OFF = 2,
  HF_CAN_STATE_RECOVERING = 3,
};

enum class hf_can_error_state_t : uint8_t {
  HF_CAN_ERROR_ACTIVE = 0,
  HF_CAN_ERROR_WARNING = 1,
  HF_CAN_ERROR_PASSIVE = 2,
  HF_CAN_ERROR_BUS_OFF = 3,
  HF_CAN_ERROR_RECOVERING = 4,
};

enum class hf_can_alert_t : uint32_t {
  HF_CAN_ALERT_NONE = 0x00000000,
  HF_CAN_ALERT_TX_IDLE = 0x00000001,
  HF_CAN_ALERT_TX_SUCCESS = 0x00000002,
  HF_CAN_ALERT_RX_DATA = 0x00000004,
  HF_CAN_ALERT_TX_FAILED = 0x00000008,
  HF_CAN_ALERT_ERR_ACTIVE = 0x00000010,
  HF_CAN_ALERT_ABOVE_ERR_WARN = 0x00000020,
  HF_CAN_ALERT_BELOW_ERR_WARN = 0x00000040,
  HF_CAN_ALERT_ERR_PASS = 0x00000080,
  HF_CAN_ALERT_BUS_OFF = 0x00000100,
  HF_CAN_ALERT_BUS_RECOVERED = 0x00000200,
  HF_CAN_ALERT_ARB_LOST = 0x00000400,
  HF_CAN_ALERT_BUS_ERROR = 0x00000800,
  HF_CAN_ALERT_RX_QUEUE_FULL = 0x00001000,
  HF_CAN_ALERT_RECOVERY_IN_PROGRESS = 0x00002000,
  HF_CAN_ALERT_ALL_ERRORS = 0x00001F88,
  HF_CAN_ALERT_ALL = 0xFFFFFFFF,
};

enum class hf_can_clock_source_t : uint8_t {
  HF_CAN_CLK_SRC_DEFAULT = 0,
  HF_CAN_CLK_SRC_XTAL = 1,
};

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// PLATFORM-AGNOSTIC CAN CONFIGURATION ENUMS
//==============================================================================

/**
 * @brief Platform-agnostic CAN transmission strategy.
 * @details Defines how messages should be transmitted and retried.
 */
enum class hf_can_transmission_strategy_t : uint8_t {
  HF_CAN_TX_NORMAL = 0,        ///< Normal transmission with retries
  HF_CAN_TX_SINGLE_SHOT = 1,   ///< Single shot (no retries)
  HF_CAN_TX_PRIORITY_HIGH = 2, ///< High priority transmission
  HF_CAN_TX_BACKGROUND = 3,    ///< Background/low priority transmission
};

/**
 * @brief Platform-agnostic CAN frame format.
 * @details Standard vs Extended ID format.
 */
enum class hf_can_frame_format_t : uint8_t {
  HF_CAN_FRAME_STANDARD = 0, ///< Standard 11-bit ID
  HF_CAN_FRAME_EXTENDED = 1, ///< Extended 29-bit ID
};

/**
 * @brief Platform-agnostic CAN frame type.
 * @details Data frame vs Remote Transmission Request.
 */
enum class hf_can_frame_type_t : uint8_t {
  HF_CAN_FRAME_DATA = 0, ///< Data frame
  HF_CAN_FRAME_RTR = 1,  ///< Remote Transmission Request
};

/**
 * @brief Platform-agnostic CAN power mode.
 * @details Power management states for CAN controllers.
 */
enum class hf_can_power_mode_t : uint8_t {
  HF_CAN_POWER_ACTIVE = 0,   ///< Full power, normal operation
  HF_CAN_POWER_SLEEP = 1,    ///< Sleep mode, wake on activity
  HF_CAN_POWER_STANDBY = 2,  ///< Standby mode, reduced power
  HF_CAN_POWER_OFF = 3,      ///< Power off, maximum power saving
};

//==============================================================================
// HF CAN HANDLE AND BASIC TYPES
//==============================================================================

/**
 * @brief Platform-agnostic CAN handle type.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_can_handle_t = twai_handle_t;
#else
using hf_can_handle_t = void *;
#endif

/**
 * @brief Platform-agnostic CAN error code (for compatibility).
 * @note This is a simple integer wrapper for platform-specific error codes.
 *       Full error handling is provided by HfCanErr enum in BaseCan.h
 */
using hf_can_err_t = esp_err_t;

/**
 * @brief Alert config structure for CAN alerts.
 */
struct hf_can_alert_config_t {
  uint32_t enabled_alerts;      ///< Bitmask of enabled alerts
  uint32_t alert_queue_size;    ///< Alert queue size
  uint32_t alert_timeout_ms;    ///< Alert timeout in milliseconds

  hf_can_alert_config_t() noexcept
      : enabled_alerts(static_cast<uint32_t>(hf_can_alert_t::HF_CAN_ALERT_ALL_ERRORS)),
        alert_queue_size(10), alert_timeout_ms(1000) {}
};

/**
 * @brief Power management config structure for CAN.
 */
struct hf_can_power_config_t {
  hf_can_power_mode_t power_mode;    ///< Current power mode
  bool auto_power_management;        ///< Enable automatic power management
  uint32_t idle_timeout_ms;          ///< Idle timeout before power saving
  uint32_t wakeup_timeout_ms;        ///< Wakeup timeout

  hf_can_power_config_t() noexcept
      : power_mode(hf_can_power_mode_t::HF_CAN_POWER_ACTIVE), auto_power_management(false),
        idle_timeout_ms(5000), wakeup_timeout_ms(100) {}
};

//==============================================================================
// HF CAN CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief Platform-agnostic CAN timing configuration.
 * @details Bit timing parameters for different baud rates.
 */
struct hf_can_timing_config_t {
  uint32_t brp;                  ///< Baud rate prescaler (1-16384)
  uint8_t tseg_1;                ///< Time segment 1 (1-16)
  uint8_t tseg_2;                ///< Time segment 2 (1-8)
  uint8_t sjw;                   ///< Synchronization jump width (1-4)
  bool triple_sampling;          ///< Enable triple sampling for noise immunity
  uint32_t quanta_resolution_hz; ///< Time quantum resolution (0 = auto)
  
  // Calculated values (read-only)
  uint32_t nominal_baudrate;     ///< Calculated nominal baudrate
  uint32_t actual_baudrate;      ///< Actual achieved baudrate
  float baudrate_accuracy;       ///< Baudrate accuracy percentage
  uint32_t bit_time_ns;          ///< Total bit time in nanoseconds
  uint32_t sample_point_percent; ///< Sample point percentage (60-90%)

  hf_can_timing_config_t() noexcept
      : brp(8), tseg_1(15), tseg_2(4), sjw(3), triple_sampling(false), 
        quanta_resolution_hz(0), nominal_baudrate(0), actual_baudrate(0), 
        baudrate_accuracy(0.0f), bit_time_ns(0), sample_point_percent(87) {}
};

/**
 * @brief Platform-agnostic CAN general configuration.
 * @details Main configuration for CAN controller setup.
 */
struct hf_can_general_config_t {
  hf_can_controller_id_t controller_id; ///< Controller ID (0, 1, etc.)
  hf_can_mode_t mode;                   ///< Operating mode
  HfPinNumber tx_io;                    ///< TX GPIO pin number
  HfPinNumber rx_io;                    ///< RX GPIO pin number
  HfPinNumber clkout_io;                ///< Clock output GPIO (optional)
  HfPinNumber bus_off_io;               ///< Bus-off indicator GPIO (optional)

  // Queue configuration
  uint32_t tx_queue_len;     ///< TX queue length (1-64)
  uint32_t rx_queue_len;     ///< RX queue length (1-64)
  uint32_t alerts_enabled;   ///< Enabled alert flags bitmask
  uint32_t clkout_divider;   ///< Clock output divider (0 = disabled)
  uint32_t intr_flags;       ///< Interrupt allocation flags

  // Power and recovery settings
  bool sleep_retention_enable;     ///< Enable sleep retention
  bool auto_recovery_enable;       ///< Enable automatic bus-off recovery
  uint32_t recovery_timeout_ms;    ///< Recovery timeout in milliseconds
  bool power_management_enable;    ///< Enable power management
  bool clock_gating_enable;        ///< Enable clock gating for power saving
  uint32_t idle_timeout_ms;        ///< Idle timeout before power saving

  // Error thresholds
  uint32_t error_warning_limit;      ///< Error warning threshold (default: 96)
  uint32_t error_passive_limit;      ///< Error passive threshold (default: 128)
  uint32_t bus_off_recovery_time_ms; ///< Bus-off recovery time
  bool enable_advanced_diagnostics;  ///< Enable advanced error diagnostics

  hf_can_general_config_t() noexcept
      : controller_id(hf_can_controller_id_t::HF_CAN_CONTROLLER_0),
        mode(hf_can_mode_t::HF_CAN_MODE_NORMAL), tx_io(HF_INVALID_PIN), 
        rx_io(HF_INVALID_PIN), clkout_io(HF_INVALID_PIN), bus_off_io(HF_INVALID_PIN),
        tx_queue_len(10), rx_queue_len(10), 
        alerts_enabled(static_cast<uint32_t>(hf_can_alert_t::HF_CAN_ALERT_ALL_ERRORS)),
        clkout_divider(0), intr_flags(0), sleep_retention_enable(false),
        auto_recovery_enable(true), recovery_timeout_ms(2000), 
        power_management_enable(false), clock_gating_enable(false), 
        idle_timeout_ms(5000), error_warning_limit(96), error_passive_limit(128),
        bus_off_recovery_time_ms(1000), enable_advanced_diagnostics(true) {}
};

/**
 * @brief Platform-agnostic CAN filter configuration.
 * @details Hardware acceptance filter configuration.
 */
struct hf_can_filter_config_t {
  uint32_t acceptance_code;     ///< Acceptance code for filtering
  uint32_t acceptance_mask;     ///< Acceptance mask (0 = don't care)
  bool single_filter;           ///< Single vs dual filter mode

  // Extended filtering features
  uint32_t acceptance_code_ext; ///< Extended acceptance code (29-bit)
  uint32_t acceptance_mask_ext; ///< Extended acceptance mask (29-bit)
  bool enable_std_filter;       ///< Enable standard frame filtering
  bool enable_ext_filter;       ///< Enable extended frame filtering
  bool enable_rtr_filter;       ///< Enable RTR frame filtering

  hf_can_filter_config_t() noexcept
      : acceptance_code(0), acceptance_mask(0xFFFFFFFF), single_filter(true),
        acceptance_code_ext(0), acceptance_mask_ext(0x1FFFFFFF), 
        enable_std_filter(true), enable_ext_filter(true), enable_rtr_filter(true) {}
};

/**
 * @brief Platform-agnostic CAN status information.
 * @details Comprehensive status structure with diagnostics and performance metrics.
 * @note For CAN message structures, use CanMessage defined in BaseCan.h
 */
struct hf_can_status_info_t {
  hf_can_state_t state;          ///< Current driver state
  hf_can_error_state_t error_state; ///< Current error state

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

  // Error statistics
  uint32_t arbitration_lost_count; ///< Arbitration lost count
  uint32_t bus_error_count;        ///< Bus error count
  uint32_t stuff_error_count;      ///< Bit stuffing error count
  uint32_t form_error_count;       ///< Frame format error count
  uint32_t crc_error_count;        ///< CRC error count
  uint32_t ack_error_count;        ///< Acknowledgment error count

  // Performance metrics
  uint32_t messages_transmitted; ///< Total messages transmitted
  uint32_t messages_received;    ///< Total messages received
  uint32_t bytes_transmitted;    ///< Total bytes transmitted
  uint32_t bytes_received;       ///< Total bytes received
  uint64_t bus_uptime_us;        ///< Bus uptime in microseconds

  // Real-time bus conditions
  uint8_t bus_load_percent; ///< Current bus load percentage
  float bit_error_rate;     ///< Bit error rate (errors/bits)

  hf_can_status_info_t() noexcept
      : state(hf_can_state_t::HF_CAN_STATE_STOPPED),
        error_state(hf_can_error_state_t::HF_CAN_ERROR_ACTIVE),
        tx_error_counter(0), rx_error_counter(0), tx_failed_count(0), 
        rx_missed_count(0), rx_queue_len(0), tx_queue_len(0), rx_queue_peak(0), 
        tx_queue_peak(0), arbitration_lost_count(0), bus_error_count(0), 
        stuff_error_count(0), form_error_count(0), crc_error_count(0), 
        ack_error_count(0), messages_transmitted(0), messages_received(0), 
        bytes_transmitted(0), bytes_received(0), bus_uptime_us(0), 
        bus_load_percent(0), bit_error_rate(0.0f) {}
};

/**
 * @brief Platform-agnostic CAN capabilities structure.
 * @details Static capability information for runtime feature detection.
 */
struct hf_can_capabilities_t {
  uint8_t num_controllers;        ///< Number of CAN controllers
  uint8_t max_tx_queue_size;      ///< Maximum TX queue size
  uint8_t max_rx_queue_size;      ///< Maximum RX queue size
  uint32_t max_baudrate;          ///< Maximum supported baudrate
  uint32_t min_baudrate;          ///< Minimum supported baudrate
  bool supports_canfd;            ///< CAN-FD support
  bool supports_sleep_retention;  ///< Sleep retention support
  bool supports_dual_controllers; ///< Dual controller support
  bool supports_advanced_filters; ///< Advanced filtering support
  bool supports_power_management; ///< Power management support
  uint8_t num_hardware_filters;   ///< Number of hardware filters

  // Constructor with platform-specific defaults
  hf_can_capabilities_t() noexcept
#ifdef HF_MCU_ESP32C6
      : num_controllers(2), max_tx_queue_size(64), max_rx_queue_size(64), 
        max_baudrate(1000000), min_baudrate(1000), supports_canfd(false), 
        supports_sleep_retention(true), supports_dual_controllers(true),
        supports_advanced_filters(true), supports_power_management(true), 
        num_hardware_filters(2) {}
#elif defined(HF_MCU_ESP32)
      : num_controllers(1), max_tx_queue_size(64), max_rx_queue_size(64), 
        max_baudrate(1000000), min_baudrate(1000), supports_canfd(false), 
        supports_sleep_retention(false), supports_dual_controllers(false),
        supports_advanced_filters(true), supports_power_management(false), 
        num_hardware_filters(2) {}
#else
      : num_controllers(1), max_tx_queue_size(32), max_rx_queue_size(32), 
        max_baudrate(1000000), min_baudrate(1000), supports_canfd(false), 
        supports_sleep_retention(false), supports_dual_controllers(false),
        supports_advanced_filters(false), supports_power_management(false), 
        num_hardware_filters(1) {}
#endif
};

/**
 * @brief Platform-agnostic CAN performance statistics.
 * @details Comprehensive runtime statistics for monitoring and debugging.
 */
struct hf_can_statistics_t {
  // Message counters
  uint64_t messages_sent;         ///< Total messages successfully sent
  uint64_t messages_received;     ///< Total messages successfully received
  uint64_t bytes_transmitted;     ///< Total bytes transmitted
  uint64_t bytes_received;        ///< Total bytes received
  
  // Error counters
  uint32_t send_failures;         ///< Failed send operations
  uint32_t receive_failures;      ///< Failed receive operations
  uint32_t bus_error_count;       ///< Total bus errors
  uint32_t arbitration_lost_count;///< Arbitration lost events
  uint32_t tx_failed_count;       ///< Transmission failures
  uint32_t bus_off_events;        ///< Bus-off occurrences
  
  // Performance metrics
  uint64_t uptime_seconds;        ///< Total uptime in seconds
  uint32_t last_activity_timestamp; ///< Last activity timestamp
  HfCanErr last_error;            ///< Last error encountered
  
  // Queue statistics
  uint32_t tx_queue_peak;         ///< Peak TX queue usage
  uint32_t rx_queue_peak;         ///< Peak RX queue usage
  uint32_t tx_queue_overflows;    ///< TX queue overflow count
  uint32_t rx_queue_overflows;    ///< RX queue overflow count

  hf_can_statistics_t() noexcept
      : messages_sent(0), messages_received(0), bytes_transmitted(0), bytes_received(0),
        send_failures(0), receive_failures(0), bus_error_count(0), arbitration_lost_count(0),
        tx_failed_count(0), bus_off_events(0), uptime_seconds(0), last_activity_timestamp(0),
        last_error(HfCanErr::CAN_SUCCESS), tx_queue_peak(0), rx_queue_peak(0),
        tx_queue_overflows(0), rx_queue_overflows(0) {}
};

//==============================================================================
// HF CAN CONSTANTS AND LIMITS
//==============================================================================

// Basic CAN protocol constants
static constexpr HfPinNumber HF_CAN_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_CAN_MAX_DATA_LEN = 8;         ///< Classic CAN max data length
static constexpr uint32_t HF_CAN_STD_ID_MASK = 0x7FF;      ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_CAN_EXT_ID_MASK = 0x1FFFFFFF; ///< Extended ID mask (29-bit)
static constexpr uint32_t HF_CAN_MAX_STD_ID = 0x7FF;       ///< Maximum standard ID
static constexpr uint32_t HF_CAN_MAX_EXT_ID = 0x1FFFFFFF;  ///< Maximum extended ID

// Platform-specific constants
#ifdef HF_MCU_ESP32C6
static constexpr uint8_t HF_CAN_MAX_CONTROLLERS = 2;       ///< ESP32C6 has 2 TWAI controllers per docs
static constexpr uint32_t HF_CAN_APB_CLOCK_HZ = 40000000;  ///< ESP32C6 APB clock frequency
static constexpr uint32_t HF_CAN_MIN_BAUDRATE = 1000;      ///< Minimum baud rate
static constexpr uint32_t HF_CAN_MAX_BAUDRATE = 1000000;   ///< Maximum baud rate
static constexpr uint32_t HF_CAN_MIN_QUEUE_SIZE = 1;       ///< Minimum queue size
static constexpr uint32_t HF_CAN_MAX_QUEUE_SIZE = 64;      ///< Maximum queue size
static constexpr uint32_t HF_CAN_MIN_BRP = 2;              ///< Minimum BRP value (even numbers only)
static constexpr uint32_t HF_CAN_MAX_BRP = 32768;          ///< Maximum BRP value
static constexpr uint32_t HF_CAN_ERROR_WARNING_LIMIT = 96; ///< Default error warning threshold
static constexpr uint32_t HF_CAN_ERROR_PASSIVE_LIMIT = 128; ///< Error passive threshold
#elif defined(HF_MCU_ESP32)
static constexpr uint8_t HF_CAN_MAX_CONTROLLERS = 1;       ///< ESP32 has 1 TWAI controller
static constexpr uint32_t HF_CAN_APB_CLOCK_HZ = 80000000;  ///< ESP32 APB clock frequency
static constexpr uint32_t HF_CAN_MIN_BAUDRATE = 1000;      ///< Minimum baud rate
static constexpr uint32_t HF_CAN_MAX_BAUDRATE = 1000000;   ///< Maximum baud rate
static constexpr uint32_t HF_CAN_MIN_QUEUE_SIZE = 1;       ///< Minimum queue size
static constexpr uint32_t HF_CAN_MAX_QUEUE_SIZE = 64;      ///< Maximum queue size
static constexpr uint32_t HF_CAN_MIN_BRP = 2;              ///< Minimum BRP value (even numbers only)
static constexpr uint32_t HF_CAN_MAX_BRP = 16384;          ///< Maximum BRP value (ESP32 limit, even up to 256)
static constexpr uint32_t HF_CAN_ERROR_WARNING_LIMIT = 96; ///< Default error warning threshold
static constexpr uint32_t HF_CAN_ERROR_PASSIVE_LIMIT = 128; ///< Error passive threshold
#else
static constexpr uint8_t HF_CAN_MAX_CONTROLLERS = 1;
static constexpr uint32_t HF_CAN_APB_CLOCK_HZ = 80000000;
static constexpr uint32_t HF_CAN_MIN_BAUDRATE = 1000;
static constexpr uint32_t HF_CAN_MAX_BAUDRATE = 1000000;
static constexpr uint32_t HF_CAN_MIN_QUEUE_SIZE = 1;
static constexpr uint32_t HF_CAN_MAX_QUEUE_SIZE = 32;
static constexpr uint32_t HF_CAN_MIN_BRP = 1;
static constexpr uint32_t HF_CAN_MAX_BRP = 64;
#endif

//==============================================================================
// HF CAN TIMING CONFIGURATION PRESETS
//==============================================================================

/**
 * @brief Predefined timing configurations for common baudrates.
 * @details Based on ESP-IDF TWAI timing macros, optimized for ESP32C6 40MHz APB clock.
 */

// Common baudrate timing configurations for ESP32C6 (40MHz APB)
#ifdef HF_MCU_ESP32C6
#define HF_CAN_TIMING_CONFIG_1MBITS()   {2, 15, 4, 3, false, 0, 1000000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_800KBITS() {2, 20, 4, 3, false, 0, 800000, 0, 0.0f, 0, 84}  
#define HF_CAN_TIMING_CONFIG_500KBITS() {4, 15, 4, 3, false, 0, 500000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_250KBITS() {8, 15, 4, 3, false, 0, 250000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_125KBITS() {16, 15, 4, 3, false, 0, 125000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_100KBITS() {20, 15, 4, 3, false, 0, 100000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_50KBITS()  {40, 15, 4, 3, false, 0, 50000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_25KBITS()  {80, 15, 4, 3, false, 0, 25000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_20KBITS()  {100, 15, 4, 3, false, 0, 20000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_10KBITS()  {200, 15, 4, 3, false, 0, 10000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_5KBITS()   {400, 15, 4, 3, false, 0, 5000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_1KBITS()   {2000, 15, 4, 3, false, 0, 1000, 0, 0.0f, 0, 80}

// ESP32 timing configurations (80MHz APB)
#elif defined(HF_MCU_ESP32)
#define HF_CAN_TIMING_CONFIG_1MBITS()   {4, 15, 4, 3, false, 0, 1000000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_800KBITS() {5, 15, 4, 3, false, 0, 800000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_500KBITS() {8, 15, 4, 3, false, 0, 500000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_250KBITS() {16, 15, 4, 3, false, 0, 250000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_125KBITS() {32, 15, 4, 3, false, 0, 125000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_100KBITS() {40, 15, 4, 3, false, 0, 100000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_50KBITS()  {80, 15, 4, 3, false, 0, 50000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_25KBITS()  {128, 15, 4, 3, false, 0, 25000, 0, 0.0f, 0, 80}

// Generic timing configurations
#else
#define HF_CAN_TIMING_CONFIG_500KBITS() {8, 15, 4, 3, false, 0, 500000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_250KBITS() {16, 15, 4, 3, false, 0, 250000, 0, 0.0f, 0, 80}
#define HF_CAN_TIMING_CONFIG_125KBITS() {32, 15, 4, 3, false, 0, 125000, 0, 0.0f, 0, 80}
#endif

//==============================================================================
// HF CAN FUNCTION MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// Modern ESP-IDF v5.4.2+ TWAI driver API mappings (handle-based v2 API)
#define HF_CAN_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) \
  twai_driver_install_v2(gconfig, tconfig, fconfig, handle)
#define HF_CAN_DRIVER_UNINSTALL_V2(handle) twai_driver_uninstall_v2(handle)
#define HF_CAN_START_V2(handle) twai_start_v2(handle)
#define HF_CAN_STOP_V2(handle) twai_stop_v2(handle)
#define HF_CAN_TRANSMIT_V2(handle, message, ticks) twai_transmit_v2(handle, message, ticks)
#define HF_CAN_RECEIVE_V2(handle, message, ticks) twai_receive_v2(handle, message, ticks)
#define HF_CAN_GET_STATUS_INFO_V2(handle, status) twai_get_status_info_v2(handle, status)
#define HF_CAN_CLEAR_TRANSMIT_QUEUE_V2(handle) twai_clear_transmit_queue_v2(handle)
#define HF_CAN_CLEAR_RECEIVE_QUEUE_V2(handle) twai_clear_receive_queue_v2(handle)
#define HF_CAN_RECONFIGURE_ALERTS_V2(handle, alerts, prev) \
  twai_reconfigure_alerts_v2(handle, alerts, prev)
#define HF_CAN_READ_ALERTS_V2(handle, alerts, ticks) twai_read_alerts_v2(handle, alerts, ticks)
#define HF_CAN_INITIATE_RECOVERY_V2(handle) twai_initiate_recovery_v2(handle)

// Legacy ESP-IDF TWAI driver API mappings (handle-free v1 API)
#define HF_CAN_DRIVER_INSTALL(gconfig, tconfig, fconfig) \
  twai_driver_install(gconfig, tconfig, fconfig)
#define HF_CAN_DRIVER_UNINSTALL() twai_driver_uninstall()
#define HF_CAN_START() twai_start()
#define HF_CAN_STOP() twai_stop()
#define HF_CAN_TRANSMIT(message, ticks) twai_transmit(message, ticks)
#define HF_CAN_RECEIVE(message, ticks) twai_receive(message, ticks)
#define HF_CAN_GET_STATUS_INFO(status) twai_get_status_info(status)
#define HF_CAN_RECONFIGURE_ALERTS(alerts, prev) twai_reconfigure_alerts(alerts, prev)
#define HF_CAN_READ_ALERTS(alerts, ticks) twai_read_alerts(alerts, ticks)
#define HF_CAN_INITIATE_RECOVERY() twai_initiate_recovery()

#else
// Non-ESP32 platforms - placeholder definitions
#define HF_CAN_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_DRIVER_UNINSTALL_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_START_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_STOP_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_TRANSMIT_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECEIVE_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_GET_STATUS_INFO_V2(handle, status) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_CLEAR_TRANSMIT_QUEUE_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_CLEAR_RECEIVE_QUEUE_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECONFIGURE_ALERTS_V2(handle, alerts, prev) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_READ_ALERTS_V2(handle, alerts, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_INITIATE_RECOVERY_V2(handle) HF_CAN_ERR_NOT_SUPPORTED

#define HF_CAN_DRIVER_INSTALL(gconfig, tconfig, fconfig) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_DRIVER_UNINSTALL() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_START() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_STOP() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_TRANSMIT(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECEIVE(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_GET_STATUS_INFO(status) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECONFIGURE_ALERTS(alerts, prev) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_READ_ALERTS(alerts, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_INITIATE_RECOVERY() HF_CAN_ERR_NOT_SUPPORTED
#endif

//==============================================================================
// HF CAN UTILITY MACROS
//==============================================================================

/**
 * @brief CAN timing calculation macros.
 */
#define HF_CAN_CALCULATE_BIT_TIME_NS(brp, tseg1, tseg2) \
  (((brp) * ((tseg1) + (tseg2) + 1) * 1000000000ULL) / HF_CAN_APB_CLOCK_HZ)

#define HF_CAN_CALCULATE_BAUDRATE(brp, tseg1, tseg2) \
  (HF_CAN_APB_CLOCK_HZ / ((brp) * ((tseg1) + (tseg2) + 1)))

#define HF_CAN_CALCULATE_SAMPLE_POINT_PERCENT(tseg1, tseg2) \
  (((tseg1) + 1) * 100 / ((tseg1) + (tseg2) + 1))

/**
 * @brief CAN validation macros.
 */
#define HF_CAN_IS_VALID_CONTROLLER_ID(id) ((id) < HF_CAN_MAX_CONTROLLERS)
#define HF_CAN_IS_VALID_BAUDRATE(rate) \
  ((rate) >= HF_CAN_MIN_BAUDRATE && (rate) <= HF_CAN_MAX_BAUDRATE)
#define HF_CAN_IS_VALID_QUEUE_SIZE(size) \
  ((size) >= HF_CAN_MIN_QUEUE_SIZE && (size) <= HF_CAN_MAX_QUEUE_SIZE)
#define HF_CAN_IS_VALID_STD_ID(id) ((id) <= HF_CAN_MAX_STD_ID)
#define HF_CAN_IS_VALID_EXT_ID(id) ((id) <= HF_CAN_MAX_EXT_ID)
#define HF_CAN_IS_VALID_DLC(dlc) ((dlc) <= HF_CAN_MAX_DATA_LEN)
#define HF_CAN_IS_VALID_BRP(brp) ((brp) >= HF_CAN_MIN_BRP && (brp) <= HF_CAN_MAX_BRP)

/**
 * @brief CAN message flag manipulation macros.
 * @note These macros work with the CanMessage structure defined in BaseCan.h
 */
#define HF_CAN_MESSAGE_SET_EXTENDED(msg) ((msg).is_extended = true)
#define HF_CAN_MESSAGE_SET_STANDARD(msg) ((msg).is_extended = false)
#define HF_CAN_MESSAGE_SET_RTR(msg) ((msg).is_rtr = true)
#define HF_CAN_MESSAGE_SET_DATA(msg) ((msg).is_rtr = false)
#define HF_CAN_MESSAGE_SET_SINGLE_SHOT(msg) ((msg).is_ss = true)
#define HF_CAN_MESSAGE_SET_SELF_RX(msg) ((msg).is_self = true)

/**
 * @brief CAN message validation macros.
 * @note These macros work with the CanMessage structure defined in BaseCan.h
 */
#define HF_CAN_MESSAGE_IS_VALID_ID(msg) ((msg).IsValidId())
#define HF_CAN_MESSAGE_IS_VALID_DLC(msg) ((msg).IsValidDLC((msg).GetEffectiveDLC()))
#define HF_CAN_MESSAGE_IS_EXTENDED(msg) ((msg).is_extended)
#define HF_CAN_MESSAGE_IS_RTR(msg) ((msg).is_rtr)
#define HF_CAN_MESSAGE_IS_CANFD(msg) ((msg).is_canfd)

/**
 * @brief CAN-FD specific macros.
 * @note These macros work with the CanMessage structure defined in BaseCan.h
 */
#define HF_CAN_MESSAGE_SET_CANFD(msg) ((msg).is_canfd = true)
#define HF_CAN_MESSAGE_SET_CLASSIC(msg) ((msg).is_canfd = false)
#define HF_CAN_MESSAGE_SET_BRS(msg) ((msg).is_brs = true)
#define HF_CAN_MESSAGE_SET_ESI(msg) ((msg).is_esi = true)

/**
 * @brief CAN filter configuration macros.
 */
#define HF_CAN_FILTER_ACCEPT_ALL() {0, 0xFFFFFFFF, true, 0, 0x1FFFFFFF, true, true, true}
#define HF_CAN_FILTER_ACCEPT_NONE() {0xFFFFFFFF, 0, true, 0xFFFFFFFF, 0, true, true, true}
#define HF_CAN_FILTER_ACCEPT_STD_ONLY() {0, 0xFFFFFFFF, true, 0xFFFFFFFF, 0, true, false, true}
#define HF_CAN_FILTER_ACCEPT_EXT_ONLY() {0xFFFFFFFF, 0, true, 0, 0x1FFFFFFF, false, true, true}

//==============================================================================
// END OF HF CAN TYPES HEADER
//==============================================================================
