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
 *
 * @example PwmComprehensiveTest.cpp
 * This example demonstrates comprehensive PWM testing including signal generation,
 * frequency control, and hardware-specific capabilities for ESP32-C6.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

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
  X(PWM_ERR_INVALID_DEVICE_ID, 23, "Invalid device ID")                                            \
  X(PWM_ERR_UNSUPPORTED_OPERATION, 24, "Unsupported operation")                                    \
  X(PWM_ERR_UNKNOWN, 25, "Unknown error")

// Generate enum class
enum class hf_pwm_err_t : hf_u32_t {
#define X(name, value, description) name = value,
  HF_PWM_ERR_LIST(X)
#undef X
      PWM_ERR_COUNT // Total number of error codes
};

/**
 * @brief Convert PWM error code to string view
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfPwmErrToString(hf_pwm_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_pwm_err_t::NAME:                                                                         \
    return DESC;
    HF_PWM_ERR_LIST(X)
#undef X
  default:
    return HfPwmErrToString(hf_pwm_err_t::PWM_ERR_UNKNOWN);
  }
}

//--------------------------------------
//  PWM Configuration Structures
//--------------------------------------

/**
 * @brief PWM statistics information
 */
struct hf_pwm_statistics_t {
  hf_u32_t duty_updates_count;       ///< Total duty cycle updates
  hf_u32_t frequency_changes_count;  ///< Total frequency changes
  hf_u32_t fade_operations_count;    ///< Total fade operations
  hf_u32_t error_count;              ///< Total error count
  hf_u32_t channel_enables_count;    ///< Total channel enable operations
  hf_u32_t channel_disables_count;   ///< Total channel disable operations
  hf_u64_t last_activity_timestamp;  ///< Last activity timestamp
  hf_u64_t initialization_timestamp; ///< Initialization timestamp

  hf_pwm_statistics_t() noexcept
      : duty_updates_count(0), frequency_changes_count(0), fade_operations_count(0), error_count(0),
        channel_enables_count(0), channel_disables_count(0), last_activity_timestamp(0),
        initialization_timestamp(0) {}
};

/**
 * @brief PWM diagnostics information
 */
struct hf_pwm_diagnostics_t {
  bool hardware_initialized;      ///< Hardware is initialized
  bool fade_functionality_ready;  ///< Hardware fade is ready
  hf_u8_t active_channels;        ///< Number of active channels
  hf_u8_t active_timers;          ///< Number of active timers
  hf_u32_t system_uptime_ms;      ///< System uptime in milliseconds
  hf_pwm_err_t last_global_error; ///< Last global error

  hf_pwm_diagnostics_t() noexcept
      : hardware_initialized(false), fade_functionality_ready(false), active_channels(0),
        active_timers(0), system_uptime_ms(0), last_global_error(hf_pwm_err_t::PWM_SUCCESS) {}
};

//--------------------------------------
//  Callback Types
//--------------------------------------

/**
 * @brief Callback for PWM period complete events
 * @param channel_id Channel that completed a period
 * @param user_data User-provided data
 */
using hf_pwm_period_callback_t = std::function<void(hf_channel_id_t channel_id, void* user_data)>;

/**
 * @brief Callback for PWM fault/error events
 * @param channel_id Channel that encountered fault
 * @param error Error that occurred
 * @param user_data User-provided data
 */
using hf_pwm_fault_callback_t =
    std::function<void(hf_channel_id_t channel_id, hf_pwm_err_t error, void* user_data)>;

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
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Ensure PWM is initialized (lazy initialization pattern)
   * @return true if initialized successfully, false on failure
   * @note This method should be called at the beginning of all public methods
   *       that require initialization. It implements lazy initialization.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = (Initialize() == hf_pwm_err_t::PWM_SUCCESS);
    }
    return initialized_;
  }

  /**
   * @brief Ensure PWM is deinitialized (lazy deinitialization pattern)
   * @return true if deinitialized successfully, false on failure
   * @note This method can be called to ensure proper cleanup when needed.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !(Deinitialize() == hf_pwm_err_t::PWM_SUCCESS);
      return !initialized_;
    }
    return true;
  }
  //==============================================================================
  // CHANNEL MANAGEMENT
  //==============================================================================

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
  virtual hf_pwm_err_t SetDutyCycleRaw(hf_channel_id_t channel_id, hf_u32_t raw_value) noexcept = 0;

  /**
   * @brief Set frequency for a channel
   * @param channel_id Channel identifier
   * @param frequency_hz Frequency in Hz
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetFrequency(hf_channel_id_t channel_id,
                                    hf_frequency_hz_t frequency_hz) noexcept = 0;

  /**
   * @brief Set phase shift for a channel (if supported)
   * @param channel_id Channel identifier
   * @param phase_shift_degrees Phase shift in degrees (0-360)
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t SetPhaseShift(hf_channel_id_t channel_id,
                                     float phase_shift_degrees) noexcept = 0;

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
                                              hf_u32_t deadtime_ns) noexcept = 0;

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
   * @brief Get PWM statistics
   * @param statistics Statistics structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t GetStatistics(hf_pwm_statistics_t& statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get PWM diagnostics
   * @param diagnostics Diagnostics structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  virtual hf_pwm_err_t GetDiagnostics(hf_pwm_diagnostics_t& diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset PWM operation statistics.
   * @return hf_pwm_err_t::PWM_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_pwm_err_t ResetStatistics() noexcept {
    statistics_ = hf_pwm_statistics_t{}; // Reset statistics to default values
    return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset PWM diagnostic information.
   * @return hf_pwm_err_t::PWM_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_pwm_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_pwm_diagnostics_t{}; // Reset diagnostics to default values
    return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
  }

  //==============================================================================
  // UTILITY FUNCTIONS
  //==============================================================================

  /**
   * @brief Calculate raw duty value from percentage
   * @param duty_cycle Duty cycle (0.0 - 1.0)
   * @param resolution_bits PWM resolution in bits
   * @return Raw duty value
   */
  static constexpr hf_u32_t DutyCycleToRaw(float duty_cycle, hf_u8_t resolution_bits) noexcept {
    if (duty_cycle < 0.0f)
      duty_cycle = 0.0f;
    if (duty_cycle > 1.0f)
      duty_cycle = 1.0f;
    return static_cast<hf_u32_t>(duty_cycle * ((1U << resolution_bits) - 1));
  }

  /**
   * @brief Calculate duty cycle percentage from raw value
   * @param raw_value Raw duty value
   * @param resolution_bits PWM resolution in bits
   * @return Duty cycle (0.0 - 1.0)
   */
  static constexpr float RawToDutyCycle(hf_u32_t raw_value, hf_u8_t resolution_bits) noexcept {
    hf_u32_t max_value = (1U << resolution_bits) - 1;
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
  static constexpr bool IsValidFrequency(hf_u32_t frequency_hz, hf_u32_t min_freq_hz,
                                         hf_u32_t max_freq_hz) noexcept {
    return (frequency_hz >= min_freq_hz && frequency_hz <= max_freq_hz);
  }

  /**
   * @brief Validate raw duty value against resolution
   * @param raw_value Raw duty value to validate
   * @param resolution_bits PWM resolution in bits
   * @return true if valid, false otherwise
   */
  static constexpr bool IsValidRawDuty(hf_u32_t raw_value, hf_u8_t resolution_bits) noexcept {
    if (resolution_bits == 0 || resolution_bits > 16)
      return false;
    hf_u32_t max_value = (1U << resolution_bits) - 1;
    return (raw_value <= max_value);
  }

  /**
   * @brief Calculate frequency accuracy percentage
   * @param target_freq Target frequency
   * @param actual_freq Actual achieved frequency
   * @return Accuracy percentage (0.0-1.0)
   */
  static constexpr float CalculateFrequencyAccuracy(hf_u32_t target_freq,
                                                    hf_u32_t actual_freq) noexcept {
    if (target_freq == 0)
      return 0.0f;
    float diff = static_cast<float>(target_freq > actual_freq ? target_freq - actual_freq
                                                              : actual_freq - target_freq);
    return 1.0f - (diff / static_cast<float>(target_freq));
  }

  /**
   * @brief Clamp duty cycle to valid range
   * @param duty_cycle Duty cycle to clamp
   * @return Clamped duty cycle (0.0 - 1.0)
   */
  static constexpr float ClampDutyCycle(float duty_cycle) noexcept {
    if (duty_cycle < 0.0f)
      return 0.0f;
    if (duty_cycle > 1.0f)
      return 1.0f;
    return duty_cycle;
  }

protected:
  BasePwm() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}
  BasePwm(const BasePwm&) = delete;
  BasePwm& operator=(const BasePwm&) = delete;
  BasePwm(BasePwm&&) = delete;
  BasePwm& operator=(BasePwm&&) = delete;

  bool initialized_;                 ///< Initialization state
  hf_pwm_statistics_t statistics_;   ///< PWM operation statistics
  hf_pwm_diagnostics_t diagnostics_; ///< PWM diagnostic information
};
