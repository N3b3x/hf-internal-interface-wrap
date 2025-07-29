/**
 * @file BaseBluetooth.h
 * @ingroup bluetooth
 * @brief Unified Bluetooth base class for all Bluetooth implementations.
 *
 * This file contains the declaration of the BaseBluetooth abstract class, which provides
 * a comprehensive Bluetooth abstraction that serves as the base for all Bluetooth
 * implementations in the HardFOC system. It supports both Bluetooth Classic and
 * Bluetooth Low Energy (BLE), device discovery, pairing, connection management,
 * and works across different hardware platforms including ESP32, and other
 * Bluetooth-capable MCUs.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

/**
 * @defgroup bluetooth Bluetooth Module
 * @brief All Bluetooth-related types, enums, and functions for wireless communication operations.
 * @details This module provides comprehensive Bluetooth functionality including:
 *          - Bluetooth Classic operations (SPP, A2DP, HFP, etc.)
 *          - Bluetooth Low Energy (BLE) operations
 *          - Device discovery and scanning
 *          - Pairing and bonding management
 *          - Connection management
 *          - Service and characteristic handling (BLE)
 *          - Event handling and callbacks
 *          - Error handling and diagnostics
 */

/**
 * @ingroup bluetooth
 * @brief HardFOC Bluetooth error codes macro list
 * @details X-macro pattern for comprehensive error enumeration. Each entry contains:
 *          X(NAME, VALUE, DESCRIPTION)
 */
#define HF_BLUETOOTH_ERR_LIST(X)                                         \
  /* Success codes */                                                    \
  X(BLUETOOTH_SUCCESS, 0, "Success")                                     \
                                                                         \
  /* General errors */                                                   \
  X(BLUETOOTH_ERR_FAILURE, 1, "General failure")                        \
  X(BLUETOOTH_ERR_INVALID_PARAM, 2, "Invalid parameter")                \
  X(BLUETOOTH_ERR_NOT_INITIALIZED, 3, "Bluetooth not initialized")      \
  X(BLUETOOTH_ERR_ALREADY_INITIALIZED, 4, "Bluetooth already initialized") \
  X(BLUETOOTH_ERR_NOT_ENABLED, 5, "Bluetooth not enabled")              \
  X(BLUETOOTH_ERR_ALREADY_ENABLED, 6, "Bluetooth already enabled")      \
  X(BLUETOOTH_ERR_NOT_CONNECTED, 7, "Bluetooth not connected")          \
  X(BLUETOOTH_ERR_ALREADY_CONNECTED, 8, "Bluetooth already connected")  \
  X(BLUETOOTH_ERR_CONNECTION_FAILED, 9, "Connection failed")            \
  X(BLUETOOTH_ERR_DISCONNECTION_FAILED, 10, "Disconnection failed")     \
  X(BLUETOOTH_ERR_SCAN_FAILED, 11, "Device scan failed")                \
  X(BLUETOOTH_ERR_PAIR_FAILED, 12, "Pairing failed")                    \
  X(BLUETOOTH_ERR_UNPAIR_FAILED, 13, "Unpairing failed")                \
  X(BLUETOOTH_ERR_TIMEOUT, 14, "Operation timeout")                     \
  X(BLUETOOTH_ERR_NO_MEMORY, 15, "Insufficient memory")                 \
  X(BLUETOOTH_ERR_INVALID_ADDRESS, 16, "Invalid Bluetooth address")     \
  X(BLUETOOTH_ERR_DEVICE_NOT_FOUND, 17, "Device not found")             \
  X(BLUETOOTH_ERR_SERVICE_NOT_FOUND, 18, "Service not found")           \
  X(BLUETOOTH_ERR_CHARACTERISTIC_NOT_FOUND, 19, "Characteristic not found") \
  X(BLUETOOTH_ERR_AUTHENTICATION_FAILED, 20, "Authentication failed")   \
  X(BLUETOOTH_ERR_AUTHORIZATION_FAILED, 21, "Authorization failed")     \
  X(BLUETOOTH_ERR_ENCRYPTION_FAILED, 22, "Encryption failed")           \
  X(BLUETOOTH_ERR_OPERATION_NOT_SUPPORTED, 23, "Operation not supported") \
  X(BLUETOOTH_ERR_GATT_ERROR, 24, "GATT operation error")               \
  X(BLUETOOTH_ERR_INVALID_STATE, 25, "Invalid state for operation")

/**
 * @ingroup bluetooth
 * @brief Generate Bluetooth error enumeration using X-macro pattern
 */
#define X(name, value, desc) name = value,
enum class HfBluetoothErr : uint8_t { HF_BLUETOOTH_ERR_LIST(X) };
#undef X

/**
 * @ingroup bluetooth
 * @brief Bluetooth operating modes
 */
enum class HfBluetoothMode : uint8_t {
  DISABLED = 0,        /**< Bluetooth disabled */
  CLASSIC = 1,         /**< Bluetooth Classic only */
  BLE = 2,             /**< Bluetooth Low Energy only */
  DUAL = 3             /**< Both Classic and BLE */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth device types
 */
enum class HfBluetoothDeviceType : uint8_t {
  UNKNOWN = 0,         /**< Unknown device type */
  CLASSIC = 1,         /**< Bluetooth Classic device */
  BLE = 2,             /**< Bluetooth Low Energy device */
  DUAL = 3             /**< Dual-mode device */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth connection states
 */
enum class HfBluetoothState : uint8_t {
  DISABLED = 0,        /**< Bluetooth disabled */
  ENABLED = 1,         /**< Bluetooth enabled but not connected */
  SCANNING = 2,        /**< Scanning for devices */
  CONNECTING = 3,      /**< Attempting to connect */
  CONNECTED = 4,       /**< Connected to device */
  DISCONNECTING = 5,   /**< Disconnecting from device */
  PAIRING = 6,         /**< Pairing with device */
  PAIRED = 7           /**< Paired with device */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth security levels
 */
enum class HfBluetoothSecurity : uint8_t {
  NONE = 0,            /**< No security */
  UNAUTHENTICATED = 1, /**< Unauthenticated pairing */
  AUTHENTICATED = 2,   /**< Authenticated pairing */
  AUTHORIZED = 3,      /**< Authorized connection */
  ENCRYPTED = 4,       /**< Encrypted connection */
  AUTHENTICATED_SC = 5 /**< Authenticated Secure Connections */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth scan types
 */
enum class HfBluetoothScanType : uint8_t {
  PASSIVE = 0,         /**< Passive scanning */
  ACTIVE = 1           /**< Active scanning */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth event types for callback functions
 */
enum class HfBluetoothEvent : uint8_t {
  ENABLED = 0,                    /**< Bluetooth enabled */
  DISABLED = 1,                   /**< Bluetooth disabled */
  SCAN_START = 2,                 /**< Scan started */
  SCAN_STOP = 3,                  /**< Scan stopped */
  DEVICE_FOUND = 4,               /**< Device discovered */
  PAIR_REQUEST = 5,               /**< Pairing request received */
  PAIR_SUCCESS = 6,               /**< Pairing successful */
  PAIR_FAILED = 7,                /**< Pairing failed */
  UNPAIR_SUCCESS = 8,             /**< Unpairing successful */
  CONNECT_SUCCESS = 9,            /**< Connection successful */
  CONNECT_FAILED = 10,            /**< Connection failed */
  DISCONNECT = 11,                /**< Device disconnected */
  DATA_RECEIVED = 12,             /**< Data received */
  DATA_SENT = 13,                 /**< Data sent */
  GATT_SERVICE_DISCOVERED = 14,   /**< GATT service discovered */
  GATT_CHARACTERISTIC_READ = 15,  /**< GATT characteristic read */
  GATT_CHARACTERISTIC_WRITE = 16, /**< GATT characteristic written */
  GATT_NOTIFICATION = 17          /**< GATT notification received */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth address structure (6 bytes)
 */
struct HfBluetoothAddress {
  uint8_t addr[6];               /**< Bluetooth address bytes */
  
  /**
   * @brief Convert address to string format
   * @return Address string in format "XX:XX:XX:XX:XX:XX"
   */
  std::string toString() const;
  
  /**
   * @brief Parse address from string
   * @param addr_str Address string in format "XX:XX:XX:XX:XX:XX"
   * @return true if parsed successfully, false otherwise
   */
  bool fromString(const std::string& addr_str);
  
  /**
   * @brief Check if address is valid (not all zeros)
   * @return true if valid, false otherwise
   */
  bool isValid() const;
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth device information structure
 */
struct HfBluetoothDeviceInfo {
  HfBluetoothAddress address;     /**< Device Bluetooth address */
  std::string name;               /**< Device name */
  HfBluetoothDeviceType type;     /**< Device type */
  int8_t rssi;                    /**< Signal strength (dBm) */
  uint32_t class_of_device;       /**< Class of Device (Classic only) */
  std::vector<std::string> uuids; /**< Service UUIDs */
  bool is_bonded;                 /**< True if device is bonded */
  bool is_connected;              /**< True if device is connected */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth Classic configuration structure
 */
struct HfBluetoothClassicConfig {
  std::string device_name;        /**< Local device name */
  bool discoverable;              /**< Make device discoverable */
  bool connectable;               /**< Make device connectable */
  uint32_t discovery_timeout_ms;  /**< Discovery timeout in milliseconds */
  HfBluetoothSecurity security;   /**< Security level */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth Low Energy configuration structure
 */
struct HfBluetoothBleConfig {
  std::string device_name;        /**< Local device name */
  bool advertising;               /**< Enable advertising */
  bool scannable;                 /**< Make device scannable */
  bool connectable;               /**< Make device connectable */
  uint16_t advertising_interval_ms; /**< Advertising interval in milliseconds */
  uint16_t scan_interval_ms;      /**< Scan interval in milliseconds */
  uint16_t scan_window_ms;        /**< Scan window in milliseconds */
  HfBluetoothScanType scan_type;  /**< Scan type */
  HfBluetoothSecurity security;   /**< Security level */
};

/**
 * @ingroup bluetooth
 * @brief GATT service structure
 */
struct HfBluetoothGattService {
  std::string uuid;               /**< Service UUID */
  bool is_primary;                /**< True if primary service */
  uint16_t handle;                /**< Service handle */
};

/**
 * @ingroup bluetooth
 * @brief GATT characteristic structure
 */
struct HfBluetoothGattCharacteristic {
  std::string uuid;               /**< Characteristic UUID */
  uint16_t handle;                /**< Characteristic handle */
  uint8_t properties;             /**< Characteristic properties */
  std::vector<uint8_t> value;     /**< Characteristic value */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth event callback function type
 */
using HfBluetoothEventCallback = std::function<void(HfBluetoothEvent event, void* event_data)>;

/**
 * @ingroup bluetooth
 * @brief Data received callback function type
 */
using HfBluetoothDataCallback = std::function<void(const HfBluetoothAddress& address, 
                                                   const std::vector<uint8_t>& data)>;

/**
 * @class BaseBluetooth
 * @ingroup bluetooth
 * @brief Abstract base class for Bluetooth functionality
 * @details This class provides the interface for Bluetooth operations including:
 *          - Bluetooth Classic and BLE operations
 *          - Device discovery and scanning
 *          - Pairing and bonding management
 *          - Connection management
 *          - Data transmission and reception
 *          - GATT operations (for BLE)
 *          - Event handling and callbacks
 */
class BaseBluetooth {
public:
  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~BaseBluetooth() = default;

  // ========== Initialization and Configuration ==========

  /**
   * @brief Initialize the Bluetooth subsystem
   * @param mode Bluetooth operating mode
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr init(HfBluetoothMode mode) = 0;

  /**
   * @brief Deinitialize the Bluetooth subsystem
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr deinit() = 0;

  /**
   * @brief Check if Bluetooth is initialized
   * @return true if initialized, false otherwise
   */
  virtual bool isInitialized() const = 0;

  /**
   * @brief Enable Bluetooth
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr enable() = 0;

  /**
   * @brief Disable Bluetooth
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr disable() = 0;

  /**
   * @brief Check if Bluetooth is enabled
   * @return true if enabled, false otherwise
   */
  virtual bool isEnabled() const = 0;

  /**
   * @brief Set Bluetooth operating mode
   * @param mode Bluetooth operating mode
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr setMode(HfBluetoothMode mode) = 0;

  /**
   * @brief Get current Bluetooth operating mode
   * @return Current Bluetooth mode
   */
  virtual HfBluetoothMode getMode() const = 0;

  // ========== Device Management ==========

  /**
   * @brief Get local Bluetooth address
   * @param address Reference to store local address
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr getLocalAddress(HfBluetoothAddress& address) const = 0;

  /**
   * @brief Set local device name
   * @param name Device name
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr setDeviceName(const std::string& name) = 0;

  /**
   * @brief Get local device name
   * @return Device name string
   */
  virtual std::string getDeviceName() const = 0;

  // ========== Classic Bluetooth Operations ==========

  /**
   * @brief Configure Bluetooth Classic parameters
   * @param config Classic configuration
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr configureClassic(const HfBluetoothClassicConfig& config) = 0;

  /**
   * @brief Make device discoverable
   * @param discoverable True to make discoverable, false otherwise
   * @param timeout_ms Discoverable timeout in milliseconds (0 for indefinite)
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr setDiscoverable(bool discoverable, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief Check if device is discoverable
   * @return true if discoverable, false otherwise
   */
  virtual bool isDiscoverable() const = 0;

  // ========== BLE Operations ==========

  /**
   * @brief Configure Bluetooth Low Energy parameters
   * @param config BLE configuration
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr configureBle(const HfBluetoothBleConfig& config) = 0;

  /**
   * @brief Start BLE advertising
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr startAdvertising() = 0;

  /**
   * @brief Stop BLE advertising
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr stopAdvertising() = 0;

  /**
   * @brief Check if BLE advertising is active
   * @return true if advertising, false otherwise
   */
  virtual bool isAdvertising() const = 0;

  // ========== Device Discovery ==========

  /**
   * @brief Start device discovery/scanning
   * @param duration_ms Scan duration in milliseconds (0 for indefinite)
   * @param type Scan type (BLE only)
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr startScan(uint32_t duration_ms = 0, HfBluetoothScanType type = HfBluetoothScanType::ACTIVE) = 0;

  /**
   * @brief Stop device discovery/scanning
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr stopScan() = 0;

  /**
   * @brief Check if scanning is in progress
   * @return true if scanning, false otherwise
   */
  virtual bool isScanning() const = 0;

  /**
   * @brief Get discovered devices
   * @param devices Vector to store discovered devices
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr getDiscoveredDevices(std::vector<HfBluetoothDeviceInfo>& devices) = 0;

  /**
   * @brief Clear discovered devices list
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr clearDiscoveredDevices() = 0;

  // ========== Connection Management ==========

  /**
   * @brief Connect to a remote device
   * @param address Remote device address
   * @param timeout_ms Connection timeout in milliseconds (0 for default)
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr connect(const HfBluetoothAddress& address, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief Disconnect from a remote device
   * @param address Remote device address
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr disconnect(const HfBluetoothAddress& address) = 0;

  /**
   * @brief Check if connected to a device
   * @param address Remote device address
   * @return true if connected, false otherwise
   */
  virtual bool isConnected(const HfBluetoothAddress& address) const = 0;

  /**
   * @brief Get list of connected devices
   * @param devices Vector to store connected devices
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr getConnectedDevices(std::vector<HfBluetoothDeviceInfo>& devices) = 0;

  // ========== Pairing and Bonding ==========

  /**
   * @brief Pair with a remote device
   * @param address Remote device address
   * @param pin PIN code (for Classic, empty for BLE)
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr pair(const HfBluetoothAddress& address, const std::string& pin = "") = 0;

  /**
   * @brief Unpair from a remote device
   * @param address Remote device address
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr unpair(const HfBluetoothAddress& address) = 0;

  /**
   * @brief Check if paired with a device
   * @param address Remote device address
   * @return true if paired, false otherwise
   */
  virtual bool isPaired(const HfBluetoothAddress& address) const = 0;

  /**
   * @brief Get list of paired devices
   * @param devices Vector to store paired devices
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr getPairedDevices(std::vector<HfBluetoothDeviceInfo>& devices) = 0;

  // ========== Data Transmission ==========

  /**
   * @brief Send data to a connected device
   * @param address Remote device address
   * @param data Data to send
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr sendData(const HfBluetoothAddress& address, const std::vector<uint8_t>& data) = 0;

  /**
   * @brief Check if data is available to read
   * @param address Remote device address
   * @return Number of bytes available, or -1 on error
   */
  virtual int getAvailableData(const HfBluetoothAddress& address) const = 0;

  /**
   * @brief Read available data from a connected device
   * @param address Remote device address
   * @param data Vector to store received data
   * @param max_bytes Maximum bytes to read (0 for all available)
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr readData(const HfBluetoothAddress& address, std::vector<uint8_t>& data, size_t max_bytes = 0) = 0;

  // ========== GATT Operations (BLE) ==========

  /**
   * @brief Discover GATT services on a connected device
   * @param address Remote device address
   * @param services Vector to store discovered services
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr discoverServices(const HfBluetoothAddress& address, std::vector<HfBluetoothGattService>& services) = 0;

  /**
   * @brief Discover GATT characteristics for a service
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristics Vector to store discovered characteristics
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr discoverCharacteristics(const HfBluetoothAddress& address, 
                                                  const std::string& service_uuid,
                                                  std::vector<HfBluetoothGattCharacteristic>& characteristics) = 0;

  /**
   * @brief Read GATT characteristic value
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristic_uuid Characteristic UUID
   * @param value Vector to store characteristic value
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr readCharacteristic(const HfBluetoothAddress& address,
                                             const std::string& service_uuid,
                                             const std::string& characteristic_uuid,
                                             std::vector<uint8_t>& value) = 0;

  /**
   * @brief Write GATT characteristic value
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristic_uuid Characteristic UUID
   * @param value Value to write
   * @param with_response True to wait for write response
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr writeCharacteristic(const HfBluetoothAddress& address,
                                              const std::string& service_uuid,
                                              const std::string& characteristic_uuid,
                                              const std::vector<uint8_t>& value,
                                              bool with_response = true) = 0;

  /**
   * @brief Subscribe to GATT characteristic notifications
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristic_uuid Characteristic UUID
   * @param enable True to enable notifications, false to disable
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr subscribeCharacteristic(const HfBluetoothAddress& address,
                                                  const std::string& service_uuid,
                                                  const std::string& characteristic_uuid,
                                                  bool enable) = 0;

  // ========== State and Status ==========

  /**
   * @brief Get current Bluetooth state
   * @return Current Bluetooth state
   */
  virtual HfBluetoothState getState() const = 0;

  /**
   * @brief Get signal strength for a connected device
   * @param address Remote device address
   * @return Signal strength in dBm, or INT8_MIN on error
   */
  virtual int8_t getRssi(const HfBluetoothAddress& address) const = 0;

  // ========== Event Handling ==========

  /**
   * @brief Register event callback
   * @param callback Event callback function
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr registerEventCallback(HfBluetoothEventCallback callback) = 0;

  /**
   * @brief Unregister event callback
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr unregisterEventCallback() = 0;

  /**
   * @brief Register data received callback
   * @param callback Data callback function
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr registerDataCallback(HfBluetoothDataCallback callback) = 0;

  /**
   * @brief Unregister data received callback
   * @return HfBluetoothErr::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual HfBluetoothErr unregisterDataCallback() = 0;

  // ========== Utility Functions ==========

  /**
   * @brief Get error description string
   * @param error Error code
   * @return Error description string
   */
  static const char* getErrorString(HfBluetoothErr error);

protected:
  /**
   * @brief Protected constructor - only derived classes can instantiate
   */
  BaseBluetooth() = default;

  /**
   * @brief Copy constructor - deleted to prevent copying
   */
  BaseBluetooth(const BaseBluetooth&) = delete;

  /**
   * @brief Assignment operator - deleted to prevent copying
   */
  BaseBluetooth& operator=(const BaseBluetooth&) = delete;
};

/**
 * @ingroup bluetooth
 * @brief Helper function to convert error enum to string
 */
inline const char* BaseBluetooth::getErrorString(HfBluetoothErr error) {
#define X(name, value, desc) case HfBluetoothErr::name: return desc;
  switch (error) {
    HF_BLUETOOTH_ERR_LIST(X)
    default: return "Unknown error";
  }
#undef X
}

/**
 * @ingroup bluetooth
 * @brief Helper function implementations for HfBluetoothAddress
 */
inline std::string HfBluetoothAddress::toString() const {
  char buffer[18];
  snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return std::string(buffer);
}

inline bool HfBluetoothAddress::fromString(const std::string& addr_str) {
  if (addr_str.length() != 17) return false;
  
  int values[6];
  int result = sscanf(addr_str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
                      &values[0], &values[1], &values[2], 
                      &values[3], &values[4], &values[5]);
  
  if (result != 6) return false;
  
  for (int i = 0; i < 6; i++) {
    addr[i] = static_cast<uint8_t>(values[i]);
  }
  
  return true;
}

inline bool HfBluetoothAddress::isValid() const {
  for (int i = 0; i < 6; i++) {
    if (addr[i] != 0) return true;
  }
  return false;
}