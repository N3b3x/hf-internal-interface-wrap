/**
 * @file McuAdc.h
 * @brief Advanced platform-agnostic MCU ADC driver with comprehensive ESP32C6/ESP-IDF v5.5+ features.
 *
 * This class provides a comprehensive implementation of BaseAdc.
 * On ESP32C6, it utilizes the latest ESP-IDF v5.5+ ADC capabilities including continuous
 * mode with DMA, advanced digital filters, real-time threshold monitors, intelligent
 * calibration systems, and hardware-accelerated operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class includes unified basic and advanced ADC functionality in a single interface.
 * @note Advanced features require ESP32C6 with ESP-IDF v5.5+ for full functionality.
 * @note Graceful feature degradation on older platforms and ESP-IDF versions.
 */

#pragma once

#include "BaseAdc.h"
#include "McuTypes_ADC.h"
#include "RtosMutex.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

// Callback function types
using AdcConversionCallback =
    std::function<void(uint8_t channel, uint32_t rawValue, float voltage, void *userData)>;
using AdcThresholdCallback = std::function<void(uint8_t monitorId, uint8_t channel, uint32_t value,
                                                bool isHigh, void *userData)>;
using AdcErrorCallback = std::function<void(HfAdcErr error, void *userData)>;

/**
 * @class McuAdc
 * @brief Advanced platform-agnostic MCU ADC driver with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This class provides a comprehensive implementation of ADC operations that automatically
 * adapts to the current MCU platform with support for both basic and advanced features.
 * On ESP32C6, it utilizes the latest ESP-IDF v5.5+ ADC features including continuous
 * mode sampling, digital filters, threshold monitors, and advanced calibration.
 *
 * Features:
 * - High-performance ADC with multiple sampling strategies
 * - Support for 9-12 bit resolution with configurable attenuation
 * - Advanced ESP32C6/ESP-IDF v5.5+ features:
 *   - Continuous mode sampling with DMA
 *   - Hardware IIR digital filters
 *   - ADC threshold monitors with interrupts
 *   - Advanced calibration with multiple schemes
 *   - Hardware oversampling
 *   - Multiple trigger sources (Timer, GPIO, PWM, External)
 *   - Power management & ULP mode support
 *   - Zero-crossing detection
 * - Thread-safe operation with mutex protection
 * - Comprehensive error handling and diagnostics
 * - Performance monitoring and statistics
 * - Temperature sensor integration
 * - Multi-channel support with individual configuration
 *
 * @note This class is thread-safe when used with multiple threads.
 * @note Advanced features require ESP32C6 with ESP-IDF v5.5+ for full functionality.
 */
class McuAdc : public BaseAdc {
public:
  /**
   * @brief Constructor for basic ADC functionality.
   */
  McuAdc() noexcept;

  /**
   * @brief Constructor with advanced configuration.
   * @param config Advanced ADC configuration
   */
  explicit McuAdc(const hf_adc_advanced_config_t &config) noexcept;

  /**
   * @brief Constructor with platform-specific parameters.
   * @param adc_unit ADC unit number
   * @param attenuation Attenuation setting  
   * @param width Resolution setting
   */
  McuAdc(hf_adc_unit_t adc_unit, uint32_t attenuation, hf_adc_resolution_t width) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~McuAdc() noexcept override;

  //==============================================================================
  // BASIC ADC OPERATIONS (BaseAdc Interface)
  //==============================================================================

  /**
   * @brief Initialize the ADC system.
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the ADC system.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Get the maximum number of channels supported.
   * @return Maximum channel count
   */
  uint8_t GetMaxChannels() const noexcept override;

  /**
   * @brief Check if a specific channel is available.
   * @param channel_id Channel ID to check (using BaseAdc interface)
   * @return true if available, false otherwise
   */
  [[nodiscard]] bool IsChannelAvailable(HfChannelId channel_id) const noexcept override;

  /**
   * @brief Read channel voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  HfAdcErr ReadChannelV(HfChannelId channel_id, float &channel_reading_v,
                        uint8_t numOfSamplesToAvg = 1,
                        HfTimeoutMs timeBetweenSamples = 0) noexcept override;

  /**
   * @brief Read channel count (raw ADC value).
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  HfAdcErr ReadChannelCount(HfChannelId channel_id, uint32_t &channel_reading_count,
                            uint8_t numOfSamplesToAvg = 1,
                            HfTimeoutMs timeBetweenSamples = 0) noexcept override;

  /**
   * @brief Read both channel count and voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  HfAdcErr ReadChannel(HfChannelId channel_id, uint32_t &channel_reading_count,
                       float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                       HfTimeoutMs timeBetweenSamples = 0) noexcept override;

  //==============================================//
  // ADVANCED FEATURES (OPTIONAL IMPLEMENTATIONS) //
  //==============================================//

  /**
   * @brief Read multiple channels simultaneously (BaseAdc override)
   * @param channel_ids Array of channel IDs
   * @param num_channels Number of channels
   * @param readings Array to store raw readings
   * @param voltages Array to store voltage readings
   * @return HfAdcErr error code
   */
  HfAdcErr ReadMultipleChannels(const HfChannelId *channel_ids, uint8_t num_channels,
                                uint32_t *readings, float *voltages) noexcept override;

  /**
   * @brief Start continuous/DMA sampling with callback (BaseAdc override)
   * @param channel_id Channel to sample
   * @param callback Callback for sample data
   * @param user_data User data for callback
   * @return HfAdcErr error code
   */
  HfAdcErr StartContinuousSampling(HfChannelId channel_id, AdcCallback callback,
                                   void *user_data = nullptr) noexcept override;

  /**
   * @brief Stop continuous/DMA sampling (BaseAdc override)
   * @param channel_id Channel to stop
   * @return HfAdcErr error code
   */
  HfAdcErr StopContinuousSampling(HfChannelId channel_id) noexcept override;

  /**
   * @brief Stop continuous sampling (convenience method).
   * @return HfAdcErr result code
   */
  HfAdcErr StopContinuousSampling() noexcept;

  //==============================================================================
  // CALIBRATION SUPPORT (BaseAdc overrides)
  //==============================================================================

  /**
   * @brief Perform ADC calibration for a specific channel
   * @param channel_id Channel to calibrate
   * @param config Calibration configuration
   * @param progress_callback Optional progress callback for long operations
   * @param user_data User data for progress callback
   * @return HfAdcErr error code
   */
  HfAdcErr CalibrateChannel(HfChannelId channel_id,
                            CalibrationProgressCallback progress_callback = nullptr,
                            void *user_data = nullptr) noexcept override;

  /**
   * @brief Perform automatic calibration using known reference voltages (BaseAdc override)
   * @param channel_id Channel to calibrate
   * @param reference_voltages Array of known reference voltages
   * @param num_references Number of reference voltages
   * @param calibration_type Type of calibration to perform
   * @return HfAdcErr error code
   */
  HfAdcErr AutoCalibrate(HfChannelId channel_id,
                          const float *reference_voltages,
                          uint8_t num_references) noexcept override;

  /**
   * @brief Save calibration data to non-volatile storage
   * @param channel_id Channel calibration to save
   * @return HfAdcErr error code
   */
  HfAdcErr SaveCalibration(HfChannelId channel_id) noexcept override;

  /**
   * @brief Load calibration data from non-volatile storage
   * @param channel_id Channel calibration to load
   * @return HfAdcErr error code
   */
  HfAdcErr LoadCalibration(HfChannelId channel_id) noexcept override;

  /**
   * @brief Clear/reset calibration for a channel (BaseAdc override)
   * @param channel_id Channel to reset
   * @return HfAdcErr error code
   */
  HfAdcErr ClearCalibration(HfChannelId channel_id) noexcept override;

  /**
   * @brief Verify calibration accuracy using known reference
   * @param channel_id Channel to verify
   * @param reference_voltage Known reference voltage to test
   * @param measured_voltage Output: voltage measured by ADC
   * @param error_percent Output: percentage error from reference
   * @return HfAdcErr error code
   */
  HfAdcErr VerifyCalibration(HfChannelId channel_id, float reference_voltage,
                             float &measured_voltage, float &error_percent) noexcept override;

  //==============================================================================
  // ADVANCED ADC OPERATIONS
  //==============================================================================

  /**
   * @brief Configure advanced ADC features (BaseAdc override)
   * @param channel_id Channel to configure
   * @param config Advanced configuration
   * @return HfAdcErr error code
   */
  HfAdcErr ConfigureAdvanced(HfChannelId channel_id, const hf_adc_advanced_config_t &config) noexcept;

  /**
   * @brief Initialize with advanced configuration.
   * @param config Advanced configuration parameters
   * @return HfAdcErr result code
   */
  HfAdcErr initializeAdvanced(const hf_adc_advanced_config_t &config) noexcept;

  /**
   * @brief Reconfigure ADC with new settings.
   * @param config New configuration parameters
   * @return HfAdcErr result code
   */
  HfAdcErr reconfigure(const hf_adc_advanced_config_t &config) noexcept;

  /**
   * @brief Get current ADC configuration.
   * @return Current configuration
   */
  hf_adc_advanced_config_t getCurrentConfiguration() const noexcept;

  //==============================================================================
  // DIGITAL FILTER OPERATIONS
  //==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
  /**
   * @brief Configure digital filter for a channel.
   * @param config Filter configuration
   * @return HfAdcErr result code
   */
  HfAdcErr configureFilter(const hf_adc_filter_config_t &config) noexcept;

  /**
   * @brief Enable digital filter for a channel.
   * @param channelId Channel ID
   * @param enable Enable/disable filter
   * @return HfAdcErr result code
   */
  HfAdcErr enableFilter(HfChannelId channelId, bool enable) noexcept;

  /**
   * @brief Get filter configuration for a channel.
   * @param channelId Channel ID
   * @param config Reference to store configuration
   * @return HfAdcErr result code
   */
  HfAdcErr getFilterConfig(HfChannelId channelId, hf_adc_filter_config_t &config) const noexcept;

#endif // HF_MCU_FAMILY_ESP32

  //==============================================================================
  // THRESHOLD MONITOR OPERATIONS
  //==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
  /**
   * @brief Configure threshold monitor.
   * @param config Monitor configuration
   * @return HfAdcErr result code
   */
  HfAdcErr configureMonitor(const hf_adc_monitor_config_t &config) noexcept;

  /**
   * @brief Enable threshold monitor.
   * @param monitorId Monitor ID
   * @param enable Enable/disable monitor
   * @return HfAdcErr result code
   */
  HfAdcErr enableMonitor(HfChannelId monitorId, bool enable) noexcept;

  /**
   * @brief Set threshold callback.
   * @param callback Callback function
   * @param userData User data for callback
   */
  void setThresholdCallback(AdcThresholdCallback callback, void *userData = nullptr) noexcept;

#endif // HF_MCU_FAMILY_ESP32

  //==============================================================================
  // CALIBRATION OPERATIONS
  //==============================================================================

  /**
   * @brief Perform ADC calibration.
   * @param config Calibration configuration
   * @return HfAdcErr result code
   */
  HfAdcErr performCalibration(const hf_adc_calibration_config_t &config) noexcept;

  /**
   * @brief Check if calibration is valid.
   * @return true if calibration is valid, false otherwise
   */
  bool isCalibrationValid() const noexcept;

  /**
   * @brief Convert raw value to voltage using calibration.
   * @param channelId Channel ID
   * @param rawValue Raw ADC value
   * @param voltage Reference to store voltage
   * @return HfAdcErr result code
   */
  HfAdcErr rawToVoltage(HfChannelId channelId, uint32_t rawValue, float &voltage) const noexcept;

  /**
   * @brief Convert raw value to voltage (utility method).
   * @param channelId Channel ID
   * @param rawValue Raw ADC value
   * @return Voltage value
   */
  float convertRawToVoltage(HfChannelId channelId, uint32_t rawValue) noexcept;

  /**
   * @brief Convert voltage to raw value.
   * @param channelId Channel ID
   * @param voltage Voltage value
   * @return Raw ADC value
   */
  uint32_t convertVoltageToRaw(HfChannelId channelId, float voltage) noexcept;

  //==============================================================================
  // ADVANCED FEATURES
  //==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

  /**
   * @brief Enable hardware oversampling.
   * @param channelId Channel ID
   * @param ratio Oversampling ratio (2^n)
   * @return HfAdcErr result code
   */
  HfAdcErr enableOversampling(HfChannelId channelId, uint8_t ratio) noexcept;

  /**
   * @brief Configure trigger source.
   * @param source Trigger source
   * @param parameter Source-specific parameter
   * @return HfAdcErr result code
   */
  HfAdcErr configureTriggerSource(hf_adc_trigger_source_t source, uint32_t parameter = 0) noexcept;

  /**
   * @brief Start triggered sampling.
   * @param channels Vector of channels to sample
   * @return HfAdcErr result code
   */
  HfAdcErr startTriggeredSampling(const std::vector<HfChannelId> &channels) noexcept;

  /**
   * @brief Stop triggered sampling.
   * @return HfAdcErr result code
   */
  HfAdcErr stopTriggeredSampling() noexcept;

#endif // HF_MCU_FAMILY_ESP32

  //==============================================================================
  // POWER MANAGEMENT
  //==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

  /**
   * @brief Set power mode.
   * @param mode Power mode to set
   * @return HfAdcErr result code
   */
  HfAdcErr setPowerMode(hf_adc_power_mode_t mode) noexcept;

  /**
   * @brief Get current power mode.
   * @return Current power mode
   */
  hf_adc_power_mode_t getPowerMode() const noexcept;

  /**
   * @brief Enter low-power mode.
   * @return HfAdcErr result code
   */
  HfAdcErr enterLowPowerMode() noexcept;

  /**
   * @brief Exit low-power mode.
   * @return HfAdcErr result code
   */
  HfAdcErr exitLowPowerMode() noexcept;

#endif // HF_MCU_FAMILY_ESP32

  //==============================================================================
  // STATISTICS AND DIAGNOSTICS
  //==============================================================================

  /**
   * @brief Get operation statistics.
   * @return Current statistics
   */
  hf_adc_statistics_t getStatistics() const noexcept;

  /**
   * @brief Reset operation statistics.
   */
  void resetStatistics() noexcept;

  /**
   * @brief Get diagnostic information.
   * @return Current diagnostics
   */
  hf_adc_diagnostics_t getDiagnostics() noexcept;

  /**
   * @brief Check ADC health status.
   * @return true if ADC is healthy, false otherwise
   */
  bool isAdcHealthy() noexcept;

  //==============================================================================
  // DEMONSTRATION AND TESTING
  //==============================================================================

  /**
   * @brief Demonstrate the statistics functionality by performing sample operations.
   * 
   * This function showcases the statistics and diagnostics features by performing
   * multiple ADC operations and displaying the collected statistics and health
   * information. Useful for testing and validating the statistics implementation.
   */
  void demonstrateStatistics() noexcept;

private:
  //==============================================================================
  // PRIVATE METHODS
  //==============================================================================

  /**
   * @brief Convert platform-specific error to HfAdcErr.
   * @param platformError Platform-specific error code
   * @return Corresponding HfAdcErr
   */
  HfAdcErr ConvertPlatformError(int32_t platformError) noexcept;

  /**
   * @brief Validate channel number.
   * @param channelNum Channel number to validate
   * @return true if valid, false otherwise
   */
  bool IsValidChannel(HfChannelId channelNum) const noexcept {
    return HF_ADC_IS_VALID_CHANNEL(channelNum);
  }

  /**
   * @brief Handle platform-specific error.
   * @param error Platform error code
   */
  void HandlePlatformError(int32_t error) noexcept {
    // Convert to HfAdcErr and log the error
    HfAdcErr hfError = ConvertPlatformError(error);
    
    // Update diagnostics
    diagnostics_.consecutiveErrors++;
    diagnostics_.lastErrorCode = static_cast<uint32_t>(hfError);
    diagnostics_.adcHealthy = false;
  }

  /**
   * @brief Validate and adjust advanced configuration parameters.
   */
  void ValidateAdvancedConfig() noexcept;

  /**
   * @brief Initialize advanced ADC features.
   * @return HfAdcErr result code
   */
  HfAdcErr InitializeAdvancedFeatures() noexcept;

  /**
   * @brief Initialize calibration system.
   * @return true if successful, false otherwise
   */
  bool InitializeCalibration() noexcept;

  /**
   * @brief Deinitialize calibration system.
   */
  void DeinitializeCalibration() noexcept;

  /**
   * @brief DMA data processing task (static function).
   * @param pvParameters Pointer to McuAdc instance
   */
  static void DmaProcessingTask(void* pvParameters);

  /**
   * @brief Process DMA data and invoke callbacks.
   * @param buffer Data buffer from DMA
   * @param length Buffer length in bytes
   * @return Number of samples processed
   */
  size_t ProcessDmaData(uint8_t* buffer, size_t length) noexcept;

  //==============================================================================
  // ESP32C6 HARDWARE FEATURE IMPLEMENTATIONS (PRIVATE METHODS)
  //==============================================================================

  /**
   * @brief Configure ESP32C6 hardware IIR filter for a channel.
   * @param channelId Channel ID to configure filter for
   * @param filterCoeff Filter coefficient (2, 4, 8, 16, 32, 64)
   * @return HfAdcErr result code
   */
  HfAdcErr ConfigureHardwareFilter(HfChannelId channelId, uint8_t filterCoeff) noexcept;

  /**
   * @brief Configure ESP32C6 threshold monitor for a channel.
   * @param channelId Channel ID to monitor
   * @param lowThreshold Low threshold value
   * @param highThreshold High threshold value
   * @param callback Callback function for threshold events
   * @return HfAdcErr result code
   */
  HfAdcErr ConfigureThresholdMonitor(HfChannelId channelId, uint32_t lowThreshold, uint32_t highThreshold,
                                     std::function<void(uint8_t, uint8_t, uint32_t, bool, void*)> callback) noexcept;

  /**
   * @brief Configure ESP32C6 hardware oversampling.
   * @param channelId Channel ID to configure oversampling for
   * @param oversampleRatio Oversampling ratio (power of 2)
   * @param decimationFactor Decimation factor
   * @return HfAdcErr result code
   */
  HfAdcErr ConfigureOversampling(HfChannelId channelId, uint8_t oversampleRatio, uint8_t decimationFactor) noexcept;

  /**
   * @brief Initialize ESP32C6 continuous mode with DMA.
   * @param channel_num Channel number to configure for continuous mode
   * @param sample_rate_hz Sampling rate in Hz
   * @return HfAdcErr result code
   */
  HfAdcErr InitializeContinuousMode(HfChannelId channel_num, uint32_t sample_rate_hz) noexcept;

  /**
   * @brief Deinitialize ESP32C6 continuous mode and clean up resources.
   * @return HfAdcErr result code
   */
  HfAdcErr DeinitializeContinuousMode() noexcept;

  /**
   * @brief Get ESP32C6 ADC channel mapping.
   * @param channel_num Generic channel number
   * @return ESP32C6-specific ADC channel
   */
  adc_channel_t GetMcuChannel(HfChannelId channel_num) const noexcept;
                 
  /**
   * @brief Update operation statistics.
   * @param conversionTimeUs Conversion time in microseconds
   * @param success Whether the operation was successful
   */
  void updateStatistics(uint64_t conversionTimeUs, bool success) const noexcept;
  
  //==============================================================================
  // PRIVATE MEMBER VARIABLES
  //==============================================================================
  
  // Configuration
  hf_adc_advanced_config_t advanced_config_; ///< Advanced configuration
  bool use_advanced_config_;                 ///< Flag indicating advanced config usage
  bool advanced_initialized_;                ///< Advanced features initialized flag

  // Platform-specific handles (using void* for platform independence)
  void *adc_handle_;        ///< Main ADC handle
  void *cali_handle_;       ///< Calibration handle  
  bool cali_enable_;        ///< Calibration enabled flag
  
  // ESP32-specific members
  hf_adc_unit_t unit_;                ///< ADC unit number
  hf_adc_attenuation_t attenuation_;  ///< Attenuation setting
  hf_adc_resolution_t bitwidth_;      ///< Resolution setting

#ifdef HF_MCU_FAMILY_ESP32

  uint8_t *dma_buffer_;         ///< DMA buffer for continuous mode
  void *adc_continuous_handle_; ///< Continuous mode handle
  void *dma_task_handle_;       ///< DMA processing task handle
  bool dma_mode_active_;        ///< DMA mode status
  uint8_t current_dma_channel_; ///< Current DMA channel

  // Callback management
  AdcCallback active_callback_;    ///< Active callback function
  void *callback_user_data_;       ///< User data for callback
  
  // Constants
  static constexpr size_t DMA_BUFFER_SIZE = 4096;

#endif // HF_MCU_FAMILY_ESP32

  // Advanced feature configurations and state
  std::unordered_map<HfChannelId, hf_adc_filter_config_t> filter_configs_;    ///< Filter configurations per channel
  std::unordered_map<HfChannelId, hf_adc_monitor_config_t> monitor_configs_;  ///< Monitor configurations per monitor ID
  hf_adc_calibration_config_t calibration_config_;                            ///< Calibration configuration
    
  // Advanced callbacks
  AdcThresholdCallback threshold_callback_;             ///< Threshold monitor callback
  void *threshold_callback_user_data_;                  ///< User data for threshold callback
  
  // Trigger configuration
  hf_adc_trigger_source_t trigger_source_;              ///< Current trigger source
  uint32_t trigger_parameter_;                          ///< Trigger parameter
  std::vector<HfChannelId> triggered_channels_;         ///< Channels for triggered sampling
  bool triggered_sampling_active_;                      ///< Triggered sampling status

  // Statistics and diagnostics
  mutable hf_adc_statistics_t statistics_;              ///< Operation statistics
  mutable hf_adc_diagnostics_t diagnostics_;            ///< Diagnostic information
  mutable RtosMutex mutex_;                             ///< Mutex for thread safety    

  static constexpr const char* TAG = "McuAdc";          ///< Logging tag

#endif
};
