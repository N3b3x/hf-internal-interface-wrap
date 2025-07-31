/**
 * @file EspBluetooth.h
 * @brief ESP32-C6 Bluetooth 5.0 LE implementation using NimBLE host stack for ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This implementation supports:
 * - Bluetooth 5.0 LE (ESP32-C6 certified for Bluetooth LE 5.3)
 * - NimBLE host stack (lightweight, efficient)
 * - ESP-IDF v5.5 APIs
 * - Modern C++17 features
 * - Thread-safe operations
 * - Power management
 * - Extended advertising and scanning
 * - Connection parameter optimization
 */

#ifndef ESP_BLUETOOTH_H
#define ESP_BLUETOOTH_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <array>

// ESP-IDF v5.5 includes
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// NimBLE host stack includes for ESP-IDF v5.5
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_store.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// ESP controller includes
#include "esp_nimble_hci.h"
#include "esp_bt.h"

// Local includes
#include "utils/EspTypes_Bluetooth.h"

namespace esp32 {
namespace bluetooth {

/**
 * @brief Forward declarations
 */
class BluetoothDevice;
class GattService;
class GattCharacteristic;

/**
 * @brief Bluetooth 5.0 LE stack implementation using NimBLE for ESP32-C6
 * 
 * This class provides a comprehensive interface for Bluetooth Low Energy operations
 * on ESP32-C6 with ESP-IDF v5.5, featuring:
 * - Modern C++17 design patterns
 * - Thread-safe operations
 * - Power-efficient implementation
 * - Extended advertising support
 * - Connection management
 * - GATT server/client functionality
 */
class EspBluetooth {
public:
    /**
     * @brief Bluetooth operation modes
     */
    enum class Mode {
        DISABLED = 0,       ///< Bluetooth disabled
        PERIPHERAL,         ///< BLE Peripheral (GATT Server)
        CENTRAL,           ///< BLE Central (GATT Client)
        OBSERVER,          ///< BLE Observer (Scanner only)
        BROADCASTER        ///< BLE Broadcaster (Advertiser only)
    };

    /**
     * @brief Bluetooth power levels for transmission
     */
    enum class PowerLevel {
        MIN = -12,         ///< Minimum power (-12 dBm)
        LOW = -9,          ///< Low power (-9 dBm)
        MEDIUM = -6,       ///< Medium power (-6 dBm)
        HIGH = -3,         ///< High power (-3 dBm)
        MAX = 9            ///< Maximum power (9 dBm)
    };

    /**
     * @brief Advertisement types for Bluetooth 5.0
     */
    enum class AdvertisementType {
        CONNECTABLE_UNDIRECTED = 0,    ///< Connectable undirected
        CONNECTABLE_DIRECTED,          ///< Connectable directed
        SCANNABLE_UNDIRECTED,          ///< Scannable undirected
        NON_CONNECTABLE_UNDIRECTED,    ///< Non-connectable undirected
        SCAN_RESPONSE,                 ///< Scan response
        EXTENDED_CONNECTABLE,          ///< Extended connectable (BT 5.0+)
        EXTENDED_SCANNABLE,            ///< Extended scannable (BT 5.0+)
        EXTENDED_NON_CONNECTABLE       ///< Extended non-connectable (BT 5.0+)
    };

    /**
     * @brief Connection parameters for optimized performance
     */
    struct ConnectionParams {
        uint16_t interval_min{24};        ///< Minimum connection interval (1.25ms units)
        uint16_t interval_max{40};        ///< Maximum connection interval (1.25ms units)
        uint16_t latency{0};              ///< Peripheral latency
        uint16_t timeout{400};            ///< Supervision timeout (10ms units)
        uint16_t min_ce_len{0};           ///< Minimum connection event length
        uint16_t max_ce_len{0};           ///< Maximum connection event length
    };

    /**
     * @brief Advertisement data structure
     */
    struct AdvertisementData {
        std::string local_name;           ///< Device local name
        std::vector<uint8_t> manufacturer_data; ///< Manufacturer specific data
        std::vector<ble_uuid_t> service_uuids;  ///< Service UUIDs
        int8_t tx_power{0};               ///< TX power level
        uint16_t appearance{0};           ///< Device appearance
        bool include_name{true};          ///< Include device name in advertisement
        bool include_tx_power{false};     ///< Include TX power in advertisement
    };

    /**
     * @brief Scan parameters for discovery
     */
    struct ScanParams {
        uint16_t interval{0x0010};        ///< Scan interval (0.625ms units)
        uint16_t window{0x0010};          ///< Scan window (0.625ms units)
        uint8_t filter_policy{0};         ///< Scan filter policy
        bool limited_discovery{false};    ///< Limited discoverable mode
        bool passive{false};              ///< Passive scanning
        uint32_t duration_ms{30000};     ///< Scan duration in milliseconds
    };

    /**
     * @brief Security configuration
     */
    struct SecurityConfig {
        bool bonding{false};              ///< Enable bonding
        bool mitm{false};                 ///< Man-in-the-Middle protection
        bool secure_connections{true};    ///< LE Secure Connections
        uint8_t io_capabilities{BLE_HS_IO_NO_INPUT_OUTPUT}; ///< I/O capabilities
        uint8_t oob_flag{0};              ///< Out-of-Band authentication
        uint8_t max_key_size{16};         ///< Maximum encryption key size
        uint8_t init_key_dist{0};         ///< Initiator key distribution
        uint8_t resp_key_dist{0};         ///< Responder key distribution
    };

    /**
     * @brief Connection event callback types
     */
    using ConnectionCallback = std::function<void(uint16_t conn_handle, const ble_gap_conn_desc& desc)>;
    using DisconnectionCallback = std::function<void(uint16_t conn_handle, int reason)>;
    using AdvertisementCallback = std::function<void(const ble_gap_disc_desc& desc)>;
    using ScanCompleteCallback = std::function<void(int reason)>;
    using GattEventCallback = std::function<int(uint16_t conn_handle, uint16_t attr_handle, 
                                              struct ble_gatt_access_ctxt* ctxt)>;

    /**
     * @brief Singleton instance getter
     * @return Reference to the singleton instance
     */
    static EspBluetooth& getInstance();

    /**
     * @brief Initialize Bluetooth subsystem
     * @param mode Operating mode
     * @param device_name Device name for advertising
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initialize(Mode mode, const std::string& device_name = "ESP32-C6");

    /**
     * @brief Deinitialize Bluetooth subsystem
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitialize();

    /**
     * @brief Check if Bluetooth is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized_.load(); }

    /**
     * @brief Get current operating mode
     * @return Current mode
     */
    Mode getMode() const { return current_mode_; }

    /**
     * @brief Start advertising with specified parameters
     * @param adv_data Advertisement data
     * @param type Advertisement type
     * @param interval_ms Advertisement interval in milliseconds
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t startAdvertising(const AdvertisementData& adv_data,
                              AdvertisementType type = AdvertisementType::CONNECTABLE_UNDIRECTED,
                              uint32_t interval_ms = 100);

    /**
     * @brief Stop advertising
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stopAdvertising();

    /**
     * @brief Check if currently advertising
     * @return true if advertising, false otherwise
     */
    bool isAdvertising() const { return advertising_.load(); }

    /**
     * @brief Start scanning for devices
     * @param params Scan parameters
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t startScan(const ScanParams& params = ScanParams{});

    /**
     * @brief Stop scanning
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t stopScan();

    /**
     * @brief Check if currently scanning
     * @return true if scanning, false otherwise
     */
    bool isScanning() const { return scanning_.load(); }

    /**
     * @brief Connect to a remote device
     * @param addr Remote device address
     * @param addr_type Address type (public/random)
     * @param params Connection parameters
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t connect(const std::array<uint8_t, 6>& addr, 
                     uint8_t addr_type = BLE_ADDR_PUBLIC,
                     const ConnectionParams& params = ConnectionParams{});

    /**
     * @brief Disconnect from a device
     * @param conn_handle Connection handle
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t disconnect(uint16_t conn_handle);

    /**
     * @brief Update connection parameters
     * @param conn_handle Connection handle
     * @param params New connection parameters
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t updateConnectionParams(uint16_t conn_handle, const ConnectionParams& params);

    /**
     * @brief Get number of active connections
     * @return Number of active connections
     */
    size_t getConnectionCount() const { return connection_count_.load(); }

    /**
     * @brief Set TX power level
     * @param power Power level
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setTxPower(PowerLevel power);

    /**
     * @brief Get current TX power level
     * @return Current power level
     */
    PowerLevel getTxPower() const { return current_power_; }

    /**
     * @brief Configure security settings
     * @param config Security configuration
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureSecurity(const SecurityConfig& config);

    /**
     * @brief Add a GATT service
     * @param service Shared pointer to GATT service
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t addGattService(std::shared_ptr<GattService> service);

    /**
     * @brief Remove a GATT service
     * @param service_uuid Service UUID to remove
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t removeGattService(const ble_uuid_t& service_uuid);

    /**
     * @brief Register connection event callback
     * @param callback Callback function
     */
    void onConnection(ConnectionCallback callback) { 
        std::lock_guard<std::mutex> lock(callback_mutex_);
        connection_callback_ = std::move(callback); 
    }

    /**
     * @brief Register disconnection event callback
     * @param callback Callback function
     */
    void onDisconnection(DisconnectionCallback callback) { 
        std::lock_guard<std::mutex> lock(callback_mutex_);
        disconnection_callback_ = std::move(callback); 
    }

    /**
     * @brief Register advertisement received callback
     * @param callback Callback function
     */
    void onAdvertisementReceived(AdvertisementCallback callback) { 
        std::lock_guard<std::mutex> lock(callback_mutex_);
        advertisement_callback_ = std::move(callback); 
    }

    /**
     * @brief Register scan complete callback
     * @param callback Callback function
     */
    void onScanComplete(ScanCompleteCallback callback) { 
        std::lock_guard<std::mutex> lock(callback_mutex_);
        scan_complete_callback_ = std::move(callback); 
    }

    /**
     * @brief Get device MAC address
     * @return MAC address as array
     */
    std::array<uint8_t, 6> getMacAddress() const;

    /**
     * @brief Enable/disable low power mode
     * @param enable Enable low power mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setLowPowerMode(bool enable);

    /**
     * @brief Check if low power mode is enabled
     * @return true if enabled, false otherwise
     */
    bool isLowPowerMode() const { return low_power_mode_.load(); }

    /**
     * @brief Get Bluetooth stack statistics
     * @return String containing statistics
     */
    std::string getStatistics() const;

    /**
     * @brief Reset Bluetooth stack (emergency recovery)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t reset();

private:
    /**
     * @brief Private constructor for singleton
     */
    EspBluetooth() = default;

    /**
     * @brief Destructor
     */
    ~EspBluetooth();

    /**
     * @brief Delete copy constructor and assignment operator
     */
    EspBluetooth(const EspBluetooth&) = delete;
    EspBluetooth& operator=(const EspBluetooth&) = delete;

    /**
     * @brief Initialize NVS flash
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeNVS();

    /**
     * @brief Initialize NimBLE stack
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeNimBLE();

    /**
     * @brief Configure GAP settings
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureGAP();

    /**
     * @brief Configure GATT settings
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t configureGATT();

    /**
     * @brief NimBLE host task entry point
     */
    static void nimbleHostTask(void* param);

    /**
     * @brief GAP event handler
     */
    static int gapEventHandler(struct ble_gap_event* event, void* arg);

    /**
     * @brief GATT access callback
     */
    static int gattAccessCallback(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt* ctxt, void* arg);

    /**
     * @brief Handle connection event
     */
    void handleConnectionEvent(const ble_gap_event* event);

    /**
     * @brief Handle disconnection event
     */
    void handleDisconnectionEvent(const ble_gap_event* event);

    /**
     * @brief Handle advertisement received event
     */
    void handleAdvertisementEvent(const ble_gap_event* event);

    /**
     * @brief Handle scan complete event
     */
    void handleScanCompleteEvent(const ble_gap_event* event);

    /**
     * @brief Convert power level enum to dBm value
     */
    static int powerLevelToDbm(PowerLevel power);

    /**
     * @brief Convert advertisement type to NimBLE type
     */
    static uint8_t advertisementTypeToNimBLE(AdvertisementType type);

    // State variables
    std::atomic<bool> initialized_{false};
    std::atomic<bool> advertising_{false};
    std::atomic<bool> scanning_{false};
    std::atomic<bool> low_power_mode_{false};
    std::atomic<size_t> connection_count_{0};
    Mode current_mode_{Mode::DISABLED};
    PowerLevel current_power_{PowerLevel::MEDIUM};
    std::string device_name_;

    // Synchronization
    mutable std::mutex state_mutex_;
    mutable std::mutex callback_mutex_;
    SemaphoreHandle_t nimble_semaphore_{nullptr};

    // Callbacks
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    AdvertisementCallback advertisement_callback_;
    ScanCompleteCallback scan_complete_callback_;

    // GATT services
    std::vector<std::shared_ptr<GattService>> gatt_services_;

    // Configuration
    SecurityConfig security_config_;
    ConnectionParams default_conn_params_;

    // Task handle
    TaskHandle_t nimble_task_handle_{nullptr};

    // Statistics
    mutable std::mutex stats_mutex_;
    struct {
        uint32_t connections_established{0};
        uint32_t connections_dropped{0};
        uint32_t advertisements_sent{0};
        uint32_t scan_results_received{0};
        uint32_t gatt_operations{0};
        uint32_t errors{0};
    } statistics_;

    // Logging tag
    static constexpr const char* TAG = "EspBluetooth";
};

} // namespace bluetooth
} // namespace esp32

#endif // ESP_BLUETOOTH_H