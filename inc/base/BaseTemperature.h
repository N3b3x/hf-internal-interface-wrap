/**
 * @file BaseTemperature.h
 * @brief Abstract base class for temperature sensor implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for temperature sensing
 * that provides a consistent API across different temperature sensor implementations.
 * Concrete implementations for various platforms and sensor types inherit from this class.
 *
 * @author HardFOC Development Team
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BasePwm/BaseAdc.
 * @note Users should program against this interface, not specific implementations.
 * @note Temperature readings are provided in Celsius, with conversion utilities available.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>

//--------------------------------------
//  HardFOC Temperature Error Codes (Table)
//--------------------------------------

/**
 * @brief HardFOC Temperature sensor error codes
 * @details Comprehensive error enumeration for all temperature sensor operations in the system.
 *          This enumeration is used across all temperature-related classes to provide
 *          consistent error reporting and handling.
 */
#define HF_TEMP_ERR_LIST(X)                                                \
  /* Success codes */                                                      \
  X(TEMP_SUCCESS, 0, "Success")                                            \
  /* General errors */                                                     \
  X(TEMP_ERR_FAILURE, 1, "General failure")                               \
  X(TEMP_ERR_NOT_INITIALIZED, 2, "Not initialized")                       \
  X(TEMP_ERR_ALREADY_INITIALIZED, 3, "Already initialized")               \
  X(TEMP_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                   \
  X(TEMP_ERR_NULL_POINTER, 5, "Null pointer")                             \
  X(TEMP_ERR_OUT_OF_MEMORY, 6, "Out of memory")                           \
  /* Sensor specific errors */                                            \
  X(TEMP_ERR_SENSOR_NOT_AVAILABLE, 7, "Temperature sensor not available") \
  X(TEMP_ERR_SENSOR_BUSY, 8, "Sensor already in use")                     \
  X(TEMP_ERR_SENSOR_DISABLED, 9, "Sensor is disabled")                    \
  X(TEMP_ERR_SENSOR_NOT_READY, 10, "Sensor not ready")                    \
  /* Reading errors */                                                     \
  X(TEMP_ERR_READ_FAILED, 11, "Failed to read temperature")               \
  X(TEMP_ERR_INVALID_READING, 12, "Invalid temperature reading")          \
  X(TEMP_ERR_OUT_OF_RANGE, 13, "Temperature out of sensor range")         \
  X(TEMP_ERR_TIMEOUT, 14, "Operation timeout")                            \
  /* Configuration errors */                                              \
  X(TEMP_ERR_INVALID_RANGE, 15, "Invalid temperature range")              \
  X(TEMP_ERR_RANGE_TOO_NARROW, 16, "Temperature range too narrow")        \
  X(TEMP_ERR_RANGE_TOO_WIDE, 17, "Temperature range too wide")            \
  X(TEMP_ERR_UNSUPPORTED_RANGE, 18, "Unsupported temperature range")      \
  /* Calibration errors */                                                \
  X(TEMP_ERR_CALIBRATION_FAILED, 19, "Calibration failed")                \
  X(TEMP_ERR_NOT_CALIBRATED, 20, "Sensor not calibrated")                 \
  X(TEMP_ERR_INVALID_CALIBRATION, 21, "Invalid calibration data")         \
  /* Communication errors */                                              \
  X(TEMP_ERR_COMMUNICATION_FAILED, 22, "Communication with sensor failed") \
  X(TEMP_ERR_CHECKSUM_FAILED, 23, "Data checksum verification failed")    \
  /* Power management errors */                                           \
  X(TEMP_ERR_POWER_DOWN_FAILED, 24, "Failed to power down sensor")        \
  X(TEMP_ERR_POWER_UP_FAILED, 25, "Failed to power up sensor")            \
  /* Hardware errors */                                                   \
  X(TEMP_ERR_HARDWARE_FAULT, 26, "Hardware fault detected")               \
  X(TEMP_ERR_OVERCURRENT, 27, "Overcurrent condition")                    \
  X(TEMP_ERR_OVERVOLTAGE, 28, "Overvoltage condition")                    \
  X(TEMP_ERR_UNDERVOLTAGE, 29, "Undervoltage condition")                  \
  X(TEMP_ERR_OVERHEATING, 30, "Sensor overheating")                       \
  /* Resource errors */                                                   \
  X(TEMP_ERR_RESOURCE_BUSY, 31, "Required resource is busy")              \
  X(TEMP_ERR_RESOURCE_UNAVAILABLE, 32, "Required resource unavailable")   \
  X(TEMP_ERR_INSUFFICIENT_RESOURCES, 33, "Insufficient system resources") \
  /* Operation errors */                                                  \
  X(TEMP_ERR_OPERATION_ABORTED, 34, "Operation was aborted")              \
  X(TEMP_ERR_OPERATION_PENDING, 35, "Operation is pending")               \
  X(TEMP_ERR_INVALID_STATE, 36, "Invalid sensor state")                   \
  X(TEMP_ERR_STATE_TRANSITION_FAILED, 37, "State transition failed")      \
  /* Data processing errors */                                            \
  X(TEMP_ERR_DATA_CORRUPTION, 38, "Data corruption detected")             \
  X(TEMP_ERR_CONVERSION_FAILED, 39, "Temperature conversion failed")      \
  X(TEMP_ERR_FILTERING_FAILED, 40, "Temperature filtering failed")        \
  /* Threshold and monitoring errors */                                   \
  X(TEMP_ERR_THRESHOLD_EXCEEDED, 41, "Temperature threshold exceeded")    \
  X(TEMP_ERR_INVALID_THRESHOLD, 42, "Invalid threshold value")            \
  X(TEMP_ERR_MONITORING_FAILED, 43, "Temperature monitoring failed")      \
  /* Maximum error code marker */                                         \
  X(TEMP_ERR_MAX, 44, "Maximum error code")

//--------------------------------------
//  HardFOC Temperature Error Enum
//--------------------------------------

/**
 * @brief Temperature sensor error codes enumeration
 */
typedef enum {
#define TEMP_ERROR_ENUM(name, code, desc) name = code,
  HF_TEMP_ERR_LIST(TEMP_ERROR_ENUM)
#undef TEMP_ERROR_ENUM
} HfTempError_t;

//--------------------------------------
//  HardFOC Temperature Types
//--------------------------------------

/**
 * @brief Temperature sensor types
 */
typedef enum {
  TEMP_SENSOR_TYPE_UNKNOWN = 0,        ///< Unknown or unspecified sensor type
  TEMP_SENSOR_TYPE_INTERNAL,           ///< Internal chip temperature sensor
  TEMP_SENSOR_TYPE_EXTERNAL_DIGITAL,   ///< External digital temperature sensor
  TEMP_SENSOR_TYPE_EXTERNAL_ANALOG,    ///< External analog temperature sensor
  TEMP_SENSOR_TYPE_THERMOCOUPLE,       ///< Thermocouple temperature sensor
  TEMP_SENSOR_TYPE_RTD,                ///< Resistance Temperature Detector
  TEMP_SENSOR_TYPE_THERMISTOR,         ///< Thermistor temperature sensor
  TEMP_SENSOR_TYPE_INFRARED,           ///< Infrared temperature sensor
  TEMP_SENSOR_TYPE_MAX                 ///< Maximum sensor type marker
} HfTempSensorType_t;

/**
 * @brief Temperature sensor states
 */
typedef enum {
  TEMP_STATE_UNINITIALIZED = 0,        ///< Sensor is not initialized
  TEMP_STATE_INITIALIZED,              ///< Sensor is initialized but not enabled
  TEMP_STATE_ENABLED,                  ///< Sensor is enabled and ready
  TEMP_STATE_READING,                  ///< Sensor is performing a reading
  TEMP_STATE_ERROR,                    ///< Sensor is in error state
  TEMP_STATE_DISABLED,                 ///< Sensor is disabled
  TEMP_STATE_CALIBRATING,              ///< Sensor is being calibrated
  TEMP_STATE_SLEEPING,                 ///< Sensor is in low power mode
  TEMP_STATE_MAX                       ///< Maximum state marker
} HfTempState_t;

/**
 * @brief Temperature measurement units
 */
typedef enum {
  TEMP_UNIT_CELSIUS = 0,               ///< Celsius (°C)
  TEMP_UNIT_FAHRENHEIT,                ///< Fahrenheit (°F)
  TEMP_UNIT_KELVIN,                    ///< Kelvin (K)
  TEMP_UNIT_RANKINE,                   ///< Rankine (°R)
  TEMP_UNIT_MAX                        ///< Maximum unit marker
} HfTempUnit_t;

/**
 * @brief Temperature sensor capabilities flags
 */
typedef enum {
  TEMP_CAP_NONE = 0x00,                ///< No special capabilities
  TEMP_CAP_THRESHOLD_MONITORING = 0x01, ///< Supports threshold monitoring
  TEMP_CAP_CONTINUOUS_READING = 0x02,   ///< Supports continuous reading
  TEMP_CAP_CALIBRATION = 0x04,         ///< Supports calibration
  TEMP_CAP_POWER_MANAGEMENT = 0x08,    ///< Supports power management
  TEMP_CAP_SELF_TEST = 0x10,           ///< Supports self-test
  TEMP_CAP_ALARM_OUTPUT = 0x20,        ///< Has alarm output capability
  TEMP_CAP_HIGH_PRECISION = 0x40,      ///< High precision measurements
  TEMP_CAP_FAST_RESPONSE = 0x80        ///< Fast response time
} HfTempCapabilities_t;

//--------------------------------------
//  HardFOC Temperature Configuration
//--------------------------------------

/**
 * @brief Temperature sensor configuration structure
 */
typedef struct {
  float range_min_celsius;             ///< Minimum temperature range in Celsius
  float range_max_celsius;             ///< Maximum temperature range in Celsius
  float resolution;                    ///< Temperature resolution in Celsius
  uint32_t sample_rate_hz;             ///< Sampling rate in Hz (0 = on-demand)
  bool enable_threshold_monitoring;    ///< Enable threshold monitoring
  float high_threshold_celsius;        ///< High temperature threshold in Celsius
  float low_threshold_celsius;         ///< Low temperature threshold in Celsius
  bool enable_power_management;        ///< Enable power management features
  bool enable_calibration;             ///< Enable calibration if supported
  uint32_t timeout_ms;                 ///< Operation timeout in milliseconds
  HfTempSensorType_t sensor_type;      ///< Sensor type (hint for implementation)
  uint32_t capabilities;               ///< Required capabilities (HfTempCapabilities_t flags)
} HfTempConfig_t;

/**
 * @brief Temperature reading structure
 */
typedef struct {
  float temperature_celsius;           ///< Temperature in Celsius
  float temperature_raw;               ///< Raw sensor value (implementation specific)
  uint64_t timestamp_us;               ///< Timestamp of reading in microseconds
  HfTempError_t error;                 ///< Error code for this reading
  bool is_valid;                       ///< Whether this reading is valid
  float accuracy_celsius;              ///< Estimated accuracy in Celsius
} HfTempReading_t;

/**
 * @brief Temperature sensor information structure
 */
typedef struct {
  HfTempSensorType_t sensor_type;      ///< Type of temperature sensor
  float min_temp_celsius;              ///< Minimum measurable temperature
  float max_temp_celsius;              ///< Maximum measurable temperature
  float resolution_celsius;            ///< Temperature resolution
  float accuracy_celsius;              ///< Typical accuracy
  uint32_t response_time_ms;           ///< Typical response time
  uint32_t capabilities;               ///< Sensor capabilities (HfTempCapabilities_t flags)
  const char* manufacturer;            ///< Sensor manufacturer
  const char* model;                   ///< Sensor model
  const char* version;                 ///< Driver/firmware version
} HfTempSensorInfo_t;

//--------------------------------------
//  HardFOC Temperature Callback Types
//--------------------------------------

/**
 * @brief Temperature threshold callback function type
 * @param temperature_celsius Current temperature in Celsius
 * @param threshold_type Type of threshold (0=low, 1=high)
 * @param user_data User-provided data pointer
 */
typedef void (*HfTempThresholdCallback_t)(float temperature_celsius, uint32_t threshold_type, void* user_data);

/**
 * @brief Temperature reading callback function type (for continuous monitoring)
 * @param reading Temperature reading structure
 * @param user_data User-provided data pointer
 */
typedef void (*HfTempReadingCallback_t)(const HfTempReading_t* reading, void* user_data);

/**
 * @brief Temperature error callback function type
 * @param error Error code
 * @param error_description Human-readable error description
 * @param user_data User-provided data pointer
 */
typedef void (*HfTempErrorCallback_t)(HfTempError_t error, const char* error_description, void* user_data);

//--------------------------------------
//  Utility Functions and Macros
//--------------------------------------

/**
 * @brief Convert Celsius to Fahrenheit
 * @param celsius Temperature in Celsius
 * @return Temperature in Fahrenheit
 */
#define TEMP_CELSIUS_TO_FAHRENHEIT(celsius) ((celsius) * 9.0f / 5.0f + 32.0f)

/**
 * @brief Convert Fahrenheit to Celsius
 * @param fahrenheit Temperature in Fahrenheit
 * @return Temperature in Celsius
 */
#define TEMP_FAHRENHEIT_TO_CELSIUS(fahrenheit) (((fahrenheit) - 32.0f) * 5.0f / 9.0f)

/**
 * @brief Convert Celsius to Kelvin
 * @param celsius Temperature in Celsius
 * @return Temperature in Kelvin
 */
#define TEMP_CELSIUS_TO_KELVIN(celsius) ((celsius) + 273.15f)

/**
 * @brief Convert Kelvin to Celsius
 * @param kelvin Temperature in Kelvin
 * @return Temperature in Celsius
 */
#define TEMP_KELVIN_TO_CELSIUS(kelvin) ((kelvin) - 273.15f)

/**
 * @brief Get error description string
 * @param error Error code
 * @return Human-readable error description
 */
const char* hf_temp_get_error_string(HfTempError_t error);

/**
 * @brief Check if temperature is within range
 * @param temperature Temperature to check
 * @param min_temp Minimum temperature
 * @param max_temp Maximum temperature
 * @return true if within range, false otherwise
 */
bool hf_temp_is_in_range(float temperature, float min_temp, float max_temp);

/**
 * @brief Default temperature sensor configuration
 */
#define HF_TEMP_CONFIG_DEFAULT() {        \
    .range_min_celsius = -40.0f,          \
    .range_max_celsius = 125.0f,          \
    .resolution = 0.1f,                   \
    .sample_rate_hz = 0,                  \
    .enable_threshold_monitoring = false, \
    .high_threshold_celsius = 100.0f,     \
    .low_threshold_celsius = -20.0f,      \
    .enable_power_management = false,     \
    .enable_calibration = false,          \
    .timeout_ms = 1000,                   \
    .sensor_type = TEMP_SENSOR_TYPE_UNKNOWN, \
    .capabilities = TEMP_CAP_NONE         \
}

//--------------------------------------
//  BaseTemperature Abstract Class
//--------------------------------------

namespace HardFOC {

/**
 * @class BaseTemperature
 * @brief Abstract base class for all temperature sensor implementations
 * 
 * This class defines the common interface that all temperature sensor implementations
 * must follow. It provides a consistent API for temperature measurement, configuration,
 * and monitoring across different sensor types and platforms.
 * 
 * Key features:
 * - Abstract interface for temperature reading
 * - Standardized error handling
 * - Support for various temperature units
 * - Threshold monitoring capabilities
 * - Power management support
 * - Calibration interface
 * 
 * @note This is a header-only abstract base class
 * @note All concrete implementations must inherit from this class
 * @note Thread safety is implementation-dependent
 */
class BaseTemperature {
public:
    //--------------------------------------
    //  Constructor & Destructor
    //--------------------------------------
    
    /**
     * @brief Default constructor
     */
    BaseTemperature() = default;
    
    /**
     * @brief Virtual destructor
     */
    virtual ~BaseTemperature() = default;
    
    //--------------------------------------
    //  Core Temperature Interface
    //--------------------------------------
    
    /**
     * @brief Initialize the temperature sensor
     * @param config Configuration structure
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t initialize(const HfTempConfig_t* config) = 0;
    
    /**
     * @brief Deinitialize the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t deinitialize() = 0;
    
    /**
     * @brief Enable the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t enable() = 0;
    
    /**
     * @brief Disable the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t disable() = 0;
    
    /**
     * @brief Check if the sensor is initialized
     * @return true if initialized, false otherwise
     */
    virtual bool is_initialized() const = 0;
    
    /**
     * @brief Check if the sensor is enabled
     * @return true if enabled, false otherwise
     */
    virtual bool is_enabled() const = 0;
    
    /**
     * @brief Get current sensor state
     * @return Current sensor state
     */
    virtual HfTempState_t get_state() const = 0;
    
    //--------------------------------------
    //  Temperature Reading Interface
    //--------------------------------------
    
    /**
     * @brief Read temperature in Celsius (blocking)
     * @param temperature_celsius Pointer to store temperature value
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t read_celsius(float* temperature_celsius) = 0;
    
    /**
     * @brief Read temperature with full information (blocking)
     * @param reading Pointer to store complete reading information
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t read_temperature(HfTempReading_t* reading) = 0;
    
    /**
     * @brief Start asynchronous temperature reading
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t start_async_read() = 0;
    
    /**
     * @brief Check if asynchronous reading is complete
     * @return true if reading is complete, false otherwise
     */
    virtual bool is_read_complete() const = 0;
    
    /**
     * @brief Get result of asynchronous reading
     * @param reading Pointer to store reading result
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_async_result(HfTempReading_t* reading) = 0;
    
    //--------------------------------------
    //  Temperature Conversion Utilities
    //--------------------------------------
    
    /**
     * @brief Read temperature in Fahrenheit
     * @param temperature_fahrenheit Pointer to store temperature value
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t read_fahrenheit(float* temperature_fahrenheit) {
        float celsius;
        HfTempError_t error = read_celsius(&celsius);
        if (error == TEMP_SUCCESS) {
            *temperature_fahrenheit = TEMP_CELSIUS_TO_FAHRENHEIT(celsius);
        }
        return error;
    }
    
    /**
     * @brief Read temperature in Kelvin
     * @param temperature_kelvin Pointer to store temperature value
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t read_kelvin(float* temperature_kelvin) {
        float celsius;
        HfTempError_t error = read_celsius(&celsius);
        if (error == TEMP_SUCCESS) {
            *temperature_kelvin = TEMP_CELSIUS_TO_KELVIN(celsius);
        }
        return error;
    }
    
    /**
     * @brief Read temperature in specified unit
     * @param temperature Pointer to store temperature value
     * @param unit Desired temperature unit
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t read_temperature_unit(float* temperature, HfTempUnit_t unit) {
        float celsius;
        HfTempError_t error = read_celsius(&celsius);
        if (error == TEMP_SUCCESS) {
            switch (unit) {
                case TEMP_UNIT_CELSIUS:
                    *temperature = celsius;
                    break;
                case TEMP_UNIT_FAHRENHEIT:
                    *temperature = TEMP_CELSIUS_TO_FAHRENHEIT(celsius);
                    break;
                case TEMP_UNIT_KELVIN:
                    *temperature = TEMP_CELSIUS_TO_KELVIN(celsius);
                    break;
                case TEMP_UNIT_RANKINE:
                    *temperature = TEMP_CELSIUS_TO_KELVIN(celsius) * 9.0f / 5.0f;
                    break;
                default:
                    error = TEMP_ERR_INVALID_PARAMETER;
                    break;
            }
        }
        return error;
    }
    
    //--------------------------------------
    //  Configuration Interface
    //--------------------------------------
    
    /**
     * @brief Set temperature measurement range
     * @param min_celsius Minimum temperature in Celsius
     * @param max_celsius Maximum temperature in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t set_range(float min_celsius, float max_celsius) = 0;
    
    /**
     * @brief Get temperature measurement range
     * @param min_celsius Pointer to store minimum temperature
     * @param max_celsius Pointer to store maximum temperature
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_range(float* min_celsius, float* max_celsius) const = 0;
    
    /**
     * @brief Set measurement resolution
     * @param resolution_celsius Resolution in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t set_resolution(float resolution_celsius) = 0;
    
    /**
     * @brief Get measurement resolution
     * @param resolution_celsius Pointer to store resolution
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_resolution(float* resolution_celsius) const = 0;
    
    //--------------------------------------
    //  Threshold Monitoring Interface
    //--------------------------------------
    
    /**
     * @brief Set temperature thresholds
     * @param low_threshold_celsius Low temperature threshold
     * @param high_threshold_celsius High temperature threshold
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t set_thresholds(float low_threshold_celsius, float high_threshold_celsius) = 0;
    
    /**
     * @brief Get temperature thresholds
     * @param low_threshold_celsius Pointer to store low threshold
     * @param high_threshold_celsius Pointer to store high threshold
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_thresholds(float* low_threshold_celsius, float* high_threshold_celsius) const = 0;
    
    /**
     * @brief Enable threshold monitoring
     * @param callback Callback function for threshold events
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t enable_threshold_monitoring(HfTempThresholdCallback_t callback, void* user_data) = 0;
    
    /**
     * @brief Disable threshold monitoring
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t disable_threshold_monitoring() = 0;
    
    //--------------------------------------
    //  Continuous Monitoring Interface
    //--------------------------------------
    
    /**
     * @brief Start continuous temperature monitoring
     * @param sample_rate_hz Sampling rate in Hz
     * @param callback Callback function for each reading
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t start_continuous_monitoring(uint32_t sample_rate_hz, 
                                                     HfTempReadingCallback_t callback, 
                                                     void* user_data) = 0;
    
    /**
     * @brief Stop continuous temperature monitoring
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t stop_continuous_monitoring() = 0;
    
    /**
     * @brief Check if continuous monitoring is active
     * @return true if monitoring is active, false otherwise
     */
    virtual bool is_monitoring_active() const = 0;
    
    //--------------------------------------
    //  Calibration Interface
    //--------------------------------------
    
    /**
     * @brief Perform sensor calibration
     * @param reference_temperature_celsius Known reference temperature
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t calibrate(float reference_temperature_celsius) = 0;
    
    /**
     * @brief Set calibration offset
     * @param offset_celsius Calibration offset in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t set_calibration_offset(float offset_celsius) = 0;
    
    /**
     * @brief Get calibration offset
     * @param offset_celsius Pointer to store calibration offset
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_calibration_offset(float* offset_celsius) const = 0;
    
    /**
     * @brief Reset calibration to default
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t reset_calibration() = 0;
    
    //--------------------------------------
    //  Power Management Interface
    //--------------------------------------
    
    /**
     * @brief Enter low power mode
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t enter_sleep_mode() = 0;
    
    /**
     * @brief Exit low power mode
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t exit_sleep_mode() = 0;
    
    /**
     * @brief Check if sensor is in sleep mode
     * @return true if in sleep mode, false otherwise
     */
    virtual bool is_sleeping() const = 0;
    
    //--------------------------------------
    //  Information Interface
    //--------------------------------------
    
    /**
     * @brief Get sensor information
     * @param info Pointer to store sensor information
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t get_sensor_info(HfTempSensorInfo_t* info) const = 0;
    
    /**
     * @brief Get sensor capabilities
     * @return Capabilities flags (HfTempCapabilities_t)
     */
    virtual uint32_t get_capabilities() const = 0;
    
    /**
     * @brief Check if a specific capability is supported
     * @param capability Capability to check
     * @return true if supported, false otherwise
     */
    virtual bool has_capability(HfTempCapabilities_t capability) const {
        return (get_capabilities() & capability) != 0;
    }
    
    //--------------------------------------
    //  Error Handling Interface
    //--------------------------------------
    
    /**
     * @brief Set error callback function
     * @param callback Error callback function
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t set_error_callback(HfTempErrorCallback_t callback, void* user_data) = 0;
    
    /**
     * @brief Clear error callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t clear_error_callback() = 0;
    
    /**
     * @brief Get last error
     * @return Last error code
     */
    virtual HfTempError_t get_last_error() const = 0;
    
    /**
     * @brief Clear last error
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t clear_last_error() = 0;
    
    //--------------------------------------
    //  Self-Test Interface
    //--------------------------------------
    
    /**
     * @brief Perform sensor self-test
     * @return Error code (TEMP_SUCCESS on success)
     */
    virtual HfTempError_t self_test() = 0;
    
    /**
     * @brief Check sensor health status
     * @return Error code (TEMP_SUCCESS if healthy)
     */
    virtual HfTempError_t check_health() = 0;

protected:
    //--------------------------------------
    //  Protected Members
    //--------------------------------------
    
    // Implementation-specific members can be added by derived classes
    
private:
    //--------------------------------------
    //  Prevent Copying
    //--------------------------------------
    
    BaseTemperature(const BaseTemperature&) = delete;
    BaseTemperature& operator=(const BaseTemperature&) = delete;
};

} // namespace HardFOC

//--------------------------------------
//  Error String Utility Implementation
//--------------------------------------

/**
 * @brief Get human-readable error description
 * @param error Error code
 * @return Error description string
 */
inline const char* hf_temp_get_error_string(HfTempError_t error) {
    switch (error) {
#define TEMP_ERROR_STRING(name, code, desc) case name: return desc;
        HF_TEMP_ERR_LIST(TEMP_ERROR_STRING)
#undef TEMP_ERROR_STRING
        default: return "Unknown error";
    }
}

/**
 * @brief Check if temperature is within range
 * @param temperature Temperature to check
 * @param min_temp Minimum temperature
 * @param max_temp Maximum temperature
 * @return true if within range, false otherwise
 */
inline bool hf_temp_is_in_range(float temperature, float min_temp, float max_temp) {
    return (temperature >= min_temp) && (temperature <= max_temp);
}