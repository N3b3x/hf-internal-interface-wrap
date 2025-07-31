/**
 * @file EspBluetooth.cpp
 * @brief ESP32-C6 Bluetooth 5.0 LE implementation using NimBLE host stack for ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This implementation provides a comprehensive Bluetooth Low Energy interface
 * for ESP32-C6 with ESP-IDF v5.5, featuring modern C++17 design patterns,
 * thread-safe operations, and power-efficient implementation.
 */

#include "mcu/esp32/EspBluetooth.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace esp32 {
namespace bluetooth {

// Static member definitions
constexpr const char* EspBluetooth::TAG;

// Global instance for NimBLE callbacks
static EspBluetooth* g_instance = nullptr;

// GATT service definitions
static struct ble_gatt_svc_def default_services[] = {
    {
        // GAP Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP),
        .characteristics = nullptr,
    },
    {
        // GATT Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_GATT),
        .characteristics = nullptr,
    },
    {
        0, // No more services
    },
};

EspBluetooth& EspBluetooth::getInstance() {
    static EspBluetooth instance;
    return instance;
}

EspBluetooth::~EspBluetooth() {
    if (initialized_.load()) {
        deinitialize();
    }
}

esp_err_t EspBluetooth::initialize(Mode mode, const std::string& device_name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (initialized_.load()) {
        ESP_LOGW(TAG, "Bluetooth already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing ESP32-C6 Bluetooth 5.0 LE with NimBLE");
    
    // Store global instance for callbacks
    g_instance = this;
    current_mode_ = mode;
    device_name_ = device_name;

    esp_err_t ret;

    // Initialize NVS flash
    ret = initializeNVS();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize NimBLE stack
    ret = initializeNimBLE();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NimBLE: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure GAP settings
    ret = configureGAP();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GAP: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure GATT settings
    ret = configureGATT();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GATT: %s", esp_err_to_name(ret));
        return ret;
    }

    // Start NimBLE host task
    nimble_port_freertos_init(nimbleHostTask);

    initialized_.store(true);
    ESP_LOGI(TAG, "Bluetooth initialized successfully in mode: %d", static_cast<int>(mode));

    return ESP_OK;
}

esp_err_t EspBluetooth::deinitialize() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!initialized_.load()) {
        ESP_LOGW(TAG, "Bluetooth not initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing Bluetooth");

    // Stop advertising if active
    if (advertising_.load()) {
        stopAdvertising();
    }

    // Stop scanning if active
    if (scanning_.load()) {
        stopScan();
    }

    // Stop NimBLE host
    int ret = nimble_port_stop();
    if (ret == 0) {
        nimble_port_deinit();
        
        ret = esp_nimble_hci_deinit();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to deinitialize HCI: %s", esp_err_to_name(ret));
        }
    }

    // Clean up GATT services
    gatt_services_.clear();

    // Reset state
    initialized_.store(false);
    advertising_.store(false);
    scanning_.store(false);
    connection_count_.store(0);
    current_mode_ = Mode::DISABLED;

    // Clear global instance
    g_instance = nullptr;

    ESP_LOGI(TAG, "Bluetooth deinitialized successfully");
    return ESP_OK;
}

esp_err_t EspBluetooth::startAdvertising(const AdvertisementData& adv_data, 
                                        AdvertisementType type, 
                                        uint32_t interval_ms) {
    if (!initialized_.load()) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (advertising_.load()) {
        ESP_LOGW(TAG, "Already advertising, stopping first");
        stopAdvertising();
    }

    ESP_LOGI(TAG, "Starting advertising with interval %lu ms", interval_ms);

    // Prepare advertisement data
    struct ble_hs_adv_fields fields = {0};
    
    if (adv_data.include_name && !adv_data.local_name.empty()) {
        fields.name = reinterpret_cast<const uint8_t*>(adv_data.local_name.c_str());
        fields.name_len = adv_data.local_name.length();
        fields.name_is_complete = 1;
    }

    if (adv_data.include_tx_power) {
        fields.tx_pwr_lvl = adv_data.tx_power;
        fields.tx_pwr_lvl_is_present = 1;
    }

    if (adv_data.appearance > 0) {
        fields.appearance = adv_data.appearance;
        fields.appearance_is_present = 1;
    }

    if (!adv_data.manufacturer_data.empty()) {
        fields.mfg_data = adv_data.manufacturer_data.data();
        fields.mfg_data_len = adv_data.manufacturer_data.size();
    }

    // Set advertisement data
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to set advertisement fields: %d", rc);
        return ESP_FAIL;
    }

    // Configure advertisement parameters
    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = (type == AdvertisementType::CONNECTABLE_UNDIRECTED) ? 
                          BLE_GAP_CONN_MODE_UND : BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = (interval_ms * 1000) / 625; // Convert to 0.625ms units
    adv_params.itvl_max = adv_params.itvl_min + 10;
    adv_params.channel_map = 0;
    adv_params.filter_policy = 0;
    adv_params.high_duty_cycle = 0;

    // Start advertising
    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, nullptr, BLE_HS_FOREVER,
                          &adv_params, gapEventHandler, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to start advertising: %d", rc);
        return ESP_FAIL;
    }

    advertising_.store(true);
    statistics_.advertisements_sent++;
    
    ESP_LOGI(TAG, "Advertising started successfully");
    return ESP_OK;
}

esp_err_t EspBluetooth::stopAdvertising() {
    if (!advertising_.load()) {
        ESP_LOGW(TAG, "Not currently advertising");
        return ESP_OK;
    }

    int rc = ble_gap_adv_stop();
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to stop advertising: %d", rc);
        return ESP_FAIL;
    }

    advertising_.store(false);
    ESP_LOGI(TAG, "Advertising stopped");
    return ESP_OK;
}

esp_err_t EspBluetooth::startScan(const ScanParams& params) {
    if (!initialized_.load()) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (scanning_.load()) {
        ESP_LOGW(TAG, "Already scanning, stopping first");
        stopScan();
    }

    ESP_LOGI(TAG, "Starting scan for %lu ms", params.duration_ms);

    // Configure scan parameters
    struct ble_gap_disc_params disc_params = {0};
    disc_params.itvl = params.interval;
    disc_params.window = params.window;
    disc_params.filter_policy = params.filter_policy;
    disc_params.limited = params.limited_discovery ? 1 : 0;
    disc_params.passive = params.passive ? 1 : 0;
    disc_params.filter_duplicates = 1;

    // Start scanning
    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, params.duration_ms, 
                         &disc_params, gapEventHandler, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to start scan: %d", rc);
        return ESP_FAIL;
    }

    scanning_.store(true);
    ESP_LOGI(TAG, "Scan started successfully");
    return ESP_OK;
}

esp_err_t EspBluetooth::stopScan() {
    if (!scanning_.load()) {
        ESP_LOGW(TAG, "Not currently scanning");
        return ESP_OK;
    }

    int rc = ble_gap_disc_cancel();
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to stop scan: %d", rc);
        return ESP_FAIL;
    }

    scanning_.store(false);
    ESP_LOGI(TAG, "Scan stopped");
    return ESP_OK;
}

esp_err_t EspBluetooth::connect(const std::array<uint8_t, 6>& addr, 
                               uint8_t addr_type, 
                               const ConnectionParams& params) {
    if (!initialized_.load()) {
        ESP_LOGE(TAG, "Bluetooth not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Connecting to device");

    // Convert address format
    ble_addr_t peer_addr;
    peer_addr.type = addr_type;
    std::copy(addr.begin(), addr.end(), peer_addr.val);

    // Configure connection parameters
    struct ble_gap_conn_params conn_params = {0};
    conn_params.scan_itvl = 0x0010;
    conn_params.scan_window = 0x0010;
    conn_params.itvl_min = params.interval_min;
    conn_params.itvl_max = params.interval_max;
    conn_params.latency = params.latency;
    conn_params.supervision_timeout = params.timeout;
    conn_params.min_ce_len = params.min_ce_len;
    conn_params.max_ce_len = params.max_ce_len;

    // Initiate connection
    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &peer_addr, 30000, 
                            &conn_params, gapEventHandler, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to initiate connection: %d", rc);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connection initiated");
    return ESP_OK;
}

esp_err_t EspBluetooth::disconnect(uint16_t conn_handle) {
    int rc = ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to disconnect: %d", rc);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Disconnection initiated for handle %d", conn_handle);
    return ESP_OK;
}

esp_err_t EspBluetooth::updateConnectionParams(uint16_t conn_handle, 
                                              const ConnectionParams& params) {
    struct ble_gap_upd_params upd_params = {0};
    upd_params.itvl_min = params.interval_min;
    upd_params.itvl_max = params.interval_max;
    upd_params.latency = params.latency;
    upd_params.supervision_timeout = params.timeout;
    upd_params.min_ce_len = params.min_ce_len;
    upd_params.max_ce_len = params.max_ce_len;

    int rc = ble_gap_update_params(conn_handle, &upd_params);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to update connection parameters: %d", rc);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connection parameter update initiated");
    return ESP_OK;
}

esp_err_t EspBluetooth::setTxPower(PowerLevel power) {
    int power_dbm = powerLevelToDbm(power);
    
    esp_err_t ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, power_dbm);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set TX power: %s", esp_err_to_name(ret));
        return ret;
    }

    current_power_ = power;
    ESP_LOGI(TAG, "TX power set to %d dBm", power_dbm);
    return ESP_OK;
}

esp_err_t EspBluetooth::configureSecurity(const SecurityConfig& config) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    security_config_ = config;

    // Configure security parameters
    ble_hs_cfg.sm_io_cap = config.io_capabilities;
    ble_hs_cfg.sm_bonding = config.bonding ? 1 : 0;
    ble_hs_cfg.sm_mitm = config.mitm ? 1 : 0;
    ble_hs_cfg.sm_sc = config.secure_connections ? 1 : 0;
    ble_hs_cfg.sm_our_key_dist = config.init_key_dist;
    ble_hs_cfg.sm_their_key_dist = config.resp_key_dist;

    ESP_LOGI(TAG, "Security configuration updated");
    return ESP_OK;
}

esp_err_t EspBluetooth::addGattService(std::shared_ptr<GattService> service) {
    if (!service) {
        ESP_LOGE(TAG, "Invalid GATT service");
        return ESP_ERR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(state_mutex_);
    gatt_services_.push_back(service);
    
    ESP_LOGI(TAG, "GATT service added");
    return ESP_OK;
}

esp_err_t EspBluetooth::removeGattService(const ble_uuid_t& service_uuid) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto it = std::remove_if(gatt_services_.begin(), gatt_services_.end(),
        [&service_uuid](const auto& service) {
            // Compare UUIDs (implementation specific)
            return false; // Placeholder - implement UUID comparison
        });
    
    if (it != gatt_services_.end()) {
        gatt_services_.erase(it, gatt_services_.end());
        ESP_LOGI(TAG, "GATT service removed");
        return ESP_OK;
    }
    
    ESP_LOGW(TAG, "GATT service not found");
    return ESP_ERR_NOT_FOUND;
}

std::array<uint8_t, 6> EspBluetooth::getMacAddress() const {
    std::array<uint8_t, 6> mac{};
    uint8_t own_addr_type;
    
    int rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc == 0) {
        ble_hs_id_copy_addr(own_addr_type, mac.data(), nullptr);
    }
    
    return mac;
}

esp_err_t EspBluetooth::setLowPowerMode(bool enable) {
    // Configure power management
    esp_err_t ret = ESP_OK;
    
    if (enable) {
        // Enable power saving features
        esp_ble_gap_config_local_privacy(true);
        ret = esp_pm_configure(&(esp_pm_config_esp32c6_t){
            .max_freq_mhz = 160,
            .min_freq_mhz = 10,
            .light_sleep_enable = true
        });
    } else {
        // Disable power saving
        esp_ble_gap_config_local_privacy(false);
    }

    if (ret == ESP_OK) {
        low_power_mode_.store(enable);
        ESP_LOGI(TAG, "Low power mode %s", enable ? "enabled" : "disabled");
    }

    return ret;
}

std::string EspBluetooth::getStatistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    std::ostringstream oss;
    
    oss << "Bluetooth Statistics:\n"
        << "  Connections established: " << statistics_.connections_established << "\n"
        << "  Connections dropped: " << statistics_.connections_dropped << "\n"
        << "  Advertisements sent: " << statistics_.advertisements_sent << "\n"
        << "  Scan results received: " << statistics_.scan_results_received << "\n"
        << "  GATT operations: " << statistics_.gatt_operations << "\n"
        << "  Errors: " << statistics_.errors << "\n"
        << "  Current connections: " << connection_count_.load() << "\n"
        << "  Mode: " << static_cast<int>(current_mode_) << "\n"
        << "  Power level: " << static_cast<int>(current_power_) << "\n"
        << "  Low power mode: " << (low_power_mode_.load() ? "enabled" : "disabled");
        
    return oss.str();
}

esp_err_t EspBluetooth::reset() {
    ESP_LOGW(TAG, "Performing Bluetooth stack reset");
    
    esp_err_t ret = deinitialize();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize during reset");
        return ret;
    }

    // Wait for stack to settle
    vTaskDelay(pdMS_TO_TICKS(1000));

    ret = initialize(current_mode_, device_name_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reinitialize during reset");
        return ret;
    }

    ESP_LOGI(TAG, "Bluetooth stack reset completed");
    return ESP_OK;
}

// Private methods implementation

esp_err_t EspBluetooth::initializeNVS() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t EspBluetooth::initializeNimBLE() {
    // Initialize NimBLE stack
    nimble_port_init();
    
    // Initialize HCI transport
    esp_err_t ret = esp_nimble_hci_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HCI transport: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure NimBLE host
    ble_hs_cfg.reset_cb = [](int reason) {
        ESP_LOGE(TAG, "NimBLE host reset, reason: %d", reason);
    };

    ble_hs_cfg.sync_cb = []() {
        ESP_LOGI(TAG, "NimBLE host synchronized");
        
        // Generate and set random address
        int rc = ble_hs_util_ensure_addr(0);
        if (rc != 0) {
            ESP_LOGE(TAG, "Failed to generate address: %d", rc);
        }
        
        // Start advertising if in peripheral mode
        if (g_instance && g_instance->current_mode_ == Mode::PERIPHERAL) {
            AdvertisementData adv_data;
            adv_data.local_name = g_instance->device_name_;
            g_instance->startAdvertising(adv_data);
        }
    };

    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    return ESP_OK;
}

esp_err_t EspBluetooth::configureGAP() {
    // Set device name
    int rc = ble_svc_gap_device_name_set(device_name_.c_str());
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to set device name: %d", rc);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t EspBluetooth::configureGATT() {
    // Initialize GATT services
    int rc = ble_gatts_count_cfg(default_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to configure GATT services: %d", rc);
        return ESP_FAIL;
    }

    rc = ble_gatts_add_svcs(default_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to add GATT services: %d", rc);
        return ESP_FAIL;
    }

    return ESP_OK;
}

void EspBluetooth::nimbleHostTask(void* param) {
    ESP_LOGI(TAG, "NimBLE host task started");
    nimble_port_run();
    ESP_LOGI(TAG, "NimBLE host task finished");
    nimble_port_freertos_deinit();
}

int EspBluetooth::gapEventHandler(struct ble_gap_event* event, void* arg) {
    if (!g_instance) {
        return 0;
    }

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            g_instance->handleConnectionEvent(event);
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            g_instance->handleDisconnectionEvent(event);
            break;

        case BLE_GAP_EVENT_DISC:
            g_instance->handleAdvertisementEvent(event);
            break;

        case BLE_GAP_EVENT_DISC_COMPLETE:
            g_instance->handleScanCompleteEvent(event);
            break;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "Advertising complete");
            g_instance->advertising_.store(false);
            break;

        case BLE_GAP_EVENT_CONN_UPDATE:
            ESP_LOGI(TAG, "Connection parameters updated");
            break;

        default:
            ESP_LOGD(TAG, "Unhandled GAP event: %d", event->type);
            break;
    }

    return 0;
}

int EspBluetooth::gattAccessCallback(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt* ctxt, void* arg) {
    // Handle GATT access operations
    ESP_LOGD(TAG, "GATT access: conn_handle=%d, attr_handle=%d, op=%d", 
             conn_handle, attr_handle, ctxt->op);
    
    if (g_instance) {
        std::lock_guard<std::mutex> lock(g_instance->stats_mutex_);
        g_instance->statistics_.gatt_operations++;
    }

    return 0;
}

void EspBluetooth::handleConnectionEvent(const ble_gap_event* event) {
    if (event->connect.status == 0) {
        ESP_LOGI(TAG, "Connection established, handle: %d", event->connect.conn_handle);
        
        connection_count_.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            statistics_.connections_established++;
        }
        
        if (connection_callback_) {
            ble_gap_conn_desc desc;
            if (ble_gap_conn_find(event->connect.conn_handle, &desc) == 0) {
                connection_callback_(event->connect.conn_handle, desc);
            }
        }
    } else {
        ESP_LOGE(TAG, "Connection failed, status: %d", event->connect.status);
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            statistics_.errors++;
        }
    }
}

void EspBluetooth::handleDisconnectionEvent(const ble_gap_event* event) {
    ESP_LOGI(TAG, "Disconnected, handle: %d, reason: %d", 
             event->disconnect.conn.conn_handle, event->disconnect.reason);
    
    if (connection_count_.load() > 0) {
        connection_count_.fetch_sub(1);
    }
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        statistics_.connections_dropped++;
    }
    
    if (disconnection_callback_) {
        disconnection_callback_(event->disconnect.conn.conn_handle, event->disconnect.reason);
    }
    
    // Restart advertising if in peripheral mode
    if (current_mode_ == Mode::PERIPHERAL && !advertising_.load()) {
        AdvertisementData adv_data;
        adv_data.local_name = device_name_;
        startAdvertising(adv_data);
    }
}

void EspBluetooth::handleAdvertisementEvent(const ble_gap_event* event) {
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        statistics_.scan_results_received++;
    }
    
    if (advertisement_callback_) {
        advertisement_callback_(event->disc);
    }
}

void EspBluetooth::handleScanCompleteEvent(const ble_gap_event* event) {
    ESP_LOGI(TAG, "Scan complete, reason: %d", event->disc_complete.reason);
    scanning_.store(false);
    
    if (scan_complete_callback_) {
        scan_complete_callback_(event->disc_complete.reason);
    }
}

int EspBluetooth::powerLevelToDbm(PowerLevel power) {
    return static_cast<int>(power);
}

uint8_t EspBluetooth::advertisementTypeToNimBLE(AdvertisementType type) {
    switch (type) {
        case AdvertisementType::CONNECTABLE_UNDIRECTED:
            return BLE_GAP_CONN_MODE_UND;
        case AdvertisementType::NON_CONNECTABLE_UNDIRECTED:
            return BLE_GAP_CONN_MODE_NON;
        default:
            return BLE_GAP_CONN_MODE_UND;
    }
}

} // namespace bluetooth
} // namespace esp32
