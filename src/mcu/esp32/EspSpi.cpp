/**
 * @file EspSpi.cpp
 * @brief Implementation of MCU-integrated SPI controller for ESP32C6 with ESP-IDF v5.5+ features.
 *
 * This file provides the implementation for SPI bus communication using the ESP32C6's
 * built-in SPI peripheral with full ESP-IDF v5.5+ capabilities. The implementation supports:
 *
 * - Full-duplex and half-duplex communication modes
 * - DMA-accelerated transfers for high-performance applications
 * - Multiple clock sources (APB, XTAL) for power optimization
 * - IOMUX pins for maximum performance (up to 80MHz)
 * - Advanced timing control with input delay compensation
 * - Transaction queuing with interrupt and polling modes
 * - Bus acquisition for exclusive device access
 * - Comprehensive error handling and validation
 * - Thread-safe operations with RTOS mutex protection
 * - Small data optimization for transfers <= 32 bits
 * - Proper BaseSpi class integration for portability
 *
 * The implementation closely follows ESP-IDF v5.5 SPI Master driver documentation:
 * https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/peripherals/spi_master.html
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Fully compliant with ESP-IDF v5.5 SPI Master driver API
 * @note Optimized for ESP32C6 hardware capabilities
 * @note Thread-safe for multi-device SPI bus management
 */

#include "EspSpi.h"
#include "utils/memory_utils.h"
#include <cstring>

// ESP-IDF additional includes for ESP32C6 features
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_err.h"           // For esp_err_to_name
#include "freertos/FreeRTOS.h" // For portMAX_DELAY
#include "freertos/task.h"     // For pdMS_TO_TICKS
#include "soc/spi_reg.h"       // For SOC constants
#ifdef __cplusplus
}
#endif

static const char* TAG = "EspSpi";

//======================================================//
// ESP SPI BUS IMPLEMENTATION
//======================================================//

EspSpiBus::EspSpiBus(const hf_spi_bus_config_t& config) noexcept
    : config_(config), initialized_(false) {}
EspSpiBus::~EspSpiBus() noexcept {
  Deinitialize();
}

bool EspSpiBus::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_)
    return true;

  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num = config_.mosi_pin;
  bus_cfg.miso_io_num = config_.miso_pin;
  bus_cfg.sclk_io_num = config_.sclk_pin;
  bus_cfg.quadwp_io_num = -1; // Set to -1 unless using quad SPI
  bus_cfg.quadhd_io_num = -1; // Set to -1 unless using quad SPI

  // ESP32C6 allows larger transfer sizes with proper DMA configuration
  bus_cfg.max_transfer_sz = (config_.dma_channel != 0xFF) ? 4092 : 64;

  // Enhanced flags based on ESP-IDF v5.5+ capabilities
  bus_cfg.flags = 0;
  if (config_.use_iomux) {
    bus_cfg.flags |= SPICOMMON_BUSFLAG_IOMUX_PINS;
  }

  // Add ESP32C6-specific optimizations
  bus_cfg.flags |= SPICOMMON_BUSFLAG_MASTER; // Explicit master mode

  spi_host_device_t host = static_cast<spi_host_device_t>(config_.host);
  int dma_chan = (config_.dma_channel == 0xFF)
                     ? static_cast<int>(SPI_DMA_DISABLED)
                     : (config_.dma_channel ? static_cast<int>(config_.dma_channel)
                                            : static_cast<int>(SPI_DMA_CH_AUTO));

  esp_err_t err = spi_bus_initialize(host, &bus_cfg, dma_chan);
  if (err != ESP_OK) {
    ESP_LOGE("EspSpiBus", "Failed to initialize SPI bus: %s", esp_err_to_name(err));
    return false;
  }

  initialized_ = true;
  return true;
}

bool EspSpiBus::IsInitialized() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return initialized_;
}

bool EspSpiBus::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing SPI bus");

  // Deinitialize all devices first
  for (auto& device : devices_) {
    if (device && device->IsInitialized()) {
      device->Deinitialize(); // This removes from ESP-IDF bus
    }
  }
  devices_.clear();

  // Free the SPI bus from ESP-IDF
  spi_host_device_t host = static_cast<spi_host_device_t>(config_.host);
  esp_err_t err = spi_bus_free(host);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to free SPI bus: %s", esp_err_to_name(err));
  }

  initialized_ = false;
  ESP_LOGI(TAG, "SPI bus deinitialized successfully");
  return true;
}

int EspSpiBus::CreateDevice(const hf_spi_device_config_t& device_config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) {
    return -1; // Bus must be initialized first
  }

  // Create C++ wrapper class first (no ESP-IDF device yet)
  auto device = hf::utils::make_unique_nothrow<EspSpiDevice>(this, device_config);
  if (!device) {
    ESP_LOGE(TAG, "Failed to allocate memory for EspSpiDevice");
    return -1;
  }

  // Store in vector (C++ wrapper only)
  devices_.push_back(std::move(device));
  return static_cast<int>(devices_.size() - 1);
}

BaseSpi* EspSpiBus::GetDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }
  return devices_[device_index].get();
}

const BaseSpi* EspSpiBus::GetDevice(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }
  return devices_[device_index].get();
}

EspSpiDevice* EspSpiBus::GetEspDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }
  return devices_[device_index].get();
}

const EspSpiDevice* EspSpiBus::GetEspDevice(int device_index) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return nullptr;
  }
  return devices_[device_index].get();
}

std::size_t EspSpiBus::GetDeviceCount() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return devices_.size();
}

bool EspSpiBus::RemoveDevice(int device_index) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (device_index < 0 || device_index >= static_cast<int>(devices_.size())) {
    return false;
  }

  // Remove the device (destructor will call spi_bus_remove_device)
  devices_.erase(devices_.begin() + device_index);
  return true;
}

const hf_spi_bus_config_t& EspSpiBus::GetConfig() const noexcept {
  return config_;
}

spi_host_device_t EspSpiBus::GetHost() const noexcept {
  return static_cast<spi_host_device_t>(config_.host);
}

//======================================================//
// ESP SPI DEVICE IMPLEMENTATION
//======================================================//

EspSpiDevice::EspSpiDevice(EspSpiBus* parent, const hf_spi_device_config_t& config)
    : BaseSpi(), parent_bus_(parent), handle_(nullptr), config_(config), initialized_(false) {
  // Device is NOT initialized yet - no ESP-IDF handle
  ESP_LOGI(TAG, "EspSpiDevice created (not yet initialized), CS pin %d", config.cs_pin);
}

EspSpiDevice::~EspSpiDevice() noexcept {
  if (initialized_)
    Deinitialize();
}

bool EspSpiDevice::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_) {
    return true;
  }

  // NOW create the ESP-IDF device and add to bus
  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = config_.clock_speed_hz;
  dev_cfg.mode = static_cast<uint8_t>(config_.mode);
  dev_cfg.spics_io_num = config_.cs_pin;
  dev_cfg.queue_size = config_.queue_size;
  dev_cfg.command_bits = config_.command_bits;
  dev_cfg.address_bits = config_.address_bits;
  dev_cfg.dummy_bits = config_.dummy_bits;
  dev_cfg.duty_cycle_pos = config_.duty_cycle_pos;
  dev_cfg.cs_ena_pretrans = config_.cs_ena_pretrans;
  dev_cfg.cs_ena_posttrans = config_.cs_ena_posttrans;
  dev_cfg.flags = config_.flags;
  dev_cfg.input_delay_ns = config_.input_delay_ns;

  // Set callbacks if provided
  if (config_.pre_cb) {
    dev_cfg.pre_cb = reinterpret_cast<transaction_cb_t>(config_.pre_cb);
  }
  if (config_.post_cb) {
    dev_cfg.post_cb = reinterpret_cast<transaction_cb_t>(config_.post_cb);
  }

  // Set clock source and sampling point with ESP32-C6 compatible defaults
  dev_cfg.clock_source = (config_.clock_source != 0)
                             ? static_cast<spi_clock_source_t>(config_.clock_source)
                             : SPI_CLK_SRC_DEFAULT;

  // ESP32-C6 only supports PHASE_0 sampling point
  if (config_.sampling_point != 0) {
    dev_cfg.sample_point = static_cast<spi_sampling_point_t>(config_.sampling_point);
    if (dev_cfg.sample_point != SPI_SAMPLING_POINT_PHASE_0) {
      ESP_LOGW(TAG, "ESP32-C6 only supports PHASE_0 sampling point, using default");
      dev_cfg.sample_point = SPI_SAMPLING_POINT_PHASE_0;
    }
  } else {
    dev_cfg.sample_point = SPI_SAMPLING_POINT_PHASE_0;
  }

  // Add ESP-IDF specific flags
  if (config_.flags & HF_SPI_DEVICE_HALFDUPLEX) {
    dev_cfg.flags |= SPI_DEVICE_HALFDUPLEX;
  }
  if (config_.flags & HF_SPI_DEVICE_POSITIVE_CS) {
    dev_cfg.flags |= SPI_DEVICE_POSITIVE_CS;
  }

  // Add device to ESP-IDF bus
  spi_host_device_t host = parent_bus_->GetHost();
  esp_err_t err = spi_bus_add_device(host, &dev_cfg, &handle_);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(err));
    return false;
  }

  initialized_ = true;
  ESP_LOGI(TAG, "EspSpiDevice initialized successfully, handle %p", handle_);
  return true;
}

bool EspSpiDevice::MarkAsDeinitialized() noexcept {
  if (!initialized_) {
    ESP_LOGI(TAG, "Device is already marked as deinitialized");
    return true;
  }
  ESP_LOGI(TAG, "Marking SPI device as deinitialized");
  initialized_ = false;
  return true;
}

bool EspSpiDevice::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) {
    return true;
  }

  // Remove from ESP-IDF bus
  if (handle_) {
    esp_err_t err = spi_bus_remove_device(handle_);
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to remove SPI device: %s", esp_err_to_name(err));
    }
    handle_ = nullptr;
  }

  initialized_ = false;
  ESP_LOGI(TAG, "EspSpiDevice deinitialized");
  return true;
}
hf_spi_err_t EspSpiDevice::Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data, hf_u16_t length,
                                    hf_u32_t timeout_ms) noexcept {
  // Note: timeout_ms parameter is reserved for future use. ESP-IDF spi_device_transmit()
  // uses the queue_size timeout configured during device initialization.
  (void)timeout_ms; // Suppress unused parameter warning for now

  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_)
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

  // Validate parameters
  if (length == 0)
    return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
  if (length > HF_SPI_MAX_TRANSFER_SIZE)
    return hf_spi_err_t::SPI_ERR_TRANSFER_TOO_LONG;
  if (!tx_data && !rx_data)
    return hf_spi_err_t::SPI_ERR_NULL_POINTER;

  spi_transaction_t trans = {};
  trans.length = static_cast<size_t>(length) * 8; // ESP-IDF expects bits, not bytes
  trans.tx_buffer = tx_data;
  trans.rx_buffer = rx_data;

  // // Use small data optimization for transfers <= 32 bits (4 bytes)
  // if (length <= 4) {
  //   if (tx_data) {
  //     trans.flags |= SPI_TRANS_USE_TXDATA;
  //     std::memcpy(trans.tx_data, tx_data, length);
  //     trans.tx_buffer = nullptr;
  //   }
  //   if (rx_data) {
  //     trans.flags |= SPI_TRANS_USE_RXDATA;
  //     trans.rx_buffer = nullptr;
  //   }
  // }

  esp_err_t err = spi_device_transmit(handle_, &trans);

  // // Copy received data for small transfers
  // if (err == ESP_OK && rx_data && (trans.flags & SPI_TRANS_USE_RXDATA)) {
  //   std::memcpy(rx_data, trans.rx_data, length);
  // }

  // Enhanced error mapping based on ESP-IDF v5.5 documentation
  switch (err) {
    case ESP_OK:
      return hf_spi_err_t::SPI_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    case ESP_ERR_TIMEOUT:
      return hf_spi_err_t::SPI_ERR_TRANSFER_TIMEOUT;
    case ESP_ERR_NO_MEM:
      return hf_spi_err_t::SPI_ERR_OUT_OF_MEMORY;
    case ESP_ERR_INVALID_STATE:
      return hf_spi_err_t::SPI_ERR_BUS_BUSY;
    default:
      return hf_spi_err_t::SPI_ERR_TRANSFER_FAILED;
  }
}

const void* EspSpiDevice::GetDeviceConfig() const noexcept {
  return &config_;
}

spi_device_handle_t EspSpiDevice::GetHandle() const noexcept {
  return handle_;
}

const hf_spi_device_config_t& EspSpiDevice::GetConfig() const noexcept {
  return config_;
}

hf_spi_err_t EspSpiDevice::AcquireBus(hf_u32_t timeout_ms) noexcept {
  if (!initialized_)
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

  TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  esp_err_t err = spi_device_acquire_bus(handle_, ticks);

  switch (err) {
    case ESP_OK:
      return hf_spi_err_t::SPI_SUCCESS;
    case ESP_ERR_TIMEOUT:
      return hf_spi_err_t::SPI_ERR_BUS_TIMEOUT;
    case ESP_ERR_INVALID_ARG:
      return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    default:
      return hf_spi_err_t::SPI_ERR_BUS_BUSY;
  }
}

hf_spi_err_t EspSpiDevice::ReleaseBus() noexcept {
  if (!initialized_)
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

  spi_device_release_bus(handle_);
  return hf_spi_err_t::SPI_SUCCESS;
}

hf_spi_err_t EspSpiDevice::GetActualClockFrequency(hf_u32_t& actual_freq_hz) const noexcept {
  if (!initialized_)
    return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

  int freq_khz = 0;
  esp_err_t err = spi_device_get_actual_freq(handle_, &freq_khz);

  if (err == ESP_OK) {
    actual_freq_hz = static_cast<hf_u32_t>(freq_khz * 1000);
    return hf_spi_err_t::SPI_SUCCESS;
  } else {
    return hf_spi_err_t::SPI_ERR_HARDWARE_FAULT;
  }
}
