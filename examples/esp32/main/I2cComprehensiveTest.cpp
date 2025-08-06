/**
 * @file I2cComprehensiveTest.cpp
 * @brief Comprehensive I2C testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BaseI2c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspI2c.h"

#include "TestFramework.h"

static const char* TAG = "I2C_Test";

static TestResults g_test_results;

bool test_i2c_initialization() noexcept {
  ESP_LOGI(TAG, "Testing I2C initialization...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;
  EspI2cBus test_i2c_bus(i2c_cfg);

  if (!test_i2c_bus.IsInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C initialization successful");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_i2c_initialization);
  print_test_summary(g_test_results, "I2C", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
