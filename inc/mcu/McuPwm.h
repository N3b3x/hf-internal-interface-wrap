/**
 * @file McuPwm.h
 * @brief MCU-integrated PWM controller implementation for ESP32C6.
 *
 * This header provides a PWM implementation for microcontrollers with
 * built-in PWM peripherals. On ESP32C6, this wraps the LEDC (LED Controller)
 * peripheral which provides high-resolution PWM generation.
 *
 * Features:
 * - Up to 8 PWM channels using LEDC peripheral
 * - Configurable frequency and resolution per channel
 * - Support for complementary outputs with deadtime
 * - Hardware fade support for smooth transitions
 * - Interrupt-driven period callbacks
 * - Multiple timer groups for independent frequency control
 */

#ifndef MCU_PWM_H
#define MCU_PWM_H

#include "BasePwm.h"
#include "McuTypes.h"
#include <array>
#include <mutex>

/**
 * @class McuPwm
 * @brief PWM implementation for microcontrollers with integrated PWM peripherals.
 *
 * This class provides PWM generation using the microcontroller's built-in
 * PWM peripheral. On ESP32C6, it uses the LEDC (LED Controller) peripheral
 * which offers high-resolution PWM with hardware fade support.
 *
 * ESP32C6 LEDC Features:
 * - 8 independent PWM channels
 * - 4 timer groups for different frequency domains
 * - Up to 14-bit resolution at high frequencies
 * - Hardware fade functionality
 * - Interrupt support for period complete events
 * - Low power mode support
 *
 * Key Design Features:
 * - Thread-safe channel management
 * - Automatic timer allocation and management
 * - Hardware fault detection and recovery
 * - Comprehensive error reporting
 * - Support for synchronized updates across channels
 * - Motor control oriented features (complementary outputs, deadtime)
 */
class McuPwm : public BasePwm {
public:
  //==============================================================================
  // CONSTANTS
  //==============================================================================

  static constexpr uint8_t MAX_CHANNELS = 8;          ///< Maximum PWM channels
  static constexpr uint8_t MAX_TIMERS = 4;            ///< Maximum timer groups
  static constexpr uint8_t MAX_RESOLUTION = 14;       ///< Maximum resolution bits
  static constexpr uint32_t MIN_FREQUENCY = 1;        ///< Minimum frequency (Hz)
  static constexpr uint32_t MAX_FREQUENCY = 40000000; ///< Maximum frequency (Hz)

  //==============================================================================
  // CONSTRUCTOR AND DESTRUCTOR
  //==============================================================================

  /**
   * @brief Constructor for MCU PWM controller
   * @param base_clock_hz Base clock frequency for timers (default: 80MHz)
   */
  explicit McuPwm(uint32_t base_clock_hz = 80000000) noexcept;

  /**
   * @brief Destructor - ensures clean shutdown
   */
  virtual ~McuPwm() noexcept override;

  // Prevent copying and moving
  McuPwm(const McuPwm &) = delete;
  McuPwm &operator=(const McuPwm &) = delete;
  McuPwm(McuPwm &&) = delete;
  McuPwm &operator=(McuPwm &&) = delete;

  //==============================================================================
  // LIFECYCLE (BasePwm Interface)
  //==============================================================================

  HfPwmErr Initialize() noexcept override;
  HfPwmErr Deinitialize() noexcept override;
  bool IsInitialized() const noexcept override;

  //==============================================================================
  // CHANNEL MANAGEMENT (BasePwm Interface)
  //==============================================================================

  HfPwmErr ConfigureChannel(HfChannelId channel_id,
                            const PwmChannelConfig &config) noexcept override;
  HfPwmErr EnableChannel(HfChannelId channel_id) noexcept override;
  HfPwmErr DisableChannel(HfChannelId channel_id) noexcept override;
  bool IsChannelEnabled(HfChannelId channel_id) const noexcept override;

  //==============================================================================
  // PWM CONTROL (BasePwm Interface)
  //==============================================================================

  HfPwmErr SetDutyCycle(HfChannelId channel_id, float duty_cycle) noexcept override;
  HfPwmErr SetDutyCycleRaw(HfChannelId channel_id, uint32_t raw_value) noexcept override;
  HfPwmErr SetFrequency(HfChannelId channel_id, HfFrequencyHz frequency_hz) noexcept override;
  HfPwmErr SetPhaseShift(HfChannelId channel_id, float phase_shift_degrees) noexcept override;

  //==============================================================================
  // ADVANCED FEATURES (BasePwm Interface)
  //==============================================================================

  HfPwmErr StartAll() noexcept override;
  HfPwmErr StopAll() noexcept override;
  HfPwmErr UpdateAll() noexcept override;
  HfPwmErr SetComplementaryOutput(HfChannelId primary_channel, HfChannelId complementary_channel,
                                  uint32_t deadtime_ns) noexcept override;

  //==============================================================================
  // STATUS AND INFORMATION (BasePwm Interface)
  //==============================================================================

  float GetDutyCycle(HfChannelId channel_id) const noexcept override;
  HfFrequencyHz GetFrequency(HfChannelId channel_id) const noexcept override;
  HfPwmErr GetChannelStatus(HfChannelId channel_id,
                            PwmChannelStatus &status) const noexcept override;
  HfPwmErr GetCapabilities(PwmCapabilities &capabilities) const noexcept override;
  HfPwmErr GetLastError(HfChannelId channel_id) const noexcept override;

  //==============================================================================
  // CALLBACKS (BasePwm Interface)
  //==============================================================================

  void SetPeriodCallback(PwmPeriodCallback callback, void *user_data = nullptr) noexcept override;
  void SetFaultCallback(PwmFaultCallback callback, void *user_data = nullptr) noexcept override;

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
  HfPwmErr SetHardwareFade(HfChannelId channel_id, float target_duty_cycle,
                           uint32_t fade_time_ms) noexcept;

  /**
   * @brief Stop hardware fade for a channel
   * @param channel_id Channel identifier
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr StopHardwareFade(HfChannelId channel_id) noexcept;

  /**
   * @brief Check if hardware fade is active on a channel
   * @param channel_id Channel identifier
   * @return true if fade is active, false otherwise
   */
  bool IsFadeActive(HfChannelId channel_id) const noexcept;

  /**
   * @brief Set idle output level for a channel
   * @param channel_id Channel identifier
   * @param idle_level Idle level (0 or 1)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetIdleLevel(HfChannelId channel_id, uint8_t idle_level) noexcept;

  /**
   * @brief Get current timer assignment for a channel
   * @param channel_id Channel identifier
   * @return Timer number (0-3), or -1 if channel not configured
   */
  int8_t GetTimerAssignment(HfChannelId channel_id) const noexcept;

  /**
   * @brief Force a specific timer for a channel (advanced usage)
   * @param channel_id Channel identifier
   * @param timer_id Timer identifier (0-3)
   * @return PWM_SUCCESS on success, error code on failure
   * @note Use with caution - automatic timer allocation is usually better
   */
  HfPwmErr ForceTimerAssignment(HfChannelId channel_id, uint8_t timer_id) noexcept;

private:
  //==============================================================================
  // INTERNAL STRUCTURES
  //==============================================================================

  /**
   * @brief Internal channel state
   */
  struct ChannelState {
    bool configured;         ///< Channel is configured
    bool enabled;            ///< Channel is enabled
    PwmChannelConfig config; ///< Channel configuration
    uint8_t assigned_timer;  ///< Assigned timer (0-3)
    uint32_t raw_duty_value; ///< Current raw duty value
    HfPwmErr last_error;     ///< Last error for this channel
    bool fade_active;        ///< Hardware fade is active

    ChannelState() noexcept
        : configured(false), enabled(false), assigned_timer(0xFF), raw_duty_value(0),
          last_error(HfPwmErr::PWM_SUCCESS), fade_active(false) {}
  };

  /**
   * @brief Internal timer state
   */
  struct TimerState {
    bool in_use;             ///< Timer is in use
    uint32_t frequency_hz;   ///< Timer frequency
    uint8_t resolution_bits; ///< Timer resolution
    uint8_t channel_count;   ///< Number of channels using this timer

    TimerState() noexcept : in_use(false), frequency_hz(0), resolution_bits(0), channel_count(0) {}
  };

  /**
   * @brief Complementary output pair configuration
   */
  struct ComplementaryPair {
    uint8_t primary_channel;       ///< Primary channel
    uint8_t complementary_channel; ///< Complementary channel
    uint32_t deadtime_ns;          ///< Deadtime in nanoseconds
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
  bool IsValidChannelId(HfChannelId channel_id) const noexcept;

  /**
   * @brief Find or allocate a timer for the given frequency and resolution
   * @param frequency_hz Required frequency
   * @param resolution_bits Required resolution
   * @return Timer ID (0-3), or -1 if no timer available
   */
  int8_t FindOrAllocateTimer(uint32_t frequency_hz, uint8_t resolution_bits) noexcept;

  /**
   * @brief Release a timer if no longer needed
   * @param timer_id Timer to potentially release
   */
  void ReleaseTimerIfUnused(uint8_t timer_id) noexcept;

  /**
   * @brief Configure platform timer
   * @param timer_id Timer to configure
   * @param frequency_hz Timer frequency
   * @param resolution_bits Timer resolution
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr ConfigurePlatformTimer(uint8_t timer_id, uint32_t frequency_hz,
                                  uint8_t resolution_bits) noexcept;

  /**
   * @brief Configure platform channel
   * @param channel_id Channel to configure
   * @param config Channel configuration
   * @param timer_id Assigned timer
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr ConfigurePlatformChannel(HfChannelId channel_id, const PwmChannelConfig &config,
                                    uint8_t timer_id) noexcept;

  /**
   * @brief Update platform duty cycle
   * @param channel_id Channel to update
   * @param raw_duty_value Raw duty value
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr UpdatePlatformDuty(HfChannelId channel_id, uint32_t raw_duty_value) noexcept;

  /**
   * @brief Set error for a channel
   * @param channel_id Channel identifier
   * @param error Error to set
   */
  void SetChannelError(HfChannelId channel_id, HfPwmErr error) noexcept;

  /**
   * @brief Platform-specific interrupt handler
   * @param channel_id Channel that generated interrupt
   */
  static void IRAM_ATTR InterruptHandler(HfChannelId channel_id, void *user_data) noexcept;

  /**
   * @brief Handle fade complete interrupt
   * @param channel_id Channel that completed fade
   */
  void HandleFadeComplete(HfChannelId channel_id) noexcept;

  //==============================================================================
  // MEMBER VARIABLES
  //==============================================================================

  mutable std::mutex mutex_; ///< Thread safety mutex
  bool initialized_;         ///< Initialization state
  uint32_t base_clock_hz_;   ///< Base clock frequency

  std::array<ChannelState, MAX_CHANNELS> channels_;                     ///< Channel states
  std::array<TimerState, MAX_TIMERS> timers_;                           ///< Timer states
  std::array<ComplementaryPair, MAX_CHANNELS / 2> complementary_pairs_; ///< Complementary pairs

  PwmPeriodCallback period_callback_; ///< Period complete callback
  void *period_callback_user_data_;   ///< Period callback user data
  PwmFaultCallback fault_callback_;   ///< Fault callback
  void *fault_callback_user_data_;    ///< Fault callback user data

  HfPwmErr last_global_error_; ///< Last global error
};

#endif // MCU_PWM_H
