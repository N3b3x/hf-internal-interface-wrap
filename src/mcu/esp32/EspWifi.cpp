/**
 * @file EspWifi.cpp
 * @brief Advanced ESP32 implementation of the unified BaseWifi class with ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseWifi class
 * for ESP32 microcontrollers with full ESP-IDF v5.5+ API support including
 * WPA3 security, mesh networking, enterprise authentication, and advanced
 * power management features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Requires ESP-IDF v5.5 or higher for full feature support
 * @note Thread-safe implementation with proper synchronization
 */

#include "mcu/esp32/EspWifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <cstring>
#include <algorithm>

static const char* TAG = "EspWifi";

// Event group bits for WiFi events
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1
#define WIFI_SCAN_DONE_BIT    BIT2
#define WIFI_AP_STARTED_BIT   BIT3
#define WIFI_SMARTCONFIG_BIT  BIT4

// Default values for advanced configuration
static const EspWifiAdvancedConfig DEFAULT_ADVANCED_CONFIG = {
  .enable_power_save = false,
  .power_save_type = WIFI_PS_NONE,
  .listen_interval = 3,
  .tx_power = 20,
  .bandwidth = WIFI_BW_HT20,
  .enable_ampdu_rx = true,
  .enable_ampdu_tx = true,
  .enable_fast_connect = false,
  .enable_pmf_required = false,
  .enable_wpa3_transition = true,
  .enable_11r = false,
  .enable_11k = false,
  .enable_11v = false,
  .enable_enterprise = false,
  .enable_mesh = false,
  .mesh_max_layer = 6,
  .mesh_max_connection = 10,
  .enable_smartconfig = false,
  .smartconfig_type = SC_TYPE_ESPTOUCH,
  .enable_wps = false,
  .wps_type = WPS_TYPE_PBC
};

/**
 * @brief Convert HardFOC WiFi mode to ESP-IDF WiFi mode
 */
static wifi_mode_t ConvertToEspMode(hf_wifi_mode_t mode) {
  switch (mode) {
    case hf_wifi_mode_t::HF_WIFI_MODE_STATION:
      return WIFI_MODE_STA;
    case hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT:
      return WIFI_MODE_AP;
    case hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP:
      return WIFI_MODE_APSTA;
    default:
      return WIFI_MODE_NULL;
  }
}

/**
 * @brief Convert ESP-IDF WiFi mode to HardFOC WiFi mode
 */
static hf_wifi_mode_t ConvertFromEspMode(wifi_mode_t mode) {
  switch (mode) {
    case WIFI_MODE_STA:
      return hf_wifi_mode_t::HF_WIFI_MODE_STATION;
    case WIFI_MODE_AP:
      return hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT;
    case WIFI_MODE_APSTA:
      return hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP;
    default:
      return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  }
}

/**
 * @brief Convert HardFOC WiFi security to ESP-IDF auth mode
 */
static wifi_auth_mode_t ConvertToEspAuthMode(hf_wifi_security_t security) {
  switch (security) {
    case hf_wifi_security_t::HF_WIFI_SECURITY_OPEN:
      return WIFI_AUTH_OPEN;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WEP:
      return WIFI_AUTH_WEP;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA_PSK:
      return WIFI_AUTH_WPA_PSK;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK:
      return WIFI_AUTH_WPA2_PSK;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA_WPA2_PSK:
      return WIFI_AUTH_WPA_WPA2_PSK;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_ENTERPRISE:
      return WIFI_AUTH_WPA2_ENTERPRISE;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_PSK:
      return WIFI_AUTH_WPA3_PSK;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_WPA3_PSK:
      return WIFI_AUTH_WPA2_WPA3_PSK;
    case hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_ENTERPRISE:
      return WIFI_AUTH_WPA3_ENTERPRISE;
    default:
      return WIFI_AUTH_OPEN;
  }
}

/**
 * @brief Convert ESP-IDF auth mode to HardFOC WiFi security
 */
static hf_wifi_security_t ConvertFromEspAuthMode(wifi_auth_mode_t auth_mode) {
  switch (auth_mode) {
    case WIFI_AUTH_OPEN:
      return hf_wifi_security_t::HF_WIFI_SECURITY_OPEN;
    case WIFI_AUTH_WEP:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WEP;
    case WIFI_AUTH_WPA_PSK:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA_PSK;
    case WIFI_AUTH_WPA2_PSK:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_PSK;
    case WIFI_AUTH_WPA_WPA2_PSK:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA_WPA2_PSK;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_ENTERPRISE;
    case WIFI_AUTH_WPA3_PSK:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_PSK;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA2_WPA3_PSK;
    case WIFI_AUTH_WPA3_ENTERPRISE:
      return hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_ENTERPRISE;
    default:
      return hf_wifi_security_t::HF_WIFI_SECURITY_OPEN;
  }
}

EspWifi::EspWifi(const EspWifiAdvancedConfig* advanced_config)
    : m_sta_netif(nullptr)
    , m_ap_netif(nullptr)
    , m_event_group(nullptr)
    , m_initialized(false)
    , m_mode(hf_wifi_mode_t::HF_WIFI_MODE_DISABLED)
    , is_connected_(false)
    , m_scanning(false)
    , m_smartconfig_active(false)
    , m_wps_active(false)
    , retry_count_(0)
    , max_retry_count_(5)
    , connection_timeout_ms_(10000)
    , scan_timeout_ms_(10000)
    , m_event_callback(nullptr)
    , m_event_user_data(nullptr)
    , m_scan_callback(nullptr)
    , m_scan_user_data(nullptr) {
  
  // Copy advanced configuration or use defaults
  if (advanced_config) {
    advanced_config_ = *advanced_config;
  } else {
    advanced_config_ = DEFAULT_ADVANCED_CONFIG;
  }
  
  ESP_LOGI(TAG, "EspWifi created with advanced config");
}

EspWifi::~EspWifi() {
  Deinitialize();
}

hf_wifi_err_t EspWifi::Initialize(hf_wifi_mode_t mode) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (m_initialized) {
    ESP_LOGW(TAG, "WiFi already initialized");
    return hf_wifi_err_t::WIFI_SUCCESS;
  }
  
  ESP_LOGI(TAG, "Initializing ESP32 WiFi with ESP-IDF v5.5+ features");
  
  // Initialize NVS if needed
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  
  // Initialize the underlying TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());
  
  // Create default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  
  // Create event group for WiFi events
  m_event_group = xEventGroupCreate();
  if (!m_event_group) {
    ESP_LOGE(TAG, "Failed to create event group");
    return hf_wifi_err_t::WIFI_INIT_FAILED;
  }
  
  // Create default station and AP network interfaces
  m_sta_netif = esp_netif_create_default_wifi_sta();
  m_ap_netif = esp_netif_create_default_wifi_ap();
  
  // Initialize WiFi with default configuration
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  
  // Apply advanced configuration
  cfg.ampdu_rx_enable = advanced_config_.enable_ampdu_rx ? 1 : 0;
  cfg.ampdu_tx_enable = advanced_config_.enable_ampdu_tx ? 1 : 0;
  cfg.static_tx_buf_num = 16;  // Optimize for performance
  cfg.dynamic_tx_buf_num = 32;
  cfg.static_rx_buf_num = 10;
  cfg.dynamic_rx_buf_num = 32;
  cfg.cache_tx_buf_num = 0;
  
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
    cleanup();
    return hf_wifi_err_t::WIFI_INIT_FAILED;
  }
  
  // Register event handlers
  ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::WifiEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(ret));
    cleanup();
    return hf_wifi_err_t::WIFI_INIT_FAILED;
  }
  
  ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::IpEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
    cleanup();
    return hf_wifi_err_t::WIFI_INIT_FAILED;
  }
  
  // Initialize SmartConfig if enabled
  if (advanced_config_.enable_smartconfig) {
    ret = esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::SmartconfigEventHandler, this);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register SmartConfig event handler: %s", esp_err_to_name(ret));
      cleanup();
      return hf_wifi_err_t::WIFI_INIT_FAILED;
    }
  }
  
  // Set WiFi mode to NULL initially
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  
  // Start WiFi
  ret = esp_wifi_start();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
    cleanup();
    return hf_wifi_err_t::WIFI_INIT_FAILED;
  }
  
  // Apply advanced power management settings
  if (advanced_config_.enable_power_save) {
    ESP_ERROR_CHECK(esp_wifi_set_ps(advanced_config_.power_save_type));
  }
  
  // Set TX power
  int8_t power = advanced_config_.tx_power * 4; // Convert to 0.25dBm units
  ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(power));
  
  m_initialized = true;
  
  ESP_LOGI(TAG, "ESP32 WiFi initialized successfully");
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::Deinitialize() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_SUCCESS;
  }
  
  ESP_LOGI(TAG, "Deinitializing ESP32 WiFi");
  
  // Stop ongoing operations
  if (m_smartconfig_active) {
    esp_smartconfig_stop();
    m_smartconfig_active = false;
  }
  
  if (m_wps_active) {
    esp_wifi_wps_disable();
    m_wps_active = false;
  }
  
  // Disconnect if connected
  if (is_connected_) {
    esp_wifi_disconnect();
    is_connected_ = false;
  }
  
  // Stop WiFi
  esp_wifi_stop();
  
  // Unregister event handlers
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::WifiEventHandler);
  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::IpEventHandler);
  
  if (advanced_config_.enable_smartconfig) {
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::SmartconfigEventHandler);
  }
  
  cleanup();
  
  m_initialized = false;
  m_mode = hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  
  ESP_LOGI(TAG, "ESP32 WiFi deinitialized successfully");
  return hf_wifi_err_t::WIFI_SUCCESS;
}

bool EspWifi::IsInitialized() const {
  return m_initialized;
}

hf_wifi_err_t EspWifi::SetMode(hf_wifi_mode_t mode) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::WIFI_NOT_INITIALIZED;
  }
  
  wifi_mode_t esp_mode = ConvertToEspMode(mode);
  if (esp_mode == WIFI_MODE_NULL && mode != hf_wifi_mode_t::HF_WIFI_MODE_DISABLED) {
    ESP_LOGE(TAG, "Invalid WiFi mode");
    return hf_wifi_err_t::WIFI_INVALID_PARAMETER;
  }
  
  esp_err_t ret = esp_wifi_set_mode(esp_mode);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::WIFI_MODE_SET_FAILED;
  }
  
  m_mode = mode;
  
  ESP_LOGI(TAG, "WiFi mode set to %d", static_cast<int>(mode));
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_mode_t EspWifi::GetMode() const {
  return m_mode;
}

hf_wifi_err_t EspWifi::StartStation(const hf_wifi_station_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  // Validate configuration
  if (config.ssid.empty() || config.ssid.length() > 32) {
    ESP_LOGE(TAG, "Invalid SSID");
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  if (config.password.length() > 64) {
    ESP_LOGE(TAG, "Invalid password");
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  // Set mode to station or station+AP
  hf_wifi_mode_t new_mode = (m_mode == hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT) ? 
                        hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP : hf_wifi_mode_t::HF_WIFI_MODE_STATION;
  
  hf_wifi_err_t err = SetMode(new_mode);
  if (err != hf_wifi_err_t::SUCCESS) {
    return err;
  }
  
  // Configure station
  wifi_config_t wifi_config = {};
  memcpy(wifi_config.sta.ssid, config.ssid.c_str(), config.ssid.length());
  memcpy(wifi_config.sta.password, config.password.c_str(), config.password.length());
  
  // Advanced configuration
  wifi_config.sta.threshold.authmode = convertToEspAuthMode(config.security);
  wifi_config.sta.pmf_cfg.capable = true;
  wifi_config.sta.pmf_cfg.required = advanced_config_.enable_pmf_required;
  
  if (config.use_static_ip) {
    wifi_config.sta.bssid_set = false; // Use DHCP static IP reservation if needed
  }
  
  // Apply channel hint if specified
  if (config.channel > 0 && config.channel <= 14) {
    wifi_config.sta.channel = config.channel;
  }
  
  // Configure fast connect if enabled
  if (advanced_config_.enable_fast_connect) {
    wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
  }
  
  // Configure enterprise authentication if enabled
  if (advanced_config_.enable_enterprise && !advanced_config_.enterprise_username.empty()) {
    esp_wifi_sta_enterprise_enable();
    esp_wifi_sta_enterprise_set_username(
        reinterpret_cast<const uint8_t*>(advanced_config_.enterprise_username.c_str()),
        advanced_config_.enterprise_username.length());
    esp_wifi_sta_enterprise_set_password(
        reinterpret_cast<const uint8_t*>(advanced_config_.enterprise_password.c_str()),
        advanced_config_.enterprise_password.length());
    
    if (!advanced_config_.enterprise_ca_cert.empty()) {
      esp_wifi_sta_enterprise_set_ca_cert(
          reinterpret_cast<const uint8_t*>(advanced_config_.enterprise_ca_cert.c_str()),
          advanced_config_.enterprise_ca_cert.length());
    }
  }
  
  esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set station config: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONFIG_FAILED;
  }
  
  // Store station configuration
  station_config_ = config;
  
  ESP_LOGI(TAG, "Station configuration set for SSID: %s", config.ssid.c_str());
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::StartAccessPoint(const hf_wifi_ap_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  // Validate configuration
  if (config.ssid.empty() || config.ssid.length() > 32) {
    ESP_LOGE(TAG, "Invalid SSID");
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  if (config.security != hf_wifi_security_t::OPEN && 
      (config.password.empty() || config.password.length() < 8 || config.password.length() > 64)) {
    ESP_LOGE(TAG, "Invalid password for secure AP");
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  if (config.channel < 1 || config.channel > 14) {
    ESP_LOGE(TAG, "Invalid channel");
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  // Set mode to AP or station+AP
  hf_wifi_mode_t new_mode = (m_mode == hf_wifi_mode_t::HF_WIFI_MODE_STATION) ? 
                        hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP : hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT;
  
  hf_wifi_err_t err = SetMode(new_mode);
  if (err != hf_wifi_err_t::SUCCESS) {
    return err;
  }
  
  // Configure access point
  wifi_config_t wifi_config = {};
  memcpy(wifi_config.ap.ssid, config.ssid.c_str(), config.ssid.length());
  wifi_config.ap.ssid_len = config.ssid.length();
  memcpy(wifi_config.ap.password, config.password.c_str(), config.password.length());
  wifi_config.ap.channel = config.channel;
  wifi_config.ap.max_connection = std::min(config.max_connections, static_cast<uint8_t>(10));
  wifi_config.ap.authmode = convertToEspAuthMode(config.security);
  wifi_config.ap.ssid_hidden = config.hidden ? 1 : 0;
  wifi_config.ap.beacon_interval = 100;
  wifi_config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_CCMP;
  
  // Advanced configuration
  wifi_config.ap.pmf_cfg.capable = true;
  wifi_config.ap.pmf_cfg.required = advanced_config_.enable_pmf_required;
  
  esp_err_t ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set AP config: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONFIG_FAILED;
  }
  
  // Configure AP IP if specified
  if (config.ip.isValid()) {
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = config.ip.toUint32();
    ip_info.gw.addr = config.gateway.toUint32();
    ip_info.netmask.addr = config.netmask.toUint32();
    
    esp_netif_dhcps_stop(m_ap_netif);
    esp_netif_set_ip_info(m_ap_netif, &ip_info);
    esp_netif_dhcps_start(m_ap_netif);
  }
  
  // Store AP configuration
  ap_config_ = config;
  
  ESP_LOGI(TAG, "Access Point configuration set for SSID: %s", config.ssid.c_str());
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::Connect() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  if (m_mode != hf_wifi_mode_t::HF_WIFI_MODE_STATION && m_mode != hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP) {
    ESP_LOGE(TAG, "Not in station mode");
    return hf_wifi_err_t::INVALID_MODE;
  }
  
  if (station_config_.ssid.empty()) {
    ESP_LOGE(TAG, "Station not configured");
    return hf_wifi_err_t::NOT_CONFIGURED;
  }
  
  // Clear event bits
  xEventGroupClearBits(m_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
  
  retry_count_ = 0;
  
  esp_err_t ret = esp_wifi_connect();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to connect: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONNECTION_FAILED;
  }
  
  ESP_LOGI(TAG, "Connecting to WiFi network: %s", station_config_.ssid.c_str());
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::Disconnect() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  esp_err_t ret = esp_wifi_disconnect();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disconnect: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::DISCONNECTION_FAILED;
  }
  
  is_connected_ = false;
  
  ESP_LOGI(TAG, "Disconnected from WiFi network");
  return hf_wifi_err_t::SUCCESS;
}

bool EspWifi::IsConnected() const {
  return is_connected_;
}

hf_wifi_connection_info_t EspWifi::GetConnectionInfo() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  hf_wifi_connection_info_t info = {};
  
  if (!is_connected_) {
    return info;
  }
  
  // Get WiFi configuration
  wifi_config_t wifi_config;
  if (esp_wifi_get_config(WIFI_IF_STA, &wifi_config) == ESP_OK) {
    info.ssid = std::string(reinterpret_cast<char*>(wifi_config.sta.ssid));
  }
  
  // Get AP info
  wifi_ap_record_t ap_info;
  if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    memcpy(info.bssid, ap_info.bssid, 6);
    info.channel = ap_info.primary;
    info.rssi = ap_info.rssi;
    info.security = convertFromEspAuthMode(ap_info.authmode);
  }
  
  // Get IP info
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(m_sta_netif, &ip_info) == ESP_OK) {
    info.ip = hf_network_address_t(ip_info.ip.addr);
    info.gateway = hf_network_address_t(ip_info.gw.addr);
    info.netmask = hf_network_address_t(ip_info.netmask.addr);
  }
  
  // Get DNS info
  esp_netif_dns_info_t dns_info;
  if (esp_netif_get_dns_info(m_sta_netif, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
    info.dns_primary = hf_network_address_t(dns_info.ip.u_addr.ip4.addr);
  }
  
  if (esp_netif_get_dns_info(m_sta_netif, ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
    info.dns_secondary = hf_network_address_t(dns_info.ip.u_addr.ip4.addr);
  }
  
  return info;
}

hf_wifi_err_t EspWifi::StartScan(const hf_wifi_scan_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  if (m_scanning) {
    ESP_LOGW(TAG, "Scan already in progress");
    return hf_wifi_err_t::SCAN_IN_PROGRESS;
  }
  
  // Clear scan done bit
  xEventGroupClearBits(m_event_group, WIFI_SCAN_DONE_BIT);
  
  wifi_scan_config_t scan_config = {};
  
  if (!config.ssid.empty()) {
    scan_config.ssid = reinterpret_cast<uint8_t*>(const_cast<char*>(config.ssid.c_str()));
  }
  
  if (config.channel > 0 && config.channel <= 14) {
    scan_config.channel = config.channel;
  }
  
  scan_config.show_hidden = config.show_hidden;
  scan_config.scan_type = config.active_scan ? WIFI_SCAN_TYPE_ACTIVE : WIFI_SCAN_TYPE_PASSIVE;
  
  // Set scan time based on configuration
  scan_config.scan_time.active.min = 120;
  scan_config.scan_time.active.max = 1500;
  scan_config.scan_time.passive = 360;
  
  esp_err_t ret = esp_wifi_scan_start(&scan_config, false);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start scan: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::SCAN_FAILED;
  }
  
  m_scanning = true;
  
  ESP_LOGI(TAG, "WiFi scan started");
  return hf_wifi_err_t::SUCCESS;
}

std::vector<hf_wifi_scan_result_t> EspWifi::GetScanResults() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  std::vector<hf_wifi_scan_result_t> results;
  
  if (m_scanning) {
    ESP_LOGW(TAG, "Scan still in progress");
    return results;
  }
  
  uint16_t number = 0;
  esp_wifi_scan_get_ap_num(&number);
  
  if (number == 0) {
    ESP_LOGW(TAG, "No scan results available");
    return results;
  }
  
  wifi_ap_record_t* ap_info = new wifi_ap_record_t[number];
  esp_wifi_scan_get_ap_records(&number, ap_info);
  
  results.reserve(number);
  
  for (uint16_t i = 0; i < number; i++) {
    hf_wifi_scan_result_t result;
    result.ssid = std::string(reinterpret_cast<char*>(ap_info[i].ssid));
    memcpy(result.bssid, ap_info[i].bssid, 6);
    result.channel = ap_info[i].primary;
    result.rssi = ap_info[i].rssi;
    result.security = convertFromEspAuthMode(ap_info[i].authmode);
    result.is_hidden = (result.ssid.empty());
    
    results.push_back(result);
  }
  
  delete[] ap_info;
  
  ESP_LOGI(TAG, "Retrieved %d scan results", results.size());
  return results;
}

hf_wifi_err_t EspWifi::StartSmartConfig() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!advanced_config_.enable_smartconfig) {
    ESP_LOGE(TAG, "SmartConfig not enabled in configuration");
    return hf_wifi_err_t::NOT_SUPPORTED;
  }
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  if (m_smartconfig_active) {
    ESP_LOGW(TAG, "SmartConfig already in progress");
    return hf_wifi_err_t::ALREADY_STARTED;
  }
  
  // Set WiFi mode to station
  hf_wifi_err_t err = SetMode(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::SUCCESS) {
    return err;
  }
  
  // Clear SmartConfig bit
  xEventGroupClearBits(m_event_group, WIFI_SMARTCONFIG_BIT);
  
  esp_err_t ret = esp_smartconfig_set_type(advanced_config_.smartconfig_type);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set SmartConfig type: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONFIG_FAILED;
  }
  
  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
  ret = esp_smartconfig_start(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start SmartConfig: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::START_FAILED;
  }
  
  m_smartconfig_active = true;
  
  ESP_LOGI(TAG, "SmartConfig started");
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::StopSmartConfig() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_smartconfig_active) {
    return hf_wifi_err_t::SUCCESS;
  }
  
  esp_err_t ret = esp_smartconfig_stop();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stop SmartConfig: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::STOP_FAILED;
  }
  
  m_smartconfig_active = false;
  
  ESP_LOGI(TAG, "SmartConfig stopped");
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::StartWPS() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!advanced_config_.enable_wps) {
    ESP_LOGE(TAG, "WPS not enabled in configuration");
    return hf_wifi_err_t::NOT_SUPPORTED;
  }
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  if (m_wps_active) {
    ESP_LOGW(TAG, "WPS already in progress");
    return hf_wifi_err_t::ALREADY_STARTED;
  }
  
  // Set WiFi mode to station
  hf_wifi_err_t err = SetMode(hf_wifi_mode_t::HF_WIFI_MODE_STATION);
  if (err != hf_wifi_err_t::SUCCESS) {
    return err;
  }
  
  esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(advanced_config_.wps_type);
  esp_err_t ret = esp_wifi_wps_enable(&config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable WPS: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::START_FAILED;
  }
  
  ret = esp_wifi_wps_start(0);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WPS: %s", esp_err_to_name(ret));
    esp_wifi_wps_disable();
    return hf_wifi_err_t::START_FAILED;
  }
  
  m_wps_active = true;
  
  ESP_LOGI(TAG, "WPS started");
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::StopWPS() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_wps_active) {
    return hf_wifi_err_t::SUCCESS;
  }
  
  esp_err_t ret = esp_wifi_wps_disable();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stop WPS: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::STOP_FAILED;
  }
  
  m_wps_active = false;
  
  ESP_LOGI(TAG, "WPS stopped");
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::SetPowerSave(bool enable, hf_wifi_power_save_mode_t mode) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  wifi_ps_type_t ps_type = WIFI_PS_NONE;
  
  if (enable) {
    switch (mode) {
      case hf_wifi_power_save_mode_t::NONE:
        ps_type = WIFI_PS_NONE;
        break;
      case hf_wifi_power_save_mode_t::MIN_MODEM:
        ps_type = WIFI_PS_MIN_MODEM;
        break;
      case hf_wifi_power_save_mode_t::MAX_MODEM:
        ps_type = WIFI_PS_MAX_MODEM;
        break;
    }
  }
  
  esp_err_t ret = esp_wifi_set_ps(ps_type);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set power save mode: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONFIG_FAILED;
  }
  
  ESP_LOGI(TAG, "Power save mode set to %s", enable ? "enabled" : "disabled");
  return hf_wifi_err_t::SUCCESS;
}

hf_wifi_err_t EspWifi::SetTxPower(int8_t power_dbm) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  // Validate power range (ESP32 supports 2-20 dBm)
  if (power_dbm < 2 || power_dbm > 20) {
    ESP_LOGE(TAG, "Invalid TX power: %d dBm (range: 2-20)", power_dbm);
    return hf_wifi_err_t::INVALID_PARAMETER;
  }
  
  // Convert to 0.25dBm units
  int8_t power_quarter_dbm = power_dbm * 4;
  
  esp_err_t ret = esp_wifi_set_max_tx_power(power_quarter_dbm);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set TX power: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::CONFIG_FAILED;
  }
  
  advanced_config_.tx_power = power_dbm;
  
  ESP_LOGI(TAG, "TX power set to %d dBm", power_dbm);
  return hf_wifi_err_t::SUCCESS;
}

int8_t EspWifi::GetTxPower() const {
  int8_t power;
  esp_err_t ret = esp_wifi_get_max_tx_power(&power);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get TX power: %s", esp_err_to_name(ret));
    return -1;
  }
  
  // Convert from 0.25dBm units
  return power / 4;
}

void EspWifi::SetEventCallback(hf_wifi_m_event_callbackt callback, void* user_data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_event_callback = callback;
  m_event_user_data = user_data;
}

void EspWifi::SetScanCallback(hf_wifi_m_scan_callbackt callback, void* user_data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_scan_callback = callback;
  m_scan_user_data = user_data;
}

hf_wifi_err_t EspWifi::WaitForConnection(uint32_t timeout_ms) {
  if (!m_event_group) {
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  EventBits_t bits = xEventGroupWaitBits(m_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         pdMS_TO_TICKS(timeout_ms));
  
  if (bits & WIFI_CONNECTED_BIT) {
    return hf_wifi_err_t::SUCCESS;
  } else if (bits & WIFI_FAIL_BIT) {
    return hf_wifi_err_t::CONNECTION_FAILED;
  } else {
    return hf_wifi_err_t::TIMEOUT;
  }
}

hf_wifi_err_t EspWifi::WaitForScan(uint32_t timeout_ms) {
  if (!m_event_group) {
    return hf_wifi_err_t::NOT_INITIALIZED;
  }
  
  EventBits_t bits = xEventGroupWaitBits(m_event_group,
                                         WIFI_SCAN_DONE_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         pdMS_TO_TICKS(timeout_ms));
  
  if (bits & WIFI_SCAN_DONE_BIT) {
    return hf_wifi_err_t::SUCCESS;
  } else {
    return hf_wifi_err_t::TIMEOUT;
  }
}

void EspWifi::Cleanup() {
  if (m_event_group) {
    vEventGroupDelete(m_event_group);
    m_event_group = nullptr;
  }
  
  // Cleanup network interfaces
  if (m_sta_netif) {
    esp_netif_destroy_default_wifi(m_sta_netif);
    m_sta_netif = nullptr;
  }
  
  if (m_ap_netif) {
    esp_netif_destroy_default_wifi(m_ap_netif);
    m_ap_netif = nullptr;
  }
  
  // Deinitialize WiFi
  esp_wifi_deinit();
}

void EspWifi::WifiEventHandler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleWifiEvent(event_base, event_id, event_data);
}

void EspWifi::IpEventHandler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleIpEvent(event_base, event_id, event_data);
}

void EspWifi::SmartconfigEventHandler(void* arg, esp_event_base_t event_base,
                                     int32_t event_id, void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleSmartconfigEvent(event_base, event_id, event_data);
}

void EspWifi::HandleWifiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "WiFi station started");
      break;
      
    case WIFI_EVENT_STA_STOP:
      ESP_LOGI(TAG, "WiFi station stopped");
      is_connected_ = false;
      break;
      
    case WIFI_EVENT_STA_CONNECTED: {
      wifi_event_sta_connected_t* event = static_cast<wifi_event_sta_connected_t*>(event_data);
      ESP_LOGI(TAG, "Connected to AP SSID:%s authmode:%d", 
               event->ssid, event->authmode);
      break;
    }
    
    case WIFI_EVENT_STA_DISCONNECTED: {
      wifi_event_sta_disconnected_t* event = static_cast<wifi_event_sta_disconnected_t*>(event_data);
      ESP_LOGI(TAG, "Disconnected from AP SSID:%s, reason:%d", 
               event->ssid, event->reason);
      
      is_connected_ = false;
      
      // Retry connection if within retry limit
      if (retry_count_ < max_retry_count_) {
        esp_wifi_connect();
        retry_count_++;
        ESP_LOGI(TAG, "Retry to connect to the AP (%d/%d)", retry_count_, max_retry_count_);
      } else {
        xEventGroupSetBits(m_event_group, WIFI_FAIL_BIT);
        ESP_LOGW(TAG, "Connect to the AP failed after %d retries", max_retry_count_);
      }
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::DISCONNECTED;
        hf_event.disconnected.reason = static_cast<hf_wifi_disconnect_reason_t>(event->reason);
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    case WIFI_EVENT_AP_START:
      ESP_LOGI(TAG, "WiFi AP started");
      xEventGroupSetBits(m_event_group, WIFI_AP_STARTED_BIT);
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::AP_STARTED;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
      
    case WIFI_EVENT_AP_STOP:
      ESP_LOGI(TAG, "WiFi AP stopped");
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::AP_STOPPED;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
      
    case WIFI_EVENT_AP_STACONNECTED: {
      wifi_event_ap_staconnected_t* event = static_cast<wifi_event_ap_staconnected_t*>(event_data);
      ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d",
               MAC2STR(event->mac), event->aid);
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::AP_STATION_CONNECTED;
        memcpy(hf_event.ap_station_connected.mac, event->mac, 6);
        hf_event.ap_station_connected.aid = event->aid;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    case WIFI_EVENT_AP_STADISCONNECTED: {
      wifi_event_ap_stadisconnected_t* event = static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
      ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d",
               MAC2STR(event->mac), event->aid);
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::AP_STATION_DISCONNECTED;
        memcpy(hf_event.ap_station_disconnected.mac, event->mac, 6);
        hf_event.ap_station_disconnected.aid = event->aid;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    case WIFI_EVENT_SCAN_DONE: {
      wifi_event_sta_scan_done_t* event = static_cast<wifi_event_sta_scan_done_t*>(event_data);
      ESP_LOGI(TAG, "Scan completed, found %d APs", event->number);
      
      m_scanning = false;
      xEventGroupSetBits(m_event_group, WIFI_SCAN_DONE_BIT);
      
      if (m_scan_callback) {
        std::vector<hf_wifi_scan_result_t> results = GetScanResults();
        m_scan_callback(results, m_scan_user_data);
      }
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::SCAN_COMPLETED;
        hf_event.scan_completed.num_results = event->number;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    default:
      ESP_LOGD(TAG, "Unhandled WiFi event: %ld", event_id);
      break;
  }
}

void EspWifi::HandleIpEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
      ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
      ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
      
      is_connected_ = true;
      retry_count_ = 0;
      xEventGroupSetBits(m_event_group, WIFI_CONNECTED_BIT);
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::CONNECTED;
        hf_event.connected.ip = hf_network_address_t(event->ip_info.ip.addr);
        hf_event.connected.gateway = hf_network_address_t(event->ip_info.gw.addr);
        hf_event.connected.netmask = hf_network_address_t(event->ip_info.netmask.addr);
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    case IP_EVENT_STA_LOST_IP:
      ESP_LOGI(TAG, "Lost IP address");
      is_connected_ = false;
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::IP_LOST;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
      
    default:
      ESP_LOGD(TAG, "Unhandled IP event: %ld", event_id);
      break;
  }
}

void EspWifi::HandleSmartconfigEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case SC_EVENT_SCAN_DONE:
      ESP_LOGI(TAG, "SmartConfig scan completed");
      break;
      
    case SC_EVENT_FOUND_CHANNEL:
      ESP_LOGI(TAG, "SmartConfig found channel");
      break;
      
    case SC_EVENT_GOT_SSID_PSWD: {
      smartconfig_event_got_ssid_pswd_t* evt = 
          static_cast<smartconfig_event_got_ssid_pswd_t*>(event_data);
      
      ESP_LOGI(TAG, "SmartConfig got SSID and password");
      
      wifi_config_t wifi_config = {};
      memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
      memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
      wifi_config.sta.bssid_set = evt->bssid_set;
      if (wifi_config.sta.bssid_set) {
        memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
      }
      
      ESP_ERROR_CHECK(esp_wifi_disconnect());
      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
      ESP_ERROR_CHECK(esp_wifi_connect());
      
      // Update stored station config
      station_config_.ssid = std::string(reinterpret_cast<char*>(evt->ssid));
      station_config_.password = std::string(reinterpret_cast<char*>(evt->password));
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::SMARTCONFIG_RECEIVED;
        hf_event.smartconfig_received.ssid = station_config_.ssid;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
    }
    
    case SC_EVENT_SEND_ACK_DONE:
      ESP_LOGI(TAG, "SmartConfig send ACK done");
      xEventGroupSetBits(m_event_group, WIFI_SMARTCONFIG_BIT);
      esp_smartconfig_stop();
      m_smartconfig_active = false;
      
      if (m_event_callback) {
        hf_wifi_event_t hf_event = {};
        hf_event.type = hf_wifi_event_tType::SMARTCONFIG_COMPLETED;
        m_event_callback(hf_event, m_event_user_data);
      }
      break;
      
    default:
      ESP_LOGD(TAG, "Unhandled SmartConfig event: %ld", event_id);
      break;
  }
}

hf_wifi_err_t EspWifi::ConvertEspError(esp_err_t esp_err) const {
  switch (esp_err) {
    case ESP_OK:
      return hf_wifi_err_t::HF_WIFI_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_INVALID_STATE:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_STATE;
    case ESP_ERR_NO_MEM:
      return hf_wifi_err_t::HF_WIFI_ERR_NO_MEMORY;
    case ESP_ERR_TIMEOUT:
      return hf_wifi_err_t::HF_WIFI_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_wifi_err_t::HF_WIFI_ERR_NETWORK_NOT_FOUND;
    case ESP_ERR_NOT_SUPPORTED:
      return hf_wifi_err_t::HF_WIFI_ERR_OPERATION_NOT_SUPPORTED;
    case ESP_ERR_WIFI_BASE:
      return hf_wifi_err_t::HF_WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_NOT_INIT:
      return hf_wifi_err_t::HF_WIFI_ERR_INIT_FAILED;
    case ESP_ERR_WIFI_NOT_STARTED:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_STATE;
    case ESP_ERR_WIFI_NOT_STOPPED:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_STATE;
    case ESP_ERR_WIFI_IF:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_MODE:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_STATE:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_STATE;
    case ESP_ERR_WIFI_CONN:
      return hf_wifi_err_t::HF_WIFI_ERR_CONNECTION_FAILED;
    case ESP_ERR_WIFI_NVS:
      return hf_wifi_err_t::HF_WIFI_ERR_STORAGE_ERROR;
    case ESP_ERR_WIFI_MAC:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_SSID:
      return hf_wifi_err_t::HF_WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_PASSWORD:
      return hf_wifi_err_t::HF_WIFI_ERR_AUTHENTICATION_FAILED;
    case ESP_ERR_WIFI_TIMEOUT:
      return hf_wifi_err_t::HF_WIFI_ERR_TIMEOUT;
    case ESP_ERR_WIFI_WAKE_FAIL:
      return hf_wifi_err_t::HF_WIFI_ERR_POWER_MANAGEMENT_ERROR;
    case ESP_ERR_WIFI_WOULD_BLOCK:
      return hf_wifi_err_t::HF_WIFI_ERR_BUSY;
    case ESP_ERR_WIFI_NOT_CONNECT:
      return hf_wifi_err_t::HF_WIFI_ERR_NOT_CONNECTED;
    default:
      return hf_wifi_err_t::HF_WIFI_ERR_FAILURE;
  }
}
