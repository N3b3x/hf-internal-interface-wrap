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

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

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
    std::cout << "=== HardFOC WiFi & Bluetooth Demo ===" << std::endl;
    
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
    
    std::cout << "WiFi and Bluetooth instances created successfully!" << std::endl;
  }
  
  /**
   * @brief Destructor - cleanup
   */
  ~WiFiBluetoothDemo() {
    stopDemo();
    std::cout << "Demo cleanup completed." << std::endl;
  }
  
  /**
   * @brief Start the comprehensive demo
   */
  void startDemo() {
    m_demo_running = true;
    
    std::cout << "\n--- Starting WiFi & Bluetooth Demo ---" << std::endl;
    
    // Initialize both subsystems
    if (!initializeWifi() || !initializeBluetooth()) {
      std::cerr << "Failed to initialize WiFi or Bluetooth!" << std::endl;
      return;
    }
    
    // Register event callbacks
    registerEventHandlers();
    
    // Run WiFi demonstrations
    demonstrateWifiFeatures();
    
    // Run Bluetooth demonstrations
    demonstrateBluetoothFeatures();
    
    // Keep demo running
    std::cout << "\nDemo running... Press Ctrl+C to stop." << std::endl;
    while (m_demo_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      
      // Print status updates
      printStatus();
      
      std::this_thread::sleep_for(std::chrono::seconds(4));
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
    std::cout << "\n=== WiFi Initialization ===" << std::endl;
    
    // Initialize WiFi in station + AP mode
    auto result = m_wifi->init(HfWifiMode::STATION_AP);
    if (result != HfWifiErr::WIFI_SUCCESS) {
      std::cerr << "WiFi initialization failed: " << BaseWifi::getErrorString(result) << std::endl;
      return false;
    }
    
    std::cout << "WiFi initialized successfully!" << std::endl;
    return true;
  }
  
  /**
   * @brief Initialize Bluetooth subsystem
   */
  bool initializeBluetooth() {
    std::cout << "\n=== Bluetooth Initialization ===" << std::endl;
    
    // Initialize Bluetooth in dual mode (Classic + BLE)
    auto result = m_bluetooth->init(HfBluetoothMode::DUAL);
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cerr << "Bluetooth initialization failed: " << BaseBluetooth::getErrorString(result) << std::endl;
      return false;
    }
    
    // Enable Bluetooth
    result = m_bluetooth->enable();
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cerr << "Bluetooth enable failed: " << BaseBluetooth::getErrorString(result) << std::endl;
      return false;
    }
    
    // Set device name
    result = m_bluetooth->setDeviceName(Example::BT_DEVICE_NAME);
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cerr << "Failed to set Bluetooth device name" << std::endl;
    }
    
    std::cout << "Bluetooth initialized and enabled successfully!" << std::endl;
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
    std::cout << "\n=== WiFi Features Demonstration ===" << std::endl;
    
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
    std::cout << "\n--- WiFi Network Scanning ---" << std::endl;
    
    // Start scan
    auto result = m_wifi->startScan(true, false, 5000); // Show hidden, active scan, 5s timeout
    if (result != HfWifiErr::WIFI_SUCCESS) {
      std::cerr << "Failed to start WiFi scan" << std::endl;
      return;
    }
    
    std::cout << "WiFi scan started..." << std::endl;
    
    // Wait for scan to complete
    std::this_thread::sleep_for(std::chrono::seconds(6));
    
    // Get scan results
    std::vector<HfWifiNetworkInfo> networks;
    result = m_wifi->getScanResults(networks, 10); // Get up to 10 networks
    
    if (result == HfWifiErr::WIFI_SUCCESS && !networks.empty()) {
      std::cout << "Found " << networks.size() << " networks:" << std::endl;
      for (const auto& network : networks) {
        std::cout << "  SSID: " << network.ssid 
                  << ", RSSI: " << static_cast<int>(network.rssi) << " dBm"
                  << ", Channel: " << static_cast<int>(network.channel)
                  << ", Security: " << static_cast<int>(network.security) << std::endl;
      }
    } else {
      std::cout << "No networks found or scan failed." << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate WiFi station mode
   */
  void demonstrateWifiStation() {
    std::cout << "\n--- WiFi Station Mode ---" << std::endl;
    
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
      std::cerr << "Failed to configure station mode" << std::endl;
      return;
    }
    
    // Attempt connection
    std::cout << "Connecting to " << Example::WIFI_SSID << "..." << std::endl;
    result = m_wifi->connect(15000); // 15 second timeout
    
    if (result == HfWifiErr::WIFI_SUCCESS) {
      std::cout << "WiFi connection successful!" << std::endl;
      
      // Get IP information
      HfWifiIpInfo ip_info;
      if (m_wifi->getIpInfo(ip_info) == HfWifiErr::WIFI_SUCCESS) {
        std::cout << "IP: " << std::hex << ip_info.ip << std::dec << std::endl;
        std::cout << "Netmask: " << std::hex << ip_info.netmask << std::dec << std::endl;
        std::cout << "Gateway: " << std::hex << ip_info.gateway << std::dec << std::endl;
      }
    } else {
      std::cout << "WiFi connection failed: " << BaseWifi::getErrorString(result) << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate WiFi Access Point mode
   */
  void demonstrateWifiAccessPoint() {
    std::cout << "\n--- WiFi Access Point Mode ---" << std::endl;
    
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
      std::cerr << "Failed to configure Access Point" << std::endl;
      return;
    }
    
    // Start Access Point
    result = m_wifi->startAccessPoint();
    if (result == HfWifiErr::WIFI_SUCCESS) {
      std::cout << "Access Point '" << Example::AP_SSID << "' started successfully!" << std::endl;
    } else {
      std::cerr << "Failed to start Access Point: " << BaseWifi::getErrorString(result) << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate WiFi power management
   */
  void demonstrateWifiPowerManagement() {
    std::cout << "\n--- WiFi Power Management ---" << std::endl;
    
    // Set power save mode
    auto result = m_wifi->setPowerSave(HfWifiPowerSave::MIN_MODEM);
    if (result == HfWifiErr::WIFI_SUCCESS) {
      std::cout << "WiFi power save enabled (MIN_MODEM)" << std::endl;
    } else {
      std::cerr << "Failed to set WiFi power save mode" << std::endl;
    }
    
    // Get current power save mode
    auto power_mode = m_wifi->getPowerSave();
    std::cout << "Current power save mode: " << static_cast<int>(power_mode) << std::endl;
  }
  
  /**
   * @brief Demonstrate Bluetooth features
   */
  void demonstrateBluetoothFeatures() {
    std::cout << "\n=== Bluetooth Features Demonstration ===" << std::endl;
    
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
    std::cout << "\n--- Bluetooth BLE Advertising ---" << std::endl;
    
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
      std::cerr << "Failed to configure BLE" << std::endl;
      return;
    }
    
    // Start advertising
    result = m_bluetooth->startAdvertising();
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cout << "BLE advertising started successfully!" << std::endl;
    } else {
      std::cerr << "Failed to start BLE advertising: " << BaseBluetooth::getErrorString(result) << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate Bluetooth scanning
   */
  void demonstrateBluetoothScanning() {
    std::cout << "\n--- Bluetooth BLE Scanning ---" << std::endl;
    
    // Start scan
    auto result = m_bluetooth->startScan(10000, HfBluetoothScanType::ACTIVE); // 10 second scan
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cerr << "Failed to start Bluetooth scan" << std::endl;
      return;
    }
    
    std::cout << "Bluetooth scan started for 10 seconds..." << std::endl;
    
    // Wait for scan to complete
    std::this_thread::sleep_for(std::chrono::seconds(11));
    
    // Get discovered devices
    std::vector<HfBluetoothDeviceInfo> devices;
    result = m_bluetooth->getDiscoveredDevices(devices);
    
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS && !devices.empty()) {
      std::cout << "Found " << devices.size() << " Bluetooth devices:" << std::endl;
      for (const auto& device : devices) {
        std::cout << "  Name: " << device.name 
                  << ", Address: " << device.address.toString()
                  << ", RSSI: " << static_cast<int>(device.rssi) << " dBm"
                  << ", Type: " << static_cast<int>(device.type) << std::endl;
      }
    } else {
      std::cout << "No Bluetooth devices found or scan failed." << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate GATT server setup
   */
  void demonstrateBluetoothGattServer() {
    std::cout << "\n--- Bluetooth GATT Server ---" << std::endl;
    
    // Cast to EspBluetooth to access ESP-specific features
    auto esp_bluetooth = dynamic_cast<EspBluetooth*>(m_bluetooth.get());
    if (!esp_bluetooth) {
      std::cerr << "Failed to cast to EspBluetooth" << std::endl;
      return;
    }
    
    // Create custom GATT service
    uint16_t service_handle = esp_bluetooth->createGattService(Example::CUSTOM_SERVICE_UUID, true, 10);
    if (service_handle == 0) {
      std::cerr << "Failed to create GATT service" << std::endl;
      return;
    }
    
    std::cout << "Created GATT service with handle: " << service_handle << std::endl;
    
    // Add characteristic
    uint16_t char_handle = esp_bluetooth->addGattCharacteristic(
      service_handle,
      Example::DATA_CHARACTERISTIC_UUID,
      static_cast<esp_gatt_char_prop_t>(ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY),
      static_cast<esp_gatt_perm_t>(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE)
    );
    
    if (char_handle == 0) {
      std::cerr << "Failed to add GATT characteristic" << std::endl;
      return;
    }
    
    std::cout << "Added GATT characteristic with handle: " << char_handle << std::endl;
    
    // Start service
    auto result = esp_bluetooth->startGattService(service_handle);
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cout << "GATT service started successfully!" << std::endl;
    } else {
      std::cerr << "Failed to start GATT service" << std::endl;
    }
  }
  
  /**
   * @brief Demonstrate Classic Bluetooth setup
   */
  void demonstrateBluetoothClassic() {
    std::cout << "\n--- Bluetooth Classic Setup ---" << std::endl;
    
    // Configure Classic Bluetooth
    HfBluetoothClassicConfig classic_config = {};
    classic_config.device_name = Example::BT_DEVICE_NAME;
    classic_config.discoverable = true;
    classic_config.connectable = true;
    classic_config.discovery_timeout_ms = 30000; // 30 seconds
    classic_config.security = HfBluetoothSecurity::AUTHENTICATED;
    
    auto result = m_bluetooth->configureClassic(classic_config);
    if (result != HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cerr << "Failed to configure Classic Bluetooth" << std::endl;
      return;
    }
    
    // Make device discoverable
    result = m_bluetooth->setDiscoverable(true, 60000); // 60 seconds
    if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cout << "Device is now discoverable for 60 seconds!" << std::endl;
    } else {
      std::cerr << "Failed to make device discoverable" << std::endl;
    }
    
    // Enable SPP (Serial Port Profile) for ESP32
    auto esp_bluetooth = dynamic_cast<EspBluetooth*>(m_bluetooth.get());
    if (esp_bluetooth) {
      result = esp_bluetooth->enableSpp(true);
      if (result == HfBluetoothErr::BLUETOOTH_SUCCESS) {
        std::cout << "SPP (Serial Port Profile) enabled!" << std::endl;
      }
    }
  }
  
  /**
   * @brief Handle WiFi events
   */
  void handleWifiEvent(HfWifiEvent event, void* event_data) {
    switch (event) {
      case HfWifiEvent::STA_CONNECTED:
        std::cout << "[WiFi Event] Station connected to AP" << std::endl;
        break;
      case HfWifiEvent::STA_DISCONNECTED:
        std::cout << "[WiFi Event] Station disconnected from AP" << std::endl;
        break;
      case HfWifiEvent::STA_GOT_IP:
        std::cout << "[WiFi Event] Station got IP address" << std::endl;
        break;
      case HfWifiEvent::AP_START:
        std::cout << "[WiFi Event] Access Point started" << std::endl;
        break;
      case HfWifiEvent::AP_STACONNECTED:
        std::cout << "[WiFi Event] Station connected to our AP" << std::endl;
        break;
      case HfWifiEvent::SCAN_DONE:
        std::cout << "[WiFi Event] Network scan completed" << std::endl;
        break;
      default:
        std::cout << "[WiFi Event] Event ID: " << static_cast<int>(event) << std::endl;
        break;
    }
  }
  
  /**
   * @brief Handle Bluetooth events
   */
  void handleBluetoothEvent(HfBluetoothEvent event, void* event_data) {
    switch (event) {
      case HfBluetoothEvent::ENABLED:
        std::cout << "[Bluetooth Event] Bluetooth enabled" << std::endl;
        break;
      case HfBluetoothEvent::DEVICE_FOUND:
        std::cout << "[Bluetooth Event] Device discovered" << std::endl;
        break;
      case HfBluetoothEvent::CONNECT_SUCCESS:
        std::cout << "[Bluetooth Event] Device connected" << std::endl;
        break;
      case HfBluetoothEvent::DISCONNECT:
        std::cout << "[Bluetooth Event] Device disconnected" << std::endl;
        break;
      case HfBluetoothEvent::PAIR_SUCCESS:
        std::cout << "[Bluetooth Event] Pairing successful" << std::endl;
        break;
      case HfBluetoothEvent::DATA_RECEIVED:
        std::cout << "[Bluetooth Event] Data received" << std::endl;
        break;
      default:
        std::cout << "[Bluetooth Event] Event ID: " << static_cast<int>(event) << std::endl;
        break;
    }
  }
  
  /**
   * @brief Handle Bluetooth data
   */
  void handleBluetoothData(const HfBluetoothAddress& address, const std::vector<uint8_t>& data) {
    std::cout << "[Bluetooth Data] Received " << data.size() << " bytes from " 
              << address.toString() << std::endl;
    
    // Echo the data back
    m_bluetooth->sendData(address, data);
  }
  
  /**
   * @brief Print current status
   */
  void printStatus() {
    std::cout << "\n--- Status Update ---" << std::endl;
    
    // WiFi status
    std::cout << "WiFi State: " << static_cast<int>(m_wifi->getState()) << std::endl;
    std::cout << "WiFi Connected: " << (m_wifi->isConnected() ? "Yes" : "No") << std::endl;
    std::cout << "AP Active: " << (m_wifi->isAccessPointActive() ? "Yes" : "No") << std::endl;
    
    if (m_wifi->isConnected()) {
      std::cout << "RSSI: " << static_cast<int>(m_wifi->getRssi()) << " dBm" << std::endl;
      std::cout << "Connected SSID: " << m_wifi->getConnectedSsid() << std::endl;
    }
    
    if (m_wifi->isAccessPointActive()) {
      std::cout << "AP Stations: " << m_wifi->getConnectedStationCount() << std::endl;
    }
    
    // Bluetooth status
    std::cout << "Bluetooth State: " << static_cast<int>(m_bluetooth->getState()) << std::endl;
    std::cout << "Bluetooth Enabled: " << (m_bluetooth->isEnabled() ? "Yes" : "No") << std::endl;
    std::cout << "BLE Advertising: " << (m_bluetooth->isAdvertising() ? "Yes" : "No") << std::endl;
    std::cout << "BLE Scanning: " << (m_bluetooth->isScanning() ? "Yes" : "No") << std::endl;
    std::cout << "Discoverable: " << (m_bluetooth->isDiscoverable() ? "Yes" : "No") << std::endl;
    
    // Get connected devices
    std::vector<HfBluetoothDeviceInfo> connected_devices;
    if (m_bluetooth->getConnectedDevices(connected_devices) == HfBluetoothErr::BLUETOOTH_SUCCESS) {
      std::cout << "Connected BT Devices: " << connected_devices.size() << std::endl;
    }
  }
};

/**
 * @brief Main function - entry point for the demo
 */
int main() {
  try {
    // Create and start the demo
    WiFiBluetoothDemo demo;
    demo.startDemo();
    
  } catch (const std::exception& e) {
    std::cerr << "Demo failed with exception: " << e.what() << std::endl;
    return -1;
  }
  
  std::cout << "Demo completed successfully!" << std::endl;
  return 0;
}