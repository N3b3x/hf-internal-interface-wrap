/**
 * @file EspTypes_PWM.h
 * @ingroup esp32_types
 * @brief ESP32 PWM type definitions for LEDC peripheral hardware abstraction.
 *
 * This header defines essential PWM-specific types and constants for the EspPwm
 * implementation, providing a clean abstraction over ESP-IDF LEDC peripheral
 * capabilities across different ESP32 variants.
 *
 * ## LEDC Peripheral Overview:
 * The LED Controller (LEDC) peripheral is designed to control the intensity of LEDs,
 * but is also perfectly suited for general-purpose PWM generation. Key features:
 *
 * - **High Resolution:** Up to 20-bit resolution on ESP32 classic, 14-bit on newer variants
 * - **Multiple Clock Sources:** APB, XTAL, RC_FAST with different frequency ranges
 * - **Hardware Fade:** Smooth transitions without CPU intervention
 * - **Timer Sharing:** Multiple channels can share timers for efficiency
 * - **Low Power:** Optimized for battery-powered applications
 *
 * ## ESP32 Variant Differences:
 * Different ESP32 variants have different LEDC capabilities:
 * - **Channels:** 16 (ESP32), 8 (S2/S3), 6 (C3/C6), 4 (H2)
 * - **Timers:** 8 (ESP32), 4 (S2/S3/C3/C6), 2 (H2)
 * - **Clock Sources:** Variant-specific availability and constraints
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "../../../base/BasePwm.h"       // For hf_pwm_err_t
#include "../../../base/HardwareTypes.h" // For basic hardware types
#include "../../../utils/McuSelect.h"    // Central MCU platform selection (includes all ESP-IDF)
#include "EspTypes_Base.h"
#include "EspTypes_GPIO.h" // For hf_gpio_num_t

//==============================================================================
// ESP32 PWM TYPE MAPPINGS
//==============================================================================

// Direct ESP-IDF type usage - no unnecessary aliases
// These types are used internally by EspPwm implementation

//==============================================================================
// ESP32 PWM CONSTANTS
//==============================================================================

// Number of LEDC channels depends on the MCU variant. For ESP32-C6, there are 6.
// Use centralized selection from McuSelect.h when available.
#ifdef HF_MCU_PWM_MAX_CHANNELS
static constexpr uint8_t HF_PWM_MAX_CHANNELS = HF_MCU_PWM_MAX_CHANNELS;
#else
static constexpr uint8_t HF_PWM_MAX_CHANNELS = 8;
#endif
static constexpr uint8_t HF_PWM_MAX_TIMERS = 4;
static constexpr uint8_t HF_PWM_MAX_RESOLUTION = 14;
static constexpr uint32_t HF_PWM_MIN_FREQUENCY =
    100; // ESP32-C6 LEDC practical minimum with 10-bit resolution
static constexpr uint32_t HF_PWM_MAX_FREQUENCY = 20000000; // ESP32-C6 LEDC practical maximum
static constexpr uint32_t HF_PWM_DEFAULT_FREQUENCY = 1000;
static constexpr uint8_t HF_PWM_DEFAULT_RESOLUTION = 10;
static constexpr uint32_t HF_PWM_APB_CLOCK_HZ = 80000000;

//==============================================================================
// ESP32 PWM ENUMS
//==============================================================================

/**
 * @brief ESP32 PWM clock source selection with frequency and constraint details.
 *
 * @details Clock source selection is critical for PWM performance and determines
 * the maximum achievable frequency for a given resolution. The formula is:
 * **Max Frequency = Clock Source Frequency / (2^resolution_bits)**
 *
 * ## Clock Source Specifications:
 *
 * ### APB_CLK (80MHz) - Recommended for most applications
 * - **Frequency:** 80MHz (stable, derived from main crystal)
 * - **Stability:** High (crystal-locked)
 * - **Max PWM Freq:** ~78kHz @ 10-bit, ~19.5kHz @ 12-bit, ~4.9kHz @ 14-bit
 * - **Use Cases:** Motor control, servo control, LED dimming, audio PWM
 *
 * ### XTAL_CLK (40MHz) - Power-efficient option
 * - **Frequency:** 40MHz (main crystal oscillator)
 * - **Stability:** High (primary crystal)
 * - **Max PWM Freq:** ~39kHz @ 10-bit, ~9.8kHz @ 12-bit, ~2.4kHz @ 14-bit
 * - **Use Cases:** Low-frequency PWM, power-sensitive applications
 *
 * ### RC_FAST_CLK (~17.5MHz) - Lowest power consumption
 * - **Frequency:** ~17.5MHz (internal RC oscillator)
 * - **Stability:** Moderate (temperature dependent)
 * - **Max PWM Freq:** ~17kHz @ 10-bit, ~4.3kHz @ 12-bit, ~1.1kHz @ 14-bit
 * - **Use Cases:** Low-power applications, simple LED control
 *
 * @warning **ESP32 Variant Constraints:**
 * - **ESP32 Classic:** Each timer can use different clock sources independently
 * - **ESP32-S2/S3/C3/C6/H2:** All timers typically share the same clock source
 * - Always verify your target variant's clock source flexibility before design
 */
enum class hf_pwm_clock_source_t : uint8_t {
  HF_PWM_CLK_SRC_DEFAULT = 0, ///< Default clock source (typically APB_CLK)
  HF_PWM_CLK_SRC_APB = 1,     ///< APB clock (80MHz) - recommended for most uses
  HF_PWM_CLK_SRC_XTAL = 2,    ///< Crystal oscillator (40MHz) - power efficient
  HF_PWM_CLK_SRC_RC_FAST = 3  ///< RC fast clock (~17.5MHz) - lowest power
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
 * @brief Timer eviction policy for resource management.
 * @details Controls how the PWM system handles timer resource conflicts.
 */
enum class hf_pwm_eviction_policy_t : uint8_t {
  STRICT_NO_EVICTION = 0,          ///< Never evict existing channels (default, safest)
  ALLOW_EVICTION_WITH_CONSENT = 1, ///< Require callback approval before eviction
  ALLOW_EVICTION_NON_CRITICAL = 2, ///< Only evict channels marked as non-critical
  FORCE_EVICTION = 3               ///< Aggressive eviction (advanced users only)
};

/**
 * @brief Channel priority levels for eviction decisions.
 */
enum class hf_pwm_channel_priority_t : uint8_t {
  PRIORITY_LOW = 0,     ///< Low priority - can be evicted first
  PRIORITY_NORMAL = 1,  ///< Normal priority - default
  PRIORITY_HIGH = 2,    ///< High priority - protect from eviction
  PRIORITY_CRITICAL = 3 ///< Critical priority - never evict
};

/**
 * @brief Eviction request information passed to user callback.
 */
struct hf_pwm_eviction_request_t {
  hf_channel_id_t affected_channel;   ///< Channel that would be affected
  hf_u8_t current_timer;              ///< Timer that would be reconfigured
  hf_u32_t current_frequency;         ///< Current timer frequency
  hf_u8_t current_resolution;         ///< Current timer resolution
  hf_u32_t requested_frequency;       ///< Requested new frequency
  hf_u8_t requested_resolution;       ///< Requested new resolution
  hf_channel_id_t requesting_channel; ///< Channel requesting the change

  hf_pwm_eviction_request_t() noexcept
      : affected_channel(0), current_timer(0), current_frequency(0), current_resolution(0),
        requested_frequency(0), requested_resolution(0), requesting_channel(0) {}
};

/**
 * @brief Eviction decision from user callback.
 */
enum class hf_pwm_eviction_decision_t : uint8_t {
  DENY_EVICTION = 0,      ///< Deny the eviction request
  ALLOW_EVICTION = 1,     ///< Allow the eviction to proceed
  SUGGEST_ALTERNATIVE = 2 ///< Suggest alternative (not implemented yet)
};

// Forward declarations for callback types
/**
 * @brief Callback function for eviction consent.
 * @param request Information about the eviction request
 * @param user_data User-provided data
 * @return Decision on whether to allow eviction
 */
using hf_pwm_eviction_callback_t =
    hf_pwm_eviction_decision_t (*)(const hf_pwm_eviction_request_t& request, void* user_data);

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
 * @brief ESP32 PWM channel configuration with comprehensive LEDC feature support.
 *
 * @details This structure provides complete control over LEDC channel configuration,
 * including advanced features like hardware fade, phase shifting, and resource
 * protection. All parameters are validated against hardware constraints.
 *
 * ## Core Configuration:
 * - **GPIO Pin:** Any valid GPIO pin (check ESP32 variant pin matrix)
 * - **Channel/Timer:** Automatic assignment or manual control
 * - **Frequency/Resolution:** Explicit control with validation
 * - **Clock Source:** Per-channel preference (subject to variant constraints)
 *
 * ## Advanced Features:
 * - **Phase Shift (hpoint):** Delay PWM start within period (0 to max_duty)
 * - **Output Inversion:** Hardware-level signal inversion
 * - **Idle Level:** Output state when PWM is disabled
 * - **Priority System:** Protection against resource eviction
 *
 * ## Usage Examples:
 * ```cpp
 * // Basic LED dimming
 * hf_pwm_channel_config_t led_config = {};
 * led_config.gpio_pin = 2;
 * led_config.frequency_hz = 1000;      // 1kHz
 * led_config.resolution_bits = 10;     // 10-bit (0-1023)
 * led_config.duty_initial = 512;       // 50% brightness
 *
 * // Motor control with high resolution
 * hf_pwm_channel_config_t motor_config = {};
 * motor_config.gpio_pin = 4;
 * motor_config.frequency_hz = 20000;   // 20kHz (above audible range)
 * motor_config.resolution_bits = 12;   // 12-bit (0-4095) for smooth control
 * motor_config.is_critical = true;     // Protect from eviction
 * motor_config.description = "Motor PWM";
 *
 * // Servo control with precise timing
 * hf_pwm_channel_config_t servo_config = {};
 * servo_config.gpio_pin = 18;
 * servo_config.frequency_hz = 50;      // 50Hz (20ms period)
 * servo_config.resolution_bits = 14;   // 14-bit for microsecond precision
 * servo_config.clock_source = HF_PWM_CLK_SRC_APB; // Stable timing
 * ```
 */
struct hf_pwm_channel_config_t {
  hf_gpio_num_t gpio_pin;   ///< GPIO pin for PWM output (check pin matrix)
  uint8_t channel_id;       ///< Channel ID (0 to variant max)
  uint8_t timer_id;         ///< Timer ID (0 to variant max)
  hf_pwm_mode_t speed_mode; ///< Speed mode configuration

  // Explicit frequency and resolution control
  uint32_t frequency_hz;              ///< PWM frequency in Hz (validated against clock source)
  uint8_t resolution_bits;            ///< PWM resolution in bits (4-14, validated)
  hf_pwm_clock_source_t clock_source; ///< Preferred clock source for this channel

  uint32_t duty_initial;        ///< Initial duty cycle value (RAW for specified resolution)
  hf_pwm_intr_type_t intr_type; ///< Interrupt type configuration
  bool invert_output;           ///< Invert output signal polarity

  // Advanced LEDC features
  uint32_t hpoint;    ///< High point timing for phase shift (0 to max_duty)
  uint8_t idle_level; ///< Idle state level when disabled (0 or 1)
  bool output_invert; ///< Hardware output inversion (different from invert_output)

  // Resource protection and management
  hf_pwm_channel_priority_t priority; ///< Channel priority for eviction decisions
  bool is_critical;                   ///< Mark as critical (never evict)
  const char* description;            ///< Optional description for debugging/logging

  hf_pwm_channel_config_t() noexcept
      : gpio_pin(static_cast<hf_gpio_num_t>(HF_INVALID_PIN)), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_BASIC), frequency_hz(HF_PWM_DEFAULT_FREQUENCY),
        resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        clock_source(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB), duty_initial(0),
        intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false), hpoint(0),
        idle_level(0), output_invert(false), priority(hf_pwm_channel_priority_t::PRIORITY_NORMAL),
        is_critical(false), description(nullptr) {}
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
