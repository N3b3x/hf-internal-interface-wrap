#include "PeriodicTimer.h"
#include <esp_log.h>

static const char *TAG = "PeriodicTimer";

PeriodicTimer::PeriodicTimer(Callback cb, void *arg) noexcept
    : handle(nullptr), userCb(cb), userArg(arg), running(false) {}

PeriodicTimer::~PeriodicTimer() noexcept {
  Stop();
  if (handle) {
    esp_timer_delete(handle);
    handle = nullptr;
  }
}

bool PeriodicTimer::CreateHandle() noexcept {
  if (handle)
    return true;
  esp_timer_create_args_t args = {};
  args.callback = &PeriodicTimer::Dispatch;
  args.arg = this;
  args.dispatch_method = ESP_TIMER_TASK;
  esp_err_t ret = esp_timer_create(&args, &handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_timer_create failed: %d", ret);
    return false;
  }
  return true;
}

bool PeriodicTimer::Start(uint64_t periodUs) noexcept {
  if (!CreateHandle())
    return false;
  esp_err_t ret = esp_timer_start_periodic(handle, periodUs);
  if (ret == ESP_OK)
    running = true;
  else
    ESP_LOGE(TAG, "esp_timer_start_periodic failed: %d", ret);
  return ret == ESP_OK;
}

bool PeriodicTimer::Stop() noexcept {
  if (!handle || !running)
    return true;
  esp_err_t ret = esp_timer_stop(handle);
  if (ret == ESP_OK)
    running = false;
  else
    ESP_LOGE(TAG, "esp_timer_stop failed: %d", ret);
  return ret == ESP_OK;
}

void PeriodicTimer::Dispatch(void *arg) {
  auto self = static_cast<PeriodicTimer *>(arg);
  if (self && self->userCb)
    self->userCb(self->userArg);
}
