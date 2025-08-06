/**
 * @file NvsComprehensiveTest.cpp
 * @brief Comprehensive NVS (Non-Volatile Storage) testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BaseNvs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspNvs.h"

#include "TestFramework.h"

static const char* TAG = "NVS_Test";

static TestResults g_test_results;

bool test_nvs_initialization() noexcept {
  ESP_LOGI(TAG, "Testing NVS initialization...");

  // NVS configuration is handled internally by EspNvs
  EspNvs test_nvs("hardfoc");
  auto nvs_init = test_nvs.IsInitialized();

  if (!nvs_init) {
    ESP_LOGE(TAG, "Failed to initialize NVS");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] NVS initialization successful");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 NVS COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_nvs_initialization);
  print_test_summary(g_test_results, "NVS", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
