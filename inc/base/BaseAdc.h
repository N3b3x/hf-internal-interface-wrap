/**
 * @file BaseAdc.h
 * @brief Abstract base class for ADC implementations in the HardFOC system.
 *
 * This file contains the declaration of the BaseAdc abstract class, which provides
 * a common interface and comprehensive features for all ADC implementations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class defines the unified ADC API that all ADC controller implementations
 * must provide. It ensures a consistent API across different platforms and ADC
 * controller types, making the system extensible and maintainable.
 *
 * @note This is a header-only abstract base class.
 *
 * @example EspAdc.h
 * This example demonstrates the ESP32 ADC implementation that shows how to use
 * the base ADC API with ESP32-specific features and hardware capabilities.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>
#include <string_view>

//=======================================//
//  ADC Error Codes (Table)
//=======================================//

/**
 * @brief ADC error codes
 * @details Comprehensive error enumeration for all ADC operations in the system.
 *          This enumeration is used across all ADC-related classes to provide
 *          consistent error reporting and handling.
 */

#define HF_ADC_ERR_LIST(X)                                                                         \
  /* Success codes */                                                                              \
  X(ADC_SUCCESS, 0, "Success")                                                                     \
  /* General errors */                                                                             \
  X(ADC_ERR_FAILURE, 1, "General failure")                                                         \
  X(ADC_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                 \
  X(ADC_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                         \
  X(ADC_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                             \
  X(ADC_ERR_NULL_POINTER, 5, "Null pointer")                                                       \
  X(ADC_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                     \
  /* Channel errors */                                                                             \
  X(ADC_ERR_CHANNEL_NOT_FOUND, 7, "Channel not found")                                             \
  X(ADC_ERR_CHANNEL_NOT_ENABLED, 8, "Channel not enabled")                                         \
  X(ADC_ERR_CHANNEL_NOT_CONFIGURED, 9, "Channel not configured")                                   \
  X(ADC_ERR_CHANNEL_ALREADY_REGISTERED, 10, "Channel already registered")                          \
  X(ADC_ERR_CHANNEL_READ_ERR, 11, "Channel read error")                                            \
  X(ADC_ERR_CHANNEL_WRITE_ERR, 12, "Channel write error")                                          \
  X(ADC_ERR_INVALID_CHANNEL, 13, "Invalid channel")                                                \
  X(ADC_ERR_CHANNEL_BUSY, 14, "Channel busy")                                                      \
  /* Sampling errors */                                                                            \
  X(ADC_ERR_INVALID_SAMPLE_COUNT, 15, "Invalid sample count")                                      \
  X(ADC_ERR_SAMPLE_TIMEOUT, 16, "Sample timeout")                                                  \
  X(ADC_ERR_SAMPLE_OVERFLOW, 17, "Sample overflow")                                                \
  X(ADC_ERR_SAMPLE_UNDERFLOW, 18, "Sample underflow")                                              \
  /* Hardware errors */                                                                            \
  X(ADC_ERR_HARDWARE_FAULT, 19, "Hardware fault")                                                  \
  X(ADC_ERR_COMMUNICATION_FAILURE, 20, "Communication failure")                                    \
  X(ADC_ERR_DEVICE_NOT_RESPONDING, 21, "Device not responding")                                    \
  X(ADC_ERR_CALIBRATION_FAILURE, 22, "Calibration failure")                                        \
  X(ADC_ERR_VOLTAGE_OUT_OF_RANGE, 23, "Voltage out of range")                                      \
  /* Configuration errors */                                                                       \
  X(ADC_ERR_INVALID_CONFIGURATION, 24, "Invalid configuration")                                    \
  X(ADC_ERR_UNSUPPORTED_OPERATION, 25, "Unsupported operation")                                    \
  X(ADC_ERR_RESOURCE_BUSY, 26, "Resource busy")                                                    \
  X(ADC_ERR_RESOURCE_UNAVAILABLE, 27, "Resource unavailable")                                      \
  /* Calibration specific errors */                                                                \
  X(ADC_ERR_CALIBRATION_NOT_FOUND, 28, "Calibration data not found")                               \
  X(ADC_ERR_CALIBRATION_INVALID, 29, "Invalid calibration data")                                   \
  X(ADC_ERR_CALIBRATION_EXPIRED, 30, "Calibration has expired")                                    \
  X(ADC_ERR_CALIBRATION_DRIFT, 31, "Calibration drift detected")                                   \
  X(ADC_ERR_CALIBRATION_POINTS_INSUFFICIENT, 32, "Insufficient calibration points")                \
  X(ADC_ERR_CALIBRATION_POINTS_INVALID, 33, "Invalid calibration points")                          \
  X(ADC_ERR_CALIBRATION_LINEARITY_ERROR, 34, "Calibration linearity error")                        \
  X(ADC_ERR_CALIBRATION_STORAGE_FAILURE, 35, "Calibration storage failure")                        \
  X(ADC_ERR_CALIBRATION_LOAD_FAILURE, 36, "Calibration load failure")                              \
  X(ADC_ERR_CALIBRATION_VERIFICATION_FAILED, 37, "Calibration verification failed")                \
  X(ADC_ERR_CALIBRATION_TEMPERATURE_ERROR, 38, "Temperature compensation error")                   \
  X(ADC_ERR_CALIBRATION_POLYNOMIAL_ERROR, 39, "Polynomial calibration error")                      \
  /* System errors */                                                                              \
  X(ADC_ERR_SYSTEM_ERROR, 40, "System error")                                                      \
  X(ADC_ERR_PERMISSION_DENIED, 41, "Permission denied")                                            \
  X(ADC_ERR_OPERATION_ABORTED, 42, "Operation aborted")                                            \
                                                                                                   \
  /* Extended ADC errors (for ESP32 compatibility) */                                              \
  X(ADC_ERR_INITIALIZATION_FAILED, 43, "Initialization failed")                                    \
  X(ADC_ERR_INVALID_PARAM, 44, "Invalid parameter")                                                \
  X(ADC_ERR_TIMEOUT, 45, "Operation timeout")                                                      \
  X(ADC_ERR_NOT_SUPPORTED, 46, "Not supported")                                                    \
                                                                                                   \
  /* Additional missing error codes */                                                             \
  X(ADC_ERR_INVALID_STATE, 47, "Invalid state")                                                    \
  X(ADC_ERR_DRIVER_ERROR, 48, "Driver error")                                                      \
  X(ADC_ERR_DMA_ERROR, 49, "DMA error")                                                            \
  X(ADC_ERR_FILTER_ERROR, 50, "Filter configuration error")                                        \
  X(ADC_ERR_NO_CALLBACK, 51, "No callback provided")                                               \
  X(ADC_ERR_NOT_STARTED, 52, "Operation not started")                                              \
  X(ADC_ERR_CALIBRATION, 53, "Calibration error")                                                  \
  X(ADC_ERR_BUSY, 54, "Resource busy")                                                             \
  X(ADC_ERR_HARDWARE_FAILURE, 55, "Hardware failure")                                              \
  X(ADC_ERR_CHANNEL_DISABLED, 56, "Channel disabled")                                              \
  X(ADC_ERR_UNKNOWN, 57, "Unknown error")

enum class hf_adc_err_t : hf_u8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_ADC_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert hf_adc_err_t to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfAdcErrToString(hf_adc_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_adc_err_t::NAME:                                                                         \
    return DESC;
    HF_ADC_ERR_LIST(X)
#undef X
  default:
    return HfAdcErrToString(hf_adc_err_t::ADC_ERR_UNKNOWN);
  }
}

//==============================================================================
// PLATFORM-AGNOSTIC ADC DRIVER ENUMS AND TYPES
//==============================================================================

/**
 * @brief ADC operation statistics.
 */
struct hf_adc_statistics_t {
  hf_u32_t totalConversions;        ///< Total conversions performed
  hf_u32_t successfulConversions;   ///< Successful conversions
  hf_u32_t failedConversions;       ///< Failed conversions
  hf_u32_t averageConversionTimeUs; ///< Average conversion time (microseconds)
  hf_u32_t maxConversionTimeUs;     ///< Maximum conversion time
  hf_u32_t minConversionTimeUs;     ///< Minimum conversion time
  hf_u32_t calibrationCount;        ///< Number of calibrations performed
  hf_u32_t thresholdViolations;     ///< Threshold monitor violations
  hf_u32_t calibration_errors;      ///< Calibration errors

  hf_adc_statistics_t()
      : totalConversions(0), successfulConversions(0), failedConversions(0),
        averageConversionTimeUs(0), maxConversionTimeUs(0), minConversionTimeUs(UINT32_MAX),
        calibrationCount(0), thresholdViolations(0), calibration_errors(0) {}
};

/**
 * @brief ADC diagnostic information.
 */
struct hf_adc_diagnostics_t {
  bool adcHealthy;             ///< Overall ADC health status
  hf_adc_err_t lastErrorCode;  ///< Last error code
  hf_u32_t lastErrorTimestamp; ///< Last error timestamp
  hf_u32_t consecutiveErrors;  ///< Consecutive error count
  float temperatureC;          ///< ADC temperature (if available)
  float referenceVoltage;      ///< Reference voltage
  bool calibrationValid;       ///< Calibration validity
  hf_u32_t enabled_channels;   ///< Bit mask of enabled channels
  bool initialization_state;   ///< Initialization state

  hf_adc_diagnostics_t()
      : adcHealthy(true), lastErrorCode(hf_adc_err_t::ADC_SUCCESS), lastErrorTimestamp(0),
        consecutiveErrors(0), temperatureC(25.0f), referenceVoltage(3.3f), calibrationValid(false),
        enabled_channels(0), initialization_state(false) {}
};

//==============================================================================

//=======================================//
//  BASE ADC CLASS
//=======================================//

/**
 * @class BaseAdc
 * @brief Base class for ADCs.
 * @details This class provides a common interface for all ADC implementations
 *          in the HardFOC system. It supports lazy initialization, robust error
 *          handling, and consistent API across different ADC hardware.
 */
class BaseAdc {
public:
  //==============================================//
  // CONSTRUCTION
  //==============================================//

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

  //==============================================//
  // LAZY-INITIALIZATION
  //==============================================//

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
   * @brief Ensures that the ADC is initialized (lazy initialization).
   * @return true if the ADC is initialized, false otherwise.
   */
  bool EnsureDeinitialized() noexcept {
    if (initialized_) {
      initialized_ = !Deinitialize();
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

  //==============================================//
  // PURE VIRTUAL FUNCTIONS [MUST BE OVERRIDDEN]  //
  //==============================================//

  /**
   * @brief Initializes the ADC peripheral (must be implemented by derived classes).
   * @return True if the initialization is successful, false otherwise.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitializes the ADC peripheral (must be implemented by derived classes)..
   * @return True if deinitialization is successful, false otherwise.
   */
  virtual bool Deinitialize() noexcept = 0;

  /**
   * @brief Get the maximum number of channels supported by this ADC.
   * @return Maximum channel count
   */
  [[nodiscard]] virtual hf_u8_t GetMaxChannels() const noexcept = 0;

  /**
   * @brief Check if a specific channel is available.
   * @param channel_id Channel ID to check
   * @return true if channel is available, false otherwise
   */
  [[nodiscard]] virtual bool IsChannelAvailable(hf_channel_id_t channel_id) const noexcept = 0;

  /**
   * @brief Read channel voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return hf_adc_err_t error code
   */
  virtual hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, float& channel_reading_v,
                                    hf_u8_t numOfSamplesToAvg = 1,
                                    hf_time_t timeBetweenSamples = 0) noexcept = 0;

  /**
   * @brief Read channel count (raw ADC value).
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return hf_adc_err_t error code
   */
  virtual hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                                        hf_u8_t numOfSamplesToAvg = 1,
                                        hf_time_t timeBetweenSamples = 0) noexcept = 0;

  /**
   * @brief Read both channel count and voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return hf_adc_err_t error code
   */
  virtual hf_adc_err_t ReadChannel(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                                   float& channel_reading_v, hf_u8_t numOfSamplesToAvg = 1,
                                   hf_time_t timeBetweenSamples = 0) noexcept = 0;

  //==============================================//
  // (OPTIONAL IMPLEMENTATIONS)                   //
  //==============================================//

  /**
   * @brief Read multiple channels simultaneously
   * @param channel_ids Array of channel IDs
   * @param num_channels Number of channels
   * @param readings Array to store raw readings
   * @param voltages Array to store voltage readings
   * @return hf_adc_err_t error code
   * @note Default implementation reads channels sequentially
   */
  virtual hf_adc_err_t ReadMultipleChannels(const hf_channel_id_t* channel_ids,
                                            hf_u8_t num_channels, hf_u32_t* readings,
                                            float* voltages) noexcept {
    if (!channel_ids || !readings || !voltages) {
      return hf_adc_err_t::ADC_ERR_NULL_POINTER;
    }

    for (hf_u8_t i = 0; i < num_channels; ++i) {
      hf_adc_err_t err = ReadChannel(channel_ids[i], readings[i], voltages[i]);
      if (err != hf_adc_err_t::ADC_SUCCESS) {
        return err;
      }
    }
    return hf_adc_err_t::ADC_SUCCESS;
  }

  //==============================================//
  //==============================================//
  /**
   * @brief Reset ADC operation statistics.
   * @return hf_adc_err_t::ADC_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics reset
   */
  virtual hf_adc_err_t ResetStatistics() noexcept {
    statistics_ = hf_adc_statistics_t{}; // Reset statistics to default values
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Reset ADC diagnostic information.
   * @return hf_adc_err_t::ADC_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics reset
   */
  virtual hf_adc_err_t ResetDiagnostics() noexcept {
    diagnostics_ = hf_adc_diagnostics_t{}; // Reset diagnostics to default values
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get ADC operation statistics.
   * @param statistics Reference to statistics structure to fill
   * @return hf_adc_err_t::ADC_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific statistics
   */
  virtual hf_adc_err_t GetStatistics(hf_adc_statistics_t& statistics) noexcept {
    statistics = statistics_; // Return empty statistics by default
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Get ADC diagnostic information.
   * @param diagnostics Reference to diagnostics structure to fill
   * @return hf_adc_err_t::ADC_SUCCESS if successful, error code otherwise
   * @note Override this method to provide platform-specific diagnostics
   */
  virtual hf_adc_err_t GetDiagnostics(hf_adc_diagnostics_t& diagnostics) noexcept {
    diagnostics = diagnostics_; // Return empty diagnostics by default
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
  }

protected:
  /**
   * @brief Protected default constructor
   */
  BaseAdc() noexcept : initialized_(false), statistics_{}, diagnostics_{} {}

  //==============================================//
  // VARIABLES                                    //
  //==============================================//

  bool initialized_;                 ///< Initialization status
  hf_adc_statistics_t statistics_;   ///< ADC operation statistics
  hf_adc_diagnostics_t diagnostics_; ///< ADC diagnostic information

private:
  //==============================================//
  //==============================================//
};
