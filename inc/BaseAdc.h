/**
  *  Contains the declaration of the abstract Adc class, which provides features
  * common to
  *    ADC classes.  Adc's derived classes are intended to employ lazy
  * initialization;
  *    they are initialized the first time the pin in manipulated.
  *
  *  Note:  These functions are not thread or interrupt-safe and should be
  called
  *          called with appropriate guards if used within an ISR or shared
  between tasks.
  -------------------------------------------------------------------------------------**/

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_

#include <cstdint>
#include <string_view>

/**
 * @brief HardFOC ADC error codes
 * @details Comprehensive error enumeration for all ADC operations in the system.
 *          This enumeration is used across all ADC-related classes to provide
 *          consistent error reporting and handling.
 */
enum class HfAdcErr : uint8_t {
    // Success codes
    ADC_SUCCESS = 0,
    
    // General errors
    ADC_ERR_FAILURE,
    ADC_ERR_NOT_INITIALIZED,
    ADC_ERR_ALREADY_INITIALIZED,
    ADC_ERR_INVALID_PARAMETER,
    ADC_ERR_NULL_POINTER,
    ADC_ERR_OUT_OF_MEMORY,
    
    // Channel errors
    ADC_ERR_CHANNEL_NOT_FOUND,
    ADC_ERR_CHANNEL_NOT_ENABLED,
    ADC_ERR_CHANNEL_NOT_CONFIGURED,
    ADC_ERR_CHANNEL_ALREADY_REGISTERED,
    ADC_ERR_CHANNEL_READ_ERR,
    ADC_ERR_CHANNEL_WRITE_ERR,
    ADC_ERR_INVALID_CHANNEL,
    ADC_ERR_CHANNEL_BUSY,
    
    // Sampling errors
    ADC_ERR_INVALID_SAMPLE_COUNT,
    ADC_ERR_SAMPLE_TIMEOUT,
    ADC_ERR_SAMPLE_OVERFLOW,
    ADC_ERR_SAMPLE_UNDERFLOW,
    
    // Hardware errors
    ADC_ERR_HARDWARE_FAULT,
    ADC_ERR_COMMUNICATION_FAILURE,
    ADC_ERR_DEVICE_NOT_RESPONDING,
    ADC_ERR_CALIBRATION_FAILURE,
    ADC_ERR_VOLTAGE_OUT_OF_RANGE,
    
    // Configuration errors
    ADC_ERR_INVALID_CONFIGURATION,
    ADC_ERR_UNSUPPORTED_OPERATION,
    ADC_ERR_RESOURCE_BUSY,
    ADC_ERR_RESOURCE_UNAVAILABLE,
    
    // System errors
    ADC_ERR_SYSTEM_ERROR,
    ADC_ERR_PERMISSION_DENIED,
    ADC_ERR_OPERATION_ABORTED,
    
    // Count for validation
    ADC_ERR_COUNT
};

/**
 * @brief Convert HfAdcErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfAdcErrToString(HfAdcErr err) noexcept {
    switch (err) {
        case HfAdcErr::ADC_SUCCESS: return "Success";
        case HfAdcErr::ADC_ERR_FAILURE: return "General failure";
        case HfAdcErr::ADC_ERR_NOT_INITIALIZED: return "Not initialized";
        case HfAdcErr::ADC_ERR_ALREADY_INITIALIZED: return "Already initialized";
        case HfAdcErr::ADC_ERR_INVALID_PARAMETER: return "Invalid parameter";
        case HfAdcErr::ADC_ERR_NULL_POINTER: return "Null pointer";
        case HfAdcErr::ADC_ERR_OUT_OF_MEMORY: return "Out of memory";
        case HfAdcErr::ADC_ERR_CHANNEL_NOT_FOUND: return "Channel not found";
        case HfAdcErr::ADC_ERR_CHANNEL_NOT_ENABLED: return "Channel not enabled";
        case HfAdcErr::ADC_ERR_CHANNEL_NOT_CONFIGURED: return "Channel not configured";
        case HfAdcErr::ADC_ERR_CHANNEL_ALREADY_REGISTERED: return "Channel already registered";
        case HfAdcErr::ADC_ERR_CHANNEL_READ_ERR: return "Channel read error";
        case HfAdcErr::ADC_ERR_CHANNEL_WRITE_ERR: return "Channel write error";
        case HfAdcErr::ADC_ERR_INVALID_CHANNEL: return "Invalid channel";
        case HfAdcErr::ADC_ERR_CHANNEL_BUSY: return "Channel busy";
        case HfAdcErr::ADC_ERR_INVALID_SAMPLE_COUNT: return "Invalid sample count";
        case HfAdcErr::ADC_ERR_SAMPLE_TIMEOUT: return "Sample timeout";
        case HfAdcErr::ADC_ERR_SAMPLE_OVERFLOW: return "Sample overflow";
        case HfAdcErr::ADC_ERR_SAMPLE_UNDERFLOW: return "Sample underflow";
        case HfAdcErr::ADC_ERR_HARDWARE_FAULT: return "Hardware fault";
        case HfAdcErr::ADC_ERR_COMMUNICATION_FAILURE: return "Communication failure";
        case HfAdcErr::ADC_ERR_DEVICE_NOT_RESPONDING: return "Device not responding";
        case HfAdcErr::ADC_ERR_CALIBRATION_FAILURE: return "Calibration failure";
        case HfAdcErr::ADC_ERR_VOLTAGE_OUT_OF_RANGE: return "Voltage out of range";
        case HfAdcErr::ADC_ERR_INVALID_CONFIGURATION: return "Invalid configuration";
        case HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION: return "Unsupported operation";
        case HfAdcErr::ADC_ERR_RESOURCE_BUSY: return "Resource busy";
        case HfAdcErr::ADC_ERR_RESOURCE_UNAVAILABLE: return "Resource unavailable";
        case HfAdcErr::ADC_ERR_SYSTEM_ERROR: return "System error";
        case HfAdcErr::ADC_ERR_PERMISSION_DENIED: return "Permission denied";
        case HfAdcErr::ADC_ERR_OPERATION_ABORTED: return "Operation aborted";
        default: return "Unknown error";
    }
}

/**
 * @class BaseAdc
 * @brief Base class for ADCs on ESP32-C6 (ESP-IDF).
 * @details This class provides a common interface for all ADC implementations
 *          in the HardFOC system. It supports lazy initialization, robust error
 *          handling, and consistent API across different ADC hardware.
 */
class BaseAdc {
public:
    /**
     * @brief ADC error codes (deprecated - use HfAdcErr instead)
     * @deprecated Use HfAdcErr enumeration for new code
     */
    using AdcErr = HfAdcErr;
    /**
     * @brief Constructor
     */
    BaseAdc() noexcept : initialized_(false) {}

    /**
     * @brief Virtual destructor
     */
    virtual ~BaseAdc() noexcept = default;

    // Disable copy constructor and assignment operator for safety
    BaseAdc(const BaseAdc&) = delete;
    BaseAdc& operator=(const BaseAdc&) = delete;

    // Allow move operations
    BaseAdc(BaseAdc&&) noexcept = default;
    BaseAdc& operator=(BaseAdc&&) noexcept = default;

    /**
     * @brief Ensures that the ADC is initialized (lazy initialization).
     * @return true if the ADC is initialized, false otherwise.
     */
    bool EnsureInitialized() noexcept {
        if (!initialized_) {
            initialized_ = Initialize();
        }
        return initialized_;
    }

    /**
     * @brief Checks if the class is initialized.
     * @return true if initialized, false otherwise
     */
    [[nodiscard]] bool IsInitialized() const noexcept {
        return initialized_;
    }

    /**
     * @brief Initializes the ADC peripheral (must be implemented by derived classes).
     * @return True if the initialization is successful, false otherwise.
     */
    virtual bool Initialize() noexcept = 0;

    /**
     * @brief Deinitializes the ADC peripheral.
     * @return True if deinitialization is successful, false otherwise.
     */
    virtual bool Deinitialize() noexcept {
        initialized_ = false;
        return true;
    }

    /**
     * @brief Get the maximum number of channels supported by this ADC.
     * @return Maximum channel count
     */
    [[nodiscard]] virtual uint8_t GetMaxChannels() const noexcept = 0;

    /**
     * @brief Check if a specific channel is available.
     * @param channel_num Channel number to check
     * @return true if channel is available, false otherwise
     */
    [[nodiscard]] virtual bool IsChannelAvailable(uint8_t channel_num) const noexcept = 0;

    // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN
    
    /**
     * @brief Read channel voltage.
     * @param channel_num Channel number to read from
     * @param channel_reading_v Reference to store voltage reading
     * @param numOfSamplesToAvg Number of samples to average (default 1)
     * @param timeBetweenSamples Time between samples in milliseconds (default 0)
     * @return HfAdcErr error code
     */
    virtual HfAdcErr ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                                  uint8_t numOfSamplesToAvg = 1,
                                  uint32_t timeBetweenSamples = 0) noexcept = 0;
    
    /**
     * @brief Read channel count (raw ADC value).
     * @param channel_num Channel number to read from
     * @param channel_reading_count Reference to store count reading
     * @param numOfSamplesToAvg Number of samples to average (default 1)
     * @param timeBetweenSamples Time between samples in milliseconds (default 0)
     * @return HfAdcErr error code
     */
    virtual HfAdcErr ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                                      uint8_t numOfSamplesToAvg = 1,
                                      uint32_t timeBetweenSamples = 0) noexcept = 0;
    
    /**
     * @brief Read both channel count and voltage.
     * @param channel_num Channel number to read from
     * @param channel_reading_count Reference to store count reading
     * @param channel_reading_v Reference to store voltage reading
     * @param numOfSamplesToAvg Number of samples to average (default 1)
     * @param timeBetweenSamples Time between samples in milliseconds (default 0)
     * @return HfAdcErr error code
     */
    virtual HfAdcErr ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                                 float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                                 uint32_t timeBetweenSamples = 0) noexcept = 0;

protected:
    /**
     * @brief Validate input parameters for read operations.
     * @param channel_num Channel number to validate
     * @param numOfSamplesToAvg Number of samples to validate
     * @return HfAdcErr validation result
     */
    [[nodiscard]] HfAdcErr ValidateReadParameters(uint8_t channel_num, 
                                                  uint8_t numOfSamplesToAvg) const noexcept {
        if (!initialized_) {
            return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
        }
        
        if (numOfSamplesToAvg == 0 || numOfSamplesToAvg > 255) {
            return HfAdcErr::ADC_ERR_INVALID_SAMPLE_COUNT;
        }
        
        if (channel_num >= GetMaxChannels()) {
            return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
        }
        
        if (!IsChannelAvailable(channel_num)) {
            return HfAdcErr::ADC_ERR_CHANNEL_NOT_FOUND;
        }
        
        return HfAdcErr::ADC_SUCCESS;
    }

private:
    bool initialized_;  ///< Initialization status
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_BASEADC_H_ */
