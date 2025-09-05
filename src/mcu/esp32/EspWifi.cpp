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

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include <algorithm>
#include <cstring>

static const char* TAG = "EspWifi";

// Event group bits for WiFi events
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_SCAN_DONE_BIT BIT2
#define WIFI_AP_STARTED_BIT BIT3
#define WIFI_SMARTCONFIG_BIT BIT4

// Global WiFi initialization flag to prevent multiple initializations
static std::atomic<bool> g_wifi_initialized{false};

// Global network interface creation flags
static std::atomic<bool> g_sta_netif_created{false};
static std::atomic<bool> g_ap_netif_created{false};

/**
 * @brief Ensure default network interfaces are created only once globally
 * @return hf_wifi_err_t::WIFI_SUCCESS on success, error code otherwise
 */
static hf_wifi_err_t ensureDefaultNetifs() {
  static std::atomic<bool> netifs_initialized{false};

  if (netifs_initialized.load()) {
    return hf_wifi_err_t::WIFI_SUCCESS;
  }

  // Try to get existing interfaces first
  esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  // Create only if they don't exist
  if (!sta_netif) {
    sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
      ESP_LOGE(TAG, "Failed to create default STA netif");
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    }
  }

  if (!ap_netif) {
    ap_netif = esp_netif_create_default_wifi_ap();
    if (!ap_netif) {
      ESP_LOGE(TAG, "Failed to create default AP netif");
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    }
  }

  netifs_initialized.store(true);
  return hf_wifi_err_t::WIFI_SUCCESS;
}

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
[[maybe_unused]] static hf_wifi_mode_t ConvertFromEspMode(wifi_mode_t mode) {
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
[[maybe_unused]] static wifi_auth_mode_t ConvertToEspAuthMode(hf_wifi_security_t security) {
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
[[maybe_unused]] static hf_wifi_security_t ConvertFromEspAuthMode(wifi_auth_mode_t auth_mode) {
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

  // Initialize NVS if needed (only once globally)
  if (!g_wifi_initialized) {
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

    g_wifi_initialized = true;
  }

  // Create event group for WiFi events
  m_event_group = xEventGroupCreate();
  if (!m_event_group) {
    ESP_LOGE(TAG, "Failed to create event group");
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Ensure default network interfaces are created only once globally
  hf_wifi_err_t netif_err = ensureDefaultNetifs();
  if (netif_err != hf_wifi_err_t::WIFI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to ensure default netifs");
    DeinitNetif();
    return netif_err;
  }

  // Get references to the default network interfaces
  m_sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  m_ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  if (!m_sta_netif || !m_ap_netif) {
    ESP_LOGE(TAG, "Failed to get default netif handles");
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

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

  esp_err_t ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Register event handlers (unregister first to avoid duplicates)
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::wifiEventHandler);
  ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::wifiEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::ipEventHandler);
  ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::ipEventHandler, this);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

  // Initialize SmartConfig if enabled
  if (m_advanced_config.enable_smartconfig) {
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::smartconfigEventHandler);
    ret = esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::smartconfigEventHandler,
                                     this);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register SmartConfig event handler: %s", esp_err_to_name(ret));
      DeinitNetif();
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    }
  }

  // Set WiFi mode
  wifi_mode_t esp_mode = ConvertToEspMode(mode);
  ret = esp_wifi_set_mode(esp_mode);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
    DeinitNetif();
    return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
  }

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

  // Note: We don't call esp_wifi_deinit() here because it can cause crashes
  // if other instances are still using the WiFi system. The WiFi system
  // will be cleaned up when the last instance is destroyed.

  // Unregister event handlers (ignore errors as they may already be unregistered)
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifi::wifiEventHandler);
  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifi::ipEventHandler);

  if (m_advanced_config.enable_smartconfig) {
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &EspWifi::smartconfigEventHandler);
  }

  // Clean up event group
  if (m_event_group) {
    vEventGroupDelete(m_event_group);
    m_event_group = nullptr;
  }

  DeinitNetif();

  // Reset all state variables
  m_initialized = false;
  m_enabled = false;
  m_mode = hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
  m_connected = false;
  m_ap_active = false;
  m_scanning = false;
  m_rssi = 0;
  m_channel = 0;
  m_event_callback = nullptr;
  m_scan_callback = nullptr;
  m_event_user_data = nullptr;
  m_scan_user_data = nullptr;

  // Clear scan results
  {
    RtosLockGuard<RtosMutex> scan_lock(m_scan_mutex);
    m_scan_results.clear();
  }

  // Clear event queue
  {
    RtosLockGuard<RtosMutex> event_lock(m_event_mutex);
    while (!m_event_queue.empty()) {
      m_event_queue.pop();
    }
  }

  // Add a small delay to ensure all resources are properly cleaned up
  vTaskDelay(pdMS_TO_TICKS(100));

  // Note: We don't reset g_wifi_initialized here because other instances
  // might still be using the global WiFi infrastructure

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

  // Stop WiFi first
  esp_err_t err = esp_wifi_stop();
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to stop WiFi during mode change: %s", esp_err_to_name(err));
  }

  // Set new mode
  wifi_mode_t esp_mode = ConvertToEspMode(mode);
  err = esp_wifi_set_mode(esp_mode);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(err));
    return ConvertEspError(err);
  }

  // Restart WiFi
  err = esp_wifi_start();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to restart WiFi after mode change: %s", esp_err_to_name(err));
    return ConvertEspError(err);
  }

  // Update internal mode
  m_mode = mode;

  ESP_LOGI(TAG, "WiFi mode changed successfully");
  return hf_wifi_err_t::WIFI_SUCCESS;
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
  // Convert positive threshold_rssi to negative (ESP-IDF expects negative values)
  wifi_config.sta.threshold.rssi = -static_cast<int8_t>(config.threshold_rssi);
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

  // Ensure we're in AP or APSTA mode
  wifi_mode_t current_mode;
  esp_err_t err = esp_wifi_get_mode(&current_mode);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  if (current_mode != WIFI_MODE_AP && current_mode != WIFI_MODE_APSTA) {
    ESP_LOGE(TAG, "WiFi not in AP mode, current mode: %d", current_mode);
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  // AP is automatically started when WiFi is started in AP mode
  // Just update our internal state
  m_ap_active = true;

  ESP_LOGI(TAG, "Access Point started successfully");
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::StopAccessPoint() {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  // To stop AP, we need to change mode to STA only or NULL
  wifi_mode_t current_mode;
  esp_err_t err = esp_wifi_get_mode(&current_mode);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  if (current_mode == WIFI_MODE_AP) {
    // Switch to NULL mode to stop AP
    err = esp_wifi_set_mode(WIFI_MODE_NULL);
    if (err != ESP_OK) {
      return ConvertEspError(err);
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
      return ConvertEspError(err);
    }
  } else if (current_mode == WIFI_MODE_APSTA) {
    // Switch to STA mode to stop AP but keep STA
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
      return ConvertEspError(err);
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
      return ConvertEspError(err);
    }
  }

  m_ap_active = false;

  ESP_LOGI(TAG, "Access Point stopped successfully");
  return hf_wifi_err_t::WIFI_SUCCESS;
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
    ESP_LOGW(TAG, "Scan already in progress");
    return hf_wifi_err_t::WIFI_ERR_FAILURE;
  }

  // Ensure we're in STA or APSTA mode for scanning
  wifi_mode_t current_mode;
  esp_err_t err = esp_wifi_get_mode(&current_mode);
  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  if (current_mode != WIFI_MODE_STA && current_mode != WIFI_MODE_APSTA) {
    ESP_LOGE(TAG, "WiFi not in STA mode, cannot scan. Current mode: %d", current_mode);
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  wifi_scan_config_t scan_config = {};
  scan_config.show_hidden = show_hidden;
  scan_config.scan_type = passive ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;

  // Set scan time - convert ms to seconds, with reasonable limits
  uint32_t scan_time_sec = (duration_ms > 0) ? (duration_ms / 1000) : 5;
  if (scan_time_sec < 1)
    scan_time_sec = 1;
  if (scan_time_sec > 10)
    scan_time_sec = 10;

  scan_config.scan_time.active.min = scan_time_sec;
  scan_config.scan_time.active.max = scan_time_sec;
  scan_config.scan_time.passive = scan_time_sec;

  ESP_LOGI(TAG, "Starting WiFi scan (hidden: %s, passive: %s, duration: %dms)",
           show_hidden ? "yes" : "no", passive ? "yes" : "no", duration_ms);

  err = esp_wifi_scan_start(&scan_config, false);
  if (err == ESP_OK) {
    m_scanning = true;
    ESP_LOGI(TAG, "WiFi scan started successfully");
  } else {
    ESP_LOGE(TAG, "Failed to start WiFi scan: %s", esp_err_to_name(err));
  }
  return ConvertEspError(err);
}

hf_wifi_err_t EspWifi::GetScanResults(std::vector<hf_wifi_network_info_t>& networks,
                                      uint16_t max_count) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  // Clear the output vector
  networks.clear();

  // Get the number of available scan results
  uint16_t scan_count = 0;
  esp_err_t err = esp_wifi_scan_get_ap_num(&scan_count);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get scan result count: %s", esp_err_to_name(err));
    return ConvertEspError(err);
  }

  ESP_LOGI(TAG, "Found %d scan results", scan_count);

  if (scan_count == 0) {
    ESP_LOGI(TAG, "No networks found in scan results");
    return hf_wifi_err_t::WIFI_SUCCESS;
  }

  // Limit the number of results if requested
  if (max_count > 0 && scan_count > max_count) {
    scan_count = max_count;
  }

  // Allocate memory for scan results
  wifi_ap_record_t* scan_results = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * scan_count);
  if (!scan_results) {
    ESP_LOGE(TAG, "Failed to allocate memory for scan results");
    return hf_wifi_err_t::WIFI_ERR_NO_MEMORY;
  }

  // Get the actual scan results
  err = esp_wifi_scan_get_ap_records(&scan_count, scan_results);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get scan records: %s", esp_err_to_name(err));
    free(scan_results);
    return ConvertEspError(err);
  }

  // Convert ESP-IDF scan results to our format
  networks.reserve(scan_count);
  for (uint16_t i = 0; i < scan_count; i++) {
    hf_wifi_network_info_t network;

    // Copy SSID (ensure null termination)
    char ssid_str[33] = {0}; // 32 chars + null terminator
    strncpy(ssid_str, (char*)scan_results[i].ssid, sizeof(ssid_str) - 1);
    network.ssid = std::string(ssid_str);

    // Copy BSSID
    memcpy(network.bssid, scan_results[i].bssid, 6);

    // Convert security type
    network.security = ConvertFromEspAuthMode(scan_results[i].authmode);

    // Copy other fields
    network.rssi = scan_results[i].rssi;
    network.channel = scan_results[i].primary;
    network.hidden = (network.ssid.empty()); // Consider empty SSID as hidden

    networks.push_back(network);
  }

  // Free allocated memory
  free(scan_results);

  ESP_LOGI(TAG, "Successfully retrieved %zu network results", networks.size());
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
  // Note: We don't destroy default netifs as they are managed by ESP-IDF
  // and destroying them can cause crashes when other components try to use them
  // Just set pointers to nullptr to avoid dangling references
  m_sta_netif = nullptr;
  m_ap_netif = nullptr;
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
  ESP_LOGI(TAG, "WiFi event received: %d", event_id);

  switch (event_id) {
  case WIFI_EVENT_STA_START:
    ESP_LOGI(TAG, "WiFi station started");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
    break;

  case WIFI_EVENT_STA_STOP:
    ESP_LOGI(TAG, "WiFi station stopped");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
    m_connected = false;
    break;

  case WIFI_EVENT_STA_CONNECTED:
    ESP_LOGI(TAG, "WiFi station connected to AP");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
    m_connected = true;
    break;

  case WIFI_EVENT_STA_DISCONNECTED:
    ESP_LOGI(TAG, "WiFi station disconnected from AP");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
    m_connected = false;
    break;

  case WIFI_EVENT_AP_START:
    ESP_LOGI(TAG, "WiFi Access Point started");
    m_ap_active = true;
    break;

  case WIFI_EVENT_AP_STOP:
    ESP_LOGI(TAG, "WiFi Access Point stopped");
    m_ap_active = false;
    break;

  case WIFI_EVENT_AP_STACONNECTED:
    ESP_LOGI(TAG, "Station connected to our AP");
    break;

  case WIFI_EVENT_AP_STADISCONNECTED:
    ESP_LOGI(TAG, "Station disconnected from our AP");
    break;

  case WIFI_EVENT_SCAN_DONE:
    ESP_LOGI(TAG, "WiFi scan completed");
    m_scanning = false;
    break;

  case WIFI_EVENT_AP_PROBEREQRECVED:
    // Probe request received - this is normal for AP mode
    ESP_LOGD(TAG, "Probe request received");
    break;

  default:
    ESP_LOGW(TAG, "Unhandled WiFi event: %d", event_id);
    break;
  }

  // Notify user callback if registered
  if (m_event_callback) {
    m_event_callback(static_cast<hf_wifi_event_t>(event_id), event_data);
  }
}

void EspWifi::handleIpEvent(int32_t event_id, void* event_data) {
  ESP_LOGI(TAG, "IP event received: %d", event_id);

  switch (event_id) {
  case IP_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "Station got IP address");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_CONNECTED;
    m_connected = true;
    break;

  case IP_EVENT_STA_LOST_IP:
    ESP_LOGI(TAG, "Station lost IP address");
    m_state = hf_wifi_state_t::HF_WIFI_STATE_DISCONNECTED;
    m_connected = false;
    break;

  default:
    ESP_LOGW(TAG, "Unhandled IP event: %d", event_id);
    break;
  }

  // Notify user callback if registered
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
  case hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_ENTERPRISE:
    return WIFI_AUTH_WPA3_ENTERPRISE;
  case hf_wifi_security_t::HF_WIFI_SECURITY_WAPI_PSK:
    return WIFI_AUTH_WAPI_PSK;
  default:
    ESP_LOGW(TAG, "Unknown security type: %d, defaulting to OPEN", static_cast<int>(security));
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
  case WIFI_AUTH_WPA3_ENTERPRISE:
    return hf_wifi_security_t::HF_WIFI_SECURITY_WPA3_ENTERPRISE;
  case WIFI_AUTH_WAPI_PSK:
    return hf_wifi_security_t::HF_WIFI_SECURITY_WAPI_PSK;
  default:
    ESP_LOGW(TAG, "Unknown auth mode: %d, defaulting to OPEN", auth_mode);
    return hf_wifi_security_t::HF_WIFI_SECURITY_OPEN;
  }
}

//==============================================================================
// MISSING METHOD IMPLEMENTATIONS FOR FUNCTIONAL TESTS
//==============================================================================

hf_wifi_err_t EspWifi::GetAdvancedConfig(EspWifiAdvancedConfig& config) const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  config = m_advanced_config;
  return hf_wifi_err_t::WIFI_SUCCESS;
}

hf_wifi_err_t EspWifi::SetTxPower(uint8_t power) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  if (power > 20) {
    return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
  }

  int8_t power_quarter_dbm = power * 4; // Convert to 0.25dBm units
  esp_err_t err = esp_wifi_set_max_tx_power(power_quarter_dbm);
  if (err == ESP_OK) {
    m_advanced_config.tx_power = power;
  }
  return ConvertEspError(err);
}

int8_t EspWifi::GetTxPower() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return -1;
  }

  return static_cast<int8_t>(m_advanced_config.tx_power);
}

hf_wifi_err_t EspWifi::SetBandwidth(wifi_bandwidth_t bandwidth) {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
  }

  m_advanced_config.bandwidth = bandwidth;
  return hf_wifi_err_t::WIFI_SUCCESS;
}

wifi_bandwidth_t EspWifi::GetBandwidth() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);

  if (!m_initialized) {
    return WIFI_BW_HT20;
  }

  return m_advanced_config.bandwidth;
}
