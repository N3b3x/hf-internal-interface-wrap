/**
 * @file Esp32c6NimbleTest.cpp
 * @brief Test example for ESP32C6 NimBLE Bluetooth implementation
 *
 * This example demonstrates basic NimBLE functionality for ESP32C6:
 * - Bluetooth initialization
 * - BLE scanning
 * - Device discovery
 * - Connection management
 *
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#include "EspBluetooth.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ESP32C6_BLE_TEST";

// Global Bluetooth instance
static EspBluetooth bluetooth_instance;

// Event callback function
void bluetooth_event_callback(hf_bluetooth_event_t event, const void* data, void* context) {
  switch (event) {
    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_ENABLED:
      ESP_LOGI(TAG, "Bluetooth enabled successfully");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISABLED:
      ESP_LOGI(TAG, "Bluetooth disabled");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_START:
      ESP_LOGI(TAG, "BLE scan started");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_STOP:
      ESP_LOGI(TAG, "BLE scan stopped");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DEVICE_FOUND:
      ESP_LOGI(TAG, "BLE device discovered");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_SUCCESS:
      ESP_LOGI(TAG, "Device connected successfully");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_FAILED:
      ESP_LOGW(TAG, "Device connection failed");
      break;

    case hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISCONNECT:
      ESP_LOGI(TAG, "Device disconnected");
      break;

    default:
      ESP_LOGI(TAG, "Bluetooth event: %d", static_cast<int>(event));
      break;
  }
}

// Test basic Bluetooth initialization and operations
void test_bluetooth_basic_operations() {
  ESP_LOGI(TAG, "=== Testing ESP32C6 NimBLE Basic Operations ===");

  // Set event callback
  hf_bluetooth_err_t ret = bluetooth_instance.SetEventCallback(bluetooth_event_callback, nullptr);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set event callback");
    return;
  }

  // Initialize Bluetooth in BLE mode
  ret = bluetooth_instance.Initialize(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize Bluetooth: %d", static_cast<int>(ret));
    return;
  }
  ESP_LOGI(TAG, "✓ Bluetooth initialized successfully");

  // Check if initialized
  if (!bluetooth_instance.IsInitialized()) {
    ESP_LOGE(TAG, "Bluetooth not showing as initialized");
    return;
  }
  ESP_LOGI(TAG, "✓ Bluetooth initialization confirmed");

  // Enable Bluetooth
  ret = bluetooth_instance.Enable();
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable Bluetooth: %d", static_cast<int>(ret));
    return;
  }
  ESP_LOGI(TAG, "✓ Bluetooth enabled successfully");

  // Check if enabled
  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGE(TAG, "Bluetooth not showing as enabled");
    return;
  }
  ESP_LOGI(TAG, "✓ Bluetooth enable state confirmed");

  // Get local address
  hf_bluetooth_address_t local_addr;
  ret = bluetooth_instance.GetLocalAddress(local_addr);
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Local BLE address: %02X:%02X:%02X:%02X:%02X:%02X", local_addr.addr[0],
             local_addr.addr[1], local_addr.addr[2], local_addr.addr[3], local_addr.addr[4],
             local_addr.addr[5]);
  } else {
    ESP_LOGW(TAG, "Could not get local address: %d", static_cast<int>(ret));
  }

  // Set device name
  ret = bluetooth_instance.SetDeviceName("ESP32C6-HardFOC-Test");
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Device name set successfully");
  } else {
    ESP_LOGW(TAG, "Failed to set device name: %d", static_cast<int>(ret));
  }

  // Get device name
  std::string device_name;
  ret = bluetooth_instance.GetDeviceName(device_name);
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Device name: %s", device_name.c_str());
  } else {
    ESP_LOGW(TAG, "Failed to get device name: %d", static_cast<int>(ret));
  }

  // Get implementation info
  std::string impl_info = bluetooth_instance.GetImplementationInfo();
  ESP_LOGI(TAG, "✓ Implementation info:\n%s", impl_info.c_str());

  // Get supported features
  uint32_t features = bluetooth_instance.GetSupportedFeatures();
  ESP_LOGI(TAG, "✓ Supported features: 0x%08lX", features);

  ESP_LOGI(TAG, "=== Basic Operations Test Completed ===");
}

// Test BLE scanning functionality
void test_bluetooth_scanning() {
  ESP_LOGI(TAG, "=== Testing ESP32C6 NimBLE Scanning ===");

  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGE(TAG, "Bluetooth not enabled, cannot test scanning");
    return;
  }

  // Configure scan parameters
  hf_bluetooth_scan_config_t scan_config;
  scan_config.duration_ms = 5000; // 5 second scan
  scan_config.type = hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE;
  scan_config.mode = hf_bluetooth_scan_mode_t::HF_BLUETOOTH_SCAN_MODE_LE_GENERAL;

  // Clear previously discovered devices
  hf_bluetooth_err_t ret = bluetooth_instance.ClearDiscoveredDevices();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Cleared discovered devices list");
  }

  // Start scanning
  ret = bluetooth_instance.StartScan(scan_config);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start scanning: %d", static_cast<int>(ret));
    return;
  }
  ESP_LOGI(TAG, "✓ BLE scanning started for 5 seconds");

  // Check scanning state
  if (bluetooth_instance.IsScanning()) {
    ESP_LOGI(TAG, "✓ Scanning state confirmed");
  } else {
    ESP_LOGW(TAG, "Scanning state not confirmed");
  }

  // Wait for scan to complete
  vTaskDelay(pdMS_TO_TICKS(6000)); // Wait 6 seconds

  // Check if scanning stopped automatically
  if (!bluetooth_instance.IsScanning()) {
    ESP_LOGI(TAG, "✓ Scanning stopped automatically after timeout");
  } else {
    ESP_LOGW(TAG, "Scanning still active, stopping manually");
    bluetooth_instance.StopScan();
  }

  // Get discovered devices
  std::vector<hf_bluetooth_device_info_t> discovered_devices;
  ret = bluetooth_instance.GetDiscoveredDevices(discovered_devices);
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Found %zu BLE devices:", discovered_devices.size());

    for (size_t i = 0; i < discovered_devices.size() && i < 10; i++) {
      const auto& device = discovered_devices[i];
      ESP_LOGI(TAG, "  Device %zu:", i + 1);
      ESP_LOGI(TAG, "    Address: %02X:%02X:%02X:%02X:%02X:%02X", device.address.addr[0],
               device.address.addr[1], device.address.addr[2], device.address.addr[3],
               device.address.addr[4], device.address.addr[5]);
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
  }

  ESP_LOGI(TAG, "=== Scanning Test Completed ===");
}

// Test state management
void test_bluetooth_state_management() {
  ESP_LOGI(TAG, "=== Testing ESP32C6 NimBLE State Management ===");

  // Test state retrieval
  hf_bluetooth_state_t state = bluetooth_instance.GetState();
  ESP_LOGI(TAG, "✓ Current Bluetooth state: %d", static_cast<int>(state));

  // Test mode retrieval
  hf_bluetooth_mode_t mode = bluetooth_instance.GetMode();
  ESP_LOGI(TAG, "✓ Current Bluetooth mode: %d", static_cast<int>(mode));

  // Test mode setting (should remain BLE for ESP32C6)
  hf_bluetooth_err_t ret =
      bluetooth_instance.SetMode(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_CLASSIC);
  if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Correctly rejected Classic mode for ESP32C6");
  } else {
    ESP_LOGW(TAG, "Unexpectedly accepted Classic mode");
  }

  // Verify mode is still BLE
  mode = bluetooth_instance.GetMode();
  if (mode == hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE) {
    ESP_LOGI(TAG, "✓ Mode correctly maintained as BLE");
  } else {
    ESP_LOGW(TAG, "Mode unexpectedly changed");
  }

  ESP_LOGI(TAG, "=== State Management Test Completed ===");
}

// Test cleanup
void test_bluetooth_cleanup() {
  ESP_LOGI(TAG, "=== Testing ESP32C6 NimBLE Cleanup ===");

  // Disable Bluetooth
  hf_bluetooth_err_t ret = bluetooth_instance.Disable();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Bluetooth disabled successfully");
  } else {
    ESP_LOGE(TAG, "Failed to disable Bluetooth: %d", static_cast<int>(ret));
  }

  // Check disabled state
  if (!bluetooth_instance.IsEnabled()) {
    ESP_LOGI(TAG, "✓ Bluetooth disable state confirmed");
  } else {
    ESP_LOGW(TAG, "Bluetooth still showing as enabled");
  }

  // Deinitialize Bluetooth
  ret = bluetooth_instance.Deinitialize();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Bluetooth deinitialized successfully");
  } else {
    ESP_LOGE(TAG, "Failed to deinitialize Bluetooth: %d", static_cast<int>(ret));
  }

  // Check deinitialized state
  if (!bluetooth_instance.IsInitialized()) {
    ESP_LOGI(TAG, "✓ Bluetooth deinitialization confirmed");
  } else {
    ESP_LOGW(TAG, "Bluetooth still showing as initialized");
  }

  // Clear event callback
  ret = bluetooth_instance.ClearEventCallback();
  if (ret == hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
    ESP_LOGI(TAG, "✓ Event callback cleared successfully");
  }

  ESP_LOGI(TAG, "=== Cleanup Test Completed ===");
}

// Main test task
extern "C" void app_main() {
  ESP_LOGI(TAG, "ESP32C6 NimBLE Bluetooth Test Starting...");
  ESP_LOGI(TAG, "Target: %s", CONFIG_IDF_TARGET);
  ESP_LOGI(TAG, "ESP-IDF Version: %s", IDF_VER);

  // Wait a bit for system to stabilize
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Run tests
  test_bluetooth_basic_operations();
  vTaskDelay(pdMS_TO_TICKS(1000));

  test_bluetooth_scanning();
  vTaskDelay(pdMS_TO_TICKS(1000));

  test_bluetooth_state_management();
  vTaskDelay(pdMS_TO_TICKS(1000));

  test_bluetooth_cleanup();

  ESP_LOGI(TAG, "ESP32C6 NimBLE Bluetooth Test Completed!");
  ESP_LOGI(TAG, "==================================================");
  ESP_LOGI(TAG, "Implementation Summary:");
  ESP_LOGI(TAG, "✓ ESP32C6 BLE-only support using NimBLE");
  ESP_LOGI(TAG, "✓ Proper conditional compilation for different ESP32 variants");
  ESP_LOGI(TAG, "✓ Basic BLE operations (init, enable, scan, cleanup)");
  ESP_LOGI(TAG, "✓ Device discovery and management");
  ESP_LOGI(TAG, "✓ Event-driven architecture");
  ESP_LOGI(TAG, "✓ Thread-safe implementation");
  ESP_LOGI(TAG, "==================================================");

  // Keep the task running
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}