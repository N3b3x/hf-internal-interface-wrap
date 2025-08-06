/**
 * @file TimerComprehensiveTest.cpp
 * @brief Comprehensive Periodic Timer testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BasePeriodicTimer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspPeriodicTimer.h"

#include "TestFramework.h"

static const char* TAG = "TIMER_Test";

static TestResults g_test_results;

bool test_timer_initialization() noexcept {
  ESP_LOGI(TAG, "Testing periodic timer initialization...");

  auto timer_callback = [](void* user_data) {
    static int count = 0;
    count++;
    if (count % 10 == 0) {
      ESP_LOGI("Timer", "Timer callback executed %d times", count);
    }
  };

  EspPeriodicTimer test_timer(timer_callback, nullptr);
  auto timer_init = test_timer.IsInitialized();

  if (!timer_init) {
    ESP_LOGE(TAG, "Failed to initialize periodic timer");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Periodic timer initialization successful");

  // Test starting the timer
  auto start_result = test_timer.Start(1000000); // 1 second interval
  if (start_result == hf_timer_err_t::TIMER_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Periodic timer started with 1-second interval");
    vTaskDelay(pdMS_TO_TICKS(3000)); // Let it run for 3 seconds
    test_timer.Stop();
    ESP_LOGI(TAG, "[SUCCESS] Periodic timer stopped");
  } else {
    ESP_LOGW(TAG, "Could not start timer: %d, but initialization was successful", static_cast<int>(start_result));
  }

  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                  ESP32-C6 TIMER COMPREHENSIVE TEST SUITE                   ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_timer_initialization);
  print_test_summary(g_test_results, "TIMER", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
