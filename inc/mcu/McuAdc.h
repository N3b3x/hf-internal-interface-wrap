/**
 * @file McuAdc.h
 * @brief Advanced platform-agnostic MCU ADC driver interface with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This class provides a comprehensive implementation of BaseAdc that automatically
 * adapts to the current MCU platform with support for both basic and advanced features.
 * On ESP32C6, it utilizes the latest ESP-IDF v5.5+ ADC features including continuous
 * mode, digital filters, threshold monitors, and advanced calibration. The implementation
 * includes multi-channel conversions, DMA support, and hardware-accelerated operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class includes both basic and advanced ADC functionality in a unified interface.
 * @note Advanced features require ESP32C6 with ESP-IDF v5.5+ for full functionality.
 */

#pragma once

#include "RtosMutex.h"
#include "BaseAdc.h"
#include "McuTypes.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declare ESP32-specific types to avoid including ESP-IDF headers when not on ESP32
#ifdef HF_TARGET_MCU_ESP32C6
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "soc/adc_channel.h"
#endif

// Type aliases for platform compatibility
using hf_adc_unit_t = uint8_t;
using hf_adc_resolution_t = uint8_t;

//--------------------------------------
//  Advanced ADC Configuration Types
//--------------------------------------

/**
 * @brief ADC continuous mode sampling strategies.
 */
enum class AdcSamplingStrategy : uint8_t {
  Single = 0,      ///< Single-shot conversion
  Continuous = 1,  ///< Continuous conversion with DMA
  Burst = 2,       ///< Burst mode (fixed number of samples)
  Triggered = 3,   ///< External trigger-based sampling
  ZeroCrossing = 4 ///< Zero-crossing detection mode
};

/**
 * @brief ADC trigger sources for advanced sampling.
 */
enum class AdcTriggerSource : uint8_t {
  Software = 0, ///< Software trigger (manual)
  Timer = 1,    ///< Timer-based trigger
  GPIO = 2,     ///< GPIO edge trigger
  PWM = 3,      ///< PWM sync trigger
  External = 4, ///< External trigger signal
  ULP = 5       ///< ULP processor trigger
};

/**
 * @brief ADC digital filter types supported by ESP32C6.
 */
enum class AdcFilterType : uint8_t {
  None = 0,         ///< No filtering
  IIR = 1,          ///< IIR digital filter
  FIR = 2,          ///< FIR digital filter (if available)
  MovingAverage = 3 ///< Moving average filter
};

/**
 * @brief ADC power mode settings.
 */
enum class AdcPowerMode : uint8_t {
  FullPower = 0,     ///< Maximum performance, highest power
  LowPower = 1,      ///< Reduced power consumption
  UltraLowPower = 2, ///< Minimal power, reduced functionality
  Sleep = 3          ///< Power-down mode
};

/**
 * @brief ADC calibration schemes.
 */
enum class AdcCalibrationScheme : uint8_t {
  None = 0,        ///< No calibration
  LineFitting = 1, ///< Line fitting calibration
  Curve = 2,       ///< Curve fitting calibration
  TwoPoint = 3     ///< Two-point calibration
};

/**
 * @brief Continuous mode configuration structure.
 */
struct AdcContinuousConfig {
  uint32_t sampleFreqHz; ///< Sampling frequency in Hz
  uint32_t convMode;     ///< Conversion mode (platform-specific)
  uint32_t format;       ///< Output data format
  size_t bufferSize;     ///< DMA buffer size
  size_t bufferCount;    ///< Number of DMA buffers
  bool enableDMA;        ///< Enable DMA transfers

  AdcContinuousConfig()
      : sampleFreqHz(20000), convMode(0), format(0), bufferSize(4096), bufferCount(2),
        enableDMA(true) {}
};

/**
 * @brief ADC digital filter configuration.
 */
struct AdcFilterConfig {
  uint8_t channelId;        ///< Channel to apply filter
  AdcFilterType filterType; ///< Type of filter
  uint8_t filterCoeff;      ///< Filter coefficient (0-15 for IIR)
  bool enabled;             ///< Enable/disable filter

  AdcFilterConfig()
      : channelId(0), filterType(AdcFilterType::None), filterCoeff(2), enabled(false) {}
};

/**
 * @brief ADC threshold monitor configuration.
 */
struct AdcMonitorConfig {
  uint8_t monitorId;       ///< Monitor ID (0-1 for ESP32C6)
  uint8_t channelId;       ///< Channel to monitor
  uint32_t highThreshold;  ///< High threshold value
  uint32_t lowThreshold;   ///< Low threshold value
  bool highThresholdIntEn; ///< Enable high threshold interrupt
  bool lowThresholdIntEn;  ///< Enable low threshold interrupt

  AdcMonitorConfig()
      : monitorId(0), channelId(0), highThreshold(4000), lowThreshold(100),
        highThresholdIntEn(false), lowThresholdIntEn(false) {}
};

/**
 * @brief ADC calibration configuration.
 */
struct AdcCalibrationConfig {
  AdcCalibrationScheme scheme; ///< Calibration scheme
  uint32_t attenuation;        ///< Attenuation setting
  uint32_t bitWidth;           ///< Bit width for calibration
  bool autoCalibrate;          ///< Enable automatic calibration

  AdcCalibrationConfig()
      : scheme(AdcCalibrationScheme::LineFitting), attenuation(0), bitWidth(12),
        autoCalibrate(true) {}
};

/**
 * @brief Advanced ADC configuration structure.
 */
struct AdcAdvancedConfig {
  // Basic configuration
  uint8_t adcUnit;      ///< ADC unit (1 or 2)
  uint32_t resolution;  ///< Resolution in bits (12, 11, 10, 9)
  uint32_t attenuation; ///< Input attenuation
  uint32_t sampleTime;  ///< Sample time setting

  // Advanced configuration
  AdcSamplingStrategy samplingStrategy; ///< Sampling strategy
  AdcTriggerSource triggerSource;       ///< Trigger source
  AdcPowerMode powerMode;               ///< Power mode setting
  bool oversamplingEnabled;             ///< Enable hardware oversampling
  uint8_t oversamplingRatio;            ///< Oversampling ratio (2^n)

  // Continuous mode
  bool continuousMode;                  ///< Enable continuous mode
  AdcContinuousConfig continuousConfig; ///< Continuous mode configuration

  // Calibration
  AdcCalibrationConfig calibrationConfig; ///< Calibration configuration

  // Statistics and diagnostics
  bool statisticsEnabled;  ///< Enable operation statistics
  bool diagnosticsEnabled; ///< Enable diagnostic features

  AdcAdvancedConfig()
      : adcUnit(1), resolution(12), attenuation(0), sampleTime(0),
        samplingStrategy(AdcSamplingStrategy::Single), triggerSource(AdcTriggerSource::Software),
        powerMode(AdcPowerMode::FullPower), oversamplingEnabled(false), oversamplingRatio(1),
        continuousMode(false), statisticsEnabled(false), diagnosticsEnabled(false) {}
};

/**
 * @brief ADC operation statistics.
 */
struct AdcStatistics {
  uint64_t totalConversions;        ///< Total conversions performed
  uint64_t successfulConversions;   ///< Successful conversions
  uint64_t failedConversions;       ///< Failed conversions
  uint64_t averageConversionTimeUs; ///< Average conversion time (microseconds)
  uint64_t maxConversionTimeUs;     ///< Maximum conversion time
  uint64_t minConversionTimeUs;     ///< Minimum conversion time
  uint32_t calibrationCount;        ///< Number of calibrations performed
  uint32_t thresholdViolations;     ///< Threshold monitor violations

  AdcStatistics()
      : totalConversions(0), successfulConversions(0), failedConversions(0),
        averageConversionTimeUs(0), maxConversionTimeUs(0), minConversionTimeUs(UINT64_MAX),
        calibrationCount(0), thresholdViolations(0) {}
};

/**
 * @brief ADC diagnostic information.
 */
struct AdcDiagnostics {
  bool adcHealthy;             ///< Overall ADC health status
  uint32_t lastErrorCode;      ///< Last error code
  uint64_t lastErrorTimestamp; ///< Last error timestamp
  uint32_t consecutiveErrors;  ///< Consecutive error count
  double temperatureC;         ///< ADC temperature (if available)
  double referenceVoltage;     ///< Reference voltage
  bool calibrationValid;       ///< Calibration validity

  AdcDiagnostics()
      : adcHealthy(true), lastErrorCode(0), lastErrorTimestamp(0), consecutiveErrors(0),
        temperatureC(25.0), referenceVoltage(3.3), calibrationValid(false) {}
};

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
  explicit McuAdc(const AdcAdvancedConfig &config) noexcept;

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

  //==============================================================================
  // ADVANCED ADC OPERATIONS
  //==============================================================================

  /**
   * @brief Initialize with advanced configuration.
   * @param config Advanced configuration parameters
   * @return HfAdcErr result code
   */
  HfAdcErr initializeAdvanced(const AdcAdvancedConfig &config) noexcept;

  /**
   * @brief Reconfigure ADC with new settings.
   * @param config New configuration parameters
   * @return HfAdcErr result code
   */
  HfAdcErr reconfigure(const AdcAdvancedConfig &config) noexcept;

  /**
   * @brief Get current ADC configuration.
   * @return Current configuration
   */
  AdcAdvancedConfig getCurrentConfiguration() const noexcept;

  //==============================================================================
  // CONTINUOUS MODE OPERATIONS
  //==============================================================================

  /**
   * @brief Start continuous mode sampling.
   * @param channels Vector of channels to sample
   * @param config Continuous mode configuration
   * @return HfAdcErr result code
   */
  HfAdcErr startContinuous(const std::vector<uint8_t> &channels,
                           const AdcContinuousConfig &config) noexcept;

  /**
   * @brief Stop continuous mode sampling.
   * @return HfAdcErr result code
   */
  HfAdcErr stopContinuous() noexcept;

  /**
   * @brief Read samples from continuous mode.
   * @param buffer Buffer to store samples
   * @param maxSamples Maximum number of samples to read
   * @param samplesRead Reference to store actual samples read
   * @param timeoutMs Timeout in milliseconds
   * @return HfAdcErr result code
   */
  HfAdcErr readContinuous(uint8_t *buffer, size_t maxSamples, size_t &samplesRead,
                          uint32_t timeoutMs = 1000) noexcept;

  /**
   * @brief Set continuous mode callback.
   * @param callback Callback function
   * @param userData User data for callback
   */
  void setContinuousCallback(AdcConversionCallback callback, void *userData = nullptr) noexcept;

  //==============================================================================
  // DIGITAL FILTER OPERATIONS
  //==============================================================================

  /**
   * @brief Configure digital filter for a channel.
   * @param config Filter configuration
   * @return HfAdcErr result code
   */
  HfAdcErr configureFilter(const AdcFilterConfig &config) noexcept;

  /**
   * @brief Enable digital filter for a channel.
   * @param channelId Channel ID
   * @param enable Enable/disable filter
   * @return HfAdcErr result code
   */
  HfAdcErr enableFilter(uint8_t channelId, bool enable) noexcept;

  /**
   * @brief Get filter configuration for a channel.
   * @param channelId Channel ID
   * @param config Reference to store configuration
   * @return HfAdcErr result code
   */
  HfAdcErr getFilterConfig(uint8_t channelId, AdcFilterConfig &config) const noexcept;

  //==============================================================================
  // THRESHOLD MONITOR OPERATIONS
  //==============================================================================

  /**
   * @brief Configure threshold monitor.
   * @param config Monitor configuration
   * @return HfAdcErr result code
   */
  HfAdcErr configureMonitor(const AdcMonitorConfig &config) noexcept;

  /**
   * @brief Enable threshold monitor.
   * @param monitorId Monitor ID
   * @param enable Enable/disable monitor
   * @return HfAdcErr result code
   */
  HfAdcErr enableMonitor(uint8_t monitorId, bool enable) noexcept;

  /**
   * @brief Set threshold callback.
   * @param callback Callback function
   * @param userData User data for callback
   */
  void setThresholdCallback(AdcThresholdCallback callback, void *userData = nullptr) noexcept;

  //==============================================================================
  // CALIBRATION OPERATIONS
  //==============================================================================

  /**
   * @brief Perform ADC calibration.
   * @param config Calibration configuration
   * @return HfAdcErr result code
   */
  HfAdcErr performCalibration(const AdcCalibrationConfig &config) noexcept;

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
  HfAdcErr rawToVoltage(uint8_t channelId, uint32_t rawValue, float &voltage) const noexcept;

  //==============================================================================
  // POWER MANAGEMENT
  //==============================================================================

  /**
   * @brief Set power mode.
   * @param mode Power mode to set
   * @return HfAdcErr result code
   */
  HfAdcErr setPowerMode(AdcPowerMode mode) noexcept;

  /**
   * @brief Get current power mode.
   * @return Current power mode
   */
  AdcPowerMode getPowerMode() const noexcept;

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

  //==============================================================================
  // STATISTICS AND DIAGNOSTICS
  //==============================================================================

  /**
   * @brief Get operation statistics.
   * @return Current statistics
   */
  AdcStatistics getStatistics() const noexcept;

  /**
   * @brief Reset operation statistics.
   */
  void resetStatistics() noexcept;

  /**
   * @brief Get diagnostic information.
   * @return Current diagnostics
   */
  AdcDiagnostics getDiagnostics() noexcept;

  /**
   * @brief Check ADC health status.
   * @return true if ADC is healthy, false otherwise
   */
  bool isAdcHealthy() noexcept;

  //==============================================================================
  // ADVANCED FEATURES
  //==============================================================================

  /**
   * @brief Enable hardware oversampling.
   * @param channelId Channel ID
   * @param ratio Oversampling ratio (2^n)
   * @return HfAdcErr result code
   */
  HfAdcErr enableOversampling(uint8_t channelId, uint8_t ratio) noexcept;

  /**
   * @brief Configure trigger source.
   * @param source Trigger source
   * @param parameter Source-specific parameter
   * @return HfAdcErr result code
   */
  HfAdcErr configureTriggerSource(AdcTriggerSource source, uint32_t parameter = 0) noexcept;

  /**
   * @brief Start triggered sampling.
   * @param channels Vector of channels to sample
   * @return HfAdcErr result code
   */
  HfAdcErr startTriggeredSampling(const std::vector<uint8_t> &channels) noexcept;

  /**
   * @brief Stop triggered sampling.
   * @return HfAdcErr result code
   */
  HfAdcErr stopTriggeredSampling() noexcept;

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
  HfAdcErr CalibrateChannel(HfChannelId channel_id, const CalibrationConfig &config,
                            CalibrationProgressCallback progress_callback = nullptr,
                            void *user_data = nullptr) noexcept override;

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
   * @brief Verify calibration accuracy using known reference
   * @param channel_id Channel to verify
   * @param reference_voltage Known reference voltage to test
   * @param measured_voltage Output: voltage measured by ADC
   * @param error_percent Output: percentage error from reference
   * @return HfAdcErr error code
   */
  HfAdcErr VerifyCalibration(HfChannelId channel_id, float reference_voltage,
                             float &measured_voltage, float &error_percent) noexcept override;

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
  bool IsValidChannel(uint8_t channelNum) const noexcept;

  /**
   * @brief Update operation statistics.
   * @param success Operation success status
   * @param operationTimeUs Operation time in microseconds
   */
  void UpdateStatistics(bool success, uint64_t operationTimeUs) noexcept;

  /**
   * @brief Handle platform-specific error.
   * @param error Platform error code
   */
  void HandlePlatformError(int32_t error) noexcept;

  /**
   * @brief Initialize ESP32 ADC continuous mode.
   * @return HfAdcErr result code
   */
  HfAdcErr InitializeEsp32Continuous() noexcept;

  /**
   * @brief Initialize ESP32 ADC filters.
   * @return HfAdcErr result code
   */
  HfAdcErr InitializeEsp32Filters() noexcept;
  /**
   * @brief Initialize ESP32 ADC monitors.
   * @return HfAdcErr result code
   */
  HfAdcErr InitializeEsp32Monitors() noexcept;

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
   * @brief Convert ESP-IDF error codes to HfAdcErr.
   * @param esp_error ESP-IDF error code
   * @return Corresponding HfAdcErr
   */
  HfAdcErr ConvertEspError(int32_t esp_error) noexcept;

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
  // PRIVATE MEMBER VARIABLES
  //==============================================================================
  // Configuration
  AdcAdvancedConfig advanced_config_; ///< Advanced configuration
  bool use_advanced_config_;          ///< Flag indicating advanced config usage
  bool advanced_initialized_;         ///< Advanced features initialized flag

  // Platform-specific handles (using void* for platform independence)
  void *adc_handle_;        ///< Main ADC handle
  void *cali_handle_;       ///< Calibration handle  
  bool cali_enable_;        ///< Calibration enabled flag
  
  // ESP32-specific members
  hf_adc_unit_t unit_;      ///< ADC unit number
  uint32_t attenuation_;    ///< Attenuation setting
  hf_adc_resolution_t bitwidth_; ///< Resolution setting
  
#ifdef HF_MCU_FAMILY_ESP32
  uint8_t *dma_buffer_;     ///< DMA buffer for continuous mode
  void *adc_continuous_handle_; ///< Continuous mode handle
  void *dma_task_handle_;   ///< DMA processing task handle
  bool dma_mode_active_;    ///< DMA mode status
  uint8_t current_dma_channel_; ///< Current DMA channel
  
  // Callback management
  AdcCallback active_callback_;    ///< Active callback function
  void *callback_user_data_;       ///< User data for callback
  
  // Constants
  static constexpr size_t DMA_BUFFER_SIZE = 4096;
#endif
};