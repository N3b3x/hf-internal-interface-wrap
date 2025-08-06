/**
 * @file UtilsComprehensiveTest.cpp
 * @brief Comprehensive utilities testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for utility functions
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * utility functionalities including memory utilities, hardware types, ASCII art
 * generation, and other support utilities.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <memory>

#include "base/HardwareTypes.h"
#include "utils/AsciiArtGenerator.h"
#include "utils/memory_utils.h"

// Shared test framework
#include "TestFramework.h"

static const char* TAG = "UTILS_Test";

static TestResults g_test_results;

// Forward declarations
bool test_ascii_art_generator() noexcept;
bool test_memory_utilities() noexcept;
bool test_hardware_types() noexcept;

bool test_ascii_art_generator() noexcept {
  ESP_LOGI(TAG, "Testing ASCII Art Generator...");
  
  AsciiArtGenerator art_gen;
  
  // Test basic generation
  std::string hardfoc_art = art_gen.Generate("HardFOC");
  if (hardfoc_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for HardFOC");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for HardFOC:\n%s", hardfoc_art.c_str());
  
  // Test with different text
  std::string esp32_art = art_gen.Generate("ESP32-C6");
  if (esp32_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for ESP32-C6");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated ASCII art for ESP32-C6:\n%s", esp32_art.c_str());
  
  // Test completion message
  std::string complete_art = art_gen.Generate("UTILS TESTS COMPLETE");
  if (complete_art.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for completion message");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Generated completion ASCII art:\n%s", complete_art.c_str());
  
  return true;
}

bool test_memory_utilities() noexcept {
  ESP_LOGI(TAG, "Testing memory utilities...");
  
  // Test make_unique_nothrow with int
  auto unique_int = hf::utils::make_unique_nothrow<int>(42);
  if (!unique_int) {
    ESP_LOGE(TAG, "Failed to create unique_ptr<int>");
    return false;
  }
  
  if (*unique_int != 42) {
    ESP_LOGE(TAG, "Unique int value mismatch: expected 42, got %d", *unique_int);
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] make_unique_nothrow created int with value: %d", *unique_int);
  
  // Test make_unique with array
  auto unique_array = std::make_unique<int[]>(10);
  if (!unique_array) {
    ESP_LOGE(TAG, "Failed to create unique_ptr<int[]>");
    return false;
  }
  
  // Initialize and test array
  for (int i = 0; i < 10; i++) {
    unique_array[i] = i * i;
  }
  
  // Verify values
  for (int i = 0; i < 10; i++) {
    if (unique_array[i] != i * i) {
      ESP_LOGE(TAG, "Array value mismatch at index %d: expected %d, got %d", i, i * i, unique_array[i]);
      return false;
    }
  }
  ESP_LOGI(TAG, "[SUCCESS] make_unique created array, element[5] = %d", unique_array[5]);
  
  return true;
}

bool test_hardware_types() noexcept {
  ESP_LOGI(TAG, "Testing hardware types...");
  
  // Test basic hardware type definitions
  hf_pin_num_t test_pin = 5;
  hf_port_num_t test_port = 0;
  hf_frequency_hz_t test_freq = 1000000;
  hf_timestamp_us_t test_timestamp = 12345678;
  uint32_t test_voltage = 3300;
  
  ESP_LOGI(TAG, "[SUCCESS] Pin: %d, Port: %d, Freq: %lu Hz", test_pin, test_port, test_freq);
  ESP_LOGI(TAG, "[SUCCESS] Timestamp: %llu us, Voltage: %d mV", test_timestamp, test_voltage);
  
  // Test GPIO state enum
  hf_gpio_state_t test_state = hf_gpio_state_t::HF_GPIO_STATE_ACTIVE;
  if (test_state != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    ESP_LOGE(TAG, "GPIO state test failed");
    return false;
  }
  
  ESP_LOGI(TAG, "[SUCCESS] GPIO state: %s",
           test_state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE ? "ACTIVE" : "INACTIVE");
  
  // Test state toggle
  test_state = hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
  ESP_LOGI(TAG, "[SUCCESS] GPIO state toggled: %s",
           test_state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE ? "ACTIVE" : "INACTIVE");
  
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                  ESP32-C6 UTILITIES COMPREHENSIVE TEST SUITE                ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  RUN_TEST(test_ascii_art_generator);
  RUN_TEST(test_memory_utilities);
  RUN_TEST(test_hardware_types);
  
  print_test_summary(g_test_results, "UTILITIES", TAG);
  
  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG, "[SUCCESS] ALL UTILITIES TESTS PASSED!");
  } else {
    ESP_LOGE(TAG, "[FAILED] Some tests failed.");
  }
  
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
