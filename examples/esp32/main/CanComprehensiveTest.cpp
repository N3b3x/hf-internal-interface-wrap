/**
 * @file CanComprehensiveTest.cpp
 * @brief Comprehensive CAN testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "base/BaseCan.h"
#include "mcu/esp32/EspCan.h"

#include "TestFramework.h"

static const char* TAG = "CAN_Test";

static TestResults g_test_results;

bool test_can_initialization() noexcept {
  ESP_LOGI(TAG, "Testing CAN bus initialization...");
  
  hf_esp_can_config_t can_cfg = {};
  can_cfg.controller_id = hf_can_controller_id_t::HF_CAN_CONTROLLER_0;
  can_cfg.tx_pin = 7;
  can_cfg.rx_pin = 6;
  can_cfg.tx_queue_len = 8;
  EspCan test_can(can_cfg);
  
  if (!test_can.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize CAN");
    return false;
  }
  
  ESP_LOGI(TAG, "[SUCCESS] CAN initialization successful");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 CAN COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_can_initialization);
  print_test_summary(g_test_results, "CAN", TAG);
  while (true) vTaskDelay(pdMS_TO_TICKS(10000));
}
