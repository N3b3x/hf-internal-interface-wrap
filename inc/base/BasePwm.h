/**
 * @file BasePwm.h
 * @brief Abstract base class for PWM implementations in the HardFOC system.
 * 
 * This header-only file defines the abstract base class for PWM generation
 * that provides a consistent API across different PWM implementations.
 * Concrete implementations for various platforms inherit from this class.
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan/BaseAdc.
 * @note Users should program against this interface, not specific implementations.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>

//--------------------------------------
//  HardFOC PWM Error Codes (Table)
//--------------------------------------

/**
 * @brief HardFOC PWM error codes
 * @details Comprehensive error enumeration for all PWM operations in the system.
 *          This enumeration is used across all PWM-related classes to provide
 *          consistent error reporting and handling.
 */
#define HF_PWM_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(PWM_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(PWM_ERR_FAILURE, 1, "General failure")                                                         \
  X(PWM_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(PWM_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(PWM_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(PWM_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(PWM_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Channel errors */                                                                             \
  X(PWM_ERR_INVALID_CHANNEL, 7, "Invalid PWM channel")                                             \
  X(PWM_ERR_CHANNEL_BUSY, 8, "Channel already in use")                                             \
  X(PWM_ERR_CHANNEL_NOT_AVAILABLE, 9, "Channel not available")                                     \
  X(PWM_ERR_INSUFFICIENT_CHANNELS, 10, "Insufficient channels available")                          \
  /* Frequency/timing errors */                                                                    \
  X(PWM_ERR_INVALID_FREQUENCY, 11, "Invalid frequency")                                            \
  X(PWM_ERR_FREQUENCY_TOO_HIGH, 12, "Frequency too high")                                          \
  X(PWM_ERR_FREQUENCY_TOO_LOW, 13, "Frequency too low")                                            \
  X(PWM_ERR_RESOLUTION_NOT_SUPPORTED, 14, "Resolution not supported")                              \
  /* Duty cycle errors */                                                                          \
  X(PWM_ERR_INVALID_DUTY_CYCLE, 15, "Invalid duty cycle")                                          \
  X(PWM_ERR_DUTY_OUT_OF_RANGE, 16, "Duty cycle out of range")                                      \
  /* Hardware errors */                                                                            \
  X(PWM_ERR_HARDWARE_FAULT, 17, "Hardware fault")                                                  \
  X(PWM_ERR_TIMER_CONFLICT, 18, "Timer resource conflict")                                         \
  X(PWM_ERR_PIN_CONFLICT, 19, "Pin already in use")                                                \
  /* Communication errors (for external PWM ICs) */                                                \
  X(PWM_ERR_COMMUNICATION_TIMEOUT, 20, "Communication timeout")                                    \
  X(PWM_ERR_COMMUNICATION_FAILURE, 21, "Communication failure")                                    \
  X(PWM_ERR_DEVICE_NOT_RESPONDING, 22, "Device not responding")                                    \
  X(PWM_ERR_INVALID_DEVICE_ID, 23, "Invalid device ID")

// Generate enum class
enum class hf_pwm_err_t : uint32_t {
#define X(name, value, description) name = value,
  HF_PWM_ERR_LIST(X)
#undef X
      PWM_ERR_COUNT // Total number of error codes
};

// Generate error message function
constexpr const char *hf_pwm_err_tToString(hf_pwm_err_t error) noexcept {
  switch (error) {
#define X(name, value, description)                                                                \
  case hf_pwm_err_t::name:                                                                             \
    return description;
    HF_PWM_ERR_LIST(X)
#undef X
  default:
    return "Unknown PWM error";
  }
}

//--------------------------------------
//  PWM Configuration Structures
//--------------------------------------

/**
 * @brief PWM output mode configuration
 */
enum class hf_pwm_output_mode_t : uint8_t {
  Normal = 0,        ///< Normal PWM output
  Inverted = 1,      ///< Inverted PWM output
  Complementary = 2, ///< Complementary output (for motor control)
  Differential = 3   ///< Differential output
};

/**
 * @brief PWM alignment mode
 */
enum class hf_pwm_alignment_t : uint8_t {
  EdgeAligned = 0,  ///< Edge-aligned PWM (standard)
  CenterAligned = 1 ///< Center-aligned PWM (better for motor control)
};

/**
 * @brief PWM idle state
 */
enum class hf_pwm_idle_state_t : uint8_t {
  Low = 0, ///< Output low when idle
  High = 1 ///< Output high when idle
};

/**
 * @brief PWM channel configuration structure
 */
struct hf_pwm_channel_config_t {
  hf_pin_num_t output_pin;        ///< GPIO pin for PWM output
  uint32_t frequency_hz;         ///< PWM frequency in Hz
  uint8_t resolution_bits;       ///< PWM resolution (8-16 bits typical)
  hf_pwm_output_mode_t output_mode; ///< Output mode configuration
  hf_pwm_alignment_t alignment;     ///< PWM alignment mode
  hf_pwm_idle_state_t idle_state;   ///< Idle state configuration
  float initial_duty_cycle;      ///< Initial duty cycle (0.0 - 1.0)
  bool invert_output;            ///< Invert the output signal

  // Default constructor with sensible defaults
  hf_pwm_channel_config_t() noexcept
      : output_pin(-1), frequency_hz(1000), resolution_bits(12), output_mode(hf_pwm_output_mode_t::Normal),
        alignment(hf_pwm_alignment_t::EdgeAligned), idle_state(hf_pwm_idle_state_t::Low),
        initial_duty_cycle(0.0f), invert_output(false) {}
};

/**
 * @brief PWM timer configuration (for MCU implementations)
 */
struct hf_pwm_timer_config_t {
  uint8_t timer_number;          ///< Timer/group number
  uint32_t base_frequency_hz;    ///< Base timer frequency
  uint8_t resolution_bits;       ///< Timer resolution
  hf_pwm_alignment_t alignment;  ///< Timer alignment mode

  hf_pwm_timer_config_t() noexcept
      : timer_number(0), base_frequency_hz(80000000),
        resolution_bits(12), alignment(hf_pwm_alignment_t::EdgeAligned) {}
};

//--------------------------------------
//  PWM Status and Information
//--------------------------------------

/**
 * @brief PWM channel status information
 */
struct hf_pwm_channel_status_t {
  bool is_enabled;               ///< Channel is enabled
  bool is_running;               ///< Channel is actively generating PWM
  uint32_t current_frequency_hz; ///< Current frequency
  float current_duty_cycle;      ///< Current duty cycle (0.0 - 1.0)
  uint32_t raw_duty_value;       ///< Raw duty register value
  hf_pwm_err_t last_error;           ///< Last error encountered

  hf_pwm_channel_status_t() noexcept
      : is_enabled(false), is_running(false), current_frequency_hz(0), current_duty_cycle(0.0f),
        raw_duty_value(0), last_error(hf_pwm_err_t::PWM_SUCCESS) {}
};

/**
 * @brief PWM capability information (what the implementation supports)
 */
struct hf_pwm_capabilities_t {
  uint8_t max_channels;         ///< Maximum number of channels
  uint8_t max_timers;           ///< Maximum number of timers
  uint32_t min_frequency_hz;    ///< Minimum supported frequency
  uint32_t max_frequency_hz;    ///< Maximum supported frequency
  uint8_t min_resolution_bits;  ///< Minimum resolution
  uint8_t max_resolution_bits;  ///< Maximum resolution
  bool supports_complementary;  ///< Supports complementary outputs
  bool supports_center_aligned; ///< Supports center-aligned PWM
  bool supports_deadtime;       ///< Supports deadtime insertion
  bool supports_phase_shift;    ///< Supports phase shifting
};

//--------------------------------------
//  Callback Types
//--------------------------------------

/**
 * @brief Callback for PWM period complete events
 * @param channel_id Channel that completed a period
 * @param user_data User-provided data
 */
using hf_pwm_period_callback_t = std::function<void(hf_channel_id_t channel_id, void *user_data)>;

/**
 * @brief Callback for PWM fault/error events
 * @param channel_id Channel that encountered fault
 * @param error Error that occurred
 * @param user_data User-provided data
 */
using hf_pwm_fault_callback_t =
    std::function<void(hf_channel_id_t channel_id, hf_pwm_err_t error, void *user_data)>;

//--------------------------------------
//  Abstract Base Class
//--------------------------------------

/**
 * @class BasePwm
 * @brief Abstract base class for PWM implementations.
 *
 * This class defines the common interface that all PWM implementations must provide.
 * It supports both on-chip PWM peripherals and external PWM controllers.
 *
 * Key features:
 * - Multi-channel PWM support
 * - Configurable frequency and resolution
 * - Hardware abstraction for different PWM sources
 * - Event callbacks for period and fault events
 * - Comprehensive error handling
 * - Thread-safe design when used with SfPwm wrapper
 *
 * Possible implementations include on-chip controllers or dedicated PWM chips.
 * - CustomPwm: For custom/proprietary PWM implementations
 */
class BasePwm {
public:
  //==============================================================================
  // LIFECYCLE
  //==============================================================================

  virtual ~BasePwm() noexcept = default;

  /**
   * @brief Initialize the PWM system
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the PWM system
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Check if PWM system is initialized
   * @return true if initialized, false otherwise
   */
  virtual bool IsInitialized() const noexcept = 0;

  //==============================================================================
  // CHANNEL MANAGEMENT
  //==============================================================================

  /**
   * @brief Configure a PWM channel
   * @param channel_id Channel identifier (0-based)
   * @param config Channel configuration
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t ConfigureChannel(hf_channel_id_t channel_id,
                                    const hf_pwm_channel_config_t &config) noexcept = 0;

  /**
   * @brief Enable a PWM channel
   * @param channel_id Channel to enable
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t EnableChannel(hf_channel_id_t channel_id) noexcept = 0;

  /**
   * @brief Disable a PWM channel
   * @param channel_id Channel to disable
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t DisableChannel(hf_channel_id_t channel_id) noexcept = 0;

  /**
   * @brief Check if a channel is enabled
   * @param channel_id Channel to check
   * @return true if enabled, false otherwise
   */
  virtual bool IsChannelEnabled(hf_channel_id_t channel_id) const noexcept = 0;

  //==============================================================================
  // PWM CONTROL
  //==============================================================================

  /**
   * @brief Set duty cycle for a channel
   * @param channel_id Channel identifier
   * @param duty_cycle Duty cycle (0.0 - 1.0)
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept = 0;

  /**
   * @brief Set raw duty value for a channel
   * @param channel_id Channel identifier
   * @param raw_value Raw duty register value
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetDutyCycleRaw(hf_channel_id_t channel_id, uint32_t raw_value) noexcept = 0;

  /**
   * @brief Set frequency for a channel
   * @param channel_id Channel identifier
   * @param frequency_hz Frequency in Hz
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetFrequency(hf_channel_id_t channel_id, hf_frequency_hz_t frequency_hz) noexcept = 0;

  /**
   * @brief Set phase shift for a channel (if supported)
   * @param channel_id Channel identifier
   * @param phase_shift_degrees Phase shift in degrees (0-360)
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetPhaseShift(hf_channel_id_t channel_id, float phase_shift_degrees) noexcept = 0;

  //==============================================================================
  // ADVANCED FEATURES
  //==============================================================================

  /**
   * @brief Start all enabled channels simultaneously
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t StartAll() noexcept = 0;

  /**
   * @brief Stop all channels
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t StopAll() noexcept = 0;

  /**
   * @brief Update all channel outputs simultaneously (for synchronized updates)
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t UpdateAll() noexcept = 0;

  /**
   * @brief Set complementary output configuration (for motor control)
   * @param primary_channel Primary channel
   * @param complementary_channel Complementary channel
   * @param deadtime_ns Deadtime in nanoseconds
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetComplementaryOutput(hf_channel_id_t primary_channel,
                                          hf_channel_id_t complementary_channel,
                                          uint32_t deadtime_ns) noexcept = 0;

  //==============================================================================
  // STATUS AND INFORMATION
  //==============================================================================

  /**
   * @brief Get current duty cycle for a channel
   * @param channel_id Channel identifier
   * @return Current duty cycle (0.0 - 1.0), or -1.0 on error
   */
  virtual float GetDutyCycle(hf_channel_id_t channel_id) const noexcept = 0;

  /**
   * @brief Get current frequency for a channel
   * @param channel_id Channel identifier
   * @return Current frequency in Hz, or 0 on error
   */
  virtual hf_frequency_hz_t GetFrequency(hf_channel_id_t channel_id) const noexcept = 0;

  /**
   * @brief Get channel status
   * @param channel_id Channel identifier
   * @param status Status structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t GetChannelStatus(hf_channel_id_t channel_id,
                                    hf_pwm_channel_status_t &status) const noexcept = 0;

  /**
   * @brief Get PWM implementation capabilities
   * @param capabilities Capabilities structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t GetCapabilities(hf_pwm_capabilities_t &capabilities) const noexcept = 0;

  /**
   * @brief Get last error for a specific channel
   * @param channel_id Channel identifier
   * @return Last error code for the channel
   */
  virtual hf_pwm_err_t GetLastError(hf_channel_id_t channel_id) const noexcept = 0;

  //==============================================================================
  // CALLBACKS
  //==============================================================================

  /**
   * @brief Set period complete callback
   * @param callback Callback function
   * @param user_data User data passed to callback
   */
  virtual void SetPeriodCallback(hf_pwm_period_callback_t callback,
                                 void *user_data = nullptr) noexcept = 0;

  /**
   * @brief Set fault/error callback
   * @param callback Callback function
   * @param user_data User data passed to callback
   */
  virtual void SetFaultCallback(hf_pwm_fault_callback_t callback, void *user_data = nullptr) noexcept = 0;

  //==============================================================================
  // UTILITY FUNCTIONS
  //==============================================================================

  /**
   * @brief Calculate raw duty value from percentage
   * @param duty_cycle Duty cycle (0.0 - 1.0)
   * @param resolution_bits PWM resolution in bits
   * @return Raw duty value
   */
  static constexpr uint32_t DutyCycleToRaw(float duty_cycle, uint8_t resolution_bits) noexcept {
    if (duty_cycle < 0.0f)
      duty_cycle = 0.0f;
    if (duty_cycle > 1.0f)
      duty_cycle = 1.0f;
    return static_cast<uint32_t>(duty_cycle * ((1U << resolution_bits) - 1));
  }

  /**
   * @brief Calculate duty cycle percentage from raw value
   * @param raw_value Raw duty value
   * @param resolution_bits PWM resolution in bits
   * @return Duty cycle (0.0 - 1.0)
   */
  static constexpr float RawToDutyCycle(uint32_t raw_value, uint8_t resolution_bits) noexcept {
    uint32_t max_value = (1U << resolution_bits) - 1;
    if (raw_value > max_value)
      raw_value = max_value;
    return static_cast<float>(raw_value) / static_cast<float>(max_value);
  }

  /**
   * @brief Validate duty cycle range
   * @param duty_cycle Duty cycle to validate
   * @return true if valid (0.0 - 1.0), false otherwise
   */
  static constexpr bool IsValidDutyCycle(float duty_cycle) noexcept {
    return (duty_cycle >= 0.0f && duty_cycle <= 1.0f);
  }

  /**
   * @brief Validate frequency range
   * @param frequency_hz Frequency to validate
   * @param min_freq_hz Minimum allowed frequency
   * @param max_freq_hz Maximum allowed frequency
   * @return true if valid, false otherwise
   */
  static constexpr bool IsValidFrequency(uint32_t frequency_hz, uint32_t min_freq_hz,
                                         uint32_t max_freq_hz) noexcept {
    return (frequency_hz >= min_freq_hz && frequency_hz <= max_freq_hz);
  }

protected:
  BasePwm() noexcept = default;
  BasePwm(const BasePwm &) = delete;
  BasePwm &operator=(const BasePwm &) = delete;
  BasePwm(BasePwm &&) = delete;
  BasePwm &operator=(BasePwm &&) = delete;
};
