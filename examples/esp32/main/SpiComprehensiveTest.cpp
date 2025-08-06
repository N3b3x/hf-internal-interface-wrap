/**
 * @file SpiComprehensiveTest.cpp
 * @brief Comprehensive SPI testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspSpi class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * SPI functionalities including bus-device architecture, various transfer modes,
 * and hardware-specific capabilities.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "base/BaseSpi.h"
#include "mcu/esp32/EspSpi.h"
#include "mcu/esp32/utils/EspTypes_SPI.h"

#include "TestFramework.h"

static const char* TAG = "SPI_Test";

static TestResults g_test_results;

// Forward declarations
bool test_spi_bus_initialization() noexcept;
bool test_spi_multiple_bus_initialization() noexcept;
bool test_spi_bus_configuration_validation() noexcept;
bool test_spi_device_operations() noexcept;
bool test_spi_multiple_devices_on_bus() noexcept;
bool test_spi_device_configuration_variations() noexcept;
bool test_spi_transfer_modes() noexcept;
bool test_spi_data_transfer_variations() noexcept;
bool test_spi_bus_acquisition() noexcept;
bool test_spi_error_handling() noexcept;
bool test_spi_performance() noexcept;
bool test_spi_clock_frequency_testing() noexcept;
bool test_spi_device_removal() noexcept;
bool test_spi_concurrent_operations() noexcept;

bool test_spi_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing SPI bus initialization...");

  // Test basic bus initialization
  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.clock_speed_hz = 1000000;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);

  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return false;
  }

  // Test bus configuration retrieval
  const auto& config = test_spi_bus.GetConfig();
  if (config.mosi_pin != 10 || config.miso_pin != 9 || config.sclk_pin != 11) {
    ESP_LOGE(TAG, "Bus configuration mismatch");
    return false;
  }

  // Test host retrieval
  if (test_spi_bus.GetHost() != SPI2_HOST) {
    ESP_LOGE(TAG, "Host mismatch");
    return false;
  }

  // Test double initialization (should be safe)
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Double initialization failed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI bus initialization successful");
  return true;
}

bool test_spi_multiple_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing multiple SPI bus initialization...");

  // Test SPI2 bus
  hf_spi_bus_config_t spi2_cfg = {};
  spi2_cfg.mosi_pin = 10;
  spi2_cfg.miso_pin = 9;
  spi2_cfg.sclk_pin = 11;
  spi2_cfg.host = SPI2_HOST;
  spi2_cfg.dma_channel = 0xFF; // Auto DMA

  EspSpiBus spi2_bus(spi2_cfg);
  if (!spi2_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI2 bus");
    return false;
  }

  // Test SPI3 bus (if available)
  hf_spi_bus_config_t spi3_cfg = {};
  spi3_cfg.mosi_pin = 13;
  spi3_cfg.miso_pin = 12;
  spi3_cfg.sclk_pin = 14;
  spi3_cfg.host = SPI3_HOST;
  spi3_cfg.use_iomux = true;

  EspSpiBus spi3_bus(spi3_cfg);
  if (!spi3_bus.Initialize()) {
    ESP_LOGW(TAG, "SPI3 initialization failed (may not be available on this ESP32 variant)");
  } else {
    ESP_LOGI(TAG, "SPI3 bus initialized successfully");
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple SPI bus initialization completed");
  return true;
}

bool test_spi_bus_configuration_validation() noexcept {
  ESP_LOGI(TAG, "Testing SPI bus configuration validation...");

  // Test different clock speeds
  for (uint32_t clock : {100000, 1000000, 5000000, 10000000}) {
    hf_spi_bus_config_t cfg = {};
    cfg.mosi_pin = 10;
    cfg.miso_pin = 9;
    cfg.sclk_pin = 11;
    cfg.clock_speed_hz = clock;
    cfg.host = SPI2_HOST;

    EspSpiBus bus(cfg);
    if (!bus.Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize bus with clock %lu Hz", clock);
      return false;
    }
    ESP_LOGI(TAG, "Bus initialized with clock %lu Hz", clock);
  }

  // Test DMA configuration variations
  for (uint8_t dma : {0xFF, 1, 2}) {
    hf_spi_bus_config_t cfg = {};
    cfg.mosi_pin = 10;
    cfg.miso_pin = 9;
    cfg.sclk_pin = 11;
    cfg.host = SPI2_HOST;
    cfg.dma_channel = dma;

    EspSpiBus bus(cfg);
    if (!bus.Initialize()) {
      ESP_LOGE(TAG, "Failed to initialize bus with DMA channel %d", dma);
      return false;
    }
    ESP_LOGI(TAG, "Bus initialized with DMA channel %d", dma);
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI bus configuration validation completed");
  return true;
}

bool test_spi_device_operations() noexcept {
  ESP_LOGI(TAG, "Testing SPI device operations...");

  // Create SPI bus first
  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.clock_speed_hz = 1000000;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);

  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus for device test");
    return false;
  }

  // Create SPI device on the bus
  hf_spi_device_config_t spi_dev_cfg = {};
  spi_dev_cfg.clock_speed_hz = 1000000;
  spi_dev_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  spi_dev_cfg.cs_pin = 12;
  spi_dev_cfg.queue_size = 7;
  int device_index = test_spi_bus.CreateDevice(spi_dev_cfg);

  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create SPI device");
    return false;
  }

  // Test device retrieval
  BaseSpi* device = test_spi_bus.GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to retrieve SPI device");
    return false;
  }

  // Test ESP-specific device retrieval
  EspSpiDevice* esp_device = test_spi_bus.GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to retrieve ESP SPI device");
    return false;
  }

  // Test device configuration retrieval
  const auto& dev_config = esp_device->GetConfig();
  if (dev_config.clock_speed_hz != 1000000 || dev_config.cs_pin != 12) {
    ESP_LOGE(TAG, "Device configuration mismatch");
    return false;
  }

  // Test device count
  if (test_spi_bus.GetDeviceCount() != 1) {
    ESP_LOGE(TAG, "Device count mismatch");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI device created with index: %d", device_index);
  return true;
}

bool test_spi_multiple_devices_on_bus() noexcept {
  ESP_LOGI(TAG, "Testing multiple SPI devices on bus...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return false;
  }

  // Create multiple devices with different configurations
  const uint8_t cs_pins[] = {12, 13, 14, 15};
  const uint32_t clock_speeds[] = {1000000, 2000000, 5000000, 10000000};
  const hf_spi_mode_t modes[] = {
    hf_spi_mode_t::HF_SPI_MODE_0,
    hf_spi_mode_t::HF_SPI_MODE_1,
    hf_spi_mode_t::HF_SPI_MODE_2,
    hf_spi_mode_t::HF_SPI_MODE_3
  };

  int device_indices[4];
  for (int i = 0; i < 4; i++) {
    hf_spi_device_config_t cfg = {};
    cfg.clock_speed_hz = clock_speeds[i];
    cfg.mode = modes[i];
    cfg.cs_pin = cs_pins[i];
    cfg.queue_size = 7;

    device_indices[i] = test_spi_bus.CreateDevice(cfg);
    if (device_indices[i] < 0) {
      ESP_LOGE(TAG, "Failed to create device %d", i);
      return false;
    }
    ESP_LOGI(TAG, "Created device %d with CS pin %d, clock %lu Hz, mode %d", 
             i, cs_pins[i], clock_speeds[i], static_cast<int>(modes[i]));
  }

  // Verify device count
  if (test_spi_bus.GetDeviceCount() != 4) {
    ESP_LOGE(TAG, "Expected 4 devices, got %zu", test_spi_bus.GetDeviceCount());
    return false;
  }

  // Test retrieving all devices
  for (int i = 0; i < 4; i++) {
    BaseSpi* device = test_spi_bus.GetDevice(device_indices[i]);
    if (!device) {
      ESP_LOGE(TAG, "Failed to retrieve device %d", i);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple SPI devices created and verified");
  return true;
}

bool test_spi_device_configuration_variations() noexcept {
  ESP_LOGI(TAG, "Testing SPI device configuration variations...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return false;
  }

  // Test device with command and address bits
  hf_spi_device_config_t advanced_cfg = {};
  advanced_cfg.clock_speed_hz = 5000000;
  advanced_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  advanced_cfg.cs_pin = 12;
  advanced_cfg.command_bits = 8;
  advanced_cfg.address_bits = 24;
  advanced_cfg.dummy_bits = 4;
  advanced_cfg.duty_cycle_pos = 128; // 50% duty cycle

  int device_index = test_spi_bus.CreateDevice(advanced_cfg);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create advanced SPI device");
    return false;
  }

  EspSpiDevice* esp_device = test_spi_bus.GetEspDevice(device_index);
  if (!esp_device) {
    ESP_LOGE(TAG, "Failed to retrieve ESP device");
    return false;
  }

  // Test actual clock frequency retrieval
  uint32_t actual_freq = 0;
  hf_spi_err_t freq_result = esp_device->GetActualClockFrequency(actual_freq);
  if (freq_result == hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGI(TAG, "Requested: %lu Hz, Actual: %lu Hz", advanced_cfg.clock_speed_hz, actual_freq);
  } else {
    ESP_LOGW(TAG, "Could not retrieve actual clock frequency");
  }

  ESP_LOGI(TAG, "[SUCCESS] Advanced SPI device configuration tested");
  return true;
}

bool test_spi_transfer_modes() noexcept {
  ESP_LOGI(TAG, "Testing SPI transfer modes...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus");
    return false;
  }

  hf_spi_device_config_t device_cfg = {};
  device_cfg.clock_speed_hz = 1000000;
  device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_cfg.cs_pin = 12;

  int device_index = test_spi_bus.CreateDevice(device_cfg);
  if (device_index < 0) {
    ESP_LOGE(TAG, "Failed to create SPI device");
    return false;
  }

  BaseSpi* device = test_spi_bus.GetDevice(device_index);
  if (!device) {
    ESP_LOGE(TAG, "Failed to retrieve SPI device");
    return false;
  }

  // Test write-only transfer
  uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04};
  hf_spi_err_t result = device->Transfer(tx_data, nullptr, sizeof(tx_data));
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Write-only transfer failed: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Write-only transfer completed");

  // Test read-only transfer
  uint8_t rx_data[4] = {0};
  result = device->Transfer(nullptr, rx_data, sizeof(rx_data));
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Read-only transfer failed: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Read-only transfer completed");

  // Test full-duplex transfer
  uint8_t tx_duplex[] = {0xAA, 0xBB, 0xCC, 0xDD};
  uint8_t rx_duplex[4] = {0};
  result = device->Transfer(tx_duplex, rx_duplex, sizeof(tx_duplex));
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Full-duplex transfer failed: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Full-duplex transfer completed");

  ESP_LOGI(TAG, "[SUCCESS] SPI transfer modes tested");
  return true;
}

bool test_spi_data_transfer_variations() noexcept {
  ESP_LOGI(TAG, "Testing SPI data transfer variations...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  hf_spi_device_config_t device_cfg = {};
  device_cfg.clock_speed_hz = 1000000;
  device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_cfg.cs_pin = 12;

  int device_index = test_spi_bus.CreateDevice(device_cfg);
  BaseSpi* device = test_spi_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test different data sizes
  const size_t test_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  for (size_t size : test_sizes) {
    if (size > HF_SPI_MAX_TRANSFER_SIZE) continue;

    std::vector<uint8_t> tx_data(size);
    std::vector<uint8_t> rx_data(size);
    
    // Fill with test pattern
    for (size_t i = 0; i < size; i++) {
      tx_data[i] = static_cast<uint8_t>(i & 0xFF);
    }

    hf_spi_err_t result = device->Transfer(tx_data.data(), rx_data.data(), size);
    if (result != hf_spi_err_t::SPI_SUCCESS) {
      ESP_LOGE(TAG, "Transfer failed for size %zu: %d", size, static_cast<int>(result));
      return false;
    }
    ESP_LOGI(TAG, "Successfully transferred %zu bytes", size);
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI data transfer variations tested");
  return true;
}

bool test_spi_bus_acquisition() noexcept {
  ESP_LOGI(TAG, "Testing SPI bus acquisition...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  hf_spi_device_config_t device_cfg = {};
  device_cfg.clock_speed_hz = 1000000;
  device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_cfg.cs_pin = 12;

  int device_index = test_spi_bus.CreateDevice(device_cfg);
  EspSpiDevice* esp_device = test_spi_bus.GetEspDevice(device_index);
  if (!esp_device) {
    return false;
  }

  // Test bus acquisition
  hf_spi_err_t result = esp_device->AcquireBus(1000); // 1 second timeout
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to acquire bus: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Bus acquired successfully");

  // Perform a transfer while bus is acquired
  uint8_t tx_data[] = {0x11, 0x22, 0x33};
  uint8_t rx_data[3] = {0};
  result = esp_device->Transfer(tx_data, rx_data, sizeof(tx_data));
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Transfer failed while bus acquired: %d", static_cast<int>(result));
    esp_device->ReleaseBus();
    return false;
  }
  ESP_LOGI(TAG, "Transfer completed while bus acquired");

  // Release the bus
  result = esp_device->ReleaseBus();
  if (result != hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Failed to release bus: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Bus released successfully");

  ESP_LOGI(TAG, "[SUCCESS] SPI bus acquisition tested");
  return true;
}

bool test_spi_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing SPI error handling...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  hf_spi_device_config_t device_cfg = {};
  device_cfg.clock_speed_hz = 1000000;
  device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_cfg.cs_pin = 12;

  int device_index = test_spi_bus.CreateDevice(device_cfg);
  BaseSpi* device = test_spi_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Test invalid parameters
  uint8_t valid_data[] = {0x01, 0x02};
  
  // Test zero length transfer
  hf_spi_err_t result = device->Transfer(valid_data, nullptr, 0);
  if (result == hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "Zero length transfer should have failed");
    return false;
  }
  ESP_LOGI(TAG, "Zero length transfer correctly rejected");

  // Test NULL pointers for both TX and RX
  result = device->Transfer(nullptr, nullptr, 2);
  if (result == hf_spi_err_t::SPI_SUCCESS) {
    ESP_LOGE(TAG, "NULL pointer transfer should have failed");
    return false;
  }
  ESP_LOGI(TAG, "NULL pointer transfer correctly rejected");

  // Test oversized transfer
  if (HF_SPI_MAX_TRANSFER_SIZE < 10000) {
    result = device->Transfer(valid_data, nullptr, HF_SPI_MAX_TRANSFER_SIZE + 1);
    if (result == hf_spi_err_t::SPI_SUCCESS) {
      ESP_LOGE(TAG, "Oversized transfer should have failed");
      return false;
    }
    ESP_LOGI(TAG, "Oversized transfer correctly rejected");
  }

  // Test invalid device index
  BaseSpi* invalid_device = test_spi_bus.GetDevice(999);
  if (invalid_device != nullptr) {
    ESP_LOGE(TAG, "Invalid device index should return nullptr");
    return false;
  }
  ESP_LOGI(TAG, "Invalid device index correctly handled");

  ESP_LOGI(TAG, "[SUCCESS] SPI error handling tested");
  return true;
}

bool test_spi_performance() noexcept {
  ESP_LOGI(TAG, "Testing SPI performance...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;
  spi_bus_cfg.dma_channel = 0xFF; // Enable DMA

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  hf_spi_device_config_t device_cfg = {};
  device_cfg.clock_speed_hz = 10000000; // 10 MHz
  device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device_cfg.cs_pin = 12;

  int device_index = test_spi_bus.CreateDevice(device_cfg);
  BaseSpi* device = test_spi_bus.GetDevice(device_index);
  if (!device) {
    return false;
  }

  // Performance test with different data sizes
  const size_t test_sizes[] = {16, 64, 256, 1024};
  const int iterations = 100;

  for (size_t size : test_sizes) {
    if (size > HF_SPI_MAX_TRANSFER_SIZE) continue;

    std::vector<uint8_t> tx_data(size, 0xAA);
    std::vector<uint8_t> rx_data(size);

    uint64_t start_time = esp_timer_get_time();
    
    for (int i = 0; i < iterations; i++) {
      hf_spi_err_t result = device->Transfer(tx_data.data(), rx_data.data(), size);
      if (result != hf_spi_err_t::SPI_SUCCESS) {
        ESP_LOGE(TAG, "Performance test failed at iteration %d for size %zu", i, size);
        return false;
      }
    }

    uint64_t end_time = esp_timer_get_time();
    uint64_t total_time_us = end_time - start_time;
    double avg_time_us = static_cast<double>(total_time_us) / iterations;
    double throughput_mbps = (size * 8.0 * 1000000.0) / (avg_time_us * 1024.0 * 1024.0);

    ESP_LOGI(TAG, "Size: %zu bytes, Avg Time: %.2f μs, Throughput: %.2f Mbps", 
             size, avg_time_us, throughput_mbps);
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI performance testing completed");
  return true;
}

bool test_spi_clock_frequency_testing() noexcept {
  ESP_LOGI(TAG, "Testing SPI clock frequency variations...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  // Test different clock frequencies
  const uint32_t clock_frequencies[] = {100000, 1000000, 5000000, 10000000, 20000000};
  
  for (uint32_t clock_freq : clock_frequencies) {
    hf_spi_device_config_t device_cfg = {};
    device_cfg.clock_speed_hz = clock_freq;
    device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
    device_cfg.cs_pin = 12;

    int device_index = test_spi_bus.CreateDevice(device_cfg);
    if (device_index < 0) {
      ESP_LOGW(TAG, "Failed to create device with clock %lu Hz", clock_freq);
      continue;
    }

    EspSpiDevice* esp_device = test_spi_bus.GetEspDevice(device_index);
    if (esp_device) {
      uint32_t actual_freq = 0;
      hf_spi_err_t result = esp_device->GetActualClockFrequency(actual_freq);
      if (result == hf_spi_err_t::SPI_SUCCESS) {
        ESP_LOGI(TAG, "Requested: %lu Hz, Actual: %lu Hz (%.2f%% accuracy)", 
                 clock_freq, actual_freq, 
                 (static_cast<double>(actual_freq) / clock_freq) * 100.0);
      }

      // Test a simple transfer at this frequency
      uint8_t test_data[] = {0x12, 0x34};
      uint8_t rx_data[2] = {0};
      result = esp_device->Transfer(test_data, rx_data, 2);
      if (result != hf_spi_err_t::SPI_SUCCESS) {
        ESP_LOGW(TAG, "Transfer failed at %lu Hz", clock_freq);
      } else {
        ESP_LOGI(TAG, "Transfer successful at %lu Hz", clock_freq);
      }
    }

    // Remove device for next test
    test_spi_bus.RemoveDevice(device_index);
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI clock frequency testing completed");
  return true;
}

bool test_spi_device_removal() noexcept {
  ESP_LOGI(TAG, "Testing SPI device removal...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  // Create multiple devices
  std::vector<int> device_indices;
  for (int i = 0; i < 3; i++) {
    hf_spi_device_config_t device_cfg = {};
    device_cfg.clock_speed_hz = 1000000;
    device_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
    device_cfg.cs_pin = 12 + i;

    int device_index = test_spi_bus.CreateDevice(device_cfg);
    if (device_index < 0) {
      ESP_LOGE(TAG, "Failed to create device %d", i);
      return false;
    }
    device_indices.push_back(device_index);
  }

  // Verify device count
  if (test_spi_bus.GetDeviceCount() != 3) {
    ESP_LOGE(TAG, "Expected 3 devices, got %zu", test_spi_bus.GetDeviceCount());
    return false;
  }

  // Remove middle device
  bool removed = test_spi_bus.RemoveDevice(device_indices[1]);
  if (!removed) {
    ESP_LOGE(TAG, "Failed to remove device");
    return false;
  }

  // Verify device count decreased
  if (test_spi_bus.GetDeviceCount() != 2) {
    ESP_LOGE(TAG, "Expected 2 devices after removal, got %zu", test_spi_bus.GetDeviceCount());
    return false;
  }

  // Test removal of invalid index
  removed = test_spi_bus.RemoveDevice(999);
  if (removed) {
    ESP_LOGE(TAG, "Removal of invalid device should have failed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI device removal tested");
  return true;
}

bool test_spi_concurrent_operations() noexcept {
  ESP_LOGI(TAG, "Testing SPI concurrent operations...");

  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.host = SPI2_HOST;

  EspSpiBus test_spi_bus(spi_bus_cfg);
  if (!test_spi_bus.Initialize()) {
    return false;
  }

  // Create two devices
  hf_spi_device_config_t device1_cfg = {};
  device1_cfg.clock_speed_hz = 1000000;
  device1_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
  device1_cfg.cs_pin = 12;

  hf_spi_device_config_t device2_cfg = {};
  device2_cfg.clock_speed_hz = 2000000;
  device2_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_1;
  device2_cfg.cs_pin = 13;

  int device1_index = test_spi_bus.CreateDevice(device1_cfg);
  int device2_index = test_spi_bus.CreateDevice(device2_cfg);

  if (device1_index < 0 || device2_index < 0) {
    ESP_LOGE(TAG, "Failed to create devices for concurrent test");
    return false;
  }

  BaseSpi* device1 = test_spi_bus.GetDevice(device1_index);
  BaseSpi* device2 = test_spi_bus.GetDevice(device2_index);

  if (!device1 || !device2) {
    ESP_LOGE(TAG, "Failed to retrieve devices");
    return false;
  }

  // Perform interleaved transfers
  uint8_t tx1[] = {0x11, 0x22};
  uint8_t tx2[] = {0x33, 0x44};
  uint8_t rx1[2] = {0};
  uint8_t rx2[2] = {0};

  for (int i = 0; i < 10; i++) {
    hf_spi_err_t result1 = device1->Transfer(tx1, rx1, 2);
    hf_spi_err_t result2 = device2->Transfer(tx2, rx2, 2);

    if (result1 != hf_spi_err_t::SPI_SUCCESS || result2 != hf_spi_err_t::SPI_SUCCESS) {
      ESP_LOGE(TAG, "Concurrent transfer failed at iteration %d", i);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] SPI concurrent operations tested");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 SPI COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Bus and initialization tests
  RUN_TEST(test_spi_bus_initialization);
  RUN_TEST(test_spi_multiple_bus_initialization);
  RUN_TEST(test_spi_bus_configuration_validation);

  // Device management tests
  RUN_TEST(test_spi_device_operations);
  RUN_TEST(test_spi_multiple_devices_on_bus);
  RUN_TEST(test_spi_device_configuration_variations);

  // Transfer and communication tests
  RUN_TEST(test_spi_transfer_modes);
  RUN_TEST(test_spi_data_transfer_variations);
  RUN_TEST(test_spi_bus_acquisition);

  // Error handling and robustness tests
  RUN_TEST(test_spi_error_handling);
  RUN_TEST(test_spi_device_removal);
  RUN_TEST(test_spi_concurrent_operations);

  // Performance and frequency tests
  RUN_TEST(test_spi_performance);
  RUN_TEST(test_spi_clock_frequency_testing);

  print_test_summary(g_test_results, "SPI", TAG);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
