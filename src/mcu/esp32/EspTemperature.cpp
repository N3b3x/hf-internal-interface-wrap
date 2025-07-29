/**
 * @file EspTemperature.cpp
 * @brief ESP32-C6 temperature sensor implementation for the HardFOC system.
 *
 * This file implements the ESP32-C6 specific temperature sensor functionality
 * using the ESP-IDF temperature sensor driver. It provides a complete implementation
 * of the BaseTemperature interface with ESP32-specific optimizations.
 *
 * @author HardFOC Development Team
 * @date 2025
 * @copyright HardFOC
 */

#include "EspTemperature.h"
#include <algorithm>
#include <cstring>
#include <cmath>

extern "C" {
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

namespace HardFOC {

//--------------------------------------
//  Static Members
//--------------------------------------

const char* EspTemperature::TAG = "EspTemperature";

//--------------------------------------
//  Constructor & Destructor
//--------------------------------------

EspTemperature::EspTemperature() 
    : BaseTemperature()
    , current_state_(TEMP_STATE_UNINITIALIZED)
    , initialized_(false)
    , enabled_(false)
{
    // Initialize ESP32-specific state
    memset(&esp_state_, 0, sizeof(esp_state_));
    esp_state_.handle = nullptr;
    esp_state_.current_range = ESP_TEMP_RANGE_UNKNOWN;
    esp_state_.calibration_offset = 0.0f;
    esp_state_.is_monitoring = false;
    esp_state_.monitoring_timer = nullptr;
    esp_state_.reading_callback = nullptr;
    esp_state_.callback_user_data = nullptr;
    esp_state_.threshold_callback = nullptr;
    esp_state_.threshold_user_data = nullptr;
    esp_state_.low_threshold = -20.0f;
    esp_state_.high_threshold = 100.0f;
    esp_state_.threshold_monitoring_enabled = false;
    esp_state_.last_error = TEMP_SUCCESS;
    esp_state_.error_callback = nullptr;
    esp_state_.error_user_data = nullptr;
    
    // Initialize configuration with defaults
    esp_config_ = ESP_TEMP_CONFIG_DEFAULT();
    base_config_ = HF_TEMP_CONFIG_DEFAULT();
    
    ESP_LOGI(TAG, "EspTemperature instance created");
}

EspTemperature::~EspTemperature() {
    ESP_LOGI(TAG, "EspTemperature destructor called");
    
    // Ensure proper cleanup
    if (initialized_) {
        deinitialize();
    }
}

//--------------------------------------
//  Core Temperature Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::initialize(const HfTempConfig_t* config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        ESP_LOGW(TAG, "Temperature sensor already initialized");
        return TEMP_ERR_ALREADY_INITIALIZED;
    }
    
    if (config == nullptr) {
        ESP_LOGE(TAG, "Configuration pointer is null");
        return TEMP_ERR_NULL_POINTER;
    }
    
    // Validate configuration
    HfTempError_t error = validate_config(config);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Invalid configuration: %s", hf_temp_get_error_string(error));
        return error;
    }
    
    // Store base configuration
    base_config_ = *config;
    
    // Convert to ESP32-specific configuration if needed
    if (esp_config_.auto_range_selection) {
        esp_config_.preferred_range = find_optimal_range(config->range_min_celsius, config->range_max_celsius);
    }
    
    // Configure ESP-IDF sensor
    error = configure_esp_sensor();
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure ESP sensor: %s", hf_temp_get_error_string(error));
        return error;
    }
    
    initialized_ = true;
    current_state_ = TEMP_STATE_INITIALIZED;
    
    ESP_LOGI(TAG, "Temperature sensor initialized successfully");
    ESP_LOGI(TAG, "Range: %.1f°C to %.1f°C", config->range_min_celsius, config->range_max_celsius);
    ESP_LOGI(TAG, "Resolution: %.2f°C", config->resolution);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::initialize_esp32(const EspTempConfig_t* esp_config) {
    if (esp_config == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    // Validate ESP32-specific configuration
    HfTempError_t error = validate_esp_config(esp_config);
    if (error != TEMP_SUCCESS) {
        return error;
    }
    
    esp_config_ = *esp_config;
    
    // Create base configuration from ESP32 config
    HfTempConfig_t base_config = HF_TEMP_CONFIG_DEFAULT();
    
    // Set range based on ESP32 range selection
    float min_temp, max_temp, accuracy;
    get_range_config(esp_config->preferred_range, &min_temp, &max_temp, &accuracy);
    base_config.range_min_celsius = min_temp;
    base_config.range_max_celsius = max_temp;
    base_config.resolution = ESP_TEMP_RESOLUTION_CELSIUS;
    base_config.enable_power_management = esp_config->enable_power_management;
    base_config.timeout_ms = esp_config->timeout_ms;
    base_config.sensor_type = TEMP_SENSOR_TYPE_INTERNAL;
    base_config.capabilities = ESP_TEMP_CAPABILITIES;
    
    return initialize(&base_config);
}

HfTempError_t EspTemperature::deinitialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGW(TAG, "Temperature sensor not initialized");
        return TEMP_ERR_NOT_INITIALIZED;
    }
    
    // Stop any ongoing monitoring
    if (esp_state_.is_monitoring) {
        stop_continuous_monitoring();
    }
    
    // Disable sensor if enabled
    if (enabled_) {
        disable();
    }
    
    // Cleanup ESP-IDF resources
    if (esp_state_.handle != nullptr) {
        esp_err_t esp_err = temperature_sensor_uninstall(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to uninstall temperature sensor: %s", esp_err_to_name(esp_err));
        }
        esp_state_.handle = nullptr;
    }
    
    // Reset state
    initialized_ = false;
    enabled_ = false;
    current_state_ = TEMP_STATE_UNINITIALIZED;
    esp_state_.current_range = ESP_TEMP_RANGE_UNKNOWN;
    
    ESP_LOGI(TAG, "Temperature sensor deinitialized");
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::enable() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return set_last_error(TEMP_ERR_NOT_INITIALIZED), TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (enabled_) {
        ESP_LOGW(TAG, "Temperature sensor already enabled");
        return TEMP_SUCCESS;
    }
    
    if (esp_state_.handle == nullptr) {
        ESP_LOGE(TAG, "ESP sensor handle is null");
        return set_last_error(TEMP_ERR_SENSOR_NOT_AVAILABLE), TEMP_ERR_SENSOR_NOT_AVAILABLE;
    }
    
    esp_err_t esp_err = temperature_sensor_enable(esp_state_.handle);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable temperature sensor: %s", esp_err_to_name(esp_err));
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    enabled_ = true;
    current_state_ = TEMP_STATE_ENABLED;
    
    ESP_LOGI(TAG, "Temperature sensor enabled");
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::disable() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return set_last_error(TEMP_ERR_NOT_INITIALIZED), TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (!enabled_) {
        ESP_LOGW(TAG, "Temperature sensor already disabled");
        return TEMP_SUCCESS;
    }
    
    // Stop monitoring if active
    if (esp_state_.is_monitoring) {
        stop_continuous_monitoring();
    }
    
    if (esp_state_.handle != nullptr) {
        esp_err_t esp_err = temperature_sensor_disable(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to disable temperature sensor: %s", esp_err_to_name(esp_err));
        }
    }
    
    enabled_ = false;
    current_state_ = TEMP_STATE_DISABLED;
    
    ESP_LOGI(TAG, "Temperature sensor disabled");
    
    return TEMP_SUCCESS;
}

bool EspTemperature::is_initialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

bool EspTemperature::is_enabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

HfTempState_t EspTemperature::get_state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_state_;
}

//--------------------------------------
//  Temperature Reading Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::read_celsius(float* temperature_celsius) {
    if (temperature_celsius == nullptr) {
        return set_last_error(TEMP_ERR_NULL_POINTER), TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return set_last_error(TEMP_ERR_NOT_INITIALIZED), TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (!enabled_) {
        ESP_LOGE(TAG, "Temperature sensor not enabled");
        return set_last_error(TEMP_ERR_SENSOR_DISABLED), TEMP_ERR_SENSOR_DISABLED;
    }
    
    if (esp_state_.handle == nullptr) {
        ESP_LOGE(TAG, "ESP sensor handle is null");
        return set_last_error(TEMP_ERR_SENSOR_NOT_AVAILABLE), TEMP_ERR_SENSOR_NOT_AVAILABLE;
    }
    
    current_state_ = TEMP_STATE_READING;
    
    float raw_temp;
    esp_err_t esp_err = temperature_sensor_get_celsius(esp_state_.handle, &raw_temp);
    
    current_state_ = TEMP_STATE_ENABLED;
    
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", esp_err_to_name(esp_err));
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    // Apply calibration offset
    *temperature_celsius = raw_temp + esp_state_.calibration_offset;
    
    // Check if within valid range
    float min_temp, max_temp, accuracy;
    get_range_config(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
    
    if (!hf_temp_is_in_range(*temperature_celsius, min_temp, max_temp)) {
        ESP_LOGW(TAG, "Temperature %.2f°C is outside range [%.1f, %.1f]°C", 
                 *temperature_celsius, min_temp, max_temp);
        return set_last_error(TEMP_ERR_OUT_OF_RANGE), TEMP_ERR_OUT_OF_RANGE;
    }
    
    // Check thresholds if enabled
    if (esp_state_.threshold_monitoring_enabled) {
        check_thresholds(*temperature_celsius);
    }
    
    ESP_LOGD(TAG, "Temperature reading: %.2f°C (raw: %.2f°C, offset: %.2f°C)", 
             *temperature_celsius, raw_temp, esp_state_.calibration_offset);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::read_temperature(HfTempReading_t* reading) {
    if (reading == nullptr) {
        return set_last_error(TEMP_ERR_NULL_POINTER), TEMP_ERR_NULL_POINTER;
    }
    
    // Initialize reading structure
    memset(reading, 0, sizeof(HfTempReading_t));
    reading->timestamp_us = esp_timer_get_time();
    
    float temperature;
    HfTempError_t error = read_celsius(&temperature);
    
    reading->temperature_celsius = temperature;
    reading->error = error;
    reading->is_valid = (error == TEMP_SUCCESS);
    
    if (error == TEMP_SUCCESS) {
        // Get accuracy for current range
        float min_temp, max_temp, accuracy;
        get_range_config(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
        reading->accuracy_celsius = accuracy;
        
        // Store raw temperature (before calibration)
        reading->temperature_raw = temperature - esp_state_.calibration_offset;
    }
    
    return error;
}

HfTempError_t EspTemperature::start_async_read() {
    // ESP32-C6 temperature sensor doesn't support true asynchronous reading
    // We implement this as a synchronous read for now
    // Future versions could use FreeRTOS tasks for async behavior
    ESP_LOGW(TAG, "Async read not yet implemented, performing synchronous read");
    return TEMP_ERR_OPERATION_PENDING;
}

bool EspTemperature::is_read_complete() const {
    // Since we don't support true async reads yet, always return true
    return true;
}

HfTempError_t EspTemperature::get_async_result(HfTempReading_t* reading) {
    // Since async reads are not implemented, just perform a synchronous read
    return read_temperature(reading);
}

//--------------------------------------
//  Configuration Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::set_range(float min_celsius, float max_celsius) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (min_celsius >= max_celsius) {
        ESP_LOGE(TAG, "Invalid range: min=%.1f >= max=%.1f", min_celsius, max_celsius);
        return set_last_error(TEMP_ERR_INVALID_RANGE), TEMP_ERR_INVALID_RANGE;
    }
    
    // Find optimal range for the specified temperature range
    EspTempRange_t optimal_range = find_optimal_range(min_celsius, max_celsius);
    if (optimal_range == ESP_TEMP_RANGE_UNKNOWN) {
        ESP_LOGE(TAG, "No suitable range found for [%.1f, %.1f]°C", min_celsius, max_celsius);
        return set_last_error(TEMP_ERR_UNSUPPORTED_RANGE), TEMP_ERR_UNSUPPORTED_RANGE;
    }
    
    // Update configuration
    base_config_.range_min_celsius = min_celsius;
    base_config_.range_max_celsius = max_celsius;
    esp_config_.preferred_range = optimal_range;
    
    // If initialized, reconfigure the sensor
    if (initialized_) {
        HfTempError_t error = setup_range(optimal_range);
        if (error != TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Failed to setup new range: %s", hf_temp_get_error_string(error));
            return set_last_error(error), error;
        }
    }
    
    ESP_LOGI(TAG, "Range set to [%.1f, %.1f]°C (ESP range: %d)", 
             min_celsius, max_celsius, optimal_range);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::get_range(float* min_celsius, float* max_celsius) const {
    if (min_celsius == nullptr || max_celsius == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    *min_celsius = base_config_.range_min_celsius;
    *max_celsius = base_config_.range_max_celsius;
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::set_resolution(float resolution_celsius) {
    // ESP32-C6 has fixed resolution, but we can store the requested value
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (resolution_celsius <= 0.0f) {
        ESP_LOGE(TAG, "Invalid resolution: %.3f", resolution_celsius);
        return set_last_error(TEMP_ERR_INVALID_PARAMETER), TEMP_ERR_INVALID_PARAMETER;
    }
    
    base_config_.resolution = resolution_celsius;
    
    if (resolution_celsius < ESP_TEMP_RESOLUTION_CELSIUS) {
        ESP_LOGW(TAG, "Requested resolution %.3f°C is better than hardware capability %.3f°C", 
                 resolution_celsius, ESP_TEMP_RESOLUTION_CELSIUS);
    }
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::get_resolution(float* resolution_celsius) const {
    if (resolution_celsius == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Return the actual hardware resolution
    *resolution_celsius = ESP_TEMP_RESOLUTION_CELSIUS;
    
    return TEMP_SUCCESS;
}

//--------------------------------------
//  Threshold Monitoring Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::set_thresholds(float low_threshold_celsius, float high_threshold_celsius) {
    if (low_threshold_celsius >= high_threshold_celsius) {
        ESP_LOGE(TAG, "Invalid thresholds: low=%.1f >= high=%.1f", 
                 low_threshold_celsius, high_threshold_celsius);
        return set_last_error(TEMP_ERR_INVALID_THRESHOLD), TEMP_ERR_INVALID_THRESHOLD;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.low_threshold = low_threshold_celsius;
    esp_state_.high_threshold = high_threshold_celsius;
    
    ESP_LOGI(TAG, "Thresholds set: low=%.1f°C, high=%.1f°C", 
             low_threshold_celsius, high_threshold_celsius);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::get_thresholds(float* low_threshold_celsius, float* high_threshold_celsius) const {
    if (low_threshold_celsius == nullptr || high_threshold_celsius == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    *low_threshold_celsius = esp_state_.low_threshold;
    *high_threshold_celsius = esp_state_.high_threshold;
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::enable_threshold_monitoring(HfTempThresholdCallback_t callback, void* user_data) {
    if (callback == nullptr) {
        ESP_LOGE(TAG, "Threshold callback is null");
        return set_last_error(TEMP_ERR_NULL_POINTER), TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.threshold_callback = callback;
    esp_state_.threshold_user_data = user_data;
    esp_state_.threshold_monitoring_enabled = true;
    
    ESP_LOGI(TAG, "Threshold monitoring enabled");
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::disable_threshold_monitoring() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.threshold_monitoring_enabled = false;
    esp_state_.threshold_callback = nullptr;
    esp_state_.threshold_user_data = nullptr;
    
    ESP_LOGI(TAG, "Threshold monitoring disabled");
    
    return TEMP_SUCCESS;
}

//--------------------------------------
//  Continuous Monitoring Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::start_continuous_monitoring(uint32_t sample_rate_hz, 
                                                         HfTempReadingCallback_t callback, 
                                                         void* user_data) {
    if (callback == nullptr) {
        ESP_LOGE(TAG, "Reading callback is null");
        return set_last_error(TEMP_ERR_NULL_POINTER), TEMP_ERR_NULL_POINTER;
    }
    
    if (sample_rate_hz == 0 || sample_rate_hz > 1000) {
        ESP_LOGE(TAG, "Invalid sample rate: %lu Hz (valid range: 1-1000)", sample_rate_hz);
        return set_last_error(TEMP_ERR_INVALID_PARAMETER), TEMP_ERR_INVALID_PARAMETER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !enabled_) {
        ESP_LOGE(TAG, "Temperature sensor must be initialized and enabled");
        return set_last_error(TEMP_ERR_INVALID_STATE), TEMP_ERR_INVALID_STATE;
    }
    
    if (esp_state_.is_monitoring) {
        ESP_LOGW(TAG, "Continuous monitoring already active");
        return TEMP_SUCCESS;
    }
    
    // Setup callback and user data
    esp_state_.reading_callback = callback;
    esp_state_.callback_user_data = user_data;
    
    // Create timer for monitoring
    uint64_t period_us = 1000000ULL / sample_rate_hz;
    
    esp_timer_create_args_t timer_args = {
        .callback = monitoring_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "temp_monitor",
        .skip_unhandled_events = true
    };
    
    esp_err_t esp_err = esp_timer_create(&timer_args, &esp_state_.monitoring_timer);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create monitoring timer: %s", esp_err_to_name(esp_err));
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    esp_err = esp_timer_start_periodic(esp_state_.monitoring_timer, period_us);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start monitoring timer: %s", esp_err_to_name(esp_err));
        esp_timer_delete(esp_state_.monitoring_timer);
        esp_state_.monitoring_timer = nullptr;
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    esp_state_.is_monitoring = true;
    
    ESP_LOGI(TAG, "Continuous monitoring started at %lu Hz", sample_rate_hz);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::stop_continuous_monitoring() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!esp_state_.is_monitoring) {
        ESP_LOGW(TAG, "Continuous monitoring not active");
        return TEMP_SUCCESS;
    }
    
    if (esp_state_.monitoring_timer != nullptr) {
        esp_timer_stop(esp_state_.monitoring_timer);
        esp_timer_delete(esp_state_.monitoring_timer);
        esp_state_.monitoring_timer = nullptr;
    }
    
    esp_state_.is_monitoring = false;
    esp_state_.reading_callback = nullptr;
    esp_state_.callback_user_data = nullptr;
    
    ESP_LOGI(TAG, "Continuous monitoring stopped");
    
    return TEMP_SUCCESS;
}

bool EspTemperature::is_monitoring_active() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.is_monitoring;
}

//--------------------------------------
//  Calibration Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::calibrate(float reference_temperature_celsius) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !enabled_) {
        ESP_LOGE(TAG, "Sensor must be initialized and enabled for calibration");
        return set_last_error(TEMP_ERR_INVALID_STATE), TEMP_ERR_INVALID_STATE;
    }
    
    // Read current raw temperature
    float raw_temp;
    esp_err_t esp_err = temperature_sensor_get_celsius(esp_state_.handle, &raw_temp);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature for calibration: %s", esp_err_to_name(esp_err));
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    // Calculate new offset
    float new_offset = reference_temperature_celsius - raw_temp;
    esp_state_.calibration_offset = new_offset;
    
    ESP_LOGI(TAG, "Calibration completed: offset=%.3f°C (ref=%.2f°C, raw=%.2f°C)", 
             new_offset, reference_temperature_celsius, raw_temp);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::set_calibration_offset(float offset_celsius) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.calibration_offset = offset_celsius;
    
    ESP_LOGI(TAG, "Calibration offset set to %.3f°C", offset_celsius);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::get_calibration_offset(float* offset_celsius) const {
    if (offset_celsius == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    *offset_celsius = esp_state_.calibration_offset;
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::reset_calibration() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.calibration_offset = 0.0f;
    
    ESP_LOGI(TAG, "Calibration reset to default");
    
    return TEMP_SUCCESS;
}

//--------------------------------------
//  Power Management Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::enter_sleep_mode() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Sensor not initialized");
        return set_last_error(TEMP_ERR_NOT_INITIALIZED), TEMP_ERR_NOT_INITIALIZED;
    }
    
    // Stop monitoring if active
    if (esp_state_.is_monitoring) {
        stop_continuous_monitoring();
    }
    
    // Disable sensor to save power
    if (enabled_) {
        disable();
    }
    
    current_state_ = TEMP_STATE_SLEEPING;
    
    ESP_LOGI(TAG, "Entered sleep mode");
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::exit_sleep_mode() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (current_state_ != TEMP_STATE_SLEEPING) {
        ESP_LOGW(TAG, "Sensor not in sleep mode");
        return TEMP_SUCCESS;
    }
    
    current_state_ = TEMP_STATE_INITIALIZED;
    
    ESP_LOGI(TAG, "Exited sleep mode");
    
    return TEMP_SUCCESS;
}

bool EspTemperature::is_sleeping() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (current_state_ == TEMP_STATE_SLEEPING);
}

//--------------------------------------
//  Information Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::get_sensor_info(HfTempSensorInfo_t* info) const {
    if (info == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    info->sensor_type = TEMP_SENSOR_TYPE_INTERNAL;
    info->min_temp_celsius = ESP_TEMP_MIN_CELSIUS;
    info->max_temp_celsius = ESP_TEMP_MAX_CELSIUS;
    info->resolution_celsius = ESP_TEMP_RESOLUTION_CELSIUS;
    
    // Get accuracy for current range
    float min_temp, max_temp, accuracy;
    get_range_config(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
    info->accuracy_celsius = accuracy;
    
    info->response_time_ms = ESP_TEMP_RESPONSE_TIME_MS;
    info->capabilities = ESP_TEMP_CAPABILITIES;
    info->manufacturer = "Espressif";
    info->model = "ESP32-C6 Internal";
    info->version = "1.0.0";
    
    return TEMP_SUCCESS;
}

uint32_t EspTemperature::get_capabilities() const {
    return ESP_TEMP_CAPABILITIES;
}

//--------------------------------------
//  Error Handling Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::set_error_callback(HfTempErrorCallback_t callback, void* user_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.error_callback = callback;
    esp_state_.error_user_data = user_data;
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::clear_error_callback() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.error_callback = nullptr;
    esp_state_.error_user_data = nullptr;
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::get_last_error() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.last_error;
}

HfTempError_t EspTemperature::clear_last_error() {
    std::lock_guard<std::mutex> lock(mutex_);
    esp_state_.last_error = TEMP_SUCCESS;
    return TEMP_SUCCESS;
}

//--------------------------------------
//  Self-Test Interface Implementation
//--------------------------------------

HfTempError_t EspTemperature::self_test() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Sensor not initialized");
        return set_last_error(TEMP_ERR_NOT_INITIALIZED), TEMP_ERR_NOT_INITIALIZED;
    }
    
    ESP_LOGI(TAG, "Starting self-test...");
    
    // Test 1: Check if sensor can be enabled/disabled
    bool was_enabled = enabled_;
    
    if (!enabled_) {
        HfTempError_t error = enable();
        if (error != TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Self-test failed: unable to enable sensor");
            return set_last_error(TEMP_ERR_HARDWARE_FAULT), TEMP_ERR_HARDWARE_FAULT;
        }
    }
    
    // Test 2: Try to read temperature
    float temp;
    HfTempError_t error = read_celsius(&temp);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Self-test failed: unable to read temperature");
        if (!was_enabled) disable();
        return set_last_error(TEMP_ERR_HARDWARE_FAULT), TEMP_ERR_HARDWARE_FAULT;
    }
    
    // Test 3: Check if temperature is reasonable
    if (temp < -50.0f || temp > 150.0f) {
        ESP_LOGE(TAG, "Self-test failed: unreasonable temperature reading %.2f°C", temp);
        if (!was_enabled) disable();
        return set_last_error(TEMP_ERR_INVALID_READING), TEMP_ERR_INVALID_READING;
    }
    
    // Test 4: Multiple readings for consistency
    float temp1, temp2, temp3;
    read_celsius(&temp1);
    vTaskDelay(pdMS_TO_TICKS(100));
    read_celsius(&temp2);
    vTaskDelay(pdMS_TO_TICKS(100));
    read_celsius(&temp3);
    
    float max_diff = std::max({std::abs(temp1 - temp2), std::abs(temp2 - temp3), std::abs(temp1 - temp3)});
    if (max_diff > 5.0f) {
        ESP_LOGW(TAG, "Self-test warning: temperature readings vary significantly (max diff: %.2f°C)", max_diff);
    }
    
    // Restore original state
    if (!was_enabled) {
        disable();
    }
    
    ESP_LOGI(TAG, "Self-test passed. Temperature: %.2f°C", temp);
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::check_health() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (esp_state_.handle == nullptr) {
        return TEMP_ERR_HARDWARE_FAULT;
    }
    
    // Check if we can read temperature
    if (enabled_) {
        float temp;
        HfTempError_t error = read_celsius(&temp);
        if (error != TEMP_SUCCESS) {
            return TEMP_ERR_HARDWARE_FAULT;
        }
    }
    
    return TEMP_SUCCESS;
}

//--------------------------------------
//  ESP32-Specific Methods Implementation
//--------------------------------------

HfTempError_t EspTemperature::set_measurement_range(EspTempRange_t range) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (range <= ESP_TEMP_RANGE_UNKNOWN || range >= ESP_TEMP_RANGE_MAX) {
        ESP_LOGE(TAG, "Invalid measurement range: %d", range);
        return set_last_error(TEMP_ERR_INVALID_PARAMETER), TEMP_ERR_INVALID_PARAMETER;
    }
    
    esp_config_.preferred_range = range;
    
    if (initialized_) {
        HfTempError_t error = setup_range(range);
        if (error != TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Failed to setup measurement range: %s", hf_temp_get_error_string(error));
            return set_last_error(error), error;
        }
    }
    
    ESP_LOGI(TAG, "Measurement range set to %d", range);
    
    return TEMP_SUCCESS;
}

EspTempRange_t EspTemperature::get_measurement_range() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.current_range;
}

HfTempError_t EspTemperature::get_range_info(EspTempRange_t range, 
                                            float* min_celsius, 
                                            float* max_celsius, 
                                            float* accuracy_celsius) const {
    if (min_celsius == nullptr || max_celsius == nullptr || accuracy_celsius == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    get_range_config(range, min_celsius, max_celsius, accuracy_celsius);
    
    return TEMP_SUCCESS;
}

EspTempRange_t EspTemperature::find_optimal_range(float min_celsius, float max_celsius) const {
    struct RangeInfo {
        EspTempRange_t range;
        float min_temp;
        float max_temp;
        float accuracy;
    };
    
    RangeInfo ranges[] = {
        {ESP_TEMP_RANGE_NEG10_80, -10.0f, 80.0f, 1.0f},  // Best accuracy
        {ESP_TEMP_RANGE_20_100, 20.0f, 100.0f, 2.0f},
        {ESP_TEMP_RANGE_NEG30_50, -30.0f, 50.0f, 2.0f},
        {ESP_TEMP_RANGE_50_125, 50.0f, 125.0f, 3.0f},
        {ESP_TEMP_RANGE_NEG40_20, -40.0f, 20.0f, 3.0f}
    };
    
    // Find ranges that can cover the required range
    EspTempRange_t best_range = ESP_TEMP_RANGE_UNKNOWN;
    float best_accuracy = 999.0f;
    
    for (const auto& range : ranges) {
        if (range.min_temp <= min_celsius && range.max_temp >= max_celsius) {
            if (range.accuracy < best_accuracy) {
                best_accuracy = range.accuracy;
                best_range = range.range;
            }
        }
    }
    
    return best_range;
}

HfTempError_t EspTemperature::read_raw(float* raw_value) {
    if (raw_value == nullptr) {
        return set_last_error(TEMP_ERR_NULL_POINTER), TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !enabled_) {
        ESP_LOGE(TAG, "Sensor must be initialized and enabled");
        return set_last_error(TEMP_ERR_INVALID_STATE), TEMP_ERR_INVALID_STATE;
    }
    
    esp_err_t esp_err = temperature_sensor_get_celsius(esp_state_.handle, raw_value);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read raw temperature: %s", esp_err_to_name(esp_err));
        HfTempError_t error = convert_esp_error(esp_err);
        return set_last_error(error), error;
    }
    
    return TEMP_SUCCESS;
}

temperature_sensor_handle_t EspTemperature::get_esp_handle() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.handle;
}

//--------------------------------------
//  Private Methods Implementation
//--------------------------------------

HfTempError_t EspTemperature::convert_esp_error(esp_err_t esp_err) const {
    switch (esp_err) {
        case ESP_OK:
            return TEMP_SUCCESS;
        case ESP_ERR_INVALID_ARG:
            return TEMP_ERR_INVALID_PARAMETER;
        case ESP_ERR_INVALID_STATE:
            return TEMP_ERR_INVALID_STATE;
        case ESP_ERR_NO_MEM:
            return TEMP_ERR_OUT_OF_MEMORY;
        case ESP_ERR_NOT_FOUND:
            return TEMP_ERR_SENSOR_NOT_AVAILABLE;
        case ESP_ERR_TIMEOUT:
            return TEMP_ERR_TIMEOUT;
        case ESP_FAIL:
            return TEMP_ERR_READ_FAILED;
        default:
            return TEMP_ERR_FAILURE;
    }
}

HfTempError_t EspTemperature::configure_esp_sensor() {
    // Setup range configuration
    HfTempError_t error = setup_range(esp_config_.preferred_range);
    if (error != TEMP_SUCCESS) {
        return error;
    }
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::setup_range(EspTempRange_t range) {
    float min_temp, max_temp, accuracy;
    get_range_config(range, &min_temp, &max_temp, &accuracy);
    
    // Configure ESP-IDF temperature sensor
    temperature_sensor_config_t temp_config = {
        .range_min = (int)min_temp,
        .range_max = (int)max_temp,
        .clk_src = esp_config_.clk_src,
        .flags = {
            .allow_pd = esp_config_.enable_power_management ? 1U : 0U
        }
    };
    
    // Install temperature sensor if not already done
    if (esp_state_.handle == nullptr) {
        esp_err_t esp_err = temperature_sensor_install(&temp_config, &esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to install temperature sensor: %s", esp_err_to_name(esp_err));
            return convert_esp_error(esp_err);
        }
    }
    
    esp_state_.current_range = range;
    
    ESP_LOGI(TAG, "Temperature range configured: %.0f to %.0f°C (accuracy: ±%.1f°C)", 
             min_temp, max_temp, accuracy);
    
    return TEMP_SUCCESS;
}

void EspTemperature::get_range_config(EspTempRange_t range, 
                                     float* min_celsius, 
                                     float* max_celsius, 
                                     float* accuracy_celsius) const {
    switch (range) {
        case ESP_TEMP_RANGE_50_125:
            *min_celsius = 50.0f;
            *max_celsius = 125.0f;
            *accuracy_celsius = 3.0f;
            break;
        case ESP_TEMP_RANGE_20_100:
            *min_celsius = 20.0f;
            *max_celsius = 100.0f;
            *accuracy_celsius = 2.0f;
            break;
        case ESP_TEMP_RANGE_NEG10_80:
            *min_celsius = -10.0f;
            *max_celsius = 80.0f;
            *accuracy_celsius = 1.0f;
            break;
        case ESP_TEMP_RANGE_NEG30_50:
            *min_celsius = -30.0f;
            *max_celsius = 50.0f;
            *accuracy_celsius = 2.0f;
            break;
        case ESP_TEMP_RANGE_NEG40_20:
            *min_celsius = -40.0f;
            *max_celsius = 20.0f;
            *accuracy_celsius = 3.0f;
            break;
        default:
            *min_celsius = -10.0f;
            *max_celsius = 80.0f;
            *accuracy_celsius = 1.0f;
            break;
    }
}

void EspTemperature::set_last_error(HfTempError_t error) {
    esp_state_.last_error = error;
    
    if (error != TEMP_SUCCESS && esp_state_.error_callback != nullptr) {
        esp_state_.error_callback(error, hf_temp_get_error_string(error), esp_state_.error_user_data);
    }
}

void EspTemperature::monitoring_timer_callback(void* arg) {
    EspTemperature* instance = static_cast<EspTemperature*>(arg);
    if (instance == nullptr) {
        return;
    }
    
    HfTempReading_t reading;
    HfTempError_t error = instance->read_temperature(&reading);
    
    if (instance->esp_state_.reading_callback != nullptr) {
        instance->esp_state_.reading_callback(&reading, instance->esp_state_.callback_user_data);
    }
    
    if (error != TEMP_SUCCESS) {
        ESP_LOGW(TAG, "Error during continuous monitoring: %s", hf_temp_get_error_string(error));
    }
}

void EspTemperature::check_thresholds(float temperature) {
    bool threshold_exceeded = false;
    uint32_t threshold_type = 0;
    
    if (temperature < esp_state_.low_threshold) {
        threshold_exceeded = true;
        threshold_type = 0; // Low threshold
    } else if (temperature > esp_state_.high_threshold) {
        threshold_exceeded = true;
        threshold_type = 1; // High threshold
    }
    
    if (threshold_exceeded && esp_state_.threshold_callback != nullptr) {
        esp_state_.threshold_callback(temperature, threshold_type, esp_state_.threshold_user_data);
    }
}

HfTempError_t EspTemperature::validate_config(const HfTempConfig_t* config) const {
    if (config == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    if (config->range_min_celsius >= config->range_max_celsius) {
        return TEMP_ERR_INVALID_RANGE;
    }
    
    if (config->range_min_celsius < ESP_TEMP_MIN_CELSIUS || 
        config->range_max_celsius > ESP_TEMP_MAX_CELSIUS) {
        return TEMP_ERR_UNSUPPORTED_RANGE;
    }
    
    if (config->resolution <= 0.0f) {
        return TEMP_ERR_INVALID_PARAMETER;
    }
    
    if (config->timeout_ms == 0) {
        return TEMP_ERR_INVALID_PARAMETER;
    }
    
    return TEMP_SUCCESS;
}

HfTempError_t EspTemperature::validate_esp_config(const EspTempConfig_t* esp_config) const {
    if (esp_config == nullptr) {
        return TEMP_ERR_NULL_POINTER;
    }
    
    if (esp_config->preferred_range <= ESP_TEMP_RANGE_UNKNOWN || 
        esp_config->preferred_range >= ESP_TEMP_RANGE_MAX) {
        return TEMP_ERR_INVALID_PARAMETER;
    }
    
    if (esp_config->timeout_ms == 0) {
        return TEMP_ERR_INVALID_PARAMETER;
    }
    
    return TEMP_SUCCESS;
}

} // namespace HardFOC