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

#include <cstdint>

//==============================================================================
// MCU PLATFORM SELECTION
//==============================================================================

// Primary MCU platform detection and definition
#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(ESP32C6)
    #define HF_MCU_ESP32C6
    #define HF_MCU_PLATFORM_NAME "ESP32-C6"
    #define HF_MCU_ARCHITECTURE "RISC-V RV32IMAC"
#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(ESP_PLATFORM)
    #define HF_MCU_ESP32
    #define HF_MCU_PLATFORM_NAME "ESP32"
    #define HF_MCU_ARCHITECTURE "Xtensa LX6"
#else
    #warning "Unknown MCU platform - defaulting to ESP32-C6"
    #define HF_MCU_ESP32C6
    #define HF_MCU_PLATFORM_NAME "ESP32-C6"
    #define HF_MCU_ARCHITECTURE "RISC-V RV32IMAC"
#endif

//==============================================================================
// GPIO TYPES
//==============================================================================

typedef int32_t hf_gpio_num_t;           //!< GPIO pin number type
typedef uint32_t hf_gpio_mask_t;         //!< GPIO pin mask type

enum class hf_gpio_mode_t : uint8_t {
    Input = 0,
    Output,
    OutputOD,
};

enum class hf_gpio_pull_t : uint8_t {
    None = 0,
    PullUp,
    PullDown,
};

enum class hf_gpio_intr_type_t : uint8_t {
    Disable = 0,
    RisingEdge,
    FallingEdge,
    AnyEdge,
};

enum class hf_gpio_pull_mode_t : uint8_t {
    HF_GPIO_FLOATING = 0,      ///< No pull resistor (high impedance)
    HF_GPIO_PULLUP,            ///< Internal pull-up resistor enabled
    HF_GPIO_PULLDOWN,          ///< Internal pull-down resistor enabled
};

enum class hf_gpio_drive_cap_t : uint8_t {
    HF_GPIO_DRIVE_CAP_0 = 0,   ///< Minimum drive capability (5mA)
    HF_GPIO_DRIVE_CAP_1,       ///< Medium drive capability (10mA)
    HF_GPIO_DRIVE_CAP_2,       ///< High drive capability (20mA)
    HF_GPIO_DRIVE_CAP_3,       ///< Maximum drive capability (40mA)
};

//==============================================================================
// ADC TYPES
//==============================================================================

typedef int32_t hf_adc_channel_t;        //!< ADC channel number type
typedef uint32_t hf_adc_value_t;         //!< ADC reading value type

enum class hf_adc_resolution_t : uint8_t {
    HF_ADC_RES_9BIT = 9,
    HF_ADC_RES_10BIT = 10,
    HF_ADC_RES_11BIT = 11,
    HF_ADC_RES_12BIT = 12,
    HF_ADC_RES_13BIT = 13,
};

enum class hf_adc_attenuation_t : uint8_t {
    HF_ADC_ATTEN_DB_0 = 0,     ///< No attenuation (1.1V max)
    HF_ADC_ATTEN_DB_2_5 = 1,   ///< 2.5dB attenuation (1.5V max)
    HF_ADC_ATTEN_DB_6 = 2,     ///< 6dB attenuation (2.2V max)
    HF_ADC_ATTEN_DB_11 = 3,    ///< 11dB attenuation (3.9V max)
};

enum class hf_adc_unit_t : uint8_t {
    HF_ADC_UNIT_1 = 1,         ///< SAR ADC 1
    HF_ADC_UNIT_2 = 2,         ///< SAR ADC 2
};

//==============================================================================
// I2C TYPES
//==============================================================================

typedef uint8_t hf_i2c_address_t;        //!< I2C device address type
typedef uint32_t hf_i2c_port_t;          //!< I2C port number type
typedef uint32_t hf_i2c_freq_t;          //!< I2C frequency type

enum class hf_i2c_mode_t : uint8_t {
    HF_I2C_MODE_SLAVE = 0,
    HF_I2C_MODE_MASTER,
};

//==============================================================================
// SPI TYPES
//==============================================================================

typedef uint32_t hf_spi_host_t;          //!< SPI host device type
typedef uint32_t hf_spi_freq_t;          //!< SPI frequency type

enum class hf_spi_mode_t : uint8_t {
    HF_SPI_MODE_0 = 0,         ///< CPOL=0, CPHA=0
    HF_SPI_MODE_1 = 1,         ///< CPOL=0, CPHA=1
    HF_SPI_MODE_2 = 2,         ///< CPOL=1, CPHA=0
    HF_SPI_MODE_3 = 3,         ///< CPOL=1, CPHA=1
};

enum class hf_spi_bit_order_t : uint8_t {
    HF_SPI_BIT_ORDER_MSB_FIRST = 0,
    HF_SPI_BIT_ORDER_LSB_FIRST = 1,
};

//==============================================================================
// UART TYPES
//==============================================================================

typedef uint32_t hf_uart_port_t;         //!< UART port number type
typedef uint32_t hf_uart_baudrate_t;     //!< UART baudrate type

enum class hf_uart_data_bits_t : uint8_t {
    HF_UART_DATA_5_BITS = 0,
    HF_UART_DATA_6_BITS = 1,
    HF_UART_DATA_7_BITS = 2,
    HF_UART_DATA_8_BITS = 3,
};

enum class hf_uart_parity_t : uint8_t {
    HF_UART_PARITY_DISABLE = 0,
    HF_UART_PARITY_EVEN = 2,
    HF_UART_PARITY_ODD = 3,
};

enum class hf_uart_stop_bits_t : uint8_t {
    HF_UART_STOP_BITS_1 = 1,
    HF_UART_STOP_BITS_1_5 = 2,
    HF_UART_STOP_BITS_2 = 3,
};

enum class hf_uart_flow_ctrl_t : uint8_t {
    HF_UART_HW_FLOWCTRL_DISABLE = 0,
    HF_UART_HW_FLOWCTRL_RTS = 1,
    HF_UART_HW_FLOWCTRL_CTS = 2,
    HF_UART_HW_FLOWCTRL_CTS_RTS = 3,
};

//==============================================================================
// CAN TYPES
//==============================================================================

typedef uint32_t hf_can_message_id_t;    //!< CAN message ID type
typedef uint32_t hf_can_baudrate_t;      //!< CAN baudrate type

enum class hf_can_mode_t : uint8_t {
    HF_CAN_MODE_NORMAL = 0,
    HF_CAN_MODE_NO_ACK = 1,
    HF_CAN_MODE_LISTEN_ONLY = 2,
};

enum class hf_can_frame_format_t : uint8_t {
    HF_CAN_FRAME_STANDARD = 0,
    HF_CAN_FRAME_EXTENDED = 1,
};

//==============================================================================
// PWM TYPES
//==============================================================================

typedef uint32_t hf_pwm_channel_t;       //!< PWM channel number type
typedef uint32_t hf_pwm_freq_t;          //!< PWM frequency type
typedef uint32_t hf_pwm_duty_t;          //!< PWM duty cycle type
typedef uint8_t hf_pwm_resolution_t;     //!< PWM resolution in bits

enum class hf_pwm_mode_t : uint8_t {
    HF_PWM_MODE_UP = 0,
    HF_PWM_MODE_DOWN = 1,
    HF_PWM_MODE_UP_DOWN = 2,
};

//==============================================================================
// PIO TYPES
//==============================================================================

typedef uint32_t hf_pio_program_t;       //!< PIO program type
typedef uint32_t hf_pio_state_machine_t; //!< PIO state machine type

enum class hf_pio_direction_t : uint8_t {
    HF_PIO_DIR_IN = 0,
    HF_PIO_DIR_OUT = 1,
};

//==============================================================================
// RETURN CODE TYPES
//==============================================================================

enum class hf_return_code_t : int32_t {
    HF_OK = 0,                    ///< Success
    HF_FAIL = -1,                 ///< Generic failure
    HF_ERR_NO_MEM = -2,           ///< Out of memory
    HF_ERR_INVALID_ARG = -3,      ///< Invalid argument
    HF_ERR_INVALID_STATE = -4,    ///< Invalid state
    HF_ERR_INVALID_SIZE = -5,     ///< Invalid size
    HF_ERR_NOT_FOUND = -6,        ///< Resource not found
    HF_ERR_NOT_SUPPORTED = -7,    ///< Operation not supported
    HF_ERR_TIMEOUT = -8,          ///< Operation timed out
    HF_ERR_INVALID_RESPONSE = -9, ///< Invalid response received
    HF_ERR_INVALID_CRC = -10,     ///< CRC validation failed
    HF_ERR_INVALID_VERSION = -11, ///< Version mismatch
    HF_ERR_INVALID_MAC = -12,     ///< MAC address invalid
    HF_ERR_WIFI_BASE = -13,       ///< Base for WiFi errors
};

//==============================================================================
// TIMING TYPES
//==============================================================================

typedef uint32_t hf_timeout_ms_t;        //!< Timeout in milliseconds

static constexpr hf_timeout_ms_t HF_TIMEOUT_NEVER = 0xFFFFFFFF;
static constexpr hf_timeout_ms_t HF_TIMEOUT_IMMEDIATE = 0;
static constexpr hf_timeout_ms_t HF_TIMEOUT_DEFAULT = 1000;

#endif // MCU_TYPES_H
