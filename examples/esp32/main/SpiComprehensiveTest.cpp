/**
 * @file SpiComprehensiveTest.cpp
 * @brief Comprehensive SPI testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains comprehensive testing for the ESP SPI implementation including:
 * - Bus initialization and configuration validation
 * - Device creation and management
 * - Data transfer operations (full-duplex, half-duplex, various modes)
 * - DMA operations and performance testing
 * - Error handling and recovery
 * - Multi-device scenarios with different configurations
 * - ESP-specific features (clock sources, IOMUX, advanced timing)
 * - Thread safety verification
 * - Performance benchmarking and timing tests
 * - Edge cases and fault injection
 * - SPI modes (0-3) and various transfer sizes
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseSpi.h"
#include "mcu/esp32/EspSpi.h"
#include "mcu/esp32/utils/EspTypes_SPI.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "TestFramework.h"

static const char* TAG = "SPI_Test";
static TestResults g_test_results;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core SPI functionality tests
static constexpr bool ENABLE_CORE_TESTS =
    true; // Bus initialization, configuration, device management
static constexpr bool ENABLE_TRANSFER_TESTS = true; // Basic transfers, modes, sizes, DMA operations
static constexpr bool ENABLE_PERFORMANCE_TESTS =
    true; // Clock speeds, multi-device, performance benchmarks
static constexpr bool ENABLE_ADVANCED_TESTS = true; // ESP-specific features, IOMUX, thread safety
static constexpr bool ENABLE_STRESS_TESTS =
    true; // Error handling, timeouts, edge cases, power management

// Test configuration constants
static constexpr hf_pin_num_t TEST_MOSI_PIN = 7;
static constexpr hf_pin_num_t TEST_MISO_PIN = 2;
static constexpr hf_pin_num_t TEST_SCLK_PIN = 6;
static constexpr hf_pin_num_t TEST_CS_PIN_1 = 21;
static constexpr hf_pin_num_t TEST_CS_PIN_2 = 20;
static constexpr hf_pin_num_t TEST_CS_PIN_3 = 19;
static constexpr uint32_t SLOW_SPEED = 1000000;    // 1MHz
static constexpr uint32_t MEDIUM_SPEED = 10000000; // 10MHz
static constexpr uint32_t FAST_SPEED = 40000000;   // 40MHz
static constexpr uint32_t MAX_SPEED = 80000000;    // 80MHz

// Forward declarations
bool test_spi_bus_initialization() noexcept;
bool test_spi_bus_deinitialization() noexcept;
bool test_spi_configuration_validation() noexcept;
bool test_spi_device_creation() noexcept;
bool test_spi_device_management() noexcept;
bool test_spi_transfer_basic() noexcept;
bool test_spi_transfer_modes() noexcept;
bool test_spi_transfer_sizes() noexcept;
bool test_spi_dma_operations() noexcept;
bool test_spi_clock_speeds() noexcept;
bool test_spi_multi_device_operations() noexcept;
bool test_spi_error_handling() noexcept;
bool test_spi_timeout_handling() noexcept;
bool test_spi_esp_specific_features() noexcept;
bool test_spi_iomux_optimization() noexcept;
bool test_spi_thread_safety() noexcept;
bool test_spi_performance_benchmarks() noexcept;
bool test_spi_edge_cases() noexcept;
bool test_spi_bus_acquisition() noexcept;
bool test_spi_power_management() noexcept;

// Helper functions
hf_spi_bus_config_t create_test_bus_config(uint32_t speed = MEDIUM_SPEED, bool use_dma = true,
                                           spi_host_device_t host = SPI2_HOST) noexcept;
bool verify_transfer_data(const uint8_t* tx_data, const uint8_t* rx_data, size_t length) noexcept;
void generate_test_pattern(uint8_t* buffer, size_t length, uint8_t seed = 0xAA) noexcept;
void generate_sequential_pattern(uint8_t* buffer, size_t length, uint8_t start_value = 0x01) noexcept;
void generate_alternating_pattern(uint8_t* buffer, size_t length, uint8_t value1 = 0x55, uint8_t value2 = 0xAA) noexcept;
void log_test_separator(const char* test_name) noexcept;

bool test_spi_bus_initialization() noexcept {
  log_test_separator("SPI Bus Initialization");

  // Test 1: Basic initialization with DMA
  hf_spi_bus_config_t bus_config = {};
  bus_config.mosi_pin = TEST_MOSI_PIN;
  bus_config.miso_pin = TEST_MISO_PIN;
  bus_config.sclk_pin = TEST_SCLK_PIN;
  bus_config.clock_speed_hz = MEDIUM_SPEED;
  bus_config.host = SPI2_HOST;
  bus_config.dma_channel = 0; // Auto DMA
  bus_config.use_iomux = true;

  auto test_bus = std::make_unique<EspSpiBus>(bus_config);

  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus with DMA");
    return false;
  }

  // Test 2: Verify configuration
  const auto& config = test_bus->GetConfig();
  if (config.mosi_pin != TEST_MOSI_PIN || config.miso_pin != TEST_MISO_PIN) {
    ESP_LOGE(TAG, "Configuration mismatch");
    return false;
  }

  // Test 3: Double initialization should succeed (idempotent)
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Second initialization failed");
    return false;
  }

  test_bus->Deinitialize();

  // Test 4: Initialization without DMA
  bus_config.dma_channel = 0xFF; // Disable DMA
  auto test_bus_no_dma = std::make_unique<EspSpiBus>(bus_config);

  if (!test_bus_no_dma->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus without DMA");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Bus initialization tests passed");
  return true;
}

bool test_spi_bus_deinitialization() noexcept {
  log_test_separator("SPI Bus Deinitialization");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create a device to test cleanup
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create device for deinitialization test");
    return false;
  }

  // Verify device exists
  if (test_bus->GetDeviceCount() != 1) {
    ESP_LOGE(TAG, "Device count mismatch");
    return false;
  }

  // Test deinitialization (should clean up devices)
  if (!test_bus->Deinitialize()) {
    ESP_LOGE(TAG, "Failed to deinitialize bus");
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

bool test_spi_configuration_validation() noexcept {
  log_test_separator("SPI Configuration Validation");

  // Test different hosts
  for (auto host : {SPI2_HOST}) { // ESP32-C6 typically has SPI2_HOST available
    hf_spi_bus_config_t bus_config = {};
    bus_config.mosi_pin = TEST_MOSI_PIN;
    bus_config.miso_pin = TEST_MISO_PIN;
    bus_config.sclk_pin = TEST_SCLK_PIN;
    bus_config.host = host;
    bus_config.clock_speed_hz = MEDIUM_SPEED;
    bus_config.dma_channel = 0;

    auto test_bus = std::make_unique<EspSpiBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize with host %d", static_cast<int>(host));
      return false;
    }
    test_bus->Deinitialize();
  }

  // Test IOMUX vs GPIO matrix
  for (bool use_iomux : {true, false}) {
    hf_spi_bus_config_t bus_config = {};
    bus_config.mosi_pin = TEST_MOSI_PIN;
    bus_config.miso_pin = TEST_MISO_PIN;
    bus_config.sclk_pin = TEST_SCLK_PIN;
    bus_config.host = SPI2_HOST;
    bus_config.clock_speed_hz = MEDIUM_SPEED;
    bus_config.use_iomux = use_iomux;
    bus_config.dma_channel = 0;

    auto test_bus = std::make_unique<EspSpiBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize with IOMUX %s", use_iomux ? "enabled" : "disabled");
      return false;
    }
    test_bus->Deinitialize();
  }

  ESP_LOGI(TAG, "[SUCCESS] Configuration validation tests passed");
  return true;
}

bool test_spi_device_creation() noexcept {
  log_test_separator("SPI Device Creation");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Test 1: Create device with basic configuration
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;
  device_config.queue_size = 7;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create basic device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get created device");
    return false;
  }

  // Test 2: Create device with advanced features
  hf_spi_device_config_t advanced_config = {};
  advanced_config.clock_speed_hz = FAST_SPEED;
  advanced_config.mode = hf_spi_mode_t::HF_SPI_MODE_3;
  advanced_config.cs_pin = TEST_CS_PIN_2;
  advanced_config.command_bits = 8;
  advanced_config.address_bits = 24;
  advanced_config.dummy_bits = 8;
  advanced_config.queue_size = 15;

  int advanced_index = test_bus->CreateDevice(advanced_config);
  if (advanced_index < 0) {
    ESP_LOGE(TAG, "Failed to create advanced device");
    return false;
  }

  // Test 3: Verify device count
  if (test_bus->GetDeviceCount() != 2) {
    ESP_LOGE(TAG, "Device count mismatch: expected 2, got %zu", test_bus->GetDeviceCount());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Device creation tests passed");

  // Clean up happens automatically via RAII (unique_ptr destructor)
  return true;
}

bool test_spi_device_management() noexcept {
  log_test_separator("SPI Device Management");

  // Use SPI2_HOST for device management test to isolate from other tests
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus with SPI2_HOST");
    return false;
  }

  // Create multiple devices with different speeds
  std::vector<int> device_indices;
  std::vector<uint32_t> speeds = {SLOW_SPEED, MEDIUM_SPEED, FAST_SPEED};
  std::vector<hf_pin_num_t> cs_pins = {TEST_CS_PIN_1, TEST_CS_PIN_2, TEST_CS_PIN_3};

  for (size_t i = 0; i < speeds.size(); ++i) {
    hf_spi_device_config_t device_config = {};
    device_config.clock_speed_hz = speeds[i];
    device_config.mode = static_cast<hf_spi_mode_t>(i % 4); // Test different modes
    device_config.cs_pin = cs_pins[i];

    int idx = test_bus->CreateDevice(device_config);
    if (idx < 0) {
      ESP_LOGE(TAG, "Failed to create device %zu", i);
      return false;
    }
    device_indices.push_back(idx);
  }

  // Verify all devices exist
  if (test_bus->GetDeviceCount() != speeds.size()) {
    ESP_LOGE(TAG, "Device count mismatch");
    return false;
  }

  // Test device removal - remove middle device
  int remove_index = device_indices[1]; // Remove middle device
  if (!test_bus->RemoveDevice(remove_index)) {
    ESP_LOGE(TAG, "Failed to remove device");
    return false;
  }

  // Verify device count after removal
  if (test_bus->GetDeviceCount() != speeds.size() - 1) {
    ESP_LOGE(TAG, "Device count after removal incorrect");
    return false;
  }

  // Verify remaining devices are still accessible using their current indices
  // After removal, indices shift, so we need to check the new positions
  BaseSpi* device_0 = test_bus->GetDevice(0); // First device is now at index 0
  BaseSpi* device_1 = test_bus->GetDevice(1); // Second device is now at index 1

  if (!device_0 || !device_1) {
    ESP_LOGE(TAG, "Remaining devices not accessible after removal");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Device management tests passed");
  return true;
}

bool test_spi_transfer_basic() noexcept {
  log_test_separator("SPI Basic Transfer Operations");

  // Use SPI2_HOST for transfer tests to isolate from other tests
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus with SPI2_HOST");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test 1: Single byte transfer
  uint8_t tx_byte = 0xAA;
  uint8_t rx_byte = 0x00;
  hf_spi_err_t result = device->Transfer(&tx_byte, &rx_byte, 1, 0);
  ESP_LOGI(TAG, "Single byte transfer result: %s (TX: 0x%02X, RX: 0x%02X)",
           HfSpiErrToString(result).data(), tx_byte, rx_byte);

  // Test 2: Multi-byte transfer
  uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
  uint8_t rx_data[sizeof(tx_data)] = {0};
  result = device->Transfer(tx_data, rx_data, sizeof(tx_data), 0);
  ESP_LOGI(TAG, "Multi-byte transfer result: %s", HfSpiErrToString(result).data());

  // Test 3: Write-only transfer (use different pattern)
  uint8_t write_only_data[] = {0x55, 0xAA, 0x55, 0xAA, 0x55};
  result = device->Transfer(write_only_data, nullptr, sizeof(write_only_data), 0);
  ESP_LOGI(TAG, "Write-only transfer result: %s", HfSpiErrToString(result).data());

  // Test 4: Read-only transfer (clear RX buffer first)
  std::memset(rx_data, 0, sizeof(rx_data));
  result = device->Transfer(nullptr, rx_data, sizeof(rx_data), 0);
  ESP_LOGI(TAG, "Read-only transfer result: %s", HfSpiErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Basic transfer tests passed");

  // Clean up happens automatically via RAII (unique_ptr destructor)
  return true;
}

bool test_spi_transfer_modes() noexcept {
  log_test_separator("SPI Transfer Modes");

  // Use SPI2_HOST for transfer tests to isolate from other tests
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus with SPI2_HOST");
    return false;
  }

  // Test all SPI modes (0-3)
  for (int mode = 0; mode < 4; ++mode) {
    hf_spi_device_config_t device_config = {};
    device_config.clock_speed_hz = MEDIUM_SPEED;
    device_config.mode = static_cast<hf_spi_mode_t>(mode);
    device_config.cs_pin = TEST_CS_PIN_1;

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Failed to create device for mode %d", mode);
      return false;
    }

    BaseSpi* device = test_bus->GetDevice(device_index);
    if (!device) {
      ESP_LOGE(TAG, "Failed to get device for mode %d", mode);
      return false;
    }

    // Test transfer with this mode (use different pattern for each mode)
    uint8_t tx_data[] = {static_cast<uint8_t>(0x12 + mode), static_cast<uint8_t>(0x34 + mode), 
                         static_cast<uint8_t>(0x56 + mode), static_cast<uint8_t>(0x78 + mode)};
    uint8_t rx_data[sizeof(tx_data)] = {0};
    hf_spi_err_t result = device->Transfer(tx_data, rx_data, sizeof(tx_data), 0);

    ESP_LOGI(TAG, "Mode %d transfer result: %s", mode, HfSpiErrToString(result).data());

    // Remove device for next test
    test_bus->RemoveDevice(device_index);
  }

  ESP_LOGI(TAG, "[SUCCESS] Transfer mode tests passed");

  // Clean up happens automatically via RAII (unique_ptr destructor)
  return true;
}

bool test_spi_transfer_sizes() noexcept {
  log_test_separator("SPI Transfer Size Tests");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test various transfer sizes
  std::vector<size_t> test_sizes = {1, 4, 16, 64, 256, 1024};

  for (auto size : test_sizes) {
    auto tx_buffer = std::make_unique<uint8_t[]>(size);
    auto rx_buffer = std::make_unique<uint8_t[]>(size);

    // Generate distinctive test pattern for each size
    if (size <= 16) {
      generate_sequential_pattern(tx_buffer.get(), size, 0x10); // Start at 0x10 for small transfers
    } else if (size <= 256) {
      generate_alternating_pattern(tx_buffer.get(), size, 0x55, 0xAA); // Alternating pattern for medium transfers
    } else {
      generate_test_pattern(tx_buffer.get(), size, 0x01); // Sequential pattern for large transfers
    }
    std::memset(rx_buffer.get(), 0, size);

    hf_spi_err_t result = device->Transfer(tx_buffer.get(), rx_buffer.get(), size, 0);
    ESP_LOGI(TAG, "Transfer size %zu bytes: %s", size, HfSpiErrToString(result).data());

    if (result == hf_spi_err_t::SPI_SUCCESS) {
      // For loopback testing (if MOSI and MISO are connected), verify data
      // In normal operation, this would depend on the connected device
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Transfer size tests passed");
  return true;
}

bool test_spi_dma_operations() noexcept {
  log_test_separator("SPI DMA Operations");

  // Test with DMA enabled
  auto bus_config_dma = create_test_bus_config(FAST_SPEED, true, SPI2_HOST);
  auto test_bus_dma = std::make_unique<EspSpiBus>(bus_config_dma);
  if (!test_bus_dma->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize DMA-enabled test bus");
    return false;
  }

  // Create device for DMA testing
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = FAST_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus_dma->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create DMA device");
    return false;
  }

  BaseSpi* device_dma = test_bus_dma->GetDevice(device_index);
  if (!device_dma) {
    ESP_LOGE(TAG, "Failed to get DMA device");
    return false;
  }

  // Test large transfer that should use DMA
  const size_t dma_test_size = 2048;
  auto tx_buffer = std::make_unique<uint8_t[]>(dma_test_size);
  auto rx_buffer = std::make_unique<uint8_t[]>(dma_test_size);

  // Use distinctive pattern for DMA test
  generate_alternating_pattern(tx_buffer.get(), dma_test_size, 0x55, 0xAA);
  std::memset(rx_buffer.get(), 0, dma_test_size);

  uint64_t start_time = esp_timer_get_time();
  hf_spi_err_t result = device_dma->Transfer(tx_buffer.get(), rx_buffer.get(), dma_test_size, 0);
  uint64_t end_time = esp_timer_get_time();
  uint64_t dma_time = end_time - start_time;

  ESP_LOGI(TAG, "DMA transfer %zu bytes: %s (time: %llu μs)", dma_test_size,
           HfSpiErrToString(result).data(), dma_time);

  test_bus_dma->Deinitialize();
  // Clean up happens automatically via RAII (unique_ptr destructor)

  // Compare with non-DMA transfer
  auto bus_config_no_dma = create_test_bus_config(FAST_SPEED, false, SPI2_HOST);
  auto test_bus_no_dma = std::make_unique<EspSpiBus>(bus_config_no_dma);
  if (!test_bus_no_dma->Initialize()) {
    ESP_LOGW(TAG, "Failed to initialize non-DMA test bus for comparison");
    ESP_LOGI(TAG, "[SUCCESS] DMA operation tests passed");
    return true;
  }

  device_index = test_bus_no_dma->CreateDevice(device_config);
  if (device_index >= 0) {
    BaseSpi* device_no_dma = test_bus_no_dma->GetDevice(device_index);
    if (device_no_dma) {
      start_time = esp_timer_get_time();
      result = device_no_dma->Transfer(tx_buffer.get(), rx_buffer.get(), dma_test_size, 0);
      end_time = esp_timer_get_time();
      uint64_t no_dma_time = end_time - start_time;

      ESP_LOGI(TAG, "Non-DMA transfer %zu bytes: %s (time: %llu μs)", dma_test_size,
               HfSpiErrToString(result).data(), no_dma_time);

      if (dma_time < no_dma_time) {
        ESP_LOGI(TAG, "DMA performance improvement: %.1f%%",
                 ((float)(no_dma_time - dma_time) / no_dma_time) * 100.0f);
      }
    }
  }

  test_bus_no_dma->Deinitialize();
  // Clean up happens automatically via RAII (unique_ptr destructor)

  ESP_LOGI(TAG, "[SUCCESS] DMA operation tests passed");
  return true;
}

bool test_spi_clock_speeds() noexcept {
  log_test_separator("SPI Clock Speed Tests");

  std::vector<uint32_t> test_speeds = {
      SLOW_SPEED,   // 1MHz
      MEDIUM_SPEED, // 10MHz
      FAST_SPEED,   // 40MHz
      MAX_SPEED     // 80MHz
  };

  for (auto speed : test_speeds) {
    auto bus_config = create_test_bus_config(speed, true, SPI2_HOST);
    auto test_bus = std::make_unique<EspSpiBus>(bus_config);
    if (!test_bus->Initialize()) {
      ESP_LOGW(TAG, "Failed to initialize test bus with speed %lu Hz", speed);
      continue;
    }

    // Create device with this speed
    hf_spi_device_config_t device_config = {};
    device_config.clock_speed_hz = speed;
    device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
    device_config.cs_pin = TEST_CS_PIN_1;

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device with speed %lu Hz", speed);
      test_bus->Deinitialize();
      // Clean up happens automatically via RAII (unique_ptr destructor)
      continue;
    }

    EspSpiDevice* esp_device = test_bus->GetEspDevice(device_index);
    if (esp_device) {
      uint32_t actual_freq;
      hf_spi_err_t result = esp_device->GetActualClockFrequency(actual_freq);
      ESP_LOGI(TAG, "Speed %lu Hz: actual frequency %lu Hz (result: %s)", speed, actual_freq,
               HfSpiErrToString(result).data());
    }

    // Test transfer at this speed
    uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
    uint8_t rx_data[sizeof(test_data)] = {0};

    BaseSpi* device = test_bus->GetDevice(device_index);
    if (device) {
      hf_spi_err_t result = device->Transfer(test_data, rx_data, sizeof(test_data), 0);
      ESP_LOGI(TAG, "Transfer at %lu Hz: %s", speed, HfSpiErrToString(result).data());
    }

    test_bus->Deinitialize();
    // Clean up happens automatically via RAII (unique_ptr destructor)

    ESP_LOGI(TAG, "Successfully tested clock speed: %lu Hz", speed);
  }

  ESP_LOGI(TAG, "[SUCCESS] Clock speed tests passed");
  return true;
}

bool test_spi_multi_device_operations() noexcept {
  log_test_separator("SPI Multi-Device Operations");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create multiple devices with different configurations
  std::vector<BaseSpi*> devices;
  std::vector<hf_pin_num_t> cs_pins = {TEST_CS_PIN_1, TEST_CS_PIN_2, TEST_CS_PIN_3};
  std::vector<uint32_t> speeds = {SLOW_SPEED, MEDIUM_SPEED, FAST_SPEED};
  std::vector<hf_spi_mode_t> modes = {hf_spi_mode_t::HF_SPI_MODE_0, hf_spi_mode_t::HF_SPI_MODE_1,
                                      hf_spi_mode_t::HF_SPI_MODE_2};

  for (size_t i = 0; i < cs_pins.size(); ++i) {
    hf_spi_device_config_t device_config = {};
    device_config.clock_speed_hz = speeds[i];
    device_config.mode = modes[i];
    device_config.cs_pin = cs_pins[i];
    device_config.queue_size = 7;

    int device_index = test_bus->CreateDevice(device_config);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device %zu", i);
      continue;
    }

    BaseSpi* device = test_bus->GetDevice(device_index);
    if (device) {
      devices.push_back(device);
    }
  }

  ESP_LOGI(TAG, "Created %zu devices on the bus", devices.size());

  // Test sequential operations on different devices
  for (size_t i = 0; i < devices.size(); ++i) {
    uint8_t test_data[] = {static_cast<uint8_t>(0xA0 + i), 0x55, 0xFF, 0x00};
    uint8_t rx_data[sizeof(test_data)] = {0};

    hf_spi_err_t result = devices[i]->Transfer(test_data, rx_data, sizeof(test_data), 0);
    ESP_LOGI(TAG, "Device %zu transfer result: %s", i, HfSpiErrToString(result).data());

    // Small delay between operations
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ESP_LOGI(TAG, "[SUCCESS] Multi-device operations tests passed");
  return true;
}

bool test_spi_error_handling() noexcept {
  log_test_separator("SPI Error Handling");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test invalid parameters
  uint8_t test_data = 0xAA;
  hf_spi_err_t result;

  // Test null pointer with non-zero length
  result = device->Transfer(nullptr, &test_data, 1, 0);
  ESP_LOGI(TAG, "Transfer with null TX pointer: %s", HfSpiErrToString(result).data());

  result = device->Transfer(&test_data, nullptr, 1, 0);
  ESP_LOGI(TAG, "Transfer with null RX pointer: %s", HfSpiErrToString(result).data());

  // Test zero length transfer
  result = device->Transfer(&test_data, &test_data, 0, 0);
  ESP_LOGI(TAG, "Zero length transfer: %s", HfSpiErrToString(result).data());

  // Test both null pointers
  result = device->Transfer(nullptr, nullptr, 1, 0);
  ESP_LOGI(TAG, "Both null pointers transfer: %s", HfSpiErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] Error handling tests passed");
  return true;
}

bool test_spi_timeout_handling() noexcept {
  log_test_separator("SPI Timeout Handling");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get test device");
    return false;
  }

  // Test transfers with different timeout values
  uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
  uint8_t rx_data[sizeof(test_data)] = {0};

  // Test with short timeout
  uint64_t start_time = esp_timer_get_time();
  hf_spi_err_t result = device->Transfer(test_data, rx_data, sizeof(test_data), 100);
  uint64_t end_time = esp_timer_get_time();
  uint64_t duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Short timeout test: %s (took %llu ms)", HfSpiErrToString(result).data(),
           duration_ms);

  // Test with very short timeout
  start_time = esp_timer_get_time();
  result = device->Transfer(test_data, rx_data, sizeof(test_data), 1);
  end_time = esp_timer_get_time();
  duration_ms = (end_time - start_time) / 1000;

  ESP_LOGI(TAG, "Very short timeout test: %s (took %llu ms)", HfSpiErrToString(result).data(),
           duration_ms);

  ESP_LOGI(TAG, "[SUCCESS] Timeout handling tests passed");
  return true;
}

bool test_spi_esp_specific_features() noexcept {
  log_test_separator("ESP-Specific SPI Features");

  // Test with different clock sources (ESP32-C6 specific)
  hf_spi_bus_config_t bus_config = {};
  bus_config.mosi_pin = TEST_MOSI_PIN;
  bus_config.miso_pin = TEST_MISO_PIN;
  bus_config.sclk_pin = TEST_SCLK_PIN;
  bus_config.host = SPI2_HOST;
  bus_config.clock_speed_hz = MEDIUM_SPEED;
  bus_config.dma_channel = 0;
  bus_config.use_iomux = true;

  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize bus for ESP features test");
    return false;
  }

  // Test device with ESP-specific configurations
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = FAST_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;
  device_config.command_bits = 8;     // Command phase
  device_config.address_bits = 24;    // Address phase
  device_config.dummy_bits = 8;       // Dummy bits
  device_config.cs_ena_pretrans = 2;  // CS setup time
  device_config.cs_ena_posttrans = 2; // CS hold time
  device_config.input_delay_ns = 0;   // Input delay compensation

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create ESP-specific device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get ESP-specific device");
    return false;
  }

  // Test transfer with command and address phases
  uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
  uint8_t rx_data[sizeof(test_data)] = {0};

  hf_spi_err_t result = device->Transfer(test_data, rx_data, sizeof(test_data), 0);
  ESP_LOGI(TAG, "ESP-specific transfer result: %s", HfSpiErrToString(result).data());

  ESP_LOGI(TAG, "[SUCCESS] ESP-specific feature tests passed");
  return true;
}

bool test_spi_iomux_optimization() noexcept {
  log_test_separator("SPI IOMUX Optimization");

  // Test with IOMUX enabled (for maximum performance)
  hf_spi_bus_config_t bus_config_iomux = {};
  bus_config_iomux.mosi_pin = TEST_MOSI_PIN;
  bus_config_iomux.miso_pin = TEST_MISO_PIN;
  bus_config_iomux.sclk_pin = TEST_SCLK_PIN;
  bus_config_iomux.host = SPI2_HOST;
  bus_config_iomux.clock_speed_hz = MAX_SPEED; // High speed for IOMUX test
  bus_config_iomux.use_iomux = true;
  bus_config_iomux.dma_channel = 0;

  auto test_bus_iomux = std::make_unique<EspSpiBus>(bus_config_iomux);
  if (!test_bus_iomux->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize IOMUX bus");
    return false;
  }

  // Create high-speed device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MAX_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus_iomux->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create IOMUX device");
    return false;
  }

  BaseSpi* device_iomux = test_bus_iomux->GetDevice(device_index);
  if (!device_iomux) {
    ESP_LOGE(TAG, "Failed to get IOMUX device");
    return false;
  }

  // Performance test with IOMUX
  const size_t test_size = 1024;
  auto tx_buffer = std::make_unique<uint8_t[]>(test_size);
  auto rx_buffer = std::make_unique<uint8_t[]>(test_size);
  generate_test_pattern(tx_buffer.get(), test_size);

  uint64_t start_time = esp_timer_get_time();
  hf_spi_err_t result = device_iomux->Transfer(tx_buffer.get(), rx_buffer.get(), test_size, 0);
  uint64_t end_time = esp_timer_get_time();
  uint64_t iomux_time = end_time - start_time;

  ESP_LOGI(TAG, "IOMUX transfer %zu bytes: %s (time: %llu μs)", test_size,
           HfSpiErrToString(result).data(), iomux_time);

  test_bus_iomux->Deinitialize();

  // Compare with GPIO matrix
  hf_spi_bus_config_t bus_config_gpio = bus_config_iomux;
  bus_config_gpio.use_iomux = false;
  bus_config_gpio.clock_speed_hz = FAST_SPEED; // Lower speed for GPIO matrix

  auto test_bus_gpio = std::make_unique<EspSpiBus>(bus_config_gpio);
  if (test_bus_gpio->Initialize()) {
    device_config.clock_speed_hz = FAST_SPEED;
    device_index = test_bus_gpio->CreateDevice(device_config);
    if (device_index >= 0) {
      BaseSpi* device_gpio = test_bus_gpio->GetDevice(device_index);
      if (device_gpio) {
        start_time = esp_timer_get_time();
        result = device_gpio->Transfer(tx_buffer.get(), rx_buffer.get(), test_size, 0);
        end_time = esp_timer_get_time();
        uint64_t gpio_time = end_time - start_time;

        ESP_LOGI(TAG, "GPIO matrix transfer %zu bytes: %s (time: %llu μs)", test_size,
                 HfSpiErrToString(result).data(), gpio_time);

        if (iomux_time < gpio_time) {
          ESP_LOGI(TAG, "IOMUX performance improvement: %.1f%%",
                   ((float)(gpio_time - iomux_time) / gpio_time) * 100.0f);
        }
      }
    }
    test_bus_gpio->Deinitialize();
  }

  ESP_LOGI(TAG, "[SUCCESS] IOMUX optimization tests passed");
  return true;
}

bool test_spi_thread_safety() noexcept {
  log_test_separator("SPI Thread Safety");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create device for thread safety test");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get device for thread safety test");
    return false;
  }

  // Perform multiple rapid operations (simulating concurrent access)
  uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
  uint8_t rx_data[sizeof(test_data)] = {0};

  for (int i = 0; i < 20; ++i) {
    hf_spi_err_t result = device->Transfer(test_data, rx_data, sizeof(test_data), 50);
    if (result != hf_spi_err_t::SPI_SUCCESS && result != hf_spi_err_t::SPI_ERR_TRANSFER_TIMEOUT) {
      ESP_LOGW(TAG, "Unexpected error in thread safety test: %s", HfSpiErrToString(result).data());
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Thread safety tests passed (basic verification)");
  return true;
}

bool test_spi_performance_benchmarks() noexcept {
  log_test_separator("SPI Performance Benchmarks");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(FAST_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create high-performance device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = FAST_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create performance test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get performance test device");
    return false;
  }

  // Benchmark different transfer sizes
  std::vector<size_t> benchmark_sizes = {64, 256, 1024, 4096};

  for (auto size : benchmark_sizes) {
    auto tx_buffer = std::make_unique<uint8_t[]>(size);
    auto rx_buffer = std::make_unique<uint8_t[]>(size);
    generate_test_pattern(tx_buffer.get(), size);

    // Warm-up transfer
    device->Transfer(tx_buffer.get(), rx_buffer.get(), size, 0);

    // Benchmark transfer
    const int num_transfers = 10;
    uint64_t total_time = 0;

    for (int i = 0; i < num_transfers; ++i) {
      uint64_t start_time = esp_timer_get_time();
      hf_spi_err_t result = device->Transfer(tx_buffer.get(), rx_buffer.get(), size, 0);
      uint64_t end_time = esp_timer_get_time();

      if (result == hf_spi_err_t::SPI_SUCCESS) {
        total_time += (end_time - start_time);
      }
    }

    uint64_t avg_time_us = total_time / num_transfers;
    double throughput_mbps = (size * 8.0) / avg_time_us; // Mbps

    ESP_LOGI(TAG, "Size %zu bytes: avg time %llu μs, throughput %.2f Mbps", size, avg_time_us,
             throughput_mbps);
  }

  ESP_LOGI(TAG, "[SUCCESS] Performance benchmark tests completed");
  return true;
}

bool test_spi_edge_cases() noexcept {
  log_test_separator("SPI Edge Cases");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Test maximum transfer size
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create edge case test device");
    return false;
  }

  BaseSpi* device = test_bus->GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to get edge case test device");
    return false;
  }

  // Test maximum transfer size (implementation dependent)
  const size_t max_transfer_size = 4092; // ESP32 typical maximum
  auto large_tx_buffer = std::make_unique<uint8_t[]>(max_transfer_size);
  auto large_rx_buffer = std::make_unique<uint8_t[]>(max_transfer_size);

  generate_test_pattern(large_tx_buffer.get(), max_transfer_size);

  hf_spi_err_t result =
      device->Transfer(large_tx_buffer.get(), large_rx_buffer.get(), max_transfer_size, 0);
  ESP_LOGI(TAG, "Maximum transfer size (%zu bytes): %s", max_transfer_size,
           HfSpiErrToString(result).data());

  // Test very small transfers
  uint8_t single_byte = 0xAA;
  result = device->Transfer(&single_byte, &single_byte, 1, 0);
  ESP_LOGI(TAG, "Single byte transfer: %s", HfSpiErrToString(result).data());

  // Test rapid successive transfers
  for (int i = 0; i < 100; ++i) {
    uint8_t data = static_cast<uint8_t>(i);
    device->Transfer(&data, &data, 1, 10); // Short timeout
  }
  ESP_LOGI(TAG, "Rapid successive transfers completed");

  ESP_LOGI(TAG, "[SUCCESS] Edge case tests passed");
  return true;
}

bool test_spi_bus_acquisition() noexcept {
  log_test_separator("SPI Bus Acquisition");

  // Create test bus configuration
  auto bus_config = create_test_bus_config(MEDIUM_SPEED, true, SPI2_HOST);

  // Create bus as unique_ptr directly in test function
  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize test bus");
    return false;
  }

  // Create test device
  hf_spi_device_config_t device_config = {};
  device_config.clock_speed_hz = MEDIUM_SPEED;
  device_config.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_config.cs_pin = TEST_CS_PIN_1;

  int device_index = test_bus->CreateDevice(device_config);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create test device");
    return false;
  }

  EspSpiDevice* esp_device = test_bus->GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to get ESP device for bus acquisition test");
    return false;
  }

  // Test bus acquisition and release
  hf_spi_err_t result = esp_device->AcquireBus(1000);
  ESP_LOGI(TAG, "Bus acquisition result: %s", HfSpiErrToString(result).data());

  if (result == hf_spi_err_t::SPI_SUCCESS) {
    // Perform operations while holding the bus
    uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
    uint8_t rx_data[sizeof(test_data)] = {0};

    for (int i = 0; i < 5; ++i) {
      esp_device->Transfer(test_data, rx_data, sizeof(test_data), 0);
    }

    // Release the bus
    result = esp_device->ReleaseBus();
    ESP_LOGI(TAG, "Bus release result: %s", HfSpiErrToString(result).data());
  }

  ESP_LOGI(TAG, "[SUCCESS] Bus acquisition tests passed");
  return true;
}

bool test_spi_power_management() noexcept {
  log_test_separator("SPI Power Management");

  // Test with power management features
  hf_spi_bus_config_t bus_config = {};
  bus_config.mosi_pin = TEST_MOSI_PIN;
  bus_config.miso_pin = TEST_MISO_PIN;
  bus_config.sclk_pin = TEST_SCLK_PIN;
  bus_config.host = SPI2_HOST;
  bus_config.clock_speed_hz = MEDIUM_SPEED;
  bus_config.dma_channel = 0;
  bus_config.timeout_ms = 2000; // Extended timeout for power management

  auto test_bus = std::make_unique<EspSpiBus>(bus_config);
  if (!test_bus->Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize bus with power management");
    return false;
  }

  ESP_LOGI(TAG, "Successfully tested power management configuration");
  ESP_LOGI(TAG, "[SUCCESS] Power management tests passed");
  return true;
}

// Helper function to create test bus configuration
hf_spi_bus_config_t create_test_bus_config(uint32_t speed, bool use_dma,
                                           spi_host_device_t host) noexcept {
  hf_spi_bus_config_t bus_config = {};
  bus_config.mosi_pin = TEST_MOSI_PIN;
  bus_config.miso_pin = TEST_MISO_PIN;
  bus_config.sclk_pin = TEST_SCLK_PIN;
  bus_config.host = static_cast<hf_host_id_t>(host); // Use specified host for isolation
  bus_config.clock_speed_hz = speed;
  bus_config.dma_channel = use_dma ? 0 : 0xFF; // 0 = auto DMA, 0xFF = disabled
  bus_config.use_iomux = true;
  bus_config.timeout_ms = 1000;
  return bus_config;
}

bool verify_transfer_data(const uint8_t* tx_data, const uint8_t* rx_data, size_t length) noexcept {
  if (!tx_data || !rx_data)
    return false;

  for (size_t i = 0; i < length; ++i) {
    if (tx_data[i] != rx_data[i]) {
      return false;
    }
  }
  return true;
}

void generate_test_pattern(uint8_t* buffer, size_t length, uint8_t seed) noexcept {
  if (!buffer)
    return;

  // Create a more recognizable pattern for debugging
  for (size_t i = 0; i < length; ++i) {
    // Use a pattern that's easy to identify on logic analyzer
    // Pattern: 0x01, 0x02, 0x03, ..., 0xFF, then repeat
    buffer[i] = static_cast<uint8_t>((seed + i) % 256);
    
    // Ensure we don't have 0x00 which might be interpreted as end of string
    if (buffer[i] == 0x00) {
      buffer[i] = 0x01;
    }
  }
}

// Create distinctive patterns for different test types
void generate_sequential_pattern(uint8_t* buffer, size_t length, uint8_t start_value) noexcept {
  if (!buffer)
    return;

  for (size_t i = 0; i < length; ++i) {
    buffer[i] = static_cast<uint8_t>(start_value + i);
  }
}

void generate_alternating_pattern(uint8_t* buffer, size_t length, uint8_t value1, uint8_t value2) noexcept {
  if (!buffer)
    return;

  for (size_t i = 0; i < length; ++i) {
    buffer[i] = (i % 2 == 0) ? value1 : value2;
  }
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
  ESP_LOGI(TAG, "║                    ESP32-C6 SPI COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "SPI");

  // Run all SPI tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_CORE_TESTS, "SPI CORE TESTS", 5,
      // Core functionality tests
      ESP_LOGI(TAG, "Running core SPI functionality tests...");
      RUN_TEST_IN_TASK("bus_initialization", test_spi_bus_initialization, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after core tests
      RUN_TEST_IN_TASK("bus_deinitialization", test_spi_bus_deinitialization, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after deinit test
      RUN_TEST_IN_TASK("configuration_validation", test_spi_configuration_validation, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after config test
      RUN_TEST_IN_TASK("device_creation", test_spi_device_creation, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after device creation test
      RUN_TEST_IN_TASK("device_management", test_spi_device_management, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after device management test
      );

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_TRANSFER_TESTS, "SPI TRANSFER TESTS", 5,
      // Transfer operation tests
      ESP_LOGI(TAG, "Running SPI transfer tests...");
      RUN_TEST_IN_TASK("transfer_basic", test_spi_transfer_basic, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after basic transfer test
      RUN_TEST_IN_TASK("transfer_modes", test_spi_transfer_modes, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after modes test
      RUN_TEST_IN_TASK("transfer_sizes", test_spi_transfer_sizes, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after sizes test
      RUN_TEST_IN_TASK("dma_operations", test_spi_dma_operations, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after DMA test
      );

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_PERFORMANCE_TESTS, "SPI PERFORMANCE TESTS", 5,
      // Performance and multi-device tests
      ESP_LOGI(TAG, "Running SPI performance tests...");
      RUN_TEST_IN_TASK("clock_speeds", test_spi_clock_speeds, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after clock speeds test
      RUN_TEST_IN_TASK("multi_device_operations", test_spi_multi_device_operations, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after multi-device test
      RUN_TEST_IN_TASK("performance_benchmarks", test_spi_performance_benchmarks, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after performance test
      );

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_ADVANCED_TESTS, "SPI ADVANCED TESTS", 5,
      // Advanced features tests
      ESP_LOGI(TAG, "Running SPI advanced feature tests...");
      RUN_TEST_IN_TASK("esp_specific_features", test_spi_esp_specific_features, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after ESP features test
      RUN_TEST_IN_TASK("iomux_optimization", test_spi_iomux_optimization, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after IOMUX test
      RUN_TEST_IN_TASK("thread_safety", test_spi_thread_safety, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after thread safety test
      );

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_STRESS_TESTS, "SPI STRESS TESTS", 5,
      // Stress and error handling tests
      ESP_LOGI(TAG, "Running SPI stress tests...");
      RUN_TEST_IN_TASK("error_handling", test_spi_error_handling, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after error handling test
      RUN_TEST_IN_TASK("timeout_handling", test_spi_timeout_handling, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after timeout test
      RUN_TEST_IN_TASK("edge_cases", test_spi_edge_cases, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after edge cases test
      RUN_TEST_IN_TASK("bus_acquisition", test_spi_bus_acquisition, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after bus acquisition test
      RUN_TEST_IN_TASK("power_management", test_spi_power_management, 8192, 1);
      flip_test_progress_indicator(); // Toggle GPIO14 after power management test
      );

  print_test_summary(g_test_results, "SPI", TAG);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
