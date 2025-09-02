/**
 * @file EspPeriodicTimer.h
 * @brief MCU-integrated periodic timer implementation.
 *
 * This header provides a periodic timer implementation for microcontrollers with
 * built-in timer peripherals. On ESP32, this wraps the ESP timer API,
 * on other MCUs it would wrap hardware timers, etc. The implementation supports
 * high-precision timing, periodic callbacks, and timer management features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the primary timer implementation for MCUs with integrated timer capabilities.
 */
#pragma once

#include "BasePeriodicTimer.h"
#include "EspTypes.h"
#include <cstdint>

/**
 * @class EspPeriodicTimer
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
class EspPeriodicTimer : public BasePeriodicTimer {
public:
  /**
   * @brief Constructor with callback specification.
   * @param callback Callback function to be called on timer expiry
   * @param user_data User data passed to callback function
   */
  EspPeriodicTimer(hf_timer_callback_t callback = nullptr, void* user_data = nullptr) noexcept;

  /**
   * @brief Destructor - ensures timer is stopped and resources are freed.
   */
  ~EspPeriodicTimer() noexcept override;

  // Copy/move constructors and assignment operators
  EspPeriodicTimer(const EspPeriodicTimer&) = delete;
  EspPeriodicTimer& operator=(const EspPeriodicTimer&) = delete;
  EspPeriodicTimer(EspPeriodicTimer&&) = delete;
  EspPeriodicTimer& operator=(EspPeriodicTimer&&) = delete;

  //==============================================//
  // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
  //==============================================//

  /**
   * @brief Initialize the timer.
   * @return Success or specific error code
   */
  hf_timer_err_t Initialize() noexcept override;

  /**
   * @brief Deinitialize the timer and free resources.
   * @return Success or specific error code
   */
  hf_timer_err_t Deinitialize() noexcept override;

  /**
   * @brief Start the timer with specified period.
   * @param period_us Timer period in microseconds
   * @return Success or specific error code
   */
  hf_timer_err_t Start(hf_u64_t period_us) noexcept override;

  /**
   * @brief Stop the timer.
   * @return Success or specific error code
   */
  hf_timer_err_t Stop() noexcept override;

  /**
   * @brief Change timer period (timer can be running or stopped).
   * @param new_period_us New period in microseconds
   * @return Success or specific error code
   */
  hf_timer_err_t SetPeriod(hf_u64_t new_period_us) noexcept override;

  /**
   * @brief Get current timer period.
   * @param period_us Reference to store current period
   * @return Success or specific error code
   */
  hf_timer_err_t GetPeriod(hf_u64_t& period_us) noexcept override;

  /**
   * @brief Get timer statistics and status information.
   * @param callback_count Number of callbacks executed
   * @param missed_callbacks Number of missed callbacks (if supported)
   * @param last_error Last error that occurred
   * @return Success or specific error code
   */
  hf_timer_err_t GetStats(hf_u64_t& callback_count, hf_u64_t& missed_callbacks,
                          hf_timer_err_t& last_error) noexcept override;

  /**
   * @brief Reset timer statistics.
   * @return Success or specific error code
   */
  hf_timer_err_t ResetStats() noexcept override;

  /**
   * @brief Get description of this timer implementation.
   * @return Description string
   */
  const char* GetDescription() const noexcept override;

  /**
   * @brief Get minimum supported timer period.
   * @return Minimum period in microseconds
   */
  hf_u64_t GetMinPeriod() const noexcept override;

  /**
   * @brief Get maximum supported timer period.
   * @return Maximum period in microseconds
   */
  hf_u64_t GetMaxPeriod() const noexcept override;

  /**
   * @brief Get timer resolution.
   * @return Timer resolution in microseconds
   */
  hf_u64_t GetResolution() const noexcept override;

  /**
   * @brief Get timer operation statistics.
   * @param statistics Reference to statistics structure to fill
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  hf_timer_err_t GetStatistics(hf_timer_statistics_t& statistics) const noexcept override;

  /**
   * @brief Get timer diagnostic information.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  hf_timer_err_t GetDiagnostics(hf_timer_diagnostics_t& diagnostics) const noexcept override;

private:
  hf_timer_handle_t timer_handle_; ///< Platform-specific timer handle
  hf_u64_t period_us_;             ///< Current timer period in microseconds
  hf_timer_stats_t stats_;         ///< Timer statistics

  /**
   * @brief Convert platform-specific error to hf_timer_err_t.
   * @param platform_error Platform-specific error code
   * @return Corresponding hf_timer_err_t
   */
  hf_timer_err_t ConvertError(int platform_error) const noexcept;

  /**
   * @brief Validate timer period.
   * @param period_us Period to validate in microseconds
   * @return True if valid, false otherwise
   */
  bool ValidatePeriod(hf_u64_t period_us) const noexcept;

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
   * @brief Internal timer callback dispatcher (ISR-safe C bridge).
   * @param arg User data (pointer to EspPeriodicTimer instance)
   * @note This is the C callback that bridges to C++ class methods
   */
  static void InternalTimerCallback(void* arg);
};
