/**
 * @file EspTypes_WiFi.h
 * @brief ESP32 WiFi type definitions for hardware abstraction.
 *
 * This header defines the ESP32-specific types, constants, and utility functions
 * for WiFi operations. It provides a clean interface between the generic WiFi
 * base class and ESP-IDF specific implementations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This file should be included by ESP32 WiFi implementation files.
 * @note All definitions are specific to ESP32 with ESP-IDF v5.5+.
 */

#pragma once

#include "EspTypes_Base.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_wps.h"
#include "esp_mesh.h"
#include "lwip/ip_addr.h"

#ifdef __cplusplus
}
#endif

//==============================================================================
// ESP32 WIFI CONSTANTS
//==============================================================================

/// @brief Maximum WiFi SSID length
static constexpr size_t HF_ESP_WIFI_SSID_MAX_LEN = 32;

/// @brief Maximum WiFi password length
static constexpr size_t HF_ESP_WIFI_PASSWORD_MAX_LEN = 64;

/// @brief Maximum number of WiFi scan results
static constexpr uint16_t HF_ESP_WIFI_SCAN_MAX_RESULTS = 20;

/// @brief Default WiFi connection timeout
static constexpr hf_timeout_ms_t HF_ESP_WIFI_CONNECT_TIMEOUT_DEFAULT = 10000;

/// @brief Default WiFi scan timeout
static constexpr hf_timeout_ms_t HF_ESP_WIFI_SCAN_TIMEOUT_DEFAULT = 5000;

/// @brief Default AP beacon interval
static constexpr uint16_t HF_ESP_WIFI_BEACON_INTERVAL_DEFAULT = 100;

/// @brief Default AP maximum connections
static constexpr uint8_t HF_ESP_WIFI_MAX_CONNECTIONS_DEFAULT = 4;

/// @brief WiFi channel range
static constexpr uint8_t HF_ESP_WIFI_CHANNEL_MIN = 1;
static constexpr uint8_t HF_ESP_WIFI_CHANNEL_MAX = 14;

/// @brief WiFi TX power range (dBm)
static constexpr uint8_t HF_ESP_WIFI_TX_POWER_MIN = 0;
static constexpr uint8_t HF_ESP_WIFI_TX_POWER_MAX = 20;

//==============================================================================
// ESP32 WIFI TYPE MAPPINGS
//==============================================================================

/**
 * @brief ESP32-specific WiFi authentication mode mapping
 */
enum class hf_esp_wifi_auth_mode_t : uint8_t {
  OPEN = WIFI_AUTH_OPEN,                    /**< Open authentication */
  WEP = WIFI_AUTH_WEP,                      /**< WEP authentication */
  WPA_PSK = WIFI_AUTH_WPA_PSK,              /**< WPA-PSK authentication */
  WPA2_PSK = WIFI_AUTH_WPA2_PSK,            /**< WPA2-PSK authentication */
  WPA_WPA2_PSK = WIFI_AUTH_WPA_WPA2_PSK,    /**< WPA/WPA2-PSK authentication */
  WPA2_ENTERPRISE = WIFI_AUTH_WPA2_ENTERPRISE, /**< WPA2-Enterprise authentication */
  WPA3_PSK = WIFI_AUTH_WPA3_PSK,            /**< WPA3-PSK authentication */
  WPA2_WPA3_PSK = WIFI_AUTH_WPA2_WPA3_PSK,  /**< WPA2/WPA3-PSK authentication */
  WAPI_PSK = WIFI_AUTH_WAPI_PSK             /**< WAPI-PSK authentication */
};

/**
 * @brief ESP32-specific WiFi mode mapping
 */
enum class hf_esp_wifi_mode_t : uint8_t {
  NULL_MODE = WIFI_MODE_NULL,               /**< WiFi disabled */
  STA = WIFI_MODE_STA,                      /**< Station mode */
  AP = WIFI_MODE_AP,                        /**< Access Point mode */
  APSTA = WIFI_MODE_APSTA                   /**< Station + AP mode */
};

/**
 * @brief ESP32-specific WiFi power save mode mapping
 */
enum class hf_esp_wifi_power_save_t : uint8_t {
  NONE = WIFI_PS_NONE,                      /**< No power save */
  MIN_MODEM = WIFI_PS_MIN_MODEM,            /**< Minimum modem power save */
  MAX_MODEM = WIFI_PS_MAX_MODEM             /**< Maximum modem power save */
};

/**
 * @brief ESP32-specific WiFi bandwidth mapping
 */
enum class hf_esp_wifi_bandwidth_t : uint8_t {
  HT20 = WIFI_BW_HT20,                      /**< 20MHz bandwidth */
  HT40 = WIFI_BW_HT40                       /**< 40MHz bandwidth */
};

/**
 * @brief ESP32-specific WiFi sort method
 */
enum class hf_esp_wifi_sort_method_t : uint8_t {
  SIGNAL = WIFI_CONNECT_AP_BY_SIGNAL,       /**< Sort by signal strength */
  SECURITY = WIFI_CONNECT_AP_BY_SECURITY    /**< Sort by security level */
};

/**
 * @brief ESP32-specific WiFi scan method
 */
enum class hf_esp_wifi_scan_method_t : uint8_t {
  FAST = WIFI_FAST_SCAN,                    /**< Fast scan method */
  ALL_CHANNEL = WIFI_ALL_CHANNEL_SCAN       /**< All channel scan method */
};

//==============================================================================
// ESP32 WIFI STRUCTURES
//==============================================================================

/**
 * @brief ESP32-specific WiFi statistics structure
 */
struct HfEspWifiStats {
  uint32_t tx_packets;                      /**< Transmitted packets */
  uint32_t rx_packets;                      /**< Received packets */
  uint32_t tx_bytes;                        /**< Transmitted bytes */
  uint32_t rx_bytes;                        /**< Received bytes */
  uint32_t tx_errors;                       /**< Transmission errors */
  uint32_t rx_errors;                       /**< Reception errors */
  uint32_t tx_retries;                      /**< Transmission retries */
  uint32_t rx_missed;                       /**< Missed receptions */
  int8_t noise_floor;                       /**< Noise floor (dBm) */
  uint8_t channel_utilization;              /**< Channel utilization (%) */
};

/**
 * @brief ESP32-specific WiFi calibration data
 */
struct HfEspWifiCalibration {
  bool rf_cal_valid;                        /**< RF calibration valid */
  bool phy_cal_valid;                       /**< PHY calibration valid */
  uint32_t rf_cal_data[32];                 /**< RF calibration data */
  uint32_t phy_cal_data[16];                /**< PHY calibration data */
};

/**
 * @brief ESP32-specific WiFi performance configuration
 */
struct HfEspWifiPerformanceConfig {
  bool enable_ampdu_rx;                     /**< Enable A-MPDU RX */
  bool enable_ampdu_tx;                     /**< Enable A-MPDU TX */
  uint8_t ampdu_rx_num;                     /**< A-MPDU RX number */
  uint8_t ampdu_tx_num;                     /**< A-MPDU TX number */
  bool enable_amsdu;                        /**< Enable A-MSDU */
  uint16_t rx_ba_win;                       /**< RX block ACK window */
  uint16_t tx_ba_win;                       /**< TX block ACK window */
  bool enable_csi;                          /**< Enable CSI (Channel State Information) */
  bool enable_ftm;                          /**< Enable FTM (Fine Timing Measurement) */
};

//==============================================================================
// ESP32 WIFI UTILITY FUNCTIONS
//==============================================================================

#ifdef __cplusplus

/**
 * @brief Convert HardFOC WiFi security to ESP-IDF auth mode
 * @param security HardFOC WiFi security type
 * @return ESP-IDF WiFi auth mode
 */
inline wifi_auth_mode_t hfWifiSecurityToEspAuthMode(hf_wifi_security_t security) {
  switch (security) {
    case hf_wifi_security_t::OPEN: return WIFI_AUTH_OPEN;
    case hf_wifi_security_t::WEP: return WIFI_AUTH_WEP;
    case hf_wifi_security_t::WPA_PSK: return WIFI_AUTH_WPA_PSK;
    case hf_wifi_security_t::WPA2_PSK: return WIFI_AUTH_WPA2_PSK;
    case hf_wifi_security_t::WPA_WPA2_PSK: return WIFI_AUTH_WPA_WPA2_PSK;
    case hf_wifi_security_t::WPA2_ENTERPRISE: return WIFI_AUTH_WPA2_ENTERPRISE;
    case hf_wifi_security_t::WPA3_PSK: return WIFI_AUTH_WPA3_PSK;
    case hf_wifi_security_t::WPA2_WPA3_PSK: return WIFI_AUTH_WPA2_WPA3_PSK;
    case hf_wifi_security_t::WAPI_PSK: return WIFI_AUTH_WAPI_PSK;
    default: return WIFI_AUTH_OPEN;
  }
}

/**
 * @brief Convert ESP-IDF auth mode to HardFOC WiFi security
 * @param auth_mode ESP-IDF WiFi auth mode
 * @return HardFOC WiFi security type
 */
inline hf_wifi_security_t espAuthModeTohf_wifi_security_t(wifi_auth_mode_t auth_mode) {
  switch (auth_mode) {
    case WIFI_AUTH_OPEN: return hf_wifi_security_t::OPEN;
    case WIFI_AUTH_WEP: return hf_wifi_security_t::WEP;
    case WIFI_AUTH_WPA_PSK: return hf_wifi_security_t::WPA_PSK;
    case WIFI_AUTH_WPA2_PSK: return hf_wifi_security_t::WPA2_PSK;
    case WIFI_AUTH_WPA_WPA2_PSK: return hf_wifi_security_t::WPA_WPA2_PSK;
    case WIFI_AUTH_WPA2_ENTERPRISE: return hf_wifi_security_t::WPA2_ENTERPRISE;
    case WIFI_AUTH_WPA3_PSK: return hf_wifi_security_t::WPA3_PSK;
    case WIFI_AUTH_WPA2_WPA3_PSK: return hf_wifi_security_t::WPA2_WPA3_PSK;
    case WIFI_AUTH_WAPI_PSK: return hf_wifi_security_t::WAPI_PSK;
    default: return hf_wifi_security_t::OPEN;
  }
}

/**
 * @brief Convert HardFOC WiFi mode to ESP-IDF mode
 * @param mode HardFOC WiFi mode
 * @return ESP-IDF WiFi mode
 */
inline wifi_mode_t hfWifiModeToEspMode(hf_wifi_mode_t mode) {
  switch (mode) {
    case hf_wifi_mode_t::HF_WIFI_MODE_STATION: return WIFI_MODE_STA;
    case hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT: return WIFI_MODE_AP;
    case hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP: return WIFI_MODE_APSTA;
    case hf_wifi_mode_t::HF_WIFI_MODE_DISABLED: return WIFI_MODE_NULL;
    default: return WIFI_MODE_NULL;
  }
}

/**
 * @brief Convert ESP-IDF mode to HardFOC WiFi mode
 * @param mode ESP-IDF WiFi mode
 * @return HardFOC WiFi mode
 */
inline hf_wifi_mode_t espModeTohf_wifi_mode_t(wifi_mode_t mode) {
  switch (mode) {
    case WIFI_MODE_STA: return hf_wifi_mode_t::HF_WIFI_MODE_STATION;
    case WIFI_MODE_AP: return hf_wifi_mode_t::HF_WIFI_MODE_ACCESS_POINT;
    case WIFI_MODE_APSTA: return hf_wifi_mode_t::HF_WIFI_MODE_STATION_AP;
    case WIFI_MODE_NULL: return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
    default: return hf_wifi_mode_t::HF_WIFI_MODE_DISABLED;
  }
}

/**
 * @brief Convert HardFOC power save mode to ESP-IDF power save
 * @param power_save HardFOC power save mode
 * @return ESP-IDF power save type
 */
inline wifi_ps_type_t hfWifiPowerSaveToEspPowerSave(hf_wifi_power_save_t power_save) {
  switch (power_save) {
    case hf_wifi_power_save_t::NONE: return WIFI_PS_NONE;
    case hf_wifi_power_save_t::MIN_MODEM: return WIFI_PS_MIN_MODEM;
    case hf_wifi_power_save_t::MAX_MODEM: return WIFI_PS_MAX_MODEM;
    default: return WIFI_PS_NONE;
  }
}

/**
 * @brief Convert ESP-IDF power save to HardFOC power save mode
 * @param power_save ESP-IDF power save type
 * @return HardFOC power save mode
 */
inline hf_wifi_power_save_t espPowerSaveTohf_wifi_power_save_t(wifi_ps_type_t power_save) {
  switch (power_save) {
    case WIFI_PS_NONE: return hf_wifi_power_save_t::NONE;
    case WIFI_PS_MIN_MODEM: return hf_wifi_power_save_t::MIN_MODEM;
    case WIFI_PS_MAX_MODEM: return hf_wifi_power_save_t::MAX_MODEM;
    default: return hf_wifi_power_save_t::NONE;
  }
}

/**
 * @brief Convert ESP-IDF error to HardFOC WiFi error
 * @param esp_err ESP-IDF error code
 * @return HardFOC WiFi error code
 */
inline HfWifiErr espErrorToHfWifiError(esp_err_t esp_err) {
  switch (esp_err) {
    case ESP_OK: return HfWifiErr::WIFI_SUCCESS;
    case ESP_ERR_INVALID_ARG: return HfWifiErr::WIFI_ERR_INVALID_PARAM;
    case ESP_ERR_INVALID_STATE: return HfWifiErr::WIFI_ERR_INVALID_STATE;
    case ESP_ERR_NO_MEM: return HfWifiErr::WIFI_ERR_NO_MEMORY;
    case ESP_ERR_TIMEOUT: return HfWifiErr::WIFI_ERR_TIMEOUT;
    case ESP_ERR_WIFI_NOT_INIT: return HfWifiErr::WIFI_ERR_NOT_INITIALIZED;
    case ESP_ERR_WIFI_NOT_STARTED: return HfWifiErr::WIFI_ERR_NOT_INITIALIZED;
    case ESP_ERR_WIFI_CONN: return HfWifiErr::WIFI_ERR_CONNECTION_FAILED;
    case ESP_ERR_WIFI_SSID: return HfWifiErr::WIFI_ERR_INVALID_SSID;
    case ESP_ERR_WIFI_PASSWORD: return HfWifiErr::WIFI_ERR_INVALID_PASSWORD;
    case ESP_ERR_WIFI_TIMEOUT: return HfWifiErr::WIFI_ERR_TIMEOUT;
    case ESP_ERR_WIFI_WAKE_FAIL: return HfWifiErr::WIFI_ERR_FAILURE;
    case ESP_ERR_WIFI_WOULD_BLOCK: return HfWifiErr::WIFI_ERR_TIMEOUT;
    default: return HfWifiErr::WIFI_ERR_FAILURE;
  }
}

/**
 * @brief Convert IP address from lwIP format to uint32_t
 * @param ip_addr lwIP IP address
 * @return IP address as uint32_t
 */
inline uint32_t lwipIpToUint32(const ip4_addr_t& ip_addr) {
  return ip_addr.addr;
}

/**
 * @brief Convert IP address from uint32_t to lwIP format
 * @param ip_uint32 IP address as uint32_t
 * @return lwIP IP address
 */
inline ip4_addr_t uint32ToLwipIp(uint32_t ip_uint32) {
  ip4_addr_t ip_addr;
  ip_addr.addr = ip_uint32;
  return ip_addr;
}

/**
 * @brief Check if WiFi channel is valid
 * @param channel WiFi channel number
 * @return true if valid, false otherwise
 */
inline bool isValidWifiChannel(uint8_t channel) {
  return (channel >= HF_ESP_WIFI_CHANNEL_MIN && channel <= HF_ESP_WIFI_CHANNEL_MAX);
}

/**
 * @brief Check if WiFi TX power is valid
 * @param tx_power TX power in dBm
 * @return true if valid, false otherwise
 */
inline bool isValidWifiTxPower(uint8_t tx_power) {
  return (tx_power >= HF_ESP_WIFI_TX_POWER_MIN && tx_power <= HF_ESP_WIFI_TX_POWER_MAX);
}

/**
 * @brief Check if SSID is valid
 * @param ssid SSID string
 * @return true if valid, false otherwise
 */
inline bool isValidSsid(const std::string& ssid) {
  return (!ssid.empty() && ssid.length() <= HF_ESP_WIFI_SSID_MAX_LEN);
}

/**
 * @brief Check if WiFi password is valid
 * @param password Password string
 * @param security Security type
 * @return true if valid, false otherwise
 */
inline bool isValidWifiPassword(const std::string& password, hf_wifi_security_t security) {
  if (security == hf_wifi_security_t::OPEN) {
    return password.empty();
  }
  
  if (security == hf_wifi_security_t::WEP) {
    // WEP key length constants
    static constexpr size_t WEP_KEY_64_BIT_ASCII = 5;   // 64-bit WEP ASCII key
    static constexpr size_t WEP_KEY_128_BIT_ASCII = 13; // 128-bit WEP ASCII key
    static constexpr size_t WEP_KEY_128_BIT_HEX = 16;   // 128-bit WEP hexadecimal key
    static constexpr size_t WEP_KEY_152_BIT_ASCII = 29; // 152-bit WEP ASCII key
    
    size_t len = password.length();
    return (len == WEP_KEY_64_BIT_ASCII || len == WEP_KEY_128_BIT_ASCII || 
            len == WEP_KEY_128_BIT_HEX || len == WEP_KEY_152_BIT_ASCII);
  }
  
  // WPA/WPA2/WPA3 passwords must be 8-63 characters
  return (password.length() >= 8 && password.length() <= 63);
}

/**
 * @brief Convert WiFi scan result to HardFOC network info
 * @param scan_record ESP-IDF scan record
 * @param network_info HardFOC network info output
 */
inline void espScanRecordToHfNetworkInfo(const wifi_ap_record_t& scan_record, HfWifiNetworkInfo& network_info) {
  network_info.ssid = std::string(reinterpret_cast<const char*>(scan_record.ssid));
  std::memcpy(network_info.bssid, scan_record.bssid, 6);
  network_info.security = espAuthModeTohf_wifi_security_t(scan_record.authmode);
  network_info.rssi = scan_record.rssi;
  network_info.channel = scan_record.primary;
  network_info.hidden = (network_info.ssid.empty());
}

/**
 * @brief Convert HardFOC station config to ESP-IDF config
 * @param hf_config HardFOC station configuration
 * @param esp_config ESP-IDF station configuration output
 */
inline void hfStationConfigToEspConfig(const HfWifiStationConfig& hf_config, wifi_sta_config_t& esp_config) {
  std::memset(&esp_config, 0, sizeof(esp_config));
  
  // Copy SSID
  std::strncpy(reinterpret_cast<char*>(esp_config.ssid), hf_config.ssid.c_str(), sizeof(esp_config.ssid) - 1);
  
  // Copy password
  std::strncpy(reinterpret_cast<char*>(esp_config.password), hf_config.password.c_str(), sizeof(esp_config.password) - 1);
  
  // Copy BSSID if set
  if (hf_config.bssid_set) {
    std::memcpy(esp_config.bssid, hf_config.bssid, 6);
    esp_config.bssid_set = true;
  }
  
  // Set channel
  esp_config.channel = hf_config.channel;
  
  // Set scan method
  esp_config.scan_method = static_cast<wifi_scan_method_t>(hf_config.scan_method);
  
  // Set sort method
  esp_config.sort_method = static_cast<wifi_sort_method_t>(hf_config.sort_method);
  
  // Set thresholds
  esp_config.threshold.rssi = hf_config.threshold_rssi;
  esp_config.threshold.authmode = hfWifiSecurityToEspAuthMode(hf_config.threshold_authmode);
}

/**
 * @brief Convert HardFOC AP config to ESP-IDF config
 * @param hf_config HardFOC AP configuration
 * @param esp_config ESP-IDF AP configuration output
 */
inline void hfApConfigToEspConfig(const HfWifiApConfig& hf_config, wifi_ap_config_t& esp_config) {
  std::memset(&esp_config, 0, sizeof(esp_config));
  
  // Copy SSID
  std::strncpy(reinterpret_cast<char*>(esp_config.ssid), hf_config.ssid.c_str(), sizeof(esp_config.ssid) - 1);
  esp_config.ssid_len = hf_config.ssid_len;
  
  // Copy password
  std::strncpy(reinterpret_cast<char*>(esp_config.password), hf_config.password.c_str(), sizeof(esp_config.password) - 1);
  
  // Set other parameters
  esp_config.channel = hf_config.channel;
  esp_config.authmode = hfWifiSecurityToEspAuthMode(hf_config.authmode);
  esp_config.ssid_hidden = hf_config.ssid_hidden;
  esp_config.max_connection = hf_config.max_connection;
  esp_config.beacon_interval = hf_config.beacon_interval;
}

#endif // __cplusplus