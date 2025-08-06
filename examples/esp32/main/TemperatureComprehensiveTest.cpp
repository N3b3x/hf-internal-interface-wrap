/**
 * @file TemperatureComprehensiveTest.cpp
 * @brief Comprehensive Temperature sensor testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BaseTemperature.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspTemperature.h"

#include "TestFramework.h"

static const char* TAG = "TEMP_Test";

static TestResults g_test_results;

bool test_temperature_sensor_initialization() noexcept {
  ESP_LOGI(TAG, "Testing temperature sensor initialization...");

  EspTemperature test_temp;
  auto temp_init = test_temp.IsInitialized();

  if (temp_init != hf_temp_err_t::TEMP_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize temperature sensor");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Temperature sensor initialization successful");

  // Read temperature if initialized successfully
  hf_temp_reading_t temp_reading = {};
  auto temp_read = test_temp.ReadTemperature(&temp_reading);
  if (temp_read == hf_temp_err_t::TEMP_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Chip temperature: %.2f°C", temp_reading.temperature_raw);
  } else {
    ESP_LOGW(TAG, "Could not read temperature, but initialization was successful");
  }

  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 TEMPERATURE COMPREHENSIVE TEST SUITE                ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_temperature_sensor_initialization);
  print_test_summary(g_test_results, "TEMPERATURE", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
