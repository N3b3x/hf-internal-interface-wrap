/**
 * @file EspBluetooth.h
 * @brief Advanced ESP32-C6 implementation of the unified BaseBluetooth class with ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseBluetooth class
 * for ESP32-C6 microcontrollers with support for Bluetooth 5.0 LE and NimBLE host stack.
 * It supports device discovery, pairing, GATT operations, and advanced ESP32-C6-specific
 * features like extended advertising, 2M PHY, coded PHY, and power management.
 * The implementation includes comprehensive event handling and hardware-accelerated operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation requires ESP32-C6 with ESP-IDF v5.5+ for full feature support.
 * @note Thread-safe implementation with proper synchronization mechanisms.
 */

#pragma once

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// Only include NimBLE headers that are available on ESP32-C6
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/util/util.h"
#include "host/ble_store.h"

#ifdef __cplusplus
}
#endif

// C++ headers
#include "BaseBluetooth.h"
#include "mcu/esp32/utils/EspTypes_Bluetooth.h"
#include <atomic>
#include <memory>
#include <vector>

/**
 * @class EspBluetooth
 * @brief Advanced ESP32-C6 implementation of unified BaseBluetooth with ESP-IDF v5.5+ features.
 * @details This class provides a comprehensive implementation of BaseBluetooth for ESP32-C6
 *          microcontrollers with support for both basic and advanced features including:
 *
 *          **Basic Features:**
 *          - Bluetooth 5.0 LE with NimBLE host stack
 *          - Device discovery and scanning
 *          - Connection management and pairing
 *          - GATT client and server operations
 *          - Security and bonding management
 *
 *          **Advanced Features (ESP32-C6/ESP-IDF v5.5+):**
 *          - Extended advertising and scanning
 *          - 2M PHY and coded PHY support
 *          - Advanced power management
 *          - Hardware-accelerated cryptography
 *          - Multi-connection support
 *          - Custom GATT services and characteristics
 *
 * @note This class is designed for ESP32-C6 with NimBLE host stack.
 * @note Advanced features require ESP32-C6 with ESP-IDF v5.5+ for full functionality.
 */
class EspBluetooth : public BaseBluetooth {
public:
  //==============================================================//
  // CONSTRUCTORS
  //==============================================================//
  
  /**
   * @brief Constructor for EspBluetooth with configuration.
   * @param device_name Bluetooth device name
   * @param enable_classic Enable Bluetooth Classic (ESP32-C6 supports BLE only)
   * @param enable_ble Enable Bluetooth Low Energy
   * @details Creates an ESP32-C6 Bluetooth instance with the specified configuration.
   *          **LAZY INITIALIZATION**: Bluetooth is NOT started until the first call
   *          to Initialize() or any Bluetooth operation.
   */
  explicit EspBluetooth(
      const std::string& device_name = "ESP32-C6-BLE",
      hf_bool_t enable_classic = false,  // ESP32-C6 supports BLE only
      hf_bool_t enable_ble = true) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup of Bluetooth resources.
   */
  ~EspBluetooth() override;

  //==============================================================//
  // BASEBLUETOOTH IMPLEMENTATION
  //==============================================================//

  /**
   * @brief Initialize Bluetooth adapter with specified mode.
   * @param mode Bluetooth mode (BLE only for ESP32-C6)
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Initialize(hf_bluetooth_mode_t mode) override;

  /**
   * @brief Deinitialize Bluetooth adapter and cleanup resources.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Deinitialize() override;

  /**
   * @brief Check if Bluetooth is initialized.
   * @return true if initialized, false otherwise
   */
  hf_bool_t IsInitialized() const noexcept override;

  /**
   * @brief Enable Bluetooth adapter.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Enable() override;

  /**
   * @brief Disable Bluetooth adapter.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Disable() override;

  /**
   * @brief Check if Bluetooth is enabled.
   * @return true if enabled, false otherwise
   */
  hf_bool_t IsEnabled() const noexcept override;

  /**
   * @brief Get current Bluetooth mode.
   * @return Current Bluetooth mode
   */
  hf_bluetooth_mode_t GetMode() const noexcept override;

  /**
   * @brief Set device name for Bluetooth.
   * @param name Device name to set
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetDeviceName(const std::string& name) override;

  /**
   * @brief Get current device name.
   * @return Current device name
   */
  std::string GetDeviceName() const noexcept override;

  /**
   * @brief Get device MAC address.
   * @param address Output buffer for MAC address
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetDeviceAddress(hf_bluetooth_address_t& address) const override;

  /**
   * @brief Start device discovery scan.
   * @param duration_ms Scan duration in milliseconds (0 = infinite)
   * @param scan_type Type of scan (active/passive)
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StartScan(hf_timeout_ms_t duration_ms = 0, 
                               hf_bluetooth_scan_type_t scan_type = hf_bluetooth_scan_type_t::ACTIVE) override;

  /**
   * @brief Stop device discovery scan.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StopScan() override;

  /**
   * @brief Check if currently scanning.
   * @return true if scanning, false otherwise
   */
  hf_bool_t IsScanning() const noexcept override;

  /**
   * @brief Connect to a device.
   * @param address Target device address
   * @param timeout_ms Connection timeout in milliseconds
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Connect(const hf_bluetooth_address_t& address, 
                             hf_timeout_ms_t timeout_ms = 10000) override;

  /**
   * @brief Disconnect from a device.
   * @param address Target device address
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t Disconnect(const hf_bluetooth_address_t& address) override;

  /**
   * @brief Check if connected to a device.
   * @param address Device address to check
   * @return true if connected, false otherwise
   */
  hf_bool_t IsConnected(const hf_bluetooth_address_t& address) const override;

  /**
   * @brief Get list of connected devices.
   * @param devices Output vector for connected devices
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetConnectedDevices(std::vector<hf_bluetooth_device_info_t>& devices) const override;

  /**
   * @brief Start pairing with a device.
   * @param address Target device address
   * @param security_level Required security level
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StartPairing(const hf_bluetooth_address_t& address, 
                                  hf_bluetooth_security_t security_level) override;

  /**
   * @brief Cancel ongoing pairing process.
   * @param address Target device address
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t CancelPairing(const hf_bluetooth_address_t& address) override;

  /**
   * @brief Remove bonded device.
   * @param address Target device address
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t RemoveBond(const hf_bluetooth_address_t& address) override;

  /**
   * @brief Get list of bonded devices.
   * @param devices Output vector for bonded devices
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetBondedDevices(std::vector<hf_bluetooth_device_info_t>& devices) const override;

  //==============================================================//
  // BLE GATT OPERATIONS
  //==============================================================//

  /**
   * @brief Discover services on connected device.
   * @param address Target device address
   * @param services Output vector for discovered services
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t DiscoverServices(const hf_bluetooth_address_t& address,
                                      std::vector<hf_bluetooth_service_t>& services) override;

  /**
   * @brief Read characteristic value.
   * @param address Target device address
   * @param service_uuid Service UUID
   * @param char_uuid Characteristic UUID
   * @param data Output buffer for read data
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ReadCharacteristic(const hf_bluetooth_address_t& address,
                                        const hf_bluetooth_uuid_t& service_uuid,
                                        const hf_bluetooth_uuid_t& char_uuid,
                                        std::vector<hf_u8_t>& data) override;

  /**
   * @brief Write characteristic value.
   * @param address Target device address
   * @param service_uuid Service UUID
   * @param char_uuid Characteristic UUID
   * @param data Data to write
   * @param write_type Write type (with/without response)
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t WriteCharacteristic(const hf_bluetooth_address_t& address,
                                         const hf_bluetooth_uuid_t& service_uuid,
                                         const hf_bluetooth_uuid_t& char_uuid,
                                         const std::vector<hf_u8_t>& data,
                                         hf_bluetooth_write_type_t write_type) override;

  /**
   * @brief Subscribe to characteristic notifications.
   * @param address Target device address
   * @param service_uuid Service UUID
   * @param char_uuid Characteristic UUID
   * @param enable Enable or disable notifications
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SubscribeCharacteristic(const hf_bluetooth_address_t& address,
                                             const hf_bluetooth_uuid_t& service_uuid,
                                             const hf_bluetooth_uuid_t& char_uuid,
                                             hf_bool_t enable) override;

  //==============================================================//
  // EVENT HANDLING
  //==============================================================//

  /**
   * @brief Register event callback function.
   * @param callback Callback function to register
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t RegisterEventCallback(hf_bluetooth_event_callback_t callback) override;

  /**
   * @brief Unregister event callback function.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t UnregisterEventCallback() override;

  //==============================================================//
  // ESP32-C6 SPECIFIC EXTENSIONS
  //==============================================================//

  /**
   * @brief Set Bluetooth TX power level.
   * @param power_level Power level in dBm (-12 to +9 for ESP32-C6)
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetTxPowerLevel(hf_i8_t power_level);

  /**
   * @brief Get current TX power level.
   * @return Current TX power level in dBm
   */
  hf_i8_t GetTxPowerLevel() const noexcept;

  /**
   * @brief Enable/disable extended advertising (Bluetooth 5.0).
   * @param enable Enable extended advertising
   * @param params Extended advertising parameters
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetExtendedAdvertising(hf_bool_t enable, 
                                            const hf_esp_ble_ext_adv_params_t& params);

  /**
   * @brief Set PHY preferences for connections.
   * @param tx_phy Preferred TX PHY
   * @param rx_phy Preferred RX PHY
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t SetPreferredPhy(hf_esp_ble_phy_t tx_phy, hf_esp_ble_phy_t rx_phy);

  /**
   * @brief Start advertising with custom data.
   * @param adv_data Advertisement data
   * @param scan_rsp_data Scan response data (optional)
   * @param params Advertising parameters
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StartAdvertising(const std::vector<hf_u8_t>& adv_data,
                                      const std::vector<hf_u8_t>& scan_rsp_data,
                                      const hf_esp_ble_adv_params_t& params);

  /**
   * @brief Stop advertising.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t StopAdvertising();

  /**
   * @brief Check if currently advertising.
   * @return true if advertising, false otherwise
   */
  hf_bool_t IsAdvertising() const noexcept;

  /**
   * @brief Get Bluetooth statistics.
   * @param stats Output structure for statistics
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t GetStatistics(hf_esp_bluetooth_stats_t& stats) const;

private:
  //==============================================================//
  // INTERNAL STATE MANAGEMENT
  //==============================================================//

  mutable std::mutex m_mutex_;                           ///< Thread synchronization mutex
  std::atomic<hf_bool_t> m_initialized_;                 ///< Initialization state
  std::atomic<hf_bool_t> m_enabled_;                     ///< Enable state
  std::atomic<hf_bool_t> m_scanning_;                    ///< Scanning state
  std::atomic<hf_bool_t> m_advertising_;                 ///< Advertising state
  hf_bluetooth_mode_t m_mode_;                           ///< Current Bluetooth mode
  std::string m_device_name_;                            ///< Device name
  hf_bluetooth_address_t m_device_address_;              ///< Device MAC address
  hf_i8_t m_tx_power_level_;                            ///< TX power level

  // Event handling
  hf_bluetooth_event_callback_t m_event_callback_;       ///< User event callback
  void* m_event_user_data_;                              ///< User data for callbacks

  // Connected devices tracking
  std::vector<hf_bluetooth_device_info_t> m_connected_devices_;
  std::vector<hf_bluetooth_device_info_t> m_bonded_devices_;

  // NimBLE host stack handles
  struct ble_gap_event_listener m_gap_listener_;
  struct ble_gatt_svc_def* m_gatt_services_;

  //==============================================================//
  // INTERNAL HELPER METHODS
  //==============================================================//

  /**
   * @brief Initialize NimBLE host stack.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t InitializeNimble();

  /**
   * @brief Deinitialize NimBLE host stack.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t DeinitializeNimble();

  /**
   * @brief Configure default GATT services.
   * @return HF_BLUETOOTH_SUCCESS on success, error code otherwise
   */
  hf_bluetooth_err_t ConfigureGattServices();

  /**
   * @brief Handle GAP events from NimBLE.
   * @param event GAP event structure
   * @param arg User argument
   * @return 0 on success, error code otherwise
   */
  static int HandleGapEvent(struct ble_gap_event* event, void* arg);

  /**
   * @brief Handle GATT events from NimBLE.
   * @param event GATT event structure
   * @param arg User argument
   * @return 0 on success, error code otherwise
   */
  static int HandleGattEvent(struct ble_gatt_event* event, void* arg);

  /**
   * @brief Convert ESP error to HardFOC error.
   * @param esp_err ESP-IDF error code
   * @return HardFOC Bluetooth error code
   */
  static hf_bluetooth_err_t ConvertEspError(esp_err_t esp_err);

  /**
   * @brief Validate device address format.
   * @param address Address to validate
   * @return true if valid, false otherwise
   */
  static hf_bool_t IsValidDeviceAddress(const hf_bluetooth_address_t& address);

  // Prevent copying
  EspBluetooth(const EspBluetooth&) = delete;
  EspBluetooth& operator=(const EspBluetooth&) = delete;
};