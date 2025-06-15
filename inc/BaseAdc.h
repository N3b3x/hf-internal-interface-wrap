/**

  *  Contains the declaration of the abstract Adc class, which provides features
  common to
  *    ADC classes.  Adc's derived classes are intended to employ lazy
  initialization;
  *    they are initialized the first time the pin in manipulated.
  *
  *  Note:  These functions are not thread or interrupt-safe and should be
  called
  *          called with appropriate guards if used within an ISR or shared
  between tasks.
  -------------------------------------------------------------------------------------**/

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_

#include "driver/adc.h"
#include <cstdint>

/**
 * @class BaseAdc
 * @brief Base class for ADCs on ESP32-C6 (ESP-IDF).
 */
class BaseAdc {
public:
  /**
   * @brief ADC error codes
   */
  enum class AdcErr {
    ADC_SUCCESS,
    ADC_ERR_FAILURE,
    ADC_ERR_CHANNEL_NOT_FOUND,
    ADC_ERR_CHANNEL_NOT_ENABLED,
    ADC_ERR_CHANNEL_READ_ERR,
    ADC_ERR_INVALID_SAMPLE_COUNT,
  };

  /**
   * @brief Constructor
   */
  BaseAdc() : initialized(false) {}

  /**
   * @brief Virtual destructor
   */
  virtual ~BaseAdc() {}

  /**
   * @brief Ensures that the ADC is initialized (lazy initialization).
   * @return true if the ADC is initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized) {
      initialized = Initialize();
    }
    return initialized;
  }

  /**
   * @brief Checks if the class is initialized.
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept {
    return initialized;
  }

  /**
   * @brief Initializes the ADC peripheral (must be implemented by derived
   * classes).
   * @return True if the initialization is successful, false otherwise.
   */
  virtual bool Initialize() noexcept = 0;

  // PURE VIRTUAL FUNCTIONS, MUST BE OVERRIDDEN
  virtual AdcErr ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                              uint8_t numOfSamplesToAvg = 1,
                              uint32_t timeBetweenSamples = 0) noexcept = 0;
  virtual AdcErr ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                                  uint8_t numOfSamplesToAvg = 1,
                                  uint32_t timeBetweenSamples = 0) noexcept = 0;
  virtual AdcErr ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                             float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                             uint32_t timeBetweenSamples = 0) noexcept = 0;

protected:
  bool initialized;
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_ */
