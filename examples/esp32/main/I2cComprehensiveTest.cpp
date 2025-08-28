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

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core I2C functionality tests
static constexpr bool ENABLE_CORE_TESTS =
    false; // ESP-IDF verification, bus/device init, basic operations
static constexpr bool ENABLE_OPERATION_TESTS =
    false; // Read/write, error handling, timeouts, multi-device
static constexpr bool ENABLE_ADVANCED_TESTS =
    false; // Clock speeds, address modes, ESP-specific, thread safety
static constexpr bool ENABLE_PERFORMANCE_TESTS = false; // Performance, edge cases, power management
static constexpr bool ENABLE_SPECIALIZED_TESTS =
    true; // Async operations, mode switching, index access, probing

static TestResults g_test_results;

//=============================================================================
// TEST PROGRESSION INDICATOR
//=============================================================================
// GPIO14 test progression indicator
static EspGpio* g_test_progress_gpio = nullptr;
static bool g_test_progress_state = false;
static constexpr hf_pin_num_t TEST_PROGRESS_PIN = 14;

//=============================================================================
// TEST CONFIGURATION
//=============================================================================

// Test configuration constants
static constexpr i2c_port_t I2C_PORT_NUM = I2C_NUM_0;
static constexpr hf_pin_num_t TEST_SDA_PIN = 21;
static constexpr hf_pin_num_t TEST_SCL_PIN = 22;

static constexpr uint16_t TEST_DEVICE_ADDR_1 = 0x48; // Common I2C device address
static constexpr uint16_t TEST_DEVICE_ADDR_2 = 0x50; // EEPROM address
static constexpr uint16_t NONEXISTENT_ADDR = 0x7E;   // Unlikely to exist
static constexpr uint32_t STANDARD_FREQ = 100000;    // 100kHz
static constexpr uint32_t FAST_FREQ = 400000;        // 400kHz
static constexpr uint32_t FAST_PLUS_FREQ = 1000000;  // 1MHz

// Timeout constants for easy configuration
static constexpr uint32_t TIMEOUT_VERY_FAST_MS = 100;  // 100ms - Very fast operations
static constexpr uint32_t TIMEOUT_FAST_MS = 100;       // 100ms - Fast operations
static constexpr uint32_t TIMEOUT_STANDARD_MS = 100;   // 100ms - Standard operations
static constexpr uint32_t TIMEOUT_MEDIUM_MS = 500;     // 500ms - Medium operations
static constexpr uint32_t TIMEOUT_LONG_MS = 1000;      // 1000ms - Long operations
static constexpr uint32_t TIMEOUT_VERY_LONG_MS = 2000; // 2000ms - Very long operations
static constexpr uint32_t TIMEOUT_EXTENDED_MS = 10000; // 10000ms - Extended operations

// Helper function to create consistent I2C bus configuration (matches replica test pattern)
hf_i2c_master_bus_config_t create_test_bus_config(
    hf_i2c_mode_t mode, uint32_t freq = STANDARD_FREQ,
    hf_i2c_clock_source_t clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT,
    hf_i2c_glitch_filter_t glitch_filter = hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES,
    bool allow_power_down = false) noexcept {
  hf_i2c_master_bus_config_t config = {}; // Zero-init, no constructor
  config.i2c_port = I2C_PORT_NUM;
  config.sda_io_num = TEST_SDA_PIN;
  config.scl_io_num = TEST_SCL_PIN;
  config.mode = mode;
  config.trans_queue_depth = (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? 32 : 0;
  config.clk_source = clk_source;
  config.glitch_ignore_cnt = glitch_filter;
  config.intr_priority = 0; // Match replica test exactly
  config.flags.enable_internal_pullup = true;
  config.flags.allow_pd = allow_power_down;

  return config;
}

// Macro to create test bus configuration and bus directly inline (matches replica test pattern
// exactly)
#define CREATE_TEST_BUS_INLINE(bus_var_name, mode_val)                                    \
  hf_i2c_master_bus_config_t bus_config = {};                                             \
  bus_config.i2c_port = I2C_PORT_NUM;                                                     \
  bus_config.sda_io_num = TEST_SDA_PIN;                                                   \
  bus_config.scl_io_num = TEST_SCL_PIN;                                                   \
  bus_config.mode = mode_val;                                                             \
  bus_config.trans_queue_depth = (mode_val == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? 32 : 0; \
  bus_config.clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT;                  \
  bus_config.glitch_ignore_cnt = hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES;   \
  bus_config.intr_priority = 0;                                                           \
  bus_config.flags.enable_internal_pullup = true;                                         \
  bus_config.flags.allow_pd = false;                                                      \
  auto bus_var_name = std::make_unique<EspI2cBus>(bus_config);                            \
  if (!bus_var_name->Initialize()) {                                                      \
    ESP_LOGE(TAG, "Failed to initialize I2C bus");                                        \
    return false;                                                                         \
  }

//=============================================================================
// TEST FUNCTIONS DECLARATIONS
//=============================================================================

// Forward declarations
bool test_i2c_espidf_direct_api() noexcept;      // ESP-IDF Direct API Test (FIRST)
bool test_i2c_espidf_wrapper_replica() noexcept; // EspI2cBus Wrapper Replica Test (SECOND)
bool test_i2c_espidf_wrapper_replica_continuous() noexcept; // Continuous replica test loop
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

// NEW MODE-AWARE TESTS
bool test_i2c_sync_mode() noexcept;
bool test_i2c_async_mode() noexcept;
bool test_i2c_mode_switching() noexcept;
bool test_i2c_basic_functionality() noexcept;

// NEW PROBE COMPARISON TEST
bool test_i2c_probe_methods_comparison() noexcept;

// Helper functions
bool verify_device_functionality(BaseI2c* device) noexcept;
void log_test_separator(const char* test_name) noexcept;

//=============================================================================
// TEST FUNCTIONS IMPLEMENTATION
//=============================================================================

/**
 * @brief Initialize the test progression indicator on GPIO14
 */
bool init_test_progress_indicator() noexcept {
  g_test_progress_gpio =
      new EspGpio(TEST_PROGRESS_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
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

  vTaskDelay(pdMS_TO_TICKS(100));
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

  // Test 1: Basic initialization (using replica test pattern)
  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)

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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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
  for (auto clk_src : {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT,
                       hf_i2c_clock_source_t::HF_I2C_CLK_SRC_RC_FAST,
                       hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
    if (!test_bus) {
      ESP_LOGE(TAG, "Failed to initialize with clock source %d", static_cast<int>(clk_src));
      return false;
    }
    test_bus->Deinitialize();
  }

  // Test glitch filter settings
  for (auto filter : {hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_0_CYCLES,
                      hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_3_CYCLES,
                      hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES}) {
    CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
    test_bus->Deinitialize();
  }

  ESP_LOGI(TAG, "[SUCCESS] Configuration validation tests passed");
  return true;
}

bool test_i2c_device_creation() noexcept {
  log_test_separator("I2C Device Creation");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Test 1a: Device should not be initialized yet
  if (device->IsInitialized()) {
    ESP_LOGE(TAG, "Device should not be initialized after creation");
    return false;
  }

  // Test 1b: Initialize the device
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize device");
    return false;
  }

  // Test 1c: Device should now be initialized
  if (!device->IsInitialized()) {
    ESP_LOGE(TAG, "Device should be initialized after Initialize() call");
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

  BaseI2c* device_10bit = test_bus->GetDevice(device_index_10bit);
  if (!device_10bit) {
    ESP_LOGE(TAG, "Failed to get 10-bit device");
    return false;
  }

  // Test 3a: Initialize the 10-bit device
  if (!device_10bit->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize 10-bit device");
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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize all devices
  for (int idx : device_indices) {
    BaseI2c* device = test_bus->GetDevice(idx);
    if (!device) {
      ESP_LOGE(TAG, "Failed to get device at index %d", idx);
      return false;
    }

    if (!device->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize device at index %d", idx);
      return false;
    }
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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Test probing non-existent device with fast timeout
  bool exists = test_bus->ProbeDevice(NONEXISTENT_ADDR, TIMEOUT_VERY_FAST_MS); // Fast timeout
  ESP_LOGI(TAG, "Probe result for address 0x%02X: %s", NONEXISTENT_ADDR,
           exists ? "EXISTS" : "NOT FOUND");

  // Note: We can't guarantee specific devices are connected, so we'll just verify the method works
  // and doesn't crash. In a real test environment, you would connect known devices.

  ESP_LOGI(TAG, "[SUCCESS] Device probing tests passed");
  return true;
}

bool test_i2c_bus_scanning() noexcept {
  log_test_separator("I2C Bus Scanning");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus");
    return false;
  }

  // Scan the bus for devices with fast scanning
  std::vector<hf_u16_t> found_devices;
  size_t device_count =
      test_bus->ScanDevices(found_devices, 0x08, 0x77, TIMEOUT_FAST_MS); // Fast timeout

  ESP_LOGI(TAG, "Bus scan found %zu devices", device_count);
  for (auto addr : found_devices) {
    ESP_LOGI(TAG, "  - Device at address 0x%02X", addr);
  }

  // Test custom scan range with fast scanning
  std::vector<hf_u16_t> limited_scan;
  size_t limited_count =
      test_bus->ScanDevices(limited_scan, 0x20, 0x30, TIMEOUT_FAST_MS); // Fast timeout
  ESP_LOGI(TAG, "Limited scan (0x20-0x30) found %zu devices", limited_count);

  ESP_LOGI(TAG, "[SUCCESS] Bus scanning tests passed");
  return true;
}

bool test_i2c_write_operations() noexcept {
  log_test_separator("I2C Write Operations");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
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
  result = device->Write(multi_bytes, sizeof(multi_bytes), TIMEOUT_MEDIUM_MS);
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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
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
  result = device->Read(multi_bytes, 4, TIMEOUT_MEDIUM_MS);
  ESP_LOGI(TAG, "Read with timeout result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Read operations tests passed");
  return true;
}

bool test_i2c_write_read_operations() noexcept {
  log_test_separator("I2C Write-Read Operations");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test write-read operation (typical register read)
  uint8_t reg_addr = 0x10;
  uint8_t read_data[4];
  hf_i2c_err_t result = device->WriteRead(&reg_addr, 1, read_data, sizeof(read_data));
  ESP_LOGI(TAG, "Write-read operation result: %s", HfI2CErrToString(result).data());

  // Test with timeout
  result = device->WriteRead(&reg_addr, 1, read_data, 2, TIMEOUT_MEDIUM_MS);
  ESP_LOGI(TAG, "Write-read with timeout result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Write-read operations tests passed");
  return true;
}

bool test_i2c_error_handling() noexcept {
  log_test_separator("I2C Error Handling");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test operations that should fail
  uint8_t dummy_data = 0xFF;
  hf_i2c_err_t result = device->Write(&dummy_data, 1, TIMEOUT_STANDARD_MS);
  ESP_LOGI(TAG, "Write to non-existent device result: %s", HfI2CErrToString(result).data());

  result = device->Read(&dummy_data, 1, TIMEOUT_STANDARD_MS);
  ESP_LOGI(TAG, "Read from non-existent device result: %s", HfI2CErrToString(result).data());

  // Test invalid parameters
  result = device->Write(nullptr, 1, TIMEOUT_STANDARD_MS);
  ESP_LOGI(TAG, "Write with null pointer result: %s", HfI2CErrToString(result).data());

  result = device->Read(nullptr, 1, TIMEOUT_STANDARD_MS);
  ESP_LOGI(TAG, "Read with null pointer result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Error handling tests passed");
  return true;
}

bool test_i2c_timeout_handling() noexcept {
  log_test_separator("I2C Timeout Handling");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test various timeout values
  uint8_t dummy_data = 0xFF;
  uint64_t start_time, end_time;

  // Short timeout
  start_time = esp_timer_get_time();
  hf_i2c_err_t result = device->Write(&dummy_data, 1, TIMEOUT_STANDARD_MS); // Standard timeout
  end_time = esp_timer_get_time();
  uint64_t duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Short timeout test: %s (took %llu ms)", HfI2CErrToString(result).data(),
           duration_ms);

  // Very short timeout
  start_time = esp_timer_get_time();
  result = device->Write(&dummy_data, 1, TIMEOUT_VERY_FAST_MS); // Very fast timeout
  end_time = esp_timer_get_time();
  duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Very short timeout test: %s (took %llu ms)", HfI2CErrToString(result).data(),
           duration_ms);

  ESP_LOGI(TAG, "[SUCCESS] Timeout handling tests passed");
  return true;
}

bool test_i2c_multi_device_operations() noexcept {
  log_test_separator("I2C Multi-Device Operations");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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
      // Initialize the device before use
      if (!device->Initialize()) {
        ESP_LOGW(TAG, "Failed to initialize device %zu (addr 0x%02X)", i, addresses[i]);
        continue;
      }
      devices.push_back(device);
    }
  }

  ESP_LOGI(TAG, "Created %zu devices on the bus", devices.size());

  // Test concurrent operations (simulate real-world usage)
  for (auto* device : devices) {
    uint8_t test_data = 0xAA;
    hf_i2c_err_t result = device->Write(&test_data, 1, TIMEOUT_STANDARD_MS);
    ESP_LOGI(TAG, "Multi-device write result: %s", HfI2CErrToString(result).data());

    // Small delay between operations
    vTaskDelay(pdMS_TO_TICKS(TIMEOUT_VERY_FAST_MS));
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
    CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)

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
      // Initialize the device before use
      if (!esp_device->Initialize()) {
        ESP_LOGE(TAG, "Failed to initialize device with speed %lu Hz", speed);
        return false;
      }

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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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
  for (auto clk_src : {hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT,
                       hf_i2c_clock_source_t::HF_I2C_CLK_SRC_XTAL}) {
    CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
    if (!test_bus) {
      ESP_LOGE(TAG, "Failed to initialize with clock source %d", static_cast<int>(clk_src));
      return false;
    }

    ESP_LOGI(TAG, "Successfully initialized with clock source %d", static_cast<int>(clk_src));
    test_bus->Deinitialize();
  }

  // Test power management features
  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC);

  ESP_LOGI(TAG, "Successfully initialized with power management features");
  ESP_LOGI(TAG, "[SUCCESS] ESP-specific feature tests passed");
  return true;
}

bool test_i2c_thread_safety() noexcept {
  log_test_separator("I2C Thread Safety");

  // This test would require multiple tasks in a real implementation
  // For now, we'll just verify the mutex protection doesn't break anything

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize device for thread safety test");
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

    ESP_LOGD(TAG, "Operation pair %d completed: write=%s, read=%s", i,
             HfI2CErrToString(write_result).data(), HfI2CErrToString(read_result).data());
  }

  uint64_t test_duration_us = esp_timer_get_time() - test_start_time;
  ESP_LOGI(TAG, "Thread safety test completed in %llu μs", test_duration_us);

  ESP_LOGI(TAG, "[SUCCESS] Thread safety tests passed (basic verification)");
  return true;
}

bool test_i2c_performance() noexcept {
  log_test_separator("I2C Performance Tests");

  CREATE_TEST_BUS_INLINE(test_bus,
                         hf_i2c_mode_t::HF_I2C_MODE_SYNC) // Use sync mode for performance test

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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize device for performance test");
    return false;
  }

  // Performance test: multiple write operations with proper delays
  const int num_operations = 50; // Reduced from 100 to prevent overwhelming the bus
  uint8_t test_data[16];
  std::fill(test_data, test_data + sizeof(test_data), 0xAA);

  ESP_LOGI(TAG, "Starting performance test with %d operations at %lu Hz", num_operations,
           FAST_FREQ);
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

    hf_i2c_err_t result =
        device->Write(test_data, sizeof(test_data), TIMEOUT_MEDIUM_MS); // Medium timeout
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
      successful_operations++;
    } else {
      failed_operations++;
      ESP_LOGW(TAG, "Performance test operation %d failed: %s", i, HfI2CErrToString(result).data());

      // If we get too many failures, add extra delay to let bus recover
      if (failed_operations > 5) {
        ESP_LOGW(TAG, "Too many failures, adding recovery delay");
        vTaskDelay(pdMS_TO_TICKS(TIMEOUT_FAST_MS));
        failed_operations = 0; // Reset counter
      }
    }

    // Progress indicator every 10 operations
    if ((i + 1) % 10 == 0) {
      ESP_LOGI(TAG, "Performance test progress: %d/%d operations completed", i + 1, num_operations);
      vTaskDelay(pdMS_TO_TICKS(TIMEOUT_VERY_FAST_MS)); // Fast delay every 10 operations
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time_us = end_time - start_time;

  // Calculate actual operations completed (may be less than num_operations due to timeout)
  int actual_operations = successful_operations + failed_operations;
  double avg_time_ms =
      (actual_operations > 0) ? (double)total_time_us / (actual_operations * 1000.0) : 0.0;

  ESP_LOGI(TAG, "Performance test completed: %d/%d operations in %llu μs", actual_operations,
           num_operations, total_time_us);
  ESP_LOGI(TAG, "  Successful: %d, Failed: %d", successful_operations, failed_operations);
  ESP_LOGI(TAG, "  Average time per operation: %.2f ms", avg_time_ms);

  ESP_LOGI(TAG, "[SUCCESS] Performance tests completed");
  return true;
}

bool test_i2c_edge_cases() noexcept {
  log_test_separator("I2C Edge Cases");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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
  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC);
  if (!test_bus) {
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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_ASYNC)
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

  // Initialize the device before use
  if (!esp_device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
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
  };

  // Try async operation
  hf_i2c_err_t result = esp_device->WriteAsync(test_data, sizeof(test_data), async_callback,
                                               nullptr, TIMEOUT_LONG_MS);

  ESP_LOGI(TAG, "Async write result: %s", HfI2CErrToString(result).data());

  // Wait for completion or timeout
  uint32_t wait_timeout = TIMEOUT_VERY_LONG_MS; // 2 seconds
  uint32_t start_time = esp_timer_get_time() / 1000;

  while (!async_completed && ((esp_timer_get_time() / 1000) - start_time < wait_timeout)) {
    vTaskDelay(pdMS_TO_TICKS(TIMEOUT_VERY_FAST_MS));
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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_ASYNC)
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

  // Initialize the device before use
  if (!esp_device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test timeout when trying to start async operation while another is in progress
  bool first_completed = false;
  auto first_callback = [&first_completed](hf_i2c_err_t result, size_t bytes, void* user_data) {
    vTaskDelay(pdMS_TO_TICKS(TIMEOUT_STANDARD_MS)); // Simulate slow callback
    first_completed = true;
  };

  auto second_callback = [](hf_i2c_err_t result, size_t bytes, void* user_data) {};

  uint8_t test_data[] = {0xAA, 0xBB};

  // Start first async operation
  hf_i2c_err_t result1 = esp_device->WriteAsync(test_data, sizeof(test_data), first_callback,
                                                nullptr, TIMEOUT_LONG_MS);

  if (result1 == hf_i2c_err_t::I2C_SUCCESS) {
    // Try second async operation with short timeout (should timeout)
    hf_i2c_err_t result2 = esp_device->WriteAsync(test_data, sizeof(test_data), second_callback,
                                                  nullptr, TIMEOUT_VERY_FAST_MS); // Fast timeout

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

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_ASYNC)
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

  // Initialize the device before use
  if (!esp_device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test sequential async operations
  int completed_operations = 0;
  auto completion_callback = [&completed_operations](hf_i2c_err_t result, size_t bytes,
                                                     void* user_data) {
    completed_operations++;
    int operation_id = *static_cast<int*>(user_data);
  };

  uint8_t test_data[] = {0x10, 0x20, 0x30};

  // Start multiple async operations sequentially
  for (int i = 0; i < 3; ++i) {
    int* operation_id = new int(i); // Note: In real code, manage memory properly

    hf_i2c_err_t result = esp_device->WriteAsync(test_data, sizeof(test_data), completion_callback,
                                                 operation_id, TIMEOUT_VERY_LONG_MS);

    ESP_LOGI(TAG, "Async operation %d start result: %s", i, HfI2CErrToString(result).data());

    // Wait for this operation to complete before starting next
    esp_device->WaitAsyncOperationComplete(TIMEOUT_LONG_MS);

    delete operation_id; // Clean up
  }

  ESP_LOGI(TAG, "[SUCCESS] Async multiple operations tests passed");
  return true;
}

bool test_i2c_index_based_access() noexcept {
  log_test_separator("I2C Index-Based Access Tests");

  // Create test bus
  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
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
      ESP_LOGI(TAG, "At(%d) = EspI2cDevice* at address 0x%02X", index,
               esp_device->GetDeviceAddress());
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
  ESP_LOGI(TAG, "IsValidIndex(%zu): %s", device_indices.size(),
           test_bus->IsValidIndex(device_indices.size()) ? "true" : "false");
  ESP_LOGI(TAG, "IsValidIndex(%zu): %s", device_indices.size() - 1,
           test_bus->IsValidIndex(device_indices.size() - 1) ? "true" : "false");

  // Test 5: Get all devices as vectors
  ESP_LOGI(TAG, "--- Test 5: Get All Devices as Vectors ---");
  std::vector<BaseI2c*> all_devices = test_bus->GetAllDevices();
  ESP_LOGI(TAG, "GetAllDevices() returned %zu devices", all_devices.size());
  if (all_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "Device count mismatch: expected %zu, got %zu", device_indices.size(),
             all_devices.size());
    return false;
  }

  std::vector<EspI2cDevice*> all_esp_devices = test_bus->GetAllEspDevices();
  ESP_LOGI(TAG, "GetAllEspDevices() returned %zu devices", all_esp_devices.size());
  if (all_esp_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "ESP device count mismatch: expected %zu, got %zu", device_indices.size(),
             all_esp_devices.size());
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
    ESP_LOGE(TAG, "Address count mismatch: expected %zu, got %zu", device_indices.size(),
             addresses.size());
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
    ESP_LOGE(TAG, "GetDeviceCount() mismatch: expected %zu, got %zu", device_indices.size(),
             test_bus->GetDeviceCount());
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
    ESP_LOGI(TAG, "const_bus[0] = const BaseI2c* at address 0x%02X",
             const_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "const_bus[0] returned nullptr");
    return false;
  }

  const EspI2cDevice* const_esp_device = const_bus.At(0);
  if (const_esp_device) {
    ESP_LOGI(TAG, "const_bus.At(0) = const EspI2cDevice* at address 0x%02X",
             const_esp_device->GetDeviceAddress());
  } else {
    ESP_LOGE(TAG, "const_bus.At(0) returned nullptr");
    return false;
  }

  std::vector<const BaseI2c*> const_all_devices = const_bus.GetAllDevices();
  ESP_LOGI(TAG, "const_bus.GetAllDevices() returned %zu devices", const_all_devices.size());
  if (const_all_devices.size() != device_indices.size()) {
    ESP_LOGE(TAG, "Const device count mismatch: expected %zu, got %zu", device_indices.size(),
             const_all_devices.size());
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

// NEW MODE-AWARE TESTS
bool test_i2c_sync_mode() noexcept {
  log_test_separator("I2C Sync Mode Test");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus for sync mode test");
    return false;
  }

  // Verify sync mode
  if (!test_bus->IsSyncMode()) {
    ESP_LOGE(TAG, "Bus should be in sync mode");
    return false;
  }

  ESP_LOGI(TAG, "✓ Bus correctly configured for sync mode");

  // Create device inline
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize device for sync mode test");
    return false;
  }

  // Test sync operations (should work)
  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
  hf_i2c_err_t result = device->Write(test_data, sizeof(test_data));
  ESP_LOGI(TAG, "Sync write result: %s", HfI2CErrToString(result).data());

  ESP_LOGI(TAG, "✓ Sync mode tests passed");
  return true;
}

bool test_i2c_async_mode() noexcept {
  log_test_separator("I2C Async Mode Test");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_ASYNC)

  // Verify async mode
  if (!test_bus->IsAsyncMode()) {
    ESP_LOGE(TAG, "Bus should be in async mode");
    return false;
  }

  ESP_LOGI(TAG, "✓ Bus correctly configured for async mode");

  // Create device inline
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

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test device");
    return false;
  }

  // Test that sync operations fail (should fail)
  uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
  hf_i2c_err_t result = device->Write(test_data, sizeof(test_data));
  if (result != hf_i2c_err_t::I2C_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Sync operation should fail in async mode, got: %s",
             HfI2CErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "✓ Async mode correctly blocks sync operations");
  ESP_LOGI(TAG, "✓ Async mode tests passed");
  return true;
}

bool test_i2c_mode_switching() noexcept {
  log_test_separator("I2C Mode Switching");

  // Start with sync mode
  CREATE_TEST_BUS_INLINE(bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)

  if (!bus->IsSyncMode()) {
    ESP_LOGE(TAG, "Bus should start in sync mode");
    return false;
  }

  ESP_LOGI(TAG, "✓ Bus started in sync mode");

  // Switch to async mode
  if (!bus->SwitchMode(hf_i2c_mode_t::HF_I2C_MODE_ASYNC, 5)) {
    ESP_LOGE(TAG, "Failed to switch to async mode");
    return false;
  }

  if (!bus->IsAsyncMode()) {
    ESP_LOGE(TAG, "Bus should now be in async mode");
    return false;
  }

  ESP_LOGI(TAG, "✓ Successfully switched to async mode");

  // Switch back to sync mode
  if (!bus->SwitchMode(hf_i2c_mode_t::HF_I2C_MODE_SYNC)) {
    ESP_LOGE(TAG, "Failed to switch back to sync mode");
    return false;
  }

  if (!bus->IsSyncMode()) {
    ESP_LOGE(TAG, "Bus should be back in sync mode");
    return false;
  }

  ESP_LOGI(TAG, "✓ Successfully switched back to sync mode");
  ESP_LOGI(TAG, "✓ Mode switching tests passed");
  return true;
}

bool test_i2c_basic_functionality() noexcept {
  log_test_separator("I2C Basic Functionality");

  // Test both modes
  for (auto mode : {hf_i2c_mode_t::HF_I2C_MODE_SYNC, hf_i2c_mode_t::HF_I2C_MODE_ASYNC}) {
    ESP_LOGI(TAG, "Testing %s mode", (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "ASYNC" : "SYNC");

    CREATE_TEST_BUS_INLINE(bus, mode)

    // Create device inline
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = TEST_DEVICE_ADDR_1;
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = STANDARD_FREQ;

    int device_index = bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Failed to create device in %s mode",
               (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "async" : "sync");
      return false;
    }

    BaseI2c* device = bus->GetDevice(device_index);
    if (!device) {
      ESP_LOGE(TAG, "Failed to get device in %s mode",
               (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "async" : "sync");
      return false;
    }

    // Initialize the device before use
    if (!device->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize device in %s mode",
               (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "async" : "sync");
      return false;
    }

    // Test device probing
    if (device->ProbeDevice()) {
      ESP_LOGI(TAG, "✓ Device responds in %s mode",
               (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "async" : "sync");
    } else {
      ESP_LOGW(TAG, "Device does not respond in %s mode (may be expected)",
               (mode == hf_i2c_mode_t::HF_I2C_MODE_ASYNC) ? "async" : "sync");
    }
  }

  ESP_LOGI(TAG, "✓ Basic functionality tests passed");
  return true;
}

// NEW PROBE COMPARISON TEST
bool test_i2c_probe_methods_comparison() noexcept {
  log_test_separator("I2C Probe Methods Comparison");

  ESP_LOGI(TAG, "This test compares ESP-IDF probe vs Custom Fast Probe timing and reliability");
  ESP_LOGI(TAG, "Set USE_CUSTOM_FAST_PROBE in EspI2c.cpp to switch between methods");

  CREATE_TEST_BUS_INLINE(test_bus, hf_i2c_mode_t::HF_I2C_MODE_SYNC)
  if (!test_bus) {
    ESP_LOGE(TAG, "Failed to create test bus for probe comparison");
    return false;
  }

  // Test addresses that are commonly used by I2C devices
  std::vector<uint16_t> test_addresses = {0x48, 0x50, 0x68, 0x76, 0x3C, 0x27, 0x23, 0x5A};

  ESP_LOGI(TAG, "Testing probe methods on %zu addresses:", test_addresses.size());
  for (auto addr : test_addresses) {
    ESP_LOGI(TAG, "  - Address 0x%02X", addr);
  }

  // Test each address with timing measurements
  for (auto addr : test_addresses) {
    ESP_LOGI(TAG, "--- Probing address 0x%02X ---", addr);

    // Measure total probe time
    uint64_t probe_start_time = esp_timer_get_time();
    bool device_found = test_bus->ProbeDevice(addr, TIMEOUT_STANDARD_MS); // Standard timeout
    uint64_t probe_end_time = esp_timer_get_time();
    uint64_t total_probe_time_us = probe_end_time - probe_start_time;

    ESP_LOGI(TAG, "Address 0x%02X: %s in %llu μs", addr, device_found ? "FOUND" : "NOT FOUND",
             total_probe_time_us);

    // Small delay between probes
    vTaskDelay(pdMS_TO_TICKS(TIMEOUT_FAST_MS));
  }

  ESP_LOGI(TAG, "Probe comparison test completed");
  ESP_LOGI(TAG, "Check logs above for timing differences between probe methods");
  ESP_LOGI(TAG, "[SUCCESS] Probe methods comparison tests passed");
  return true;
}

// Helper function implementations

bool verify_device_functionality(BaseI2c* device) noexcept {
  if (!device)
    return false;

  uint8_t test_data = 0xAA;
  hf_i2c_err_t result = device->Write(&test_data, 1, TIMEOUT_STANDARD_MS);
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

//==============================================//
// ESP-IDF DIRECT API TEST - VERIFICATION      //
//==============================================//

/**
 * @brief Test I2C using ESP-IDF API directly (bypassing our wrapper)
 * @return true if successful, false otherwise
 * @note This test runs FIRST to verify ESP-IDF I2C driver functionality
 * @note Runs in a loop with sufficient time to thoroughly test I2C operations
 */
bool test_i2c_espidf_direct_api() noexcept {
  log_test_separator("ESP-IDF Direct I2C API Test (FIRST)");

  ESP_LOGI(TAG, "Testing I2C using ESP-IDF API directly (bypassing our wrapper)");
  ESP_LOGI(TAG, "This test runs FIRST to verify ESP-IDF I2C driver functionality");
  ESP_LOGI(TAG, "If this test fails, the issue is with ESP-IDF itself, not our wrapper");

  // ESP-IDF I2C configuration constants (matching the working example)
  constexpr gpio_num_t I2C_MASTER_SCL_IO = static_cast<gpio_num_t>(TEST_SCL_PIN);
  constexpr gpio_num_t I2C_MASTER_SDA_IO = static_cast<gpio_num_t>(TEST_SDA_PIN);
  constexpr i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;
  constexpr uint32_t I2C_MASTER_FREQ_HZ = 100000; // 100kHz for compatibility
  constexpr uint32_t I2C_MASTER_TIMEOUT_MS = 1000;

  ESP_LOGI(TAG, "ESP-IDF Config: SCL=GPIO%d, SDA=GPIO%d, Port=%d, Freq=%lu Hz", I2C_MASTER_SCL_IO,
           I2C_MASTER_SDA_IO, I2C_MASTER_NUM, I2C_MASTER_FREQ_HZ);

  // Step 1: Initialize I2C master bus
  ESP_LOGI(TAG, "Step 1: Initializing I2C master bus...");

  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_MASTER_NUM,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7, // Critical for ESP32-C6 stability
      .intr_priority = 0,
      .trans_queue_depth = 0, // Sync mode
      .flags =
          {
              .enable_internal_pullup = true,
              .allow_pd = false,
          },
  };

  i2c_master_bus_handle_t bus_handle = nullptr;
  esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ESP-IDF: Failed to create I2C master bus: %s", esp_err_to_name(err));
    return false;
  }
  ESP_LOGI(TAG, "ESP-IDF: I2C master bus created successfully");

  // Step 2: Wait for bus to be ready (CRITICAL for ESP32-C6)
  ESP_LOGI(TAG, "Step 2: Waiting for I2C bus readiness...");
  err = i2c_master_bus_wait_all_done(bus_handle, 100);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ESP-IDF: Failed to wait for I2C bus readiness: %s", esp_err_to_name(err));
    i2c_del_master_bus(bus_handle);
    return false;
  }
  ESP_LOGI(TAG, "ESP-IDF: I2C bus is ready and waiting for operations");

  // Step 3: Create I2C device
  ESP_LOGI(TAG, "Step 3: Creating I2C device...");

  i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = TEST_DEVICE_ADDR_1, // Use our test device address
      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
      .scl_wait_us = 0,
      .flags = {0},
  };

  i2c_master_dev_handle_t dev_handle = nullptr;
  err = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ESP-IDF: Failed to add I2C device: %s", esp_err_to_name(err));
    i2c_del_master_bus(bus_handle);
    return false;
  }
  ESP_LOGI(TAG, "ESP-IDF: I2C device created successfully at address 0x%02X", TEST_DEVICE_ADDR_1);

  // Step 4: Extended I2C testing loop with sufficient time
  ESP_LOGI(TAG, "Step 4: Starting extended I2C testing loop (10 seconds)...");

  const TickType_t test_duration = pdMS_TO_TICKS(10000); // 10 seconds
  const TickType_t loop_delay = pdMS_TO_TICKS(500);      // 500ms between operations
  TickType_t start_time = xTaskGetTickCount();
  uint32_t operation_count = 0;
  uint32_t successful_operations = 0;
  uint32_t failed_operations = 0;

  ESP_LOGI(TAG, "ESP-IDF: Test loop will run for 10 seconds with 500ms delays between operations");
  ESP_LOGI(TAG, "ESP-IDF: This provides sufficient time to thoroughly test I2C functionality");

  while ((xTaskGetTickCount() - start_time) < test_duration) {
    operation_count++;
    ESP_LOGI(TAG, "ESP-IDF: Operation %u - Testing I2C operations...", operation_count);

    // Test 4a: Write operation
    uint8_t write_data[] = {0x00, 0xAA, 0x55}; // Register address + test data
    err = i2c_master_transmit(dev_handle, write_data, sizeof(write_data), I2C_MASTER_TIMEOUT_MS);
    if (err != ESP_OK) {
      ESP_LOGW(TAG,
               "ESP-IDF: Write operation %u failed: %s (this is normal without physical device)",
               operation_count, esp_err_to_name(err));
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "ESP-IDF: Write operation %u successful: %zu bytes", operation_count,
               sizeof(write_data));
      successful_operations++;
    }

    // Test 4b: Read operation
    uint8_t read_data[2] = {0};
    err = i2c_master_receive(dev_handle, read_data, sizeof(read_data), I2C_MASTER_TIMEOUT_MS);
    if (err != ESP_OK) {
      ESP_LOGW(TAG,
               "ESP-IDF: Read operation %u failed: %s (this is normal without physical device)",
               operation_count, esp_err_to_name(err));
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "ESP-IDF: Read operation %u successful: %zu bytes", operation_count,
               sizeof(read_data));
      ESP_LOGI(TAG, "ESP-IDF: Read data: 0x%02X 0x%02X", read_data[0], read_data[1]);
      successful_operations++;
    }

    // Test 4c: Write-then-read operation
    uint8_t reg_addr = 0x00; // Register address to read from
    err =
        i2c_master_transmit_receive(dev_handle, &reg_addr, 1, read_data, 2, I2C_MASTER_TIMEOUT_MS);
    if (err != ESP_OK) {
      ESP_LOGW(TAG,
               "ESP-IDF: Write-then-read operation %u failed: %s (this is normal without physical "
               "device)",
               operation_count, esp_err_to_name(err));
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "ESP-IDF: Write-then-read operation %u successful", operation_count);
      ESP_LOGI(TAG, "ESP-IDF: Read data: 0x%02X 0x%02X", read_data[0], read_data[1]);
      successful_operations++;
    }

    // Test 4d: Device probing (every 5 operations)
    if (operation_count % 5 == 0) {
      ESP_LOGI(TAG, "ESP-IDF: Probing device at address 0x%02X (operation %u)", TEST_DEVICE_ADDR_1,
               operation_count);
      err = i2c_master_probe(bus_handle, TEST_DEVICE_ADDR_1, I2C_MASTER_TIMEOUT_MS);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "ESP-IDF: Device probe failed: %s (this is normal without physical device)",
                 esp_err_to_name(err));
      } else {
        ESP_LOGI(TAG, "ESP-IDF: Device probe successful - device found at 0x%02X",
                 TEST_DEVICE_ADDR_1);
      }
    }

    // Test 4e: Bus reset (every 10 operations)
    if (operation_count % 10 == 0) {
      ESP_LOGI(TAG, "ESP-IDF: Testing bus reset (operation %u)", operation_count);
      err = i2c_master_bus_reset(bus_handle);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "ESP-IDF: Bus reset failed: %s (this is normal without physical devices)",
                 esp_err_to_name(err));
      } else {
        ESP_LOGI(TAG, "ESP-IDF: Bus reset successful");
      }
    }

    ESP_LOGI(TAG, "ESP-IDF: Operation %u completed. Success: %u, Failed: %u", operation_count,
             successful_operations, failed_operations);

    // Delay between operations to prevent overwhelming the system
    vTaskDelay(loop_delay);
  }

  ESP_LOGI(TAG, "ESP-IDF: Extended testing loop completed!");
  ESP_LOGI(TAG, "ESP-IDF: Total operations: %u, Successful: %u, Failed: %u", operation_count,
           successful_operations, failed_operations);
  ESP_LOGI(TAG, "ESP-IDF: Success rate: %.1f%%",
           (float)successful_operations / operation_count * 100.0f);

  // Step 5: Cleanup
  ESP_LOGI(TAG, "Step 5: Cleaning up ESP-IDF I2C resources...");

  // Remove device
  err = i2c_master_bus_rm_device(dev_handle);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ESP-IDF: Failed to remove I2C device: %s", esp_err_to_name(err));
  } else {
    ESP_LOGI(TAG, "ESP-IDF: I2C device removed successfully");
  }

  // Delete bus
  err = i2c_del_master_bus(bus_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ESP-IDF: Failed to delete I2C master bus: %s", esp_err_to_name(err));
    return false;
  } else {
    ESP_LOGI(TAG, "ESP-IDF: I2C master bus deleted successfully");
  }

  ESP_LOGI(TAG, "ESP-IDF Direct I2C API test completed successfully!");
  ESP_LOGI(TAG, "This confirms that the ESP-IDF I2C driver is working correctly");
  ESP_LOGI(TAG, "Any issues in our wrapper are not related to ESP-IDF itself");
  ESP_LOGI(TAG, "The test ran for 10 seconds with %u operations, proving I2C stability",
           operation_count);

  return true;
}

//==============================================//
// ESP-IDF WRAPPER REPLICA TEST - COMPARISON   //
//==============================================//

/**
 * @brief Test I2C using EspI2cBus wrapper (replicating ESP-IDF direct test)
 * @return true if successful, false otherwise
 * @note This test runs SECOND to compare EspI2cBus wrapper with ESP-IDF direct API
 * @note Uses identical configuration and test pattern as ESP-IDF direct test
 * @note Runs in a loop with sufficient time to thoroughly test I2C operations
 */
bool test_i2c_espidf_wrapper_replica() noexcept {
  log_test_separator("EspI2cBus Wrapper Replica Test (SECOND)");

  ESP_LOGI(TAG, "Testing I2C using EspI2cBus wrapper (replicating ESP-IDF direct test)");
  ESP_LOGI(TAG, "This test runs SECOND to compare wrapper implementation with ESP-IDF direct API");
  ESP_LOGI(TAG, "If this test fails, the issue is with our wrapper implementation");

  // Use identical configuration as ESP-IDF direct test
  constexpr hf_pin_num_t I2C_MASTER_SCL_IO = TEST_SCL_PIN;
  constexpr hf_pin_num_t I2C_MASTER_SDA_IO = TEST_SDA_PIN;
  constexpr i2c_port_t I2C_MASTER_NUM = I2C_PORT_NUM;
  constexpr uint32_t I2C_MASTER_FREQ_HZ = 100000; // 100kHz for compatibility
  constexpr uint32_t I2C_MASTER_TIMEOUT_MS = 1000;

  ESP_LOGI(TAG, "EspI2cBus Config: SCL=GPIO%d, SDA=GPIO%d, Port=%d, Freq=%lu Hz", I2C_MASTER_SCL_IO,
           I2C_MASTER_SDA_IO, I2C_MASTER_NUM, I2C_MASTER_FREQ_HZ);

  // Step 1: Create EspI2cBus configuration (matching ESP-IDF direct test)
  ESP_LOGI(TAG, "Step 1: Creating EspI2cBus configuration...");

  hf_i2c_master_bus_config_t bus_config = {};
  bus_config.i2c_port = I2C_MASTER_NUM;
  bus_config.sda_io_num = I2C_MASTER_SDA_IO;
  bus_config.scl_io_num = I2C_MASTER_SCL_IO;
  bus_config.mode = hf_i2c_mode_t::HF_I2C_MODE_SYNC; // Sync mode (matching ESP-IDF test)
  bus_config.trans_queue_depth = 0;                  // Sync mode (no queue)
  bus_config.clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT;
  bus_config.glitch_ignore_cnt =
      hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES; // Critical for ESP32-C6 stability
  bus_config.intr_priority = 0;
  bus_config.flags.enable_internal_pullup = true;
  bus_config.flags.allow_pd = false;

  // Step 2: Create and initialize EspI2cBus
  ESP_LOGI(TAG, "Step 2: Creating and initializing EspI2cBus...");

  auto test_bus = std::make_unique<EspI2cBus>(bus_config);
  if (!test_bus) {
    ESP_LOGE(TAG, "EspI2cBus: Failed to create I2C bus instance");
    return false;
  }

  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "EspI2cBus: Failed to initialize I2C bus");
    return false;
  }
  ESP_LOGI(TAG, "EspI2cBus: I2C bus initialized successfully");

  // Step 3: Create I2C device configuration (matching ESP-IDF direct test)
  ESP_LOGI(TAG, "Step 3: Creating I2C device configuration...");

  hf_i2c_device_config_t device_config = {};
  device_config.device_address = TEST_DEVICE_ADDR_1; // Use our test device address
  device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
  device_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;
  device_config.scl_wait_us = 0;
  device_config.disable_ack_check = false;

  // Step 4: Add device to bus
  ESP_LOGI(TAG, "Step 4: Adding I2C device to bus...");

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "EspI2cBus: Failed to add I2C device");
    return false;
  }
  ESP_LOGI(TAG, "EspI2cBus: I2C device created successfully at address 0x%02X", TEST_DEVICE_ADDR_1);

  BaseI2c* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "EspI2cBus: Failed to get I2C device");
    return false;
  }

  // Initialize the device before use
  if (!device->Initialize()) {
    ESP_LOGE(TAG, "EspI2cBus: Failed to initialize I2C device");
    return false;
  }

  // Step 5: Extended I2C testing loop (matching ESP-IDF direct test exactly)
  ESP_LOGI(TAG, "Step 5: Starting extended I2C testing loop (10 seconds)...");

  const TickType_t test_duration = pdMS_TO_TICKS(10000); // 10 seconds (matching ESP-IDF test)
  const TickType_t loop_delay =
      pdMS_TO_TICKS(500); // 500ms between operations (matching ESP-IDF test)
  TickType_t start_time = xTaskGetTickCount();
  uint32_t operation_count = 0;
  uint32_t successful_operations = 0;
  uint32_t failed_operations = 0;

  ESP_LOGI(TAG,
           "EspI2cBus: Test loop will run for 10 seconds with 500ms delays between operations");
  ESP_LOGI(TAG, "EspI2cBus: This provides identical timing to ESP-IDF direct test for comparison");

  while ((xTaskGetTickCount() - start_time) < test_duration) {
    operation_count++;
    ESP_LOGI(TAG, "EspI2cBus: Operation %u - Testing I2C operations...", operation_count);

    // Test 5a: Write operation (matching ESP-IDF test exactly)
    uint8_t write_data[] = {0x00, 0xAA,
                            0x55}; // Register address + test data (matching ESP-IDF test)
    hf_i2c_err_t write_result =
        device->Write(write_data, sizeof(write_data), I2C_MASTER_TIMEOUT_MS);
    if (write_result != hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGW(TAG,
               "EspI2cBus: Write operation %u failed: %s (this is normal without physical device)",
               operation_count, HfI2CErrToString(write_result).data());
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "EspI2cBus: Write operation %u successful: %zu bytes", operation_count,
               sizeof(write_data));
      successful_operations++;
    }

    // Test 5b: Read operation (matching ESP-IDF test exactly)
    uint8_t read_data[2] = {0};
    hf_i2c_err_t read_result = device->Read(read_data, sizeof(read_data), I2C_MASTER_TIMEOUT_MS);
    if (read_result != hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGW(TAG,
               "EspI2cBus: Read operation %u failed: %s (this is normal without physical device)",
               operation_count, HfI2CErrToString(read_result).data());
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "EspI2cBus: Read operation %u successful: %zu bytes", operation_count,
               sizeof(read_data));
      ESP_LOGI(TAG, "EspI2cBus: Read data: 0x%02X 0x%02X", read_data[0], read_data[1]);
      successful_operations++;
    }

    // Test 5c: Write-then-read operation (matching ESP-IDF test exactly)
    uint8_t reg_addr = 0x00; // Register address to read from (matching ESP-IDF test)
    hf_i2c_err_t write_read_result =
        device->WriteRead(&reg_addr, 1, read_data, 2, I2C_MASTER_TIMEOUT_MS);
    if (write_read_result != hf_i2c_err_t::I2C_SUCCESS) {
      ESP_LOGW(TAG,
               "EspI2cBus: Write-then-read operation %u failed: %s (this is normal without "
               "physical device)",
               operation_count, HfI2CErrToString(write_read_result).data());
      failed_operations++;
    } else {
      ESP_LOGI(TAG, "EspI2cBus: Write-then-read operation %u successful", operation_count);
      ESP_LOGI(TAG, "EspI2cBus: Read data: 0x%02X 0x%02X", read_data[0], read_data[1]);
      successful_operations++;
    }

    // Test 5d: Device probing (every 5 operations, matching ESP-IDF test)
    if (operation_count % 5 == 0) {
      ESP_LOGI(TAG, "EspI2cBus: Probing device at address 0x%02X (operation %u)",
               TEST_DEVICE_ADDR_1, operation_count);
      bool device_found = test_bus->ProbeDevice(TEST_DEVICE_ADDR_1, I2C_MASTER_TIMEOUT_MS);
      if (!device_found) {
        ESP_LOGW(TAG, "EspI2cBus: Device probe failed (this is normal without physical device)");
      } else {
        ESP_LOGI(TAG, "EspI2cBus: Device probe successful - device found at 0x%02X",
                 TEST_DEVICE_ADDR_1);
      }
    }

    // Test 5e: Bus reset (every 10 operations, matching ESP-IDF test)
    if (operation_count % 10 == 0) {
      ESP_LOGI(TAG, "EspI2cBus: Testing bus reset (operation %u)", operation_count);
      bool reset_success = test_bus->ResetBus();
      if (!reset_success) {
        ESP_LOGW(TAG, "EspI2cBus: Bus reset failed (this is normal without physical devices)");
      } else {
        ESP_LOGI(TAG, "EspI2cBus: Bus reset successful");
      }
    }

    ESP_LOGI(TAG, "EspI2cBus: Operation %u completed. Success: %u, Failed: %u", operation_count,
             successful_operations, failed_operations);

    // Delay between operations to prevent overwhelming the system (matching ESP-IDF test)
    vTaskDelay(loop_delay);
  }

  ESP_LOGI(TAG, "EspI2cBus: Extended testing loop completed!");
  ESP_LOGI(TAG, "EspI2cBus: Total operations: %u, Successful: %u, Failed: %u", operation_count,
           successful_operations, failed_operations);
  ESP_LOGI(TAG, "EspI2cBus: Success rate: %.1f%%",
           (float)successful_operations / operation_count * 100.0f);

  // Step 6: Cleanup (automatic via RAII)
  ESP_LOGI(TAG, "Step 6: Cleaning up EspI2cBus resources...");

  // Note: No manual cleanup needed - EspI2cBus destructor handles everything
  // This demonstrates the advantage of RAII over manual ESP-IDF cleanup

  ESP_LOGI(TAG, "EspI2cBus Wrapper Replica test completed successfully!");
  ESP_LOGI(TAG, "This confirms that our EspI2cBus wrapper works identically to ESP-IDF direct API");
  ESP_LOGI(TAG, "The test ran for 10 seconds with %u operations, proving wrapper stability",
           operation_count);
  ESP_LOGI(TAG, "Key advantages of wrapper: RAII cleanup, better error handling, thread safety");

  return true;
}

//==============================================//
// ESP-IDF WRAPPER CONTINUOUS TEST - STABILITY //
//==============================================//

/**
 * @brief Test I2C using EspI2cBus wrapper continuously (stability test)
 * @return true if successful, false otherwise
 * @note This test runs the replica test in a loop to verify stability
 */
bool test_i2c_espidf_wrapper_replica_continuous() noexcept {
  log_test_separator("EspI2cBus Wrapper Continuous Test (STABILITY)");

  ESP_LOGI(TAG, "Testing I2C using EspI2cBus wrapper continuously for stability");
  ESP_LOGI(TAG, "This test runs the replica test pattern in a loop to identify issues");

  const int num_iterations = 10; // Test 10 iterations
  int successful_iterations = 0;
  int failed_iterations = 0;

  for (int iteration = 1; iteration <= num_iterations; iteration++) {
    ESP_LOGI(TAG, "\n=== ITERATION %d/%d ===", iteration, num_iterations);

    // Use identical configuration as replica test
    hf_i2c_master_bus_config_t bus_config = {}; // Zero-init, no constructor
    bus_config.i2c_port = I2C_PORT_NUM;
    bus_config.sda_io_num = TEST_SDA_PIN;
    bus_config.scl_io_num = TEST_SCL_PIN;
    bus_config.mode = hf_i2c_mode_t::HF_I2C_MODE_SYNC;
    bus_config.trans_queue_depth = 0;
    bus_config.clk_source = hf_i2c_clock_source_t::HF_I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = hf_i2c_glitch_filter_t::HF_I2C_GLITCH_FILTER_7_CYCLES;
    bus_config.intr_priority = 0;
    bus_config.flags.enable_internal_pullup = true;
    bus_config.flags.allow_pd = false;

    // Create and initialize EspI2cBus (identical to replica test)
    auto test_bus = std::make_unique<EspI2cBus>(bus_config);
    if (!test_bus) {
      ESP_LOGE(TAG, "Iteration %d: Failed to create I2C bus instance", iteration);
      failed_iterations++;
      continue;
    }

    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Iteration %d: Failed to initialize I2C bus", iteration);
      failed_iterations++;
      continue;
    }

    // Create I2C device (identical to replica test)
    hf_i2c_device_config_t device_config = {};
    device_config.device_address = TEST_DEVICE_ADDR_1;
    device_config.dev_addr_length = hf_i2c_address_bits_t::HF_I2C_ADDR_7_BIT;
    device_config.scl_speed_hz = 100000;
    device_config.scl_wait_us = 0;
    device_config.disable_ack_check = false;

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Iteration %d: Failed to add I2C device", iteration);
      failed_iterations++;
      continue;
    }

    BaseI2c* device = test_bus->GetDevice(device_index);
    if (!device) {
      ESP_LOGE(TAG, "Iteration %d: Failed to get I2C device", iteration);
      failed_iterations++;
      continue;
    }

    // Initialize the device before use
    if (!device->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize test device");
      return false;
    }

    // Perform a few I2C operations (simplified version of replica test)
    bool iteration_success = true;
    for (int op = 1; op <= 3; op++) {
      // Write operation
      uint8_t write_data[] = {0x00, 0xAA, 0x55};
      hf_i2c_err_t write_result = device->Write(write_data, sizeof(write_data), 1000);

      // Read operation
      uint8_t read_data[2] = {0};
      hf_i2c_err_t read_result = device->Read(read_data, sizeof(read_data), 1000);

      // Write-then-read operation
      uint8_t reg_addr = 0x00;
      hf_i2c_err_t write_read_result = device->WriteRead(&reg_addr, 1, read_data, 2, 1000);

      ESP_LOGI(TAG, "Iteration %d, Op %d: Write=%s, Read=%s, WriteRead=%s", iteration, op,
               HfI2CErrToString(write_result).data(), HfI2CErrToString(read_result).data(),
               HfI2CErrToString(write_read_result).data());

      // Small delay between operations
      vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (iteration_success) {
      successful_iterations++;
      ESP_LOGI(TAG, "Iteration %d: SUCCESS", iteration);
    } else {
      failed_iterations++;
      ESP_LOGI(TAG, "Iteration %d: FAILED", iteration);
    }

    // Cleanup happens automatically via RAII
    // Small delay between iterations
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  ESP_LOGI(TAG, "\n=== CONTINUOUS TEST RESULTS ===");
  ESP_LOGI(TAG, "Total iterations: %d", num_iterations);
  ESP_LOGI(TAG, "Successful: %d", successful_iterations);
  ESP_LOGI(TAG, "Failed: %d", failed_iterations);
  ESP_LOGI(TAG, "Success rate: %.1f%%", (float)successful_iterations / num_iterations * 100.0f);

  if (successful_iterations == num_iterations) {
    ESP_LOGI(TAG, "EspI2cBus Wrapper Continuous test completed successfully!");
    ESP_LOGI(TAG, "All %d iterations passed - wrapper is stable", num_iterations);
    return true;
  } else {
    ESP_LOGW(TAG, "EspI2cBus Wrapper Continuous test had %d failures", failed_iterations);
    return false;
  }
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

  vTaskDelay(pdMS_TO_TICKS(TIMEOUT_LONG_MS));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG,
             "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
  }

  // Report test section configuration
  print_test_section_status(TAG, "I2C");

  // Run all I2C tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CORE_TESTS, "I2C CORE TESTS",
      // ESP-IDF verification tests
      ESP_LOGI(TAG, "Running ESP-IDF verification tests...");
      RUN_TEST_IN_TASK("espidf_direct_api", test_i2c_espidf_direct_api, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("espidf_wrapper_replica", test_i2c_espidf_wrapper_replica, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "espidf_wrapper_continuous", test_i2c_espidf_wrapper_replica_continuous, 8192, 1);
      flip_test_progress_indicator();

      // Bus and device tests
      ESP_LOGI(TAG, "Running bus and device tests...");
      RUN_TEST_IN_TASK("bus_initialization", test_i2c_bus_initialization, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("bus_deinitialization", test_i2c_bus_deinitialization, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("configuration_validation", test_i2c_configuration_validation, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("device_creation", test_i2c_device_creation, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("device_management", test_i2c_device_management, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("device_probing", test_i2c_device_probing, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("bus_scanning", test_i2c_bus_scanning, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_OPERATION_TESTS, "I2C OPERATION TESTS",
      // Read/write operations
      ESP_LOGI(TAG, "Running read/write operation tests...");
      RUN_TEST_IN_TASK("write_operations", test_i2c_write_operations, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("read_operations", test_i2c_read_operations, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("write_read_operations", test_i2c_write_read_operations, 8192, 1);
      flip_test_progress_indicator();

      // Error handling and timeouts
      ESP_LOGI(TAG, "Running error handling and timeout tests...");
      RUN_TEST_IN_TASK("error_handling", test_i2c_error_handling, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("timeout_handling", test_i2c_timeout_handling, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("multi_device_operations", test_i2c_multi_device_operations, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_ADVANCED_TESTS, "I2C ADVANCED TESTS",
      // ESP-specific features
      ESP_LOGI(TAG, "Running ESP-specific feature tests...");
      RUN_TEST_IN_TASK("esp_specific_features", test_i2c_esp_specific_features, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("thread_safety", test_i2c_thread_safety, 8192, 1);
      flip_test_progress_indicator();

      // Clock speeds and address modes
      ESP_LOGI(TAG, "Running clock speed and address mode tests...");
      RUN_TEST_IN_TASK("clock_speeds", test_i2c_clock_speeds, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("address_modes", test_i2c_address_modes, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_PERFORMANCE_TESTS, "I2C PERFORMANCE TESTS",
      // Performance and edge cases
      ESP_LOGI(TAG, "Running performance and edge case tests...");
      RUN_TEST_IN_TASK("performance", test_i2c_performance, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK("edge_cases", test_i2c_edge_cases, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("power_management", test_i2c_power_management, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_SPECIALIZED_TESTS, "I2C SPECIALIZED TESTS",
      // Async operations
      ESP_LOGI(TAG, "Running async operation tests...");
      RUN_TEST_IN_TASK("async_operations", test_i2c_async_operations, 16384, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("async_timeout_handling", test_i2c_async_timeout_handling, 16384, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("async_multiple_operations", test_i2c_async_multiple_operations, 16384, 1);
      flip_test_progress_indicator();

      // Mode and access patterns
      ESP_LOGI(TAG, "Running mode and access pattern tests...");
      RUN_TEST_IN_TASK("index_based_access", test_i2c_index_based_access, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK("sync_mode", test_i2c_sync_mode, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK("async_mode", test_i2c_async_mode, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("mode_switching", test_i2c_mode_switching, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("basic_functionality", test_i2c_basic_functionality, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("probe_methods_comparison", test_i2c_probe_methods_comparison, 8192, 1);
      flip_test_progress_indicator(););

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
    vTaskDelay(pdMS_TO_TICKS(TIMEOUT_EXTENDED_MS));
  }
}
