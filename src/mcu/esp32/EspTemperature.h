/**
 * @file EspTemperature.h
 * @brief ESP32-C6 temperature sensor implementation for the HardFOC system.
 *
 * This header file defines the ESP32-C6 specific implementation of the BaseTemperature
 * abstract class. It provides access to the internal chip temperature sensor using
 * the ESP-IDF temperature sensor driver.
 *
 * @author HardFOC Development Team  
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is specific to ESP32-C6 and requires ESP-IDF
 * @note The internal temperature sensor measures chip temperature, not ambient temperature
 * @note Requires esp_driver_tsens component in ESP-IDF
 */

#pragma once

#include "BaseTemperature.h"
#include <memory>
#include <mutex>

// ESP-IDF includes
extern "C" {
#include "driver/temperature_sensor.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_timer.h"
}

//--------------------------------------
//  ESP32 Temperature Specific Types
//--------------------------------------

/**
 * @brief ESP32-C6 temperature sensor predefined ranges with accuracy
 * @details Based on ESP32-C6 hardware specifications
 */
typedef enum {
    ESP_TEMP_RANGE_UNKNOWN = -1,      ///< Unknown or invalid range
    ESP_TEMP_RANGE_50_125,            ///< 50°C ~ 125°C, error < 3°C
    ESP_TEMP_RANGE_20_100,            ///< 20°C ~ 100°C, error < 2°C  
    ESP_TEMP_RANGE_NEG10_80,          ///< -10°C ~ 80°C, error < 1°C (default/best accuracy)
    ESP_TEMP_RANGE_NEG30_50,          ///< -30°C ~ 50°C, error < 2°C
    ESP_TEMP_RANGE_NEG40_20,          ///< -40°C ~ 20°C, error < 3°C
    ESP_TEMP_RANGE_MAX                ///< Maximum range marker
} EspTempRange_t;

/**
 * @brief ESP32-C6 temperature sensor configuration
 */
typedef struct {
    EspTempRange_t preferred_range;   ///< Preferred measurement range for optimal accuracy
    bool enable_power_management;     ///< Enable power down in light sleep mode
    temperature_sensor_clk_src_t clk_src; ///< Clock source for temperature sensor
    uint32_t timeout_ms;              ///< Operation timeout in milliseconds
    bool auto_range_selection;        ///< Automatically select best range based on config
} EspTempConfig_t;

/**
 * @brief ESP32-C6 temperature sensor internal state
 */
typedef struct {
    temperature_sensor_handle_t handle; ///< ESP-IDF temperature sensor handle
    EspTempRange_t current_range;      ///< Currently configured range
    float calibration_offset;          ///< Calibration offset in Celsius
    bool is_monitoring;                ///< Whether continuous monitoring is active
    esp_timer_handle_t monitoring_timer; ///< Timer for continuous monitoring
    HfTempReadingCallback_t reading_callback; ///< User callback for readings
    void* callback_user_data;          ///< User data for callback
    HfTempThresholdCallback_t threshold_callback; ///< Threshold callback
    void* threshold_user_data;         ///< User data for threshold callback
    float low_threshold;               ///< Low temperature threshold
    float high_threshold;              ///< High temperature threshold
    bool threshold_monitoring_enabled; ///< Whether threshold monitoring is enabled
    HfTempError_t last_error;          ///< Last error code
    HfTempErrorCallback_t error_callback; ///< Error callback function
    void* error_user_data;             ///< User data for error callback
} EspTempState_t;

//--------------------------------------
//  ESP32 Temperature Constants
//--------------------------------------

/**
 * @brief ESP32-C6 temperature sensor capabilities
 */
#define ESP_TEMP_CAPABILITIES (TEMP_CAP_THRESHOLD_MONITORING | \
                              TEMP_CAP_CONTINUOUS_READING | \
                              TEMP_CAP_POWER_MANAGEMENT | \
                              TEMP_CAP_SELF_TEST | \
                              TEMP_CAP_HIGH_PRECISION | \
                              TEMP_CAP_FAST_RESPONSE)

/**
 * @brief ESP32-C6 temperature sensor specifications
 */
#define ESP_TEMP_MIN_CELSIUS        (-40.0f)    ///< Minimum measurable temperature
#define ESP_TEMP_MAX_CELSIUS        (125.0f)    ///< Maximum measurable temperature  
#define ESP_TEMP_RESOLUTION_CELSIUS (0.25f)     ///< Temperature resolution
#define ESP_TEMP_RESPONSE_TIME_MS   (50)        ///< Typical response time
#define ESP_TEMP_DEFAULT_TIMEOUT_MS (1000)      ///< Default operation timeout

/**
 * @brief Default ESP32 temperature configuration
 */
#define ESP_TEMP_CONFIG_DEFAULT() {              \
    .preferred_range = ESP_TEMP_RANGE_NEG10_80,  \
    .enable_power_management = false,            \
    .clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT, \
    .timeout_ms = ESP_TEMP_DEFAULT_TIMEOUT_MS,   \
    .auto_range_selection = true                 \
}

//--------------------------------------
//  EspTemperature Class Declaration
//--------------------------------------

namespace HardFOC {

/**
 * @class EspTemperature
 * @brief ESP32-C6 internal temperature sensor implementation
 * 
 * This class provides a concrete implementation of the BaseTemperature interface
 * specifically for the ESP32-C6 internal temperature sensor. It leverages the
 * ESP-IDF temperature sensor driver to provide accurate chip temperature readings.
 * 
 * Key features:
 * - Direct access to ESP32-C6 internal temperature sensor
 * - Multiple predefined measurement ranges with varying accuracy
 * - Threshold monitoring with interrupt support
 * - Continuous monitoring capabilities
 * - Power management support for low-power applications
 * - Automatic range selection for optimal accuracy
 * - Calibration support with offset adjustment
 * 
 * Hardware specifications:
 * - Temperature range: -40°C to +125°C
 * - Resolution: 0.25°C
 * - Accuracy: ±1°C to ±3°C (depending on range)
 * - Response time: ~50ms
 * 
 * @note This implementation is thread-safe
 * @note The sensor measures internal chip temperature, not ambient temperature
 * @note Requires ESP-IDF esp_driver_tsens component
 */
class EspTemperature : public BaseTemperature {
public:
    //--------------------------------------
    //  Constructor & Destructor
    //--------------------------------------
    
    /**
     * @brief Constructor
     */
    EspTemperature();
    
    /**
     * @brief Destructor
     */
    virtual ~EspTemperature();
    
    //--------------------------------------
    //  Core Temperature Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Initialize the ESP32-C6 temperature sensor
     * @param config Configuration structure
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t initialize(const HfTempConfig_t* config) override;
    
    /**
     * @brief Deinitialize the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t deinitialize() override;
    
    /**
     * @brief Enable the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t enable() override;
    
    /**
     * @brief Disable the temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t disable() override;
    
    /**
     * @brief Check if the sensor is initialized
     * @return true if initialized, false otherwise
     */
    bool is_initialized() const override;
    
    /**
     * @brief Check if the sensor is enabled
     * @return true if enabled, false otherwise
     */
    bool is_enabled() const override;
    
    /**
     * @brief Get current sensor state
     * @return Current sensor state
     */
    HfTempState_t get_state() const override;
    
    //--------------------------------------
    //  Temperature Reading Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Read temperature in Celsius (blocking)
     * @param temperature_celsius Pointer to store temperature value
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t read_celsius(float* temperature_celsius) override;
    
    /**
     * @brief Read temperature with full information (blocking)
     * @param reading Pointer to store complete reading information
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t read_temperature(HfTempReading_t* reading) override;
    
    /**
     * @brief Start asynchronous temperature reading
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t start_async_read() override;
    
    /**
     * @brief Check if asynchronous reading is complete
     * @return true if reading is complete, false otherwise
     */
    bool is_read_complete() const override;
    
    /**
     * @brief Get result of asynchronous reading
     * @param reading Pointer to store reading result
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_async_result(HfTempReading_t* reading) override;
    
    //--------------------------------------
    //  Configuration Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Set temperature measurement range
     * @param min_celsius Minimum temperature in Celsius
     * @param max_celsius Maximum temperature in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_range(float min_celsius, float max_celsius) override;
    
    /**
     * @brief Get temperature measurement range
     * @param min_celsius Pointer to store minimum temperature
     * @param max_celsius Pointer to store maximum temperature
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_range(float* min_celsius, float* max_celsius) const override;
    
    /**
     * @brief Set measurement resolution
     * @param resolution_celsius Resolution in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_resolution(float resolution_celsius) override;
    
    /**
     * @brief Get measurement resolution
     * @param resolution_celsius Pointer to store resolution
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_resolution(float* resolution_celsius) const override;
    
    //--------------------------------------
    //  Threshold Monitoring Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Set temperature thresholds
     * @param low_threshold_celsius Low temperature threshold
     * @param high_threshold_celsius High temperature threshold
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_thresholds(float low_threshold_celsius, float high_threshold_celsius) override;
    
    /**
     * @brief Get temperature thresholds
     * @param low_threshold_celsius Pointer to store low threshold
     * @param high_threshold_celsius Pointer to store high threshold
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_thresholds(float* low_threshold_celsius, float* high_threshold_celsius) const override;
    
    /**
     * @brief Enable threshold monitoring
     * @param callback Callback function for threshold events
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t enable_threshold_monitoring(HfTempThresholdCallback_t callback, void* user_data) override;
    
    /**
     * @brief Disable threshold monitoring
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t disable_threshold_monitoring() override;
    
    //--------------------------------------
    //  Continuous Monitoring Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Start continuous temperature monitoring
     * @param sample_rate_hz Sampling rate in Hz
     * @param callback Callback function for each reading
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t start_continuous_monitoring(uint32_t sample_rate_hz, 
                                             HfTempReadingCallback_t callback, 
                                             void* user_data) override;
    
    /**
     * @brief Stop continuous temperature monitoring
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t stop_continuous_monitoring() override;
    
    /**
     * @brief Check if continuous monitoring is active
     * @return true if monitoring is active, false otherwise
     */
    bool is_monitoring_active() const override;
    
    //--------------------------------------
    //  Calibration Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Perform sensor calibration
     * @param reference_temperature_celsius Known reference temperature
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t calibrate(float reference_temperature_celsius) override;
    
    /**
     * @brief Set calibration offset
     * @param offset_celsius Calibration offset in Celsius
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_calibration_offset(float offset_celsius) override;
    
    /**
     * @brief Get calibration offset
     * @param offset_celsius Pointer to store calibration offset
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_calibration_offset(float* offset_celsius) const override;
    
    /**
     * @brief Reset calibration to default
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t reset_calibration() override;
    
    //--------------------------------------
    //  Power Management Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Enter low power mode
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t enter_sleep_mode() override;
    
    /**
     * @brief Exit low power mode
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t exit_sleep_mode() override;
    
    /**
     * @brief Check if sensor is in sleep mode
     * @return true if in sleep mode, false otherwise
     */
    bool is_sleeping() const override;
    
    //--------------------------------------
    //  Information Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Get sensor information
     * @param info Pointer to store sensor information
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_sensor_info(HfTempSensorInfo_t* info) const override;
    
    /**
     * @brief Get sensor capabilities
     * @return Capabilities flags (HfTempCapabilities_t)
     */
    uint32_t get_capabilities() const override;
    
    //--------------------------------------
    //  Error Handling Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Set error callback function
     * @param callback Error callback function
     * @param user_data User data to pass to callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_error_callback(HfTempErrorCallback_t callback, void* user_data) override;
    
    /**
     * @brief Clear error callback
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t clear_error_callback() override;
    
    /**
     * @brief Get last error
     * @return Last error code
     */
    HfTempError_t get_last_error() const override;
    
    /**
     * @brief Clear last error
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t clear_last_error() override;
    
    //--------------------------------------
    //  Self-Test Interface Implementation
    //--------------------------------------
    
    /**
     * @brief Perform sensor self-test
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t self_test() override;
    
    /**
     * @brief Check sensor health status
     * @return Error code (TEMP_SUCCESS if healthy)
     */
    HfTempError_t check_health() override;
    
    //--------------------------------------
    //  ESP32-Specific Methods
    //--------------------------------------
    
    /**
     * @brief Initialize with ESP32-specific configuration
     * @param esp_config ESP32-specific configuration
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t initialize_esp32(const EspTempConfig_t* esp_config);
    
    /**
     * @brief Set specific measurement range for optimal accuracy
     * @param range Predefined range selection
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t set_measurement_range(EspTempRange_t range);
    
    /**
     * @brief Get current measurement range
     * @return Current measurement range
     */
    EspTempRange_t get_measurement_range() const;
    
    /**
     * @brief Get range information
     * @param range Range to query
     * @param min_celsius Pointer to store minimum temperature
     * @param max_celsius Pointer to store maximum temperature  
     * @param accuracy_celsius Pointer to store accuracy
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t get_range_info(EspTempRange_t range, 
                                float* min_celsius, 
                                float* max_celsius, 
                                float* accuracy_celsius) const;
    
    /**
     * @brief Find optimal range for given temperature range
     * @param min_celsius Minimum required temperature
     * @param max_celsius Maximum required temperature
     * @return Optimal range selection
     */
    EspTempRange_t find_optimal_range(float min_celsius, float max_celsius) const;
    
    /**
     * @brief Read raw temperature sensor value
     * @param raw_value Pointer to store raw value
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t read_raw(float* raw_value);
    
    /**
     * @brief Get ESP-IDF handle (for advanced users)
     * @return ESP-IDF temperature sensor handle
     */
    temperature_sensor_handle_t get_esp_handle() const;

private:
    //--------------------------------------
    //  Private Members
    //--------------------------------------
    
    mutable std::mutex mutex_;            ///< Mutex for thread safety
    HfTempState_t current_state_;         ///< Current sensor state
    EspTempState_t esp_state_;            ///< ESP32-specific state
    EspTempConfig_t esp_config_;          ///< ESP32-specific configuration
    HfTempConfig_t base_config_;          ///< Base configuration
    bool initialized_;                    ///< Initialization flag
    bool enabled_;                        ///< Enable flag
    
    //--------------------------------------
    //  Private Methods
    //--------------------------------------
    
    /**
     * @brief Convert ESP-IDF error to HardFOC error
     * @param esp_err ESP-IDF error code
     * @return HardFOC error code
     */
    HfTempError_t convert_esp_error(esp_err_t esp_err) const;
    
    /**
     * @brief Configure ESP-IDF temperature sensor
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t configure_esp_sensor();
    
    /**
     * @brief Setup temperature range based on configuration
     * @param range Desired range
     * @return Error code (TEMP_SUCCESS on success)
     */
    HfTempError_t setup_range(EspTempRange_t range);
    
    /**
     * @brief Get range configuration
     * @param range Range to get configuration for
     * @param min_celsius Output minimum temperature
     * @param max_celsius Output maximum temperature
     * @param accuracy_celsius Output accuracy
     */
    void get_range_config(EspTempRange_t range, 
                         float* min_celsius, 
                         float* max_celsius, 
                         float* accuracy_celsius) const;
    
    /**
     * @brief Set last error and call error callback if set
     * @param error Error code to set
     */
    void set_last_error(HfTempError_t error);
    
    /**
     * @brief Timer callback for continuous monitoring
     * @param arg Timer argument (pointer to EspTemperature instance)
     */
    static void monitoring_timer_callback(void* arg);
    
    /**
     * @brief Check and process threshold violations
     * @param temperature Current temperature reading
     */
    void check_thresholds(float temperature);
    
    /**
     * @brief Validate configuration parameters
     * @param config Configuration to validate
     * @return Error code (TEMP_SUCCESS if valid)
     */
    HfTempError_t validate_config(const HfTempConfig_t* config) const;
    
    /**
     * @brief Validate ESP32-specific configuration
     * @param esp_config ESP32 configuration to validate
     * @return Error code (TEMP_SUCCESS if valid)
     */
    HfTempError_t validate_esp_config(const EspTempConfig_t* esp_config) const;
    
    //--------------------------------------
    //  Static Members
    //--------------------------------------
    
    static const char* TAG;               ///< Logging tag for ESP_LOG
    
    //--------------------------------------
    //  Prevent Copying
    //--------------------------------------
    
    EspTemperature(const EspTemperature&) = delete;
    EspTemperature& operator=(const EspTemperature&) = delete;
};

} // namespace HardFOC