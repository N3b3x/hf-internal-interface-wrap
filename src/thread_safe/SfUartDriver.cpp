/**
 * @file SfUartDriver.cpp
 * @brief Implementation of the SfUartDriver class.
 */

#include "../thread_safe/SfUartDriver.h"

static const char *TAG = "SfUartDriver";

SfUartDriver::SfUartDriver(hf_uart_port_t p, const hf_uart_config_t &cfg, hf_gpio_num_t tx, hf_gpio_num_t rx,
                           hf_semaphore_handle_t mtx) noexcept
    : port(p), config(cfg), txPin(tx), rxPin(rx), mutex(mtx), initialized(false) {}

SfUartDriver::~SfUartDriver() noexcept {
  if (initialized) {
    Close();
  }
}

bool SfUartDriver::Open() noexcept {
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

bool SfUartDriver::Close() noexcept {
  if (!initialized)
    return true;
  uart_driver_delete(port);
  initialized = false;
  return true;
}

bool SfUartDriver::Write(const uint8_t *data, uint16_t length, uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  int ret = uart_write_bytes(port, data, length);
  xSemaphoreGive(mutex);
  return ret == length;
}

bool SfUartDriver::Read(uint8_t *data, uint16_t length, TickType_t ticksToWait) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(mutex, ticksToWait) != pdTRUE)
    return false;
  int ret = uart_read_bytes(port, data, length, ticksToWait);
  xSemaphoreGive(mutex);
  return ret == length;
}

bool SfUartDriver::Lock() noexcept {
  if (!initialized)
    return false;
  return xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE;
}

bool SfUartDriver::Unlock() noexcept {
  if (!initialized)
    return false;
  return xSemaphoreGive(mutex) == pdTRUE;
}
