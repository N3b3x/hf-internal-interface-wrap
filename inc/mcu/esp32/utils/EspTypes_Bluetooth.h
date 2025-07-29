/**
 * @file EspTypes_Bluetooth.h
 * @brief ESP32 Bluetooth type definitions for hardware abstraction.
 *
 * This header defines the ESP32-specific types, constants, and utility functions
 * for Bluetooth operations. It provides a clean interface between the generic Bluetooth
 * base class and ESP-IDF specific implementations, supporting both Classic and BLE.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This file should be included by ESP32 Bluetooth implementation files.
 * @note All definitions are specific to ESP32 with ESP-IDF v5.5+.
 */

#pragma once

#include "EspTypes_Base.h"
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

// Bluetooth Classic headers
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_hf_client_api.h"

// Bluetooth Low Energy headers
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"

#ifdef __cplusplus
}
#endif

//==============================================================================
// ESP32 BLUETOOTH CONSTANTS
//==============================================================================

/// @brief Bluetooth address length
static constexpr size_t HF_ESP_BT_ADDRESS_LEN = 6;

/// @brief Maximum Bluetooth device name length
static constexpr size_t HF_ESP_BT_DEVICE_NAME_MAX_LEN = 32;

/// @brief Maximum number of simultaneous connections
static constexpr uint16_t HF_ESP_BT_MAX_CONNECTIONS_DEFAULT = 4;

/// @brief Default connection timeout
static constexpr hf_timeout_ms_t HF_ESP_BT_CONNECT_TIMEOUT_DEFAULT = 10000;

/// @brief Default scan timeout
static constexpr hf_timeout_ms_t HF_ESP_BT_SCAN_TIMEOUT_DEFAULT = 10000;

/// @brief Default pairing timeout
static constexpr hf_timeout_ms_t HF_ESP_BT_PAIR_TIMEOUT_DEFAULT = 30000;

/// @brief Maximum GATT services per device
static constexpr uint16_t HF_ESP_BT_MAX_GATT_SERVICES = 16;

/// @brief Maximum GATT characteristics per service
static constexpr uint16_t HF_ESP_BT_MAX_GATT_CHARACTERISTICS = 32;

/// @brief Default GATT MTU size
static constexpr uint16_t HF_ESP_BT_GATT_MTU_DEFAULT = 23;

/// @brief Maximum GATT MTU size
static constexpr uint16_t HF_ESP_BT_GATT_MTU_MAX = 517;

/// @brief BLE connection interval range (in 1.25ms units)
static constexpr uint16_t HF_ESP_BLE_CONN_INTERVAL_MIN = 6;    // 7.5ms
static constexpr uint16_t HF_ESP_BLE_CONN_INTERVAL_MAX = 3200; // 4000ms

/// @brief BLE slave latency range
static constexpr uint16_t HF_ESP_BLE_SLAVE_LATENCY_MIN = 0;
static constexpr uint16_t HF_ESP_BLE_SLAVE_LATENCY_MAX = 499;

/// @brief BLE supervision timeout range (in 10ms units)
static constexpr uint16_t HF_ESP_BLE_SUPERVISION_TIMEOUT_MIN = 10;   // 100ms
static constexpr uint16_t HF_ESP_BLE_SUPERVISION_TIMEOUT_MAX = 3200; // 32000ms

//==============================================================================
// ESP32 BLUETOOTH TYPE MAPPINGS
//==============================================================================

/**
 * @brief ESP32-specific Bluetooth mode mapping
 */
enum class HfEspBluetoothMode : uint8_t {
  IDLE = ESP_BT_MODE_IDLE,                  /**< Bluetooth disabled */
  BLE = ESP_BT_MODE_BLE,                    /**< BLE only */
  CLASSIC_BT = ESP_BT_MODE_CLASSIC_BT,      /**< Classic Bluetooth only */
  BTDM = ESP_BT_MODE_BTDM                   /**< Dual mode (Classic + BLE) */
};

/**
 * @brief ESP32-specific BLE address type mapping
 */
enum class HfEspBleAddrType : uint8_t {
  PUBLIC = BLE_ADDR_TYPE_PUBLIC,            /**< Public address */
  RANDOM = BLE_ADDR_TYPE_RANDOM,            /**< Random address */
  RPA_PUBLIC = BLE_ADDR_TYPE_RPA_PUBLIC,    /**< Resolvable private address with public identity */
  RPA_RANDOM = BLE_ADDR_TYPE_RPA_RANDOM     /**< Resolvable private address with random identity */
};

/**
 * @brief ESP32-specific BLE scan type mapping
 */
enum class HfEspBleScanType : uint8_t {
  PASSIVE = BLE_SCAN_TYPE_PASSIVE,          /**< Passive scanning */
  ACTIVE = BLE_SCAN_TYPE_ACTIVE             /**< Active scanning */
};

/**
 * @brief ESP32-specific BLE advertising type mapping
 */
enum class HfEspBleAdvType : uint8_t {
  ADV_IND = ADV_TYPE_IND,                   /**< Connectable undirected advertising */
  ADV_DIRECT_IND_HIGH = ADV_TYPE_DIRECT_IND_HIGH, /**< Connectable directed advertising (high duty cycle) */
  ADV_SCAN_IND = ADV_TYPE_SCAN_IND,         /**< Scannable undirected advertising */
  ADV_NONCONN_IND = ADV_TYPE_NONCONN_IND,   /**< Non-connectable undirected advertising */
  ADV_DIRECT_IND_LOW = ADV_TYPE_DIRECT_IND_LOW /**< Connectable directed advertising (low duty cycle) */
};

/**
 * @brief ESP32-specific GATT characteristic properties
 */
enum class HfEspGattCharProp : uint8_t {
  BROADCAST = ESP_GATT_CHAR_PROP_BIT_BROADCAST,           /**< Broadcast */
  READ = ESP_GATT_CHAR_PROP_BIT_READ,                     /**< Read */
  WRITE_NR = ESP_GATT_CHAR_PROP_BIT_WRITE_NR,             /**< Write without response */
  WRITE = ESP_GATT_CHAR_PROP_BIT_WRITE,                   /**< Write */
  NOTIFY = ESP_GATT_CHAR_PROP_BIT_NOTIFY,                 /**< Notify */
  INDICATE = ESP_GATT_CHAR_PROP_BIT_INDICATE,             /**< Indicate */
  AUTH = ESP_GATT_CHAR_PROP_BIT_AUTH,                     /**< Authenticated signed writes */
  EXT_PROP = ESP_GATT_CHAR_PROP_BIT_EXT_PROP              /**< Extended properties */
};

/**
 * @brief ESP32-specific GATT characteristic permissions
 */
enum class HfEspGattCharPerm : uint16_t {
  READ = ESP_GATT_PERM_READ,                              /**< Read permission */
  READ_ENCRYPTED = ESP_GATT_PERM_READ_ENCRYPTED,          /**< Read encrypted permission */
  READ_ENC_MITM = ESP_GATT_PERM_READ_ENC_MITM,            /**< Read encrypted with MITM protection */
  WRITE = ESP_GATT_PERM_WRITE,                            /**< Write permission */
  WRITE_ENCRYPTED = ESP_GATT_PERM_WRITE_ENCRYPTED,        /**< Write encrypted permission */
  WRITE_ENC_MITM = ESP_GATT_PERM_WRITE_ENC_MITM,          /**< Write encrypted with MITM protection */
  WRITE_SIGNED = ESP_GATT_PERM_WRITE_SIGNED,              /**< Write signed permission */
  WRITE_SIGNED_MITM = ESP_GATT_PERM_WRITE_SIGNED_MITM     /**< Write signed with MITM protection */
};

//==============================================================================
// ESP32 BLUETOOTH STRUCTURES
//==============================================================================

/**
 * @brief ESP32-specific Bluetooth statistics structure
 */
struct HfEspBluetoothStats {
  uint32_t tx_bytes;                        /**< Transmitted bytes */
  uint32_t rx_bytes;                        /**< Received bytes */
  uint32_t tx_packets;                      /**< Transmitted packets */
  uint32_t rx_packets;                      /**< Received packets */
  uint32_t tx_errors;                       /**< Transmission errors */
  uint32_t rx_errors;                       /**< Reception errors */
  uint32_t connections_attempted;           /**< Connection attempts */
  uint32_t connections_successful;          /**< Successful connections */
  uint32_t disconnections;                  /**< Disconnections */
  uint32_t pairing_attempts;                /**< Pairing attempts */
  uint32_t pairing_successful;              /**< Successful pairings */
};

/**
 * @brief ESP32-specific BLE advertising parameters
 */
struct HfEspBleAdvParams {
  uint16_t adv_int_min;                     /**< Minimum advertising interval */
  uint16_t adv_int_max;                     /**< Maximum advertising interval */
  HfEspBleAdvType adv_type;                 /**< Advertising type */
  esp_ble_addr_type_t own_addr_type;        /**< Own address type */
  esp_ble_addr_type_t peer_addr_type;       /**< Peer address type */
  esp_bd_addr_t peer_addr;                  /**< Peer address */
  esp_ble_adv_channel_t channel_map;        /**< Advertising channel map */
  esp_ble_adv_filter_t filter_policy;       /**< Advertising filter policy */
};

/**
 * @brief ESP32-specific BLE scan parameters
 */
struct HfEspBleScanParams {
  HfEspBleScanType scan_type;               /**< Scan type */
  esp_ble_addr_type_t own_addr_type;        /**< Own address type */
  esp_ble_scan_filter_t scan_filter_policy; /**< Scan filter policy */
  uint16_t scan_interval;                   /**< Scan interval */
  uint16_t scan_window;                     /**< Scan window */
  esp_ble_scan_duplicate_t scan_duplicate;  /**< Scan duplicate filter */
};

/**
 * @brief ESP32-specific BLE connection parameters
 */
struct HfEspBleConnParams {
  uint16_t interval_min;                    /**< Minimum connection interval */
  uint16_t interval_max;                    /**< Maximum connection interval */
  uint16_t latency;                         /**< Slave latency */
  uint16_t timeout;                         /**< Supervision timeout */
  uint16_t min_ce_len;                      /**< Minimum connection event length */
  uint16_t max_ce_len;                      /**< Maximum connection event length */
};

//==============================================================================
// ESP32 BLUETOOTH UTILITY FUNCTIONS
//==============================================================================

#ifdef __cplusplus

/**
 * @brief Convert HardFOC Bluetooth mode to ESP-IDF mode
 * @param mode HardFOC Bluetooth mode
 * @return ESP-IDF Bluetooth mode
 */
inline esp_bt_mode_t hfBluetoothModeToEspMode(HfBluetoothMode mode) {
  switch (mode) {
    case HfBluetoothMode::DISABLED: return ESP_BT_MODE_IDLE;
    case HfBluetoothMode::CLASSIC: return ESP_BT_MODE_CLASSIC_BT;
    case HfBluetoothMode::BLE: return ESP_BT_MODE_BLE;
    case HfBluetoothMode::DUAL: return ESP_BT_MODE_BTDM;
    default: return ESP_BT_MODE_IDLE;
  }
}

/**
 * @brief Convert ESP-IDF Bluetooth mode to HardFOC mode
 * @param mode ESP-IDF Bluetooth mode
 * @return HardFOC Bluetooth mode
 */
inline HfBluetoothMode espModeToHfBluetoothMode(esp_bt_mode_t mode) {
  switch (mode) {
    case ESP_BT_MODE_IDLE: return HfBluetoothMode::DISABLED;
    case ESP_BT_MODE_CLASSIC_BT: return HfBluetoothMode::CLASSIC;
    case ESP_BT_MODE_BLE: return HfBluetoothMode::BLE;
    case ESP_BT_MODE_BTDM: return HfBluetoothMode::DUAL;
    default: return HfBluetoothMode::DISABLED;
  }
}

/**
 * @brief Convert HardFOC scan type to ESP-IDF scan type
 * @param scan_type HardFOC scan type
 * @return ESP-IDF scan type
 */
inline esp_ble_scan_type_t hfScanTypeToEspScanType(HfBluetoothScanType scan_type) {
  switch (scan_type) {
    case HfBluetoothScanType::PASSIVE: return BLE_SCAN_TYPE_PASSIVE;
    case HfBluetoothScanType::ACTIVE: return BLE_SCAN_TYPE_ACTIVE;
    default: return BLE_SCAN_TYPE_ACTIVE;
  }
}

/**
 * @brief Convert ESP-IDF scan type to HardFOC scan type
 * @param scan_type ESP-IDF scan type
 * @return HardFOC scan type
 */
inline HfBluetoothScanType espScanTypeToHfScanType(esp_ble_scan_type_t scan_type) {
  switch (scan_type) {
    case BLE_SCAN_TYPE_PASSIVE: return HfBluetoothScanType::PASSIVE;
    case BLE_SCAN_TYPE_ACTIVE: return HfBluetoothScanType::ACTIVE;
    default: return HfBluetoothScanType::ACTIVE;
  }
}

/**
 * @brief Convert ESP-IDF error to HardFOC Bluetooth error
 * @param esp_err ESP-IDF error code
 * @return HardFOC Bluetooth error code
 */
inline HfBluetoothErr espErrorToHfBluetoothError(esp_err_t esp_err) {
  switch (esp_err) {
    case ESP_OK: return HfBluetoothErr::BLUETOOTH_SUCCESS;
    case ESP_ERR_INVALID_ARG: return HfBluetoothErr::BLUETOOTH_ERR_INVALID_PARAM;
    case ESP_ERR_INVALID_STATE: return HfBluetoothErr::BLUETOOTH_ERR_INVALID_STATE;
    case ESP_ERR_NO_MEM: return HfBluetoothErr::BLUETOOTH_ERR_NO_MEMORY;
    case ESP_ERR_TIMEOUT: return HfBluetoothErr::BLUETOOTH_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND: return HfBluetoothErr::BLUETOOTH_ERR_DEVICE_NOT_FOUND;
    case ESP_ERR_NOT_SUPPORTED: return HfBluetoothErr::BLUETOOTH_ERR_OPERATION_NOT_SUPPORTED;
    case ESP_ERR_BT_NIMBLE_BASE: return HfBluetoothErr::BLUETOOTH_ERR_FAILURE;
    default: return HfBluetoothErr::BLUETOOTH_ERR_FAILURE;
  }
}

/**
 * @brief Convert HardFOC Bluetooth address to ESP-IDF address
 * @param hf_addr HardFOC Bluetooth address
 * @param esp_addr ESP-IDF address output
 */
inline void hfBluetoothAddressToEspAddress(const HfBluetoothAddress& hf_addr, esp_bd_addr_t esp_addr) {
  std::memcpy(esp_addr, hf_addr.addr, HF_ESP_BT_ADDRESS_LEN);
}

/**
 * @brief Convert ESP-IDF address to HardFOC Bluetooth address
 * @param esp_addr ESP-IDF address
 * @param hf_addr HardFOC Bluetooth address output
 */
inline void espAddressToHfBluetoothAddress(const esp_bd_addr_t esp_addr, HfBluetoothAddress& hf_addr) {
  std::memcpy(hf_addr.addr, esp_addr, HF_ESP_BT_ADDRESS_LEN);
}

/**
 * @brief Convert HardFOC device type to ESP-IDF device type
 * @param device_type HardFOC device type
 * @return ESP-IDF device type
 */
inline esp_bt_dev_type_t hfDeviceTypeToEspDeviceType(HfBluetoothDeviceType device_type) {
  switch (device_type) {
    case HfBluetoothDeviceType::CLASSIC: return ESP_BT_DEV_TYPE_BREDR;
    case HfBluetoothDeviceType::BLE: return ESP_BT_DEV_TYPE_BLE;
    case HfBluetoothDeviceType::DUAL: return ESP_BT_DEV_TYPE_DUMO;
    case HfBluetoothDeviceType::UNKNOWN:
    default: return ESP_BT_DEV_TYPE_BREDR;
  }
}

/**
 * @brief Convert ESP-IDF device type to HardFOC device type
 * @param device_type ESP-IDF device type
 * @return HardFOC device type
 */
inline HfBluetoothDeviceType espDeviceTypeToHfDeviceType(esp_bt_dev_type_t device_type) {
  switch (device_type) {
    case ESP_BT_DEV_TYPE_BREDR: return HfBluetoothDeviceType::CLASSIC;
    case ESP_BT_DEV_TYPE_BLE: return HfBluetoothDeviceType::BLE;
    case ESP_BT_DEV_TYPE_DUMO: return HfBluetoothDeviceType::DUAL;
    default: return HfBluetoothDeviceType::UNKNOWN;
  }
}

/**
 * @brief Convert HardFOC security level to ESP-IDF security level
 * @param security HardFOC security level
 * @return ESP-IDF security level
 */
inline esp_ble_sec_act_t hfSecurityToEspSecurity(HfBluetoothSecurity security) {
  switch (security) {
    case HfBluetoothSecurity::NONE: return ESP_BLE_SEC_NONE;
    case HfBluetoothSecurity::UNAUTHENTICATED: return ESP_BLE_SEC_ENCRYPT;
    case HfBluetoothSecurity::AUTHENTICATED: return ESP_BLE_SEC_ENCRYPT_MITM;
    case HfBluetoothSecurity::AUTHORIZED: return ESP_BLE_SEC_ENCRYPT_MITM;
    case HfBluetoothSecurity::ENCRYPTED: return ESP_BLE_SEC_ENCRYPT;
    case HfBluetoothSecurity::AUTHENTICATED_SC: return ESP_BLE_SEC_ENCRYPT_MITM;
    default: return ESP_BLE_SEC_NONE;
  }
}

/**
 * @brief Convert ESP-IDF security level to HardFOC security level
 * @param security ESP-IDF security level
 * @return HardFOC security level
 */
inline HfBluetoothSecurity espSecurityToHfSecurity(esp_ble_sec_act_t security) {
  switch (security) {
    case ESP_BLE_SEC_NONE: return HfBluetoothSecurity::NONE;
    case ESP_BLE_SEC_ENCRYPT: return HfBluetoothSecurity::ENCRYPTED;
    case ESP_BLE_SEC_ENCRYPT_NO_MITM: return HfBluetoothSecurity::UNAUTHENTICATED;
    case ESP_BLE_SEC_ENCRYPT_MITM: return HfBluetoothSecurity::AUTHENTICATED;
    default: return HfBluetoothSecurity::NONE;
  }
}

/**
 * @brief Check if BLE connection interval is valid
 * @param interval Connection interval in 1.25ms units
 * @return true if valid, false otherwise
 */
inline bool isValidBleConnectionInterval(uint16_t interval) {
  return (interval >= HF_ESP_BLE_CONN_INTERVAL_MIN && interval <= HF_ESP_BLE_CONN_INTERVAL_MAX);
}

/**
 * @brief Check if BLE slave latency is valid
 * @param latency Slave latency
 * @return true if valid, false otherwise
 */
inline bool isValidBleSlaveLatency(uint16_t latency) {
  return (latency >= HF_ESP_BLE_SLAVE_LATENCY_MIN && latency <= HF_ESP_BLE_SLAVE_LATENCY_MAX);
}

/**
 * @brief Check if BLE supervision timeout is valid
 * @param timeout Supervision timeout in 10ms units
 * @return true if valid, false otherwise
 */
inline bool isValidBleSupervisionTimeout(uint16_t timeout) {
  return (timeout >= HF_ESP_BLE_SUPERVISION_TIMEOUT_MIN && timeout <= HF_ESP_BLE_SUPERVISION_TIMEOUT_MAX);
}

/**
 * @brief Check if device name is valid
 * @param name Device name string
 * @return true if valid, false otherwise
 */
inline bool isValidDeviceName(const std::string& name) {
  return (!name.empty() && name.length() <= HF_ESP_BT_DEVICE_NAME_MAX_LEN);
}

/**
 * @brief Check if GATT MTU size is valid
 * @param mtu MTU size
 * @return true if valid, false otherwise
 */
inline bool isValidGattMtu(uint16_t mtu) {
  return (mtu >= HF_ESP_BT_GATT_MTU_DEFAULT && mtu <= HF_ESP_BT_GATT_MTU_MAX);
}

/**
 * @brief Parse hex string to unsigned integer without exceptions
 * @param hex_str Hex string to parse
 * @param max_chars Maximum number of characters to parse
 * @param result Output result
 * @return true if parsing successful, false otherwise
 */
inline bool parseHexString(const std::string& hex_str, size_t max_chars, uint32_t& result) {
  result = 0;
  size_t len = std::min(hex_str.length(), max_chars);
  
  for (size_t i = 0; i < len; i++) {
    char c = hex_str[i];
    uint32_t digit = 0;
    
    if (c >= '0' && c <= '9') {
      digit = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      digit = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      digit = c - 'a' + 10;
    } else {
      return false; // Invalid hex character
    }
    
    result = (result << 4) | digit;
  }
  
  return true;
}

/**
 * @brief Convert UUID string to ESP-IDF UUID structure
 * @param uuid_str UUID string (16, 32, or 128-bit)
 * @param esp_uuid ESP-IDF UUID structure output
 * @return true if conversion successful, false otherwise
 */
inline bool stringToEspUuid(const std::string& uuid_str, esp_bt_uuid_t& esp_uuid) {
  if (uuid_str.length() == 4) {
    // 16-bit UUID
    esp_uuid.len = ESP_UUID_LEN_16;
    uint32_t value;
    if (!parseHexString(uuid_str, 4, value) || value > 0xFFFF) {
      return false;
    }
    esp_uuid.uuid.uuid16 = static_cast<uint16_t>(value);
    return true;
  } else if (uuid_str.length() == 8) {
    // 32-bit UUID
    esp_uuid.len = ESP_UUID_LEN_32;
    uint32_t value;
    if (!parseHexString(uuid_str, 8, value)) {
      return false;
    }
    esp_uuid.uuid.uuid32 = value;
    return true;
  } else if (uuid_str.length() == 36) {
    // 128-bit UUID (with dashes)
    esp_uuid.len = ESP_UUID_LEN_128;
    std::string clean_uuid = uuid_str;
    clean_uuid.erase(std::remove(clean_uuid.begin(), clean_uuid.end(), '-'), clean_uuid.end());
    
    static constexpr int UUID_128_BYTE_LENGTH = 16; // 128-bit UUID byte length
    
    if (clean_uuid.length() == 32) {
      for (int i = 0; i < UUID_128_BYTE_LENGTH; i++) {
        std::string byte_str = clean_uuid.substr(30 - (i * 2), 2);
        uint32_t value;
        if (!parseHexString(byte_str, 2, value) || value > 0xFF) {
          return false;
        }
        esp_uuid.uuid.uuid128[i] = static_cast<uint8_t>(value);
      }
      return true;
    }
  } else if (uuid_str.length() == 32) {
    // 128-bit UUID (without dashes)
    static constexpr int UUID_128_BYTE_LENGTH = 16; // 128-bit UUID byte length
    esp_uuid.len = ESP_UUID_LEN_128;
    for (int i = 0; i < UUID_128_BYTE_LENGTH; i++) {
      std::string byte_str = uuid_str.substr(30 - (i * 2), 2);
      uint32_t value;
      if (!parseHexString(byte_str, 2, value) || value > 0xFF) {
        return false;
      }
      esp_uuid.uuid.uuid128[i] = static_cast<uint8_t>(value);
    }
    return true;
  }
  
  return false;
}

/**
 * @brief Convert ESP-IDF UUID structure to UUID string
 * @param esp_uuid ESP-IDF UUID structure
 * @return UUID string
 */
inline std::string espUuidToString(const esp_bt_uuid_t& esp_uuid) {
  char buffer[64];
  
  switch (esp_uuid.len) {
    case ESP_UUID_LEN_16:
      std::snprintf(buffer, sizeof(buffer), "%04X", esp_uuid.uuid.uuid16);
      break;
    case ESP_UUID_LEN_32:
      std::snprintf(buffer, sizeof(buffer), "%08X", esp_uuid.uuid.uuid32);
      break;
    case ESP_UUID_LEN_128:
      std::snprintf(buffer, sizeof(buffer),
                   "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                   esp_uuid.uuid.uuid128[15], esp_uuid.uuid.uuid128[14],
                   esp_uuid.uuid.uuid128[13], esp_uuid.uuid.uuid128[12],
                   esp_uuid.uuid.uuid128[11], esp_uuid.uuid.uuid128[10],
                   esp_uuid.uuid.uuid128[9], esp_uuid.uuid.uuid128[8],
                   esp_uuid.uuid.uuid128[7], esp_uuid.uuid.uuid128[6],
                   esp_uuid.uuid.uuid128[5], esp_uuid.uuid.uuid128[4],
                   esp_uuid.uuid.uuid128[3], esp_uuid.uuid.uuid128[2],
                   esp_uuid.uuid.uuid128[1], esp_uuid.uuid.uuid128[0]);
      break;
    default:
      return "";
  }
  
  return std::string(buffer);
}

/**
 * @brief Create default BLE advertising parameters
 * @return Default advertising parameters
 */
inline HfEspBleAdvParams createDefaultBleAdvParams() {
  HfEspBleAdvParams params;
  params.adv_int_min = 0x20;  // 32 * 0.625ms = 20ms
  params.adv_int_max = 0x40;  // 64 * 0.625ms = 40ms
  params.adv_type = HfEspBleAdvType::ADV_IND;
  params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
  params.peer_addr_type = BLE_ADDR_TYPE_PUBLIC;
  std::memset(params.peer_addr, 0, sizeof(params.peer_addr));
  params.channel_map = ADV_CHNL_ALL;
  params.filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
  return params;
}

/**
 * @brief Create default BLE scan parameters
 * @return Default scan parameters
 */
inline HfEspBleScanParams createDefaultBleScanParams() {
  HfEspBleScanParams params;
  params.scan_type = HfEspBleScanType::ACTIVE;
  params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
  params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
  params.scan_interval = 0x50;  // 80 * 0.625ms = 50ms
  params.scan_window = 0x30;    // 48 * 0.625ms = 30ms
  params.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE;
  return params;
}

/**
 * @brief Create default BLE connection parameters
 * @return Default connection parameters
 */
inline HfEspBleConnParams createDefaultBleConnParams() {
  HfEspBleConnParams params;
  params.interval_min = 0x18;   // 24 * 1.25ms = 30ms
  params.interval_max = 0x28;   // 40 * 1.25ms = 50ms
  params.latency = 0;           // No slave latency
  params.timeout = 0x64;        // 100 * 10ms = 1000ms
  params.min_ce_len = 0x10;     // 16 * 0.625ms = 10ms
  params.max_ce_len = 0x20;     // 32 * 0.625ms = 20ms
  return params;
}

#endif // __cplusplus