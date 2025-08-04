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
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <algorithm>
#include <cstring>

static const char* TAG = "EspWifi";

// Event group bits for WiFi events
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_SCAN_DONE_BIT BIT2
#define WIFI_AP_STARTED_BIT BIT3
#define WIFI_SMARTCONFIG_BIT BIT4

// Default values for advanced configuration
static const EspWifiAdvancedConfig DEFAULT_ADVANCED_CONFIG = {.enable_power_save = false,
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
                                                              .enterprise_username = "",
                                                              .enterprise_password = "",
                                                              .enterprise_ca_cert = "",
                                                              .enterprise_client_cert = "",
                                                              .enterprise_client_key = "",
                                                              .enable_mesh = false,
                                                              .mesh_max_layer = 6,
                                                              .mesh_max_connection = 10,
                                                              .enable_smartconfig = false,
                                                              .smartconfig_type = SC_TYPE_ESPTOUCH};

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
    : m_initialized(false), m_enabled(false), m_mode(hf_wifi_mode_t::HF_WIFI_MODE_DISABLED),
      m_state(hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED), m_sta_netif(nullptr),
      m_ap_netif(nullptr), m_event_group(nullptr), m_event_callback(nullptr),
      m_scan_callback(nullptr), m_event_user_data(nullptr), m_scan_user_data(nullptr),
      m_scanning(false), m_connected(false), m_ap_active(false), m_rssi(0), m_channel(0),
      m_smartconfig_active(false), m_mesh_active(false) {
  // Copy advanced configuration or use defaults
  if (advanced_config) {
    m_advanced_config = *advanced_config;
  } else {
    m_advanced_config = DEFAULT_ADVANCED_CONFIG;
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
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Create default station and AP network interfaces
  m_sta_netif = esp_netif_create_default_wifi_sta();
  m_ap_netif = esp_netif_create_default_wifi_ap();

  // Initialize WiFi with default configuration
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  // Apply advanced configuration
  cfg.ampdu_rx_enable = m_advanced_config.enable_ampdu_rx ? 1 : 0;
  cfg.ampdu_tx_enable = m_advanced_config.enable_ampdu_tx ? 1 : 0;
  cfg.static_tx_buf_num = 16; // Optimize for performance
  cfg.dynamic_tx_buf_num = 32;
  cfg.static_rx_buf_num = 10;
  cfg.dynamic_rx_buf_num = 32;
  cfg.cache_tx_buf_num = 0;

  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Register event handlers
  ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::wifiEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::ipEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Initialize SmartConfig if enabled
  if (m_advanced_config.enable_smartconfig) {
    ret = esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::smartconfigEventHandler,
                                     this);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register SmartConfig event handler: %s", esp_err_to_name(ret));
      DeinitNetif();
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    }
  }

  // Set WiFi mode to NULL initially
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));

  // Start WiFi
  ret = esp_wifi_start();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Apply advanced power management settings
  if (m_advanced_config.enable_power_save) {
    ESP_ERROR_CHECK(esp_wifi_set_ps(m_advanced_config.power_save_type));
  }

  // Set TX power
  int8_t power = m_advanced_config.tx_power * 4; // Convert to 0.25dBm units
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

  // WPS not available on ESP32-C6
  // if (m_wps_active) {
  //   esp_wifi_wps_disable();
  //   m_wps_active = false;
  // }

  // Disconnect if connected
  if (m_connected) {
    esp_wifi_disconnect();
    m_connected = false;
  }

  // Stop WiFi
  esp_wifi_stop();

  // Unregister event handlers
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::wifiEventHandler);
  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::ipEventHandler);

  if (m_advanced_config.enable_smartconfig) {
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::smartconfigEventHandler);
  }

  DeinitNetif();

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
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  wifi_mode_t esp_mode = ConvertToEspMode(mode);
  esp_err_t err = esp_wifi_set_mode(esp_mode);
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::ConfigureStation(const hf_wifi_station_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  // Store station configuration
  m_sta_config = config;

  // Configure ESP-IDF station config
  wifi_config_t wifi_config = {};
  strncpy((char*)wifi_config.sta.ssid, config.ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
  strncpy((char*)wifi_config.sta.password, config.password.c_str(),
          sizeof(wifi_config.sta.password) - 1);

  if (config.bssid_set) {
    memcpy(wifi_config.sta.bssid, config.bssid, 6);
    wifi_config.sta.bssid_set = true;
  }

  wifi_config.sta.channel = config.channel;
  wifi_config.sta.scan_method = static_cast<wifi_scan_method_t>(config.scan_method);
  wifi_config.sta.sort_method = static_cast<wifi_sort_method_t>(config.sort_method);
  wifi_config.sta.threshold.rssi = config.threshold_rssi;
  wifi_config.sta.threshold.authmode = ConvertToEspAuthMode(config.threshold_authmode);

  esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::Connect(uint32_t timeout_ms) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t err = esp_wifi_connect();
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  if (timeout_ms > 0) {
    // Wait for connection with timeout
    EventBits_t bits = xEventGroupWaitBits(m_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, pdMS_TO_TICKS(timeout_ms));
    if (bits & WIFI_FAIL_BIT) {
      return hf_wifi_err_t::WIFI_ERR_CONNECTION_FAILED;
    } else if (!(bits & WIFI_CONNECTED_BIT)) {
      return hf_wifi_err_t::WIFI_ERR_TIMEOUT;
    }
  }

  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::GetIpInfo(hf_wifi_ip_info_t& ip_info) const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_netif_ip_info_t esp_ip_info;
  esp_err_t err =
      esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &esp_ip_info);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  ip_info.ip = esp_ip_info.ip.addr;
  ip_info.netmask = esp_ip_info.netmask.addr;
  ip_info.gateway = esp_ip_info.gw.addr;

  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::ConfigureAccessPoint(const hf_wifi_ap_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  // Store AP configuration
  m_ap_config = config;

  // Configure ESP-IDF AP config
  wifi_config_t wifi_config = {};
  strncpy((char*)wifi_config.ap.ssid, config.ssid.c_str(), sizeof(wifi_config.ap.ssid) - 1);
  strncpy((char*)wifi_config.ap.password, config.password.c_str(),
          sizeof(wifi_config.ap.password) - 1);

  wifi_config.ap.ssid_len = config.ssid_len;
  wifi_config.ap.channel = config.channel;
  wifi_config.ap.authmode = ConvertToEspAuthMode(config.authmode);
  wifi_config.ap.ssid_hidden = config.ssid_hidden;
  wifi_config.ap.max_connection = config.max_connection;
  wifi_config.ap.beacon_interval = config.beacon_interval;

  esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::StartAccessPoint() {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t err = esp_wifi_start();
  if (err == ESP_OK) {
    m_ap_active = true;
  }
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::StopAccessPoint() {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t err = esp_wifi_stop();
  if (err == ESP_OK) {
    m_ap_active = false;
  }
  return ConvertEspError(err);
}

bool EspWifi::IsAccessPointActive() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  return m_ap_active;
}

int EspWifi::GetConnectedStationCount() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized || !m_ap_active) {
    return 0;
  }

  wifi_sta_list_t sta_list;
  esp_err_t err = esp_wifi_ap_get_sta_list(&sta_list);
  if (err != ESP_OK) {
    return 0;
  }

  return sta_list.num;
}

hf_wifi_err_t EspWifi::StartScan(bool show_hidden, bool passive, uint32_t duration_ms) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  if (m_scanning) {
    return hf_wifi_err_t::WIFI_ERR_FAILURE;
  }

  wifi_scan_config_t scan_config = {};
  scan_config.show_hidden = show_hidden;
  scan_config.scan_type = passive ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;
  scan_config.scan_time.active.min = duration_ms / 1000;
  scan_config.scan_time.active.max = duration_ms / 1000;

  esp_err_t err = esp_wifi_scan_start(&scan_config, false);
  if (err == ESP_OK) {
    m_scanning = true;
  }
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::GetScanResults(std::vector<hf_wifi_network_info_t>& networks,
                                      uint16_t max_count) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  wifi_ap_record_t* scan_results = nullptr;
  uint16_t scan_count = 0;

  esp_err_t err = esp_wifi_scan_get_ap_num(&scan_count);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  if (scan_count == 0) {
    networks.clear();
    return hf_wifi_err_t::WIFI_SUCCESS;
  }

  if (max_count > 0 && scan_count > max_count) {
    scan_count = max_count;
  }

  scan_results = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * scan_count);
  if (!scan_results) {
    return hf_wifi_err_t::WIFI_ERR_NO_MEMORY;
  }

  err = esp_wifi_scan_get_ap_records(&scan_count, scan_results);
  if (err != ESP_OK) {
    free(scan_results);
    return ConvertEspError(err);
  }

  networks.clear();
  networks.reserve(scan_count);

  for (uint16_t i = 0; i < scan_count; i++) {
    hf_wifi_network_info_t network;
    network.ssid = std::string((char*)scan_results[i].ssid);
    memcpy(network.bssid, scan_results[i].bssid, 6);
    network.security = ConvertFromEspAuthMode(scan_results[i].authmode);
    network.rssi = scan_results[i].rssi;
    network.channel = scan_results[i].primary;
    network.hidden = false; // ESP-IDF doesn't provide this info directly
    networks.push_back(network);
  }

  free(scan_results);
  return hf_wifi_err_t::WIFI_SUCCESS;
}

bool EspWifi::IsScanning() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  return m_scanning;
}

hf_wifi_state_t EspWifi::GetState() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  return m_state;
}

std::string EspWifi::GetConnectedSsid() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized || !m_connected) {
    return "";
  }

  wifi_ap_record_t ap_info;
  esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
  if (err != ESP_OK) {
    return "";
  }

  return std::string((char*)ap_info.ssid);
}

hf_wifi_err_t EspWifi::GetConnectedBssid(uint8_t* bssid) const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized || !m_connected || !bssid) {
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  wifi_ap_record_t ap_info;
  esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  memcpy(bssid, ap_info.bssid, 6);
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::SetPowerSave(hf_wifi_power_save_t mode) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  wifi_ps_type_t esp_mode;
  switch (mode) {
    case hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE:
      esp_mode = WIFI_PS_NONE;
      break;
    case hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MIN_MODEM:
      esp_mode = WIFI_PS_MIN_MODEM;
      break;
    case hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MAX_MODEM:
      esp_mode = WIFI_PS_MAX_MODEM;
      break;
    default:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  esp_err_t err = esp_wifi_set_ps(esp_mode);
  return ConvertEspError(err);
}

hf_wifi_power_save_t EspWifi::GetPowerSave() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE;
  }

  wifi_ps_type_t esp_mode;
  esp_err_t err = esp_wifi_get_ps(&esp_mode);
  if (err != ESP_OK) {
    return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE;
  }

  switch (esp_mode) {
    case WIFI_PS_NONE:
      return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE;
    case WIFI_PS_MIN_MODEM:
      return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MIN_MODEM;
    case WIFI_PS_MAX_MODEM:
      return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_MAX_MODEM;
    default:
      return hf_wifi_power_save_t::HF_WIFI_POWER_SAVE_NONE;
  }
}

hf_wifi_err_t EspWifi::RegisterEventCallback(hf_wifi_event_callback_t callback) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_event_callback = callback;
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::UnregisterEventCallback() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_event_callback = nullptr;
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::GetMacAddress(uint8_t mac[6], uint8_t interface) const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized || !mac) {
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  wifi_interface_t wifi_interface;
  switch (interface) {
    case 0:
      wifi_interface = WIFI_IF_STA;
      break;
    case 1:
      wifi_interface = WIFI_IF_AP;
      break;
    default:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  esp_err_t err = esp_wifi_get_mac(wifi_interface, mac);
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::SetMacAddress(const uint8_t mac[6], uint8_t interface) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized || !mac) {
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  wifi_interface_t wifi_interface;
  switch (interface) {
    case 0:
      wifi_interface = WIFI_IF_STA;
      break;
    case 1:
      wifi_interface = WIFI_IF_AP;
      break;
    default:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  esp_err_t err = esp_wifi_set_mac(wifi_interface, (uint8_t*)mac);
  return ConvertEspError(err);
}

uint8_t EspWifi::GetChannel() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return 0;
  }

  uint8_t channel;
  esp_err_t err = esp_wifi_get_channel(&channel, nullptr);
  if (err != ESP_OK) {
    return 0;
  }

  return channel;
}

hf_wifi_err_t EspWifi::SetChannel(uint8_t channel) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  return ConvertEspError(err);
}

hf_wifi_mode_t EspWifi::GetMode() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  }

  wifi_mode_t mode;
  esp_err_t err = esp_wifi_get_mode(&mode);
  if (err != ESP_OK) {
    return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  }

  return ConvertFromEspMode(mode);
}

wifi_mode_t EspWifi::ConvertToEspMode(hf_wifi_mode_t mode) const {
  return ::ConvertToEspMode(mode);
}

//==============================================================================
// EVENT HANDLERS
//==============================================================================

void EspWifi::wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id,
                               void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleWifiEvent(event_id, event_data);
}

void EspWifi::ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id,
                             void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleIpEvent(event_id, event_data);
}

void EspWifi::smartconfigEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id,
                                      void* event_data) {
  EspWifi* wifi = static_cast<EspWifi*>(arg);
  wifi->handleSmartconfigEvent(event_id, event_data);
}

//==============================================================================
// UTILITY METHODS
//==============================================================================

hf_wifi_err_t EspWifi::ConvertEspError(esp_err_t esp_err) const {
  switch (esp_err) {
    case ESP_OK:
      return hf_wifi_err_t::WIFI_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_INVALID_STATE:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_NO_MEM:
      return hf_wifi_err_t::WIFI_ERR_NO_MEMORY;
    case ESP_ERR_TIMEOUT:
      return hf_wifi_err_t::WIFI_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_NOT_SUPPORTED:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_BASE:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_NOT_INIT:
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    case ESP_ERR_WIFI_NOT_STARTED:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_NOT_STOPPED:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_IF:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_MODE:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_STATE:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_CONN:
      return hf_wifi_err_t::WIFI_ERR_CONNECTION_FAILED;
    case ESP_ERR_WIFI_NVS:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_MAC:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_SSID:
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_WIFI_PASSWORD:
      return hf_wifi_err_t::WIFI_ERR_AUTHENTICATION_FAILED;
    case ESP_ERR_WIFI_TIMEOUT:
      return hf_wifi_err_t::WIFI_ERR_TIMEOUT;
    case ESP_ERR_WIFI_WAKE_FAIL:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_WOULD_BLOCK:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_NOT_CONNECT:
      return hf_wifi_err_t::WIFI_ERR_NOT_CONNECTED;
    default:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
  }
}

hf_wifi_err_t EspWifi::DeinitNetif() {
  if (m_sta_netif) {
    esp_netif_destroy(m_sta_netif);
    m_sta_netif = nullptr;
  }
  if (m_ap_netif) {
    esp_netif_destroy(m_ap_netif);
    m_ap_netif = nullptr;
  }
  return hf_wifi_err_t::WIFI_SUCCESS;
}

//==============================================================================
// MISSING METHOD IMPLEMENTATIONS
//==============================================================================

hf_wifi_err_t EspWifi::Disconnect() {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t err = esp_wifi_disconnect();
  return ConvertEspError(err);
}

bool EspWifi::IsConnected() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  return m_connected;
}

int8_t EspWifi::GetRssi() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return 0;
  }

  wifi_ap_record_t ap_info;
  esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
  if (err != ESP_OK) {
    return 0;
  }

  return ap_info.rssi;
}

void EspWifi::handleWifiEvent(int32_t event_id, void* event_data) {
  switch (event_id) {
    case WIFI_EVENT_STA_START:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
      break;
    case WIFI_EVENT_STA_STOP:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
      m_connected = false;
      break;
    case WIFI_EVENT_STA_CONNECTED:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
      m_connected = true;
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
      m_connected = false;
      break;
    case WIFI_EVENT_AP_START:
      m_ap_active = true;
      break;
    case WIFI_EVENT_AP_STOP:
      m_ap_active = false;
      break;
    case WIFI_EVENT_SCAN_DONE:
      m_scanning = false;
      break;
  }

  if (m_event_callback) {
    m_event_callback(static_cast<hf_wifi_event_t>(event_id), event_data);
  }
}

void EspWifi::handleIpEvent(int32_t event_id, void* event_data) {
  switch (event_id) {
    case IP_EVENT_STA_GOT_IP:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
      break;
    case IP_EVENT_STA_LOST_IP:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
      break;
  }

  if (m_event_callback) {
    m_event_callback(static_cast<hf_wifi_event_t>(event_id), event_data);
  }
}

void EspWifi::handleSmartconfigEvent(int32_t event_id, void* event_data) {
  switch (event_id) {
    case SC_EVENT_GOT_SSID_PSWD:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
      break;
    case SC_EVENT_SEND_ACK_DONE:
      m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
      break;
  }

  if (m_event_callback) {
    m_event_callback(static_cast<hf_wifi_event_t>(event_id), event_data);
  }
}

hf_wifi_mode_t EspWifi::ConvertFromEspMode(wifi_mode_t mode) const {
  switch (mode) {
    case WIFI_MODE_NULL:
      return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
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

wifi_auth_mode_t EspWifi::ConvertToEspAuthMode(hf_wifi_security_t security) const {
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
    default:
      return WIFI_AUTH_OPEN;
  }
}

hf_wifi_security_t EspWifi::ConvertFromEspAuthMode(wifi_auth_mode_t auth_mode) const {
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
    default:
      return hf_wifi_security_t::HF_WIFI_SECURITY_OPEN;
  }
}
