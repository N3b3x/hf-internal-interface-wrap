/**
 * @file EspPwm.h
 * @brief ESP32C6 LEDC (PWM) controller implementation for the HardFOC system.
 *
 * This header provides a comprehensive PWM implementation for ESP32C6 using the
 * LEDC (LED Controller) peripheral which provides high-resolution PWM generation.
 * The implementation supports multiple channels, configurable frequency and resolution,
 * complementary outputs with deadtime, hardware fade support, and interrupt-driven
 * period callbacks.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Features include up to 8 PWM channels using LEDC peripheral, configurable
 *       frequency and resolution per channel, support for complementary outputs with
 *       deadtime, hardware fade support, interrupt-driven period callbacks, and
 *       multiple timer groups for independent frequency control.
 * @note This implementation follows the lazy initialization pattern established in
 *       other ESP32 modules (EspAdc, EspGpio, etc.).
 */

#pragma once

#include "BasePwm.h"
#include "RtosMutex.h"
#include "utils/EspTypes_PWM.h"
#include <array>
#include <atomic>

/**
 * @class EspPwm
 * @brief ESP32C6 PWM implementation using LEDC peripheral.
 *
 * This class provides PWM generation using the ESP32C6's built-in LEDC (LED Controller)
 * peripheral which offers high-resolution PWM with hardware fade support.
 *
 * ESP32C6 LEDC Features:
 * - 8 independent PWM channels (0-7)
 * - 4 timer groups for different frequency domains (0-3)
 * - Up to 14-bit resolution at high frequencies
 * - Hardware fade functionality for smooth transitions
 * - Interrupt support for period complete events
 * - Low power mode support with sleep retention
 * - Configurable clock sources (APB, XTAL, RC_FAST)
 * - Channel-specific idle state configuration
 *
 * Key Design Features:
 * - Lazy initialization pattern (no hardware action until needed)
 * - Thread-safe channel management with RtosMutex
 * - Automatic timer allocation and management
 * - Hardware fault detection and recovery
 * - Comprehensive error reporting
 * - Support for synchronized updates across channels
 * - Motor control oriented features (complementary outputs, deadtime)
 * - Proper abstraction of ESP-IDF types
 */
class EspPwm : public BasePwm {
public:
  //==============================================================================
  // CONSTANTS
  //==============================================================================

  static constexpr hf_u8_t MAX_CHANNELS = HF_PWM_MAX_CHANNELS;     ///< Maximum PWM channels
  static constexpr hf_u8_t MAX_TIMERS = HF_PWM_MAX_TIMERS;         ///< Maximum timer groups
  static constexpr hf_u8_t MAX_RESOLUTION = HF_PWM_MAX_RESOLUTION; ///< Maximum resolution bits
  static constexpr hf_u32_t MIN_FREQUENCY = HF_PWM_MIN_FREQUENCY;  ///< Minimum frequency (Hz)
  static constexpr hf_u32_t MAX_FREQUENCY = HF_PWM_MAX_FREQUENCY;  ///< Maximum frequency (Hz)

  //==============================================================================
  // CONSTRUCTOR AND DESTRUCTOR
  //==============================================================================

  /**
   * @brief Constructor for ESP32C6 PWM controller
   * @param config PWM unit configuration
   * @note Uses lazy initialization - no hardware action until first operation
   */
  explicit EspPwm(const hf_pwm_unit_config_t& config = hf_pwm_unit_config_t{}) noexcept;
  EspPwm(hf_u32_t base_clock_hz) noexcept;

  /**
   * @brief Destructor - ensures clean shutdown
   */
  virtual ~EspPwm() noexcept override;

  // Prevent copying and moving
  EspPwm(const EspPwm&) = delete;
  EspPwm& operator=(const EspPwm&) = delete;
  EspPwm(EspPwm&&) = delete;
  EspPwm& operator=(EspPwm&&) = delete;

  //==============================================================================
  // LIFECYCLE (BasePwm Interface)
  //==============================================================================

  hf_pwm_err_t Initialize() noexcept override;
  hf_pwm_err_t Deinitialize() noexcept override;

  /**
   * @brief Set PWM operating mode
   * @param mode Operating mode (Basic or Fade)
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t SetMode(hf_pwm_mode_t mode) noexcept;

  /**
   * @brief Get current PWM operating mode
   * @return Current operating mode
   */
  hf_pwm_mode_t GetMode() const noexcept;

  //==============================================================================
  // CHANNEL MANAGEMENT (BasePwm Interface)
  //==============================================================================

  hf_pwm_err_t ConfigureChannel(hf_channel_id_t channel_id,
                                const hf_pwm_channel_config_t& config) noexcept;
  hf_pwm_err_t EnableChannel(hf_channel_id_t channel_id) noexcept override;
  hf_pwm_err_t DisableChannel(hf_channel_id_t channel_id) noexcept override;
  bool IsChannelEnabled(hf_channel_id_t channel_id) const noexcept override;

  //==============================================================================
  // PWM CONTROL (BasePwm Interface)
  //==============================================================================

  hf_pwm_err_t SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept override;
  hf_pwm_err_t SetDutyCycleRaw(hf_channel_id_t channel_id, hf_u32_t raw_value) noexcept override;
  hf_pwm_err_t SetFrequency(hf_channel_id_t channel_id,
                            hf_frequency_hz_t frequency_hz) noexcept override;
  hf_pwm_err_t SetPhaseShift(hf_channel_id_t channel_id,
                             float phase_shift_degrees) noexcept override;

  //==============================================================================
  // ADVANCED FEATURES (BasePwm Interface)
  //==============================================================================

  hf_pwm_err_t StartAll() noexcept override;
  hf_pwm_err_t StopAll() noexcept override;
  hf_pwm_err_t UpdateAll() noexcept override;
  hf_pwm_err_t SetComplementaryOutput(hf_channel_id_t primary_channel,
                                      hf_channel_id_t complementary_channel,
                                      hf_u32_t deadtime_ns) noexcept override;

  //==============================================================================
  // STATUS AND INFORMATION (BasePwm Interface)
  //==============================================================================

  float GetDutyCycle(hf_channel_id_t channel_id) const noexcept override;
  hf_frequency_hz_t GetFrequency(hf_channel_id_t channel_id) const noexcept override;
  hf_pwm_err_t GetChannelStatus(hf_channel_id_t channel_id,
                                hf_pwm_channel_status_t& status) const noexcept;
  hf_pwm_err_t GetCapabilities(hf_pwm_capabilities_t& capabilities) const noexcept;
  hf_pwm_err_t GetLastError(hf_channel_id_t channel_id) const noexcept;

  //==============================================================================
  // CALLBACKS (BasePwm Interface)
  //==============================================================================

  void SetPeriodCallback(hf_pwm_period_callback_t callback, void* user_data = nullptr) noexcept;
  void SetFaultCallback(hf_pwm_fault_callback_t callback, void* user_data = nullptr) noexcept;

  //==============================================================================
  // ESP32C6-SPECIFIC FEATURES
  //==============================================================================

  /**
   * @brief Set hardware fade for smooth duty cycle transitions
   * @param channel_id Channel identifier
   * @param target_duty_cycle Target duty cycle (0.0 - 1.0)
   * @param fade_time_ms Fade duration in milliseconds
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t SetHardwareFade(hf_channel_id_t channel_id, float target_duty_cycle,
                               hf_u32_t fade_time_ms) noexcept;

  /**
   * @brief Stop hardware fade for a channel
   * @param channel_id Channel identifier
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t StopHardwareFade(hf_channel_id_t channel_id) noexcept;

  /**
   * @brief Check if hardware fade is active on a channel
   * @param channel_id Channel identifier
   * @return true if fade is active, false otherwise
   */
  bool IsFadeActive(hf_channel_id_t channel_id) const noexcept;

  /**
   * @brief Set idle output level for a channel
   * @param channel_id Channel identifier
   * @param idle_level Idle level (0 or 1)
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t SetIdleLevel(hf_channel_id_t channel_id, hf_u8_t idle_level) noexcept;

  /**
   * @brief Get current timer assignment for a channel
   * @param channel_id Channel identifier
   * @return Timer number (0-3), or -1 if channel not configured
   */
  int8_t GetTimerAssignment(hf_channel_id_t channel_id) const noexcept;

  /**
   * @brief Force a specific timer for a channel (advanced usage)
   * @param channel_id Channel identifier
   * @param timer_id Timer identifier (0-3)
   * @return PWM_SUCCESS on success, error code on failure
   * @note Use with caution - automatic timer allocation is usually better
   */
  hf_pwm_err_t ForceTimerAssignment(hf_channel_id_t channel_id, hf_u8_t timer_id) noexcept;

  /**
   * @brief Set clock source for PWM timers
   * @param clock_source Clock source selection
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t SetClockSource(hf_pwm_clock_source_t clock_source) noexcept;

  /**
   * @brief Get current clock source
   * @return Current clock source
   */
  hf_pwm_clock_source_t GetClockSource() const noexcept;

  /**
   * @brief Get PWM statistics
   * @param statistics Statistics structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t GetStatistics(hf_pwm_statistics_t& statistics) const noexcept override;

  /**
   * @brief Get PWM diagnostics
   * @param diagnostics Diagnostics structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t GetDiagnostics(hf_pwm_diagnostics_t& diagnostics) const noexcept override;

private:
  //==============================================================================
  // INTERNAL STRUCTURES
  //==============================================================================

  /**
   * @brief Internal channel state
   */
  struct ChannelState {
    bool configured;                ///< Channel is configured
    bool enabled;                   ///< Channel is enabled
    hf_pwm_channel_config_t config; ///< Channel configuration
    hf_u8_t assigned_timer;         ///< Assigned timer (0-3)
    hf_u32_t raw_duty_value;        ///< Current raw duty value
    hf_pwm_err_t last_error;        ///< Last error for this channel
    bool fade_active;               ///< Hardware fade is active

    ChannelState() noexcept
        : configured(false), enabled(false), assigned_timer(0xFF), raw_duty_value(0),
          last_error(hf_pwm_err_t::PWM_SUCCESS), fade_active(false) {}
  };

  /**
   * @brief Internal timer state
   */
  struct TimerState {
    bool in_use;             ///< Timer is in use
    hf_u32_t frequency_hz;   ///< Timer frequency
    hf_u8_t resolution_bits; ///< Timer resolution
    hf_u8_t channel_count;   ///< Number of channels using this timer

    TimerState() noexcept : in_use(false), frequency_hz(0), resolution_bits(0), channel_count(0) {}
  };

  /**
   * @brief Complementary output pair configuration
   */
  struct ComplementaryPair {
    hf_u8_t primary_channel;       ///< Primary channel
    hf_u8_t complementary_channel; ///< Complementary channel
    hf_u32_t deadtime_ns;          ///< Deadtime in nanoseconds
    bool active;                   ///< Pair is active

    ComplementaryPair() noexcept
        : primary_channel(0xFF), complementary_channel(0xFF), deadtime_ns(0), active(false) {}
  };

  //==============================================================================
  // INTERNAL METHODS
  //==============================================================================

  /**
   * @brief Validate channel ID
   * @param channel_id Channel to validate
   * @return true if valid, false otherwise
   */
  bool IsValidChannelId(hf_channel_id_t channel_id) const noexcept;

  /**
   * @brief Find or allocate a timer for the given frequency/resolution
   * @param frequency_hz Required frequency
   * @param resolution_bits Required resolution
   * @return Timer ID (0-3), or -1 if no timer available
   */
  int8_t FindOrAllocateTimer(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept;

  /**
   * @brief Release a timer if no longer needed
   * @param timer_id Timer to potentially release
   */
  void ReleaseTimerIfUnused(hf_u8_t timer_id) noexcept;

  /**
   * @brief Configure platform timer
   * @param timer_id Timer to configure
   * @param frequency_hz Timer frequency
   * @param resolution_bits Timer resolution
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t ConfigurePlatformTimer(hf_u8_t timer_id, hf_u32_t frequency_hz,
                                      hf_u8_t resolution_bits) noexcept;

  /**
   * @brief Configure platform channel
   * @param channel_id Channel to configure
   * @param config Channel configuration
   * @param timer_id Assigned timer
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t ConfigurePlatformChannel(hf_channel_id_t channel_id,
                                        const hf_pwm_channel_config_t& config,
                                        hf_u8_t timer_id) noexcept;

  /**
   * @brief Update platform duty cycle
   * @param channel_id Channel to update
   * @param raw_duty_value Raw duty value
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t UpdatePlatformDuty(hf_channel_id_t channel_id, hf_u32_t raw_duty_value) noexcept;

  /**
   * @brief Set error for a channel
   * @param channel_id Channel identifier
   * @param error Error to set
   */
  void SetChannelError(hf_channel_id_t channel_id, hf_pwm_err_t error) noexcept;

  /**
   * @brief Platform-specific interrupt handler
   * @param channel_id Channel that generated interrupt
   * @param user_data User data passed to interrupt handler (EspPwm instance)
   */
  static void IRAM_ATTR InterruptHandler(hf_channel_id_t channel_id, void* user_data) noexcept;

  /**
   * @brief Handle fade complete interrupt
   * @param channel_id Channel that completed fade
   */
  void HandleFadeComplete(hf_channel_id_t channel_id) noexcept;

  /**
   * @brief Initialize LEDC fade functionality
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t InitializeFadeFunctionality() noexcept;

  /**
   * @brief Initialize PWM timers
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t InitializeTimers() noexcept;

  /**
   * @brief Initialize PWM channels
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t InitializeChannels() noexcept;

  /**
   * @brief Enable fade functionality
   * @return PWM_SUCCESS on success, error code on failure
   */
  hf_pwm_err_t EnableFade() noexcept;

  /**
   * @brief Calculate optimal clock divider for frequency
   * @param frequency_hz Target frequency
   * @param resolution_bits PWM resolution
   * @return Clock divider value
   */
  hf_u32_t CalculateClockDivider(hf_u32_t frequency_hz, hf_u8_t resolution_bits) const noexcept;

  //==============================================================================
  // MEMBER VARIABLES
  //==============================================================================

  mutable RtosMutex mutex_;            ///< Thread safety mutex
  std::atomic<bool> initialized_;      ///< Initialization state (atomic for lazy init)
  hf_u32_t base_clock_hz_;             ///< Base clock frequency
  hf_pwm_clock_source_t clock_source_; ///< Current clock source

  std::array<ChannelState, MAX_CHANNELS> channels_;                     ///< Channel states
  std::array<TimerState, MAX_TIMERS> timers_;                           ///< Timer states
  std::array<ComplementaryPair, MAX_CHANNELS / 2> complementary_pairs_; ///< Complementary pairs

  hf_pwm_period_callback_t period_callback_; ///< Period complete callback
  void* period_callback_user_data_;          ///< Period callback user data
  hf_pwm_fault_callback_t fault_callback_;   ///< Fault callback
  void* fault_callback_user_data_;           ///< Fault callback user data

  hf_pwm_err_t last_global_error_;    ///< Last global error
  bool fade_functionality_installed_; ///< LEDC fade functionality installed

  // New member variables for enhanced functionality
  hf_pwm_unit_config_t unit_config_; ///< Unit configuration
  hf_pwm_mode_t current_mode_;       ///< Current operating mode
  hf_pwm_statistics_t statistics_;   ///< PWM statistics
  hf_pwm_diagnostics_t diagnostics_; ///< PWM diagnostics
};
