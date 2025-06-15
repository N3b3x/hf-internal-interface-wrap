/**
 * @file UartDriver.cpp
 * @brief Implementation of the UartDriver class.
 */

#include "UartDriver.h"
#include <esp_log.h>

static const char *TAG = "UartDriver";

UartDriver::UartDriver(uart_port_t p, const uart_config_t &cfg, int tx, int rx) noexcept
    : port(p), config(cfg), txPin(tx), rxPin(rx), initialized(false) {}

UartDriver::~UartDriver() noexcept {
  if (initialized) {
    Close();
  }
}

bool UartDriver::Open() noexcept {
  if (initialized)
    return true;
  esp_err_t err = uart_param_config(port, &config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "param config failed: %d", err);
    return false;
  }
  err = uart_set_pin(port, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "set pin failed: %d", err);
    return false;
  }
  err = uart_driver_install(port, config.rx_flow_ctrl_thresh ? config.rx_flow_ctrl_thresh : 256,
                            config.rx_flow_ctrl_thresh ? config.rx_flow_ctrl_thresh : 256, 0,
                            nullptr, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "driver install failed: %d", err);
    return false;
  }
  initialized = true;
  return true;
}

bool UartDriver::Close() noexcept {
  if (!initialized)
    return true;
  uart_driver_delete(port);
  initialized = false;
  return true;
}

bool UartDriver::Write(const uint8_t *data, uint16_t length) noexcept {
  if (!initialized)
    return false;
  int ret = uart_write_bytes(port, data, length);
  return (ret == length);
}

bool UartDriver::Read(uint8_t *data, uint16_t length, TickType_t ticksToWait) noexcept {
  if (!initialized)
    return false;
  int ret = uart_read_bytes(port, data, length, ticksToWait);
  return (ret == length);
}
