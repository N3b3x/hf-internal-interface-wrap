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
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <unistd.h>

Esp32C6Adc::Esp32C6Adc(adc_unit_t adc_unit, adc_atten_t attenuation,
                       adc_bits_width_t width)
    : adcUnit(adc_unit), attenuation(attenuation), width(width),
      initialized(false) {}

Esp32C6Adc::~Esp32C6Adc() noexcept {}

bool Esp32C6Adc::Initialize() noexcept {
  // For ESP32, ADC configuration is mostly static, but width/attenuation can be
  // set per channel. This function can be extended for more advanced setups.
  initialized = true;
  return true;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannelCount(
    uint8_t channel_num, uint32_t &channel_reading_count,
    uint8_t numOfSamplesToAvg, uint32_t timeBetweenSamples) noexcept {
  if (!EnsureInitialized())
    return AdcErr::ADC_ERR_FAILURE;
  uint32_t sum = 0;
  for (uint8_t i = 0; i < numOfSamplesToAvg; ++i) {
    int raw = 0;
    if (adcUnit == ADC_UNIT_1) {
      adc1_config_width(width);
      adc1_config_channel_atten((adc1_channel_t)channel_num, attenuation);
      raw = adc1_get_raw((adc1_channel_t)channel_num);
    } else {
      // Only ADC1 supported in ESP32-C6 for now
      return AdcErr::ADC_ERR_CHANNEL_NOT_FOUND;
    }
    sum += raw;
    if (timeBetweenSamples > 0)
      usleep(timeBetweenSamples * 1000);
  }
  channel_reading_count = sum / numOfSamplesToAvg;
  return AdcErr::ADC_SUCCESS;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannelV(uint8_t channel_num,
                                         float &channel_reading_v,
                                         uint8_t numOfSamplesToAvg,
                                         uint32_t timeBetweenSamples) noexcept {
  uint32_t count = 0;
  auto err = ReadChannelCount(channel_num, count, numOfSamplesToAvg,
                              timeBetweenSamples);
  if (err != AdcErr::ADC_SUCCESS)
    return err;
  // Use ESP-IDF calibration API if available
  esp_adc_cal_characteristics_t characteristics;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      adcUnit, attenuation, width, 0, &characteristics);
  channel_reading_v = esp_adc_cal_raw_to_voltage(count, &characteristics) /
                      1000.0f; // Convert mV to V
  return AdcErr::ADC_SUCCESS;
}

BaseAdc::AdcErr Esp32C6Adc::ReadChannel(uint8_t channel_num,
                                        uint32_t &channel_reading_count,
                                        float &channel_reading_v,
                                        uint8_t numOfSamplesToAvg,
                                        uint32_t timeBetweenSamples) noexcept {
  auto err = ReadChannelCount(channel_num, channel_reading_count,
                              numOfSamplesToAvg, timeBetweenSamples);
  if (err != AdcErr::ADC_SUCCESS)
    return err;
  return ReadChannelV(channel_num, channel_reading_v, numOfSamplesToAvg,
                      timeBetweenSamples);
}
