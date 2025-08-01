/**
 * @file EspBluetooth.h
 * @ingroup bluetooth
 * @brief ESP32 Bluetooth implementation using NimBLE for BLE-only support (ESP32C6)
 *
 * This file contains the ESP32-specific implementation of the Bluetooth interface
 * using NimBLE stack for ESP32C6 BLE-only support on ESP-IDF v5.5.
 * 
 * ESP32C6 supports Bluetooth LE 5.0 and is certified for Bluetooth LE 5.3.
 * This implementation provides:
 * - BLE advertising and scanning
 * - GATT client and server operations
 * - Connection management
 * - Service and characteristic handling
 * - Proper conditional compilation for different ESP32 variants
 *
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is specifically optimized for ESP32C6 using NimBLE
 * @note For ESP32 and ESP32S3, both Classic BT and BLE are supported
 * @note For ESP32C3/H2, BLE-only using Bluedroid
 * @note ESP32S2 has no Bluetooth support
 */

#pragma once

#include "BaseBluetooth.h"
#include "HardwareTypes.h"

// ESP-IDF includes
#include "esp_log.h"
#include "nvs_flash.h"

// ESP32 variant detection and feature matrix
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)
// Full Classic BT + BLE support
#define HAS_CLASSIC_BLUETOOTH 1
#define HAS_BLE_SUPPORT 1
#define HAS_A2DP_SUPPORT 1
#define HAS_AVRCP_SUPPORT 1
#define HAS_SPP_SUPPORT 1
#if defined(CONFIG_BT_NIMBLE_ENABLED)
#define HAS_NIMBLE_SUPPORT 1
#define HAS_BLUEDROID_SUPPORT 0
#else
#define HAS_NIMBLE_SUPPORT 0
#define HAS_BLUEDROID_SUPPORT 1
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
// BLE-only with NimBLE (preferred for ESP32C6)
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_BLE_SUPPORT 1
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_SPP_SUPPORT 0
#if defined(CONFIG_BT_NIMBLE_ENABLED)
#define HAS_NIMBLE_SUPPORT 1
#define HAS_BLUEDROID_SUPPORT 0
#else
#define HAS_NIMBLE_SUPPORT 0
#define HAS_BLUEDROID_SUPPORT 1
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32H2)
// BLE-only with Bluedroid
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_BLE_SUPPORT 1
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_SPP_SUPPORT 0
#if defined(CONFIG_BT_NIMBLE_ENABLED)
#define HAS_NIMBLE_SUPPORT 1
#define HAS_BLUEDROID_SUPPORT 0
#else
#define HAS_NIMBLE_SUPPORT 0
#define HAS_BLUEDROID_SUPPORT 1
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
// No Bluetooth support
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_BLE_SUPPORT 0
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_SPP_SUPPORT 0
#define HAS_NIMBLE_SUPPORT 0
#define HAS_BLUEDROID_SUPPORT 0
#else
// Default to no Classic Bluetooth for unknown targets
#define HAS_CLASSIC_BLUETOOTH 0
#define HAS_BLE_SUPPORT 1
#define HAS_A2DP_SUPPORT 0
#define HAS_AVRCP_SUPPORT 0
#define HAS_SPP_SUPPORT 0
#define HAS_NIMBLE_SUPPORT 0
#define HAS_BLUEDROID_SUPPORT 1
#endif

// Conditional includes based on target capabilities
#if HAS_BLE_SUPPORT

#if HAS_NIMBLE_SUPPORT
// NimBLE headers for ESP32C6 (ESP-IDF v5.5+)
// Check if NimBLE headers are actually available
#ifdef CONFIG_BT_NIMBLE_ENABLED
#if __has_include("nimble/nimble_port.h")
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_att.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#define NIMBLE_HEADERS_AVAILABLE 1
#else
#warning "NimBLE enabled but headers not found - disabling NimBLE functionality"
#define NIMBLE_HEADERS_AVAILABLE 0
#endif
#else
#define NIMBLE_HEADERS_AVAILABLE 0
#endif
#else
#define NIMBLE_HEADERS_AVAILABLE 0
#endif

#if HAS_BLUEDROID_SUPPORT
// Bluedroid BLE headers for other targets
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"
#endif

#if HAS_CLASSIC_BLUETOOTH
// Classic Bluetooth headers (ESP32/ESP32S3 only)
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_hf_client_api.h"
#if HAS_A2DP_SUPPORT
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#endif
#endif

#endif // HAS_BLE_SUPPORT

#include <map>
#include <mutex>
#include <vector>
#include <functional>

/**
 * @ingroup bluetooth
 * @brief ESP32 Bluetooth implementation class
 * 
 * This class provides ESP32-specific Bluetooth functionality with conditional
 * compilation for different ESP32 variants:
 * - ESP32C6: BLE-only using NimBLE (optimized)
 * - ESP32/ESP32S3: Full Bluetooth with Classic + BLE
 * - ESP32C3/H2: BLE-only using Bluedroid
 * - ESP32S2: No Bluetooth support
 */
class EspBluetooth : public BaseBluetooth {
private:
    // Internal state
    bool m_initialized;
    bool m_enabled;
    hf_bluetooth_mode_t m_mode;
    hf_bluetooth_state_t m_state;
    
    // Device management
    std::vector<hf_bluetooth_device_info_t> m_discovered_devices;
    std::map<std::string, hf_bluetooth_device_info_t> m_connected_devices;
    
    // Configuration
    hf_bluetooth_ble_config_t m_ble_config;
    
    // Synchronization
    mutable std::mutex m_device_mutex;
    mutable std::mutex m_state_mutex;
    
    // Callback functions
    hf_bluetooth_event_callback_t m_event_callback;
    hf_bluetooth_data_callback_t m_data_callback;
    void* m_callback_context;

#if HAS_NIMBLE_SUPPORT
    // NimBLE-specific members
    static EspBluetooth* s_instance;
    uint16_t m_conn_handle;
    uint8_t m_addr_type;
    
    // NimBLE event handlers
    static int GapEventHandler(struct ble_gap_event *event, void *arg);
    static int GattSvrCharAccess(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);
    
    // NimBLE utility functions
    hf_bluetooth_err_t InitializeNimBLE();
    hf_bluetooth_err_t DeinitializeNimBLE();
    hf_bluetooth_err_t StartScanning();
    hf_bluetooth_err_t StopScanning();
    
    // Address conversion utilities
#if NIMBLE_HEADERS_AVAILABLE
    static void ConvertBleAddr(const ble_addr_t* ble_addr, hf_bluetooth_address_t& hf_addr);
    static void ConvertHfAddr(const hf_bluetooth_address_t& hf_addr, ble_addr_t* ble_addr);
#endif
#endif

#if HAS_BLUEDROID_SUPPORT
    // Bluedroid-specific members and methods
    hf_bluetooth_err_t InitializeBluedroid();
    hf_bluetooth_err_t DeinitializeBluedroid();
    
    // Bluedroid event handlers
    static void GapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static void GattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                 esp_ble_gattc_cb_param_t *param);
    static void GattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                 esp_ble_gatts_cb_param_t *param);
#endif

#if HAS_CLASSIC_BLUETOOTH
    // Classic Bluetooth methods
    hf_bluetooth_err_t InitializeClassic();
    hf_bluetooth_err_t DeinitializeClassic();
    
    // Classic BT event handlers
    static void ClassicGapEventHandler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
    static void SppEventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
#endif

    // Internal utility methods
    void TriggerEvent(hf_bluetooth_event_t event, const void* data = nullptr);
    hf_bluetooth_err_t ValidateAddress(const hf_bluetooth_address_t& address) const;
    std::string AddressToString(const hf_bluetooth_address_t& address) const;
    hf_bluetooth_address_t StringToAddress(const std::string& address_str) const;

public:
    /**
     * @brief Constructor
     */
    EspBluetooth();
    
    /**
     * @brief Destructor
     */
    virtual ~EspBluetooth();

    // ========== Initialization and Configuration ==========
    
    /**
     * @brief Initialize the Bluetooth subsystem
     * @param mode Bluetooth operating mode
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     * 
     * @note For ESP32C6, only BLE mode is supported
     */
    hf_bluetooth_err_t Initialize(hf_bluetooth_mode_t mode) override;
    
    /**
     * @brief Deinitialize the Bluetooth subsystem
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Deinitialize() override;
    
    /**
     * @brief Check if Bluetooth is initialized
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const override;
    
    /**
     * @brief Enable Bluetooth
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Enable() override;
    
    /**
     * @brief Disable Bluetooth
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Disable() override;
    
    /**
     * @brief Check if Bluetooth is enabled
     * @return true if enabled, false otherwise
     */
    bool IsEnabled() const override;
    
    /**
     * @brief Set Bluetooth operating mode
     * @param mode Bluetooth operating mode
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t SetMode(hf_bluetooth_mode_t mode) override;
    
    /**
     * @brief Get current Bluetooth operating mode
     * @return Current Bluetooth mode
     */
    hf_bluetooth_mode_t GetMode() const override;

    // ========== Device Management ==========
    
    /**
     * @brief Get local Bluetooth address
     * @param address Reference to store the local address
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t GetLocalAddress(hf_bluetooth_address_t& address) const override;
    
    /**
     * @brief Set local device name
     * @param name Device name string
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t SetDeviceName(const std::string& name) override;
    
    /**
     * @brief Get local device name
     * @return Device name string
     */
    std::string GetDeviceName() const override;

    // ========== Discovery and Scanning ==========
    
    /**
     * @brief Start device discovery/scanning
     * @param duration_ms Scan duration in milliseconds (0 for indefinite)
     * @param type Scan type (BLE only)
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t StartScan(
        uint32_t duration_ms = 0,
        hf_bluetooth_scan_type_t type = hf_bluetooth_scan_type_t::HF_BLUETOOTH_SCAN_TYPE_ACTIVE) override;
    
    /**
     * @brief Stop device discovery/scanning
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t StopScan() override;
    
    /**
     * @brief Check if currently scanning
     * @return true if scanning, false otherwise
     */
    bool IsScanning() const override;
    
    /**
     * @brief Get list of discovered devices
     * @param devices Vector to store discovered devices
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t GetDiscoveredDevices(std::vector<hf_bluetooth_device_info_t>& devices) override;
    
    /**
     * @brief Clear discovered devices list
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t ClearDiscoveredDevices() override;

    // ========== Advertising (BLE) ==========
    
    /**
     * @brief Start BLE advertising
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t StartAdvertising() override;
    
    /**
     * @brief Stop BLE advertising
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t StopAdvertising() override;
    
    /**
     * @brief Check if currently advertising
     * @return true if advertising, false otherwise
     */
    bool IsAdvertising() const override;

    // ========== Connection Management ==========
    
    /**
     * @brief Connect to a remote device
     * @param address Remote device address
     * @param timeout_ms Connection timeout in milliseconds (0 for default)
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Connect(const hf_bluetooth_address_t& address, uint32_t timeout_ms = 0) override;
    
    /**
     * @brief Disconnect from a remote device
     * @param address Remote device address
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Disconnect(const hf_bluetooth_address_t& address) override;
    
    /**
     * @brief Check if connected to a device
     * @param address Remote device address
     * @return true if connected, false otherwise
     */
    bool IsConnected(const hf_bluetooth_address_t& address) const override;
    
    /**
     * @brief Get list of connected devices
     * @param devices Vector to store connected devices
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t GetConnectedDevices(std::vector<hf_bluetooth_device_info_t>& devices) override;

    // ========== Pairing and Bonding ==========
    
    /**
     * @brief Pair with a remote device
     * @param address Remote device address
     * @param pin PIN code (for Classic, empty for BLE)
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Pair(const hf_bluetooth_address_t& address, const std::string& pin = "") override;
    
    /**
     * @brief Unpair from a remote device
     * @param address Remote device address
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t Unpair(const hf_bluetooth_address_t& address) override;
    
    /**
     * @brief Check if paired with a device
     * @param address Remote device address
     * @return true if paired, false otherwise
     */
    bool IsPaired(const hf_bluetooth_address_t& address) const override;

    // ========== Data Transfer ==========
    
    /**
     * @brief Send data to a connected device
     * @param address Remote device address
     * @param data Data to send
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t SendData(const hf_bluetooth_address_t& address, const std::vector<uint8_t>& data) override;
    
    /**
     * @brief Check if data is available to read
     * @param address Remote device address
     * @return Number of bytes available, or -1 on error
     */
    int GetAvailableData(const hf_bluetooth_address_t& address) const override;
    
    /**
     * @brief Read available data from a connected device
     * @param address Remote device address
     * @param data Vector to store received data
     * @param max_bytes Maximum bytes to read (0 for all available)
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t ReadData(const hf_bluetooth_address_t& address, std::vector<uint8_t>& data, size_t max_bytes = 0) override;

    // ========== GATT Operations (BLE) ==========
    
    /**
     * @brief Discover GATT services on a connected device
     * @param address Remote device address
     * @param services Vector to store discovered services
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t DiscoverServices(const hf_bluetooth_address_t& address, std::vector<hf_bluetooth_gatt_service_t>& services) override;
    
    /**
     * @brief Discover GATT characteristics for a service
     * @param address Remote device address
     * @param service_uuid Service UUID
     * @param characteristics Vector to store discovered characteristics
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t DiscoverCharacteristics(const hf_bluetooth_address_t& address, const std::string& service_uuid, std::vector<hf_bluetooth_gatt_characteristic_t>& characteristics) override;
    
    /**
     * @brief Read GATT characteristic value
     * @param address Remote device address
     * @param service_uuid Service UUID
     * @param characteristic_uuid Characteristic UUID
     * @param value Vector to store characteristic value
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t ReadCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, std::vector<uint8_t>& value) override;
    
    /**
     * @brief Write GATT characteristic value
     * @param address Remote device address
     * @param service_uuid Service UUID
     * @param characteristic_uuid Characteristic UUID
     * @param value Value to write
     * @param with_response True to wait for write response
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t WriteCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, const std::vector<uint8_t>& value, bool with_response = true) override;
    
    /**
     * @brief Subscribe to GATT characteristic notifications
     * @param address Remote device address
     * @param service_uuid Service UUID
     * @param characteristic_uuid Characteristic UUID
     * @param enable True to enable notifications, false to disable
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t SubscribeCharacteristic(const hf_bluetooth_address_t& address, const std::string& service_uuid, const std::string& characteristic_uuid, bool enable) override;

    // ========== State and Status ==========
    
    /**
     * @brief Get current Bluetooth state
     * @return Current Bluetooth state
     */
    hf_bluetooth_state_t GetState() const override;
    
    /**
     * @brief Get signal strength for a connected device
     * @param address Remote device address
     * @return Signal strength in dBm, or INT8_MIN on error
     */
    int8_t GetRssi(const hf_bluetooth_address_t& address) const override;

    // ========== Event Handling ==========
    
    /**
     * @brief Register event callback function
     * @param callback Callback function
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t RegisterEventCallback(hf_bluetooth_event_callback_t callback) override;
    
    /**
     * @brief Register data callback function
     * @param callback Callback function
     * @return hf_bluetooth_err_t::BLUETOOTH_SUCCESS on success, error code otherwise
     */
    hf_bluetooth_err_t RegisterDataCallback(hf_bluetooth_data_callback_t callback) override;

    // ========== Utility Methods ==========
    
    /**
     * @brief Get implementation-specific information
     * @return String containing implementation details
     */
    std::string GetImplementationInfo() const;
    
    /**
     * @brief Get supported features for current target
     * @return Bitmask of supported features
     */
    uint32_t GetSupportedFeatures() const;
};

#if !HAS_BLE_SUPPORT
/**
 * @brief Stub implementation for targets without Bluetooth support (ESP32S2)
 */
class EspBluetoothStub : public BaseBluetooth {
public:
    // All methods return NOT_SUPPORTED error
    hf_bluetooth_err_t Initialize(hf_bluetooth_mode_t mode) override { 
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED; 
    }
    
    hf_bluetooth_err_t Deinitialize() override { 
        return hf_bluetooth_err_t::BLUETOOTH_ERR_NOT_SUPPORTED; 
    }
    
    bool IsInitialized() const override { return false; }
    
    // ... (other methods would return appropriate errors)
    // For brevity, showing only key methods
};

// Use stub for ESP32S2
using EspBluetoothImpl = EspBluetoothStub;
#else
// Use full implementation for other targets
using EspBluetoothImpl = EspBluetooth;
#endif