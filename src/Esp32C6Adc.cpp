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
    : BaseAdc(), adcUnit_(adc_unit), attenuation_(attenuation), width_(width),
      adc_handle_(nullptr), cali_handle_(nullptr) {
    // ESP32-C6 only supports ADC_UNIT_1
    if (adc_unit != ADC_UNIT_1) {
        // Log warning or handle error as needed
        adcUnit_ = ADC_UNIT_1;  // Force to ADC1
    }
}

Esp32C6Adc::~Esp32C6Adc() noexcept {
    Deinitialize();
}

bool Esp32C6Adc::Initialize() noexcept {
    if (IsInitialized()) {
        return true;
    }

    // Configure ADC oneshot unit
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = adcUnit_,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle_);
    if (ret != ESP_OK) {
        return false;
    }

    // Configure ADC calibration
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = adcUnit_,
        .chan = ADC_CHANNEL_0,
        .atten = attenuation_,
        .bitwidth = width_
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = adcUnit_,
        .chan = ADC_CHANNEL_0,
        .atten = attenuation_,
        .bitwidth = width_
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle_);
#endif

    if (ret != ESP_OK) {
        // Calibration failed, but we can still use raw readings
        cali_handle_ = nullptr;
    }

    return BaseAdc::Initialize();  // Call parent to set initialized flag
}

bool Esp32C6Adc::Deinitialize() noexcept {
    if (cali_handle_) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_delete_scheme_curve_fitting(cali_handle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_delete_scheme_line_fitting(cali_handle_);
#endif
        cali_handle_ = nullptr;
    }
    
    if (adc_handle_) {
        adc_oneshot_del_unit(adc_handle_);
        adc_handle_ = nullptr;
    }
    
    return BaseAdc::Deinitialize();  // Call parent to clear initialized flag
}

uint8_t Esp32C6Adc::GetMaxChannels() const noexcept {
    return ESP32_C6_ADC1_MAX_CHANNELS;
}

bool Esp32C6Adc::IsChannelAvailable(uint8_t channel_num) const noexcept {
    // ESP32-C6 ADC1 supports channels 0-6
    return channel_num < ESP32_C6_ADC1_MAX_CHANNELS;
}

HfAdcErr Esp32C6Adc::ValidateChannel(uint8_t channel_num) const noexcept {
    HfAdcErr baseValidation = ValidateReadParameters(channel_num, 1);
    if (baseValidation != HfAdcErr::ADC_SUCCESS) {
        return baseValidation;
    }
    
    if (!IsChannelAvailable(channel_num)) {
        return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
    }
    
    return HfAdcErr::ADC_SUCCESS;
}

HfAdcErr Esp32C6Adc::ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                                      uint8_t numOfSamplesToAvg,
                                      uint32_t timeBetweenSamples) noexcept {
    HfAdcErr validation = ValidateChannel(channel_num);
    if (validation != HfAdcErr::ADC_SUCCESS) {
        return validation;
    }

    if (numOfSamplesToAvg == 0) {
        return HfAdcErr::ADC_ERR_INVALID_SAMPLE_COUNT;
    }

    // Configure channel
    adc_oneshot_chan_cfg_t config = {
        .atten = attenuation_,
        .bitwidth = width_,
    };
    
    esp_err_t ret = adc_oneshot_config_channel(adc_handle_, (adc_channel_t)channel_num, &config);
    if (ret != ESP_OK) {
        return HfAdcErr::ADC_ERR_CHANNEL_NOT_CONFIGURED;
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < numOfSamplesToAvg; ++i) {
        int raw = 0;
        ret = adc_oneshot_read(adc_handle_, (adc_channel_t)channel_num, &raw);
        if (ret != ESP_OK) {
            return HfAdcErr::ADC_ERR_CHANNEL_READ_ERR;
        }
        sum += static_cast<uint32_t>(raw);
        
        if (i < numOfSamplesToAvg - 1 && timeBetweenSamples > 0) {
            usleep(timeBetweenSamples * 1000);  // Convert ms to us
        }
    }
    
    channel_reading_count = sum / numOfSamplesToAvg;
    return HfAdcErr::ADC_SUCCESS;
}

HfAdcErr Esp32C6Adc::ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                                  uint8_t numOfSamplesToAvg,
                                  uint32_t timeBetweenSamples) noexcept {
    uint32_t raw_reading = 0;
    HfAdcErr result = ReadChannelCount(channel_num, raw_reading, numOfSamplesToAvg, timeBetweenSamples);
    
    if (result != HfAdcErr::ADC_SUCCESS) {
        return result;
    }

    // Convert raw reading to voltage
    if (cali_handle_) {
        int voltage_mv = 0;
        esp_err_t ret = adc_cali_raw_to_voltage(cali_handle_, static_cast<int>(raw_reading), &voltage_mv);
        if (ret == ESP_OK) {
            channel_reading_v = static_cast<float>(voltage_mv) / 1000.0f;  // Convert mV to V
            return HfAdcErr::ADC_SUCCESS;
        }
    }
    
    // Fallback to basic conversion if calibration is not available
    // This is a simplified conversion - actual values depend on attenuation
    const float VREF = 3.3f;  // Reference voltage for ESP32-C6
    const uint32_t MAX_ADC_VALUE = (1 << static_cast<int>(width_)) - 1;  // 2^bits - 1
    
    channel_reading_v = (static_cast<float>(raw_reading) / static_cast<float>(MAX_ADC_VALUE)) * VREF;
    return HfAdcErr::ADC_SUCCESS;
}

HfAdcErr Esp32C6Adc::ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                                 float &channel_reading_v, uint8_t numOfSamplesToAvg,
                                 uint32_t timeBetweenSamples) noexcept {
    // Read the raw count first
    HfAdcErr result = ReadChannelCount(channel_num, channel_reading_count, numOfSamplesToAvg, timeBetweenSamples);
    if (result != HfAdcErr::ADC_SUCCESS) {
        return result;
    }    // Convert to voltage using the raw reading we just obtained
    if (cali_handle_) {
        int voltage_mv = 0;
        esp_err_t ret = adc_cali_raw_to_voltage(cali_handle_, static_cast<int>(channel_reading_count), &voltage_mv);
        if (ret == ESP_OK) {
            channel_reading_v = static_cast<float>(voltage_mv) / 1000.0f;  // Convert mV to V
            return HfAdcErr::ADC_SUCCESS;
        }
    }
    
    // Fallback to basic conversion
    const float VREF = 3.3f;  // Reference voltage for ESP32-C6
    const uint32_t MAX_ADC_VALUE = (1 << static_cast<int>(width_)) - 1;  // 2^bits - 1
    
    channel_reading_v = (static_cast<float>(channel_reading_count) / static_cast<float>(MAX_ADC_VALUE)) * VREF;
    return HfAdcErr::ADC_SUCCESS;
}
