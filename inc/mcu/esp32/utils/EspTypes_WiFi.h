/**
 * @file EspTypes_WiFi.h
 * @brief ESP32 WiFi type definitions for hardware abstraction.
 *
 * This header defines only the essential WiFi-specific types and constants used by
 * the EspWifi implementation. It follows a clean, minimal pattern providing only
 * necessary types without redundant or duplicate definitions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseWifi.h" // For hf_wifi_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF)

#ifdef HF_MCU_FAMILY_ESP32

//==============================================================================
// ESSENTIAL WIFI TYPES (ESP32)
//==============================================================================

/**
 * @brief WiFi mode type for ESP32.
 */
using hf_esp_wifi_mode_t = hf_wifi_mode_t;

/**
 * @brief WiFi channel bandwidth types.
 * @details Channel bandwidth options for WiFi 6 support.
 */
enum class hf_esp_wifi_bandwidth_t : hf_u8_t {
  HF_WIFI_BW_20 = 0,    ///< 20 MHz bandwidth
  HF_WIFI_BW_40 = 1,    ///< 40 MHz bandwidth
  HF_WIFI_BW_80 = 2,    ///< 80 MHz bandwidth (WiFi 6)
  HF_WIFI_BW_160 = 3    ///< 160 MHz bandwidth (WiFi 6)
};

/**
 * @brief WiFi protocol bitmap.
 * @details Supported WiFi protocol standards.
 */
enum class hf_esp_wifi_protocol_t : hf_u8_t {
  HF_WIFI_PROTOCOL_11B = 0x01,   ///< 802.11b
  HF_WIFI_PROTOCOL_11G = 0x02,   ///< 802.11g
  HF_WIFI_PROTOCOL_11N = 0x04,   ///< 802.11n
  HF_WIFI_PROTOCOL_LR = 0x08,    ///< Long Range mode
  HF_WIFI_PROTOCOL_11AX = 0x10   ///< 802.11ax (WiFi 6)
};

/**
 * @brief WiFi security authentication modes.
 * @details Security types supported by ESP32-C6.
 */
enum class hf_esp_wifi_auth_mode_t : hf_u8_t {
  HF_WIFI_AUTH_OPEN = 0,             ///< Open (no security)
  HF_WIFI_AUTH_WEP = 1,             ///< WEP (deprecated)
  HF_WIFI_AUTH_WPA_PSK = 2,         ///< WPA-PSK
  HF_WIFI_AUTH_WPA2_PSK = 3,        ///< WPA2-PSK
  HF_WIFI_AUTH_WPA_WPA2_PSK = 4,    ///< WPA/WPA2-PSK
  HF_WIFI_AUTH_WPA2_ENTERPRISE = 5, ///< WPA2-Enterprise
  HF_WIFI_AUTH_WPA3_PSK = 6,        ///< WPA3-PSK
  HF_WIFI_AUTH_WPA2_WPA3_PSK = 7,   ///< WPA2/WPA3-PSK
  HF_WIFI_AUTH_WAPI_PSK = 8,        ///< WAPI-PSK
  HF_WIFI_AUTH_WPA3_ENT_192 = 9     ///< WPA3-Enterprise 192-bit
};

/**
 * @brief WiFi cipher types.
 * @details Encryption cipher types.
 */
enum class hf_esp_wifi_cipher_type_t : hf_u8_t {
  HF_WIFI_CIPHER_TYPE_NONE = 0,       ///< No cipher
  HF_WIFI_CIPHER_TYPE_WEP40 = 1,      ///< WEP40
  HF_WIFI_CIPHER_TYPE_WEP104 = 2,     ///< WEP104
  HF_WIFI_CIPHER_TYPE_TKIP = 3,       ///< TKIP
  HF_WIFI_CIPHER_TYPE_CCMP = 4,       ///< CCMP (AES)
  HF_WIFI_CIPHER_TYPE_TKIP_CCMP = 5,  ///< TKIP + CCMP
  HF_WIFI_CIPHER_TYPE_AES_CMAC128 = 6, ///< AES-CMAC-128
  HF_WIFI_CIPHER_TYPE_SMS4 = 7,       ///< SMS4 (WAPI)
  HF_WIFI_CIPHER_TYPE_GCMP = 8,       ///< GCMP (WPA3)
  HF_WIFI_CIPHER_TYPE_GCMP256 = 9     ///< GCMP-256 (WPA3)
};

//==============================================================================
// CONFIGURATION STRUCTURES
//==============================================================================

/**
 * @brief WiFi station configuration structure.
 * @details Configuration parameters for station mode.
 */
struct hf_esp_wifi_sta_config_t {
  char ssid[33];                           ///< SSID (32 bytes + null terminator)
  char password[65];                       ///< Password (64 bytes + null terminator)
  hf_u8_t bssid[6];                       ///< Target BSSID (optional)
  hf_bool_t bssid_set;                    ///< Use specific BSSID
  hf_u8_t channel;                        ///< Channel (0 = scan all)
  hf_esp_wifi_auth_mode_t threshold_authmode; ///< Minimum security mode
  hf_i8_t rssi_threshold;                 ///< RSSI threshold
  hf_bool_t pmf_capable;                  ///< Protected Management Frame capable
  hf_bool_t pmf_required;                 ///< Protected Management Frame required
  hf_bool_t rm_enabled;                   ///< 802.11k enabled
  hf_bool_t btm_enabled;                  ///< 802.11v enabled
  hf_bool_t mbo_enabled;                  ///< Multi-band Operation enabled
  hf_bool_t ft_enabled;                   ///< 802.11r Fast Transition enabled
  hf_bool_t owe_enabled;                  ///< Opportunistic Wireless Encryption
  hf_bool_t transition_disable;           ///< Disable transition mode
  hf_u8_t sae_pwe_h2e;                   ///< SAE PWE derivation mode
  hf_u8_t failure_retry_cnt;             ///< Connection failure retry count
};

/**
 * @brief WiFi access point configuration structure.
 * @details Configuration parameters for access point mode.
 */
struct hf_esp_wifi_ap_config_t {
  char ssid[33];                          ///< SSID (32 bytes + null terminator)
  char password[65];                      ///< Password (64 bytes + null terminator)
  hf_u8_t ssid_len;                      ///< SSID length (0 = use strlen)
  hf_u8_t channel;                       ///< WiFi channel (1-14)
  hf_esp_wifi_auth_mode_t authmode;       ///< Authentication mode
  hf_u8_t ssid_hidden;                   ///< Hide SSID (0=broadcast, 1=hidden)
  hf_u8_t max_connection;                ///< Maximum number of stations
  hf_u16_t beacon_interval;              ///< Beacon interval (100-60000ms)
  hf_esp_wifi_cipher_type_t pairwise_cipher; ///< Pairwise cipher
  hf_bool_t ftm_responder;               ///< FTM responder support
  hf_bool_t pmf_capable;                 ///< PMF capable
  hf_bool_t pmf_required;                ///< PMF required
  hf_u8_t sae_pwe_h2e;                  ///< SAE PWE derivation mode
};

/**
 * @brief Advanced WiFi configuration for ESP32-C6.
 * @details Extended configuration for advanced features.
 */
struct hf_esp_wifi_advanced_config_t {
  hf_esp_wifi_protocol_t protocol_bitmap; ///< Enabled protocols
  hf_esp_wifi_bandwidth_t bandwidth;      ///< Channel bandwidth
  hf_bool_t country_policy;               ///< Country code policy auto
  hf_u8_t country_code[3];               ///< Country code (2 chars + null)
  hf_u8_t max_tx_power;                  ///< Maximum TX power
  hf_bool_t ampdu_rx_enable;             ///< A-MPDU RX enable
  hf_bool_t ampdu_tx_enable;             ///< A-MPDU TX enable
  hf_bool_t amsdu_tx_enable;             ///< A-MSDU TX enable
  hf_bool_t nvs_enable;                  ///< NVS WiFi storage
  hf_bool_t nano_enable;                 ///< Nano format enable
  hf_u16_t rx_ba_win;                    ///< RX block ack window size
  hf_u16_t wifi_task_core_id;           ///< WiFi task core ID
  hf_u16_t beacon_timeout;               ///< Beacon timeout
  hf_u16_t txop_limit_enable;            ///< TXOP limit enable
  hf_u16_t tx_ba_win;                    ///< TX block ack window size
  hf_u8_t rx_mgmt_action_on_off;        ///< RX management action enable
  hf_bool_t espnow_enable;               ///< ESP-NOW enable
  hf_bool_t magic_packet_filter_enable;  ///< Magic packet filter enable
};

/**
 * @brief WiFi enterprise configuration.
 * @details Configuration for WPA2/WPA3 Enterprise authentication.
 */
struct hf_esp_wifi_enterprise_config_t {
  hf_u8_t eap_method;                    ///< EAP method
  char identity[64];                      ///< Identity/username
  char username[64];                      ///< Username (for tunneled methods)
  char password[128];                     ///< Password
  char ca_cert[2048];                     ///< CA certificate
  char client_cert[2048];                 ///< Client certificate
  char client_key[2048];                  ///< Client private key
  hf_bool_t disable_time_check;          ///< Disable certificate time check
  hf_bool_t use_wpa2_task;               ///< Use WPA2 enterprise task
  hf_u8_t ttls_phase2_type;             ///< TTLS phase 2 type
};

//==============================================================================
// STATUS AND INFORMATION STRUCTURES
//==============================================================================

/**
 * @brief WiFi connection information structure.
 * @details Information about current WiFi connection.
 */
struct hf_esp_wifi_ap_record_t {
  hf_u8_t bssid[6];                      ///< BSSID of AP
  char ssid[33];                          ///< SSID of AP
  hf_u8_t primary;                       ///< Primary channel
  hf_u8_t second;                        ///< Secondary channel
  hf_i8_t rssi;                          ///< Signal strength
  hf_esp_wifi_auth_mode_t authmode;       ///< Authentication mode
  hf_esp_wifi_cipher_type_t pairwise_cipher; ///< Pairwise cipher
  hf_esp_wifi_cipher_type_t group_cipher; ///< Group cipher
  hf_u32_t phy_11b:1;                    ///< 802.11b support
  hf_u32_t phy_11g:1;                    ///< 802.11g support
  hf_u32_t phy_11n:1;                    ///< 802.11n support
  hf_u32_t phy_lr:1;                     ///< Long range support
  hf_u32_t phy_11ax:1;                   ///< 802.11ax support
  hf_u32_t wps:1;                        ///< WPS support
  hf_u32_t ftm_responder:1;              ///< FTM responder
  hf_u32_t ftm_initiator:1;              ///< FTM initiator
  hf_u32_t reserved:24;                  ///< Reserved
  hf_esp_wifi_bandwidth_t bandwidth;      ///< Bandwidth
};

/**
 * @brief WiFi statistics structure.
 * @details Comprehensive WiFi statistics for monitoring and debugging.
 */
struct hf_esp_wifi_stats_t {
  hf_u32_t tx_packets;                   ///< Transmitted packets
  hf_u32_t rx_packets;                   ///< Received packets
  hf_u32_t tx_bytes;                     ///< Transmitted bytes
  hf_u32_t rx_bytes;                     ///< Received bytes
  hf_u32_t tx_dropped;                   ///< Dropped TX packets
  hf_u32_t rx_dropped;                   ///< Dropped RX packets
  hf_u32_t tx_errors;                    ///< TX errors
  hf_u32_t rx_errors;                    ///< RX errors
  hf_u32_t beacon_timeout;               ///< Beacon timeouts
  hf_u32_t no_ack;                       ///< No ACK count
  hf_u32_t fcs_bad;                      ///< Bad FCS count
  hf_u32_t mib_timeout;                  ///< MIB timeout count
  hf_u32_t ack_timeout;                  ///< ACK timeout count
  hf_u32_t noise_floor;                  ///< Noise floor
  hf_i8_t rssi;                          ///< Current RSSI
  hf_u8_t channel;                       ///< Current channel
  hf_u32_t uptime_ms;                    ///< WiFi uptime in milliseconds
};

/**
 * @brief Station information for AP mode.
 * @details Information about connected stations.
 */
struct hf_esp_wifi_sta_info_t {
  hf_u8_t mac[6];                        ///< Station MAC address
  hf_i8_t rssi;                          ///< Station RSSI
  hf_u32_t phy_11b:1;                    ///< 802.11b support
  hf_u32_t phy_11g:1;                    ///< 802.11g support
  hf_u32_t phy_11n:1;                    ///< 802.11n support
  hf_u32_t phy_lr:1;                     ///< Long range support
  hf_u32_t phy_11ax:1;                   ///< 802.11ax support
  hf_u32_t is_mesh_child:1;              ///< Is mesh child
  hf_u32_t reserved:26;                  ///< Reserved
};

//==============================================================================
// EVENT AND SCAN STRUCTURES  
//==============================================================================

/**
 * @brief WiFi scan result structure.
 * @details Information about scanned access points.
 */
struct hf_esp_wifi_scan_result_t {
  hf_u8_t bssid[6];                      ///< BSSID
  char ssid[33];                          ///< SSID
  hf_u8_t channel;                       ///< Channel
  hf_i8_t rssi;                          ///< Signal strength
  hf_esp_wifi_auth_mode_t authmode;       ///< Authentication mode
  hf_bool_t wps;                         ///< WPS support
  hf_bool_t hidden;                      ///< Hidden SSID
  hf_esp_wifi_bandwidth_t bandwidth;      ///< Bandwidth
  hf_u32_t timestamp;                    ///< Scan timestamp
};

/**
 * @brief WiFi event data structure.
 * @details Data passed with WiFi events.
 */
struct hf_esp_wifi_event_data_t {
  hf_wifi_state_t old_state;             ///< Previous WiFi state
  hf_wifi_state_t new_state;             ///< Current WiFi state
  hf_u8_t reason;                        ///< Disconnect/failure reason
  hf_i8_t rssi;                          ///< Signal strength
  hf_u8_t channel;                       ///< Channel
  hf_u8_t bssid[6];                      ///< BSSID
  char ssid[33];                          ///< SSID
  hf_u32_t ip;                           ///< Assigned IP address
  hf_u32_t netmask;                      ///< Network mask
  hf_u32_t gateway;                      ///< Gateway address
};

//==============================================================================
// ERROR HANDLING AND UTILITIES
//==============================================================================

/**
 * @brief Convert ESP-IDF WiFi error to HardFOC error.
 * @param esp_err ESP-IDF error code
 * @return HardFOC WiFi error code
 */
constexpr hf_wifi_err_t HfConvertEspWifiError(hf_i32_t esp_err) noexcept {
  switch (esp_err) {
    case 0: // ESP_OK
      return hf_wifi_err_t::WIFI_SUCCESS;
    case 0x101: // ESP_ERR_NO_MEM
      return hf_wifi_err_t::WIFI_ERR_NO_MEMORY;
    case 0x102: // ESP_ERR_INVALID_ARG
      return hf_wifi_err_t::WIFI_ERR_INVALID_PARAM;
    case 0x103: // ESP_ERR_INVALID_STATE
      return hf_wifi_err_t::WIFI_ERR_NOT_INITIALIZED;
    case 0x106: // ESP_ERR_TIMEOUT
      return hf_wifi_err_t::WIFI_ERR_TIMEOUT;
    case 0x107: // ESP_ERR_NOT_FOUND
      return hf_wifi_err_t::WIFI_ERR_CONNECTION_FAILED;
    case 0x108: // ESP_ERR_NOT_SUPPORTED
      return hf_wifi_err_t::WIFI_ERR_INIT_FAILED;
    default:
      return hf_wifi_err_t::WIFI_ERR_FAILURE;
  }
}

/**
 * @brief Validate SSID string.
 * @param ssid SSID to validate
 * @return true if valid, false otherwise
 */
constexpr hf_bool_t HfIsValidSsid(const char* ssid) noexcept {
  if (!ssid) return false;
  
  hf_u8_t len = 0;
  while (ssid[len] != '\0' && len < 32) {
    len++;
  }
  
  return (len > 0 && len <= 32);
}

/**
 * @brief Validate password string.
 * @param password Password to validate
 * @return true if valid, false otherwise
 */
constexpr hf_bool_t HfIsValidPassword(const char* password) noexcept {
  if (!password) return false;
  
  hf_u8_t len = 0;
  while (password[len] != '\0' && len < 64) {
    len++;
  }
  
  // Password can be empty (for open networks) or 8-63 characters for secured
  return (len == 0 || (len >= 8 && len <= 63));
}

/**
 * @brief Validate WiFi channel.
 * @param channel Channel number to validate
 * @return true if valid, false otherwise
 */
constexpr hf_bool_t HfIsValidWifiChannel(hf_u8_t channel) noexcept {
  // Channels 1-14 are valid for 2.4GHz
  return (channel >= 1 && channel <= 14);
}

/**
 * @brief Convert RSSI to signal quality percentage.
 * @param rssi RSSI value in dBm
 * @return Signal quality percentage (0-100)
 */
constexpr hf_u8_t HfRssiToQuality(hf_i8_t rssi) noexcept {
  if (rssi <= -100) return 0;
  if (rssi >= -50) return 100;
  return static_cast<hf_u8_t>(2 * (rssi + 100));
}

/**
 * @brief Convert MAC address array to string representation.
 * @param mac MAC address array (6 bytes)
 * @param str Output string buffer (minimum 18 bytes)
 * @return true on success, false on error
 */
hf_bool_t HfMacAddressToString(const hf_u8_t mac[6], char* str) noexcept;

/**
 * @brief Convert string to MAC address array.
 * @param str String representation (e.g., "AA:BB:CC:DD:EE:FF")
 * @param mac Output MAC address array (6 bytes)
 * @return true on success, false on error
 */
hf_bool_t HfStringToMacAddress(const char* str, hf_u8_t mac[6]) noexcept;

//==============================================================================
// PLATFORM-SPECIFIC CONSTANTS
//==============================================================================

/**
 * @brief Maximum number of WiFi access points in scan results.
 */
constexpr hf_u16_t HF_ESP32_MAX_SCAN_RESULTS = 64;

/**
 * @brief Maximum number of concurrent connections in AP mode.
 */
constexpr hf_u8_t HF_ESP32_MAX_AP_CONNECTIONS = 10;

/**
 * @brief Default beacon interval in milliseconds.
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_BEACON_INTERVAL = 100;

/**
 * @brief Maximum TX power for ESP32-C6 in dBm.
 */
constexpr hf_i8_t HF_ESP32_MAX_WIFI_TX_POWER = 20;

/**
 * @brief Minimum TX power for ESP32-C6 in dBm.
 */
constexpr hf_i8_t HF_ESP32_MIN_WIFI_TX_POWER = 2;

/**
 * @brief Default scan timeout in milliseconds.
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_SCAN_TIMEOUT = 10000;

/**
 * @brief Default connection timeout in milliseconds.
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_CONNECT_TIMEOUT = 10000;

#endif // HF_MCU_FAMILY_ESP32