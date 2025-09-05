/**
 * @file BaseTemperature.h
 * @brief Abstract base class for temperature sensor implementations in the HardFOC system.
 *
 * This header-only file defines the abstract base class for temperature sensing
 * that provides a consistent API across different temperature sensor implementations.
 * Concrete implementations for various platforms and sensor types inherit from this class.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is a header-only abstract base class following the same pattern as BaseGpio/BaseAdc.
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
#define HF_TEMP_ERR_LIST(X)                                                                        \
  /* Success codes */                                                                              \
  X(TEMP_SUCCESS, 0, "Success")                                                                    \
  /* General errors */                                                                             \
  X(TEMP_ERR_FAILURE, 1, "General failure")                                                        \
  X(TEMP_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                \
  X(TEMP_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                        \
  X(TEMP_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                            \
  X(TEMP_ERR_NULL_POINTER, 5, "Null pointer")                                                      \
  X(TEMP_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                    \
  /* Sensor specific errors */                                                                     \
  X(TEMP_ERR_SENSOR_NOT_AVAILABLE, 7, "Temperature sensor not available")                          \
  X(TEMP_ERR_SENSOR_BUSY, 8, "Sensor already in use")                                              \
  X(TEMP_ERR_SENSOR_DISABLED, 9, "Sensor is disabled")                                             \
  X(TEMP_ERR_SENSOR_NOT_READY, 10, "Sensor not ready")                                             \
  /* Reading errors */                                                                             \
  X(TEMP_ERR_READ_FAILED, 11, "Failed to read temperature")                                        \
  X(TEMP_ERR_INVALID_READING, 12, "Invalid temperature reading")                                   \
  X(TEMP_ERR_OUT_OF_RANGE, 13, "Temperature out of sensor range")                                  \
  X(TEMP_ERR_TIMEOUT, 14, "Operation timeout")                                                     \
  /* Configuration errors */                                                                       \
  X(TEMP_ERR_INVALID_RANGE, 15, "Invalid temperature range")                                       \
  X(TEMP_ERR_RANGE_TOO_NARROW, 16, "Temperature range too narrow")                                 \
  X(TEMP_ERR_RANGE_TOO_WIDE, 17, "Temperature range too wide")                                     \
  X(TEMP_ERR_UNSUPPORTED_RANGE, 18, "Unsupported temperature range")                               \
  /* Calibration errors */                                                                         \
  X(TEMP_ERR_CALIBRATION_FAILED, 19, "Calibration failed")                                         \
  X(TEMP_ERR_NOT_CALIBRATED, 20, "Sensor not calibrated")                                          \
  X(TEMP_ERR_INVALID_CALIBRATION, 21, "Invalid calibration data")                                  \
  /* Communication errors */                                                                       \
  X(TEMP_ERR_COMMUNICATION_FAILED, 22, "Communication with sensor failed")                         \
  X(TEMP_ERR_CHECKSUM_FAILED, 23, "Data checksum verification failed")                             \
  /* Power management errors */                                                                    \
  X(TEMP_ERR_POWER_DOWN_FAILED, 24, "Failed to power down sensor")                                 \
  X(TEMP_ERR_POWER_UP_FAILED, 25, "Failed to power up sensor")                                     \
  /* Hardware errors */                                                                            \
  X(TEMP_ERR_HARDWARE_FAULT, 26, "Hardware fault detected")                                        \
  X(TEMP_ERR_OVERCURRENT, 27, "Overcurrent condition")                                             \
  X(TEMP_ERR_OVERVOLTAGE, 28, "Overvoltage condition")                                             \
  X(TEMP_ERR_UNDERVOLTAGE, 29, "Undervoltage condition")                                           \
  X(TEMP_ERR_OVERHEATING, 30, "Sensor overheating")                                                \
  /* Resource errors */                                                                            \
  X(TEMP_ERR_RESOURCE_BUSY, 31, "Required resource is busy")                                       \
  X(TEMP_ERR_RESOURCE_UNAVAILABLE, 32, "Required resource unavailable")                            \
  X(TEMP_ERR_INSUFFICIENT_RESOURCES, 33, "Insufficient system resources")                          \
  /* Operation errors */                                                                           \
  X(TEMP_ERR_OPERATION_ABORTED, 34, "Operation was aborted")                                       \
  X(TEMP_ERR_OPERATION_PENDING, 35, "Operation is pending")                                        \
  X(TEMP_ERR_INVALID_STATE, 36, "Invalid sensor state")                                            \
  X(TEMP_ERR_STATE_TRANSITION_FAILED, 37, "State transition failed")                               \
  /* Data processing errors */                                                                     \
  X(TEMP_ERR_DATA_CORRUPTION, 38, "Data corruption detected")                                      \
  X(TEMP_ERR_CONVERSION_FAILED, 39, "Temperature conversion failed")                               \
  X(TEMP_ERR_FILTERING_FAILED, 40, "Temperature filtering failed")                                 \
  /* Threshold and monitoring errors */                                                            \
  X(TEMP_ERR_THRESHOLD_EXCEEDED, 41, "Temperature threshold exceeded")                             \
  X(TEMP_ERR_INVALID_THRESHOLD, 42, "Invalid threshold value")                                     \
  X(TEMP_ERR_MONITORING_FAILED, 43, "Temperature monitoring failed")                               \
  /* Advanced feature errors */                                                                    \
  X(TEMP_ERR_UNSUPPORTED_OPERATION, 44, "Operation not supported")                                 \
  X(TEMP_ERR_DRIVER_ERROR, 45, "Driver error")                                                     \
  /* Maximum error code marker */                                                                  \
  X(TEMP_ERR_MAX, 46, "Maximum error code")

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
} hf_temp_err_t;

//--------------------------------------
//  HardFOC Temperature Types
//--------------------------------------

/**
 * @brief Temperature sensor types
 */
typedef enum {
  HF_TEMP_SENSOR_TYPE_UNKNOWN = 0,      ///< Unknown or unspecified sensor type
  HF_TEMP_SENSOR_TYPE_INTERNAL,         ///< Internal chip temperature sensor
  HF_TEMP_SENSOR_TYPE_EXTERNAL_DIGITAL, ///< External digital temperature sensor
  HF_TEMP_SENSOR_TYPE_EXTERNAL_ANALOG,  ///< External analog temperature sensor
  HF_TEMP_SENSOR_TYPE_THERMOCOUPLE,     ///< Thermocouple temperature sensor
  HF_TEMP_SENSOR_TYPE_RTD,              ///< Resistance Temperature Detector
  HF_TEMP_SENSOR_TYPE_THERMISTOR,       ///< Thermistor temperature sensor
  HF_TEMP_SENSOR_TYPE_INFRARED,         ///< Infrared temperature sensor
  HF_TEMP_SENSOR_TYPE_MAX               ///< Maximum sensor type marker
} hf_temp_sensor_type_t;

/**
 * @brief Temperature sensor states
 */
typedef enum {
  HF_TEMP_STATE_UNINITIALIZED = 0, ///< Sensor is not initialized
  HF_TEMP_STATE_INITIALIZED,       ///< Sensor is initialized but not enabled
  HF_TEMP_STATE_ENABLED,           ///< Sensor is enabled and ready
  HF_TEMP_STATE_READING,           ///< Sensor is performing a reading
  HF_TEMP_STATE_ERROR,             ///< Sensor is in error state
  HF_TEMP_STATE_DISABLED,          ///< Sensor is disabled
  HF_TEMP_STATE_CALIBRATING,       ///< Sensor is being calibrated
  HF_TEMP_STATE_SLEEPING,          ///< Sensor is in low power mode
  HF_TEMP_STATE_MAX                ///< Maximum state marker
} hf_temp_state_t;

/**
 * @brief Temperature measurement units
 */
typedef enum {
  HF_TEMP_UNIT_CELSIUS = 0, ///< Celsius (°C)
  HF_TEMP_UNIT_FAHRENHEIT,  ///< Fahrenheit (°F)
  HF_TEMP_UNIT_KELVIN,      ///< Kelvin (K)
  HF_TEMP_UNIT_RANKINE,     ///< Rankine (°R)
  HF_TEMP_UNIT_MAX          ///< Maximum unit marker
} hf_temp_unit_t;

/**
 * @brief Temperature sensor capabilities flags
 */
typedef enum {
  HF_TEMP_CAP_NONE = 0x00,                 ///< No special capabilities
  HF_TEMP_CAP_THRESHOLD_MONITORING = 0x01, ///< Supports threshold monitoring
  HF_TEMP_CAP_CONTINUOUS_READING = 0x02,   ///< Supports continuous reading
  HF_TEMP_CAP_CALIBRATION = 0x04,          ///< Supports calibration
  HF_TEMP_CAP_POWER_MANAGEMENT = 0x08,     ///< Supports power management
  HF_TEMP_CAP_SELF_TEST = 0x10,            ///< Supports self-test
  HF_TEMP_CAP_ALARM_OUTPUT = 0x20,         ///< Has alarm output capability
  HF_TEMP_CAP_HIGH_PRECISION = 0x40,       ///< High precision measurements
  HF_TEMP_CAP_FAST_RESPONSE = 0x80         ///< Fast response time
} hf_temp_capabilities_t;

//--------------------------------------
//  HardFOC Temperature Configuration
//--------------------------------------

/**
 * @brief Temperature sensor configuration structure
 */
typedef struct {
  float range_min_celsius;           ///< Minimum temperature range in Celsius
  float range_max_celsius;           ///< Maximum temperature range in Celsius
  float resolution;                  ///< Temperature resolution in Celsius
  hf_u32_t sample_rate_hz;           ///< Sampling rate in Hz (0 = on-demand)
  bool enable_threshold_monitoring;  ///< Enable threshold monitoring
  float high_threshold_celsius;      ///< High temperature threshold in Celsius
  float low_threshold_celsius;       ///< Low temperature threshold in Celsius
  bool enable_power_management;      ///< Enable power management features
  bool enable_calibration;           ///< Enable calibration if supported
  hf_u32_t timeout_ms;               ///< Operation timeout in milliseconds
  hf_temp_sensor_type_t sensor_type; ///< Sensor type (hint for implementation)
  hf_u32_t capabilities;             ///< Required capabilities (hf_temp_capabilities_t flags)
} hf_temp_config_t;

/**
 * @brief Temperature reading structure
 */
typedef struct {
  float temperature_celsius; ///< Temperature in Celsius
  float temperature_raw;     ///< Raw sensor value (implementation specific)
  hf_u64_t timestamp_us;     ///< Timestamp of reading in microseconds
  hf_temp_err_t error;       ///< Error code for this reading
  bool is_valid;             ///< Whether this reading is valid
  float accuracy_celsius;    ///< Estimated accuracy in Celsius
} hf_temp_reading_t;

/**
 * @brief Temperature sensor information structure
 */
typedef struct {
  hf_temp_sensor_type_t sensor_type; ///< Type of temperature sensor
  float min_temp_celsius;            ///< Minimum measurable temperature
  float max_temp_celsius;            ///< Maximum measurable temperature
  float resolution_celsius;          ///< Temperature resolution
  float accuracy_celsius;            ///< Typical accuracy
  hf_u32_t response_time_ms;         ///< Typical response time
  hf_u32_t capabilities;             ///< Sensor capabilities (hf_temp_capabilities_t flags)
  const char* manufacturer;          ///< Sensor manufacturer
  const char* model;                 ///< Sensor model
  const char* version;               ///< Driver/firmware version
} hf_temp_sensor_info_t;

/**
 * @brief Temperature sensor statistics
 */
typedef struct {
  hf_u32_t total_operations;          ///< Total operations performed
  hf_u32_t successful_operations;     ///< Successful operations
  hf_u32_t failed_operations;         ///< Failed operations
  hf_u32_t temperature_readings;      ///< Number of temperature readings
  hf_u32_t calibration_count;         ///< Number of calibrations performed
  hf_u32_t threshold_violations;      ///< Number of threshold violations
  hf_u32_t average_operation_time_us; ///< Average operation time (microseconds)
  hf_u32_t max_operation_time_us;     ///< Maximum operation time
  hf_u32_t min_operation_time_us;     ///< Minimum operation time
  float min_temperature_celsius;      ///< Minimum temperature recorded
  float max_temperature_celsius;      ///< Maximum temperature recorded
  float avg_temperature_celsius;      ///< Average temperature
} hf_temp_statistics_t;

/**
 * @brief Temperature sensor diagnostics
 */
typedef struct {
  bool sensor_healthy;                 ///< Overall sensor health status
  hf_temp_err_t last_error_code;       ///< Last error code
  hf_u32_t last_error_timestamp;       ///< Last error timestamp
  hf_u32_t consecutive_errors;         ///< Consecutive error count
  bool sensor_available;               ///< Sensor availability status
  bool threshold_monitoring_supported; ///< Threshold monitoring support
  bool threshold_monitoring_enabled;   ///< Threshold monitoring enabled status
  bool continuous_monitoring_active;   ///< Continuous monitoring status
  hf_u32_t current_temperature_raw;    ///< Current raw temperature reading
  bool calibration_valid;              ///< Calibration validity status
} hf_temp_diagnostics_t;

//--------------------------------------
//  HardFOC Temperature Callback Types
//--------------------------------------

// Forward declaration
class BaseTemperature;

/**
 * @brief Temperature threshold callback function type
 * @param temp_sensor Pointer to temperature sensor instance
 * @param temperature_celsius Current temperature in Celsius
 * @param threshold_type Type of threshold (0=low, 1=high)
 * @param user_data User-provided data pointer
 */
using hf_temp_threshold_callback_t =
    std::function<void(BaseTemperature* temp_sensor, float temperature_celsius,
                       hf_u32_t threshold_type, void* user_data)>;

/**
 * @brief Temperature reading callback function type (for continuous monitoring)
 * @param temp_sensor Pointer to temperature sensor instance
 * @param reading Temperature reading structure
 * @param user_data User-provided data pointer
 */
using hf_temp_reading_callback_t = std::function<void(
    BaseTemperature* temp_sensor, const hf_temp_reading_t* reading, void* user_data)>;

/**
 * @brief Temperature error callback function type
 * @param temp_sensor Pointer to temperature sensor instance
 * @param error Error code
 * @param error_description Human-readable error description
 * @param user_data User-provided data pointer
 */
using hf_temp_error_callback_t =
    std::function<void(BaseTemperature* temp_sensor, hf_temp_err_t error,
                       const char* error_description, void* user_data)>;

//--------------------------------------
//  Utility Functions and Macros
//--------------------------------------

/**
 * @brief Convert Celsius to Fahrenheit
 * @param celsius Temperature in Celsius
 * @return Temperature in Fahrenheit
 */
#define HF_TEMP_CELSIUS_TO_FAHRENHEIT(celsius) ((celsius) * 9.0f / 5.0f + 32.0f)

/**
 * @brief Convert Fahrenheit to Celsius
 * @param fahrenheit Temperature in Fahrenheit
 * @return Temperature in Celsius
 */
#define HF_TEMP_FAHRENHEIT_TO_CELSIUS(fahrenheit) (((fahrenheit) - 32.0f) * 5.0f / 9.0f)

/**
 * @brief Convert Celsius to Kelvin
 * @param celsius Temperature in Celsius
 * @return Temperature in Kelvin
 */
#define HF_TEMP_CELSIUS_TO_KELVIN(celsius) ((celsius) + 273.15f)

/**
 * @brief Convert Kelvin to Celsius
 * @param kelvin Temperature in Kelvin
 * @return Temperature in Celsius
 */
#define HF_TEMP_KELVIN_TO_CELSIUS(kelvin) ((kelvin) - 273.15f)

/**
 * @brief Default temperature sensor configuration
 */
#define HF_TEMP_CONFIG_DEFAULT()                                                                   \
  {.range_min_celsius = -40.0f,                                                                    \
   .range_max_celsius = 125.0f,                                                                    \
   .resolution = 0.1f,                                                                             \
   .sample_rate_hz = 0,                                                                            \
   .enable_threshold_monitoring = false,                                                           \
   .high_threshold_celsius = 100.0f,                                                               \
   .low_threshold_celsius = -20.0f,                                                                \
   .enable_power_management = false,                                                               \
   .enable_calibration = false,                                                                    \
   .timeout_ms = 1000,                                                                             \
   .sensor_type = HF_TEMP_SENSOR_TYPE_UNKNOWN,                                                     \
   .capabilities = HF_TEMP_CAP_NONE}

/**
 * @brief Get error description string
 * @param error Error code
 * @return Human-readable error description
 */
const char* GetTempErrorString(hf_temp_err_t error) noexcept;

/**
 * @brief Check if temperature is within range
 * @param temperature Temperature to check
 * @param min_temp Minimum temperature
 * @param max_temp Maximum temperature
 * @return true if within range, false otherwise
 */
bool IsTempInRange(float temperature, float min_temp, float max_temp) noexcept;

//--------------------------------------
//  BaseTemperature Abstract Class
//--------------------------------------

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
 * - Threshold monitoring capabilities (advanced feature)
 * - Power management support (advanced feature)
 * - Calibration interface (advanced feature)
 * - Lazy initialization pattern
 *
 * @note This is a header-only abstract base class
 * @note All concrete implementations must inherit from this class
 * @note Thread safety is implementation-dependent
 * @note Advanced features return TEMP_ERR_UNSUPPORTED_OPERATION if not supported
 */
class BaseTemperature {
public:
  //==============================================================//
  // CONSTRUCTORS AND DESTRUCTOR
  //==============================================================//

  /**
   * @brief Copy constructor is deleted to avoid copying instances.
   */
  BaseTemperature(const BaseTemperature&) = delete;

  /**
   * @brief Assignment operator is deleted to avoid copying instances.
   */
  BaseTemperature& operator=(const BaseTemperature&) = delete;

  /**
   * @brief Virtual destructor for proper cleanup of derived classes.
   */
  virtual ~BaseTemperature() = default;

  //==============================================================//
  // INITIALIZATION AND STATUS
  //==============================================================//

  /**
   * @brief Check if the temperature sensor is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept;

  /**
   * @brief Ensures the sensor is initialized (lazy initialization).
   * @return true if initialized successfully, false otherwise
   */
  bool EnsureInitialized() noexcept;

  /**
   * @brief Ensures the sensor is deinitialized (lazy deinitialization).
   * @return true if deinitialized successfully, false otherwise
   */
  bool EnsureDeinitialized() noexcept;

  /**
   * @brief Get current sensor state
   * @return Current sensor state
   */
  [[nodiscard]] hf_temp_state_t GetState() const noexcept;

protected:
  //==============================================================//
  // [PROTECTED] PURE VIRTUAL IMPLEMENTATIONS - PLATFORM SPECIFIC
  //==============================================================//

  /**
   * @brief Platform-specific implementation for initialization
   * @return true if initialization successful, false otherwise
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Platform-specific implementation for deinitialization
   * @return true if deinitialization successful, false otherwise
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Platform-specific implementation for reading temperature in Celsius
   * @param temperature_celsius Pointer to store temperature value
   * @return Error code (TEMP_SUCCESS on success)
   */
  virtual hf_temp_err_t ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept = 0;

public:
  //==============================================================//
  // CORE TEMPERATURE INTERFACE
  //==============================================================//

  /**
   * @brief Read temperature in Celsius (blocking)
   * @param temperature_celsius Pointer to store temperature value
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperatureCelsius(float* temperature_celsius) noexcept;

  /**
   * @brief Read temperature with full information (blocking)
   * @param reading Pointer to store complete reading information
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperature(hf_temp_reading_t* reading) noexcept;

  //==============================================================//
  // TEMPERATURE CONVERSION UTILITIES
  //==============================================================//

  /**
   * @brief Read temperature in Fahrenheit
   * @param temperature_fahrenheit Pointer to store temperature value
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperatureFahrenheit(float* temperature_fahrenheit) noexcept;

  /**
   * @brief Read temperature in Kelvin
   * @param temperature_kelvin Pointer to store temperature value
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperatureKelvin(float* temperature_kelvin) noexcept;

  /**
   * @brief Read temperature in specified unit
   * @param temperature Pointer to store temperature value
   * @param unit Desired temperature unit
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperatureUnit(float* temperature, hf_temp_unit_t unit) noexcept;

  //==============================================================//
  // INFORMATION INTERFACE
  //==============================================================//

  /**
   * @brief Get sensor information
   * @param info Pointer to store sensor information
   * @return Error code (TEMP_SUCCESS on success)
   */
  virtual hf_temp_err_t GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept = 0;

  /**
   * @brief Get sensor capabilities
   * @return Capabilities flags (hf_temp_capabilities_t)
   */
  [[nodiscard]] virtual hf_u32_t GetCapabilities() const noexcept = 0;

  /**
   * @brief Check if a specific capability is supported
   * @param capability Capability to check
   * @return true if supported, false otherwise
   */
  [[nodiscard]] bool HasCapability(hf_temp_capabilities_t capability) const noexcept;

  //==============================================================//
  // ADVANCED FEATURES (OPTIONAL - MAY RETURN UNSUPPORTED)
  //==============================================================//

  /**
   * @brief Set temperature measurement range (advanced feature)
   * @param min_celsius Minimum temperature in Celsius
   * @param max_celsius Maximum temperature in Celsius
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t SetRange(float min_celsius, float max_celsius) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get temperature measurement range (advanced feature)
   * @param min_celsius Pointer to store minimum temperature
   * @param max_celsius Pointer to store maximum temperature
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetRange(float* min_celsius, float* max_celsius) const noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Set measurement resolution (advanced feature)
   * @param resolution_celsius Resolution in Celsius
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t SetResolution(float resolution_celsius) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get measurement resolution (advanced feature)
   * @param resolution_celsius Pointer to store resolution
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetResolution(float* resolution_celsius) const noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Set temperature thresholds (advanced feature)
   * @param low_threshold_celsius Low temperature threshold
   * @param high_threshold_celsius High temperature threshold
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t SetThresholds(float low_threshold_celsius,
                                      float high_threshold_celsius) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get temperature thresholds (advanced feature)
   * @param low_threshold_celsius Pointer to store low threshold
   * @param high_threshold_celsius Pointer to store high threshold
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetThresholds(float* low_threshold_celsius,
                                      float* high_threshold_celsius) const noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Enable threshold monitoring (advanced feature)
   * @param callback Callback function for threshold events
   * @param user_data User data to pass to callback
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t EnableThresholdMonitoring(hf_temp_threshold_callback_t callback,
                                                  void* user_data) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Disable threshold monitoring (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t DisableThresholdMonitoring() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Start continuous temperature monitoring (advanced feature)
   * @param sample_rate_hz Sampling rate in Hz
   * @param callback Callback function for each reading
   * @param user_data User data to pass to callback
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t StartContinuousMonitoring(hf_u32_t sample_rate_hz,
                                                  hf_temp_reading_callback_t callback,
                                                  void* user_data) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Stop continuous temperature monitoring (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t StopContinuousMonitoring() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Check if continuous monitoring is active (advanced feature)
   * @return true if monitoring is active, false otherwise
   */
  [[nodiscard]] virtual bool IsMonitoringActive() const noexcept {
    return false; // Default: not supported
  }

  /**
   * @brief Perform sensor calibration (advanced feature)
   * @param reference_temperature_celsius Known reference temperature
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t Calibrate(float reference_temperature_celsius) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Set calibration offset (advanced feature)
   * @param offset_celsius Calibration offset in Celsius
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t SetCalibrationOffset(float offset_celsius) noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get calibration offset (advanced feature)
   * @param offset_celsius Pointer to store calibration offset
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetCalibrationOffset(float* offset_celsius) const noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset calibration to default (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t ResetCalibration() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Enter low power mode (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t EnterSleepMode() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Exit low power mode (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t ExitSleepMode() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Check if sensor is in sleep mode (advanced feature)
   * @return true if in sleep mode, false otherwise
   */
  [[nodiscard]] virtual bool IsSleeping() const noexcept {
    return false; // Default: not supported
  }

  /**
   * @brief Perform sensor self-test (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t SelfTest() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Check sensor health status (advanced feature)
   * @return Error code (TEMP_SUCCESS if healthy, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t CheckHealth() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get operation statistics (advanced feature)
   * @param statistics Reference to statistics structure to fill
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetStatistics(hf_temp_statistics_t& statistics) noexcept {
    statistics = hf_temp_statistics_t{}; // Reset to defaults
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get diagnostic information (advanced feature)
   * @param diagnostics Reference to diagnostics structure to fill
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t GetDiagnostics(hf_temp_diagnostics_t& diagnostics) noexcept {
    diagnostics = hf_temp_diagnostics_t{}; // Reset to defaults
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset operation statistics (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t ResetStatistics() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset diagnostic information (advanced feature)
   * @return Error code (TEMP_SUCCESS on success, TEMP_ERR_UNSUPPORTED_OPERATION if not supported)
   */
  virtual hf_temp_err_t ResetDiagnostics() noexcept {
    return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  //==============================================================//
  // [PROTECTED] CONSTRUCTOR AND MEMBER VARIABLES
  //==============================================================//

  /**
   * @brief Protected default constructor
   */
  BaseTemperature() noexcept : initialized_(false), current_state_(HF_TEMP_STATE_UNINITIALIZED) {}

  /**
   * @brief Protected helper to validate basic operation preconditions
   * @return Error code (TEMP_SUCCESS if valid to proceed)
   */
  hf_temp_err_t ValidateBasicOperation() const noexcept;

  //==============================================================//
  // [PROTECTED] MEMBER VARIABLES
  //==============================================================//

  bool initialized_;              ///< Initialization status
  hf_temp_state_t current_state_; ///< Current sensor state
};

//--------------------------------------
//  Inline Implementations
//--------------------------------------

inline bool BaseTemperature::IsInitialized() const noexcept {
  return initialized_;
}

inline bool BaseTemperature::EnsureInitialized() noexcept {
  if (!initialized_) {
    initialized_ = Initialize();
    if (initialized_) {
      current_state_ = HF_TEMP_STATE_INITIALIZED;
    }
  }
  return initialized_;
}

inline bool BaseTemperature::EnsureDeinitialized() noexcept {
  if (initialized_) {
    initialized_ = !Deinitialize();
    if (!initialized_) {
      current_state_ = HF_TEMP_STATE_UNINITIALIZED;
    }
  }
  return !initialized_;
}

inline hf_temp_state_t BaseTemperature::GetState() const noexcept {
  return current_state_;
}

inline bool BaseTemperature::HasCapability(hf_temp_capabilities_t capability) const noexcept {
  return (GetCapabilities() & capability) != 0;
}

inline hf_temp_err_t BaseTemperature::ValidateBasicOperation() const noexcept {
  if (!initialized_) {
    return hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
  }
  return hf_temp_err_t::TEMP_SUCCESS;
}

inline hf_temp_err_t BaseTemperature::ReadTemperatureCelsius(float* temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
  }

  hf_temp_err_t validation = ValidateBasicOperation();
  if (validation != hf_temp_err_t::TEMP_SUCCESS) {
    return validation;
  }

  current_state_ = HF_TEMP_STATE_READING;
  hf_temp_err_t result = ReadTemperatureCelsiusImpl(temperature_celsius);
  current_state_ =
      (result == hf_temp_err_t::TEMP_SUCCESS) ? HF_TEMP_STATE_INITIALIZED : HF_TEMP_STATE_ERROR;

  return result;
}

inline hf_temp_err_t BaseTemperature::ReadTemperature(hf_temp_reading_t* reading) noexcept {
  if (reading == nullptr) {
    return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
  }

  // Initialize reading structure
  reading->error = hf_temp_err_t::TEMP_SUCCESS;
  reading->is_valid = false;
  reading->timestamp_us = 0; // Implementation should fill this
  reading->accuracy_celsius = 0.0f;
  reading->temperature_raw = 0.0f;

  float temperature;
  hf_temp_err_t error = ReadTemperatureCelsius(&temperature);

  reading->temperature_celsius = temperature;
  reading->error = error;
  reading->is_valid = (error == hf_temp_err_t::TEMP_SUCCESS);

  return error;
}

inline hf_temp_err_t BaseTemperature::ReadTemperatureFahrenheit(
    float* temperature_fahrenheit) noexcept {
  if (temperature_fahrenheit == nullptr) {
    return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
  }

  float celsius;
  hf_temp_err_t error = ReadTemperatureCelsius(&celsius);
  if (error == hf_temp_err_t::TEMP_SUCCESS) {
    *temperature_fahrenheit = HF_TEMP_CELSIUS_TO_FAHRENHEIT(celsius);
  }
  return error;
}

inline hf_temp_err_t BaseTemperature::ReadTemperatureKelvin(float* temperature_kelvin) noexcept {
  if (temperature_kelvin == nullptr) {
    return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
  }

  float celsius;
  hf_temp_err_t error = ReadTemperatureCelsius(&celsius);
  if (error == hf_temp_err_t::TEMP_SUCCESS) {
    *temperature_kelvin = HF_TEMP_CELSIUS_TO_KELVIN(celsius);
  }
  return error;
}

inline hf_temp_err_t BaseTemperature::ReadTemperatureUnit(float* temperature,
                                                          hf_temp_unit_t unit) noexcept {
  if (temperature == nullptr) {
    return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
  }

  float celsius;
  hf_temp_err_t error = ReadTemperatureCelsius(&celsius);
  if (error == hf_temp_err_t::TEMP_SUCCESS) {
    switch (unit) {
    case HF_TEMP_UNIT_CELSIUS:
      *temperature = celsius;
      break;
    case HF_TEMP_UNIT_FAHRENHEIT:
      *temperature = HF_TEMP_CELSIUS_TO_FAHRENHEIT(celsius);
      break;
    case HF_TEMP_UNIT_KELVIN:
      *temperature = HF_TEMP_CELSIUS_TO_KELVIN(celsius);
      break;
    case HF_TEMP_UNIT_RANKINE:
      *temperature = HF_TEMP_CELSIUS_TO_KELVIN(celsius) * 9.0f / 5.0f;
      break;
    default:
      error = hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER;
      break;
    }
  }
  return error;
}

//--------------------------------------
//  Error String Utility Implementation
//--------------------------------------

/**
 * @brief Get human-readable error description
 * @param error Error code
 * @return Error description string
 */
inline const char* GetTempErrorString(hf_temp_err_t error) noexcept {
  switch (error) {
#define TEMP_ERROR_STRING(name, code, desc)                                                        \
  case name:                                                                                       \
    return desc;
    HF_TEMP_ERR_LIST(TEMP_ERROR_STRING)
#undef TEMP_ERROR_STRING
  default:
    return "Unknown error";
  }
}

/**
 * @brief Check if temperature is within range
 * @param temperature Temperature to check
 * @param min_temp Minimum temperature
 * @param max_temp Maximum temperature
 * @return true if within range, false otherwise
 */
inline bool IsTempInRange(float temperature, float min_temp, float max_temp) noexcept {
  return (temperature >= min_temp) && (temperature <= max_temp);
}
