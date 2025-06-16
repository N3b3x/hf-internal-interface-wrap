#ifndef DACOUTPUT_H
#define DACOUTPUT_H

#include "esp_idf_version.h"
#include <cstdint>

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
#include <driver/dac_oneshot.h>
#include <driver/dac_types.h>
#else
#include <driver/dac_common.h>
#endif

/**
 * @file DacOutput.h
 * @brief Thin wrapper for the ESP-IDF DAC driver.
 */
class DacOutput {
public:
  explicit DacOutput(dac_channel_t channel) noexcept;
  ~DacOutput() noexcept;
  DacOutput(const DacOutput &) = delete;
  DacOutput &operator=(const DacOutput &) = delete;

  bool Enable() noexcept;                ///< Enable the channel
  bool Disable() noexcept;               ///< Disable the channel
  bool SetValue(uint8_t value) noexcept; ///< Output 8-bit value

  bool IsEnabled() const noexcept {
    return enabled;
  }

private:
  dac_channel_t channel;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
  dac_oneshot_handle_t handle;
#endif
  bool enabled;
};

#endif // DACOUTPUT_H
