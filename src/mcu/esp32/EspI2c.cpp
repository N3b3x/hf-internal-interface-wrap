/**
 * @file EspI2c.cpp
 * @brief Implementation of ESP32-integrated I2C controller with proper bus-device architecture.
 *
 * This file provides the implementation for I2C bus communication using the ESP32's
 * built-in I2C peripheral with full ESP-IDF v5.5+ capabilities and proper separation
 * of bus and device management following the same pattern as EspSpi.
 *
 * Key architectural improvements:
 * - Proper bus-device separation matching ESP-IDF v5.5+ API design
 * - EspI2cBus manages the I2C bus and device creation
 * - EspI2cDevice represents individual devices and inherits from BaseI2c
 * - Thread-safe operations with proper resource management
 * - Comprehensive error handling with ESP-IDF error conversion
 * - Per-device statistics and diagnostics
 * - Automatic resource cleanup and lifecycle management
 *
 * The implementation closely follows ESP-IDF v5.5 I2C Master driver documentation:
 * https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/i2c.html
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Fully compliant with ESP-IDF v5.5 I2C Master driver API
 * @note Optimized for ESP32C6 hardware capabilities
 * @note Thread-safe for multi-device I2C bus management
 * @note Follows the same architectural pattern as EspSpi
 */

#include "EspI2c.h"
#include "utils/memory_utils.h"
#include <algorithm>
#include <cstring>

// ESP-IDF additional includes for ESP32C6 features
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

static const char* TAG = "EspI2c";

//======================================================//
// ESP I2C BUS IMPLEMENTATION
//======================================================//

EspI2cBus::EspI2cBus(const hf_i2c_master_bus_config_t& config) noexcept
    : config_(config), bus_handle_(nullptr), initialized_(false) {
  ESP_LOGI(TAG, "Creating I2C bus on port %d", config_.i2c_port);
}

EspI2cBus::~EspI2cBus() noexcept {
  ESP_LOGI(TAG, "Destroying I2C bus");
  Deinitialize();
}

bool EspI2cBus::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_) {
    ESP_LOGW(TAG, "I2C bus already initialized");
    return true;
  }

  ESP_LOGI(TAG, "Initializing I2C bus on port %d", config_.i2c_port);

  // Create ESP-IDF bus configuration
  i2c_master_bus_config_t bus_cfg = {};
  bus_cfg.i2c_port = config_.i2c_port;
  bus_cfg.sda_io_num = static_cast<gpio_num_t>(config_.sda_io_num);
  bus_cfg.scl_io_num = static_cast<gpio_num_t>(config_.scl_io_num);
  
  // Map our clock source to ESP32-C6 clock source values
  // ESP32-C6: SOC_MOD_CLK_XTAL = 0, SOC_MOD_CLK_RC_FAST = 1
  if (config_.clk_source == hf_i2c_clock_source_t::HF_I2C_CLK_SRC_RC_FAST) {
    bus_cfg.clk_source = I2C_CLK_SRC_RC_FAST;  // Use RC_FAST for testing
  } else {
    bus_cfg.clk_source = I2C_CLK_SRC_XTAL;     // Default to XTAL (most stable)
  }
  
  bus_cfg.glitch_ignore_cnt = static_cast<uint8_t>(config_.glitch_ignore_cnt);
  bus_cfg.intr_priority = 0;  // Use default priority (1,2,3) instead of custom value
  bus_cfg.trans_queue_depth = config_.trans_queue_depth;

  // Configure flags
  bus_cfg.flags.enable_internal_pullup = config_.enable_internal_pullup;
  bus_cfg.flags.allow_pd = config_.allow_pd;

  // Create the master bus
  esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(err));
    return false;
  }

  initialized_ = true;
  ESP_LOGI(TAG, "I2C bus initialized successfully");
  return true;
}

bool EspI2cBus::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing I2C bus");

  // Remove all devices from ESP-IDF first
  for (auto& device : devices_) {
    if (device && device->GetHandle()) {
      ESP_LOGI(TAG, "Removing device 0x%02X from ESP-IDF bus", device->GetDeviceAddress());
      
      // Bus removes from ESP-IDF
      esp_err_t err = i2c_master_bus_rm_device(device->GetHandle());
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to remove device 0x%02X from ESP-IDF bus: %s", 
                 device->GetDeviceAddress(), esp_err_to_name(err));
        // Continue with other devices even if one fails
      } else {
        ESP_LOGI(TAG, "Successfully removed device 0x%02X from ESP-IDF bus", 
                 device->GetDeviceAddress());
      }
      
      // Mark device as deinitialized (don't call Deinitialize())
      device->MarkAsDeinitialized();
    }
  }
  
  // Clear device vector (destructors won't try to remove from ESP-IDF)
  devices_.clear();

  // Now delete the master bus (should succeed since all devices are removed)
  if (bus_handle_) {
    ESP_LOGI(TAG, "Deleting I2C master bus");
    esp_err_t err = i2c_del_master_bus(bus_handle_);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(err));
      // Force a delay to allow ESP-IDF to clean up internal state
      vTaskDelay(pdMS_TO_TICKS(100));
    } else {
      ESP_LOGI(TAG, "Successfully deleted I2C master bus");
    }
    bus_handle_ = nullptr;
  }
  
  // Force a delay to ensure ESP-IDF has fully released the bus
  vTaskDelay(pdMS_TO_TICKS(200));

  initialized_ = false;
  ESP_LOGI(TAG, "I2C bus deinitialized successfully");
  return true;
}

int EspI2cBus::CreateDevice(const hf_i2c_device_config_t& device_config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return -1;
  }

  ESP_LOGI(TAG, "Creating I2C device at address 0x%02X", device_config.device_address);

  // Check if device already exists
  if (FindDeviceIndexByAddress(device_config.device_address) >= 0) {
    ESP_LOGW(TAG, "Device at address 0x%02X already exists", device_config.device_address);
    return -1;
  }

  // Create ESP-IDF device configuration
  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = static_cast<i2c_addr_bit_len_t>(device_config.dev_addr_length);
  dev_cfg.device_address = device_config.device_address;
  dev_cfg.scl_speed_hz = device_config.scl_speed_hz;
  dev_cfg.scl_wait_us = device_config.scl_wait_us;
  dev_cfg.flags.disable_ack_check = device_config.disable_ack_check;

  // Add device to the bus
  i2c_master_dev_handle_t dev_handle;
  esp_err_t err = i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(err));
    return -1;
  }

  // Create EspI2cDevice instance using nothrow allocation
  auto device = hf::utils::make_unique_nothrow<EspI2cDevice>(this, dev_handle, device_config);
  if (!device) {
    ESP_LOGE(TAG, "Failed to allocate memory for EspI2cDevice");
    esp_err_t cleanup_err = i2c_master_bus_rm_device(dev_handle);
    if (cleanup_err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to remove I2C device during cleanup: %s", esp_err_to_name(cleanup_err));
    }
    return -1;
  }

  // Check if devices vector can accommodate new device
  if (devices_.size() >= devices_.max_size()) {
    ESP_LOGE(TAG, "Failed to add EspI2cDevice: maximum devices reached");
    i2c_master_bus_rm_device(dev_handle);
    return -1;
  }

  devices_.push_back(std::move(device));
  int device_index = devices_.size() - 1;

  ESP_LOGI(TAG, "I2C device created successfully at index %d", device_index);
  return device_index;
}

BaseI2c* EspI2cBus::GetDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    ESP_LOGW(TAG, "Invalid device index: %d", device_index);
    return nullptr;
  }

  return devices_[device_index].get();
}

const BaseI2c* EspI2cBus::GetDevice(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }

  return devices_[device_index].get();
}

EspI2cDevice* EspI2cBus::GetEspDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }

  return devices_[device_index].get();
}

const EspI2cDevice* EspI2cBus::GetEspDevice(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }

  return devices_[device_index].get();
}

BaseI2c* EspI2cBus::GetDeviceByAddress(hf_u16_t device_address) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  int index = FindDeviceIndexByAddress(device_address);
  if (index >= 0) {
    return devices_[index].get();
  }

  return nullptr;
}

std::size_t EspI2cBus::GetDeviceCount() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return devices_.size();
}

bool EspI2cBus::RemoveDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    ESP_LOGW(TAG, "Invalid device index for removal: %d", device_index);
    return false;
  }

  auto& device = devices_[device_index];
  if (!device) {
    ESP_LOGW(TAG, "Device at index %d is null", device_index);
    return false;
  }

  ESP_LOGI(TAG, "Removing I2C device at index %d (address 0x%02X)", 
           device_index, device->GetDeviceAddress());

  // Remove from ESP-IDF bus first
  if (device->GetHandle()) {
    esp_err_t err = i2c_master_bus_rm_device(device->GetHandle());
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to remove device 0x%02X from ESP-IDF bus: %s", 
               device->GetDeviceAddress(), esp_err_to_name(err));
      return false;
    }
    ESP_LOGI(TAG, "Successfully removed device 0x%02X from ESP-IDF bus", 
             device->GetDeviceAddress());
  }
  
  // Mark device as deinitialized
  device->MarkAsDeinitialized();

  // Remove from vector (destructor won't try to remove from ESP-IDF)
  devices_.erase(devices_.begin() + device_index);

  ESP_LOGI(TAG, "I2C device removed successfully from index %d", device_index);
  return true;
}

bool EspI2cBus::RemoveDeviceByAddress(hf_u16_t device_address) noexcept {
  int index = FindDeviceIndexByAddress(device_address);
  if (index >= 0) {
    ESP_LOGI(TAG, "Found device at address 0x%02X at index %d, removing...", device_address, index);
    return RemoveDevice(index);
  }

  ESP_LOGW(TAG, "Device at address 0x%02X not found for removal", device_address);
  return false;
}

const hf_i2c_master_bus_config_t& EspI2cBus::GetConfig() const noexcept {
  return config_;
}

i2c_master_bus_handle_t EspI2cBus::GetHandle() const noexcept {
  return bus_handle_;
}

int EspI2cBus::GetPort() const noexcept {
  return config_.i2c_port;
}

bool EspI2cBus::IsInitialized() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return initialized_;
}

size_t EspI2cBus::ScanDevices(std::vector<hf_u16_t>& found_devices, hf_u16_t start_addr,
                              hf_u16_t end_addr) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return 0;
  }

  found_devices.clear();
  ESP_LOGI(TAG, "Scanning I2C bus from 0x%02X to 0x%02X", start_addr, end_addr);

  for (hf_u16_t addr = start_addr; addr <= end_addr; ++addr) {
    if (ProbeDevice(addr)) {
      found_devices.push_back(addr);
      ESP_LOGI(TAG, "Found I2C device at address 0x%02X", addr);
    }
  }

  ESP_LOGI(TAG, "I2C bus scan completed. Found %zu devices", found_devices.size());
  return found_devices.size();
}

bool EspI2cBus::ProbeDevice(hf_u16_t device_addr) noexcept {
  if (!initialized_) {
    return false;
  }

  esp_err_t err = i2c_master_probe(bus_handle_, device_addr, 1000);
  return (err == ESP_OK);
}

bool EspI2cBus::ResetBus() noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return false;
  }

  esp_err_t err = i2c_master_bus_reset(bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to reset I2C bus: %s", esp_err_to_name(err));
    return false;
  }

  ESP_LOGI(TAG, "I2C bus reset successfully");
  return true;
}

int EspI2cBus::FindDeviceIndexByAddress(hf_u16_t device_address) const noexcept {
  for (size_t i = 0; i < devices_.size(); ++i) {
    if (devices_[i]->GetDeviceAddress() == device_address) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

//==============================================//
// INDEX-BASED ACCESS AND ITERATION METHODS    //
//==============================================//

BaseI2c* EspI2cBus::operator[](int device_index) noexcept {
  return GetDevice(device_index);
}

const BaseI2c* EspI2cBus::operator[](int device_index) const noexcept {
  return GetDevice(device_index);
}

EspI2cDevice* EspI2cBus::At(int device_index) noexcept {
  return GetEspDevice(device_index);
}

const EspI2cDevice* EspI2cBus::At(int device_index) const noexcept {
  return GetEspDevice(device_index);
}

bool EspI2cBus::IsValidIndex(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return (device_index >= 0 && device_index < static_cast<int>(devices_.size()));
}

BaseI2c* EspI2cBus::GetFirstDevice() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (devices_.empty()) {
    return nullptr;
  }
  return devices_[0].get();
}

const BaseI2c* EspI2cBus::GetFirstDevice() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (devices_.empty()) {
    return nullptr;
  }
  return devices_[0].get();
}

BaseI2c* EspI2cBus::GetLastDevice() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (devices_.empty()) {
    return nullptr;
  }
  return devices_[devices_.size() - 1].get();
}

const BaseI2c* EspI2cBus::GetLastDevice() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (devices_.empty()) {
    return nullptr;
  }
  return devices_[devices_.size() - 1].get();
}

std::vector<BaseI2c*> EspI2cBus::GetAllDevices() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  std::vector<BaseI2c*> result;
  result.reserve(devices_.size());
  for (const auto& device : devices_) {
    result.push_back(device.get());
  }
  return result;
}

std::vector<const BaseI2c*> EspI2cBus::GetAllDevices() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  std::vector<const BaseI2c*> result;
  result.reserve(devices_.size());
  for (const auto& device : devices_) {
    result.push_back(device.get());
  }
  return result;
}

std::vector<EspI2cDevice*> EspI2cBus::GetAllEspDevices() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  std::vector<EspI2cDevice*> result;
  result.reserve(devices_.size());
  for (const auto& device : devices_) {
    result.push_back(device.get());
  }
  return result;
}

std::vector<const EspI2cDevice*> EspI2cBus::GetAllEspDevices() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  std::vector<const EspI2cDevice*> result;
  result.reserve(devices_.size());
  for (const auto& device : devices_) {
    result.push_back(device.get());
  }
  return result;
}

int EspI2cBus::FindDeviceIndex(hf_u16_t device_address) const noexcept {
  return FindDeviceIndexByAddress(device_address);
}

std::vector<hf_u16_t> EspI2cBus::GetDeviceAddresses() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  std::vector<hf_u16_t> addresses;
  addresses.reserve(devices_.size());
  for (const auto& device : devices_) {
    addresses.push_back(device->GetDeviceAddress());
  }
  return addresses;
}

bool EspI2cBus::HasDevices() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return !devices_.empty();
}

bool EspI2cBus::IsEmpty() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return devices_.empty();
}

bool EspI2cBus::ClearAllDevices() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (devices_.empty()) {
    return true;
  }

  ESP_LOGI(TAG, "Clearing all %zu devices from I2C bus", devices_.size());
  
  // Clear all devices (destructors will handle ESP-IDF cleanup)
  devices_.clear();
  
  ESP_LOGI(TAG, "All devices cleared successfully");
  return true;
}

hf_i2c_err_t EspI2cBus::ConvertEspError(esp_err_t esp_error) const noexcept {
  switch (esp_error) {
    case ESP_OK:
      return hf_i2c_err_t::I2C_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    case ESP_ERR_TIMEOUT:
      return hf_i2c_err_t::I2C_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_i2c_err_t::I2C_ERR_DEVICE_NOT_FOUND;
    case ESP_ERR_NO_MEM:
      return hf_i2c_err_t::I2C_ERR_OUT_OF_MEMORY;
    case ESP_ERR_INVALID_STATE:
      return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    case ESP_FAIL:
      return hf_i2c_err_t::I2C_ERR_FAILURE;
    default:
      return hf_i2c_err_t::I2C_ERR_FAILURE;
  }
}

//======================================================//
// ESP I2C DEVICE IMPLEMENTATION
//======================================================//

EspI2cDevice::EspI2cDevice(EspI2cBus* parent, i2c_master_dev_handle_t handle,
                           const hf_i2c_device_config_t& config)
          : BaseI2c(), parent_bus_(parent), handle_(handle), config_(config), initialized_(true),
        async_operation_in_progress_(false), sync_operation_in_progress_(false), current_callback_(nullptr), current_user_data_(nullptr),
        async_start_time_(0), current_op_type_(hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE) {
  // Initialize statistics and diagnostics
  statistics_ = hf_i2c_statistics_t{};
  diagnostics_ = hf_i2c_diagnostics_t{};

  diagnostics_.bus_healthy = true;
  diagnostics_.last_error_code = hf_i2c_err_t::I2C_SUCCESS;

  ESP_LOGI(TAG, "EspI2cDevice created for address 0x%02X", config_.device_address);
}

EspI2cDevice::~EspI2cDevice() noexcept {
  ESP_LOGI(TAG, "Destroying EspI2cDevice for address 0x%02X", config_.device_address);
  // Don't call Deinitialize() - ESP-IDF cleanup is handled by the bus
  // Just ensure internal state is clean
  
  // Safety check: if we still have a handle, something went wrong
  if (handle_) {
    ESP_LOGW(TAG, "Device destroyed while still having ESP-IDF handle - this indicates improper cleanup");
    // Don't try to remove from ESP-IDF - let the bus handle it
    handle_ = nullptr;
  }
  
  if (initialized_) {
    ESP_LOGW(TAG, "Device destroyed while still initialized - this may indicate improper cleanup");
    initialized_ = false;
  }
  
  // Clear any pending operations
  async_operation_in_progress_ = false;
  sync_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;
}

bool EspI2cDevice::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_) {
    return true;
  }

  // Device is already initialized when created by the bus
  initialized_ = true;
  return true;
}

bool EspI2cDevice::MarkAsDeinitialized() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Marking EspI2cDevice as deinitialized for address 0x%02X", config_.device_address);

  // Clear internal state but don't remove from ESP-IDF bus
  // The bus handles ESP-IDF cleanup
  
  // Clear any pending async operations
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Device marked as deinitialized while async operation in progress");
    async_operation_in_progress_ = false;
    current_callback_ = nullptr;
    current_user_data_ = nullptr;
  }

  if (sync_operation_in_progress_) {
    ESP_LOGW(TAG, "Device marked as deinitialized while sync operation in progress");
    sync_operation_in_progress_ = false;
  }

  // Clear handle reference (bus will handle ESP-IDF cleanup)
  handle_ = nullptr;
  
  initialized_ = false;
  ESP_LOGI(TAG, "EspI2cDevice marked as deinitialized for address 0x%02X", config_.device_address);
  return true;
}

bool EspI2cDevice::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing EspI2cDevice for address 0x%02X", config_.device_address);

  // Don't remove from ESP-IDF bus - the bus handles this
  // Just clear internal state and mark as deinitialized
  
  // Clear any pending async operations
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Device deinitialized while async operation in progress");
    async_operation_in_progress_ = false;
    current_callback_ = nullptr;
    current_user_data_ = nullptr;
  }

  if (sync_operation_in_progress_) {
    ESP_LOGW(TAG, "Device deinitialized while sync operation in progress");
    sync_operation_in_progress_ = false;
  }

  // Clear handle reference (bus will handle ESP-IDF cleanup)
  handle_ = nullptr;

  initialized_ = false;
  ESP_LOGI(TAG, "EspI2cDevice deinitialized for address 0x%02X", config_.device_address);
  return true;
}

hf_i2c_err_t EspI2cDevice::Write(const hf_u8_t* data, hf_u16_t length,
                                 hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot write: device not properly initialized or handle invalid");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  hf_u64_t start_time = esp_timer_get_time();

  // Ensure no async operations are running before sync operation
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Cannot perform sync write: async operation in progress");
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }
  
  // Clear any existing callbacks to ensure blocking behavior
  esp_err_t clear_err = i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  if (clear_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to clear callbacks for sync write: %s", esp_err_to_name(clear_err));
    return hf_i2c_err_t::I2C_ERR_FAILURE;
  }

  // Mark sync operation as in progress
  sync_operation_in_progress_ = true;

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err = i2c_master_transmit(handle_, data, length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, operation_time);

  // Clear sync operation flag
  sync_operation_in_progress_ = false;

  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C write successful: %d bytes in %lld us", length, operation_time);
  }

  return result;
}

hf_i2c_err_t EspI2cDevice::Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot read: device not properly initialized or handle invalid");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  hf_u64_t start_time = esp_timer_get_time();

  // Ensure no async operations are running before sync operation
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Cannot perform sync read: async operation in progress");
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }
  
  // Clear any existing callbacks to ensure blocking behavior
  esp_err_t clear_err = i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  if (clear_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to clear callbacks for sync read: %s", esp_err_to_name(clear_err));
    return hf_i2c_err_t::I2C_ERR_FAILURE;
  }

  // Mark sync operation as in progress
  sync_operation_in_progress_ = true;

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err = i2c_master_receive(handle_, data, length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, operation_time);

  // Clear sync operation flag
  sync_operation_in_progress_ = false;

  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C read successful: %d bytes in %lld us", length, operation_time);
  }

  return result;
}

hf_i2c_err_t EspI2cDevice::WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length, hf_u8_t* rx_data,
                                     hf_u16_t rx_length, hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  hf_u64_t start_time = esp_timer_get_time();

  // Ensure no async operations are running before sync operation
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Cannot perform sync write-read: async operation in progress");
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }
  
  // Clear any existing callbacks to ensure blocking behavior
  esp_err_t clear_err = i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  if (clear_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to clear callbacks for sync write-read: %s", esp_err_to_name(clear_err));
    return hf_i2c_err_t::I2C_ERR_FAILURE;
  }

  // Mark sync operation as in progress
  sync_operation_in_progress_ = true;

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err =
      i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, tx_length + rx_length, operation_time);

  // Clear sync operation flag
  sync_operation_in_progress_ = false;

  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C write-read failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C write-read successful: %d+%d bytes in %lld us", tx_length, rx_length,
              operation_time);
  }

  return result;
}

//==============================================//
// ASYNC OPERATIONS - ESP-IDF v5.5+ FEATURES   //
//==============================================//

hf_i2c_err_t EspI2cDevice::WriteAsync(const hf_u8_t* data, hf_u16_t length,
                                      hf_i2c_async_callback_t callback,
                                      void* user_data,
                                      hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0 || !callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Set operation context for this write operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE;
  
  // Register temporary callback for this operation
  if (!RegisterTemporaryCallback(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Start async transmission (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - now start tracking
  StartAsyncOperationTracking();

  ESP_LOGD(TAG, "Async write started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::ReadAsync(hf_u8_t* data, hf_u16_t length,
                                     hf_i2c_async_callback_t callback,
                                     void* user_data,
                                     hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0 || !callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Set operation context for this read operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_READ;
  
  // Register temporary callback for this operation
  if (!RegisterTemporaryCallback(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Start async reception (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_receive(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - now start tracking
  StartAsyncOperationTracking();

  ESP_LOGD(TAG, "Async read started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::WriteReadAsync(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                          hf_u8_t* rx_data, hf_u16_t rx_length,
                                          hf_i2c_async_callback_t callback,
                                          void* user_data,
                                          hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0 || !callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Set operation context for this write-read operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE_READ;
  
  // Register temporary callback for this operation
  if (!RegisterTemporaryCallback(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Start async write-read (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit_receive(handle_, tx_data, tx_length, 
                                              rx_data, rx_length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - now start tracking
  StartAsyncOperationTracking();

  ESP_LOGD(TAG, "Async write-read started for device 0x%02X: %d+%d bytes", 
           config_.device_address, tx_length, rx_length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

bool EspI2cDevice::IsAsyncModeSupported() const noexcept {
  return initialized_ && handle_ != nullptr;
}

bool EspI2cDevice::IsAsyncOperationInProgress() const noexcept {
  return async_operation_in_progress_;
}

bool EspI2cDevice::IsSyncOperationInProgress() const noexcept {
  return sync_operation_in_progress_;
}

bool EspI2cDevice::WaitAsyncOperationComplete(hf_u32_t timeout_ms) noexcept {
  if (!async_operation_in_progress_) {
    return true;
  }

  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = static_cast<uint64_t>(timeout_ms) * 1000;

  while (async_operation_in_progress_) {
    if (timeout_ms > 0 && (esp_timer_get_time() - start_time) > timeout_us) {
      ESP_LOGW(TAG, "Timeout waiting for async operation to complete");
      return false;
    }

    vTaskDelay(pdMS_TO_TICKS(1)); // Small delay to avoid busy waiting
  }

  ESP_LOGI(TAG, "Async operation completed for device 0x%02X", config_.device_address);
  return true;
}

hf_i2c_err_t EspI2cDevice::GetStatistics(hf_i2c_statistics_t& statistics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics = statistics_;
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::GetDiagnostics(hf_i2c_diagnostics_t& diagnostics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  diagnostics = diagnostics_;
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_u16_t EspI2cDevice::GetDeviceAddress() const noexcept {
  return config_.device_address;
}

hf_i2c_err_t EspI2cDevice::ResetStatistics() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics_ = hf_i2c_statistics_t{};
  return hf_i2c_err_t::I2C_SUCCESS;
}

i2c_master_dev_handle_t EspI2cDevice::GetHandle() const noexcept {
  return handle_;
}

const hf_i2c_device_config_t& EspI2cDevice::GetConfig() const noexcept {
  return config_;
}

hf_i2c_err_t EspI2cDevice::GetActualClockFrequency(hf_u32_t& actual_freq_hz) const noexcept {
  // ESP-IDF doesn't provide a direct way to get actual clock frequency
  // Return the configured frequency
  actual_freq_hz = config_.scl_speed_hz;
  return hf_i2c_err_t::I2C_SUCCESS;
}

bool EspI2cDevice::ProbeDevice() noexcept {
  if (!initialized_ || !handle_ || !parent_bus_) {
    return false;
  }

  return parent_bus_->ProbeDevice(config_.device_address);
}

void EspI2cDevice::UpdateStatistics(bool success, size_t bytes_transferred,
                                    hf_u64_t operation_time_us) noexcept {
  statistics_.total_transactions++;

  if (success) {
    statistics_.successful_transactions++;
    statistics_.bytes_written += bytes_transferred; // This could be split based on operation type
    statistics_.bytes_read += bytes_transferred;    // This could be split based on operation type
  } else {
    statistics_.failed_transactions++;
  }

  statistics_.total_transaction_time_us += operation_time_us;

  if (operation_time_us > statistics_.max_transaction_time_us) {
    statistics_.max_transaction_time_us = static_cast<hf_u32_t>(operation_time_us);
  }

  if (statistics_.min_transaction_time_us == 0 ||
      operation_time_us < statistics_.min_transaction_time_us) {
    statistics_.min_transaction_time_us = static_cast<hf_u32_t>(operation_time_us);
  }
}

hf_i2c_err_t EspI2cDevice::ConvertEspError(esp_err_t esp_error) const noexcept {
  switch (esp_error) {
    case ESP_OK:
      return hf_i2c_err_t::I2C_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    case ESP_ERR_TIMEOUT:
      return hf_i2c_err_t::I2C_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_i2c_err_t::I2C_ERR_DEVICE_NOT_FOUND;
    case ESP_ERR_NO_MEM:
      return hf_i2c_err_t::I2C_ERR_OUT_OF_MEMORY;
    case ESP_ERR_INVALID_STATE:
      return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    case ESP_FAIL:
      return hf_i2c_err_t::I2C_ERR_FAILURE;
    default:
      return hf_i2c_err_t::I2C_ERR_FAILURE;
  }
}

//==============================================//
// ASYNC CALLBACK BRIDGE SYSTEM                //
//==============================================//

bool EspI2cDevice::RegisterTemporaryCallback(hf_i2c_async_callback_t callback,
                                             void* user_data,
                                             hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Safety check: ensure device is properly initialized
  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot register callback: device not properly initialized or handle invalid");
    return false;
  }

  // Wait for any existing async operation to complete
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Waiting for existing async operation to complete...");
    
    // Simple lock-free waiting - callback will clear async_operation_in_progress_
    uint64_t start_time = esp_timer_get_time();
    uint64_t timeout_us = static_cast<uint64_t>(timeout_ms) * 1000;
    
    while (async_operation_in_progress_) {
      if (timeout_ms > 0 && (esp_timer_get_time() - start_time) > timeout_us) {
        ESP_LOGW(TAG, "Timeout waiting for async slot availability");
        return false;
      }
      
      // Small delay to avoid busy waiting
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  // Store callback information
  current_callback_ = callback;
  current_user_data_ = user_data;
  
  // Register the ESP-IDF callback - only ONE callback for ALL operation types
  i2c_master_event_callbacks_t cbs = {};
  cbs.on_trans_done = InternalAsyncCallback;  // Single callback for write/read/write-read completion
  
  esp_err_t err = i2c_master_register_event_callbacks(handle_, &cbs, this);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register ESP-IDF callback: %s", esp_err_to_name(err));
    current_callback_ = nullptr;
    return false;
  }

  ESP_LOGD(TAG, "Temporary async callback registered for device 0x%02X", config_.device_address);
  return true;
}

void EspI2cDevice::UnregisterTemporaryCallback() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!async_operation_in_progress_) {
    return;
  }

  // Unregister ESP-IDF callback
  esp_err_t err = i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to unregister ESP-IDF callback: %s", esp_err_to_name(err));
  }

  // Clear callback state
  async_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;

  ESP_LOGD(TAG, "Temporary async callback unregistered for device 0x%02X", config_.device_address);
}

void EspI2cDevice::StartAsyncOperationTracking() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  // Safety check: ensure device is properly initialized
  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot start async tracking: device not properly initialized or handle invalid");
    return;
  }
  
  // Start tracking the async operation
  async_operation_in_progress_ = true;
  async_start_time_ = esp_timer_get_time();
  
  ESP_LOGD(TAG, "Async operation tracking started for device 0x%02X", config_.device_address);
}

bool EspI2cDevice::InternalAsyncCallback(i2c_master_dev_handle_t i2c_dev,
                                        const i2c_master_event_data_t* evt_data,
                                        void* arg) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(arg);
  if (!device) {
    return false;
  }

  // Handle different ESP-IDF v5.5 I2C events
  hf_i2c_err_t result;
  
  if (evt_data) {
    switch (evt_data->event) {
      case I2C_EVENT_DONE:
        // Transaction completed successfully
        result = hf_i2c_err_t::I2C_SUCCESS;
        break;
        
      case I2C_EVENT_NACK:
        // No ACK received - transaction failed
        result = hf_i2c_err_t::I2C_ERR_DEVICE_NOT_FOUND;
        break;
        
      case I2C_EVENT_TIMEOUT:
        // Transaction timed out
        result = hf_i2c_err_t::I2C_ERR_TIMEOUT;
        break;
        
      case I2C_EVENT_ALIVE:
        // Bus is alive but transaction not complete yet
        return false; // Don't complete yet
        
      default:
        // Unknown event - assume failure
        result = hf_i2c_err_t::I2C_ERR_FAILURE;
        break;
    }
  } else {
    // No event data - assume failure
    result = hf_i2c_err_t::I2C_ERR_FAILURE;
  }

  // Handle completion in the device - ESP-IDF doesn't provide bytes transferred
  device->HandleAsyncCompletion(result);

  return false; // No high priority wake needed
}

void EspI2cDevice::HandleAsyncCompletion(hf_i2c_err_t result) noexcept {
  // This method is called from ISR context - keep it minimal!
  
  // Safety check: ensure device is still valid
  if (!initialized_ || !handle_) {
    ESP_LOGW(TAG, "Async completion for deinitialized device - ignoring");
    return;
  }
  
  // Store callback info before clearing it
  hf_i2c_async_callback_t callback = current_callback_;
  void* user_data = current_user_data_;
  
  // Clear the async state
  async_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;
  
  // Unregister the ESP-IDF callback (this allows next async operation)
  i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  
  // Call user callback - ESP-IDF doesn't provide actual bytes transferred
  // Pass 0 to indicate we don't have this information
  if (callback) {
    callback(result, 0, user_data);
  }

  ESP_LOGD(TAG, "Async operation completed for device 0x%02X: %s", 
           config_.device_address, HfI2CErrToString(result).data());
}
