/**
 * @file BaseWifi.h
 * @ingroup wifi
 * @brief Unified WiFi base class for all WiFi implementations.
 *
 * This file contains the declaration of the BaseWifi abstract class, which provides
 * a comprehensive WiFi abstraction that serves as the base for all WiFi
 * implementations in the HardFOC system. It supports station and access point modes,
 * security configurations, connection management, and works across different
 * hardware platforms including ESP32, and other WiFi-capable MCUs.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string>

/**
 * @defgroup wifi WiFi Module
 * @brief All WiFi-related types, enums, and functions for wireless networking operations.
 * @details This module provides comprehensive WiFi functionality including:
 *          - Station mode (client) operations
 *          - Access Point mode operations
 *          - Network scanning and discovery
 *          - Security configuration (WPA/WPA2/WPA3)
 *          - Connection management
 *          - Event handling and callbacks
 *          - Error handling and diagnostics
 */

/**
 * @ingroup wifi
 * @brief HardFOC WiFi error codes macro list
 * @details X-macro pattern for comprehensive error enumeration. Each entry contains:
 *          X(NAME, VALUE, DESCRIPTION)
 */
#define HF_WIFI_ERR_LIST(X)                                              \
  /* Success codes */                                                    \
  X(WIFI_SUCCESS, 0, "Success")                                          \
                                                                         \
  /* General errors */                                                   \
  X(WIFI_ERR_FAILURE, 1, "General failure")                             \
  X(WIFI_ERR_INVALID_PARAM, 2, "Invalid parameter")                     \
  X(WIFI_ERR_NOT_INITIALIZED, 3, "WiFi not initialized")                \
  X(WIFI_ERR_ALREADY_INITIALIZED, 4, "WiFi already initialized")        \
  X(WIFI_ERR_NOT_CONNECTED, 5, "WiFi not connected")                    \
  X(WIFI_ERR_ALREADY_CONNECTED, 6, "WiFi already connected")            \
  X(WIFI_ERR_CONNECTION_FAILED, 7, "Connection failed")                 \
  X(WIFI_ERR_DISCONNECTION_FAILED, 8, "Disconnection failed")           \
  X(WIFI_ERR_SCAN_FAILED, 9, "Network scan failed")                     \
  X(WIFI_ERR_AP_START_FAILED, 10, "Access Point start failed")          \
  X(WIFI_ERR_AP_STOP_FAILED, 11, "Access Point stop failed")            \
  X(WIFI_ERR_TIMEOUT, 12, "Operation timeout")                          \
  X(WIFI_ERR_NO_MEMORY, 13, "Insufficient memory")                      \
  X(WIFI_ERR_INVALID_SSID, 14, "Invalid SSID")                          \
  X(WIFI_ERR_INVALID_PASSWORD, 15, "Invalid password")                  \
  X(WIFI_ERR_WEAK_SIGNAL, 16, "Weak signal strength")                   \
  X(WIFI_ERR_AUTHENTICATION_FAILED, 17, "Authentication failed")        \
  X(WIFI_ERR_ASSOCIATION_FAILED, 18, "Association failed")              \
  X(WIFI_ERR_HANDSHAKE_FAILED, 19, "4-way handshake failed")

/**
 * @ingroup wifi
 * @brief Generate WiFi error enumeration using X-macro pattern
 */
#define X(name, value, desc) name = value,
enum class HfWifiErr : uint8_t { HF_WIFI_ERR_LIST(X) };
#undef X

/**
 * @ingroup wifi
 * @brief WiFi operating modes
 */
enum class HfWifiMode : uint8_t {
  STATION = 0,      /**< Station mode (client) */
  ACCESS_POINT = 1, /**< Access Point mode */
  STATION_AP = 2,   /**< Station + Access Point mode */
  DISABLED = 3      /**< WiFi disabled */
};

/**
 * @ingroup wifi
 * @brief WiFi security types
 */
enum class HfWifiSecurity : uint8_t {
  OPEN = 0,        /**< Open network (no security) */
  WEP = 1,         /**< WEP security (deprecated) */
  WPA_PSK = 2,     /**< WPA Personal */
  WPA2_PSK = 3,    /**< WPA2 Personal */
  WPA_WPA2_PSK = 4,/**< WPA/WPA2 Mixed Personal */
  WPA2_ENTERPRISE = 5, /**< WPA2 Enterprise */
  WPA3_PSK = 6,    /**< WPA3 Personal */
  WPA2_WPA3_PSK = 7,   /**< WPA2/WPA3 Mixed Personal */
  WAPI_PSK = 8     /**< WAPI Personal */
};

/**
 * @ingroup wifi
 * @brief WiFi connection states
 */
enum class HfWifiState : uint8_t {
  DISCONNECTED = 0,     /**< Disconnected from network */
  CONNECTING = 1,       /**< Attempting to connect */
  CONNECTED = 2,        /**< Connected to network */
  DISCONNECTING = 3,    /**< Disconnecting from network */
  RECONNECTING = 4,     /**< Attempting to reconnect */
  AP_STARTED = 5,       /**< Access Point started */
  AP_STOPPED = 6,       /**< Access Point stopped */
  SCANNING = 7          /**< Scanning for networks */
};

/**
 * @ingroup wifi
 * @brief WiFi power save modes
 */
enum class HfWifiPowerSave : uint8_t {
  NONE = 0,           /**< No power save */
  MIN_MODEM = 1,      /**< Minimum modem power save */
  MAX_MODEM = 2       /**< Maximum modem power save */
};

/**
 * @ingroup wifi
 * @brief WiFi event types for callback functions
 */
enum class HfWifiEvent : uint8_t {
  STA_START = 0,              /**< Station start */
  STA_STOP = 1,               /**< Station stop */
  STA_CONNECTED = 2,          /**< Station connected to AP */
  STA_DISCONNECTED = 3,       /**< Station disconnected from AP */
  STA_AUTHMODE_CHANGE = 4,    /**< Station auth mode changed */
  STA_GOT_IP = 5,             /**< Station got IP from DHCP */
  STA_LOST_IP = 6,            /**< Station lost IP */
  AP_START = 7,               /**< Access Point started */
  AP_STOP = 8,                /**< Access Point stopped */
  AP_STACONNECTED = 9,        /**< Station connected to our AP */
  AP_STADISCONNECTED = 10,    /**< Station disconnected from our AP */
  SCAN_DONE = 11              /**< Network scan completed */
};

/**
 * @ingroup wifi
 * @brief WiFi network information structure
 */
struct HfWifiNetworkInfo {
  std::string ssid;                    /**< Network SSID */
  uint8_t bssid[6];                   /**< Network BSSID (MAC address) */
  HfWifiSecurity security;            /**< Security type */
  int8_t rssi;                        /**< Signal strength (dBm) */
  uint8_t channel;                    /**< WiFi channel */
  bool hidden;                        /**< True if network is hidden */
};

/**
 * @ingroup wifi
 * @brief WiFi station configuration structure
 */
struct HfWifiStationConfig {
  std::string ssid;                   /**< Target network SSID */
  std::string password;               /**< Network password */
  uint8_t bssid[6];                  /**< Target BSSID (optional, all zeros if not used) */
  uint8_t channel;                   /**< Target channel (0 for any) */
  bool bssid_set;                    /**< True if BSSID should be used */
  uint32_t scan_method;              /**< Scan method */
  bool sort_method;                  /**< Sort method for AP list */
  uint16_t threshold_rssi;           /**< Minimum RSSI threshold */
  HfWifiSecurity threshold_authmode; /**< Minimum auth mode threshold */
};

/**
 * @ingroup wifi
 * @brief WiFi Access Point configuration structure
 */
struct HfWifiApConfig {
  std::string ssid;                  /**< AP SSID */
  std::string password;              /**< AP password */
  uint8_t ssid_len;                  /**< SSID length (0 for auto) */
  uint8_t channel;                   /**< WiFi channel */
  HfWifiSecurity authmode;           /**< Authentication mode */
  uint8_t ssid_hidden;               /**< Broadcast SSID (0) or hide (1) */
  uint8_t max_connection;            /**< Maximum concurrent connections */
  uint16_t beacon_interval;          /**< Beacon interval (ms) */
};

/**
 * @ingroup wifi
 * @brief WiFi IP configuration structure
 */
struct HfWifiIpInfo {
  uint32_t ip;                       /**< IP address */
  uint32_t netmask;                  /**< Netmask */
  uint32_t gateway;                  /**< Gateway address */
};

/**
 * @ingroup wifi
 * @brief WiFi event callback function type
 */
using HfWifiEventCallback = std::function<void(HfWifiEvent event, void* event_data)>;

/**
 * @class BaseWifi
 * @ingroup wifi
 * @brief Abstract base class for WiFi functionality
 * @details This class provides the interface for WiFi operations including:
 *          - Station mode operations (connecting to networks)
 *          - Access Point mode operations (creating hotspots)
 *          - Network scanning and discovery
 *          - Security configuration
 *          - Event handling and callbacks
 *          - Power management
 */
class BaseWifi {
public:
  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~BaseWifi() = default;

  // ========== Initialization and Configuration ==========

  /**
   * @brief Initialize the WiFi subsystem
   * @param mode WiFi operating mode
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr init(HfWifiMode mode) = 0;

  /**
   * @brief Deinitialize the WiFi subsystem
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr deinit() = 0;

  /**
   * @brief Check if WiFi is initialized
   * @return true if initialized, false otherwise
   */
  virtual bool isInitialized() const = 0;

  /**
   * @brief Set WiFi operating mode
   * @param mode WiFi operating mode
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr setMode(HfWifiMode mode) = 0;

  /**
   * @brief Get current WiFi operating mode
   * @return Current WiFi mode
   */
  virtual HfWifiMode getMode() const = 0;

  // ========== Station Mode Operations ==========

  /**
   * @brief Configure station parameters
   * @param config Station configuration
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr configureStation(const HfWifiStationConfig& config) = 0;

  /**
   * @brief Connect to a WiFi network (station mode)
   * @param timeout_ms Connection timeout in milliseconds (0 for default)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr connect(uint32_t timeout_ms = 0) = 0;

  /**
   * @brief Disconnect from WiFi network
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr disconnect() = 0;

  /**
   * @brief Check if connected to a network
   * @return true if connected, false otherwise
   */
  virtual bool isConnected() const = 0;

  /**
   * @brief Get signal strength (RSSI)
   * @return Signal strength in dBm, or INT8_MIN on error
   */
  virtual int8_t getRssi() const = 0;

  /**
   * @brief Get current IP information
   * @param ip_info Reference to store IP information
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr getIpInfo(HfWifiIpInfo& ip_info) const = 0;

  // ========== Access Point Mode Operations ==========

  /**
   * @brief Configure Access Point parameters
   * @param config AP configuration
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr configureAccessPoint(const HfWifiApConfig& config) = 0;

  /**
   * @brief Start Access Point
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr startAccessPoint() = 0;

  /**
   * @brief Stop Access Point
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr stopAccessPoint() = 0;

  /**
   * @brief Check if Access Point is running
   * @return true if AP is active, false otherwise
   */
  virtual bool isAccessPointActive() const = 0;

  /**
   * @brief Get number of connected stations
   * @return Number of connected stations, or -1 on error
   */
  virtual int getConnectedStationCount() const = 0;

  // ========== Network Scanning ==========

  /**
   * @brief Start network scan
   * @param show_hidden Include hidden networks in scan
   * @param passive Use passive scanning
   * @param max_scan_time_ms Maximum scan time per channel
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr startScan(bool show_hidden = false, bool passive = false, uint32_t max_scan_time_ms = 0) = 0;

  /**
   * @brief Get scan results
   * @param networks Vector to store found networks
   * @param max_networks Maximum number of networks to return (0 for all)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr getScanResults(std::vector<HfWifiNetworkInfo>& networks, uint16_t max_networks = 0) = 0;

  /**
   * @brief Check if scan is in progress
   * @return true if scanning, false otherwise
   */
  virtual bool isScanning() const = 0;

  // ========== State and Status ==========

  /**
   * @brief Get current WiFi state
   * @return Current WiFi state
   */
  virtual HfWifiState getState() const = 0;

  /**
   * @brief Get connected network SSID
   * @return SSID string, empty if not connected
   */
  virtual std::string getConnectedSsid() const = 0;

  /**
   * @brief Get connected network BSSID
   * @param bssid Buffer to store BSSID (6 bytes)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr getConnectedBssid(uint8_t bssid[6]) const = 0;

  // ========== Power Management ==========

  /**
   * @brief Set power save mode
   * @param mode Power save mode
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr setPowerSave(HfWifiPowerSave mode) = 0;

  /**
   * @brief Get current power save mode
   * @return Current power save mode
   */
  virtual HfWifiPowerSave getPowerSave() const = 0;

  // ========== Event Handling ==========

  /**
   * @brief Register event callback
   * @param callback Event callback function
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr registerEventCallback(HfWifiEventCallback callback) = 0;

  /**
   * @brief Unregister event callback
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr unregisterEventCallback() = 0;

  // ========== Utility Functions ==========

  /**
   * @brief Get MAC address
   * @param mac Buffer to store MAC address (6 bytes)
   * @param interface WiFi interface (0 for station, 1 for AP)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr getMacAddress(uint8_t mac[6], uint8_t interface = 0) const = 0;

  /**
   * @brief Set MAC address
   * @param mac MAC address to set (6 bytes)
   * @param interface WiFi interface (0 for station, 1 for AP)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr setMacAddress(const uint8_t mac[6], uint8_t interface = 0) = 0;

  /**
   * @brief Get WiFi channel
   * @return Current channel, or 0 on error
   */
  virtual uint8_t getChannel() const = 0;

  /**
   * @brief Set WiFi channel
   * @param channel Channel number (1-14)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  virtual HfWifiErr setChannel(uint8_t channel) = 0;

  /**
   * @brief Get error description string
   * @param error Error code
   * @return Error description string
   */
  static const char* getErrorString(HfWifiErr error);

protected:
  /**
   * @brief Protected constructor - only derived classes can instantiate
   */
  BaseWifi() = default;

  /**
   * @brief Copy constructor - deleted to prevent copying
   */
  BaseWifi(const BaseWifi&) = delete;

  /**
   * @brief Assignment operator - deleted to prevent copying
   */
  BaseWifi& operator=(const BaseWifi&) = delete;
};

/**
 * @ingroup wifi
 * @brief Helper function to convert error enum to string
 */
inline const char* BaseWifi::getErrorString(HfWifiErr error) {
#define X(name, value, desc) case HfWifiErr::name: return desc;
  switch (error) {
    HF_WIFI_ERR_LIST(X)
    default: return "Unknown error";
  }
#undef X
}