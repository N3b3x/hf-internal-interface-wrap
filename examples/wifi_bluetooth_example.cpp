/**
 * @file wifi_bluetooth_example.cpp
 * @brief Comprehensive example demonstrating WiFi and Bluetooth usage with ESP32 implementations
 *
 * This example shows how to use the unified BaseWifi and BaseBluetooth classes
 * with their ESP32 implementations (EspWifi and EspBluetooth). It demonstrates
 * both basic and advanced features including:
 * 
 * WiFi Features:
 * - Station mode connection
 * - Access Point mode
 * - Network scanning
 * - WPA3 security
 * - Power management
 * - Enterprise authentication
 * 
 * Bluetooth Features:
 * - BLE advertising and scanning
 * - Classic Bluetooth pairing
 * - GATT server and client operations
 * - Multiple profile support
 * - Advanced security features
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "BaseWifi.h"
#include "BaseBluetooth.h"
#include "mcu/esp32/EspWifi.h"
#include "mcu/esp32/EspBluetooth.h"

// ESP-IDF headers
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

#include <vector>
#include <memory>

// ESP-IDF logging tags
static const char* TAG_MAIN = "HARDFOC_DEMO";
static const char* TAG_WIFI = "HARDFOC_WIFI";
static const char* TAG_BT = "HARDFOC_BT";

// Example configuration
namespace Example {
  // WiFi configuration
  constexpr const char* WIFI_SSID = "MyNetwork";
  constexpr const char* WIFI_PASSWORD = "MyPassword";
  constexpr const char* AP_SSID = "ESP32_HardFOC";
  constexpr const char* AP_PASSWORD = "HardFOC123";
  
  // Bluetooth configuration
  constexpr const char* BT_DEVICE_NAME = "HardFOC_Device";
  
  // GATT service UUIDs
  constexpr const char* CUSTOM_SERVICE_UUID = "12345678-1234-1234-1234-123456789ABC";
  constexpr const char* DATA_CHARACTERISTIC_UUID = "87654321-4321-4321-4321-CBA987654321";
}

/**
 * @class WiFiBluetoothDemo
 * @brief Demonstration class for WiFi and Bluetooth functionality
 */
class WiFiBluetoothDemo {
private:
  std::unique_ptr<BaseWifi> m_wifi;
  std::unique_ptr<BaseBluetooth> m_bluetooth;
  bool m_demo_running;

public:
  /**
   * @brief Constructor - initializes WiFi and Bluetooth with ESP32 implementations
   */
     WiFiBluetoothDemo() : m_demo_running(false) {
     ESP_LOGI(TAG_MAIN, "=== HardFOC WiFi & Bluetooth Demo ===");
    
    // Create ESP32-specific advanced configurations
    EspWifiAdvancedConfig wifi_config = {};
    wifi_config.enable_power_save = true;
    wifi_config.power_save_type = WIFI_PS_MIN_MODEM;
    wifi_config.tx_power = 15; // 15 dBm
    wifi_config.bandwidth = WIFI_BW_HT20;
    wifi_config.enable_wpa3_transition = true;
    wifi_config.enable_11k = true;
    wifi_config.enable_11r = true;
    wifi_config.enable_11v = true;
    
    EspBluetoothAdvancedConfig bt_config = {};
    bt_config.enable_power_save = true;
    bt_config.tx_power_level = ESP_PWR_LVL_P3; // +3 dBm
    bt_config.max_connections = 4;
    bt_config.enable_gatt_server = true;
    bt_config.enable_gatt_client = true;
    bt_config.enable_spp = true;
    bt_config.enable_secure_connections = true;
    bt_config.enable_privacy = true;
    bt_config.io_capability = ESP_IO_CAP_NO;
    
         // Create instances with advanced configurations
     m_wifi = std::make_unique<EspWifi>(&wifi_config);
     m_bluetooth = std::make_unique<EspBluetooth>(&bt_config);
     
     ESP_LOGI(TAG_MAIN, "WiFi and Bluetooth instances created successfully!");
  }
  
  /**
   * @brief Destructor - cleanup
   */
     ~WiFiBluetoothDemo() {
     stopDemo();
     ESP_LOGI(TAG_MAIN, "Demo cleanup completed.");
   }
  
  /**
   * @brief Start the comprehensive demo
   */
  void startDemo() {
    m_demo_running = true;
    
         ESP_LOGI(TAG_MAIN, "--- Starting WiFi & Bluetooth Demo ---");
     
     // Initialize both subsystems
     if (!initializeWifi() || !initializeBluetooth()) {
       ESP_LOGE(TAG_MAIN, "Failed to initialize WiFi or Bluetooth!");
       return;
     }
    
    // Register event callbacks
    registerEventHandlers();
    
    // Run WiFi demonstrations
    demonstrateWifiFeatures();
    
    // Run Bluetooth demonstrations
    demonstrateBluetoothFeatures();
    
         // Keep demo running
     ESP_LOGI(TAG_MAIN, "Demo running... Press reset to stop.");
     while (m_demo_running) {
       vTaskDelay(pdMS_TO_TICKS(1000));
       
       // Print status updates
       printStatus();
       
       vTaskDelay(pdMS_TO_TICKS(4000));
     }
  }
  
  /**
   * @brief Stop the demo
   */
  void stopDemo() {
    m_demo_running = false;
    
    if (m_wifi) {
      m_wifi->disconnect();
      m_wifi->stopAccessPoint();
      m_wifi->deinit();
    }
    
    if (m_bluetooth) {
      m_bluetooth->stopAdvertising();
      m_bluetooth->stopScan();
      m_bluetooth->disable();
      m_bluetooth->deinit();
    }
  }

private:
  /**
   * @brief Initialize WiFi subsystem
   */
     bool initializeWifi() {
     ESP_LOGI(TAG_WIFI, "=== WiFi Initialization ===");
     
     // Initialize WiFi in station + AP mode
     auto result = m_wifi->init(HfWifiMode::STATION_AP);
     if (result != HfWifiErr::WIFI_SUCCESS) {
       ESP_LOGE(TAG_WIFI, "WiFi initialization failed: %s", BaseWifi::getErrorString(result));
       return false;
     }
     
     ESP_LOGI(TAG_WIFI, "WiFi initialized successfully!");
     return true;
   }
  
  /**
   * @brief Initialize Bluetooth subsystem
   */
     bool initializeBluetooth() {
     ESP_LOGI(TAG_BT, "=== Bluetooth Initialization ===");
     
     // Initialize Bluetooth in dual mode (Classic + BLE)
     auto result = m_bluetooth->init(HfBluetoothMode::DUAL);
     if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
       ESP_LOGE(TAG_BT, "Bluetooth initialization failed: %s", BaseBluetooth::getErrorString(result));
       return false;
     }
     
     // Enable Bluetooth
     result = m_bluetooth->enable();
     if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
       ESP_LOGE(TAG_BT, "Bluetooth enable failed: %s", BaseBluetooth::getErrorString(result));
       return false;
     }
     
     // Set device name
     result = m_bluetooth->setDeviceName(Example::BT_DEVICE_NAME);
     if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
       ESP_LOGW(TAG_BT, "Failed to set Bluetooth device name");
     }
     
     ESP_LOGI(TAG_BT, "Bluetooth initialized and enabled successfully!");
     return true;
   }
  
  /**
   * @brief Register event handlers for WiFi and Bluetooth
   */
  void registerEventHandlers() {
    // WiFi event handler
    m_wifi->registerEventCallback([this](HfWifiEvent event, void* event_data) {
      handleWifiEvent(event, event_data);
    });
    
    // Bluetooth event handler
    m_bluetooth->registerEventCallback([this](HfBluetoothEvent event, void* event_data) {
      handleBluetoothEvent(event, event_data);
    });
    
    // Bluetooth data handler
    m_bluetooth->registerDataCallback([this](const HfBluetoothAddress& address, const std::vector<uint8_t>& data) {
      handleBluetoothData(address, data);
    });
  }
  
  /**
   * @brief Demonstrate WiFi features
   */
     void demonstrateWifiFeatures() {
     ESP_LOGI(TAG_WIFI, "=== WiFi Features Demonstration ===");
    
    // 1. Network scanning
    demonstrateWifiScanning();
    
    // 2. Station mode connection
    demonstrateWifiStation();
    
    // 3. Access Point mode
    demonstrateWifiAccessPoint();
    
    // 4. Power management
    demonstrateWifiPowerManagement();
  }
  
  /**
   * @brief Demonstrate WiFi network scanning
   */
     void demonstrateWifiScanning() {
     ESP_LOGI(TAG_WIFI, "--- WiFi Network Scanning ---");
     
     // Start scan
     auto result = m_wifi->startScan(true, false, 5000); // Show hidden, active scan, 5s timeout
     if (result != HfWifiErr::WIFI_SUCCESS) {
       ESP_LOGE(TAG_WIFI, "Failed to start WiFi scan");
       return;
     }
     
     ESP_LOGI(TAG_WIFI, "WiFi scan started...");
     
     // Wait for scan to complete
     vTaskDelay(pdMS_TO_TICKS(6000));
     
     // Get scan results
     std::vector<HfWifiNetworkInfo> networks;
     result = m_wifi->getScanResults(networks, 10); // Get up to 10 networks
     
     if (result == HfWifiErr::WIFI_SUCCESS && !networks.empty()) {
       ESP_LOGI(TAG_WIFI, "Found %d networks:", networks.size());
       for (const auto& network : networks) {
         ESP_LOGI(TAG_WIFI, "  SSID: %s, RSSI: %d dBm, Channel: %d, Security: %d", 
                  network.ssid.c_str(), 
                  static_cast<int>(network.rssi),
                  static_cast<int>(network.channel),
                  static_cast<int>(network.security));
       }
     } else {
       ESP_LOGW(TAG_WIFI, "No networks found or scan failed.");
     }
   }
  
  /**
   * @brief Demonstrate WiFi station mode
   */
  void demonstrateWifiStation() {
    ESP_LOGI(TAG_WIFI, "--- WiFi Station Mode ---");
    
    // Configure station
    HfWifiStationConfig station_config = {};
    station_config.ssid = Example::WIFI_SSID;
    station_config.password = Example::WIFI_PASSWORD;
    station_config.bssid_set = false;
    station_config.channel = 0; // Any channel
    station_config.scan_method = 0; // Fast scan
    station_config.sort_method = 0; // Sort by signal
    station_config.threshold_rssi = -80; // Minimum RSSI
    station_config.threshold_authmode = HfWifiSecurity::WPA2_PSK;
    
    auto result = m_wifi->configureStation(station_config);
    if (result != HfWifiErr::WIFI_SUCCESS) {
      ESP_LOGE(TAG_WIFI, "Failed to configure station mode");
      return;
    }
    
    // Attempt connection
    ESP_LOGI(TAG_WIFI, "Connecting to %s...", Example::WIFI_SSID);
    result = m_wifi->connect(15000); // 15 second timeout
    
    if (result == HfWifiErr::WIFI_SUCCESS) {
      ESP_LOGI(TAG_WIFI, "WiFi connection successful!");
      
      // Get IP information
      HfWifiIpInfo ip_info;
      if (m_wifi->getIpInfo(ip_info) == HfWifiErr::WIFI_SUCCESS) {
        ESP_LOGI(TAG_WIFI, "IP: 0x%08x", ip_info.ip);
        ESP_LOGI(TAG_WIFI, "Netmask: 0x%08x", ip_info.netmask);
        ESP_LOGI(TAG_WIFI, "Gateway: 0x%08x", ip_info.gateway);
      }
    } else {
      ESP_LOGE(TAG_WIFI, "WiFi connection failed: %s", BaseWifi::getErrorString(result));
    }
  }
  
  /**
   * @brief Demonstrate WiFi Access Point mode
   */
  void demonstrateWifiAccessPoint() {
    ESP_LOGI(TAG_WIFI, "--- WiFi Access Point Mode ---");
    
    // Configure Access Point
    HfWifiApConfig ap_config = {};
    ap_config.ssid = Example::AP_SSID;
    ap_config.password = Example::AP_PASSWORD;
    ap_config.ssid_len = 0; // Auto-detect length
    ap_config.channel = 6;
    ap_config.authmode = HfWifiSecurity::WPA2_PSK;
    ap_config.ssid_hidden = 0; // Broadcast SSID
    ap_config.max_connection = 4;
    ap_config.beacon_interval = 100;
    
    auto result = m_wifi->configureAccessPoint(ap_config);
    if (result != HfWifiErr::WIFI_SUCCESS) {
      ESP_LOGE(TAG_WIFI, "Failed to configure Access Point");
      return;
    }
    
    // Start Access Point
    result = m_wifi->startAccessPoint();
    if (result == HfWifiErr::WIFI_SUCCESS) {
      ESP_LOGI(TAG_WIFI, "Access Point '%s' started successfully!", Example::AP_SSID);
    } else {
      ESP_LOGE(TAG_WIFI, "Failed to start Access Point: %s", BaseWifi::getErrorString(result));
    }
  }
  
  /**
   * @brief Demonstrate WiFi power management
   */
  void demonstrateWifiPowerManagement() {
    ESP_LOGI(TAG_WIFI, "--- WiFi Power Management ---");
    
    // Set power save mode
    auto result = m_wifi->setPowerSave(HfWifiPowerSave::MIN_MODEM);
    if (result == HfWifiErr::WIFI_SUCCESS) {
      ESP_LOGI(TAG_WIFI, "WiFi power save enabled (MIN_MODEM)");
    } else {
      ESP_LOGE(TAG_WIFI, "Failed to set WiFi power save mode");
    }
    
    // Get current power save mode
    auto power_mode = m_wifi->getPowerSave();
    ESP_LOGI(TAG_WIFI, "Current power save mode: %d", static_cast<int>(power_mode));
  }
  
  /**
   * @brief Demonstrate Bluetooth features
   */
  void demonstrateBluetoothFeatures() {
    ESP_LOGI(TAG_BT, "=== Bluetooth Features Demonstration ===");
    
    // 1. BLE advertising
    demonstrateBluetoothAdvertising();
    
    // 2. BLE scanning
    demonstrateBluetoothScanning();
    
    // 3. GATT server setup
    demonstrateBluetoothGattServer();
    
    // 4. Classic Bluetooth setup
    demonstrateBluetoothClassic();
  }
  
  /**
   * @brief Demonstrate Bluetooth advertising
   */
  void demonstrateBluetoothAdvertising() {
    ESP_LOGI(TAG_BT, "--- Bluetooth BLE Advertising ---");
    
    // Configure BLE
    HfBluetoothBleConfig ble_config = {};
    ble_config.device_name = Example::BT_DEVICE_NAME;
    ble_config.advertising = true;
    ble_config.scannable = true;
    ble_config.connectable = true;
    ble_config.advertising_interval_ms = 100; // 100ms
    ble_config.scan_interval_ms = 50;
    ble_config.scan_window_ms = 30;
    ble_config.scan_type = HfBluetoothScanType::ACTIVE;
    ble_config.security = HfBluetoothSecurity::AUTHENTICATED;
    
    auto result = m_bluetooth->configureBle(ble_config);
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGE(TAG_BT, "Failed to configure BLE");
      return;
    }
    
    // Start advertising
    result = m_bluetooth->startAdvertising();
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGI(TAG_BT, "BLE advertising started successfully!");
    } else {
      ESP_LOGE(TAG_BT, "Failed to start BLE advertising: %s", BaseBluetooth::getErrorString(result));
    }
  }
  
  /**
   * @brief Demonstrate Bluetooth scanning
   */
  void demonstrateBluetoothScanning() {
    ESP_LOGI(TAG_BT, "--- Bluetooth BLE Scanning ---");
    
    // Start scan
    auto result = m_bluetooth->startScan(10000, HfBluetoothScanType::ACTIVE); // 10 second scan
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGE(TAG_BT, "Failed to start Bluetooth scan");
      return;
    }
    
    ESP_LOGI(TAG_BT, "Bluetooth scan started for 10 seconds...");
    
    // Wait for scan to complete using FreeRTOS delay
    vTaskDelay(11000 / portTICK_PERIOD_MS); // 11 seconds
    
    // Get discovered devices
    std::vector<HfBluetoothDeviceInfo> devices;
    result = m_bluetooth->getDiscoveredDevices(devices);
    
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS && !devices.empty()) {
      ESP_LOGI(TAG_BT, "Found %d Bluetooth devices:", devices.size());
      for (const auto& device : devices) {
        ESP_LOGI(TAG_BT, "  Name: %s, Address: %s, RSSI: %d dBm, Type: %d",
                 device.name.c_str(), device.address.toString().c_str(),
                 static_cast<int>(device.rssi), static_cast<int>(device.type));
      }
    } else {
      ESP_LOGW(TAG_BT, "No Bluetooth devices found or scan failed.");
    }
  }
  
  /**
   * @brief Demonstrate GATT server setup
   */
  void demonstrateBluetoothGattServer() {
    ESP_LOGI(TAG_BT, "--- Bluetooth GATT Server ---");
    
    // Cast to EspBluetooth to access ESP-specific features
    auto esp_bluetooth = dynamic_cast<EspBluetooth*>(m_bluetooth.get());
    if (!esp_bluetooth) {
      ESP_LOGE(TAG_BT, "Failed to cast to EspBluetooth");
      return;
    }
    
    // Create custom GATT service
    uint16_t service_handle = esp_bluetooth->createGattService(Example::CUSTOM_SERVICE_UUID, true, 10);
    if (service_handle == 0) {
      ESP_LOGE(TAG_BT, "Failed to create GATT service");
      return;
    }
    
    ESP_LOGI(TAG_BT, "Created GATT service with handle: %d", service_handle);
    
    // Add characteristic
    uint16_t char_handle = esp_bluetooth->addGattCharacteristic(
      service_handle,
      Example::DATA_CHARACTERISTIC_UUID,
      static_cast<esp_gatt_char_prop_t>(ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY),
      static_cast<esp_gatt_perm_t>(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE)
    );
    
    if (char_handle == 0) {
      ESP_LOGE(TAG_BT, "Failed to add GATT characteristic");
      return;
    }
    
    ESP_LOGI(TAG_BT, "Added GATT characteristic with handle: %d", char_handle);
    
    // Start service
    auto result = esp_bluetooth->startGattService(service_handle);
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGI(TAG_BT, "GATT service started successfully!");
    } else {
      ESP_LOGE(TAG_BT, "Failed to start GATT service");
    }
  }
  
  /**
   * @brief Demonstrate Classic Bluetooth setup
   */
  void demonstrateBluetoothClassic() {
    ESP_LOGI(TAG_BT, "--- Bluetooth Classic Setup ---");
    
    // Configure Classic Bluetooth
    HfBluetoothClassicConfig classic_config = {};
    classic_config.device_name = Example::BT_DEVICE_NAME;
    classic_config.discoverable = true;
    classic_config.connectable = true;
    classic_config.discovery_timeout_ms = 30000; // 30 seconds
    classic_config.security = HfBluetoothSecurity::AUTHENTICATED;
    
    auto result = m_bluetooth->configureClassic(classic_config);
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGE(TAG_BT, "Failed to configure Classic Bluetooth");
      return;
    }
    
    // Make device discoverable
    result = m_bluetooth->setDiscoverable(true, 60000); // 60 seconds
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      ESP_LOGI(TAG_BT, "Device is now discoverable for 60 seconds!");
    } else {
      ESP_LOGE(TAG_BT, "Failed to make device discoverable");
    }
    
    // Enable SPP (Serial Port Profile) for ESP32
    auto esp_bluetooth = dynamic_cast<EspBluetooth*>(m_bluetooth.get());
    if (esp_bluetooth) {
      result = esp_bluetooth->enableSpp(true);
      if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
        ESP_LOGI(TAG_BT, "SPP (Serial Port Profile) enabled!");
      }
    }
  }
  
  /**
   * @brief Handle WiFi events
   */
     void handleWifiEvent(HfWifiEvent event, void* event_data) {
     switch (event) {
       case HfWifiEvent::STA_CONNECTED:
         ESP_LOGI(TAG_WIFI, "Station connected to AP");
         break;
       case HfWifiEvent::STA_DISCONNECTED:
         ESP_LOGI(TAG_WIFI, "Station disconnected from AP");
         break;
       case HfWifiEvent::STA_GOT_IP:
         ESP_LOGI(TAG_WIFI, "Station got IP address");
         break;
       case HfWifiEvent::AP_START:
         ESP_LOGI(TAG_WIFI, "Access Point started");
         break;
       case HfWifiEvent::AP_STACONNECTED:
         ESP_LOGI(TAG_WIFI, "Station connected to our AP");
         break;
       case HfWifiEvent::SCAN_DONE:
         ESP_LOGI(TAG_WIFI, "Network scan completed");
         break;
       default:
         ESP_LOGD(TAG_WIFI, "WiFi Event ID: %d", static_cast<int>(event));
         break;
     }
   }
  
  /**
   * @brief Handle Bluetooth events
   */
     void handleBluetoothEvent(HfBluetoothEvent event, void* event_data) {
     switch (event) {
       case HfBluetoothEvent::ENABLED:
         ESP_LOGI(TAG_BT, "Bluetooth enabled");
         break;
       case HfBluetoothEvent::DEVICE_FOUND:
         ESP_LOGI(TAG_BT, "Device discovered");
         break;
       case HfBluetoothEvent::CONNECT_SUCCESS:
         ESP_LOGI(TAG_BT, "Device connected");
         break;
       case HfBluetoothEvent::DISCONNECT:
         ESP_LOGI(TAG_BT, "Device disconnected");
         break;
       case HfBluetoothEvent::PAIR_SUCCESS:
         ESP_LOGI(TAG_BT, "Pairing successful");
         break;
       case HfBluetoothEvent::DATA_RECEIVED:
         ESP_LOGI(TAG_BT, "Data received");
         break;
       default:
         ESP_LOGD(TAG_BT, "Bluetooth Event ID: %d", static_cast<int>(event));
         break;
     }
   }
  
  /**
   * @brief Handle Bluetooth data
   */
     void handleBluetoothData(const HfBluetoothAddress& address, const std::vector<uint8_t>& data) {
     ESP_LOGI(TAG_BT, "Received %d bytes from %s", data.size(), address.toString().c_str());
     
     // Echo the data back
     m_bluetooth->sendData(address, data);
   }
  
  /**
   * @brief Print current status
   */
     void printStatus() {
     ESP_LOGI(TAG_MAIN, "--- Status Update ---");
     
     // WiFi status
     ESP_LOGI(TAG_WIFI, "WiFi State: %d, Connected: %s, AP Active: %s", 
              static_cast<int>(m_wifi->getState()),
              m_wifi->isConnected() ? "Yes" : "No",
              m_wifi->isAccessPointActive() ? "Yes" : "No");
     
     if (m_wifi->isConnected()) {
       ESP_LOGI(TAG_WIFI, "RSSI: %d dBm, SSID: %s", 
                static_cast<int>(m_wifi->getRssi()),
                m_wifi->getConnectedSsid().c_str());
     }
     
     if (m_wifi->isAccessPointActive()) {
       ESP_LOGI(TAG_WIFI, "AP Stations: %d", m_wifi->getConnectedStationCount());
     }
     
     // Bluetooth status
     ESP_LOGI(TAG_BT, "BT State: %d, Enabled: %s, Advertising: %s, Scanning: %s, Discoverable: %s", 
              static_cast<int>(m_bluetooth->getState()),
              m_bluetooth->isEnabled() ? "Yes" : "No",
              m_bluetooth->isAdvertising() ? "Yes" : "No",
              m_bluetooth->isScanning() ? "Yes" : "No",
              m_bluetooth->isDiscoverable() ? "Yes" : "No");
     
     // Get connected devices
     std::vector<HfBluetoothDeviceInfo> connected_devices;
     if (m_bluetooth->getConnectedDevices(connected_devices) == HfBluetoothErr::BLUETOOTH_SUCCESS) {
       ESP_LOGI(TAG_BT, "Connected BT Devices: %d", connected_devices.size());
     }
   }
};

/**
 * @brief Main function - entry point for the demo
 */
extern "C" void app_main() {
  try {
    // Create and start the demo
    WiFiBluetoothDemo demo;
    demo.startDemo();
    
  } catch (const std::exception& e) {
    ESP_LOGE(TAG_MAIN, "Demo failed with exception: %s", e.what());
    return;
  }
  
  ESP_LOGI(TAG_MAIN, "Demo completed successfully!");
}