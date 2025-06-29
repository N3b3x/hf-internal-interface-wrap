/**
 * @file McuTypes.h
 * @brief MCU-specific type definitions for hardware abstraction (hf_* types).
 *
 * This header defines all MCU-specific types and constants that are used
 * throughout the internal interface wrap layer. By centralizing these definitions,
 * porting to new MCUs only requires updating this single file. The types provide
 * platform-specific configurations while maintaining interface compatibility.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note All interface classes (CAN, UART, I2C, SPI, GPIO, ADC, PIO, PWM) must use only these types.
 */

#pragma once

#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF headers)
#include <cstdint>
#include <atomic>

//==============================================================================
// PLATFORM-SPECIFIC TYPE DEFINITIONS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32-specific type definitions - includes already handled by McuSelect.h
// Additional ESP-IDF includes for specific types only if needed
#ifdef __cplusplus
extern "C" {
#endif

// ESP32-specific includes and type definitions
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "driver/twai.h"           // ESP-IDF v5.5+ TWAI driver
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"        // For QueueHandle_t
#include "esp_timer.h"             // For esp_timer_handle_t

#ifdef __cplusplus
}
#endif

// ESP32 ADC specific mappings
using hf_adc_unit_native_t = adc_unit_t;
using hf_adc_channel_native_t = adc_channel_t;
using hf_adc_atten_native_t = adc_atten_t;
using hf_adc_bitwidth_native_t = adc_bitwidth_t;

// ESP32 GPIO specific mappings
using hf_gpio_num_native_t = gpio_num_t;
using hf_gpio_mode_native_t = gpio_mode_t;
using hf_gpio_pull_native_t = gpio_pull_mode_t;

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
using hf_adc_unit_native_t = uint8_t;
using hf_adc_channel_native_t = uint8_t;
using hf_adc_atten_native_t = uint8_t;
using hf_adc_bitwidth_native_t = uint8_t;
using hf_gpio_num_native_t = uint32_t;
using hf_gpio_mode_native_t = uint8_t;
using hf_gpio_pull_native_t = uint8_t;

// Generic CAN types for non-ESP32 platforms
using hf_twai_handle_native_t = void*;
using hf_twai_general_config_native_t = struct { int dummy; };
using hf_twai_timing_config_native_t = struct { int dummy; };
using hf_twai_filter_config_native_t = struct { int dummy; };
using hf_twai_message_native_t = struct { int dummy; };
using hf_twai_status_info_native_t = struct { int dummy; };
using hf_twai_mode_native_t = uint8_t;
using hf_twai_error_state_native_t = uint8_t;
using hf_esp_timer_handle_native_t = void*;

// Generic RTOS handle types for non-ESP32 platforms
using SemaphoreHandle_t = void*;
using QueueHandle_t = void*;
using i2c_config_t = struct { int dummy; };
using spi_bus_config_t = struct { int dummy; };
using spi_device_interface_config_t = struct { int dummy; };
using uart_config_t = struct { int dummy; };
#endif

//==============================================================================
// MCU-SPECIFIC TYPES (FOR INTERNAL MCU LAYER USE ONLY)
//==============================================================================

/**
 * @brief MCU-specific ADC resolution configuration.
 * @details Used internally by MCU implementations for platform-specific ADC setup.
 */
enum class hf_adc_resolution_t : uint8_t {
  HF_ADC_RES_9BIT = 9,
  HF_ADC_RES_10BIT = 10,
  HF_ADC_RES_11BIT = 11,
  HF_ADC_RES_12BIT = 12,
  HF_ADC_RES_13BIT = 13,
};

/**
 * @brief MCU-specific ADC attenuation configuration.
 * @details Used internally by MCU implementations for platform-specific ADC setup.
 */
enum class hf_adc_attenuation_t : uint8_t {
  HF_ADC_ATTEN_DB_0 = 0,   ///< No attenuation (1.1V max)
  HF_ADC_ATTEN_DB_2_5 = 1, ///< 2.5dB attenuation (1.5V max)
  HF_ADC_ATTEN_DB_6 = 2,   ///< 6dB attenuation (2.2V max)
  HF_ADC_ATTEN_DB_11 = 3,  ///< 11dB attenuation (3.9V max)
};

/**
 * @brief MCU-specific ADC unit identifier.
 * @details Used internally by MCU implementations for platform-specific ADC setup.
 */
enum class hf_adc_unit_t : uint8_t {
  HF_ADC_UNIT_1 = 1, ///< SAR ADC 1
  HF_ADC_UNIT_2 = 2, ///< SAR ADC 2
};

/**
 * @brief MCU-specific GPIO mode configuration.
 */
enum class hf_gpio_mode_t : uint8_t {
  HF_GPIO_MODE_INPUT = 0,
  HF_GPIO_MODE_OUTPUT,
  HF_GPIO_MODE_OUTPUT_OD,
};

/**
 * @brief MCU-specific GPIO pull resistor configuration.
 */
enum class hf_gpio_pull_t : uint8_t {
  HF_GPIO_PULL_NONE = 0,
  HF_GPIO_PULL_UP,
  HF_GPIO_PULL_DOWN,
};

/**
 * @brief MCU-specific GPIO interrupt trigger configuration.
 */
enum class hf_gpio_intr_type_t : uint8_t {
  HF_GPIO_INTR_DISABLE = 0,
  HF_GPIO_INTR_POSEDGE,
  HF_GPIO_INTR_NEGEDGE,
  HF_GPIO_INTR_ANYEDGE,
  HF_GPIO_INTR_LOW_LEVEL,
  HF_GPIO_INTR_HIGH_LEVEL,
};

/**
 * @brief MCU-specific GPIO drive capability.
 */
enum class hf_gpio_drive_cap_t : uint8_t {
  HF_GPIO_DRIVE_CAP_0 = 0, ///< Minimum drive capability (5mA)
  HF_GPIO_DRIVE_CAP_1,     ///< Medium drive capability (10mA)
  HF_GPIO_DRIVE_CAP_2,     ///< High drive capability (20mA)
  HF_GPIO_DRIVE_CAP_3,     ///< Maximum drive capability (40mA)
};

/**
 * @brief MCU-specific I2C mode configuration.
 */
enum class hf_i2c_mode_t : uint8_t {
  HF_I2C_MODE_SLAVE = 0,
  HF_I2C_MODE_MASTER,
};

/**
 * @brief MCU-specific SPI mode configuration.
 */
enum class hf_spi_mode_t : uint8_t {
  HF_SPI_MODE_0 = 0, ///< CPOL=0, CPHA=0
  HF_SPI_MODE_1 = 1, ///< CPOL=0, CPHA=1
  HF_SPI_MODE_2 = 2, ///< CPOL=1, CPHA=0
  HF_SPI_MODE_3 = 3, ///< CPOL=1, CPHA=1
};

/**
 * @brief MCU-specific UART data bits configuration.
 */
enum class hf_uart_data_bits_t : uint8_t {
  HF_UART_DATA_5_BITS = 0,
  HF_UART_DATA_6_BITS = 1,
  HF_UART_DATA_7_BITS = 2,
  HF_UART_DATA_8_BITS = 3,
};

/**
 * @brief MCU-specific UART parity configuration.
 */
enum class hf_uart_parity_t : uint8_t {
  HF_UART_PARITY_DISABLE = 0,
  HF_UART_PARITY_EVEN = 2,
  HF_UART_PARITY_ODD = 3,
};

/**
 * @brief MCU-specific UART stop bits configuration.
 */
enum class hf_uart_stop_bits_t : uint8_t {
  HF_UART_STOP_BITS_1 = 1,
  HF_UART_STOP_BITS_1_5 = 2,
  HF_UART_STOP_BITS_2 = 3,
};

/**
 * @brief MCU-specific UART flow control configuration.
 */
enum class hf_uart_flow_ctrl_t : uint8_t {
  HF_UART_HW_FLOWCTRL_DISABLE = 0,
  HF_UART_HW_FLOWCTRL_RTS = 1,
  HF_UART_HW_FLOWCTRL_CTS = 2,
  HF_UART_HW_FLOWCTRL_CTS_RTS = 3,
};

/**
 * @brief MCU-specific CAN mode configuration.
 */
enum class hf_can_mode_t : uint8_t {
  HF_CAN_MODE_NORMAL = 0,      ///< Normal mode with acknowledgment
  HF_CAN_MODE_NO_ACK = 1,      ///< No acknowledgment mode (self-test)
  HF_CAN_MODE_LISTEN_ONLY = 2, ///< Listen-only mode (monitoring)
  HF_CAN_MODE_LOOPBACK = 3,    ///< Loopback mode for testing
};

/**
 * @brief MCU-specific CAN error states for ESP32C6 TWAI controller.
 * @details Maps to ESP-IDF v5.5+ twai_error_state_t enum.
 */
enum class hf_can_error_state_t : uint8_t {
  HF_CAN_ERROR_ACTIVE = 0,  ///< Error active state: TEC/REC < 96
  HF_CAN_ERROR_WARNING = 1, ///< Error warning state: TEC/REC >= 96 and < 128
  HF_CAN_ERROR_PASSIVE = 2, ///< Error passive state: TEC/REC >= 128 and < 256
  HF_CAN_ERROR_BUS_OFF = 3, ///< Bus-off state: TEC >= 256 (node offline)
};

/**
 * @brief MCU-specific CAN return codes for ESP-IDF v5.5+ TWAI operations.
 * @details Used internally by MCU implementations for platform-specific error handling.
 */
enum class hf_can_err_t : uint8_t {
  HF_CAN_OK = 0,                  ///< Operation successful
  HF_CAN_ERR_INVALID_ARG = 1,     ///< Invalid argument
  HF_CAN_ERR_INVALID_STATE = 2,   ///< Invalid state
  HF_CAN_ERR_TIMEOUT = 3,         ///< Operation timed out
  HF_CAN_ERR_NOT_SUPPORTED = 4,   ///< Operation not supported
  HF_CAN_ERR_NO_MEM = 5,          ///< Out of memory
  HF_CAN_ERR_FAIL = 6,            ///< Generic failure
};

/**
 * @brief MCU-specific CAN controller identifier for ESP32C6 dual TWAI support.
 * @details ESP32C6 has 2 TWAI controllers that can be used independently.
 */
enum class hf_can_controller_id_t : uint8_t {
  HF_CAN_CONTROLLER_0 = 0,  ///< TWAI0 controller (primary)
  HF_CAN_CONTROLLER_1 = 1,  ///< TWAI1 controller (secondary, ESP32C6 only)
  HF_CAN_CONTROLLER_MAX     ///< Maximum number of controllers
};

/**
 * @brief MCU-specific CAN alert flags for ESP-IDF v5.5+ TWAI monitoring.
 * @details Used for comprehensive error detection and bus monitoring.
 */
enum class hf_can_alert_t : uint32_t {
  HF_CAN_ALERT_TX_IDLE = (1UL << 0),           ///< No more messages queued for TX
  HF_CAN_ALERT_TX_SUCCESS = (1UL << 1),        ///< Previous TX succeeded
  HF_CAN_ALERT_RX_DATA = (1UL << 2),           ///< Frame received successfully  
  HF_CAN_ALERT_BELOW_ERR_WARN = (1UL << 3),    ///< Error count below warning threshold
  HF_CAN_ALERT_ERR_ACTIVE = (1UL << 4),        ///< Node is error active
  HF_CAN_ALERT_RECOVERY_IN_PROGRESS = (1UL << 5), ///< Node is recovering
  HF_CAN_ALERT_BUS_RECOVERED = (1UL << 6),     ///< Bus has recovered
  HF_CAN_ALERT_ARB_LOST = (1UL << 7),          ///< Arbitration lost
  HF_CAN_ALERT_ABOVE_ERR_WARN = (1UL << 8),    ///< Error count above warning threshold
  HF_CAN_ALERT_BUS_ERROR = (1UL << 9),         ///< Bus error occurred
  HF_CAN_ALERT_TX_FAILED = (1UL << 10),        ///< Previous TX failed
  HF_CAN_ALERT_RX_QUEUE_FULL = (1UL << 11),    ///< RX queue is full
  HF_CAN_ALERT_ERR_PASS = (1UL << 12),         ///< Node is error passive
  HF_CAN_ALERT_BUS_OFF = (1UL << 13),          ///< Node is bus-off
  HF_CAN_ALERT_RX_FIFO_OVERRUN = (1UL << 14),  ///< RX FIFO overrun
  HF_CAN_ALERT_TX_RETRIED = (1UL << 15),       ///< TX message retried
  HF_CAN_ALERT_PERIPH_RESET = (1UL << 16),     ///< Peripheral reset detected
  HF_CAN_ALERT_AND_LOG = (1UL << 17),          ///< Alert occurred and was logged
  HF_CAN_ALERT_ALL = 0x3FFFF                   ///< All alerts enabled
};

/**
 * @brief MCU-specific CAN statistics for performance monitoring.
 * @details Thread-safe statistics collection for production diagnostics.
 */
struct hf_can_statistics_t {
  std::atomic<uint32_t> tx_message_count;      ///< Total messages transmitted
  std::atomic<uint32_t> rx_message_count;      ///< Total messages received
  std::atomic<uint32_t> tx_error_count;        ///< Transmission errors
  std::atomic<uint32_t> rx_error_count;        ///< Reception errors
  std::atomic<uint32_t> arbitration_lost_count; ///< Arbitration lost events
  std::atomic<uint32_t> bus_error_count;       ///< Bus error events
  std::atomic<uint32_t> total_error_count;     ///< Total error count
  std::atomic<uint32_t> bus_off_count;         ///< Bus-off events
  std::atomic<uint32_t> recovery_count;        ///< Recovery events
  std::atomic<float> bus_load_percentage;      ///< Estimated bus load %
  uint64_t last_activity_timestamp;            ///< Last activity timestamp
  uint64_t initialization_timestamp;           ///< Initialization timestamp
  
  hf_can_statistics_t() noexcept 
      : tx_message_count(0), rx_message_count(0), tx_error_count(0), rx_error_count(0),
        arbitration_lost_count(0), bus_error_count(0), total_error_count(0), 
        bus_off_count(0), recovery_count(0), bus_load_percentage(0.0f),
        last_activity_timestamp(0), initialization_timestamp(0) {}
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
      : alerts_enabled(static_cast<uint32_t>(hf_can_alert_t::HF_CAN_ALERT_ALL)),
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
  bool sleep_retention_enable;     ///< Enable sleep retention
  bool allow_pd_in_light_sleep;    ///< Allow power down in light sleep
  bool allow_pd_in_deep_sleep;     ///< Allow power down in deep sleep
  uint32_t wakeup_filter_count;    ///< Wakeup filter frame count
  uint32_t wakeup_filter_id;       ///< Wakeup filter ID
  uint32_t wakeup_filter_mask;     ///< Wakeup filter mask
  
  hf_can_power_config_t() noexcept
      : sleep_retention_enable(false), allow_pd_in_light_sleep(false),
        allow_pd_in_deep_sleep(false), wakeup_filter_count(1),
        wakeup_filter_id(0), wakeup_filter_mask(0xFFFFFFFF) {}
};

/**
 * @brief ESP32C6 CAN handle types for v5.5+ API.
 * @details Platform-specific handle types for ESP-IDF v5.5+ TWAI driver.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_can_handle_t = void*;           ///< TWAI driver handle (twai_handle_t in ESP-IDF)
using hf_can_obj_config_t = void*;       ///< TWAI object config (twai_obj_config_t in ESP-IDF)
using hf_can_filter_handle_t = void*;    ///< TWAI filter handle
#else
using hf_can_handle_t = void*;           ///< Generic handle for other platforms
using hf_can_obj_config_t = void*;       ///< Generic config for other platforms  
using hf_can_filter_handle_t = void*;    ///< Generic filter handle
#endif

/**
 * @brief MCU-specific CAN constants for ESP32C6 TWAI controller.
 */
static constexpr hf_gpio_num_t HF_CAN_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_CAN_MAX_DATA_LEN = 8;  ///< Classic CAN max data length
static constexpr uint32_t HF_CAN_STD_ID_MASK = 0x7FF;     ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_CAN_EXT_ID_MASK = 0x1FFFFFFF; ///< Extended ID mask (29-bit)

/**
 * @brief Convert milliseconds to RTOS ticks for CAN operations.
 * @param ms Milliseconds to convert
 * @return RTOS ticks (implementation-specific)
 */
#ifdef HF_MCU_FAMILY_ESP32
#define HF_TICKS_FROM_MS(ms) (pdMS_TO_TICKS(ms))
#else
#define HF_TICKS_FROM_MS(ms) (ms)
#endif

/**
 * @brief MCU-specific CAN driver function pointers for ESP-IDF v5.5+ abstraction.
 * @details Function-like macros that map to actual ESP-IDF TWAI functions.
 */
#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF v5.5+ TWAI driver function mappings (will be resolved at compile time)
#define HF_CAN_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) \
    twai_driver_install_v2(gconfig, tconfig, fconfig, handle)
#define HF_CAN_DRIVER_UNINSTALL_V2(handle) twai_driver_uninstall_v2(handle)
#define HF_CAN_START_V2(handle) twai_start_v2(handle)
#define HF_CAN_STOP_V2(handle) twai_stop_v2(handle)
#define HF_CAN_TRANSMIT_V2(handle, message, ticks) twai_transmit_v2(handle, message, ticks)
#define HF_CAN_RECEIVE_V2(handle, message, ticks) twai_receive_v2(handle, message, ticks)
#define HF_CAN_GET_STATUS_INFO_V2(handle, status) twai_get_status_info_v2(handle, status)
#define HF_CAN_RECONFIGURE_ALERTS_V2(handle, alerts, prev) twai_reconfigure_alerts_v2(handle, alerts, prev)
#define HF_CAN_READ_ALERTS_V2(handle, alerts, ticks) twai_read_alerts_v2(handle, alerts, ticks)

// Legacy v1 API for backwards compatibility (single controller)
#define HF_CAN_DRIVER_INSTALL(gconfig, tconfig, fconfig) \
    twai_driver_install(gconfig, tconfig, fconfig)
#define HF_CAN_DRIVER_UNINSTALL() twai_driver_uninstall()
#define HF_CAN_START() twai_start()
#define HF_CAN_STOP() twai_stop()
#define HF_CAN_TRANSMIT(message, ticks) twai_transmit(message, ticks)
#define HF_CAN_RECEIVE(message, ticks) twai_receive(message, ticks)
#define HF_CAN_GET_STATUS_INFO(status) twai_get_status_info(status)
#else
// Non-ESP32 platforms - placeholder definitions
#define HF_CAN_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_DRIVER_UNINSTALL_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_START_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_STOP_V2(handle) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_TRANSMIT_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECEIVE_V2(handle, message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_GET_STATUS_INFO_V2(handle, status) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECONFIGURE_ALERTS_V2(handle, alerts, prev) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_READ_ALERTS_V2(handle, alerts, ticks) HF_CAN_ERR_NOT_SUPPORTED

#define HF_CAN_DRIVER_INSTALL(gconfig, tconfig, fconfig) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_DRIVER_UNINSTALL() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_START() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_STOP() HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_TRANSMIT(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_RECEIVE(message, ticks) HF_CAN_ERR_NOT_SUPPORTED
#define HF_CAN_GET_STATUS_INFO(status) HF_CAN_ERR_NOT_SUPPORTED
#endif

//==============================================================================
// MCU-SPECIFIC CONSTANTS
//==============================================================================

static constexpr hf_timeout_ms_t HF_TIMEOUT_NEVER = 0xFFFFFFFF;
static constexpr hf_timeout_ms_t HF_TIMEOUT_IMMEDIATE = 0;
static constexpr hf_timeout_ms_t HF_TIMEOUT_DEFAULT = 1000;

//==============================================================================
// COMMUNICATION TYPES
//==============================================================================

// Communication types use platform-agnostic base types and hf_* MCU types defined above

//==============================================================================
// ESP32C6 ADVANCED GPIO TYPES FOR ESP-IDF v5.5+
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief ESP32C6 specific GPIO glitch filter types.
 * @details Advanced glitch filtering capabilities for noise reduction.
 */
enum class hf_gpio_glitch_filter_type_t : uint8_t {
  HF_GPIO_GLITCH_FILTER_NONE = 0, ///< No glitch filter
  HF_GPIO_GLITCH_FILTER_PIN = 1,  ///< Pin glitch filter (2 clock cycles)
  HF_GPIO_GLITCH_FILTER_FLEX = 2, ///< Flexible glitch filter (configurable)
  HF_GPIO_GLITCH_FILTER_BOTH = 3, ///< Both pin and flex filters
};

/**
 * @brief ESP32C6 GPIO drive capability levels.
 * @details Configurable output drive strength for power optimization.
 */
enum class hf_gpio_drive_strength_t : uint8_t {
  HF_GPIO_DRIVE_CAP_WEAK = 0,     ///< ~5mA drive capability
  HF_GPIO_DRIVE_CAP_STRONGER = 1, ///< ~10mA drive capability  
  HF_GPIO_DRIVE_CAP_MEDIUM = 2,   ///< ~20mA drive capability (default)
  HF_GPIO_DRIVE_CAP_STRONGEST = 3 ///< ~40mA drive capability
};

/**
 * @brief ESP32C6 low-power GPIO configuration for sleep modes.
 * @details Configuration for GPIO behavior during sleep and low-power operation.
 */
struct hf_gpio_sleep_config_t {
  hf_gpio_mode_t sleep_mode;        ///< GPIO mode during sleep
  hf_gpio_pull_t sleep_pull_mode;   ///< Pull resistor configuration during sleep
  bool sleep_output_enable;         ///< Enable output during sleep
  bool sleep_input_enable;          ///< Enable input during sleep
  bool hold_during_sleep;           ///< Hold configuration during sleep
  bool rtc_domain_enable;           ///< Route to RTC domain for ultra-low power
};

/**
 * @brief ESP32C6 flexible glitch filter configuration.
 * @details Configurable glitch filter for advanced noise rejection.
 */
struct hf_gpio_flex_filter_config_t {
  uint32_t window_width_ns;     ///< Sample window width in nanoseconds
  uint32_t window_threshold_ns; ///< Threshold for filtering in nanoseconds
  bool enable_on_init;          ///< Enable filter immediately after creation
};

/**
 * @brief ESP32C6 GPIO wake-up configuration for deep sleep.
 * @details Configuration for GPIO-based wake-up from deep sleep modes.
 */
struct hf_gpio_wakeup_config_t {
  hf_gpio_intr_type_t wake_trigger; ///< Wake-up trigger type
  bool enable_rtc_wake;             ///< Enable RTC domain wake-up
  bool enable_ext1_wake;            ///< Enable EXT1 wake-up source
  uint8_t wake_level;               ///< Wake-up level (0=low, 1=high)
  bool internal_pullup_enable;      ///< Enable internal pull-up during sleep
  bool internal_pulldown_enable;    ///< Enable internal pull-down during sleep
};

/**
 * @brief ESP32C6 GPIO configuration for advanced features.
 * @details Complete configuration structure for ESP32C6 GPIO with all advanced features.
 */
struct hf_gpio_advanced_config_t {
  hf_gpio_num_native_t gpio_num;                    ///< GPIO pin number
  hf_gpio_mode_t mode;                              ///< GPIO mode (input/output/etc)
  hf_gpio_pull_t pull_mode;                         ///< Pull resistor configuration
  hf_gpio_intr_type_t intr_type;                    ///< Interrupt trigger type
  hf_gpio_drive_strength_t drive_strength;          ///< Output drive capability
  hf_gpio_glitch_filter_type_t glitch_filter_type;  ///< Glitch filter type
  hf_gpio_flex_filter_config_t flex_filter_config;  ///< Flexible filter configuration
  hf_gpio_sleep_config_t sleep_config;              ///< Sleep mode configuration
  hf_gpio_wakeup_config_t wakeup_config;            ///< Wake-up configuration
  bool enable_hold_function;                        ///< Enable GPIO hold function
  bool enable_rtc_gpio;                             ///< Enable RTC GPIO functionality
};

/**
 * @brief ESP32C6 GPIO status information for diagnostics.
 * @details Comprehensive status information for debugging and monitoring.
 */
struct hf_gpio_status_info_t {
  uint8_t pin_number;                          ///< GPIO pin number
  hf_gpio_mode_t current_mode;                 ///< Current GPIO mode
  hf_gpio_pull_t current_pull_mode;            ///< Current pull mode
  hf_gpio_drive_strength_t current_drive_cap;  ///< Current drive capability
  bool input_enabled;                          ///< Input buffer enabled
  bool output_enabled;                         ///< Output buffer enabled
  bool open_drain;                             ///< Open drain mode
  bool sleep_sel_enabled;                      ///< Sleep selection enabled
  uint32_t function_select;                    ///< IOMUX function selection
  bool is_rtc_gpio;                            ///< Pin supports RTC GPIO
  bool glitch_filter_enabled;                  ///< Glitch filter enabled
  hf_gpio_glitch_filter_type_t filter_type;    ///< Type of glitch filter
  bool hold_enabled;                           ///< Hold function enabled
  uint32_t interrupt_count;                    ///< Number of interrupts occurred
  bool is_wake_source;                         ///< Pin configured as wake source
};

/**
 * @brief ESP32C6 GPIO pin validity checking.
 * @details Utility for validating GPIO pin numbers for different functions.
 */
struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;        ///< Pin can be used as GPIO
  bool supports_adc;         ///< Pin supports ADC functionality
  bool supports_rtc;         ///< Pin supports RTC GPIO
  bool supports_touch;       ///< Pin supports touch sensing
  bool is_strapping_pin;     ///< Pin is a strapping pin (requires caution)
  bool is_spi_flash_pin;     ///< Pin is used for SPI flash (not recommended for GPIO)
  bool is_usb_jtag_pin;      ///< Pin is used for USB-JTAG (disables JTAG if reconfigured)
  uint8_t lp_gpio_number;    ///< Low-power GPIO number (if applicable)
  uint8_t adc_unit;          ///< ADC unit number (if applicable)
  uint8_t adc_channel;       ///< ADC channel number (if applicable)
};

/**
 * @brief Native ESP-IDF v5.5+ GPIO types mapping.
 * @details Direct mappings to ESP-IDF GPIO types for maximum compatibility.
 */
using hf_gpio_config_native_t = gpio_config_t;
using hf_gpio_glitch_filter_handle_native_t = gpio_glitch_filter_handle_t;
using hf_gpio_pin_glitch_filter_config_native_t = gpio_pin_glitch_filter_config_t;
using hf_gpio_flex_glitch_filter_config_native_t = gpio_flex_glitch_filter_config_t;
using hf_rtc_gpio_mode_native_t = rtc_gpio_mode_t;

#else
// Non-ESP32 platforms - use generic GPIO types
struct hf_gpio_advanced_config_t {
  uint32_t gpio_num;
  uint8_t mode;
  uint8_t pull_mode;
  uint8_t intr_type;
  uint8_t drive_strength;
  // Simplified structure for non-ESP32 platforms
};

struct hf_gpio_status_info_t {
  uint8_t pin_number;
  uint8_t current_mode;
  uint8_t current_pull_mode;
  bool input_enabled;
  bool output_enabled;
  uint32_t interrupt_count;
};

struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;
  bool supports_adc;
  bool is_strapping_pin;
};

// Generic handle types for non-ESP32 platforms
using hf_gpio_config_native_t = struct { int dummy; };
using hf_gpio_glitch_filter_handle_native_t = void*;
using hf_gpio_pin_glitch_filter_config_native_t = struct { int dummy; };
using hf_gpio_flex_glitch_filter_config_native_t = struct { int dummy; };
using hf_rtc_gpio_mode_native_t = uint8_t;

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// UTILITY MACROS FOR ESP32C6 GPIO VALIDATION
//==============================================================================

/**
 * @brief ESP32C6 GPIO pin count and validation macros.
 */
#ifdef HF_MCU_ESP32C6
#define HF_GPIO_PIN_COUNT 31                 ///< Total GPIO pins (0-30)
#define HF_GPIO_MAX_PIN_NUMBER 30            ///< Maximum valid GPIO pin number
#define HF_GPIO_RTC_PIN_COUNT 8              ///< RTC GPIO pins (0-7)
#define HF_GPIO_ADC_PIN_COUNT 7              ///< ADC capable pins (0-6)
#define HF_GPIO_FLEX_FILTER_COUNT 8          ///< Number of flexible glitch filters

/**
 * @brief ESP32C6 specific pin validation macros.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= HF_GPIO_MAX_PIN_NUMBER)

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) \
  ((gpio_num) == 4 || (gpio_num) == 5 || (gpio_num) == 8 || (gpio_num) == 9 || (gpio_num) == 15)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) \
  ((gpio_num) >= 24 && (gpio_num) <= 30)

#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) \
  ((gpio_num) == 12 || (gpio_num) == 13)

#define HF_GPIO_SUPPORTS_ADC(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= 6)

#else
// Generic macros for non-ESP32C6 platforms
#define HF_GPIO_PIN_COUNT 32
#define HF_GPIO_MAX_PIN_NUMBER 31
#define HF_GPIO_RTC_PIN_COUNT 0
#define HF_GPIO_ADC_PIN_COUNT 0
#define HF_GPIO_FLEX_FILTER_COUNT 0

#define HF_GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) < HF_GPIO_PIN_COUNT)
#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) false
#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) false
#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) false
#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) false
#define HF_GPIO_SUPPORTS_ADC(gpio_num) false

#endif // HF_MCU_ESP32C6

//==============================================================================
// ESP32C6 POWER MANAGEMENT AND TIMING TYPES
//==============================================================================

/**
 * @brief ESP32C6 power domain configuration for GPIO operations.
 */
enum class hf_power_domain_t : uint8_t {
  HF_POWER_DOMAIN_CPU = 0,      ///< CPU power domain
  HF_POWER_DOMAIN_RTC_PERIPH,   ///< RTC peripherals power domain
  HF_POWER_DOMAIN_XTAL,         ///< Crystal oscillator domain
  HF_POWER_DOMAIN_MODEM,        ///< RF/WiFi/BT modem domain
  HF_POWER_DOMAIN_VDDSDIO,      ///< SDIO power domain
  HF_POWER_DOMAIN_TOP,          ///< SoC top domain
};

/**
 * @brief ESP32C6 sleep mode types.
 */
enum class hf_sleep_mode_t : uint8_t {
  HF_SLEEP_MODE_NONE = 0,       ///< No sleep mode
  HF_SLEEP_MODE_LIGHT,          ///< Light sleep mode
  HF_SLEEP_MODE_DEEP,           ///< Deep sleep mode
  HF_SLEEP_MODE_HIBERNATION,    ///< Hibernation mode (lowest power)
};

/**
 * @brief High-resolution timing types for GPIO operations.
 */
using hf_timestamp_us_t = uint64_t;     ///< Microsecond timestamp
using hf_timestamp_ns_t = uint64_t;     ///< Nanosecond timestamp
using hf_duration_us_t = uint32_t;      ///< Duration in microseconds
using hf_duration_ns_t = uint32_t;      ///< Duration in nanoseconds

/**
 * @brief MCU timing conversion macros.
 */
#define HF_TICKS_FROM_MS(ms) ((ms) / portTICK_PERIOD_MS)
#define HF_MS_FROM_TICKS(ticks) ((ticks) * portTICK_PERIOD_MS)
#define HF_US_TO_TICKS(us) ((us) / (portTICK_PERIOD_MS * 1000))
#define HF_TICKS_TO_US(ticks) ((ticks) * portTICK_PERIOD_MS * 1000)

//==============================================================================
// ESP32C6 ENHANCED TWAI/CAN TYPES FOR ESP-IDF v5.5+ FEATURES
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief Enhanced ESP32C6 TWAI controller IDs with dual controller support.
 * @details ESP32C6 has 2 independent TWAI controllers that can operate simultaneously.
 */
enum class hf_twai_controller_id_t : uint8_t {
  HF_TWAI_CONTROLLER_0 = 0,     ///< Primary TWAI controller (default)
  HF_TWAI_CONTROLLER_1 = 1,     ///< Secondary TWAI controller (ESP32C6 specific)
  HF_TWAI_CONTROLLER_MAX = 2,   ///< Maximum number of controllers
};

/**
 * @brief Enhanced TWAI operating modes for ESP-IDF v5.5+ with sleep support.
 * @details Extended mode configuration with power management features.
 */
enum class hf_twai_mode_t : uint8_t {
  HF_TWAI_MODE_NORMAL = 0,        ///< Normal mode with acknowledgment
  HF_TWAI_MODE_NO_ACK = 1,        ///< No acknowledgment mode (self-test)
  HF_TWAI_MODE_LISTEN_ONLY = 2,   ///< Listen-only mode (monitoring)
  HF_TWAI_MODE_LOOPBACK = 3,      ///< Internal loopback for testing
  HF_TWAI_MODE_SLEEP = 4,         ///< Sleep mode (ESP-IDF v5.5+)
  HF_TWAI_MODE_RECOVERY = 5,      ///< Bus recovery mode (ESP-IDF v5.5+)
};

/**
 * @brief Enhanced TWAI error states with detailed recovery information.
 * @details Maps to ESP-IDF v5.5+ twai_error_state_t with additional states.
 */
enum class hf_twai_error_state_t : uint8_t {
  HF_TWAI_ERROR_ACTIVE = 0,       ///< Error active: TEC/REC < 96
  HF_TWAI_ERROR_WARNING = 1,      ///< Error warning: TEC/REC >= 96 and < 128
  HF_TWAI_ERROR_PASSIVE = 2,      ///< Error passive: TEC/REC >= 128 and < 256
  HF_TWAI_ERROR_BUS_OFF = 3,      ///< Bus-off: TEC >= 256 (node offline)
  HF_TWAI_ERROR_RECOVERING = 4,   ///< Recovery in progress (ESP-IDF v5.5+)
};

/**
 * @brief Comprehensive TWAI alert flags for ESP-IDF v5.5+ monitoring.
 * @details Enhanced alert system for comprehensive error detection and diagnostics.
 */
enum class hf_twai_alert_t : uint32_t {
  HF_TWAI_ALERT_NONE = 0x00000000,             ///< No alerts
  
  // Basic operation alerts
  HF_TWAI_ALERT_TX_IDLE = 0x00000001,          ///< TX queue empty
  HF_TWAI_ALERT_TX_SUCCESS = 0x00000002,       ///< TX successful
  HF_TWAI_ALERT_RX_DATA = 0x00000004,          ///< RX data available
  HF_TWAI_ALERT_TX_FAILED = 0x00000008,        ///< TX failed
  
  // Error state alerts
  HF_TWAI_ALERT_ERR_ACTIVE = 0x00000010,       ///< Error active state
  HF_TWAI_ALERT_ERR_WARNING = 0x00000020,      ///< Error warning state
  HF_TWAI_ALERT_ERR_PASSIVE = 0x00000040,      ///< Error passive state
  HF_TWAI_ALERT_BUS_OFF = 0x00000080,          ///< Bus-off state
  
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
  HF_TWAI_ALERT_CRC_ERROR = 0x00010000,        ///< CRC error
  HF_TWAI_ALERT_ACK_ERROR = 0x00020000,        ///< Acknowledgment error
  HF_TWAI_ALERT_BIT_ERROR = 0x00040000,        ///< Bit error
  HF_TWAI_ALERT_TIMEOUT = 0x00080000,          ///< Operation timeout
  
  // Power management alerts (ESP-IDF v5.5+)
  HF_TWAI_ALERT_SLEEP_WAKEUP = 0x00100000,     ///< Wake-up from sleep
  HF_TWAI_ALERT_POWER_DOWN = 0x00200000,       ///< Power down event
  HF_TWAI_ALERT_CLOCK_LOST = 0x00400000,       ///< Clock source lost
  
  // Recovery and maintenance alerts
  HF_TWAI_ALERT_RECOVERY_START = 0x00800000,   ///< Recovery started
  HF_TWAI_ALERT_RECOVERY_COMPLETE = 0x01000000, ///< Recovery completed
  HF_TWAI_ALERT_FILTER_HIT = 0x02000000,       ///< Acceptance filter hit
  HF_TWAI_ALERT_FILTER_MISS = 0x04000000,      ///< Acceptance filter miss
  
  // Critical system alerts
  HF_TWAI_ALERT_DRIVER_ERROR = 0x08000000,     ///< Driver internal error
  HF_TWAI_ALERT_HARDWARE_FAULT = 0x10000000,   ///< Hardware fault detected
  HF_TWAI_ALERT_MEMORY_ERROR = 0x20000000,     ///< Memory allocation error
  HF_TWAI_ALERT_CONFIG_ERROR = 0x40000000,     ///< Configuration error
  
  // Convenience masks
  HF_TWAI_ALERT_ALL_ERRORS = 0x7FFFFE10,       ///< All error alerts
  HF_TWAI_ALERT_ALL_OPERATIONS = 0x0000000F,   ///< All operation alerts
  HF_TWAI_ALERT_ALL = 0x7FFFFFFF,              ///< All alerts enabled
};

/**
 * @brief ESP32C6 enhanced TWAI timing configuration with optimization support.
 * @details Platform-specific timing parameters optimized for ESP32C6 40MHz APB clock.
 */
struct hf_twai_timing_config_t {
  uint32_t brp;                   ///< Baud rate prescaler (1-16384)
  uint8_t tseg_1;                 ///< Time segment 1 (1-16)
  uint8_t tseg_2;                 ///< Time segment 2 (1-8)
  uint8_t sjw;                    ///< Synchronization jump width (1-4)
  bool triple_sampling;           ///< Enable triple sampling for noise immunity
  uint32_t quanta_resolution_hz;  ///< Time quantum resolution (0 = auto)
  
  // ESP-IDF v5.5+ enhanced timing features
  uint8_t sync_seg;               ///< Synchronization segment (always 1)
  uint32_t nominal_baudrate;      ///< Calculated nominal baudrate
  uint32_t actual_baudrate;       ///< Actual achieved baudrate
  float baudrate_accuracy;        ///< Baudrate accuracy percentage
  uint32_t bit_time_ns;           ///< Total bit time in nanoseconds
  uint32_t sample_point_percent;  ///< Sample point percentage (60-90%)
  
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
  hf_twai_mode_t mode;              ///< Operating mode
  hf_gpio_num_t tx_io;              ///< TX GPIO pin number
  hf_gpio_num_t rx_io;              ///< RX GPIO pin number
  hf_gpio_num_t clkout_io;          ///< Clock output GPIO (optional)
  hf_gpio_num_t bus_off_io;         ///< Bus-off indicator GPIO (optional)
  
  // Queue configuration
  uint32_t tx_queue_len;            ///< TX queue length (1-64)
  uint32_t rx_queue_len;            ///< RX queue length (1-64)
  
  // Alert and monitoring configuration
  uint32_t alerts_enabled;          ///< Enabled alert flags bitmask
  uint32_t clkout_divider;          ///< Clock output divider (0 = disabled)
  uint32_t intr_flags;              ///< Interrupt allocation flags
  
  // ESP32C6 specific features
  hf_twai_controller_id_t controller_id; ///< Controller ID (0 or 1)
  bool sleep_retention_enable;      ///< Enable sleep retention (ESP-IDF v5.5+)
  bool auto_recovery_enable;        ///< Enable automatic bus-off recovery
  uint32_t recovery_timeout_ms;     ///< Recovery timeout in milliseconds
  
  // Power management (ESP-IDF v5.5+)
  bool power_management_enable;     ///< Enable power management
  bool clock_gating_enable;         ///< Enable clock gating for power saving
  uint32_t idle_timeout_ms;         ///< Idle timeout before power saving
  
  // Performance and reliability features
  uint32_t error_warning_limit;     ///< Error warning threshold (default: 96)
  uint32_t error_passive_limit;     ///< Error passive threshold (default: 128)
  uint32_t bus_off_recovery_time_ms; ///< Bus-off recovery time
  bool enable_advanced_diagnostics; ///< Enable advanced error diagnostics
  
  hf_twai_general_config_t() noexcept
      : mode(hf_twai_mode_t::HF_TWAI_MODE_NORMAL), tx_io(HF_INVALID_PIN), rx_io(HF_INVALID_PIN),
        clkout_io(HF_INVALID_PIN), bus_off_io(HF_INVALID_PIN), tx_queue_len(10), rx_queue_len(10),
        alerts_enabled(static_cast<uint32_t>(hf_twai_alert_t::HF_TWAI_ALERT_ALL_ERRORS)),
        clkout_divider(0), intr_flags(0), controller_id(hf_twai_controller_id_t::HF_TWAI_CONTROLLER_0),
        sleep_retention_enable(false), auto_recovery_enable(true), recovery_timeout_ms(2000),
        power_management_enable(false), clock_gating_enable(false), idle_timeout_ms(5000),
        error_warning_limit(96), error_passive_limit(128), bus_off_recovery_time_ms(1000),
        enable_advanced_diagnostics(true) {}
};

/**
 * @brief Enhanced TWAI filter configuration with multi-filter support.
 * @details Hardware acceptance filter configuration with ESP-IDF v5.5+ enhancements.
 */
struct hf_twai_filter_config_t {
  uint32_t acceptance_code;         ///< Acceptance code for filtering
  uint32_t acceptance_mask;         ///< Acceptance mask (0 = don't care)
  bool single_filter;               ///< Single vs dual filter mode
  
  // ESP-IDF v5.5+ enhanced filtering features
  uint32_t acceptance_code_ext;     ///< Extended acceptance code (29-bit)
  uint32_t acceptance_mask_ext;     ///< Extended acceptance mask (29-bit)
  bool enable_std_filter;           ///< Enable standard frame filtering
  bool enable_ext_filter;           ///< Enable extended frame filtering
  bool enable_rtr_filter;           ///< Enable RTR frame filtering
  
  // Advanced filter configuration
  uint8_t filter_priority;          ///< Filter priority (0-7)
  bool filter_invert;               ///< Invert filter logic
  uint32_t filter_hit_counter;      ///< Filter hit counter (read-only)
  uint32_t filter_miss_counter;     ///< Filter miss counter (read-only)
  
  hf_twai_filter_config_t() noexcept
      : acceptance_code(0), acceptance_mask(0xFFFFFFFF), single_filter(true),
        acceptance_code_ext(0), acceptance_mask_ext(0x1FFFFFFF), enable_std_filter(true),
        enable_ext_filter(true), enable_rtr_filter(true), filter_priority(0),
        filter_invert(false), filter_hit_counter(0), filter_miss_counter(0) {}
};

/**
 * @brief Enhanced TWAI message structure with ESP-IDF v5.5+ metadata.
 * @details Native message format with comprehensive timing and diagnostic information.
 */
struct hf_twai_message_t {
  uint32_t id;                      ///< Message ID (11 or 29-bit)
  uint8_t dlc;                      ///< Data length code (0-8)
  uint8_t data[8];                  ///< Message data
  
  // Standard CAN flags
  bool is_extended;                 ///< Extended ID flag (29-bit vs 11-bit)
  bool is_rtr;                      ///< Remote transmission request flag
  bool is_ss;                       ///< Single shot flag (no retransmission)
  bool is_self;                     ///< Self reception request flag
  bool dlc_non_comp;                ///< DLC is non-compliant (< 8)
  
  // ESP-IDF v5.5+ enhanced metadata
  uint64_t timestamp_us;            ///< Precise timestamp in microseconds
  uint32_t sequence_number;         ///< Message sequence number
  uint8_t controller_id;            ///< Originating controller ID
  uint8_t queue_position;           ///< Position in queue when received
  
  // Reception and transmission metadata
  uint8_t retry_count;              ///< Number of transmission retries
  uint8_t error_count;              ///< Associated error count
  uint16_t bit_timing_errors;       ///< Bit timing error flags
  uint16_t reception_flags;         ///< Reception condition flags
  
  // Advanced diagnostics (ESP-IDF v5.5+)
  float signal_quality;             ///< Signal quality indicator (0.0-1.0)
  uint8_t bus_load_percent;         ///< Bus load percentage when transmitted
  uint16_t inter_frame_gap_us;      ///< Inter-frame gap in microseconds
  uint32_t crc_calculated;          ///< Calculated CRC for validation
  
  hf_twai_message_t() noexcept
      : id(0), dlc(0), data{}, is_extended(false), is_rtr(false), is_ss(false),
        is_self(false), dlc_non_comp(false), timestamp_us(0), sequence_number(0),
        controller_id(0), queue_position(0), retry_count(0), error_count(0),
        bit_timing_errors(0), reception_flags(0), signal_quality(1.0f),
        bus_load_percent(0), inter_frame_gap_us(0), crc_calculated(0) {}
};

/**
 * @brief Comprehensive TWAI status information for ESP32C6 with ESP-IDF v5.5+.
 * @details Enhanced status structure with detailed diagnostics and performance metrics.
 */
struct hf_twai_status_info_t {
  hf_twai_error_state_t state;      ///< Current error state
  
  // Error counters
  uint32_t tx_error_counter;        ///< Transmit error counter
  uint32_t rx_error_counter;        ///< Receive error counter
  uint32_t tx_failed_count;         ///< Failed transmission count
  uint32_t rx_missed_count;         ///< Missed reception count
  
  // Queue status
  uint32_t rx_queue_len;            ///< Current RX queue length
  uint32_t tx_queue_len;            ///< Current TX queue length
  uint32_t rx_queue_peak;           ///< Peak RX queue usage
  uint32_t tx_queue_peak;           ///< Peak TX queue usage
  
  // Advanced error statistics (ESP-IDF v5.5+)
  uint32_t arbitration_lost_count;  ///< Arbitration lost count
  uint32_t bus_error_count;         ///< Bus error count
  uint32_t stuff_error_count;       ///< Bit stuffing error count
  uint32_t form_error_count;        ///< Frame format error count
  uint32_t crc_error_count;         ///< CRC error count
  uint32_t ack_error_count;         ///< Acknowledgment error count
  uint32_t bit_error_count;         ///< Bit error count
  
  // Performance metrics
  uint32_t messages_transmitted;    ///< Total messages transmitted
  uint32_t messages_received;       ///< Total messages received
  uint32_t bytes_transmitted;       ///< Total bytes transmitted
  uint32_t bytes_received;          ///< Total bytes received
  uint64_t bus_uptime_us;           ///< Bus uptime in microseconds
  
  // Real-time bus conditions
  uint8_t bus_load_percent;         ///< Current bus load percentage
  uint32_t dominant_bit_count;      ///< Dominant bit count in last frame
  uint32_t recessive_bit_count;     ///< Recessive bit count in last frame
  float bit_error_rate;             ///< Bit error rate (errors/bits)
  
  // Power and clock status (ESP-IDF v5.5+)
  bool clock_stable;                ///< Clock source is stable
  bool power_domain_active;         ///< Power domain is active
  uint32_t sleep_wakeup_count;      ///< Number of sleep/wake cycles
  uint32_t clock_recovery_count;    ///< Clock recovery events
  
  hf_twai_status_info_t() noexcept
      : state(hf_twai_error_state_t::HF_TWAI_ERROR_ACTIVE), tx_error_counter(0), rx_error_counter(0),
        tx_failed_count(0), rx_missed_count(0), rx_queue_len(0), tx_queue_len(0),
        rx_queue_peak(0), tx_queue_peak(0), arbitration_lost_count(0), bus_error_count(0),
        stuff_error_count(0), form_error_count(0), crc_error_count(0), ack_error_count(0),
        bit_error_count(0), messages_transmitted(0), messages_received(0), bytes_transmitted(0),
        bytes_received(0), bus_uptime_us(0), bus_load_percent(0), dominant_bit_count(0),
        recessive_bit_count(0), bit_error_rate(0.0f), clock_stable(true), power_domain_active(true),
        sleep_wakeup_count(0), clock_recovery_count(0) {}
};

/**
 * @brief ESP32C6 TWAI controller capabilities and limitations.
 * @details Static capability information for runtime feature detection.
 */
struct hf_twai_capabilities_t {
  uint8_t num_controllers;          ///< Number of TWAI controllers (2 for ESP32C6)
  uint8_t max_tx_queue_size;        ///< Maximum TX queue size
  uint8_t max_rx_queue_size;        ///< Maximum RX queue size
  uint32_t max_baudrate;            ///< Maximum supported baudrate
  uint32_t min_baudrate;            ///< Minimum supported baudrate
  bool supports_canfd;              ///< CAN-FD support (false for ESP32C6)
  bool supports_sleep_retention;    ///< Sleep retention support
  bool supports_dual_controllers;   ///< Dual controller support
  bool supports_advanced_filters;   ///< Advanced filtering support
  bool supports_power_management;   ///< Power management support
  uint8_t num_hardware_filters;     ///< Number of hardware filters
  uint32_t min_bit_time_ns;         ///< Minimum bit time in nanoseconds
  uint32_t max_bit_time_ns;         ///< Maximum bit time in nanoseconds
  
  // Constructor with ESP32C6 defaults
  hf_twai_capabilities_t() noexcept
      : num_controllers(2), max_tx_queue_size(64), max_rx_queue_size(64),
        max_baudrate(1000000), min_baudrate(10000), supports_canfd(false),
        supports_sleep_retention(true), supports_dual_controllers(true),
        supports_advanced_filters(true), supports_power_management(true),
        num_hardware_filters(2), min_bit_time_ns(1000), max_bit_time_ns(100000000) {}
};

#else
// Non-ESP32 platforms - use simplified TWAI types

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
  HF_TWAI_ERROR_WARNING = 1,
  HF_TWAI_ERROR_PASSIVE = 2,
  HF_TWAI_ERROR_BUS_OFF = 3,
};

enum class hf_twai_alert_t : uint32_t {
  HF_TWAI_ALERT_NONE = 0x00000000,
  HF_TWAI_ALERT_ALL = 0x7FFFFFFF,
};

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
      : state(hf_twai_error_state_t::HF_TWAI_ERROR_ACTIVE), tx_error_counter(0), rx_error_counter(0),
        tx_failed_count(0), rx_missed_count(0), rx_queue_len(0), tx_queue_len(0) {}
};

struct hf_twai_capabilities_t {
  uint8_t num_controllers;
  uint8_t max_tx_queue_size;
  uint8_t max_rx_queue_size;
  uint32_t max_baudrate;
  uint32_t min_baudrate;
  bool supports_canfd;
  
  hf_twai_capabilities_t() noexcept
      : num_controllers(1), max_tx_queue_size(32), max_rx_queue_size(32),
        max_baudrate(1000000), min_baudrate(10000), supports_canfd(false) {}
};

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// ENHANCED ESP-IDF v5.5+ TWAI DRIVER FUNCTION MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// Enhanced ESP-IDF v5.5+ TWAI driver function mappings with comprehensive error handling

// Core driver lifecycle operations
#define HF_TWAI_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle) \
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
#define HF_TWAI_RECONFIGURE_ALERTS_V2(handle, alerts, prev) twai_reconfigure_alerts_v2(handle, alerts, prev)
#define HF_TWAI_READ_ALERTS_V2(handle, alerts, ticks) twai_read_alerts_v2(handle, alerts, ticks)

// Error recovery operations
#define HF_TWAI_INITIATE_RECOVERY_V2(handle) twai_initiate_recovery_v2(handle)
#define HF_TWAI_GET_ERROR_COUNTERS_V2(handle, tx_err, rx_err) twai_get_error_counters_v2(handle, tx_err, rx_err)

// Legacy v1 API for single controller backwards compatibility
#define HF_TWAI_DRIVER_INSTALL(gconfig, tconfig, fconfig) \
    twai_driver_install(gconfig, tconfig, fconfig)
#define HF_TWAI_DRIVER_UNINSTALL() twai_driver_uninstall()
#define HF_TWAI_START() twai_start()
#define HF_TWAI_STOP() twai_stop()
#define HF_TWAI_TRANSMIT(message, ticks) twai_transmit(message, ticks)
#define HF_TWAI_RECEIVE(message, ticks) twai_receive(message, ticks)
#define HF_TWAI_GET_STATUS_INFO(status) twai_get_status_info(status)

// ESP-IDF v5.5+ specific features (may not be available in all versions)
#ifdef ESP_IDF_VERSION_MAJOR
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable) twai_configure_sleep_retention_v2(handle, enable)
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) twai_set_power_management_v2(handle, enable)
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) twai_get_capabilities_v2(handle, caps)
#else
// Fallback definitions for older ESP-IDF versions
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) ESP_ERR_NOT_SUPPORTED
#endif
#else
// Default to not supported if version detection fails
#define HF_TWAI_CONFIGURE_SLEEP_RETENTION_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_SET_POWER_MANAGEMENT_V2(handle, enable) ESP_ERR_NOT_SUPPORTED
#define HF_TWAI_GET_CAPABILITIES_V2(handle, caps) ESP_ERR_NOT_SUPPORTED
#endif

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
#endif

//==============================================================================
// TWAI UTILITY MACROS AND CONSTANTS
//==============================================================================

/**
 * @brief ESP32C6 TWAI controller physical limits and capabilities.
 */
#ifdef HF_MCU_ESP32C6
static constexpr uint8_t HF_TWAI_MAX_CONTROLLERS = 2;           ///< ESP32C6 has 2 TWAI controllers
static constexpr uint32_t HF_TWAI_APB_CLOCK_HZ = 40000000;      ///< ESP32C6 APB clock frequency
static constexpr uint32_t HF_TWAI_MAX_BRP = 16384;              ///< Maximum baud rate prescaler
static constexpr uint8_t HF_TWAI_MAX_TSEG1 = 16;                ///< Maximum time segment 1
static constexpr uint8_t HF_TWAI_MAX_TSEG2 = 8;                 ///< Maximum time segment 2
static constexpr uint8_t HF_TWAI_MAX_SJW = 4;                   ///< Maximum synchronization jump width
static constexpr uint32_t HF_TWAI_MIN_BAUDRATE = 1000;          ///< Minimum supported baudrate
static constexpr uint32_t HF_TWAI_MAX_BAUDRATE = 1000000;       ///< Maximum supported baudrate
static constexpr uint8_t HF_TWAI_MAX_QUEUE_SIZE = 64;           ///< Maximum queue size
static constexpr uint8_t HF_TWAI_MIN_QUEUE_SIZE = 1;            ///< Minimum queue size
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
static constexpr uint8_t HF_TWAI_MAX_DATA_LEN = 8;              ///< Classic CAN max data length
static constexpr uint32_t HF_TWAI_STD_ID_MASK = 0x7FF;         ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_TWAI_EXT_ID_MASK = 0x1FFFFFFF;    ///< Extended ID mask (29-bit)
static constexpr uint32_t HF_TWAI_MAX_STD_ID = 0x7FF;          ///< Maximum standard ID
static constexpr uint32_t HF_TWAI_MAX_EXT_ID = 0x1FFFFFFF;     ///< Maximum extended ID

/**
 * @brief TWAI timing calculation macros for ESP32C6.
 */
#define HF_TWAI_CALCULATE_BIT_TIME_NS(brp, tseg1, tseg2) \
    (((brp) * ((tseg1) + (tseg2) + 1) * 1000000000ULL) / HF_TWAI_APB_CLOCK_HZ)

#define HF_TWAI_CALCULATE_BAUDRATE(brp, tseg1, tseg2) \
    (HF_TWAI_APB_CLOCK_HZ / ((brp) * ((tseg1) + (tseg2) + 1)))

#define HF_TWAI_CALCULATE_SAMPLE_POINT_PERCENT(tseg1, tseg2) \
    (((tseg1) + 1) * 100 / ((tseg1) + (tseg2) + 1))

/**
 * @brief TWAI validation macros.
 */
#define HF_TWAI_IS_VALID_CONTROLLER_ID(id) \
    ((id) < HF_TWAI_MAX_CONTROLLERS)

#define HF_TWAI_IS_VALID_BAUDRATE(rate) \
    ((rate) >= HF_TWAI_MIN_BAUDRATE && (rate) <= HF_TWAI_MAX_BAUDRATE)

#define HF_TWAI_IS_VALID_QUEUE_SIZE(size) \
    ((size) >= HF_TWAI_MIN_QUEUE_SIZE && (size) <= HF_TWAI_MAX_QUEUE_SIZE)

#define HF_TWAI_IS_VALID_STD_ID(id) \
    ((id) <= HF_TWAI_MAX_STD_ID)

#define HF_TWAI_IS_VALID_EXT_ID(id) \
    ((id) <= HF_TWAI_MAX_EXT_ID)

#define HF_TWAI_IS_VALID_DLC(dlc) \
    ((dlc) <= HF_TWAI_MAX_DATA_LEN)

//==============================================================================
// ESP32C6 ENHANCED GPIO CONSTANTS AND CAPABILITIES
//==============================================================================

#ifdef HF_MCU_ESP32C6

/**
 * @brief ESP32C6 GPIO physical limits and pin capabilities.
 * @details Based on ESP32C6 technical reference manual and ESP-IDF v5.5+ documentation.
 */
static constexpr uint8_t HF_GPIO_PIN_COUNT = 31;                ///< ESP32C6 has GPIO0-GPIO30 (31 pins)
static constexpr uint8_t HF_GPIO_MAX_PIN = 30;                  ///< Maximum GPIO pin number
static constexpr uint8_t HF_GPIO_MIN_PIN = 0;                   ///< Minimum GPIO pin number

/**
 * @brief ESP32C6 RTC/Low-Power GPIO capabilities.
 * @details GPIO0-7 support RTC/LP_GPIO functionality for deep sleep and analog.
 */
static constexpr uint8_t HF_GPIO_RTC_PIN_COUNT = 8;             ///< Number of RTC-capable pins
static constexpr uint8_t HF_GPIO_RTC_MAX_PIN = 7;               ///< Maximum RTC GPIO pin number
static constexpr uint8_t HF_GPIO_RTC_MIN_PIN = 0;               ///< Minimum RTC GPIO pin number

/**
 * @brief ESP32C6 ADC-capable GPIO pins.
 * @details GPIO0-6 support ADC1_CH0 through ADC1_CH6 functionality.
 */
static constexpr uint8_t HF_GPIO_ADC_PIN_COUNT = 7;             ///< Number of ADC-capable pins
static constexpr uint8_t HF_GPIO_ADC_MAX_PIN = 6;               ///< Maximum ADC GPIO pin number
static constexpr uint8_t HF_GPIO_ADC_MIN_PIN = 0;               ///< Minimum ADC GPIO pin number

/**
 * @brief ESP32C6 strapping pins that require careful handling.
 * @details These pins affect boot behavior and should be handled with caution.
 */
static constexpr uint8_t HF_GPIO_STRAPPING_PINS[] = {4, 5, 8, 9, 15};
static constexpr uint8_t HF_GPIO_STRAPPING_PIN_COUNT = 5;

/**
 * @brief ESP32C6 USB-JTAG pins (normally reserved).
 * @details GPIO12-13 are used by USB-JTAG by default.
 */
static constexpr uint8_t HF_GPIO_USB_JTAG_PINS[] = {12, 13};
static constexpr uint8_t HF_GPIO_USB_JTAG_PIN_COUNT = 2;

/**
 * @brief ESP32C6 SPI flash pins (normally reserved).
 * @details GPIO24-30 are typically used for SPI flash (especially in SiP variants).
 */
static constexpr uint8_t HF_GPIO_SPI_FLASH_PINS[] = {24, 25, 26, 27, 28, 29, 30};
static constexpr uint8_t HF_GPIO_SPI_FLASH_PIN_COUNT = 7;

/**
 * @brief ESP32C6 unavailable pins on some variants.
 * @details GPIO10-11 not available on SiP flash variants, GPIO14 not available on non-flash variants.
 */
static constexpr uint8_t HF_GPIO_UNAVAILABLE_SIP_PINS[] = {10, 11};     ///< Not available on SiP flash variants
static constexpr uint8_t HF_GPIO_UNAVAILABLE_NOFLASH_PINS[] = {14};     ///< Not available on non-flash variants

/**
 * @brief ESP32C6 glitch filter capabilities.
 * @details Hardware filters for removing glitch pulses from GPIO inputs.
 */
static constexpr uint8_t HF_GPIO_GLITCH_FILTER_COUNT = 8;        ///< Number of flexible glitch filters
static constexpr uint32_t HF_GPIO_GLITCH_FILTER_MIN_NS = 50;     ///< Minimum filter window (nanoseconds)
static constexpr uint32_t HF_GPIO_GLITCH_FILTER_MAX_NS = 1000000; ///< Maximum filter window (nanoseconds)

/**
 * @brief ESP32C6 GPIO validation macros.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num) \
    ((gpio_num) >= HF_GPIO_MIN_PIN && (gpio_num) <= HF_GPIO_MAX_PIN)

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) \
    ((gpio_num) >= HF_GPIO_RTC_MIN_PIN && (gpio_num) <= HF_GPIO_RTC_MAX_PIN)

#define HF_GPIO_IS_VALID_ADC_GPIO(gpio_num) \
    ((gpio_num) >= HF_GPIO_ADC_MIN_PIN && (gpio_num) <= HF_GPIO_ADC_MAX_PIN)

#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) \
    ((gpio_num) == 4 || (gpio_num) == 5 || (gpio_num) == 8 || (gpio_num) == 9 || (gpio_num) == 15)

#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) \
    ((gpio_num) == 12 || (gpio_num) == 13)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) \
    ((gpio_num) >= 24 && (gpio_num) <= 30)

#else
// Generic GPIO constants for non-ESP32C6 platforms
static constexpr uint8_t HF_GPIO_PIN_COUNT = 32;
static constexpr uint8_t HF_GPIO_MAX_PIN = 31;
static constexpr uint8_t HF_GPIO_MIN_PIN = 0;
static constexpr uint8_t HF_GPIO_RTC_PIN_COUNT = 0;
static constexpr uint8_t HF_GPIO_GLITCH_FILTER_COUNT = 0;

#define HF_GPIO_IS_VALID_GPIO(gpio_num) \
    ((gpio_num) >= HF_GPIO_MIN_PIN && (gpio_num) <= HF_GPIO_MAX_PIN)
#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) (false)
#define HF_GPIO_IS_VALID_ADC_GPIO(gpio_num) (false)
#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) (false)
#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) (false)
#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) (false)
#endif
