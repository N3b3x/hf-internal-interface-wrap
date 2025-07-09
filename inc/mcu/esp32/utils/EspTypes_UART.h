/**
 * @file EspTypes_UART.h
 * @brief ESP32 UART type definitions for hardware abstraction.
 *
 * This header defines only the essential UART-specific types used by
 * the EspUart implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseUart.h"        // For hf_uart_err_t
#include "HardwareTypes.h"   // For basic hardware types
#include "McuSelect.h"       // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"

//==============================================================================
// ESP32 UART TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspUart implementation

//==============================================================================
// ESP32 UART CONSTANTS
//==============================================================================

static constexpr hf_pin_num_t HF_UART_IO_UNUSED = 0xFFFFFFFF;
static constexpr uint32_t HF_UART_BREAK_MIN_DURATION = 1;  ///< Minimum break duration (ms)
static constexpr uint32_t HF_UART_BREAK_MAX_DURATION = 1000; ///< Maximum break duration (ms)

//==============================================================================
// ESP32 UART ENUMS
//==============================================================================

/**
 * @brief ESP32 UART operating modes.
 */
enum class hf_uart_mode_t : uint8_t {
  HF_UART_MODE_UART = 0,     ///< Standard UART mode
  HF_UART_MODE_RS485 = 1,    ///< RS485 mode
  HF_UART_MODE_IRDA = 2      ///< IrDA mode
};

/**
 * @brief ESP32 UART data bits configuration.
 */
enum class hf_uart_data_bits_t : uint8_t {
  HF_UART_DATA_5_BITS = 0,   ///< 5 data bits
  HF_UART_DATA_6_BITS = 1,   ///< 6 data bits
  HF_UART_DATA_7_BITS = 2,   ///< 7 data bits
  HF_UART_DATA_8_BITS = 3    ///< 8 data bits
};

/**
 * @brief ESP32 UART parity configuration.
 */
enum class hf_uart_parity_t : uint8_t {
  HF_UART_PARITY_DISABLE = 0, ///< No parity
  HF_UART_PARITY_EVEN = 1,    ///< Even parity
  HF_UART_PARITY_ODD = 2      ///< Odd parity
};

/**
 * @brief ESP32 UART stop bits configuration.
 */
enum class hf_uart_stop_bits_t : uint8_t {
  HF_UART_STOP_BITS_1 = 1,   ///< 1 stop bit
  HF_UART_STOP_BITS_1_5 = 2, ///< 1.5 stop bits
  HF_UART_STOP_BITS_2 = 3    ///< 2 stop bits
};

/**
 * @brief ESP32 UART flow control configuration.
 */
enum class hf_uart_flow_ctrl_t : uint8_t {
  HF_UART_HW_FLOWCTRL_DISABLE = 0, ///< No flow control
  HF_UART_HW_FLOWCTRL_RTS = 1,     ///< RTS flow control
  HF_UART_HW_FLOWCTRL_CTS = 2,     ///< CTS flow control
  HF_UART_HW_FLOWCTRL_CTS_RTS = 3  ///< CTS and RTS flow control
};

/**
 * @brief ESP32 UART operating mode.
 */
enum class hf_uart_operating_mode_t : uint8_t {
  HF_UART_MODE_POLLING = 0,  ///< Polling mode
  HF_UART_MODE_INTERRUPT = 1, ///< Interrupt mode
  HF_UART_MODE_DMA = 2       ///< DMA mode
};

//==============================================================================
// ESP32 UART CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief ESP32 UART port configuration.
 */
struct hf_uart_port_config_t {
  hf_port_number_t port_number;                    ///< UART port number (0, 1, 2)
  hf_baud_rate_t baud_rate;                        ///< Baud rate in bits per second
  hf_uart_data_bits_t data_bits;                   ///< Data bits (5-8)
  hf_uart_parity_t parity;                         ///< Parity configuration
  hf_uart_stop_bits_t stop_bits;                   ///< Stop bits (1, 1.5, 2)
  hf_uart_flow_ctrl_t flow_control;                ///< Hardware flow control
  hf_pin_num_t tx_pin;                             ///< TX pin number
  hf_pin_num_t rx_pin;                             ///< RX pin number
  hf_pin_num_t rts_pin;                            ///< RTS pin number (optional)
  hf_pin_num_t cts_pin;                            ///< CTS pin number (optional)
  uint16_t tx_buffer_size;                         ///< TX buffer size in bytes
  uint16_t rx_buffer_size;                         ///< RX buffer size in bytes
  uint8_t event_queue_size;                        ///< Event queue size for interrupt mode
  hf_uart_operating_mode_t operating_mode;         ///< Operating mode (polling/interrupt/DMA)
  hf_timeout_ms_t timeout_ms;                      ///< Default timeout for operations
  bool enable_pattern_detection;                   ///< Enable pattern detection
  bool enable_wakeup;                              ///< Enable UART wakeup from sleep
  bool enable_loopback;                            ///< Enable loopback mode for testing
  
  hf_uart_port_config_t() noexcept
      : port_number(0), baud_rate(115200),
        data_bits(hf_uart_data_bits_t::HF_UART_DATA_8_BITS),
        parity(hf_uart_parity_t::HF_UART_PARITY_DISABLE),
        stop_bits(hf_uart_stop_bits_t::HF_UART_STOP_BITS_1),
        flow_control(hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE),
        tx_pin(HF_UART_IO_UNUSED), rx_pin(HF_UART_IO_UNUSED),
        rts_pin(HF_UART_IO_UNUSED), cts_pin(HF_UART_IO_UNUSED),
        tx_buffer_size(1024), rx_buffer_size(1024), event_queue_size(10),
        operating_mode(hf_uart_operating_mode_t::HF_UART_MODE_POLLING),
        timeout_ms(1000), enable_pattern_detection(false),
        enable_wakeup(false), enable_loopback(false) {}
};

/**
 * @brief ESP32 UART flow control configuration.
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
        xon_char(0x11), xoff_char(0x13), rx_flow_ctrl_thresh(122),
        tx_flow_ctrl_thresh(80), auto_rts(false), auto_cts(false) {}
};

/**
 * @brief ESP32 UART power management configuration.
 */
struct hf_uart_power_config_t {
  bool sleep_retention_enable;  ///< Enable sleep retention
  bool allow_pd_in_light_sleep; ///< Allow power down in light sleep
  bool allow_pd_in_deep_sleep;  ///< Allow power down in deep sleep
  bool wakeup_enable;          ///< Enable UART wakeup capability
  uint8_t wakeup_threshold;    ///< Wakeup threshold character count
  uint32_t wakeup_timeout_ms;  ///< Wakeup timeout in milliseconds
  
  hf_uart_power_config_t() noexcept
      : sleep_retention_enable(true), allow_pd_in_light_sleep(true),
        allow_pd_in_deep_sleep(false), wakeup_enable(false),
        wakeup_threshold(3), wakeup_timeout_ms(1000) {}
};

/**
 * @brief ESP32 UART pattern detection configuration.
 */
struct hf_uart_pattern_config_t {
  bool enable_pattern_detection;   ///< Enable pattern detection feature
  char pattern_char;              ///< Character to detect (e.g., '+' for AT commands)
  uint8_t pattern_char_num;       ///< Number of consecutive pattern characters
  uint16_t char_timeout;          ///< Timeout between pattern characters (baud cycles)
  uint16_t post_idle;             ///< Idle time after last pattern char (baud cycles)
  uint16_t pre_idle;              ///< Idle time before first pattern char (baud cycles)
  
  hf_uart_pattern_config_t() noexcept
      : enable_pattern_detection(false), pattern_char('+'),
        pattern_char_num(1), char_timeout(5), post_idle(5), pre_idle(5) {}
};

/**
 * @brief ESP32 UART RS485 configuration.
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
      : mode(hf_uart_mode_t::HF_UART_MODE_UART), enable_collision_detect(false),
        enable_echo_suppression(false), auto_rts_control(false),
        rts_delay_ms(0), rts_timeout_ms(0), collision_timeout_ms(0) {}
};

/**
 * @brief ESP32 UART IrDA configuration.
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
 * @brief ESP32 UART wakeup configuration.
 */
struct hf_uart_wakeup_config_t {
  bool enable_wakeup;             ///< Enable UART wakeup from light sleep
  uint8_t wakeup_threshold;       ///< Number of RX edges to trigger wakeup (3-1023)
  bool use_ref_tick;              ///< Use REF_TICK as clock source during sleep
  
  hf_uart_wakeup_config_t() noexcept
      : enable_wakeup(false), wakeup_threshold(3), use_ref_tick(false) {}
};

//==============================================================================
// ESP32 UART CALLBACK TYPES
//==============================================================================

/**
 * @brief UART event callback function type.
 * @param event Pointer to UART event
 * @param user_data User data pointer
 * @return true to yield to higher priority task, false otherwise
 */
using hf_uart_event_callback_t = bool (*)(const void* event, void* user_data);

/**
 * @brief UART pattern detection callback function type.
 * @param pattern_pos Pattern position in buffer
 * @param user_data User data pointer
 * @return true to yield to higher priority task, false otherwise
 */
using hf_uart_pattern_callback_t = bool (*)(int pattern_pos, void* user_data);

/**
 * @brief UART break detection callback function type.
 * @param break_duration Break duration in milliseconds
 * @param user_data User data pointer
 * @return true to yield to higher priority task, false otherwise
 */
using hf_uart_break_callback_t = bool (*)(uint32_t break_duration, void* user_data);

//==============================================================================
// END OF ESPUART TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
