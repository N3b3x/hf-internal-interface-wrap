/**
 * @file McuTypes.h
 * @brief MCU-specific type definitions for hardware abstraction (hf_* types).
 *
 * This header defines all MCU-specific types and constants that are used
 * throughout the internal interface wrap layer. By centralizing these definitions,
 * porting to new MCUs only requires updating this single file.
 *
 * All interface classes (CAN, UART, I2C, SPI, GPIO, ADC, PIO, PWM) must use only these types.
 */

#ifndef MCU_TYPES_H
#define MCU_TYPES_H

#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection
#include <cstdint>

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
  HF_CAN_MODE_NORMAL = 0,
  HF_CAN_MODE_NO_ACK = 1,
  HF_CAN_MODE_LISTEN_ONLY = 2,
};

/**
 * @brief MCU-specific PWM mode configuration.
 */
enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_UP = 0,
  HF_PWM_MODE_DOWN = 1,
  HF_PWM_MODE_UP_DOWN = 2,
};

//==============================================================================
// MCU-SPECIFIC TYPE ALIASES (FOR INTERNAL MCU LAYER USE)
//==============================================================================

// Basic hardware identifiers (map platform-agnostic to MCU types)
using hf_gpio_num_t = HfPinNumber;    ///< GPIO pin number
using hf_i2c_port_t = HfPortNumber;   ///< I2C port number
using hf_spi_host_t = HfHostId;       ///< SPI host identifier
using hf_uart_port_t = HfPortNumber;  ///< UART port number
using hf_adc_channel_t = HfChannelId; ///< ADC channel identifier
using hf_pwm_channel_t = HfChannelId; ///< PWM channel identifier

// Communication parameters
using hf_i2c_freq_t = HfFrequencyHz;   ///< I2C frequency in Hz
using hf_spi_freq_t = HfFrequencyHz;   ///< SPI frequency in Hz
using hf_pwm_freq_t = HfFrequencyHz;   ///< PWM frequency in Hz
using hf_uart_baudrate_t = HfBaudRate; ///< UART baud rate
using hf_can_baudrate_t = HfBaudRate;  ///< CAN baud rate

// Data and timing types
using hf_timeout_ms_t = HfTimeoutMs;  ///< Timeout in milliseconds
using hf_adc_value_t = uint32_t;      ///< ADC reading value
using hf_pwm_duty_t = uint32_t;       ///< PWM duty cycle value
using hf_can_message_id_t = uint32_t; ///< CAN message ID
using hf_i2c_address_t = uint8_t;     ///< I2C device address

// Low-level driver configuration structures
using hf_i2c_config_t = i2c_config_t;                         ///< I2C config
using hf_spi_bus_config_t = spi_bus_config_t;                 ///< SPI bus config
using hf_spi_device_config_t = spi_device_interface_config_t; ///< SPI device config
using hf_uart_config_t = uart_config_t;                       ///< UART config

// Platform-specific handles
using hf_timer_handle_t = void *;                ///< Timer handle (platform-specific)
using hf_mutex_handle_t = SemaphoreHandle_t;     ///< RTOS mutex handle
using hf_semaphore_handle_t = SemaphoreHandle_t; ///< Generic semaphore handle

//==============================================================================
// MCU-SPECIFIC STRUCTURES FOR TIMERS AND PWM
//==============================================================================

/**
 * @brief Timer statistics structure for MCU timer implementations.
 * @details Used internally by MCU implementations for detailed timer monitoring.
 */
struct TimerStats {
  uint64_t callback_count;   ///< Number of callbacks executed
  uint64_t missed_callbacks; ///< Number of missed callbacks
  uint64_t start_count;      ///< Number of times timer was started
  uint64_t stop_count;       ///< Number of times timer was stopped
  HfTimerErr last_error;     ///< Last error that occurred

  TimerStats() noexcept
      : callback_count(0), missed_callbacks(0), start_count(0), stop_count(0),
        last_error(HfTimerErr::TIMER_SUCCESS) {}
};

//==============================================================================
// TIMER ERROR DEFINITIONS (Forward declarations from BasePeriodicTimer.h)
//==============================================================================

// Timer error codes are defined in BasePeriodicTimer.h
// This forward declaration ensures McuTypes.h can reference HfTimerErr
#ifndef HF_TIMER_ERR_DEFINED
#define HF_TIMER_ERR_DEFINED
enum class HfTimerErr : int32_t;
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

#endif // MCU_TYPES_H
