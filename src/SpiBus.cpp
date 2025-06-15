/**
 * @file SpiBus.cpp
 * @brief Implementation of the SpiBus class for ESP32 (ESP-IDF), providing SPI
 * master communication.
 *
 * This class abstracts the ESP-IDF SPI master driver and provides SPI
 * transactions. It replaces Synergy/ThreadX/SSP-specific code with ESP-IDF
 * APIs.
 *
 * @author Nebula Tech Corporation
 * @copyright Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 */

#include "SpiBus.h"
#include <cstring>
#include <esp_log.h>

static const char *TAG = "SpiBus";

SpiBus::SpiBus(spi_host_device_t host, const spi_bus_config_t &buscfg,
               const spi_device_interface_config_t &devcfg) noexcept
    : spiHost(host), spiHandle(nullptr), busConfig(buscfg), devConfig(devcfg),
      initialized(false) {
  // No additional initialization required here.
}

SpiBus::~SpiBus() noexcept {
  if (initialized) {
    Close();
  }
}

bool SpiBus::Open() noexcept {
  if (initialized)
    return true;
  esp_err_t ret = spi_bus_initialize(spiHost, &busConfig, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %d", ret);
    return false;
  }
  ret = spi_bus_add_device(spiHost, &devConfig, &spiHandle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %d", ret);
    return false;
  }
  initialized = true;
  return true;
}

bool SpiBus::Close() noexcept {
  if (!initialized)
    return true;
  spi_bus_remove_device(spiHandle);
  spi_bus_free(spiHost);
  initialized = false;
  return true;
}

bool SpiBus::Write(const uint8_t *data, uint16_t sizeBytes,
                   uint32_t /*timeoutMsec*/) noexcept {
  if (!initialized)
    return false;
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = data;
  t.rx_buffer = nullptr;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  return (ret == ESP_OK);
}

bool SpiBus::Read(uint8_t *data, uint16_t sizeBytes,
                  uint32_t /*timeoutMsec*/) noexcept {
  if (!initialized)
    return false;
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = nullptr;
  t.rx_buffer = data;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  return (ret == ESP_OK);
}

bool SpiBus::WriteRead(const uint8_t *tx, uint8_t *rx, uint16_t sizeBytes,
                       uint32_t /*timeoutMsec*/) noexcept {
  if (!initialized)
    return false;
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = tx;
  t.rx_buffer = rx;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  return (ret == ESP_OK);
}

uint32_t SpiBus::GetClockHz() const noexcept {
  return devConfig.clock_speed_hz;
}
