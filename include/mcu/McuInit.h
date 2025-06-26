/**
 * @file McuInit.h
 * @brief MCU-agnostic hardware initialization interface.
 * 
 * This header provides MCU-agnostic hardware initialization functions
 * that wrap MCU-specific hardware configuration. This layer isolates
 * all MCU-specific details while providing a clean, portable interface.
 */

#ifndef MCU_INIT_H
#define MCU_INIT_H

#include "McuTypes.h"
#include "McuSelect.h"

#include <cstdint>
#include <cstddef>

namespace HardFOC {
namespace Mcu {

/**
 * @brief GPIO pin configuration structure.
 */
struct GpioPinConfig {
    hf_gpio_num_t pin_number;            ///< GPIO pin number
    bool is_output;                      ///< true for output, false for input
    bool has_pullup;                     ///< Enable internal pullup resistor
    bool has_pulldown;                   ///< Enable internal pulldown resistor
    hf_gpio_drive_cap_t drive_capability; ///< GPIO drive capability
};

/**
 * @brief ADC channel configuration structure.
 */
struct AdcChannelConfig {
    hf_adc_channel_t channel_number;     ///< ADC channel number
    hf_adc_resolution_t resolution_bits; ///< ADC resolution
    hf_adc_attenuation_t attenuation;    ///< ADC input attenuation
    hf_adc_unit_t unit;                  ///< ADC unit (1 or 2)
    float max_voltage_v;                 ///< Maximum input voltage
    bool is_differential;                ///< true for differential, false for single-ended
};

/**
 * @brief I2C bus configuration structure.
 */
struct I2cBusConfig {
    hf_i2c_port_t port_number;           ///< I2C port number
    hf_gpio_num_t sda_pin;               ///< SDA pin number
    hf_gpio_num_t scl_pin;               ///< SCL pin number
    hf_i2c_freq_t frequency_hz;          ///< Bus frequency in Hz
    bool enable_pullups;                 ///< Enable internal pullup resistors
};

/**
 * @brief SPI bus configuration structure.
 */
struct SpiBusConfig {
    hf_spi_host_t host_id;               ///< SPI host device
    hf_gpio_num_t mosi_pin;              ///< MOSI pin number
    hf_gpio_num_t miso_pin;              ///< MISO pin number
    hf_gpio_num_t sclk_pin;              ///< SCLK pin number
    hf_spi_freq_t max_frequency_hz;      ///< Maximum bus frequency
};

/**
 * @brief UART configuration structure.
 */
struct UartConfig {
    hf_uart_port_t port_number;          ///< UART port number
    hf_gpio_num_t tx_pin;                ///< TX pin number
    hf_gpio_num_t rx_pin;                ///< RX pin number
    hf_uart_baudrate_t baudrate;         ///< Baudrate
    hf_uart_data_bits_t data_bits;       ///< Data bits
    hf_uart_parity_t parity;             ///< Parity setting
    hf_uart_stop_bits_t stop_bits;       ///< Stop bits
    hf_uart_flow_ctrl_t flow_control;    ///< Flow control
};

/**
 * @brief CAN bus configuration structure.
 */
struct CanBusConfig {
    hf_gpio_num_t tx_pin;                ///< CAN TX pin
    hf_gpio_num_t rx_pin;                ///< CAN RX pin
    hf_can_baudrate_t baudrate;          ///< CAN baudrate
    hf_can_mode_t mode;                  ///< CAN mode
};

/**
 * @brief PWM channel configuration structure.
 */
struct PwmChannelConfig {
    hf_pwm_channel_t channel_number;     ///< PWM channel number
    hf_gpio_num_t output_pin;            ///< Output pin number
    hf_pwm_freq_t frequency_hz;          ///< PWM frequency
    hf_pwm_resolution_t resolution_bits; ///< PWM resolution
    hf_pwm_mode_t mode;                  ///< PWM mode
};

//==============================================================================
// INITIALIZATION FUNCTIONS
//==============================================================================

/**
 * @brief Initialize GPIO hardware with specified configurations.
 * 
 * This function configures multiple GPIO pins with their specific settings
 * using the underlying MCU GPIO driver. It groups similar configurations
 * to minimize driver calls.
 * 
 * @param pin_configs Array of GPIO pin configurations
 * @param num_pins Number of pins to configure
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeGpio(const GpioPinConfig* pin_configs, size_t num_pins);

/**
 * @brief Initialize ADC hardware with specified channel configurations.
 * 
 * @param channel_configs Array of ADC channel configurations
 * @param num_channels Number of channels to configure
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeAdc(const AdcChannelConfig* channel_configs, size_t num_channels);

/**
 * @brief Initialize I2C bus with specified configuration.
 * 
 * @param bus_config I2C bus configuration
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeI2c(const I2cBusConfig& bus_config);

/**
 * @brief Initialize SPI bus with specified configuration.
 * 
 * @param bus_config SPI bus configuration
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeSpi(const SpiBusConfig& bus_config);

/**
 * @brief Initialize UART with specified configuration.
 * 
 * @param uart_config UART configuration
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeUart(const UartConfig& uart_config);

/**
 * @brief Initialize CAN bus with specified configuration.
 * 
 * @param can_config CAN bus configuration
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeCan(const CanBusConfig& can_config);

/**
 * @brief Initialize PWM channels with specified configurations.
 * 
 * @param channel_configs Array of PWM channel configurations
 * @param num_channels Number of channels to configure
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializePwm(const PwmChannelConfig* channel_configs, size_t num_channels);

/**
 * @brief Initialize all hardware subsystems.
 * 
 * This function performs a complete MCU hardware initialization sequence,
 * setting up all peripherals in the correct order.
 * 
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t initializeAllHardware();

/**
 * @brief Deinitialize all hardware subsystems.
 * 
 * This function performs a complete hardware cleanup, releasing all
 * resources and putting peripherals into low-power states.
 * 
 * @return hf_return_code_t HF_OK on success, error code on failure
 */
hf_return_code_t deinitializeAllHardware();

} // namespace Mcu
} // namespace HardFOC

#endif // MCU_INIT_H
