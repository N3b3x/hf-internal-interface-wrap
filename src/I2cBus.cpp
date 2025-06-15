/**
 * @file I2cBus.cpp
 * @brief Implementation of the I2cBus class for ESP32-C6.
 */

#include "I2cBus.h"
#include <esp_log.h>

static const char *TAG = "I2cBus";

I2cBus::I2cBus(i2c_port_t port, const i2c_config_t &cfg) noexcept
    : i2cPort(port), config(cfg), initialized(false) {}

I2cBus::~I2cBus() noexcept {
  if (initialized) {
    Close();
  }
}

bool I2cBus::Open() noexcept {
  if (initialized)
    return true;

  esp_err_t ret = i2c_param_config(i2cPort, &config);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "param_config failed: %d", ret);
    return false;
  }
  ret = i2c_driver_install(i2cPort, config.mode, 0, 0, 0);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "driver_install failed: %d", ret);
    return false;
  }
  initialized = true;
  return true;
}

bool I2cBus::Close() noexcept {
  if (!initialized)
    return true;
  i2c_driver_delete(i2cPort);
  initialized = false;
  return true;
}

bool I2cBus::Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
                   uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  esp_err_t ret =
      i2c_master_write_to_device(i2cPort, addr, data, sizeBytes, pdMS_TO_TICKS(timeoutMsec));
  return ret == ESP_OK;
}

bool I2cBus::Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  esp_err_t ret =
      i2c_master_read_from_device(i2cPort, addr, data, sizeBytes, pdMS_TO_TICKS(timeoutMsec));
  return ret == ESP_OK;
}

bool I2cBus::WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                       uint16_t rxSizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  esp_err_t ret = i2c_master_write_read_device(i2cPort, addr, txData, txSizeBytes, rxData,
                                               rxSizeBytes, pdMS_TO_TICKS(timeoutMsec));
  return ret == ESP_OK;
}

uint32_t I2cBus::GetClockHz() const noexcept {
  return config.master.clk_speed;
}
