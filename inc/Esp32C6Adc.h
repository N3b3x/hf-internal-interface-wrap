/**
 * @file Esp32C6Adc.h
 * @brief ADC driver class for ESP32-C6 using ESP-IDF.
 *
 * This class provides an implementation of BaseAdc for the ESP32-C6, using ESP-IDF's ADC APIs.
 *
 * @note This class is not thread-safe. Use appropriate synchronization if accessed from multiple contexts.
 */
#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_

#include <internal_interface_drivers/BaseAdc.h>
#include "driver/adc.h"
#include <cstdint>

class Esp32C6Adc : public BaseAdc {
public:
    /**
     * @brief Constructor for ESP32-C6 ADC driver.
     * @param adc_unit ADC unit (ADC_UNIT_1 or ADC_UNIT_2)
     * @param attenuation ADC attenuation (ADC_ATTEN_DB_0, etc.)
     * @param width ADC width (ADC_WIDTH_BIT_12, etc.)
     */
    Esp32C6Adc(adc_unit_t adc_unit, adc_atten_t attenuation, adc_bits_width_t width = ADC_WIDTH_BIT_12);

    /**
     * @brief Destructor.
     */
    ~Esp32C6Adc() noexcept;

    /**
     * @brief Initializes the ADC peripheral.
     * @return True if initialization is successful, false otherwise.
     */
    virtual bool Initialize() noexcept override;

    /**
     * @brief Reads the specified channel and returns the reading in volts.
     * @param channel_num Channel number to read (ADC1_CHANNEL_0, etc.)
     * @param channel_reading_v Variable to store the channel voltage reading.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    virtual AdcErr ReadChannelV(uint8_t channel_num, float& channel_reading_v, uint8_t numOfSamplesToAvg = 1, uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Reads the specified channel and returns the raw ADC count.
     * @param channel_num Channel number to read.
     * @param channel_reading_count Variable to store the raw ADC count.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    virtual AdcErr ReadChannelCount(uint8_t channel_num, uint32_t& channel_reading_count, uint8_t numOfSamplesToAvg = 1, uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Reads the specified channel and returns both raw count and voltage.
     * @param channel_num Channel number to read.
     * @param channel_reading_count Variable to store the raw ADC count.
     * @param channel_reading_v Variable to store the channel voltage reading.
     * @param numOfSamplesToAvg Number of samples to average (default is 1).
     * @param timeBetweenSamples Time between consecutive samples in ms (default is 0).
     * @return Error status indicating the success or failure of the operation.
     */
    virtual AdcErr ReadChannel(uint8_t channel_num, uint32_t& channel_reading_count, float& channel_reading_v, uint8_t numOfSamplesToAvg = 1, uint32_t timeBetweenSamples = 0) noexcept override;

private:
    adc_unit_t adcUnit;
    adc_atten_t attenuation;
    adc_bits_width_t width;
    bool initialized;
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_ESP32C6ADC_H_ */
