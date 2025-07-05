/**
 * @file BaseAdc.h
 * @brief Abstract base class for ADC implementations in the HardFOC system.
 *
 * This file contains the declaration of the BaseAdc abstract class, which provides
 * a common interface and comprehensive features for all ADC implementations.
 * The class supports multi-channel conversions, calibration management, continuous
 * sampling modes, threshold monitoring, and advanced ESP32C6-specific features.
 * ADC derived classes employ lazy initialization - they are initialized the first
 * time a channel operation is performed.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note These functions are not thread or interrupt-safe and should be called
 *       with appropriate synchronization guards if used within an ISR or shared tasks.
 * @note This is a header-only abstract base class following the same pattern as BaseCan.
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
  /* Extended ADC errors (for ESP32 compatibility) */                                             \
  X(ADC_ERR_INITIALIZATION_FAILED, 43, "Initialization failed")                                    \
  X(ADC_ERR_INVALID_PARAM, 44, "Invalid parameter")                                                \
  X(ADC_ERR_TIMEOUT, 45, "Operation timeout")                                                      \
  X(ADC_ERR_NOT_SUPPORTED, 46, "Not supported")                                                    \
                                                                                                   \
  /* Additional missing error codes */                                                             \
  X(ADC_ERR_INVALID_STATE, 47, "Invalid state")                                                    \
  X(ADC_ERR_DRIVER_ERROR, 48, "Driver error")                                                      \
  X(ADC_ERR_DMA_ERROR, 49, "DMA error")                                                            \
  X(ADC_ERR_FILTER_ERROR, 50, "Filter configuration error")

enum class HfAdcErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_ADC_ERR_LIST(X)
#undef X
};

/**
 * @brief Convert HfAdcErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfAdcErrToString(HfAdcErr err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case HfAdcErr::NAME:                                                                             \
    return DESC;
    HF_ADC_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

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
  /**
   * @brief Constructor
   */
  BaseAdc() noexcept : initialized_(false) {}

  /**
   * @brief Virtual destructor
   */
  virtual ~BaseAdc() noexcept = default;

  // Disable copy constructor and assignment operator for safety
  BaseAdc(const BaseAdc &) = delete;
  BaseAdc &operator=(const BaseAdc &) = delete;

  // Allow move operations
  BaseAdc(BaseAdc &&) noexcept = default;
  BaseAdc &operator=(BaseAdc &&) noexcept = default;

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
  // PURE VIRTUAL FUNCTIONS - MUST BE OVERRIDDEN  //
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
  [[nodiscard]] virtual uint8_t GetMaxChannels() const noexcept = 0;

  /**
   * @brief Check if a specific channel is available.
   * @param channel_id Channel ID to check
   * @return true if channel is available, false otherwise
   */
  [[nodiscard]] virtual bool IsChannelAvailable(HfChannelId channel_id) const noexcept = 0;

  /**
   * @brief Read channel voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  virtual HfAdcErr ReadChannelV(HfChannelId channel_id, float &channel_reading_v,
                                uint8_t numOfSamplesToAvg = 1,
                                HfTimeoutMs timeBetweenSamples = 0) noexcept = 0;

  /**
   * @brief Read channel count (raw ADC value).
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  virtual HfAdcErr ReadChannelCount(HfChannelId channel_id, uint32_t &channel_reading_count,
                                    uint8_t numOfSamplesToAvg = 1,
                                    HfTimeoutMs timeBetweenSamples = 0) noexcept = 0;

  /**
   * @brief Read both channel count and voltage.
   * @param channel_id Channel ID to read from
   * @param channel_reading_count Reference to store count reading
   * @param channel_reading_v Reference to store voltage reading
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in milliseconds (default 0)
   * @return HfAdcErr error code
   */
  virtual HfAdcErr ReadChannel(HfChannelId channel_id, uint32_t &channel_reading_count,
                               float &channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                               HfTimeoutMs timeBetweenSamples = 0) noexcept = 0;


  //==============================================//
  // ADVANCED FEATURES (OPTIONAL IMPLEMENTATIONS) //
  //==============================================//

  /**
   * @brief ADC callback for continuous/DMA operations
   * @param channel_id Channel that completed conversion
   * @param samples Pointer to sample data
   * @param num_samples Number of samples in buffer
   * @param user_data User-provided data
   */
  using AdcCallback = std::function<void(HfChannelId channel_id, const uint16_t *samples,
                                          size_t num_samples, void *user_data)>;
                                          
  /**
   * @brief Read multiple channels simultaneously
   * @param channel_ids Array of channel IDs
   * @param num_channels Number of channels
   * @param readings Array to store raw readings
   * @param voltages Array to store voltage readings
   * @return HfAdcErr error code
   * @note Default implementation reads channels sequentially
   */
  virtual HfAdcErr ReadMultipleChannels(const HfChannelId *channel_ids, uint8_t num_channels,
                                        uint32_t *readings, float *voltages) noexcept {
    if (!channel_ids || !readings || !voltages) {
      return HfAdcErr::ADC_ERR_NULL_POINTER;
    }

    for (uint8_t i = 0; i < num_channels; ++i) {
      HfAdcErr err = ReadChannel(channel_ids[i], readings[i], voltages[i]);
      if (err != HfAdcErr::ADC_SUCCESS) {
        return err;
      }
    }
    return HfAdcErr::ADC_SUCCESS;
  }

  /**
   * @brief Start continuous/DMA sampling with callback
   * @param channel_id Channel to sample
   * @param callback Callback for sample data
   * @param user_data User data for callback
   * @return HfAdcErr error code
   * @note Default implementation returns unsupported operation
   */
  virtual HfAdcErr StartContinuousSampling(HfChannelId channel_id, AdcCallback callback,
                                           void *user_data = nullptr) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Stop continuous/DMA sampling
   * @param channel_id Channel to stop
   * @return HfAdcErr error code
   * @note Default implementation returns unsupported operation
   */
  virtual HfAdcErr StopContinuousSampling(HfChannelId channel_id) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  //==============================================//
  // CALIBRATION SUPPORT                          //
  //==============================================//

  /**
   * @brief Calibration progress callback for long operations
   * @param channel_id Channel being calibrated
   * @param progress_percent Progress percentage (0-100)
   * @param current_step Description of current calibration step
   * @param user_data User-provided data
   */
  using CalibrationProgressCallback = std::function<void(
      HfChannelId channel_id, float progress_percent, const char *current_step, void *user_data)>;

  /**
   * @brief Perform ADC calibration for a specific channel
   * @param channel_id Channel to calibrate
   * @param config Calibration configuration
   * @param progress_callback Optional progress callback for long operations
   * @param user_data User data for progress callback
   * @return HfAdcErr error code
   * @note Default implementation returns unsupported operation
   */
  virtual HfAdcErr CalibrateChannel(HfChannelId channel_id,
                                    CalibrationProgressCallback progress_callback = nullptr,
                                    void *user_data = nullptr) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Perform automatic calibration using known reference voltages
   * @param channel_id Channel to calibrate
   * @param reference_voltages Array of known reference voltages
   * @param num_references Number of reference voltages
   * @param calibration_type Type of calibration to perform
   * @return HfAdcErr error code
   * @note Implementation should prompt user to apply each reference voltage
   */
  virtual HfAdcErr AutoCalibrate(HfChannelId channel_id,
                                  const float *reference_voltages,
                                  uint8_t num_references) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Save calibration data to non-volatile storage
   * @param channel_id Channel calibration to save
   * @return HfAdcErr error code
   */
  virtual HfAdcErr SaveCalibration(HfChannelId channel_id) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Load calibration data from non-volatile storage
   * @param channel_id Channel calibration to load
   * @return HfAdcErr error code
   */
  virtual HfAdcErr LoadCalibration(HfChannelId channel_id) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Clear/reset calibration for a channel
   * @param channel_id Channel to reset
   * @return HfAdcErr error code
   */
  virtual HfAdcErr ClearCalibration(HfChannelId channel_id) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Verify calibration accuracy using known reference
   * @param channel_id Channel to verify
   * @param reference_voltage Known reference voltage to test
   * @param measured_voltage Output: voltage measured by ADC
   * @param error_percent Output: percentage error from reference
   * @return HfAdcErr error code
   */
  virtual HfAdcErr VerifyCalibration(HfChannelId channel_id, float reference_voltage,
                                     float &measured_voltage, float &error_percent) noexcept {
    return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
  }

  /**
   * @brief Apply temperature compensation to a reading
   * @param raw_reading Raw ADC reading
   * @param current_temp_c Current temperature in Celsius
   * @param calibration_temp_c Temperature when calibration was performed
   * @param temp_coefficient Temperature coefficient (ppm/°C)
   * @return Temperature-compensated reading
   */
  static uint32_t ApplyTemperatureCompensation(uint32_t raw_reading, float current_temp_c,
                                               float calibration_temp_c,
                                               float temp_coefficient) noexcept {
    float temp_delta = current_temp_c - calibration_temp_c;
    float compensation_factor = 1.0f + (temp_coefficient * temp_delta / 1000000.0f);
    return static_cast<uint32_t>(raw_reading * compensation_factor);
  }

protected:
  /**
   * @brief Validate input parameters for read operations.
   * @param channel_id Channel ID to validate
   * @param numOfSamplesToAvg Number of samples to validate
   * @return HfAdcErr validation result
   */
  [[nodiscard]] HfAdcErr ValidateReadParameters(HfChannelId channel_id,
                                                uint8_t numOfSamplesToAvg) const noexcept {
    if (!initialized_) {
      return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
    }

    if (numOfSamplesToAvg == 0 || numOfSamplesToAvg > 255) {
      return HfAdcErr::ADC_ERR_INVALID_SAMPLE_COUNT;
    }

    if (channel_id >= GetMaxChannels()) {
      return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
    }

    if (!IsChannelAvailable(channel_id)) {
      return HfAdcErr::ADC_ERR_CHANNEL_NOT_FOUND;
    }

    return HfAdcErr::ADC_SUCCESS;
  }

private:
  bool initialized_; ///< Initialization status
};
