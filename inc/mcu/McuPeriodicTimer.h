/**
 * @file McuPeriodicTimer.h
 * @brief MCU-integrated periodic timer implementation.
 *
 * This header provides a periodic timer implementation for microcontrollers with
 * built-in timer peripherals. On ESP32, this wraps the ESP timer API,
 * on other MCUs it would wrap hardware timers, etc.
 *
 * This is the primary timer implementation for MCUs with integrated timer capabilities.
 */
#ifndef MCU_PERIODIC_TIMER_H
#define MCU_PERIODIC_TIMER_H

#include "BasePeriodicTimer.h"
#include "McuTypes.h"
#include <cstdint>

/**
 * @class McuPeriodicTimer
 * @brief MCU-integrated periodic timer implementation.
 *
 * This class provides periodic timer functionality using the microcontroller's built-in
 * timer peripherals. On ESP32, it uses the ESP timer API. The implementation handles
 * platform-specific details while providing the unified BasePeriodicTimer API.
 *
 * Features:
 * - High-precision periodic timing using MCU's integrated timers
 * - Microsecond resolution timing
 * - Callback-based event notification
 * - Start/stop control with period adjustment
 * - Comprehensive error handling and status reporting
 * - Lazy initialization support
 *
 * @note This implementation uses hardware timers for precise timing
 */
class McuPeriodicTimer : public BasePeriodicTimer {
public:
  /**
   * @brief Constructor with callback specification.
   * @param callback Callback function to be called on timer expiry
   * @param user_data User data passed to callback function
   */
  McuPeriodicTimer(TimerCallback callback = nullptr, void *user_data = nullptr) noexcept;

  /**
   * @brief Destructor - ensures timer is stopped and resources are freed.
   */
  ~McuPeriodicTimer() noexcept override;

  // Copy/move constructors and assignment operators
  McuPeriodicTimer(const McuPeriodicTimer &) = delete;
  McuPeriodicTimer &operator=(const McuPeriodicTimer &) = delete;
  McuPeriodicTimer(McuPeriodicTimer &&) = delete;
  McuPeriodicTimer &operator=(McuPeriodicTimer &&) = delete;

  //==============================================//
  // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
  //==============================================//

  /**
   * @brief Initialize the timer.
   * @return Success or specific error code
   */
  HfTimerErr Initialize() noexcept override;

  /**
   * @brief Deinitialize the timer and free resources.
   * @return Success or specific error code
   */
  HfTimerErr Deinitialize() noexcept override;

  /**
   * @brief Start the timer with specified period.
   * @param period_us Timer period in microseconds
   * @return Success or specific error code
   */
  HfTimerErr Start(uint64_t period_us) noexcept override;

  /**
   * @brief Stop the timer.
   * @return Success or specific error code
   */
  HfTimerErr Stop() noexcept override;

  /**
   * @brief Change timer period (timer can be running or stopped).
   * @param new_period_us New period in microseconds
   * @return Success or specific error code
   */
  HfTimerErr SetPeriod(uint64_t new_period_us) noexcept override;

  /**
   * @brief Get current timer period.
   * @param period_us Reference to store current period
   * @return Success or specific error code
   */
  HfTimerErr GetPeriod(uint64_t &period_us) noexcept override;

  /**
   * @brief Get timer statistics and status information.
   * @param callback_count Number of callbacks executed
   * @param missed_callbacks Number of missed callbacks (if supported)
   * @param last_error Last error that occurred
   * @return Success or specific error code
   */
  HfTimerErr GetStats(uint64_t &callback_count, uint64_t &missed_callbacks,
                      HfTimerErr &last_error) noexcept override;

  /**
   * @brief Reset timer statistics.
   * @return Success or specific error code
   */
  HfTimerErr ResetStats() noexcept override;

  /**
   * @brief Get description of this timer implementation.
   * @return Description string
   */
  const char *GetDescription() const noexcept override;

  /**
   * @brief Get minimum supported timer period.
   * @return Minimum period in microseconds
   */
  uint64_t GetMinPeriod() const noexcept override;

  /**
   * @brief Get maximum supported timer period.
   * @return Maximum period in microseconds
   */
  uint64_t GetMaxPeriod() const noexcept override;

  /**
   * @brief Get timer resolution.
   * @return Timer resolution in microseconds
   */
  uint64_t GetResolution() const noexcept override;

private:
  hf_timer_handle_t timer_handle_; ///< Platform-specific timer handle
  uint64_t period_us_;             ///< Current timer period in microseconds
  TimerStats stats_;               ///< Timer statistics

  /**
   * @brief Convert platform-specific error to HfTimerErr.
   * @param platform_error Platform-specific error code
   * @return Corresponding HfTimerErr
   */
  HfTimerErr ConvertError(int platform_error) const noexcept;

  /**
   * @brief Validate timer period.
   * @param period_us Period to validate in microseconds
   * @return True if valid, false otherwise
   */
  bool ValidatePeriod(uint64_t period_us) const noexcept;

  /**
   * @brief Create platform-specific timer handle.
   * @return Success if handle created successfully
   */
  bool CreateTimerHandle() noexcept;

  /**
   * @brief Destroy platform-specific timer handle.
   */
  void DestroyTimerHandle() noexcept;

  /**
   * @brief Internal timer callback dispatcher.
   * @param timer_handle Platform-specific timer handle
   */
  static void TimerCallbackDispatcher(hf_timer_handle_t timer_handle);
};

#endif // MCU_PERIODIC_TIMER_H
