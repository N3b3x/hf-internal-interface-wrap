/**
 * @file McuTypes_UART.h
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

#include "McuTypes_Base.h"

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
  uint16_t rts_delay_microsec;    ///< RTS assertion delay (microseconds)

  hf_uart_rs485_config_t() noexcept
      : mode(hf_uart_mode_t::HF_UART_MODE_UART), enable_collision_detect(false),
        enable_echo_suppression(true), auto_rts_control(true), rts_delay_microsec(0) {}
};

/**
 * @brief MCU-specific UART IrDA configuration.
 * @details Infrared Data Association protocol settings.
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
 * @details Sleep wakeup settings for low power applications.
 */
struct hf_uart_wakeup_config_t {
  bool enable_wakeup;             ///< Enable UART wakeup from light sleep
  uint8_t wakeup_threshold;       ///< Number of RX edges to trigger wakeup (3-1023)
  bool use_ref_tick;              ///< Use REF_TICK as clock source during sleep

  hf_uart_wakeup_config_t() noexcept
      : enable_wakeup(false), wakeup_threshold(3), use_ref_tick(false) {}
};

//==============================================================================
// MCU-SPECIFIC UART CONSTANTS
//==============================================================================

static constexpr hf_gpio_num_t HF_UART_IO_UNUSED = HF_INVALID_PIN;
static constexpr uint32_t HF_UART_MAX_PORTS = 3;          ///< ESP32C6 has 3 UART ports (0, 1, 2)
static constexpr uint32_t HF_UART_DEFAULT_BUFFER_SIZE = 256; ///< Default buffer size (bytes)
static constexpr uint32_t HF_UART_MIN_BAUD_RATE = 1200;   ///< Minimum supported baud rate
static constexpr uint32_t HF_UART_MAX_BAUD_RATE = 5000000; ///< Maximum supported baud rate
static constexpr uint32_t HF_UART_BREAK_MIN_DURATION = 1;  ///< Minimum break duration (ms)
static constexpr uint32_t HF_UART_BREAK_MAX_DURATION = 1000; ///< Maximum break duration (ms)

//==============================================================================
// UART FUNCTION MACROS
//==============================================================================

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
#define HF_UART_SET_SW_FLOW_CTRL(port, enable, xon_thresh, xoff_thresh) \
  uart_set_sw_flow_ctrl(port, enable, xon_thresh, xoff_thresh)
#define HF_UART_ENABLE_PATTERN_DET(port, pattern_chr, chr_num, chr_tout, post_idle, pre_idle) \
  uart_enable_pattern_det_baud_intr(port, pattern_chr, chr_num, chr_tout, post_idle, pre_idle)
#define HF_UART_DISABLE_PATTERN_DET(port) uart_disable_pattern_det_intr(port)
#define HF_UART_PATTERN_POP_POS(port) uart_pattern_pop_pos(port)
#define HF_UART_PATTERN_GET_POS(port) uart_pattern_get_pos(port)
#define HF_UART_PATTERN_QUEUE_RESET(port, queue_length) uart_pattern_queue_reset(port, queue_length)
#define HF_UART_SET_WAKEUP_THRESHOLD(port, threshold) uart_set_wakeup_threshold(port, threshold)
#define HF_UART_GET_WAKEUP_THRESHOLD(port, threshold) uart_get_wakeup_threshold(port, threshold)
#define HF_UART_GET_COLLISION_FLAG(port, flag) uart_get_collision_flag(port, flag)
#define HF_UART_SET_RX_FULL_THRESHOLD(port, threshold) uart_set_rx_full_threshold(port, threshold)
#define HF_UART_SET_TX_EMPTY_THRESHOLD(port, threshold) uart_set_tx_empty_threshold(port, threshold)
#define HF_UART_SET_RX_TIMEOUT(port, tout_thresh) uart_set_rx_timeout(port, tout_thresh)
#define HF_UART_ENABLE_RX_INTR(port) uart_enable_rx_intr(port)
#define HF_UART_DISABLE_RX_INTR(port) uart_disable_rx_intr(port)
#define HF_UART_ENABLE_TX_INTR(port, enable, thresh) uart_enable_tx_intr(port, enable, thresh)
#define HF_UART_DISABLE_TX_INTR(port) uart_disable_tx_intr(port)
#define HF_UART_SET_ALWAYS_RX_TIMEOUT(port, enable) uart_set_always_rx_timeout(port, enable)
#define HF_UART_INTR_CONFIG(port, intr_conf) uart_intr_config(port, intr_conf)
#else
// Non-ESP32 platforms - placeholder definitions
#define HF_UART_DRIVER_INSTALL(port, tx_size, rx_size, queue_size, queue, intr_flags) (-1)
#define HF_UART_DRIVER_DELETE(port) (-1)
#define HF_UART_PARAM_CONFIG(port, config) (-1)
#define HF_UART_SET_PIN(port, tx_pin, rx_pin, rts_pin, cts_pin) (-1)
#define HF_UART_WRITE_BYTES(port, data, length) (-1)
#define HF_UART_READ_BYTES(port, data, length, timeout) (-1)
#define HF_UART_FLUSH(port) (-1)
#define HF_UART_FLUSH_INPUT(port) (-1)
#define HF_UART_GET_BUFFERED_DATA_LEN(port, length) (-1)
#define HF_UART_WAIT_TX_DONE(port, timeout) (-1)
#define HF_UART_SET_BAUDRATE(port, baudrate) (-1)
#define HF_UART_SET_WORD_LENGTH(port, data_bits) (-1)
#define HF_UART_SET_PARITY(port, parity) (-1)
#define HF_UART_SET_STOP_BITS(port, stop_bits) (-1)
#define HF_UART_SET_HW_FLOW_CTRL(port, flow_ctrl, thresh) (-1)
#define HF_UART_SET_RTS(port, level) (-1)
#define HF_UART_GET_CTS(port, cts) (-1)
#define HF_UART_SET_LINE_INVERSE(port, inverse_mask) (-1)
#define HF_UART_SET_MODE(port, mode) (-1)
#define HF_UART_SET_SW_FLOW_CTRL(port, enable, xon_thresh, xoff_thresh) (-1)
#define HF_UART_ENABLE_PATTERN_DET(port, pattern_chr, chr_num, chr_tout, post_idle, pre_idle) (-1)
#define HF_UART_DISABLE_PATTERN_DET(port) (-1)
#define HF_UART_PATTERN_POP_POS(port) (-1)
#define HF_UART_PATTERN_GET_POS(port) (-1)
#define HF_UART_PATTERN_QUEUE_RESET(port, queue_length) (-1)
#define HF_UART_SET_WAKEUP_THRESHOLD(port, threshold) (-1)
#define HF_UART_GET_WAKEUP_THRESHOLD(port, threshold) (-1)
#define HF_UART_GET_COLLISION_FLAG(port, flag) (-1)
#define HF_UART_SET_RX_FULL_THRESHOLD(port, threshold) (-1)
#define HF_UART_SET_TX_EMPTY_THRESHOLD(port, threshold) (-1)
#define HF_UART_SET_RX_TIMEOUT(port, tout_thresh) (-1)
#define HF_UART_ENABLE_RX_INTR(port) (-1)
#define HF_UART_DISABLE_RX_INTR(port) (-1)
#define HF_UART_ENABLE_TX_INTR(port, enable, thresh) (-1)
#define HF_UART_DISABLE_TX_INTR(port) (-1)
#define HF_UART_SET_ALWAYS_RX_TIMEOUT(port, enable) (-1)
#define HF_UART_INTR_CONFIG(port, intr_conf) (-1)
#endif
