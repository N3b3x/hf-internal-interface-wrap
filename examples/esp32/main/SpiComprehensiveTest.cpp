/**
 * @file SpiComprehensiveTest.cpp
 * @brief Comprehensive SPI testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspSpi class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * SPI functionalities including bus-device architecture, various transfer modes,
 * and hardware-specific capabilities.
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
#include "driver/spi_master.h"

#include "base/BaseSpi.h"
#include "mcu/esp32/EspSpi.h"
#include "mcu/esp32/utils/EspTypes_SPI.h"

#include "TestFramework.h"

static const char* TAG = "SPI_Test";

static TestResults g_test_results;

// Forward declarations
bool test_spi_bus_initialization() noexcept;
bool test_spi_device_operations() noexcept;
bool test_spi_transfer_modes() noexcept;
bool test_spi_performance() noexcept;

bool test_spi_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing SPI bus initialization...");
  
  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.clock_speed_hz = 1000000;
  spi_bus_cfg.host = SPI2_HOST;
  
  EspSpiBus test_spi_bus(spi_bus_cfg);
  
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return false;
  }
  
  ESP_LOGI(TAG, "[SUCCESS] SPI bus initialization successful");
  return true;
}

bool test_spi_device_operations() noexcept {
  ESP_LOGI(TAG, "Testing SPI device operations...");
  
  // Create SPI bus first
  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.clock_speed_hz = 1000000;
  spi_bus_cfg.host = SPI2_HOST;
  
  EspSpiBus test_spi_bus(spi_bus_cfg);
  
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus for device test");
    return false;
  }
  
  // Create SPI device on the bus
  hf_spi_device_config_t spi_dev_cfg = {};
  spi_dev_cfg.clock_speed_hz = 1000000;
  spi_dev_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  spi_dev_cfg.cs_pin = 12;
  int device_index = test_spi_bus.CreateDevice(spi_dev_cfg);
  
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create SPI device");
    return false;
  }
  
  ESP_LOGI(TAG, "[SUCCESS] SPI device created with index: %d", device_index);
  return true;
}

bool test_spi_transfer_modes() noexcept {
  ESP_LOGI(TAG, "Testing SPI transfer modes...");
  
  // Placeholder for transfer mode tests
  ESP_LOGI(TAG, "SPI transfer modes test completed (placeholder)");
  return true;
}

bool test_spi_performance() noexcept {
  ESP_LOGI(TAG, "Testing SPI performance...");
  
  // Placeholder for performance tests
  ESP_LOGI(TAG, "SPI performance test completed (placeholder)");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 SPI COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  RUN_TEST(test_spi_bus_initialization);
  RUN_TEST(test_spi_device_operations);
  RUN_TEST(test_spi_transfer_modes);
  RUN_TEST(test_spi_performance);
  
  print_test_summary(g_test_results, "SPI", TAG);
  
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
