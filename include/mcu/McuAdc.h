/**
 * @file McuAdc.h
 * @brief Platform-agnostic MCU ADC driver interface.
 *
 * This class provides an implementation of BaseAdc that automatically
 * adapts to the current MCU platform. Currently supports ESP32-C6,
 * but designed to be easily portable to other MCUs.
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */
#ifndef MCU_ADC_H_
#define MCU_ADC_H_

#include "BaseAdc.h"
#include "McuTypes.h"
#include <cstdint>

/**
 * @class McuAdc
 * @brief Platform-agnostic MCU ADC driver.
 * 
 * This class provides a unified interface for ADC operations that automatically
 * adapts to the current MCU platform. All MCU-specific details are handled
 * internally through McuTypes.h definitions.
 */
class McuAdc : public BaseAdc {
public:
    /**
     * @brief Constructor for MCU ADC driver.
     * 
     * @param adc_unit ADC unit identifier (platform-agnostic)
     * @param attenuation ADC attenuation level (platform-specific, mapped internally)
     * @param width ADC bit width (defaults to 12-bit)
     * 
     * @note Parameters are abstracted through hf_* types for platform independence
     */
    McuAdc(hf_adc_unit_t adc_unit, uint32_t attenuation, hf_adc_resolution_t width = hf_adc_resolution_t::Bits12);

    /**
     * @brief Destructor.
     */
    ~McuAdc() noexcept override;

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
     * @return Maximum channel count
     */
    uint8_t GetMaxChannels() const noexcept override;

    /**
     * @brief Check if a specific channel is available.
     * @param channel_num Channel number to check
     * @return True if channel is available, false otherwise
     */
    bool IsChannelAvailable(uint8_t channel_num) const noexcept override;

    /**
     * @brief Read raw ADC count from specified channel.
     * 
     * @param channel_num ADC channel number
     * @param channel_reading_count Output: raw ADC count
     * @param numOfSamplesToAvg Number of samples to average (default: 1)
     * @param timeBetweenSamples Time between samples in microseconds (default: 0)
     * @return HfAdcErr::ADC_OK if successful, error code otherwise
     */
    HfAdcErr ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                             uint8_t numOfSamplesToAvg = 1,
                             uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Read voltage from specified channel.
     * 
     * @param channel_num ADC channel number
     * @param channel_reading_v Output: voltage reading in volts
     * @param numOfSamplesToAvg Number of samples to average (default: 1)
     * @param timeBetweenSamples Time between samples in microseconds (default: 0)
     * @return HfAdcErr::ADC_OK if successful, error code otherwise
     */
    HfAdcErr ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                         uint8_t numOfSamplesToAvg = 1,
                         uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Read both raw count and voltage from specified channel.
     * 
     * @param channel_num ADC channel number
     * @param channel_reading_count Output: raw ADC count
     * @param channel_reading_v Output: voltage reading in volts
     * @param numOfSamplesToAvg Number of samples to average (default: 1)
     * @param timeBetweenSamples Time between samples in microseconds (default: 0)
     * @return HfAdcErr::ADC_OK if successful, error code otherwise
     */
    HfAdcErr ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                        float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                        uint32_t timeBetweenSamples = 0) noexcept override;

    /**
     * @brief Legacy utility: Read single sample from channel (voltage).
     * @param channel_num ADC channel number
     * @return Voltage reading in volts, or -1.0f on error
     */
    float ReadVoltage(uint8_t channel_num) noexcept {
        float voltage = -1.0f;
        ReadChannelV(channel_num, voltage);
        return voltage;
    }

    /**
     * @brief Legacy utility: Read single sample from channel (raw count).
     * @param channel_num ADC channel number
     * @return Raw ADC count, or 0 on error
     */
    uint32_t ReadRaw(uint8_t channel_num) noexcept {
        uint32_t count = 0;
        ReadChannelCount(channel_num, count);
        return count;
    }

    /**
     * @brief Legacy utility: Read averaged samples from channel.
     * @param channel_num ADC channel number
     * @param num_samples Number of samples to average
     * @return Averaged voltage reading in volts, or -1.0f on error
     */
    float ReadAveraged(uint8_t channel_num, uint8_t num_samples) noexcept {
        float voltage = -1.0f;
        ReadChannelV(channel_num, voltage, num_samples);
        return voltage;
    }

    /**
     * @brief Legacy utility: Convert raw ADC count to voltage using calibration.
     * @param raw_count Raw ADC count
     * @return Voltage in volts
     */
    float CountToVoltage(uint32_t raw_count) const noexcept;

    /**
     * @brief Legacy utility: Get ADC resolution in bits.
     * @return Resolution in bits
     */
    uint8_t GetResolutionBits() const noexcept;

    /**
     * @brief Legacy utility: Get maximum ADC count value.
     * @return Maximum count value based on resolution
     */
    uint32_t GetMaxCount() const noexcept;

    /**
     * @brief Legacy utility: Get reference voltage.
     * @return Reference voltage in volts
     */
    float GetReferenceVoltage() const noexcept;

    /**
     * @brief Legacy utility: Check if calibration is enabled.
     * @return true if calibration is enabled
     */
    bool IsCalibrationEnabled() const noexcept { return cali_enable_; }

    //==============================================//
    // ADVANCED FEATURES IMPLEMENTATION            //
    //==============================================//

    /**
     * @brief Configure advanced ADC features (DMA, triggering)
     * @param channel_num Channel to configure
     * @param config Advanced configuration
     * @return HfAdcErr error code
     */
    HfAdcErr ConfigureAdvanced(uint8_t channel_num, 
                               const AdcAdvancedConfig& config) noexcept override;

    /**
     * @brief Start continuous/DMA sampling with callback
     * @param channel_num Channel to sample
     * @param callback Callback for sample data
     * @param user_data User data for callback
     * @return HfAdcErr error code
     */
    HfAdcErr StartContinuousSampling(uint8_t channel_num,
                                     AdcCallback callback,
                                     void* user_data = nullptr) noexcept override;

    /**
     * @brief Stop continuous/DMA sampling
     * @param channel_num Channel to stop
     * @return HfAdcErr error code
     */
    HfAdcErr StopContinuousSampling(uint8_t channel_num) noexcept override;    //==============================================//
    // CALIBRATION SUPPORT                         //
    //==============================================//

    /**
     * @brief Perform calibration for a channel
     * @param channel_num Channel to calibrate
     * @param config Calibration configuration
     * @param progress_callback Optional progress callback
     * @param user_data User data for callback
     * @return HfAdcErr error code
     */
    HfAdcErr CalibrateChannel(uint8_t channel_num,
                              const CalibrationConfig& config,
                              CalibrationProgressCallback progress_callback = nullptr,
                              void* user_data = nullptr) noexcept override;

    /**
     * @brief Save calibration data to non-volatile storage
     * @param channel_num Channel number
     * @return HfAdcErr error code
     */
    HfAdcErr SaveCalibration(uint8_t channel_num) noexcept override;

    /**
     * @brief Load calibration data from non-volatile storage
     * @param channel_num Channel number
     * @return HfAdcErr error code
     */
    HfAdcErr LoadCalibration(uint8_t channel_num) noexcept override;

    /**
     * @brief Verify current calibration accuracy
     * @param channel_num Channel to verify
     * @param reference_voltage Known reference voltage
     * @param measured_voltage Output: measured voltage
     * @param error_percent Output: percentage error
     * @return HfAdcErr error code
     */
    HfAdcErr VerifyCalibration(uint8_t channel_num, float reference_voltage, 
                               float& measured_voltage, float& error_percent) noexcept override;

private:
    // Platform-agnostic private members using hf_* types
    hf_adc_unit_t unit_;
    uint32_t attenuation_;
    hf_adc_resolution_t bitwidth_;
    void* adc_handle_;              // Platform-specific handle (oneshot)
    void* cali_handle_;             // Platform-specific calibration handle
    bool cali_enable_;
    bool initialized_;

    // DMA and continuous conversion support
    void* adc_continuous_handle_;   // Platform-specific continuous handle
    bool dma_mode_active_;
    AdcCallback active_callback_;
    void* callback_user_data_;
    void* dma_task_handle_;         // Platform-specific task handle
    
    // DMA buffer management
    static constexpr size_t DMA_BUFFER_SIZE = 1024;
    uint8_t* dma_buffer_;
    uint8_t current_dma_channel_;

    /**
     * @brief Initialize ADC calibration (platform-specific implementation).
     * @return True if successful, false otherwise
     */
    bool InitializeCalibration() noexcept;

    /**
     * @brief Deinitialize ADC calibration (platform-specific implementation).
     */
    void DeinitializeCalibration() noexcept;

    /**
     * @brief Initialize continuous/DMA mode (ESP32C6 specific).
     * @param channel_num Channel to configure
     * @param sample_rate_hz Desired sample rate
     * @return HfAdcErr error code
     */
    HfAdcErr InitializeContinuousMode(uint8_t channel_num, uint32_t sample_rate_hz) noexcept;

    /**
     * @brief Deinitialize continuous/DMA mode.
     * @return HfAdcErr error code
     */
    HfAdcErr DeinitializeContinuousMode() noexcept;

    /**
     * @brief DMA processing task (static function for FreeRTOS).
     * @param pvParameters Pointer to McuAdc instance
     */
    static void DmaProcessingTask(void* pvParameters);

    /**
     * @brief Process DMA data buffer and invoke callback.
     * @param buffer Raw DMA buffer data
     * @param length Buffer length in bytes
     */
    void ProcessDmaData(uint8_t* buffer, size_t length) noexcept;

    /**
     * @brief Convert ADC channel number to platform-specific channel.
     * @param channel_num Generic channel number
     * @return Platform-specific channel identifier
     */
    hf_adc_channel_t GetMcuChannel(uint8_t channel_num) const noexcept;
};

#endif // MCU_ADC_H_
