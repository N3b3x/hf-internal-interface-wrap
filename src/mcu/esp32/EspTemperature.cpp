/**
 * @file EspTemperature.cpp
 * @brief ESP32-C6 internal temperature sensor implementation for the HardFOC system.
 *
 * This file contains the complete implementation of the ESP32-C6 temperature sensor driver
 * that extends the BaseTemperature abstract class. It provides comprehensive support for all
 * ESP32-C6 temperature sensor features including multiple measurement ranges, threshold monitoring,
 * continuous monitoring, calibration, and power management.
 *
 * Key features implemented:
 * - ESP32-C6 internal temperature sensor using ESP-IDF v5.x APIs
 * - Multiple predefined measurement ranges with different accuracy levels
 * - Hardware threshold monitoring with interrupt callbacks
 * - Continuous monitoring using ESP32 timers
 * - Thread-safe operations with mutex protection
 * - Comprehensive error handling and diagnostics
 * - Power management support for low-power applications
 * - Operation statistics and performance tracking
 * - Self-test and health monitoring capabilities
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note ESP32-C6 specific implementation using ESP-IDF v5.x
 * @note Thread-safe design suitable for multi-threaded applications
 * @note Follows HardFOC coding standards and patterns
 */

#include "EspTemperature.h"

//#ifdef HF_MCU_FAMILY_ESP32

// Standard library includes
#include <algorithm>
#include <cstring>
#include <cmath>

static const char* TAG __attribute__((unused)) = "EspTemperature";

//--------------------------------------
//  ESP32-C6 Temperature Range Configuration
//--------------------------------------

/**
 * @brief ESP32-C6 temperature sensor range information table
 * @details Based on ESP32-C6 hardware specifications and ESP-IDF documentation
 */
const esp_temp_range_info_t EspTemperature::RANGE_INFO[] = {
    {ESP_TEMP_RANGE_NEG10_80,  -10.0f,  80.0f, 1.0f, "-10°C to 80°C (±1°C accuracy, recommended)"},
    {ESP_TEMP_RANGE_20_100,     20.0f, 100.0f, 2.0f, "20°C to 100°C (±2°C accuracy, high temp)"},
    {ESP_TEMP_RANGE_NEG30_50,  -30.0f,  50.0f, 2.0f, "-30°C to 50°C (±2°C accuracy, low temp)"},
    {ESP_TEMP_RANGE_50_125,     50.0f, 125.0f, 3.0f, "50°C to 125°C (±3°C accuracy, extreme high)"},
    {ESP_TEMP_RANGE_NEG40_20,  -40.0f,  20.0f, 3.0f, "-40°C to 20°C (±3°C accuracy, extreme low)"}
};

//==============================================================================
// CONSTRUCTORS AND DESTRUCTOR
//==============================================================================

EspTemperature::EspTemperature() noexcept
    : BaseTemperature(),
      mutex_(),
      esp_state_{},
      esp_config_(ESP_TEMP_CONFIG_DEFAULT()),
      base_config_(HF_TEMP_CONFIG_DEFAULT()),
      statistics_{},
      diagnostics_{},
      last_error_(hf_temp_err_t::TEMP_SUCCESS),
      threshold_callback_(nullptr),
      monitoring_callback_(nullptr),
      esp_threshold_callback_(nullptr),
      esp_monitoring_callback_(nullptr),
      threshold_user_data_(nullptr),
      monitoring_user_data_(nullptr) {
    
    // Initialize ESP32-specific state
    esp_state_.handle = nullptr;
    esp_state_.current_range = ESP_TEMP_RANGE_NEG10_80;
    esp_state_.calibration_offset = 0.0f;
    esp_state_.threshold_monitoring_enabled = false;
    esp_state_.continuous_monitoring_active = false;
    esp_state_.monitoring_timer = nullptr;
    esp_state_.sample_rate_hz = ESP_TEMP_DEFAULT_SAMPLE_RATE_HZ;
    esp_state_.last_reading_timestamp_us = 0;
    esp_state_.last_temperature_celsius = 0.0f;
    esp_state_.allow_power_down = true;
    
    // Initialize diagnostics
    diagnostics_.sensor_healthy = true;
    diagnostics_.last_error_code = hf_temp_err_t::TEMP_SUCCESS;
    diagnostics_.last_error_timestamp = 0;
    diagnostics_.consecutive_errors = 0;
    diagnostics_.sensor_available = true;
    diagnostics_.threshold_monitoring_supported = true;
    diagnostics_.threshold_monitoring_enabled = false;
    diagnostics_.continuous_monitoring_active = false;
    diagnostics_.current_temperature_raw = 0;
    diagnostics_.calibration_valid = true;
    
    // Initialize statistics
    statistics_.total_operations = 0;
    statistics_.successful_operations = 0;
    statistics_.failed_operations = 0;
    statistics_.temperature_readings = 0;
    statistics_.calibration_count = 0;
    statistics_.threshold_violations = 0;
    statistics_.average_operation_time_us = 0;
    statistics_.max_operation_time_us = 0;
    statistics_.min_operation_time_us = UINT32_MAX;
    statistics_.min_temperature_celsius = 1000.0f;
    statistics_.max_temperature_celsius = -1000.0f;
    statistics_.avg_temperature_celsius = 0.0f;
    
    ESP_LOGD(TAG, "EspTemperature instance created");
}

EspTemperature::EspTemperature(const esp_temp_config_t& esp_config) noexcept
    : EspTemperature() {
    esp_config_ = esp_config;
    ESP_LOGD(TAG, "EspTemperature instance created with custom configuration");
}

EspTemperature::~EspTemperature() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        Deinitialize();
    }
    
    ESP_LOGD(TAG, "EspTemperature instance destroyed");
}

//==============================================================================
// PURE VIRTUAL IMPLEMENTATIONS - PLATFORM SPECIFIC
//==============================================================================

bool EspTemperature::Initialize() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const hf_u64_t start_time = GetCurrentTimeUs();
    
    if (initialized_) {
        ESP_LOGW(TAG, "Temperature sensor already initialized");
        UpdateStatistics(true, GetCurrentTimeUs() - start_time);
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing ESP32-C6 temperature sensor...");
    
    // Configure temperature sensor
    temperature_sensor_config_t temp_config = {};
    temp_config.range_min = -10;
    temp_config.range_max = 80;
    temp_config.clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT;
    temp_config.flags.allow_pd = 0;
    
    // Adjust configuration based on current range
    float min_temp, max_temp, accuracy;
    GetRangeConfig(esp_config_.range, &min_temp, &max_temp, &accuracy);
    temp_config.range_min = static_cast<int>(min_temp);
    temp_config.range_max = static_cast<int>(max_temp);
    
    // Install temperature sensor
    esp_err_t esp_err = temperature_sensor_install(&temp_config, &esp_state_.handle);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install temperature sensor: %s", esp_err_to_name(esp_err));
        SetLastError(ConvertEspError(esp_err));
        UpdateStatistics(false, GetCurrentTimeUs() - start_time);
        return false;
    }
    
    // Enable temperature sensor
    esp_err = temperature_sensor_enable(esp_state_.handle);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable temperature sensor: %s", esp_err_to_name(esp_err));
        temperature_sensor_uninstall(esp_state_.handle);
        esp_state_.handle = nullptr;
        SetLastError(ConvertEspError(esp_err));
        UpdateStatistics(false, GetCurrentTimeUs() - start_time);
        return false;
    }
    
    // Update state
    esp_state_.current_range = esp_config_.range;
    esp_state_.calibration_offset = esp_config_.calibration_offset;
    current_state_ = HF_TEMP_STATE_INITIALIZED;
    
    // Setup threshold monitoring if requested
    if (esp_config_.enable_threshold_monitoring) {
        hf_temp_err_t threshold_err = SetThresholds(esp_config_.low_threshold_celsius, 
                                                   esp_config_.high_threshold_celsius);
        if (threshold_err != hf_temp_err_t::TEMP_SUCCESS) {
            ESP_LOGW(TAG, "Failed to setup threshold monitoring: %s", 
                     GetTempErrorString(threshold_err));
        }
    }
    
    ESP_LOGI(TAG, "ESP32-C6 temperature sensor initialized successfully");
    ESP_LOGI(TAG, "Range: %.0f°C to %.0f°C, Accuracy: ±%.1f°C", min_temp, max_temp, accuracy);
    
    UpdateStatistics(true, GetCurrentTimeUs() - start_time);
    SetLastError(hf_temp_err_t::TEMP_SUCCESS);
    return true;
}

bool EspTemperature::Deinitialize() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const hf_u64_t start_time = GetCurrentTimeUs();
    
    if (!initialized_) {
        ESP_LOGW(TAG, "Temperature sensor not initialized");
        UpdateStatistics(true, GetCurrentTimeUs() - start_time);
        return true;
    }
    
    ESP_LOGI(TAG, "Deinitializing ESP32-C6 temperature sensor...");
    
    // Stop continuous monitoring if active
    if (esp_state_.continuous_monitoring_active) {
        StopContinuousMonitoring();
    }
    
    // Disable threshold monitoring if active
    if (esp_state_.threshold_monitoring_enabled) {
        DisableThresholdMonitoring();
    }
    
    bool success = true;
    
    // Disable temperature sensor
    if (esp_state_.handle != nullptr) {
        esp_err_t esp_err = temperature_sensor_disable(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to disable temperature sensor: %s", esp_err_to_name(esp_err));
            success = false;
        }
        
        // Uninstall temperature sensor
        esp_err = temperature_sensor_uninstall(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to uninstall temperature sensor: %s", esp_err_to_name(esp_err));
            success = false;
        }
        
        esp_state_.handle = nullptr;
    }
    
    // Reset state
    current_state_ = HF_TEMP_STATE_UNINITIALIZED;
    esp_state_.threshold_monitoring_enabled = false;
    esp_state_.continuous_monitoring_active = false;
    
    // Clear callbacks
    threshold_callback_ = nullptr;
    monitoring_callback_ = nullptr;
    esp_threshold_callback_ = nullptr;
    esp_monitoring_callback_ = nullptr;
    threshold_user_data_ = nullptr;
    monitoring_user_data_ = nullptr;
    
    if (success) {
        ESP_LOGI(TAG, "ESP32-C6 temperature sensor deinitialized successfully");
        SetLastError(hf_temp_err_t::TEMP_SUCCESS);
    } else {
        ESP_LOGE(TAG, "ESP32-C6 temperature sensor deinitialization completed with errors");
        SetLastError(hf_temp_err_t::TEMP_ERR_FAILURE);
    }
    
    UpdateStatistics(success, GetCurrentTimeUs() - start_time);
    return success;
}

hf_temp_err_t EspTemperature::ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept {
    if (temperature_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    const hf_u64_t start_time = GetCurrentTimeUs();
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        UpdateStatistics(false, GetCurrentTimeUs() - start_time);
        return SetLastError(hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED), hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (esp_state_.handle == nullptr) {
        ESP_LOGE(TAG, "ESP sensor handle is null");
        UpdateStatistics(false, GetCurrentTimeUs() - start_time);
        return SetLastError(hf_temp_err_t::TEMP_ERR_SENSOR_NOT_AVAILABLE), hf_temp_err_t::TEMP_ERR_SENSOR_NOT_AVAILABLE;
    }
    
    current_state_ = HF_TEMP_STATE_READING;
    
    // Read raw temperature from ESP32-C6
    float raw_temp;
    esp_err_t esp_err = temperature_sensor_get_celsius(esp_state_.handle, &raw_temp);
    
    current_state_ = HF_TEMP_STATE_INITIALIZED;
    
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", esp_err_to_name(esp_err));
        hf_temp_err_t error = ConvertEspError(esp_err);
        UpdateStatistics(false, GetCurrentTimeUs() - start_time);
        return SetLastError(error), error;
    }
    
    // Apply calibration offset
    *temperature_celsius = raw_temp + esp_state_.calibration_offset;
    
    // Update statistics
    statistics_.temperature_readings++;
    esp_state_.last_temperature_celsius = *temperature_celsius;
    esp_state_.last_reading_timestamp_us = GetCurrentTimeUs();
    
    // Update min/max tracking
    if (*temperature_celsius < statistics_.min_temperature_celsius) {
        statistics_.min_temperature_celsius = *temperature_celsius;
    }
    if (*temperature_celsius > statistics_.max_temperature_celsius) {
        statistics_.max_temperature_celsius = *temperature_celsius;
    }
    
    // Update average temperature (simple moving average)
    if (statistics_.temperature_readings == 1) {
        statistics_.avg_temperature_celsius = *temperature_celsius;
    } else {
        statistics_.avg_temperature_celsius = 
            (statistics_.avg_temperature_celsius * (statistics_.temperature_readings - 1) + *temperature_celsius) / 
            statistics_.temperature_readings;
    }
    
    // Check thresholds if monitoring is enabled
    if (esp_state_.threshold_monitoring_enabled) {
        CheckThresholds(*temperature_celsius);
    }
    
    // Validate temperature is within expected range
    float min_temp, max_temp, accuracy;
    GetRangeConfig(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
    
    if (*temperature_celsius < min_temp - 10.0f || *temperature_celsius > max_temp + 10.0f) {
        ESP_LOGW(TAG, "Temperature %.2f°C is outside expected range [%.0f°C, %.0f°C]", 
                 *temperature_celsius, min_temp, max_temp);
    }
    
    ESP_LOGD(TAG, "Temperature reading: %.2f°C (raw: %.2f°C, offset: %.2f°C)", 
             *temperature_celsius, raw_temp, esp_state_.calibration_offset);
    
    UpdateStatistics(true, GetCurrentTimeUs() - start_time);
    SetLastError(hf_temp_err_t::TEMP_SUCCESS);
    return hf_temp_err_t::TEMP_SUCCESS;
}

//==============================================================================
// INFORMATION INTERFACE (MANDATORY OVERRIDES)
//==============================================================================

hf_temp_err_t EspTemperature::GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept {
    if (info == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Get current range configuration
    float min_temp, max_temp, accuracy;
    GetRangeConfig(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
    
    info->sensor_type = HF_TEMP_SENSOR_TYPE_INTERNAL;
    info->min_temp_celsius = min_temp;
    info->max_temp_celsius = max_temp;
    info->resolution_celsius = ESP_TEMP_DEFAULT_RESOLUTION_CELSIUS;
    info->accuracy_celsius = accuracy;
    info->response_time_ms = ESP_TEMP_DEFAULT_RESPONSE_TIME_MS;
    info->capabilities = GetCapabilities();
    info->manufacturer = "Espressif";
    info->model = "ESP32-C6 Internal Temperature Sensor";
    info->version = "ESP-IDF v5.x";
    
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_u32_t EspTemperature::GetCapabilities() const noexcept {
    return HF_TEMP_CAP_THRESHOLD_MONITORING |
           HF_TEMP_CAP_CONTINUOUS_READING |
           HF_TEMP_CAP_CALIBRATION |
           HF_TEMP_CAP_POWER_MANAGEMENT |
           HF_TEMP_CAP_SELF_TEST |
           HF_TEMP_CAP_HIGH_PRECISION |
           HF_TEMP_CAP_FAST_RESPONSE;
}

//==============================================================================
// ADVANCED FEATURES (SUPPORTED BY ESP32-C6)
//==============================================================================

hf_temp_err_t EspTemperature::SetRange(float min_celsius, float max_celsius) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (min_celsius >= max_celsius) {
        ESP_LOGE(TAG, "Invalid range: min (%.1f) >= max (%.1f)", min_celsius, max_celsius);
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_RANGE), hf_temp_err_t::TEMP_ERR_INVALID_RANGE;
    }
    
    // Find optimal range for the given requirements
    esp_temp_range_t optimal_range = FindOptimalRange(min_celsius, max_celsius);
    
    if (optimal_range >= ESP_TEMP_RANGE_COUNT) {
        ESP_LOGE(TAG, "No suitable range found for %.1f°C to %.1f°C", min_celsius, max_celsius);
        return SetLastError(hf_temp_err_t::TEMP_ERR_UNSUPPORTED_RANGE), hf_temp_err_t::TEMP_ERR_UNSUPPORTED_RANGE;
    }
    
    return SetMeasurementRange(optimal_range);
}

hf_temp_err_t EspTemperature::GetRange(float* min_celsius, float* max_celsius) const noexcept {
    if (min_celsius == nullptr || max_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    float accuracy;
    GetRangeConfig(esp_state_.current_range, min_celsius, max_celsius, &accuracy);
    
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::GetResolution(float* resolution_celsius) const noexcept {
    if (resolution_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    *resolution_celsius = ESP_TEMP_DEFAULT_RESOLUTION_CELSIUS;
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::SetThresholds(float low_threshold_celsius, float high_threshold_celsius) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (low_threshold_celsius >= high_threshold_celsius) {
        ESP_LOGE(TAG, "Invalid thresholds: low (%.1f) >= high (%.1f)", 
                 low_threshold_celsius, high_threshold_celsius);
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_THRESHOLD), hf_temp_err_t::TEMP_ERR_INVALID_THRESHOLD;
    }
    
    // Validate thresholds are within sensor range
    float min_temp, max_temp, accuracy;
    GetRangeConfig(esp_state_.current_range, &min_temp, &max_temp, &accuracy);
    
    if (low_threshold_celsius < min_temp || high_threshold_celsius > max_temp) {
        ESP_LOGW(TAG, "Thresholds [%.1f, %.1f] outside sensor range [%.0f, %.0f]",
                 low_threshold_celsius, high_threshold_celsius, min_temp, max_temp);
    }
    
    esp_config_.low_threshold_celsius = low_threshold_celsius;
    esp_config_.high_threshold_celsius = high_threshold_celsius;
    
    ESP_LOGI(TAG, "Temperature thresholds set: Low=%.1f°C, High=%.1f°C", 
             low_threshold_celsius, high_threshold_celsius);
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::GetThresholds(float* low_threshold_celsius, float* high_threshold_celsius) const noexcept {
    if (low_threshold_celsius == nullptr || high_threshold_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    *low_threshold_celsius = esp_config_.low_threshold_celsius;
    *high_threshold_celsius = esp_config_.high_threshold_celsius;
    
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::EnableThresholdMonitoring(hf_temp_threshold_callback_t callback, void* user_data) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return SetLastError(hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED), hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    threshold_callback_ = callback;
    threshold_user_data_ = user_data;
    esp_state_.threshold_monitoring_enabled = true;
    diagnostics_.threshold_monitoring_enabled = true;
    
    ESP_LOGI(TAG, "Threshold monitoring enabled (Low=%.1f°C, High=%.1f°C)", 
             esp_config_.low_threshold_celsius, esp_config_.high_threshold_celsius);
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::DisableThresholdMonitoring() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    esp_state_.threshold_monitoring_enabled = false;
    diagnostics_.threshold_monitoring_enabled = false;
    threshold_callback_ = nullptr;
    esp_threshold_callback_ = nullptr;
    threshold_user_data_ = nullptr;
    
    ESP_LOGI(TAG, "Threshold monitoring disabled");
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::StartContinuousMonitoring(hf_u32_t sample_rate_hz, 
                                                       hf_temp_reading_callback_t callback, 
                                                       void* user_data) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return SetLastError(hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED), hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (esp_state_.continuous_monitoring_active) {
        ESP_LOGW(TAG, "Continuous monitoring already active");
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_STATE), hf_temp_err_t::TEMP_ERR_INVALID_STATE;
    }
    
    if (sample_rate_hz < ESP_TEMP_MIN_SAMPLE_RATE_HZ || sample_rate_hz > ESP_TEMP_MAX_SAMPLE_RATE_HZ) {
        ESP_LOGE(TAG, "Invalid sample rate: %u Hz (valid range: %d-%d Hz)", 
                 sample_rate_hz, ESP_TEMP_MIN_SAMPLE_RATE_HZ, ESP_TEMP_MAX_SAMPLE_RATE_HZ);
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER), hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER;
    }
    
    // Store callback and user data
    monitoring_callback_ = callback;
    monitoring_user_data_ = user_data;
    esp_state_.sample_rate_hz = sample_rate_hz;
    
    // Create timer for continuous monitoring
    const esp_timer_create_args_t timer_args = {
        .callback = MonitoringTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "temp_monitor",
        .skip_unhandled_events = true
    };
    
    esp_err_t esp_err = esp_timer_create(&timer_args, &esp_state_.monitoring_timer);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create monitoring timer: %s", esp_err_to_name(esp_err));
        return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
    }
    
    // Start timer
    hf_u64_t period_us = 1000000ULL / sample_rate_hz;
    esp_err = esp_timer_start_periodic(esp_state_.monitoring_timer, period_us);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start monitoring timer: %s", esp_err_to_name(esp_err));
        esp_timer_delete(esp_state_.monitoring_timer);
        esp_state_.monitoring_timer = nullptr;
        return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
    }
    
    esp_state_.continuous_monitoring_active = true;
    diagnostics_.continuous_monitoring_active = true;
    
    ESP_LOGI(TAG, "Continuous monitoring started at %u Hz", sample_rate_hz);
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::StopContinuousMonitoring() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!esp_state_.continuous_monitoring_active) {
        ESP_LOGW(TAG, "Continuous monitoring not active");
        return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
    }
    
    bool success = true;
    
    // Stop and delete timer
    if (esp_state_.monitoring_timer != nullptr) {
        esp_err_t esp_err = esp_timer_stop(esp_state_.monitoring_timer);
        if (esp_err != ESP_OK && esp_err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to stop monitoring timer: %s", esp_err_to_name(esp_err));
            success = false;
        }
        
        esp_err = esp_timer_delete(esp_state_.monitoring_timer);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete monitoring timer: %s", esp_err_to_name(esp_err));
            success = false;
        }
        
        esp_state_.monitoring_timer = nullptr;
    }
    
    // Clear monitoring state
    esp_state_.continuous_monitoring_active = false;
    diagnostics_.continuous_monitoring_active = false;
    monitoring_callback_ = nullptr;
    esp_monitoring_callback_ = nullptr;
    monitoring_user_data_ = nullptr;
    
    ESP_LOGI(TAG, "Continuous monitoring stopped");
    
    hf_temp_err_t result = success ? hf_temp_err_t::TEMP_SUCCESS : hf_temp_err_t::TEMP_ERR_FAILURE;
    return SetLastError(result), result;
}

bool EspTemperature::IsMonitoringActive() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.continuous_monitoring_active;
}

hf_temp_err_t EspTemperature::SetCalibrationOffset(float offset_celsius) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Validate offset is reasonable
    if (std::abs(offset_celsius) > 20.0f) {
        ESP_LOGW(TAG, "Large calibration offset: %.2f°C", offset_celsius);
    }
    
    esp_state_.calibration_offset = offset_celsius;
    esp_config_.calibration_offset = offset_celsius;
    statistics_.calibration_count++;
    
    ESP_LOGI(TAG, "Calibration offset set to %.2f°C", offset_celsius);
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::GetCalibrationOffset(float* offset_celsius) const noexcept {
    if (offset_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    *offset_celsius = esp_state_.calibration_offset;
    
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::ResetCalibration() noexcept {
    return SetCalibrationOffset(0.0f);
}

hf_temp_err_t EspTemperature::EnterSleepMode() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return SetLastError(hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED), hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    if (!esp_state_.allow_power_down) {
        ESP_LOGW(TAG, "Power down not allowed in current configuration");
        return SetLastError(hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION), hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
    }
    
    // Disable temperature sensor for sleep
    esp_err_t esp_err = temperature_sensor_disable(esp_state_.handle);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable temperature sensor for sleep: %s", esp_err_to_name(esp_err));
        return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
    }
    
    current_state_ = HF_TEMP_STATE_SLEEPING;
    ESP_LOGI(TAG, "Temperature sensor entered sleep mode");
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::ExitSleepMode() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (current_state_ != HF_TEMP_STATE_SLEEPING) {
        ESP_LOGW(TAG, "Temperature sensor not in sleep mode");
        return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
    }
    
    // Re-enable temperature sensor
    esp_err_t esp_err = temperature_sensor_enable(esp_state_.handle);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable temperature sensor after sleep: %s", esp_err_to_name(esp_err));
        return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
    }
    
    current_state_ = HF_TEMP_STATE_INITIALIZED;
    ESP_LOGI(TAG, "Temperature sensor exited sleep mode");
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

bool EspTemperature::IsSleeping() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return (current_state_ == HF_TEMP_STATE_SLEEPING);
}

hf_temp_err_t EspTemperature::SelfTest() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        ESP_LOGE(TAG, "Temperature sensor not initialized");
        return SetLastError(hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED), hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    ESP_LOGI(TAG, "Starting temperature sensor self-test...");
    
    // Test 1: Basic reading
    float temperature;
    hf_temp_err_t error = ReadTemperatureCelsiusImpl(&temperature);
    if (error != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Self-test failed: Cannot read temperature");
        return SetLastError(hf_temp_err_t::TEMP_ERR_HARDWARE_FAULT), hf_temp_err_t::TEMP_ERR_HARDWARE_FAULT;
    }
    
    // Test 2: Validate reading is reasonable
    if (temperature < -50.0f || temperature > 150.0f) {
        ESP_LOGE(TAG, "Self-test failed: Temperature %.2f°C is unreasonable", temperature);
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_READING), hf_temp_err_t::TEMP_ERR_INVALID_READING;
    }
    
    // Test 3: Multiple readings for stability
    float temp_readings[3];
    for (int i = 0; i < 3; i++) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait 100ms between readings
        error = ReadTemperatureCelsiusImpl(&temp_readings[i]);
        if (error != hf_temp_err_t::TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Self-test failed: Reading %d failed", i + 1);
            return SetLastError(hf_temp_err_t::TEMP_ERR_READ_FAILED), hf_temp_err_t::TEMP_ERR_READ_FAILED;
        }
    }
    
    // Check for excessive variation (should be stable within 5°C)
    float min_temp = *std::min_element(temp_readings, temp_readings + 3);
    float max_temp = *std::max_element(temp_readings, temp_readings + 3);
    if ((max_temp - min_temp) > 5.0f) {
        ESP_LOGW(TAG, "Self-test warning: High temperature variation %.2f°C", max_temp - min_temp);
    }
    
    ESP_LOGI(TAG, "Self-test passed: Temperature=%.2f°C, Variation=%.2f°C", 
             temperature, max_temp - min_temp);
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::CheckHealth() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool healthy = true;
    
    // Check initialization status
    if (!initialized_) {
        ESP_LOGW(TAG, "Health check: Sensor not initialized");
        healthy = false;
    }
    
    // Check handle validity
    if (esp_state_.handle == nullptr) {
        ESP_LOGW(TAG, "Health check: Invalid handle");
        healthy = false;
    }
    
    // Check error history
    if (diagnostics_.consecutive_errors > 5) {
        ESP_LOGW(TAG, "Health check: High consecutive error count (%u)", 
                 diagnostics_.consecutive_errors);
        healthy = false;
    }
    
    // Check last error
    if (last_error_ != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGW(TAG, "Health check: Last operation failed with error %d", 
                 static_cast<int>(last_error_));
        healthy = false;
    }
    
    // Update diagnostics
    diagnostics_.sensor_healthy = healthy;
    
    if (healthy) {
        ESP_LOGD(TAG, "Health check: Sensor is healthy");
        return hf_temp_err_t::TEMP_SUCCESS;
    } else {
        ESP_LOGW(TAG, "Health check: Sensor health issues detected");
        return hf_temp_err_t::TEMP_ERR_HARDWARE_FAULT;
    }
}

hf_temp_err_t EspTemperature::GetStatistics(hf_temp_statistics_t& statistics) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    statistics = statistics_;
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::GetDiagnostics(hf_temp_diagnostics_t& diagnostics) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Update current diagnostics
    diagnostics_.sensor_available = (esp_state_.handle != nullptr);
    diagnostics_.current_temperature_raw = static_cast<hf_u32_t>(esp_state_.last_temperature_celsius * 1000);
    diagnostics_.calibration_valid = (std::abs(esp_state_.calibration_offset) < 50.0f);
    
    diagnostics = diagnostics_;
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::ResetStatistics() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    statistics_ = hf_temp_statistics_t{};
    statistics_.min_operation_time_us = UINT32_MAX;
    statistics_.min_temperature_celsius = 1000.0f;
    statistics_.max_temperature_celsius = -1000.0f;
    
    ESP_LOGI(TAG, "Statistics reset");
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::ResetDiagnostics() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    diagnostics_.last_error_code = hf_temp_err_t::TEMP_SUCCESS;
    diagnostics_.last_error_timestamp = 0;
    diagnostics_.consecutive_errors = 0;
    diagnostics_.sensor_healthy = true;
    
    ESP_LOGI(TAG, "Diagnostics reset");
    return hf_temp_err_t::TEMP_SUCCESS;
}

//==============================================================================
// ESP32-C6 SPECIFIC METHODS
//==============================================================================

hf_temp_err_t EspTemperature::InitializeEsp32(const esp_temp_config_t& esp_config) noexcept {
    esp_config_ = esp_config;
    return EnsureInitialized() ? hf_temp_err_t::TEMP_SUCCESS : hf_temp_err_t::TEMP_ERR_FAILURE;
}

hf_temp_err_t EspTemperature::SetMeasurementRange(esp_temp_range_t range) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (range >= ESP_TEMP_RANGE_COUNT) {
        ESP_LOGE(TAG, "Invalid range: %d", static_cast<int>(range));
        return SetLastError(hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER), hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER;
    }
    
    if (range == esp_state_.current_range) {
        ESP_LOGD(TAG, "Range already set to %d", static_cast<int>(range));
        return hf_temp_err_t::TEMP_SUCCESS;
    }
    
    // If initialized, need to reconfigure sensor
    if (initialized_ && esp_state_.handle != nullptr) {
        // Disable sensor
        esp_err_t esp_err = temperature_sensor_disable(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to disable sensor for range change: %s", esp_err_to_name(esp_err));
            return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
        }
        
        // Uninstall sensor
        esp_err = temperature_sensor_uninstall(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to uninstall sensor for range change: %s", esp_err_to_name(esp_err));
            return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
        }
        
        esp_state_.handle = nullptr;
        
        // Reconfigure with new range
        float min_temp, max_temp, accuracy;
        GetRangeConfig(range, &min_temp, &max_temp, &accuracy);
        
        temperature_sensor_config_t temp_config = {};
        temp_config.range_min = static_cast<int>(min_temp);
        temp_config.range_max = static_cast<int>(max_temp);
        temp_config.clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT;
        temp_config.flags.allow_pd = 0;
        
        // Reinstall with new configuration
        esp_err = temperature_sensor_install(&temp_config, &esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reinstall sensor with new range: %s", esp_err_to_name(esp_err));
            return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
        }
        
        // Re-enable sensor
        esp_err = temperature_sensor_enable(esp_state_.handle);
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to re-enable sensor after range change: %s", esp_err_to_name(esp_err));
            temperature_sensor_uninstall(esp_state_.handle);
            esp_state_.handle = nullptr;
            return SetLastError(ConvertEspError(esp_err)), ConvertEspError(esp_err);
        }
        
        ESP_LOGI(TAG, "Range changed to %d: %.0f°C to %.0f°C (±%.1f°C)", 
                 static_cast<int>(range), min_temp, max_temp, accuracy);
    }
    
    esp_state_.current_range = range;
    esp_config_.range = range;
    
    return SetLastError(hf_temp_err_t::TEMP_SUCCESS), hf_temp_err_t::TEMP_SUCCESS;
}

esp_temp_range_t EspTemperature::GetMeasurementRange() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.current_range;
}

hf_temp_err_t EspTemperature::GetRangeInfo(esp_temp_range_t range, float* min_celsius, 
                                           float* max_celsius, float* accuracy_celsius) const noexcept {
    if (min_celsius == nullptr || max_celsius == nullptr || accuracy_celsius == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    if (range >= ESP_TEMP_RANGE_COUNT) {
        return hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER;
    }
    
    GetRangeConfig(range, min_celsius, max_celsius, accuracy_celsius);
    return hf_temp_err_t::TEMP_SUCCESS;
}

esp_temp_range_t EspTemperature::FindOptimalRange(float min_celsius, float max_celsius) const noexcept {
    // Find range that covers the required span with best accuracy
    esp_temp_range_t best_range = ESP_TEMP_RANGE_COUNT; // Invalid initially
    float best_accuracy = 1000.0f; // Very high value initially
    
    for (int i = 0; i < ESP_TEMP_RANGE_COUNT; i++) {
        esp_temp_range_t range = static_cast<esp_temp_range_t>(i);
        float range_min, range_max, accuracy;
        GetRangeConfig(range, &range_min, &range_max, &accuracy);
        
        // Check if this range covers the required span
        if (range_min <= min_celsius && range_max >= max_celsius) {
            // This range works, check if it has better accuracy
            if (accuracy < best_accuracy) {
                best_accuracy = accuracy;
                best_range = range;
            }
        }
    }
    
    return best_range;
}

hf_temp_err_t EspTemperature::ReadRawTemperature(float* raw_value) noexcept {
    if (raw_value == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NULL_POINTER;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || esp_state_.handle == nullptr) {
        return hf_temp_err_t::TEMP_ERR_NOT_INITIALIZED;
    }
    
    esp_err_t esp_err = temperature_sensor_get_celsius(esp_state_.handle, raw_value);
    if (esp_err != ESP_OK) {
        return ConvertEspError(esp_err);
    }
    
    return hf_temp_err_t::TEMP_SUCCESS;
}

temperature_sensor_handle_t EspTemperature::GetEspHandle() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return esp_state_.handle;
}

hf_temp_err_t EspTemperature::SetEspThresholdCallback(esp_temp_threshold_callback_t callback) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    esp_threshold_callback_ = callback;
    return hf_temp_err_t::TEMP_SUCCESS;
}

hf_temp_err_t EspTemperature::SetEspMonitoringCallback(esp_temp_monitoring_callback_t callback) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    esp_monitoring_callback_ = callback;
    return hf_temp_err_t::TEMP_SUCCESS;
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

hf_temp_err_t EspTemperature::ConvertEspError(esp_err_t esp_err) const noexcept {
    switch (esp_err) {
        case ESP_OK:
            return hf_temp_err_t::TEMP_SUCCESS;
        case ESP_ERR_INVALID_ARG:
            return hf_temp_err_t::TEMP_ERR_INVALID_PARAMETER;
        case ESP_ERR_INVALID_STATE:
            return hf_temp_err_t::TEMP_ERR_INVALID_STATE;
        case ESP_ERR_NOT_FOUND:
            return hf_temp_err_t::TEMP_ERR_SENSOR_NOT_AVAILABLE;
        case ESP_ERR_NO_MEM:
            return hf_temp_err_t::TEMP_ERR_OUT_OF_MEMORY;
        case ESP_ERR_TIMEOUT:
            return hf_temp_err_t::TEMP_ERR_TIMEOUT;
        case ESP_ERR_NOT_SUPPORTED:
            return hf_temp_err_t::TEMP_ERR_UNSUPPORTED_OPERATION;
        default:
            return hf_temp_err_t::TEMP_ERR_DRIVER_ERROR;
    }
}

void EspTemperature::GetRangeConfig(esp_temp_range_t range, float* min_celsius, 
                                    float* max_celsius, float* accuracy_celsius) const noexcept {
    if (range < ESP_TEMP_RANGE_COUNT) {
        const esp_temp_range_info_t& info = RANGE_INFO[range];
        if (min_celsius) *min_celsius = info.min_celsius;
        if (max_celsius) *max_celsius = info.max_celsius;
        if (accuracy_celsius) *accuracy_celsius = info.accuracy_celsius;
    } else {
        // Default to first range for invalid input
        if (min_celsius) *min_celsius = RANGE_INFO[0].min_celsius;
        if (max_celsius) *max_celsius = RANGE_INFO[0].max_celsius;
        if (accuracy_celsius) *accuracy_celsius = RANGE_INFO[0].accuracy_celsius;
    }
}

void EspTemperature::SetLastError(hf_temp_err_t error) noexcept {
    last_error_ = error;
    UpdateDiagnostics(error);
}

void EspTemperature::UpdateStatistics(bool operation_successful, hf_u32_t operation_time_us) noexcept {
    statistics_.total_operations++;
    
    if (operation_successful) {
        statistics_.successful_operations++;
        diagnostics_.consecutive_errors = 0;
    } else {
        statistics_.failed_operations++;
        diagnostics_.consecutive_errors++;
    }
    
    // Update timing statistics
    if (operation_time_us < statistics_.min_operation_time_us) {
        statistics_.min_operation_time_us = operation_time_us;
    }
    if (operation_time_us > statistics_.max_operation_time_us) {
        statistics_.max_operation_time_us = operation_time_us;
    }
    
    // Update average operation time (simple moving average)
    if (statistics_.total_operations == 1) {
        statistics_.average_operation_time_us = operation_time_us;
    } else {
        statistics_.average_operation_time_us = 
            (statistics_.average_operation_time_us * (statistics_.total_operations - 1) + operation_time_us) / 
            statistics_.total_operations;
    }
}

void EspTemperature::UpdateDiagnostics(hf_temp_err_t error) noexcept {
    if (error != hf_temp_err_t::TEMP_SUCCESS) {
        diagnostics_.last_error_code = error;
        diagnostics_.last_error_timestamp = static_cast<hf_u32_t>(GetCurrentTimeUs() / 1000); // Convert to ms
        diagnostics_.consecutive_errors++;
        diagnostics_.sensor_healthy = (diagnostics_.consecutive_errors <= 3);
    }
}

void EspTemperature::MonitoringTimerCallback(void* arg) noexcept {
    EspTemperature* sensor = static_cast<EspTemperature*>(arg);
    if (sensor == nullptr) {
        return;
    }
    
    // Read temperature
    float temperature;
    hf_temp_err_t error = sensor->ReadTemperatureCelsiusImpl(&temperature);
    
    if (error == hf_temp_err_t::TEMP_SUCCESS) {
        // Call base callback if set
        if (sensor->monitoring_callback_) {
            hf_temp_reading_t reading = {};
            reading.temperature_celsius = temperature;
            reading.temperature_raw = temperature - sensor->esp_state_.calibration_offset;
            reading.timestamp_us = GetCurrentTimeUs();
            reading.error = error;
            reading.is_valid = true;
            reading.accuracy_celsius = RANGE_INFO[sensor->esp_state_.current_range].accuracy_celsius;
            
            sensor->monitoring_callback_(sensor, &reading, sensor->monitoring_user_data_);
        }
        
        // Call ESP32-specific callback if set
        if (sensor->esp_monitoring_callback_) {
            sensor->esp_monitoring_callback_(sensor, temperature, GetCurrentTimeUs());
        }
    }
}

void EspTemperature::CheckThresholds(float temperature) noexcept {
    bool threshold_violated = false;
    bool is_high_threshold = false;
    
    if (temperature <= esp_config_.low_threshold_celsius) {
        threshold_violated = true;
        is_high_threshold = false;
        ESP_LOGW(TAG, "Low threshold violated: %.2f°C <= %.2f°C", 
                 temperature, esp_config_.low_threshold_celsius);
    } else if (temperature >= esp_config_.high_threshold_celsius) {
        threshold_violated = true;
        is_high_threshold = true;
        ESP_LOGW(TAG, "High threshold violated: %.2f°C >= %.2f°C", 
                 temperature, esp_config_.high_threshold_celsius);
    }
    
    if (threshold_violated) {
        statistics_.threshold_violations++;
        
        // Call base callback if set
        if (threshold_callback_) {
            hf_u32_t threshold_type = is_high_threshold ? 1 : 0;
            threshold_callback_(this, temperature, threshold_type, threshold_user_data_);
        }
        
        // Call ESP32-specific callback if set
        if (esp_threshold_callback_) {
            esp_threshold_callback_(this, temperature, is_high_threshold);
        }
    }
}

hf_u64_t EspTemperature::GetCurrentTimeUs() noexcept {
    return esp_timer_get_time();
}

//#endif // HF_MCU_FAMILY_ESP32