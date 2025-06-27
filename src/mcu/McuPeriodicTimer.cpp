/**
 * @file McuPeriodicTimer.cpp
 * @brief Implementation of MCU-integrated periodic timer.
 */

#include "McuPeriodicTimer.h"

// Platform-specific includes
#ifdef HF_MCU_FAMILY_ESP32
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#else
#error "Unsupported MCU platform. Please add support for your target MCU."
#endif

static const char *TAG = "McuPeriodicTimer";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

McuPeriodicTimer::McuPeriodicTimer(TimerCallback callback, void *user_data) noexcept
    : BasePeriodicTimer(callback, user_data), timer_handle_(nullptr), period_us_(0), stats_{} {
  ESP_LOGD(TAG, "McuPeriodicTimer constructor");
}

McuPeriodicTimer::~McuPeriodicTimer() noexcept {
  Deinitialize();
  ESP_LOGD(TAG, "McuPeriodicTimer destructor");
}

//==============================================================================
// BASEPERIODICTIMER INTERFACE IMPLEMENTATION
//==============================================================================

HfTimerErr McuPeriodicTimer::Initialize() noexcept {
  if (IsInitialized()) {
    ESP_LOGW(TAG, "Timer already initialized");
    return HfTimerErr::TIMER_ERR_ALREADY_INITIALIZED;
  }

  if (!HasValidCallback()) {
    ESP_LOGE(TAG, "No callback function provided");
    return HfTimerErr::TIMER_ERR_NULL_POINTER;
  }

  // Create ESP32 timer handle
  if (!CreateTimerHandle()) {
    ESP_LOGE(TAG, "Failed to create timer handle");
    return HfTimerErr::TIMER_ERR_HARDWARE_FAULT;
  }

  SetInitialized(true);
  stats_ = {}; // Reset statistics
  ESP_LOGI(TAG, "Timer initialized successfully");
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::Deinitialize() noexcept {
  if (!IsInitialized()) {
    return HfTimerErr::TIMER_SUCCESS;
  }

  // Stop timer if running
  if (IsRunning()) {
    Stop();
  }

  // Clean up timer handle
  DestroyTimerHandle();

  SetInitialized(false);
  period_us_ = 0;
  ESP_LOGI(TAG, "Timer deinitialized");
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::Start(uint64_t period_us) noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }

  if (IsRunning()) {
    ESP_LOGW(TAG, "Timer already running");
    return HfTimerErr::TIMER_ERR_ALREADY_RUNNING;
  }

  if (!ValidatePeriod(period_us)) {
    ESP_LOGE(TAG, "Invalid period: %llu us", period_us);
    return HfTimerErr::TIMER_ERR_INVALID_PERIOD;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret =
      esp_timer_start_periodic(static_cast<esp_timer_handle_t>(timer_handle_), period_us);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_start_periodic failed: %s", esp_err_to_name(ret));
    stats_.last_error = ConvertError(ret);
    return stats_.last_error;
  }
#endif

  period_us_ = period_us;
  SetRunning(true);
  stats_.start_count++;
  ESP_LOGI(TAG, "Timer started with period %llu us", period_us);
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::Stop() noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }

  if (!IsRunning()) {
    ESP_LOGW(TAG, "Timer not running");
    return HfTimerErr::TIMER_ERR_NOT_RUNNING;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = esp_timer_stop(static_cast<esp_timer_handle_t>(timer_handle_));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_stop failed: %s", esp_err_to_name(ret));
    stats_.last_error = ConvertError(ret);
    return stats_.last_error;
  }
#endif

  SetRunning(false);
  stats_.stop_count++;
  ESP_LOGI(TAG, "Timer stopped");
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::SetPeriod(uint64_t new_period_us) noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }

  if (!ValidatePeriod(new_period_us)) {
    ESP_LOGE(TAG, "Invalid period: %llu us", new_period_us);
    return HfTimerErr::TIMER_ERR_INVALID_PERIOD;
  }

  bool was_running = IsRunning();

  // Stop timer if running
  if (was_running) {
    HfTimerErr stop_result = Stop();
    if (stop_result != HfTimerErr::TIMER_SUCCESS) {
      return stop_result;
    }
  }

  period_us_ = new_period_us;

  // Restart with new period if it was running
  if (was_running) {
    return Start(new_period_us);
  }

  ESP_LOGD(TAG, "Period set to %llu us", new_period_us);
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::GetPeriod(uint64_t &period_us) noexcept {
  if (!IsInitialized()) {
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }
  period_us = period_us_;
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::GetStats(uint64_t &callback_count, uint64_t &missed_callbacks,
                                      HfTimerErr &last_error) noexcept {
  if (!IsInitialized()) {
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }
  callback_count = stats_.callback_count;
  missed_callbacks = stats_.missed_callbacks;
  last_error = stats_.last_error;
  return HfTimerErr::TIMER_SUCCESS;
}

HfTimerErr McuPeriodicTimer::ResetStats() noexcept {
  if (!IsInitialized()) {
    return HfTimerErr::TIMER_ERR_NOT_INITIALIZED;
  }
  stats_ = {};
  return HfTimerErr::TIMER_SUCCESS;
}

const char *McuPeriodicTimer::GetDescription() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return "ESP32 MCU Periodic Timer (ESP Timer API)";
#else
  return "MCU Periodic Timer (Unknown platform)";
#endif
}

uint64_t McuPeriodicTimer::GetMinPeriod() const noexcept {
  // ESP32 timer supports periods from 1us
  return 1;
}

uint64_t McuPeriodicTimer::GetMaxPeriod() const noexcept {
  // ESP32 timer supports very large periods, but we limit for safety
  return UINT64_MAX / 2;
}

uint64_t McuPeriodicTimer::GetResolution() const noexcept {
  // ESP32 timer has 1us resolution
  return 1;
}
//==============================================================================
// PRIVATE METHODS
//==============================================================================

HfTimerErr McuPeriodicTimer::ConvertError(int platform_error) const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  switch (platform_error) {
  case ESP_OK:
    return HfTimerErr::TIMER_SUCCESS;
  case ESP_ERR_INVALID_ARG:
    return HfTimerErr::TIMER_ERR_INVALID_PARAMETER;
  case ESP_ERR_NO_MEM:
    return HfTimerErr::TIMER_ERR_OUT_OF_MEMORY;
  case ESP_ERR_INVALID_STATE:
    return HfTimerErr::TIMER_ERR_ALREADY_RUNNING;
  default:
    return HfTimerErr::TIMER_ERR_FAILURE;
  }
#else
  return HfTimerErr::TIMER_ERR_FAILURE;
#endif
}

bool McuPeriodicTimer::ValidatePeriod(uint64_t period_us) const noexcept {
  return (period_us >= GetMinPeriod() && period_us <= GetMaxPeriod());
}

bool McuPeriodicTimer::CreateTimerHandle() noexcept {
  if (timer_handle_) {
    return true; // Already created
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_timer_create_args_t timer_args = {};
  timer_args.callback = [](void *arg) {
    auto *self = static_cast<McuPeriodicTimer *>(arg);
    if (self && self->HasValidCallback()) {
      self->stats_.callback_count++;
      self->ExecuteCallback();
    }
  };
  timer_args.arg = this;
  timer_args.dispatch_method = ESP_TIMER_TASK;
  timer_args.name = "HardFOC_Timer";

  esp_timer_handle_t esp_handle;
  esp_err_t ret = esp_timer_create(&timer_args, &esp_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_create failed: %s", esp_err_to_name(ret));
    return false;
  }

  timer_handle_ = static_cast<hf_timer_handle_t>(esp_handle);
  return true;
#else
  return false;
#endif
}

void McuPeriodicTimer::DestroyTimerHandle() noexcept {
  if (!timer_handle_) {
    return;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_timer_handle_t esp_handle = static_cast<esp_timer_handle_t>(timer_handle_);
  esp_err_t ret = esp_timer_delete(esp_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_delete failed: %s", esp_err_to_name(ret));
  }
#endif

  timer_handle_ = nullptr;
}
