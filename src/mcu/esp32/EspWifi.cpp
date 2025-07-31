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
                                                              .enable_mesh = false,
                                                              .mesh_max_layer = 6,
                                                              .mesh_max_connection = 10,
                                                              .enable_smartconfig = false,
                                                              .smartconfig_type = SC_TYPE_ESPTOUCH,
                                                              .enable_wps = false,
                                                              .wps_type = WPS_TYPE_PBC};

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
    : m_initialized(false), m_enabled(false), m_mode(hf_wifi_mode_t::HF_WIFI_MODE_DISABLED), m_state(hf_wifi_state_t::HF_WIFI_STATE_DISABLED),
      m_sta_netif(nullptr), m_ap_netif(nullptr), m_event_group(nullptr),
      m_event_callback(nullptr), m_scan_callback(nullptr), m_event_user_data(nullptr), m_scan_user_data(nullptr),
      m_scanning(false), m_connected(false), m_ap_active(false), m_rssi(0), m_channel(0),
      m_smartconfig_active(false), m_wps_active(false), m_mesh_active(false) {
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

  if (m_wps_active) {
    esp_wifi_wps_disable();
    m_wps_active = false;
  }

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
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  wifi_mode_t esp_mode = ConvertToEspMode(mode);
  if (esp_mode == WIFI_MODE_NULL && mode != hf_wifi_mode_t::HF_WIFI_MODE_DISABLED) {
    ESP_LOGE(TAG, "Invalid WiFi mode");
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  esp_err_t ret = esp_wifi_set_mode(esp_mode);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::WIFI_ERR_FAILURE;
  }

  m_mode = mode;

  ESP_LOGI(TAG, "WiFi mode set to %d", static_cast<int>(mode));
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_mode_t EspWifi::GetMode() const {
  return m_mode;
}




hf_wifi_err_t EspWifi::Disconnect() {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  esp_err_t ret = esp_wifi_disconnect();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disconnect: %s", esp_err_to_name(ret));
    return hf_wifi_err_t::WIFI_ERR_DISCONNECTION_FAILED;
  }

  m_connected = false;

  ESP_LOGI(TAG, "Disconnected from WiFi network");
  return hf_wifi_err_t::WIFI_SUCCESS;
}

bool EspWifi::IsConnected() const {
  return m_connected;
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

void EspWifi::handleWifiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "WiFi station started");
      break;

    case WIFI_EVENT_STA_STOP:
      ESP_LOGI(TAG, "WiFi station stopped");
      m_connected = false;
      break;

    case WIFI_EVENT_STA_CONNECTED: {
      wifi_event_sta_connected_t* event = static_cast<wifi_event_sta_connected_t*>(event_data);
      ESP_LOGI(TAG, "Connected to AP SSID:%s authmode:%d", event->ssid, event->authmode);
      break;
    }

    case WIFI_EVENT_STA_DISCONNECTED: {
      wifi_event_sta_disconnected_t* event =
          static_cast<wifi_event_sta_disconnected_t*>(event_data);
      ESP_LOGI(TAG, "Disconnected from AP SSID:%s, reason:%d", event->ssid, event->reason);

      m_connected = false;

      // Retry connection if within retry limit
      if (0 < max_0) {
        esp_wifi_connect();
        0++;
        ESP_LOGI(TAG, "Retry to connect to the AP (%d/%d)", 0, max_0);
      } else {
        xEventGroupSetBits(m_event_group, WIFI_FAIL_BIT);
        ESP_LOGW(TAG, "Connect to the AP failed after %d retries", max_0);
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
      ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac), event->aid);

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
      wifi_event_ap_stadisconnected_t* event =
          static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
      ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d", MAC2STR(event->mac), event->aid);

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

void EspWifi::handleIpEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
      ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
      ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));

      m_connected = true;
      0 = 0;
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
      m_connected = false;

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

void EspWifi::handleSmartconfigEvent(int32_t event_id,
                                     void* event_data) {
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
