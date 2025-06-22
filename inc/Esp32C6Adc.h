/**
 * @file Esp32C6Adc.h
 * @brief ADC driver class for ESP32-C6 using ESP-IDF.
 *
 * This class provides an implementation of BaseAdc for the ESP32-C6, using
 * ESP-IDF's ADC APIs.
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */
#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_

#include "BaseAdc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include <cstdint>

class Esp32C6Adc : public BaseAdc {
public:
    /**
     * @brief Constructor for ESP32-C6 ADC driver.
     * @param adc_unit ADC unit (only ADC_UNIT_1 is supported on ESP32-C6)
     * @param attenuation ADC attenuation (ADC_ATTEN_DB_0, etc.)
     * @param width ADC width (ADC_BITWIDTH_12, etc.)
     */
    Esp32C6Adc(adc_unit_t adc_unit, adc_atten_t attenuation, adc_bitwidth_t width = ADC_BITWIDTH_12);

    /**
     * @brief Destructor.
     */
    ~Esp32C6Adc() noexcept override;

    /**
     * @brief Initializes the ADC peripheral.
     * @return True if initialization is successful, false otherwise.
     */
    bool Initialize() noexcept override;

    /**
     * @brief Deinitializes the ADC peripheral.
     * @return True if deinitialization is successful, false otherwise.
     */
    bool Deinitialize() noexcept override;

    /**
     * @brief Get the maximum number of channels supported by this ADC.
     * @return Maximum channel count (7 for ESP32-C6 ADC1)
     */
    [[nodiscard]] uint8_t GetMaxChannels() const noexcept override;

    /**
     * @brief Check if a specific channel is available.
     * @param channel_num Channel number to check (0-6 for ESP32-C6 ADC1)
     * @return true if channel is available, false otherwise
     */
    [[nodiscard]] bool IsChannelAvailable(uint8_t channel_num) const noexcept override;

    /**
     * @brief Reads the specified channel and returns the reading in volts.
     * @param channel_num Channel number to read (0-6 for ESP32-C6 ADC1)
     * @param channel_reading_v Variable to store the channel voltage reading.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    HfAdcErr ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                          uint8_t numOfSamplesToAvg = 1,
                          uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Reads the specified channel and returns the raw ADC count.
     * @param channel_num Channel number to read (0-6 for ESP32-C6 ADC1).
     * @param channel_reading_count Variable to store the raw ADC count.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    HfAdcErr ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                              uint8_t numOfSamplesToAvg = 1,
                              uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Reads the specified channel and returns both raw count and voltage.
     * @param channel_num Channel number to read (0-6 for ESP32-C6 ADC1).
     * @param channel_reading_count Variable to store the raw ADC count.
     * @param channel_reading_v Variable to store the channel voltage reading.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    HfAdcErr ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                         float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                         uint32_t timeBetweenSamples = 0) noexcept override;

private:
    static constexpr uint8_t ESP32_C6_ADC1_MAX_CHANNELS = 7;  ///< ESP32-C6 ADC1 has channels 0-6

    adc_unit_t adcUnit_;
    adc_atten_t attenuation_;
    adc_bitwidth_t width_;
    adc_oneshot_unit_handle_t adc_handle_;
    adc_cali_handle_t cali_handle_;

    /**
     * @brief Validate channel number for ESP32-C6.
     * @param channel_num Channel number to validate.
     * @return HfAdcErr validation result.
     */
    [[nodiscard]] HfAdcErr ValidateChannel(uint8_t channel_num) const noexcept;
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_ */
