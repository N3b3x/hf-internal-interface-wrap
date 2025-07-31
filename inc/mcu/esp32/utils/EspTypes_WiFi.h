/**
 * @file EspTypes_WiFi.h
 * @brief Type definitions for ESP32-C6 WiFi 6 implementation using ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This file provides comprehensive type definitions for ESP32-C6 WiFi 6 implementation
 * using ESP-IDF v5.5. It includes modern C++17 features and supports IEEE 802.11ax
 * (WiFi 6) certified features for ESP32-C6.
 */

#ifndef ESP_TYPES_WIFI_H
#define ESP_TYPES_WIFI_H

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <memory>
#include <optional>

// ESP-IDF v5.5 includes for WiFi
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_system.h"

// Network and IP related includes
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"

namespace esp32 {
namespace wifi {

/**
 * @brief MAC address type (6 bytes)
 */
using MacAddress = std::array<uint8_t, 6>;

/**
 * @brief WiFi operating modes
 */
enum class WifiMode : uint8_t {
    NONE = 0,           // WiFi disabled
    STATION = 1,        // Station mode only
    ACCESS_POINT = 2,   // Access Point mode only
    STATION_AP = 3      // Station + Access Point mode
};

/**
 * @brief WiFi security types (ESP32-C6 supports up to WPA3)
 */
enum class SecurityType : uint8_t {
    OPEN = 0,                   // No security
    WEP = 1,                    // WEP (deprecated, not recommended)
    WPA_PSK = 2,                // WPA-PSK
    WPA2_PSK = 3,               // WPA2-PSK
    WPA_WPA2_PSK = 4,           // WPA/WPA2-PSK mixed mode
    WPA3_PSK = 5,               // WPA3-PSK (SAE)
    WPA2_WPA3_PSK = 6,          // WPA2/WPA3-PSK mixed mode
    WPA2_ENTERPRISE = 7,        // WPA2-Enterprise (802.1X)
    WPA3_ENTERPRISE = 8,        // WPA3-Enterprise
    WPA3_ENTERPRISE_192 = 9,    // WPA3-Enterprise 192-bit
    UNKNOWN = 255               // Unknown security type
};

/**
 * @brief WiFi power save modes
 */
enum class PowerSaveMode : uint8_t {
    NONE = 0,           // No power save
    MIN_MODEM = 1,      // Minimum modem power save
    MAX_MODEM = 2       // Maximum modem power save
};

/**
 * @brief WiFi bandwidth modes (ESP32-C6 supports up to 40MHz)
 */
enum class BandwidthMode : uint8_t {
    HT20 = 0,          // 20MHz bandwidth
    HT40 = 1           // 40MHz bandwidth
};

/**
 * @brief WiFi protocol modes (ESP32-C6 supports WiFi 6)
 */
enum class ProtocolMode : uint8_t {
    B = 0,             // 802.11b only
    BG = 1,            // 802.11b/g
    BGN = 2,           // 802.11b/g/n
    BGNAX = 3          // 802.11b/g/n/ax (WiFi 6) - ESP32-C6
};

/**
 * @brief WiFi interface types
 */
enum class WifiInterface : uint8_t {
    STATION = 0,       // Station interface
    ACCESS_POINT = 1   // Access Point interface
};

/**
 * @brief WiFi scan result structure
 */
struct WifiScanResult {
    std::string ssid;              // Network SSID
    std::string bssid;             // BSSID (MAC address)
    int8_t rssi;                   // Signal strength in dBm
    uint8_t channel;               // Primary channel
    uint8_t secondary_channel;     // Secondary channel (for 40MHz)
    SecurityType security;         // Security type
    bool hidden;                   // Hidden network flag
    
    // WiFi 6 specific features (ESP32-C6)
    bool wifi6_support;            // 802.11ax support
    bool bandwidth_40mhz;          // 40MHz bandwidth support
    bool beamforming_support;      // Beamforming capability
    uint16_t max_data_rate;        // Maximum data rate in Mbps
    
    // Extended information
    uint16_t beacon_interval;      // Beacon interval in TU
    uint16_t capability_info;      // Capability information
    std::vector<uint8_t> information_elements; // Raw IE data
    std::chrono::steady_clock::time_point timestamp;
    
    // Convenience methods
    std::string getSecurityString() const;
    bool isWiFi6() const { return wifi6_support; }
    uint16_t getFrequency() const;
};

/**
 * @brief Connected station information (for AP mode)
 */
struct ConnectedStation {
    std::string mac_address;       // Station MAC address
    uint16_t aid;                  // Association ID
    int8_t rssi;                   // Signal strength
    std::chrono::steady_clock::time_point connection_time;
    uint32_t tx_bytes;             // Transmitted bytes
    uint32_t rx_bytes;             // Received bytes
    uint16_t tx_rate;              // Current TX rate in Mbps
    uint16_t rx_rate;              // Current RX rate in Mbps
};

/**
 * @brief Access Point configuration
 */
struct AccessPointConfig {
    std::string ssid;              // AP SSID (max 32 chars)
    std::string password;          // AP password (8-63 chars for secure)
    uint8_t channel;               // WiFi channel (1-14 for 2.4GHz)
    SecurityType security;         // Security type
    uint8_t max_connections;       // Maximum connected stations (1-10)
    bool hidden;                   // Hidden SSID flag
    uint16_t beacon_interval;      // Beacon interval in ms (100-60000)
    
    // Advanced configuration
    bool pmf_required;             // Protected Management Frames
    bool pmf_capable;              // PMF capability
    uint8_t dtim_period;           // DTIM period (1-255)
    
    // WiFi 6 features (ESP32-C6)
    bool enable_wifi6;             // Enable 802.11ax features
    BandwidthMode bandwidth;       // Channel bandwidth
    ProtocolMode protocol;         // WiFi protocol version
    int8_t tx_power;               // TX power in dBm
    
    // IP configuration
    std::string ip_address;        // AP IP address
    std::string gateway;           // Gateway address
    std::string netmask;           // Network mask
    bool dhcp_server_enabled;      // Enable DHCP server
    std::string dhcp_start_ip;     // DHCP pool start IP
    std::string dhcp_end_ip;       // DHCP pool end IP
    uint32_t dhcp_lease_time;      // DHCP lease time in seconds
};

/**
 * @brief WiFi status information
 */
struct WifiStatus {
    bool initialized;              // WiFi initialized flag
    bool station_started;          // Station started flag
    bool ap_started;               // AP started flag
    bool connected;                // Station connected flag
    bool got_ip;                   // Got IP address flag
    WifiMode current_mode;         // Current WiFi mode
    bool scan_in_progress;         // Scan in progress flag
    
    // Connection information
    std::string connected_ssid;    // Connected network SSID
    std::string bssid;             // Connected AP BSSID
    int8_t rssi;                   // Signal strength
    uint8_t channel;               // Connected channel
    SecurityType security;         // Connection security
    
    // Network information
    std::string ip_address;        // Station IP address
    std::string netmask;           // Network mask
    std::string gateway;           // Gateway address
    std::string primary_dns;       // Primary DNS server
    std::string secondary_dns;     // Secondary DNS server
    
    // Connection statistics
    uint8_t retry_count;           // Current retry count
    uint8_t max_retry_count;       // Maximum retry count
    uint32_t connected_stations_count; // Number of connected stations (AP mode)
    std::chrono::steady_clock::time_point connection_time;
    
    // Performance metrics
    uint32_t tx_bytes;             // Transmitted bytes
    uint32_t rx_bytes;             // Received bytes
    uint16_t current_tx_rate;      // Current TX rate in Mbps
    uint16_t current_rx_rate;      // Current RX rate in Mbps
};

/**
 * @brief Network information structure
 */
struct NetworkInfo {
    std::string ip_address;        // IP address
    std::string netmask;           // Network mask
    std::string gateway;           // Gateway address
    std::string primary_dns;       // Primary DNS server
    std::string secondary_dns;     // Secondary DNS server
    std::string mac_address;       // MAC address
    uint32_t lease_time;           // DHCP lease time remaining
    std::string hostname;          // Device hostname
    
    // IPv6 support (optional)
    std::vector<std::string> ipv6_addresses; // IPv6 addresses
    std::string ipv6_gateway;      // IPv6 gateway
};

/**
 * @brief WiFi event types
 */
enum class WifiEventType {
    ADAPTER_STATE_CHANGED,
    STATION_STARTED,
    STATION_STOPPED,
    STATION_CONNECTED,
    STATION_DISCONNECTED,
    STATION_AUTHMODE_CHANGED,
    STATION_GOT_IP,
    STATION_LOST_IP,
    AP_STARTED,
    AP_STOPPED,
    AP_STATION_CONNECTED,
    AP_STATION_DISCONNECTED,
    AP_STATION_IP_ASSIGNED,
    SCAN_STARTED,
    SCAN_COMPLETED,
    WPS_STARTED,
    WPS_COMPLETED,
    WPS_FAILED,
    SMART_CONFIG_STARTED,
    SMART_CONFIG_COMPLETED,
    SMART_CONFIG_FAILED
};

/**
 * @brief WiFi event data structure
 */
struct WifiEvent {
    WifiEventType type;
    std::chrono::steady_clock::time_point timestamp;
    
    union {
        struct {
            bool enabled;
        } adapter_state_changed;
        
        struct {
            std::string ssid;
            std::string bssid;
            uint8_t channel;
            SecurityType security;
        } station_connected;
        
        struct {
            uint8_t reason;
            int8_t rssi;
        } station_disconnected;
        
        struct {
            std::string ip_address;
            std::string netmask;
            std::string gateway;
        } station_got_ip;
        
        struct {
            ConnectedStation station;
        } ap_station_connected;
        
        struct {
            std::string mac_address;
            uint16_t aid;
        } ap_station_disconnected;
        
        struct {
            uint16_t found_networks;
            uint32_t scan_duration_ms;
        } scan_completed;
    };
};

/**
 * @brief WiFi callback function types
 */
struct WifiEventCallbacks {
    std::function<void()> station_connected;
    std::function<void()> station_disconnected;
    std::function<void()> station_connection_failed;
    std::function<void(const std::string& ip)> got_ip;
    std::function<void()> lost_ip;
    std::function<void(const ConnectedStation& station)> ap_station_connected;
    std::function<void(const ConnectedStation& station)> ap_station_disconnected;
    std::function<void(const std::vector<WifiScanResult>& results)> scan_completed;
    std::function<void(WifiEventType event_type)> generic_event;
};

/**
 * @brief WiFi Enterprise configuration (for WPA2/WPA3 Enterprise)
 */
struct WifiEnterpriseConfig {
    std::string username;          // Enterprise username
    std::string password;          // Enterprise password
    std::string ca_cert;           // CA certificate (PEM format)
    std::string client_cert;       // Client certificate (PEM format)
    std::string private_key;       // Private key (PEM format)
    std::string identity;          // EAP identity
    std::string anonymous_identity; // Anonymous identity
    
    // EAP method configuration
    bool use_eap_tls;              // Use EAP-TLS
    bool use_eap_ttls;             // Use EAP-TTLS
    bool use_eap_peap;             // Use EAP-PEAP
    bool validate_server_cert;     // Validate server certificate
    
    // Phase 2 authentication (for TTLS/PEAP)
    std::string phase2_method;     // Phase 2 method (MSCHAPV2, GTC, etc.)
};

/**
 * @brief WiFi mesh configuration (ESP-MESH support)
 */
struct WifiMeshConfig {
    std::string mesh_id;           // Mesh network ID
    std::string mesh_password;     // Mesh password
    uint8_t max_layer;             // Maximum mesh layers
    uint8_t max_connection;        // Maximum connections per node
    bool allow_channel_switch;     // Allow channel switching
    bool enable_encryption;        // Enable mesh encryption
    uint16_t announce_interval;    // Announce interval in ms
    uint16_t attempt_count;        // Connection attempt count
    uint16_t monitor_duration;     // Monitor duration in ms
};

/**
 * @brief WiFi QoS (Quality of Service) configuration
 */
struct WifiQoSConfig {
    bool enable_wmm;               // Enable WMM (WiFi Multimedia)
    bool enable_uapsd;             // Enable U-APSD (Unscheduled APSD)
    uint8_t voice_ac_params[4];    // Voice AC parameters
    uint8_t video_ac_params[4];    // Video AC parameters
    uint8_t best_effort_ac_params[4]; // Best effort AC parameters
    uint8_t background_ac_params[4];  // Background AC parameters
};

/**
 * @brief WiFi advanced configuration options
 */
struct WifiAdvancedConfig {
    // Power management
    PowerSaveMode power_save_mode; // Power save mode
    uint8_t listen_interval;       // Listen interval
    int8_t tx_power;               // TX power in dBm
    
    // Performance tuning
    BandwidthMode bandwidth;       // Channel bandwidth
    ProtocolMode protocol;         // WiFi protocol version
    bool enable_ampdu_tx;          // Enable A-MPDU TX
    bool enable_ampdu_rx;          // Enable A-MPDU RX
    bool enable_amsdu_tx;          // Enable A-MSDU TX
    bool enable_amsdu_rx;          // Enable A-MSDU RX
    
    // Security and privacy
    bool enable_pmf;               // Protected Management Frames
    bool enable_wpa3_transition;   // WPA3 transition mode
    bool enable_opportunistic_key_caching; // OKC support
    bool enable_11r_fast_transition; // 802.11r fast BSS transition
    bool enable_11k_neighbor_report; // 802.11k neighbor reports
    bool enable_11v_bss_transition;  // 802.11v BSS transition
    
    // WiFi 6 features (ESP32-C6)
    bool enable_he_su_beamformer;  // HE SU beamformer
    bool enable_he_su_beamformee;  // HE SU beamformee
    bool enable_he_mu_beamformer;  // HE MU beamformer (if supported)
    bool enable_he_mu_beamformee;  // HE MU beamformee (if supported)
    bool enable_target_wake_time;  // Target Wake Time (TWT)
    bool enable_spatial_reuse;     // Spatial Reuse (SR)
    uint8_t he_mcs_map;            // HE MCS map
    
    // Connection parameters
    uint16_t beacon_timeout;       // Beacon timeout in ms
    uint16_t connection_timeout;   // Connection timeout in ms
    uint8_t max_retry_count;       // Maximum connection retries
    bool auto_reconnect;           // Auto-reconnect on disconnection
    
    // Scanning parameters
    uint16_t scan_timeout;         // Scan timeout in ms
    bool passive_scan;             // Use passive scanning
    bool scan_all_channels;        // Scan all available channels
    uint8_t scan_probe_count;      // Number of probe requests per channel
    
    // Roaming parameters
    bool enable_roaming;           // Enable automatic roaming
    int8_t roaming_rssi_threshold; // RSSI threshold for roaming
    uint8_t roaming_scan_interval; // Roaming scan interval in seconds
};

/**
 * @brief WiFi statistics structure
 */
struct WifiStatistics {
    // Transmission statistics
    uint32_t tx_packets;           // Transmitted packets
    uint32_t tx_bytes;             // Transmitted bytes
    uint32_t tx_errors;            // Transmission errors
    uint32_t tx_dropped;           // Dropped TX packets
    uint32_t tx_retries;           // TX retries
    
    // Reception statistics
    uint32_t rx_packets;           // Received packets
    uint32_t rx_bytes;             // Received bytes
    uint32_t rx_errors;            // Reception errors
    uint32_t rx_dropped;           // Dropped RX packets
    uint32_t rx_crc_errors;        // CRC errors
    
    // Connection statistics
    uint32_t connection_attempts;  // Connection attempts
    uint32_t successful_connections; // Successful connections
    uint32_t failed_connections;   // Failed connections
    uint32_t disconnections;       // Disconnections
    uint32_t roaming_events;       // Roaming events
    
    // Signal quality
    int8_t current_rssi;           // Current RSSI
    int8_t min_rssi;               // Minimum RSSI
    int8_t max_rssi;               // Maximum RSSI
    uint8_t signal_quality;        // Signal quality percentage
    
    // Performance metrics
    uint16_t current_tx_rate;      // Current TX rate in Mbps
    uint16_t max_tx_rate;          // Maximum TX rate in Mbps
    uint16_t current_rx_rate;      // Current RX rate in Mbps
    uint16_t max_rx_rate;          // Maximum RX rate in Mbps
    
    // Timing information
    std::chrono::steady_clock::time_point stats_start_time;
    std::chrono::steady_clock::time_point last_update_time;
    uint32_t uptime_seconds;       // Uptime in seconds
};

/**
 * @brief WiFi country configuration
 */
struct WifiCountryConfig {
    std::string country_code;      // Two-letter country code (e.g., "US", "EU")
    uint8_t schan;                 // Start channel
    uint8_t nchan;                 // Number of channels
    int8_t max_tx_power;           // Maximum TX power in dBm
    wifi_country_policy_t policy;  // Country policy
};

/**
 * @brief WiFi error codes
 */
enum class WifiError : int {
    SUCCESS = 0,
    INVALID_PARAMETER = -1,
    NOT_INITIALIZED = -2,
    ALREADY_INITIALIZED = -3,
    OPERATION_FAILED = -4,
    CONNECTION_FAILED = -5,
    DISCONNECTION_FAILED = -6,
    AUTHENTICATION_FAILED = -7,
    NETWORK_NOT_FOUND = -8,
    SCAN_FAILED = -9,
    TIMEOUT = -10,
    INVALID_PASSWORD = -11,
    INVALID_STATE = -12,
    NOT_CONNECTED = -13,
    ALREADY_CONNECTED = -14,
    AP_START_FAILED = -15,
    AP_STOP_FAILED = -16,
    CONFIGURATION_FAILED = -17,
    MEMORY_ALLOCATION_FAILED = -18,
    INTERFACE_ERROR = -19,
    SECURITY_ERROR = -20,
    NOT_SUPPORTED = -21,
    RESOURCE_BUSY = -22,
    INVALID_SSID = -23,
    INVALID_CHANNEL = -24,
    POWER_SAVE_ERROR = -25,
    ENTERPRISE_CONFIG_ERROR = -26,
    MESH_ERROR = -27,
    QOS_ERROR = -28
};

/**
 * @brief WPS (WiFi Protected Setup) configuration
 */
struct WPSConfig {
    bool enable_wps;               // Enable WPS
    wifi_wps_type_t wps_type;      // WPS type (PBC, PIN, DISPLAY)
    std::string pin;               // WPS PIN (for PIN method)
    uint32_t timeout_ms;           // WPS timeout in milliseconds
};

/**
 * @brief SmartConfig configuration
 */
struct SmartConfigConfig {
    bool enable_smartconfig;       // Enable SmartConfig
    smartconfig_type_t type;       // SmartConfig type
    uint32_t timeout_ms;           // SmartConfig timeout
    bool enable_ack;               // Enable ACK to phone
};

/**
 * @brief Standard WiFi channels for different regions
 */
namespace WifiChannels {
    // 2.4 GHz channels
    constexpr uint8_t CHANNEL_1 = 1;     // 2412 MHz
    constexpr uint8_t CHANNEL_6 = 6;     // 2437 MHz
    constexpr uint8_t CHANNEL_11 = 11;   // 2462 MHz
    constexpr uint8_t CHANNEL_14 = 14;   // 2484 MHz (Japan only)
    
    // Channel ranges
    constexpr uint8_t CHANNEL_MIN_2G = 1;
    constexpr uint8_t CHANNEL_MAX_2G = 14;
}

/**
 * @brief WiFi frequency bands
 */
enum class WifiFrequencyBand : uint8_t {
    BAND_2_4_GHZ = 0,              // 2.4 GHz band
    BAND_5_GHZ = 1,                // 5 GHz band (not supported by ESP32-C6)
    BAND_6_GHZ = 2                 // 6 GHz band (not supported by ESP32-C6)
};

/**
 * @brief Helper functions for WiFi operations
 */
namespace WifiUtils {
    /**
     * @brief Convert channel number to frequency
     * @param channel Channel number
     * @return Frequency in MHz
     */
    uint16_t channelToFrequency(uint8_t channel);
    
    /**
     * @brief Convert frequency to channel number
     * @param frequency Frequency in MHz
     * @return Channel number (0 if invalid)
     */
    uint8_t frequencyToChannel(uint16_t frequency);
    
    /**
     * @brief Convert RSSI to signal quality percentage
     * @param rssi RSSI value in dBm
     * @return Signal quality (0-100%)
     */
    uint8_t rssiToQuality(int8_t rssi);
    
    /**
     * @brief Validate SSID string
     * @param ssid SSID to validate
     * @return true if valid, false otherwise
     */
    bool isValidSSID(const std::string& ssid);
    
    /**
     * @brief Validate password for given security type
     * @param password Password to validate
     * @param security Security type
     * @return true if valid, false otherwise
     */
    bool isValidPassword(const std::string& password, SecurityType security);
    
    /**
     * @brief Convert MAC address to string
     * @param mac MAC address array
     * @return MAC address string (xx:xx:xx:xx:xx:xx)
     */
    std::string macToString(const MacAddress& mac);
    
    /**
     * @brief Convert string to MAC address
     * @param mac_str MAC address string
     * @return MAC address array
     */
    MacAddress stringToMac(const std::string& mac_str);
    
    /**
     * @brief Get security type string representation
     * @param security Security type
     * @return Security type string
     */
    std::string securityTypeToString(SecurityType security);
    
    /**
     * @brief Convert WiFi error to string
     * @param error WiFi error code
     * @return Error description string
     */
    std::string wifiErrorToString(WifiError error);
}

/**
 * @brief Standard WiFi service identifiers
 */
namespace WifiServices {
    constexpr uint16_t HTTP_PORT = 80;
    constexpr uint16_t HTTPS_PORT = 443;
    constexpr uint16_t FTP_PORT = 21;
    constexpr uint16_t SSH_PORT = 22;
    constexpr uint16_t TELNET_PORT = 23;
    constexpr uint16_t SMTP_PORT = 25;
    constexpr uint16_t DNS_PORT = 53;
    constexpr uint16_t DHCP_SERVER_PORT = 67;
    constexpr uint16_t DHCP_CLIENT_PORT = 68;
    constexpr uint16_t SNMP_PORT = 161;
    constexpr uint16_t NTP_PORT = 123;
}

} // namespace wifi
} // namespace esp32

#endif // ESP_TYPES_WIFI_H