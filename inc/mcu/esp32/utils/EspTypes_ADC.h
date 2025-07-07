/**
 * @file EspTypes_ADC.h
 * @brief MCU-specific ADC type definitions for hardware abstraction.
 *
 * This header defines all ADC-specific types and constants that are used
 * throughout the internal interface wrap layer for ESP32 ADC operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h"  // For basic hardware types
#include "McuSelect.h"      // Central MCU platform selection (includes all ESP-IDF)
#include "McuTypes_ADC.h"   // For base ADC types and constants

#include "EspTypes_Base.h"
#include "BaseAdc.h"       

//==============================================================================
// [ESP32-C6] SPECIFIC ADC TYPES AND CONSTANTS
//==============================================================================

#ifdef HF_MCU_ESP32C6

/**
 * @brief ADC operating modes supported by ESP32-C6
 */
enum class hf_adc_mode_t : uint8_t {
  ONESHOT = 0,      ///< One-shot mode for single conversions
  CONTINUOUS = 1    ///< Continuous mode with DMA for high-speed sampling
};

/**
 * @brief ADC attenuation levels for ESP32-C6
 * These control the input voltage range that can be measured
 */
enum class hf_adc_atten_t : uint8_t {
  ATTEN_DB_0 = 0,   ///< No attenuation (0dB) - Input range: 0V to ~0.95V 
  ATTEN_DB_2_5 = 1, ///< 2.5dB attenuation - Input range: 0V to ~1.32V
  ATTEN_DB_6 = 2,   ///< 6dB attenuation - Input range: 0V to ~1.98V  
  ATTEN_DB_12 = 3   ///< 12dB attenuation - Input range: 0V to ~3.3V
};

/**
 * @brief ADC resolution/bit width settings for ESP32-C6
 */
enum class hf_adc_bitwidth_t : uint8_t {
  WIDTH_9BIT = 9,   ///< 9-bit resolution (0-511)
  WIDTH_10BIT = 10, ///< 10-bit resolution (0-1023) 
  WIDTH_11BIT = 11, ///< 11-bit resolution (0-2047)
  WIDTH_12BIT = 12, ///< 12-bit resolution (0-4095) - Default and maximum for ESP32-C6
  WIDTH_DEFAULT = 12 ///< Default width (12-bit for ESP32-C6)
};

/**
 * @brief ADC filter coefficient enumeration.
 */
enum class hf_adc_filter_coeff_t : uint8_t {
    COEFF_2 = 0,        ///< Coefficient 2
    COEFF_4 = 1,        ///< Coefficient 4
    COEFF_8 = 2,        ///< Coefficient 8
    COEFF_16 = 3,       ///< Coefficient 16
    COEFF_64 = 4        ///< Coefficient 64
};

/**
 * @brief ADC monitor event type enumeration.
 */
enum class hf_adc_monitor_event_type_t : uint8_t {
    HIGH_THRESH = 0,    ///< High threshold exceeded
    LOW_THRESH = 1      ///< Below low threshold
};

/**
 * @brief ADC channel configuration structure.
 */
struct hf_adc_channel_config_t {
    hf_channel_id_t channel_id;        ///< Channel ID
    hf_adc_atten_t attenuation;        ///< Attenuation level
    hf_adc_bitwidth_t bitwidth;        ///< Bit width
    bool enabled;                      ///< Channel enabled flag

    hf_adc_channel_config_t()
        : channel_id(0), attenuation(hf_adc_atten_t::ATTEN_DB_12), 
          bitwidth(hf_adc_bitwidth_t::WIDTH_DEFAULT), enabled(false) {}
};

/**
 * @brief ADC continuous mode configuration structure.
 */
struct hf_adc_continuous_config_t {
    uint32_t sample_freq_hz;           ///< Sampling frequency in Hz
    uint32_t conv_frame_size;          ///< Conversion frame size
    bool flush_pool;                   ///< Flush pool flag

    hf_adc_continuous_config_t()
        : sample_freq_hz(1000), conv_frame_size(1024), flush_pool(false) {}
};

/**
 * @brief ADC filter configuration structure.
 */
struct hf_adc_filter_config_t {
    uint8_t filter_id;                 ///< Filter ID (0-1)
    hf_channel_id_t channel_id;        ///< Channel ID to filter
    hf_adc_filter_coeff_t coefficient; ///< Filter coefficient

    hf_adc_filter_config_t()
        : filter_id(0), channel_id(0), coefficient(hf_adc_filter_coeff_t::COEFF_4) {}
};

/**
 * @brief ADC monitor configuration structure.
 */
struct hf_adc_monitor_config_t {
    uint8_t monitor_id;                ///< Monitor ID (0-1)
    hf_channel_id_t channel_id;        ///< Channel ID to monitor
    uint32_t high_threshold;           ///< High threshold value (raw)
    uint32_t low_threshold;            ///< Low threshold value (raw)

    hf_adc_monitor_config_t()
        : monitor_id(0), channel_id(0), high_threshold(3000), low_threshold(1000) {}
};

/**
 * @brief ADC calibration configuration structure.
 */
struct hf_adc_calibration_config_t {
    bool enable_calibration;           ///< Enable calibration flag
    bool auto_calibration;             ///< Auto calibration flag
    uint32_t calibration_interval_ms;  ///< Calibration interval in milliseconds

    hf_adc_calibration_config_t()
        : enable_calibration(true), auto_calibration(true), calibration_interval_ms(60000) {}
};

/**
 * @brief ADC unit configuration structure.
 */
struct hf_adc_unit_config_t {
    uint8_t unit_id;                               ///< ADC unit ID
    hf_adc_mode_t mode;                            ///< Operating mode
    hf_adc_bitwidth_t bit_width;                   ///< ADC bit width
    hf_adc_channel_config_t channel_configs[7];    ///< Channel configurations (ESP32-C6 has 7 channels)
    hf_adc_continuous_config_t continuous_config;  ///< Continuous mode configuration
    hf_adc_calibration_config_t calibration_config; ///< Calibration configuration

    hf_adc_unit_config_t()
        : unit_id(1), mode(hf_adc_mode_t::ONESHOT), bit_width(hf_adc_bitwidth_t::WIDTH_DEFAULT) {}
};

/**
 * @brief ADC continuous data structure.
 */
struct hf_adc_continuous_data_t {
    uint8_t* buffer;                   ///< Data buffer
    uint32_t size;                     ///< Data size in bytes
    uint32_t conversion_count;         ///< Number of conversions
    uint64_t timestamp_us;             ///< Timestamp in microseconds

    hf_adc_continuous_data_t()
        : buffer(nullptr), size(0), conversion_count(0), timestamp_us(0) {}
};

/**
 * @brief ADC monitor event structure.
 */
struct hf_adc_monitor_event_t {
    uint8_t monitor_id;                        ///< Monitor ID
    hf_channel_id_t channel_id;                ///< Channel ID
    uint32_t raw_value;                        ///< Raw ADC value
    hf_adc_monitor_event_type_t event_type;    ///< Event type
    uint64_t timestamp_us;                     ///< Timestamp in microseconds

    hf_adc_monitor_event_t()
        : monitor_id(0), channel_id(0), raw_value(0), 
          event_type(hf_adc_monitor_event_type_t::HIGH_THRESH), timestamp_us(0) {}
};

#endif // HF_MCU_ESP32C6

//==============================================================================
// CALLBACK TYPE DEFINITIONS
//==============================================================================

/**
 * @brief ADC continuous mode data callback function.
 * @param data Continuous data structure
 * @param user_data User-provided data
 * @return true to yield to higher priority task, false to continue
 */
using hf_adc_continuous_callback_t = std::function<bool(const hf_adc_continuous_data_t* data, void* user_data)>;

/**
 * @brief ADC threshold monitor callback function.
 * @param event Monitor event structure
 * @param user_data User-provided data
 */
using hf_adc_monitor_callback_t = std::function<void(const hf_adc_monitor_event_t* event, void* user_data)>;

//==============================================================================
// ESP32-C6 SPECIFIC CONSTANTS
//==============================================================================


//==============================================================================
// COMMON ADC UTILITY FUNCTIONS
//==============================================================================

#ifdef HF_MCU_ESP32C6

/**
 * @brief Convert GPIO pin to ADC channel for ESP32-C6
 * @param gpio_pin GPIO pin number (0-6)
 * @return ADC channel ID or HF_INVALID_CHANNEL if invalid
 */
inline constexpr hf_channel_id_t GpioToAdcChannel(hf_pin_num_t gpio_pin) noexcept {
  return (gpio_pin >= 0 && gpio_pin <= 6) ? static_cast<hf_channel_id_t>(gpio_pin) : HF_INVALID_CHANNEL;
}

/**
 * @brief Convert ADC channel to GPIO pin for ESP32-C6  
 * @param channel_id ADC channel ID (0-6)
 * @return GPIO pin number or HF_INVALID_PIN if invalid
 */
inline constexpr hf_pin_num_t AdcChannelToGpio(hf_channel_id_t channel_id) noexcept {
  return (channel_id <= 6) ? static_cast<hf_pin_num_t>(channel_id) : HF_INVALID_PIN;
}

/**
 * @brief Get maximum input voltage for given attenuation
 * @param atten Attenuation level
 * @return Maximum input voltage in millivolts
 */
inline constexpr uint32_t GetMaxInputVoltage(hf_adc_atten_t atten) noexcept {
  switch (atten) {
    case hf_adc_atten_t::ATTEN_DB_0:   return 950;   // ~0.95V
    case hf_adc_atten_t::ATTEN_DB_2_5: return 1320;  // ~1.32V
    case hf_adc_atten_t::ATTEN_DB_6:   return 1980;  // ~1.98V  
    case hf_adc_atten_t::ATTEN_DB_12:  return 3300;  // ~3.3V
    default: return 0;
  }
}

/**
 * @brief Get maximum raw value for given bit width
 * @param bitwidth ADC bit width
 * @return Maximum raw value
 */
inline constexpr uint32_t GetMaxRawValue(hf_adc_bitwidth_t bitwidth) noexcept {
  switch (bitwidth) {
    case hf_adc_bitwidth_t::WIDTH_9BIT:  return 511;   // 2^9 - 1
    case hf_adc_bitwidth_t::WIDTH_10BIT: return 1023;  // 2^10 - 1
    case hf_adc_bitwidth_t::WIDTH_11BIT: return 2047;  // 2^11 - 1
    case hf_adc_bitwidth_t::WIDTH_12BIT: return 4095;  // 2^12 - 1
    default: return 4095;
  }
}

#endif // HF_MCU_ESP32C6

//==============================================================================
//==============================================================================