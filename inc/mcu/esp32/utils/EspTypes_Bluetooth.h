/**
 * @file EspTypes_Bluetooth.h
 * @brief ESP32 Bluetooth type definitions for hardware abstraction.
 *
 * This header defines only the essential Bluetooth-specific types and constants used by
 * the EspBluetooth implementation. It follows a clean, minimal pattern providing only
 * necessary types without redundant or duplicate definitions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseBluetooth.h" // For hf_bluetooth_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF)

#ifdef HF_MCU_FAMILY_ESP32

//==============================================================================
// ESSENTIAL BLUETOOTH TYPES (ESP32)
//==============================================================================

/**
 * @brief Bluetooth PHY types for ESP32-C6.
 * @details Physical layer options for enhanced performance.
 */
enum class hf_esp_ble_phy_t : hf_u8_t {
  HF_BLE_PHY_1M = 1,    ///< 1M PHY (standard)
  HF_BLE_PHY_2M = 2,    ///< 2M PHY (enhanced throughput)
  HF_BLE_PHY_CODED = 3  ///< Coded PHY (enhanced range)
};

/**
 * @brief Extended advertising parameters for Bluetooth 5.0+.
 * @details Configuration for extended advertising features.
 */
struct hf_esp_ble_ext_adv_params_t {
  hf_u16_t interval_min;        ///< Minimum advertising interval (0.625ms units)
  hf_u16_t interval_max;        ///< Maximum advertising interval (0.625ms units)
  hf_u8_t type;                ///< Advertisement type
  hf_u8_t own_addr_type;       ///< Own address type
  hf_u8_t peer_addr_type;      ///< Peer address type
  hf_u8_t peer_addr[6];        ///< Peer address
  hf_u8_t channel_map;         ///< Advertising channel map
  hf_u8_t filter_policy;       ///< Advertising filter policy
  hf_i8_t tx_power;           ///< TX power level
  hf_esp_ble_phy_t primary_phy; ///< Primary PHY
  hf_u8_t max_skip;            ///< Maximum skip
  hf_esp_ble_phy_t secondary_phy; ///< Secondary PHY
  hf_u8_t sid;                 ///< Set ID
  hf_bool_t scan_req_notif;    ///< Scan request notification
};

/**
 * @brief Standard advertising parameters.
 * @details Configuration for standard advertising.
 */
struct hf_esp_ble_adv_params_t {
  hf_u16_t interval_min;        ///< Minimum advertising interval (0.625ms units)
  hf_u16_t interval_max;        ///< Maximum advertising interval (0.625ms units)
  hf_u8_t type;                ///< Advertisement type
  hf_u8_t own_addr_type;       ///< Own address type
  hf_u8_t peer_addr_type;      ///< Peer address type
  hf_u8_t peer_addr[6];        ///< Peer address
  hf_u8_t channel_map;         ///< Advertising channel map
  hf_u8_t filter_policy;       ///< Advertising filter policy
};

/**
 * @brief Bluetooth statistics structure.
 * @details Comprehensive statistics for monitoring and debugging.
 */
struct hf_esp_bluetooth_stats_t {
  hf_u32_t connections_established; ///< Total connections established
  hf_u32_t connections_dropped;     ///< Total connections dropped
  hf_u32_t advertisements_sent;     ///< Total advertisements sent
  hf_u32_t scan_results_received;   ///< Total scan results received
  hf_u32_t gatt_operations;         ///< Total GATT operations
  hf_u32_t bonding_operations;      ///< Total bonding operations
  hf_u32_t errors;                  ///< Total error count
  hf_u32_t bytes_transmitted;       ///< Total bytes transmitted
  hf_u32_t bytes_received;          ///< Total bytes received
  hf_i8_t avg_rssi;                ///< Average RSSI
  hf_u32_t uptime_ms;              ///< Uptime in milliseconds
};

//==============================================================================
// GATT SERVICE AND CHARACTERISTIC DEFINITIONS
//==============================================================================

/**
 * @brief Maximum number of GATT services.
 */
constexpr hf_u8_t HF_ESP_MAX_GATT_SERVICES = 16;

/**
 * @brief Maximum number of characteristics per service.
 */
constexpr hf_u8_t HF_ESP_MAX_GATT_CHARACTERISTICS = 32;

/**
 * @brief Maximum attribute value length.
 */
constexpr hf_u16_t HF_ESP_MAX_ATTR_LEN = 512;

/**
 * @brief GATT characteristic properties.
 * @details Properties that define how a characteristic can be used.
 */
enum class hf_esp_gatt_char_prop_t : hf_u8_t {
  HF_GATT_CHAR_PROP_BROADCAST = 0x01,       ///< Broadcast
  HF_GATT_CHAR_PROP_READ = 0x02,           ///< Read
  HF_GATT_CHAR_PROP_WRITE_NR = 0x04,       ///< Write without response
  HF_GATT_CHAR_PROP_WRITE = 0x08,          ///< Write
  HF_GATT_CHAR_PROP_NOTIFY = 0x10,         ///< Notify
  HF_GATT_CHAR_PROP_INDICATE = 0x20,       ///< Indicate
  HF_GATT_CHAR_PROP_AUTH = 0x40,           ///< Authenticated signed writes
  HF_GATT_CHAR_PROP_EXTENDED = 0x80        ///< Extended properties
};

/**
 * @brief GATT permissions.
 * @details Access permissions for GATT attributes.
 */
enum class hf_esp_gatt_perm_t : hf_u8_t {
  HF_GATT_PERM_READ = 0x01,               ///< Read permission
  HF_GATT_PERM_WRITE = 0x02,              ///< Write permission
  HF_GATT_PERM_READ_ENCRYPTED = 0x04,     ///< Encrypted read permission
  HF_GATT_PERM_WRITE_ENCRYPTED = 0x08,    ///< Encrypted write permission
  HF_GATT_PERM_READ_ENCRYPTED_MITM = 0x10, ///< MITM encrypted read permission
  HF_GATT_PERM_WRITE_ENCRYPTED_MITM = 0x20 ///< MITM encrypted write permission
};

//==============================================================================
// CONNECTION AND PAIRING TYPES
//==============================================================================

/**
 * @brief Connection parameters structure.
 * @details Parameters for optimizing BLE connections.
 */
struct hf_esp_ble_conn_params_t {
  hf_u16_t interval_min;        ///< Minimum connection interval (1.25ms units)
  hf_u16_t interval_max;        ///< Maximum connection interval (1.25ms units)
  hf_u16_t latency;            ///< Peripheral latency
  hf_u16_t timeout;            ///< Supervision timeout (10ms units)
  hf_u16_t min_ce_len;         ///< Minimum connection event length
  hf_u16_t max_ce_len;         ///< Maximum connection event length
};

/**
 * @brief Security parameters structure.
 * @details Configuration for pairing and bonding security.
 */
struct hf_esp_ble_security_params_t {
  hf_bool_t bonding;           ///< Enable bonding
  hf_bool_t mitm;              ///< Man-in-the-Middle protection
  hf_bool_t secure_conn;       ///< LE Secure Connections
  hf_bool_t keypress_notif;    ///< Keypress notifications
  hf_u8_t io_cap;             ///< I/O capabilities
  hf_u8_t oob_flag;           ///< Out-of-Band data flag
  hf_u8_t max_key_size;       ///< Maximum encryption key size
  hf_u8_t init_key_dist;      ///< Initiator key distribution
  hf_u8_t resp_key_dist;      ///< Responder key distribution
};

//==============================================================================
// SCAN AND ADVERTISING TYPES
//==============================================================================

/**
 * @brief Scan parameters structure.
 * @details Configuration for BLE scanning operations.
 */
struct hf_esp_ble_scan_params_t {
  hf_u8_t scan_type;          ///< Scan type (active/passive)
  hf_u8_t own_addr_type;      ///< Own address type
  hf_u8_t scan_filter_policy; ///< Scan filter policy
  hf_u16_t scan_interval;     ///< Scan interval (0.625ms units)
  hf_u16_t scan_window;       ///< Scan window (0.625ms units)
  hf_u16_t scan_duration;     ///< Scan duration (10ms units)
  hf_u16_t scan_period;       ///< Scan period (1.28s units)
};

/**
 * @brief Device information structure.
 * @details Information about discovered or connected devices.
 */
struct hf_esp_ble_device_info_t {
  hf_u8_t addr[6];            ///< Device address
  hf_u8_t addr_type;          ///< Address type
  hf_i8_t rssi;              ///< Signal strength
  hf_u8_t adv_data_len;      ///< Advertisement data length
  hf_u8_t adv_data[31];      ///< Advertisement data
  hf_u8_t scan_rsp_len;      ///< Scan response data length
  hf_u8_t scan_rsp[31];      ///< Scan response data
  hf_u8_t event_type;        ///< Advertisement event type
  hf_bool_t connectable;     ///< Device is connectable
  hf_bool_t scannable;       ///< Device is scannable
  hf_bool_t directed;        ///< Directed advertisement
};

//==============================================================================
// ERROR HANDLING AND UTILITIES
//==============================================================================

/**
 * @brief Convert ESP-IDF Bluetooth error to HardFOC error.
 * @param esp_err ESP-IDF error code
 * @return HardFOC Bluetooth error code
 */
constexpr hf_bluetooth_err_t HfConvertEspBluetoothError(hf_i32_t esp_err) noexcept {
  switch (esp_err) {
    case 0: // ESP_OK
      return hf_bluetooth_err_t::BLUETOOTH_SUCCESS;
    case 0x101: // ESP_ERR_NO_MEM
      return hf_bluetooth_err_t::BLUETOOTH_ERR_NO_MEMORY;
    case 0x102: // ESP_ERR_INVALID_ARG
      return hf_bluetooth_err_t::BLUETOOTH_ERR_INVALID_PARAM;
    case 0x103: // ESP_ERR_INVALID_STATE
      return hf_bluetooth_err_t::BLUETOOTH_ERR_INVALID_STATE;
    case 0x106: // ESP_ERR_TIMEOUT
      return hf_bluetooth_err_t::BLUETOOTH_ERR_TIMEOUT;
    case 0x107: // ESP_ERR_NOT_FOUND
      return hf_bluetooth_err_t::BLUETOOTH_ERR_DEVICE_NOT_FOUND;
    case 0x108: // ESP_ERR_NOT_SUPPORTED
      return hf_bluetooth_err_t::BLUETOOTH_ERR_OPERATION_NOT_SUPPORTED;
    default:
      return hf_bluetooth_err_t::BLUETOOTH_ERR_FAILURE;
  }
}

/**
 * @brief Validate Bluetooth device address.
 * @param addr Address to validate
 * @return true if valid, false otherwise
 */
constexpr hf_bool_t HfIsValidBluetoothAddress(const hf_u8_t addr[6]) noexcept {
  // Check for all zeros (invalid)
  for (hf_u8_t i = 0; i < 6; i++) {
    if (addr[i] != 0) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Convert address array to string representation.
 * @param addr Address array (6 bytes)
 * @param str Output string buffer (minimum 18 bytes)
 * @return true on success, false on error
 */
hf_bool_t HfBluetoothAddressToString(const hf_u8_t addr[6], char* str) noexcept;

/**
 * @brief Convert string to address array.
 * @param str String representation (e.g., "AA:BB:CC:DD:EE:FF")
 * @param addr Output address array (6 bytes)
 * @return true on success, false on error
 */
hf_bool_t HfStringToBluetoothAddress(const char* str, hf_u8_t addr[6]) noexcept;

//==============================================================================
// PLATFORM-SPECIFIC CONSTANTS
//==============================================================================

/**
 * @brief Maximum number of concurrent connections for ESP32-C6.
 */
constexpr hf_u8_t HF_ESP32_MAX_BLE_CONNECTIONS = 9;

/**
 * @brief Default advertising interval in 0.625ms units (100ms).
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_ADV_INTERVAL = 160;

/**
 * @brief Default scan interval in 0.625ms units (50ms).
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_SCAN_INTERVAL = 80;

/**
 * @brief Default scan window in 0.625ms units (30ms).
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_SCAN_WINDOW = 48;

/**
 * @brief Default connection interval in 1.25ms units (30ms).
 */
constexpr hf_u16_t HF_ESP32_DEFAULT_CONN_INTERVAL = 24;

/**
 * @brief Maximum TX power for ESP32-C6 in dBm.
 */
constexpr hf_i8_t HF_ESP32_MAX_TX_POWER = 9;

/**
 * @brief Minimum TX power for ESP32-C6 in dBm.
 */
constexpr hf_i8_t HF_ESP32_MIN_TX_POWER = -12;

#endif // HF_MCU_FAMILY_ESP32