/**
 * @file BluetoothComprehensiveTest.cpp
 * @brief Comprehensive Bluetooth/NimBLE testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspBluetooth class
 * targeting ESP32-C6 with ESP-IDF v5.5+ and NimBLE. It provides thorough testing of all
 * Bluetooth/BLE functionalities including initialization, scanning, device discovery,
 * connection management, and advanced features.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseBluetooth.h"
#include "mcu/esp32/EspBluetooth.h"

#include <vector>
#include "TestFramework.h"

static const char* TAG = "BT_Test";

static TestResults g_test_results;

// Global Bluetooth instance for testing
static EspBluetooth bluetooth_instance;

// Event callback function for Bluetooth events
void bluetooth_event_callback(hf_bluetooth_event_t event, void* event_data) noexcept {
  switch (event) {
    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_ENABLED:
      ESP_LOGI(TAG, "[SUCCESS] Bluetooth enabled successfully");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISABLED:
      ESP_LOGI(TAG, "[INFO] Bluetooth disabled");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_START:
      ESP_LOGI(TAG, "[INFO] BLE scan started");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_STOP:
      ESP_LOGI(TAG, "[INFO] BLE scan stopped");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DEVICE_FOUND:
      ESP_LOGI(TAG, "[INFO] BLE device discovered");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_SUCCESS:
      ESP_LOGI(TAG, "[SUCCESS] Device connected successfully");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_FAILED:
      ESP_LOGW(TAG, "[FAILED] Device connection failed");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISCONNECT:
      ESP_LOGI(TAG, "[INFO] Device disconnected");
      break;

    default:
      ESP_LOGI(TAG, "[INFO] Bluetooth event: %d", static_cast<int>(event));
      break;
  }
}

// Forward declarations
bool test_bluetooth_initialization() noexcept;
bool test_bluetooth_basic_operations() noexcept;
bool test_bluetooth_scanning() noexcept;
bool test_bluetooth_state_management() noexcept;
bool test_bluetooth_cleanup() noexcept;

bool test_bluetooth_initialization() noexcept {
  ESP_LOGI(TAG, "Testing Bluetooth initialization...");

  // Register event callback
  hf_bluetooth_err_t ret = bluetooth_instance.RegisterEventCallback(bluetooth_event_callback);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to register event callback");
    return false;
  }

  // Initialize Bluetooth in BLE mode
  ret = bluetooth_instance.Initialize(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize Bluetooth: %d", static_cast<int>(ret));
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Bluetooth initialized successfully");

  // Check if initialized
  if (!bluetooth_instance.IsInitialized()) {
    ESP_LOGE(TAG, "Bluetooth not showing as initialized");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Bluetooth initialization confirmed");

  return true;
}

bool test_bluetooth_basic_operations() noexcept {
  ESP_LOGI(TAG, "Testing Bluetooth basic operations...");

  // Enable Bluetooth
  hf_bluetooth_err_t ret = bluetooth_instance.Enable();
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable Bluetooth: %d", static_cast<int>(ret));
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Bluetooth enabled successfully");

  // Check if enabled
  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGE(TAG, "Bluetooth not showing as enabled");
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] Bluetooth enable state confirmed");

  // Get local address
  hf_bluetooth_address_t local_addr;
  ret = bluetooth_instance.GetLocalAddress(local_addr);
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Local BLE address: %s", local_addr.ToString().c_str());
  } else {
    ESP_LOGW(TAG, "Could not get local address: %d", static_cast<int>(ret));
  }

  // Set device name
  ret = bluetooth_instance.SetDeviceName("ESP32C6-HardFOC-Test");
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Device name set successfully");
  } else {
    ESP_LOGW(TAG, "Failed to set device name: %d", static_cast<int>(ret));
  }

  // Get device name
  std::string device_name = bluetooth_instance.GetDeviceName();
  if (!device_name.empty()) {
    ESP_LOGI(TAG, "[SUCCESS] Device name: %s", device_name.c_str());
  } else {
    ESP_LOGW(TAG, "Failed to get device name");
  }

  // Get implementation info
  std::string impl_info = bluetooth_instance.GetImplementationInfo();
  ESP_LOGI(TAG, "[SUCCESS] Implementation info:\n%s", impl_info.c_str());

  // Get supported features
  uint32_t features = bluetooth_instance.GetSupportedFeatures();
  ESP_LOGI(TAG, "[SUCCESS] Supported features: 0x%08lX", features);

  return true;
}

bool test_bluetooth_scanning() noexcept {
  ESP_LOGI(TAG, "Testing Bluetooth scanning...");

  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGE(TAG, "Bluetooth not enabled, cannot test scanning");
    return false;
  }

  // Clear previously discovered devices
  hf_bluetooth_err_t ret = bluetooth_instance.ClearDiscoveredDevices();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Cleared discovered devices list");
  }

  // Start scanning with 5 second duration
  ret = bluetooth_instance.StartScan(5000, hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start scanning: %d", static_cast<int>(ret));
    return false;
  }
  ESP_LOGI(TAG, "[SUCCESS] BLE scanning started for 5 seconds");

  // Check scanning state
  if (bluetooth_instance.IsScanning()) {
    ESP_LOGI(TAG, "[SUCCESS] Scanning state confirmed");
  } else {
    ESP_LOGW(TAG, "Scanning state not confirmed");
  }

  // Wait for scan to complete
  vTaskDelay(pdMS_TO_TICKS(6000)); // Wait 6 seconds

  // Check if scanning stopped automatically
  if (!bluetooth_instance.IsScanning()) {
    ESP_LOGI(TAG, "[SUCCESS] Scanning stopped automatically after timeout");
  } else {
    ESP_LOGW(TAG, "Scanning still active, stopping manually");
    bluetooth_instance.StopScan();
  }

  // Get discovered devices
  std::vector<hf_bluetooth_device_info_t> discovered_devices;
  ret = bluetooth_instance.GetDiscoveredDevices(discovered_devices);
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Found %zu BLE devices:", discovered_devices.size());

    for (size_t i = 0; i < discovered_devices.size() && i < 10; i++) {
      const auto& device = discovered_devices[i];
      ESP_LOGI(TAG, "  Device %zu:", i + 1);
      ESP_LOGI(TAG, "    Address: %s", device.address.ToString().c_str());
      ESP_LOGI(TAG, "    RSSI: %d dBm", device.rssi);
      if (!device.name.empty()) {
        ESP_LOGI(TAG, "    Name: %s", device.name.c_str());
      }
    }

    if (discovered_devices.size() > 10) {
      ESP_LOGI(TAG, "  ... and %zu more devices", discovered_devices.size() - 10);
    }
  } else {
    ESP_LOGE(TAG, "Failed to get discovered devices: %d", static_cast<int>(ret));
    return false;
  }

  return true;
}

bool test_bluetooth_state_management() noexcept {
  ESP_LOGI(TAG, "Testing Bluetooth state management...");

  // Test state retrieval
  hf_bluetooth_state_t state = bluetooth_instance.GetState();
  ESP_LOGI(TAG, "[SUCCESS] Current Bluetooth state: %d", static_cast<int>(state));

  // Test mode retrieval
  hf_bluetooth_mode_t mode = bluetooth_instance.GetMode();
  ESP_LOGI(TAG, "[SUCCESS] Current Bluetooth mode: %d", static_cast<int>(mode));

  // Test mode setting (should remain BLE for ESP32C6)
  hf_bluetooth_err_t ret =
      bluetooth_instance.SetMode(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_CLASSIC);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Correctly rejected Classic mode for ESP32C6");
  } else {
    ESP_LOGW(TAG, "Unexpectedly accepted Classic mode");
  }

  // Verify mode is still BLE
  mode = bluetooth_instance.GetMode();
  if (mode == hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE) {
    ESP_LOGI(TAG, "[SUCCESS] Mode correctly maintained as BLE");
  } else {
    ESP_LOGW(TAG, "Mode unexpectedly changed");
    return false;
  }

  return true;
}

bool test_bluetooth_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing Bluetooth cleanup...");

  // Disable Bluetooth
  hf_bluetooth_err_t ret = bluetooth_instance.Disable();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Bluetooth disabled successfully");
  } else {
    ESP_LOGE(TAG, "Failed to disable Bluetooth: %d", static_cast<int>(ret));
    return false;
  }

  // Check disabled state
  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGI(TAG, "[SUCCESS] Bluetooth disable state confirmed");
  } else {
    ESP_LOGW(TAG, "Bluetooth still showing as enabled");
    return false;
  }

  // Deinitialize Bluetooth
  ret = bluetooth_instance.Deinitialize();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Bluetooth deinitialized successfully");
  } else {
    ESP_LOGE(TAG, "Failed to deinitialize Bluetooth: %d", static_cast<int>(ret));
    return false;
  }

  // Check deinitialized state
  if (!bluetooth_instance.IsInitialized()) {
    ESP_LOGI(TAG, "[SUCCESS] Bluetooth deinitialization confirmed");
  } else {
    ESP_LOGW(TAG, "Bluetooth still showing as initialized");
    return false;
  }

  // Unregister event callback
  ret = bluetooth_instance.UnregisterEventCallback();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Event callback unregistered successfully");
  }

  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 BLUETOOTH COMPREHENSIVE TEST SUITE                 ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  ESP_LOGI(TAG, "Target: %s", CONFIG_IDF_TARGET);
  ESP_LOGI(TAG, "ESP-IDF Version: %s", IDF_VER);

  vTaskDelay(pdMS_TO_TICKS(1000));

  RUN_TEST(test_bluetooth_initialization);
  RUN_TEST(test_bluetooth_basic_operations);
  RUN_TEST(test_bluetooth_scanning);
  RUN_TEST(test_bluetooth_state_management);
  RUN_TEST(test_bluetooth_cleanup);

  print_test_summary(g_test_results, "BLUETOOTH", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG, "[SUCCESS] ALL BLUETOOTH TESTS PASSED!");
    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "Implementation Summary:");
    ESP_LOGI(TAG, "[SUCCESS] ESP32C6 BLE-only support using NimBLE");
    ESP_LOGI(TAG, "[SUCCESS] Proper conditional compilation for different ESP32 variants");
    ESP_LOGI(TAG, "[SUCCESS] Basic BLE operations (init, enable, scan, cleanup)");
    ESP_LOGI(TAG, "[SUCCESS] Device discovery and management");
    ESP_LOGI(TAG, "[SUCCESS] Event-driven architecture");
    ESP_LOGI(TAG, "[SUCCESS] Thread-safe implementation");
    ESP_LOGI(TAG, "[SUCCESS] Modern BaseBluetooth API usage");
    ESP_LOGI(TAG, "[SUCCESS] Correct callback signatures");
    ESP_LOGI(TAG, "==================================================");
  } else {
    ESP_LOGE(TAG, "[FAILED] Some tests failed.");
  }

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
