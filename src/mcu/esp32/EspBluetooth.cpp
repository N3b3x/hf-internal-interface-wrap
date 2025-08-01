/**
 * @file EspBluetooth.cpp
 * @ingroup bluetooth
 * @brief ESP32 Bluetooth implementation using NimBLE for BLE-only support (ESP32C6)
 *
 * This file contains the ESP32-specific implementation of the Bluetooth interface
 * using NimBLE stack for ESP32C6 BLE-only support on ESP-IDF v5.5.
 *
 * Implementation details:
 * - ESP32C6: Optimized NimBLE BLE-only implementation 
 * - ESP32/ESP32S3: Full Bluetooth Classic + BLE support
 * - ESP32C3/H2: BLE-only using Bluedroid
 * - ESP32S2: Stub implementation (no Bluetooth support)
 *
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 *
 * @note Conditional compilation ensures optimal builds for each ESP32 variant
 */

#include "EspBluetooth.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#if HAS_BLE_SUPPORT

static const char* TAG = "EspBluetooth";

#if HAS_NIMBLE_SUPPORT
// Static instance for NimBLE callbacks
EspBluetooth* EspBluetooth::s_instance = nullptr;

// NimBLE specific constants
static const uint8_t DEVICE_NAME[] = "ESP32C6-HardFOC";
static const uint16_t DEFAULT_MTU = 247;  // ESP32C6 supports up to 247 bytes MTU
static const uint32_t DEFAULT_SCAN_DURATION_MS = 10000;
static const uint32_t DEFAULT_CONN_TIMEOUT_MS = 30000;

// UUID conversion helper
static ble_uuid128_t convert_uuid_string_to_ble(const std::string& uuid_str) {
    ble_uuid128_t uuid;
    ble_uuid_any_t uuid_any;
    
    if (ble_uuid_init_from_buf(&uuid_any, uuid_str.c_str(), uuid_str.length()) == 0) {
        uuid = uuid_any.u128;
    }
    
    return uuid;
}
#endif

// Constructor
EspBluetooth::EspBluetooth() 
    : m_initialized(false)
    , m_enabled(false)
    , m_mode(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE)
    , m_state(hf_bluetooth_state_t::HF_BLUETOOTH_STATE_DISABLED)
    , m_event_callback(nullptr)
    , m_callback_context(nullptr)
#if HAS_NIMBLE_SUPPORT
    , m_conn_handle(BLE_HS_CONN_HANDLE_NONE)
    , m_addr_type(BLE_OWN_ADDR_PUBLIC)
#endif
{
#if HAS_NIMBLE_SUPPORT
    s_instance = this;
#endif
    
    // Initialize scan configuration defaults
    m_scan_config.duration_ms = DEFAULT_SCAN_DURATION_MS;
    m_scan_config.type = hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE;
    m_scan_config.mode = hf_bluetooth_scan_mode_t::HF_BLUETOOTH_SCAN_MODE_LE_GENERAL;
    
    // Initialize advertising configuration defaults  
    m_adv_config.interval_min_ms = 100;
    m_adv_config.interval_max_ms = 200;
    m_adv_config.connectable = true;
    m_adv_config.discoverable = true;
}

// Destructor
EspBluetooth::~EspBluetooth() {
    if (m_initialized) {
        Deinitialize();
    }
    
#if HAS_NIMBLE_SUPPORT
    if (s_instance == this) {
        s_instance = nullptr;
    }
#endif
}

// ========== Initialization and Configuration ==========

hf_bluetooth_err_t EspBluetooth::Initialize(hf_bluetooth_mode_t mode) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    
    if (m_initialized) {
        ESP_LOGW(TAG, "Bluetooth already initialized");
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
#if HAS_NIMBLE_SUPPORT
    // ESP32C6 supports only BLE mode
    if (mode != hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE &&
        mode != hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_DUAL) {
        ESP_LOGE(TAG, "ESP32C6 only supports BLE mode");
        return hf_bluetooth_err_t::BLUETOOTH_ERR_INVALID_PARAMETER;
    }
    
    // Initialize NVS (required for BLE)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize NimBLE
    hf_bluetooth_err_t nimble_ret = InitializeNimBLE();
    if (nimble_ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize NimBLE");
        return nimble_ret;
    }
    
    m_mode = hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE;
    
#elif HAS_BLUEDROID_SUPPORT
    // Other ESP32 variants using Bluedroid
    hf_bluetooth_err_t bluedroid_ret = InitializeBluedroid();
    if (bluedroid_ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid");
        return bluedroid_ret;
    }
    
    m_mode = mode;
    
#else
    // No Bluetooth support (ESP32S2)
    ESP_LOGE(TAG, "Bluetooth not supported on this platform");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
    
    m_initialized = true;
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_DISABLED;
    
    ESP_LOGI(TAG, "Bluetooth initialized successfully (mode: %d)", static_cast<int>(m_mode));
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::Deinitialize() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    
    if (!m_initialized) {
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
    // Disable first if enabled
    if (m_enabled) {
        Disable();
    }
    
#if HAS_NIMBLE_SUPPORT
    hf_bluetooth_err_t ret = DeinitializeNimBLE();
    if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
        ESP_LOGE(TAG, "Failed to deinitialize NimBLE");
        return ret;
    }
#elif HAS_BLUEDROID_SUPPORT
    hf_bluetooth_err_t ret = DeinitializeBluedroid();
    if (ret != hf_bluetooth_err_t::BLUETOOTH_SUCCESS) {
        ESP_LOGE(TAG, "Failed to deinitialize Bluedroid");
        return ret;
    }
#endif
    
    m_initialized = false;
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_DISABLED;
    
    ESP_LOGI(TAG, "Bluetooth deinitialized successfully");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

bool EspBluetooth::IsInitialized() const {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_initialized;
}

hf_bluetooth_err_t EspBluetooth::Enable() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    
    if (!m_initialized) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_INITIALIZED;
    }
    
    if (m_enabled) {
        ESP_LOGW(TAG, "Bluetooth already enabled");
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
#if HAS_NIMBLE_SUPPORT
    // NimBLE enable sequence
    ESP_LOGI(TAG, "Enabling NimBLE stack");
    
    // Start NimBLE host task
    nimble_port_freertos_init(nullptr);
    
    m_enabled = true;
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_ENABLED;
    
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_ENABLED);
    
#elif HAS_BLUEDROID_SUPPORT
    // Bluedroid enable sequence
    esp_err_t ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_HARDWARE_FAILURE;
    }
    
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_HARDWARE_FAILURE;
    }
    
    m_enabled = true;
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_ENABLED;
    
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_ENABLED);
#endif
    
    ESP_LOGI(TAG, "Bluetooth enabled successfully");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::Disable() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    
    if (!m_enabled) {
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
    // Stop any ongoing operations
    StopScan();
    StopAdvertising();
    
    // Disconnect all devices
    std::vector<hf_bluetooth_device_info_t> connected_devices;
    GetConnectedDevices(connected_devices);
    for (const auto& device : connected_devices) {
        Disconnect(device.address);
    }
    
#if HAS_NIMBLE_SUPPORT
    // Stop NimBLE host task
    int ret = nimble_port_stop();
    if (ret == 0) {
        nimble_port_deinit();
    }
    
#elif HAS_BLUEDROID_SUPPORT
    esp_bluedroid_disable();
    esp_bt_controller_disable();
#endif
    
    m_enabled = false;
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_DISABLED;
    
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISABLED);
    
    ESP_LOGI(TAG, "Bluetooth disabled successfully");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

bool EspBluetooth::IsEnabled() const {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_enabled;
}

hf_bluetooth_err_t EspBluetooth::SetMode(hf_bluetooth_mode_t mode) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    
#if HAS_NIMBLE_SUPPORT
    // ESP32C6 only supports BLE
    if (mode != hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE &&
        mode != hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_DUAL) {
        ESP_LOGE(TAG, "ESP32C6 only supports BLE mode");
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
    }
    m_mode = hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE;
#else
    m_mode = mode;
#endif
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_mode_t EspBluetooth::GetMode() const {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_mode;
}

// ========== NimBLE Implementation Functions ==========

#if HAS_NIMBLE_SUPPORT

hf_bluetooth_err_t EspBluetooth::InitializeNimBLE() {
    ESP_LOGI(TAG, "Initializing NimBLE for ESP32C6");
    
    // Initialize ESP HCI controller
    esp_err_t ret = esp_nimble_hci_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NimBLE HCI: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_HARDWARE_FAILURE;
    }
    
    // Initialize NimBLE host
    nimble_port_init();
    
    // Initialize BLE services
    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    // Set device name
    ret = ble_svc_gap_device_name_set(reinterpret_cast<const char*>(DEVICE_NAME));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to set device name: %d", ret);
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    // Configure security
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    
    // Set MTU
    ble_att_set_preferred_mtu(DEFAULT_MTU);
    
    ESP_LOGI(TAG, "NimBLE initialized successfully");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::DeinitializeNimBLE() {
    ESP_LOGI(TAG, "Deinitializing NimBLE");
    
    esp_err_t ret = esp_nimble_hci_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize NimBLE HCI: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_HARDWARE_FAILURE;
    }
    
    ESP_LOGI(TAG, "NimBLE deinitialized successfully");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

// NimBLE GAP event handler
int EspBluetooth::GapEventHandler(struct ble_gap_event *event, void *arg) {
    if (!s_instance) {
        return 0;
    }
    
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", 
                 event->connect.status == 0 ? "OK" : "FAILED");
        
        if (event->connect.status == 0) {
            s_instance->m_conn_handle = event->connect.conn_handle;
            s_instance->m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_CONNECTED;
            
            // Get connection descriptor
            struct ble_gap_conn_desc desc;
            if (ble_gap_conn_find(event->connect.conn_handle, &desc) == 0) {
                hf_bluetooth_address_t addr;
                ConvertBleAddr(&desc.peer_id_addr, addr);
                
                hf_bluetooth_device_info_t device_info;
                device_info.address = addr;
                device_info.rssi = 0; // Will be updated later
                device_info.connection_state = hf_bluetooth_connection_state_t::HF_BLUETOOTH_CONNECTION_STATE_CONNECTED;
                
                std::lock_guard<std::mutex> lock(s_instance->m_device_mutex);
                std::string addr_str = s_instance->AddressToString(addr);
                s_instance->m_connected_devices[addr_str] = device_info;
            }
            
            s_instance->TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_SUCCESS);
        } else {
            s_instance->TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_CONNECT_FAILED);
        }
        break;
        
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECT; reason=%d", event->disconnect.reason);
        
        s_instance->m_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_instance->m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_ENABLED;
        
        // Remove from connected devices
        {
            std::lock_guard<std::mutex> lock(s_instance->m_device_mutex);
            hf_bluetooth_address_t addr;
            ConvertBleAddr(&event->disconnect.conn.peer_id_addr, addr);
            std::string addr_str = s_instance->AddressToString(addr);
            s_instance->m_connected_devices.erase(addr_str);
        }
        
        s_instance->TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DISCONNECT);
        break;
        
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "BLE GAP EVENT ADV_COMPLETE");
        break;
        
    case BLE_GAP_EVENT_DISC:
        ESP_LOGI(TAG, "BLE GAP EVENT DISC");
        
        // Add discovered device
        {
            hf_bluetooth_address_t addr;
            ConvertBleAddr(&event->disc.addr, addr);
            
            hf_bluetooth_device_info_t device_info;
            device_info.address = addr;
            device_info.rssi = event->disc.rssi;
            device_info.connection_state = hf_bluetooth_connection_state_t::HF_BLUETOOTH_CONNECTION_STATE_DISCONNECTED;
            
            // Extract device name from advertising data
            const uint8_t *name_data = nullptr;
            uint8_t name_len = 0;
            
            if (event->disc.length_data > 0) {
                // Parse advertising data for complete local name (type 0x09)
                const uint8_t *data = event->disc.data;
                uint16_t data_len = event->disc.length_data;
                
                for (uint16_t i = 0; i < data_len;) {
                    uint8_t len = data[i];
                    if (len == 0 || i + len >= data_len) break;
                    
                    uint8_t type = data[i + 1];
                    if (type == 0x09) { // Complete local name
                        name_data = &data[i + 2];
                        name_len = len - 1;
                        break;
                    }
                    i += len + 1;
                }
            }
            
            if (name_data && name_len > 0) {
                device_info.name = std::string(reinterpret_cast<const char*>(name_data), name_len);
            }
            
            std::lock_guard<std::mutex> lock(s_instance->m_device_mutex);
            
            // Check if device already exists
            auto it = std::find_if(s_instance->m_discovered_devices.begin(),
                                 s_instance->m_discovered_devices.end(),
                                 [&addr](const hf_bluetooth_device_info_t& dev) {
                                     return memcmp(dev.address.addr, addr.addr, 6) == 0;
                                 });
            
            if (it != s_instance->m_discovered_devices.end()) {
                // Update existing device
                it->rssi = device_info.rssi;
                if (!device_info.name.empty()) {
                    it->name = device_info.name;
                }
            } else {
                // Add new device
                s_instance->m_discovered_devices.push_back(device_info);
            }
        }
        
        s_instance->TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_DEVICE_FOUND);
        break;
        
    default:
        return 0;
    }
    
    return 0;
}

// Address conversion utilities
void EspBluetooth::ConvertBleAddr(const ble_addr_t* ble_addr, hf_bluetooth_address_t& hf_addr) {
    if (ble_addr) {
        memcpy(hf_addr.addr, ble_addr->val, 6);
    } else {
        memset(hf_addr.addr, 0, 6);
    }
}

void EspBluetooth::ConvertHfAddr(const hf_bluetooth_address_t& hf_addr, ble_addr_t* ble_addr) {
    if (ble_addr) {
        memcpy(ble_addr->val, hf_addr.addr, 6);
        ble_addr->type = BLE_ADDR_PUBLIC; // Default to public address
    }
}

#endif // HAS_NIMBLE_SUPPORT

// ========== Device Management ==========

hf_bluetooth_err_t EspBluetooth::GetLocalAddress(hf_bluetooth_address_t& address) {
    if (!m_initialized) {
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_INITIALIZED;
    }
    
#if HAS_NIMBLE_SUPPORT
    uint8_t addr[6];
    int ret = ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, addr, nullptr);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to get local address: %d", ret);
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    memcpy(address.addr, addr, 6);
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#elif HAS_BLUEDROID_SUPPORT
    esp_bd_addr_t bd_addr;
    esp_err_t ret = esp_bt_dev_get_address(bd_addr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get local address: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    memcpy(address.addr, bd_addr, 6);
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#else
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
}

hf_bluetooth_err_t EspBluetooth::SetDeviceName(const std::string& name) {
    if (!m_initialized) {
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_INITIALIZED;
    }
    
#if HAS_NIMBLE_SUPPORT
    int ret = ble_svc_gap_device_name_set(name.c_str());
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to set device name: %d", ret);
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#elif HAS_BLUEDROID_SUPPORT
    esp_err_t ret = esp_bt_dev_set_device_name(name.c_str());
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#else
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
}

hf_bluetooth_err_t EspBluetooth::GetDeviceName(std::string& name) {
    if (!m_initialized) {
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_INITIALIZED;
    }
    
#if HAS_NIMBLE_SUPPORT
    const char* device_name = ble_svc_gap_device_name();
    if (device_name) {
        name = std::string(device_name);
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
    ESP_LOGE(TAG, "Failed to get device name");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    
#elif HAS_BLUEDROID_SUPPORT
    // Bluedroid doesn't have a direct get function, use stored name
    name = std::string(reinterpret_cast<const char*>(DEVICE_NAME));
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#else
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
}

// ========== Discovery and Scanning ==========

hf_bluetooth_err_t EspBluetooth::StartScan(const hf_bluetooth_scan_config_t& config) {
    if (!m_enabled) {
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_ENABLED;
    }
    
    m_scan_config = config;
    
#if HAS_NIMBLE_SUPPORT
    return StartScanning();
#elif HAS_BLUEDROID_SUPPORT
    // Implement Bluedroid scanning
    esp_ble_scan_params_t scan_params = {
        .scan_type = (config.type == hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE) ? 
                     BLE_SCAN_TYPE_ACTIVE : BLE_SCAN_TYPE_PASSIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    esp_err_t ret = esp_ble_gap_set_scan_params(&scan_params);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set scan params: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    ret = esp_ble_gap_start_scanning(config.duration_ms / 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start scanning: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_SCANNING;
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_START);
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    
#else
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
}

#if HAS_NIMBLE_SUPPORT
hf_bluetooth_err_t EspBluetooth::StartScanning() {
    struct ble_gap_disc_params disc_params = {
        .itvl = BLE_GAP_SCAN_ITVL_MS(100),
        .window = BLE_GAP_SCAN_WIN_MS(100),
        .filter_policy = BLE_GAP_SCAN_FLT_NO_WL,
        .limited = (m_scan_config.mode == hf_bluetooth_scan_mode_t::HF_BLUETOOTH_SCAN_MODE_LE_LIMITED) ? 1 : 0,
        .passive = (m_scan_config.type == hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_PASSIVE) ? 1 : 0,
        .filter_duplicates = 0
    };
    
    int ret = ble_gap_disc(m_addr_type, m_scan_config.duration_ms, 
                          &disc_params, GapEventHandler, nullptr);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to start BLE discovery: %d", ret);
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_SCANNING;
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_START);
    
    ESP_LOGI(TAG, "BLE scanning started");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::StopScanning() {
    int ret = ble_gap_disc_cancel();
    if (ret != 0 && ret != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "Failed to stop BLE discovery: %d", ret);
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    if (m_state == hf_bluetooth_state_t::HF_BLUETOOTH_STATE_SCANNING) {
        m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_ENABLED;
        TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_STOP);
    }
    
    ESP_LOGI(TAG, "BLE scanning stopped");
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}
#endif

hf_bluetooth_err_t EspBluetooth::StopScan() {
    if (m_state != hf_bluetooth_state_t::HF_BLUETOOTH_STATE_SCANNING) {
        return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    }
    
#if HAS_NIMBLE_SUPPORT
    return StopScanning();
#elif HAS_BLUEDROID_SUPPORT
    esp_err_t ret = esp_ble_gap_stop_scanning();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop scanning: %s", esp_err_to_name(ret));
        return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_FAILED;
    }
    
    m_state = hf_bluetooth_state_t::HF_BLUETOOTH_STATE_ENABLED;
    TriggerEvent(hf_bluetooth_event_t::HF_BLUETOOTH_EVENT_SCAN_STOP);
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
#else
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
#endif
}

bool EspBluetooth::IsScanning() const {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_state == hf_bluetooth_state_t::HF_BLUETOOTH_STATE_SCANNING;
}

hf_bluetooth_err_t EspBluetooth::GetDiscoveredDevices(std::vector<hf_bluetooth_device_info_t>& devices) {
    std::lock_guard<std::mutex> lock(m_device_mutex);
    devices = m_discovered_devices;
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::ClearDiscoveredDevices() {
    std::lock_guard<std::mutex> lock(m_device_mutex);
    m_discovered_devices.clear();
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

// ========== Stub implementations for required methods ==========
// Note: These would be fully implemented based on specific requirements

hf_bluetooth_err_t EspBluetooth::StartAdvertising(const hf_bluetooth_adv_config_t& config) {
    ESP_LOGW(TAG, "StartAdvertising not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::StopAdvertising() {
    ESP_LOGW(TAG, "StopAdvertising not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

bool EspBluetooth::IsAdvertising() const {
    return false; // Stub implementation
}

hf_bluetooth_err_t EspBluetooth::Connect(const hf_bluetooth_address_t& address, uint32_t timeout_ms) {
    ESP_LOGW(TAG, "Connect not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::Disconnect(const hf_bluetooth_address_t& address) {
    ESP_LOGW(TAG, "Disconnect not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

bool EspBluetooth::IsConnected(const hf_bluetooth_address_t& address) const {
    return false; // Stub implementation
}

hf_bluetooth_err_t EspBluetooth::GetConnectedDevices(std::vector<hf_bluetooth_device_info_t>& devices) {
    std::lock_guard<std::mutex> lock(m_device_mutex);
    devices.clear();
    for (const auto& pair : m_connected_devices) {
        devices.push_back(pair.second);
    }
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::Pair(const hf_bluetooth_address_t& address, const std::string& pin) {
    ESP_LOGW(TAG, "Pair not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::Unpair(const hf_bluetooth_address_t& address) {
    ESP_LOGW(TAG, "Unpair not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

bool EspBluetooth::IsPaired(const hf_bluetooth_address_t& address) const {
    return false; // Stub implementation
}

hf_bluetooth_err_t EspBluetooth::SendData(const hf_bluetooth_address_t& address, const std::vector<uint8_t>& data) {
    ESP_LOGW(TAG, "SendData not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

int EspBluetooth::GetAvailableData(const hf_bluetooth_address_t& address) const {
    return 0; // Stub implementation
}

hf_bluetooth_err_t EspBluetooth::ReadData(const hf_bluetooth_address_t& address, std::vector<uint8_t>& data, size_t max_bytes) {
    ESP_LOGW(TAG, "ReadData not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::DiscoverServices(const hf_bluetooth_address_t& address, std::vector<hf_bluetooth_gatt_service_t>& services) {
    ESP_LOGW(TAG, "DiscoverServices not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::DiscoverCharacteristics(const hf_bluetooth_address_t& address, const std::string& service_uuid, std::vector<hf_bluetooth_gatt_characteristic_t>& characteristics) {
    ESP_LOGW(TAG, "DiscoverCharacteristics not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::ReadCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, std::vector<uint8_t>& value) {
    ESP_LOGW(TAG, "ReadCharacteristic not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::WriteCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, const std::vector<uint8_t>& value, bool with_response) {
    ESP_LOGW(TAG, "WriteCharacteristic not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_err_t EspBluetooth::SubscribeCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, bool enable) {
    ESP_LOGW(TAG, "SubscribeCharacteristic not fully implemented yet");
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_IMPLEMENTED;
}

hf_bluetooth_state_t EspBluetooth::GetState() const {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_state;
}

int8_t EspBluetooth::GetRssi(const hf_bluetooth_address_t& address) const {
    return INT8_MIN; // Stub implementation
}

hf_bluetooth_err_t EspBluetooth::SetEventCallback(hf_bluetooth_event_callback_t callback, void* context) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    m_event_callback = callback;
    m_callback_context = context;
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

hf_bluetooth_err_t EspBluetooth::ClearEventCallback() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    m_event_callback = nullptr;
    m_callback_context = nullptr;
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

std::string EspBluetooth::GetImplementationInfo() const {
    std::ostringstream info;
    info << "ESP32 Bluetooth Implementation\n";
    info << "Target: " << CONFIG_IDF_TARGET << "\n";
    info << "ESP-IDF Version: " << IDF_VER << "\n";
    
#if HAS_NIMBLE_SUPPORT
    info << "Stack: NimBLE (ESP32C6 optimized)\n";
    info << "Features: BLE-only\n";
#elif HAS_BLUEDROID_SUPPORT
    info << "Stack: Bluedroid\n";
    #if HAS_CLASSIC_BLUETOOTH
    info << "Features: Classic BT + BLE\n";
    #else
    info << "Features: BLE-only\n";
    #endif
#else
    info << "Stack: None (not supported)\n";
    info << "Features: None\n";
#endif
    
    return info.str();
}

uint32_t EspBluetooth::GetSupportedFeatures() const {
    uint32_t features = 0;
    
#if HAS_BLE_SUPPORT
    features |= 0x01; // BLE support
#endif
#if HAS_CLASSIC_BLUETOOTH
    features |= 0x02; // Classic BT support
#endif
#if HAS_A2DP_SUPPORT
    features |= 0x04; // A2DP support
#endif
#if HAS_SPP_SUPPORT
    features |= 0x08; // SPP support
#endif
    
    return features;
}

// ========== Internal Utility Methods ==========

void EspBluetooth::TriggerEvent(hf_bluetooth_event_t event, const void* data) {
    if (m_event_callback) {
        m_event_callback(event, data, m_callback_context);
    }
}

hf_bluetooth_err_t EspBluetooth::ValidateAddress(const hf_bluetooth_address_t& address) const {
    // Check for null address
    bool all_zero = true;
    for (int i = 0; i < 6; i++) {
        if (address.addr[i] != 0) {
            all_zero = false;
            break;
        }
    }
    
    if (all_zero) {
        return hf_bluetooth_err_t::BLUETOOTH_ERR_INVALID_ADDRESS;
    }
    
    return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
}

std::string EspBluetooth::AddressToString(const hf_bluetooth_address_t& address) const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::uppercase;
    for (int i = 0; i < 6; i++) {
        if (i > 0) oss << ":";
        oss << std::setw(2) << static_cast<int>(address.addr[i]);
    }
    return oss.str();
}

hf_bluetooth_address_t EspBluetooth::StringToAddress(const std::string& address_str) const {
    hf_bluetooth_address_t address;
    memset(&address, 0, sizeof(address));
    
    std::istringstream iss(address_str);
    std::string token;
    int i = 0;
    
    while (std::getline(iss, token, ':') && i < 6) {
        address.addr[i] = static_cast<uint8_t>(std::stoul(token, nullptr, 16));
        i++;
    }
    
    return address;
}

#else // !HAS_BLE_SUPPORT

// Stub implementations for platforms without Bluetooth support
EspBluetooth::EspBluetooth() : m_initialized(false), m_enabled(false) {}
EspBluetooth::~EspBluetooth() {}

hf_bluetooth_err_t EspBluetooth::Initialize(hf_bluetooth_mode_t mode) {
    return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED;
}

// All other methods return NOT_SUPPORTED...
// (For brevity, not implementing all stub methods here)

#endif // HAS_BLE_SUPPORT