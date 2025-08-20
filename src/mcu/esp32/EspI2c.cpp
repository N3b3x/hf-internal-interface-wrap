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
#include "base/BaseI2c.h" // For HfI2CErrToString
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
    : config_(config), bus_handle_(nullptr), initialized_(false), 
      async_device_index_(-1), total_pending_async_operations_(0) {
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
  bus_cfg.clk_source = static_cast<i2c_clock_source_t>(config_.clk_source);
  bus_cfg.glitch_ignore_cnt = static_cast<uint8_t>(config_.glitch_ignore_cnt);
  bus_cfg.intr_priority = config_.intr_priority;
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

  // Remove all devices first
  devices_.clear(); // This will call destructors and remove devices from ESP-IDF

  // Delete the master bus
  if (bus_handle_) {
    esp_err_t err = i2c_del_master_bus(bus_handle_);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(err));
      return false;
    }
    bus_handle_ = nullptr;
  }

  initialized_ = false;
  ESP_LOGI(TAG, "I2C bus deinitialized successfully");
  return true;
}

//==============================================//
// ASYNC DEVICE MANAGEMENT                      //
//==============================================//

bool EspI2cBus::IsAsyncModeInUse() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return async_device_index_ >= 0;
}

EspI2cDevice* EspI2cBus::GetAsyncDevice() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (async_device_index_ >= 0 && async_device_index_ < static_cast<int>(devices_.size())) {
    return devices_[async_device_index_].get();
  }
  
  return nullptr;
}

bool EspI2cBus::CanEnableAsyncMode(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return false;
  }
  
  // Only one device per bus can use async mode
  return async_device_index_ == -1 || async_device_index_ == device_index;
}

bool EspI2cBus::ReserveAsyncMode(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    ESP_LOGE(TAG, "Invalid device index for async mode reservation: %d", device_index);
    return false;
  }
  
  if (async_device_index_ >= 0 && async_device_index_ != device_index) {
    ESP_LOGW(TAG, "Async mode already reserved by device %d", async_device_index_);
    return false;
  }
  
  async_device_index_ = device_index;
  ESP_LOGI(TAG, "Async mode reserved for device %d", device_index);
  return true;
}

bool EspI2cBus::ReleaseAsyncMode(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (async_device_index_ != device_index) {
    ESP_LOGW(TAG, "Device %d is not the current async device", device_index);
    return false;
  }
  
  async_device_index_ = -1;
  ESP_LOGI(TAG, "Async mode released by device %d", device_index);
  return true;
}

bool EspI2cBus::WaitAllAsyncOperationsComplete(hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return false;
  }
  
  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = static_cast<uint64_t>(timeout_ms) * 1000;
  
  while (total_pending_async_operations_ > 0) {
    if (timeout_ms > 0 && (esp_timer_get_time() - start_time) > timeout_us) {
      ESP_LOGW(TAG, "Timeout waiting for async operations to complete");
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1)); // Small delay to avoid busy waiting
  }
  
  ESP_LOGI(TAG, "All async operations completed");
  return true;
}

size_t EspI2cBus::GetTotalPendingAsyncOperations() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return total_pending_async_operations_;
}

void EspI2cBus::UpdateTotalPendingAsyncOperations(int delta) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  if (delta > 0) {
    total_pending_async_operations_ += static_cast<size_t>(delta);
  } else if (delta < 0) {
    size_t abs_delta = static_cast<size_t>(-delta);
    if (total_pending_async_operations_ >= abs_delta) {
      total_pending_async_operations_ -= abs_delta;
    } else {
      total_pending_async_operations_ = 0;
    }
  }
  
  ESP_LOGD(TAG, "Total pending async operations: %zu", total_pending_async_operations_);
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

  ESP_LOGI(TAG, "Removing I2C device at index %d", device_index);

  // If this device was using async mode, release it
  if (device_index == async_device_index_) {
    ReleaseAsyncMode(device_index);
  }

  // Remove the device (destructor will handle ESP-IDF cleanup)
  devices_.erase(devices_.begin() + device_index);

  ESP_LOGI(TAG, "I2C device removed successfully");
  return true;
}

bool EspI2cBus::RemoveDeviceByAddress(hf_u16_t device_address) noexcept {
  int index = FindDeviceIndexByAddress(device_address);
  if (index >= 0) {
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
      async_mode_enabled_(false), pending_async_operations_(0) {
  // Initialize statistics and diagnostics
  statistics_ = hf_i2c_statistics_t{};
  diagnostics_ = hf_i2c_diagnostics_t{};

  diagnostics_.bus_healthy = true;
  diagnostics_.last_error_code = hf_i2c_err_t::I2C_SUCCESS;

  // Initialize default event callbacks
  InitializeDefaultEventCallbacks();

  ESP_LOGI(TAG, "EspI2cDevice created for address 0x%02X", config_.device_address);
}

EspI2cDevice::~EspI2cDevice() noexcept {
  ESP_LOGI(TAG, "Destroying EspI2cDevice for address 0x%02X", config_.device_address);
  Deinitialize();
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

bool EspI2cDevice::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  // Cancel any pending async operations
  if (async_mode_enabled_) {
    CancelAllAsyncOperations();
    UnregisterEventCallbacks();
  }

  // Remove device from ESP-IDF
  if (handle_) {
    esp_err_t err = i2c_master_bus_rm_device(handle_);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to remove I2C device: %s", esp_err_to_name(err));
      return false;
    }
    handle_ = nullptr;
  }

  initialized_ = false;
  ESP_LOGI(TAG, "EspI2cDevice deinitialized for address 0x%02X", config_.device_address);
  return true;
}

hf_i2c_err_t EspI2cDevice::Write(const hf_u8_t* data, hf_u16_t length,
                                 hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  hf_u64_t start_time = esp_timer_get_time();

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err = i2c_master_transmit(handle_, data, length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, operation_time);

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
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  hf_u64_t start_time = esp_timer_get_time();

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err = i2c_master_receive(handle_, data, length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, operation_time);

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

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  esp_err_t err =
      i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, timeout);

  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t operation_time = end_time - start_time;

  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, tx_length + rx_length, operation_time);

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
// ASYNC OPERATIONS - ESP-IDF v5.5+ FEATURES   //
//==============================================//

hf_i2c_err_t EspI2cDevice::WriteAsync(const hf_u8_t* data, hf_u16_t length,
                                       hf_i2c_async_callback_t callback,
                                       void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!async_mode_enabled_) {
    ESP_LOGE(TAG, "Async mode not enabled for device 0x%02X", config_.device_address);
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Store callback for completion
  async_callback_ = callback;
  async_user_data_ = user_data;

  // Increment pending operations count
  pending_async_operations_++;
  if (parent_bus_) {
    parent_bus_->UpdateTotalPendingAsyncOperations(1);
  }

  // Start async transmission (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Decrement count on failure
    pending_async_operations_--;
    if (parent_bus_) {
      parent_bus_->UpdateTotalPendingAsyncOperations(-1);
    }
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async write started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::ReadAsync(hf_u8_t* data, hf_u16_t length,
                                      hf_i2c_async_callback_t callback,
                                      void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!async_mode_enabled_) {
    ESP_LOGE(TAG, "Async mode not enabled for device 0x%02X", config_.device_address);
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  if (!data || length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Store callback for completion
  async_callback_ = callback;
  async_user_data_ = user_data;

  // Increment pending operations count
  pending_async_operations_++;
  if (parent_bus_) {
    parent_bus_->UpdateTotalPendingAsyncOperations(1);
  }

  // Start async reception (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_receive(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Decrement count on failure
    pending_async_operations_--;
    if (parent_bus_) {
      parent_bus_->UpdateTotalPendingAsyncOperations(-1);
    }
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async read started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::WriteReadAsync(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                           hf_u8_t* rx_data, hf_u16_t rx_length,
                                           hf_i2c_async_callback_t callback,
                                           void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  if (!async_mode_enabled_) {
    ESP_LOGE(TAG, "Async mode not enabled for device 0x%02X", config_.device_address);
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  }

  if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Store callback for completion
  async_callback_ = callback;
  async_user_data_ = user_data;

  // Increment pending operations count
  pending_async_operations_++;
  if (parent_bus_) {
    parent_bus_->UpdateTotalPendingAsyncOperations(1);
  }

  // Start async write-read (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit_receive(handle_, tx_data, tx_length, 
                                              rx_data, rx_length, 0);

  if (err != ESP_OK) {
    // Decrement count on failure
    pending_async_operations_--;
    if (parent_bus_) {
      parent_bus_->UpdateTotalPendingAsyncOperations(-1);
    }
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async write-read started for device 0x%02X: %d+%d bytes", 
           config_.device_address, tx_length, rx_length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

//==============================================//
// EVENT CALLBACK REGISTRATION                  //
//==============================================//

bool EspI2cDevice::RegisterEventCallbacks(const i2c_master_event_callbacks_t& callbacks,
                                          void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Device not initialized");
    return false;
  }

  if (async_mode_enabled_) {
    ESP_LOGW(TAG, "Event callbacks already registered for device 0x%02X", config_.device_address);
    return true;
  }

  // Check if this device can enable async mode
  if (parent_bus_) {
    // Find our device index first
    int device_index = -1;
    for (int i = 0; i < static_cast<int>(parent_bus_->GetDeviceCount()); ++i) {
      if (parent_bus_->GetDevice(i) == this) {
        device_index = i;
        break;
      }
    }
    
    if (device_index >= 0 && !parent_bus_->CanEnableAsyncMode(device_index)) {
      ESP_LOGE(TAG, "Cannot enable async mode - another device is using it");
      return false;
    }
  }

  // Store callbacks
  event_callbacks_ = callbacks;
  callback_user_data_ = user_data;

  // Register with ESP-IDF
  esp_err_t err = i2c_master_register_event_callbacks(handle_, &event_callbacks_, user_data);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register event callbacks: %s", esp_err_to_name(err));
    return false;
  }

  // Reserve async mode on the bus
  if (parent_bus_) {
    // Find our device index
    for (int i = 0; i < static_cast<int>(parent_bus_->GetDeviceCount()); ++i) {
      if (parent_bus_->GetDevice(i) == this) {
        if (!parent_bus_->ReserveAsyncMode(i)) {
          ESP_LOGE(TAG, "Failed to reserve async mode on bus");
          // Unregister callbacks on failure
          i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
          return false;
        }
        break;
      }
    }
  }

  async_mode_enabled_ = true;
  ESP_LOGI(TAG, "Event callbacks registered for device 0x%02X", config_.device_address);
  return true;
}

bool EspI2cDevice::UnregisterEventCallbacks() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!async_mode_enabled_) {
    return true;
  }

  // Wait for any pending operations to complete
  if (pending_async_operations_ > 0) {
    ESP_LOGW(TAG, "Unregistering callbacks with %zu pending operations", pending_async_operations_);
    // Wait a reasonable time for operations to complete
    uint32_t timeout_ms = 1000;
    uint64_t start_time = esp_timer_get_time();
    
    while (pending_async_operations_ > 0) {
      if ((esp_timer_get_time() - start_time) / 1000 > timeout_ms) {
        ESP_LOGW(TAG, "Timeout waiting for pending operations");
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  // Unregister from ESP-IDF
  esp_err_t err = i2c_master_register_event_callbacks(handle_, nullptr, nullptr);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to unregister event callbacks: %s", esp_err_to_name(err));
    return false;
  }

  // Release async mode on the bus
  if (parent_bus_) {
    for (int i = 0; i < static_cast<int>(parent_bus_->GetDeviceCount()); ++i) {
      if (parent_bus_->GetDevice(i) == this) {
        parent_bus_->ReleaseAsyncMode(i);
        break;
      }
    }
  }

  async_mode_enabled_ = false;
  pending_async_operations_ = 0;
  ESP_LOGI(TAG, "Event callbacks unregistered for device 0x%02X", config_.device_address);
  return true;
}

bool EspI2cDevice::IsAsyncModeSupported() const noexcept {
  return initialized_ && handle_ != nullptr;
}

bool EspI2cDevice::IsAsyncModeEnabled() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return async_mode_enabled_;
}

//==============================================//
// ASYNC OPERATION MANAGEMENT                  //
//==============================================//

bool EspI2cDevice::WaitAllAsyncOperationsComplete(hf_u32_t timeout_ms) noexcept {
  if (!async_mode_enabled_) {
    return true;
  }

  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = static_cast<uint64_t>(timeout_ms) * 1000;

  while (pending_async_operations_ > 0) {
    if (timeout_ms > 0 && (esp_timer_get_time() - start_time) > timeout_us) {
      ESP_LOGW(TAG, "Timeout waiting for async operations to complete");
      return false;
    }

    vTaskDelay(pdMS_TO_TICKS(1)); // Small delay to avoid busy waiting
  }

  ESP_LOGI(TAG, "All async operations completed for device 0x%02X", config_.device_address);
  return true;
}

bool EspI2cDevice::CancelAllAsyncOperations() noexcept {
  if (!async_mode_enabled_ || pending_async_operations_ == 0) {
    return true;
  }

  ESP_LOGW(TAG, "Cancelling %zu pending async operations for device 0x%02X", 
           pending_async_operations_, config_.device_address);

  // Note: ESP-IDF doesn't provide a direct way to cancel pending operations
  // We'll just reset the count and let operations complete naturally
  // In a real implementation, you might want to implement a more sophisticated
  // cancellation mechanism using device reset or bus reset

  if (parent_bus_) {
    parent_bus_->UpdateTotalPendingAsyncOperations(-static_cast<int>(pending_async_operations_));
  }
  
  pending_async_operations_ = 0;
  return true;
}

size_t EspI2cDevice::GetPendingAsyncOperationCount() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return pending_async_operations_;
}

//==============================================//
// INTERNAL CALLBACK HANDLERS                   //
//==============================================//

// Note: These callback functions are placeholders for ESP-IDF v5.5
// The actual implementation will depend on the specific ESP-IDF version
// For now, we'll use simple callbacks that don't rely on complex event data

bool EspI2cDevice::OnTransmitDoneCallback(i2c_master_dev_handle_t dev,
                                          const void* edata,
                                          void* user_data) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(user_data);
  if (device) {
    // For now, assume successful transmission with unknown byte count
    device->HandleAsyncOperationComplete(hf_i2c_err_t::I2C_SUCCESS, 
                                       0, // Unknown byte count
                                       hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE);
  }
  return false; // No high priority wake requested
}

bool EspI2cDevice::OnReceiveDoneCallback(i2c_master_dev_handle_t dev,
                                         const void* edata,
                                         void* user_data) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(user_data);
  if (device) {
    // For now, assume successful reception with unknown byte count
    device->HandleAsyncOperationComplete(hf_i2c_err_t::I2C_SUCCESS, 
                                       0, // Unknown byte count
                                       hf_i2c_transaction_type_t::HF_I2C_TRANS_READ);
  }
  return false; // No high priority wake requested
}

bool EspI2cDevice::OnTransmitErrorCallback(i2c_master_dev_handle_t dev,
                                           const void* edata,
                                           void* user_data) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(user_data);
  if (device) {
    device->HandleAsyncOperationComplete(hf_i2c_err_t::I2C_ERR_WRITE_FAILURE, 
                                       0, 
                                       hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE);
  }
  return false; // No high priority wake requested
}

bool EspI2cDevice::OnReceiveErrorCallback(i2c_master_dev_handle_t dev,
                                          const void* edata,
                                          void* user_data) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(user_data);
  if (device) {
    device->HandleAsyncOperationComplete(hf_i2c_err_t::I2C_ERR_READ_FAILURE, 
                                       0, 
                                       hf_i2c_transaction_type_t::HF_I2C_TRANS_READ);
  }
  return false; // No high priority wake requested
}

//==============================================//
// INTERNAL HELPER METHODS                      //
//==============================================//

void EspI2cDevice::InitializeDefaultEventCallbacks() noexcept {
  // Initialize callback structure with our internal handlers
  // Note: ESP-IDF v5.5 may use different member names
  // We'll set them to nullptr for now and let the user provide their own
  // or implement a version-specific approach
  
  // For now, disable all callbacks to avoid compilation issues
  // Users can register their own callbacks using RegisterEventCallbacks()
  event_callbacks_.on_trans_done = nullptr;
  event_callbacks_.on_recv_done = nullptr;
  event_callbacks_.on_trans_err = nullptr;
  event_callbacks_.on_recv_err = nullptr;
  
  // Initialize other fields
  callback_user_data_ = nullptr;
  async_callback_ = nullptr;
  async_user_data_ = nullptr;
}

void EspI2cDevice::HandleAsyncOperationComplete(hf_i2c_err_t result, size_t bytes_transferred,
                                                hf_i2c_transaction_type_t operation_type) noexcept {
  // Decrement pending operations count
  if (pending_async_operations_ > 0) {
    pending_async_operations_--;
  }
  
  if (parent_bus_) {
    parent_bus_->UpdateTotalPendingAsyncOperations(-1);
  }

  // Update statistics
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, bytes_transferred, 0);

  // Call user callback if provided
  if (async_callback_) {
    async_callback_(result, bytes_transferred, async_user_data_);
  }

  // Update diagnostics
  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = esp_timer_get_time();
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
  }

  ESP_LOGD(TAG, "Async operation completed for device 0x%02X: %s, %zu bytes", 
           config_.device_address, HfI2CErrToString(result).data(), bytes_transferred);
}
