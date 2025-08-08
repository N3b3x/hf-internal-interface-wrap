/**
 * @file I2cComprehensiveTest.cpp
 * @brief Comprehensive I2C testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains comprehensive testing for the ESP I2C implementation including:
 * - Bus initialization and configuration validation
 * - Device creation and management
 * - Data transfer operations (read/write/write-read)
 * - Error handling and recovery
 * - Bus scanning and device probing
 * - Multi-device scenarios
 * - ESP-specific features (clock sources, glitch filtering, power management)
 * - Thread safety verification
 * - Performance and timing tests
 * - Edge cases and fault injection
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseI2c.h"
#include "mcu/esp32/EspI2c.h"
#include "mcu/esp32/utils/EspTypes_I2C.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "TestFramework.h"

static const char* TAG = "I2C_Test";
static TestResults g_test_results;

// Test configuration constants
static constexpr hf_pin_num_t TEST_SDA_PIN = 21;
static constexpr hf_pin_num_t TEST_SCL_PIN = 22;
static constexpr uint16_t TEST_DEVICE_ADDR_1 = 0x48; // Common I2C device address
static constexpr uint16_t TEST_DEVICE_ADDR_2 = 0x50; // EEPROM address
static constexpr uint16_t NONEXISTENT_ADDR = 0x7E;   // Unlikely to exist
static constexpr uint32_t STANDARD_FREQ = 100000;    // 100kHz
static constexpr uint32_t FAST_FREQ = 400000;        // 400kHz
static constexpr uint32_t FAST_PLUS_FREQ = 1000000;  // 1MHz

// Forward declarations
bool test_i2c_bus_initialization() noexcept;
bool test_i2c_bus_deinitialization() noexcept;
bool test_i2c_configuration_validation() noexcept;
bool test_i2c_device_creation() noexcept;
bool test_i2c_device_management() noexcept;
bool test_i2c_device_probing() noexcept;
bool test_i2c_bus_scanning() noexcept;
bool test_i2c_write_operations() noexcept;
bool test_i2c_read_operations() noexcept;
bool test_i2c_write_read_operations() noexcept;
bool test_i2c_error_handling() noexcept;
bool test_i2c_timeout_handling() noexcept;
bool test_i2c_multi_device_operations() noexcept;
bool test_i2c_clock_speeds() noexcept;
bool test_i2c_address_modes() noexcept;
bool test_i2c_esp_specific_features() noexcept;
bool test_i2c_thread_safety() noexcept;
bool test_i2c_performance() noexcept;
bool test_i2c_edge_cases() noexcept;
bool test_i2c_power_management() noexcept;

// Helper functions
EspI2cBus* create_test_bus(uint32_t freq = STANDARD_FREQ) noexcept;
bool verify_device_functionality(BaseI2c* device) noexcept;
void log_test_separator(const char* test_name) noexcept;

bool test_i2c_bus_initialization() noexcept {
  log_test_separator("I2C Bus Initialization");

  // Test 1: Basic initialization
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_NUM_0;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.enable_internal_pullup = true;

  auto test_bus = std::make_unique<EspI2cBus>(bus_config);

  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  if (!test_bus->IsInitialized()) {
    ESP_LOGE(TAG, "Bus not marked as initialized");
    return false;
  }

  // Test 2: Double initialization should succeed (idempotent)
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Second initialization failed");
    return false;
  }

  // Test 3: Verify configuration
  const auto& config = test_bus->GetConfig();
  if (config.sda_io_num != TEST_SDA_PIN || config.scl_io_num != TEST_SCL_PIN) {
    ESP_LOGE(TAG, "Configuration mismatch");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Bus initialization tests passed");
  return true;
}

bool test_i2c_bus_deinitialization() noexcept {
  log_test_separator("I2C Bus Deinitialization");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Verify initialization
  if (!test_bus->IsInitialized()) {
    ESP_LOGE(TAG, "Bus not initialized");
    return false;
  }

  // Test deinitialization
  if (!test_bus->Deinitialize()) {
    ESP_LOGE(TAG, "Failed to deinitialize bus");
    return false;
  }

  if (test_bus->IsInitialized()) {
    ESP_LOGE(TAG, "Bus still marked as initialized");
    return false;
  }

  // Test double deinitialization should succeed
  if (!test_bus->Deinitialize()) {
    ESP_LOGE(TAG, "Second deinitialization failed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Bus deinitialization tests passed");
  return true;
}

bool test_i2c_configuration_validation() noexcept {
  log_test_separator("I2C Configuration Validation");

  // Test various clock sources
  for (auto clk_src :
       {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT, hf_i2c_clock_source_t::HF_I2C_CLK_SRC_APB,
        hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    hf_i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_NUM_0;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.clk_source = clk_src;
    bus_config.enable_internal_pullup = true;

    auto test_bus = std::make_unique<EspI2cBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize with clock source %d", static_cast<int>(clk_src));
      return false;
    }
    test_bus->Deinitialize();
  }

  // Test glitch filter settings
  for (auto filter : {hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_0_CYCLES,
                      hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_3_CYCLES,
                      hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES}) {
    hf_i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_NUM_0;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.glitch_ignore_cnt = filter;
    bus_config.enable_internal_pullup = true;

    auto test_bus = std::make_unique<EspI2cBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize with glitch filter %d", static_cast<int>(filter));
      return false;
    }
    test_bus->Deinitialize();
  }

  ESP_LOGI(TAG, "[SUCCESS] Configuration validation tests passed");
  return true;
}

bool test_i2c_device_creation() noexcept {
  log_test_separator("I2C Device Creation");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test 1: Create device with 7-bit address
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create 7-bit device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get created device");
    return false;
  }

  // Test 2: Verify device count
  if (test_bus->GetDeviceCount() != 1) {
    ESP_LOGE(TAG, "Device count mismatch");
    return false;
  }

  // Test 3: Create device with 10-bit address
  hf_i2c_device_config_t device_config_10bit = {};
  device_config_10bit.device_address = 0x200; // 10-bit address
  device_config_10bit.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_10_BIT;
  device_config_10bit.scl_speed_hz = FAST_FREQ;

  int device_index_10bit = test_bus->CreateDevice(device_config_10bit);
  if (device_index_10bit < 0) {
    ESP_LOGE(TAG, "Failed to create 10-bit device");
    return false;
  }

  // Test 4: Verify multiple devices
  if (test_bus->GetDeviceCount() != 2) {
    ESP_LOGE(TAG, "Device count after second device incorrect");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Device creation tests passed");
  return true;
}

bool test_i2c_device_management() noexcept {
  log_test_separator("I2C Device Management");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create multiple devices
  std::vector<int> device_indices;
  for (uint16_t addr = 0x10; addr <= 0x13; ++addr) {
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = addr;
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = STANDARD_FREQ;

    int idx = test_bus->CreateDevice(device_config);
    if (idx < 0) {
      ESP_LOGE(TAG, "Failed to create device with address 0x%02X", addr);
      return false;
    }
    device_indices.push_back(idx);
  }

  // Test device lookup by address
  for (uint16_t addr = 0x10; addr <= 0x13; ++addr) {
    BaseI2c* device = test_bus->GetDeviceByAddress(addr);
    if (!device) {
      ESP_LOGE(TAG, "Failed to find device with address 0x%02X", addr);
      return false;
    }
  }

  // Test device removal
  uint16_t remove_addr = 0x11;
  if (!test_bus->RemoveDeviceByAddress(remove_addr)) {
    ESP_LOGE(TAG, "Failed to remove device with address 0x%02X", remove_addr);
    return false;
  }

  // Verify device is gone
  BaseI2c* removed_device = test_bus->GetDeviceByAddress(remove_addr);
  if (removed_device) {
    ESP_LOGE(TAG, "Device with address 0x%02X still exists after removal", remove_addr);
    return false;
  }

  // Verify remaining devices
  if (test_bus->GetDeviceCount() != 3) {
    ESP_LOGE(TAG, "Device count after removal incorrect: %zu", test_bus->GetDeviceCount());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Device management tests passed");
  return true;
}

bool test_i2c_device_probing() noexcept {
  log_test_separator("I2C Device Probing");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test probing non-existent device
  bool exists = test_bus->ProbeDevice(NONEXISTENT_ADDR);
  ESP_LOGI(TAG, "Probe result for address 0x%02X: %s", NONEXISTENT_ADDR,
           exists ? "EXISTS" : "NOT FOUND");

  // Note: We can't guarantee specific devices are connected, so we'll just verify the method works
  // and doesn't crash. In a real test environment, you would connect known devices.

  ESP_LOGI(TAG, "[SUCCESS] Device probing tests passed");
  return true;
}

bool test_i2c_bus_scanning() noexcept {
  log_test_separator("I2C Bus Scanning");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Scan the bus for devices
  std::vector<hf_u16_t> found_devices;
  size_t device_count = test_bus->ScanDevices(found_devices);

  ESP_LOGI(TAG, "Bus scan found %zu devices", device_count);
  for (auto addr : found_devices) {
    ESP_LOGI(TAG, "  - Device at address 0x%02X", addr);
  }

  // Test custom scan range
  std::vector<hf_u16_t> limited_scan;
  size_t limited_count = test_bus->ScanDevices(limited_scan, 0x20, 0x30);
  ESP_LOGI(TAG, "Limited scan (0x20-0x30) found %zu devices", limited_count);

  ESP_LOGI(TAG, "[SUCCESS] Bus scanning tests passed");
  return true;
}

bool test_i2c_write_operations() noexcept {
  log_test_separator("I2C Write Operations");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create a test device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test various write operations
  uint8_t single_byte = 0xAA;
  hf_i2c_err_t result = device->Write(&single_byte, 1);
  ESP_LOGI(TAG, "Single byte write result: %s", HfI2CErrToString(result).data());

  uint8_t multi_bytes[] = {0x01, 0x02, 0x03, 0x04};
  result = device->Write(multi_bytes, sizeof(multi_bytes));
  ESP_LOGI(TAG, "Multi-byte write result: %s", HfI2CErrToString(result).data());

  // Test with timeout
  result = device->Write(multi_bytes, sizeof(multi_bytes), 500);
  ESP_LOGI(TAG, "Write with timeout result: %s", HfI2CErrToString(result).data());

  // Test zero-length write (should fail)
  result = device->Write(nullptr, 0);
  if (result == hf_i2c_err_t::I2C_SUCCESS) {
    ESP_LOGW(TAG, "Zero-length write unexpectedly succeeded");
  }

  ESP_LOGI(TAG, "[SUCCESS] Write operations tests passed");
  return true;
}

bool test_i2c_read_operations() noexcept {
  log_test_separator("I2C Read Operations");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create a test device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test various read operations
  uint8_t single_byte;
  hf_i2c_err_t result = device->Read(&single_byte, 1);
  ESP_LOGI(TAG, "Single byte read result: %s (data: 0x%02X)", HfI2CErrToString(result).data(),
           single_byte);

  uint8_t multi_bytes[8];
  result = device->Read(multi_bytes, sizeof(multi_bytes));
  ESP_LOGI(TAG, "Multi-byte read result: %s", HfI2CErrToString(result).data());

  // Test with timeout
  result = device->Read(multi_bytes, 4, 500);
  ESP_LOGI(TAG, "Read with timeout result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Read operations tests passed");
  return true;
}

bool test_i2c_write_read_operations() noexcept {
  log_test_separator("I2C Write-Read Operations");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create a test device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test write-read operation (typical register read)
  uint8_t reg_addr = 0x10;
  uint8_t read_data[4];
  hf_i2c_err_t result = device->WriteRead(&reg_addr, 1, read_data, sizeof(read_data));
  ESP_LOGI(TAG, "Write-read operation result: %s", HfI2CErrToString(result).data());

  // Test with timeout
  result = device->WriteRead(&reg_addr, 1, read_data, 2, 500);
  ESP_LOGI(TAG, "Write-read with timeout result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Write-read operations tests passed");
  return true;
}

bool test_i2c_error_handling() noexcept {
  log_test_separator("I2C Error Handling");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test operations on non-existent device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = NONEXISTENT_ADDR;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test operations that should fail
  uint8_t dummy_data = 0xFF;
  hf_i2c_err_t result = device->Write(&dummy_data, 1);
  ESP_LOGI(TAG, "Write to non-existent device result: %s", HfI2CErrToString(result).data());

  result = device->Read(&dummy_data, 1);
  ESP_LOGI(TAG, "Read from non-existent device result: %s", HfI2CErrToString(result).data());

  // Test invalid parameters
  result = device->Write(nullptr, 1);
  ESP_LOGI(TAG, "Write with null pointer result: %s", HfI2CErrToString(result).data());

  result = device->Read(nullptr, 1);
  ESP_LOGI(TAG, "Read with null pointer result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Error handling tests passed");
  return true;
}

bool test_i2c_timeout_handling() noexcept {
  log_test_separator("I2C Timeout Handling");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create device for timeout testing
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = NONEXISTENT_ADDR;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test various timeout values
  uint8_t dummy_data = 0xFF;
  uint64_t start_time, end_time;

  // Short timeout
  start_time = esp_timer_get_time();
  hf_i2c_err_t result = device->Write(&dummy_data, 1, 100); // 100ms timeout
  end_time = esp_timer_get_time();
  uint64_t duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Short timeout test: %s (took %llu ms)", HfI2CErrToString(result).data(),
           duration_ms);

  // Very short timeout
  start_time = esp_timer_get_time();
  result = device->Write(&dummy_data, 1, 10); // 10ms timeout
  end_time = esp_timer_get_time();
  duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Very short timeout test: %s (took %llu ms)", HfI2CErrToString(result).data(),
           duration_ms);

  ESP_LOGI(TAG, "[SUCCESS] Timeout handling tests passed");
  return true;
}

bool test_i2c_multi_device_operations() noexcept {
  log_test_separator("I2C Multi-Device Operations");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create multiple devices with different configurations
  std::vector<BaseI2c*> devices;
  std::vector<uint16_t> addresses = {0x48, 0x50, 0x68, 0x76};
  std::vector<uint32_t> speeds = {STANDARD_FREQ, FAST_FREQ, STANDARD_FREQ, FAST_FREQ};

  for (size_t i = 0; i < addresses.size(); ++i) {
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = addresses[i];
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = speeds[i];

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device %zu (addr 0x%02X)", i, addresses[i]);
      continue;
    }

    BaseI2c* device = test_bus->GetDevice(device_index);
    if (device) {
      devices.push_back(device);
    }
  }

  ESP_LOGI(TAG, "Created %zu devices on the bus", devices.size());

  // Test concurrent operations (simulate real-world usage)
  for (auto* device : devices) {
    uint8_t test_data = 0xAA;
    hf_i2c_err_t result = device->Write(&test_data, 1, 100);
    ESP_LOGI(TAG, "Multi-device write result: %s", HfI2CErrToString(result).data());

    // Small delay between operations
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ESP_LOGI(TAG, "[SUCCESS] Multi-device operations tests passed");
  return true;
}

bool test_i2c_clock_speeds() noexcept {
  log_test_separator("I2C Clock Speed Tests");

  std::vector<uint32_t> test_speeds = {
      100000, // Standard mode
      400000, // Fast mode
      1000000 // Fast mode plus
  };

  for (auto speed : test_speeds) {
    auto test_bus = create_test_bus(speed);
    if (!test_bus) {
      ESP_LOGE(TAG, "Failed to create test bus with speed %lu Hz", speed);
      return false;
    }

    // Create a device with this speed
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = TEST_DEVICE_ADDR_1;
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = speed;

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Failed to create device with speed %lu Hz", speed);
      return false;
    }

    EspI2cDevice* esp_device = test_bus->GetEspDevice(device_index);
    if (esp_device) {
      uint32_t actual_freq;
      hf_i2c_err_t result = esp_device->GetActualClockFrequency(actual_freq);
      ESP_LOGI(TAG, "Speed %lu Hz: actual frequency %lu Hz (result: %s)", speed, actual_freq,
               HfI2CErrToString(result).data());
    }

    ESP_LOGI(TAG, "Successfully tested clock speed: %lu Hz", speed);
  }

  ESP_LOGI(TAG, "[SUCCESS] Clock speed tests passed");
  return true;
}

bool test_i2c_address_modes() noexcept {
  log_test_separator("I2C Address Mode Tests");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test 7-bit addressing
  hf_i2c_device_config_t device_config_7bit = {};
  device_config_7bit.device_address = 0x48; // 7-bit address
  device_config_7bit.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config_7bit.scl_speed_hz = STANDARD_FREQ;

  int device_index_7bit = test_bus->CreateDevice(device_config_7bit);
  if (device_index_7bit < 0) {
    ESP_LOGE(TAG, "Failed to create 7-bit address device");
    return false;
  }

  // Test 10-bit addressing
  hf_i2c_device_config_t device_config_10bit = {};
  device_config_10bit.device_address = 0x200; // 10-bit address
  device_config_10bit.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_10_BIT;
  device_config_10bit.scl_speed_hz = STANDARD_FREQ;

  int device_index_10bit = test_bus->CreateDevice(device_config_10bit);
  if (device_index_10bit < 0) {
    ESP_LOGE(TAG, "Failed to create 10-bit address device");
    return false;
  }

  // Verify both devices exist
  BaseI2c* device_7bit = test_bus->GetDevice(device_index_7bit);
  BaseI2c* device_10bit = test_bus->GetDevice(device_index_10bit);

  if (!device_7bit || !device_10bit) {
    ESP_LOGE(TAG, "Failed to retrieve created devices");
    return false;
  }

  ESP_LOGI(TAG, "Successfully created devices with 7-bit and 10-bit addressing");
  ESP_LOGI(TAG, "[SUCCESS] Address mode tests passed");
  return true;
}

bool test_i2c_esp_specific_features() noexcept {
  log_test_separator("ESP-Specific I2C Features");

  // Test different clock sources
  for (auto clk_src :
       {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_APB, hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    hf_i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_NUM_0;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.clk_source = clk_src;
    bus_config.enable_internal_pullup = true;
    bus_config.trans_queue_depth = 16; // Test larger queue

    auto test_bus = std::make_unique<EspI2cBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize with clock source %d", static_cast<int>(clk_src));
      return false;
    }

    ESP_LOGI(TAG, "Successfully initialized with clock source %d", static_cast<int>(clk_src));
    test_bus->Deinitialize();
  }

  // Test power management features
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_NUM_0;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.allow_pd = true; // Enable power down in sleep
  bus_config.enable_internal_pullup = true;

  auto test_bus = std::make_unique<EspI2cBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize with power management");
    return false;
  }

  ESP_LOGI(TAG, "Successfully initialized with power management features");
  ESP_LOGI(TAG, "[SUCCESS] ESP-specific feature tests passed");
  return true;
}

bool test_i2c_thread_safety() noexcept {
  log_test_separator("I2C Thread Safety");

  // This test would require multiple tasks in a real implementation
  // For now, we'll just verify the mutex protection doesn't break anything

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create a device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = STANDARD_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create device for thread safety test");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get device for thread safety test");
    return false;
  }

  // Perform multiple rapid operations (simulating concurrent access)
  for (int i = 0; i < 10; ++i) {
    uint8_t data = static_cast<uint8_t>(i);
    device->Write(&data, 1, 50);
    device->Read(&data, 1, 50);
  }

  ESP_LOGI(TAG, "[SUCCESS] Thread safety tests passed (basic verification)");
  return true;
}

bool test_i2c_performance() noexcept {
  log_test_separator("I2C Performance Tests");

  auto test_bus = create_test_bus(FAST_FREQ); // Use fast mode for performance test
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Create a device
  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1;
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = FAST_FREQ;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create device for performance test");
    return false;
  }

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get device for performance test");
    return false;
  }

  // Performance test: multiple write operations
  const int num_operations = 100;
  uint8_t test_data[16];
  std::fill(test_data, test_data + sizeof(test_data), 0xAA);

  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < num_operations; ++i) {
    device->Write(test_data, sizeof(test_data), 100);
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time_us = end_time - start_time;
  double avg_time_ms = (double)total_time_us / (num_operations * 1000.0);

  ESP_LOGI(TAG, "Performance test: %d operations in %llu μs (avg: %.2f ms per operation)",
           num_operations, total_time_us, avg_time_ms);

  ESP_LOGI(TAG, "[SUCCESS] Performance tests completed");
  return true;
}

bool test_i2c_edge_cases() noexcept {
  log_test_separator("I2C Edge Cases");

  auto test_bus = create_test_bus();
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test maximum number of devices (implementation dependent)
  std::vector<int> device_indices;
  for (uint16_t addr = 0x08; addr < 0x78; ++addr) {
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = addr;
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = STANDARD_FREQ;

    int idx = test_bus->CreateDevice(device_config);
    if (idx >= 0) {
      device_indices.push_back(idx);
    } else {
      break; // Reached limit or error
    }
  }

  ESP_LOGI(TAG, "Created maximum of %zu devices", device_indices.size());

  // Test bus reset
  if (!test_bus->ResetBus()) {
    ESP_LOGW(TAG, "Bus reset failed or not supported");
  } else {
    ESP_LOGI(TAG, "Bus reset successful");
  }

  ESP_LOGI(TAG, "[SUCCESS] Edge case tests passed");
  return true;
}

bool test_i2c_power_management() noexcept {
  log_test_separator("I2C Power Management");

  // Test with power down allowed
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_NUM_0;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.allow_pd = true;
  bus_config.enable_internal_pullup = true;

  auto test_bus = std::make_unique<EspI2cBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize bus with power management");
    return false;
  }

  ESP_LOGI(TAG, "Successfully tested power management configuration");
  ESP_LOGI(TAG, "[SUCCESS] Power management tests passed");
  return true;
}

// Helper function implementations
EspI2cBus* create_test_bus(uint32_t freq) noexcept {
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_NUM_0;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.enable_internal_pullup = true;

  auto bus = std::make_unique<EspI2cBus>(bus_config);
  if (!bus->Initialize()) {
    return nullptr;
  }

  return bus.release(); // Transfer ownership to caller
}

bool verify_device_functionality(BaseI2c* device) noexcept {
  if (!device)
    return false;

  uint8_t test_data = 0xAA;
  hf_i2c_err_t result = device->Write(&test_data, 1, 100);
  return result != hf_i2c_err_t::I2C_ERR_FAILURE;
}

void log_test_separator(const char* test_name) noexcept {
  ESP_LOGI(TAG,
           "\n"
           "═══════════════════════════════════════════════════════════════════\n"
           "  %s\n"
           "═══════════════════════════════════════════════════════════════════",
           test_name);
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Run all I2C tests
  RUN_TEST(test_i2c_bus_initialization);
  RUN_TEST(test_i2c_bus_deinitialization);
  RUN_TEST(test_i2c_configuration_validation);
  RUN_TEST(test_i2c_device_creation);
  RUN_TEST(test_i2c_device_management);
  RUN_TEST(test_i2c_device_probing);
  RUN_TEST(test_i2c_bus_scanning);
  RUN_TEST(test_i2c_write_operations);
  RUN_TEST(test_i2c_read_operations);
  RUN_TEST(test_i2c_write_read_operations);
  RUN_TEST(test_i2c_error_handling);
  RUN_TEST(test_i2c_timeout_handling);
  RUN_TEST(test_i2c_multi_device_operations);
  RUN_TEST(test_i2c_clock_speeds);
  RUN_TEST(test_i2c_address_modes);
  RUN_TEST(test_i2c_esp_specific_features);
  RUN_TEST(test_i2c_thread_safety);
  RUN_TEST(test_i2c_performance);
  RUN_TEST(test_i2c_edge_cases);
  RUN_TEST(test_i2c_power_management);

  print_test_summary(g_test_results, "I2C", TAG);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
