/**
 * @file DacOutput.cpp
 * @brief Implementation of the DacOutput class.
 */

#include "DacOutput.h"
#include <driver/dac_oneshot.h>
#include <esp_log.h>

static const char *TAG = "DacOutput";

DacOutput::DacOutput(dac_channel_t ch) noexcept : channel(ch), handle(nullptr), enabled(false) {}

DacOutput::~DacOutput() noexcept {
  if (enabled) {
    Disable();
  }
}

bool DacOutput::Enable() noexcept {
  if (enabled)
    return true;
  dac_oneshot_config_t cfg{.chan_id = channel};
  esp_err_t err = dac_oneshot_new_channel(&cfg, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "enable failed: %d", err);
    return false;
  }
  enabled = true;
  return true;
}

bool DacOutput::Disable() noexcept {
  if (!enabled)
    return true;
  esp_err_t err = dac_oneshot_del_channel(handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "disable failed: %d", err);
    return false;
  }
  handle = nullptr;
  enabled = false;
  return true;
}

bool DacOutput::SetValue(uint8_t value) noexcept {
  if (!enabled)
    return false;
  esp_err_t err = dac_oneshot_output_voltage(handle, value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "set value failed: %d", err);
    return false;
  }
  return true;
}
