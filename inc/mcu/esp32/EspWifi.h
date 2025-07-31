/**
 * @file EspWifi.h
 * @brief ESP32-C6 WiFi 6 implementation for ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This implementation supports:
 * - WiFi 6 (IEEE 802.11ax) on ESP32-C6
 * - Station and Access Point modes
 * - WiFi 6 features: OFDMA, MU-MIMO, BSS Coloring, TWT
 * - ESP-IDF v5.5 APIs
 * - Modern C++17 features
 * - Thread-safe operations
 * - Power management
 * - Enterprise security (WPA2/WPA3-Enterprise)
 * - WiFi mesh networking
 */

#ifndef ESP_WIFI_H
#define ESP_WIFI_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <array>
#include <map>
#include <chrono>

// ESP-IDF v5.5 includes
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// WiFi 6 includes for ESP-IDF v5.5
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_smartconfig.h"
#include "esp_wps.h"
#include "esp_now.h"
#include "esp_mesh.h"

// Local includes
#include "utils/EspTypes_WiFi.h"

namespace esp32 {
namespace wifi {

/**
 * @brief Forward declarations
 */
class AccessPoint;
class Station;
class MeshNode;

/**
 * @brief WiFi 6 implementation for ESP32-C6 with ESP-IDF v5.5
 * 
 * This class provides comprehensive WiFi 6 functionality for ESP32-C6:
 * - Modern C++17 design patterns
 * - Thread-safe operations
 * - Power-efficient implementation
 * - Station and AP modes
 * - WiFi 6 advanced features
 * - Enterprise security
 * - Mesh networking
 */
class EspWifi {
public:
    /**
     * @brief WiFi operating modes
     */
    enum class Mode {
        DISABLED = WIFI_MODE_NULL,     ///< WiFi disabled
        STATION = WIFI_MODE_STA,       ///< Station mode only
        ACCESS_POINT = WIFI_MODE_AP,   ///< Access Point mode only
        APSTA = WIFI_MODE_APSTA        ///< Station + AP mode
    };

    /**
     * @brief WiFi power save modes
     */
    enum class PowerSaveMode {
        NONE = WIFI_PS_NONE,           ///< No power save
        MIN_MODEM = WIFI_PS_MIN_MODEM, ///< Minimum modem power save
        MAX_MODEM = WIFI_PS_MAX_MODEM  ///< Maximum modem power save
    };

    /**
     * @brief WiFi protocol standards
     */
    enum class Protocol {
        WIFI_B = WIFI_PROTOCOL_11B,    ///< 802.11b
        WIFI_G = WIFI_PROTOCOL_11G,    ///< 802.11g
        WIFI_N = WIFI_PROTOCOL_11N,    ///< 802.11n
        WIFI_LR = WIFI_PROTOCOL_LR,    ///< Long Range
        WIFI_AX = WIFI_PROTOCOL_11AX   ///< 802.11ax (WiFi 6)
    };

    /**
     * @brief WiFi security types
     */
    enum class Security {
        OPEN = WIFI_AUTH_OPEN,                    ///< Open (no security)
        WEP = WIFI_AUTH_WEP,                     ///< WEP (deprecated)
        WPA_PSK = WIFI_AUTH_WPA_PSK,             ///< WPA-PSK
        WPA2_PSK = WIFI_AUTH_WPA2_PSK,           ///< WPA2-PSK
        WPA_WPA2_PSK = WIFI_AUTH_WPA_WPA2_PSK,   ///< WPA/WPA2-PSK
        WPA2_ENTERPRISE = WIFI_AUTH_WPA2_ENTERPRISE, ///< WPA2-Enterprise
        WPA3_PSK = WIFI_AUTH_WPA3_PSK,           ///< WPA3-PSK
        WPA2_WPA3_PSK = WIFI_AUTH_WPA2_WPA3_PSK, ///< WPA2/WPA3-PSK
        WPA3_ENT_192 = WIFI_AUTH_WPA3_ENT_192    ///< WPA3-Enterprise 192-bit
    };

    /**
     * @brief WiFi 6 specific features
     */
    struct Wifi6Features {
        bool ofdma_enabled{false};         ///< OFDMA support
        bool mu_mimo_enabled{false};       ///< MU-MIMO support
        bool bss_coloring_enabled{false};  ///< BSS Coloring
        bool twt_enabled{false};           ///< Target Wake Time
        uint8_t bss_color{0};              ///< BSS Color value (1-63)
        uint16_t he_mcs_set{0};            ///< HE MCS set
    };

    /**
     * @brief Station configuration
     */
    struct StationConfig {
        std::string ssid;                  ///< Network SSID
        std::string password;              ///< Network password
        std::array<uint8_t, 6> bssid{};    ///< Target BSSID (optional)
        bool bssid_set{false};             ///< Use specific BSSID
        Security security{Security::WPA2_PSK}; ///< Security type
        uint8_t channel{0};                ///< Channel (0 = scan all)
        int8_t rssi_threshold{-127};       ///< RSSI threshold
        wifi_sort_method_t sort_method{WIFI_CONNECT_AP_BY_SIGNAL}; ///< AP selection method
        wifi_scan_method_t scan_method{WIFI_FAST_SCAN}; ///< Scan method
        uint32_t scan_timeout_ms{15000};   ///< Scan timeout
        Wifi6Features wifi6;               ///< WiFi 6 features
    };

    /**
     * @brief Access Point configuration
     */
    struct AccessPointConfig {
        std::string ssid;                  ///< AP SSID
        std::string password;              ///< AP password
        Security security{Security::WPA2_PSK}; ///< Security type
        uint8_t channel{1};                ///< WiFi channel
        uint8_t max_connections{4};        ///< Maximum stations
        bool ssid_hidden{false};           ///< Hide SSID
        uint16_t beacon_interval{100};     ///< Beacon interval (ms)
        uint8_t dtim_period{2};            ///< DTIM period
        Wifi6Features wifi6;               ///< WiFi 6 features
    };

    /**
     * @brief Scan parameters
     */
    struct ScanConfig {
        std::string ssid;                  ///< Specific SSID to scan (empty = all)
        std::array<uint8_t, 6> bssid{};    ///< Specific BSSID to scan
        uint8_t channel{0};                ///< Channel to scan (0 = all)
        bool show_hidden{false};           ///< Show hidden networks
        wifi_scan_type_t scan_type{WIFI_SCAN_TYPE_ACTIVE}; ///< Scan type
        uint32_t scan_time_min{120};       ///< Minimum scan time per channel
        uint32_t scan_time_max{120};       ///< Maximum scan time per channel
        uint32_t scan_time_passive{360};   ///< Passive scan time per channel
    };

    /**
     * @brief Network information
     */
    struct NetworkInfo {
        std::string ssid;                  ///< Network SSID
        std::array<uint8_t, 6> bssid;      ///< BSSID
        int8_t rssi;                       ///< Signal strength
        Security security;                 ///< Security type
        uint8_t channel;                   ///< Channel
        bool wifi6_supported;              ///< WiFi 6 support
        uint8_t bss_color;                 ///< BSS Color (WiFi 6)
    };

    /**
     * @brief Connection information
     */
    struct ConnectionInfo {
        std::string ssid;                  ///< Connected SSID
        std::array<uint8_t, 6> bssid;      ///< Connected BSSID
        uint8_t channel;                   ///< Channel
        int8_t rssi;                       ///< Signal strength
        uint32_t phy_lr;                   ///< PHY rate (low rate)
        uint32_t phy_11b;                  ///< PHY rate 11b
        uint32_t phy_11g;                  ///< PHY rate 11g
        uint32_t phy_11n;                  ///< PHY rate 11n
        uint32_t phy_11ax;                 ///< PHY rate 11ax (WiFi 6)
        bool is_11ax_enabled;              ///< WiFi 6 enabled
        Wifi6Features wifi6_features;      ///< Active WiFi 6 features
    };

    /**
     * @brief Enterprise configuration for WPA2/WPA3-Enterprise
     */
    struct EnterpriseConfig {
        std::string identity;              ///< User identity
        std::string username;              ///< Username
        std::string password;              ///< Password
        std::string ca_cert;               ///< CA certificate
        std::string client_cert;           ///< Client certificate
        std::string client_key;            ///< Client private key
        esp_eap_method_t method{ESP_EAP_PEAP}; ///< EAP method
        bool disable_time_check{false};    ///< Disable time validation
    };

    /**
     * @brief Event callback types
     */
    using ConnectionCallback = std::function<void(const ConnectionInfo& info)>;
    using DisconnectionCallback = std::function<void(int reason)>;
    using ScanCompleteCallback = std::function<void(const std::vector<NetworkInfo>& networks)>;
    using ApStationCallback = std::function<void(const std::array<uint8_t, 6>& mac, bool connected)>;
    using IpAssignedCallback = std::function<void(uint32_t ip, uint32_t netmask, uint32_t gateway)>;

    /**
     * @brief Singleton instance getter
     * @return Reference to the singleton instance
     */
    static EspWifi& getInstance();

    /**
     * @brief Initialize WiFi subsystem
     * @param mode WiFi operating mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initialize(Mode mode = Mode::STATION);

    /**
     * @brief Deinitialize WiFi subsystem
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitialize();

    /**
     * @brief Check if WiFi is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized_.load(); }

    /**
     * @brief Set WiFi mode
     * @param mode WiFi mode to set
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setMode(Mode mode);

    /**
     * @brief Get current WiFi mode
     * @return Current WiFi mode
     */
    Mode getMode() const { return current_mode_; }

    /**
     * @brief Configure station mode
     * @param config Station configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureStation(const StationConfig& config);

    /**
     * @brief Configure access point mode
     * @param config AP configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureAccessPoint(const AccessPointConfig& config);

    /**
     * @brief Start WiFi
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t start();

    /**
     * @brief Stop WiFi
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stop();

    /**
     * @brief Check if WiFi is started
     * @return true if started, false otherwise
     */
    bool isStarted() const { return started_.load(); }

    /**
     * @brief Connect to configured network (station mode)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t connect();

    /**
     * @brief Disconnect from network (station mode)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t disconnect();

    /**
     * @brief Check if connected to network
     * @return true if connected, false otherwise
     */
    bool isConnected() const { return connected_.load(); }

    /**
     * @brief Get connection information
     * @return Connection information
     */
    ConnectionInfo getConnectionInfo() const;

    /**
     * @brief Start network scan
     * @param config Scan configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t startScan(const ScanConfig& config = ScanConfig{});

    /**
     * @brief Stop network scan
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stopScan();

    /**
     * @brief Check if currently scanning
     * @return true if scanning, false otherwise
     */
    bool isScanning() const { return scanning_.load(); }

    /**
     * @brief Get scan results
     * @return Vector of detected networks
     */
    std::vector<NetworkInfo> getScanResults() const;

    /**
     * @brief Set power save mode
     * @param mode Power save mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setPowerSaveMode(PowerSaveMode mode);

    /**
     * @brief Get current power save mode
     * @return Current power save mode
     */
    PowerSaveMode getPowerSaveMode() const { return power_save_mode_; }

    /**
     * @brief Set WiFi protocol
     * @param interface WiFi interface (STA/AP)
     * @param protocols Protocol bitmap
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setProtocol(wifi_interface_t interface, uint8_t protocols);

    /**
     * @brief Enable/disable WiFi 6 features
     * @param interface WiFi interface
     * @param features WiFi 6 features configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureWifi6Features(wifi_interface_t interface, const Wifi6Features& features);

    /**
     * @brief Configure enterprise authentication
     * @param config Enterprise configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureEnterprise(const EnterpriseConfig& config);

    /**
     * @brief Set country configuration
     * @param country_code Two-letter country code
     * @param start_channel Start channel
     * @param total_channels Total number of channels
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setCountry(const std::string& country_code, uint8_t start_channel = 1, uint8_t total_channels = 13);

    /**
     * @brief Set TX power
     * @param power TX power in dBm
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setTxPower(int8_t power);

    /**
     * @brief Get TX power
     * @return TX power in dBm
     */
    int8_t getTxPower() const;

    /**
     * @brief Get MAC address
     * @param interface WiFi interface
     * @return MAC address
     */
    std::array<uint8_t, 6> getMacAddress(wifi_interface_t interface) const;

    /**
     * @brief Set MAC address
     * @param interface WiFi interface
     * @param mac MAC address to set
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setMacAddress(wifi_interface_t interface, const std::array<uint8_t, 6>& mac);

    /**
     * @brief Get connected stations (AP mode)
     * @return List of connected station MAC addresses
     */
    std::vector<std::array<uint8_t, 6>> getConnectedStations() const;

    /**
     * @brief Disconnect a station (AP mode)
     * @param mac Station MAC address
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t disconnectStation(const std::array<uint8_t, 6>& mac);

    /**
     * @brief Enable/disable promiscuous mode
     * @param enable Enable promiscuous mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setPromiscuousMode(bool enable);

    /**
     * @brief Start SmartConfig provisioning
     * @param type SmartConfig type
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t startSmartConfig(smartconfig_type_t type = SC_TYPE_ESPTOUCH);

    /**
     * @brief Stop SmartConfig provisioning
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stopSmartConfig();

    /**
     * @brief Start WPS
     * @param config WPS configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t startWps(esp_wps_config_t config);

    /**
     * @brief Stop WPS
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stopWps();

    /**
     * @brief Initialize ESP-NOW
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeEspNow();

    /**
     * @brief Deinitialize ESP-NOW
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitializeEspNow();

    /**
     * @brief Register connection event callback
     * @param callback Callback function
     */
    void onConnection(ConnectionCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        connection_callback_ = std::move(callback);
    }

    /**
     * @brief Register disconnection event callback
     * @param callback Callback function
     */
    void onDisconnection(DisconnectionCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        disconnection_callback_ = std::move(callback);
    }

    /**
     * @brief Register scan complete event callback
     * @param callback Callback function
     */
    void onScanComplete(ScanCompleteCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        scan_complete_callback_ = std::move(callback);
    }

    /**
     * @brief Register AP station event callback
     * @param callback Callback function
     */
    void onApStation(ApStationCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        ap_station_callback_ = std::move(callback);
    }

    /**
     * @brief Register IP assigned event callback
     * @param callback Callback function
     */
    void onIpAssigned(IpAssignedCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        ip_assigned_callback_ = std::move(callback);
    }

    /**
     * @brief Get WiFi statistics
     * @return String containing statistics
     */
    std::string getStatistics() const;

    /**
     * @brief Reset WiFi stack (emergency recovery)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t reset();

    /**
     * @brief Enable low power mode with advanced power management
     * @param enable Enable low power mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setLowPowerMode(bool enable);

    /**
     * @brief Check if low power mode is enabled
     * @return true if enabled, false otherwise
     */
    bool isLowPowerMode() const { return low_power_mode_.load(); }

private:
    /**
     * @brief Private constructor for singleton
     */
    EspWifi() = default;

    /**
     * @brief Destructor
     */
    ~EspWifi();

    /**
     * @brief Delete copy constructor and assignment operator
     */
    EspWifi(const EspWifi&) = delete;
    EspWifi& operator=(const EspWifi&) = delete;

    /**
     * @brief Initialize NVS flash
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeNVS();

    /**
     * @brief Initialize TCP/IP adapter
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeNetif();

    /**
     * @brief Initialize event loop
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeEventLoop();

    /**
     * @brief Register event handlers
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t registerEventHandlers();

    /**
     * @brief WiFi event handler
     */
    static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

    /**
     * @brief IP event handler
     */
    static void ipEventHandler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data);

    /**
     * @brief SmartConfig event handler
     */
    static void smartConfigEventHandler(void* arg, esp_event_base_t event_base,
                                       int32_t event_id, void* event_data);

    /**
     * @brief Handle WiFi connect event
     */
    void handleConnectEvent(void* event_data);

    /**
     * @brief Handle WiFi disconnect event
     */
    void handleDisconnectEvent(void* event_data);

    /**
     * @brief Handle scan done event
     */
    void handleScanDoneEvent(void* event_data);

    /**
     * @brief Handle AP station connected event
     */
    void handleApStationConnectedEvent(void* event_data);

    /**
     * @brief Handle AP station disconnected event
     */
    void handleApStationDisconnectedEvent(void* event_data);

    /**
     * @brief Handle IP assigned event
     */
    void handleIpAssignedEvent(void* event_data);

    /**
     * @brief Convert ESP security type to our enum
     */
    static Security convertSecurityType(wifi_auth_mode_t auth_mode);

    /**
     * @brief Convert our security type to ESP type
     */
    static wifi_auth_mode_t convertToEspSecurityType(Security security);

    // State variables
    std::atomic<bool> initialized_{false};
    std::atomic<bool> started_{false};
    std::atomic<bool> connected_{false};
    std::atomic<bool> scanning_{false};
    std::atomic<bool> low_power_mode_{false};
    Mode current_mode_{Mode::DISABLED};
    PowerSaveMode power_save_mode_{PowerSaveMode::NONE};

    // Configuration
    StationConfig station_config_;
    AccessPointConfig ap_config_;
    EnterpriseConfig enterprise_config_;

    // Network interfaces
    esp_netif_t* sta_netif_{nullptr};
    esp_netif_t* ap_netif_{nullptr};

    // Event handling
    esp_event_loop_handle_t event_loop_handle_{nullptr};

    // Synchronization
    mutable std::mutex state_mutex_;
    mutable std::mutex callback_mutex_;
    EventGroupHandle_t event_group_{nullptr};

    // Callbacks
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    ScanCompleteCallback scan_complete_callback_;
    ApStationCallback ap_station_callback_;
    IpAssignedCallback ip_assigned_callback_;

    // Scan results
    mutable std::mutex scan_mutex_;
    std::vector<NetworkInfo> scan_results_;

    // Statistics
    mutable std::mutex stats_mutex_;
    struct {
        uint32_t connections_established{0};
        uint32_t connections_dropped{0};
        uint32_t scans_performed{0};
        uint32_t packets_sent{0};
        uint32_t packets_received{0};
        uint32_t errors{0};
        std::chrono::steady_clock::time_point last_connection_time;
        std::chrono::steady_clock::time_point last_scan_time;
    } statistics_;

    // Event group bits
    static const int WIFI_CONNECTED_BIT = BIT0;
    static const int WIFI_FAIL_BIT = BIT1;
    static const int WIFI_SCAN_DONE_BIT = BIT2;
    static const int WIFI_SMARTCONFIG_DONE_BIT = BIT3;

    // Logging tag
    static constexpr const char* TAG = "EspWifi";
};

} // namespace wifi
} // namespace esp32

#endif // ESP_WIFI_H