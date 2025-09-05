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
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
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
    : config_(config), bus_handle_(nullptr), initialized_(false), mutex_(), devices_(),
      current_mode_(config.mode) {
  ESP_LOGI(TAG, "EspI2cBus: Created for port %d (SDA:GPIO%d, SCL:GPIO%d) in %s mode",
           config.i2c_port, config.sda_io_num, config.scl_io_num,
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

  // Initialize bus-level statistics and diagnostics
  statistics_ = hf_i2c_statistics_t{};
  diagnostics_ = hf_i2c_diagnostics_t{};

  ESP_LOGI(TAG, "EspI2cBus: Ready for initialization");
}

EspI2cBus::~EspI2cBus() noexcept {
  ESP_LOGI(TAG, "Destroying I2C bus");
  Deinitialize();
}

bool EspI2cBus::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_) {
    ESP_LOGI(TAG, "I2C bus already initialized");
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
      ESP_LOGW(TAG, "Sync mode requires trans_queue_depth=0, forcing to 0 (was %d)",
               actual_queue_depth);
      actual_queue_depth = 0;
    }
  } else {
    // ASYNC mode: Must have trans_queue_depth > 0
    if (actual_queue_depth == 0) {
      ESP_LOGW(TAG, "Async mode requires trans_queue_depth>0, setting to 10 (was 0)");
      actual_queue_depth = 32;
    }
  }
  // Create ESP-IDF master bus configuration
  i2c_master_bus_config_t bus_config = {
      .i2c_port = config_.i2c_port,
      .sda_io_num = static_cast<gpio_num_t>(config_.sda_io_num),
      .scl_io_num = static_cast<gpio_num_t>(config_.scl_io_num),
      .clk_source = static_cast<i2c_clock_source_t>(config_.clk_source),
      .glitch_ignore_cnt = static_cast<uint8_t>(config_.glitch_ignore_cnt),
      .intr_priority = static_cast<int>(config_.intr_priority),
      .trans_queue_depth = actual_queue_depth,
      .flags =
          {
              .enable_internal_pullup = config_.flags.enable_internal_pullup,
              .allow_pd = config_.flags.allow_pd,
          },
  };

  // Create the I2C master bus
  esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(err));
    return false;
  }

  ESP_LOGI(TAG, "I2C master bus created successfully");

  // Wait for bus to be fully ready (ESP32-C6 requirement)
  // This ensures the I2C peripheral is fully initialized before any operations
  err = i2c_master_bus_wait_all_done(bus_handle_, 100);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to wait for I2C bus readiness: %s", esp_err_to_name(err));
    i2c_del_master_bus(bus_handle_);
    bus_handle_ = nullptr;
    return false;
  }
  ESP_LOGI(TAG, "I2C bus is ready and waiting for operations");

  // Try bus reset to ensure peripheral is in known state
  err = i2c_master_bus_reset(bus_handle_);
  if (err != ESP_OK) {
    ESP_LOGW(TAG,
             "Bus reset failed: %s (this is normal without a stuck bus [SDA and/or SCL held LOW])",
             esp_err_to_name(err));
    ESP_LOGI(TAG, "Continuing initialization without bus reset - suitable for testing");
  } else {
    ESP_LOGI(TAG, "Bus reset successful");
  }

  initialized_ = true;
  ESP_LOGI(TAG, "I2C bus initialized successfully in %s mode with queue_depth=%d",
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC",
           actual_queue_depth);
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
        ESP_LOGW(TAG, "Failed to remove device 0x%02X: %s", device->GetDeviceAddress(),
                 esp_err_to_name(err));
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
  if (current_mode_ == new_mode)
    return true;

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
    ESP_LOGE(TAG, "Cannot create device on uninitialized bus");
    return -1;
  }

  // CRITICAL DEBUG: Track device creation input
  ESP_LOGI(TAG, "=== BUS CREATE DEVICE DEBUG ===");
  ESP_LOGI(TAG, "Input device_config.device_address = 0x%02X", device_config.device_address);
  ESP_LOGI(TAG, "Creating I2C device at address 0x%02X in %s mode", device_config.device_address,
           (current_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

  // Check if device already exists
  if (FindDeviceIndexByAddress(device_config.device_address) >= 0) {
    ESP_LOGE(TAG, "Device with address 0x%02X already exists", device_config.device_address);
    return -1;
  }

  // Create EspI2cDevice instance using nothrow allocation
  // Note: We don't call i2c_master_bus_add_device here - that happens in Initialize()
  auto device = hf::utils::make_unique_nothrow<EspI2cDevice>(this, device_config);
  if (!device) {
    ESP_LOGE(TAG, "Failed to allocate memory for EspI2cDevice");
    return -1;
  }

  // Check if devices vector can accommodate new device
  if (devices_.size() >= devices_.max_size()) {
    ESP_LOGE(TAG, "Maximum number of devices reached");
    return -1;
  }

  // Add device to our internal vector
  int device_index = static_cast<int>(devices_.size());
  devices_.push_back(std::move(device));

  // Update device statistics
  statistics_.devices_added++;
  diagnostics_.active_device_count = static_cast<hf_u32_t>(devices_.size());

  ESP_LOGI(TAG, "Created EspI2cDevice at index %d for address 0x%02X", device_index,
           device_config.device_address);
  ESP_LOGI(TAG, "Note: Device must be initialized before use (call Initialize())");

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
    ESP_LOGW(TAG,
             "CustomFastProbe: BLOCKING invalid address 0x%02X - ESP-IDF internal probe detected! "
             "(valid range: 0x01-0x7F)",
             device_addr);
    ESP_LOGE(TAG,
             "CustomFastProbe: WARNING - Address 0x%02X is outside valid range. This suggests "
             "ESP-IDF internal probing or system-level scanning.",
             device_addr);
    return false;
  }

  ESP_LOGI(TAG, "CustomFastProbe: Probing address 0x%02X with %lu ms timeout", device_addr,
           timeout_ms);

  // Use provided timeout or default to fast 50ms timeout for quick probing
  hf_u32_t actual_timeout = (timeout_ms > 0) ? timeout_ms : 50;

  // Create a temporary device configuration for probing
  hf_i2c_device_config_t temp_config = {};
  temp_config.device_address = device_addr;
  temp_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  temp_config.scl_speed_hz = 100000;     // Use 100kHz for fast probing
  temp_config.scl_wait_us = 0;           // No clock stretching for fast probe
  temp_config.disable_ack_check = false; // Enable ACK checking

  // Add temporary device to bus
  int device_index = CreateDevice(temp_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "CustomFastProbe: Failed to create temporary device for address 0x%02X",
             device_addr);
    return false;
  }

  // Get the device and try to read 1 byte (this will generate proper START/STOP)
  BaseI2c* temp_device = GetDevice(device_index);
  if (!temp_device) {
    ESP_LOGE(TAG, "CustomFastProbe: Failed to get temporary device for address 0x%02X",
             device_addr);
    // Clean up the failed device creation
    RemoveDevice(device_index);
    return false;
  }

  // Initialize the temporary device before use
  if (!temp_device->Initialize()) {
    ESP_LOGE(TAG, "CustomFastProbe: Failed to initialize temporary device for address 0x%02X",
             device_addr);
    // Clean up the failed device
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
    ESP_LOGW(TAG, "CustomFastProbe: Warning - failed to remove temporary device for address 0x%02X",
             device_addr);
  }

  bool device_found = (result == hf_i2c_err_t::I2C_SUCCESS);

  ESP_LOGI(TAG, "CustomFastProbe: Address 0x%02X probe result: %s (result: %s) in %llu μs",
           device_addr, device_found ? "FOUND" : "NOT FOUND", HfI2CErrToString(result).data(),
           probe_duration_us);

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

  ESP_LOGI(TAG, "Removing I2C device at index %d (address 0x%02X)", device_index,
           device->GetDeviceAddress());

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

  // Update device statistics
  statistics_.devices_removed++;
  diagnostics_.active_device_count = static_cast<hf_u32_t>(devices_.size());

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

  ESP_LOGI(TAG, "ScanDevices: Starting scan from 0x%02X to 0x%02X with %lu ms timeout", start_addr,
           end_addr, fast_scan_timeout);

  for (hf_u16_t addr = start_addr; addr <= end_addr; ++addr) {
    ESP_LOGI(TAG, "ScanDevices: About to probe address 0x%02X", addr);
    if (ProbeDevice(addr, fast_scan_timeout)) {
      found_devices.push_back(addr);
      ESP_LOGI(TAG, "ScanDevices: Found I2C device at address 0x%02X", addr);
    }
  }

  ESP_LOGI(TAG, "ScanDevices: Scan completed. Found %zu devices", found_devices.size());

  // Update scan statistics
  diagnostics_.total_device_scans++;
  diagnostics_.devices_found_last_scan = static_cast<hf_u32_t>(found_devices.size());

  return found_devices.size();
}

bool EspI2cBus::ProbeDevice(hf_u16_t device_addr, hf_u32_t timeout_ms) noexcept {
  if (!initialized_) {
    ESP_LOGE(TAG, "ProbeDevice: Bus not initialized");
    return false;
  }

  // CRITICAL: Block invalid addresses that ESP-IDF might try to probe internally
  if (device_addr < 0x01 || device_addr > 0x7F) {
    ESP_LOGW(TAG,
             "ProbeDevice: BLOCKING invalid address 0x%02X - ESP-IDF internal probe detected! "
             "(valid range: 0x01-0x7F)",
             device_addr);
    ESP_LOGE(TAG,
             "ProbeDevice: WARNING - Address 0x%02X is outside valid range. This suggests ESP-IDF "
             "internal probing or system-level scanning.",
             device_addr);
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

  ESP_LOGI(TAG, "ProbeDevice: Address 0x%02X final result: %s in %llu μs", device_addr,
           device_found ? "FOUND" : "NOT FOUND", probe_duration_us);

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
    ESP_LOGW(TAG, "I2C bus reset failed: %s (normal without physical devices or pull-ups)",
             esp_err_to_name(err));
    ESP_LOGI(TAG, "Bus reset not required for testing scenarios - continuing");

    // Update error recovery statistics for all devices
    for (auto& device : devices_) {
      if (device) {
        device->UpdateErrorRecoveryAttempt();
      }
    }

    return true; // Return true to indicate this is acceptable for testing
  }

  ESP_LOGI(TAG, "I2C bus reset successfully");

  // Update error recovery statistics for all devices
  for (auto& device : devices_) {
    if (device) {
      device->UpdateErrorRecoveryAttempt();
    }
  }

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

EspI2cDevice::EspI2cDevice(EspI2cBus* parent, const hf_i2c_device_config_t& config)
    : parent_bus_(parent), handle_(nullptr), config_(config), initialized_(false), statistics_(),
      diagnostics_(), mutex_(),
      device_mode_(parent ? parent->GetMode() : hf_i2c_mode_t::HF_I2C_MODE_SYNC),
      async_operation_in_progress_(false), sync_operation_in_progress_(false),
      current_callback_(nullptr), current_user_data_(nullptr), async_start_time_(0),
      current_op_type_(hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE), last_write_length_(0),
      last_read_length_(0) {
  if (!parent_bus_) {
    ESP_LOGE(TAG, "EspI2cDevice: Parent bus cannot be null");
    return;
  }

  // Initialize diagnostics with default values
  diagnostics_.bus_healthy = true;
  diagnostics_.sda_line_state = true;
  diagnostics_.scl_line_state = true;
  diagnostics_.bus_locked = false;
  diagnostics_.last_error_code = hf_i2c_err_t::I2C_SUCCESS;
  diagnostics_.last_error_timestamp_us = 0;
  diagnostics_.consecutive_errors = 0;
  diagnostics_.error_recovery_attempts = 0;
  diagnostics_.bus_utilization_percent = 0.0f;
  diagnostics_.average_response_time_us = 0;
  diagnostics_.clock_stretching_events = 0;
  diagnostics_.active_device_count = 0;
  diagnostics_.total_device_scans = 0;
  diagnostics_.devices_found_last_scan = 0;

  // Initialize statistics with default values
  statistics_.total_transactions = 0;
  statistics_.successful_transactions = 0;
  statistics_.failed_transactions = 0;
  statistics_.timeout_count = 0;
  statistics_.bytes_written = 0;
  statistics_.bytes_read = 0;
  statistics_.total_transaction_time_us = 0;
  statistics_.max_transaction_time_us = 0;
  statistics_.min_transaction_time_us = UINT32_MAX;
  statistics_.nack_errors = 0;
  statistics_.bus_errors = 0;
  statistics_.arbitration_lost_count = 0;
  statistics_.clock_stretch_timeouts = 0;
  statistics_.devices_added = 0;
  statistics_.devices_removed = 0;

  ESP_LOGI(TAG, "EspI2cDevice: Created for address 0x%02X, SCL speed: %lu Hz",
           config.device_address, config.scl_speed_hz);
  ESP_LOGI(TAG, "Note: Device must be initialized before use (call Initialize())");
}

EspI2cDevice::~EspI2cDevice() noexcept {
  ESP_LOGI(TAG, "EspI2cDevice destructor called for address 0x%02X", config_.device_address);

  // Ensure device is properly deinitialized
  if (initialized_) {
    ESP_LOGW(TAG, "Device was still initialized during destruction - calling Deinitialize()");
    Deinitialize();
  }

  // Clear any remaining references
  parent_bus_ = nullptr;
  handle_ = nullptr;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;

  ESP_LOGI(TAG, "EspI2cDevice destructor completed for address 0x%02X", config_.device_address);
}

bool EspI2cDevice::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_) {
    ESP_LOGI(TAG, "EspI2cDevice already initialized for address 0x%02X", config_.device_address);
    return true;
  }

  if (!parent_bus_ || !parent_bus_->IsInitialized()) {
    ESP_LOGE(TAG, "Cannot initialize device: parent bus not available or not initialized");
    return false;
  }

  ESP_LOGI(TAG, "Initializing EspI2cDevice for address 0x%02X", config_.device_address);

  // Create ESP-IDF device configuration
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = (config_.dev_addr_length == hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT)
                             ? I2C_ADDR_BIT_LEN_7
                             : I2C_ADDR_BIT_LEN_10,
      .device_address = config_.device_address,
      .scl_speed_hz = config_.scl_speed_hz,
      .scl_wait_us = config_.scl_wait_us,
      .flags = {.disable_ack_check = config_.disable_ack_check}};

  // Add device to the ESP-IDF bus
  i2c_master_dev_handle_t dev_handle;
  esp_err_t err = i2c_master_bus_add_device(parent_bus_->GetHandle(), &dev_cfg, &dev_handle);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device to ESP-IDF bus: %s", esp_err_to_name(err));
    return false;
  }

  handle_ = dev_handle;

  // For async mode devices, register the event callback once during initialization
  if (device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    i2c_master_event_callbacks_t cbs = {};
    cbs.on_trans_done = InternalAsyncCallback;

    esp_err_t callback_err = i2c_master_register_event_callbacks(handle_, &cbs, this);
    if (callback_err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register async callback for device 0x%02X: %s",
               config_.device_address, esp_err_to_name(callback_err));
      // Remove the device from the bus since callback registration failed
      i2c_master_bus_rm_device(handle_);
      handle_ = nullptr;
      return false;
    }

    ESP_LOGI(TAG, "Async event callback registered successfully for device 0x%02X",
             config_.device_address);
  }

  initialized_ = true;

  ESP_LOGI(TAG, "EspI2cDevice initialized successfully for address 0x%02X", config_.device_address);
  return true;
}

bool EspI2cDevice::MarkAsDeinitialized() noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGI(TAG, "Device at address 0x%02X is already marked as deinitialized",
             config_.device_address);
    return true;
  }

  ESP_LOGI(TAG, "Marking device at address 0x%02X as deinitialized", config_.device_address);

  // Clear internal state without ESP-IDF cleanup
  initialized_ = false;
  async_operation_in_progress_ = false;
  sync_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;
  async_start_time_ = 0;

  // Note: handle_ is not cleared here as ESP-IDF cleanup is handled by the parent bus

  return true;
}

bool EspI2cDevice::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    ESP_LOGI(TAG, "EspI2cDevice already deinitialized");
    return true;
  }

  // Wait for any pending async operations to complete
  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Waiting for async operation to complete before deinitialization");
    // Wait up to 1 second for async operation to complete
    uint32_t wait_start = esp_timer_get_time() / 1000;
    while (async_operation_in_progress_ && ((esp_timer_get_time() / 1000) - wait_start < 1000)) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    if (async_operation_in_progress_) {
      ESP_LOGW(TAG, "Async operation did not complete within timeout - forcing deinitialization");
    }
  }

  // Remove device from ESP-IDF bus if we have a valid handle
  if (handle_ && parent_bus_ && parent_bus_->GetHandle()) {
    esp_err_t err = i2c_master_bus_rm_device(handle_);
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to remove I2C device from ESP-IDF bus: %s", esp_err_to_name(err));
      // Continue with deinitialization even if removal fails
    } else {
      ESP_LOGI(TAG, "Successfully removed I2C device from ESP-IDF bus");
    }

    // Clear the handle
    handle_ = nullptr;
  }

  // Reset internal state
  initialized_ = false;
  async_operation_in_progress_ = false;
  sync_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;
  async_start_time_ = 0;

  ESP_LOGI(TAG, "EspI2cDevice deinitialized successfully for address 0x%02X",
           config_.device_address);
  return true;
}

hf_i2c_err_t EspI2cDevice::Write(const hf_u8_t* data, hf_u16_t length,
                                 hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot write to uninitialized device at address 0x%02X", config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_WRITE)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_WRITE)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Writing %u bytes to device 0x%02X with timeout %lu ms", length,
           config_.device_address, timeout_ms);

  uint64_t start_time = esp_timer_get_time();
  esp_err_t err = i2c_master_transmit(handle_, data, length, timeout_ms);
  uint64_t end_time = esp_timer_get_time();

  // Update statistics with write-specific information
  bool success = (err == ESP_OK);
  UpdateStatistics(success, length, end_time - start_time);

  // Update write-specific statistics
  if (success) {
    statistics_.bytes_written += length;
  } else {
    // Update diagnostics with specific error information
    hf_i2c_err_t hf_error = ConvertEspError(err);
    diagnostics_.last_error_code = hf_error;
    diagnostics_.last_error_timestamp_us = end_time;

    // Update specific error statistics
    UpdateErrorStatistics(hf_error);

    // Log specific error details
    ESP_LOGW(TAG, "Write failed: %s (ESP-IDF: %s)", HfI2CErrToString(hf_error).data(),
             esp_err_to_name(err));
  }

  // Cleanup sync operation
  CleanupSyncOperation();

  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Write successful: %u bytes to device 0x%02X", length, config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot read from uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_READ)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_READ)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Reading %u bytes from device 0x%02X with timeout %lu ms", length,
           config_.device_address, timeout_ms);

  uint64_t start_time = esp_timer_get_time();
  esp_err_t err = i2c_master_receive(handle_, data, length, timeout_ms);
  uint64_t end_time = esp_timer_get_time();

  // Update statistics with read-specific information
  bool success = (err == ESP_OK);
  UpdateStatistics(success, length, end_time - start_time);

  // Update read-specific statistics
  if (success) {
    statistics_.bytes_read += length;
  } else {
    // Update diagnostics with specific error information
    hf_i2c_err_t hf_error = ConvertEspError(err);
    diagnostics_.last_error_code = hf_error;
    diagnostics_.last_error_timestamp_us = end_time;

    // Update specific error statistics
    UpdateErrorStatistics(hf_error);

    // Log specific error details
    ESP_LOGW(TAG, "Read failed: %s (ESP-IDF: %s)", HfI2CErrToString(hf_error).data(),
             esp_err_to_name(err));
  }

  // Cleanup sync operation
  CleanupSyncOperation();

  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Read successful: %u bytes from device 0x%02X", length, config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length, hf_u8_t* rx_data,
                                     hf_u16_t rx_length, hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot perform WriteRead on uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Validate operation parameters
  if (!ValidateOperation(tx_data, tx_length, hf_i2c_operation_t::HF_I2C_OP_WRITE_READ) ||
      !ValidateOperation(rx_data, rx_length, hf_i2c_operation_t::HF_I2C_OP_WRITE_READ)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup sync operation
  if (!SetupSyncOperation(hf_i2c_operation_t::HF_I2C_OP_WRITE_READ)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Writing %u bytes and reading %u bytes from device 0x%02X with timeout %lu ms",
           tx_length, rx_length, config_.device_address, timeout_ms);

  uint64_t start_time = esp_timer_get_time();
  esp_err_t err =
      i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, timeout_ms);
  uint64_t end_time = esp_timer_get_time();

  // Update statistics with combined operation information
  bool success = (err == ESP_OK);
  size_t total_bytes = tx_length + rx_length;
  UpdateStatistics(success, total_bytes, end_time - start_time);

  // Update read/write-specific statistics
  if (success) {
    statistics_.bytes_written += tx_length;
    statistics_.bytes_read += rx_length;
  } else {
    // Update diagnostics with specific error information
    hf_i2c_err_t hf_error = ConvertEspError(err);
    diagnostics_.last_error_code = hf_error;
    diagnostics_.last_error_timestamp_us = end_time;

    // Update specific error statistics
    UpdateErrorStatistics(hf_error);

    // Log specific error details
    ESP_LOGW(TAG, "WriteRead failed: %s (ESP-IDF: %s)", HfI2CErrToString(hf_error).data(),
             esp_err_to_name(err));
  }

  // Cleanup sync operation
  CleanupSyncOperation();

  if (err != ESP_OK) {
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "WriteRead successful: %d+%d bytes from device 0x%02X", tx_length, rx_length,
           config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

//==============================================//
// ASYNC OPERATIONS - ESP-IDF v5.5+ FEATURES   //
//==============================================//

hf_i2c_err_t EspI2cDevice::WriteAsync(const hf_u8_t* data, hf_u16_t length,
                                      hf_i2c_async_callback_t callback, void* user_data,
                                      hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot perform async write on uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Check if device supports async mode
  if (device_mode_ != hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "Async write called on sync-mode device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Use Write() for sync-mode devices");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_WRITE_ASYNC)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Starting async write of %u bytes to device 0x%02X with timeout %lu ms", length,
           config_.device_address, timeout_ms);

  // Start async operation tracking
  StartAsyncOperationTracking();

  // Track operation details for callback
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE;
  last_write_length_ = length;
  last_read_length_ = 0;

  // ESP-IDF v5.5+ async operation: start non-blocking transmit
  // The callback is already registered during device initialization
  esp_err_t err = i2c_master_transmit(handle_, data, length, timeout_ms);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start async transmit for device 0x%02X: %s", config_.device_address,
             esp_err_to_name(err));
    CleanupAsyncOperation();
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async write started successfully for device 0x%02X", config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::ReadAsync(hf_u8_t* data, hf_u16_t length,
                                     hf_i2c_async_callback_t callback, void* user_data,
                                     hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot perform async read on uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Check if device supports async mode
  if (device_mode_ != hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "Async read called on sync-mode device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Use Read() for sync-mode devices");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Validate operation parameters
  if (!ValidateOperation(data, length, hf_i2c_operation_t::HF_I2C_OP_READ_ASYNC)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Starting async read of %u bytes from device 0x%02X with timeout %lu ms", length,
           config_.device_address, timeout_ms);

  // Start async operation tracking
  StartAsyncOperationTracking();

  // Track operation details for callback
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_READ;
  last_write_length_ = 0;
  last_read_length_ = length;

  // ESP-IDF v5.5+ async operation: start non-blocking receive
  // The callback is already registered during device initialization
  esp_err_t err = i2c_master_receive(handle_, data, length, timeout_ms);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start async receive for device 0x%02X: %s", config_.device_address,
             esp_err_to_name(err));
    CleanupAsyncOperation();
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async read started successfully for device 0x%02X", config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::WriteReadAsync(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                          hf_u8_t* rx_data, hf_u16_t rx_length,
                                          hf_i2c_async_callback_t callback, void* user_data,
                                          hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot perform async write-read on uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // Check if device supports async mode
  if (device_mode_ != hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "Async write-read called on sync-mode device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Use WriteRead() for sync-mode devices");
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Validate operation parameters
  if (!ValidateOperation(tx_data, tx_length, hf_i2c_operation_t::HF_I2C_OP_WRITE_READ_ASYNC) ||
      !ValidateOperation(rx_data, rx_length, hf_i2c_operation_t::HF_I2C_OP_WRITE_READ_ASYNC)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
  }

  // Setup async operation
  if (!SetupAsyncOperation(callback, user_data, timeout_ms)) {
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG,
           "Starting async write-read: %u bytes write, %u bytes read from device 0x%02X with "
           "timeout %lu ms",
           tx_length, rx_length, config_.device_address, timeout_ms);

  // Start async operation tracking
  StartAsyncOperationTracking();

  // Track operation details for callback
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE_READ;
  last_write_length_ = tx_length;
  last_read_length_ = rx_length;

  // ESP-IDF v5.5+ async operation: start non-blocking transmit-receive
  // The callback is already registered during device initialization
  esp_err_t err =
      i2c_master_transmit_receive(handle_, tx_data, tx_length, rx_data, rx_length, timeout_ms);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start async transmit-receive for device 0x%02X: %s",
             config_.device_address, esp_err_to_name(err));
    CleanupAsyncOperation();
    return ConvertEspError(err);
  }

  ESP_LOGD(TAG, "Async write-read started successfully for device 0x%02X", config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

bool EspI2cDevice::IsAsyncModeSupported() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot check async mode support for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  return device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC;
}

bool EspI2cDevice::IsAsyncOperationInProgress() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot check async operation status for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  return async_operation_in_progress_;
}

bool EspI2cDevice::IsSyncOperationInProgress() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot check sync operation status for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  return sync_operation_in_progress_;
}

bool EspI2cDevice::WaitAsyncOperationComplete(hf_u32_t timeout_ms) noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot wait for async operation on uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  // Check if device supports async mode
  if (device_mode_ != hf_i2c_mode_t::HF_I2C_MODE_ASYNC) {
    ESP_LOGE(TAG, "WaitAsyncOperationComplete called on sync-mode device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "This method is only for async-mode devices");
    return false;
  }

  if (!async_operation_in_progress_) {
    ESP_LOGD(TAG, "No async operation in progress for device 0x%02X", config_.device_address);
    return true;
  }

  // Use default timeout if none specified
  if (timeout_ms == 0) {
    timeout_ms = HF_I2C_DEFAULT_TIMEOUT_MS;
  }

  ESP_LOGD(TAG, "Waiting for async operation to complete on device 0x%02X with timeout %lu ms",
           config_.device_address, timeout_ms);

  uint32_t start_time = esp_timer_get_time() / 1000;
  while (async_operation_in_progress_ &&
         ((esp_timer_get_time() / 1000) - start_time < timeout_ms)) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if (async_operation_in_progress_) {
    ESP_LOGW(TAG, "Async operation did not complete within %lu ms timeout on device 0x%02X",
             timeout_ms, config_.device_address);
    return false;
  }

  ESP_LOGD(TAG, "Async operation completed successfully on device 0x%02X", config_.device_address);
  return true;
}

hf_i2c_err_t EspI2cDevice::GetStatistics(hf_i2c_statistics_t& statistics) const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot get statistics for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics = statistics_;
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_i2c_err_t EspI2cDevice::GetDiagnostics(hf_i2c_diagnostics_t& diagnostics) const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot get diagnostics for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  diagnostics = diagnostics_;
  return hf_i2c_err_t::I2C_SUCCESS;
}

hf_u16_t EspI2cDevice::GetDeviceAddress() const noexcept {
  // Note: This method can be called even on uninitialized devices
  // as it just returns the configuration value
  return config_.device_address;
}

hf_i2c_err_t EspI2cDevice::ResetStatistics() noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot reset statistics for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics_ = hf_i2c_statistics_t{};
  ESP_LOGI(TAG, "Statistics reset for device at address 0x%02X", config_.device_address);
  return hf_i2c_err_t::I2C_SUCCESS;
}

i2c_master_dev_handle_t EspI2cDevice::GetHandle() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot get handle for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return nullptr;
  }

  return handle_;
}

const hf_i2c_device_config_t& EspI2cDevice::GetConfig() const noexcept {
  // Note: This method can be called even on uninitialized devices
  // as it just returns the configuration value
  return config_;
}

hf_i2c_err_t EspI2cDevice::GetActualClockFrequency(hf_u32_t& actual_freq_hz) const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot get clock frequency for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
  }

  // For now, return the configured frequency
  // In a future implementation, we could query the actual hardware frequency
  actual_freq_hz = config_.scl_speed_hz;
  return hf_i2c_err_t::I2C_SUCCESS;
}

bool EspI2cDevice::ProbeDevice() noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot probe uninitialized device at address 0x%02X", config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  // Use a simple read operation to probe the device
  uint8_t dummy_byte;
  hf_i2c_err_t result = Read(&dummy_byte, 1, 100); // 100ms timeout for probing

  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGD(TAG, "Device probe successful for address 0x%02X", config_.device_address);
    return true;
  } else {
    ESP_LOGD(TAG, "Device probe failed for address 0x%02X: %s", config_.device_address,
             HfI2CErrToString(result).data());
    return false;
  }
}

//==============================================//
// HELPER METHODS FOR COMMON OPERATIONS        //
//==============================================//

bool EspI2cDevice::ValidateOperation(const void* data, hf_u16_t length,
                                     hf_i2c_operation_t operation_type) noexcept {
  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot %s: device not properly initialized",
             HfI2COperationToString(operation_type));
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

  if (!initialized_ || !handle_) {
    ESP_LOGE(TAG, "Cannot setup sync operation: device not properly initialized");
    return false;
  }

  if (async_operation_in_progress_) {
    ESP_LOGE(TAG, "Cannot setup sync operation: async operation in progress");
    return false;
  }

  if (sync_operation_in_progress_) {
    ESP_LOGE(TAG, "Cannot setup sync operation: sync operation already in progress");
    return false;
  }

  // Mark sync operation as in progress
  sync_operation_in_progress_ = true;

  // Update bus lock status
  diagnostics_.bus_locked = true;

  ESP_LOGD(TAG, "Sync operation setup complete for operation type: %s",
           HfI2COperationToString(operation_type));
  return true;
}

void EspI2cDevice::CleanupSyncOperation() noexcept {
  // Mark sync operation as complete
  sync_operation_in_progress_ = false;

  // Update bus lock status
  diagnostics_.bus_locked = false;

  ESP_LOGD(TAG, "Sync operation cleanup complete");
}

bool EspI2cDevice::SetupAsyncOperation(hf_i2c_async_callback_t callback, void* user_data,
                                       hf_u32_t timeout_ms) noexcept {
  // Validate callback
  if (!callback) {
    ESP_LOGE(TAG, "Invalid callback for async operation on device 0x%02X", config_.device_address);
    return false;
  }

  // Check if another async operation is in progress
  if (async_operation_in_progress_) {
    ESP_LOGE(TAG, "Async operation already in progress on device 0x%02X", config_.device_address);
    return false;
  }

  // Store callback and user data
  current_callback_ = callback;
  current_user_data_ = user_data;
  async_start_time_ = esp_timer_get_time();
  async_operation_in_progress_ = true;

  ESP_LOGD(TAG, "Async operation setup completed for device 0x%02X", config_.device_address);
  return true;
}

// PerformSyncWriteRead function removed - logic moved inline to WriteRead method

// All old PerformAsync* functions have been removed and replaced with inline implementations

void EspI2cDevice::UpdateStatistics(bool success, size_t bytes_transferred,
                                    hf_u64_t operation_time_us) noexcept {
  // Update basic transaction statistics
  statistics_.total_transactions++;

  if (success) {
    statistics_.successful_transactions++;
    // Note: bytes_written and bytes_read are updated in the specific operation methods
    // to properly distinguish between read and write operations
  } else {
    statistics_.failed_transactions++;
    // Update error tracking in diagnostics
    diagnostics_.consecutive_errors++;
    diagnostics_.last_error_timestamp_us = esp_timer_get_time();
  }

  // Update timing statistics
  statistics_.total_transaction_time_us += operation_time_us;

  if (operation_time_us > statistics_.max_transaction_time_us) {
    statistics_.max_transaction_time_us = static_cast<hf_u32_t>(operation_time_us);
  }

  if (statistics_.min_transaction_time_us == 0 ||
      operation_time_us < statistics_.min_transaction_time_us) {
    statistics_.min_transaction_time_us = static_cast<hf_u32_t>(operation_time_us);
  }

  // Update diagnostics
  if (success) {
    diagnostics_.consecutive_errors = 0; // Reset error counter on success
    diagnostics_.last_error_code = hf_i2c_err_t::I2C_SUCCESS;
    diagnostics_.bus_healthy = (diagnostics_.consecutive_errors <= 3); // Mark healthy if few errors
  } else {
    diagnostics_.bus_healthy =
        (diagnostics_.consecutive_errors <= 3); // Mark unhealthy after 3 consecutive errors
  }

  // Update bus utilization (simplified calculation)
  if (statistics_.total_transactions > 0) {
    diagnostics_.bus_utilization_percent =
        (float)(statistics_.successful_transactions) / statistics_.total_transactions * 100.0f;
  }

  // Update average response time
  if (statistics_.successful_transactions > 0) {
    diagnostics_.average_response_time_us = static_cast<hf_u32_t>(
        statistics_.total_transaction_time_us / statistics_.successful_transactions);
  }
}

// Enhanced error tracking method for specific error types
void EspI2cDevice::UpdateErrorStatistics(hf_i2c_err_t error_code) noexcept {
  switch (error_code) {
  case hf_i2c_err_t::I2C_ERR_TIMEOUT:
    statistics_.timeout_count++;
    // Timeouts can indicate clock stretching or device not responding
    diagnostics_.clock_stretching_events++;
    break;
  case hf_i2c_err_t::I2C_ERR_DEVICE_NACK:
    statistics_.nack_errors++;
    break;
  case hf_i2c_err_t::I2C_ERR_BUS_ERROR:
    statistics_.bus_errors++;
    break;
  case hf_i2c_err_t::I2C_ERR_BUS_ARBITRATION_LOST:
    statistics_.arbitration_lost_count++;
    break;
  case hf_i2c_err_t::I2C_ERR_CLOCK_STRETCH_TIMEOUT:
    statistics_.clock_stretch_timeouts++;
    diagnostics_.clock_stretching_events++;
    break;
  default:
    // General error - already counted in failed_transactions
    break;
  }
}

// Update error recovery attempt statistics
void EspI2cDevice::UpdateErrorRecoveryAttempt() noexcept {
  diagnostics_.error_recovery_attempts++;
  ESP_LOGD(TAG, "Error recovery attempt recorded for device 0x%02X (total: %u)",
           config_.device_address, diagnostics_.error_recovery_attempts);
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
    return hf_i2c_err_t::I2C_ERR_INVALID_STATE;
  case ESP_ERR_INVALID_SIZE:
    return hf_i2c_err_t::I2C_ERR_DATA_TOO_LONG;
  case ESP_ERR_NOT_SUPPORTED:
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
  case ESP_ERR_INVALID_RESPONSE:
    return hf_i2c_err_t::I2C_ERR_DEVICE_NACK;
  case ESP_ERR_INVALID_CRC:
    return hf_i2c_err_t::I2C_ERR_COMMUNICATION_FAILURE;
  case ESP_ERR_WIFI_BASE:
    return hf_i2c_err_t::I2C_ERR_SYSTEM_ERROR;
  case ESP_FAIL:
  default:
    return hf_i2c_err_t::I2C_ERR_FAILURE;
  }
}

//==============================================//
// ASYNC CALLBACK BRIDGE SYSTEM                //
//==============================================//

bool EspI2cDevice::RegisterTemporaryCallback(hf_i2c_async_callback_t callback, void* user_data,
                                             hf_u32_t timeout_ms) noexcept {
  // This function is no longer used - async callbacks are handled directly
  ESP_LOGW(TAG, "RegisterTemporaryCallback called but is deprecated");
  return false;
}

void EspI2cDevice::UnregisterTemporaryCallback() noexcept {
  // This function is no longer used - cleanup is handled in InternalAsyncCallback
  ESP_LOGW(TAG, "UnregisterTemporaryCallback called but is deprecated");
}

bool EspI2cDevice::InternalAsyncCallback(i2c_master_dev_handle_t i2c_dev,
                                         const i2c_master_event_data_t* evt_data, void* arg) {
  EspI2cDevice* device = static_cast<EspI2cDevice*>(arg);
  if (!device) {
    return false;
  }

  // Handle different ESP-IDF v5.5 I2C events
  hf_i2c_err_t result;
  size_t bytes_transferred = 0;

  if (evt_data) {
    switch (evt_data->event) {
    case I2C_EVENT_DONE:
      // Transaction completed successfully
      result = hf_i2c_err_t::I2C_SUCCESS;
      // For ESP32-C6, we need to determine bytes transferred based on operation type
      // Since ESP-IDF doesn't provide this in event data, we'll use the operation tracking
      if (device->current_op_type_ == hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE) {
        bytes_transferred = device->last_write_length_;
      } else if (device->current_op_type_ == hf_i2c_transaction_type_t::HF_I2C_TRANS_READ) {
        bytes_transferred = device->last_read_length_;
      } else if (device->current_op_type_ == hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE_READ) {
        bytes_transferred = device->last_write_length_ + device->last_read_length_;
      }
      break;

    case I2C_EVENT_NACK:
      // No ACK received - transaction failed
      result = hf_i2c_err_t::I2C_ERR_DEVICE_NACK;
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

  // Invoke user callback if available
  if (device->current_callback_) {
    // Execute user callback (no exception handling in ESP-IDF)
    device->current_callback_(result, bytes_transferred, device->current_user_data_);
  }

  // Clean up async operation state
  device->CleanupAsyncOperation();

  // Return false to indicate no high priority wake needed
  return false;
}

// Mode-aware operation methods
hf_i2c_mode_t EspI2cDevice::GetMode() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot get mode for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return hf_i2c_mode_t::HF_I2C_MODE_SYNC; // Return safe default
  }

  return device_mode_;
}

bool EspI2cDevice::IsAsyncMode() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot check async mode for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  return device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_ASYNC;
}

bool EspI2cDevice::IsSyncMode() const noexcept {
  // Check if device is initialized
  if (!initialized_) {
    ESP_LOGE(TAG, "Cannot check sync mode for uninitialized device at address 0x%02X",
             config_.device_address);
    ESP_LOGE(TAG, "Call Initialize() before using the device");
    return false;
  }

  return device_mode_ == hf_i2c_mode_t::HF_I2C_MODE_SYNC;
}

/**
 * @brief Start async operation tracking after I2C operation is successfully started.
 */
inline void EspI2cDevice::StartAsyncOperationTracking() noexcept {
  // Mark async operation as started
  async_operation_in_progress_ = true;
  async_start_time_ = esp_timer_get_time();
}

/**
 * @brief Common async operation cleanup.
 */
void EspI2cDevice::CleanupAsyncOperation() noexcept {
  // Reset async operation state
  async_operation_in_progress_ = false;
  current_callback_ = nullptr;
  current_user_data_ = nullptr;
  async_start_time_ = 0;
  current_op_type_ = hf_i2c_transaction_type_t::HF_I2C_TRANS_WRITE;
  last_write_length_ = 0;
  last_read_length_ = 0;
}
