/**
 * @file EspTypes_Bluetooth.h
 * @brief Type definitions for ESP32-C6 Bluetooth 5.0 LE implementation using ESP-IDF v5.5
 * @version 2.0.0
 * @date 2024
 * 
 * This file provides comprehensive type definitions for ESP32-C6 Bluetooth Low Energy
 * implementation using NimBLE host stack with ESP-IDF v5.5. It includes modern C++17
 * features and supports Bluetooth 5.0 LE certified features.
 */

#ifndef ESP_TYPES_BLUETOOTH_H
#define ESP_TYPES_BLUETOOTH_H

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <memory>
#include <optional>

// ESP-IDF v5.5 includes for NimBLE
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

// NimBLE specific includes for ESP-IDF v5.5
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_att.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_store.h"
#include "host/ble_sm.h"

namespace esp32 {
namespace bluetooth {

/**
 * @brief Bluetooth address type (6 bytes)
 */
using BluetoothAddress = std::array<uint8_t, 6>;

/**
 * @brief UUID types for Bluetooth services and characteristics
 */
class BluetoothUUID {
public:
    enum class Type {
        UUID16,
        UUID32,
        UUID128
    };
    
    BluetoothUUID() = default;
    explicit BluetoothUUID(uint16_t uuid16);
    explicit BluetoothUUID(uint32_t uuid32);
    explicit BluetoothUUID(const std::array<uint8_t, 16>& uuid128);
    explicit BluetoothUUID(const std::string& uuid_string);
    
    Type getType() const { return type_; }
    uint16_t getUUID16() const { return uuid16_; }
    uint32_t getUUID32() const { return uuid32_; }
    const std::array<uint8_t, 16>& getUUID128() const { return uuid128_; }
    
    std::string toString() const;
    bool operator==(const BluetoothUUID& other) const;
    bool operator!=(const BluetoothUUID& other) const;
    
    // Convert to NimBLE UUID format
    ble_uuid_any_t toNimbleUUID() const;
    
private:
    Type type_ = Type::UUID16;
    uint16_t uuid16_ = 0;
    uint32_t uuid32_ = 0;
    std::array<uint8_t, 16> uuid128_ = {};
};

/**
 * @brief Bluetooth Low Energy address types
 */
enum class BLEAddressType : uint8_t {
    PUBLIC = BLE_ADDR_PUBLIC,
    RANDOM_STATIC = BLE_ADDR_RANDOM,
    RANDOM_PRIVATE_RESOLVABLE = BLE_ADDR_RANDOM,
    RANDOM_PRIVATE_NON_RESOLVABLE = BLE_ADDR_RANDOM
};

/**
 * @brief Bluetooth Low Energy device power levels (ESP32-C6 specific)
 */
enum class BLEPowerLevel : int8_t {
    POWER_N12_DBM = -12,    // -12 dBm
    POWER_N9_DBM = -9,      // -9 dBm
    POWER_N6_DBM = -6,      // -6 dBm
    POWER_N3_DBM = -3,      // -3 dBm
    POWER_0_DBM = 0,        // 0 dBm
    POWER_3_DBM = 3,        // 3 dBm
    POWER_6_DBM = 6,        // 6 dBm
    POWER_9_DBM = 9,        // 9 dBm (maximum for ESP32-C6)
};

/**
 * @brief Bluetooth Low Energy PHY types (Bluetooth 5.0 LE features)
 */
enum class BLEPHYType : uint8_t {
    PHY_1M = BLE_GAP_LE_PHY_1M,          // 1M PHY
    PHY_2M = BLE_GAP_LE_PHY_2M,          // 2M PHY (Bluetooth 5.0)
    PHY_CODED = BLE_GAP_LE_PHY_CODED     // Coded PHY (Bluetooth 5.0, long range)
};

/**
 * @brief Bluetooth Low Energy security levels
 */
enum class BLESecurityLevel : uint8_t {
    NONE = 0,              // No security
    UNAUTHENTICATED = 1,   // Encryption without authentication
    AUTHENTICATED = 2,     // Encryption with authentication
    SECURE_CONNECTIONS = 3  // LE Secure Connections (Bluetooth 4.2+)
};

/**
 * @brief Bluetooth Low Energy I/O capabilities
 */
enum class BLEIOCapability : uint8_t {
    DISPLAY_ONLY = BLE_SM_IO_CAP_DISP_ONLY,
    DISPLAY_YES_NO = BLE_SM_IO_CAP_DISP_YES_NO,
    KEYBOARD_ONLY = BLE_SM_IO_CAP_KEYBOARD_ONLY,
    NO_INPUT_OUTPUT = BLE_SM_IO_CAP_NO_IO,
    KEYBOARD_DISPLAY = BLE_SM_IO_CAP_KEYBOARD_DISP
};

/**
 * @brief Bluetooth Low Energy bonding flags
 */
enum class BLEBondingFlag : uint8_t {
    NO_BONDING = BLE_SM_PAIR_AUTHREQ_BOND_NO,
    BONDING = BLE_SM_PAIR_AUTHREQ_BOND
};

/**
 * @brief GATT characteristic properties
 */
enum class GATTCharacteristicProperty : uint16_t {
    BROADCAST = BLE_GATT_CHR_F_BROADCAST,
    READ = BLE_GATT_CHR_F_READ,
    WRITE_WITHOUT_RESPONSE = BLE_GATT_CHR_F_WRITE_NO_RSP,
    WRITE = BLE_GATT_CHR_F_WRITE,
    NOTIFY = BLE_GATT_CHR_F_NOTIFY,
    INDICATE = BLE_GATT_CHR_F_INDICATE,
    AUTHENTICATED_SIGNED_WRITES = BLE_GATT_CHR_F_AUTH_SIGN_WRITE,
    EXTENDED_PROPERTIES = BLE_GATT_CHR_F_RELIABLE_WRITE
};

/**
 * @brief GATT descriptor types
 */
enum class GATTDescriptorType : uint16_t {
    CHARACTERISTIC_EXTENDED_PROPERTIES = 0x2900,
    CHARACTERISTIC_USER_DESCRIPTION = 0x2901,
    CLIENT_CHARACTERISTIC_CONFIGURATION = 0x2902,
    SERVER_CHARACTERISTIC_CONFIGURATION = 0x2903,
    CHARACTERISTIC_PRESENTATION_FORMAT = 0x2904,
    CHARACTERISTIC_AGGREGATE_FORMAT = 0x2905
};

/**
 * @brief Advertising types for BLE
 */
enum class BLEAdvertisingType : uint8_t {
    CONNECTABLE_UNDIRECTED = BLE_GAP_CONN_MODE_UND,
    CONNECTABLE_DIRECTED_HIGH_DUTY = BLE_GAP_CONN_MODE_DIR,
    SCANNABLE_UNDIRECTED = BLE_GAP_CONN_MODE_NON,
    NON_CONNECTABLE_UNDIRECTED = BLE_GAP_CONN_MODE_NON,
    CONNECTABLE_DIRECTED_LOW_DUTY = BLE_GAP_CONN_MODE_DIR
};

/**
 * @brief Extended advertising types (Bluetooth 5.0 LE)
 */
enum class BLEExtendedAdvertisingType : uint8_t {
    LEGACY_CONNECTABLE_SCANNABLE = 0,
    LEGACY_CONNECTABLE_DIRECTED = 1,
    LEGACY_SCANNABLE = 2,
    LEGACY_NON_CONNECTABLE = 3,
    EXTENDED_CONNECTABLE = 4,
    EXTENDED_SCANNABLE = 5,
    EXTENDED_NON_CONNECTABLE = 6
};

/**
 * @brief Scan types for BLE
 */
enum class BLEScanType : uint8_t {
    PASSIVE = BLE_GAP_DISC_MODE_PASSIVE,
    ACTIVE = BLE_GAP_DISC_MODE_GEN,
    LIMITED = BLE_GAP_DISC_MODE_LTD
};

/**
 * @brief Connection parameters for BLE
 */
struct BLEConnectionParams {
    uint16_t interval_min;          // Connection interval minimum (units of 1.25ms)
    uint16_t interval_max;          // Connection interval maximum (units of 1.25ms)
    uint16_t latency;               // Slave latency
    uint16_t supervision_timeout;   // Supervision timeout (units of 10ms)
    uint16_t min_connection_event_length;  // Minimum connection event length
    uint16_t max_connection_event_length;  // Maximum connection event length
};

/**
 * @brief Advertising parameters for BLE
 */
struct BLEAdvertisingParams {
    BLEAdvertisingType type = BLEAdvertisingType::CONNECTABLE_UNDIRECTED;
    uint16_t interval_min = 0x0020;    // 20ms minimum
    uint16_t interval_max = 0x0040;    // 40ms maximum
    BLEAddressType own_addr_type = BLEAddressType::PUBLIC;
    BLEAddressType peer_addr_type = BLEAddressType::PUBLIC;
    BluetoothAddress peer_addr = {};
    uint8_t channel_map = 0x07;        // All channels
    uint8_t filter_policy = BLE_GAP_ADV_FILTER_TRANS_CONN;
    int8_t tx_power = 0;               // TX power in dBm
    
    // Bluetooth 5.0 LE extended advertising parameters
    bool use_extended_advertising = false;
    BLEExtendedAdvertisingType extended_type = BLEExtendedAdvertisingType::LEGACY_CONNECTABLE_SCANNABLE;
    uint8_t primary_phy = static_cast<uint8_t>(BLEPHYType::PHY_1M);
    uint8_t secondary_phy = static_cast<uint8_t>(BLEPHYType::PHY_1M);
    uint16_t max_events = 0;           // 0 = no limit
    uint16_t duration = 0;             // 0 = no limit (units of 10ms)
};

/**
 * @brief Scan parameters for BLE
 */
struct BLEScanParams {
    BLEScanType type = BLEScanType::ACTIVE;
    uint16_t interval = 0x0010;        // 10ms
    uint16_t window = 0x0010;          // 10ms
    BLEAddressType own_addr_type = BLEAddressType::PUBLIC;
    uint8_t filter_policy = BLE_GAP_DISC_FILTER_NONE;
    uint8_t filter_duplicates = 1;
    uint16_t duration = 0;             // 0 = scan indefinitely
    uint16_t period = 0;               // 0 = scan continuously
    
    // Bluetooth 5.0 LE extended scanning parameters
    bool use_extended_scanning = false;
    uint8_t primary_phy = static_cast<uint8_t>(BLEPHYType::PHY_1M);
    uint8_t secondary_phy = static_cast<uint8_t>(BLEPHYType::PHY_1M);
};

/**
 * @brief Security parameters for BLE
 */
struct BLESecurityParams {
    BLESecurityLevel security_level = BLESecurityLevel::UNAUTHENTICATED;
    BLEIOCapability io_capability = BLEIOCapability::NO_INPUT_OUTPUT;
    BLEBondingFlag bonding = BLEBondingFlag::BONDING;
    bool mitm_protection = false;
    bool secure_connections = true;    // LE Secure Connections (recommended)
    bool keypress_notifications = false;
    uint8_t key_size = 16;             // Maximum key size
    uint8_t init_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    uint8_t resp_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    uint32_t passkey = 0;              // For passkey entry
};

/**
 * @brief Scanned device information
 */
struct BLEScannedDevice {
    BluetoothAddress address;
    BLEAddressType address_type;
    int8_t rssi;
    std::vector<uint8_t> advertising_data;
    std::vector<uint8_t> scan_response_data;
    std::chrono::steady_clock::time_point timestamp;
    
    // Bluetooth 5.0 LE extended advertising data
    bool is_extended_advertising = false;
    BLEPHYType primary_phy = BLEPHYType::PHY_1M;
    BLEPHYType secondary_phy = BLEPHYType::PHY_1M;
    uint16_t periodic_advertising_interval = 0;
    
    // Convenience methods
    std::string getAddressString() const;
    std::string getName() const;
    std::vector<BluetoothUUID> getServiceUUIDs() const;
    std::optional<std::vector<uint8_t>> getManufacturerData() const;
    int8_t getTxPower() const;
    bool hasService(const BluetoothUUID& uuid) const;
};

/**
 * @brief Connected device information
 */
struct BLEConnectedDevice {
    uint16_t connection_handle;
    BluetoothAddress address;
    BLEAddressType address_type;
    BLEConnectionParams connection_params;
    int8_t rssi;
    BLEPHYType tx_phy = BLEPHYType::PHY_1M;
    BLEPHYType rx_phy = BLEPHYType::PHY_1M;
    uint16_t mtu = 23;                 // Default ATT MTU
    std::chrono::steady_clock::time_point connection_time;
    
    std::string getAddressString() const;
};

/**
 * @brief GATT service definition
 */
struct GATTService {
    BluetoothUUID uuid;
    uint16_t handle;
    bool is_primary;
    std::vector<uint16_t> characteristic_handles;
    
    // For service definition
    struct ble_gatt_svc_def* nimble_service_def = nullptr;
};

/**
 * @brief GATT characteristic definition
 */
struct GATTCharacteristic {
    BluetoothUUID uuid;
    uint16_t handle;
    uint16_t value_handle;
    uint16_t properties;
    std::vector<uint8_t> value;
    std::vector<uint16_t> descriptor_handles;
    
    // Access callbacks
    std::function<int(uint16_t, uint16_t, struct ble_gatt_access_ctxt*, void*)> access_callback;
    
    // For characteristic definition
    struct ble_gatt_chr_def* nimble_char_def = nullptr;
};

/**
 * @brief GATT descriptor definition
 */
struct GATTDescriptor {
    BluetoothUUID uuid;
    uint16_t handle;
    std::vector<uint8_t> value;
    uint16_t permissions;
    
    // Access callbacks
    std::function<int(uint16_t, uint16_t, struct ble_gatt_access_ctxt*, void*)> access_callback;
    
    // For descriptor definition
    struct ble_gatt_dsc_def* nimble_desc_def = nullptr;
};

/**
 * @brief Advertisement data builder
 */
class BLEAdvertisementData {
public:
    BLEAdvertisementData() = default;
    
    // Standard advertisement data methods
    void setName(const std::string& name);
    void setCompleteServiceUUIDs(const std::vector<BluetoothUUID>& uuids);
    void setIncompleteServiceUUIDs(const std::vector<BluetoothUUID>& uuids);
    void setManufacturerData(uint16_t company_id, const std::vector<uint8_t>& data);
    void setServiceData(const BluetoothUUID& service_uuid, const std::vector<uint8_t>& data);
    void setTxPowerLevel(int8_t power);
    void setAppearance(uint16_t appearance);
    void setFlags(uint8_t flags);
    
    // Custom data
    void addCustomData(uint8_t type, const std::vector<uint8_t>& data);
    
    // Build final advertisement data
    std::vector<uint8_t> build() const;
    size_t getSize() const;
    
    // Clear all data
    void clear();
    
private:
    std::vector<std::pair<uint8_t, std::vector<uint8_t>>> data_elements_;
};

/**
 * @brief Bluetooth event types
 */
enum class BluetoothEventType {
    ADAPTER_STATE_CHANGED,
    DEVICE_DISCOVERED,
    DEVICE_CONNECTED,
    DEVICE_DISCONNECTED,
    PAIRING_STARTED,
    PAIRING_COMPLETED,
    PAIRING_FAILED,
    BONDING_COMPLETED,
    SERVICE_DISCOVERED,
    CHARACTERISTIC_READ,
    CHARACTERISTIC_WRITTEN,
    CHARACTERISTIC_NOTIFICATION,
    DESCRIPTOR_READ,
    DESCRIPTOR_WRITTEN,
    MTU_CHANGED,
    PHY_CHANGED,
    CONNECTION_PARAMS_UPDATED,
    ADVERTISING_STARTED,
    ADVERTISING_STOPPED,
    SCAN_STARTED,
    SCAN_STOPPED,
    SCAN_RESULT
};

/**
 * @brief Bluetooth event data
 */
struct BluetoothEvent {
    BluetoothEventType type;
    uint16_t connection_handle = 0;
    
    union {
        struct {
            bool enabled;
        } adapter_state_changed;
        
        struct {
            BLEScannedDevice device;
        } device_discovered;
        
        struct {
            BLEConnectedDevice device;
        } device_connected;
        
        struct {
            uint16_t connection_handle;
            uint8_t reason;
        } device_disconnected;
        
        struct {
            uint16_t connection_handle;
            BluetoothUUID service_uuid;
            BluetoothUUID characteristic_uuid;
            std::vector<uint8_t> data;
        } characteristic_event;
        
        struct {
            uint16_t connection_handle;
            uint16_t mtu;
        } mtu_changed;
        
        struct {
            uint16_t connection_handle;
            BLEPHYType tx_phy;
            BLEPHYType rx_phy;
        } phy_changed;
        
        struct {
            uint16_t connection_handle;
            BLEConnectionParams params;
        } connection_params_updated;
    };
};

/**
 * @brief Callback function types
 */
using BluetoothEventCallback = std::function<void(const BluetoothEvent& event)>;
using CharacteristicAccessCallback = std::function<int(uint16_t conn_handle, uint16_t attr_handle, 
                                                      struct ble_gatt_access_ctxt* ctxt, void* arg)>;
using GAPEventCallback = std::function<int(struct ble_gap_event* event, void* arg)>;

/**
 * @brief Bluetooth initialization configuration
 */
struct BluetoothConfig {
    std::string device_name = "ESP32-C6-BLE";
    BLESecurityParams security;
    BLEPowerLevel tx_power = BLEPowerLevel::POWER_0_DBM;
    bool enable_privacy = false;
    uint16_t att_mtu = 247;            // Maximum ATT MTU for ESP32-C6
    uint8_t max_connections = 4;       // Maximum concurrent connections
    uint32_t bond_storage_size = 8;    // Number of bonded devices to store
    
    // Bluetooth 5.0 LE features
    bool enable_extended_advertising = false;
    bool enable_periodic_advertising = false;
    bool enable_phy_2m = true;         // Enable 2M PHY
    bool enable_phy_coded = false;     // Enable Coded PHY (long range)
    
    // NimBLE specific configuration
    uint16_t nimble_max_attrs = 256;   // Maximum GATT attributes
    uint16_t nimble_max_services = 16; // Maximum GATT services
    uint16_t nimble_max_client_configs = 32; // Maximum client configurations
};

/**
 * @brief Standard Bluetooth service UUIDs
 */
namespace StandardUUIDs {
    // Standard services
    inline const BluetoothUUID GENERIC_ACCESS_SERVICE(0x1800);
    inline const BluetoothUUID GENERIC_ATTRIBUTE_SERVICE(0x1801);
    inline const BluetoothUUID DEVICE_INFORMATION_SERVICE(0x180A);
    inline const BluetoothUUID BATTERY_SERVICE(0x180F);
    inline const BluetoothUUID HEART_RATE_SERVICE(0x180D);
    inline const BluetoothUUID NORDIC_UART_SERVICE(BluetoothUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"));
    
    // Standard characteristics
    inline const BluetoothUUID DEVICE_NAME_CHARACTERISTIC(0x2A00);
    inline const BluetoothUUID APPEARANCE_CHARACTERISTIC(0x2A01);
    inline const BluetoothUUID PERIPHERAL_PREFERRED_CONNECTION_PARAMS(0x2A04);
    inline const BluetoothUUID SERVICE_CHANGED_CHARACTERISTIC(0x2A05);
    inline const BluetoothUUID BATTERY_LEVEL_CHARACTERISTIC(0x2A19);
    inline const BluetoothUUID MANUFACTURER_NAME_CHARACTERISTIC(0x2A29);
    inline const BluetoothUUID MODEL_NUMBER_CHARACTERISTIC(0x2A24);
    inline const BluetoothUUID SERIAL_NUMBER_CHARACTERISTIC(0x2A25);
    inline const BluetoothUUID FIRMWARE_REVISION_CHARACTERISTIC(0x2A26);
    inline const BluetoothUUID HARDWARE_REVISION_CHARACTERISTIC(0x2A27);
    inline const BluetoothUUID SOFTWARE_REVISION_CHARACTERISTIC(0x2A28);
}

/**
 * @brief Error codes for Bluetooth operations
 */
enum class BluetoothError : int {
    SUCCESS = 0,
    INVALID_PARAMETER = -1,
    NOT_INITIALIZED = -2,
    ALREADY_INITIALIZED = -3,
    OPERATION_FAILED = -4,
    CONNECTION_FAILED = -5,
    DISCONNECTION_FAILED = -6,
    SERVICE_NOT_FOUND = -7,
    CHARACTERISTIC_NOT_FOUND = -8,
    DESCRIPTOR_NOT_FOUND = -9,
    READ_FAILED = -10,
    WRITE_FAILED = -11,
    NOTIFICATION_FAILED = -12,
    INDICATION_FAILED = -13,
    ADVERTISING_FAILED = -14,
    SCAN_FAILED = -15,
    PAIRING_FAILED = -16,
    BONDING_FAILED = -17,
    SECURITY_FAILED = -18,
    TIMEOUT = -19,
    BUFFER_TOO_SMALL = -20,
    NOT_CONNECTED = -21,
    ALREADY_CONNECTED = -22,
    NOT_SUPPORTED = -23,
    RESOURCE_EXHAUSTED = -24,
    INVALID_STATE = -25
};

/**
 * @brief Convert BluetoothError to string
 */
std::string bluetoothErrorToString(BluetoothError error);

/**
 * @brief Helper functions for address conversion
 */
std::string bluetoothAddressToString(const BluetoothAddress& address);
BluetoothAddress stringToBluetoothAddress(const std::string& address_str);

/**
 * @brief Helper functions for UUID operations
 */
bool isValidUUIDString(const std::string& uuid_str);
std::string formatUUIDString(const std::string& uuid_str);

} // namespace bluetooth
} // namespace esp32

#endif // ESP_TYPES_BLUETOOTH_H