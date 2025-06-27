/**
 * @file SfPwm.h
 * @brief Thread-safe PWM interface wrapper.
 *
 * This file provides a thread-safe wrapper around the BasePwm interface for use
 * in multi-threaded applications. All operations are synchronized using mutexes
 * to ensure thread safety when multiple threads access the same PWM interface.
 *
 * This is the recommended interface for component handlers and application threads
 * that need PWM generation capabilities.
 */
#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_SFPWM_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_SFPWM_H_

#include "../base/BasePwm.h"
#include "../mcu/McuPwm.h"
#include "../mcu/McuTypes.h"
#include "../utils/RtosMutex.h"
#include <cstdint>
#include <memory>

/**
 * @class SfPwm
 * @brief Thread-safe PWM interface.
 *
 * This class provides a thread-safe wrapper around the BasePwm interface.
 * All methods are protected by mutexes to ensure safe concurrent access
 * from multiple threads.
 *
 * Key Features:
 * - Complete thread-safe wrapper for all BasePwm operations
 * - Automatic resource management with RAII
 * - Configurable timeout for mutex operations
 * - Thread-safe callback management
 * - Comprehensive error handling with thread context
 * - Performance optimized with minimal locking overhead
 *
 * Usage example:
 * @code
 * // Create thread-safe PWM interface
 * SfPwm sf_pwm(std::make_unique<McuPwm>());
 *
 * if (sf_pwm.Initialize() == HfPwmErr::PWM_SUCCESS) {
 *     // Configure a PWM channel
 *     PwmChannelConfig config{};
 *     config.output_pin = GPIO_NUM_2;
 *     config.frequency_hz = 1000;
 *     config.resolution_bits = 12;
 *     config.initial_duty_cycle = 0.5f;
 *
 *     sf_pwm.ConfigureChannel(0, config);
 *     sf_pwm.EnableChannel(0);
 *
 *     // Change duty cycle from multiple threads safely
 *     sf_pwm.SetDutyCycle(0, 0.75f);
 * }
 * @endcode
 */
class SfPwm {
public:
  //==============================================================================
  // TYPES
  //==============================================================================

  /// Configuration for thread-safe PWM wrapper
  struct Config {
    uint32_t mutex_timeout_ms; ///< Mutex acquisition timeout
    bool enable_statistics;    ///< Enable performance statistics

    Config() noexcept
        : mutex_timeout_ms(5000) // 5 second default timeout
          ,
          enable_statistics(false) {}
  };

  /// Performance statistics (optional)
  struct Statistics {
    uint64_t total_operations;     ///< Total operations performed
    uint64_t mutex_timeouts;       ///< Number of mutex timeouts
    uint64_t average_lock_time_us; ///< Average lock acquisition time
    uint64_t max_lock_time_us;     ///< Maximum lock acquisition time

    Statistics() noexcept
        : total_operations(0), mutex_timeouts(0), average_lock_time_us(0), max_lock_time_us(0) {}
  };

  //==============================================================================
  // CONSTRUCTOR AND DESTRUCTOR
  //==============================================================================

  /**
   * @brief Constructor with PWM implementation
   * @param pwm_impl PWM implementation (McuPwm, Pca9685Pwm, etc.)
   * @param config Thread-safe wrapper configuration
   */
  explicit SfPwm(std::unique_ptr<BasePwm> pwm_impl, const Config &config = Config{}) noexcept;

  /**
   * @brief Destructor - ensures clean shutdown with proper synchronization
   */
  ~SfPwm() noexcept;

  // Prevent copying and moving
  SfPwm(const SfPwm &) = delete;
  SfPwm &operator=(const SfPwm &) = delete;
  SfPwm(SfPwm &&) = delete;
  SfPwm &operator=(SfPwm &&) = delete;

  //==============================================================================
  // LIFECYCLE
  //==============================================================================

  /**
   * @brief Initialize the PWM system (thread-safe)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr Initialize() noexcept;

  /**
   * @brief Deinitialize the PWM system (thread-safe)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr Deinitialize() noexcept;

  /**
   * @brief Check if PWM system is initialized (thread-safe)
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept;

  //==============================================================================
  // CHANNEL MANAGEMENT
  //==============================================================================

  /**
   * @brief Configure a PWM channel (thread-safe)
   * @param channel_id Channel identifier (0-based)
   * @param config Channel configuration
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr ConfigureChannel(uint8_t channel_id, const PwmChannelConfig &config,
                            uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Enable a PWM channel (thread-safe)
   * @param channel_id Channel to enable
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr EnableChannel(uint8_t channel_id, uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Disable a PWM channel (thread-safe)
   * @param channel_id Channel to disable
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr DisableChannel(uint8_t channel_id, uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Check if a channel is enabled (thread-safe)
   * @param channel_id Channel to check
   * @return true if enabled, false otherwise
   */
  bool IsChannelEnabled(uint8_t channel_id) const noexcept;

  //==============================================================================
  // PWM CONTROL
  //==============================================================================

  /**
   * @brief Set duty cycle for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @param duty_cycle Duty cycle (0.0 - 1.0)
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetDutyCycle(uint8_t channel_id, float duty_cycle, uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Set raw duty value for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @param raw_value Raw duty register value
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetDutyCycleRaw(uint8_t channel_id, uint32_t raw_value,
                           uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Set frequency for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @param frequency_hz Frequency in Hz
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetFrequency(uint8_t channel_id, uint32_t frequency_hz,
                        uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Set phase shift for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @param phase_shift_degrees Phase shift in degrees (0-360)
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetPhaseShift(uint8_t channel_id, float phase_shift_degrees,
                         uint32_t timeout_ms = 0) noexcept;

  //==============================================================================
  // BULK OPERATIONS (OPTIMIZED FOR THREAD SAFETY)
  //==============================================================================

  /**
   * @brief Set duty cycles for multiple channels atomically
   * @param channel_duties Array of {channel_id, duty_cycle} pairs
   * @param count Number of pairs in the array
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  struct ChannelDuty {
    uint8_t channel_id;
    float duty_cycle;
  };
  HfPwmErr SetMultipleDutyCycles(const ChannelDuty *channel_duties, size_t count,
                                 uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Enable multiple channels atomically
   * @param channel_ids Array of channel IDs to enable
   * @param count Number of channels in the array
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr EnableMultipleChannels(const uint8_t *channel_ids, size_t count,
                                  uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Disable multiple channels atomically
   * @param channel_ids Array of channel IDs to disable
   * @param count Number of channels in the array
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr DisableMultipleChannels(const uint8_t *channel_ids, size_t count,
                                   uint32_t timeout_ms = 0) noexcept;

  //==============================================================================
  // ADVANCED FEATURES
  //==============================================================================

  /**
   * @brief Start all enabled channels simultaneously (thread-safe)
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr StartAll(uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Stop all channels (thread-safe)
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr StopAll(uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Update all channel outputs simultaneously (thread-safe)
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr UpdateAll(uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Set complementary output configuration (thread-safe)
   * @param primary_channel Primary channel
   * @param complementary_channel Complementary channel
   * @param deadtime_ns Deadtime in nanoseconds
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetComplementaryOutput(uint8_t primary_channel, uint8_t complementary_channel,
                                  uint32_t deadtime_ns, uint32_t timeout_ms = 0) noexcept;

  //==============================================================================
  // STATUS AND INFORMATION
  //==============================================================================

  /**
   * @brief Get current duty cycle for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @return Current duty cycle (0.0 - 1.0), or -1.0 on error
   */
  float GetDutyCycle(uint8_t channel_id) const noexcept;

  /**
   * @brief Get current frequency for a channel (thread-safe)
   * @param channel_id Channel identifier
   * @return Current frequency in Hz, or 0 on error
   */
  uint32_t GetFrequency(uint8_t channel_id) const noexcept;

  /**
   * @brief Get channel status (thread-safe)
   * @param channel_id Channel identifier
   * @param status Status structure to fill
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr GetChannelStatus(uint8_t channel_id, PwmChannelStatus &status,
                            uint32_t timeout_ms = 0) const noexcept;

  /**
   * @brief Get PWM implementation capabilities (thread-safe)
   * @param capabilities Capabilities structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr GetCapabilities(PwmCapabilities &capabilities) const noexcept;

  /**
   * @brief Get last error for a specific channel (thread-safe)
   * @param channel_id Channel identifier
   * @return Last error code for the channel
   */
  HfPwmErr GetLastError(uint8_t channel_id) const noexcept;

  //==============================================================================
  // THREAD-SAFE CALLBACK MANAGEMENT
  //==============================================================================

  /**
   * @brief Set period complete callback (thread-safe)
   * @param callback Callback function
   * @param user_data User data passed to callback
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetPeriodCallback(PwmPeriodCallback callback, void *user_data = nullptr,
                             uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Set fault/error callback (thread-safe)
   * @param callback Callback function
   * @param user_data User data passed to callback
   * @param timeout_ms Mutex timeout in milliseconds (0 = use default)
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr SetFaultCallback(PwmFaultCallback callback, void *user_data = nullptr,
                            uint32_t timeout_ms = 0) noexcept;

  //==============================================================================
  // THREAD SAFETY AND DIAGNOSTICS
  //==============================================================================

  /**
   * @brief Try to acquire exclusive access (non-blocking)
   * @return true if lock acquired, false if already locked
   */
  bool TryLock() noexcept;

  /**
   * @brief Acquire exclusive access (blocking with timeout)
   * @param timeout_ms Timeout in milliseconds
   * @return true if lock acquired, false on timeout
   */
  bool Lock(uint32_t timeout_ms) noexcept;

  /**
   * @brief Release exclusive access
   */
  void Unlock() noexcept;

  /**
   * @brief Get performance statistics (if enabled)
   * @param stats Statistics structure to fill
   * @return PWM_SUCCESS on success, error code on failure
   */
  HfPwmErr GetStatistics(Statistics &stats) const noexcept;

  /**
   * @brief Reset performance statistics
   */
  void ResetStatistics() noexcept;

  /**
   * @brief Get underlying PWM implementation (thread-safe access)
   * @note Use with extreme caution - direct access bypasses thread safety
   * @return Pointer to underlying PWM implementation, or nullptr if locked
   */
  BasePwm *GetUnsafeDirectAccess() const noexcept;

private:
  //==============================================================================
  // INTERNAL METHODS
  //==============================================================================

  /**
   * @brief Get effective timeout (use default if 0 specified)
   * @param timeout_ms Requested timeout
   * @return Effective timeout to use
   */
  uint32_t GetEffectiveTimeout(uint32_t timeout_ms) const noexcept;

  /**
   * @brief Record operation statistics
   * @param operation_time_us Time taken for operation
   * @param lock_time_us Time taken to acquire lock
   * @param timed_out Whether operation timed out
   */
  void RecordStatistics(uint64_t operation_time_us, uint64_t lock_time_us, bool timed_out) noexcept;

  /**
   * @brief Scoped lock helper with timeout and statistics
   */
  class ScopedLock {
  public:
    ScopedLock(SfPwm *parent, uint32_t timeout_ms) noexcept;
    ~ScopedLock() noexcept;

    bool IsLocked() const noexcept {
      return locked_;
    }
    HfPwmErr GetError() const noexcept {
      return error_;
    }

  private:
    SfPwm *parent_;
    bool locked_;
    HfPwmErr error_;
    uint64_t start_time_us_;
    uint64_t lock_acquired_time_us_;
  };

  //==============================================================================
  // MEMBER VARIABLES
  //==============================================================================

  std::unique_ptr<BasePwm> pwm_impl_; ///< Underlying PWM implementation
  mutable RtosMutex mutex_;           ///< Thread safety mutex
  Config config_;                     ///< Configuration
  bool initialized_;                  ///< Initialization state

  // Statistics (protected by mutex)
  mutable Statistics statistics_; ///< Performance statistics

  static constexpr uint32_t DEFAULT_TIMEOUT_MS = 5000; ///< Default mutex timeout
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_SFPWM_H_
