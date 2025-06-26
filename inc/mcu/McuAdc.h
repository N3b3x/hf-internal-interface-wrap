/**
 * @file McuAdc.h
 * @brief Advanced platform-agnostic MCU ADC driver interface with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This class provides a comprehensive implementation of BaseAdc that automatically
 * adapts to the current MCU platform with support for both basic and advanced features.
 * On ESP32C6, it utilizes the latest ESP-IDF v5.5+ ADC features including continuous
 * mode, digital filters, threshold monitors, and advanced calibration.
 *
 * @note This class includes both basic and advanced ADC functionality in a unified interface.
 * @note Advanced features require ESP32C6 with ESP-IDF v5.5+ for full functionality.
 */
#ifndef MCU_ADC_H_
#define MCU_ADC_H_

#include "BaseAdc.h"
#include "McuTypes.h"
#include <cstdint>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>

#ifdef ESP_PLATFORM
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_monitor.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/adc_channel.h"
#include "hal/adc_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#endif

//--------------------------------------
//  Advanced ADC Configuration Types
//--------------------------------------

/**
 * @brief ADC continuous mode sampling strategies.
 */
enum class AdcSamplingStrategy : uint8_t {
    Single = 0,             ///< Single-shot conversion
    Continuous = 1,         ///< Continuous conversion with DMA
    Burst = 2,              ///< Burst mode (fixed number of samples)
    Triggered = 3,          ///< External trigger-based sampling
    ZeroCrossing = 4        ///< Zero-crossing detection mode
};

/**
 * @brief ADC trigger sources for advanced sampling.
 */
enum class AdcTriggerSource : uint8_t {
    Software = 0,           ///< Software trigger (manual)
    Timer = 1,              ///< Timer-based trigger
    GPIO = 2,               ///< GPIO edge trigger
    PWM = 3,                ///< PWM sync trigger
    External = 4,           ///< External trigger signal
    ULP = 5                 ///< ULP processor trigger
};

/**
 * @brief ADC digital filter types supported by ESP32C6.
 */
enum class AdcFilterType : uint8_t {
    None = 0,               ///< No filtering
    IIR = 1,                ///< IIR digital filter
    FIR = 2,                ///< FIR digital filter (if available)
    MovingAverage = 3       ///< Moving average filter
};

/**
 * @brief ADC power mode settings.
 */
enum class AdcPowerMode : uint8_t {
    FullPower = 0,          ///< Maximum performance, highest power
    LowPower = 1,           ///< Reduced power consumption
    UltraLowPower = 2,      ///< Minimal power, reduced functionality
    Sleep = 3               ///< Power-down mode
};

/**
 * @brief ADC calibration schemes.
 */
enum class AdcCalibrationScheme : uint8_t {
    None = 0,               ///< No calibration
    LineFitting = 1,        ///< Line fitting calibration
    Curve = 2,              ///< Curve fitting calibration
    TwoPoint = 3            ///< Two-point calibration
};

/**
 * @brief Continuous mode configuration structure.
 */
struct AdcContinuousConfig {
    uint32_t sampleFreqHz;              ///< Sampling frequency in Hz
    uint32_t convMode;                  ///< Conversion mode (platform-specific)
    uint32_t format;                    ///< Output data format
    size_t bufferSize;                  ///< DMA buffer size
    size_t bufferCount;                 ///< Number of DMA buffers
    bool enableDMA;                     ///< Enable DMA transfers
    
    AdcContinuousConfig() : sampleFreqHz(20000), convMode(0), format(0)
        , bufferSize(4096), bufferCount(2), enableDMA(true) {}
};

/**
 * @brief ADC digital filter configuration.
 */
struct AdcFilterConfig {
    uint8_t channelId;                  ///< Channel to apply filter
    AdcFilterType filterType;           ///< Type of filter
    uint8_t filterCoeff;                ///< Filter coefficient (0-15 for IIR)
    bool enabled;                       ///< Enable/disable filter
    
    AdcFilterConfig() : channelId(0), filterType(AdcFilterType::None)
        , filterCoeff(2), enabled(false) {}
};

/**
 * @brief ADC threshold monitor configuration.
 */
struct AdcMonitorConfig {
    uint8_t monitorId;                  ///< Monitor ID (0-1 for ESP32C6)
    uint8_t channelId;                  ///< Channel to monitor
    uint32_t highThreshold;             ///< High threshold value
    uint32_t lowThreshold;              ///< Low threshold value
    bool highThresholdIntEn;            ///< Enable high threshold interrupt
    bool lowThresholdIntEn;             ///< Enable low threshold interrupt
    
    AdcMonitorConfig() : monitorId(0), channelId(0)
        , highThreshold(4000), lowThreshold(100)
        , highThresholdIntEn(false), lowThresholdIntEn(false) {}
};

/**
 * @brief ADC calibration configuration.
 */
struct AdcCalibrationConfig {
    AdcCalibrationScheme scheme;        ///< Calibration scheme
    uint32_t attenuation;               ///< Attenuation setting
    uint32_t bitWidth;                  ///< Bit width for calibration
    bool autoCalibrate;                 ///< Enable automatic calibration
    
    AdcCalibrationConfig() : scheme(AdcCalibrationScheme::LineFitting)
        , attenuation(0), bitWidth(12), autoCalibrate(true) {}
};

/**
 * @brief Advanced ADC configuration structure.
 */
struct AdcAdvancedConfig {
    // Basic configuration
    uint8_t adcUnit;                    ///< ADC unit (1 or 2)
    uint32_t resolution;                ///< Resolution in bits (12, 11, 10, 9)
    uint32_t attenuation;               ///< Input attenuation
    uint32_t sampleTime;                ///< Sample time setting
    
    // Advanced configuration
    AdcSamplingStrategy samplingStrategy; ///< Sampling strategy
    AdcTriggerSource triggerSource;     ///< Trigger source
    AdcPowerMode powerMode;             ///< Power mode setting
    bool oversamplingEnabled;           ///< Enable hardware oversampling
    uint8_t oversamplingRatio;          ///< Oversampling ratio (2^n)
    
    // Continuous mode
    bool continuousMode;                ///< Enable continuous mode
    AdcContinuousConfig continuousConfig; ///< Continuous mode configuration
    
    // Calibration
    AdcCalibrationConfig calibrationConfig; ///< Calibration configuration
    
    // Statistics and diagnostics
    bool statisticsEnabled;             ///< Enable operation statistics
    bool diagnosticsEnabled;            ///< Enable diagnostic features
    
    AdcAdvancedConfig() : adcUnit(1), resolution(12), attenuation(0), sampleTime(0)
        , samplingStrategy(AdcSamplingStrategy::Single)
        , triggerSource(AdcTriggerSource::Software)
        , powerMode(AdcPowerMode::FullPower)
        , oversamplingEnabled(false), oversamplingRatio(1)
        , continuousMode(false), statisticsEnabled(false), diagnosticsEnabled(false) {}
};

/**
 * @brief ADC operation statistics.
 */
struct AdcStatistics {
    uint64_t totalConversions;          ///< Total conversions performed
    uint64_t successfulConversions;     ///< Successful conversions
    uint64_t failedConversions;         ///< Failed conversions
    uint64_t averageConversionTimeUs;   ///< Average conversion time (microseconds)
    uint64_t maxConversionTimeUs;       ///< Maximum conversion time
    uint64_t minConversionTimeUs;       ///< Minimum conversion time
    uint32_t calibrationCount;          ///< Number of calibrations performed
    uint32_t thresholdViolations;       ///< Threshold monitor violations
    
    AdcStatistics() : totalConversions(0), successfulConversions(0), failedConversions(0)
        , averageConversionTimeUs(0), maxConversionTimeUs(0), minConversionTimeUs(UINT64_MAX)
        , calibrationCount(0), thresholdViolations(0) {}
};

/**
 * @brief ADC diagnostic information.
 */
struct AdcDiagnostics {
    bool adcHealthy;                    ///< Overall ADC health status
    uint32_t lastErrorCode;             ///< Last error code
    uint64_t lastErrorTimestamp;        ///< Last error timestamp
    uint32_t consecutiveErrors;         ///< Consecutive error count
    double temperatureC;                ///< ADC temperature (if available)
    double referenceVoltage;            ///< Reference voltage
    bool calibrationValid;              ///< Calibration validity
    
    AdcDiagnostics() : adcHealthy(true), lastErrorCode(0), lastErrorTimestamp(0)
        , consecutiveErrors(0), temperatureC(25.0), referenceVoltage(3.3)
        , calibrationValid(false) {}
};

// Callback function types
using AdcConversionCallback = std::function<void(uint8_t channel, uint32_t rawValue, float voltage, void* userData)>;
using AdcThresholdCallback = std::function<void(uint8_t monitorId, uint8_t channel, uint32_t value, bool isHigh, void* userData)>;
using AdcErrorCallback = std::function<void(HfAdcErr error, void* userData)>;

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
    explicit McuAdc(const AdcAdvancedConfig& config) noexcept;

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
     * @param channel_num Channel number to check
     * @return true if available, false otherwise
     */
    bool IsChannelAvailable(uint8_t channel_num) const noexcept override;

    /**
     * @brief Configure a channel with specified settings.
     * @param channel_num Channel number to configure
     * @param config Channel configuration
     * @return HfAdcErr result code
     */
    HfAdcErr ConfigureChannel(uint8_t channel_num, const AdcChannelConfig& config) noexcept override;

    /**
     * @brief Perform a single ADC conversion.
     * @param channel_num Channel number to read
     * @param raw_value Reference to store raw ADC value
     * @return HfAdcErr result code
     */
    HfAdcErr ReadRaw(uint8_t channel_num, uint32_t& raw_value) noexcept override;

    /**
     * @brief Perform ADC conversion and return calibrated voltage.
     * @param channel_num Channel number to read
     * @param voltage Reference to store voltage value
     * @return HfAdcErr result code
     */
    HfAdcErr ReadVoltage(uint8_t channel_num, float& voltage) noexcept override;

    /**
     * @brief Perform multiple conversions and average the result.
     * @param channel_num Channel number to read
     * @param samples Number of samples to average
     * @param raw_value Reference to store averaged raw value
     * @return HfAdcErr result code
     */
    HfAdcErr ReadRawAveraged(uint8_t channel_num, uint8_t samples, uint32_t& raw_value) noexcept override;

    /**
     * @brief Get channel configuration.
     * @param channel_num Channel number
     * @param config Reference to store configuration
     * @return HfAdcErr result code
     */
    HfAdcErr GetChannelConfig(uint8_t channel_num, AdcChannelConfig& config) const noexcept override;

    /**
     * @brief Read the internal temperature sensor.
     * @param temperature Reference to store temperature in Celsius
     * @return HfAdcErr result code
     */
    HfAdcErr ReadTemperature(float& temperature) noexcept override;

    //==============================================================================
    // ADVANCED ADC OPERATIONS
    //==============================================================================

    /**
     * @brief Initialize with advanced configuration.
     * @param config Advanced configuration parameters
     * @return HfAdcErr result code
     */
    HfAdcErr initializeAdvanced(const AdcAdvancedConfig& config) noexcept;

    /**
     * @brief Reconfigure ADC with new settings.
     * @param config New configuration parameters
     * @return HfAdcErr result code
     */
    HfAdcErr reconfigure(const AdcAdvancedConfig& config) noexcept;

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
    HfAdcErr startContinuous(const std::vector<uint8_t>& channels, const AdcContinuousConfig& config) noexcept;

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
    HfAdcErr readContinuous(uint8_t* buffer, size_t maxSamples, size_t& samplesRead, uint32_t timeoutMs = 1000) noexcept;

    /**
     * @brief Set continuous mode callback.
     * @param callback Callback function
     * @param userData User data for callback
     */
    void setContinuousCallback(AdcConversionCallback callback, void* userData = nullptr) noexcept;

    //==============================================================================
    // DIGITAL FILTER OPERATIONS
    //==============================================================================

    /**
     * @brief Configure digital filter for a channel.
     * @param config Filter configuration
     * @return HfAdcErr result code
     */
    HfAdcErr configureFilter(const AdcFilterConfig& config) noexcept;

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
    HfAdcErr getFilterConfig(uint8_t channelId, AdcFilterConfig& config) const noexcept;

    //==============================================================================
    // THRESHOLD MONITOR OPERATIONS
    //==============================================================================

    /**
     * @brief Configure threshold monitor.
     * @param config Monitor configuration
     * @return HfAdcErr result code
     */
    HfAdcErr configureMonitor(const AdcMonitorConfig& config) noexcept;

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
    void setThresholdCallback(AdcThresholdCallback callback, void* userData = nullptr) noexcept;

    //==============================================================================
    // CALIBRATION OPERATIONS
    //==============================================================================

    /**
     * @brief Perform ADC calibration.
     * @param config Calibration configuration
     * @return HfAdcErr result code
     */
    HfAdcErr performCalibration(const AdcCalibrationConfig& config) noexcept;

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
    HfAdcErr rawToVoltage(uint8_t channelId, uint32_t rawValue, float& voltage) const noexcept;

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
    HfAdcErr startTriggeredSampling(const std::vector<uint8_t>& channels) noexcept;

    /**
     * @brief Stop triggered sampling.
     * @return HfAdcErr result code
     */
    HfAdcErr stopTriggeredSampling() noexcept;

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

    //==============================================================================
    // PRIVATE MEMBER VARIABLES
    //==============================================================================

    // Configuration
    AdcAdvancedConfig advanced_config_;     ///< Advanced configuration
    bool use_advanced_config_;              ///< Flag indicating advanced config usage
    bool advanced_initialized_;             ///< Advanced features initialized flag

    // Platform-specific handles
    void* continuous_handle_;               ///< Continuous mode handle
    void* calibration_handle_;              ///< Calibration handle
    std::unordered_map<uint8_t, void*> filter_handles_; ///< Filter handles
    std::unordered_map<uint8_t, void*> monitor_handles_; ///< Monitor handles

    // Channel configurations
    std::vector<AdcFilterConfig> filter_configs_;     ///< Filter configurations
    std::vector<AdcMonitorConfig> monitor_configs_;   ///< Monitor configurations
    AdcCalibrationConfig calibration_config_;         ///< Calibration configuration

    // State management
    mutable std::mutex mutex_;              ///< Thread synchronization mutex
    bool continuous_running_;               ///< Continuous mode status
    bool triggered_sampling_;               ///< Triggered sampling status
    AdcPowerMode current_power_mode_;       ///< Current power mode

    // Callback functions
    AdcConversionCallback conversion_callback_; ///< Conversion callback
    AdcThresholdCallback threshold_callback_;   ///< Threshold callback
    AdcErrorCallback error_callback_;           ///< Error callback
    void* callback_userdata_;               ///< Callback user data

    // Statistics and diagnostics
    mutable AdcStatistics statistics_;      ///< Operation statistics
    AdcDiagnostics diagnostics_;           ///< Diagnostic information
    uint64_t last_operation_time_;         ///< Last operation timestamp

    // Platform-specific constants
    static constexpr uint8_t MAX_CHANNELS_ESP32C6 = 7;
    static constexpr uint8_t MAX_FILTERS = 2;
    static constexpr uint8_t MAX_MONITORS = 2;
    static constexpr uint32_t DEFAULT_SAMPLE_FREQ = 20000;
    static constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
};

#endif // MCU_ADC_H_
