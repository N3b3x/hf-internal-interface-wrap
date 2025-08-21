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
#include "mcu/esp32/EspGpio.h"

static const char* TAG = "I2C_Test";
static TestResults g_test_results;

// GPIO14 test progression indicator
static EspGpio* g_test_progress_gpio = nullptr;
static bool g_test_progress_state = false;
static constexpr hf_pin_num_t TEST_PROGRESS_PIN = 14;

// Test configuration constants
static constexpr i2c_port_t I2C_PORT_NUM = I2C_NUM_0;
static constexpr hf_pin_num_t TEST_SDA_PIN = 6;
static constexpr hf_pin_num_t TEST_SCL_PIN = 7;
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

// NEW ASYNC OPERATION TESTS
bool test_i2c_async_operations() noexcept;
bool test_i2c_async_timeout_handling() noexcept;
bool test_i2c_async_multiple_operations() noexcept;

// NEW INDEX-BASED ACCESS TESTS
bool test_i2c_index_based_access() noexcept;

// Helper functions
std::unique_ptr<EspI2cBus> create_test_bus(uint32_t freq = STANDARD_FREQ) noexcept;
bool verify_device_functionality(BaseI2c* device) noexcept;
void log_test_separator(const char* test_name) noexcept;

/**
 * @brief Initialize the test progression indicator on GPIO14
 */
bool init_test_progress_indicator() noexcept {
  g_test_progress_gpio = new EspGpio(TEST_PROGRESS_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                                     hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                                     hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);
  
  if (!g_test_progress_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO14");
    return false;
  }
  
  g_test_progress_gpio->SetInactive();
  ESP_LOGI(TAG, "Test progression indicator GPIO14 initialized");
  return true;
}

/**
 * @brief Flip the test progression indicator to show next test
 */
void flip_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_state = !g_test_progress_state;
    if (g_test_progress_state) {
      g_test_progress_gpio->SetActive();
    } else {
      g_test_progress_gpio->SetInactive();
    }
    ESP_LOGI(TAG, "Test progression indicator: %s", g_test_progress_state ? "HIGH" : "LOW");
  }
}

/**
 * @brief Cleanup the test progression indicator GPIO
 */
void cleanup_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_gpio->SetInactive(); // Ensure pin is low
    delete g_test_progress_gpio;
    g_test_progress_gpio = nullptr;
  }
}

bool test_i2c_bus_initialization() noexcept {
  log_test_separator("I2C Bus Initialization");

  // Test 1: Basic initialization
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_PORT_NUM;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.flags.enable_internal_pullup = true;

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
       {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT, hf_i2c_clock_source_t::HF_I2C_CLK_SRC_RC_FAST,
        hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    hf_i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_PORT_NUM;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.clk_source = clk_src;
    bus_config.flags.enable_internal_pullup = true;

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
    bus_config.i2c_port = I2C_PORT_NUM;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.glitch_ignore_cnt = filter;
    bus_config.flags.enable_internal_pullup = true;

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

  // Test probing non-existent device with fast timeout
  bool exists = test_bus->ProbeDevice(NONEXISTENT_ADDR, 10); // Fast 10ms timeout
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

  // Scan the bus for devices with fast scanning (10ms timeout)
  std::vector<hf_u16_t> found_devices;
  size_t device_count = test_bus->ScanDevices(found_devices, 0x08, 0x77, 50); // Fast 10ms timeout

  ESP_LOGI(TAG, "Bus scan found %zu devices", device_count);
  for (auto addr : found_devices) {
    ESP_LOGI(TAG, "  - Device at address 0x%02X", addr);
  }

  // Test custom scan range with fast scanning
  std::vector<hf_u16_t> limited_scan;
  size_t limited_count = test_bus->ScanDevices(limited_scan, 0x20, 0x30, 50); // Fast 10ms timeout
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
       {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT, hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    hf_i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_PORT_NUM;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.clk_source = clk_src;
    bus_config.flags.enable_internal_pullup = true;
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
  bus_config.i2c_port = I2C_PORT_NUM;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.flags.allow_pd = true; // Enable power down in sleep
  bus_config.flags.enable_internal_pullup = true;

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

  // Perform multiple operations with proper delays to prevent bus hanging
  ESP_LOGI(TAG, "Testing thread safety with %d operation pairs", 10);
  
  // Add overall timeout protection to prevent infinite hanging
  uint64_t test_start_time = esp_timer_get_time();
  const uint64_t max_test_duration_us = 30000000; // 30 seconds max
  
  for (int i = 0; i < 10; ++i) {
    // Check overall timeout
    if ((esp_timer_get_time() - test_start_time) > max_test_duration_us) {
      ESP_LOGW(TAG, "Thread safety test timeout - stopping after %d operations", i);
      break;
    }
    
    uint8_t data = static_cast<uint8_t>(i);
    
    // Write operation with reasonable timeout
    hf_i2c_err_t write_result = device->Write(&data, 1, 200);
    if (write_result != hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGW(TAG, "Write operation %d failed: %s", i, HfI2CErrToString(write_result).data());
      // Continue testing other operations
    }
    
    // Small delay to allow bus to stabilize
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Read operation with reasonable timeout
    uint8_t read_data;
    hf_i2c_err_t read_result = device->Read(&read_data, 1, 200);
    if (read_result != hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGW(TAG, "Read operation %d failed: %s", i, HfI2CErrToString(read_result).data());
      // Continue testing other operations
    }
    
    // Additional delay between operation pairs to prevent bus congestion
    vTaskDelay(pdMS_TO_TICKS(20));
    
    ESP_LOGD(TAG, "Operation pair %d completed: write=%s, read=%s", 
             i, HfI2CErrToString(write_result).data(), HfI2CErrToString(read_result).data());
  }
  
  uint64_t test_duration_us = esp_timer_get_time() - test_start_time;
  ESP_LOGI(TAG, "Thread safety test completed in %llu μs", test_duration_us);

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

  // Performance test: multiple write operations with proper delays
  const int num_operations = 50; // Reduced from 100 to prevent overwhelming the bus
  uint8_t test_data[16];
  std::fill(test_data, test_data + sizeof(test_data), 0xAA);

  ESP_LOGI(TAG, "Starting performance test with %d operations at %lu Hz", num_operations, FAST_FREQ);
  uint64_t start_time = esp_timer_get_time();
  
  // Add overall timeout protection to prevent infinite hanging
  const uint64_t max_test_duration_us = 60000000; // 60 seconds max for performance test

  int successful_operations = 0;
  int failed_operations = 0;

  for (int i = 0; i < num_operations; ++i) {
    // Check overall timeout to prevent infinite hanging
    if ((esp_timer_get_time() - start_time) > max_test_duration_us) {
      ESP_LOGW(TAG, "Performance test timeout - stopping after %d operations", i);
      break;
    }
    
    // Add small delay between operations to prevent bus contention
    if (i > 0) {
      vTaskDelay(pdMS_TO_TICKS(2)); // 2ms delay between operations
    }
    
    hf_i2c_err_t result = device->Write(test_data, sizeof(test_data), 500); // Increased timeout
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
      successful_operations++;
    } else {
      failed_operations++;
      ESP_LOGW(TAG, "Performance test operation %d failed: %s", i, HfI2CErrToString(result).data());
      
      // If we get too many failures, add extra delay to let bus recover
      if (failed_operations > 5) {
        ESP_LOGW(TAG, "Too many failures, adding recovery delay");
        vTaskDelay(pdMS_TO_TICKS(50));
        failed_operations = 0; // Reset counter
      }
    }
    
    // Progress indicator every 10 operations
    if ((i + 1) % 10 == 0) {
      ESP_LOGI(TAG, "Performance test progress: %d/%d operations completed", i + 1, num_operations);
      vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay every 10 operations
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time_us = end_time - start_time;
  
  // Calculate actual operations completed (may be less than num_operations due to timeout)
  int actual_operations = successful_operations + failed_operations;
  double avg_time_ms = (actual_operations > 0) ? 
                       (double)total_time_us / (actual_operations * 1000.0) : 0.0;

  ESP_LOGI(TAG, "Performance test completed: %d/%d operations in %llu μs", 
           actual_operations, num_operations, total_time_us);
  ESP_LOGI(TAG, "  Successful: %d, Failed: %d", successful_operations, failed_operations);
  ESP_LOGI(TAG, "  Average time per operation: %.2f ms", avg_time_ms);

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
  bus_config.i2c_port = I2C_PORT_NUM;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.flags.allow_pd = true;
  bus_config.flags.enable_internal_pullup = true; 

  auto test_bus = std::make_unique<EspI2cBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize bus with power management");
    return false;
  }

  ESP_LOGI(TAG, "Successfully tested power management configuration");
  ESP_LOGI(TAG, "[SUCCESS] Power management tests passed");
  return true;
}

//==============================================//
// NEW ASYNC OPERATION TESTS                   //
//==============================================//

bool test_i2c_async_operations() noexcept {
  log_test_separator("I2C Async Operations");

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

  EspI2cDevice* esp_device = test_bus->GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to get ESP device");
    return false;
  }

  // Test async mode support
  if (!esp_device->IsAsyncModeSupported()) {
    ESP_LOGW(TAG, "Async mode not supported on this device");
    return true; // Not a failure, just not supported
  }

  // Test async write operation
  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
  bool async_completed = false;
  hf_i2c_err_t async_result = hf_i2c_err_t::I2C_ERR_FAILURE;
  size_t async_bytes = 0;

  auto async_callback = [&async_completed, &async_result, &async_bytes](
      hf_i2c_err_t result, size_t bytes, void* user_data) {
    async_completed = true;
    async_result = result;
    async_bytes = bytes;
    ESP_LOGI(TAG, "Async write completed: %s, %zu bytes", 
             HfI2CErrToString(result).data(), bytes);
  };

  // Try async operation
  hf_i2c_err_t result = esp_device->WriteAsync(test_data, sizeof(test_data), 
                                               async_callback, nullptr, 1000);
  
  ESP_LOGI(TAG, "Async write result: %s", HfI2CErrToString(result).data());

  // Wait for completion or timeout
  uint32_t wait_timeout = 2000; // 2 seconds
  uint32_t start_time = esp_timer_get_time() / 1000;
  
  while (!async_completed && ((esp_timer_get_time() / 1000) - start_time < wait_timeout)) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if (async_completed) {
    ESP_LOGI(TAG, "Async operation completed successfully");
  } else {
    ESP_LOGI(TAG, "Async operation did not complete (expected for non-existent device)");
  }

  ESP_LOGI(TAG, "[SUCCESS] Async operations tests passed");
  return true;
}

bool test_i2c_async_timeout_handling() noexcept {
  log_test_separator("I2C Async Timeout Handling");

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

  EspI2cDevice* esp_device = test_bus->GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to get ESP device");
    return false;
  }

  // Test timeout when trying to start async operation while another is in progress
  bool first_completed = false;
  auto first_callback = [&first_completed](hf_i2c_err_t result, size_t bytes, void* user_data) {
    vTaskDelay(pdMS_TO_TICKS(100)); // Simulate slow callback
    first_completed = true;
    ESP_LOGI(TAG, "First async operation completed");
  };

  auto second_callback = [](hf_i2c_err_t result, size_t bytes, void* user_data) {
    ESP_LOGI(TAG, "Second async operation completed");
  };

  uint8_t test_data[] = {0xAA, 0xBB};
  
  // Start first async operation
  hf_i2c_err_t result1 = esp_device->WriteAsync(test_data, sizeof(test_data), 
                                                first_callback, nullptr, 1000);
  
  if (result1 == hf_i2c_err_t::I2C_SUCCESS) {
    // Try second async operation with short timeout (should timeout)
    hf_i2c_err_t result2 = esp_device->WriteAsync(test_data, sizeof(test_data), 
                                                  second_callback, nullptr, 10); // 10ms timeout
    
    if (result2 == hf_i2c_err_t::I2C_ERR_BUS_BUSY) {
      ESP_LOGI(TAG, "Second async operation correctly timed out");
    } else {
      ESP_LOGW(TAG, "Second async operation result: %s", HfI2CErrToString(result2).data());
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Async timeout handling tests passed");
  return true;
}

bool test_i2c_async_multiple_operations() noexcept {
  log_test_separator("I2C Async Multiple Operations");

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

  EspI2cDevice* esp_device = test_bus->GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to get ESP device");
    return false;
  }

  // Test sequential async operations
  int completed_operations = 0;
  auto completion_callback = [&completed_operations](hf_i2c_err_t result, size_t bytes, void* user_data) {
    completed_operations++;
    int operation_id = *static_cast<int*>(user_data);
    ESP_LOGI(TAG, "Async operation %d completed: %s", operation_id, HfI2CErrToString(result).data());
  };

  uint8_t test_data[] = {0x10, 0x20, 0x30};
  
  // Start multiple async operations sequentially
  for (int i = 0; i < 3; ++i) {
    int* operation_id = new int(i); // Note: In real code, manage memory properly
    
    hf_i2c_err_t result = esp_device->WriteAsync(test_data, sizeof(test_data), 
                                                completion_callback, operation_id, 2000);
    
    ESP_LOGI(TAG, "Async operation %d start result: %s", i, HfI2CErrToString(result).data());
    
    // Wait for this operation to complete before starting next
    esp_device->WaitAsyncOperationComplete(1000);
    
    delete operation_id; // Clean up
  }

  ESP_LOGI(TAG, "[SUCCESS] Async multiple operations tests passed");
  return true;
}

bool test_i2c_index_based_access() noexcept {
  log_test_separator("I2C Index-Based Access Tests");

  // Create test bus
  auto test_bus = create_test_bus(STANDARD_FREQ);
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus for index access tests");
    return false;
  }

  // Create multiple device configurations
  std::vector<hf_i2c_device_config_t> device_configs;
  
  // Create configs using the default constructor and then set fields
  hf_i2c_device_config_t config1;
  config1.device_address = TEST_DEVICE_ADDR_1;
  config1.scl_speed_hz = STANDARD_FREQ;
  config1.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  
  hf_i2c_device_config_t config2;
  config2.device_address = TEST_DEVICE_ADDR_2;
  config2.scl_speed_hz = STANDARD_FREQ;
  config2.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  
  hf_i2c_device_config_t config3;
  config3.device_address = 0x4A;
  config3.scl_speed_hz = STANDARD_FREQ;
  config3.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  
  hf_i2c_device_config_t config4;
  config4.device_address = 0x4B;
  config4.scl_speed_hz = STANDARD_FREQ;
  config4.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  
  device_configs.push_back(config1);
  device_configs.push_back(config2);
  device_configs.push_back(config3);
  device_configs.push_back(config4);

  // Create devices and store their indices
  std::vector<int> device_indices;
  for (const auto& config : device_configs) {
    int index = test_bus->CreateDevice(config);
    if (index >= 0) {
      device_indices.push_back(index);
      ESP_LOGI(TAG, "Created device at address 0x%02X with index %d", config.device_address, index);
    } else {
      ESP_LOGE(TAG, "Failed to create device at address 0x%02X", config.device_address);
      return false;
    }
  }

  ESP_LOGI(TAG, "Created %zu devices for index access testing", device_indices.size());

  // Test 1: Basic index-based access using operator[]
  ESP_LOGI(TAG, "--- Test 1: Basic Index-Based Access ---");
  for (int index : device_indices) {
    BaseI2c* device = (*test_bus)[index];
    if (device) {
      ESP_LOGI(TAG, "Device[%d] = BaseI2c* at address 0x%02X", index, device->GetDeviceAddress());
    } else {
      ESP_LOGE(TAG, "Device[%d] returned nullptr", index);
      return false;
    }
  }

  // Test 2: ESP-specific device access using At()
  ESP_LOGI(TAG, "--- Test 2: ESP-Specific Device Access ---");
  for (int index : device_indices) {
    EspI2cDevice* esp_device = test_bus->At(index);
    if (esp_device) {
      ESP_LOGI(TAG, "At(%d) = EspI2cDevice* at address 0x%02X", index, esp_device->GetDeviceAddress());
    } else {
      ESP_LOGE(TAG, "At(%d) returned nullptr", index);
      return false;
    }
  }

  // Test 3: First and Last device access
  ESP_LOGI(TAG, "--- Test 3: First and Last Device Access ---");
  BaseI2c* first_device = test_bus->GetFirstDevice();
  BaseI2c* last_device = test_bus->GetLastDevice();

  if (first_device) {
    ESP_LOGI(TAG, "First device: BaseI2c* at address 0x%02X", first_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "GetFirstDevice() returned nullptr");
    return false;
  }
  if (last_device) {
    ESP_LOGI(TAG, "Last device: BaseI2c* at address 0x%02X", last_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "GetLastDevice() returned nullptr");
    return false;
  }

  // Test 4: Index validation
  ESP_LOGI(TAG, "--- Test 4: Index Validation ---");
  ESP_LOGI(TAG, "IsValidIndex(-1): %s", test_bus->IsValidIndex(-1) ? "true" : "false");
  ESP_LOGI(TAG, "IsValidIndex(0): %s", test_bus->IsValidIndex(0) ? "true" : "false");
  ESP_LOGI(TAG, "IsValidIndex(%zu): %s", device_indices.size(), test_bus->IsValidIndex(device_indices.size()) ? "true" : "false");
  ESP_LOGI(TAG, "IsValidIndex(%zu): %s", device_indices.size() - 1, test_bus->IsValidIndex(device_indices.size() - 1) ? "true" : "false");

  // Test 5: Get all devices as vectors
  ESP_LOGI(TAG, "--- Test 5: Get All Devices as Vectors ---");
  std::vector<BaseI2c*> all_devices = test_bus->GetAllDevices();
  ESP_LOGI(TAG, "GetAllDevices() returned %zu devices", all_devices.size());
  if (all_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "Device count mismatch: expected %zu, got %zu", device_indices.size(), all_devices.size());
    return false;
  }

  std::vector<EspI2cDevice*> all_esp_devices = test_bus->GetAllEspDevices();
  ESP_LOGI(TAG, "GetAllEspDevices() returned %zu devices", all_esp_devices.size());
  if (all_esp_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "ESP device count mismatch: expected %zu, got %zu", device_indices.size(), all_esp_devices.size());
    return false;
  }

  // Test 6: Get device addresses
  ESP_LOGI(TAG, "--- Test 6: Get Device Addresses ---");
  std::vector<hf_u16_t> addresses = test_bus->GetDeviceAddresses();
  ESP_LOGI(TAG, "Device addresses:");
  for (size_t i = 0; i < addresses.size(); ++i) {
    ESP_LOGI(TAG, "  [%zu]: 0x%02X", i, addresses[i]);
  }
  if (addresses.size() != device_indices.size()) {
    ESP_LOGE(TAG, "Address count mismatch: expected %zu, got %zu", device_indices.size(), addresses.size());
    return false;
  }

  // Test 7: Bus state queries
  ESP_LOGI(TAG, "--- Test 7: Bus State Queries ---");
  ESP_LOGI(TAG, "HasDevices(): %s", test_bus->HasDevices() ? "true" : "false");
  ESP_LOGI(TAG, "IsEmpty(): %s", test_bus->IsEmpty() ? "true" : "false");
  ESP_LOGI(TAG, "GetDeviceCount(): %zu", test_bus->GetDeviceCount());
  
  if (!test_bus->HasDevices()) {
    ESP_LOGE(TAG, "HasDevices() returned false when devices exist");
    return false;
  }
  if (test_bus->IsEmpty()) {
    ESP_LOGE(TAG, "IsEmpty() returned true when devices exist");
    return false;
  }
  if (test_bus->GetDeviceCount() != device_indices.size()) {
    ESP_LOGE(TAG, "GetDeviceCount() mismatch: expected %zu, got %zu", device_indices.size(), test_bus->GetDeviceCount());
    return false;
  }

  // Test 8: Find device by address
  ESP_LOGI(TAG, "--- Test 8: Find Device by Address ---");
  for (hf_u16_t addr : addresses) {
    int found_index = test_bus->FindDeviceIndex(addr);
    ESP_LOGI(TAG, "FindDeviceIndex(0x%02X) = %d", addr, found_index);
    if (found_index < 0) {
      ESP_LOGE(TAG, "Failed to find device at address 0x%02X", addr);
      return false;
    }
  }

  // Test 9: Out-of-bounds access (should return nullptr)
  ESP_LOGI(TAG, "--- Test 9: Out-of-Bounds Access ---");
  BaseI2c* out_of_bounds = (*test_bus)[1000];
  ESP_LOGI(TAG, "Device[1000] = %s", out_of_bounds ? "valid pointer" : "nullptr (expected)");
  if (out_of_bounds != nullptr) {
    ESP_LOGE(TAG, "Out-of-bounds access should return nullptr");
    return false;
  }

  // Test 10: Const access methods
  ESP_LOGI(TAG, "--- Test 10: Const Access Methods ---");
  const EspI2cBus& const_bus = *test_bus;
  const BaseI2c* const_device = const_bus[0];
  if (const_device) {
    ESP_LOGI(TAG, "const_bus[0] = const BaseI2c* at address 0x%02X", const_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "const_bus[0] returned nullptr");
    return false;
  }

  const EspI2cDevice* const_esp_device = const_bus.At(0);
  if (const_esp_device) {
    ESP_LOGI(TAG, "const_bus.At(0) = const EspI2cDevice* at address 0x%02X", const_esp_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "const_bus.At(0) returned nullptr");
    return false;
  }

  std::vector<const BaseI2c*> const_all_devices = const_bus.GetAllDevices();
  ESP_LOGI(TAG, "const_bus.GetAllDevices() returned %zu devices", const_all_devices.size());
  if (const_all_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "Const device count mismatch: expected %zu, got %zu", device_indices.size(), const_all_devices.size());
    return false;
  }

  // Test 11: Iteration over devices using indices
  ESP_LOGI(TAG, "--- Test 11: Iteration Using Indices ---");
  ESP_LOGI(TAG, "Iterating over devices using indices:");
  for (size_t i = 0; i < test_bus->GetDeviceCount(); ++i) {
    BaseI2c* device = (*test_bus)[i];
    if (device) {
      ESP_LOGI(TAG, "  [%zu]: BaseI2c* at address 0x%02X", i, device->GetDeviceAddress());
    } else {
      ESP_LOGE(TAG, "Device[%zu] returned nullptr", i);
      return false;
    }
  }

  // Test 12: Iteration using GetAllDevices()
  ESP_LOGI(TAG, "--- Test 12: Iteration Using GetAllDevices() ---");
  ESP_LOGI(TAG, "Iterating over devices using GetAllDevices():");
  auto devices = test_bus->GetAllDevices();
  for (size_t i = 0; i < devices.size(); ++i) {
    if (devices[i]) {
      ESP_LOGI(TAG, "Device[%zu]: BaseI2c* at address 0x%02X", i, devices[i]->GetDeviceAddress());
    } else {
      ESP_LOGE(TAG, "Device[%zu] returned nullptr", i, i);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Index-based access tests passed");
  return true;
}

// Helper function implementations
std::unique_ptr<EspI2cBus> create_test_bus(uint32_t freq) noexcept {
  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_PORT_NUM;
  bus_config.sda_io_num = TEST_SDA_PIN;
  bus_config.scl_io_num = TEST_SCL_PIN;
  bus_config.trans_queue_depth = 32;
  bus_config.clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT;
  bus_config.glitch_ignore_cnt = hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES;
  bus_config.intr_priority = 1;
  bus_config.flags.enable_internal_pullup = true;
  bus_config.flags.allow_pd = false;

  auto bus = std::make_unique<EspI2cBus>(bus_config);
  if (!bus->Initialize()) {
    return nullptr;
  }

  return bus; // Transfer ownership to caller
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
  ESP_LOGI(TAG, "║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                  ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                               ║");
  ESP_LOGI(TAG, "║ Features: I2C, Bus Initialization, Device Management, Probing, Scanning,     ║");
  ESP_LOGI(TAG, "║ Write/Read, Error Handling, Timeout Handling, Multi-Device Operations,       ║");
  ESP_LOGI(TAG, "║ Clock Speeds, Address Modes, ESP-Specific Features, Thread Safety,           ║");
  ESP_LOGI(TAG, "║ Performance, Edge Cases, Power Management, Async Operations,                 ║");
  ESP_LOGI(TAG, "║ Timeout Handling, Multiple Operations, Index-Based Access, Cleanup           ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
  }

  // Run all I2C tests
  ESP_LOGI(TAG, "\n=== I2C BUS TESTS ===");
  RUN_TEST(test_i2c_bus_initialization);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_bus_deinitialization);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_configuration_validation);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C DEVICE TESTS ===");
  RUN_TEST(test_i2c_device_creation);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_device_management);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_device_probing);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_bus_scanning);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C WRITE/READ TESTS ===");
  RUN_TEST(test_i2c_write_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_read_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_write_read_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_error_handling);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_timeout_handling);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_multi_device_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_clock_speeds);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_address_modes);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C ESP-SPECIFIC FEATURES ===");
  RUN_TEST(test_i2c_esp_specific_features);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_thread_safety);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C PERFORMANCE TESTS ===");
  RUN_TEST(test_i2c_performance);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_edge_cases);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_power_management);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C ASYNC OPERATION TESTS ===");
  RUN_TEST(test_i2c_async_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_async_timeout_handling);
  flip_test_progress_indicator();
  RUN_TEST(test_i2c_async_multiple_operations);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== I2C INDEX-BASED ACCESS TESTS ===");
  RUN_TEST(test_i2c_index_based_access);
  flip_test_progress_indicator();
  
  print_test_summary(g_test_results, "I2C", TAG);

  ESP_LOGI(TAG, "I2C comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");
  ESP_LOGI(TAG, "\n");

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 I2C COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Cleanup test progression indicator
  cleanup_test_progress_indicator();

  while (true) {
    ESP_LOGI(TAG, "System up and running for %d seconds", esp_timer_get_time() / 1000000);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
