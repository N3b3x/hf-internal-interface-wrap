/**
 * @file WifiComprehensiveTest.cpp
 * @brief Comprehensive WiFi testing suite for ESP32-C6 DevKit-M-1 with ESP-IDF v5.5f
 *
 * This test suite provides both interface testing and functional testing of the EspWifi implementation:
 * 
 * INTERFACE TESTS (Default - ENABLED):
 * - Class structure, data types, and interface validation
 * - No actual WiFi hardware operations
 * - Safe for CI/CD and development environments
 * 
 * FUNCTIONAL TESTS (Default - ENABLED):
 * - Real WiFi hardware operations using EspWifi library
 * - Access Point creation (visible on phones/computers)
 * - Network scanning and discovery
 * - Station mode connection attempts
 * - Power management and advanced features
 * - Event handling and callbacks
 * 
 * To enable functional tests, set ENABLE_FUNCTIONAL_TESTS = true
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BaseWifi.h"
#include "mcu/esp32/EspWifi.h"

static const char* TAG = "WIFI_Test";

static TestResults g_test_results;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core WiFi interface tests
static constexpr bool ENABLE_CORE_TESTS = true;         // Data structures, enums, error codes
static constexpr bool ENABLE_INTERFACE_TESTS = true;    // Interface validation, integration
static constexpr bool ENABLE_PERFORMANCE_TESTS = true;  // Performance, stress testing
static constexpr bool ENABLE_FUNCTIONAL_TESTS = true;   // Real WiFi functionality tests

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
  station_config.threshold_rssi = 70; // Using positive value for unsigned field
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
  auto modes = {hf_wifi_mode_t::HF_WIFI_MODE_STATION, hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT,
                hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP, hf_wifi_mode_t::HF_WIFI_MODE_DISABLED};

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
    ESP_LOGE(TAG, "Error string for WIFI_ERR_FAILURE incorrect: %s",
             std::string(error_string_failure).c_str());
    return false;
  }

  auto error_string_invalid = HfWifiErrToString(hf_wifi_err_t::WIFI_ERR_INVALID_PARAM);
  if (error_string_invalid != "Invalid parameter") {
    ESP_LOGE(TAG, "Error string for WIFI_ERR_INVALID_PARAM incorrect: %s",
             std::string(error_string_invalid).c_str());
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
  config.threshold_rssi = 80; // Using positive value for unsigned field

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
    config.threshold_rssi = 50 + (i % 50); // Using positive values for unsigned field

    // Verify the data was set
    if (config.ssid != "TestSSID") {
      ESP_LOGE(TAG, "Performance test failed at iteration %d", i);
      return false;
    }
  }

  auto end_time = esp_timer_get_time();
  auto duration_us = end_time - start_time;
  auto duration_ms = duration_us / 1000;

  ESP_LOGI(TAG,
           "Performance test completed: 1000 config creations in %lld ms (%lld us per operation)",
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
  station_config.threshold_rssi = 70; // Using positive value for unsigned field

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
  if (station_config.ssid != "IntegrationTest" || ap_config.ssid != "IntegrationAP" ||
      network_info.ssid != "IntegrationNetwork") {
    ESP_LOGE(TAG, "Integration test: SSID values not set correctly");
    return false;
  }

  if (station_config.channel != 6 || ap_config.channel != 11 || network_info.channel != 1) {
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
    config.threshold_rssi = 30 + (i % 70); // Using positive values for unsigned field
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
// FUNCTIONAL WIFI TESTS (ACTUAL WIFI OPERATIONS)
//==============================================================================

bool test_wifi_initialization() noexcept {
  ESP_LOGI(TAG, "Testing WiFi initialization and mode switching...");

  // Create WiFi instance
  EspWifi wifi;
  
  // Test initialization
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  if (!wifi.IsInitialized()) {
    ESP_LOGE(TAG, "WiFi not marked as initialized");
    return false;
  }

  // Test mode switching
  err = wifi.SetMode(hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set AP mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  hf_wifi_mode_t current_mode = wifi.GetMode();
  if (current_mode != hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT) {
    ESP_LOGE(TAG, "Mode not set correctly, expected AP mode");
    return false;
  }

  // Test deinitialization
  err = wifi.Deinitialize();
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to deinitialize WiFi: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Add delay to ensure proper cleanup
  vTaskDelay(pdMS_TO_TICKS(500));

  ESP_LOGI(TAG, "[SUCCESS] WiFi initialization test successful");
  return true;
}

bool test_wifi_access_point_creation() noexcept {
  ESP_LOGI(TAG, "Testing Access Point creation and management...");

  EspWifi wifi;
  
  // Initialize in AP mode
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi in AP mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Configure Access Point
  hf_wifi_ap_config_t ap_config;
  ap_config.ssid = "ESP32-C6_TestAP";
  ap_config.password = "testpassword123";
  ap_config.ssid_len = 0; // Auto-detect length
  ap_config.channel = 6;
  ap_config.authmode = hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;
  ap_config.ssid_hidden = 0; // Visible
  ap_config.max_connection = 4;
  ap_config.beacon_interval = 100;

  err = wifi.ConfigureAccessPoint(ap_config);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure AP: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Start Access Point
  err = wifi.StartAccessPoint();
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start AP: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Wait a moment for AP to start
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Check if AP is active
  if (!wifi.IsAccessPointActive()) {
    ESP_LOGE(TAG, "Access Point not marked as active");
    return false;
  }

  // Get connected station count (should be 0 initially)
  int station_count = wifi.GetConnectedStationCount();
  ESP_LOGI(TAG, "Connected stations: %d", station_count);

  // Test MAC address retrieval
  uint8_t ap_mac[6];
  err = wifi.GetMacAddress(ap_mac, 1); // Interface 1 for AP
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get AP MAC address: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "AP MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", 
           ap_mac[0], ap_mac[1], ap_mac[2], ap_mac[3], ap_mac[4], ap_mac[5]);
  
  // Keep AP running for 15 seconds so you can see it on your phone
  ESP_LOGI(TAG, "Access Point 'ESP32-C6_TestAP' is now running for 15 seconds...");
  ESP_LOGI(TAG, "Refresh your phone's WiFi list to see the network!");
  
  // Use a shorter delay to avoid test timeout
  vTaskDelay(pdMS_TO_TICKS(15000)); // 15 seconds

  // Check connected station count after 15 seconds
  int final_station_count = wifi.GetConnectedStationCount();
  ESP_LOGI(TAG, "Connected stations after 15 seconds: %d", final_station_count);

  // Stop Access Point
  err = wifi.StopAccessPoint();
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to stop AP: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Access Point creation test successful");
  ESP_LOGI(TAG, "NOTE: Look for 'ESP32-C6_TestAP' network on your phone/computer");
  return true;
}

bool test_wifi_network_scanning() noexcept {
  ESP_LOGI(TAG, "Testing network scanning functionality...");

  EspWifi wifi;
  
  // Initialize in station mode
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi in station mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Wait a moment for WiFi to fully initialize
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Start network scan
  err = wifi.StartScan(true, false, 5000); // Show hidden, active scan, 5s timeout
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start scan: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Wait for scan to complete
  int scan_wait_count = 0;
  while (wifi.IsScanning() && scan_wait_count < 20) {
    vTaskDelay(pdMS_TO_TICKS(500));
    scan_wait_count++;
    ESP_LOGI(TAG, "Waiting for scan to complete... (%d/20)", scan_wait_count);
  }

  if (wifi.IsScanning()) {
    ESP_LOGE(TAG, "Scan did not complete within timeout");
    return false;
  }

  // Get scan results
  std::vector<hf_wifi_network_info_t> networks;
  err = wifi.GetScanResults(networks, 20); // Limit to 20 networks
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get scan results: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "Found %zu networks:", networks.size());
  for (size_t i = 0; i < networks.size() && i < 10; i++) { // Show first 10
    const auto& network = networks[i];
    ESP_LOGI(TAG, "  %zu. SSID: '%s', RSSI: %d dBm, Channel: %d, Security: %d", 
             i + 1, network.ssid.c_str(), network.rssi, network.channel, 
             static_cast<int>(network.security));
  }

  if (networks.size() > 10) {
    ESP_LOGI(TAG, "  ... and %zu more networks", networks.size() - 10);
  }

  ESP_LOGI(TAG, "[SUCCESS] Network scanning test successful");
  return true;
}

bool test_wifi_station_connection() noexcept {
  ESP_LOGI(TAG, "Testing station mode connection (will attempt to connect to test network)...");

  EspWifi wifi;
  
  // Initialize in station mode
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi in station mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Configure station (using a test network that may not exist)
  hf_wifi_station_config_t sta_config;
  sta_config.ssid = "TestNetwork_ESP32";
  sta_config.password = "testpassword123";
  sta_config.bssid_set = false;
  sta_config.channel = 0; // Any channel
  sta_config.scan_method = 0; // Fast scan
  sta_config.sort_method = true; // Sort by signal strength
  sta_config.threshold_rssi = 70;
  sta_config.threshold_authmode = hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;

  err = wifi.ConfigureStation(sta_config);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure station: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Attempt to connect (will likely fail, but tests the functionality)
  ESP_LOGI(TAG, "Attempting to connect to '%s'...", sta_config.ssid.c_str());
  err = wifi.Connect(10000); // 10 second timeout
  
  if (err == hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGI(TAG, "Successfully connected to network!");
    
    // Get connection info
    std::string connected_ssid = wifi.GetConnectedSsid();
    int8_t rssi = wifi.GetRssi();
    uint8_t channel = wifi.GetChannel();
    
    ESP_LOGI(TAG, "Connected to: %s", connected_ssid.c_str());
    ESP_LOGI(TAG, "RSSI: %d dBm", rssi);
    ESP_LOGI(TAG, "Channel: %d", channel);
    
    // Get IP info
    hf_wifi_ip_info_t ip_info;
    err = wifi.GetIpInfo(ip_info);
    if (err == hf_wifi_err_t::WIFI_SUCCESS) {
      ESP_LOGI(TAG, "IP: %d.%d.%d.%d", 
               (ip_info.ip >> 0) & 0xFF, (ip_info.ip >> 8) & 0xFF,
               (ip_info.ip >> 16) & 0xFF, (ip_info.ip >> 24) & 0xFF);
    }
    
    // Disconnect
    err = wifi.Disconnect();
    if (err != hf_wifi_err_t::WIFI_SUCCESS) {
      ESP_LOGE(TAG, "Failed to disconnect: %s", std::string(HfWifiErrToString(err)).c_str());
    }
  } else {
    ESP_LOGI(TAG, "Connection failed (expected): %s", std::string(HfWifiErrToString(err)).c_str());
    ESP_LOGI(TAG, "This is normal if the test network doesn't exist");
  }

  ESP_LOGI(TAG, "[SUCCESS] Station connection test completed");
  return true;
}

bool test_wifi_power_management() noexcept {
  ESP_LOGI(TAG, "Testing WiFi power management features...");

  EspWifi wifi;
  
  // Initialize WiFi
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Test power save mode setting
  err = wifi.SetPowerSave(hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MIN_MODEM);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set power save mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Verify power save mode
  hf_wifi_power_save_t current_power_save = wifi.GetPowerSave();
  if (current_power_save != hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MIN_MODEM) {
    ESP_LOGE(TAG, "Power save mode not set correctly");
    return false;
  }

  ESP_LOGI(TAG, "Power save mode set to: %d", static_cast<int>(current_power_save));

  // Test channel setting
  err = wifi.SetChannel(6);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set channel: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  uint8_t current_channel = wifi.GetChannel();
  ESP_LOGI(TAG, "Current channel: %d", current_channel);

  // Test MAC address operations
  uint8_t current_mac[6];
  err = wifi.GetMacAddress(current_mac, 0); // Station interface
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get MAC address: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "Station MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
           current_mac[0], current_mac[1], current_mac[2], 
           current_mac[3], current_mac[4], current_mac[5]);

  ESP_LOGI(TAG, "[SUCCESS] Power management test successful");
  return true;
}

bool test_wifi_advanced_features() noexcept {
  ESP_LOGI(TAG, "Testing WiFi advanced features...");

  EspWifi wifi;
  
  // Initialize with advanced configuration
  EspWifiAdvancedConfig advanced_config;
  advanced_config.enable_power_save = true;
  advanced_config.power_save_type = WIFI_PS_MIN_MODEM;
  advanced_config.tx_power = 15; // 15 dBm
  advanced_config.bandwidth = WIFI_BW_HT20;
  advanced_config.enable_ampdu_rx = true;
  advanced_config.enable_ampdu_tx = true;
  advanced_config.enable_fast_connect = false;
  advanced_config.enable_pmf_required = false;
  advanced_config.enable_wpa3_transition = true;
  advanced_config.enable_11r = false;
  advanced_config.enable_11k = false;
  advanced_config.enable_11v = false;
  advanced_config.enable_enterprise = false;
  advanced_config.enable_mesh = false;
  advanced_config.enable_smartconfig = false;

  // Create WiFi with advanced config
  EspWifi advanced_wifi(&advanced_config);
  
  hf_wifi_err_t err = advanced_wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize advanced WiFi: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Test advanced configuration retrieval
  EspWifiAdvancedConfig retrieved_config;
  err = advanced_wifi.GetAdvancedConfig(retrieved_config);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get advanced config: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "Advanced config - TX Power: %d dBm", retrieved_config.tx_power);
  ESP_LOGI(TAG, "Advanced config - Power Save: %s", retrieved_config.enable_power_save ? "Enabled" : "Disabled");
  ESP_LOGI(TAG, "Advanced config - A-MPDU RX: %s", retrieved_config.enable_ampdu_rx ? "Enabled" : "Disabled");
  ESP_LOGI(TAG, "Advanced config - A-MPDU TX: %s", retrieved_config.enable_ampdu_tx ? "Enabled" : "Disabled");

  // Test TX power setting
  err = advanced_wifi.SetTxPower(18); // 18 dBm
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set TX power: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  int8_t current_tx_power = advanced_wifi.GetTxPower();
  ESP_LOGI(TAG, "Current TX power: %d dBm", current_tx_power);

  // Test bandwidth setting
  err = advanced_wifi.SetBandwidth(WIFI_BW_HT40);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set bandwidth: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  wifi_bandwidth_t current_bandwidth = advanced_wifi.GetBandwidth();
  ESP_LOGI(TAG, "Current bandwidth: %d", static_cast<int>(current_bandwidth));

  ESP_LOGI(TAG, "[SUCCESS] Advanced features test successful");
  return true;
}

bool test_wifi_event_handling() noexcept {
  ESP_LOGI(TAG, "Testing WiFi event handling...");

  EspWifi wifi;
  
  // Initialize WiFi
  hf_wifi_err_t err = wifi.Initialize(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Register event callback
  bool event_received = false;
  hf_wifi_event_callback_t callback = [&event_received](hf_wifi_event_t event, void* event_data) {
    ESP_LOGI(TAG, "WiFi event received: %d", static_cast<int>(event));
    event_received = true;
  };

  err = wifi.RegisterEventCallback(callback);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to register event callback: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Trigger some events by changing modes
  err = wifi.SetMode(hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT);
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set AP mode: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  // Wait a moment for events
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Unregister callback
  err = wifi.UnregisterEventCallback();
  if (err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to unregister event callback: %s", std::string(HfWifiErrToString(err)).c_str());
    return false;
  }

  ESP_LOGI(TAG, "Event handling test completed (events may have been received)");
  ESP_LOGI(TAG, "[SUCCESS] Event handling test successful");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 WIFI COMPREHENSIVE TEST SUITE                        ║");
  ESP_LOGI(TAG, "║                       HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                  ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                               ║");
  ESP_LOGI(TAG, "║ Features: WiFi, Access Point, Network Scanning, Station Connection, Power    ║");
  ESP_LOGI(TAG, "║ Management, Advanced Features, Event Handling, Functional Tests              ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "WIFI");

  // Run all WiFi tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CORE_TESTS, "WIFI CORE TESTS",
      // Data Structure Tests
      ESP_LOGI(TAG, "Running WiFi data structure tests...");
      RUN_TEST_IN_TASK("data_structures", test_wifi_data_structures, 8192, 1);
      RUN_TEST_IN_TASK("enums", test_wifi_enums, 8192, 1);
      RUN_TEST_IN_TASK("error_codes", test_wifi_error_codes, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_INTERFACE_TESTS, "WIFI INTERFACE TESTS",
      // Interface Validation Tests
      ESP_LOGI(TAG, "Running WiFi interface validation tests...");
      RUN_TEST_IN_TASK("interface_validation", test_wifi_interface_validation, 8192, 1);
      RUN_TEST_IN_TASK("integration_interface", test_wifi_integration_interface, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_PERFORMANCE_TESTS, "WIFI PERFORMANCE TESTS",
      // Performance and Stress Tests
      ESP_LOGI(TAG, "Running WiFi performance and stress tests...");
      RUN_TEST_IN_TASK("performance_interface", test_wifi_performance_interface, 8192, 1);
      RUN_TEST_IN_TASK("stress_interface", test_wifi_stress_interface, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_FUNCTIONAL_TESTS, "WIFI FUNCTIONAL TESTS",
      // Functional WiFi Tests (ACTUAL WIFI OPERATIONS)
      ESP_LOGI(TAG, "Running WiFi functional tests (REAL WIFI OPERATIONS)...");
      ESP_LOGI(TAG, "WARNING: These tests will use actual WiFi hardware!");
      RUN_TEST_IN_TASK("wifi_initialization", test_wifi_initialization, 16384, 1);
      RUN_TEST_IN_TASK("wifi_access_point", test_wifi_access_point_creation, 16384, 1);
      RUN_TEST_IN_TASK("wifi_network_scanning", test_wifi_network_scanning, 16384, 1);
      RUN_TEST_IN_TASK("wifi_station_connection", test_wifi_station_connection, 16384, 1);
      RUN_TEST_IN_TASK("wifi_power_management", test_wifi_power_management, 16384, 1);
      RUN_TEST_IN_TASK("wifi_advanced_features", test_wifi_advanced_features, 16384, 1);
      RUN_TEST_IN_TASK("wifi_event_handling", test_wifi_event_handling, 16384, 1););

  // Print final summary
  print_test_summary(g_test_results, "WIFI", TAG);

  ESP_LOGI(TAG,"\n");
  ESP_LOGI(TAG,
           "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, 
           "║                   WIFI COMPREHENSIVE TEST SUITE COMPLETE                     ║");
  ESP_LOGI(TAG,
           "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, 
           "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  // Keep the system running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
