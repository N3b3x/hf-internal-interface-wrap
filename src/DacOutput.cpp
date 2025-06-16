/**
 * @file DacOutput.cpp
 * @brief Implementation of the DacOutput class.
 */

#include "DacOutput.h"
#include "esp_idf_version.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
#include <driver/dac_oneshot.h>
#else
#include <driver/dac_output.h>
#endif
#include <esp_log.h>

static const char *TAG = "DacOutput";

DacOutput::DacOutput(dac_channel_t ch) noexcept
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    : channel(ch), handle(nullptr), enabled(false){}
#else
    : channel(ch), enabled(false) {
}
#endif

      DacOutput::~DacOutput() noexcept {
  if (enabled) {
    Disable();
  }
}

bool DacOutput::Enable() noexcept {
  if (enabled)
    return true;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  dac_oneshot_config_t cfg{.chan_id = channel};
  esp_err_t err = dac_oneshot_new_channel(&cfg, &handle);
#else
  esp_err_t err = dac_output_enable(channel);
#endif
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
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  esp_err_t err = dac_oneshot_del_channel(handle);
#else
  esp_err_t err = dac_output_disable(channel);
#endif
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "disable failed: %d", err);
    return false;
  }
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  handle = nullptr;
#endif
  enabled = false;
  return true;
}

bool DacOutput::SetValue(uint8_t value) noexcept {
  if (!enabled)
    return false;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  esp_err_t err = dac_oneshot_output_voltage(handle, value);
#else
  esp_err_t err = dac_output_voltage(channel, value);
#endif
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "set value failed: %d", err);
    return false;
  }
  return true;
}
