/**
 * @file PwmComprehensiveTest.cpp
 * @brief Comprehensive PWM testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BasePwm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspPwm.h"

#include "TestFramework.h"

static const char* TAG = "PWM_Test";

static TestResults g_test_results;

bool test_pwm_initialization() noexcept {
  ESP_LOGI(TAG, "Testing PWM initialization...");

  hf_pwm_unit_config_t pwm_cfg = {};
  pwm_cfg.unit_id = 0;
  pwm_cfg.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE;
  pwm_cfg.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  pwm_cfg.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  pwm_cfg.enable_fade = true;
  pwm_cfg.enable_interrupts = true;
  EspPwm test_pwm(pwm_cfg);

  if (!test_pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] PWM initialization successful");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 PWM COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  vTaskDelay(pdMS_TO_TICKS(1000));
  RUN_TEST(test_pwm_initialization);
  print_test_summary(g_test_results, "PWM", TAG);
  while (true)
    vTaskDelay(pdMS_TO_TICKS(10000));
}
