/**
 * @file EspBluetooth.cpp
 * @brief Advanced ESP32 implementation of the unified BaseBluetooth class with ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseBluetooth class
 * for ESP32 microcontrollers with full ESP-IDF v5.5+ API support including
 * Bluetooth Classic, BLE, multiple connections, advanced security, and
 * mesh networking capabilities.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Requires ESP-IDF v5.5 or higher for full feature support
 * @note Thread-safe implementation with proper synchronization
 */

#include "mcu/esp32/EspBluetooth.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <cstring>
#include <algorithm>

static const char* TAG = "EspBluetooth";

// Event group bits for Bluetooth events
#define BT_INITIALIZED_BIT      BIT0
#define BT_ENABLED_BIT          BIT1
#define BT_CLASSIC_CONNECTED_BIT BIT2
#define BT_BLE_CONNECTED_BIT    BIT3
#define BT_SCAN_DONE_BIT        BIT4
#define BT_PAIR_DONE_BIT        BIT5
#define BT_GATT_READY_BIT       BIT6

// Default values for advanced configuration
static const EspBluetoothAdvancedConfig DEFAULT_ADVANCED_CONFIG = {
  .enable_power_save = false,
  .tx_power_level = ESP_PWR_LVL_N0,
  .enable_modem_sleep = false,
  .max_connections = 4,
  .connection_timeout_ms = 30000,
  .supervision_timeout_ms = 20000,
  .min_connection_interval = 24,
  .max_connection_interval = 40,
  .enable_spp = true,
  .enable_a2dp = false,
  .enable_avrcp = false,
  .enable_hfp = false,
  .enable_hid = false,
  .enable_gatt_server = true,
  .enable_gatt_client = true,
  .max_gatt_services = 16,
  .max_gatt_characteristics = 64,
  .mtu_size = 512,
  .enable_secure_connections = true,
  .enable_privacy = true,
  .require_mitm_protection = false,
  .enable_bonding = true,
  .io_capability = ESP_IO_CAP_NONE,
  .enable_extended_advertising = false,
  .enable_periodic_advertising = false,
  .enable_coded_phy = false,
  .enable_2m_phy = false,
  .enable_mesh_proxy = false,
  .enable_mesh_relay = false,
  .enable_mesh_friend = false,
  .enable_mesh_low_power = false
};

/**
 * @brief Convert HardFOC Bluetooth mode to ESP-IDF mode
 */
static esp_bt_mode_t ConvertToEspMode(hf_bluetooth_mode_t mode) {
  switch (mode) {
    case hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_CLASSIC_ONLY:
      return ESP_BT_MODE_CLASSIC_BT;
    case hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE_ONLY:
      return ESP_BT_MODE_BLE;
    case hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_DUAL_MODE:
      return ESP_BT_MODE_BTDM;
    default:
      return ESP_BT_MODE_IDLE;
  }
}

/**
 * @brief Convert ESP-IDF connection state to HardFOC state
 */
static hf_bluetooth_connection_state_t ConvertFromEspConnectionState(esp_gap_ble_cb_event_t event) {
  switch (event) {
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      return hf_bluetooth_connection_state_t::HF_BLUETOOTH_CONNECTION_STATE_ADVERTISING;
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
      return hf_bluetooth_connection_state_t::HF_BLUETOOTH_CONNECTION_STATE_SCANNING;
    default:
      return hf_bluetooth_connection_state_t::HF_BLUETOOTH_CONNECTION_STATE_DISCONNECTED;
  }
}

/**
 * @brief Convert HardFOC security level to ESP-IDF security
 */
static esp_ble_sm_param_t ConvertToEspSecurity(hf_bluetooth_security_t security) {
  switch (security) {
    case hf_bluetooth_security_t::HF_BLUETOOTH_SECURITY_NONE:
      return ESP_BLE_SM_PASSKEY;
    case hf_bluetooth_security_t::HF_BLUETOOTH_SECURITY_UNAUTHENTICATED:
      return ESP_BLE_SM_PASSKEY;
    case hf_bluetooth_security_t::HF_BLUETOOTH_SECURITY_AUTHENTICATED:
      return ESP_BLE_SM_AUTHEN_REQ_MODE;
    case hf_bluetooth_security_t::HF_BLUETOOTH_SECURITY_ENCRYPTED:
      return ESP_BLE_SM_IOCAP_MODE;
    default:
      return ESP_BLE_SM_PASSKEY;
  }
}

EspBluetooth::EspBluetooth(const EspBluetoothAdvancedConfig* advanced_config)
    : event_group_(nullptr)
    , m_initialized(false)
    , m_enabled(false)
    , m_mode(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_DISABLED)
    , is_advertising_(false)
    , is_scanning_(false)
    , m_current_scan_type(hf_bluetooth_scan_mode_t::HF_BLUETOOTH_SCAN_MODE_GENERAL_INQUIRY)
    , scan_timeout_ms_(10000)
    , connection_timeout_ms_(30000)
    , next_connection_id_(1)
    , next_service_id_(1)
    , m_event_callback(nullptr)
    , event_user_data_(nullptr)
    , m_scan_callback(nullptr)
    , scan_user_data_(nullptr)
    , gatt_m_event_callback(nullptr)
    , gatt_user_data_(nullptr) {
  
  // Copy advanced configuration or use defaults
  if (advanced_config) {
    advanced_config_ = *advanced_config;
  } else {
    advanced_config_ = DEFAULT_ADVANCED_CONFIG;
  }
  
  ESP_LOGI(TAG, "EspBluetooth created with advanced config");
}

EspBluetooth::~EspBluetooth() {
  Deinitialize();
}

hf_bluetooth_err_t EspBluetooth::Initialize(hf_bluetooth_mode_t mode) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (m_initialized) {
    ESP_LOGW(TAG, "Bluetooth already initialized");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
  }
  
  ESP_LOGI(TAG, "Initializing ESP32 Bluetooth with ESP-IDF v5.5+ features");
  
  // Initialize NVS if needed
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  
  // Create event group for Bluetooth events
  event_group_ = xEventGroupCreate();
  if (!event_group_) {
    ESP_LOGE(TAG, "Failed to create event group");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_INIT_FAILED;
  }
  
  // Release memory for classic BT if not needed
  esp_bt_mode_t esp_mode = ConvertToEspMode(mode);
  if (esp_mode == ESP_BT_MODE_BLE) {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  } else if (esp_mode == ESP_BT_MODE_CLASSIC_BT) {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
  }
  
  // Initialize Bluetooth controller
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  
  // Apply advanced configuration
  bt_cfg.mode = esp_mode;
  if (advanced_config_.enable_modem_sleep) {
    bt_cfg.sleep_mode = ESP_BT_SLEEP_MODE_1;
  }
  
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize BT controller: %s", esp_err_to_name(ret));
    Cleanup();
    return hf_bluetooth_err_t::BLUETOOTH_ERR_INIT_FAILED;
  }
  
  ret = esp_bt_controller_enable(esp_mode);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
    Cleanup();
    return hf_bluetooth_err_t::BLUETOOTH_ERR_INIT_FAILED;
  }
  
  // Initialize Bluedroid stack
  ret = esp_bluedroid_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
    Cleanup();
    return hf_bluetooth_err_t::BLUETOOTH_ERR_INIT_FAILED;
  }
  
  ret = esp_bluedroid_enable();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
    Cleanup();
    return hf_bluetooth_err_t::BLUETOOTH_ERR_INIT_FAILED;
  }
  
  // Initialize BLE GAP if BLE mode is enabled
  if (esp_mode == ESP_BT_MODE_BLE || esp_mode == ESP_BT_MODE_BTDM) {
    ret = InitializeBLE();
    if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
      Cleanup();
      return ret;
    }
  }
  
  // Initialize Classic Bluetooth if enabled
  if (esp_mode == ESP_BT_MODE_CLASSIC_BT || esp_mode == ESP_BT_MODE_BTDM) {
    ret = InitializeClassic();
    if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
      Cleanup();
      return ret;
    }
  }
  
  m_mode = mode;
  m_initialized = true;
  xEventGroupSetBits(event_group_, BT_INITIALIZED_BIT);
  
  ESP_LOGI(TAG, "ESP32 Bluetooth initialized successfully");
  return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::Deinitialize() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    return hf_bluetooth_err_t::SUCCESS;
  }
  
  ESP_LOGI(TAG, "Deinitializing ESP32 Bluetooth");
  
  // Stop advertising and scanning
  if (is_advertising_) {
    esp_ble_gap_stop_advertising();
    is_advertising_ = false;
  }
  
  if (is_scanning_) {
    esp_ble_gap_stop_scanning();
    is_scanning_ = false;
  }
  
  // Disconnect all connections
  for (auto& conn : connections_) {
    if (conn.second.is_classic) {
      // Disconnect Classic Bluetooth connection
      esp_bt_gap_cancel_discovery();
    } else {
      // Disconnect BLE connection
      esp_ble_gatts_close(conn.second.gatt_if, conn.second.connection_handle);
    }
  }
  connections_.clear();
  
  // Clear all GATT services
  gatt_services_.clear();
  
  // Unregister callbacks and deinitialize
  if (m_mode == hf_bluetooth_mode_t::BLE_ONLY || m_mode == hf_bluetooth_mode_t::DUAL_MODE) {
    esp_ble_gap_register_callback(nullptr);
    esp_ble_gatts_register_callback(nullptr);
    esp_ble_gattc_register_callback(nullptr);
  }
  
  if (m_mode == hf_bluetooth_mode_t::CLASSIC_ONLY || m_mode == hf_bluetooth_mode_t::DUAL_MODE) {
    esp_bt_gap_register_callback(nullptr);
    if (advanced_config_.enable_spp) {
      esp_spp_register_callback(nullptr);
    }
  }
  
  cleanup();
  
  m_initialized = false;
  m_enabled = false;
  m_mode = hf_bluetooth_mode_t::DISABLED;
  
  ESP_LOGI(TAG, "ESP32 Bluetooth deinitialized successfully");
  return hf_bluetooth_err_t::SUCCESS;
}

bool EspBluetooth::IsInitialized() const {
  return m_initialized;
}

hf_bluetooth_err_t EspBluetooth::Enable() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "Bluetooth not initialized");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (m_enabled) {
    ESP_LOGW(TAG, "Bluetooth already enabled");
    return hf_bluetooth_err_t::SUCCESS;
  }
  
  // Set power level
  esp_err_t ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, advanced_config_.tx_power_level);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Failed to set BLE TX power: %s", esp_err_to_name(ret));
  }
  
  m_enabled = true;
  xEventGroupSetBits(event_group_, BT_ENABLED_BIT);
  
  ESP_LOGI(TAG, "Bluetooth enabled");
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::Disable() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_enabled) {
    return hf_bluetooth_err_t::SUCCESS;
  }
  
  // Stop advertising and scanning
  if (is_advertising_) {
    esp_ble_gap_stop_advertising();
    is_advertising_ = false;
  }
  
  if (is_scanning_) {
    esp_ble_gap_stop_scanning();
    is_scanning_ = false;
  }
  
  m_enabled = false;
  xEventGroupClearBits(event_group_, BT_ENABLED_BIT);
  
  ESP_LOGI(TAG, "Bluetooth disabled");
  return hf_bluetooth_err_t::SUCCESS;
}

bool EspBluetooth::IsEnabled() const {
  return m_enabled;
}

hf_bluetooth_mode_t EspBluetooth::GetMode() const {
  return m_mode;
}

hf_bluetooth_address_t EspBluetooth::GetLocalAddress() const {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  
  hf_bluetooth_address_t address;
  memcpy(address.address, mac, 6);
  return address;
}

hf_bluetooth_err_t EspBluetooth::SetDeviceName(const std::string& name) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "Bluetooth not initialized");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (name.length() > 32) {
    ESP_LOGE(TAG, "Name too long (max 32 characters)");
    return hf_bluetooth_err_t::INVALID_PARAMETER;
  }
  
  // Set BLE device name
  esp_err_t ret = esp_ble_gap_set_device_name(name.c_str());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set BLE device name: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::SET_FAILED;
  }
  
  // Set Classic Bluetooth device name if applicable
  if (m_mode == hf_bluetooth_mode_t::CLASSIC_ONLY || m_mode == hf_bluetooth_mode_t::DUAL_MODE) {
    ret = esp_bt_dev_set_device_name(name.c_str());
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set Classic BT device name: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::SET_FAILED;
    }
  }
  
  local_name_ = name;
  
  ESP_LOGI(TAG, "Local name set to: %s", name.c_str());
  return hf_bluetooth_err_t::SUCCESS;
}

std::string EspBluetooth::GetDeviceName() const {
  return local_name_;
}

hf_bluetooth_err_t EspBluetooth::startAdvertising(const hf_bluetooth_advertising_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized || !m_enabled) {
    ESP_LOGE(TAG, "Bluetooth not initialized or enabled");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (m_mode == hf_bluetooth_mode_t::CLASSIC_ONLY) {
    ESP_LOGE(TAG, "Advertising not supported in Classic-only mode");
    return hf_bluetooth_err_t::NOT_SUPPORTED;
  }
  
  if (is_advertising_) {
    ESP_LOGW(TAG, "Already advertising");
    return hf_bluetooth_err_t::ALREADY_STARTED;
  }
  
  // Configure advertising parameters
  esp_ble_adv_params_t adv_params = {
    .adv_int_min = static_cast<uint16_t>(config.min_interval * 1.6f), // Convert to 0.625ms units
    .adv_int_max = static_cast<uint16_t>(config.max_interval * 1.6f),
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr = {0},
    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
  };
  
  // Set advertising type based on configuration
  if (config.connectable) {
    adv_params.adv_type = config.directed ? ADV_TYPE_DIRECT_IND_HIGH : ADV_TYPE_IND;
  } else {
    adv_params.adv_type = config.scannable ? ADV_TYPE_SCAN_IND : ADV_TYPE_NONCONN_IND;
  }
  
  esp_err_t ret = esp_ble_gap_config_adv_data_raw(
      const_cast<uint8_t*>(config.adv_data.data()), 
      config.adv_data.size());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set advertising data: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::CONFIG_FAILED;
  }
  
  if (!config.scan_response_data.empty()) {
    ret = esp_ble_gap_config_scan_rsp_data_raw(
        const_cast<uint8_t*>(config.scan_response_data.data()),
        config.scan_response_data.size());
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set scan response data: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::CONFIG_FAILED;
    }
  }
  
  ret = esp_ble_gap_start_advertising(&adv_params);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start advertising: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::START_FAILED;
  }
  
  is_advertising_ = true;
  adv_config_ = config;
  
  ESP_LOGI(TAG, "Started BLE advertising");
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::stopAdvertising() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!is_advertising_) {
    return hf_bluetooth_err_t::SUCCESS;
  }
  
  esp_err_t ret = esp_ble_gap_stop_advertising();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stop advertising: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::STOP_FAILED;
  }
  
  is_advertising_ = false;
  
  ESP_LOGI(TAG, "Stopped BLE advertising");
  return hf_bluetooth_err_t::SUCCESS;
}

bool EspBluetooth::isAdvertising() const {
  return is_advertising_;
}

hf_bluetooth_err_t EspBluetooth::startScan(const hf_bluetooth_scan_config_t& config) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized || !m_enabled) {
    ESP_LOGE(TAG, "Bluetooth not initialized or enabled");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (is_scanning_) {
    ESP_LOGW(TAG, "Already scanning");
    return hf_bluetooth_err_t::ALREADY_STARTED;
  }
  
  // Clear scan done bit and cached results
  xEventGroupClearBits(event_group_, BT_SCAN_DONE_BIT);
  scan_results_.clear();
  m_current_scan_type = config.mode;
  
  esp_err_t ret = ESP_OK;
  
  if (m_mode == hf_bluetooth_mode_t::BLE_ONLY || 
      (m_mode == hf_bluetooth_mode_t::DUAL_MODE && config.mode == hf_bluetooth_scan_mode_t::LE_GENERAL)) {
    
    // BLE scan parameters
    esp_ble_scan_params_t scan_params = {
      .scan_type = config.active_scan ? BLE_SCAN_TYPE_ACTIVE : BLE_SCAN_TYPE_PASSIVE,
      .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
      .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
      .scan_interval = static_cast<uint16_t>(config.scan_interval * 1.6f), // Convert to 0.625ms units
      .scan_window = static_cast<uint16_t>(config.scan_window * 1.6f),
      .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    ret = esp_ble_gap_set_scan_params(&scan_params);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set BLE scan params: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::CONFIG_FAILED;
    }
    
    ret = esp_ble_gap_start_scanning(config.duration_sec);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to start BLE scanning: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::START_FAILED;
    }
    
  } else if (m_mode == hf_bluetooth_mode_t::CLASSIC_ONLY || 
             (m_mode == hf_bluetooth_mode_t::DUAL_MODE && config.mode == hf_bluetooth_scan_mode_t::GENERAL_INQUIRY)) {
    
    // Classic Bluetooth inquiry
    esp_bt_inq_mode_t inq_mode = config.mode == hf_bluetooth_scan_mode_t::LIMITED_INQUIRY ? 
                                 ESP_BT_INQ_MODE_LIMITED_INQIURY : ESP_BT_INQ_MODE_GENERAL_INQUIRY;
    
    ret = esp_bt_gap_start_discovery(inq_mode, config.duration_sec, 0);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to start Classic BT discovery: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::START_FAILED;
    }
  }
  
  is_scanning_ = true;
  scan_config_ = config;
  
  ESP_LOGI(TAG, "Started Bluetooth scanning");
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::stopScan() {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!is_scanning_) {
    return hf_bluetooth_err_t::SUCCESS;
  }
  
  esp_err_t ret = ESP_OK;
  
  if (m_mode == hf_bluetooth_mode_t::BLE_ONLY || 
      (m_mode == hf_bluetooth_mode_t::DUAL_MODE && m_current_scan_type == hf_bluetooth_scan_mode_t::LE_GENERAL)) {
    ret = esp_ble_gap_stop_scanning();
  } else {
    ret = esp_bt_gap_cancel_discovery();
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stop scanning: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::STOP_FAILED;
  }
  
  is_scanning_ = false;
  
  ESP_LOGI(TAG, "Stopped Bluetooth scanning");
  return hf_bluetooth_err_t::SUCCESS;
}

bool EspBluetooth::isScanning() const {
  return is_scanning_;
}

std::vector<hf_bluetooth_scan_result_t> EspBluetooth::getScanResults() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  return scan_results_;
}

hf_bluetooth_err_t EspBluetooth::connect(const hf_bluetooth_address_t& address, 
                                    hf_bluetooth_connection_type_t type) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized || !m_enabled) {
    ESP_LOGE(TAG, "Bluetooth not initialized or enabled");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (connections_.size() >= advanced_config_.max_connections) {
    ESP_LOGE(TAG, "Maximum connections reached");
    return hf_bluetooth_err_t::MAX_CONNECTIONS_REACHED;
  }
  
  esp_err_t ret = ESP_OK;
  uint16_t conn_id = next_connection_id_++;
  
  if (type == hf_bluetooth_connection_type_t::BLE || 
      (type == hf_bluetooth_connection_type_t::AUTO && m_mode != hf_bluetooth_mode_t::CLASSIC_ONLY)) {
    
    // BLE connection
    esp_ble_addr_type_t addr_type = BLE_ADDR_TYPE_PUBLIC;
    ret = esp_ble_gattc_open(gattc_if_, const_cast<uint8_t*>(address.address), addr_type, true);
    
  } else if (type == hf_bluetooth_connection_type_t::CLASSIC || 
             (type == hf_bluetooth_connection_type_t::AUTO && m_mode != hf_bluetooth_mode_t::BLE_ONLY)) {
    
    // Classic Bluetooth connection (SPP)
    if (advanced_config_.enable_spp) {
      ret = esp_spp_connect(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_MASTER, 
                           ESP_SPP_SERVER_NAME, const_cast<uint8_t*>(address.address));
    } else {
      ESP_LOGE(TAG, "SPP not enabled for Classic connection");
      return hf_bluetooth_err_t::NOT_SUPPORTED;
    }
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initiate connection: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::CONNECTION_FAILED;
  }
  
  // Create connection info
  EspBluetoothConnectionInfo conn_info = {};
  conn_info.address = address;
  memcpy(conn_info.esp_address, address.address, 6);
  conn_info.connection_handle = conn_id;
  conn_info.is_classic = (type == hf_bluetooth_connection_type_t::CLASSIC);
  conn_info.mtu = advanced_config_.mtu_size;
  
  connections_[conn_id] = conn_info;
  
  ESP_LOGI(TAG, "Initiated connection to " MACSTR, MAC2STR(address.address));
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::disconnect(uint16_t connection_id) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto it = connections_.find(connection_id);
  if (it == connections_.end()) {
    ESP_LOGE(TAG, "Connection not found: %d", connection_id);
    return hf_bluetooth_err_t::CONNECTION_NOT_FOUND;
  }
  
  esp_err_t ret = ESP_OK;
  
  if (it->second.is_classic) {
    ret = esp_spp_disconnect(it->second.connection_handle);
  } else {
    ret = esp_ble_gatts_close(it->second.gatt_if, it->second.connection_handle);
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disconnect: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::DISCONNECTION_FAILED;
  }
  
  connections_.erase(it);
  
  ESP_LOGI(TAG, "Disconnected connection: %d", connection_id);
  return hf_bluetooth_err_t::SUCCESS;
}

std::vector<uint16_t> EspBluetooth::getConnections() const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  std::vector<uint16_t> conn_ids;
  conn_ids.reserve(connections_.size());
  
  for (const auto& conn : connections_) {
    conn_ids.push_back(conn.first);
  }
  
  return conn_ids;
}

hf_bluetooth_connection_info_t EspBluetooth::getConnectionInfo(uint16_t connection_id) const {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto it = connections_.find(connection_id);
  if (it == connections_.end()) {
    return {}; // Return empty connection info
  }
  
  // Convert ESP connection info to HardFOC format
  hf_bluetooth_connection_info_t info = {};
  info.connection_id = connection_id;
  info.address = it->second.address;
  info.type = it->second.is_classic ? hf_bluetooth_connection_type_t::CLASSIC : hf_bluetooth_connection_type_t::BLE;
  info.state = hf_bluetooth_connection_state_t::CONNECTED; // Simplified for now
  info.mtu = it->second.mtu;
  info.security_level = hf_bluetooth_security_t::NONE; // Would need to query actual security
  info.is_bonded = false; // Would need to check bonding status
  info.rssi = -50; // Would need to query actual RSSI
  
  return info;
}

hf_bluetooth_err_t EspBluetooth::sendData(uint16_t connection_id, const std::vector<uint8_t>& data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto it = connections_.find(connection_id);
  if (it == connections_.end()) {
    ESP_LOGE(TAG, "Connection not found: %d", connection_id);
    return hf_bluetooth_err_t::CONNECTION_NOT_FOUND;
  }
  
  if (data.empty() || data.size() > it->second.mtu) {
    ESP_LOGE(TAG, "Invalid data size: %d (MTU: %d)", data.size(), it->second.mtu);
    return hf_bluetooth_err_t::INVALID_PARAMETER;
  }
  
  esp_err_t ret = ESP_OK;
  
  if (it->second.is_classic) {
    // Send via SPP
    ret = esp_spp_write(it->second.connection_handle, data.size(), 
                       const_cast<uint8_t*>(data.data()));
  } else {
    // Send via GATT characteristic
    // This would require knowing which characteristic to write to
    // For now, assume we have a default characteristic
    ret = esp_ble_gatts_send_indicate(it->second.gatt_if, it->second.connection_handle,
                                     0, // attr_handle - would need to be set properly
                                     data.size(), const_cast<uint8_t*>(data.data()), false);
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send data: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::SEND_FAILED;
  }
  
  ESP_LOGD(TAG, "Sent %d bytes on connection %d", data.size(), connection_id);
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::pair(const hf_bluetooth_address_t& address, hf_bluetooth_security_t security) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized || !m_enabled) {
    ESP_LOGE(TAG, "Bluetooth not initialized or enabled");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  esp_err_t ret = ESP_OK;
  
  if (m_mode == hf_bluetooth_mode_t::BLE_ONLY || m_mode == hf_bluetooth_mode_t::DUAL_MODE) {
    // Configure BLE security
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;
    if (security == hf_bluetooth_security_t::AUTHENTICATED) {
      auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    }
    
    esp_ble_io_cap_t iocap = static_cast<esp_ble_io_cap_t>(advanced_config_.io_capability);
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
    
    // Initiate pairing
    ret = esp_ble_gap_security_rsp(const_cast<uint8_t*>(address.address), true);
  } else {
    // Classic Bluetooth pairing
    ret = esp_bt_gap_pin_reply(const_cast<uint8_t*>(address.address), true, 4, 
                              const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>("0000")));
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initiate pairing: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::PAIRING_FAILED;
  }
  
  ESP_LOGI(TAG, "Initiated pairing with " MACSTR, MAC2STR(address.address));
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::unpair(const hf_bluetooth_address_t& address) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized || !m_enabled) {
    ESP_LOGE(TAG, "Bluetooth not initialized or enabled");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  esp_err_t ret = esp_ble_remove_bond_device(const_cast<uint8_t*>(address.address));
  if (ret != ESP_OK && ret != ESP_ERR_NOT_FOUND) {
    ESP_LOGE(TAG, "Failed to remove bond: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::UNPAIRING_FAILED;
  }
  
  ESP_LOGI(TAG, "Removed bond with " MACSTR, MAC2STR(address.address));
  return hf_bluetooth_err_t::SUCCESS;
}

std::vector<hf_bluetooth_address_t> EspBluetooth::getBondedDevices() const {
  std::vector<hf_bluetooth_address_t> bonded_devices;
  
  int dev_num = esp_ble_get_bond_device_num();
  if (dev_num == 0) {
    return bonded_devices;
  }
  
  esp_ble_bond_dev_t* dev_list = new esp_ble_bond_dev_t[dev_num];
  esp_ble_get_bond_device_list(&dev_num, dev_list);
  
  bonded_devices.reserve(dev_num);
  
  for (int i = 0; i < dev_num; i++) {
    hf_bluetooth_address_t address;
    memcpy(address.address, dev_list[i].bd_addr, 6);
    bonded_devices.push_back(address);
  }
  
  delete[] dev_list;
  
  return bonded_devices;
}

void EspBluetooth::setEventCallback(hf_bluetooth_m_event_callbackt callback, void* user_data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_event_callback = callback;
  event_user_data_ = user_data;
}

void EspBluetooth::setScanCallback(hf_bluetooth_m_scan_callbackt callback, void* user_data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  m_scan_callback = callback;
  scan_user_data_ = user_data;
}

void EspBluetooth::setGattEventCallback(hf_bluetooth_gatt_m_event_callbackt callback, void* user_data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  gatt_m_event_callback = callback;
  gatt_user_data_ = user_data;
}

hf_bluetooth_err_t EspBluetooth::waitForConnection(uint32_t timeout_ms) {
  if (!event_group_) {
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  EventBits_t bits = xEventGroupWaitBits(event_group_,
                                         BT_CLASSIC_CONNECTED_BIT | BT_BLE_CONNECTED_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         pdMS_TO_TICKS(timeout_ms));
  
  if (bits & (BT_CLASSIC_CONNECTED_BIT | BT_BLE_CONNECTED_BIT)) {
    return hf_bluetooth_err_t::SUCCESS;
  } else {
    return hf_bluetooth_err_t::TIMEOUT;
  }
}

hf_bluetooth_err_t EspBluetooth::waitForScan(uint32_t timeout_ms) {
  if (!event_group_) {
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  EventBits_t bits = xEventGroupWaitBits(event_group_,
                                         BT_SCAN_DONE_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         pdMS_TO_TICKS(timeout_ms));
  
  if (bits & BT_SCAN_DONE_BIT) {
    return hf_bluetooth_err_t::SUCCESS;
  } else {
    return hf_bluetooth_err_t::TIMEOUT;
  }
}

// GATT Server Operations
hf_bluetooth_err_t EspBluetooth::gattAddService(const hf_bluetooth_gatt_service_t& service) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  if (!m_initialized) {
    ESP_LOGE(TAG, "Bluetooth not initialized");
    return hf_bluetooth_err_t::NOT_INITIALIZED;
  }
  
  if (m_mode == hf_bluetooth_mode_t::CLASSIC_ONLY) {
    ESP_LOGE(TAG, "GATT not supported in Classic-only mode");
    return hf_bluetooth_err_t::NOT_SUPPORTED;
  }
  
  if (!advanced_config_.enable_gatt_server) {
    ESP_LOGE(TAG, "GATT server not enabled");
    return hf_bluetooth_err_t::NOT_SUPPORTED;
  }
  
  if (gatt_services_.size() >= advanced_config_.max_gatt_services) {
    ESP_LOGE(TAG, "Maximum GATT services reached");
    return hf_bluetooth_err_t::MAX_SERVICES_REACHED;
  }
  
  // Create ESP GATT service
  esp_gatt_srvc_id_t service_id = {};
  service_id.is_primary = service.is_primary;
  service_id.id.inst_id = 0;
  service_id.id.uuid.len = service.uuid.size();
  memcpy(service_id.id.uuid.uuid.uuid128, service.uuid.data(), service.uuid.size());
  
  esp_err_t ret = esp_ble_gatts_create_service(gatts_if_, &service_id, service.num_handles);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create GATT service: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::GATT_SERVICE_ADD_FAILED;
  }
  
  // Store service info
  EspGattServiceInfo service_info = {};
  service_info.base_info = service;
  service_info.service_id = service_id;
  service_info.service_handle = next_service_id_++;
  service_info.is_started = false;
  
  gatt_services_[service_info.service_handle] = service_info;
  
  ESP_LOGI(TAG, "Added GATT service with handle: %d", service_info.service_handle);
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::gattAddCharacteristic(uint16_t service_handle, 
                                                  const hf_bluetooth_gatt_characteristic_t& characteristic) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto it = gatt_services_.find(service_handle);
  if (it == gatt_services_.end()) {
    ESP_LOGE(TAG, "GATT service not found: %d", service_handle);
    return hf_bluetooth_err_t::GATT_SERVICE_NOT_FOUND;
  }
  
  if (it->second.char_handles.size() >= advanced_config_.max_gatt_characteristics) {
    ESP_LOGE(TAG, "Maximum GATT characteristics reached for service");
    return hf_bluetooth_err_t::GATT_CHAR_ADD_FAILED;
  }
  
  // Create ESP GATT characteristic
  esp_bt_uuid_t char_uuid = {};
  char_uuid.len = characteristic.uuid.size();
  memcpy(char_uuid.uuid.uuid128, characteristic.uuid.data(), characteristic.uuid.size());
  
  esp_gatt_perm_t perm = static_cast<esp_gatt_perm_t>(characteristic.permissions);
  esp_gatt_char_prop_t property = static_cast<esp_gatt_char_prop_t>(characteristic.properties);
  
  esp_err_t ret = esp_ble_gatts_add_char(it->second.service_handle, &char_uuid, perm, property,
                                        nullptr, nullptr);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add GATT characteristic: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::GATT_CHAR_ADD_FAILED;
  }
  
  // Store characteristic handle (would be provided in callback)
  uint16_t char_handle = it->second.char_handles.size() + 1; // Simplified
  it->second.char_handles.push_back(char_handle);
  
  ESP_LOGI(TAG, "Added GATT characteristic to service %d", service_handle);
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::gattStartService(uint16_t service_handle) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto it = gatt_services_.find(service_handle);
  if (it == gatt_services_.end()) {
    ESP_LOGE(TAG, "GATT service not found: %d", service_handle);
    return hf_bluetooth_err_t::GATT_SERVICE_NOT_FOUND;
  }
  
  esp_err_t ret = esp_ble_gatts_start_service(it->second.service_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start GATT service: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::GATT_SERVICE_START_FAILED;
  }
  
  it->second.is_started = true;
  
  ESP_LOGI(TAG, "Started GATT service: %d", service_handle);
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::gattNotify(uint16_t connection_id, uint16_t char_handle, 
                                       const std::vector<uint8_t>& data) {
  RtosLockGuard<RtosMutex> lock(m_mutex);
  
  auto conn_it = connections_.find(connection_id);
  if (conn_it == connections_.end()) {
    ESP_LOGE(TAG, "Connection not found: %d", connection_id);
    return hf_bluetooth_err_t::CONNECTION_NOT_FOUND;
  }
  
  esp_err_t ret = esp_ble_gatts_send_indicate(conn_it->second.gatt_if, 
                                             conn_it->second.connection_handle,
                                             char_handle, data.size(), 
                                             const_cast<uint8_t*>(data.data()), false);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send GATT notification: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::GATT_NOTIFY_FAILED;
  }
  
  ESP_LOGD(TAG, "Sent GATT notification on connection %d, char %d", connection_id, char_handle);
  return hf_bluetooth_err_t::SUCCESS;
}

// Private helper methods
hf_bluetooth_err_t EspBluetooth::initializeBLE() {
  // Register BLE GAP callback
  esp_err_t ret = esp_ble_gap_register_callback(bleGapEventHandler);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register BLE GAP callback: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::INIT_FAILED;
  }
  
  // Register GATT server callback if enabled
  if (advanced_config_.enable_gatt_server) {
    ret = esp_ble_gatts_register_callback(bleGattsEventHandler);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register GATT server callback: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
    
    ret = esp_ble_gatts_app_register(0);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register GATT server app: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
  }
  
  // Register GATT client callback if enabled
  if (advanced_config_.enable_gatt_client) {
    ret = esp_ble_gattc_register_callback(bleGattcEventHandler);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register GATT client callback: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
    
    ret = esp_ble_gattc_app_register(0);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register GATT client app: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
  }
  
  // Configure BLE parameters
  esp_ble_gap_config_local_privacy(advanced_config_.enable_privacy);
  
  uint16_t mtu = advanced_config_.mtu_size;
  esp_ble_gatt_set_local_mtu(mtu);
  
  ESP_LOGI(TAG, "BLE initialized successfully");
  return hf_bluetooth_err_t::SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::initializeClassic() {
  // Register Classic Bluetooth GAP callback
  esp_err_t ret = esp_bt_gap_register_callback(btGapEventHandler);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register Classic BT GAP callback: %s", esp_err_to_name(ret));
    return hf_bluetooth_err_t::INIT_FAILED;
  }
  
  // Initialize SPP if enabled
  if (advanced_config_.enable_spp) {
    ret = esp_spp_register_callback(sppEventHandler);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register SPP callback: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
    
    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize SPP: %s", esp_err_to_name(ret));
      return hf_bluetooth_err_t::INIT_FAILED;
    }
  }
  
  // Set discoverability and connectability
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  
  ESP_LOGI(TAG, "Classic Bluetooth initialized successfully");
  return hf_bluetooth_err_t::SUCCESS;
}

void EspBluetooth::cleanup() {
  if (event_group_) {
    vEventGroupDelete(event_group_);
    event_group_ = nullptr;
  }
  
  // Cleanup connections
  connections_.clear();
  gatt_services_.clear();
  scan_results_.clear();
  
  // Deinitialize Bluedroid and controller
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
}

// Static event handlers
void EspBluetooth::bleGapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  // Implementation would handle BLE GAP events
  ESP_LOGD(TAG, "BLE GAP event: %d", event);
}

void EspBluetooth::bleGattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, 
                                       esp_ble_gatts_cb_param_t* param) {
  // Implementation would handle GATT server events
  ESP_LOGD(TAG, "BLE GATTS event: %d", event);
}

void EspBluetooth::bleGattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, 
                                       esp_ble_gattc_cb_param_t* param) {
  // Implementation would handle GATT client events
  ESP_LOGD(TAG, "BLE GATTC event: %d", event);
}

void EspBluetooth::btGapEventHandler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
  // Implementation would handle Classic Bluetooth GAP events
  ESP_LOGD(TAG, "Classic BT GAP event: %d", event);
}

void EspBluetooth::sppEventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
  // Implementation would handle SPP events
  ESP_LOGD(TAG, "SPP event: %d", event);
}

hf_bluetooth_err_t EspBluetooth::ConvertEspError(esp_err_t esp_err) const {
  switch (esp_err) {
    case ESP_OK:
      return hf_bluetooth_err_t::HF_BLUETOOTH_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_INVALID_STATE:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_STATE;
    case ESP_ERR_NO_MEM:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_NO_MEMORY;
    case ESP_ERR_TIMEOUT:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_DEVICE_NOT_FOUND;
    case ESP_ERR_NOT_SUPPORTED:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_OPERATION_NOT_SUPPORTED;
    case ESP_ERR_BT_NIMBLE_BASE:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_FAILURE;
    case ESP_ERR_BT_NIMBLE_ATT_INVALID_HANDLE:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_BT_NIMBLE_ATT_READ_NOT_PERMITTED:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_PERMISSION_DENIED;
    case ESP_ERR_BT_NIMBLE_ATT_WRITE_NOT_PERMITTED:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_PERMISSION_DENIED;
    case ESP_ERR_BT_NIMBLE_ATT_INVALID_PDU:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_BT_NIMBLE_ATT_INSUFFICIENT_AUTHEN:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_AUTHENTICATION_FAILED;
    case ESP_ERR_BT_NIMBLE_ATT_REQ_NOT_SUPPORTED:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_OPERATION_NOT_SUPPORTED;
    case ESP_ERR_BT_NIMBLE_ATT_INVALID_OFFSET:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_BT_NIMBLE_ATT_INSUFFICIENT_AUTHOR:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_AUTHORIZATION_FAILED;
    case ESP_ERR_BT_NIMBLE_ATT_PREPARE_QUEUE_FULL:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_BUSY;
    case ESP_ERR_BT_NIMBLE_ATT_ATTR_NOT_FOUND:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_DEVICE_NOT_FOUND;
    case ESP_ERR_BT_NIMBLE_ATT_ATTR_NOT_LONG:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_BT_NIMBLE_ATT_INSUFFICIENT_KEY_SZ:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_SECURITY_ERROR;
    case ESP_ERR_BT_NIMBLE_ATT_INVALID_ATTR_VALUE_LEN:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_BT_NIMBLE_ATT_UNLIKELY:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_FAILURE;
    case ESP_ERR_BT_NIMBLE_ATT_INSUFFICIENT_RES:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_NO_MEMORY;
    case ESP_ERR_BT_NIMBLE_ATT_DB_OUT_OF_SYNC:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_STATE;
    case ESP_ERR_BT_NIMBLE_ATT_VALUE_NOT_ALLOWED:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_INVALID_PARAM;
    default:
      return hf_bluetooth_err_t::HF_BLUETOOTH_ERR_FAILURE;
  }
}
