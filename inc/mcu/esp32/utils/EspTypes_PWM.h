/**
 * @file EspTypes_PWM.h
 * @brief ESP32 PWM type definitions for hardware abstraction.
 *
 * This header defines only the essential PWM-specific types used by
 * the EspPwm implementation. Clean and minimal approach.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BasePwm.h" // For hf_pwm_err_t
#include "EspTypes_Base.h"
#include "EspTypes_GPIO.h" // For hf_gpio_num_t
#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"     // Central MCU platform selection (includes all ESP-IDF)

//==============================================================================
// ESP32 PWM TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspPwm implementation

//==============================================================================
// ESP32 PWM CONSTANTS
//==============================================================================

static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 14;
static constexpr uint32_t HF_PWM_MIN_FREQUENCY = 1;
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 40000000;
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000;
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 10;
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;

//==============================================================================
// ESP32 PWM ENUMS
//==============================================================================

/**
 * @brief ESP32 PWM clock source selection.
 */
enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0, ///< Default clock source
  HF_PWM_CLK_SRC_APB = 1,     ///< APB clock (80MHz)
  HF_PWM_CLK_SRC_XTAL = 2,    ///< Crystal oscillator (40MHz)
  HF_PWM_CLK_SRC_RC_FAST = 3  ///< RC fast clock
};

/**
 * @brief ESP32 PWM resolution options.
 */
enum class hf_pwm_resolution_t : uint8_t {
  HF_PWM_RES_8BIT = 8,   ///< 8-bit resolution
  HF_PWM_RES_10BIT = 10, ///< 10-bit resolution
  HF_PWM_RES_12BIT = 12, ///< 12-bit resolution
  HF_PWM_RES_14BIT = 14  ///< 14-bit resolution
};

/**
 * @brief ESP32 PWM mode configuration.
 */
enum class hf_pwm_mode_t : uint8_t {
  HF_PWM_MODE_BASIC = 0, ///< Basic PWM mode
  HF_PWM_MODE_FADE = 1   ///< Fade mode with hardware fade support
};

/**
 * @brief ESP32 PWM fade mode configuration.
 */
enum class hf_pwm_fade_mode_t : uint8_t {
  HF_PWM_FADE_NO_WAIT = 0,  ///< Non-blocking fade
  HF_PWM_FADE_WAIT_DONE = 1 ///< Blocking fade
};

/**
 * @brief ESP32 PWM interrupt type configuration.
 */
enum class hf_pwm_intr_type_t : uint8_t {
  HF_PWM_INTR_DISABLE = 0, ///< Disable interrupts
  HF_PWM_INTR_FADE_END = 1 ///< Fade end interrupt
};

/**
 * @brief ESP32 PWM unit configuration.
 */
struct hf_pwm_unit_config_t {
  uint8_t unit_id;                    ///< PWM unit ID
  hf_pwm_mode_t mode;                 ///< Operating mode
  uint32_t base_clock_hz;             ///< Base clock frequency
  hf_pwm_clock_source_t clock_source; ///< Clock source
  bool enable_fade;                   ///< Enable fade functionality
  bool enable_interrupts;             ///< Enable interrupts

  hf_pwm_unit_config_t() noexcept
      : unit_id(0), mode(hf_pwm_mode_t::HF_PWM_MODE_BASIC), base_clock_hz(HF_PWM_APB_CLOCK_HZ),
        clock_source(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT), enable_fade(true),
        enable_interrupts(false) {}
};

/**
 * @brief PWM channel status information.
 */
struct hf_pwm_channel_status_t {
  bool enabled;                        ///< Channel is enabled
  bool configured;                     ///< Channel is configured
  float current_duty_cycle;            ///< Current duty cycle (0.0-1.0)
  hf_frequency_hz_t current_frequency; ///< Current frequency
  uint8_t resolution_bits;             ///< Current resolution
  uint32_t raw_duty_value;             ///< Raw duty cycle value
  bool fade_active;                    ///< Hardware fade is active
  hf_pwm_err_t last_error;             ///< Last error for this channel

  hf_pwm_channel_status_t() noexcept
      : enabled(false), configured(false), current_duty_cycle(0.0f), current_frequency(0),
        resolution_bits(0), raw_duty_value(0), fade_active(false),
        last_error(hf_pwm_err_t::PWM_SUCCESS) {}
};

//==============================================================================
// ESP32 PWM CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief ESP32 PWM timing configuration with optimization support.
 * @details Platform-specific timing parameters optimized for ESP32 80MHz APB clock.
 */
struct hf_pwm_timing_config_t {
  uint32_t frequency_hz;         ///< PWM frequency in Hz
  uint8_t resolution_bits;       ///< PWM resolution (8-14 bits)
  hf_pwm_clock_source_t clk_src; ///< Clock source selection
  uint32_t clk_divider;          ///< Clock divider (calculated automatically)

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
 * @brief ESP32 PWM channel configuration with advanced features.
 * @details Comprehensive configuration for ESP-IDF LEDC channel features.
 */
struct hf_pwm_channel_config_t {
  hf_gpio_num_t gpio_pin;       ///< GPIO pin for PWM output
  uint8_t channel_id;           ///< Channel ID (0-7)
  uint8_t timer_id;             ///< Timer ID (0-3)
  hf_pwm_mode_t speed_mode;     ///< Speed mode configuration
  uint32_t duty_initial;        ///< Initial duty cycle value
  hf_pwm_intr_type_t intr_type; ///< Interrupt type
  bool invert_output;           ///< Invert output signal

  // Advanced configuration
  uint32_t hpoint;    ///< High point timing (phase shift)
  uint8_t idle_level; ///< Idle state level (0 or 1)
  bool output_invert; ///< Hardware output inversion

  hf_pwm_channel_config_t() noexcept
      : gpio_pin(static_cast<hf_gpio_num_t>(HF_INVALID_PIN)), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_BASIC), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false), hpoint(0),
        idle_level(0), output_invert(false) {}
};

/**
 * @brief ESP32 PWM fade configuration for smooth transitions.
 */
struct hf_pwm_fade_config_t {
  uint32_t target_duty;         ///< Target duty cycle value
  uint32_t fade_time_ms;        ///< Fade duration in milliseconds
  hf_pwm_fade_mode_t fade_mode; ///< Fade mode (blocking/non-blocking)
  uint32_t scale;               ///< Fade scale factor
  uint32_t cycle_num;           ///< Number of fade cycles

  hf_pwm_fade_config_t() noexcept
      : target_duty(0), fade_time_ms(1000), fade_mode(hf_pwm_fade_mode_t::HF_PWM_FADE_NO_WAIT),
        scale(0), cycle_num(0) {}
};

/**
 * @brief PWM capabilities and limitations for ESP32.
 * @details Static capability information for runtime feature detection.
 */
struct hf_pwm_capabilities_t {
  uint8_t num_channels;            ///< Number of PWM channels (8 for ESP32)
  uint8_t num_timers;              ///< Number of timer groups (4 for ESP32)
  uint8_t max_resolution_bits;     ///< Maximum resolution bits (14 for ESP32)
  uint32_t max_frequency_hz;       ///< Maximum supported frequency
  uint32_t min_frequency_hz;       ///< Minimum supported frequency
  bool supports_fade;              ///< Hardware fade support
  bool supports_sleep_retention;   ///< Sleep retention support
  bool supports_complementary;     ///< Complementary outputs (software)
  bool supports_deadtime;          ///< Deadtime insertion (software)
  bool supports_phase_shift;       ///< Phase shifting support
  uint8_t available_clock_sources; ///< Number of available clock sources

  // Constructor with ESP32 defaults
  hf_pwm_capabilities_t() noexcept
      : num_channels(HF_PWM_MAX_CHANNELS), num_timers(HF_PWM_MAX_TIMERS),
        max_resolution_bits(HF_PWM_MAX_RESOLUTION), max_frequency_hz(HF_PWM_MAX_FREQUENCY),
        min_frequency_hz(HF_PWM_MIN_FREQUENCY), supports_fade(true), supports_sleep_retention(true),
        supports_complementary(true), supports_deadtime(true), supports_phase_shift(false),
        available_clock_sources(4) {}
};

//==============================================================================
// END OF ESPPWM TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================
