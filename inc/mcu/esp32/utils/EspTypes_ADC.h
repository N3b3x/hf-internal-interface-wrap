/**
 * @file EspTypes_ADC.h
 * @brief ESP32 ADC type definitions for hardware abstraction.
 *
 * This header defines only the essential ADC-specific types and constants used by
 * the EspAdc implementation. It follows a clean, minimal pattern providing only
 * necessary types without redundant or duplicate definitions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h"
#include "McuSelect.h"      // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"
#include "BaseAdc.h"       

//==============================================================================
// ESSENTIAL ADC TYPES (ESP32)
//==============================================================================

/**
 * @brief ADC operating modes supported by ESP32
 */
enum class hf_adc_mode_t : uint8_t {
  ONESHOT = 0,      ///< One-shot mode for single conversions
  CONTINUOUS = 1    ///< Continuous mode with DMA for high-speed sampling
};

/**
 * @brief ADC attenuation levels for ESP32
 * These control the input voltage range that can be measured
 * Values must match ESP-IDF adc_atten_t enum
 */
enum class hf_adc_atten_t : uint8_t {
  ATTEN_DB_0 = ADC_ATTEN_DB_0,     ///< No attenuation (0dB) - Input range: 0V to ~0.95V 
  ATTEN_DB_2_5 = ADC_ATTEN_DB_2_5, ///< 2.5dB attenuation - Input range: 0V to ~1.32V
  ATTEN_DB_6 = ADC_ATTEN_DB_6,     ///< 6dB attenuation - Input range: 0V to ~1.98V  
  ATTEN_DB_12 = ADC_ATTEN_DB_12    ///< 12dB attenuation - Input range: 0V to ~3.3V
};

/**
 * @brief ADC resolution/bit width settings for ESP32
 * Values must match ESP-IDF adc_bitwidth_t enum
 */
enum class hf_adc_bitwidth_t : uint8_t {
  WIDTH_9BIT = ADC_BITWIDTH_9,     ///< 9-bit resolution (0-511)
  WIDTH_10BIT = ADC_BITWIDTH_10,   ///< 10-bit resolution (0-1023) 
  WIDTH_11BIT = ADC_BITWIDTH_11,   ///< 11-bit resolution (0-2047)
  WIDTH_12BIT = ADC_BITWIDTH_12,   ///< 12-bit resolution (0-4095) - Default for ESP32
  WIDTH_13BIT = ADC_BITWIDTH_13,   ///< 13-bit resolution (0-8191)
  WIDTH_DEFAULT = ADC_BITWIDTH_DEFAULT ///< Default width (12-bit for ESP32)
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
 * 
 * This structure provides a user-friendly way to configure continuous mode ADC.
 * The frame size is automatically calculated based on samples_per_frame and enabled channels.
 */
struct hf_adc_continuous_config_t {
    uint32_t sample_freq_hz;           ///< Sampling frequency in Hz
    uint32_t samples_per_frame;        ///< Number of samples per frame per enabled channel (64-1024 recommended)
    uint32_t max_store_frames;         ///< Maximum number of frames to store in buffer pool (1-8 recommended)
    bool flush_pool;                   ///< Flush pool flag

    hf_adc_continuous_config_t()
        : sample_freq_hz(1000), samples_per_frame(64), max_store_frames(4), flush_pool(false) {}
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
    hf_adc_channel_config_t channel_configs[7];    ///< Channel configurations (ESP32 has 7 channels)
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

//==============================================================================
// CALLBACK TYPE DEFINITIONS
//==============================================================================

/**
 * @brief ADC continuous mode data callback function.
 * 
 * @warning This callback is executed in ISR context and must be ISR-safe:
 * - Use only ISR-safe functions (no malloc, free, printf, etc.)
 * - Keep execution time as short as possible
 * - Avoid calling blocking functions or FreeRTOS APIs that are not ISR-safe
 * - Use only stack variables or pre-allocated memory
 * - Consider using xQueueSendFromISR() or similar to defer processing
 * 
 * @param data Continuous data structure containing sampled data
 * @param user_data User-provided data pointer
 * @return true to yield to higher priority task, false to continue
 */
using hf_adc_continuous_callback_t = bool (*)(const hf_adc_continuous_data_t* data, void* user_data);

/**
 * @brief ADC threshold monitor callback function.
 * 
 * @warning This callback is executed in ISR context and must be ISR-safe:
 * - Use only ISR-safe functions (no malloc, free, printf, etc.)
 * - Keep execution time as short as possible
 * - Avoid calling blocking functions or FreeRTOS APIs that are not ISR-safe
 * - Use only stack variables or pre-allocated memory
 * - Consider using xQueueSendFromISR() or similar to defer processing
 * 
 * @param event Monitor event structure containing threshold event details
 * @param user_data User-provided data pointer
 */
using hf_adc_monitor_callback_t = void (*)(const hf_adc_monitor_event_t* event, void* user_data);

/**
 * @brief Example of ISR-safe ADC callback implementation
 * 
 * @code{.cpp}
 * // Queue handle (global or class member)
 * static QueueHandle_t adc_data_queue;
 * 
 * // ISR-safe callback function
 * bool adc_continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) {
 *     // Create a lightweight message for the queue
 *     AdcDataMessage msg;
 *     msg.timestamp = data->timestamp_us;
 *     msg.conversion_count = data->conversion_count;
 *     msg.size = data->size;
 *     
 *     // Try to send to queue (non-blocking)
 *     BaseType_t higher_priority_task_woken = pdFALSE;
 *     if (xQueueSendFromISR(adc_data_queue, &msg, &higher_priority_task_woken) == pdTRUE) {
 *         return higher_priority_task_woken == pdTRUE;
 *     }
 *     
 *     return false; // Continue receiving callbacks
 * }
 * @endcode
 */

//==============================================================================
// ESP32 ADC CONSTANTS
//==============================================================================

/**
 * @brief ESP32 ADC continuous mode constants
 */
constexpr uint32_t HF_ESP32_ADC_DATA_BYTES_PER_CONV = SOC_ADC_DIGI_DATA_BYTES_PER_CONV;  ///< Bytes per conversion result from ESP-IDF
constexpr uint32_t HF_ESP32_ADC_MIN_FRAME_SIZE = 64;       ///< Minimum frame size
constexpr uint32_t HF_ESP32_ADC_MAX_FRAME_SIZE = 1024;     ///< Maximum frame size
constexpr uint32_t HF_ESP32_ADC_DEFAULT_FRAME_SIZE = 256;  ///< Default frame size

/**
 * @brief Calculate frame size in bytes based on samples per frame and enabled channels
 * @param samples_per_frame Number of samples per frame per enabled channel
 * @param enabled_channels Number of enabled channels
 * @return Frame size in bytes
 */
inline constexpr uint32_t CalcFrameSize(uint32_t samples_per_frame, uint32_t enabled_channels) noexcept {
    return samples_per_frame * enabled_channels * HF_ESP32_ADC_DATA_BYTES_PER_CONV;
}

/**
 * @brief Calculate total buffer pool size based on frames and enabled channels
 * @param samples_per_frame Number of samples per frame per enabled channel
 * @param enabled_channels Number of enabled channels  
 * @param max_store_frames Maximum frames to store
 * @return Buffer pool size in bytes
 */
inline constexpr uint32_t CalcBufferPoolSize(uint32_t samples_per_frame, uint32_t enabled_channels, uint32_t max_store_frames) noexcept {
    return CalcFrameSize(samples_per_frame, enabled_channels) * max_store_frames;
}

/**
 * @brief Validate continuous mode configuration parameters
 * @param samples_per_frame Number of samples per frame per enabled channel
 * @param enabled_channels Number of enabled channels
 * @param max_store_frames Maximum frames to store
 * @return true if valid, false otherwise
 */
inline constexpr bool IsValidContinuousConfig(uint32_t samples_per_frame, uint32_t enabled_channels, uint32_t max_store_frames) noexcept {
    if (enabled_channels == 0 || samples_per_frame == 0 || max_store_frames == 0) {
        return false;
    }
    
    uint32_t frame_size = CalcFrameSize(samples_per_frame, enabled_channels);
    uint32_t pool_size = CalcBufferPoolSize(samples_per_frame, enabled_channels, max_store_frames);
    
    return (frame_size >= HF_ESP32_ADC_MIN_FRAME_SIZE) &&
           (frame_size <= HF_ESP32_ADC_MAX_FRAME_SIZE) &&
           (pool_size <= 32768) && // Reasonable max pool size (32KB)
           ((frame_size % HF_ESP32_ADC_DATA_BYTES_PER_CONV) == 0);
}

/**
 * @brief Validate that frame size is properly aligned
 * @param frame_size Frame size in bytes
 * @return true if valid, false otherwise
 */
inline constexpr bool IsValidFrameSize(uint32_t frame_size) noexcept {
    return (frame_size >= HF_ESP32_ADC_MIN_FRAME_SIZE) &&
           (frame_size <= HF_ESP32_ADC_MAX_FRAME_SIZE) &&
           ((frame_size % HF_ESP32_ADC_DATA_BYTES_PER_CONV) == 0);
}

/**
 * @brief Calculate number of conversion results that fit in a frame
 * @param frame_size Frame size in bytes
 * @return Number of conversion results
 */
inline constexpr uint32_t GetFrameResultCount(uint32_t frame_size) noexcept {
    return frame_size / HF_ESP32_ADC_DATA_BYTES_PER_CONV;
}

//==============================================================================
// COMMON ADC UTILITY FUNCTIONS
//==============================================================================

/**
 * @brief Convert GPIO pin to ADC channel for ESP32
 * @note This is a simplified mapping function. For accurate conversions,
 *       use the ESP-IDF adc_continuous_io_to_channel() function at runtime.
 * @param gpio_pin GPIO pin number
 * @return ADC channel ID or HF_INVALID_CHANNEL if invalid
 */
inline constexpr hf_channel_id_t GpioToAdcChannel(hf_pin_num_t gpio_pin) noexcept {
  // Note: This is a simplified compile-time function.
  // For accurate GPIO-to-channel conversion, use adc_continuous_io_to_channel() 
  // at runtime, which handles all ESP32 variants correctly.
  return (gpio_pin >= 0 && gpio_pin <= 6) ? static_cast<hf_channel_id_t>(gpio_pin) : HF_INVALID_CHANNEL;
}

/**
 * @brief Convert ADC channel to GPIO pin for ESP32  
 * @note This is a simplified mapping function. For accurate conversions,
 *       use the ESP-IDF adc_continuous_channel_to_io() function at runtime.
 * @param channel_id ADC channel ID
 * @return GPIO pin number or HF_INVALID_PIN if invalid
 */
inline constexpr hf_pin_num_t AdcChannelToGpio(hf_channel_id_t channel_id) noexcept {
  // Note: This is a simplified compile-time function.
  // For accurate channel-to-GPIO conversion, use adc_continuous_channel_to_io() 
  // at runtime, which handles all ESP32 variants correctly.
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

//==============================================================================
// END OF ESPADC TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================