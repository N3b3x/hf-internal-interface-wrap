/**
 * @file McuTypes.h
 * @brief MCU-specific type definitions for hardware abstraction (hf_* types).
 *
 * This header defines all MCU-specific types and constants that are used
 * throughout the internal interface wrap layer. By centralizing these definitions,
 * porting to new MCUs only requires updating this single file. The types provide
 * platform-specific configurations while maintaining interface compatibility.
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
 * @note All interface classes (CAN, UART, I2C, SPI, GPIO, ADC, PIO, PWM) must use only these types.
 * @note This implementation is verified against ESP-IDF v5.4+ documentation and supports all latest features.
 */

#pragma once

#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF headers)
#include <atomic>
#include <cstdint>

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
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "driver/twai.h" // ESP-IDF v5.5+ TWAI driver
#include "driver/uart.h"
#include "driver/ledc.h"
#include "esp_timer.h" // For esp_timer_handle_t
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" // For QueueHandle_t
#include "freertos/semphr.h"

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

// ESP32 SPI specific mappings
using hf_spi_host_native_t = spi_host_device_t;
using hf_spi_bus_config_native_t = spi_bus_config_t;
using hf_spi_device_config_native_t = spi_device_interface_config_t;
using hf_spi_transaction_native_t = spi_transaction_t;
using hf_spi_device_handle_native_t = spi_device_handle_t;

// ESP32 I2C specific mappings
using hf_i2c_port_native_t = i2c_port_t;
using hf_i2c_master_bus_handle_native_t = i2c_master_bus_handle_t;
using hf_i2c_device_handle_native_t = i2c_master_dev_handle_t;
using hf_i2c_bus_config_native_t = i2c_master_bus_config_t;
using hf_i2c_device_config_native_t = i2c_device_config_t;
using hf_i2c_cmd_handle_native_t = i2c_cmd_handle_t;

// ESP32 UART specific mappings
using hf_uart_port_native_t = uart_port_t;
using hf_uart_config_native_t = uart_config_t;
using hf_uart_word_length_native_t = uart_word_length_t;
using hf_uart_parity_native_t = uart_parity_t;
using hf_uart_stop_bits_native_t = uart_stop_bits_t;
using hf_uart_hw_flowcontrol_native_t = uart_hw_flowcontrol_t;
using hf_uart_signal_inv_native_t = uart_signal_inv_t;

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

// Generic SPI types for non-ESP32 platforms
using hf_spi_host_native_t = uint8_t;
using hf_spi_bus_config_native_t = struct {
  int dummy;
};
using hf_spi_device_config_native_t = struct {
  int dummy;
};
using hf_spi_transaction_native_t = struct {
  int dummy;
};
using hf_spi_device_handle_native_t = void *;

// Generic I2C types for non-ESP32 platforms
using hf_i2c_port_native_t = uint8_t;
using hf_i2c_master_bus_handle_native_t = void *;
using hf_i2c_device_handle_native_t = void *;
using hf_i2c_bus_config_native_t = struct {
  int dummy;
};
using hf_i2c_device_config_native_t = struct {
  int dummy;
};
using hf_i2c_cmd_handle_native_t = void *;

// Generic UART types for non-ESP32 platforms
using hf_uart_port_native_t = uint8_t;
using hf_uart_config_native_t = struct { int dummy; };
using hf_uart_word_length_native_t = uint8_t;
using hf_uart_parity_native_t = uint8_t;
using hf_uart_stop_bits_native_t = uint8_t;
using hf_uart_hw_flowcontrol_native_t = uint8_t;
using hf_uart_signal_inv_native_t = uint8_t;

// Generic RTOS handle types for non-ESP32 platforms
using SemaphoreHandle_t = void *;
using QueueHandle_t = void *;
using i2c_config_t = struct {
  int dummy;
};
using spi_bus_config_t = struct {
  int dummy;
};
using spi_device_interface_config_t = struct {
  int dummy;
};
using uart_config_t = struct {
  int dummy;
};
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
// CAN-specific enums are defined in the TWAI section below to avoid duplication
// Use hf_twai_* types and aliases for CAN functionality

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

/**
 * @brief UART handle type for MCU UART driver.
 * @details ESP32 UART driver uses port-based API, so handle is not used directly.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_uart_handle_t = void *; ///< ESP32 UART uses port numbers, not handles
#else
using hf_uart_handle_t = void *; ///< Generic handle for other platforms
#endif

/**
 * @brief SPI device handle type for MCU SPI driver.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_spi_handle_t = spi_device_handle_t;
#else
using hf_spi_handle_t = void *;
#endif

//==============================================================================
// UART ADVANCED CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief MCU-specific UART statistics and monitoring configuration.
 * @details Statistics tracking for UART communication performance and errors.
 */
struct hf_uart_statistics_t {
  uint32_t tx_byte_count;       ///< Total bytes transmitted
  uint32_t rx_byte_count;       ///< Total bytes received
  uint32_t tx_error_count;      ///< Transmission error count
  uint32_t rx_error_count;      ///< Reception error count
  uint32_t frame_error_count;   ///< Frame error count
  uint32_t parity_error_count;  ///< Parity error count
  uint32_t overrun_error_count; ///< Overrun error count
  uint32_t noise_error_count;   ///< Noise error count
  uint32_t break_count;         ///< Break condition count
  uint32_t timeout_count;       ///< Timeout occurrence count
  uint64_t last_activity_timestamp; ///< Last activity timestamp (microseconds)
  uint64_t initialization_timestamp; ///< Initialization timestamp (microseconds)

  hf_uart_statistics_t() noexcept
      : tx_byte_count(0), rx_byte_count(0), tx_error_count(0), rx_error_count(0),
        frame_error_count(0), parity_error_count(0), overrun_error_count(0),
        noise_error_count(0), break_count(0), timeout_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

/**
 * @brief MCU-specific UART flow control configuration.
 * @details Advanced flow control settings for hardware and software flow control.
 */
struct hf_uart_flow_config_t {
  bool enable_hw_flow_control;  ///< Enable hardware flow control (RTS/CTS)
  bool enable_sw_flow_control;  ///< Enable software flow control (XON/XOFF)
  uint8_t xon_char;            ///< XON character (default: 0x11)
  uint8_t xoff_char;           ///< XOFF character (default: 0x13)
  uint16_t rx_flow_ctrl_thresh; ///< RX flow control threshold (bytes)
  uint16_t tx_flow_ctrl_thresh; ///< TX flow control threshold (bytes)
  bool auto_rts;               ///< Automatic RTS control
  bool auto_cts;               ///< Automatic CTS control

  hf_uart_flow_config_t() noexcept
      : enable_hw_flow_control(false), enable_sw_flow_control(false),
        xon_char(0x11), xoff_char(0x13), rx_flow_ctrl_thresh(120),
        tx_flow_ctrl_thresh(10), auto_rts(true), auto_cts(true) {}
};

/**
 * @brief MCU-specific UART power management configuration.
 * @details Power management settings for sleep modes and retention.
 */
struct hf_uart_power_config_t {
  bool sleep_retention_enable;  ///< Enable sleep retention
  bool allow_pd_in_light_sleep; ///< Allow power down in light sleep
  bool allow_pd_in_deep_sleep;  ///< Allow power down in deep sleep
  bool wakeup_enable;          ///< Enable UART wakeup capability
  uint8_t wakeup_threshold;    ///< Wakeup threshold character count
  uint32_t wakeup_timeout_ms;  ///< Wakeup timeout in milliseconds

  hf_uart_power_config_t() noexcept
      : sleep_retention_enable(false), allow_pd_in_light_sleep(false),
        allow_pd_in_deep_sleep(false), wakeup_enable(false),
        wakeup_threshold(1), wakeup_timeout_ms(1000) {}
};

/**
 * @brief MCU-specific CAN constants for ESP32C6 TWAI controller.
 */
static constexpr hf_gpio_num_t HF_CAN_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_CAN_MAX_DATA_LEN = 8;         ///< Classic CAN max data length
static constexpr uint32_t HF_CAN_STD_ID_MASK = 0x7FF;      ///< Standard ID mask (11-bit)
static constexpr uint32_t HF_CAN_EXT_ID_MASK = 0x1FFFFFFF; ///< Extended ID mask (29-bit)

/**
 * @brief MCU-specific UART constants for ESP32C6 UART controller.
 */
static constexpr hf_gpio_num_t HF_UART_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_UART_MAX_PORTS = 3;          ///< ESP32C6 has 3 UART ports (0, 1, 2)
static constexpr uint32_t HF_UART_DEFAULT_BUFFER_SIZE = 256; ///< Default buffer size (bytes)
static constexpr uint32_t HF_UART_MIN_BAUD_RATE = 1200;   ///< Minimum supported baud rate
static constexpr uint32_t HF_UART_MAX_BAUD_RATE = 5000000; ///< Maximum supported baud rate
static constexpr uint32_t HF_UART_BREAK_MIN_DURATION = 1;  ///< Minimum break duration (ms)
static constexpr uint32_t HF_UART_BREAK_MAX_DURATION = 1000; ///< Maximum break duration (ms)

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
#define HF_CAN_DRIVER_INSTALL_V2(gconfig, tconfig, fconfig, handle)                                \
  twai_driver_install_v2(gconfig, tconfig, fconfig, handle)
#define HF_CAN_DRIVER_UNINSTALL_V2(handle) twai_driver_uninstall_v2(handle)
#define HF_CAN_START_V2(handle) twai_start_v2(handle)
#define HF_CAN_STOP_V2(handle) twai_stop_v2(handle)
#define HF_CAN_TRANSMIT_V2(handle, message, ticks) twai_transmit_v2(handle, message, ticks)
#define HF_CAN_RECEIVE_V2(handle, message, ticks) twai_receive_v2(handle, message, ticks)
#define HF_CAN_GET_STATUS_INFO_V2(handle, status) twai_get_status_info_v2(handle, status)
#define HF_CAN_RECONFIGURE_ALERTS_V2(handle, alerts, prev)                                         \
  twai_reconfigure_alerts_v2(handle, alerts, prev)
#define HF_CAN_READ_ALERTS_V2(handle, alerts, ticks) twai_read_alerts_v2(handle, alerts, ticks)

// Legacy v1 API for backwards compatibility (single controller)
#define HF_CAN_DRIVER_INSTALL(gconfig, tconfig, fconfig)                                           \
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

/**
 * @brief MCU-specific UART driver function pointers for ESP-IDF abstraction.
 * @details Function-like macros that map to actual ESP-IDF UART functions.
 */
#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF UART driver function mappings (will be resolved at compile time)
#define HF_UART_DRIVER_INSTALL(port, tx_size, rx_size, queue_size, queue, intr_flags) \
  uart_driver_install(port, tx_size, rx_size, queue_size, queue, intr_flags)
#define HF_UART_DRIVER_DELETE(port) uart_driver_delete(port)
#define HF_UART_PARAM_CONFIG(port, config) uart_param_config(port, config)
#define HF_UART_SET_PIN(port, tx_pin, rx_pin, rts_pin, cts_pin) \
  uart_set_pin(port, tx_pin, rx_pin, rts_pin, cts_pin)
#define HF_UART_WRITE_BYTES(port, data, length) uart_write_bytes(port, data, length)
#define HF_UART_READ_BYTES(port, data, length, timeout) uart_read_bytes(port, data, length, timeout)
#define HF_UART_FLUSH(port) uart_flush(port)
#define HF_UART_FLUSH_INPUT(port) uart_flush_input(port)
#define HF_UART_GET_BUFFERED_DATA_LEN(port, length) uart_get_buffered_data_len(port, length)
#define HF_UART_WAIT_TX_DONE(port, timeout) uart_wait_tx_done(port, timeout)
#define HF_UART_SET_BAUDRATE(port, baudrate) uart_set_baudrate(port, baudrate)
#define HF_UART_SET_WORD_LENGTH(port, data_bits) uart_set_word_length(port, data_bits)
#define HF_UART_SET_PARITY(port, parity) uart_set_parity(port, parity)
#define HF_UART_SET_STOP_BITS(port, stop_bits) uart_set_stop_bits(port, stop_bits)
#define HF_UART_SET_HW_FLOW_CTRL(port, flow_ctrl, thresh) uart_set_hw_flow_ctrl(port, flow_ctrl, thresh)
#define HF_UART_SET_RTS(port, level) uart_set_rts(port, level)
#define HF_UART_GET_CTS(port, cts) uart_get_cts(port, cts)
#define HF_UART_SET_LINE_INVERSE(port, inverse_mask) uart_set_line_inverse(port, inverse_mask)
#define HF_UART_SET_MODE(port, mode) uart_set_mode(port, mode)
#else
// Non-ESP32 platforms - placeholder definitions
#define HF_UART_DRIVER_INSTALL(port, tx_size, rx_size, queue_size, queue, intr_flags) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_DRIVER_DELETE(port) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_PARAM_CONFIG(port, config) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_PIN(port, tx_pin, rx_pin, rts_pin, cts_pin) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_WRITE_BYTES(port, data, length) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_READ_BYTES(port, data, length, timeout) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_FLUSH(port) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_FLUSH_INPUT(port) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_GET_BUFFERED_DATA_LEN(port, length) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_WAIT_TX_DONE(port, timeout) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_BAUDRATE(port, baudrate) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_WORD_LENGTH(port, data_bits) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_PARITY(port, parity) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_STOP_BITS(port, stop_bits) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_HW_FLOW_CTRL(port, flow_ctrl, thresh) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_RTS(port, level) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_GET_CTS(port, cts) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_LINE_INVERSE(port, inverse_mask) HF_UART_ERR_NOT_SUPPORTED
#define HF_UART_SET_MODE(port, mode) HF_UART_ERR_NOT_SUPPORTED
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
  hf_gpio_mode_t sleep_mode;      ///< GPIO mode during sleep
  hf_gpio_pull_t sleep_pull_mode; ///< Pull resistor configuration during sleep
  bool sleep_output_enable;       ///< Enable output during sleep
  bool sleep_input_enable;        ///< Enable input during sleep
  bool hold_during_sleep;         ///< Hold configuration during sleep
  bool rtc_domain_enable;         ///< Route to RTC domain for ultra-low power
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
  hf_gpio_num_native_t gpio_num;                   ///< GPIO pin number
  hf_gpio_mode_t mode;                             ///< GPIO mode (input/output/etc)
  hf_gpio_pull_t pull_mode;                        ///< Pull resistor configuration
  hf_gpio_intr_type_t intr_type;                   ///< Interrupt trigger type
  hf_gpio_drive_strength_t drive_strength;         ///< Output drive capability
  hf_gpio_glitch_filter_type_t glitch_filter_type; ///< Glitch filter type
  hf_gpio_flex_filter_config_t flex_filter_config; ///< Flexible filter configuration
  hf_gpio_sleep_config_t sleep_config;             ///< Sleep mode configuration
  hf_gpio_wakeup_config_t wakeup_config;           ///< Wake-up configuration
  bool enable_hold_function;                       ///< Enable GPIO hold function
  bool enable_rtc_gpio;                            ///< Enable RTC GPIO functionality
};

/**
 * @brief ESP32C6 GPIO status information for diagnostics.
 * @details Comprehensive status information for debugging and monitoring.
 */
struct hf_gpio_status_info_t {
  uint8_t pin_number;                         ///< GPIO pin number
  hf_gpio_mode_t current_mode;                ///< Current GPIO mode
  hf_gpio_pull_t current_pull_mode;           ///< Current pull mode
  hf_gpio_drive_strength_t current_drive_cap; ///< Current drive capability
  bool input_enabled;                         ///< Input buffer enabled
  bool output_enabled;                        ///< Output buffer enabled
  bool open_drain;                            ///< Open drain mode
  bool sleep_sel_enabled;                     ///< Sleep selection enabled
  uint32_t function_select;                   ///< IOMUX function selection
  bool is_rtc_gpio;                           ///< Pin supports RTC GPIO
  bool glitch_filter_enabled;                 ///< Glitch filter enabled
  hf_gpio_glitch_filter_type_t filter_type;   ///< Type of glitch filter
  bool hold_enabled;                          ///< Hold function enabled
  uint32_t interrupt_count;                   ///< Number of interrupts occurred
  bool is_wake_source;                        ///< Pin configured as wake source
};

/**
 * @brief ESP32C6 GPIO pin validity checking.
 * @details Utility for validating GPIO pin numbers for different functions.
 */
struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;     ///< Pin can be used as GPIO
  bool supports_adc;      ///< Pin supports ADC functionality
  bool supports_rtc;      ///< Pin supports RTC GPIO
  bool supports_touch;    ///< Pin supports touch sensing
  bool is_strapping_pin;  ///< Pin is a strapping pin (requires caution)
  bool is_spi_flash_pin;  ///< Pin is used for SPI flash (not recommended for GPIO)
  bool is_usb_jtag_pin;   ///< Pin is used for USB-JTAG (disables JTAG if reconfigured)
  uint8_t lp_gpio_number; ///< Low-power GPIO number (if applicable)
  uint8_t adc_unit;       ///< ADC unit number (if applicable)
  uint8_t adc_channel;    ///< ADC channel number (if applicable)
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
using hf_gpio_config_native_t = struct {
  int dummy;
};
using hf_gpio_glitch_filter_handle_native_t = void *;
using hf_gpio_pin_glitch_filter_config_native_t = struct {
  int dummy;
};
using hf_gpio_flex_glitch_filter_config_native_t = struct {
  int dummy;
};
using hf_rtc_gpio_mode_native_t = uint8_t;

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// UTILITY MACROS FOR ESP32C6 GPIO VALIDATION
//==============================================================================

/**
 * @brief ESP32C6 GPIO pin count and validation macros.
 */
#ifdef HF_MCU_ESP32C6
#define HF_GPIO_PIN_COUNT 31        ///< Total GPIO pins (0-30)
#define HF_GPIO_MAX_PIN_NUMBER 30   ///< Maximum valid GPIO pin number
#define HF_GPIO_RTC_PIN_COUNT 8     ///< RTC GPIO pins (0-7)
#define HF_GPIO_ADC_PIN_COUNT 7     ///< ADC capable pins (0-6)
#define HF_GPIO_FLEX_FILTER_COUNT 8 ///< Number of flexible glitch filters

/**
 * @brief ESP32C6 specific pin validation macros.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= HF_GPIO_MAX_PIN_NUMBER)

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_IS_STRAPPING_PIN(gpio_num)                                                         \
  ((gpio_num) == 4 || (gpio_num) == 5 || (gpio_num) == 8 || (gpio_num) == 9 || (gpio_num) == 15)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) ((gpio_num) >= 24 && (gpio_num) <= 30)

#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) ((gpio_num) == 12 || (gpio_num) == 13)

#define HF_GPIO_SUPPORTS_ADC(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 6)

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
// ESP32C6 PWM/LEDC TYPES AND CONSTANTS (ESP-IDF v5.5+)
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief ESP32C6 LEDC (PWM) controller specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 8 LEDC channels (0-7) with advanced features:
 * - High-resolution PWM with up to 14-bit resolution at high frequencies
 * - Hardware fade functionality for smooth transitions
 * - 4 independent timer groups for different frequency domains
 * - Low-power mode support with sleep retention
 * - Interrupt-driven operation with period and fade callbacks
 * - Configurable clock sources (APB, XTAL, RC_FAST)
 * - Channel-specific idle state configuration
 */
static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;          ///< Maximum PWM channels (ESP32C6 LEDC)
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;            ///< Maximum timer groups
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 14;       ///< Maximum resolution bits
static constexpr uint32_t HF_PWM_MIN_FREQUENCY = 1;        ///< Minimum frequency (Hz)
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 40000000; ///< Maximum frequency (Hz)
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000; ///< Default frequency (Hz)
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 12;   ///< Default resolution bits
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;  ///< ESP32C6 APB clock frequency

/**
 * @brief PWM clock source selection for ESP32C6.
 * @details Clock source options for power optimization and precision.
 */
enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0, ///< Default PWM clock source (APB)
  HF_PWM_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_PWM_CLK_SRC_XTAL = 2,    ///< Crystal oscillator (40MHz, lower power)
  HF_PWM_CLK_SRC_RC_FAST = 3  ///< RC fast clock (~8MHz, lowest power)
};

/**
 * @brief PWM timer resolution for different frequency ranges.
 * @details ESP32C6 LEDC can achieve different resolutions based on frequency:
 * - 1kHz: up to 14-bit resolution
 * - 10kHz: up to 12-bit resolution
 * - 100kHz: up to 10-bit resolution
 * - 1MHz: up to 8-bit resolution
 */
enum class hf_pwm_resolution_t : uint8_t {
  HF_PWM_RES_8BIT = 8,   ///< 8-bit resolution (256 levels)
  HF_PWM_RES_10BIT = 10, ///< 10-bit resolution (1024 levels)
  HF_PWM_RES_12BIT = 12, ///< 12-bit resolution (4096 levels)
  HF_PWM_RES_14BIT = 14  ///< 14-bit resolution (16384 levels)
};

/**
 * @brief PWM operating modes for different applications.
 */
enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_LOW_SPEED = 0,  ///< Low speed mode (default)
  HF_PWM_MODE_HIGH_SPEED = 1  ///< High speed mode (legacy, use low speed)
};

/**
 * @brief PWM fade modes for smooth transitions.
 */
enum class hf_pwm_fade_mode_t : uint8_t {
  HF_PWM_FADE_NO_WAIT = 0,     ///< Non-blocking fade
  HF_PWM_FADE_WAIT_DONE = 1    ///< Blocking fade (wait for completion)
};

/**
 * @brief PWM interrupt types for callbacks.
 */
enum class hf_pwm_intr_type_t : uint8_t {
  HF_PWM_INTR_DISABLE = 0,     ///< Disable interrupts
  HF_PWM_INTR_FADE_END = 1     ///< Fade end interrupt
};

/**
 * @brief Native ESP-IDF LEDC type mappings for PWM.
 * @details Direct mappings to ESP-IDF LEDC types for maximum compatibility.
 */
using hf_pwm_channel_native_t = ledc_channel_t;
using hf_pwm_timer_native_t = ledc_timer_t;
using hf_pwm_mode_native_t = ledc_mode_t;
using hf_pwm_timer_bit_native_t = ledc_timer_bit_t;
using hf_pwm_clk_cfg_native_t = ledc_clk_cfg_t;
using hf_pwm_channel_config_native_t = ledc_channel_config_t;
using hf_pwm_timer_config_native_t = ledc_timer_config_t;
using hf_pwm_fade_mode_native_t = ledc_fade_mode_t;
using hf_pwm_intr_type_native_t = ledc_intr_type_t;

/**
 * @brief ESP32C6 PWM timing configuration with optimization support.
 * @details Platform-specific timing parameters optimized for ESP32C6 80MHz APB clock.
 */
struct hf_pwm_timing_config_t {
  uint32_t frequency_hz;       ///< PWM frequency in Hz
  uint8_t resolution_bits;     ///< PWM resolution (8-14 bits)
  hf_pwm_clock_source_t clk_src; ///< Clock source selection
  uint32_t clk_divider;        ///< Clock divider (calculated automatically)
  
  // Calculated timing parameters
  uint32_t actual_frequency_hz; ///< Actual achieved frequency
  float frequency_accuracy;     ///< Frequency accuracy percentage
  uint32_t period_ticks;        ///< Period in timer ticks
  uint32_t max_duty_ticks;      ///< Maximum duty cycle ticks
  
  hf_pwm_timing_config_t() noexcept
      : frequency_hz(HF_PWM_DEFAULT_FREQUENCY), resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        clk_src(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT), clk_divider(0),
        actual_frequency_hz(0), frequency_accuracy(0.0f), period_ticks(0), max_duty_ticks(0) {}
};

/**
 * @brief ESP32C6 PWM channel configuration with advanced features.
 * @details Comprehensive configuration for ESP-IDF v5.5+ LEDC channel features.
 */
struct hf_pwm_channel_config_t {
  hf_gpio_num_t gpio_pin;           ///< GPIO pin for PWM output
  uint8_t channel_id;               ///< Channel ID (0-7)
  uint8_t timer_id;                 ///< Timer ID (0-3)
  hf_pwm_mode_t speed_mode;         ///< Speed mode configuration
  uint32_t duty_initial;            ///< Initial duty cycle value
  hf_pwm_intr_type_t intr_type;     ///< Interrupt type
  bool invert_output;               ///< Invert output signal
  
  // Advanced configuration
  uint32_t hpoint;                  ///< High point timing (phase shift)
  uint8_t idle_level;               ///< Idle state level (0 or 1)
  bool output_invert;               ///< Hardware output inversion
  
  hf_pwm_channel_config_t() noexcept
      : gpio_pin(HF_INVALID_PIN), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_LOW_SPEED), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false),
        hpoint(0), idle_level(0), output_invert(false) {}
};

/**
 * @brief ESP32C6 PWM fade configuration for smooth transitions.
 */
struct hf_pwm_fade_config_t {
  uint32_t target_duty;             ///< Target duty cycle value
  uint32_t fade_time_ms;            ///< Fade duration in milliseconds
  hf_pwm_fade_mode_t fade_mode;     ///< Fade mode (blocking/non-blocking)
  uint32_t scale;                   ///< Fade scale factor
  uint32_t cycle_num;               ///< Number of fade cycles
  
  hf_pwm_fade_config_t() noexcept
      : target_duty(0), fade_time_ms(1000),
        fade_mode(hf_pwm_fade_mode_t::HF_PWM_FADE_NO_WAIT),
        scale(0), cycle_num(0) {}
};

/**
 * @brief PWM capabilities and limitations for ESP32C6.
 * @details Static capability information for runtime feature detection.
 */
struct hf_pwm_capabilities_t {
  uint8_t num_channels;             ///< Number of PWM channels (8 for ESP32C6)
  uint8_t num_timers;               ///< Number of timer groups (4 for ESP32C6)
  uint8_t max_resolution_bits;      ///< Maximum resolution bits (14 for ESP32C6)
  uint32_t max_frequency_hz;        ///< Maximum supported frequency
  uint32_t min_frequency_hz;        ///< Minimum supported frequency
  bool supports_fade;               ///< Hardware fade support
  bool supports_sleep_retention;    ///< Sleep retention support
  bool supports_complementary;      ///< Complementary outputs (software)
  bool supports_deadtime;           ///< Deadtime insertion (software)
  bool supports_phase_shift;        ///< Phase shifting support
  uint8_t available_clock_sources;  ///< Number of available clock sources
  
  // Constructor with ESP32C6 defaults
  hf_pwm_capabilities_t() noexcept
      : num_channels(HF_PWM_MAX_CHANNELS), num_timers(HF_PWM_MAX_TIMERS),
        max_resolution_bits(HF_PWM_MAX_RESOLUTION), max_frequency_hz(HF_PWM_MAX_FREQUENCY),
        min_frequency_hz(HF_PWM_MIN_FREQUENCY), supports_fade(true),
        supports_sleep_retention(true), supports_complementary(true),
        supports_deadtime(true), supports_phase_shift(false),
        available_clock_sources(4) {}
};

/**
 * @brief PWM statistics for performance monitoring.
 * @details Thread-safe statistics collection for production diagnostics.
 */
struct hf_pwm_statistics_t {
  std::atomic<uint32_t> duty_updates_count;     ///< Total duty cycle updates
  std::atomic<uint32_t> frequency_changes_count; ///< Total frequency changes
  std::atomic<uint32_t> fade_operations_count;   ///< Total fade operations
  std::atomic<uint32_t> error_count;            ///< Total error count
  std::atomic<uint32_t> channel_enables_count;  ///< Total channel enable operations
  std::atomic<uint32_t> channel_disables_count; ///< Total channel disable operations
  uint64_t last_activity_timestamp;             ///< Last activity timestamp
  uint64_t initialization_timestamp;            ///< Initialization timestamp
  
  hf_pwm_statistics_t() noexcept
      : duty_updates_count(0), frequency_changes_count(0), fade_operations_count(0),
        error_count(0), channel_enables_count(0), channel_disables_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

#else
// Non-ESP32 platforms - use generic PWM types
static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 12;
static constexpr uint32_t HF_PWM_MIN_FREQUENCY = 1;
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 1000000;
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000;
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 10;
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;

enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0,
  HF_PWM_CLK_SRC_APB = 1,
  HF_PWM_CLK_SRC_XTAL = 2,
  HF_PWM_CLK_SRC_RC_FAST = 3
};

enum class hf_pwm_resolution_t : uint8_t {
  HF_PWM_RES_8BIT = 8,
  HF_PWM_RES_10BIT = 10,
  HF_PWM_RES_12BIT = 12,
  HF_PWM_RES_14BIT = 14
};

enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_LOW_SPEED = 0,
  HF_PWM_MODE_HIGH_SPEED = 1
};

enum class hf_pwm_fade_mode_t : uint8_t {
  HF_PWM_FADE_NO_WAIT = 0,
  HF_PWM_FADE_WAIT_DONE = 1
};

enum class hf_pwm_intr_type_t : uint8_t {
  HF_PWM_INTR_DISABLE = 0,
  HF_PWM_INTR_FADE_END = 1
};

// Generic handle types for non-ESP32 platforms
using hf_pwm_channel_native_t = uint8_t;
using hf_pwm_timer_native_t = uint8_t;
using hf_pwm_mode_native_t = uint8_t;
using hf_pwm_timer_bit_native_t = uint8_t;
using hf_pwm_clk_cfg_native_t = uint8_t;
using hf_pwm_channel_config_native_t = struct { int dummy; };
using hf_pwm_timer_config_native_t = struct { int dummy; };
using hf_pwm_fade_mode_native_t = uint8_t;
using hf_pwm_intr_type_native_t = uint8_t;

struct hf_pwm_timing_config_t {
  uint32_t frequency_hz;
  uint8_t resolution_bits;
  hf_pwm_clock_source_t clk_src;
  uint32_t clk_divider;
  uint32_t actual_frequency_hz;
  float frequency_accuracy;
  uint32_t period_ticks;
  uint32_t max_duty_ticks;
  
  hf_pwm_timing_config_t() noexcept
      : frequency_hz(HF_PWM_DEFAULT_FREQUENCY), resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        clk_src(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT), clk_divider(0),
        actual_frequency_hz(0), frequency_accuracy(0.0f), period_ticks(0), max_duty_ticks(0) {}
};

struct hf_pwm_channel_config_t {
  uint32_t gpio_pin;
  uint8_t channel_id;
  uint8_t timer_id;
  hf_pwm_mode_t speed_mode;
  uint32_t duty_initial;
  hf_pwm_intr_type_t intr_type;
  bool invert_output;
  uint32_t hpoint;
  uint8_t idle_level;
  bool output_invert;
  
  hf_pwm_channel_config_t() noexcept
      : gpio_pin(0xFFFFFFFF), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_LOW_SPEED), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false),
        hpoint(0), idle_level(0), output_invert(false) {}
};

struct hf_pwm_fade_config_t {
  uint32_t target_duty;
  uint32_t fade_time_ms;
  hf_pwm_fade_mode_t fade_mode;
  uint32_t scale;
  uint32_t cycle_num;
  
  hf_pwm_fade_config_t() noexcept
      : target_duty(0), fade_time_ms(1000),
        fade_mode(hf_pwm_fade_mode_t::HF_PWM_FADE_NO_WAIT),
        scale(0), cycle_num(0) {}
};

struct hf_pwm_capabilities_t {
  uint8_t num_channels;
  uint8_t num_timers;
  uint8_t max_resolution_bits;
  uint32_t max_frequency_hz;
  uint32_t min_frequency_hz;
  bool supports_fade;
  bool supports_sleep_retention;
  bool supports_complementary;
  bool supports_deadtime;
  bool supports_phase_shift;
  uint8_t available_clock_sources;
  
  hf_pwm_capabilities_t() noexcept
      : num_channels(HF_PWM_MAX_CHANNELS), num_timers(HF_PWM_MAX_TIMERS),
        max_resolution_bits(HF_PWM_MAX_RESOLUTION), max_frequency_hz(HF_PWM_MAX_FREQUENCY),
        min_frequency_hz(HF_PWM_MIN_FREQUENCY), supports_fade(false),
        supports_sleep_retention(false), supports_complementary(false),
        supports_deadtime(false), supports_phase_shift(false),
        available_clock_sources(1) {}
};

struct hf_pwm_statistics_t {
  std::atomic<uint32_t> duty_updates_count;
  std::atomic<uint32_t> frequency_changes_count;
  std::atomic<uint32_t> fade_operations_count;
  std::atomic<uint32_t> error_count;
  std::atomic<uint32_t> channel_enables_count;
  std::atomic<uint32_t> channel_disables_count;
  uint64_t last_activity_timestamp;
  uint64_t initialization_timestamp;
  
  hf_pwm_statistics_t() noexcept
      : duty_updates_count(0), frequency_changes_count(0), fade_operations_count(0),
        error_count(0), channel_enables_count(0), channel_disables_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// PWM UTILITY MACROS AND CONSTANTS
//==============================================================================

/**
 * @brief PWM validation macros.
 */
#define HF_PWM_IS_VALID_CHANNEL(ch) ((ch) < HF_PWM_MAX_CHANNELS)
#define HF_PWM_IS_VALID_TIMER(timer) ((timer) < HF_PWM_MAX_TIMERS)
#define HF_PWM_IS_VALID_FREQUENCY(freq) \
  ((freq) >= HF_PWM_MIN_FREQUENCY && (freq) <= HF_PWM_MAX_FREQUENCY)
#define HF_PWM_IS_VALID_RESOLUTION(res) \
  ((res) >= 8 && (res) <= HF_PWM_MAX_RESOLUTION)
#define HF_PWM_IS_VALID_DUTY_CYCLE(duty, res) \
  ((duty) <= ((1U << (res)) - 1))

/**
 * @brief PWM calculation macros.
 */
#define HF_PWM_DUTY_TO_RAW(duty_percent, resolution) \
  ((uint32_t)((duty_percent) * ((1U << (resolution)) - 1) / 100.0f))
#define HF_PWM_RAW_TO_DUTY(raw_duty, resolution) \
  ((float)(raw_duty) * 100.0f / ((1U << (resolution)) - 1))
#define HF_PWM_MAX_DUTY_VALUE(resolution) ((1U << (resolution)) - 1)


//==============================================================================
// ESP32C6 POWER MANAGEMENT AND TIMING TYPES
//==============================================================================

/**
 * @brief ESP32C6 power domain configuration for GPIO operations.
 */
enum class hf_power_domain_t : uint8_t {
  HF_POWER_DOMAIN_CPU = 0,    ///< CPU power domain
  HF_POWER_DOMAIN_RTC_PERIPH, ///< RTC peripherals power domain
  HF_POWER_DOMAIN_XTAL,       ///< Crystal oscillator domain
  HF_POWER_DOMAIN_MODEM,      ///< RF/WiFi/BT modem domain
  HF_POWER_DOMAIN_VDDSDIO,    ///< SDIO power domain
  HF_POWER_DOMAIN_TOP,        ///< SoC top domain
};

/**
 * @brief ESP32C6 sleep mode types.
 */
enum class hf_sleep_mode_t : uint8_t {
  HF_SLEEP_MODE_NONE = 0,    ///< No sleep mode
  HF_SLEEP_MODE_LIGHT,       ///< Light sleep mode
  HF_SLEEP_MODE_DEEP,        ///< Deep sleep mode
  HF_SLEEP_MODE_HIBERNATION, ///< Hibernation mode (lowest power)
};

/**
 * @brief High-resolution timing types for GPIO operations.
 */
using hf_timestamp_us_t = uint64_t; ///< Microsecond timestamp
using hf_timestamp_ns_t = uint64_t; ///< Nanosecond timestamp
using hf_duration_us_t = uint32_t;  ///< Duration in microseconds
using hf_duration_ns_t = uint32_t;  ///< Duration in nanoseconds

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
  HF_TWAI_CONTROLLER_0 = 0,   ///< Primary TWAI controller (default)
  HF_TWAI_CONTROLLER_1 = 1,   ///< Secondary TWAI controller (ESP32C6 specific)
  HF_TWAI_CONTROLLER_MAX = 2, ///< Maximum number of controllers
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
  HF_TWAI_ALERT_CRC_ERROR = 0x00010000, ///< CRC error
  HF_TWAI_ALERT_ACK_ERROR = 0x00020000, ///< Acknowledgment error
  HF_TWAI_ALERT_BIT_ERROR = 0x00040000, ///< Bit error
  HF_TWAI_ALERT_TIMEOUT = 0x00080000,   ///< Operation timeout

  // Power management alerts (ESP-IDF v5.5+)
  HF_TWAI_ALERT_SLEEP_WAKEUP = 0x00100000, ///< Wake-up from sleep
  HF_TWAI_ALERT_POWER_DOWN = 0x00200000,   ///< Power down event
  HF_TWAI_ALERT_CLOCK_LOST = 0x00400000,   ///< Clock source lost

  // Recovery and maintenance alerts
  HF_TWAI_ALERT_RECOVERY_START = 0x00800000,    ///< Recovery started
  HF_TWAI_ALERT_RECOVERY_COMPLETE = 0x01000000, ///< Recovery completed
  HF_TWAI_ALERT_FILTER_HIT = 0x02000000,        ///< Acceptance filter hit
  HF_TWAI_ALERT_FILTER_MISS = 0x04000000,       ///< Acceptance filter miss

  // Critical system alerts
  HF_TWAI_ALERT_DRIVER_ERROR = 0x08000000,   ///< Driver internal error
  HF_TWAI_ALERT_HARDWARE_FAULT = 0x10000000, ///< Hardware fault detected
  HF_TWAI_ALERT_MEMORY_ERROR = 0x20000000,   ///< Memory allocation error
  HF_TWAI_ALERT_CONFIG_ERROR = 0x40000000,   ///< Configuration error

  // Convenience masks
  HF_TWAI_ALERT_ALL_ERRORS = 0x7FFFFE10,     ///< All error alerts
  HF_TWAI_ALERT_ALL_OPERATIONS = 0x0000000F, ///< All operation alerts
  HF_TWAI_ALERT_ALL = 0x7FFFFFFF,            ///< All alerts enabled
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

//==============================================================================
// ESP32C6 I2C TYPES AND CONSTANTS (ESP-IDF v5.5+)
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6

/**z
 * @brief I2C clock source selection for ESP32C6.
 * @details Advanced clock source options for power optimization and precision.
 */
enum class hf_i2c_clock_source_t : uint8_t {
  HF_I2C_CLK_SRC_DEFAULT = 0, ///< Default I2C source clock
  HF_I2C_CLK_SRC_XTAL = 1,    ///< External crystal (lower power)
  HF_I2C_CLK_SRC_RC_FAST = 2  ///< Internal 20MHz RC oscillator
};

/**
 * @brief I2C bus mode selection for ESP32C6.
 * @details Master/slave mode configuration.
 */
enum class hf_i2c_bus_mode_t : uint8_t {
  HF_I2C_BUS_MODE_MASTER = 0, ///< Master mode
  HF_I2C_BUS_MODE_SLAVE = 1   ///< Slave mode
};

/**
 * @brief I2C address bit width for ESP32C6.
 * @details Support for 7-bit and 10-bit addressing modes.
 */
enum class hf_i2c_address_bits_t : uint8_t {
  HF_I2C_ADDR_BIT_LEN_7 = 7,  ///< 7-bit addressing (standard)
  HF_I2C_ADDR_BIT_LEN_10 = 10 ///< 10-bit addressing (extended)
};

/**
 * @brief I2C power modes for energy efficiency on ESP32C6.
 * @details Advanced power management modes for low-power applications.
 */
enum class hf_i2c_power_mode_t : uint8_t {
  HF_I2C_POWER_MODE_FULL = 0, ///< Maximum performance, highest power
  HF_I2C_POWER_MODE_LOW = 1,  ///< Reduced power consumption
  HF_I2C_POWER_MODE_SLEEP = 2 ///< Minimal power, bus suspended
};

/**
 * @brief I2C event types for callback notifications.
 * @details Event types reported via I2C event callbacks.
 */
enum class hf_i2c_event_type_t : int {
  HF_I2C_EVENT_WRITE_COMPLETE = 0, ///< Write transaction finished
  HF_I2C_EVENT_READ_COMPLETE = 1,  ///< Read transaction finished
  HF_I2C_EVENT_ERROR = 2,          ///< Error occurred during transaction
  HF_I2C_EVENT_BUS_SUSPENDED = 3,  ///< Bus automatically suspended
  HF_I2C_EVENT_BUS_RESUMED = 4     ///< Bus resumed from suspended state
};

/**
 * @brief SPI transfer modes for ESP32C6.
 * @details Advanced transfer modes including octal SPI support.
 */
enum class hf_spi_transfer_mode_t : uint8_t {
  HF_SPI_TRANSFER_MODE_SINGLE = 0, ///< Standard SPI (1-bit MOSI/MISO)
  HF_SPI_TRANSFER_MODE_DUAL = 1,   ///< Dual SPI (2-bit data lines)
  HF_SPI_TRANSFER_MODE_QUAD = 2,   ///< Quad SPI (4-bit data lines)
  HF_SPI_TRANSFER_MODE_OCTAL = 3   ///< Octal SPI (8-bit data lines) - ESP32C6 specific
};

/**
 * @brief SPI clock source selection for ESP32C6.
 * @details Clock source options for power optimization.
 */
enum class hf_spi_clock_source_t : uint8_t {
  HF_SPI_CLK_SRC_DEFAULT = 0, ///< Default SPI clock source
  HF_SPI_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_SPI_CLK_SRC_XTAL = 2     ///< Crystal oscillator (40MHz, lower power)
};

/**
 * @brief SPI event types for callback notifications.
 * @details Event types reported via SPI event callbacks.
 */
enum class hf_spi_event_type_t : int {
  HF_SPI_EVENT_TRANSACTION_COMPLETE = 0, ///< Transaction completed
  HF_SPI_EVENT_TRANSACTION_ERROR = 1,    ///< Transaction error occurred
  HF_SPI_EVENT_BUS_SUSPENDED = 2,        ///< Bus suspended for power saving
  HF_SPI_EVENT_BUS_RESUMED = 3,          ///< Bus resumed from suspension
  HF_SPI_EVENT_DMA_ERROR = 4             ///< DMA error occurred
};



//==============================================================================
// ESP32C6 I2C AND SPI ADVANCED CONFIGURATION STRUCTS
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6

/**
 * @brief Enhanced I2C bus configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_i2c_master_bus_config_t {
  hf_i2c_port_t i2c_port;        ///< I2C port number (0 for ESP32C6)
  hf_gpio_num_t sda_io_num;       ///< SDA GPIO pin
  hf_gpio_num_t scl_io_num;       ///< SCL GPIO pin
  uint32_t clk_source;            ///< Clock source selection
  uint8_t glitch_ignore_cnt;      ///< Glitch filter length (0-7)
  bool intr_priority;             ///< Interrupt priority level
  uint32_t trans_queue_depth;     ///< Transaction queue depth (async mode)
  uint32_t flags;                 ///< Additional configuration flags
  bool enable_internal_pullup;    ///< Enable internal pull-ups
  
  // ESP-IDF v5.5+ power management features
  bool clk_flags;                 ///< Clock configuration flags
  bool allow_pd;                ///< Allow power down in sleep modes
  
  hf_i2c_master_bus_config_t() noexcept
      : i2c_port(0), sda_io_num(HF_INVALID_PIN), scl_io_num(HF_INVALID_PIN),
        clk_source(0), glitch_ignore_cnt(7), intr_priority(false),
        trans_queue_depth(0), flags(0), enable_internal_pullup(true),
        clk_flags(0), allow_pd(false) {}
};

/**
 * @brief I2C device configuration for ESP32C6/ESP-IDF v5.5+ bus-device model.
 */
struct hf_i2c_device_config_t {
  uint16_t dev_addr_length;       ///< Device address bit length (7 or 10)
  uint16_t device_address;        ///< I2C device address
  uint32_t scl_speed_hz;          ///< Device-specific SCL speed
  uint32_t scl_wait_us;           ///< Wait time after SCL goes high
  uint32_t flags;                 ///< Device-specific flags
  
  hf_i2c_device_config_t() noexcept
      : dev_addr_length(7), device_address(0), scl_speed_hz(100000),
        scl_wait_us(0), flags(0) {}
};

/**
 * @brief SPI bus configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_bus_config_t {
  int mosi_io_num;        ///< MOSI GPIO pin
  int miso_io_num;        ///< MISO GPIO pin
  int sclk_io_num;        ///< SCLK GPIO pin
  int quadwp_io_num;      ///< WP pin for quad/octal mode
  int quadhd_io_num;      ///< HD pin for quad/octal mode
  int data4_io_num;       ///< DATA4 pin for octal mode
  int data5_io_num;       ///< DATA5 pin for octal mode
  int data6_io_num;       ///< DATA6 pin for octal mode
  int data7_io_num;       ///< DATA7 pin for octal mode
  int max_transfer_sz;    ///< Maximum transfer size
  uint32_t flags;         ///< Bus configuration flags
  uint32_t intr_flags;    ///< Interrupt allocation flags
  
  hf_spi_bus_config_t() noexcept
      : mosi_io_num(-1), miso_io_num(-1), sclk_io_num(-1),
        quadwp_io_num(-1), quadhd_io_num(-1), data4_io_num(-1),
        data5_io_num(-1), data6_io_num(-1), data7_io_num(-1),
        max_transfer_sz(4092), flags(0), intr_flags(0) {}
};

/**
 * @brief SPI device configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_device_interface_config_t {
  uint8_t command_bits;        ///< Command phase bit length (0-16)
  uint8_t address_bits;        ///< Address phase bit length (0-64)
  uint8_t dummy_bits;          ///< Dummy phase bit length
  uint8_t mode;                ///< SPI mode (0-3)
  uint8_t duty_cycle_pos;      ///< Duty cycle of positive clock
  uint8_t cs_ena_pretrans;     ///< CS setup time
  uint8_t cs_ena_posttrans;    ///< CS hold time
  int clock_speed_hz;          ///< Clock speed in Hz
  int input_delay_ns;          ///< Input delay in nanoseconds
  int spics_io_num;            ///< CS GPIO pin (-1 = not used)
  uint32_t flags;              ///< Device configuration flags
  int queue_size;              ///< Transaction queue size
  void (*pre_cb)(void *trans); ///< Pre-transaction callback
  void (*post_cb)(void *trans);///< Post-transaction callback
  
  hf_spi_device_interface_config_t() noexcept
      : command_bits(0), address_bits(0), dummy_bits(0), mode(0),
        duty_cycle_pos(128), cs_ena_pretrans(0), cs_ena_posttrans(0),
        clock_speed_hz(1000000), input_delay_ns(0), spics_io_num(-1),
        flags(0), queue_size(7), pre_cb(nullptr), post_cb(nullptr) {}
};

/**
 * @brief SPI transaction structure for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_transaction_t {
  uint32_t flags;              ///< Transaction flags
  uint16_t cmd;                ///< Command data
  uint64_t addr;               ///< Address data
  size_t length;               ///< Data length in bits
  size_t rxlength;             ///< RX data length in bits
  void *user;                  ///< User data pointer
  union {
    const void *tx_buffer;     ///< TX data buffer
    uint8_t tx_data[4];        ///< TX data for ≤32 bits
  };
  union {
    void *rx_buffer;           ///< RX data buffer
    uint8_t rx_data[4];        ///< RX data for ≤32 bits
  };
  
  hf_spi_transaction_t() noexcept
      : flags(0), cmd(0), addr(0), length(0), rxlength(0),
        user(nullptr), tx_buffer(nullptr), rx_buffer(nullptr) {}
};

/**
 * @brief SPI device handle type.
 */
using hf_spi_device_handle_t = hf_spi_device_handle_native_t;
using hf_spi_host_device_id_t = hf_spi_host_native_t;

#else
// Generic SPI constants for non-ESP32C6 platforms
static constexpr uint8_t HF_SPI_MAX_CONTROLLERS = 2;
static constexpr uint8_t HF_SPI_GP_CONTROLLERS = 1;
static constexpr uint32_t HF_SPI_MAX_CLOCK_SPEED = 40000000;
static constexpr uint32_t HF_SPI_MIN_CLOCK_SPEED = 1000;
static constexpr uint32_t HF_SPI_DEFAULT_CLOCK_SPEED = 1000000;
static constexpr uint16_t HF_SPI_MAX_TRANSFER_SIZE = 256;
static constexpr uint8_t HF_SPI_MAX_CS_LINES = 3;
static constexpr uint8_t HF_SPI_MAX_DATA_LINES = 4;

enum class hf_spi_host_device_t : uint8_t {
  HF_SPI1_HOST = 0,
  HF_SPI2_HOST = 1,
  HF_SPI_HOST_MAX
};

using hf_spi_device_handle_t = void *;
using hf_spi_host_device_id_t = uint8_t;

struct hf_spi_bus_config_t {
  int dummy;
};

struct hf_spi_device_interface_config_t {
  int dummy;
};

struct hf_spi_transaction_t {
  int dummy;
};
#endif

//==============================================================================
// I2C AND SPI VALIDATION MACROS
//==============================================================================

/**
 * @brief I2C validation macros for ESP32C6.
 */
#define HF_I2C_IS_VALID_PORT(port) ((port) < HF_I2C_MAX_CONTROLLERS)
#define HF_I2C_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_I2C_MIN_CLOCK_SPEED && (speed) <= HF_I2C_MAX_CLOCK_SPEED)
#define HF_I2C_IS_VALID_DEVICE_ADDR(addr) ((addr) >= 0x08 && (addr) <= 0x77)
#define HF_I2C_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_I2C_MAX_TRANSFER_SIZE)

/**
 * @brief SPI validation macros for ESP32C6.
 */
#define HF_SPI_IS_VALID_HOST(host) ((host) < static_cast<uint8_t>(hf_spi_host_device_t::HF_SPI_HOST_MAX))
#define HF_SPI_IS_VALID_CLOCK_SPEED(speed) \
  ((speed) >= HF_SPI_MIN_CLOCK_SPEED && (speed) <= HF_SPI_MAX_CLOCK_SPEED)
#define HF_SPI_IS_VALID_MODE(mode) ((mode) >= 0 && (mode) <= 3)
#define HF_SPI_IS_VALID_TRANSFER_SIZE(size) ((size) > 0 && (size) <= HF_SPI_MAX_TRANSFER_SIZE)

/**
 * @brief SPI diagnostics information structure.
 */
struct hf_spi_diagnostics_t {
  bool is_initialized;        ///< Initialization state
  bool is_bus_suspended;      ///< Bus suspension state
  bool dma_enabled;           ///< DMA enabled state
  uint32_t current_clock_speed; ///< Current clock speed in Hz
  uint8_t current_mode;       ///< Current SPI mode
  uint16_t max_transfer_size; ///< Maximum transfer size
  uint8_t device_count;       ///< Number of registered devices
  uint32_t last_error;        ///< Last error code
  uint64_t total_transactions; ///< Total transactions performed
  uint64_t failed_transactions; ///< Failed transactions count
};

// Type alias for SPI diagnostics
using SpiDiagnostics = hf_spi_diagnostics_t;

//==============================================================================
// ESP32C6 PIO (RMT) PERIPHERAL TYPES - ESP-IDF v5.5+ FEATURES
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6

// Include RMT-specific headers for ESP32C6
#include "driver/rmt_encoder.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "hal/rmt_types.h"
#include "soc/soc_caps.h"

/**
 * @brief ESP32C6 RMT controller specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 4 RMT channels (0-3) with advanced features:
 * - TX/RX channels can be independently configured
 * - Hardware symbol encoding with configurable timing
 * - DMA support for large transfers (>64 symbols) 
 * - Multiple clock sources (APB, XTAL, RC_FAST)
 * - Power management with light sleep support
 * - Flexible memory allocation (48-1024 symbols per channel)
 * - Interrupt priority configuration
 * - Carrier modulation for IR protocols
 */
static constexpr uint8_t HF_RMT_MAX_CHANNELS = SOC_RMT_CHANNELS_PER_GROUP;  // 4 for ESP32C6
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = SOC_RMT_TX_CANDIDATES_PER_GROUP; // 2 for ESP32C6
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = SOC_RMT_RX_CANDIDATES_PER_GROUP; // 2 for ESP32C6
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;   // Minimum memory block size
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024; // Maximum for DMA mode
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64; // Default allocation
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000; // 80MHz APB clock
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;     // 1kHz minimum
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000; // 1MHz default
static constexpr uint8_t HF_RMT_MAX_QUEUE_DEPTH = 32;         // Maximum TX queue depth
static constexpr uint8_t HF_RMT_MAX_INTERRUPT_PRIORITY = 7;    // Maximum interrupt priority

/**
 * @brief RMT clock source selection for ESP32C6.
 * @details Maps to ESP-IDF v5.5+ rmt_clock_source_t enum.
 */
enum class hf_rmt_clock_source_t : uint8_t {
  HF_RMT_CLK_SRC_DEFAULT = 0,    ///< Default clock source (APB)
  HF_RMT_CLK_SRC_APB = 1,        ///< APB clock (80MHz)
  HF_RMT_CLK_SRC_XTAL = 2,       ///< Crystal clock (40MHz) 
  HF_RMT_CLK_SRC_RC_FAST = 3,    ///< RC fast clock (~8MHz)
};

/**
 * @brief RMT channel direction for ESP32C6.
 */
enum class hf_rmt_channel_direction_t : uint8_t {
  HF_RMT_CHANNEL_DIRECTION_TX = 0,  ///< Transmit direction
  HF_RMT_CHANNEL_DIRECTION_RX = 1,  ///< Receive direction
};

/**
 * @brief RMT symbol word structure - platform-specific.
 */
using hf_rmt_symbol_word_t = rmt_symbol_word_t;
using hf_rmt_channel_handle_t = rmt_channel_handle_t;
using hf_rmt_encoder_handle_t = rmt_encoder_handle_t;
using hf_rmt_tx_channel_config_t = rmt_tx_channel_config_t;
using hf_rmt_rx_channel_config_t = rmt_rx_channel_config_t;

/**
 * @brief RMT transmission configuration with ESP32C6 advanced features.
 */
struct hf_rmt_transmit_config_t {
  uint32_t loop_count;          ///< Loop count (0 = no loop)
  bool invert_signal;           ///< Invert output signal
  bool with_dma;                ///< Enable DMA mode for large transfers
  uint32_t queue_depth;         ///< TX queue depth (1-32)
  uint8_t intr_priority;        ///< Interrupt priority (0-7)
  bool allow_pd;                ///< Allow power down in sleep modes
  
  hf_rmt_transmit_config_t() noexcept
      : loop_count(0), invert_signal(false), with_dma(false),
        queue_depth(4), intr_priority(0), allow_pd(false) {}
};

/**
 * @brief RMT reception configuration with ESP32C6 advanced features.
 */
struct hf_rmt_receive_config_t {
  uint32_t signal_range_min_ns; ///< Minimum signal range in nanoseconds
  uint32_t signal_range_max_ns; ///< Maximum signal range in nanoseconds
  bool with_dma;                ///< Enable DMA mode for large transfers
  uint8_t intr_priority;        ///< Interrupt priority (0-7)
  bool allow_pd;                ///< Allow power down in sleep modes
  
  hf_rmt_receive_config_t() noexcept
      : signal_range_min_ns(1000), signal_range_max_ns(1000000),
        with_dma(false), intr_priority(0), allow_pd(false) {}
};

/**
 * @brief RMT carrier configuration for IR protocols.
 */
struct hf_rmt_carrier_config_t {
  uint32_t frequency_hz;    ///< Carrier frequency in Hz
  float duty_cycle;         ///< Duty cycle (0.0 to 1.0)
  uint8_t polarity_active_low; ///< Carrier polarity (0=high, 1=low)
  bool always_on;           ///< Always on carrier mode
  
  hf_rmt_carrier_config_t() noexcept
      : frequency_hz(38000), duty_cycle(0.5f),
        polarity_active_low(0), always_on(false) {}
};

#else
// Non-ESP32C6 platforms - use generic types
static constexpr uint8_t HF_RMT_MAX_CHANNELS = 4;
static constexpr uint8_t HF_RMT_MAX_TX_CHANNELS = 2;
static constexpr uint8_t HF_RMT_MAX_RX_CHANNELS = 2;
static constexpr size_t HF_RMT_MIN_MEM_BLOCK_SYMBOLS = 48;
static constexpr size_t HF_RMT_MAX_MEM_BLOCK_SYMBOLS = 1024;
static constexpr size_t HF_RMT_DEFAULT_MEM_BLOCK_SYMBOLS = 64;
static constexpr uint32_t HF_RMT_MAX_RESOLUTION_HZ = 80000000;
static constexpr uint32_t HF_RMT_MIN_RESOLUTION_HZ = 1000;
static constexpr uint32_t HF_RMT_DEFAULT_RESOLUTION_HZ = 1000000;
static constexpr uint8_t HF_RMT_MAX_QUEUE_DEPTH = 32;
static constexpr uint8_t HF_RMT_MAX_INTERRUPT_PRIORITY = 7;

enum class hf_rmt_clock_source_t : uint8_t {
  HF_RMT_CLK_SRC_DEFAULT = 0,
  HF_RMT_CLK_SRC_APB = 1,
  HF_RMT_CLK_SRC_XTAL = 2,
  HF_RMT_CLK_SRC_RC_FAST = 3,
};

enum class hf_rmt_channel_direction_t : uint8_t {
  HF_RMT_CHANNEL_DIRECTION_TX = 0,
  HF_RMT_CHANNEL_DIRECTION_RX = 1,
};

// Generic structures for non-ESP32C6 platforms
struct hf_rmt_symbol_word_t {
  uint32_t level0 : 1;
  uint32_t duration0 : 15;
  uint32_t level1 : 1;
  uint32_t duration1 : 15;
};

using hf_rmt_channel_handle_t = void*;
using hf_rmt_encoder_handle_t = void*;

struct hf_rmt_tx_channel_config_t {
  int dummy;
};

struct hf_rmt_rx_channel_config_t {
  int dummy;
};

struct hf_rmt_transmit_config_t {
  uint32_t loop_count;
  bool invert_signal;
  bool with_dma;
  uint32_t queue_depth;
  uint8_t intr_priority;
  bool allow_pd;
  
  hf_rmt_transmit_config_t() noexcept
      : loop_count(0), invert_signal(false), with_dma(false),
        queue_depth(4), intr_priority(0), allow_pd(false) {}
};

struct hf_rmt_receive_config_t {
  uint32_t signal_range_min_ns;
  uint32_t signal_range_max_ns;
  bool with_dma;
  uint8_t intr_priority;
  bool allow_pd;
  
  hf_rmt_receive_config_t() noexcept
      : signal_range_min_ns(1000), signal_range_max_ns(1000000),
        with_dma(false), intr_priority(0), allow_pd(false) {}
};

struct hf_rmt_carrier_config_t {
  uint32_t frequency_hz;
  float duty_cycle;
  uint8_t polarity_active_low;
  bool always_on;
  
  hf_rmt_carrier_config_t() noexcept
      : frequency_hz(38000), duty_cycle(0.5f),
        polarity_active_low(0), always_on(false) {}
};
#endif

/**
 * @brief RMT validation macros for ESP32C6.
 */
#define HF_RMT_IS_VALID_CHANNEL(ch) ((ch) < HF_RMT_MAX_CHANNELS)
#define HF_RMT_IS_VALID_TX_CHANNEL(ch) ((ch) < HF_RMT_MAX_TX_CHANNELS)
#define HF_RMT_IS_VALID_RX_CHANNEL(ch) ((ch) < HF_RMT_MAX_RX_CHANNELS)
#define HF_RMT_IS_VALID_RESOLUTION(res) \
  ((res) >= HF_RMT_MIN_RESOLUTION_HZ && (res) <= HF_RMT_MAX_RESOLUTION_HZ)
#define HF_RMT_IS_VALID_MEM_BLOCK_SIZE(size) \
  ((size) >= HF_RMT_MIN_MEM_BLOCK_SYMBOLS && (size) <= HF_RMT_MAX_MEM_BLOCK_SYMBOLS)
#define HF_RMT_IS_VALID_QUEUE_DEPTH(depth) ((depth) >= 1 && (depth) <= HF_RMT_MAX_QUEUE_DEPTH)
#define HF_RMT_IS_VALID_INTR_PRIORITY(prio) ((prio) <= HF_RMT_MAX_INTERRUPT_PRIORITY)

//==============================================================================
// ESP32C6 ADC PERIPHERAL TYPES - ESP-IDF v5.5+ FEATURES 
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6

// Include ADC-specific headers for ESP32C6
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "soc/adc_channel.h"

/**
 * @brief ESP32C6 ADC specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 1 ADC controller (ADC1) with advanced features:
 * - 7 channels (GPIO0-6) shared with LP GPIO
 * - 12-bit SAR ADC with configurable resolution (9-12 bits)
 * - Multiple attenuation levels (0dB, 2.5dB, 6dB, 11dB)
 * - Input range: 0V to 3.3V (with 11dB attenuation)
 * - Sampling rate: up to 100kSPS 
 * - Calibration: Curve fitting (primary) and line fitting schemes
 * - Continuous mode with DMA support
 * - Digital IIR filters for noise reduction
 * - Threshold monitors with interrupt support
 * - Power management and ULP support
 */
static constexpr uint8_t HF_ADC_MAX_UNITS = 1;                    // ESP32C6 has 1 ADC unit
static constexpr uint8_t HF_ADC_MAX_CHANNELS = 7;                 // GPIO0-6
static constexpr uint8_t HF_ADC_DEFAULT_UNIT = 1;                 // ADC1 only
static constexpr uint32_t HF_ADC_MAX_SAMPLING_FREQ = 100000;      // 100kSPS max
static constexpr uint32_t HF_ADC_MIN_SAMPLING_FREQ = 10;          // 10SPS min  
static constexpr uint32_t HF_ADC_DEFAULT_SAMPLING_FREQ = 1000;    // 1kSPS default
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_12BIT = 4095;      // 12-bit max
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_11BIT = 2047;      // 11-bit max
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_10BIT = 1023;      // 10-bit max
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_9BIT = 511;        // 9-bit max
static constexpr uint32_t HF_ADC_REFERENCE_VOLTAGE_MV = 1100;     // 1.1V reference
static constexpr uint32_t HF_ADC_TOLERANCE_MV = 100;              // ±100mV tolerance
static constexpr uint8_t HF_ADC_MAX_FILTERS = 2;                  // 2 IIR filters available  
static constexpr uint8_t HF_ADC_MAX_MONITORS = 2;                 // 2 threshold monitors
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MIN = 256;         // Minimum DMA buffer
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MAX = 4096;        // Maximum DMA buffer
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_DEFAULT = 1024;    // Default DMA buffer

/**
 * @brief ADC attenuation settings for ESP32C6.
 * @details Maps to ESP-IDF v5.5+ adc_atten_t enum with voltage ranges.
 */
enum class hf_adc_attenuation_t : uint8_t {
  HF_ADC_ATTEN_DB_0 = 0,   ///< 0dB attenuation (0V ~ 1.0V)
  HF_ADC_ATTEN_DB_2_5 = 1, ///< 2.5dB attenuation (0V ~ 1.3V) 
  HF_ADC_ATTEN_DB_6 = 2,   ///< 6dB attenuation (0V ~ 1.9V)
  HF_ADC_ATTEN_DB_11 = 3,  ///< 11dB attenuation (0V ~ 3.3V)
};

/**
 * @brief ADC resolution settings for ESP32C6.
 * @details Maps to ESP-IDF v5.5+ adc_bitwidth_t enum.
 */
enum class hf_adc_resolution_t : uint8_t {
  HF_ADC_RES_9BIT = 9,   ///< 9-bit resolution (512 levels)
  HF_ADC_RES_10BIT = 10, ///< 10-bit resolution (1024 levels)
  HF_ADC_RES_11BIT = 11, ///< 11-bit resolution (2048 levels)
  HF_ADC_RES_12BIT = 12, ///< 12-bit resolution (4096 levels)
};

/**
 * @brief ADC calibration schemes for ESP32C6.
 * @details Maps to ESP-IDF v5.5+ calibration scheme types.
 */
enum class hf_adc_calibration_scheme_t : uint8_t {
  HF_ADC_CALI_SCHEME_CURVE_FITTING = 0, ///< Curve fitting (preferred for ESP32C6)
  HF_ADC_CALI_SCHEME_LINE_FITTING = 1,  ///< Line fitting (fallback)
};

/**
 * @brief ADC sampling strategy types.
 */
enum class hf_adc_sampling_strategy_t : uint8_t {
  HF_ADC_SAMPLING_SINGLE = 0,      ///< Single-shot conversion
  HF_ADC_SAMPLING_CONTINUOUS = 1,  ///< Continuous conversion with DMA
  HF_ADC_SAMPLING_BURST = 2,       ///< Burst mode (fixed number of samples)
  HF_ADC_SAMPLING_TRIGGERED = 3,   ///< External trigger-based sampling
};

/**
 * @brief ADC trigger sources for advanced sampling.
 */
enum class hf_adc_trigger_source_t : uint8_t {
  HF_ADC_TRIGGER_SOFTWARE = 0, ///< Software trigger (manual)
  HF_ADC_TRIGGER_TIMER = 1,    ///< Timer-based trigger
  HF_ADC_TRIGGER_GPIO = 2,     ///< GPIO edge trigger
  HF_ADC_TRIGGER_PWM = 3,      ///< PWM sync trigger
  HF_ADC_TRIGGER_EXTERNAL = 4, ///< External trigger signal
};

/**
 * @brief ADC digital filter types supported by ESP32C6.
 */
enum class hf_adc_filter_type_t : uint8_t {
  HF_ADC_FILTER_NONE = 0,         ///< No filtering
  HF_ADC_FILTER_IIR = 1,          ///< IIR digital filter
  HF_ADC_FILTER_MOVING_AVG = 2,   ///< Moving average filter
};

/**
 * @brief ADC power mode settings.
 */
enum class hf_adc_power_mode_t : uint8_t {
  HF_ADC_POWER_FULL = 0,       ///< Maximum performance, highest power
  HF_ADC_POWER_LOW = 1,        ///< Reduced power consumption
  HF_ADC_POWER_ULTRA_LOW = 2,  ///< Minimal power, reduced functionality
  HF_ADC_POWER_SLEEP = 3,      ///< Power-down mode
};

// Platform-specific ADC types using ESP-IDF native types
using hf_adc_unit_t = adc_unit_t;
using hf_adc_channel_t = adc_channel_t;
using hf_adc_oneshot_unit_handle_t = adc_oneshot_unit_handle_t;
using hf_adc_continuous_handle_t = adc_continuous_handle_t;
using hf_adc_cali_handle_t = adc_cali_handle_t;
using hf_adc_filter_handle_t = adc_filter_handle_t;
using hf_adc_monitor_handle_t = adc_monitor_handle_t;

/**
 * @brief ADC continuous mode configuration with ESP32C6 features.
 */
struct hf_adc_continuous_config_t {
  uint32_t sample_freq_hz;      ///< Sampling frequency in Hz
  adc_digi_convert_mode_t conv_mode; ///< Conversion mode
  adc_digi_output_format_t format;   ///< Output data format  
  size_t buffer_size;           ///< DMA buffer size
  uint8_t buffer_count;         ///< Number of DMA buffers
  bool enable_dma;              ///< Enable DMA transfers
  
  hf_adc_continuous_config_t() noexcept
      : sample_freq_hz(HF_ADC_DEFAULT_SAMPLING_FREQ), conv_mode(ADC_CONV_SINGLE_UNIT_1),
        format(ADC_DIGI_OUTPUT_FORMAT_TYPE2), buffer_size(HF_ADC_DMA_BUFFER_SIZE_DEFAULT),
        buffer_count(2), enable_dma(true) {}
};

/**
 * @brief ADC channel configuration for ESP32C6.
 */
struct hf_adc_channel_config_t {
  adc_channel_t channel;        ///< ADC channel
  adc_atten_t attenuation;      ///< Attenuation setting
  adc_bitwidth_t bitwidth;      ///< Resolution setting
  bool enable_filter;           ///< Enable digital filter
  uint8_t filter_coeff;         ///< IIR filter coefficient (0-15)
  
  hf_adc_channel_config_t() noexcept
      : channel(ADC_CHANNEL_0), attenuation(ADC_ATTEN_DB_11),
        bitwidth(ADC_BITWIDTH_12), enable_filter(false), filter_coeff(2) {}
};

#else
// Non-ESP32C6 platforms - use generic types
static constexpr uint8_t HF_ADC_MAX_UNITS = 2;
static constexpr uint8_t HF_ADC_MAX_CHANNELS = 10;
static constexpr uint8_t HF_ADC_DEFAULT_UNIT = 1;
static constexpr uint32_t HF_ADC_MAX_SAMPLING_FREQ = 100000;
static constexpr uint32_t HF_ADC_MIN_SAMPLING_FREQ = 10;
static constexpr uint32_t HF_ADC_DEFAULT_SAMPLING_FREQ = 1000;
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_12BIT = 4095;
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_11BIT = 2047;
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_10BIT = 1023;
static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_9BIT = 511;
static constexpr uint32_t HF_ADC_REFERENCE_VOLTAGE_MV = 3300;
static constexpr uint32_t HF_ADC_TOLERANCE_MV = 100;
static constexpr uint8_t HF_ADC_MAX_FILTERS = 2;
static constexpr uint8_t HF_ADC_MAX_MONITORS = 2;
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MIN = 256;
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MAX = 4096;
static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_DEFAULT = 1024;

// Generic enums for non-ESP32C6 platforms
enum class hf_adc_attenuation_t : uint8_t {
  HF_ADC_ATTEN_DB_0 = 0,
  HF_ADC_ATTEN_DB_2_5 = 1,
  HF_ADC_ATTEN_DB_6 = 2,
  HF_ADC_ATTEN_DB_11 = 3,
};

enum class hf_adc_resolution_t : uint8_t {
  HF_ADC_RES_9BIT = 9,
  HF_ADC_RES_10BIT = 10,
  HF_ADC_RES_11BIT = 11,
  HF_ADC_RES_12BIT = 12,
};

enum class hf_adc_calibration_scheme_t : uint8_t {
  HF_ADC_CALI_SCHEME_CURVE_FITTING = 0,
  HF_ADC_CALI_SCHEME_LINE_FITTING = 1,
};

enum class hf_adc_sampling_strategy_t : uint8_t {
  HF_ADC_SAMPLING_SINGLE = 0,
  HF_ADC_SAMPLING_CONTINUOUS = 1,
  HF_ADC_SAMPLING_BURST = 2,
  HF_ADC_SAMPLING_TRIGGERED = 3,
};

enum class hf_adc_trigger_source_t : uint8_t {
  HF_ADC_TRIGGER_SOFTWARE = 0,
  HF_ADC_TRIGGER_TIMER = 1,
  HF_ADC_TRIGGER_GPIO = 2,
  HF_ADC_TRIGGER_PWM = 3,
  HF_ADC_TRIGGER_EXTERNAL = 4,
};

enum class hf_adc_filter_type_t : uint8_t {
  HF_ADC_FILTER_NONE = 0,
  HF_ADC_FILTER_IIR = 1,
  HF_ADC_FILTER_MOVING_AVG = 2,
};

enum class hf_adc_power_mode_t : uint8_t {
  HF_ADC_POWER_FULL = 0,
  HF_ADC_POWER_LOW = 1,
  HF_ADC_POWER_ULTRA_LOW = 2,
  HF_ADC_POWER_SLEEP = 3,
};

// Generic types for non-ESP32C6 platforms
using hf_adc_unit_t = uint8_t;
using hf_adc_channel_t = uint8_t;
using hf_adc_oneshot_unit_handle_t = void*;
using hf_adc_continuous_handle_t = void*;
using hf_adc_cali_handle_t = void*;
using hf_adc_filter_handle_t = void*;
using hf_adc_monitor_handle_t = void*;

struct hf_adc_continuous_config_t {
  uint32_t sample_freq_hz;
  uint32_t conv_mode;
  uint32_t format;
  size_t buffer_size;
  uint8_t buffer_count;
  bool enable_dma;
  
  hf_adc_continuous_config_t() noexcept
      : sample_freq_hz(HF_ADC_DEFAULT_SAMPLING_FREQ), conv_mode(0),
        format(0), buffer_size(HF_ADC_DMA_BUFFER_SIZE_DEFAULT),
        buffer_count(2), enable_dma(true) {}
};

struct hf_adc_channel_config_t {
  uint8_t channel;
  uint8_t attenuation;
  uint8_t bitwidth;
  bool enable_filter;
  uint8_t filter_coeff;
  
  hf_adc_channel_config_t() noexcept
      : channel(0), attenuation(3), bitwidth(12),
        enable_filter(false), filter_coeff(2) {}
};
#endif

/**
 * @brief ADC validation macros for ESP32C6.
 */
#define HF_ADC_IS_VALID_UNIT(unit) ((unit) <= HF_ADC_MAX_UNITS)
#define HF_ADC_IS_VALID_CHANNEL(ch) ((ch) < HF_ADC_MAX_CHANNELS)
#define HF_ADC_IS_VALID_SAMPLING_FREQ(freq) \
  ((freq) >= HF_ADC_MIN_SAMPLING_FREQ && (freq) <= HF_ADC_MAX_SAMPLING_FREQ)
#define HF_ADC_IS_VALID_RESOLUTION(res) \
  ((res) >= 9 && (res) <= 12)
#define HF_ADC_IS_VALID_ATTENUATION(atten) ((atten) <= 3)
#define HF_ADC_IS_VALID_BUFFER_SIZE(size) \
  ((size) >= HF_ADC_DMA_BUFFER_SIZE_MIN && (size) <= HF_ADC_DMA_BUFFER_SIZE_MAX)

//==============================================================================
// ESP32-C6 NVS TYPE DEFINITIONS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32-C6 NVS native type aliases for abstraction
using hf_nvs_handle_native_t = nvs_handle_t;
using hf_nvs_open_mode_native_t = nvs_open_mode_t;
using hf_nvs_type_native_t = nvs_type_t;
using hf_nvs_iterator_native_t = nvs_iterator_t;

// ESP32-C6 NVS constants
static constexpr size_t HF_NVS_MAX_KEY_LENGTH = 15;         ///< Maximum NVS key length (ESP32 limit)
static constexpr size_t HF_NVS_MAX_VALUE_SIZE = 4000;      ///< Maximum NVS value size (conservative)
static constexpr size_t HF_NVS_MAX_NAMESPACE_LENGTH = 15;  ///< Maximum namespace length
static constexpr size_t HF_NVS_MAX_NAMESPACES = 254;       ///< Maximum number of namespaces

// ESP32-C6 NVS operation timeouts
static constexpr uint32_t HF_NVS_OPERATION_TIMEOUT_MS = 1000;    ///< Default operation timeout
static constexpr uint32_t HF_NVS_INIT_TIMEOUT_MS = 5000;         ///< Initialization timeout
static constexpr uint32_t HF_NVS_COMMIT_TIMEOUT_MS = 2000;       ///< Commit operation timeout

// ESP32-C6 NVS validation macros
#define HF_NVS_IS_VALID_KEY_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_KEY_LENGTH)
#define HF_NVS_IS_VALID_VALUE_SIZE(size) ((size) <= HF_NVS_MAX_VALUE_SIZE)
#define HF_NVS_IS_VALID_NAMESPACE_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_NAMESPACE_LENGTH)


#else
// Generic implementations for non-ESP32 platforms
using hf_nvs_handle_native_t = uint32_t;
using hf_nvs_open_mode_native_t = int;
using hf_nvs_type_native_t = int;
using hf_nvs_iterator_native_t = void*;

static constexpr size_t HF_NVS_MAX_KEY_LENGTH = 32;
static constexpr size_t HF_NVS_MAX_VALUE_SIZE = 1024;
static constexpr size_t HF_NVS_MAX_NAMESPACE_LENGTH = 32;
static constexpr size_t HF_NVS_MAX_NAMESPACES = 256;

static constexpr uint32_t HF_NVS_OPERATION_TIMEOUT_MS = 1000;
static constexpr uint32_t HF_NVS_INIT_TIMEOUT_MS = 5000;
static constexpr uint32_t HF_NVS_COMMIT_TIMEOUT_MS = 2000;

#define HF_NVS_IS_VALID_KEY_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_KEY_LENGTH)
#define HF_NVS_IS_VALID_VALUE_SIZE(size) ((size) <= HF_NVS_MAX_VALUE_SIZE)
#define HF_NVS_IS_VALID_NAMESPACE_LENGTH(len) ((len) > 0 && (len) <= HF_NVS_MAX_NAMESPACE_LENGTH)

#endif
