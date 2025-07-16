/**
 * @file EspSpi.cpp
 * @brief Implementation of MCU-integrated SPI controller.
 *
 * This file provides the implementation for SPI bus communication using the
 * microcontroller's built-in SPI peripheral. The implementation supports
 * full-duplex communication, configurable clock speeds, multiple chip select
 * management, and DMA-accelerated transfers for high-performance applications
 * with comprehensive error handling and platform abstraction.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "EspSpi.h"
#include <cstring>

// EspSpiBus implementation
EspSpiBus::EspSpiBus(const hf_spi_bus_config_t& config) noexcept : config_(config), initialized_(false) {}
EspSpiBus::~EspSpiBus() noexcept { Deinitialize(); }

bool EspSpiBus::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_) return true;
  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num = config_.mosi_pin;
  bus_cfg.miso_io_num = config_.miso_pin;
  bus_cfg.sclk_io_num = config_.sclk_pin;
  bus_cfg.quadwp_io_num = -1;
  bus_cfg.quadhd_io_num = -1;
  bus_cfg.max_transfer_sz = 4096; // Use max supported or configurable
  bus_cfg.flags = config_.use_iomux ? SPICOMMON_BUSFLAG_IOMUX_PINS : 0;
  spi_host_device_t host = static_cast<spi_host_device_t>(config_.host);
  int dma_chan = (config_.dma_channel == 0xFF) ? SPI_DMA_DISABLED : (config_.dma_channel ? config_.dma_channel : SPI_DMA_CH_AUTO);
  esp_err_t err = spi_bus_initialize(host, &bus_cfg, dma_chan);
  initialized_ = (err == ESP_OK);
  return initialized_;
}

bool EspSpiBus::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) return true;
  spi_host_device_t host = static_cast<spi_host_device_t>(config_.host);
  spi_bus_free(host);
  initialized_ = false;
  return true;
}

std::unique_ptr<BaseSpi> EspSpiBus::createDevice(const hf_spi_device_config_t& device_config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) Initialize();
  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = device_config.clock_speed_hz;
  dev_cfg.mode = device_config.mode;
  dev_cfg.spics_io_num = device_config.cs_pin;
  dev_cfg.queue_size = device_config.queue_size;
  dev_cfg.command_bits = device_config.command_bits;
  dev_cfg.address_bits = device_config.address_bits;
  dev_cfg.dummy_bits = device_config.dummy_bits;
  dev_cfg.duty_cycle_pos = device_config.duty_cycle_pos;
  dev_cfg.cs_ena_pretrans = device_config.cs_ena_pretrans;
  dev_cfg.cs_ena_posttrans = device_config.cs_ena_posttrans;
  dev_cfg.flags = device_config.flags;
  dev_cfg.input_delay_ns = device_config.input_delay_ns;
  dev_cfg.pre_cb = reinterpret_cast<transaction_cb_t>(device_config.pre_cb);
  dev_cfg.post_cb = reinterpret_cast<transaction_cb_t>(device_config.post_cb);
  dev_cfg.user_ctx = device_config.user_ctx;
  spi_device_handle_t handle = nullptr;
  spi_host_device_t host = static_cast<spi_host_device_t>(config_.host);
  esp_err_t err = spi_bus_add_device(host, &dev_cfg, &handle);
  if (err != ESP_OK) return nullptr;
  return std::make_unique<EspSpiDevice>(this, handle, device_config);
}

// EspSpiDevice implementation
EspSpiDevice::EspSpiDevice(EspSpiBus* parent, spi_device_handle_t handle, const hf_spi_device_config_t& config)
  : parent_bus_(parent), handle_(handle), config_(config), initialized_(true) {}
EspSpiDevice::~EspSpiDevice() noexcept {
  if (initialized_) Deinitialize();
}
bool EspSpiDevice::Initialize() noexcept { return initialized_; }
bool EspSpiDevice::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) return true;
  spi_bus_remove_device(handle_);
  initialized_ = false;
  return true;
}
hf_spi_err_t EspSpiDevice::Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (!initialized_) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
  spi_transaction_t trans = {};
  trans.length = static_cast<size_t>(length) * 8;
  trans.tx_buffer = tx_data;
  trans.rx_buffer = rx_data;
  // Optionally set timeout, flags, etc. if needed
  esp_err_t err = spi_device_transmit(handle_, &trans);
  return (err == ESP_OK) ? hf_spi_err_t::SPI_SUCCESS : hf_spi_err_t::SPI_ERR_TRANSFER_FAILED;
}
hf_spi_err_t EspSpiDevice::SetChipSelect(bool /*active*/) noexcept {
  // Hardware CS is managed by ESP-IDF; for software CS, user can extend this.
  return hf_spi_err_t::SPI_SUCCESS;
}
