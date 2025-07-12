/**
 * @file BasePeriodicTimer.h
 * @brief Abstract base class for periodic timer implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for periodic timer functionality
 * that provides a consistent API across different timer implementations.
 * Concrete implementations for various platforms inherit from this class to provide
 * high-precision periodic callbacks, interval timing, and timer management features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
 * @note Users should program against this interface, not specific implementations.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>

#ifndef HF_TIMESTAMP_US_T_DEFINED
#define HF_TIMESTAMP_US_T_DEFINED
using hf_timestamp_us_t = hf_u64_t;
#endif

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
  X(TIMER_ERR_HARDWARE_FAULT, 11, "Timer hardware fault")                                          \
  X(TIMER_ERR_UNSUPPORTED_OPERATION, 12, "Unsupported operation") 

// Generate enum class from X-macro
enum class hf_timer_err_t : hf_i32_t {
#define X(name, value, desc) name = value,
  HF_TIMER_ERR_LIST(X)
#undef X
};

// Generate error description function
constexpr const char *HfTimerErrToString(hf_timer_err_t err) noexcept {
  switch (err) {
#define X(name, value, desc)                                                                       \
  case hf_timer_err_t::name:                                                                       \
    return desc;
    HF_TIMER_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @brief Timer statistics structure.
 */
struct hf_timer_stats_t {
  hf_u64_t start_count;
  hf_u64_t stop_count;
  hf_u64_t callback_count;
  hf_u64_t missed_callbacks;
  hf_timer_err_t last_error;
  hf_timestamp_us_t last_start_us;

  hf_timer_stats_t() noexcept
      : start_count(0), stop_count(0), callback_count(0), missed_callbacks(0),
        last_error(hf_timer_err_t::TIMER_SUCCESS), last_start_us(0) {}
};

/**
 * @brief Timer operation statistics.
 */
struct hf_timer_statistics_t {
  hf_u32_t totalStarts;          ///< Total timer starts
  hf_u32_t totalStops;           ///< Total timer stops
  hf_u32_t callbackExecutions;   ///< Number of callback executions
  hf_u32_t missedCallbacks;      ///< Number of missed callbacks
  hf_u32_t averageCallbackTimeUs; ///< Average callback execution time (microseconds)
  hf_u32_t maxCallbackTimeUs;    ///< Maximum callback execution time
  hf_u32_t minCallbackTimeUs;    ///< Minimum callback execution time
  hf_u64_t totalRunningTimeUs;   ///< Total running time in microseconds

  hf_timer_statistics_t()
      : totalStarts(0), totalStops(0), callbackExecutions(0), missedCallbacks(0),
        averageCallbackTimeUs(0), maxCallbackTimeUs(0), minCallbackTimeUs(UINT32_MAX),
        totalRunningTimeUs(0) {}
};

/**
 * @brief Timer diagnostic information.
 */
struct hf_timer_diagnostics_t {
  bool timerHealthy;             ///< Overall timer health status
  hf_timer_err_t lastErrorCode;  ///< Last error code
  hf_u32_t lastErrorTimestamp;   ///< Last error timestamp
  hf_u32_t consecutiveErrors;    ///< Consecutive error count
  bool timerInitialized;         ///< Timer initialization status
  bool timerRunning;             ///< Timer running status
  hf_u64_t currentPeriodUs;      ///< Current timer period in microseconds
  hf_u64_t timerResolutionUs;    ///< Timer resolution in microseconds

  hf_timer_diagnostics_t()
      : timerHealthy(true), lastErrorCode(hf_timer_err_t::TIMER_SUCCESS), lastErrorTimestamp(0), 
          consecutiveErrors(0), timerInitialized(false), timerRunning(false), 
          currentPeriodUs(0), timerResolutionUs(0) {}
};

/**
 * @brief Timer callback function type.
 * @param user_data User-provided data passed to callback
 */
using hf_timer_callback_t = std::function<void(void *user_data)>;

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
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the timer and free resources.
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Start the periodic timer with specified period.
   * @param period_us Timer period in microseconds
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t Start(hf_u64_t period_us) noexcept = 0;

  /**
   * @brief Stop the periodic timer.
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t Stop() noexcept = 0;

  /**
   * @brief Change the timer period while running.
   * @param period_us New timer period in microseconds
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t SetPeriod(hf_u64_t period_us) noexcept = 0;

  /**
   * @brief Get the current timer period.
   * @param period_us Reference to store the current period
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t GetPeriod(hf_u64_t &period_us) noexcept = 0;

  /**
   * @brief Get timer statistics and status information.
   * @param callback_count Number of callbacks executed
   * @param missed_callbacks Number of missed callbacks (if supported)
   * @param last_error Last error that occurred
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t GetStats(hf_u64_t &callback_count, hf_u64_t &missed_callbacks,
                                  hf_timer_err_t &last_error) noexcept = 0;

  /**
   * @brief Reset timer statistics.
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  virtual hf_timer_err_t ResetStats() noexcept = 0;

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
  virtual hf_u64_t GetMaxPeriod() const noexcept = 0;

  /**
   * @brief Get maximum supported timer period.
   * @return Maximum period in microseconds
   */
  virtual hf_u64_t GetMinPeriod() const noexcept = 0;

  /**
   * @brief Get timer resolution.
   * @return Timer resolution in microseconds
   */
  virtual hf_u64_t GetResolution() const noexcept = 0;

  /**
   * @brief Set new callback function.
   * @param callback New callback function
   * @param user_data New user data
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   */
  hf_timer_err_t SetCallback(hf_timer_callback_t callback, void *user_data = nullptr) noexcept {
    if (IsRunning()) {
      return hf_timer_err_t::TIMER_ERR_ALREADY_RUNNING;
    }
    callback_ = callback;
    user_data_ = user_data;
    return hf_timer_err_t::TIMER_SUCCESS;
  }

  /**
   * @brief Get current user data pointer.
   * @return User data pointer
   */
  void *GetUserData() const noexcept {
    return user_data_;
  }

  //==============================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================//

  /**
   * @brief Reset timer operation statistics.
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_timer_err_t ResetStatistics() noexcept {
    statistics_ = hf_timer_statistics_t{}; // Reset statistics to default values
    return hf_timer_err_t::TIMER_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset timer diagnostic information.
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_timer_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_timer_diagnostics_t{}; // Reset diagnostics to default values
    return hf_timer_err_t::TIMER_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get timer operation statistics
   * @param statistics Reference to store statistics data
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, TIMER_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_timer_err_t GetStatistics(hf_timer_statistics_t &statistics) const noexcept {
    statistics = statistics_; // Return statistics by default
    return hf_timer_err_t::TIMER_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get timer diagnostic information
   * @param diagnostics Reference to store diagnostics data
   * @return hf_timer_err_t::TIMER_SUCCESS if successful, TIMER_ERR_NOT_SUPPORTED if not implemented
   */
  virtual hf_timer_err_t GetDiagnostics(hf_timer_diagnostics_t &diagnostics) const noexcept {
    diagnostics = diagnostics_; // Return diagnostics by default
    return hf_timer_err_t::TIMER_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  /**
   * @brief Protected constructor with callback specification.
   * @param callback Timer callback function
   * @param user_data User data passed to callback
   */
  explicit BasePeriodicTimer(hf_timer_callback_t callback, void *user_data = nullptr) noexcept
      : callback_(callback), user_data_(user_data), initialized_(false), running_(false), statistics_{}, diagnostics_{} {}

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

  hf_timer_callback_t callback_; ///< Timer callback function
  void *user_data_;              ///< User data passed to callback
  bool initialized_;             ///< Initialization state flag
  bool running_;                 ///< Running state flag
  hf_timer_statistics_t statistics_; ///< Timer operation statistics
  hf_timer_diagnostics_t diagnostics_; ///< Timer diagnostic information
};
