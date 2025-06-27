/**
 * @file BasePeriodicTimer.h
 * @brief Abstract base class for periodic timer implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for periodic timer functionality
 * that provides a consistent API across different timer implementations.
 * Concrete implementations (like McuPeriodicTimer for ESP32 timers, OS timers) inherit from this
 * class.
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASEPERIODICTIMER_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASEPERIODICTIMER_H_

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>

//--------------------------------------
//  HardFOC Timer Error Codes (Table)
//--------------------------------------
/**
 * @brief HardFOC Timer error codes
 * @details Comprehensive error enumeration for all timer operations in the system.
 *          This enumeration is used across all timer-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_TIMER_ERR_LIST(X)                                                                       \
  /* Success codes */                                                                              \
  X(TIMER_SUCCESS, 0, "Success")                                                                   \
  /* General errors */                                                                             \
  X(TIMER_ERR_FAILURE, 1, "General failure")                                                       \
  X(TIMER_ERR_NOT_INITIALIZED, 2, "Not initialized")                                               \
  X(TIMER_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                       \
  X(TIMER_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                           \
  X(TIMER_ERR_NULL_POINTER, 5, "Null pointer")                                                     \
  X(TIMER_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                   \
  /* Timer specific errors */                                                                      \
  X(TIMER_ERR_ALREADY_RUNNING, 7, "Timer already running")                                         \
  X(TIMER_ERR_NOT_RUNNING, 8, "Timer not running")                                                 \
  X(TIMER_ERR_INVALID_PERIOD, 9, "Invalid period")                                                 \
  X(TIMER_ERR_RESOURCE_BUSY, 10, "Timer resource busy")                                            \
  X(TIMER_ERR_HARDWARE_FAULT, 11, "Timer hardware fault")

// Generate enum class from X-macro
enum class HfTimerErr : int32_t {
#define X(name, value, desc) name = value,
  HF_TIMER_ERR_LIST(X)
#undef X
};

// Generate error description function
constexpr const char *HfTimerErrToString(HfTimerErr err) noexcept {
  switch (err) {
#define X(name, value, desc)                                                                       \
  case HfTimerErr::name:                                                                           \
    return desc;
    HF_TIMER_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @brief Timer callback function type.
 * @param user_data User-provided data passed to callback
 */
using TimerCallback = std::function<void(void *user_data)>;

/**
 * @class BasePeriodicTimer
 * @brief Abstract base class for periodic timer operations.
 *
 * This class provides a consistent interface for periodic timer functionality across different
 * hardware platforms and timer implementations. It supports high-resolution timing,
 * callback-based notifications, and precise period control.
 *
 * Key Features:
 * - Microsecond resolution timing
 * - Callback-based event notification
 * - Start/stop control
 * - Period adjustment during operation
 * - Error handling and status reporting
 * - Platform-agnostic interface
 *
 * @note Implementations should handle platform-specific details internally
 * @note This class is designed to be thread-safe when properly implemented
 */
class BasePeriodicTimer {
public:
  /**
   * @brief Constructor with callback specification.
   * @param callback Timer callback function
   * @param user_data User data passed to callback
   */
  explicit BasePeriodicTimer(TimerCallback callback, void *user_data = nullptr) noexcept
      : callback_(callback), user_data_(user_data), initialized_(false), running_(false) {}

  /**
   * @brief Virtual destructor to ensure proper cleanup.
   */
  virtual ~BasePeriodicTimer() noexcept = default;

  // Disable copy constructor and assignment operator for safety
  BasePeriodicTimer(const BasePeriodicTimer &) = delete;
  BasePeriodicTimer &operator=(const BasePeriodicTimer &) = delete;

  //==============================================//
  // PURE VIRTUAL FUNCTIONS (MUST BE IMPLEMENTED) //
  //==============================================//

  /**
   * @brief Initialize the timer hardware/resources.
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the timer and free resources.
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr Deinitialize() noexcept = 0;

  /**
   * @brief Start the periodic timer with specified period.
   * @param period_us Timer period in microseconds
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr Start(uint64_t period_us) noexcept = 0;

  /**
   * @brief Stop the periodic timer.
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr Stop() noexcept = 0;

  /**
   * @brief Change the timer period while running.
   * @param period_us New timer period in microseconds
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr SetPeriod(uint64_t period_us) noexcept = 0;

  /**
   * @brief Get the current timer period.
   * @param period_us Reference to store the current period
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr GetPeriod(uint64_t &period_us) noexcept = 0;

  /**
   * @brief Get timer statistics and status information.
   * @param callback_count Number of callbacks executed
   * @param missed_callbacks Number of missed callbacks (if supported)
   * @param last_error Last error that occurred
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr GetStats(uint64_t &callback_count, uint64_t &missed_callbacks,
                              HfTimerErr &last_error) noexcept = 0;

  /**
   * @brief Reset timer statistics.
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual HfTimerErr ResetStats() noexcept = 0;

  //==============================================//
  // PUBLIC INTERFACE (IMPLEMENTED)               //
  //==============================================//

  /**
   * @brief Check if timer is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Check if timer is currently running.
   * @return true if running, false otherwise
   */
  bool IsRunning() const noexcept {
    return running_;
  }

  /**
   * @brief Get description of this timer implementation.
   * @return Description string
   */
  virtual const char *GetDescription() const noexcept = 0;

  /**
   * @brief Get minimum supported timer period.
   * @return Minimum period in microseconds
   */
  virtual uint64_t GetMinPeriod() const noexcept = 0;

  /**
   * @brief Get maximum supported timer period.
   * @return Maximum period in microseconds
   */
  virtual uint64_t GetMaxPeriod() const noexcept = 0;

  /**
   * @brief Get timer resolution.
   * @return Timer resolution in microseconds
   */
  virtual uint64_t GetResolution() const noexcept = 0;

  /**
   * @brief Set new callback function.
   * @param callback New callback function
   * @param user_data New user data
   * @return HfTimerErr::TIMER_SUCCESS if successful, error code otherwise
   */
  HfTimerErr SetCallback(TimerCallback callback, void *user_data = nullptr) noexcept {
    if (IsRunning()) {
      return HfTimerErr::TIMER_ERR_ALREADY_RUNNING;
    }
    callback_ = callback;
    user_data_ = user_data;
    return HfTimerErr::TIMER_SUCCESS;
  }

  /**
   * @brief Get current user data pointer.
   * @return User data pointer
   */
  void *GetUserData() const noexcept {
    return user_data_;
  }

protected:
  /**
   * @brief Set the initialized state.
   * @param initialized New initialization state
   */
  void SetInitialized(bool initialized) noexcept {
    initialized_ = initialized;
  }

  /**
   * @brief Set the running state.
   * @param running New running state
   */
  void SetRunning(bool running) noexcept {
    running_ = running;
  }

  /**
   * @brief Execute the timer callback (called by implementations).
   */
  void ExecuteCallback() noexcept {
    if (callback_) {
      callback_(user_data_);
    }
  }

  /**
   * @brief Check if callback is valid.
   * @return true if callback is set, false otherwise
   */
  bool HasValidCallback() const noexcept {
    return static_cast<bool>(callback_);
  }

private:
  TimerCallback callback_; ///< Timer callback function
  void *user_data_;        ///< User data passed to callback
  bool initialized_;       ///< Initialization state flag
  bool running_;           ///< Running state flag
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_BASEPERIODICTIMER_H_
