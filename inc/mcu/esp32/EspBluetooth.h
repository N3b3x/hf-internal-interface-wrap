/**
 * @file EspBluetooth.h
 * @brief Advanced ESP32 implementation of the unified BaseBluetooth class with ESP-IDF v5.5+
 * features.
 *
 * This file provides concrete implementations of the unified BaseBluetooth class
 * for ESP32 microcontrollers with support for both Bluetooth Classic and BLE features.
 * It supports ESP-IDF v5.5+ APIs, advanced security features, mesh networking,
 * multiple simultaneous connections, and enterprise-grade functionality.
 * The implementation includes comprehensive event handling, connection management,
 * GATT operations, and performance optimizations specific to ESP32 hardware.
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

// Bluetooth headers - conditionally include based on target support
// Basic BT headers not available on ESP32C6/C3/H2 (BLE-only variants)
#if HAS_BLUETOOTH_SUPPORT && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32H2)
#include "esp_bt.h"
#include "esp_bt_main.h"
#endif

// Classic Bluetooth headers (only for ESP32 and ESP32S3)
#if HAS_CLASSIC_BLUETOOTH
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "esp_spp_api.h"
#endif

// Classic Bluetooth and A2DP support matrix based on ESP-IDF documentation:
// - ESP32: Full Classic BT + BLE support (A2DP supported)
// - ESP32S3: Full Classic BT + BLE support (A2DP supported) 
// - ESP32C6: BLE only (no Classic BT, no A2DP)
// - ESP32C3: BLE only (no Classic BT, no A2DP)
// - ESP32H2: BLE only (no Classic BT, no A2DP)
// - ESP32S2: No Bluetooth support
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#define HAS_CLASSIC_BLUETOOTH 1
#define HAS_A2DP_SUPPORT 1
#define HAS_AVRCP_SUPPORT 1
#elif defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32H2)
// BLE only variants - support BLE but not basic BT headers
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_BLUETOOTH_SUPPORT 1
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
// No Bluetooth support
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_BLUETOOTH_SUPPORT 0
#else
// Default to no Classic Bluetooth for unknown targets
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#endif

// Define Bluetooth support for all variants except ESP32S2
#ifndef HAS_BLUETOOTH_SUPPORT
#define HAS_BLUETOOTH_SUPPORT 1
#endif

// Bluetooth Low Energy headers - conditional compilation for different ESP32 variants
#if HAS_BLUETOOTH_SUPPORT
// esp_bt_defs.h not available on ESP32C6/C3/H2 (BLE-only variants)
#if !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32H2)
#include "esp_bt_defs.h"
#endif
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#endif

// ESP-MESH Bluetooth headers - conditional compilation
#if HAS_BLUETOOTH_SUPPORT
#include "esp_mesh.h"
#include "esp_mesh_internal.h"
#endif

// System headers
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#ifdef __cplusplus
}
#endif

// C++ headers
#include "BaseBluetooth.h"
#include "mcu/esp32/utils/EspTypes_Base.h"
#include "mcu/esp32/utils/EspTypes_Bluetooth.h"
#include "utils/RtosMutex.h"
#include <atomic>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>

/**
 * @brief ESP32-specific Bluetooth configuration extensions
 */
struct EspBluetoothAdvancedConfig {
  // Power management
  bool enable_power_save;           /**< Enable Bluetooth power save mode */
  esp_power_level_t tx_power_level; /**< TX power level */
  bool enable_modem_sleep;          /**< Enable modem sleep */

  // Performance tuning
  uint16_t max_connections;        /**< Maximum simultaneous connections */
  uint16_t connection_timeout_ms;  /**< Connection timeout */
  uint16_t supervision_timeout_ms; /**< Link supervision timeout */
  uint8_t min_connection_interval; /**< Minimum connection interval */
  uint8_t max_connection_interval; /**< Maximum connection interval */

  // Classic Bluetooth features (only available on ESP32 and ESP32S3)
#if HAS_CLASSIC_BLUETOOTH
  bool enable_spp;   /**< Enable Serial Port Profile */
  bool enable_a2dp;  /**< Enable Advanced Audio Distribution Profile */
  bool enable_avrcp; /**< Enable Audio/Video Remote Control Profile */
  bool enable_hfp;   /**< Enable Hands-Free Profile */
  bool enable_hid;   /**< Enable Human Interface Device Profile */
#endif

  // BLE features
  bool enable_gatt_server;           /**< Enable GATT server */
  bool enable_gatt_client;           /**< Enable GATT client */
  uint16_t max_gatt_services;        /**< Maximum GATT services */
  uint16_t max_gatt_characteristics; /**< Maximum GATT characteristics */
  uint16_t mtu_size;                 /**< Maximum Transmission Unit size */

  // Security features
  bool enable_secure_connections;    /**< Enable Bluetooth 4.2+ Secure Connections */
  bool enable_privacy;               /**< Enable BLE Privacy */
  bool require_mitm_protection;      /**< Require Man-in-the-Middle protection */
  bool enable_bonding;               /**< Enable bonding */
  esp_ble_sm_io_cap_t io_capability; /**< I/O capability for pairing */

  // Advanced features
  bool enable_extended_advertising; /**< Enable BLE Extended Advertising */
  bool enable_periodic_advertising; /**< Enable BLE Periodic Advertising */
  bool enable_coded_phy;            /**< Enable BLE Coded PHY */
  bool enable_2m_phy;               /**< Enable BLE 2M PHY */
  bool enable_mesh_proxy;           /**< Enable Mesh Proxy feature */
  bool enable_mesh_relay;           /**< Enable Mesh Relay feature */
  bool enable_mesh_friend;          /**< Enable Mesh Friend feature */
  bool enable_mesh_low_power;       /**< Enable Mesh Low Power Node feature */
};

/**
 * @brief ESP32-specific connection information
 */
struct EspBluetoothConnectionInfo {
  hf_bluetooth_address_t address; /**< Device address */
  esp_bd_addr_t esp_address;      /**< ESP-IDF format address */
  uint16_t connection_handle;     /**< Connection handle */
  bool is_classic;                /**< True if Classic, false if BLE */
  uint16_t mtu;                   /**< Current MTU size */
  uint8_t connection_interval;    /**< Connection interval (BLE) */
  uint8_t slave_latency;          /**< Slave latency (BLE) */
  uint16_t supervision_timeout;   /**< Supervision timeout */
  int8_t tx_power;                /**< Current TX power */
  esp_ble_sm_key_mask_t key_mask; /**< Security key mask (BLE) */
};

/**
 * @brief ESP32-specific GATT service implementation
 */
struct EspGattServiceInfo {
  hf_bluetooth_gatt_service_t base_info; /**< Base service information */
  esp_gatt_srvc_id_t service_id;         /**< ESP-IDF service ID */
  uint16_t service_handle;               /**< Service handle */
  std::vector<uint16_t> char_handles;    /**< Characteristic handles */
  bool is_started;                       /**< Service started state */
};

/**
 * @class EspBluetooth
 * @brief Advanced ESP32 implementation of unified BaseBluetooth with ESP-IDF v5.5+ features.
 * @details This class provides a comprehensive implementation of BaseBluetooth for ESP32
 *          microcontrollers with support for both basic and advanced features including:
 *
 *          **Basic Features:**
 *          - Bluetooth Classic and BLE support
 *          - Device discovery and connection management
 *          - Pairing and bonding with security
 *          - Data transmission and reception
 *          - Thread-safe state management
 *
 *          **Advanced Features (ESP-IDF v5.5+):**
 *          - Multiple simultaneous connections
 *          - Bluetooth 5.0+ features (Extended Advertising, 2M PHY, Coded PHY)
 *          - Advanced security (Secure Connections, Privacy)
 *          - Multiple Bluetooth profiles (SPP, A2DP, AVRCP, HFP, HID)
 *          - Complete GATT server and client implementation
 *          - Mesh networking capabilities
 *          - Enterprise-grade security features
 *          - Advanced power management
 *
 *          **Performance Optimizations:**
 *          - Hardware-accelerated cryptography
 *          - Optimized buffer management
 *          - Low-latency event handling
 *          - Memory pool management
 *          - Connection parameter optimization
 *
 *          **Thread Safety:**
 *          - All public methods are thread-safe
 *          - Internal state protection with mutexes
 *          - Atomic operations for status flags
 *          - Event queue synchronization
 */
class EspBluetooth : public BaseBluetooth {
public:
  /**
   * @brief Constructor with optional advanced configuration
   * @param advanced_config Advanced ESP32-specific configuration (optional)
   */
  explicit EspBluetooth(const EspBluetoothAdvancedConfig* advanced_config = nullptr);

  /**
   * @brief Destructor - ensures proper cleanup
   */
  virtual ~EspBluetooth();

  // ========== BaseBluetooth Interface Implementation ==========

  // Initialization and Configuration
  hf_bluetooth_err_t Initialize(hf_bluetooth_mode_t mode) override;
  hf_bluetooth_err_t Deinitialize() override;
  bool IsInitialized() const override;
  hf_bluetooth_err_t Enable() override;
  hf_bluetooth_err_t Disable() override;
  bool IsEnabled() const override;
  hf_bluetooth_err_t SetMode(hf_bluetooth_mode_t mode) override;
  hf_bluetooth_mode_t GetMode() const override;

  // Device Management
  hf_bluetooth_err_t GetLocalAddress(hf_bluetooth_address_t& address) const override;
  hf_bluetooth_err_t SetDeviceName(const std::string& name) override;
  std::string GetDeviceName() const override;

  // Classic Bluetooth Operations
  hf_bluetooth_err_t ConfigureClassic(const hf_bluetooth_classic_config_t& config) override;
  hf_bluetooth_err_t SetDiscoverable(bool discoverable, uint32_t timeout_ms = 0) override;
  bool IsDiscoverable() const override;

  // BLE Operations
  hf_bluetooth_err_t ConfigureBle(const hf_bluetooth_ble_config_t& config) override;
  hf_bluetooth_err_t StartAdvertising() override;
  hf_bluetooth_err_t StopAdvertising() override;
  bool IsAdvertising() const override;

  // Device Discovery
  hf_bluetooth_err_t StartScan(
      uint32_t duration_ms = 0,
      hf_bluetooth_scan_type_t type =
          hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE) override;
  hf_bluetooth_err_t StopScan() override;
  bool IsScanning() const override;
  hf_bluetooth_err_t GetDiscoveredDevices(
      std::vector<hf_bluetooth_device_info_t>& devices) override;
  hf_bluetooth_err_t ClearDiscoveredDevices() override;

  // Connection Management
  hf_bluetooth_err_t Connect(const hf_bluetooth_address_t& address,
                             uint32_t timeout_ms = 0) override;
  hf_bluetooth_err_t Disconnect(const hf_bluetooth_address_t& address) override;
  bool IsConnected(const hf_bluetooth_address_t& address) const override;
  hf_bluetooth_err_t GetConnectedDevices(std::vector<hf_bluetooth_device_info_t>& devices) override;

  // Pairing and Bonding
  hf_bluetooth_err_t Pair(const hf_bluetooth_address_t& address,
                          const std::string& pin = "") override;
  hf_bluetooth_err_t Unpair(const hf_bluetooth_address_t& address) override;
  bool IsPaired(const hf_bluetooth_address_t& address) const override;
  hf_bluetooth_err_t GetPairedDevices(std::vector<hf_bluetooth_device_info_t>& devices) override;

  // Data Transmission
  hf_bluetooth_err_t SendData(const hf_bluetooth_address_t& address,
                              const std::vector<uint8_t>& data) override;
  int GetAvailableData(const hf_bluetooth_address_t& address) const override;
  hf_bluetooth_err_t ReadData(const hf_bluetooth_address_t& address, std::vector<uint8_t>& data,
                              size_t max_bytes = 0) override;

  // GATT Operations (BLE)
  hf_bluetooth_err_t DiscoverServices(const hf_bluetooth_address_t& address,
                                      std::vector<hf_bluetooth_gatt_service_t>& services) override;
  hf_bluetooth_err_t DiscoverCharacteristics(
      const hf_bluetooth_address_t& address, const std::string& service_uuid,
      std::vector<hf_bluetooth_gatt_characteristic_t>& characteristics) override;
  hf_bluetooth_err_t ReadCharacteristic(const hf_bluetooth_address_t& address,
                                        const std::string& service_uuid,
                                        const std::string& characteristic_uuid,
                                        std::vector<uint8_t>& value) override;
  hf_bluetooth_err_t WriteCharacteristic(const hf_bluetooth_address_t& address,
                                         const std::string& service_uuid,
                                         const std::string& characteristic_uuid,
                                         const std::vector<uint8_t>& value,
                                         bool with_response = true) override;
  hf_bluetooth_err_t SubscribeCharacteristic(const hf_bluetooth_address_t& address,
                                             const std::string& service_uuid,
                                             const std::string& characteristic_uuid,
                                             bool enable) override;

  // State and Status
  hf_bluetooth_state_t GetState() const override;
  int8_t GetRssi(const hf_bluetooth_address_t& address) const override;

  // Event Handling
  hf_bluetooth_err_t RegisterEventCallback(hf_bluetooth_event_callback_t callback) override;
  hf_bluetooth_err_t unRegisterEventCallback() override;
  hf_bluetooth_err_t RegisterDataCallback(hf_bluetooth_data_callback_t callback) override;
  hf_bluetooth_err_t unRegisterDataCallback() override;

  // ========== ESP32-Specific Extensions ==========

  /**
   * @brief Set advanced ESP32-specific configuration
   * @param config Advanced configuration structure
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetAdvancedConfig(const EspBluetoothAdvancedConfig& config);

  /**
   * @brief Get current advanced configuration
   * @param config Reference to store current configuration
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetAdvancedConfig(EspBluetoothAdvancedConfig& config) const;

  /**
   * @brief Get detailed connection information
   * @param address Device address
   * @param info Reference to store connection information
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetConnectionInfo(const hf_bluetooth_address_t& address,
                                       EspBluetoothConnectionInfo& info) const;

  /**
   * @brief Set connection parameters for BLE connection
   * @param address Device address
   * @param min_interval Minimum connection interval
   * @param max_interval Maximum connection interval
   * @param slave_latency Slave latency
   * @param supervision_timeout Supervision timeout
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetConnectionParams(const hf_bluetooth_address_t& address,
                                         uint16_t min_interval, uint16_t max_interval,
                                         uint16_t slave_latency, uint16_t supervision_timeout);

  /**
   * @brief Set PHY preference for BLE connection (2M, Coded, 1M)
   * @param address Device address
   * @param tx_phy_mask TX PHY mask
   * @param rx_phy_mask RX PHY mask
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetPhyPreference(const hf_bluetooth_address_t& address, uint8_t tx_phy_mask,
                                      uint8_t rx_phy_mask);

  /**
   * @brief Configure Extended Advertising (BLE 5.0+)
   * @param enable True to enable extended advertising
   * @param max_events Maximum advertising events (0 for continuous)
   * @param duration Duration in 10ms units (0 for continuous)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ConfigureExtendedAdvertising(bool enable, uint8_t max_events = 0,
                                                  uint16_t duration = 0);

  /**
   * @brief Configure Periodic Advertising (BLE 5.0+)
   * @param enable True to enable periodic advertising
   * @param interval_min Minimum advertising interval
   * @param interval_max Maximum advertising interval
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ConfigurePeriodicAdvertising(bool enable, uint16_t interval_min = 0x20,
                                                  uint16_t interval_max = 0x40);

  /**
   * @brief Create and start GATT service
   * @param service_uuid Service UUID
   * @param is_primary True if primary service
   * @param num_handles Number of handles to reserve
   * @return Service handle on success, 0 on error
   */
  uint16_t CreateGattService(const std::string& service_uuid, bool is_primary = true,
                             uint16_t num_handles = 10);

  /**
   * @brief Add characteristic to GATT service
   * @param service_handle Service handle
   * @param char_uuid Characteristic UUID
   * @param properties Characteristic properties
   * @param permissions Characteristic permissions
   * @return Characteristic handle on success, 0 on error
   */
  uint16_t AddGattCharacteristic(uint16_t service_handle, const std::string& char_uuid,
                                 esp_gatt_char_prop_t properties, esp_gatt_perm_t permissions);

  /**
   * @brief Start GATT service
   * @param service_handle Service handle
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StartGattService(uint16_t service_handle);

  /**
   * @brief Stop GATT service
   * @param service_handle Service handle
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StopGattService(uint16_t service_handle);

  /**
   * @brief Send GATT notification
   * @param address Device address
   * @param char_handle Characteristic handle
   * @param data Data to send
   * @param need_confirm True if confirmation needed
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SendGattNotification(const hf_bluetooth_address_t& address,
                                          uint16_t char_handle, const std::vector<uint8_t>& data,
                                          bool need_confirm = false);

#if HAS_CLASSIC_BLUETOOTH
  /**
   * @brief Enable/disable Serial Port Profile (SPP)
   * @param enable True to enable SPP
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   * @note Only available on ESP32 and ESP32S3
   */
  hf_bluetooth_err_t EnableSpp(bool enable);

  /**
   * @brief Enable/disable A2DP audio profile
   * @param enable True to enable A2DP
   * @param sink True for sink role, false for source
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   * @note Only available on ESP32 and ESP32S3
   */
  hf_bluetooth_err_t EnableA2dp(bool enable, bool sink = true);

  /**
   * @brief Enable/disable AVRCP profile
   * @param enable True to enable AVRCP
   * @param controller True for controller role, false for target
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   * @note Only available on ESP32 and ESP32S3
   */
  hf_bluetooth_err_t EnableAvrcp(bool enable, bool controller = true);
#endif // HAS_CLASSIC_BLUETOOTH

  /**
   * @brief Set Bluetooth TX power
   * @param power_level Power level
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetTxPower(esp_power_level_t power_level);

  /**
   * @brief Get Bluetooth TX power
   * @return Current power level
   */
  esp_power_level_t GetTxPower() const;

  /**
   * @brief Perform Bluetooth coexistence configuration
   * @param wifi_priority WiFi priority level (0-100)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ConfigureCoexistence(uint8_t wifi_priority);

  /**
   * @brief Get Bluetooth controller memory usage
   * @param free_mem Reference to store free memory size
   * @param total_mem Reference to store total memory size
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetMemoryUsage(size_t& free_mem, size_t& total_mem) const;

private:
  // ========== Internal State Management ==========

  mutable RtosMutex m_mutex;                 /**< Main synchronization mutex */
  std::atomic<bool> m_initialized;           /**< Initialization state */
  std::atomic<bool> m_enabled;               /**< Bluetooth enabled state */
  std::atomic<hf_bluetooth_mode_t> m_mode;   /**< Current Bluetooth mode */
  std::atomic<hf_bluetooth_state_t> m_state; /**< Current Bluetooth state */

  // Configuration storage
  hf_bluetooth_classic_config_t m_classic_config; /**< Classic configuration */
  hf_bluetooth_ble_config_t m_ble_config;         /**< BLE configuration */
  EspBluetoothAdvancedConfig m_advanced_config;   /**< Advanced configuration */

  // Device management
  std::string m_device_name;              /**< Local device name */
  hf_bluetooth_address_t m_local_address; /**< Local Bluetooth address */

  // Connection management
  std::unordered_map<std::string, EspBluetoothConnectionInfo>
      m_connections;                     /**< Active connections */
  mutable RtosMutex m_connections_mutex; /**< Connections mutex */

  // Discovery and pairing
  std::vector<hf_bluetooth_device_info_t> m_discovered_devices; /**< Discovered devices */
  std::vector<hf_bluetooth_device_info_t> m_paired_devices;     /**< Paired devices */
  std::atomic<bool> m_scanning;                                 /**< Scanning state */
  std::atomic<bool> m_discoverable;                             /**< Discoverable state */
  std::atomic<bool> m_advertising;                              /**< Advertising state */
  hf_bluetooth_scan_type_t m_current_scan_type;                 /**< Current scan type */
  mutable RtosMutex m_discovery_mutex;                          /**< Discovery mutex */

  // GATT services
  std::map<uint16_t, EspGattServiceInfo> m_gatt_services; /**< GATT services */
  mutable RtosMutex m_gatt_mutex;                         /**< GATT operations mutex */

  // Event handling
  hf_bluetooth_event_callback_t m_event_callback;                   /**< User event callback */
  hf_bluetooth_data_callback_t m_data_callback;                     /**< User data callback */
  hf_bluetooth_scan_callback_t m_scan_callback;                     /**< User scan callback */
  hf_bluetooth_gatt_event_callback_t m_gatt_event_callback;         /**< User GATT event callback */
  std::queue<std::pair<hf_bluetooth_event_t, void*>> m_event_queue; /**< Event queue */
  mutable RtosMutex m_event_mutex;                                  /**< Event queue mutex */

  // Data buffers
  std::unordered_map<std::string, std::queue<std::vector<uint8_t>>>
      m_data_buffers;             /**< Data buffers per device */
  mutable RtosMutex m_data_mutex; /**< Data buffer mutex */

  // ========== Internal Helper Methods ==========

  /**
   * @brief Initialize Bluetooth controller
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t InitController();

  /**
   * @brief Deinitialize Bluetooth controller
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t deInitController();

  /**
   * @brief Register all event handlers
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t RegisterEventHandlers();

  /**
   * @brief Unregister all event handlers
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t unRegisterEventHandlers();

  /**
   * @brief Convert HardFOC address to ESP-IDF address
   * @param hf_addr HardFOC address
   * @param esp_addr ESP-IDF address output
   */
  void ConvertToEspAddress(const hf_bluetooth_address_t& hf_addr, esp_bd_addr_t esp_addr) const;

  /**
   * @brief Convert ESP-IDF address to HardFOC address
   * @param esp_addr ESP-IDF address
   * @param hf_addr HardFOC address output
   */
  void ConvertFromEspAddress(const esp_bd_addr_t esp_addr, hf_bluetooth_address_t& hf_addr) const;

  /**
   * @brief Convert ESP-IDF error to HardFOC error
   * @param esp_err ESP-IDF error code
   * @return HardFOC Bluetooth error code
   */
  hf_bluetooth_err_t ConvertEspError(esp_err_t esp_err) const;

  /**
   * @brief Apply advanced configuration settings
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ApplyAdvancedConfig();

  /**
   * @brief Static GAP event handler for ESP-IDF
   * @param event GAP event type
   * @param param Event parameters
   */
  static void GapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

  /**
   * @brief Static GATT server event handler for ESP-IDF
   * @param event GATT server event type
   * @param gatts_if GATT server interface
   * @param param Event parameters
   */
  static void GattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t* param);

  /**
   * @brief Static GATT client event handler for ESP-IDF
   * @param event GATT client event type
   * @param gattc_if GATT client interface
   * @param param Event parameters
   */
  static void GattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                esp_ble_gattc_cb_param_t* param);

#if HAS_CLASSIC_BLUETOOTH
  /**
   * @brief Static Classic Bluetooth GAP event handler
   * @param event GAP event type
   * @param param Event parameters
   */
  static void ClassicGapEventHandler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param);

  /**
   * @brief Static SPP event handler
   * @param event SPP event type
   * @param param Event parameters
   */
  static void SppEventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t* param);
#endif // HAS_CLASSIC_BLUETOOTH

  /**
   * @brief Handle BLE GAP events internally
   * @param event GAP event type
   * @param param Event parameters
   */
  void HandleGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

  /**
   * @brief Handle GATT server events internally
   * @param event GATT server event type
   * @param gatts_if GATT server interface
   * @param param Event parameters
   */
  void HandleGattsEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                        esp_ble_gatts_cb_param_t* param);

  /**
   * @brief Handle GATT client events internally
   * @param event GATT client event type
   * @param gattc_if GATT client interface
   * @param param Event parameters
   */
  void HandleGattcEvent(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                        esp_ble_gattc_cb_param_t* param);

#if HAS_CLASSIC_BLUETOOTH
  /**
   * @brief Handle Classic Bluetooth events internally
   * @param event GAP event type
   * @param param Event parameters
   */
  void HandleClassicGapEvent(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param);

  /**
   * @brief Handle SPP events internally
   * @param event SPP event type
   * @param param Event parameters
   */
  void HandleSppEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param);
#endif // HAS_CLASSIC_BLUETOOTH

  /**
   * @brief Notify user event callback
   * @param event HardFOC Bluetooth event
   * @param event_data Event data
   */
  void NotifyEventCallback(hf_bluetooth_event_t event, void* event_data);

  /**
   * @brief Update internal state
   * @param new_state New Bluetooth state
   */
  void UpdateState(hf_bluetooth_state_t new_state);

  /**
   * @brief Add discovered device to list
   * @param device_info Device information
   */
  void AddDiscoveredDevice(const hf_bluetooth_device_info_t& device_info);

  /**
   * @brief Get connection by address
   * @param address Device address
   * @return Pointer to connection info, nullptr if not found
   */
  EspBluetoothConnectionInfo* GetConnection(const hf_bluetooth_address_t& address);

  /**
   * @brief Add new connection
   * @param address Device address
   * @param info Connection information
   */
  void AddConnection(const hf_bluetooth_address_t& address, const EspBluetoothConnectionInfo& info);

  /**
   * @brief Remove connection
   * @param address Device address
   */
  void RemoveConnection(const hf_bluetooth_address_t& address);

  /**
   * @brief Store received data for device
   * @param address Device address
   * @param data Received data
   */
  void StoreReceivedData(const hf_bluetooth_address_t& address, const std::vector<uint8_t>& data);

  // Static instance pointer for ESP-IDF callbacks
  static EspBluetooth* s_instance;

  // Prevent copying
  EspBluetooth(const EspBluetooth&) = delete;
  EspBluetooth& operator=(const EspBluetooth&) = delete;
};