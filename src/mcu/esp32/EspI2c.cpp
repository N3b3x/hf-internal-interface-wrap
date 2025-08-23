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

// Configuration: Choose probe implementation
// Set to 1 to use our fast custom probe, 0 to use ESP-IDF's broken probe
#define USE_CUSTOM_FAST_PROBE 1

static const char* TAG = "EspI2c";

//======================================================//
// OPERATION TYPE TO STRING CONVERSION                 //
//======================================================//

const char* HfI2COperationToString(hf_i2c_operation_t op) noexcept {
  switch (op) {
    case hf_i2c_operation_t::HF_I2C_OP_WRITE:
      return "Write";
    case hf_i2c_operation_t::HF_I2C_OP_READ:
      return "Read";
    case hf_i2c_operation_t::HF_I2C_OP_WRITE_READ:
      return "WriteRead";
    case hf_i2c_operation_t::HF_I2C_OP_WRITE_ASYNC:
      return "WriteAsync";
    case hf_i2c_operation_t::HF_I2C_OP_READ_ASYNC:
      return "ReadAsync";
    case hf_i2c_operation_t::HF_I2C_OP_WRITE_READ_ASYNC:
      return "WriteReadAsync";
    default:
      return "Unknown";
  }
}

//======================================================//
// ESP I2C BUS IMPLEMENTATION
//======================================================//

EspI2cBus::EspI2cBus(const hf_i2c_master_bus_config_t& config) noexcept
    : config_(config), bus_handle_(nullptr), initialized_(false), current_mode_(config.mode) {
  ESP_LOGI(TAG, "Creating I2C bus on port %d in %s mode", 
           config_.i2c_port, 
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");
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

  ESP_LOGI(TAG, "Initializing I2C bus on port %d (SDA:GPIO%d, SCL:GPIO%d) in %s mode", 
           config_.i2c_port, config_.sda_io_num, config_.scl_io_num,
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

  // ESP-IDF's sync/async mode separation
  // - trans_queue_depth = 0: SYNC mode only (no queue, blocking operations)
  // - trans_queue_depth > 0: ASYNC mode only (with queue, non-blocking operations)
  uint8_t actual_queue_depth = config_.trans_queue_depth;
  
  if (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC) {
    // SYNC mode: Must have trans_queue_depth = 0
    if (actual_queue_depth != 0) {
      ESP_LOGW(TAG, "Sync mode requires trans_queue_depth=0, forcing to 0 (was %d)", actual_queue_depth);
      actual_queue_depth = 0;
    }
  } else {
    // ASYNC mode: Must have trans_queue_depth > 0
    if (actual_queue_depth == 0) {
      ESP_LOGW(TAG, "Async mode requires trans_queue_depth>0, setting to 10 (was 0)");
      actual_queue_depth = 32;
    }
  }

  // Create ESP-IDF bus configuration
  i2c_master_bus_config_t bus_cfg = {};
  bus_cfg.i2c_port = static_cast<i2c_port_num_t>(config_.i2c_port);
  bus_cfg.sda_io_num = static_cast<gpio_num_t>(config_.sda_io_num);
  bus_cfg.scl_io_num = static_cast<gpio_num_t>(config_.scl_io_num);
  bus_cfg.clk_source = static_cast<i2c_clock_source_t>(config_.clk_source);
  bus_cfg.glitch_ignore_cnt = static_cast<uint8_t>(config_.glitch_ignore_cnt);
  bus_cfg.intr_priority = config_.intr_priority;
  bus_cfg.trans_queue_depth = actual_queue_depth;
  bus_cfg.flags.enable_internal_pullup = config_.flags.enable_internal_pullup;
  bus_cfg.flags.allow_pd = config_.flags.allow_pd;

  ESP_LOGI(TAG, "ESP-IDF config: port=%d, sda=%d, scl=%d, queue_depth=%d, mode=%s",
           bus_cfg.i2c_port, bus_cfg.sda_io_num, bus_cfg.scl_io_num, 
           actual_queue_depth, (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

  // Create the master bus
  esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(err));
    return false;
  }

  // CRITICAL FIX: Wait for bus to be fully ready (ESP32-C6 requirement)
  // This ensures the I2C peripheral is fully initialized before any operations
  err = i2c_master_bus_wait_all_done(bus_handle_, 100);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to wait for I2C bus readiness: %s", esp_err_to_name(err));
    // Clean up and return failure
    i2c_del_master_bus(bus_handle_);
    bus_handle_ = nullptr;
    return false;
  }
  ESP_LOGI(TAG, "I2C bus is ready and waiting for operations");

  // Try bus reset to ensure peripheral is in known state
  // This can fail when no physical I2C devices are connected or pull-ups are missing
  err = i2c_master_bus_reset(bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Bus reset failed: %s (this is normal without a stuck bus [SDA and/or SCL held LOW])", esp_err_to_name(err));
    ESP_LOGI(TAG, "Continuing initialization without bus reset - suitable for testing");
  } else {
    ESP_LOGI(TAG, "Bus reset successful");
  }

  initialized_ = true;
  ESP_LOGI(TAG, "I2C bus initialized successfully in %s mode with queue_depth=%d", 
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC", actual_queue_depth);
  return true;
}

bool EspI2cBus::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing I2C bus in %s mode", 
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

  // Remove all devices first - inline the helper since it's only used once
  for (auto& device : devices_) {
    if (device && device->GetHandle()) {
      ESP_LOGI(TAG, "Removing device 0x%02X from ESP-IDF bus", device->GetDeviceAddress());
      
      esp_err_t err = i2c_master_bus_rm_device(device->GetHandle());
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to remove device 0x%02X: %s", 
                 device->GetDeviceAddress(), esp_err_to_name(err));
      } else {
        ESP_LOGI(TAG, "Successfully removed device 0x%02X", device->GetDeviceAddress());
      }
      
      device->MarkAsDeinitialized();
    }
  }
  devices_.clear();

  // Delete the master bus
  if (bus_handle_) {
    ESP_LOGI(TAG, "Deleting I2C master bus");
    esp_err_t err = i2c_del_master_bus(bus_handle_);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(100));
    } else {
      ESP_LOGI(TAG, "Successfully deleted I2C master bus");
    }
    bus_handle_ = nullptr;
  }
  
  vTaskDelay(pdMS_TO_TICKS(200));
  initialized_ = false;
  ESP_LOGI(TAG, "I2C bus deinitialized successfully");
  return true;
}

// Mode management methods
hf_i2c_mode_t EspI2cBus::GetMode() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return current_mode_;
}

bool EspI2cBus::IsAsyncMode() const noexcept {
  return GetMode() == hf_i2c_mode_t::HF_I2C_MODE_ASYNC;
}

bool EspI2cBus::IsSyncMode() const noexcept {
  return GetMode() == hf_i2c_mode_t::HF_I2C_MODE_SYNC;
}

bool EspI2cBus::SwitchMode(hf_i2c_mode_t new_mode, uint8_t queue_depth) noexcept {
  if (current_mode_ == new_mode) return true;
  
  ESP_LOGI(TAG, "Switching I2C bus from %s to %s mode", 
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC",
           (new_mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");
  
  // Deinitialize current bus
  Deinitialize();
  
  // Update configuration
  config_.mode = new_mode;
  config_.trans_queue_depth = (new_mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? queue_depth : 0;
  current_mode_ = new_mode;
  
  // Reinitialize with new mode
  return Initialize();
}

int EspI2cBus::CreateDevice(const hf_i2c_device_config_t& device_config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return -1;
  }

  ESP_LOGI(TAG, "Creating I2C device at address 0x%02X in %s mode", 
           device_config.device_address,
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

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

  ESP_LOGI(TAG, "I2C device created successfully at index %d in %s mode", 
           device_index, (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");
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

//==============================================//
// CUSTOM FAST PROBE IMPLEMENTATION            //
//==============================================//

/**
 * @brief Custom fast I2C probe that works around ESP-IDF's broken probe function
 * @param device_addr Device address to probe
 * @param timeout_ms Timeout in milliseconds
 * @return true if device responds, false otherwise
 * @note This function creates a temporary device and performs a real I2C transaction
 * @note Much faster and more reliable than ESP-IDF's broken probe function
 */
bool EspI2cBus::CustomFastProbe(hf_u16_t device_addr, hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "CustomFastProbe: Bus not initialized");
    return false;
  }

  // CRITICAL: Block invalid addresses that ESP-IDF might try to probe internally
  if (device_addr < 0x01 || device_addr > 0x7F) {
    ESP_LOGW(TAG, "CustomFastProbe: BLOCKING invalid address 0x%02X - ESP-IDF internal probe detected! (valid range: 0x01-0x7F)", device_addr);
    ESP_LOGE(TAG, "CustomFastProbe: WARNING - Address 0x%02X is outside valid range. This suggests ESP-IDF internal probing or system-level scanning.", device_addr);
    return false;
  }

  ESP_LOGI(TAG, "CustomFastProbe: Probing address 0x%02X with %lu ms timeout", device_addr, timeout_ms);
  
  // Use provided timeout or default to fast 50ms timeout for quick probing
  hf_u32_t actual_timeout = (timeout_ms > 0) ? timeout_ms : 50;
  
  // Create a temporary device configuration for probing
  hf_i2c_device_config_t temp_config = {};
  temp_config.device_address = device_addr;
  temp_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  temp_config.scl_speed_hz = 100000; // Use 100kHz for fast probing
  temp_config.scl_wait_us = 0;       // No clock stretching for fast probe
  temp_config.disable_ack_check = false; // Enable ACK checking
  
  // Add temporary device to bus
  int device_index = CreateDevice(temp_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "CustomFastProbe: Failed to create temporary device for address 0x%02X", device_addr);
    return false;
  }
  
  // Get the device and try to read 1 byte (this will generate proper START/STOP)
  BaseI2c* temp_device = GetDevice(device_index);
  if (!temp_device) {
    ESP_LOGE(TAG, "CustomFastProbe: Failed to get temporary device for address 0x%02X", device_addr);
    // Clean up the failed device creation
    RemoveDevice(device_index);
    return false;
  }
  
  // Perform fast probe by reading 1 byte
  uint8_t dummy_byte = 0x11;
  uint64_t probe_start_time = esp_timer_get_time();
  hf_i2c_err_t result = temp_device->Read(&dummy_byte, 1, actual_timeout);
  uint64_t probe_end_time = esp_timer_get_time();
  uint64_t probe_duration_us = probe_end_time - probe_start_time;
  
  // Remove the temporary device immediately
  if (!RemoveDevice(device_index)) {
    ESP_LOGW(TAG, "CustomFastProbe: Warning - failed to remove temporary device for address 0x%02X", device_addr);
  }
  
  bool device_found = (result == hf_i2c_err_t::I2C_SUCCESS);
  
  ESP_LOGI(TAG, "CustomFastProbe: Address 0x%02X probe result: %s (result: %s) in %llu μs", 
           device_addr, device_found ? "FOUND" : "NOT FOUND", 
           HfI2CErrToString(result).data(), probe_duration_us);
  
  return device_found;
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
                              hf_u16_t end_addr, hf_u32_t scan_timeout_ms) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "ScanDevices: I2C bus not initialized");
    return 0;
  }

  // Validate scan range (7-bit: 0x08-0x77, 10-bit: 0x000-0x3FF)
  if (start_addr < 0x08 || end_addr > 0x77 || start_addr > end_addr) {
    ESP_LOGE(TAG, "ScanDevices: INVALID scan range: 0x%02X to 0x%02X. Valid range is 0x08-0x77", 
             start_addr, end_addr);
    return 0;
  }

  found_devices.clear();
  
  // Use fast scanning timeout (10ms) for quick device discovery
  hf_u32_t fast_scan_timeout = (scan_timeout_ms > 0) ? scan_timeout_ms : 10;
  
  ESP_LOGI(TAG, "ScanDevices: Starting scan from 0x%02X to 0x%02X with %lu ms timeout", 
           start_addr, end_addr, fast_scan_timeout);

  for (hf_u16_t addr = start_addr; addr <= end_addr; ++addr) {
    ESP_LOGI(TAG, "ScanDevices: About to probe address 0x%02X", addr);
    if (ProbeDevice(addr, fast_scan_timeout)) {
      found_devices.push_back(addr);
      ESP_LOGI(TAG, "ScanDevices: Found I2C device at address 0x%02X", addr);
    }
  }

  ESP_LOGI(TAG, "ScanDevices: Scan completed. Found %zu devices", found_devices.size());
  return found_devices.size();
}

bool EspI2cBus::ProbeDevice(hf_u16_t device_addr, hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "ProbeDevice: Bus not initialized");
    return false;
  }

  // CRITICAL: Block invalid addresses that ESP-IDF might try to probe internally
  if (device_addr < 0x01 || device_addr > 0x7F) {
    ESP_LOGW(TAG, "ProbeDevice: BLOCKING invalid address 0x%02X - ESP-IDF internal probe detected! (valid range: 0x01-0x7F)", device_addr);
    ESP_LOGE(TAG, "ProbeDevice: WARNING - Address 0x%02X is outside valid range. This suggests ESP-IDF internal probing or system-level scanning.", device_addr);
    return false;
  }

  ESP_LOGI(TAG, "ProbeDevice: Probing address 0x%02X with %lu ms timeout", device_addr, timeout_ms);
  
  // Use provided timeout or default to fast 50ms timeout for quick probing
  hf_u32_t actual_timeout = (timeout_ms > 0) ? timeout_ms : 50;
  
  bool device_found = false;
  uint64_t probe_start_time = esp_timer_get_time();
  
#if USE_CUSTOM_FAST_PROBE
  // Use our fast custom probe that actually works
  ESP_LOGI(TAG, "ProbeDevice: Using CUSTOM FAST PROBE (working implementation)");
  device_found = CustomFastProbe(device_addr, actual_timeout);
#else
  // Use ESP-IDF's broken probe function (for comparison/testing)
  ESP_LOGI(TAG, "ProbeDevice: Using ESP-IDF PROBE (known to be broken on ESP32C6)");
  
  // Use provided timeout or default to 1000ms for ESP-IDF probe
  hf_u32_t esp_timeout = (timeout_ms > 0) ? timeout_ms : 1000;
  
  esp_err_t err = i2c_master_probe(bus_handle_, device_addr, esp_timeout);
  device_found = (err == ESP_OK);
  
  ESP_LOGI(TAG, "ProbeDevice: ESP-IDF probe result: %s (err: %s)", 
           device_found ? "FOUND" : "NOT FOUND", esp_err_to_name(err));
#endif

  uint64_t probe_end_time = esp_timer_get_time();
  uint64_t probe_duration_us = probe_end_time - probe_start_time;
  
  ESP_LOGI(TAG, "ProbeDevice: Address 0x%02X final result: %s in %llu μs", 
           device_addr, device_found ? "FOUND" : "NOT FOUND", probe_duration_us);
  
  return device_found;
}

bool EspI2cBus::ResetBus() noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return false;
  }

  ESP_LOGI(TAG, "Attempting to reset I2C bus to recover from potential hanging state");
  
  // Try the standard ESP-IDF bus reset, but handle failures gracefully
  esp_err_t err = i2c_master_bus_reset(bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "I2C bus reset failed: %s (normal without physical devices or pull-ups)", esp_err_to_name(err));
    ESP_LOGI(TAG, "Bus reset not required for testing scenarios - continuing");
    return true; // Return true to indicate this is acceptable for testing
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
        device_mode_(parent->GetMode()), async_operation_in_progress_(false), sync_operation_in_progress_(false), current_callback_(nullptr), current_user_data_(nullptr),
        async_start_time_(0), current_op_type_(hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE) {
  // Initialize statistics and diagnostics
  statistics_ = hf_i2c_statistics_t{};
  diagnostics_ = hf_i2c_diagnostics_t{};

  diagnostics_.bus_healthy = true;
  diagnostics_.last_error_code = hf_i2c_err_t::I2C_SUCCESS;

  ESP_LOGI(TAG, "EspI2cDevice created for address 0x%02X in %s mode", 
           config_.device_address,
           (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");
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
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "Write() called on async-mode device. Use WriteAsync() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_WRITE)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_WRITE)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  // Perform the I2C write operation
  hf_u64_t start_time = esp_timer_get_time();
  esp_err_t err = i2c_master_transmit(handle_, data, length, timeout);
  hf_u64_t end_time = esp_timer_get_time();

  // Convert result and update statistics
  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, end_time - start_time);
  
  // Cleanup sync operation
  CleanupSyncOperation();
  
  // Log result
  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C write successful: %d bytes in %lld us", length, end_time - start_time);
  }

  return result;
}

hf_i2c_err_t EspI2cDevice::Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "Read() called on async-mode device. Use ReadAsync() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_READ)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_READ)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  // Perform the I2C read operation
  hf_u64_t start_time = esp_timer_get_time();
  esp_err_t err = i2c_master_receive(handle_, data, length, timeout);
  hf_u64_t end_time = esp_timer_get_time();

  // Convert result and update statistics
  hf_i2c_err_t result = ConvertEspError(err);
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, length, end_time - start_time);
  
  // Cleanup sync operation
  CleanupSyncOperation();
  
  // Log result
  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C read successful: %d bytes in %lld us", length, end_time - start_time);
  }

  return result;
}

hf_i2c_err_t EspI2cDevice::WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length, hf_u8_t* rx_data,
                                     hf_u16_t rx_length, hf_u32_t timeout_ms) noexcept {
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "WriteRead() called on async-mode device. Use WriteReadAsync() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
    ESP_LOGE(TAG, "Invalid parameters for WriteRead operation");
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_WRITE_READ)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Use timeout_ms if specified, otherwise use default
  int timeout = (timeout_ms == 0) ? 1000 : static_cast<int>(timeout_ms);

  // Perform the I2C write-read operation
  hf_u64_t start_time = esp_timer_get_time();
  esp_err_t err = i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, timeout);
  hf_u64_t end_time = esp_timer_get_time();

  // Convert result and update statistics
  hf_i2c_err_t result = ConvertEspError(err);
  size_t total_bytes = tx_length + rx_length;
  UpdateStatistics(result == hf_i2c_err_t::I2C_SUCCESS, total_bytes, end_time - start_time);
  
  // Cleanup sync operation
  CleanupSyncOperation();
  
  // Log result
  if (result != hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "I2C write-read failed: %s", esp_err_to_name(err));
    diagnostics_.last_error_code = result;
    diagnostics_.last_error_timestamp_us = end_time;
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    ESP_LOGD(TAG, "I2C write-read successful: %d+%d bytes in %lld us", 
             tx_length, rx_length, end_time - start_time);
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
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC) {
    ESP_LOGE(TAG, "WriteAsync() called on sync-mode device. Use Write() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_WRITE_ASYNC)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    ESP_LOGE(TAG, "Callback required for async operation");
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Set operation context for this write operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE;

  // Start async transmission (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - mark as in progress
  async_operation_in_progress_ = true;
  async_start_time_ = esp_timer_get_time();

  ESP_LOGD(TAG, "Async write started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::ReadAsync(hf_u8_t* data, hf_u16_t length,
                                     hf_i2c_async_callback_t callback,
                                     void* user_data,
                                     hf_u32_t timeout_ms) noexcept {
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC) {
    ESP_LOGE(TAG, "ReadAsync() called on sync-mode device. Use Read() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_READ_ASYNC)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    ESP_LOGE(TAG, "Callback required for async operation");
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Set operation context for this read operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_READ;

  // Start async reception (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_receive(handle_, data, length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - mark as in progress
  async_operation_in_progress_ = true;
  async_start_time_ = esp_timer_get_time();

  ESP_LOGD(TAG, "Async read started for device 0x%02X: %d bytes", 
           config_.device_address, length);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::WriteReadAsync(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                          hf_u8_t* rx_data, hf_u16_t rx_length,
                                          hf_i2c_async_callback_t callback,
                                          void* user_data,
                                          hf_u32_t timeout_ms) noexcept {
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC) {
    ESP_LOGE(TAG, "WriteReadAsync() called on sync-mode device. Use WriteRead() instead.");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }
  
  // Validate operation parameters
  if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
    ESP_LOGE(TAG, "Invalid parameters for WriteReadAsync operation");
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  if (!callback) {
    ESP_LOGE(TAG, "Callback required for async operation");
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
  }

  // Set operation context for this write-read operation
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE_READ;

  // Start async write-read operation (returns immediately if callbacks registered)
  esp_err_t err = i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, 0);

  if (err != ESP_OK) {
    // Unregister callback on failure
    UnregisterTemporaryCallback();
    return ConvertEspError(err);
  }

  // I2C operation started successfully - mark as in progress
  async_operation_in_progress_ = true;
  async_start_time_ = esp_timer_get_time();

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

  // Simple polling wait - not ideal but safe
  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = static_cast<uint64_t>(timeout_ms) * 1000;
  
  while (async_operation_in_progress_) {
    if (timeout_ms > 0 && (esp_timer_get_time() - start_time) > timeout_us) {
      return false;
    }
    
    // Small delay to avoid busy waiting
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  
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

//==============================================//
// HELPER METHODS FOR COMMON OPERATIONS        //
//==============================================//

bool EspI2cDevice::ValidateOperation(const void* data, hf_u16_t length, hf_i2c_operation_t operation_type) noexcept {
  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot %s: device not properly initialized", HfI2COperationToString(operation_type));
    return false;
  }

  if (!data || length == 0) {
    ESP_LOGE(TAG, "Invalid parameters for %s operation", HfI2COperationToString(operation_type));
    return false;
  }

  return true;
}

bool EspI2cDevice::SetupSyncOperation(hf_i2c_operation_t operation_type) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Ensure no async operations are running before sync operation
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Cannot perform sync %s: async operation in progress", HfI2COperationToString(operation_type));
    return false;
  }
  
  // Check if device is in a healthy state
  if (!handle_) {
    ESP_LOGE(TAG, "Device handle is invalid - device may have been deinitialized");
    return false;
  }
  
  // CRITICAL FIX: Only clear callbacks in async mode (queue_depth > 0)
  // In sync mode (queue_depth = 0), ESP-IDF doesn't use callbacks, so no need to clear them
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    i2c_master_event_callbacks_t empty_cbs = {.on_trans_done = nullptr};
    esp_err_t clear_err = i2c_master_register_event_callbacks(handle_, &empty_cbs, nullptr);
    if (clear_err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to clear callbacks for sync %s: %s (continuing anyway)", 
               HfI2COperationToString(operation_type), esp_err_to_name(clear_err));
      // Don't fail the operation - this is just cleanup
    }
  }

  // CRITICAL: Ensure bus is ready before operation
  esp_err_t bus_ready_err = i2c_master_bus_wait_all_done(parent_bus_->GetHandle(), 100);
  if (bus_ready_err != ESP_OK) {
    ESP_LOGE(TAG, "Bus not ready for %s operation: %s", HfI2COperationToString(operation_type), esp_err_to_name(bus_ready_err));
    return false;
  }

  // Mark sync operation as in progress
  sync_operation_in_progress_ = true;
  return true;
}

void EspI2cDevice::CleanupSyncOperation() noexcept {
  sync_operation_in_progress_ = false;
}

bool EspI2cDevice::SetupAsyncOperation(hf_i2c_async_callback_t callback, void* user_data, hf_u32_t timeout_ms) noexcept {
  // Register temporary callback for this operation
  if (!RegisterTemporaryCallback(callback, user_data, timeout_ms)) {
    return false;
  }
  return true;
}



// PerformSyncWriteRead function removed - logic moved inline to WriteRead method

// All old PerformAsync* functions have been removed and replaced with inline implementations

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
  i2c_master_event_callbacks_t empty_cbs = { .on_trans_done = nullptr };
  esp_err_t err = i2c_master_register_event_callbacks(handle_, &empty_cbs, nullptr);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to unregister ESP-IDF callback: %s (continuing cleanup)", esp_err_to_name(err));
    // Continue with cleanup even if unregistration fails
  }

  // Clear callback state
  async_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;

  ESP_LOGD(TAG, "Temporary async callback unregistered for device 0x%02X", config_.device_address);
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
      case I2C_MASTER_EVENT_DONE:
        // Transaction completed successfully
        result = hf_i2c_err_t::I2C_SUCCESS;
        break;
        
      case I2C_MASTER_EVENT_NACK:
        // No ACK received - transaction failed
        result = hf_i2c_err_t::I2C_ERR_DEVICE_NOT_FOUND;
        break;
        
      case I2C_MASTER_EVENT_TIMEOUT:
        // Transaction timed out
        result = hf_i2c_err_t::I2C_ERR_TIMEOUT;
        break;
        
      case I2C_MASTER_EVENT_ALIVE:
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

  // Direct callback execution - simple and efficient
  // Store callback info before clearing it
  hf_i2c_async_callback_t callback = device->GetCurrentCallback();
  void* user_data = device->GetCurrentUserData();
  
  // Clear the async state
  device->async_operation_in_progress_ = false;
  device->current_callback_ = nullptr;
  device->current_user_data_ = nullptr;
  
  // Unregister the ESP-IDF callback (this allows next async operation)
  i2c_master_event_callbacks_t empty_cbs = { .on_trans_done = nullptr };
  esp_err_t err = i2c_master_register_event_callbacks(i2c_dev, &empty_cbs, nullptr);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to unregister ESP-IDF callback in completion: %s (continuing)", esp_err_to_name(err));
    // Continue with callback execution even if unregistration fails
  }
  
  // Call user callback with actual bytes transferred
  if (callback) {
    size_t bytes_transferred = evt_data ? evt_data->trans_len : 0;
    callback(result, bytes_transferred, user_data);
  }

  ESP_LOGD(TAG, "Async operation completed for device 0x%02X: %s (%zu bytes)", 
           device->config_.device_address, HfI2CErrToString(result).data(), 
           evt_data ? evt_data->trans_len : 0);

  return false; // No high priority wake needed
}





// Mode-aware operation methods
hf_i2c_mode_t EspI2cDevice::GetMode() const noexcept { 
  return device_mode_; 
}

bool EspI2cDevice::IsAsyncMode() const noexcept { 
  return device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC; 
}

bool EspI2cDevice::IsSyncMode() const noexcept { 
  return device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC; 
}
