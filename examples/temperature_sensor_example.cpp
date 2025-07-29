/**
 * @file temperature_sensor_example.cpp
 * @brief Example demonstrating ESP32-C6 temperature sensor usage with HardFOC.
 *
 * This example shows how to use the BaseTemperature and EspTemperature classes
 * to read the internal chip temperature on ESP32-C6. It demonstrates:
 * - Basic temperature reading
 * - Continuous monitoring with callbacks
 * - Threshold monitoring
 * - Calibration
 * - Error handling
 * - Different temperature units
 * - Power management
 * - Self-test functionality
 *
 * @author HardFOC Development Team
 * @date 2025
 * @copyright HardFOC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ESP-IDF includes
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"
}

// HardFOC Temperature includes
#include "BaseTemperature.h"
#include "EspTemperature.h"

using namespace HardFOC;

//--------------------------------------
//  Example Configuration
//--------------------------------------

static const char* TAG = "TempExample";

// Global temperature sensor instance
static EspTemperature* temp_sensor = nullptr;

// Example state
struct ExampleState {
    uint32_t reading_count;
    float min_temperature;
    float max_temperature;
    float avg_temperature;
    bool threshold_exceeded;
    uint32_t threshold_count;
} example_state = {0};

//--------------------------------------
//  Callback Functions
//--------------------------------------

/**
 * @brief Callback for continuous temperature monitoring
 */
void temperature_reading_callback(const HfTempReading_t* reading, void* user_data) {
    if (reading == nullptr) {
        return;
    }
    
    ExampleState* state = static_cast<ExampleState*>(user_data);
    
    if (reading->is_valid) {
        float temp = reading->temperature_celsius;
        
        // Update statistics
        state->reading_count++;
        
        if (state->reading_count == 1) {
            state->min_temperature = temp;
            state->max_temperature = temp;
            state->avg_temperature = temp;
        } else {
            if (temp < state->min_temperature) state->min_temperature = temp;
            if (temp > state->max_temperature) state->max_temperature = temp;
            
            // Running average
            state->avg_temperature = (state->avg_temperature * (state->reading_count - 1) + temp) / state->reading_count;
        }
        
        ESP_LOGI(TAG, "Continuous reading #%lu: %.2f°C (accuracy: ±%.1f°C)", 
                 state->reading_count, temp, reading->accuracy_celsius);
        ESP_LOGI(TAG, "Statistics - Min: %.2f°C, Max: %.2f°C, Avg: %.2f°C", 
                 state->min_temperature, state->max_temperature, state->avg_temperature);
    } else {
        ESP_LOGE(TAG, "Invalid temperature reading: %s", hf_temp_get_error_string(reading->error));
    }
}

/**
 * @brief Callback for temperature threshold events
 */
void temperature_threshold_callback(float temperature_celsius, uint32_t threshold_type, void* user_data) {
    ExampleState* state = static_cast<ExampleState*>(user_data);
    
    state->threshold_exceeded = true;
    state->threshold_count++;
    
    const char* threshold_name = (threshold_type == 0) ? "LOW" : "HIGH";
    
    ESP_LOGW(TAG, "THRESHOLD EXCEEDED! %s threshold reached: %.2f°C (count: %lu)", 
             threshold_name, temperature_celsius, state->threshold_count);
}

/**
 * @brief Callback for temperature sensor errors
 */
void temperature_error_callback(HfTempError_t error, const char* error_description, void* user_data) {
    ESP_LOGE(TAG, "Temperature sensor error: %s (%d)", error_description, error);
}

//--------------------------------------
//  Example Functions
//--------------------------------------

/**
 * @brief Demonstrate basic temperature reading
 */
void example_basic_reading() {
    ESP_LOGI(TAG, "=== Basic Temperature Reading Example ===");
    
    // Read temperature in different units
    float celsius, fahrenheit, kelvin;
    HfTempError_t error;
    
    // Read in Celsius
    error = temp_sensor->read_celsius(&celsius);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Temperature: %.2f°C", celsius);
    } else {
        ESP_LOGE(TAG, "Failed to read Celsius: %s", hf_temp_get_error_string(error));
        return;
    }
    
    // Read in Fahrenheit
    error = temp_sensor->read_fahrenheit(&fahrenheit);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Temperature: %.2f°F", fahrenheit);
    } else {
        ESP_LOGE(TAG, "Failed to read Fahrenheit: %s", hf_temp_get_error_string(error));
    }
    
    // Read in Kelvin
    error = temp_sensor->read_kelvin(&kelvin);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Temperature: %.2fK", kelvin);
    } else {
        ESP_LOGE(TAG, "Failed to read Kelvin: %s", hf_temp_get_error_string(error));
    }
    
    // Read full temperature information
    HfTempReading_t reading;
    error = temp_sensor->read_temperature(&reading);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Full reading:");
        ESP_LOGI(TAG, "  Temperature: %.2f°C", reading.temperature_celsius);
        ESP_LOGI(TAG, "  Raw value: %.2f", reading.temperature_raw);
        ESP_LOGI(TAG, "  Accuracy: ±%.1f°C", reading.accuracy_celsius);
        ESP_LOGI(TAG, "  Timestamp: %llu μs", reading.timestamp_us);
        ESP_LOGI(TAG, "  Valid: %s", reading.is_valid ? "Yes" : "No");
    } else {
        ESP_LOGE(TAG, "Failed to read full temperature: %s", hf_temp_get_error_string(error));
    }
}

/**
 * @brief Demonstrate sensor information and capabilities
 */
void example_sensor_info() {
    ESP_LOGI(TAG, "=== Sensor Information Example ===");
    
    // Get sensor information
    HfTempSensorInfo_t info;
    HfTempError_t error = temp_sensor->get_sensor_info(&info);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Sensor Information:");
        ESP_LOGI(TAG, "  Manufacturer: %s", info.manufacturer);
        ESP_LOGI(TAG, "  Model: %s", info.model);
        ESP_LOGI(TAG, "  Version: %s", info.version);
        ESP_LOGI(TAG, "  Type: %d", info.sensor_type);
        ESP_LOGI(TAG, "  Range: %.1f°C to %.1f°C", info.min_temp_celsius, info.max_temp_celsius);
        ESP_LOGI(TAG, "  Resolution: %.3f°C", info.resolution_celsius);
        ESP_LOGI(TAG, "  Accuracy: ±%.1f°C", info.accuracy_celsius);
        ESP_LOGI(TAG, "  Response time: %lu ms", info.response_time_ms);
        ESP_LOGI(TAG, "  Capabilities: 0x%08lX", info.capabilities);
    } else {
        ESP_LOGE(TAG, "Failed to get sensor info: %s", hf_temp_get_error_string(error));
    }
    
    // Check specific capabilities
    ESP_LOGI(TAG, "Capability check:");
    ESP_LOGI(TAG, "  Threshold monitoring: %s", 
             temp_sensor->has_capability(TEMP_CAP_THRESHOLD_MONITORING) ? "Yes" : "No");
    ESP_LOGI(TAG, "  Continuous reading: %s", 
             temp_sensor->has_capability(TEMP_CAP_CONTINUOUS_READING) ? "Yes" : "No");
    ESP_LOGI(TAG, "  Calibration: %s", 
             temp_sensor->has_capability(TEMP_CAP_CALIBRATION) ? "Yes" : "No");
    ESP_LOGI(TAG, "  Power management: %s", 
             temp_sensor->has_capability(TEMP_CAP_POWER_MANAGEMENT) ? "Yes" : "No");
    ESP_LOGI(TAG, "  Self-test: %s", 
             temp_sensor->has_capability(TEMP_CAP_SELF_TEST) ? "Yes" : "No");
    ESP_LOGI(TAG, "  High precision: %s", 
             temp_sensor->has_capability(TEMP_CAP_HIGH_PRECISION) ? "Yes" : "No");
}

/**
 * @brief Demonstrate ESP32-specific range configuration
 */
void example_esp32_ranges() {
    ESP_LOGI(TAG, "=== ESP32 Range Configuration Example ===");
    
    // Display available ranges
    EspTempRange_t ranges[] = {
        ESP_TEMP_RANGE_NEG40_20,
        ESP_TEMP_RANGE_NEG30_50,
        ESP_TEMP_RANGE_NEG10_80,
        ESP_TEMP_RANGE_20_100,
        ESP_TEMP_RANGE_50_125
    };
    
    const char* range_names[] = {
        "ESP_TEMP_RANGE_NEG40_20",
        "ESP_TEMP_RANGE_NEG30_50", 
        "ESP_TEMP_RANGE_NEG10_80",
        "ESP_TEMP_RANGE_20_100",
        "ESP_TEMP_RANGE_50_125"
    };
    
    ESP_LOGI(TAG, "Available ESP32-C6 temperature ranges:");
    for (int i = 0; i < 5; i++) {
        float min_temp, max_temp, accuracy;
        HfTempError_t error = temp_sensor->get_range_info(ranges[i], &min_temp, &max_temp, &accuracy);
        if (error == TEMP_SUCCESS) {
            ESP_LOGI(TAG, "  %s: %.0f°C to %.0f°C (±%.1f°C)", 
                     range_names[i], min_temp, max_temp, accuracy);
        }
    }
    
    // Get current range
    EspTempRange_t current_range = temp_sensor->get_measurement_range();
    ESP_LOGI(TAG, "Current range: %d", current_range);
    
    // Test optimal range finding
    float test_ranges[][2] = {
        {0.0f, 50.0f},
        {-20.0f, 60.0f},
        {25.0f, 75.0f},
        {-35.0f, 40.0f}
    };
    
    ESP_LOGI(TAG, "Optimal range suggestions:");
    for (int i = 0; i < 4; i++) {
        EspTempRange_t optimal = temp_sensor->find_optimal_range(test_ranges[i][0], test_ranges[i][1]);
        ESP_LOGI(TAG, "  Range [%.1f, %.1f]°C -> %d", 
                 test_ranges[i][0], test_ranges[i][1], optimal);
    }
}

/**
 * @brief Demonstrate threshold monitoring
 */
void example_threshold_monitoring() {
    ESP_LOGI(TAG, "=== Threshold Monitoring Example ===");
    
    // Reset threshold state
    example_state.threshold_exceeded = false;
    example_state.threshold_count = 0;
    
    // Set thresholds (use current temperature ±2°C for demonstration)
    float current_temp;
    HfTempError_t error = temp_sensor->read_celsius(&current_temp);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read current temperature");
        return;
    }
    
    float low_threshold = current_temp - 2.0f;
    float high_threshold = current_temp + 2.0f;
    
    error = temp_sensor->set_thresholds(low_threshold, high_threshold);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set thresholds: %s", hf_temp_get_error_string(error));
        return;
    }
    
    ESP_LOGI(TAG, "Thresholds set: %.1f°C (low) to %.1f°C (high)", low_threshold, high_threshold);
    
    // Enable threshold monitoring
    error = temp_sensor->enable_threshold_monitoring(temperature_threshold_callback, &example_state);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable threshold monitoring: %s", hf_temp_get_error_string(error));
        return;
    }
    
    ESP_LOGI(TAG, "Threshold monitoring enabled. Reading temperatures for 10 seconds...");
    
    // Read temperatures for 10 seconds to potentially trigger thresholds
    uint64_t start_time = esp_timer_get_time();
    uint64_t duration_us = 10 * 1000000ULL; // 10 seconds
    
    while ((esp_timer_get_time() - start_time) < duration_us) {
        float temp;
        error = temp_sensor->read_celsius(&temp);
        if (error == TEMP_SUCCESS) {
            ESP_LOGI(TAG, "Temperature: %.2f°C (thresholds: %.1f - %.1f°C)", 
                     temp, low_threshold, high_threshold);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
    }
    
    // Disable threshold monitoring
    temp_sensor->disable_threshold_monitoring();
    
    ESP_LOGI(TAG, "Threshold monitoring test completed. Thresholds exceeded: %lu times", 
             example_state.threshold_count);
}

/**
 * @brief Demonstrate continuous monitoring
 */
void example_continuous_monitoring() {
    ESP_LOGI(TAG, "=== Continuous Monitoring Example ===");
    
    // Reset statistics
    memset(&example_state, 0, sizeof(example_state));
    
    // Start continuous monitoring at 2 Hz
    uint32_t sample_rate = 2; // Hz
    HfTempError_t error = temp_sensor->start_continuous_monitoring(sample_rate, 
                                                                  temperature_reading_callback, 
                                                                  &example_state);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start continuous monitoring: %s", hf_temp_get_error_string(error));
        return;
    }
    
    ESP_LOGI(TAG, "Continuous monitoring started at %lu Hz for 15 seconds...", sample_rate);
    
    // Let it run for 15 seconds
    vTaskDelay(pdMS_TO_TICKS(15000));
    
    // Stop continuous monitoring
    error = temp_sensor->stop_continuous_monitoring();
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to stop continuous monitoring: %s", hf_temp_get_error_string(error));
    } else {
        ESP_LOGI(TAG, "Continuous monitoring stopped");
    }
    
    ESP_LOGI(TAG, "Final statistics:");
    ESP_LOGI(TAG, "  Total readings: %lu", example_state.reading_count);
    ESP_LOGI(TAG, "  Temperature range: %.2f°C to %.2f°C", 
             example_state.min_temperature, example_state.max_temperature);
    ESP_LOGI(TAG, "  Average temperature: %.2f°C", example_state.avg_temperature);
}

/**
 * @brief Demonstrate calibration functionality
 */
void example_calibration() {
    ESP_LOGI(TAG, "=== Calibration Example ===");
    
    // Read current temperature and calibration offset
    float temp_before, offset_before;
    HfTempError_t error;
    
    error = temp_sensor->read_celsius(&temp_before);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", hf_temp_get_error_string(error));
        return;
    }
    
    error = temp_sensor->get_calibration_offset(&offset_before);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get calibration offset: %s", hf_temp_get_error_string(error));
        return;
    }
    
    ESP_LOGI(TAG, "Before calibration: %.2f°C (offset: %.3f°C)", temp_before, offset_before);
    
    // Simulate calibration with a known reference temperature
    // In a real application, you would use an external reference
    float reference_temp = 25.0f; // Simulated room temperature reference
    
    ESP_LOGI(TAG, "Performing calibration with reference temperature: %.1f°C", reference_temp);
    
    error = temp_sensor->calibrate(reference_temp);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Calibration failed: %s", hf_temp_get_error_string(error));
        return;
    }
    
    // Read temperature and offset after calibration
    float temp_after, offset_after;
    
    error = temp_sensor->read_celsius(&temp_after);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "After calibration: %.2f°C", temp_after);
    }
    
    error = temp_sensor->get_calibration_offset(&offset_after);
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "New calibration offset: %.3f°C", offset_after);
    }
    
    ESP_LOGI(TAG, "Calibration offset change: %.3f°C", offset_after - offset_before);
    
    // Demonstrate manual offset setting
    ESP_LOGI(TAG, "Setting manual calibration offset: +1.5°C");
    error = temp_sensor->set_calibration_offset(1.5f);
    if (error == TEMP_SUCCESS) {
        float temp_manual;
        temp_sensor->read_celsius(&temp_manual);
        ESP_LOGI(TAG, "With manual offset: %.2f°C", temp_manual);
    }
    
    // Reset calibration
    ESP_LOGI(TAG, "Resetting calibration to default");
    temp_sensor->reset_calibration();
    
    float temp_reset;
    temp_sensor->read_celsius(&temp_reset);
    ESP_LOGI(TAG, "After reset: %.2f°C", temp_reset);
}

/**
 * @brief Demonstrate self-test functionality
 */
void example_self_test() {
    ESP_LOGI(TAG, "=== Self-Test Example ===");
    
    // Perform self-test
    ESP_LOGI(TAG, "Starting self-test...");
    HfTempError_t error = temp_sensor->self_test();
    
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Self-test PASSED ✓");
    } else {
        ESP_LOGE(TAG, "Self-test FAILED: %s", hf_temp_get_error_string(error));
    }
    
    // Check health status
    ESP_LOGI(TAG, "Checking sensor health...");
    error = temp_sensor->check_health();
    
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Sensor health: GOOD ✓");
    } else {
        ESP_LOGE(TAG, "Sensor health: BAD - %s", hf_temp_get_error_string(error));
    }
}

/**
 * @brief Demonstrate power management
 */
void example_power_management() {
    ESP_LOGI(TAG, "=== Power Management Example ===");
    
    // Check current state
    HfTempState_t state = temp_sensor->get_state();
    ESP_LOGI(TAG, "Current sensor state: %d", state);
    ESP_LOGI(TAG, "Is sleeping: %s", temp_sensor->is_sleeping() ? "Yes" : "No");
    
    // Enter sleep mode
    ESP_LOGI(TAG, "Entering sleep mode...");
    HfTempError_t error = temp_sensor->enter_sleep_mode();
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Sleep mode entered");
        ESP_LOGI(TAG, "Is sleeping: %s", temp_sensor->is_sleeping() ? "Yes" : "No");
    } else {
        ESP_LOGE(TAG, "Failed to enter sleep mode: %s", hf_temp_get_error_string(error));
    }
    
    // Try to read temperature while in sleep mode (should fail)
    float temp;
    error = temp_sensor->read_celsius(&temp);
    if (error != TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Reading failed while in sleep mode (expected): %s", 
                 hf_temp_get_error_string(error));
    }
    
    // Exit sleep mode
    ESP_LOGI(TAG, "Exiting sleep mode...");
    error = temp_sensor->exit_sleep_mode();
    if (error == TEMP_SUCCESS) {
        ESP_LOGI(TAG, "Sleep mode exited");
        ESP_LOGI(TAG, "Is sleeping: %s", temp_sensor->is_sleeping() ? "Yes" : "No");
    } else {
        ESP_LOGE(TAG, "Failed to exit sleep mode: %s", hf_temp_get_error_string(error));
    }
    
    // Re-enable sensor and read temperature
    ESP_LOGI(TAG, "Re-enabling sensor...");
    error = temp_sensor->enable();
    if (error == TEMP_SUCCESS) {
        error = temp_sensor->read_celsius(&temp);
        if (error == TEMP_SUCCESS) {
            ESP_LOGI(TAG, "Temperature after sleep: %.2f°C", temp);
        }
    }
}

/**
 * @brief Initialize the temperature sensor
 */
bool initialize_temperature_sensor() {
    ESP_LOGI(TAG, "=== Temperature Sensor Initialization ===");
    
    // Create ESP32 temperature sensor instance
    temp_sensor = new EspTemperature();
    if (temp_sensor == nullptr) {
        ESP_LOGE(TAG, "Failed to create temperature sensor instance");
        return false;
    }
    
    // Set error callback
    temp_sensor->set_error_callback(temperature_error_callback, nullptr);
    
    // Method 1: Initialize with base configuration
    HfTempConfig_t config = HF_TEMP_CONFIG_DEFAULT();
    config.range_min_celsius = -10.0f;  // Use optimal range for room temperature
    config.range_max_celsius = 80.0f;
    config.resolution = 0.25f;
    config.enable_threshold_monitoring = false;
    config.enable_power_management = false;
    config.timeout_ms = 1000;
    config.sensor_type = TEMP_SENSOR_TYPE_INTERNAL;
    
    HfTempError_t error = temp_sensor->initialize(&config);
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize temperature sensor: %s", hf_temp_get_error_string(error));
        delete temp_sensor;
        temp_sensor = nullptr;
        return false;
    }
    
    // Enable the sensor
    error = temp_sensor->enable();
    if (error != TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable temperature sensor: %s", hf_temp_get_error_string(error));
        delete temp_sensor;
        temp_sensor = nullptr;
        return false;
    }
    
    ESP_LOGI(TAG, "Temperature sensor initialized and enabled successfully");
    
    // Display current configuration
    float min_range, max_range, resolution;
    temp_sensor->get_range(&min_range, &max_range);
    temp_sensor->get_resolution(&resolution);
    
    ESP_LOGI(TAG, "Configuration:");
    ESP_LOGI(TAG, "  Range: %.1f°C to %.1f°C", min_range, max_range);
    ESP_LOGI(TAG, "  Resolution: %.3f°C", resolution);
    ESP_LOGI(TAG, "  Initialized: %s", temp_sensor->is_initialized() ? "Yes" : "No");
    ESP_LOGI(TAG, "  Enabled: %s", temp_sensor->is_enabled() ? "Yes" : "No");
    
    return true;
}

/**
 * @brief Cleanup resources
 */
void cleanup_temperature_sensor() {
    if (temp_sensor != nullptr) {
        ESP_LOGI(TAG, "Cleaning up temperature sensor...");
        temp_sensor->deinitialize();
        delete temp_sensor;
        temp_sensor = nullptr;
        ESP_LOGI(TAG, "Temperature sensor cleaned up");
    }
}

//--------------------------------------
//  Main Application Task
//--------------------------------------

extern "C" void app_main() {
    ESP_LOGI(TAG, "===== ESP32-C6 Temperature Sensor Example =====");
    
    // Initialize NVS (required for some ESP-IDF components)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize temperature sensor
    if (!initialize_temperature_sensor()) {
        ESP_LOGE(TAG, "Temperature sensor initialization failed. Exiting.");
        return;
    }
    
    // Run examples
    try {
        // Basic functionality
        example_basic_reading();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        example_sensor_info();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        example_esp32_ranges();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Advanced functionality
        example_threshold_monitoring();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        example_continuous_monitoring();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        example_calibration();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // System functionality
        example_self_test();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        example_power_management();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        ESP_LOGI(TAG, "===== All Examples Completed Successfully =====");
        
        // Continuous operation for monitoring
        ESP_LOGI(TAG, "Entering continuous operation mode...");
        ESP_LOGI(TAG, "Reading temperature every 5 seconds. Press reset to restart examples.");
        
        while (true) {
            float temperature;
            HfTempError_t error = temp_sensor->read_celsius(&temperature);
            if (error == TEMP_SUCCESS) {
                ESP_LOGI(TAG, "Chip Temperature: %.2f°C", temperature);
            } else {
                ESP_LOGE(TAG, "Temperature reading failed: %s", hf_temp_get_error_string(error));
            }
            
            vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds
        }
        
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception caught: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception caught");
    }
    
    // Cleanup
    cleanup_temperature_sensor();
    
    ESP_LOGI(TAG, "Example completed");
}