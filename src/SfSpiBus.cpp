/**
 * @file SfSpiBus.cpp
 * @brief Implementation of the SfSpiBus class for ESP32 (ESP-IDF), providing
 * SPI master communication with thread safety and software-controlled CS.
 *
 * This class abstracts the ESP-IDF SPI master driver and provides thread-safe
 * SPI transactions using FreeRTOS mutexes. It allows for software-controlled
 * chip select (CS) pin, supporting multi-device SPI buses.
 *
 * @author Nebula Tech Corporation
 * @copyright Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 */

#include "SfSpiBus.h"
#include <cstring>
#include <esp_log.h>

static const char *TAG = "SfSpiBus";

SfSpiBus::SfSpiBus(spi_host_device_t host, const spi_bus_config_t &buscfg,
                   const spi_device_interface_config_t &devcfg,
                   SemaphoreHandle_t mutexHandle) noexcept
    : spiHost(host), spiHandle(nullptr), busConfig(buscfg), devConfig(devcfg),
      busMutex(mutexHandle), initialized(false),
      csPin((gpio_num_t)devcfg.spics_io_num) {
  // No additional initialization required here.
}

SfSpiBus::~SfSpiBus() noexcept {
  if (initialized) {
    Close();
  }
}

bool SfSpiBus::Open() noexcept {
  if (initialized)
    return true;
  esp_err_t ret = spi_bus_initialize(spiHost, &busConfig, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %d", ret);
    return false;
  }
  spi_device_interface_config_t devcfgCopy = devConfig;
  devcfgCopy.spics_io_num = -1; // Software-controlled CS
  ret = spi_bus_add_device(spiHost, &devcfgCopy, &spiHandle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add SPI device: %d", ret);
    return false;
  }
  gpio_set_direction(csPin, GPIO_MODE_OUTPUT);
  gpio_set_level(csPin, 1); // Inactive (high)
  initialized = true;
  return true;
}

bool SfSpiBus::Close() noexcept {
  if (!initialized)
    return true;
  spi_bus_remove_device(spiHandle);
  spi_bus_free(spiHost);
  initialized = false;
  return true;
}

bool SfSpiBus::Write(const uint8_t *data, uint16_t sizeBytes,
                     uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(busMutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  SelectDevice();
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = data;
  t.rx_buffer = nullptr;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  DeselectDevice();
  xSemaphoreGive(busMutex);
  return (ret == ESP_OK);
}

bool SfSpiBus::Read(uint8_t *data, uint16_t sizeBytes,
                    uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(busMutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  SelectDevice();
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = nullptr;
  t.rx_buffer = data;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  DeselectDevice();
  xSemaphoreGive(busMutex);
  return (ret == ESP_OK);
}

bool SfSpiBus::WriteRead(const uint8_t *write_data, uint8_t *read_data,
                         uint16_t sizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(busMutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  SelectDevice();
  spi_transaction_t t = {};
  t.length = sizeBytes * 8;
  t.tx_buffer = write_data;
  t.rx_buffer = read_data;
  esp_err_t ret = spi_device_transmit(spiHandle, &t);
  DeselectDevice();
  xSemaphoreGive(busMutex);
  return (ret == ESP_OK);
}

bool SfSpiBus::LockBus(uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  return (xSemaphoreTake(busMutex, pdMS_TO_TICKS(timeoutMsec)) == pdTRUE);
}

bool SfSpiBus::UnlockBus() noexcept {
  if (!initialized)
    return false;
  return (xSemaphoreGive(busMutex) == pdTRUE);
}

uint32_t SfSpiBus::GetClockHz() const noexcept {
  return devConfig.clock_speed_hz;
}

bool SfSpiBus::SelectDevice() noexcept {
  gpio_set_level(csPin, 0);
  return true;
}

bool SfSpiBus::DeselectDevice() noexcept {
  gpio_set_level(csPin, 1);
  return true;
}
