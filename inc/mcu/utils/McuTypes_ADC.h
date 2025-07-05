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

//==============================================================================
// PLATFORM-SPECIFIC ADC DRIVER IMPORTS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// Include ESP-IDF ADC driver headers
#include "esp_adc/adc_oneshot.h"      // Oneshot mode driver
#include "esp_adc/adc_continuous.h"   // Continuous mode driver  
#include "esp_adc/adc_cali.h"         // Calibration driver
#include "esp_adc/adc_cali_scheme.h"  // Calibration schemes
#include "hal/adc_types.h"            // HAL ADC types
#include "soc/adc_channel.h"          // SoC ADC channel mappings

// ESP32 native ADC type mappings
using hf_adc_unit_native_t = adc_unit_t;
using hf_adc_channel_native_t = adc_channel_t;
using hf_adc_atten_native_t = adc_atten_t;
using hf_adc_bitwidth_native_t = adc_bitwidth_t;
using hf_adc_ulp_mode_native_t = adc_ulp_mode_t;
using hf_adc_oneshot_clk_src_native_t = adc_oneshot_clk_src_t;
using hf_adc_continuous_clk_src_native_t = adc_continuous_clk_src_t;
using hf_adc_digi_convert_mode_native_t = adc_digi_convert_mode_t;
using hf_adc_digi_output_format_native_t = adc_digi_output_format_t;
using hf_adc_cali_scheme_ver_native_t = adc_cali_scheme_ver_t;

using hf_adc_oneshot_unit_handle_t = adc_oneshot_unit_handle_t;
using hf_adc_continuous_handle_t = adc_continuous_handle_t;  
using hf_adc_cali_handle_t = adc_cali_handle_t;
using hf_adc_filter_handle_t = adc_iir_filter_handle_t;
using hf_adc_monitor_handle_t = adc_monitor_handle_t;
#else
// Non-ESP32 platforms - use generic types
using hf_adc_unit_native_t = uint8_t;
using hf_adc_channel_native_t = uint8_t;
using hf_adc_atten_native_t = uint8_t;
using hf_adc_bitwidth_native_t = uint8_t;
using hf_adc_ulp_mode_native_t = uint8_t;
using hf_adc_oneshot_clk_src_native_t = uint8_t;
using hf_adc_continuous_clk_src_native_t = uint8_t;
using hf_adc_digi_convert_mode_native_t = uint8_t;
using hf_adc_digi_output_format_native_t = uint8_t;
using hf_adc_cali_scheme_ver_native_t = uint8_t;


// Generic handle types for non-ESP32 platforms
using hf_adc_oneshot_unit_handle_t = void*;
using hf_adc_continuous_handle_t = void*;
using hf_adc_cali_handle_t = void*;
using hf_adc_filter_handle_t = void*;
using hf_adc_monitor_handle_t = void*;
#endif

//==============================================================================
// HF NATIVE ENUM MAPPINGS FROM ESP-IDF TYPES
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
/**
 * @brief HF ADC unit mapping from ESP-IDF native types.
 */
enum class hf_adc_unit_t : uint8_t {
  HF_ADC_UNIT_1 = ADC_UNIT_1,    ///< SAR ADC 1
  HF_ADC_UNIT_2 = ADC_UNIT_2,    ///< SAR ADC 2
};

/**
 * @brief HF ADC channel mapping from ESP-IDF native types.
 */
enum class hf_adc_channel_t : uint8_t {
  HF_ADC_CHANNEL_0 = ADC_CHANNEL_0,
  HF_ADC_CHANNEL_1 = ADC_CHANNEL_1,
  HF_ADC_CHANNEL_2 = ADC_CHANNEL_2,
  HF_ADC_CHANNEL_3 = ADC_CHANNEL_3,
  HF_ADC_CHANNEL_4 = ADC_CHANNEL_4,
  HF_ADC_CHANNEL_5 = ADC_CHANNEL_5,
  HF_ADC_CHANNEL_6 = ADC_CHANNEL_6,
  HF_ADC_CHANNEL_7 = ADC_CHANNEL_7,
  HF_ADC_CHANNEL_8 = ADC_CHANNEL_8,
  HF_ADC_CHANNEL_9 = ADC_CHANNEL_9,
};

/**
 * @brief HF ADC attenuation mapping from ESP-IDF native types.
 */
enum class hf_adc_attenuation_t : uint8_t {
  HF_ADC_ATTEN_DB_0 = ADC_ATTEN_DB_0,      ///< No input attenuation (~1.1V max)
  HF_ADC_ATTEN_DB_2_5 = ADC_ATTEN_DB_2_5,  ///< 2.5dB attenuation (~1.5V max)
  HF_ADC_ATTEN_DB_6 = ADC_ATTEN_DB_6,      ///< 6dB attenuation (~2.2V max)
  HF_ADC_ATTEN_DB_11 = ADC_ATTEN_DB_11,    ///< 11dB attenuation (~3.9V max)
  HF_ADC_ATTEN_DB_12 = ADC_ATTEN_DB_12,    ///< 12dB attenuation (same as 11dB)
};

/**
 * @brief HF ADC bitwidth mapping from ESP-IDF native types.
 */
enum class hf_adc_bitwidth_t : uint8_t {
  HF_ADC_BITWIDTH_DEFAULT = ADC_BITWIDTH_DEFAULT,  ///< Default width (max supported)
  HF_ADC_BITWIDTH_9 = ADC_BITWIDTH_9,              ///< 9-bit resolution
  HF_ADC_BITWIDTH_10 = ADC_BITWIDTH_10,            ///< 10-bit resolution
  HF_ADC_BITWIDTH_11 = ADC_BITWIDTH_11,            ///< 11-bit resolution
  HF_ADC_BITWIDTH_12 = ADC_BITWIDTH_12,            ///< 12-bit resolution
  HF_ADC_BITWIDTH_13 = ADC_BITWIDTH_13,            ///< 13-bit resolution
};

/**
 * @brief HF ADC ULP mode mapping from ESP-IDF native types.
 */
enum class hf_adc_ulp_mode_t : uint8_t {
  HF_ADC_ULP_MODE_DISABLE = ADC_ULP_MODE_DISABLE,  ///< ADC ULP mode disabled
  HF_ADC_ULP_MODE_FSM = ADC_ULP_MODE_FSM,          ///< ADC controlled by ULP FSM
  HF_ADC_ULP_MODE_RISCV = ADC_ULP_MODE_RISCV,      ///< ADC controlled by ULP RISCV
};

/**
 * @brief HF ADC continuous mode convert mode mapping from ESP-IDF native types.
 */
enum class hf_adc_digi_convert_mode_t : uint8_t {
  HF_ADC_CONV_SINGLE_UNIT_1 = ADC_CONV_SINGLE_UNIT_1,  ///< Only use ADC1
  HF_ADC_CONV_SINGLE_UNIT_2 = ADC_CONV_SINGLE_UNIT_2,  ///< Only use ADC2  
  HF_ADC_CONV_BOTH_UNIT = ADC_CONV_BOTH_UNIT,          ///< Use both ADC1 and ADC2 simultaneously
  HF_ADC_CONV_ALTER_UNIT = ADC_CONV_ALTER_UNIT,        ///< Use both ADC1 and ADC2 alternately
};

/**
 * @brief HF ADC continuous mode output format mapping from ESP-IDF native types.
 */
enum class hf_adc_digi_output_format_t : uint8_t {
  HF_ADC_DIGI_OUTPUT_FORMAT_TYPE1 = ADC_DIGI_OUTPUT_FORMAT_TYPE1,  ///< Type1 format
  HF_ADC_DIGI_OUTPUT_FORMAT_TYPE2 = ADC_DIGI_OUTPUT_FORMAT_TYPE2,  ///< Type2 format
};

/**
 * @brief HF ADC calibration scheme mapping from ESP-IDF native types.
 */
enum class hf_adc_calibration_scheme_t : uint8_t {
  HF_ADC_CALI_SCHEME_LINE_FITTING = ADC_CALI_SCHEME_VER_LINE_FITTING,    ///< Line fitting scheme
  HF_ADC_CALI_SCHEME_CURVE_FITTING = ADC_CALI_SCHEME_VER_CURVE_FITTING,  ///< Curve fitting scheme
};

/**
 * @brief HF ADC oneshot clock source mapping from ESP-IDF native types.
 */
enum class hf_adc_oneshot_clk_src_t : uint8_t {
  HF_ADC_ONESHOT_CLK_SRC_DEFAULT = 0,  ///< Default clock source (system chooses)
  HF_ADC_ONESHOT_CLK_SRC_APB = 1,      ///< APB clock source
  HF_ADC_ONESHOT_CLK_SRC_XTAL = 2,     ///< XTAL clock source
};

/**
 * @brief HF ADC continuous clock source mapping from ESP-IDF native types.
 */
enum class hf_adc_continuous_clk_src_t : uint8_t {
  HF_ADC_CONTINUOUS_CLK_SRC_DEFAULT = 0,  ///< Default clock source (system chooses)
  HF_ADC_CONTINUOUS_CLK_SRC_APB = 1,      ///< APB clock source
  HF_ADC_CONTINUOUS_CLK_SRC_XTAL = 2,     ///< XTAL clock source
};

#else
// Non-ESP32 generic enum definitions
enum class hf_adc_unit_t : uint8_t {
  HF_ADC_UNIT_1 = 1,
  HF_ADC_UNIT_2 = 2,
};

enum class hf_adc_channel_t : uint8_t {
  HF_ADC_CHANNEL_0 = 0, HF_ADC_CHANNEL_1 = 1, HF_ADC_CHANNEL_2 = 2, HF_ADC_CHANNEL_3 = 3,
  HF_ADC_CHANNEL_4 = 4, HF_ADC_CHANNEL_5 = 5, HF_ADC_CHANNEL_6 = 6, HF_ADC_CHANNEL_7 = 7,
  HF_ADC_CHANNEL_8 = 8, HF_ADC_CHANNEL_9 = 9,
};

enum class hf_adc_attenuation_t : uint8_t {
  HF_ADC_ATTEN_DB_0 = 0, HF_ADC_ATTEN_DB_2_5 = 1, HF_ADC_ATTEN_DB_6 = 2, 
  HF_ADC_ATTEN_DB_11 = 3, HF_ADC_ATTEN_DB_12 = 3,
};

enum class hf_adc_bitwidth_t : uint8_t {
  HF_ADC_BITWIDTH_DEFAULT = 12, HF_ADC_BITWIDTH_9 = 9, HF_ADC_BITWIDTH_10 = 10,
  HF_ADC_BITWIDTH_11 = 11, HF_ADC_BITWIDTH_12 = 12, HF_ADC_BITWIDTH_13 = 13,
};

enum class hf_adc_ulp_mode_t : uint8_t {
  HF_ADC_ULP_MODE_DISABLE = 0, HF_ADC_ULP_MODE_FSM = 1, HF_ADC_ULP_MODE_RISCV = 2,
};

enum class hf_adc_digi_convert_mode_t : uint8_t {
  HF_ADC_CONV_SINGLE_UNIT_1 = 0, HF_ADC_CONV_SINGLE_UNIT_2 = 1,
  HF_ADC_CONV_BOTH_UNIT = 2, HF_ADC_CONV_ALTER_UNIT = 3,
};

enum class hf_adc_digi_output_format_t : uint8_t {
  HF_ADC_DIGI_OUTPUT_FORMAT_TYPE1 = 0, HF_ADC_DIGI_OUTPUT_FORMAT_TYPE2 = 1,
};

enum class hf_adc_calibration_scheme_t : uint8_t {
  HF_ADC_CALI_SCHEME_LINE_FITTING = 0, HF_ADC_CALI_SCHEME_CURVE_FITTING = 1,
};

enum class hf_adc_oneshot_clk_src_t : uint8_t {
  HF_ADC_ONESHOT_CLK_SRC_DEFAULT = 0, HF_ADC_ONESHOT_CLK_SRC_APB = 1, HF_ADC_ONESHOT_CLK_SRC_XTAL = 2,
};

enum class hf_adc_continuous_clk_src_t : uint8_t {
  HF_ADC_CONTINUOUS_CLK_SRC_DEFAULT = 0, HF_ADC_CONTINUOUS_CLK_SRC_APB = 1, HF_ADC_CONTINUOUS_CLK_SRC_XTAL = 2,
};
#endif

//==============================================================================
// PLATFORM-AGNOSTIC ADC CONFIGURATION TYPES
//==============================================================================

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

/**
 * @brief Platform-agnostic ADC oneshot configuration.
 */
struct hf_adc_oneshot_config_t {
  hf_adc_unit_t unit;                    ///< ADC unit to use
  hf_adc_channel_t channel;              ///< ADC channel
  hf_adc_attenuation_t attenuation;      ///< Attenuation setting
  hf_adc_bitwidth_t bitwidth;            ///< Resolution setting
  bool enable_calibration;               ///< Enable voltage calibration
  
  hf_adc_oneshot_config_t() noexcept
      : unit(hf_adc_unit_t::HF_ADC_UNIT_1), 
        channel(hf_adc_channel_t::HF_ADC_CHANNEL_0),
        attenuation(hf_adc_attenuation_t::HF_ADC_ATTEN_DB_11),
        bitwidth(hf_adc_bitwidth_t::HF_ADC_BITWIDTH_12),
        enable_calibration(true) {}
};

/**
 * @brief Platform-agnostic ADC continuous mode configuration.
 */
struct hf_adc_continuous_config_t {
  uint32_t sample_freq_hz;               ///< Sampling frequency in Hz
  hf_adc_digi_convert_mode_t conv_mode;  ///< Conversion mode
  hf_adc_digi_output_format_t format;    ///< Output data format  
  size_t buffer_size;                    ///< DMA buffer size
  uint8_t buffer_count;                  ///< Number of DMA buffers
  bool enable_dma;                       ///< Enable DMA transfers
  
  hf_adc_continuous_config_t() noexcept
      : sample_freq_hz(HF_ADC_DEFAULT_SAMPLING_FREQ), 
        conv_mode(hf_adc_digi_convert_mode_t::HF_ADC_CONV_SINGLE_UNIT_1),
        format(hf_adc_digi_output_format_t::HF_ADC_DIGI_OUTPUT_FORMAT_TYPE2), 
        buffer_size(HF_ADC_DMA_BUFFER_SIZE_DEFAULT),
        buffer_count(2), enable_dma(true) {}
};

/**
 * @brief Platform-agnostic ADC channel configuration.
 */
struct hf_adc_channel_config_t {
  hf_adc_channel_t channel;         ///< ADC channel
  hf_adc_attenuation_t attenuation; ///< Attenuation setting
  hf_adc_bitwidth_t bitwidth;       ///< Resolution setting
  bool enable_filter;               ///< Enable digital filter
  uint8_t filter_coeff;             ///< IIR filter coefficient (0-15)
  
  hf_adc_channel_config_t() noexcept
      : channel(hf_adc_channel_t::HF_ADC_CHANNEL_0), 
        attenuation(hf_adc_attenuation_t::HF_ADC_ATTEN_DB_11),
        bitwidth(hf_adc_bitwidth_t::HF_ADC_BITWIDTH_12), 
        enable_filter(false), filter_coeff(2) {}
};

//==============================================================================
// ESP32C6 ADC CONSTANTS AND VALIDATION MACROS
//==============================================================================

#ifdef HF_TARGET_MCU_ESP32C6
/**
 * @brief ESP32C6 ADC specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 1 ADC controller (ADC1) with advanced features:
 * - 7 channels (ADC_CHANNEL_0 to ADC_CHANNEL_6) mapped to GPIO0-6
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
static constexpr uint8_t HF_ADC_MAX_CHANNELS = 7;                 // ADC_CHANNEL_0 to ADC_CHANNEL_6
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
static constexpr uint32_t HF_ADC_DEFAULT_SAMPLING_FREQ = 1000;
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MIN = 256;
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_MAX = 4096;
static constexpr uint32_t HF_ADC_DMA_BUFFER_SIZE_DEFAULT = 1024;

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
