/**
 * @file McuTypes_PWM.h
 * @brief MCU-specific PWM type definitions for hardware abstraction.
 *
 * This header defines all PWM-specific types and constants that are used
 * throughout the internal interface wrap layer for PWM operations. This includes
 * ESP32C6 LEDC controller support with advanced features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"
#include "BasePwm.h" // For HfPwmErr

//==============================================================================
// ESP32C6 PWM/LEDC TYPES AND CONSTANTS (ESP-IDF v5.5+)
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief ESP32C6 LEDC (PWM) controller specifications - based on ESP-IDF v5.5+ documentation.
 * @details The ESP32C6 has 8 LEDC channels (0-7) with advanced features:
 * - High-resolution PWM with up to 14-bit resolution at high frequencies
 * - Hardware fade functionality for smooth transitions
 * - 4 independent timer groups for different frequency domains
 * - Low-power mode support with sleep retention
 * - Interrupt-driven operation with period and fade callbacks
 * - Configurable clock sources (APB, XTAL, RC_FAST)
 * - Channel-specific idle state configuration
 */
static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;          ///< Maximum PWM channels (ESP32C6 LEDC)
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;            ///< Maximum timer groups
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 14;       ///< Maximum resolution bits
static constexpr uint32_t HF_PWM_MIN_FREQUENCY = 1;        ///< Minimum frequency (Hz)
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 40000000; ///< Maximum frequency (Hz)
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000; ///< Default frequency (Hz)
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 12;   ///< Default resolution bits
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;  ///< ESP32C6 APB clock frequency

/**
 * @brief PWM clock source selection for ESP32C6.
 * @details Clock source options for power optimization and precision.
 */
enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0, ///< Default PWM clock source (APB)
  HF_PWM_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_PWM_CLK_SRC_XTAL = 2,    ///< Crystal oscillator (40MHz, lower power)
  HF_PWM_CLK_SRC_RC_FAST = 3  ///< RC fast clock (~8MHz, lowest power)
};

/**
 * @brief PWM timer resolution for different frequency ranges.
 * @details ESP32C6 LEDC can achieve different resolutions based on frequency:
 * - 1kHz: up to 14-bit resolution
 * - 10kHz: up to 12-bit resolution
 * - 100kHz: up to 10-bit resolution
 * - 1MHz: up to 8-bit resolution
 */
enum class hf_pwm_resolution_t : uint8_t {
  HF_PWM_RES_8BIT = 8,   ///< 8-bit resolution (256 levels)
  HF_PWM_RES_10BIT = 10, ///< 10-bit resolution (1024 levels)
  HF_PWM_RES_12BIT = 12, ///< 12-bit resolution (4096 levels)
  HF_PWM_RES_14BIT = 14  ///< 14-bit resolution (16384 levels)
};

/**
 * @brief PWM operating modes for different applications.
 */
enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_LOW_SPEED = 0,  ///< Low speed mode (default)
  HF_PWM_MODE_HIGH_SPEED = 1  ///< High speed mode (legacy, use low speed)
};

/**
 * @brief PWM fade modes for smooth transitions.
 */
enum class hf_pwm_fade_mode_t : uint8_t {
  HF_PWM_FADE_NO_WAIT = 0,     ///< Non-blocking fade
  HF_PWM_FADE_WAIT_DONE = 1    ///< Blocking fade (wait for completion)
};

/**
 * @brief PWM interrupt types for callbacks.
 */
enum class hf_pwm_intr_type_t : uint8_t {
  HF_PWM_INTR_DISABLE = 0,     ///< Disable interrupts
  HF_PWM_INTR_FADE_END = 1     ///< Fade end interrupt
};

/**
 * @brief Native ESP-IDF LEDC type mappings for PWM.
 * @details Direct mappings to ESP-IDF LEDC types for maximum compatibility.
 */
using hf_pwm_channel_native_t = ledc_channel_t;
using hf_pwm_timer_native_t = ledc_timer_t;
using hf_pwm_mode_native_t = ledc_mode_t;
using hf_pwm_timer_bit_native_t = ledc_timer_bit_t;
using hf_pwm_clk_cfg_native_t = ledc_clk_cfg_t;
using hf_pwm_channel_config_native_t = ledc_channel_config_t;
using hf_pwm_timer_config_native_t = ledc_timer_config_t;
using hf_pwm_fade_mode_native_t = ledc_fade_mode_t;
using hf_pwm_intr_type_native_t = ledc_intr_type_t;

/**
 * @brief ESP32C6 PWM timing configuration with optimization support.
 * @details Platform-specific timing parameters optimized for ESP32C6 80MHz APB clock.
 */
struct hf_pwm_timing_config_t {
  uint32_t frequency_hz;       ///< PWM frequency in Hz
  uint8_t resolution_bits;     ///< PWM resolution (8-14 bits)
  hf_pwm_clock_source_t clk_src; ///< Clock source selection
  uint32_t clk_divider;        ///< Clock divider (calculated automatically)
  
  // Calculated timing parameters
  uint32_t actual_frequency_hz; ///< Actual achieved frequency
  float frequency_accuracy;     ///< Frequency accuracy percentage
  uint32_t period_ticks;        ///< Period in timer ticks
  uint32_t max_duty_ticks;      ///< Maximum duty cycle ticks
  
  hf_pwm_timing_config_t() noexcept
      : frequency_hz(HF_PWM_DEFAULT_FREQUENCY), resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        clk_src(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT), clk_divider(0),
        actual_frequency_hz(0), frequency_accuracy(0.0f), period_ticks(0), max_duty_ticks(0) {}
};

/**
 * @brief ESP32C6 PWM channel configuration with advanced features.
 * @details Comprehensive configuration for ESP-IDF v5.5+ LEDC channel features.
 */
struct hf_pwm_channel_config_t {
  hf_gpio_num_t gpio_pin;           ///< GPIO pin for PWM output
  uint8_t channel_id;               ///< Channel ID (0-7)
  uint8_t timer_id;                 ///< Timer ID (0-3)
  hf_pwm_mode_t speed_mode;         ///< Speed mode configuration
  uint32_t duty_initial;            ///< Initial duty cycle value
  hf_pwm_intr_type_t intr_type;     ///< Interrupt type
  bool invert_output;               ///< Invert output signal
  
  // Advanced configuration
  uint32_t hpoint;                  ///< High point timing (phase shift)
  uint8_t idle_level;               ///< Idle state level (0 or 1)
  bool output_invert;               ///< Hardware output inversion
  
  hf_pwm_channel_config_t() noexcept
      : gpio_pin(static_cast<hf_gpio_num_t>(HF_INVALID_PIN)), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_LOW_SPEED), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false),
        hpoint(0), idle_level(0), output_invert(false) {}
};

/**
 * @brief ESP32C6 PWM fade configuration for smooth transitions.
 */
struct hf_pwm_fade_config_t {
  uint32_t target_duty;             ///< Target duty cycle value
  uint32_t fade_time_ms;            ///< Fade duration in milliseconds
  hf_pwm_fade_mode_t fade_mode;     ///< Fade mode (blocking/non-blocking)
  uint32_t scale;                   ///< Fade scale factor
  uint32_t cycle_num;               ///< Number of fade cycles
  
  hf_pwm_fade_config_t() noexcept
      : target_duty(0), fade_time_ms(1000),
        fade_mode(hf_pwm_fade_mode_t::HF_PWM_FADE_NO_WAIT),
        scale(0), cycle_num(0) {}
};

/**
 * @brief PWM capabilities and limitations for ESP32C6.
 * @details Static capability information for runtime feature detection.
 */
struct hf_pwm_capabilities_t {
  uint8_t num_channels;             ///< Number of PWM channels (8 for ESP32C6)
  uint8_t num_timers;               ///< Number of timer groups (4 for ESP32C6)
  uint8_t max_resolution_bits;      ///< Maximum resolution bits (14 for ESP32C6)
  uint32_t max_frequency_hz;        ///< Maximum supported frequency
  uint32_t min_frequency_hz;        ///< Minimum supported frequency
  bool supports_fade;               ///< Hardware fade support
  bool supports_sleep_retention;    ///< Sleep retention support
  bool supports_complementary;      ///< Complementary outputs (software)
  bool supports_deadtime;           ///< Deadtime insertion (software)
  bool supports_phase_shift;        ///< Phase shifting support
  uint8_t available_clock_sources;  ///< Number of available clock sources
  
  // Constructor with ESP32C6 defaults
  hf_pwm_capabilities_t() noexcept
      : num_channels(HF_PWM_MAX_CHANNELS), num_timers(HF_PWM_MAX_TIMERS),
        max_resolution_bits(HF_PWM_MAX_RESOLUTION), max_frequency_hz(HF_PWM_MAX_FREQUENCY),
        min_frequency_hz(HF_PWM_MIN_FREQUENCY), supports_fade(true),
        supports_sleep_retention(true), supports_complementary(true),
        supports_deadtime(true), supports_phase_shift(false),
        available_clock_sources(4) {}
};

/**
 * @brief PWM statistics for performance monitoring.
 * @details Thread-safe statistics collection for production diagnostics.
 */
struct hf_pwm_statistics_t {
  std::atomic<uint32_t> duty_updates_count;     ///< Total duty cycle updates
  std::atomic<uint32_t> frequency_changes_count; ///< Total frequency changes
  std::atomic<uint32_t> fade_operations_count;   ///< Total fade operations
  std::atomic<uint32_t> error_count;            ///< Total error count
  std::atomic<uint32_t> channel_enables_count;  ///< Total channel enable operations
  std::atomic<uint32_t> channel_disables_count; ///< Total channel disable operations
  uint64_t last_activity_timestamp;             ///< Last activity timestamp
  uint64_t initialization_timestamp;            ///< Initialization timestamp
  
  hf_pwm_statistics_t() noexcept
      : duty_updates_count(0), frequency_changes_count(0), fade_operations_count(0),
        error_count(0), channel_enables_count(0), channel_disables_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

#else
// Non-ESP32 platforms - use generic PWM types
static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 12;
static constexpr uint32_t HF_PWM_MIN_FREQUENCY = 1;
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 1000000;
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000;
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 10;
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;

enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0,
  HF_PWM_CLK_SRC_APB = 1,
  HF_PWM_CLK_SRC_XTAL = 2,
  HF_PWM_CLK_SRC_RC_FAST = 3
};

enum class hf_pwm_resolution_t : uint8_t {
  HF_PWM_RES_8BIT = 8,
  HF_PWM_RES_10BIT = 10,
  HF_PWM_RES_12BIT = 12,
  HF_PWM_RES_14BIT = 14
};

enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_LOW_SPEED = 0,
  HF_PWM_MODE_HIGH_SPEED = 1
};

enum class hf_pwm_fade_mode_t : uint8_t {
  HF_PWM_FADE_NO_WAIT = 0,
  HF_PWM_FADE_WAIT_DONE = 1
};

enum class hf_pwm_intr_type_t : uint8_t {
  HF_PWM_INTR_DISABLE = 0,
  HF_PWM_INTR_FADE_END = 1
};

// Generic handle types for non-ESP32 platforms
using hf_pwm_channel_native_t = uint8_t;
using hf_pwm_timer_native_t = uint8_t;
using hf_pwm_mode_native_t = uint8_t;
using hf_pwm_timer_bit_native_t = uint8_t;
using hf_pwm_clk_cfg_native_t = uint8_t;
using hf_pwm_channel_config_native_t = struct { int dummy; };
using hf_pwm_timer_config_native_t = struct { int dummy; };
using hf_pwm_fade_mode_native_t = uint8_t;
using hf_pwm_intr_type_native_t = uint8_t;

struct hf_pwm_timing_config_t {
  uint32_t frequency_hz;
  uint8_t resolution_bits;
  hf_pwm_clock_source_t clk_src;
  uint32_t clk_divider;
  uint32_t actual_frequency_hz;
  float frequency_accuracy;
  uint32_t period_ticks;
  uint32_t max_duty_ticks;
  
  hf_pwm_timing_config_t() noexcept
      : frequency_hz(HF_PWM_DEFAULT_FREQUENCY), resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        clk_src(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT), clk_divider(0),
        actual_frequency_hz(0), frequency_accuracy(0.0f), period_ticks(0), max_duty_ticks(0) {}
};

struct hf_pwm_channel_config_t {
  uint32_t gpio_pin;
  uint8_t channel_id;
  uint8_t timer_id;
  hf_pwm_mode_t speed_mode;
  uint32_t duty_initial;
  hf_pwm_intr_type_t intr_type;
  bool invert_output;
  uint32_t hpoint;
  uint8_t idle_level;
  bool output_invert;
  
  hf_pwm_channel_config_t() noexcept
      : gpio_pin(0xFFFFFFFF), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_LOW_SPEED), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false),
        hpoint(0), idle_level(0), output_invert(false) {}
};

struct hf_pwm_fade_config_t {
  uint32_t target_duty;
  uint32_t fade_time_ms;
  hf_pwm_fade_mode_t fade_mode;
  uint32_t scale;
  uint32_t cycle_num;
  
  hf_pwm_fade_config_t() noexcept
      : target_duty(0), fade_time_ms(1000),
        fade_mode(hf_pwm_fade_mode_t::HF_PWM_FADE_NO_WAIT),
        scale(0), cycle_num(0) {}
};

struct hf_pwm_capabilities_t {
  uint8_t num_channels;
  uint8_t num_timers;
  uint8_t max_resolution_bits;
  uint32_t max_frequency_hz;
  uint32_t min_frequency_hz;
  bool supports_fade;
  bool supports_sleep_retention;
  bool supports_complementary;
  bool supports_deadtime;
  bool supports_phase_shift;
  uint8_t available_clock_sources;
  
  hf_pwm_capabilities_t() noexcept
      : num_channels(HF_PWM_MAX_CHANNELS), num_timers(HF_PWM_MAX_TIMERS),
        max_resolution_bits(HF_PWM_MAX_RESOLUTION), max_frequency_hz(HF_PWM_MAX_FREQUENCY),
        min_frequency_hz(HF_PWM_MIN_FREQUENCY), supports_fade(false),
        supports_sleep_retention(false), supports_complementary(false),
        supports_deadtime(false), supports_phase_shift(false),
        available_clock_sources(1) {}
};

struct hf_pwm_statistics_t {
  std::atomic<uint32_t> duty_updates_count;
  std::atomic<uint32_t> frequency_changes_count;
  std::atomic<uint32_t> fade_operations_count;
  std::atomic<uint32_t> error_count;
  std::atomic<uint32_t> channel_enables_count;
  std::atomic<uint32_t> channel_disables_count;
  uint64_t last_activity_timestamp;
  uint64_t initialization_timestamp;
  
  hf_pwm_statistics_t() noexcept
      : duty_updates_count(0), frequency_changes_count(0), fade_operations_count(0),
        error_count(0), channel_enables_count(0), channel_disables_count(0),
        last_activity_timestamp(0), initialization_timestamp(0) {}
};

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// PWM UTILITY MACROS AND CONSTANTS
//==============================================================================

/**
 * @brief PWM validation macros.
 */
#define HF_PWM_IS_VALID_CHANNEL(ch) ((ch) < HF_PWM_MAX_CHANNELS)
#define HF_PWM_IS_VALID_TIMER(timer) ((timer) < HF_PWM_MAX_TIMERS)
#define HF_PWM_IS_VALID_FREQUENCY(freq) \
  ((freq) >= HF_PWM_MIN_FREQUENCY && (freq) <= HF_PWM_MAX_FREQUENCY)
#define HF_PWM_IS_VALID_RESOLUTION(res) \
  ((res) >= 8 && (res) <= HF_PWM_MAX_RESOLUTION)
#define HF_PWM_IS_VALID_DUTY_CYCLE(duty, res) \
  ((duty) <= ((1U << (res)) - 1))

/**
 * @brief PWM calculation macros.
 */
#define HF_PWM_DUTY_TO_RAW(duty_percent, resolution) \
  ((uint32_t)((duty_percent) * ((1U << (resolution)) - 1) / 100.0f))
#define HF_PWM_RAW_TO_DUTY(raw_duty, resolution) \
  ((float)(raw_duty) * 100.0f / ((1U << (resolution)) - 1))
#define HF_PWM_MAX_DUTY_VALUE(resolution) ((1U << (resolution)) - 1)
