/**
 * @file EspTypes_UART.h
 * @brief MCU-specific UART type definitions for hardware abstraction.
 *
 * This header defines all UART-specific types and constants that are used
 * throughout the internal interface wrap layer for UART operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"    // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"
#include "BaseUart.h" // For hf_uart_err_t

//==============================================================================
// PLATFORM-SPECIFIC UART TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
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
using hf_uart_port_native_t = uint8_t;
using hf_uart_config_native_t = struct { int dummy; };
using hf_uart_word_length_native_t = uint8_t;
using hf_uart_parity_native_t = uint8_t;
using hf_uart_stop_bits_native_t = uint8_t;
using hf_uart_hw_flowcontrol_native_t = uint8_t;
using hf_uart_signal_inv_native_t = uint8_t;
#endif

//==============================================================================
// MCU-SPECIFIC UART TYPES
//==============================================================================

/**
 * @brief MCU-specific UART communication modes (ESP32C6 supported).
 */
enum class hf_uart_mode_t : uint8_t {
  HF_UART_MODE_UART = 0,                    ///< Standard UART mode
  HF_UART_MODE_RS485_HALF_DUPLEX = 1,      ///< RS485 half-duplex mode (auto RTS control)
  HF_UART_MODE_IRDA = 2,                   ///< IrDA infrared communication mode
  HF_UART_MODE_RS485_COLLISION_DETECT = 3, ///< RS485 with collision detection
  HF_UART_MODE_RS485_APP_CTRL = 4,         ///< RS485 with application RTS control
  HF_UART_MODE_LOOPBACK = 5,               ///< Loopback mode for testing
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
 * @brief UART handle type for MCU UART driver.
 * @details ESP32 UART driver uses port-based API, so handle is not used directly.
 */
#ifdef HF_MCU_FAMILY_ESP32
using hf_uart_handle_t = void *; ///< ESP32 UART uses port numbers, not handles
#else
using hf_uart_handle_t = void *; ///< Generic handle for other platforms
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
 * @brief MCU-specific UART pattern detection configuration.
 * @details Settings for AT command pattern detection and similar applications.
 */
struct hf_uart_pattern_config_t {
  bool enable_pattern_detection;   ///< Enable pattern detection feature
  char pattern_char;              ///< Character to detect (e.g., '+' for AT commands)
  uint8_t pattern_char_num;       ///< Number of consecutive pattern characters
  uint16_t char_timeout;          ///< Timeout between pattern characters (baud cycles)
  uint16_t post_idle;             ///< Idle time after last pattern char (baud cycles)
  uint16_t pre_idle;              ///< Idle time before first pattern char (baud cycles)

  hf_uart_pattern_config_t() noexcept
      : enable_pattern_detection(false), pattern_char('+'), pattern_char_num(3),
        char_timeout(9), post_idle(12), pre_idle(12) {}
};

/**
 * @brief MCU-specific UART RS485 configuration.
 * @details RS485 communication settings including collision detection.
 */
struct hf_uart_rs485_config_t {
  hf_uart_mode_t mode;            ///< RS485 operating mode
  bool enable_collision_detect;   ///< Enable collision detection
  bool enable_echo_suppression;   ///< Suppress echo during transmission
  bool auto_rts_control;          ///< Automatic RTS line control
  uint32_t rts_delay_ms;          ///< RTS delay in milliseconds
  uint32_t rts_timeout_ms;        ///< RTS timeout in milliseconds
  uint32_t collision_timeout_ms;  ///< Collision detection timeout

  hf_uart_rs485_config_t() noexcept
      : mode(hf_uart_mode_t::HF_UART_MODE_RS485_HALF_DUPLEX), enable_collision_detect(false),
        enable_echo_suppression(true), auto_rts_control(true), rts_delay_ms(0),
        rts_timeout_ms(100), collision_timeout_ms(100) {}
};

/**
 * @brief MCU-specific UART IrDA configuration.
 * @details IrDA infrared communication settings.
 */
struct hf_uart_irda_config_t {
  bool enable_irda;               ///< Enable IrDA mode
  bool invert_tx;                 ///< Invert TX signal for IrDA
  bool invert_rx;                 ///< Invert RX signal for IrDA
  uint8_t duty_cycle;             ///< IrDA duty cycle (0-100%)

  hf_uart_irda_config_t() noexcept
      : enable_irda(false), invert_tx(false), invert_rx(false), duty_cycle(50) {}
};

/**
 * @brief MCU-specific UART wakeup configuration.
 * @details Wakeup settings for light sleep mode.
 */
struct hf_uart_wakeup_config_t {
  bool enable_wakeup;             ///< Enable UART wakeup from light sleep
  uint8_t wakeup_threshold;       ///< Number of RX edges to trigger wakeup (3-1023)
  bool use_ref_tick;              ///< Use REF_TICK as clock source during sleep

  hf_uart_wakeup_config_t() noexcept
      : enable_wakeup(false), wakeup_threshold(3), use_ref_tick(false) {}
};

//==============================================================================
// UART CONSTANTS AND LIMITS
//==============================================================================

static constexpr hf_pin_num_t HF_UART_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_UART_MAX_PORTS = 3;          ///< ESP32C6 has 3 UART ports (0, 1, 2)
static constexpr uint32_t HF_UART_DEFAULT_BUFFER_SIZE = 256; ///< Default buffer size (bytes)
static constexpr uint32_t HF_UART_MIN_BAUD_RATE = 1200;   ///< Minimum supported baud rate
static constexpr uint32_t HF_UART_MAX_BAUD_RATE = 5000000; ///< Maximum supported baud rate
static constexpr uint32_t HF_UART_BREAK_MIN_DURATION = 1;  ///< Minimum break duration (ms)
static constexpr uint32_t HF_UART_BREAK_MAX_DURATION = 1000; ///< Maximum break duration (ms)

//==============================================================================
// UART VALIDATION MACROS
//==============================================================================

/**
 * @brief Validate UART port number.
 */
#define UART_IS_VALID_PORT(port) ((port) >= 0 && (port) < HF_UART_MAX_PORTS)

/**
 * @brief Validate UART baud rate.
 */
#define UART_IS_VALID_BAUD_RATE(baud) ((baud) >= HF_UART_MIN_BAUD_RATE && (baud) <= HF_UART_MAX_BAUD_RATE)

/**
 * @brief Validate UART data bits.
 */
#define UART_IS_VALID_DATA_BITS(bits) ((bits) >= 5 && (bits) <= 8)

/**
 * @brief Validate UART parity setting.
 */
#define UART_IS_VALID_PARITY(parity) ((parity) >= 0 && (parity) <= 2)

/**
 * @brief Validate UART stop bits.
 */
#define UART_IS_VALID_STOP_BITS(stop) ((stop) >= 1 && (stop) <= 2)

/**
 * @brief Validate UART buffer size.
 */
#define UART_IS_VALID_BUFFER_SIZE(size) ((size) > 0 && (size) <= 32768)

/**
 * @brief Validate UART break duration.
 */
#define UART_IS_VALID_BREAK_DURATION(duration) ((duration) >= HF_UART_BREAK_MIN_DURATION && (duration) <= HF_UART_BREAK_MAX_DURATION)
