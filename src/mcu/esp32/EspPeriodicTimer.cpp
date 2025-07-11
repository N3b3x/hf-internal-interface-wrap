/**
 * @file EspPeriodicTimer.cpp
 * @brief Implementation of MCU-integrated periodic timer.
 *
 * This file provides the implementation for periodic timer functionality using
 * the microcontroller's built-in timer peripherals. On ESP32, this wraps the
 * ESP timer API for high-precision timing, periodic callbacks, and timer
 * management with microsecond resolution and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "EspPeriodicTimer.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>

#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#ifdef __cplusplus
}
#endif

static const char *TAG = "EspPeriodicTimer";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspPeriodicTimer::EspPeriodicTimer(hf_timer_callback_t callback, void *user_data) noexcept
    : BasePeriodicTimer(callback, user_data), timer_handle_(nullptr), period_us_(0), stats_{} {
  ESP_LOGD(TAG, "EspPeriodicTimer constructor");
}

EspPeriodicTimer::~EspPeriodicTimer() noexcept {
  Deinitialize();
  ESP_LOGD(TAG, "EspPeriodicTimer destructor");
}

//==============================================================================
// BASEPERIODICTIMER INTERFACE IMPLEMENTATION
//==============================================================================

hf_timer_err_t EspPeriodicTimer::Initialize() noexcept {
  if (IsInitialized()) {
    ESP_LOGW(TAG, "Timer already initialized");
    return hf_timer_err_t::TIMER_ERR_ALREADY_INITIALIZED;
  }

  if (!HasValidCallback()) {
    ESP_LOGE(TAG, "No callback function provided");
    return hf_timer_err_t::TIMER_ERR_NULL_POINTER;
  }

  // Create ESP32 timer handle
  if (!CreateTimerHandle()) {
    ESP_LOGE(TAG, "Failed to create timer handle");
    return hf_timer_err_t::TIMER_ERR_HARDWARE_FAULT;
  }

  SetInitialized(true);
  stats_ = {}; // Reset statistics
  ESP_LOGI(TAG, "Timer initialized successfully");
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::Deinitialize() noexcept {
  if (!IsInitialized()) {
    return hf_timer_err_t::TIMER_SUCCESS;
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
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::Start(uint64_t period_us) noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }

  if (IsRunning()) {
    ESP_LOGW(TAG, "Timer already running");
    return hf_timer_err_t::TIMER_ERR_ALREADY_RUNNING;
  }

  if (!ValidatePeriod(period_us)) {
    ESP_LOGE(TAG, "Invalid period: %llu us", period_us);
    return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;
  }

  esp_err_t ret =
      esp_timer_start_periodic(static_cast<esp_timer_handle_t>(timer_handle_), period_us);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_start_periodic failed: %s", esp_err_to_name(ret));
    stats_.last_error = ConvertError(ret);
    return stats_.last_error;
  }

  period_us_ = period_us;
  SetRunning(true);
  stats_.start_count++;
  ESP_LOGI(TAG, "Timer started with period %llu us", period_us);
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::Stop() noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }

  if (!IsRunning()) {
    ESP_LOGW(TAG, "Timer not running");
    return hf_timer_err_t::TIMER_ERR_NOT_RUNNING;
  }

  esp_err_t ret = esp_timer_stop(static_cast<esp_timer_handle_t>(timer_handle_));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_stop failed: %s", esp_err_to_name(ret));
    stats_.last_error = ConvertError(ret);
    return stats_.last_error;
  }

  SetRunning(false);
  stats_.stop_count++;
  ESP_LOGI(TAG, "Timer stopped");
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::SetPeriod(uint64_t new_period_us) noexcept {
  if (!IsInitialized()) {
    ESP_LOGE(TAG, "Timer not initialized");
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }

  if (!ValidatePeriod(new_period_us)) {
    ESP_LOGE(TAG, "Invalid period: %llu us", new_period_us);
    return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;
  }

  bool was_running = IsRunning();

  // Stop timer if running
  if (was_running) {
    hf_timer_err_t stop_result = Stop();
    if (stop_result != hf_timer_err_t::TIMER_SUCCESS) {
      return stop_result;
    }
  }

  period_us_ = new_period_us;

  // Restart with new period if it was running
  if (was_running) {
    return Start(new_period_us);
  }

  ESP_LOGD(TAG, "Period set to %llu us", new_period_us);
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::GetPeriod(uint64_t &period_us) noexcept {
  if (!IsInitialized()) {
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }
  period_us = period_us_;
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::GetStats(uint64_t &callback_count, uint64_t &missed_callbacks,
                                          hf_timer_err_t &last_error) noexcept {
  if (!IsInitialized()) {
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }
  callback_count = stats_.callback_count;
  missed_callbacks = stats_.missed_callbacks;
  last_error = stats_.last_error;
  return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::ResetStats() noexcept {
  if (!IsInitialized()) {
    return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
  }
  stats_ = {};
  return hf_timer_err_t::TIMER_SUCCESS;
}

const char *EspPeriodicTimer::GetDescription() const noexcept {
  return "ESP32 MCU Periodic Timer (ESP Timer API)";
}

uint64_t EspPeriodicTimer::GetMinPeriod() const noexcept {
  // ESP32 timer supports periods from 1us
  return 1;
}

uint64_t EspPeriodicTimer::GetMaxPeriod() const noexcept {
  // ESP32 timer supports very large periods, but we limit for safety
  return UINT64_MAX / 2;
}

uint64_t EspPeriodicTimer::GetResolution() const noexcept {
  // ESP32 timer has 1us resolution
  return 1;
}
//==============================================================================
// PRIVATE METHODS
//==============================================================================

hf_timer_err_t EspPeriodicTimer::ConvertError(int platform_error) const noexcept {
  switch (platform_error) {
  case ESP_OK:
    return hf_timer_err_t::TIMER_SUCCESS;
  case ESP_ERR_INVALID_ARG:
    return hf_timer_err_t::TIMER_ERR_INVALID_PARAMETER;
  case ESP_ERR_NO_MEM:
    return hf_timer_err_t::TIMER_ERR_OUT_OF_MEMORY;
  case ESP_ERR_INVALID_STATE:
    return hf_timer_err_t::TIMER_ERR_ALREADY_RUNNING;
  default:
    return hf_timer_err_t::TIMER_ERR_FAILURE;
  }
}

bool EspPeriodicTimer::ValidatePeriod(uint64_t period_us) const noexcept {
  return (period_us >= GetMinPeriod() && period_us <= GetMaxPeriod());
}

bool EspPeriodicTimer::CreateTimerHandle() noexcept {
  if (timer_handle_) {
    return true; // Already created
  }

  esp_timer_create_args_t timer_args = {};
  timer_args.callback = [](void *arg) {
    auto *self = static_cast<EspPeriodicTimer *>(arg);
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
}

void EspPeriodicTimer::DestroyTimerHandle() noexcept {
  if (!timer_handle_) {
    return;
  }

  esp_timer_handle_t esp_handle = static_cast<esp_timer_handle_t>(timer_handle_);
  esp_err_t ret = esp_timer_delete(esp_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_delete failed: %s", esp_err_to_name(ret));
  }

  timer_handle_ = nullptr;
}

//==============================================================================
// STATISTICS AND DIAGNOSTICS
//==============================================================================

hf_timer_err_t EspPeriodicTimer::GetStatistics(hf_timer_statistics_t &statistics) const noexcept
{
    statistics = statistics_;
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t EspPeriodicTimer::GetDiagnostics(hf_timer_diagnostics_t &diagnostics) const noexcept
{
    diagnostics = diagnostics_;
    return hf_timer_err_t::TIMER_SUCCESS;
}

#endif // HF_MCU_FAMILY_ESP32