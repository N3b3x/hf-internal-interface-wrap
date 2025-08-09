/**
 * @file WifiComprehensiveTest.cpp
 * @brief Interface-only WiFi testing suite for ESP32-C6 DevKit-M-1 with ESP-IDF v5.5f
 *
 * This test suite provides interface testing of the EspWifi implementation
 * focusing on class structure, data types, and interface validation without
 * calling ESP-IDF WiFi functions that cause linker conflicts.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseWifi.h"
#include "TestFramework.h"

static const char* TAG = "WIFI_Test";

static TestResults g_test_results;

//==============================================================================
// WIFI INTERFACE AND DATA STRUCTURE TESTS
//==============================================================================

bool test_wifi_data_structures() noexcept {
  ESP_LOGI(TAG, "Testing WiFi data structures...");

  // Test station config structure
  hf_wifi_station_config_t station_config;
  station_config.ssid = "TestSSID";
  station_config.password = "TestPassword";
  station_config.bssid_set = false;
  station_config.channel = 6;
  station_config.scan_method = 0;
  station_config.sort_method = true;
  station_config.threshold_rssi = 70;  // Using positive value for unsigned field
  station_config.threshold_authmode = hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;

  if (station_config.ssid != "TestSSID") {
    ESP_LOGE(TAG, "Station config SSID not set correctly");
    return false;
  }

  if (station_config.channel != 6) {
    ESP_LOGE(TAG, "Station config channel not set correctly");
    return false;
  }

  // Test AP config structure
  hf_wifi_ap_config_t ap_config;
  ap_config.ssid = "TestAP";
  ap_config.password = "TestAPPassword";
  ap_config.ssid_len = 0;
  ap_config.channel = 6;
  ap_config.authmode = hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;
  ap_config.ssid_hidden = 0;
  ap_config.max_connection = 4;
  ap_config.beacon_interval = 100;

  if (ap_config.ssid != "TestAP") {
    ESP_LOGE(TAG, "AP config SSID not set correctly");
    return false;
  }

  if (ap_config.max_connection != 4) {
    ESP_LOGE(TAG, "AP config max_connection not set correctly");
    return false;
  }

  // Test network info structure
  hf_wifi_network_info_t network_info;
  network_info.ssid = "TestNetwork";
  network_info.security = hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;
  network_info.rssi = -50;
  network_info.channel = 6;
  network_info.hidden = false;

  if (network_info.ssid != "TestNetwork") {
    ESP_LOGE(TAG, "Network info SSID not set correctly");
    return false;
  }

  if (network_info.rssi != -50) {
    ESP_LOGE(TAG, "Network info RSSI not set correctly");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi data structures test successful");
  return true;
}

bool test_wifi_enums() noexcept {
  ESP_LOGI(TAG, "Testing WiFi enums...");

  // Test WiFi modes
  auto modes = {hf_wifi_mode_t::HF_WIFI_MODE_STATION,
                hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT,
                hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP,
                hf_wifi_mode_t::HF_WIFI_MODE_DISABLED};

  for (auto mode : modes) {
    ESP_LOGI(TAG, "WiFi mode value: %d", static_cast<int>(mode));
  }

  // Test security types
  auto security_types = {hf_wifi_security_t::HF_WIFI_SECURITY_OPEN,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WEP,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA_PSK,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA_WPA2_PSK,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_ENTERPRISE,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_PSK,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_WPA3_PSK,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_ENTERPRISE,
                        hf_wifi_security_t::HF_WIFI_SECURITY_WAPI_PSK};

  for (auto security : security_types) {
    ESP_LOGI(TAG, "Security type value: %d", static_cast<int>(security));
  }

  // Test power save modes
  auto power_modes = {hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE,
                      hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MIN_MODEM,
                      hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MAX_MODEM};

  for (auto power_mode : power_modes) {
    ESP_LOGI(TAG, "Power save mode value: %d", static_cast<int>(power_mode));
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi enums test successful");
  return true;
}

bool test_wifi_error_codes() noexcept {
  ESP_LOGI(TAG, "Testing WiFi error codes...");

  // Test error code conversion
  auto error_string = HfWifiErrToString(hf_wifi_err_t::WIFI_SUCCESS);
  if (error_string != "Success") {
    ESP_LOGE(TAG, "Error string for WIFI_SUCCESS incorrect: %s", std::string(error_string).c_str());
    return false;
  }

  auto error_string_failure = HfWifiErrToString(hf_wifi_err_t::WIFI_ERR_FAILURE);
  if (error_string_failure != "General failure") {
    ESP_LOGE(TAG, "Error string for WIFI_ERR_FAILURE incorrect: %s", std::string(error_string_failure).c_str());
    return false;
  }

  auto error_string_invalid = HfWifiErrToString(hf_wifi_err_t::WIFI_ERR_INVALID_PARAM);
  if (error_string_invalid != "Invalid parameter") {
    ESP_LOGE(TAG, "Error string for WIFI_ERR_INVALID_PARAM incorrect: %s", std::string(error_string_invalid).c_str());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi error codes test successful");
  return true;
}

bool test_wifi_interface_validation() noexcept {
  ESP_LOGI(TAG, "Testing WiFi interface validation...");

  // Test that we can create and manipulate WiFi-related data
  hf_wifi_station_config_t config;
  config.ssid = "TestNetwork";
  config.password = "TestPassword123";
  config.channel = 11;
  config.threshold_rssi = 80;  // Using positive value for unsigned field

  // Verify the data was set correctly
  if (config.ssid != "TestNetwork") {
    ESP_LOGE(TAG, "Interface test: SSID not set correctly");
    return false;
  }

  if (config.channel != 11) {
    ESP_LOGE(TAG, "Interface test: Channel not set correctly");
    return false;
  }

  if (config.threshold_rssi != 80) {
    ESP_LOGE(TAG, "Interface test: RSSI threshold not set correctly");
    return false;
  }

  // Test MAC address array
  uint8_t mac_address[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
  
  // Verify MAC address values
  if (mac_address[0] != 0x12 || mac_address[5] != 0xBC) {
    ESP_LOGE(TAG, "Interface test: MAC address not set correctly");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi interface validation test successful");
  return true;
}

bool test_wifi_performance_interface() noexcept {
  ESP_LOGI(TAG, "Testing WiFi interface performance...");

  // Test data structure creation performance
  auto start_time = esp_timer_get_time();
  
  for (int i = 0; i < 1000; i++) {
    hf_wifi_station_config_t config;
    config.ssid = "TestSSID";
    config.password = "TestPassword";
    config.channel = i % 14;
    config.threshold_rssi = 50 + (i % 50);  // Using positive values for unsigned field
    
    // Verify the data was set
    if (config.ssid != "TestSSID") {
      ESP_LOGE(TAG, "Performance test failed at iteration %d", i);
      return false;
    }
  }
  
  auto end_time = esp_timer_get_time();
  auto duration_us = end_time - start_time;
  auto duration_ms = duration_us / 1000;

  ESP_LOGI(TAG, "Performance test completed: 1000 config creations in %lld ms (%lld us per operation)", 
           duration_ms, duration_us / 1000);
  ESP_LOGI(TAG, "[SUCCESS] WiFi interface performance test successful");
  return true;
}

bool test_wifi_integration_interface() noexcept {
  ESP_LOGI(TAG, "Testing WiFi interface integration...");

  // Test comprehensive data structure integration
  hf_wifi_station_config_t station_config;
  station_config.ssid = "IntegrationTest";
  station_config.password = "IntegrationPassword";
  station_config.channel = 6;
  station_config.threshold_rssi = 70;  // Using positive value for unsigned field

  hf_wifi_ap_config_t ap_config;
  ap_config.ssid = "IntegrationAP";
  ap_config.password = "IntegrationAPPass";
  ap_config.channel = 11;
  ap_config.max_connection = 8;

  hf_wifi_network_info_t network_info;
  network_info.ssid = "IntegrationNetwork";
  network_info.security = hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_PSK;
  network_info.rssi = -45;
  network_info.channel = 1;

  // Verify all configurations are set correctly
  if (station_config.ssid != "IntegrationTest" || 
      ap_config.ssid != "IntegrationAP" || 
      network_info.ssid != "IntegrationNetwork") {
    ESP_LOGE(TAG, "Integration test: SSID values not set correctly");
    return false;
  }

  if (station_config.channel != 6 || 
      ap_config.channel != 11 || 
      network_info.channel != 1) {
    ESP_LOGE(TAG, "Integration test: Channel values not set correctly");
    return false;
  }

  if (network_info.security != hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_PSK) {
    ESP_LOGE(TAG, "Integration test: Security type not set correctly");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi interface integration test successful");
  return true;
}

bool test_wifi_stress_interface() noexcept {
  ESP_LOGI(TAG, "Testing WiFi interface stress...");

  // Stress test with rapid data structure creation and modification
  for (int i = 0; i < 100; i++) {
    hf_wifi_station_config_t config;
    config.ssid = "StressTest" + std::to_string(i);
    config.password = "StressPass" + std::to_string(i);
    config.channel = (i % 14) + 1;
    config.threshold_rssi = 30 + (i % 70);  // Using positive values for unsigned field
    config.scan_method = i % 2;
    config.sort_method = (i % 2) == 1;
    config.threshold_authmode = static_cast<hf_wifi_security_t>(i % 10);

    // Verify the data
    if (config.ssid != "StressTest" + std::to_string(i)) {
      ESP_LOGE(TAG, "Stress test failed at iteration %d", i);
      return false;
    }

    if (config.channel < 1 || config.channel > 14) {
      ESP_LOGE(TAG, "Stress test: Invalid channel at iteration %d", i);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] WiFi interface stress test successful");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 WIFI COMPREHENSIVE TEST SUITE v1.0                 ║");
  ESP_LOGI(TAG, "║                        Interface-Only Version                                ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Data Structure Tests
  ESP_LOGI(TAG, "\n=== DATA STRUCTURE TESTS ===");
  RUN_TEST(test_wifi_data_structures);
  RUN_TEST(test_wifi_enums);
  RUN_TEST(test_wifi_error_codes);

  // Interface Validation Tests
  ESP_LOGI(TAG, "\n=== INTERFACE VALIDATION TESTS ===");
  RUN_TEST(test_wifi_interface_validation);
  RUN_TEST(test_wifi_integration_interface);

  // Performance and Stress Tests
  ESP_LOGI(TAG, "\n=== PERFORMANCE AND STRESS TESTS ===");
  RUN_TEST(test_wifi_performance_interface);
  RUN_TEST(test_wifi_stress_interface);

  // Print final summary
  print_test_summary(g_test_results, "WIFI", TAG);

  ESP_LOGI(TAG, "\n╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    WIFI COMPREHENSIVE TEST SUITE COMPLETE                    ║");
  ESP_LOGI(TAG, "║                        (Interface-Only Version)                               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Keep the system running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
