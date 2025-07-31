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
#include <cctype>
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
enum class hf_bluetooth_err_t : hf_u8_t { HF_BLUETOOTH_ERR_LIST(X) };
#undef X

/**
 * @ingroup bluetooth
 * @brief Convert hf_bluetooth_err_t to human-readable string
 * @param err The error code to convert
 * @return Pointer to error description string
 */
constexpr const char* hf_bluetooth_err_tToString(hf_bluetooth_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC) \
  case hf_bluetooth_err_t::NAME:  \
    return DESC;
    HF_BLUETOOTH_ERR_LIST(X)
#undef X
    default:
      return "Unknown error";
  }
}

/**
 * @ingroup bluetooth
 * @brief Bluetooth operating modes
 */
enum class hf_bluetooth_mode_t : hf_u8_t {
  HF_BLUETOOTH_MODE_DISABLED = 0,        /**< Bluetooth disabled */
  HF_BLUETOOTH_MODE_CLASSIC = 1,         /**< Bluetooth Classic only */
  HF_BLUETOOTH_MODE_BLE = 2,             /**< Bluetooth Low Energy only */
  HF_BLUETOOTH_MODE_DUAL = 3             /**< Both Classic and BLE */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth device types
 */
enum class hf_bluetooth_device_type_t : hf_u8_t {
  HF_BLUETOOTH_DEVICE_TYPE_UNKNOWN = 0,         /**< Unknown device type */
  HF_BLUETOOTH_DEVICE_TYPE_CLASSIC = 1,         /**< Bluetooth Classic device */
  HF_BLUETOOTH_DEVICE_TYPE_BLE = 2,             /**< Bluetooth Low Energy device */
  HF_BLUETOOTH_DEVICE_TYPE_DUAL = 3             /**< Dual-mode device */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth connection states
 */
enum class hf_bluetooth_state_t : hf_u8_t {
  HF_BLUETOOTH_STATE_DISABLED = 0,        /**< Bluetooth disabled */
  HF_BLUETOOTH_STATE_ENABLED = 1,         /**< Bluetooth enabled but not connected */
  HF_BLUETOOTH_STATE_SCANNING = 2,        /**< Scanning for devices */
  HF_BLUETOOTH_STATE_CONNECTING = 3,      /**< Attempting to connect */
  HF_BLUETOOTH_STATE_CONNECTED = 4,       /**< Connected to device */
  HF_BLUETOOTH_STATE_DISCONNECTING = 5,   /**< Disconnecting from device */
  HF_BLUETOOTH_STATE_PAIRING = 6,         /**< Pairing with device */
  HF_BLUETOOTH_STATE_PAIRED = 7           /**< Paired with device */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth security levels
 */
enum class hf_bluetooth_security_t : hf_u8_t {
  HF_BLUETOOTH_SECURITY_NONE = 0,            /**< No security */
  HF_BLUETOOTH_SECURITY_UNAUTHENTICATED = 1, /**< Unauthenticated pairing */
  HF_BLUETOOTH_SECURITY_AUTHENTICATED = 2,   /**< Authenticated pairing */
  HF_BLUETOOTH_SECURITY_AUTHORIZED = 3,      /**< Authorized connection */
  HF_BLUETOOTH_SECURITY_ENCRYPTED = 4,       /**< Encrypted connection */
  HF_BLUETOOTH_SECURITY_AUTHENTICATED_SC = 5 /**< Authenticated Secure Connections */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth scan types
 */
enum class hf_bluetooth_scan_type_t : hf_u8_t {
  HF_BLUETOOTH_SCAN_TYPE_PASSIVE = 0,         /**< Passive scanning */
  HF_BLUETOOTH_SCAN_TYPE_ACTIVE = 1           /**< Active scanning */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth event types for callback functions
 */
enum class hf_bluetooth_event_t : hf_u8_t {
  HF_BLUETOOTH_EVENT_ENABLED = 0,                    /**< Bluetooth enabled */
  HF_BLUETOOTH_EVENT_DISABLED = 1,                   /**< Bluetooth disabled */
  HF_BLUETOOTH_EVENT_SCAN_START = 2,                 /**< Scan started */
  HF_BLUETOOTH_EVENT_SCAN_STOP = 3,                  /**< Scan stopped */
  HF_BLUETOOTH_EVENT_DEVICE_FOUND = 4,               /**< Device discovered */
  HF_BLUETOOTH_EVENT_PAIR_REQUEST = 5,               /**< Pairing request received */
  HF_BLUETOOTH_EVENT_PAIR_SUCCESS = 6,               /**< Pairing successful */
  HF_BLUETOOTH_EVENT_PAIR_FAILED = 7,                /**< Pairing failed */
  HF_BLUETOOTH_EVENT_UNPAIR_SUCCESS = 8,             /**< Unpairing successful */
  HF_BLUETOOTH_EVENT_CONNECT_SUCCESS = 9,            /**< Connection successful */
  HF_BLUETOOTH_EVENT_CONNECT_FAILED = 10,            /**< Connection failed */
  HF_BLUETOOTH_EVENT_DISCONNECT = 11,                /**< Device disconnected */
  HF_BLUETOOTH_EVENT_DATA_RECEIVED = 12,             /**< Data received */
  HF_BLUETOOTH_EVENT_DATA_SENT = 13,                 /**< Data sent */
  HF_BLUETOOTH_EVENT_GATT_SERVICE_DISCOVERED = 14,   /**< GATT service discovered */
  HF_BLUETOOTH_EVENT_GATT_CHARACTERISTIC_READ = 15,  /**< GATT characteristic read */
  HF_BLUETOOTH_EVENT_GATT_CHARACTERISTIC_WRITE = 16, /**< GATT characteristic written */
  HF_BLUETOOTH_EVENT_GATT_NOTIFICATION = 17          /**< GATT notification received */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth address structure (6 bytes)
 */
struct hf_bluetooth_address_t {
  hf_u8_t addr[6];               /**< Bluetooth address bytes */
  
  /**
   * @brief Convert address to string format
   * @return Address string in format "XX:XX:XX:XX:XX:XX"
   */
  std::string ToString() const;
  
  /**
   * @brief Parse address from string
   * @param addr_str Address string in format "XX:XX:XX:XX:XX:XX"
   * @return true if parsed successfully, false otherwise
   */
  bool FromString(const std::string& addr_str);
  
  /**
   * @brief Check if address is valid (not all zeros)
   * @return true if valid, false otherwise
   */
  bool IsValid() const;
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth device information structure
 */
struct hf_bluetooth_device_info_t {
  hf_bluetooth_address_t address;          /**< Device Bluetooth address */
  std::string name;                        /**< Device name */
  hf_bluetooth_device_type_t type;         /**< Device type */
  int8_t rssi;                             /**< Signal strength (dBm) */
  uint32_t class_of_device;                /**< Class of Device (Classic only) */
  std::vector<std::string> uuids;          /**< Service UUIDs */
  bool is_bonded;                          /**< True if device is bonded */
  bool is_connected;                       /**< True if device is connected */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth Classic configuration structure
 */
struct hf_bluetooth_classic_config_t {
  std::string device_name;                 /**< Local device name */
  bool discoverable;                       /**< Make device discoverable */
  bool connectable;                        /**< Make device connectable */
  uint32_t discovery_timeout_ms;           /**< Discovery timeout in milliseconds */
  hf_bluetooth_security_t security;        /**< Security level */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth Low Energy configuration structure
 */
struct hf_bluetooth_ble_config_t {
  std::string device_name;                 /**< Local device name */
  bool advertising;                        /**< Enable advertising */
  bool scannable;                          /**< Make device scannable */
  bool connectable;                        /**< Make device connectable */
  uint16_t advertising_interval_ms;        /**< Advertising interval in milliseconds */
  uint16_t scan_interval_ms;               /**< Scan interval in milliseconds */
  uint16_t scan_window_ms;                 /**< Scan window in milliseconds */
  hf_bluetooth_scan_type_t scan_type;      /**< Scan type */
  hf_bluetooth_security_t security;        /**< Security level */
};

/**
 * @ingroup bluetooth
 * @brief GATT service structure
 */
struct hf_bluetooth_gatt_service_t {
  std::string uuid;               /**< Service UUID */
  bool is_primary;                /**< True if primary service */
  uint16_t handle;                /**< Service handle */
};

/**
 * @ingroup bluetooth
 * @brief GATT characteristic structure
 */
struct hf_bluetooth_gatt_characteristic_t {
  std::string uuid;               /**< Characteristic UUID */
  uint16_t handle;                /**< Characteristic handle */
  uint8_t properties;             /**< Characteristic properties */
  std::vector<uint8_t> value;     /**< Characteristic value */
};

/**
 * @ingroup bluetooth
 * @brief Bluetooth event callback function type
 */
using hf_bluetooth_event_callback_t = std::function<void(hf_bluetooth_event_t event, void* event_data)>;

/**
 * @ingroup bluetooth
 * @brief Data received callback function type
 */
using hf_bluetooth_data_callback_t = std::function<void(const hf_bluetooth_address_t& address, 
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
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Initialize(hf_bluetooth_mode_t mode) = 0;

  /**
   * @brief Deinitialize the Bluetooth subsystem
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Deinitialize() = 0;

  /**
   * @brief Check if Bluetooth is initialized
   * @return true if initialized, false otherwise
   */
  virtual bool IsInitialized() const = 0;

  /**
   * @brief Enable Bluetooth
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Enable() = 0;

  /**
   * @brief Disable Bluetooth
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Disable() = 0;

  /**
   * @brief Check if Bluetooth is enabled
   * @return true if enabled, false otherwise
   */
  virtual bool IsEnabled() const = 0;

  /**
   * @brief Set Bluetooth operating mode
   * @param mode Bluetooth operating mode
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t SetMode(hf_bluetooth_mode_t mode) = 0;

  /**
   * @brief Get current Bluetooth operating mode
   * @return Current Bluetooth mode
   */
  virtual hf_bluetooth_mode_t GetMode() const = 0;

  // ========== Device Management ==========

  /**
   * @brief Get local Bluetooth address
   * @param address Reference to store local address
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t GetLocalAddress(hf_bluetooth_address_t& address) const = 0;

  /**
   * @brief Set local device name
   * @param name Device name
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t SetDeviceName(const std::string& name) = 0;

  /**
   * @brief Get local device name
   * @return Device name string
   */
  virtual std::string GetDeviceName() const = 0;

  // ========== Classic Bluetooth Operations ==========

  /**
   * @brief Configure Bluetooth Classic parameters
   * @param config Classic configuration
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t ConfigureClassic(const hf_bluetooth_classic_config_t& config) = 0;

  /**
   * @brief Make device discoverable
   * @param discoverable True to make discoverable, false otherwise
   * @param timeout_ms Discoverable timeout in milliseconds (0 for indefinite)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t SetDiscoverable(bool discoverable, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief Check if device is discoverable
   * @return true if discoverable, false otherwise
   */
  virtual bool IsDiscoverable() const = 0;

  // ========== BLE Operations ==========

  /**
   * @brief Configure Bluetooth Low Energy parameters
   * @param config BLE configuration
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t ConfigureBle(const hf_bluetooth_ble_config_t& config) = 0;

  /**
   * @brief Start BLE advertising
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t StartAdvertising() = 0;

  /**
   * @brief Stop BLE advertising
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t StopAdvertising() = 0;

  /**
   * @brief Check if BLE advertising is active
   * @return true if advertising, false otherwise
   */
  virtual bool IsAdvertising() const = 0;

  // ========== Device Discovery ==========

  /**
   * @brief Start device discovery/scanning
   * @param duration_ms Scan duration in milliseconds (0 for indefinite)
   * @param type Scan type (BLE only)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t StartScan(uint32_t duration_ms = 0, hf_bluetooth_scan_type_t type = hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE) = 0;

  /**
   * @brief Stop device discovery/scanning
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t StopScan() = 0;

  /**
   * @brief Check if scanning is in progress
   * @return true if scanning, false otherwise
   */
  virtual bool IsScanning() const = 0;

  /**
   * @brief Get discovered devices
   * @param devices Vector to store discovered devices
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t GetDiscoveredDevices(std::vector<hf_bluetooth_device_info_t>& devices) = 0;

  /**
   * @brief Clear discovered devices list
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t ClearDiscoveredDevices() = 0;

  // ========== Connection Management ==========

  /**
   * @brief Connect to a remote device
   * @param address Remote device address
   * @param timeout_ms Connection timeout in milliseconds (0 for default)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Connect(const hf_bluetooth_address_t& address, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief Disconnect from a remote device
   * @param address Remote device address
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Disconnect(const hf_bluetooth_address_t& address) = 0;

  /**
   * @brief Check if connected to a device
   * @param address Remote device address
   * @return true if connected, false otherwise
   */
  virtual bool IsConnected(const hf_bluetooth_address_t& address) const = 0;

  /**
   * @brief Get list of connected devices
   * @param devices Vector to store connected devices
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t GetConnectedDevices(std::vector<hf_bluetooth_device_info_t>& devices) = 0;

  // ========== Pairing and Bonding ==========

  /**
   * @brief Pair with a remote device
   * @param address Remote device address
   * @param pin PIN code (for Classic, empty for BLE)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Pair(const hf_bluetooth_address_t& address, const std::string& pin = "") = 0;

  /**
   * @brief Unpair from a remote device
   * @param address Remote device address
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t Unpair(const hf_bluetooth_address_t& address) = 0;

  /**
   * @brief Check if paired with a device
   * @param address Remote device address
   * @return true if paired, false otherwise
   */
  virtual bool IsPaired(const hf_bluetooth_address_t& address) const = 0;

  /**
   * @brief Get list of paired devices
   * @param devices Vector to store paired devices
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t GetPairedDevices(std::vector<hf_bluetooth_device_info_t>& devices) = 0;

  // ========== Data Transmission ==========

  /**
   * @brief Send data to a connected device
   * @param address Remote device address
   * @param data Data to send
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t SendData(const hf_bluetooth_address_t& address, const std::vector<uint8_t>& data) = 0;

  /**
   * @brief Check if data is available to read
   * @param address Remote device address
   * @return Number of bytes available, or -1 on error
   */
  virtual int GetAvailableData(const hf_bluetooth_address_t& address) const = 0;

  /**
   * @brief Read available data from a connected device
   * @param address Remote device address
   * @param data Vector to store received data
   * @param max_bytes Maximum bytes to read (0 for all available)
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t ReadData(const hf_bluetooth_address_t& address, std::vector<uint8_t>& data, size_t max_bytes = 0) = 0;

  // ========== GATT Operations (BLE) ==========

  /**
   * @brief Discover GATT services on a connected device
   * @param address Remote device address
   * @param services Vector to store discovered services
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t DiscoverServices(const hf_bluetooth_address_t& address, std::vector<hf_bluetooth_gatt_service_t>& services) = 0;

  /**
   * @brief Discover GATT characteristics for a service
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristics Vector to store discovered characteristics
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t DiscoverCharacteristics(const hf_bluetooth_address_t& address, 
                                                     const std::string& service_uuid,
                                                     std::vector<hf_bluetooth_gatt_characteristic_t>& characteristics) = 0;

  /**
   * @brief Read GATT characteristic value
   * @param address Remote device address
   * @param service_uuid Service UUID
   * @param characteristic_uuid Characteristic UUID
   * @param value Vector to store characteristic value
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t ReadCharacteristic(const hf_bluetooth_address_t& address,
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
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t WriteCharacteristic(const hf_bluetooth_address_t& address,
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
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t SubscribeCharacteristic(const hf_bluetooth_address_t& address,
                                                     const std::string& service_uuid,
                                                  const std::string& characteristic_uuid,
                                                  bool enable) = 0;

  // ========== State and Status ==========

  /**
   * @brief Get current Bluetooth state
   * @return Current Bluetooth state
   */
  virtual hf_bluetooth_state_t GetState() const = 0;

  /**
   * @brief Get signal strength for a connected device
   * @param address Remote device address
   * @return Signal strength in dBm, or INT8_MIN on error
   */
  virtual int8_t GetRssi(const hf_bluetooth_address_t& address) const = 0;

  // ========== Event Handling ==========

  /**
   * @brief Register event callback
   * @param callback Event callback function
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t RegisterEventCallback(hf_bluetooth_event_callback_t callback) = 0;

  /**
   * @brief Unregister event callback
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t UnregisterEventCallback() = 0;

  /**
   * @brief Register data received callback
   * @param callback Data callback function
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t RegisterDataCallback(hf_bluetooth_data_callback_t callback) = 0;

  /**
   * @brief Unregister data received callback
   * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
   */
  virtual hf_bluetooth_err_t UnregisterDataCallback() = 0;

  // ========== Utility Functions ==========

  /**
   * @brief Get error description string
   * @param error Error code
   * @return Error description string
   */
  static const char* GetErrorString(hf_bluetooth_err_t error);

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
inline const char* BaseBluetooth::GetErrorString(hf_bluetooth_err_t error) {
#define X(name, value, desc) case hf_bluetooth_err_t::name: return desc;
  switch (error) {
    HF_BLUETOOTH_ERR_LIST(X)
    default: return "Unknown error";
  }
#undef X
}

/**
 * @ingroup bluetooth
 * @brief Helper function implementations for hf_bluetooth_address_t
 */
inline std::string hf_bluetooth_address_t::ToString() const {
  char buffer[18];
  snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return std::string(buffer);
}

inline bool hf_bluetooth_address_t::FromString(const std::string& addr_str) {
  if (addr_str.length() != 17) return false;
  
  // Validate format: XX:XX:XX:XX:XX:XX
  for (size_t i = 0; i < addr_str.length(); ++i) {
    if (i % 3 == 2) {
      if (addr_str[i] != ':') return false;
    } else {
      if (!std::isxdigit(addr_str[i])) return false;
    }
  }
  
  // Parse using no-exceptions approach with manual hex conversion
  for (int i = 0; i < 6; i++) {
    size_t pos = i * 3;
    const char* hex_start = addr_str.c_str() + pos;
    
    // Manual hex conversion without exceptions
    unsigned int value = 0;
    for (int j = 0; j < 2; j++) {
      char c = hex_start[j];
      if (c >= '0' && c <= '9') {
        value = (value << 4) + (c - '0');
      } else if (c >= 'A' && c <= 'F') {
        value = (value << 4) + (c - 'A' + 10);
      } else if (c >= 'a' && c <= 'f') {
        value = (value << 4) + (c - 'a' + 10);
      } else {
        return false; // Invalid hex character
      }
    }
    
    if (value > 255) return false;
    addr[i] = static_cast<uint8_t>(value);
  }
  
  return true;
}

inline bool hf_bluetooth_address_t::IsValid() const {
  for (int i = 0; i < 6; i++) {
    if (addr[i] != 0) return true;
  }
  return false;
}