/**
 * @file EspTemperature.h
 * @brief ESP32-C6 internal temperature sensor implementation for the HardFOC system.
 *
 * This file contains the declaration of the EspTemperature class that extends the
 * BaseTemperature abstract class to provide comprehensive ESP32-C6 temperature sensor
 * functionality using the ESP-IDF temperature sensor driver.
 *
 * Key features implemented:
 * - ESP32-C6 internal temperature sensor support using ESP-IDF v5.x APIs
 * - Multiple measurement ranges with different accuracy levels
 * - Hardware calibration and offset compensation
 * - Threshold monitoring with interrupt callbacks
 * - Continuous monitoring with timer-based sampling
 * - Thread-safe operations with mutex protection
 * - Power management (sleep/wake modes)
 * - Comprehensive error handling and diagnostics
 * - Self-test and health monitoring capabilities
 * - Operation statistics tracking
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note ESP32-C6 specific implementation using ESP-IDF v5.x
 * @note Each EspTemperature instance represents the internal chip temperature sensor
 * @note Thread-safe design suitable for multi-threaded applications
 */

#pragma once

#include "base/BaseTemperature.h"

// #ifdef HF_MCU_FAMILY_ESP32

// ESP-IDF includes for temperature sensor functionality
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/temperature_sensor.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include <atomic>
#include <functional>
#include <mutex>

//--------------------------------------
//  ESP32-C6 Temperature Constants
//--------------------------------------

/**
 * @brief ESP32-C6 Temperature sensor measurement ranges
 * @details Each range has different accuracy characteristics optimized for specific use cases.
 *          Lower ranges generally provide better accuracy.
 */
typedef enum {
  ESP_TEMP_RANGE_NEG10_80 = 0, ///< -10°C to 80°C, ±1°C accuracy (recommended for most applications)
  ESP_TEMP_RANGE_20_100 = 1,   ///< 20°C to 100°C, ±2°C accuracy (high temperature applications)
  ESP_TEMP_RANGE_NEG30_50 = 2, ///< -30°C to 50°C, ±2°C accuracy (low temperature applications)
  ESP_TEMP_RANGE_50_125 = 3,   ///< 50°C to 125°C, ±3°C accuracy (extreme high temperature)
  ESP_TEMP_RANGE_NEG40_20 = 4, ///< -40°C to 20°C, ±3°C accuracy (extreme low temperature)
  ESP_TEMP_RANGE_COUNT         ///< Number of available ranges
} esp_temp_range_t;

/**
 * @brief ESP32-C6 Temperature sensor default values
 */
#define ESP_TEMP_DEFAULT_RESOLUTION_CELSIUS 0.25f ///< Default resolution (0.25°C)
#define ESP_TEMP_DEFAULT_RESPONSE_TIME_MS 50      ///< Typical response time (50ms)
#define ESP_TEMP_DEFAULT_SAMPLE_RATE_HZ 10        ///< Default sample rate for continuous monitoring
#define ESP_TEMP_MAX_SAMPLE_RATE_HZ 1000          ///< Maximum sample rate
#define ESP_TEMP_MIN_SAMPLE_RATE_HZ 1             ///< Minimum sample rate
#define ESP_TEMP_DEFAULT_TIMEOUT_MS 1000          ///< Default operation timeout

/**
 * @brief ESP32-C6 Temperature sensor range limits
 */
#define ESP_TEMP_ABSOLUTE_MIN_CELSIUS -40.0f    ///< Absolute minimum temperature
#define ESP_TEMP_ABSOLUTE_MAX_CELSIUS 125.0f    ///< Absolute maximum temperature
#define ESP_TEMP_RECOMMENDED_MIN_CELSIUS -10.0f ///< Recommended minimum for best accuracy
#define ESP_TEMP_RECOMMENDED_MAX_CELSIUS 80.0f  ///< Recommended maximum for best accuracy

//--------------------------------------
//  ESP32-C6 Temperature Configuration
//--------------------------------------

/**
 * @brief ESP32-C6 specific temperature sensor state
 */
typedef struct {
  temperature_sensor_handle_t handle;  ///< ESP-IDF temperature sensor handle
  esp_temp_range_t current_range;      ///< Current measurement range
  float calibration_offset;            ///< Calibration offset in Celsius
  bool threshold_monitoring_enabled;   ///< Threshold monitoring status
  bool continuous_monitoring_active;   ///< Continuous monitoring status
  esp_timer_handle_t monitoring_timer; ///< Timer handle for continuous monitoring
  hf_u32_t sample_rate_hz;             ///< Current sample rate
  hf_u64_t last_reading_timestamp_us;  ///< Last reading timestamp
  float last_temperature_celsius;      ///< Last temperature reading
  bool allow_power_down;               ///< Allow power down during light sleep
} esp_temp_state_t;

/**
 * @brief ESP32-C6 specific temperature sensor configuration
 */
typedef struct {
  esp_temp_range_t range;            ///< Measurement range
  float calibration_offset;          ///< Initial calibration offset
  bool enable_threshold_monitoring;  ///< Enable threshold monitoring
  float high_threshold_celsius;      ///< High temperature threshold
  float low_threshold_celsius;       ///< Low temperature threshold
  bool enable_continuous_monitoring; ///< Enable continuous monitoring
  hf_u32_t sample_rate_hz;           ///< Sample rate for continuous monitoring
  bool allow_power_down;             ///< Allow sensor power down in light sleep
  hf_u32_t clk_src;                  ///< Clock source (usually default)
} esp_temp_config_t;

//--------------------------------------
//  ESP32-C6 Temperature Range Information
//--------------------------------------

/**
 * @brief Temperature range information structure
 */
typedef struct {
  esp_temp_range_t range;  ///< Range identifier
  float min_celsius;       ///< Minimum temperature
  float max_celsius;       ///< Maximum temperature
  float accuracy_celsius;  ///< Typical accuracy
  const char* description; ///< Human-readable description
} esp_temp_range_info_t;

//--------------------------------------
//  Forward Declarations
//--------------------------------------

class EspTemperature;

//--------------------------------------
//  ESP32-C6 Temperature Callback Types
//--------------------------------------

/**
 * @brief ESP32-C6 threshold callback function type
 */
using esp_temp_threshold_callback_t =
    std::function<void(EspTemperature* sensor, float temperature, bool is_high_threshold)>;

/**
 * @brief ESP32-C6 continuous monitoring callback function type
 */
using esp_temp_monitoring_callback_t =
    std::function<void(EspTemperature* sensor, float temperature, hf_u64_t timestamp_us)>;

//--------------------------------------
//  EspTemperature Class Declaration
//--------------------------------------

/**
 * @class EspTemperature
 * @brief ESP32-C6 internal temperature sensor implementation
 *
 * This class provides a complete implementation of the BaseTemperature interface
 * specifically for the ESP32-C6 internal temperature sensor. It leverages the
 * ESP-IDF temperature sensor driver to provide accurate temperature measurements
 * with advanced features like threshold monitoring and continuous sampling.
 *
 * Key features:
 * - Multiple measurement ranges with different accuracy levels
 * - Hardware-based threshold monitoring with interrupts
 * - Continuous temperature monitoring using ESP32 timers
 * - Thread-safe operations with mutex protection
 * - Power management support for low-power applications
 * - Comprehensive error handling and diagnostics
 * - Self-test and health monitoring capabilities
 * - Operation statistics and performance tracking
 *
 * @note This implementation is thread-safe and can be used in multi-threaded applications
 * @note The sensor provides approximately 0.25°C resolution with response time around 50ms
 * @note Different measurement ranges offer different accuracy levels (±1°C to ±3°C)
 */
class EspTemperature : public BaseTemperature {
public:
  //==============================================================//
  // CONSTRUCTORS AND DESTRUCTOR
  //==============================================================//

  /**
   * @brief Default constructor
   */
  EspTemperature() noexcept;

  /**
   * @brief Constructor with ESP32-specific configuration
   * @param esp_config ESP32-specific configuration
   */
  explicit EspTemperature(const esp_temp_config_t& esp_config) noexcept;

  /**
   * @brief Copy constructor is deleted to avoid copying instances.
   */
  EspTemperature(const EspTemperature&) = delete;

  /**
   * @brief Assignment operator is deleted to avoid copying instances.
   */
  EspTemperature& operator=(const EspTemperature&) = delete;

  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~EspTemperature() noexcept;

protected:
  //==============================================================//
  // [PROTECTED] PURE VIRTUAL IMPLEMENTATIONS - PLATFORM SPECIFIC
  //==============================================================//

  /**
   * @brief ESP32-specific implementation for initialization
   * @return true if initialization successful, false otherwise
   */
  bool Initialize() noexcept override;

  /**
   * @brief ESP32-specific implementation for deinitialization
   * @return true if deinitialization successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief ESP32-specific implementation for reading temperature in Celsius
   * @param temperature_celsius Pointer to store temperature value
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept override;

public:
  //==============================================================//
  // INFORMATION INTERFACE (MANDATORY OVERRIDES)
  //==============================================================//

  /**
   * @brief Get ESP32-C6 temperature sensor information
   * @param info Pointer to store sensor information
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept override;

  /**
   * @brief Get ESP32-C6 temperature sensor capabilities
   * @return Capabilities flags (hf_temp_capabilities_t)
   */
  [[nodiscard]] hf_u32_t GetCapabilities() const noexcept override;

  //==============================================================//
  // ADVANCED FEATURES (SUPPORTED BY ESP32-C6)
  //==============================================================//

  /**
   * @brief Set temperature measurement range (ESP32-C6 supported)
   * @param min_celsius Minimum temperature in Celsius
   * @param max_celsius Maximum temperature in Celsius
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetRange(float min_celsius, float max_celsius) noexcept override;

  /**
   * @brief Get temperature measurement range (ESP32-C6 supported)
   * @param min_celsius Pointer to store minimum temperature
   * @param max_celsius Pointer to store maximum temperature
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetRange(float* min_celsius, float* max_celsius) const noexcept override;

  /**
   * @brief Get measurement resolution (ESP32-C6 supported)
   * @param resolution_celsius Pointer to store resolution
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetResolution(float* resolution_celsius) const noexcept override;

  /**
   * @brief Set temperature thresholds (ESP32-C6 supported)
   * @param low_threshold_celsius Low temperature threshold
   * @param high_threshold_celsius High temperature threshold
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetThresholds(float low_threshold_celsius,
                              float high_threshold_celsius) noexcept override;

  /**
   * @brief Get temperature thresholds (ESP32-C6 supported)
   * @param low_threshold_celsius Pointer to store low threshold
   * @param high_threshold_celsius Pointer to store high threshold
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetThresholds(float* low_threshold_celsius,
                              float* high_threshold_celsius) const noexcept override;

  /**
   * @brief Enable threshold monitoring (ESP32-C6 supported)
   * @param callback Callback function for threshold events
   * @param user_data User data to pass to callback
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t EnableThresholdMonitoring(hf_temp_threshold_callback_t callback,
                                          void* user_data) noexcept override;

  /**
   * @brief Disable threshold monitoring (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t DisableThresholdMonitoring() noexcept override;

  /**
   * @brief Start continuous temperature monitoring (ESP32-C6 supported)
   * @param sample_rate_hz Sampling rate in Hz
   * @param callback Callback function for each reading
   * @param user_data User data to pass to callback
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t StartContinuousMonitoring(hf_u32_t sample_rate_hz,
                                          hf_temp_reading_callback_t callback,
                                          void* user_data) noexcept override;

  /**
   * @brief Stop continuous temperature monitoring (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t StopContinuousMonitoring() noexcept override;

  /**
   * @brief Check if continuous monitoring is active (ESP32-C6 supported)
   * @return true if monitoring is active, false otherwise
   */
  [[nodiscard]] bool IsMonitoringActive() const noexcept override;

  /**
   * @brief Set calibration offset (ESP32-C6 supported)
   * @param offset_celsius Calibration offset in Celsius
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetCalibrationOffset(float offset_celsius) noexcept override;

  /**
   * @brief Get calibration offset (ESP32-C6 supported)
   * @param offset_celsius Pointer to store calibration offset
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetCalibrationOffset(float* offset_celsius) const noexcept override;

  /**
   * @brief Reset calibration to default (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ResetCalibration() noexcept override;

  /**
   * @brief Enter low power mode (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t EnterSleepMode() noexcept override;

  /**
   * @brief Exit low power mode (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ExitSleepMode() noexcept override;

  /**
   * @brief Check if sensor is in sleep mode (ESP32-C6 supported)
   * @return true if in sleep mode, false otherwise
   */
  [[nodiscard]] bool IsSleeping() const noexcept override;

  /**
   * @brief Perform sensor self-test (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SelfTest() noexcept override;

  /**
   * @brief Check sensor health status (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS if healthy)
   */
  hf_temp_err_t CheckHealth() noexcept override;

  /**
   * @brief Get operation statistics (ESP32-C6 supported)
   * @param statistics Reference to statistics structure to fill
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetStatistics(hf_temp_statistics_t& statistics) noexcept override;

  /**
   * @brief Get diagnostic information (ESP32-C6 supported)
   * @param diagnostics Reference to diagnostics structure to fill
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetDiagnostics(hf_temp_diagnostics_t& diagnostics) noexcept override;

  /**
   * @brief Reset operation statistics (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ResetStatistics() noexcept override;

  /**
   * @brief Reset diagnostic information (ESP32-C6 supported)
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ResetDiagnostics() noexcept override;

  //==============================================================//
  // ESP32-C6 SPECIFIC METHODS
  //==============================================================//

  /**
   * @brief Initialize with ESP32-specific configuration
   * @param esp_config ESP32-specific configuration structure
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t InitializeEsp32(const esp_temp_config_t& esp_config) noexcept;

  /**
   * @brief Set measurement range using ESP32 enum
   * @param range ESP32-specific range identifier
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetMeasurementRange(esp_temp_range_t range) noexcept;

  /**
   * @brief Get current measurement range
   * @return Current ESP32 range identifier
   */
  [[nodiscard]] esp_temp_range_t GetMeasurementRange() const noexcept;

  /**
   * @brief Get range information for a specific range
   * @param range Range to query
   * @param min_celsius Pointer to store minimum temperature
   * @param max_celsius Pointer to store maximum temperature
   * @param accuracy_celsius Pointer to store accuracy
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t GetRangeInfo(esp_temp_range_t range, float* min_celsius, float* max_celsius,
                             float* accuracy_celsius) const noexcept;

  /**
   * @brief Find optimal range for given temperature requirements
   * @param min_celsius Minimum required temperature
   * @param max_celsius Maximum required temperature
   * @return Optimal range identifier
   */
  [[nodiscard]] esp_temp_range_t FindOptimalRange(float min_celsius,
                                                  float max_celsius) const noexcept;

  /**
   * @brief Read raw temperature value (before calibration)
   * @param raw_value Pointer to store raw temperature
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ReadRawTemperature(float* raw_value) noexcept;

  /**
   * @brief Get ESP-IDF temperature sensor handle
   * @return ESP-IDF handle (nullptr if not initialized)
   */
  [[nodiscard]] temperature_sensor_handle_t GetEspHandle() const noexcept;

  /**
   * @brief Set ESP32-specific threshold callback
   * @param callback ESP32-specific callback function
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetEspThresholdCallback(esp_temp_threshold_callback_t callback) noexcept;

  /**
   * @brief Set ESP32-specific monitoring callback
   * @param callback ESP32-specific callback function
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetEspMonitoringCallback(esp_temp_monitoring_callback_t callback) noexcept;

private:
  //==============================================================//
  // [PRIVATE] MEMBER VARIABLES
  //==============================================================//

  mutable std::mutex mutex_;          ///< Thread safety mutex
  esp_temp_state_t esp_state_;        ///< ESP32-specific state
  esp_temp_config_t esp_config_;      ///< ESP32-specific configuration
  hf_temp_config_t base_config_;      ///< Base class configuration
  hf_temp_statistics_t statistics_;   ///< Operation statistics
  hf_temp_diagnostics_t diagnostics_; ///< Diagnostic information
  hf_temp_err_t last_error_;          ///< Last error code

  // Callback storage
  hf_temp_threshold_callback_t threshold_callback_;        ///< Base threshold callback
  hf_temp_reading_callback_t monitoring_callback_;         ///< Base monitoring callback
  esp_temp_threshold_callback_t esp_threshold_callback_;   ///< ESP32-specific threshold callback
  esp_temp_monitoring_callback_t esp_monitoring_callback_; ///< ESP32-specific monitoring callback
  void* threshold_user_data_;                              ///< User data for threshold callback
  void* monitoring_user_data_;                             ///< User data for monitoring callback

  //==============================================================//
  // [PRIVATE] HELPER METHODS
  //==============================================================//

  /**
   * @brief Convert ESP-IDF error to HardFOC temperature error
   * @param esp_err ESP-IDF error code
   * @return HardFOC temperature error code
   */
  hf_temp_err_t ConvertEspError(esp_err_t esp_err) const noexcept;

  /**
   * @brief Configure ESP temperature sensor with current settings
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t ConfigureEspSensor() noexcept;

  /**
   * @brief Setup measurement range configuration
   * @param range Range to configure
   * @return Error code (TEMP_SUCCESS on success)
   */
  hf_temp_err_t SetupRange(esp_temp_range_t range) noexcept;

  /**
   * @brief Get range configuration values
   * @param range Range to query
   * @param min_celsius Pointer to store minimum temperature
   * @param max_celsius Pointer to store maximum temperature
   * @param accuracy_celsius Pointer to store accuracy
   */
  void GetRangeConfig(esp_temp_range_t range, float* min_celsius, float* max_celsius,
                      float* accuracy_celsius) const noexcept;

  /**
   * @brief Set last error code and update diagnostics
   * @param error Error code to set
   */
  void SetLastError(hf_temp_err_t error) noexcept;

  /**
   * @brief Update operation statistics
   * @param operation_successful Whether the operation was successful
   * @param operation_time_us Operation time in microseconds
   */
  void UpdateStatistics(bool operation_successful, hf_u32_t operation_time_us) noexcept;

  /**
   * @brief Update diagnostic information
   * @param error Error code from operation
   */
  void UpdateDiagnostics(hf_temp_err_t error) noexcept;

  /**
   * @brief Timer callback for continuous monitoring
   * @param arg Pointer to EspTemperature instance
   */
  static void MonitoringTimerCallback(void* arg) noexcept;

  /**
   * @brief Check thresholds and trigger callbacks if needed
   * @param temperature Current temperature reading
   */
  void CheckThresholds(float temperature) noexcept;

  /**
   * @brief Validate base configuration
   * @param config Configuration to validate
   * @return Error code (TEMP_SUCCESS if valid)
   */
  hf_temp_err_t ValidateConfig(const hf_temp_config_t* config) const noexcept;

  /**
   * @brief Validate ESP32-specific configuration
   * @param esp_config ESP32 configuration to validate
   * @return Error code (TEMP_SUCCESS if valid)
   */
  hf_temp_err_t ValidateEspConfig(const esp_temp_config_t* esp_config) const noexcept;

  /**
   * @brief Get current timestamp in microseconds
   * @return Current timestamp
   */
  static hf_u64_t GetCurrentTimeUs() noexcept;

  // Static constants
  static const char* TAG;                          ///< ESP-IDF logging tag
  static const esp_temp_range_info_t RANGE_INFO[]; ///< Range information table
};

//--------------------------------------
//  Default ESP32 Configuration
//--------------------------------------

/**
 * @brief Default ESP32-C6 temperature sensor configuration
 */
#define ESP_TEMP_CONFIG_DEFAULT()                     \
  {.range = ESP_TEMP_RANGE_NEG10_80,                  \
   .calibration_offset = 0.0f,                        \
   .enable_threshold_monitoring = false,              \
   .high_threshold_celsius = 80.0f,                   \
   .low_threshold_celsius = -10.0f,                   \
   .enable_continuous_monitoring = false,             \
   .sample_rate_hz = ESP_TEMP_DEFAULT_SAMPLE_RATE_HZ, \
   .allow_power_down = true,                          \
   .clk_src = 0}

// #endif // HF_MCU_FAMILY_ESP32
