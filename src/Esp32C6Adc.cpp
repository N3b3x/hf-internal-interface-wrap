/**
 * @file Esp32C6Adc.cpp
 * @brief Implementation of Esp32C6Adc for ESP32-C6 using ESP-IDF.
 *
 * This file provides the implementation for ADC abstraction on ESP32-C6.
 *
 * @note This implementation is intended for use with ESP-IDF. Pin configuration
 * is handled via gpio_config().
 */
#include "Esp32C6Adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include <unistd.h>

Esp32C6Adc::Esp32C6Adc(adc_unit_t adc_unit, adc_atten_t attenuation, adc_bitwidth_t width)
    : adcUnit(adc_unit), attenuation(attenuation), width(width), initialized(false),
      adc_handle(nullptr), cali_handle(nullptr) {}

Esp32C6Adc::~Esp32C6Adc() noexcept {
  if (cali_handle) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(cali_handle);
#endif
  }
  if (adc_handle) {
    adc_oneshot_del_unit(adc_handle);
  }
}

bool Esp32C6Adc::Initialize() noexcept {
  if (initialized) {
    return true;
  }

  // Configure ADC oneshot unit
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = adcUnit,
  };
  esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc_handle);
  if (ret != ESP_OK) {
    return false;
  }

  // Configure ADC calibration
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = adcUnit,
      .atten = attenuation,
      .bitwidth = width,
  };
  ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = adcUnit,
      .atten = attenuation,
      .bitwidth = width,
  };
  ret = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
#endif

  if (ret != ESP_OK) {
    // Calibration failed, but we can still use raw readings
    cali_handle = nullptr;
  }

  initialized = true;
  return true;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                                             uint8_t numOfSamplesToAvg,
                                             uint32_t timeBetweenSamples) noexcept {
  if (!EnsureInitialized())
    return AdcErr::ADC_ERR_FAILURE;

  // Configure channel
  adc_oneshot_chan_cfg_t config = {
      .atten = attenuation,
      .bitwidth = width,
  };
  esp_err_t ret = adc_oneshot_config_channel(adc_handle, (adc_channel_t)channel_num, &config);
  if (ret != ESP_OK) {
    return AdcErr::ADC_ERR_CHANNEL_NOT_FOUND;
  }

  uint32_t sum = 0;
  for (uint8_t i = 0; i < numOfSamplesToAvg; ++i) {
    int raw = 0;
    ret = adc_oneshot_read(adc_handle, (adc_channel_t)channel_num, &raw);
    if (ret != ESP_OK) {
      return AdcErr::ADC_ERR_CHANNEL_READ_ERR;
    }
    sum += raw;
    if (timeBetweenSamples > 0)
      usleep(timeBetweenSamples * 1000);
  }
  channel_reading_count = sum / numOfSamplesToAvg;
  return AdcErr::ADC_SUCCESS;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                                         uint8_t numOfSamplesToAvg,
                                         uint32_t timeBetweenSamples) noexcept {
  uint32_t count = 0;
  auto err = ReadChannelCount(channel_num, count, numOfSamplesToAvg, timeBetweenSamples);
  if (err != AdcErr::ADC_SUCCESS)
    return err;

  // Use calibration if available
  if (cali_handle) {
    int voltage_mv = 0;
    esp_err_t ret = adc_cali_raw_to_voltage(cali_handle, count, &voltage_mv);
    if (ret == ESP_OK) {
      channel_reading_v = voltage_mv / 1000.0f; // Convert mV to V
      return AdcErr::ADC_SUCCESS;
    }
  }

  // Fallback to simple linear conversion if calibration is not available
  // This is a rough approximation for ESP32-C6 with 3.3V reference
  // For 12-bit ADC: 4096 counts = 3.3V (assuming DB_11 attenuation)
  float max_voltage = 3.3f;
  if (attenuation == ADC_ATTEN_DB_0) {
    max_voltage = 1.1f;
  } else if (attenuation == ADC_ATTEN_DB_2_5) {
    max_voltage = 1.5f;
  } else if (attenuation == ADC_ATTEN_DB_6) {
    max_voltage = 2.2f;
  } else if (attenuation == ADC_ATTEN_DB_12) {
    max_voltage = 3.9f;
  }

  uint32_t max_count = (1 << 12); // Assuming 12-bit ADC
  if (width == ADC_BITWIDTH_9) {
    max_count = (1 << 9);
  } else if (width == ADC_BITWIDTH_10) {
    max_count = (1 << 10);
  } else if (width == ADC_BITWIDTH_11) {
    max_count = (1 << 11);
  }

  channel_reading_v = (count * max_voltage) / max_count;
  return AdcErr::ADC_SUCCESS;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                                        float &channel_reading_v, uint8_t numOfSamplesToAvg,
                                        uint32_t timeBetweenSamples) noexcept {
  auto err =
      ReadChannelCount(channel_num, channel_reading_count, numOfSamplesToAvg, timeBetweenSamples);
  if (err != AdcErr::ADC_SUCCESS)
    return err;
  return ReadChannelV(channel_num, channel_reading_v, numOfSamplesToAvg, timeBetweenSamples);
}
