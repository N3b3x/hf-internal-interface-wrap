/**
 * @file McuTypes_ADC.h
 * @brief MCU-specific ADC type definitions for hardware abstraction.
 *
 * This header defines all ADC-specific types and constants that are used
 * throughout the internal interface wrap layer for ADC operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"
// Include ADC-specific headers for ESP32C6
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "soc/adc_channel.h"

//==============================================================================
// PLATFORM-SPECIFIC ADC TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 ADC specific mappings
using hf_adc_unit_native_t = adc_unit_t;
using hf_adc_channel_native_t = adc_channel_t;
using hf_adc_atten_native_t = adc_atten_t;
using hf_adc_bitwidth_native_t = adc_bitwidth_t;
#else
// Non-ESP32 platforms - use generic types
using hf_adc_unit_native_t = uint8_t;
using hf_adc_channel_native_t = uint8_t;
using hf_adc_atten_native_t = uint8_t;
using hf_adc_bitwidth_native_t = uint8_t;
#endif

//==============================================================================
// MCU-SPECIFIC ADC TYPES
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

//==============================================================================
// ESP32C6 ADC CONSTANTS AND VALIDATION MACROS
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6
static constexpr uint8_t HF_ADC_MAX_UNITS = 2;            ///< Maximum ADC units
static constexpr uint8_t HF_ADC_MAX_CHANNELS = 7;         ///< Maximum ADC channels per unit
static constexpr uint32_t HF_ADC_MIN_SAMPLING_FREQ = 1;   ///< Minimum sampling frequency
static constexpr uint32_t HF_ADC_MAX_SAMPLING_FREQ = 83333; ///< Maximum sampling frequency
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MIN = 256; ///< Minimum DMA buffer size
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MAX = 4096; ///< Maximum DMA buffer size

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

#define HF_ADC_IS_VALID_UNIT(unit) ((unit) <= HF_ADC_MAX_UNITS)
#define HF_ADC_IS_VALID_CHANNEL(ch) ((ch) < HF_ADC_MAX_CHANNELS)
#define HF_ADC_IS_VALID_SAMPLING_FREQ(freq) \
  ((freq) >= HF_ADC_MIN_SAMPLING_FREQ && (freq) <= HF_ADC_MAX_SAMPLING_FREQ)
#define HF_ADC_IS_VALID_RESOLUTION(res) \
  ((res) >= 9 && (res) <= 12)
#define HF_ADC_IS_VALID_ATTENUATION(atten) ((atten) <= 3)
#define HF_ADC_IS_VALID_BUFFER_SIZE(size) \
  ((size) >= HF_ADC_DMA_BUFFER_SIZE_MIN && (size) <= HF_ADC_DMA_BUFFER_SIZE_MAX)
#else
// Generic constants for non-ESP32C6 platforms
static constexpr uint8_t HF_ADC_MAX_UNITS = 2;
static constexpr uint8_t HF_ADC_MAX_CHANNELS = 8;
static constexpr uint32_t HF_ADC_MIN_SAMPLING_FREQ = 1;
static constexpr uint32_t HF_ADC_MAX_SAMPLING_FREQ = 100000;
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MIN = 256;
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MAX = 4096;

#define HF_ADC_IS_VALID_UNIT(unit) ((unit) <= HF_ADC_MAX_UNITS)
#define HF_ADC_IS_VALID_CHANNEL(ch) ((ch) < HF_ADC_MAX_CHANNELS)
#define HF_ADC_IS_VALID_SAMPLING_FREQ(freq) \
  ((freq) >= HF_ADC_MIN_SAMPLING_FREQ && (freq) <= HF_ADC_MAX_SAMPLING_FREQ)
#define HF_ADC_IS_VALID_RESOLUTION(res) \
  ((res) >= 8 && (res) <= 12)
#define HF_ADC_IS_VALID_ATTENUATION(atten) ((atten) <= 3)
#define HF_ADC_IS_VALID_BUFFER_SIZE(size) \
  ((size) >= HF_ADC_DMA_BUFFER_SIZE_MIN && (size) <= HF_ADC_DMA_BUFFER_SIZE_MAX)
#endif
