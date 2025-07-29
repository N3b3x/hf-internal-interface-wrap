/**
 * @file EspWifi.h
 * @brief Advanced ESP32 implementation of the unified BaseWifi class with ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseWifi class
 * for ESP32 microcontrollers with support for both basic and advanced WiFi features.
 * It supports ESP-IDF v5.5+ APIs, advanced power management, WPA3 security,
 * mesh networking capabilities, and enterprise-grade security features.
 * The implementation includes comprehensive event handling, connection management,
 * and performance optimizations specific to ESP32 hardware.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Requires ESP-IDF v5.5 or higher for full feature support
 * @note Thread-safe implementation with proper synchronization
 */

#pragma once

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_wps.h"
#include "esp_mesh.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#ifdef __cplusplus
}
#endif

// C++ headers
#include "BaseWifi.h"
#include "mcu/esp32/utils/EspTypes_Base.h"
#include <mutex>
#include <memory>
#include <atomic>
#include <queue>

/**
 * @brief ESP32-specific WiFi configuration extensions
 */
struct EspWifiAdvancedConfig {
  // Power management
  bool enable_power_save;               /**< Enable WiFi power save mode */
  wifi_ps_type_t power_save_type;       /**< Power save type */
  uint16_t listen_interval;             /**< Listen interval for power save */
  
  // Performance tuning
  uint8_t tx_power;                     /**< TX power (0-20 dBm) */
  wifi_bandwidth_t bandwidth;           /**< Channel bandwidth */
  bool enable_ampdu_rx;                 /**< Enable A-MPDU RX */
  bool enable_ampdu_tx;                 /**< Enable A-MPDU TX */
  
  // Advanced features
  bool enable_fast_connect;             /**< Enable fast connect */
  bool enable_pmf_required;             /**< Require PMF (Protected Management Frames) */
  bool enable_wpa3_transition;          /**< Enable WPA2/WPA3 transition mode */
  bool enable_11r;                      /**< Enable 802.11r Fast BSS Transition */
  bool enable_11k;                      /**< Enable 802.11k Radio Resource Management */
  bool enable_11v;                      /**< Enable 802.11v BSS Transition Management */
  
  // Enterprise features
  bool enable_enterprise;               /**< Enable WPA2/WPA3 Enterprise */
  std::string enterprise_username;      /**< Enterprise username */
  std::string enterprise_password;      /**< Enterprise password */
  std::string enterprise_ca_cert;       /**< CA certificate for enterprise */
  std::string enterprise_client_cert;   /**< Client certificate for enterprise */
  std::string enterprise_client_key;    /**< Client private key for enterprise */
  
  // Mesh networking
  bool enable_mesh;                     /**< Enable ESP-MESH */
  uint8_t mesh_max_layer;               /**< Maximum mesh layers */
  uint16_t mesh_max_connection;         /**< Maximum mesh connections */
  
  // Smart config and provisioning
  bool enable_smartconfig;              /**< Enable SmartConfig */
  smartconfig_type_t smartconfig_type;  /**< SmartConfig type */
  bool enable_wps;                      /**< Enable WPS */
  wps_type_t wps_type;                  /**< WPS type */
};

/**
 * @class EspWifi
 * @brief Advanced ESP32 implementation of unified BaseWifi with ESP-IDF v5.5+ features.
 * @details This class provides a comprehensive implementation of BaseWifi for ESP32
 *          microcontrollers with support for both basic and advanced features including:
 *
 *          **Basic Features:**
 *          - Station and Access Point modes
 *          - Network scanning and connection management
 *          - WPA/WPA2/WPA3 security support
 *          - Power management and optimization
 *          - Thread-safe state management
 *
 *          **Advanced Features (ESP-IDF v5.5+):**
 *          - WPA3 Personal and Enterprise security
 *          - 802.11k/r/v roaming standards
 *          - Protected Management Frames (PMF)
 *          - A-MPDU aggregation for performance
 *          - ESP-MESH networking
 *          - SmartConfig and WPS provisioning
 *          - Enterprise authentication (EAP-TLS, PEAP, etc.)
 *          - Advanced power save modes
 *          - Fast BSS transition and roaming
 *
 *          **Performance Optimizations:**
 *          - Hardware-accelerated cryptography
 *          - DMA-based data transfers
 *          - Optimized buffer management
 *          - Low-latency event handling
 *          - Memory pool management
 *
 *          **Thread Safety:**
 *          - All public methods are thread-safe
 *          - Internal state protection with mutexes
 *          - Atomic operations for status flags
 *          - Event queue synchronization
 */
class EspWifi : public BaseWifi {
public:
  /**
   * @brief Constructor with optional advanced configuration
   * @param advanced_config Advanced ESP32-specific configuration (optional)
   */
  explicit EspWifi(const EspWifiAdvancedConfig* advanced_config = nullptr);

  /**
   * @brief Destructor - ensures proper cleanup
   */
  virtual ~EspWifi();

  // ========== BaseWifi Interface Implementation ==========

  // Initialization and Configuration
  HfWifiErr init(HfWifiMode mode) override;
  HfWifiErr deinit() override;
  bool isInitialized() const override;
  HfWifiErr setMode(HfWifiMode mode) override;
  HfWifiMode getMode() const override;

  // Station Mode Operations
  HfWifiErr configureStation(const HfWifiStationConfig& config) override;
  HfWifiErr connect(uint32_t timeout_ms = 0) override;
  HfWifiErr disconnect() override;
  bool isConnected() const override;
  int8_t getRssi() const override;
  HfWifiErr getIpInfo(HfWifiIpInfo& ip_info) const override;

  // Access Point Mode Operations
  HfWifiErr configureAccessPoint(const HfWifiApConfig& config) override;
  HfWifiErr startAccessPoint() override;
  HfWifiErr stopAccessPoint() override;
  bool isAccessPointActive() const override;
  int getConnectedStationCount() const override;

  // Network Scanning
  HfWifiErr startScan(bool show_hidden = false, bool passive = false, uint32_t max_scan_time_ms = 0) override;
  HfWifiErr getScanResults(std::vector<HfWifiNetworkInfo>& networks, uint16_t max_networks = 0) override;
  bool isScanning() const override;

  // State and Status
  HfWifiState getState() const override;
  std::string getConnectedSsid() const override;
  HfWifiErr getConnectedBssid(uint8_t bssid[6]) const override;

  // Power Management
  HfWifiErr setPowerSave(HfWifiPowerSave mode) override;
  HfWifiPowerSave getPowerSave() const override;

  // Event Handling
  HfWifiErr registerEventCallback(HfWifiEventCallback callback) override;
  HfWifiErr unregisterEventCallback() override;

  // Utility Functions
  HfWifiErr getMacAddress(uint8_t mac[6], uint8_t interface = 0) const override;
  HfWifiErr setMacAddress(const uint8_t mac[6], uint8_t interface = 0) override;
  uint8_t getChannel() const override;
  HfWifiErr setChannel(uint8_t channel) override;

  // ========== ESP32-Specific Extensions ==========

  /**
   * @brief Set advanced ESP32-specific configuration
   * @param config Advanced configuration structure
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr setAdvancedConfig(const EspWifiAdvancedConfig& config);

  /**
   * @brief Get current advanced configuration
   * @param config Reference to store current configuration
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr getAdvancedConfig(EspWifiAdvancedConfig& config) const;

  /**
   * @brief Enable WPA3 transition mode (WPA2/WPA3 mixed)
   * @param enable True to enable transition mode
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr enableWpa3Transition(bool enable);

  /**
   * @brief Configure 802.11k/r/v roaming features
   * @param enable_11k Enable 802.11k Radio Resource Management
   * @param enable_11r Enable 802.11r Fast BSS Transition
   * @param enable_11v Enable 802.11v BSS Transition Management
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr configureRoaming(bool enable_11k, bool enable_11r, bool enable_11v);

  /**
   * @brief Configure WPA2/WPA3 Enterprise authentication
   * @param username Enterprise username
   * @param password Enterprise password
   * @param ca_cert CA certificate (optional)
   * @param client_cert Client certificate (optional)
   * @param client_key Client private key (optional)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr configureEnterprise(const std::string& username,
                                const std::string& password,
                                const std::string& ca_cert = "",
                                const std::string& client_cert = "",
                                const std::string& client_key = "");

  /**
   * @brief Start SmartConfig provisioning
   * @param type SmartConfig type
   * @param timeout_ms Provisioning timeout in milliseconds
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr startSmartConfig(smartconfig_type_t type = SC_TYPE_ESPTOUCH, uint32_t timeout_ms = 60000);

  /**
   * @brief Stop SmartConfig provisioning
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr stopSmartConfig();

  /**
   * @brief Start WPS provisioning
   * @param type WPS type
   * @param timeout_ms Provisioning timeout in milliseconds
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr startWps(wps_type_t type = WPS_TYPE_PBC, uint32_t timeout_ms = 120000);

  /**
   * @brief Stop WPS provisioning
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr stopWps();

  /**
   * @brief Initialize ESP-MESH networking
   * @param mesh_id Mesh network ID
   * @param max_layer Maximum mesh layers
   * @param max_connection Maximum connections per node
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr initMesh(const uint8_t mesh_id[6], uint8_t max_layer = 6, uint16_t max_connection = 10);

  /**
   * @brief Start ESP-MESH as root node
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr startMeshRoot();

  /**
   * @brief Start ESP-MESH as child node
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr startMeshChild();

  /**
   * @brief Stop ESP-MESH networking
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr stopMesh();

  /**
   * @brief Get WiFi statistics
   * @param stats Reference to store WiFi statistics
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr getStatistics(wifi_pkt_rx_ctrl_t& stats) const;

  /**
   * @brief Set WiFi TX power
   * @param power TX power in dBm (0-20)
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr setTxPower(uint8_t power);

  /**
   * @brief Get WiFi TX power
   * @return Current TX power in dBm, or -1 on error
   */
  int8_t getTxPower() const;

  /**
   * @brief Set WiFi channel bandwidth
   * @param bandwidth Channel bandwidth
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr setBandwidth(wifi_bandwidth_t bandwidth);

  /**
   * @brief Get WiFi channel bandwidth
   * @return Current channel bandwidth
   */
  wifi_bandwidth_t getBandwidth() const;

  /**
   * @brief Perform WiFi calibration
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr performCalibration();

  /**
   * @brief Get detailed connection information
   * @param info Reference to store connection information
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr getConnectionInfo(wifi_ap_record_t& info) const;

private:
  // ========== Internal State Management ==========
  
  mutable std::mutex m_mutex;                    /**< Main synchronization mutex */
  std::atomic<bool> m_initialized;               /**< Initialization state */
  std::atomic<bool> m_enabled;                   /**< WiFi enabled state */
  std::atomic<HfWifiMode> m_mode;                /**< Current WiFi mode */
  std::atomic<HfWifiState> m_state;              /**< Current WiFi state */
  
  // Configuration storage
  HfWifiStationConfig m_sta_config;              /**< Station configuration */
  HfWifiApConfig m_ap_config;                    /**< AP configuration */
  EspWifiAdvancedConfig m_advanced_config;       /**< Advanced configuration */
  
  // ESP-IDF handles
  esp_netif_t* m_sta_netif;                      /**< Station network interface */
  esp_netif_t* m_ap_netif;                       /**< AP network interface */
  esp_event_handler_instance_t m_wifi_event_handler; /**< WiFi event handler */
  esp_event_handler_instance_t m_ip_event_handler;   /**< IP event handler */
  
  // Event handling
  HfWifiEventCallback m_event_callback;          /**< User event callback */
  std::queue<std::pair<HfWifiEvent, void*>> m_event_queue; /**< Event queue */
  mutable std::mutex m_event_mutex;              /**< Event queue mutex */
  
  // Scan results
  std::vector<HfWifiNetworkInfo> m_scan_results; /**< Last scan results */
  std::atomic<bool> m_scanning;                  /**< Scanning state */
  mutable std::mutex m_scan_mutex;               /**< Scan results mutex */
  
  // Connection state
  std::atomic<bool> m_connected;                 /**< Connection state */
  std::atomic<bool> m_ap_active;                 /**< AP active state */
  std::atomic<int8_t> m_rssi;                    /**< Current RSSI */
  std::atomic<uint8_t> m_channel;                /**< Current channel */
  
  // Advanced features state
  std::atomic<bool> m_smartconfig_active;        /**< SmartConfig active */
  std::atomic<bool> m_wps_active;                /**< WPS active */
  std::atomic<bool> m_mesh_active;               /**< Mesh active */
  
  // ========== Internal Helper Methods ==========
  
  /**
   * @brief Initialize ESP-IDF network interface
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr initNetif();
  
  /**
   * @brief Deinitialize ESP-IDF network interface
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr deinitNetif();
  
  /**
   * @brief Register ESP-IDF event handlers
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr registerEventHandlers();
  
  /**
   * @brief Unregister ESP-IDF event handlers
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr unregisterEventHandlers();
  
  /**
   * @brief Convert HardFOC WiFi mode to ESP-IDF mode
   * @param mode HardFOC WiFi mode
   * @return ESP-IDF WiFi mode
   */
  wifi_mode_t convertToEspMode(HfWifiMode mode) const;
  
  /**
   * @brief Convert ESP-IDF WiFi mode to HardFOC mode
   * @param mode ESP-IDF WiFi mode
   * @return HardFOC WiFi mode
   */
  HfWifiMode convertFromEspMode(wifi_mode_t mode) const;
  
  /**
   * @brief Convert HardFOC security to ESP-IDF auth mode
   * @param security HardFOC security type
   * @return ESP-IDF auth mode
   */
  wifi_auth_mode_t convertToEspAuthMode(HfWifiSecurity security) const;
  
  /**
   * @brief Convert ESP-IDF auth mode to HardFOC security
   * @param auth_mode ESP-IDF auth mode
   * @return HardFOC security type
   */
  HfWifiSecurity convertFromEspAuthMode(wifi_auth_mode_t auth_mode) const;
  
  /**
   * @brief Convert ESP-IDF error to HardFOC error
   * @param esp_err ESP-IDF error code
   * @return HardFOC WiFi error code
   */
  HfWifiErr convertEspError(esp_err_t esp_err) const;
  
  /**
   * @brief Static WiFi event handler for ESP-IDF
   * @param arg User argument (EspWifi instance)
   * @param event_base Event base
   * @param event_id Event ID
   * @param event_data Event data
   */
  static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
  
  /**
   * @brief Static IP event handler for ESP-IDF
   * @param arg User argument (EspWifi instance)
   * @param event_base Event base
   * @param event_id Event ID
   * @param event_data Event data
   */
  static void ipEventHandler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data);
  
  /**
   * @brief Handle WiFi events internally
   * @param event_id ESP-IDF event ID
   * @param event_data Event data
   */
  void handleWifiEvent(int32_t event_id, void* event_data);
  
  /**
   * @brief Handle IP events internally
   * @param event_id ESP-IDF event ID
   * @param event_data Event data
   */
  void handleIpEvent(int32_t event_id, void* event_data);
  
  /**
   * @brief Notify user event callback
   * @param event HardFOC WiFi event
   * @param event_data Event data
   */
  void notifyEventCallback(HfWifiEvent event, void* event_data);
  
  /**
   * @brief Update internal state
   * @param new_state New WiFi state
   */
  void updateState(HfWifiState new_state);
  
  /**
   * @brief Apply advanced configuration settings
   * @return HfWifiErr::WIFI_SUCCESS on success, error code otherwise
   */
  HfWifiErr applyAdvancedConfig();
  
  /**
   * @brief Validate configuration parameters
   * @param config Configuration to validate
   * @return true if valid, false otherwise
   */
  bool validateConfig(const HfWifiStationConfig& config) const;
  
  /**
   * @brief Validate AP configuration parameters
   * @param config AP configuration to validate
   * @return true if valid, false otherwise
   */
  bool validateApConfig(const HfWifiApConfig& config) const;
  
  // Prevent copying
  EspWifi(const EspWifi&) = delete;
  EspWifi& operator=(const EspWifi&) = delete;
};