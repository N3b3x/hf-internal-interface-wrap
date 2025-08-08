/**
 * @file WifiComprehensiveTest.cpp
 * @brief Comprehensive WiFi testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "TestFramework.h"
#include "base/BaseWifi.h"
#include "mcu/esp32/EspWifi.h"

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

static const char* TAG = "WIFI_Test";

static TestResults g_test_results;

bool test_wifi_initialization() noexcept {
  ESP_LOGI(TAG, "Testing WiFi initialization...");

  EspWifi test_wifi;
  auto wifi_init = test_wifi.IsInitialized();

  if (!wifi_init) {
    ESP_LOGE(TAG, "Failed to initialize WiFi");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi initialization successful");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                   ESP32-C6 WIFI COMPREHENSIVE TEST SUITE                   ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_wifi_initialization);
  print_test_summary(g_test_results, "WIFI", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
