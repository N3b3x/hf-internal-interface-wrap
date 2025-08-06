/**
 * @file I2cComprehensiveTest.cpp
 * @brief Comprehensive I2C testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspI2c class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * I2C functionalities including bus-device architecture, various operation modes,
 * device scanning, statistics tracking, and hardware-specific capabilities.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseI2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspI2c.h"
#include "mcu/esp32/utils/EspTypes_I2C.h"

#include "TestFramework.h"

static const char* TAG = "I2C_Test";

static TestResults g_test_results;

// Forward declarations
bool test_i2c_bus_initialization() noexcept;
bool test_i2c_multiple_bus_initialization() noexcept;
bool test_i2c_bus_configuration_validation() noexcept;
bool test_i2c_device_operations() noexcept;
bool test_i2c_multiple_devices_on_bus() noexcept;
bool test_i2c_device_configuration_variations() noexcept;
bool test_i2c_write_operations() noexcept;
bool test_i2c_read_operations() noexcept;
bool test_i2c_write_read_operations() noexcept;
bool test_i2c_device_scanning() noexcept;
bool test_i2c_device_probing() noexcept;
bool test_i2c_error_handling() noexcept;
bool test_i2c_statistics_tracking() noexcept;
bool test_i2c_clock_frequency_testing() noexcept;
bool test_i2c_device_removal() noexcept;
bool test_i2c_bus_reset() noexcept;
bool test_i2c_concurrent_operations() noexcept;
bool test_i2c_performance() noexcept;

bool test_i2c_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing I2C bus initialization...");

  // Test basic bus initialization
  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);

  if (!test_i2c_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  // Test bus status
  if (!test_i2c_bus.IsInitialized()) {
    ESP_LOGE(TAG, "Bus should be initialized");
    return false;
  }

  // Test bus configuration retrieval
  const auto& config = test_i2c_bus.GetConfig();
  if (config.i2c_port != I2C_NUM_0 || config.sda_io_num != 21 || config.scl_io_num != 22) {
    ESP_LOGE(TAG, "Bus configuration mismatch");
    return false;
  }

  // Test port retrieval
  if (test_i2c_bus.GetPort() != I2C_NUM_0) {
    ESP_LOGE(TAG, "Port mismatch");
    return false;
  }

  // Test double initialization (should be safe)
  if (!test_i2c_bus.Initialize()) {
    ESP_LOGE(TAG, "Double initialization failed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C bus initialization successful");
  return true;
}

bool test_i2c_multiple_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing multiple I2C bus initialization...");

  // Test I2C_NUM_0 bus
  hf_i2c_master_bus_config_t i2c0_cfg = {};
  i2c0_cfg.i2c_port = I2C_NUM_0;
  i2c0_cfg.sda_io_num = 21;
  i2c0_cfg.scl_io_num = 22;
  i2c0_cfg.enable_internal_pullup = true;

  EspI2cBus i2c0_bus(i2c0_cfg);
  if (!i2c0_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C0 bus");
    return false;
  }

  // Test I2C_NUM_1 bus (if available)
  hf_i2c_master_bus_config_t i2c1_cfg = {};
  i2c1_cfg.i2c_port = I2C_NUM_1;
  i2c1_cfg.sda_io_num = 18;
  i2c1_cfg.scl_io_num = 19;
  i2c1_cfg.enable_internal_pullup = false; // External pull-ups
  i2c1_cfg.clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT;

  EspI2cBus i2c1_bus(i2c1_cfg);
  if (!i2c1_bus.Initialize()) {
    ESP_LOGW(TAG, "I2C1 initialization failed (may not be available on this ESP32 variant)");
  } else {
    ESP_LOGI(TAG, "I2C1 bus initialized successfully");
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple I2C bus initialization completed");
  return true;
}

bool test_i2c_bus_configuration_validation() noexcept {
  ESP_LOGI(TAG, "Testing I2C bus configuration validation...");

  // Test different clock sources
  const hf_i2c_clock_source_t clock_sources[] = {
    hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT,
    hf_i2c_clock_source_t::HF_I2C_CLK_SRC_APB,
    hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL
  };

  for (auto clk_src : clock_sources) {
    hf_i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = I2C_NUM_0;
    cfg.sda_io_num = 21;
    cfg.scl_io_num = 22;
    cfg.clk_source = clk_src;
    cfg.enable_internal_pullup = true;

    EspI2cBus bus(cfg);
    if (!bus.Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize bus with clock source %d", static_cast<int>(clk_src));
      return false;
    }
    ESP_LOGI(TAG, "Bus initialized with clock source %d", static_cast<int>(clk_src));
  }

  // Test different glitch filter settings
  const hf_i2c_glitch_filter_t filters[] = {
    hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_1_CYCLE,
    hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_3_CYCLES,
    hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES
  };

  for (auto filter : filters) {
    hf_i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = I2C_NUM_0;
    cfg.sda_io_num = 21;
    cfg.scl_io_num = 22;
    cfg.glitch_ignore_cnt = filter;
    cfg.enable_internal_pullup = true;

    EspI2cBus bus(cfg);
    if (!bus.Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize bus with glitch filter %d", static_cast<int>(filter));
      return false;
    }
    ESP_LOGI(TAG, "Bus initialized with glitch filter %d", static_cast<int>(filter));
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C bus configuration validation completed");
  return true;
}

bool test_i2c_device_operations() noexcept {
  ESP_LOGI(TAG, "Testing I2C device operations...");

  // Create I2C bus first
  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);

  if (!test_i2c_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus for device test");
    return false;
  }

  // Create I2C device on the bus
  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48; // Common sensor address
  device_cfg.scl_speed_hz = 100000; // Standard mode
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);

  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create I2C device");
    return false;
  }

  // Test device retrieval
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to retrieve I2C device");
    return false;
  }

  // Test ESP-specific device retrieval
  EspI2cDevice* esp_device = test_i2c_bus.GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to retrieve ESP I2C device");
    return false;
  }

  // Test device configuration retrieval
  const auto& dev_config = esp_device->GetConfig();
  if (dev_config.device_address != 0x48 || dev_config.scl_speed_hz != 100000) {
    ESP_LOGE(TAG, "Device configuration mismatch");
    return false;
  }

  // Test device address retrieval
  if (esp_device->GetDeviceAddress() != 0x48) {
    ESP_LOGE(TAG, "Device address mismatch");
    return false;
  }

  // Test device count
  if (test_i2c_bus.GetDeviceCount() != 1) {
    ESP_LOGE(TAG, "Device count mismatch");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C device created with index: %d", device_index);
  return true;
}

bool test_i2c_multiple_devices_on_bus() noexcept {
  ESP_LOGI(TAG, "Testing multiple I2C devices on bus...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  // Create multiple devices with different configurations
  const uint16_t device_addresses[] = {0x48, 0x49, 0x4A, 0x4B};
  const uint32_t clock_speeds[] = {100000, 400000, 1000000, 400000}; // Standard, Fast, Fast+, Fast
  const hf_i2c_address_bits_t addr_lengths[] = {
    hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT,
    hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT,
    hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT,
    hf_i2c_address_bits_t::HF_I2C_ADDR_10_BIT
  };

  int device_indices[4];
  for (int i = 0; i < 4; i++) {
    hf_i2c_device_config_t cfg = {};
    cfg.device_address = device_addresses[i];
    cfg.scl_speed_hz = clock_speeds[i];
    cfg.dev_addr_length = addr_lengths[i];

    device_indices[i] = test_i2c_bus.CreateDevice(cfg);
    if (device_indices[i] < 0) {
      ESP_LOGE(TAG, "Failed to create device %d", i);
      return false;
    }
    ESP_LOGI(TAG, "Created device %d with address 0x%02X, clock %lu Hz, addr_bits %s", 
             i, device_addresses[i], clock_speeds[i],
             (addr_lengths[i] == hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT) ? "7-bit" : "10-bit");
  }

  // Verify device count
  if (test_i2c_bus.GetDeviceCount() != 4) {
    ESP_LOGE(TAG, "Expected 4 devices, got %zu", test_i2c_bus.GetDeviceCount());
    return false;
  }

  // Test retrieving all devices
  for (int i = 0; i < 4; i++) {
    BaseI2c* device = test_i2c_bus.GetDevice(device_indices[i]);
    if (!device) {
      ESP_LOGE(TAG, "Failed to retrieve device %d", i);
      return false;
    }
  }

  // Test retrieving device by address
  BaseI2c* device_by_addr = test_i2c_bus.GetDeviceByAddress(0x48);
  if (!device_by_addr) {
    ESP_LOGE(TAG, "Failed to retrieve device by address");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple I2C devices created and verified");
  return true;
}

bool test_i2c_device_configuration_variations() noexcept {
  ESP_LOGI(TAG, "Testing I2C device configuration variations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  // Test device with advanced configuration
  hf_i2c_device_config_t advanced_cfg = {};
  advanced_cfg.device_address = 0x68; // IMU/RTC address
  advanced_cfg.scl_speed_hz = 400000; // Fast mode
  advanced_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  advanced_cfg.scl_wait_us = 10; // Custom wait time
  advanced_cfg.disable_ack_check = false;

  int device_index = test_i2c_bus.CreateDevice(advanced_cfg);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create advanced I2C device");
    return false;
  }

  EspI2cDevice* esp_device = test_i2c_bus.GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to retrieve ESP device");
    return false;
  }

  // Test actual clock frequency retrieval
  uint32_t actual_freq = 0;
  hf_i2c_err_t freq_result = esp_device->GetActualClockFrequency(actual_freq);
  if (freq_result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGI(TAG, "Requested: %lu Hz, Actual: %lu Hz", advanced_cfg.scl_speed_hz, actual_freq);
  } else {
    ESP_LOGW(TAG, "Could not retrieve actual clock frequency");
  }

  ESP_LOGI(TAG, "[SUCCESS] Advanced I2C device configuration tested");
  return true;
}

bool test_i2c_write_operations() noexcept {
  ESP_LOGI(TAG, "Testing I2C write operations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test single byte write
  uint8_t single_byte = 0x12;
  hf_i2c_err_t result = device->Write(&single_byte, 1);
  // Note: This will likely fail if no actual device is connected, but we test the API
  ESP_LOGI(TAG, "Single byte write result: %d", static_cast<int>(result));

  // Test multi-byte write
  uint8_t multi_bytes[] = {0x01, 0x02, 0x03, 0x04, 0x05};
  result = device->Write(multi_bytes, sizeof(multi_bytes));
  ESP_LOGI(TAG, "Multi-byte write result: %d", static_cast<int>(result));

  // Test register write pattern (register address + data)
  uint8_t reg_write[] = {0x10, 0xAB}; // Write 0xAB to register 0x10
  result = device->Write(reg_write, sizeof(reg_write));
  ESP_LOGI(TAG, "Register write result: %d", static_cast<int>(result));

  // Test different data sizes
  const size_t test_sizes[] = {1, 2, 4, 8, 16, 32, 64};
  for (size_t size : test_sizes) {
    if (size > HF_I2C_MAX_TRANSFER_BYTES) continue;

    std::vector<uint8_t> test_data(size);
    for (size_t i = 0; i < size; i++) {
      test_data[i] = static_cast<uint8_t>(i & 0xFF);
    }

    result = device->Write(test_data.data(), size);
    ESP_LOGI(TAG, "Write test for %zu bytes: %d", size, static_cast<int>(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C write operations tested");
  return true;
}

bool test_i2c_read_operations() noexcept {
  ESP_LOGI(TAG, "Testing I2C read operations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test single byte read
  uint8_t single_byte = 0;
  hf_i2c_err_t result = device->Read(&single_byte, 1);
  ESP_LOGI(TAG, "Single byte read result: %d, data: 0x%02X", static_cast<int>(result), single_byte);

  // Test multi-byte read
  uint8_t multi_bytes[8] = {0};
  result = device->Read(multi_bytes, sizeof(multi_bytes));
  ESP_LOGI(TAG, "Multi-byte read result: %d", static_cast<int>(result));
  
  // Log the read data for debugging
  ESP_LOG_BUFFER_HEX_LEVEL(TAG, multi_bytes, sizeof(multi_bytes), ESP_LOG_INFO);

  // Test different data sizes
  const size_t test_sizes[] = {1, 2, 4, 8, 16, 32};
  for (size_t size : test_sizes) {
    if (size > HF_I2C_MAX_TRANSFER_BYTES) continue;

    std::vector<uint8_t> read_data(size, 0);
    result = device->Read(read_data.data(), size);
    ESP_LOGI(TAG, "Read test for %zu bytes: %d", size, static_cast<int>(result));
  }

  // Test read with timeout
  uint8_t timeout_data[4] = {0};
  result = device->Read(timeout_data, sizeof(timeout_data), 1000); // 1 second timeout
  ESP_LOGI(TAG, "Read with timeout result: %d", static_cast<int>(result));

  ESP_LOGI(TAG, "[SUCCESS] I2C read operations tested");
  return true;
}

bool test_i2c_write_read_operations() noexcept {
  ESP_LOGI(TAG, "Testing I2C write-read operations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test register read pattern (write register address, then read data)
  uint8_t reg_addr = 0x00; // Read from register 0x00
  uint8_t reg_data[4] = {0};
  hf_i2c_err_t result = device->WriteRead(&reg_addr, 1, reg_data, sizeof(reg_data));
  ESP_LOGI(TAG, "Register read result: %d", static_cast<int>(result));
  ESP_LOG_BUFFER_HEX_LEVEL(TAG, reg_data, sizeof(reg_data), ESP_LOG_INFO);

  // Test multi-byte register address read
  uint8_t reg_addr_16[] = {0x00, 0x01}; // 16-bit register address
  uint8_t reg_data_16[8] = {0};
  result = device->WriteRead(reg_addr_16, sizeof(reg_addr_16), reg_data_16, sizeof(reg_data_16));
  ESP_LOGI(TAG, "16-bit register read result: %d", static_cast<int>(result));

  // Test different write/read size combinations
  const size_t write_sizes[] = {1, 2, 4};
  const size_t read_sizes[] = {1, 2, 4, 8, 16};

  for (size_t write_size : write_sizes) {
    for (size_t read_size : read_sizes) {
      if (write_size > 4 || read_size > HF_I2C_MAX_TRANSFER_BYTES) continue;

      std::vector<uint8_t> write_data(write_size);
      std::vector<uint8_t> read_data(read_size, 0);

      // Fill write data with test pattern
      for (size_t i = 0; i < write_size; i++) {
        write_data[i] = static_cast<uint8_t>(i);
      }

      result = device->WriteRead(write_data.data(), write_size, read_data.data(), read_size);
      ESP_LOGI(TAG, "WriteRead test (write: %zu, read: %zu): %d", 
               write_size, read_size, static_cast<int>(result));
    }
  }

  // Test write-read with timeout
  uint8_t cmd = 0xFF;
  uint8_t response[2] = {0};
  result = device->WriteRead(&cmd, 1, response, sizeof(response), 1000);
  ESP_LOGI(TAG, "WriteRead with timeout result: %d", static_cast<int>(result));

  ESP_LOGI(TAG, "[SUCCESS] I2C write-read operations tested");
  return true;
}

bool test_i2c_device_scanning() noexcept {
  ESP_LOGI(TAG, "Testing I2C device scanning...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Scan for devices on the bus
  std::vector<uint16_t> found_devices;
  size_t device_count = test_i2c_bus.ScanDevices(found_devices);

  ESP_LOGI(TAG, "I2C bus scan found %zu devices:", device_count);
  for (uint16_t addr : found_devices) {
    ESP_LOGI(TAG, "  Device at address: 0x%02X", addr);
  }

  // Test scan with custom address range
  found_devices.clear();
  device_count = test_i2c_bus.ScanDevices(found_devices, 0x10, 0x50);
  ESP_LOGI(TAG, "Custom range scan (0x10-0x50) found %zu devices", device_count);

  // Test scan of specific address ranges
  const struct {
    uint16_t start;
    uint16_t end;
    const char* description;
  } scan_ranges[] = {
    {0x08, 0x0F, "Reserved range"},
    {0x10, 0x2F, "Standard sensors"},
    {0x30, 0x4F, "Displays and EEPROMs"},
    {0x50, 0x77, "Various peripherals"}
  };

  for (const auto& range : scan_ranges) {
    found_devices.clear();
    device_count = test_i2c_bus.ScanDevices(found_devices, range.start, range.end);
    ESP_LOGI(TAG, "%s (0x%02X-0x%02X): %zu devices", 
             range.description, range.start, range.end, device_count);
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C device scanning completed");
  return true;
}

bool test_i2c_device_probing() noexcept {
  ESP_LOGI(TAG, "Testing I2C device probing...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Test probing common device addresses
  const uint16_t common_addresses[] = {
    0x48, 0x49, 0x4A, 0x4B, // Temperature sensors
    0x68, 0x69,             // IMU/RTC
    0x3C, 0x3D,             // OLED displays
    0x50, 0x51, 0x52, 0x53, // EEPROMs
    0x76, 0x77              // Pressure sensors
  };

  int found_count = 0;
  for (uint16_t addr : common_addresses) {
    bool present = test_i2c_bus.ProbeDevice(addr);
    if (present) {
      ESP_LOGI(TAG, "Device found at address 0x%02X", addr);
      found_count++;
    }
  }

  ESP_LOGI(TAG, "Device probing found %d devices from common addresses", found_count);

  // Test probing with created devices
  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  if (device_index >= 0) {
    EspI2cDevice* esp_device = test_i2c_bus.GetEspDevice(device_index);
    if (esp_device) {
      bool device_responds = esp_device->ProbeDevice();
      ESP_LOGI(TAG, "Created device at 0x48 probe result: %s", device_responds ? "present" : "not present");
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C device probing completed");
  return true;
}

bool test_i2c_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing I2C error handling...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test invalid parameters
  uint8_t valid_data[] = {0x01, 0x02};
  
  // Test NULL pointer write
  hf_i2c_err_t result = device->Write(nullptr, 2);
  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "NULL pointer write should have failed");
    return false;
  }
  ESP_LOGI(TAG, "NULL pointer write correctly rejected");

  // Test NULL pointer read
  result = device->Read(nullptr, 2);
  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "NULL pointer read should have failed");
    return false;
  }
  ESP_LOGI(TAG, "NULL pointer read correctly rejected");

  // Test zero length operations
  result = device->Write(valid_data, 0);
  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGE(TAG, "Zero length write should have failed");
    return false;
  }
  ESP_LOGI(TAG, "Zero length write correctly rejected");

  // Test oversized transfer
  if (HF_I2C_MAX_TRANSFER_BYTES < 2048) {
    result = device->Write(valid_data, HF_I2C_MAX_TRANSFER_BYTES + 1);
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGE(TAG, "Oversized write should have failed");
      return false;
    }
    ESP_LOGI(TAG, "Oversized write correctly rejected");
  }

  // Test invalid device index
  BaseI2c* invalid_device = test_i2c_bus.GetDevice(999);
  if (invalid_device != nullptr) {
    ESP_LOGE(TAG, "Invalid device index should return nullptr");
    return false;
  }
  ESP_LOGI(TAG, "Invalid device index correctly handled");

  // Test duplicate device address
  int duplicate_index = test_i2c_bus.CreateDevice(device_cfg);
  if (duplicate_index >= 0) {
    ESP_LOGW(TAG, "Duplicate device creation allowed (implementation dependent)");
  } else {
    ESP_LOGI(TAG, "Duplicate device creation correctly rejected");
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C error handling tested");
  return true;
}

bool test_i2c_statistics_tracking() noexcept {
  ESP_LOGI(TAG, "Testing I2C statistics tracking...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Reset statistics first
  hf_i2c_err_t result = device->ResetStatistics();
  ESP_LOGI(TAG, "Statistics reset result: %d", static_cast<int>(result));

  // Get initial statistics
  hf_i2c_statistics_t initial_stats = {};
  result = device->GetStatistics(initial_stats);
  ESP_LOGI(TAG, "Initial statistics retrieval: %d", static_cast<int>(result));

  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGI(TAG, "Initial stats - Successful ops: %lu, Failed ops: %lu, Bytes transferred: %lu",
             initial_stats.successful_operations, initial_stats.failed_operations, 
             initial_stats.total_bytes_transferred);
  }

  // Perform some operations to update statistics
  uint8_t test_data[] = {0x10, 0x20, 0x30};
  uint8_t read_data[3] = {0};

  for (int i = 0; i < 5; i++) {
    device->Write(test_data, sizeof(test_data));
    device->Read(read_data, sizeof(read_data));
    device->WriteRead(test_data, 1, read_data, 2);
  }

  // Get updated statistics
  hf_i2c_statistics_t updated_stats = {};
  result = device->GetStatistics(updated_stats);
  ESP_LOGI(TAG, "Updated statistics retrieval: %d", static_cast<int>(result));

  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGI(TAG, "Updated stats - Successful ops: %lu, Failed ops: %lu, Bytes transferred: %lu",
             updated_stats.successful_operations, updated_stats.failed_operations, 
             updated_stats.total_bytes_transferred);
    ESP_LOGI(TAG, "Average operation time: %lu μs, Last operation time: %lu μs",
             updated_stats.average_operation_time_us, updated_stats.last_operation_time_us);
  }

  // Get diagnostics
  hf_i2c_diagnostics_t diagnostics = {};
  result = device->GetDiagnostics(diagnostics);
  ESP_LOGI(TAG, "Diagnostics retrieval: %d", static_cast<int>(result));

  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGI(TAG, "Bus errors: %lu, Timeouts: %lu, NACK count: %lu",
             diagnostics.bus_error_count, diagnostics.timeout_count, diagnostics.nack_count);
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C statistics tracking tested");
  return true;
}

bool test_i2c_clock_frequency_testing() noexcept {
  ESP_LOGI(TAG, "Testing I2C clock frequency variations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Test different clock frequencies
  const uint32_t clock_frequencies[] = {50000, 100000, 400000, 1000000}; // 50kHz, 100kHz, 400kHz, 1MHz
  const char* frequency_names[] = {"Low Speed", "Standard", "Fast", "Fast Plus"};

  for (size_t i = 0; i < sizeof(clock_frequencies) / sizeof(clock_frequencies[0]); i++) {
    uint32_t clock_freq = clock_frequencies[i];
    const char* freq_name = frequency_names[i];

    hf_i2c_device_config_t device_cfg = {};
    device_cfg.device_address = 0x48;
    device_cfg.scl_speed_hz = clock_freq;
    device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

    int device_index = test_i2c_bus.CreateDevice(device_cfg);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device with %s clock (%lu Hz)", freq_name, clock_freq);
      continue;
    }

    EspI2cDevice* esp_device = test_i2c_bus.GetEspDevice(device_index);
    if (esp_device) {
      uint32_t actual_freq = 0;
      hf_i2c_err_t result = esp_device->GetActualClockFrequency(actual_freq);
      if (result == hf_i2c_err_t::I2C_SUCCESS) {
        ESP_LOGI(TAG, "%s mode - Requested: %lu Hz, Actual: %lu Hz (%.2f%% accuracy)", 
                 freq_name, clock_freq, actual_freq, 
                 (static_cast<double>(actual_freq) / clock_freq) * 100.0);
      }

      // Test a simple operation at this frequency
      uint8_t test_data[] = {0x00};
      uint8_t read_data[1] = {0};
      result = esp_device->WriteRead(test_data, 1, read_data, 1);
      ESP_LOGI(TAG, "%s mode operation result: %d", freq_name, static_cast<int>(result));
    }

    // Remove device for next test
    test_i2c_bus.RemoveDevice(device_index);
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C clock frequency testing completed");
  return true;
}

bool test_i2c_device_removal() noexcept {
  ESP_LOGI(TAG, "Testing I2C device removal...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Create multiple devices
  std::vector<int> device_indices;
  const uint16_t addresses[] = {0x48, 0x49, 0x4A};

  for (size_t i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++) {
    hf_i2c_device_config_t device_cfg = {};
    device_cfg.device_address = addresses[i];
    device_cfg.scl_speed_hz = 100000;
    device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

    int device_index = test_i2c_bus.CreateDevice(device_cfg);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Failed to create device %zu", i);
      return false;
    }
    device_indices.push_back(device_index);
  }

  // Verify device count
  if (test_i2c_bus.GetDeviceCount() != 3) {
    ESP_LOGE(TAG, "Expected 3 devices, got %zu", test_i2c_bus.GetDeviceCount());
    return false;
  }

  // Remove device by index
  bool removed = test_i2c_bus.RemoveDevice(device_indices[1]);
  if (!removed) {
    ESP_LOGE(TAG, "Failed to remove device by index");
    return false;
  }

  // Verify device count decreased
  if (test_i2c_bus.GetDeviceCount() != 2) {
    ESP_LOGE(TAG, "Expected 2 devices after removal, got %zu", test_i2c_bus.GetDeviceCount());
    return false;
  }

  // Remove device by address
  removed = test_i2c_bus.RemoveDeviceByAddress(0x48);
  if (!removed) {
    ESP_LOGE(TAG, "Failed to remove device by address");
    return false;
  }

  // Verify device count decreased again
  if (test_i2c_bus.GetDeviceCount() != 1) {
    ESP_LOGE(TAG, "Expected 1 device after second removal, got %zu", test_i2c_bus.GetDeviceCount());
    return false;
  }

  // Test removal of invalid index/address
  removed = test_i2c_bus.RemoveDevice(999);
  if (removed) {
    ESP_LOGE(TAG, "Removal of invalid device should have failed");
    return false;
  }

  removed = test_i2c_bus.RemoveDeviceByAddress(0xFF);
  if (removed) {
    ESP_LOGE(TAG, "Removal of non-existent address should have failed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C device removal tested");
  return true;
}

bool test_i2c_bus_reset() noexcept {
  ESP_LOGI(TAG, "Testing I2C bus reset...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Create a device first
  hf_i2c_device_config_t device_cfg = {};
  device_cfg.device_address = 0x48;
  device_cfg.scl_speed_hz = 100000;
  device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device_index = test_i2c_bus.CreateDevice(device_cfg);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create device for reset test");
    return false;
  }

  // Perform some operations that might leave the bus in a bad state
  BaseI2c* device = test_i2c_bus.GetDevice(device_index);
  if (device) {
    uint8_t dummy_data[] = {0xFF, 0xFF, 0xFF};
    device->Write(dummy_data, sizeof(dummy_data)); // This might fail and leave bus in bad state
  }

  // Reset the bus
  bool reset_result = test_i2c_bus.ResetBus();
  ESP_LOGI(TAG, "Bus reset result: %s", reset_result ? "success" : "failed");

  // Try to use the bus after reset
  if (device) {
    uint8_t test_data = 0x00;
    uint8_t read_data = 0;
    hf_i2c_err_t result = device->WriteRead(&test_data, 1, &read_data, 1);
    ESP_LOGI(TAG, "Operation after reset: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C bus reset tested");
  return true;
}

bool test_i2c_concurrent_operations() noexcept {
  ESP_LOGI(TAG, "Testing I2C concurrent operations...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Create two devices
  hf_i2c_device_config_t device1_cfg = {};
  device1_cfg.device_address = 0x48;
  device1_cfg.scl_speed_hz = 100000;
  device1_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  hf_i2c_device_config_t device2_cfg = {};
  device2_cfg.device_address = 0x49;
  device2_cfg.scl_speed_hz = 400000;
  device2_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

  int device1_index = test_i2c_bus.CreateDevice(device1_cfg);
  int device2_index = test_i2c_bus.CreateDevice(device2_cfg);

  if (device1_index < 0 || device2_index < 0) {
    ESP_LOGE(TAG, "Failed to create devices for concurrent test");
    return false;
  }

  BaseI2c* device1 = test_i2c_bus.GetDevice(device1_index);
  BaseI2c* device2 = test_i2c_bus.GetDevice(device2_index);

  if (!device1 || !device2) {
    ESP_LOGE(TAG, "Failed to retrieve devices");
    return false;
  }

  // Perform interleaved operations
  uint8_t data1[] = {0x10, 0x11};
  uint8_t data2[] = {0x20, 0x21};
  uint8_t read1[2] = {0};
  uint8_t read2[2] = {0};

  for (int i = 0; i < 10; i++) {
    hf_i2c_err_t result1 = device1->WriteRead(data1, 1, read1, 1);
    hf_i2c_err_t result2 = device2->WriteRead(data2, 1, read2, 1);

    ESP_LOGI(TAG, "Iteration %d - Device1: %d, Device2: %d", 
             i, static_cast<int>(result1), static_cast<int>(result2));

    // Small delay between iterations
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C concurrent operations tested");
  return true;
}

bool test_i2c_performance() noexcept {
  ESP_LOGI(TAG, "Testing I2C performance...");

  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;

  EspI2cBus test_i2c_bus(i2c_cfg);
  if (!test_i2c_bus.Initialize()) {
    return false;
  }

  // Test performance at different clock speeds
  const uint32_t clock_speeds[] = {100000, 400000, 1000000}; // Standard, Fast, Fast+
  const char* speed_names[] = {"Standard", "Fast", "Fast Plus"};

  for (size_t speed_idx = 0; speed_idx < sizeof(clock_speeds) / sizeof(clock_speeds[0]); speed_idx++) {
    hf_i2c_device_config_t device_cfg = {};
    device_cfg.device_address = 0x48;
    device_cfg.scl_speed_hz = clock_speeds[speed_idx];
    device_cfg.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;

    int device_index = test_i2c_bus.CreateDevice(device_cfg);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device for %s mode", speed_names[speed_idx]);
      continue;
    }

    BaseI2c* device = test_i2c_bus.GetDevice(device_index);
    if (!device) {
      test_i2c_bus.RemoveDevice(device_index);
      continue;
    }

    ESP_LOGI(TAG, "Performance test for %s mode (%lu Hz):", speed_names[speed_idx], clock_speeds[speed_idx]);

    // Test different operation types and sizes
    const size_t test_sizes[] = {1, 4, 16, 32};
    const int iterations = 50;

    for (size_t size : test_sizes) {
      if (size > HF_I2C_MAX_TRANSFER_BYTES) continue;

      std::vector<uint8_t> test_data(size, 0xAA);
      std::vector<uint8_t> read_data(size);

      // Test write performance
      uint64_t start_time = esp_timer_get_time();
      for (int i = 0; i < iterations; i++) {
        device->Write(test_data.data(), size);
      }
      uint64_t write_time = esp_timer_get_time() - start_time;

      // Test read performance
      start_time = esp_timer_get_time();
      for (int i = 0; i < iterations; i++) {
        device->Read(read_data.data(), size);
      }
      uint64_t read_time = esp_timer_get_time() - start_time;

      // Test write-read performance
      start_time = esp_timer_get_time();
      for (int i = 0; i < iterations; i++) {
        device->WriteRead(test_data.data(), 1, read_data.data(), size - 1);
      }
      uint64_t writeread_time = esp_timer_get_time() - start_time;

      double write_avg = static_cast<double>(write_time) / iterations / 1000.0; // ms
      double read_avg = static_cast<double>(read_time) / iterations / 1000.0;   // ms
      double writeread_avg = static_cast<double>(writeread_time) / iterations / 1000.0; // ms

      ESP_LOGI(TAG, "  Size %zu bytes - Write: %.2f ms, Read: %.2f ms, WriteRead: %.2f ms", 
               size, write_avg, read_avg, writeread_avg);
    }

    test_i2c_bus.RemoveDevice(device_index);
  }

  ESP_LOGI(TAG, "[SUCCESS] I2C performance testing completed");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Bus and initialization tests
  RUN_TEST(test_i2c_bus_initialization);
  RUN_TEST(test_i2c_multiple_bus_initialization);
  RUN_TEST(test_i2c_bus_configuration_validation);

  // Device management tests
  RUN_TEST(test_i2c_device_operations);
  RUN_TEST(test_i2c_multiple_devices_on_bus);
  RUN_TEST(test_i2c_device_configuration_variations);

  // Communication tests
  RUN_TEST(test_i2c_write_operations);
  RUN_TEST(test_i2c_read_operations);
  RUN_TEST(test_i2c_write_read_operations);

  // Discovery and scanning tests
  RUN_TEST(test_i2c_device_scanning);
  RUN_TEST(test_i2c_device_probing);

  // Error handling and robustness tests
  RUN_TEST(test_i2c_error_handling);
  RUN_TEST(test_i2c_device_removal);
  RUN_TEST(test_i2c_bus_reset);
  RUN_TEST(test_i2c_concurrent_operations);

  // Monitoring and performance tests
  RUN_TEST(test_i2c_statistics_tracking);
  RUN_TEST(test_i2c_clock_frequency_testing);
  RUN_TEST(test_i2c_performance);

  print_test_summary(g_test_results, "I2C", TAG);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
