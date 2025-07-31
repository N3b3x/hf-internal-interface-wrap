/**
 * @file EspWifi.cpp
 * @brief ESP32-C6 WiFi 6 implementation for ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This implementation provides comprehensive WiFi 6 functionality for ESP32-C6
 * with ESP-IDF v5.5, featuring modern C++17 design patterns, thread-safe 
 * operations, and power-efficient WiFi 6 implementation.
 */

#include "mcu/esp32/EspWifi.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace esp32 {
namespace wifi {

// Static member definitions
constexpr const char* EspWifi::TAG;

// Global instance for event handling
static EspWifi* g_instance = nullptr;

/**
 * @brief WiFi event handler for ESP-IDF v5.5
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (g_instance) {
        g_instance->handleWifiEvent(event_base, event_id, event_data);
    }
}

/**
 * @brief IP event handler for ESP-IDF v5.5
 */
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data) {
    if (g_instance) {
        g_instance->handleIpEvent(event_base, event_id, event_data);
    }
}

EspWifi::EspWifi() : 
    initialized_(false),
    station_started_(false),
    ap_started_(false),
    connected_(false),
    got_ip_(false),
    current_mode_(WifiMode::NONE),
    current_power_save_(PowerSaveMode::NONE),
    current_bandwidth_(BandwidthMode::HT20),
    current_protocol_(ProtocolMode::BGN),
    scan_in_progress_(false),
    max_retry_num_(5),
    retry_num_(0),
    connection_timeout_ms_(10000) {
    
    g_instance = this;
    
    // Initialize default configurations
    initDefaultConfigurations();
}

EspWifi::~EspWifi() {
    if (initialized_) {
        deinitialize();
    }
    g_instance = nullptr;
}

esp_err_t EspWifi::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        ESP_LOGW(TAG, "WiFi already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing WiFi for ESP32-C6 with ESP-IDF v5.5");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create default WiFi station and AP interfaces
    esp_netif_sta_ = esp_netif_create_default_wifi_sta();
    esp_netif_ap_ = esp_netif_create_default_wifi_ap();
    
    if (!esp_netif_sta_ || !esp_netif_ap_) {
        ESP_LOGE(TAG, "Failed to create network interfaces");
        return ESP_FAIL;
    }
    
    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    // ESP32-C6 specific optimizations
    cfg.ampdu_rx_enable = 1;
    cfg.ampdu_tx_enable = 1;
    cfg.amsdu_tx_enable = 1;
    cfg.nvs_enable = 1;
    cfg.nano_enable = 0;  // Use full featured WiFi stack
    
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                              &wifi_event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                              &ip_event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, 
                                              &ip_event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, 
                                              &ip_event_handler, nullptr));
    
    // Set default WiFi storage to flash
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    
    // Enable WiFi 6 features on ESP32-C6
    enableWifi6Features();
    
    initialized_ = true;
    ESP_LOGI(TAG, "WiFi initialized successfully");
    
    return ESP_OK;
}

esp_err_t EspWifi::deinitialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGW(TAG, "WiFi not initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing WiFi");
    
    // Stop WiFi
    if (station_started_ || ap_started_) {
        ESP_ERROR_CHECK(esp_wifi_stop());
        station_started_ = false;
        ap_started_ = false;
    }
    
    // Unregister event handlers
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_LOST_IP, &ip_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &ip_event_handler);
    
    // Deinitialize WiFi
    ESP_ERROR_CHECK(esp_wifi_deinit());
    
    // Destroy network interfaces
    if (esp_netif_sta_) {
        esp_netif_destroy(esp_netif_sta_);
        esp_netif_sta_ = nullptr;
    }
    if (esp_netif_ap_) {
        esp_netif_destroy(esp_netif_ap_);
        esp_netif_ap_ = nullptr;
    }
    
    // Reset state
    connected_ = false;
    got_ip_ = false;
    current_mode_ = WifiMode::NONE;
    scan_results_.clear();
    connected_stations_.clear();
    
    initialized_ = false;
    ESP_LOGI(TAG, "WiFi deinitialized successfully");
    
    return ESP_OK;
}

esp_err_t EspWifi::setMode(WifiMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    wifi_mode_t esp_mode;
    switch (mode) {
        case WifiMode::STATION:
            esp_mode = WIFI_MODE_STA;
            break;
        case WifiMode::ACCESS_POINT:
            esp_mode = WIFI_MODE_AP;
            break;
        case WifiMode::STATION_AP:
            esp_mode = WIFI_MODE_APSTA;
            break;
        case WifiMode::NONE:
        default:
            esp_mode = WIFI_MODE_NULL;
            break;
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(esp_mode));
    current_mode_ = mode;
    
    ESP_LOGI(TAG, "WiFi mode set to: %d", static_cast<int>(mode));
    return ESP_OK;
}

esp_err_t EspWifi::startStation() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (station_started_) {
        ESP_LOGW(TAG, "Station already started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Starting WiFi station");
    
    // Set mode to station or station+AP if AP is running
    WifiMode new_mode = ap_started_ ? WifiMode::STATION_AP : WifiMode::STATION;
    ESP_ERROR_CHECK(setMode(new_mode));
    
    // Configure station with current settings
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config_));
    
    // Start WiFi
    if (!ap_started_) {
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    
    station_started_ = true;
    ESP_LOGI(TAG, "WiFi station started");
    
    return ESP_OK;
}

esp_err_t EspWifi::stopStation() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!station_started_) {
        ESP_LOGW(TAG, "Station not started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Stopping WiFi station");
    
    // Disconnect if connected
    if (connected_) {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }
    
    station_started_ = false;
    connected_ = false;
    got_ip_ = false;
    retry_num_ = 0;
    
    // Update mode
    if (ap_started_) {
        ESP_ERROR_CHECK(setMode(WifiMode::ACCESS_POINT));
    } else {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(setMode(WifiMode::NONE));
    }
    
    ESP_LOGI(TAG, "WiFi station stopped");
    return ESP_OK;
}

esp_err_t EspWifi::startAccessPoint() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ap_started_) {
        ESP_LOGW(TAG, "Access Point already started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Starting WiFi Access Point");
    
    // Set mode to AP or station+AP if station is running
    WifiMode new_mode = station_started_ ? WifiMode::STATION_AP : WifiMode::ACCESS_POINT;
    ESP_ERROR_CHECK(setMode(new_mode));
    
    // Configure AP with current settings
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config_));
    
    // Start WiFi
    if (!station_started_) {
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    
    ap_started_ = true;
    ESP_LOGI(TAG, "WiFi Access Point started: SSID=%s", ap_config_.ap.ssid);
    
    return ESP_OK;
}

esp_err_t EspWifi::stopAccessPoint() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!ap_started_) {
        ESP_LOGW(TAG, "Access Point not started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Stopping WiFi Access Point");
    
    ap_started_ = false;
    connected_stations_.clear();
    
    // Update mode
    if (station_started_) {
        ESP_ERROR_CHECK(setMode(WifiMode::STATION));
    } else {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(setMode(WifiMode::NONE));
    }
    
    ESP_LOGI(TAG, "WiFi Access Point stopped");
    return ESP_OK;
}

esp_err_t EspWifi::connect(const std::string& ssid, const std::string& password,
                          SecurityType security, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !station_started_) {
        ESP_LOGE(TAG, "WiFi station not ready");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid.c_str());
    
    // Configure station settings
    memset(&sta_config_, 0, sizeof(sta_config_));
    
    // Set SSID
    size_t ssid_len = std::min(ssid.length(), sizeof(sta_config_.sta.ssid) - 1);
    memcpy(sta_config_.sta.ssid, ssid.c_str(), ssid_len);
    sta_config_.sta.ssid[ssid_len] = '\0';
    
    // Set password
    if (!password.empty()) {
        size_t pass_len = std::min(password.length(), sizeof(sta_config_.sta.password) - 1);
        memcpy(sta_config_.sta.password, password.c_str(), pass_len);
        sta_config_.sta.password[pass_len] = '\0';
    }
    
    // Set security and advanced features
    configureSecurity(security, username);
    
    // WiFi 6 optimizations for ESP32-C6
    sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    sta_config_.sta.pmf_cfg.capable = true;
    sta_config_.sta.pmf_cfg.required = false;
    sta_config_.sta.bssid_set = false;
    sta_config_.sta.channel = 0;  // Auto channel selection
    
    // ESP32-C6 specific features
    sta_config_.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    sta_config_.sta.threshold.rssi = -127;  // Connect to any signal strength
    
    // Set configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config_));
    
    // Reset connection state
    connected_ = false;
    got_ip_ = false;
    retry_num_ = 0;
    
    // Start connection
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    return ESP_OK;
}

esp_err_t EspWifi::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!station_started_) {
        ESP_LOGW(TAG, "Station not started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Disconnecting from WiFi");
    
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    
    connected_ = false;
    got_ip_ = false;
    retry_num_ = 0;
    
    return ESP_OK;
}

esp_err_t EspWifi::scanNetworks(bool show_hidden, bool passive, 
                               uint32_t max_scan_time_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (scan_in_progress_) {
        ESP_LOGW(TAG, "Scan already in progress");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Starting WiFi scan");
    
    wifi_scan_config_t scan_config = {};
    scan_config.ssid = nullptr;
    scan_config.bssid = nullptr;
    scan_config.channel = 0;  // Scan all channels
    scan_config.show_hidden = show_hidden;
    scan_config.scan_type = passive ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active.min = 100;
    scan_config.scan_time.active.max = max_scan_time_ms;
    scan_config.scan_time.passive = max_scan_time_ms;
    scan_config.home_chan_dwell_time = 30;
    
    scan_in_progress_ = true;
    scan_results_.clear();
    
    esp_err_t ret = esp_wifi_scan_start(&scan_config, false);
    if (ret != ESP_OK) {
        scan_in_progress_ = false;
        ESP_LOGE(TAG, "Failed to start scan: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

std::vector<WifiScanResult> EspWifi::getScanResults() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return scan_results_;
}

esp_err_t EspWifi::configureAccessPoint(const AccessPointConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Configuring Access Point: %s", config.ssid.c_str());
    
    memset(&ap_config_, 0, sizeof(ap_config_));
    
    // Set SSID
    size_t ssid_len = std::min(config.ssid.length(), sizeof(ap_config_.ap.ssid) - 1);
    memcpy(ap_config_.ap.ssid, config.ssid.c_str(), ssid_len);
    ap_config_.ap.ssid[ssid_len] = '\0';
    ap_config_.ap.ssid_len = ssid_len;
    
    // Set password
    if (!config.password.empty()) {
        size_t pass_len = std::min(config.password.length(), sizeof(ap_config_.ap.password) - 1);
        memcpy(ap_config_.ap.password, config.password.c_str(), pass_len);
        ap_config_.ap.password[pass_len] = '\0';
        ap_config_.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ap_config_.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    // Set channel and other parameters
    ap_config_.ap.channel = config.channel;
    ap_config_.ap.max_connection = config.max_connections;
    ap_config_.ap.beacon_interval = 100;
    
    // WiFi 6 and ESP32-C6 specific settings
    ap_config_.ap.pmf_cfg.required = false;
    ap_config_.ap.pmf_cfg.capable = true;
    ap_config_.ap.ftm_responder = true;  // Fine Timing Measurement support
    
    // Apply configuration if AP is running
    if (ap_started_) {
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config_));
    }
    
    return ESP_OK;
}

esp_err_t EspWifi::setPowerSaveMode(PowerSaveMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    wifi_ps_type_t ps_type;
    switch (mode) {
        case PowerSaveMode::NONE:
            ps_type = WIFI_PS_NONE;
            break;
        case PowerSaveMode::MIN_MODEM:
            ps_type = WIFI_PS_MIN_MODEM;
            break;
        case PowerSaveMode::MAX_MODEM:
            ps_type = WIFI_PS_MAX_MODEM;
            break;
        default:
            ps_type = WIFI_PS_NONE;
            break;
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_ps(ps_type));
    current_power_save_ = mode;
    
    ESP_LOGI(TAG, "Power save mode set to: %d", static_cast<int>(mode));
    return ESP_OK;
}

esp_err_t EspWifi::setBandwidth(BandwidthMode bandwidth) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    wifi_bandwidth_t bw;
    switch (bandwidth) {
        case BandwidthMode::HT20:
            bw = WIFI_BW_HT20;
            break;
        case BandwidthMode::HT40:
            bw = WIFI_BW_HT40;
            break;
        default:
            bw = WIFI_BW_HT20;
            break;
    }
    
    // Set bandwidth for both station and AP interfaces
    if (station_started_) {
        ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, bw));
    }
    if (ap_started_) {
        ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, bw));
    }
    
    current_bandwidth_ = bandwidth;
    ESP_LOGI(TAG, "Bandwidth set to: %s", bandwidth == BandwidthMode::HT40 ? "40MHz" : "20MHz");
    
    return ESP_OK;
}

esp_err_t EspWifi::setProtocol(ProtocolMode protocol) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t protocol_bitmap = 0;
    switch (protocol) {
        case ProtocolMode::B:
            protocol_bitmap = WIFI_PROTOCOL_11B;
            break;
        case ProtocolMode::BG:
            protocol_bitmap = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G;
            break;
        case ProtocolMode::BGN:
            protocol_bitmap = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
            break;
        case ProtocolMode::BGNAX:  // WiFi 6 support on ESP32-C6
            protocol_bitmap = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | 
                             WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX;
            break;
        default:
            protocol_bitmap = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
            break;
    }
    
    // Set protocol for both interfaces
    if (station_started_) {
        ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_STA, protocol_bitmap));
    }
    if (ap_started_) {
        ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, protocol_bitmap));
    }
    
    current_protocol_ = protocol;
    ESP_LOGI(TAG, "Protocol set to: 0x%02X", protocol_bitmap);
    
    return ESP_OK;
}

WifiStatus EspWifi::getStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    WifiStatus status;
    status.initialized = initialized_;
    status.station_started = station_started_;
    status.ap_started = ap_started_;
    status.connected = connected_;
    status.got_ip = got_ip_;
    status.current_mode = current_mode_;
    status.scan_in_progress = scan_in_progress_;
    status.retry_count = retry_num_;
    status.max_retry_count = max_retry_num_;
    
    // Get additional info if initialized
    if (initialized_) {
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK && connected_) {
            status.connected_ssid = std::string(reinterpret_cast<char*>(ap_info.ssid));
            status.rssi = ap_info.rssi;
            status.channel = ap_info.primary;
            
            // Convert BSSID to string
            std::stringstream ss;
            for (int i = 0; i < 6; i++) {
                if (i > 0) ss << ":";
                ss << std::hex << std::setw(2) << std::setfill('0') << 
                      static_cast<int>(ap_info.bssid[i]);
            }
            status.bssid = ss.str();
        }
        
        // Get IP information
        if (got_ip_ && esp_netif_sta_) {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(esp_netif_sta_, &ip_info) == ESP_OK) {
                status.ip_address = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.ip));
                status.netmask = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.netmask));
                status.gateway = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.gw));
            }
        }
        
        // Get connected stations count for AP mode
        if (ap_started_) {
            wifi_sta_list_t sta_list;
            if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK) {
                status.connected_stations_count = sta_list.num;
            }
        }
    }
    
    return status;
}

NetworkInfo EspWifi::getNetworkInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    NetworkInfo info;
    
    if (!initialized_ || !esp_netif_sta_) {
        return info;
    }
    
    // Get IP information
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(esp_netif_sta_, &ip_info) == ESP_OK) {
        info.ip_address = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.ip));
        info.netmask = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.netmask));
        info.gateway = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&ip_info.gw));
    }
    
    // Get DNS information
    esp_netif_dns_info_t dns_info;
    if (esp_netif_get_dns_info(esp_netif_sta_, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
        info.primary_dns = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&dns_info.ip.u_addr.ip4));
    }
    if (esp_netif_get_dns_info(esp_netif_sta_, ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
        info.secondary_dns = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&dns_info.ip.u_addr.ip4));
    }
    
    // Get MAC address
    uint8_t mac[6];
    if (esp_wifi_get_mac(WIFI_IF_STA, mac) == ESP_OK) {
        std::stringstream ss;
        for (int i = 0; i < 6; i++) {
            if (i > 0) ss << ":";
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
        }
        info.mac_address = ss.str();
    }
    
    return info;
}

std::vector<ConnectedStation> EspWifi::getConnectedStations() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connected_stations_;
}

void EspWifi::handleWifiEvent(esp_event_base_t event_base, int32_t event_id, 
                             void* event_data) {
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi station started");
            break;
            
        case WIFI_EVENT_STA_STOP:
            ESP_LOGI(TAG, "WiFi station stopped");
            station_started_ = false;
            connected_ = false;
            got_ip_ = false;
            break;
            
        case WIFI_EVENT_STA_CONNECTED: {
            wifi_event_sta_connected_t* event = static_cast<wifi_event_sta_connected_t*>(event_data);
            ESP_LOGI(TAG, "Connected to WiFi: %s (channel %d)", 
                    event->ssid, event->channel);
            connected_ = true;
            retry_num_ = 0;
            
            if (event_callbacks_.station_connected) {
                event_callbacks_.station_connected();
            }
            break;
        }
        
        case WIFI_EVENT_STA_DISCONNECTED: {
            wifi_event_sta_disconnected_t* event = static_cast<wifi_event_sta_disconnected_t*>(event_data);
            ESP_LOGW(TAG, "Disconnected from WiFi (reason: %d)", event->reason);
            
            connected_ = false;
            got_ip_ = false;
            
            // Auto-reconnect logic
            if (retry_num_ < max_retry_num_) {
                esp_wifi_connect();
                retry_num_++;
                ESP_LOGI(TAG, "Retry connecting to WiFi (%d/%d)", retry_num_, max_retry_num_);
            } else {
                ESP_LOGE(TAG, "Failed to connect to WiFi after %d attempts", max_retry_num_);
                if (event_callbacks_.station_connection_failed) {
                    event_callbacks_.station_connection_failed();
                }
            }
            
            if (event_callbacks_.station_disconnected) {
                event_callbacks_.station_disconnected();
            }
            break;
        }
        
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WiFi Access Point started");
            break;
            
        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WiFi Access Point stopped");
            ap_started_ = false;
            connected_stations_.clear();
            break;
            
        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t* event = static_cast<wifi_event_ap_staconnected_t*>(event_data);
            ESP_LOGI(TAG, "Station connected to AP: " MACSTR " (AID %d)", 
                    MAC2STR(event->mac), event->aid);
            
            // Add to connected stations list
            ConnectedStation station;
            std::stringstream ss;
            for (int i = 0; i < 6; i++) {
                if (i > 0) ss << ":";
                ss << std::hex << std::setw(2) << std::setfill('0') << 
                      static_cast<int>(event->mac[i]);
            }
            station.mac_address = ss.str();
            station.aid = event->aid;
            station.connected_time = std::chrono::steady_clock::now();
            
            connected_stations_.push_back(station);
            
            if (event_callbacks_.ap_station_connected) {
                event_callbacks_.ap_station_connected(station);
            }
            break;
        }
        
        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t* event = static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
            ESP_LOGI(TAG, "Station disconnected from AP: " MACSTR " (AID %d)", 
                    MAC2STR(event->mac), event->aid);
            
            // Remove from connected stations list
            std::string mac_str;
            std::stringstream ss;
            for (int i = 0; i < 6; i++) {
                if (i > 0) ss << ":";
                ss << std::hex << std::setw(2) << std::setfill('0') << 
                      static_cast<int>(event->mac[i]);
            }
            mac_str = ss.str();
            
            auto it = std::remove_if(connected_stations_.begin(), connected_stations_.end(),
                [&mac_str](const ConnectedStation& station) {
                    return station.mac_address == mac_str;
                });
            connected_stations_.erase(it, connected_stations_.end());
            
            if (event_callbacks_.ap_station_disconnected) {
                ConnectedStation station;
                station.mac_address = mac_str;
                station.aid = event->aid;
                event_callbacks_.ap_station_disconnected(station);
            }
            break;
        }
        
        case WIFI_EVENT_SCAN_DONE: {
            wifi_event_sta_scan_done_t* event = static_cast<wifi_event_sta_scan_done_t*>(event_data);
            ESP_LOGI(TAG, "WiFi scan completed: %d networks found", event->number);
            
            scan_in_progress_ = false;
            processScanResults();
            
            if (event_callbacks_.scan_completed) {
                event_callbacks_.scan_completed(scan_results_);
            }
            break;
        }
        
        default:
            ESP_LOGD(TAG, "Unhandled WiFi event: %ld", event_id);
            break;
    }
}

void EspWifi::handleIpEvent(esp_event_base_t event_base, int32_t event_id, 
                           void* event_data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP: {
            ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
            ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
            
            got_ip_ = true;
            
            if (event_callbacks_.got_ip) {
                std::string ip = ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&event->ip_info.ip));
                event_callbacks_.got_ip(ip);
            }
            break;
        }
        
        case IP_EVENT_STA_LOST_IP:
            ESP_LOGW(TAG, "Lost IP address");
            got_ip_ = false;
            
            if (event_callbacks_.lost_ip) {
                event_callbacks_.lost_ip();
            }
            break;
            
        case IP_EVENT_AP_STAIPASSIGNED: {
            ip_event_ap_staipassigned_t* event = static_cast<ip_event_ap_staipassigned_t*>(event_data);
            ESP_LOGI(TAG, "Assigned IP to station: " IPSTR, IP2STR(&event->ip));
            break;
        }
        
        default:
            ESP_LOGD(TAG, "Unhandled IP event: %ld", event_id);
            break;
    }
}

void EspWifi::processScanResults() {
    uint16_t number = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&number));
    
    if (number == 0) {
        ESP_LOGW(TAG, "No networks found in scan");
        return;
    }
    
    std::vector<wifi_ap_record_t> ap_records(number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_records.data()));
    
    scan_results_.clear();
    scan_results_.reserve(number);
    
    for (const auto& record : ap_records) {
        WifiScanResult result;
        
        result.ssid = std::string(reinterpret_cast<const char*>(record.ssid));
        result.rssi = record.rssi;
        result.channel = record.primary;
        result.secondary_channel = record.second;
        
        // Convert BSSID to string
        std::stringstream ss;
        for (int i = 0; i < 6; i++) {
            if (i > 0) ss << ":";
            ss << std::hex << std::setw(2) << std::setfill('0') << 
                  static_cast<int>(record.bssid[i]);
        }
        result.bssid = ss.str();
        
        // Convert auth mode
        switch (record.authmode) {
            case WIFI_AUTH_OPEN:
                result.security = SecurityType::OPEN;
                break;
            case WIFI_AUTH_WEP:
                result.security = SecurityType::WEP;
                break;
            case WIFI_AUTH_WPA_PSK:
                result.security = SecurityType::WPA_PSK;
                break;
            case WIFI_AUTH_WPA2_PSK:
                result.security = SecurityType::WPA2_PSK;
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                result.security = SecurityType::WPA_WPA2_PSK;
                break;
            case WIFI_AUTH_WPA3_PSK:
                result.security = SecurityType::WPA3_PSK;
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                result.security = SecurityType::WPA2_WPA3_PSK;
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                result.security = SecurityType::WPA2_ENTERPRISE;
                break;
            case WIFI_AUTH_WPA3_ENTERPRISE:
                result.security = SecurityType::WPA3_ENTERPRISE;
                break;
            default:
                result.security = SecurityType::UNKNOWN;
                break;
        }
        
        // WiFi 6 features detection
        result.wifi6_support = (record.phy_11ax == 1);
        result.bandwidth_40mhz = (record.phy_11n == 1 || record.phy_11ax == 1);
        
        scan_results_.push_back(result);
    }
    
    // Sort by signal strength (strongest first)
    std::sort(scan_results_.begin(), scan_results_.end(),
        [](const WifiScanResult& a, const WifiScanResult& b) {
            return a.rssi > b.rssi;
        });
    
    ESP_LOGI(TAG, "Processed %d scan results", scan_results_.size());
}

void EspWifi::initDefaultConfigurations() {
    // Initialize default station configuration
    memset(&sta_config_, 0, sizeof(sta_config_));
    sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    sta_config_.sta.pmf_cfg.capable = true;
    sta_config_.sta.pmf_cfg.required = false;
    sta_config_.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    sta_config_.sta.threshold.rssi = -127;
    
    // Initialize default AP configuration
    memset(&ap_config_, 0, sizeof(ap_config_));
    strcpy(reinterpret_cast<char*>(ap_config_.ap.ssid), "ESP32-C6-AP");
    ap_config_.ap.ssid_len = strlen("ESP32-C6-AP");
    strcpy(reinterpret_cast<char*>(ap_config_.ap.password), "password123");
    ap_config_.ap.channel = 1;
    ap_config_.ap.max_connection = 4;
    ap_config_.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config_.ap.pmf_cfg.required = false;
    ap_config_.ap.pmf_cfg.capable = true;
    ap_config_.ap.ftm_responder = true;
}

void EspWifi::configureSecurity(SecurityType security, const std::string& username) {
    switch (security) {
        case SecurityType::OPEN:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_OPEN;
            break;
        case SecurityType::WEP:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WEP;
            break;
        case SecurityType::WPA_PSK:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
            break;
        case SecurityType::WPA2_PSK:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            break;
        case SecurityType::WPA_WPA2_PSK:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
            break;
        case SecurityType::WPA3_PSK:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK;
            break;
        case SecurityType::WPA2_WPA3_PSK:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_WPA3_PSK;
            break;
        case SecurityType::WPA2_ENTERPRISE:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE;
            // Enterprise configuration would go here
            break;
        case SecurityType::WPA3_ENTERPRISE:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA3_ENTERPRISE;
            // Enterprise configuration would go here
            break;
        default:
            sta_config_.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            break;
    }
    
    // Configure PMF (Protected Management Frames) for WPA3
    if (security == SecurityType::WPA3_PSK || security == SecurityType::WPA3_ENTERPRISE) {
        sta_config_.sta.pmf_cfg.required = true;
    }
}

esp_err_t EspWifi::enableWifi6Features() {
    ESP_LOGI(TAG, "Enabling WiFi 6 features for ESP32-C6");
    
    // Enable 802.11ax (WiFi 6) support
    esp_err_t ret = esp_wifi_set_protocol(WIFI_IF_STA, 
        WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to enable WiFi 6 on STA interface: %s", esp_err_to_name(ret));
    }
    
    ret = esp_wifi_set_protocol(WIFI_IF_AP, 
        WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to enable WiFi 6 on AP interface: %s", esp_err_to_name(ret));
    }
    
    // Set maximum bandwidth to 40MHz (ESP32-C6 limitation)
    esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);
    
    // Enable advanced features
    // Note: Some advanced WiFi 6 features like OFDMA and MU-MIMO are hardware dependent
    // and may require additional configuration through vendor-specific APIs
    
    ESP_LOGI(TAG, "WiFi 6 features enabled");
    return ESP_OK;
}

void EspWifi::setEventCallbacks(const WifiEventCallbacks& callbacks) {
    std::lock_guard<std::mutex> lock(mutex_);
    event_callbacks_ = callbacks;
}

esp_err_t EspWifi::setConnectionTimeout(uint32_t timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_timeout_ms_ = timeout_ms;
    return ESP_OK;
}

esp_err_t EspWifi::setMaxRetryCount(uint8_t max_retry) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_retry_num_ = max_retry;
    return ESP_OK;
}

std::string EspWifi::getMacAddress(WifiInterface interface) const {
    uint8_t mac[6];
    wifi_interface_t wifi_if = (interface == WifiInterface::STATION) ? WIFI_IF_STA : WIFI_IF_AP;
    
    if (esp_wifi_get_mac(wifi_if, mac) != ESP_OK) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < 6; i++) {
        if (i > 0) ss << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
    }
    
    return ss.str();
}

int8_t EspWifi::getRssi() const {
    if (!connected_) {
        return -127;  // Invalid RSSI
    }
    
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    
    return -127;
}

} // namespace wifi
} // namespace esp32
