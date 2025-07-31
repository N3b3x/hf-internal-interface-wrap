/**
 * @file EspWifi.h
 * @brief Advanced ESP32-C6 implementation of the unified BaseWifi class with ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseWifi class
 * for ESP32-C6 microcontrollers with support for WiFi 6 (802.11ax) and advanced features.
 * It supports station and access point modes, WPA3 security, enterprise authentication,
 * mesh networking, and advanced ESP32-C6-specific features like power management,
 * roaming, and performance optimization. The implementation includes comprehensive
 * event handling and hardware-accelerated operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation requires ESP32-C6 with ESP-IDF v5.5+ for full feature support.
 * @note Thread-safe implementation with proper synchronization mechanisms.
 */

#pragma once

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_wps.h"

#ifdef __cplusplus
}
#endif

// C++ headers
#include "BaseWifi.h"
#include "mcu/esp32/utils/EspTypes_WiFi.h"
#include <atomic>
#include <memory>
#include <vector>

/**
 * @class EspWifi
 * @brief Advanced ESP32-C6 implementation of unified BaseWifi with ESP-IDF v5.5+ features.
 * @details This class provides a comprehensive implementation of BaseWifi for ESP32-C6
 *          microcontrollers with support for both basic and advanced features including:
 *
 *          **Basic Features:**
 *          - Station and Access Point modes
 *          - Network scanning and connection management
 *          - WPA/WPA2/WPA3 security support
 *          - Power management and optimization
 *          - Thread-safe state management
 *
 *          **Advanced Features (ESP32-C6/ESP-IDF v5.5+):**
 *          - WiFi 6 (802.11ax) support
 *          - WPA3 Personal and Enterprise security
 *          - 802.11k/r/v roaming standards
 *          - Protected Management Frames (PMF)
 *          - ESP-MESH networking
 *          - SmartConfig and WPS provisioning
 *          - Enterprise authentication (EAP-TLS, PEAP, etc.)
 *          - Advanced power save modes
 *          - Fast BSS transition and roaming
 *
 * @note This class is designed for ESP32-C6 with WiFi 6 capabilities.
 * @note Advanced features require ESP32-C6 with ESP-IDF v5.5+ for full functionality.
 */
class EspWifi : public BaseWifi {
public:
  //==============================================================//
  // CONSTRUCTORS
  //==============================================================//

  /**
   * @brief Constructor for EspWifi with advanced configuration.
   * @param enable_sta Enable station mode
   * @param enable_ap Enable access point mode
   * @param device_name Device hostname
   * @details Creates an ESP32-C6 WiFi instance with the specified configuration.
   *          **LAZY INITIALIZATION**: WiFi is NOT started until the first call
   *          to Initialize() or any WiFi operation.
   */
  explicit EspWifi(hf_bool_t enable_sta = true,
                   hf_bool_t enable_ap = false,
                   const std::string& device_name = "ESP32-C6") noexcept;

  /**
   * @brief Destructor - ensures proper cleanup of WiFi resources.
   */
  ~EspWifi() override;

  //==============================================================//
  // BASEWIFI IMPLEMENTATION
  //==============================================================//

  /**
   * @brief Initialize WiFi adapter with specified mode.
   * @param mode WiFi operating mode
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t Initialize(hf_wifi_mode_t mode) override;

  /**
   * @brief Deinitialize WiFi adapter and cleanup resources.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t Deinitialize() override;

  /**
   * @brief Check if WiFi is initialized.
   * @return true if initialized, false otherwise
   */
  hf_bool_t IsInitialized() const noexcept override;

  /**
   * @brief Set WiFi operating mode.
   * @param mode WiFi mode to set
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetMode(hf_wifi_mode_t mode) override;

  /**
   * @brief Get current WiFi mode.
   * @return Current WiFi mode
   */
  hf_wifi_mode_t GetMode() const noexcept override;

  //==============================================================//
  // STATION MODE OPERATIONS
  //==============================================================//

  /**
   * @brief Configure station mode parameters.
   * @param config Station configuration
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t ConfigureStation(const hf_wifi_station_config_t& config) override;

  /**
   * @brief Connect to WiFi network.
   * @param timeout_ms Connection timeout in milliseconds
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t Connect(hf_timeout_ms_t timeout_ms = 10000) override;

  /**
   * @brief Disconnect from WiFi network.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t Disconnect() override;

  /**
   * @brief Check if connected to WiFi network.
   * @return true if connected, false otherwise
   */
  hf_bool_t IsConnected() const noexcept override;

  /**
   * @brief Get signal strength (RSSI).
   * @return Signal strength in dBm, or INT8_MIN if not connected
   */
  hf_i8_t GetRssi() const override;

  /**
   * @brief Get IP information for station interface.
   * @param ip_info Output structure for IP information
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetIpInfo(hf_wifi_ip_info_t& ip_info) const override;

  //==============================================================//
  // ACCESS POINT MODE OPERATIONS
  //==============================================================//

  /**
   * @brief Configure access point mode parameters.
   * @param config Access point configuration
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t ConfigureAccessPoint(const hf_wifi_ap_config_t& config) override;

  /**
   * @brief Start access point.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StartAccessPoint() override;

  /**
   * @brief Stop access point.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StopAccessPoint() override;

  /**
   * @brief Check if access point is active.
   * @return true if active, false otherwise
   */
  hf_bool_t IsAccessPointActive() const noexcept override;

  /**
   * @brief Get number of connected stations.
   * @return Number of connected stations
   */
  hf_i32_t GetConnectedStationCount() const override;

  //==============================================================//
  // NETWORK SCANNING
  //==============================================================//

  /**
   * @brief Start network scan.
   * @param show_hidden Show hidden networks
   * @param passive Use passive scanning
   * @param max_scan_time_ms Maximum scan time per channel
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StartScan(hf_bool_t show_hidden = false,
                          hf_bool_t passive = false,
                          hf_timeout_ms_t max_scan_time_ms = 120) override;

  /**
   * @brief Get scan results.
   * @param networks Output vector for network information
   * @param max_networks Maximum number of networks to return (0 = all)
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetScanResults(std::vector<hf_wifi_network_info_t>& networks,
                               hf_u16_t max_networks = 0) override;

  /**
   * @brief Check if scan is in progress.
   * @return true if scanning, false otherwise
   */
  hf_bool_t IsScanning() const noexcept override;

  //==============================================================//
  // STATE AND STATUS
  //==============================================================//

  /**
   * @brief Get current WiFi state.
   * @return Current WiFi state
   */
  hf_wifi_state_t GetState() const override;

  /**
   * @brief Get connected network SSID.
   * @return Connected SSID, empty if not connected
   */
  std::string GetConnectedSsid() const override;

  /**
   * @brief Get connected network BSSID.
   * @param bssid Output buffer for BSSID (6 bytes)
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetConnectedBssid(hf_u8_t bssid[6]) const override;

  //==============================================================//
  // POWER MANAGEMENT
  //==============================================================//

  /**
   * @brief Set power save mode.
   * @param mode Power save mode
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetPowerSave(hf_wifi_power_save_t mode) override;

  /**
   * @brief Get current power save mode.
   * @return Current power save mode
   */
  hf_wifi_power_save_t GetPowerSave() const override;

  //==============================================================//
  // EVENT HANDLING
  //==============================================================//

  /**
   * @brief Register WiFi event callback.
   * @param callback Callback function to register
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t RegisterEventCallback(hf_wifi_event_callback_t callback) override;

  /**
   * @brief Unregister WiFi event callback.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t UnregisterEventCallback() override;

  //==============================================================//
  // UTILITY FUNCTIONS
  //==============================================================//

  /**
   * @brief Get MAC address for specified interface.
   * @param mac Output buffer for MAC address (6 bytes)
   * @param interface Interface type (0=STA, 1=AP)
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetMacAddress(hf_u8_t mac[6], hf_u8_t interface = 0) const override;

  /**
   * @brief Set MAC address for specified interface.
   * @param mac MAC address to set (6 bytes)
   * @param interface Interface type (0=STA, 1=AP)
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetMacAddress(const hf_u8_t mac[6], hf_u8_t interface = 0) override;

  /**
   * @brief Get current channel.
   * @return Current channel number, 0 if not available
   */
  hf_u8_t GetChannel() const override;

  /**
   * @brief Set WiFi channel.
   * @param channel Channel number to set
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetChannel(hf_u8_t channel) override;

  //==============================================================//
  // ESP32-C6 SPECIFIC EXTENSIONS
  //==============================================================//

  /**
   * @brief Set advanced ESP32-C6 specific configuration.
   * @param config Advanced configuration structure
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetAdvancedConfig(const hf_esp_wifi_advanced_config_t& config);

  /**
   * @brief Get current advanced configuration.
   * @param config Reference to store current configuration
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetAdvancedConfig(hf_esp_wifi_advanced_config_t& config) const;

  /**
   * @brief Enable WPA3 transition mode (WPA2/WPA3 mixed).
   * @param enable True to enable transition mode
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t EnableWpa3Transition(hf_bool_t enable);

  /**
   * @brief Configure 802.11k/r/v roaming features.
   * @param enable_11k Enable 802.11k Radio Resource Management
   * @param enable_11r Enable 802.11r Fast BSS Transition
   * @param enable_11v Enable 802.11v BSS Transition Management
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t ConfigureRoaming(hf_bool_t enable_11k, hf_bool_t enable_11r, hf_bool_t enable_11v);

  /**
   * @brief Configure WPA2/WPA3 Enterprise authentication.
   * @param config Enterprise configuration
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t ConfigureEnterprise(const hf_esp_wifi_enterprise_config_t& config);

  /**
   * @brief Start SmartConfig provisioning.
   * @param type SmartConfig type
   * @param timeout_ms Provisioning timeout in milliseconds
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StartSmartConfig(smartconfig_type_t type = SC_TYPE_ESPTOUCH,
                                 hf_timeout_ms_t timeout_ms = 60000);

  /**
   * @brief Stop SmartConfig provisioning.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StopSmartConfig();

  /**
   * @brief Start WPS provisioning.
   * @param type WPS type
   * @param timeout_ms Provisioning timeout in milliseconds
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StartWps(wps_type_t type = WPS_TYPE_PBC, hf_timeout_ms_t timeout_ms = 120000);

  /**
   * @brief Stop WPS provisioning.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t StopWps();

  /**
   * @brief Set WiFi TX power.
   * @param power TX power in dBm (2-20 for ESP32-C6)
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetTxPower(hf_i8_t power);

  /**
   * @brief Get WiFi TX power.
   * @return Current TX power in dBm, or -1 on error
   */
  hf_i8_t GetTxPower() const;

  /**
   * @brief Set WiFi channel bandwidth.
   * @param bandwidth Channel bandwidth
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t SetBandwidth(wifi_bandwidth_t bandwidth);

  /**
   * @brief Get WiFi channel bandwidth.
   * @return Current channel bandwidth
   */
  wifi_bandwidth_t GetBandwidth() const;

  /**
   * @brief Get WiFi statistics.
   * @param stats Reference to store WiFi statistics
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetStatistics(hf_esp_wifi_stats_t& stats) const;

  /**
   * @brief Get detailed connection information.
   * @param info Reference to store connection information
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t GetConnectionInfo(wifi_ap_record_t& info) const;

private:
  //==============================================================//
  // INTERNAL STATE MANAGEMENT
  //==============================================================//

  mutable std::mutex m_mutex_;                          ///< Thread synchronization mutex
  std::atomic<hf_bool_t> m_initialized_;                ///< Initialization state
  std::atomic<hf_bool_t> m_enabled_;                    ///< WiFi enabled state
  std::atomic<hf_wifi_mode_t> m_mode_;                  ///< Current WiFi mode
  std::atomic<hf_wifi_state_t> m_state_;                ///< Current WiFi state

  // Configuration storage
  hf_wifi_station_config_t m_sta_config_;               ///< Station configuration
  hf_wifi_ap_config_t m_ap_config_;                     ///< AP configuration
  hf_esp_wifi_advanced_config_t m_advanced_config_;     ///< Advanced configuration

  // ESP-IDF handles
  esp_netif_t* m_sta_netif_;                           ///< Station network interface
  esp_netif_t* m_ap_netif_;                            ///< AP network interface
  EventGroupHandle_t m_event_group_;                   ///< FreeRTOS event group
  esp_event_handler_instance_t m_wifi_event_handler_;  ///< WiFi event handler
  esp_event_handler_instance_t m_ip_event_handler_;    ///< IP event handler

  // Event handling
  hf_wifi_event_callback_t m_event_callback_;          ///< User event callback
  void* m_event_user_data_;                            ///< User data for callbacks

  // Connection state
  std::atomic<hf_bool_t> m_connected_;                  ///< Connection state
  std::atomic<hf_bool_t> m_ap_active_;                  ///< AP active state
  std::atomic<hf_i8_t> m_rssi_;                        ///< Current RSSI
  std::atomic<hf_u8_t> m_channel_;                     ///< Current channel

  // Scan results
  std::vector<hf_wifi_network_info_t> m_scan_results_; ///< Last scan results
  std::atomic<hf_bool_t> m_scanning_;                   ///< Scanning state
  mutable std::mutex m_scan_mutex_;                     ///< Scan results mutex

  // Advanced features state
  std::atomic<hf_bool_t> m_smartconfig_active_;        ///< SmartConfig active
  std::atomic<hf_bool_t> m_wps_active_;                ///< WPS active

  //==============================================================//
  // INTERNAL HELPER METHODS
  //==============================================================//

  /**
   * @brief Initialize ESP-IDF network interface.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t InitNetif();

  /**
   * @brief Deinitialize ESP-IDF network interface.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t DeinitNetif();

  /**
   * @brief Register ESP-IDF event handlers.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t RegisterEventHandlers();

  /**
   * @brief Unregister ESP-IDF event handlers.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t UnregisterEventHandlers();

  /**
   * @brief Static WiFi event handler for ESP-IDF.
   * @param arg User argument (EspWifi instance)
   * @param event_base Event base
   * @param event_id Event ID
   * @param event_data Event data
   */
  static void WifiEventHandler(void* arg, esp_event_base_t event_base, 
                              hf_i32_t event_id, void* event_data);

  /**
   * @brief Static IP event handler for ESP-IDF.
   * @param arg User argument (EspWifi instance)
   * @param event_base Event base
   * @param event_id Event ID
   * @param event_data Event data
   */
  static void IpEventHandler(void* arg, esp_event_base_t event_base, 
                            hf_i32_t event_id, void* event_data);

  /**
   * @brief Handle WiFi events internally.
   * @param event_id ESP-IDF event ID
   * @param event_data Event data
   */
  void HandleWifiEvent(hf_i32_t event_id, void* event_data);

  /**
   * @brief Handle IP events internally.
   * @param event_id ESP-IDF event ID
   * @param event_data Event data
   */
  void HandleIpEvent(hf_i32_t event_id, void* event_data);

  /**
   * @brief Update internal state.
   * @param new_state New WiFi state
   */
  void UpdateState(hf_wifi_state_t new_state);

  /**
   * @brief Apply advanced configuration settings.
   * @return HF_WIFI_SUCCESS on success, error code otherwise
   */
  hf_wifi_err_t ApplyAdvancedConfig();

  /**
   * @brief Convert ESP-IDF error to HardFOC error.
   * @param esp_err ESP-IDF error code
   * @return HardFOC WiFi error code
   */
  static hf_wifi_err_t ConvertEspError(esp_err_t esp_err);

  /**
   * @brief Validate configuration parameters.
   * @param config Configuration to validate
   * @return true if valid, false otherwise
   */
  static hf_bool_t ValidateConfig(const hf_wifi_station_config_t& config);

  /**
   * @brief Validate AP configuration parameters.
   * @param config AP configuration to validate
   * @return true if valid, false otherwise
   */
  static hf_bool_t ValidateApConfig(const hf_wifi_ap_config_t& config);

  // Prevent copying
  EspWifi(const EspWifi&) = delete;
  EspWifi& operator=(const EspWifi&) = delete;
};