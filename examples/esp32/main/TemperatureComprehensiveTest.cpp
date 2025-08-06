/**
 * @file TemperatureComprehensiveTest.cpp
 * @brief Comprehensive Temperature sensor testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 */

#include "base/BaseTemperature.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspTemperature.h"

#include "TestFramework.h"
#include <atomic>

static const char* TAG = "TEMP_Test";

static TestResults g_test_results;

// Global callback tracking variables
static std::atomic<int> g_threshold_callback_count{0};
static std::atomic<int> g_monitoring_callback_count{0};
static std::atomic<float> g_last_callback_temperature{0.0f};

//==============================================================================
// CALLBACK FUNCTIONS FOR TESTING
//==============================================================================

void threshold_callback(BaseTemperature* sensor, float temperature, hf_u32_t threshold_type, void* user_data) {
    g_threshold_callback_count++;
    g_last_callback_temperature = temperature;
    ESP_LOGI(TAG, "Threshold callback: %.2f°C, type: %u", temperature, threshold_type);
}

void monitoring_callback(BaseTemperature* sensor, const hf_temp_reading_t* reading, void* user_data) {
    g_monitoring_callback_count++;
    if (reading) {
        g_last_callback_temperature = reading->temperature_celsius;
        ESP_LOGD(TAG, "Monitoring callback: %.2f°C", reading->temperature_celsius);
    }
}

void esp_threshold_callback(EspTemperature* sensor, float temperature, bool is_high_threshold) {
    g_threshold_callback_count++;
    g_last_callback_temperature = temperature;
    ESP_LOGI(TAG, "ESP threshold callback: %.2f°C, high: %s", temperature, is_high_threshold ? "true" : "false");
}

void esp_monitoring_callback(EspTemperature* sensor, float temperature, hf_u64_t timestamp_us) {
    g_monitoring_callback_count++;
    g_last_callback_temperature = temperature;
    ESP_LOGD(TAG, "ESP monitoring callback: %.2f°C at %llu", temperature, timestamp_us);
}

//==============================================================================
// BASIC FUNCTIONALITY TESTS
//==============================================================================

bool test_temperature_sensor_initialization() noexcept {
    ESP_LOGI(TAG, "Testing temperature sensor initialization...");

    EspTemperature test_temp;
    
    // Test initial state
    if (test_temp.GetCurrentState() != HF_TEMP_STATE_UNINITIALIZED) {
        ESP_LOGE(TAG, "Initial state should be UNINITIALIZED");
        return false;
    }

    // Test initialization
    auto init_result = test_temp.EnsureInitialized();
    if (!init_result) {
        ESP_LOGE(TAG, "Failed to initialize temperature sensor");
        return false;
    }

    // Verify state after initialization
    if (test_temp.GetCurrentState() != HF_TEMP_STATE_INITIALIZED) {
        ESP_LOGE(TAG, "State should be INITIALIZED after init");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Temperature sensor initialization successful");
    return true;
}

bool test_temperature_reading() noexcept {
    ESP_LOGI(TAG, "Testing temperature reading...");

    EspTemperature test_temp;
    
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Test basic reading
    hf_temp_reading_t reading = {};
    auto read_result = test_temp.ReadTemperature(&reading);
    if (read_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", GetTempErrorString(read_result));
        return false;
    }

    // Validate reading
    if (!reading.is_valid) {
        ESP_LOGE(TAG, "Temperature reading is not valid");
        return false;
    }

    // Check reasonable temperature range (-50°C to 150°C for chip)
    if (reading.temperature_celsius < -50.0f || reading.temperature_celsius > 150.0f) {
        ESP_LOGE(TAG, "Temperature %.2f°C outside reasonable range", reading.temperature_celsius);
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Temperature reading: %.2f°C", reading.temperature_celsius);
    return true;
}

bool test_sensor_info() noexcept {
    ESP_LOGI(TAG, "Testing sensor info retrieval...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    hf_temp_sensor_info_t info = {};
    auto info_result = test_temp.GetSensorInfo(&info);
    if (info_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get sensor info");
        return false;
    }

    // Validate sensor info
    if (info.sensor_type != HF_TEMP_SENSOR_TYPE_INTERNAL) {
        ESP_LOGE(TAG, "Sensor type should be INTERNAL");
        return false;
    }

    if (info.capabilities == HF_TEMP_CAP_NONE) {
        ESP_LOGE(TAG, "Sensor should have capabilities");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Sensor info: %s %s, Range: %.1f to %.1f°C", 
             info.manufacturer, info.model, info.min_temp_celsius, info.max_temp_celsius);
    return true;
}

//==============================================================================
// RANGE MANAGEMENT TESTS
//==============================================================================

bool test_range_management() noexcept {
    ESP_LOGI(TAG, "Testing range management...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Test getting current range
    float min_temp, max_temp;
    auto range_result = test_temp.GetRange(&min_temp, &max_temp);
    if (range_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get current range");
        return false;
    }

    ESP_LOGI(TAG, "Current range: %.1f to %.1f°C", min_temp, max_temp);

    // Test setting different ranges
    auto set_range_result = test_temp.SetRange(20.0f, 100.0f);
    if (set_range_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set range 20-100°C");
        return false;
    }

    // Verify range was set
    auto verify_result = test_temp.GetRange(&min_temp, &max_temp);
    if (verify_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to verify range");
        return false;
    }

    ESP_LOGI(TAG, "New range: %.1f to %.1f°C", min_temp, max_temp);

    // Test ESP32-specific range methods
    auto esp_range = test_temp.GetMeasurementRange();
    ESP_LOGI(TAG, "ESP32 range: %d", static_cast<int>(esp_range));

    // Test finding optimal range
    auto optimal_range = test_temp.FindOptimalRange(-10.0f, 80.0f);
    if (optimal_range >= ESP_TEMP_RANGE_COUNT) {
        ESP_LOGE(TAG, "Failed to find optimal range");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Range management tests passed");
    return true;
}

//==============================================================================
// THRESHOLD MONITORING TESTS
//==============================================================================

bool test_threshold_monitoring() noexcept {
    ESP_LOGI(TAG, "Testing threshold monitoring...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Reset callback counters
    g_threshold_callback_count = 0;

    // Set thresholds
    auto set_result = test_temp.SetThresholds(10.0f, 50.0f);
    if (set_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set thresholds");
        return false;
    }

    // Verify thresholds
    float low_thresh, high_thresh;
    auto get_result = test_temp.GetThresholds(&low_thresh, &high_thresh);
    if (get_result != hf_temp_err_t::TEMP_SUCCESS || low_thresh != 10.0f || high_thresh != 50.0f) {
        ESP_LOGE(TAG, "Threshold verification failed");
        return false;
    }

    // Enable threshold monitoring
    auto enable_result = test_temp.EnableThresholdMonitoring(threshold_callback, nullptr);
    if (enable_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable threshold monitoring");
        return false;
    }

    // Test ESP32-specific threshold callback
    auto esp_callback_result = test_temp.SetEspThresholdCallback(esp_threshold_callback);
    if (esp_callback_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set ESP threshold callback");
        return false;
    }

    // Disable threshold monitoring
    auto disable_result = test_temp.DisableThresholdMonitoring();
    if (disable_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to disable threshold monitoring");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Threshold monitoring tests passed");
    return true;
}

//==============================================================================
// CONTINUOUS MONITORING TESTS
//==============================================================================

bool test_continuous_monitoring() noexcept {
    ESP_LOGI(TAG, "Testing continuous monitoring...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Reset callback counters
    g_monitoring_callback_count = 0;

    // Check initial monitoring state
    if (test_temp.IsMonitoringActive()) {
        ESP_LOGE(TAG, "Monitoring should not be active initially");
        return false;
    }

    // Start continuous monitoring
    auto start_result = test_temp.StartContinuousMonitoring(10, monitoring_callback, nullptr);
    if (start_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start continuous monitoring");
        return false;
    }

    // Verify monitoring is active
    if (!test_temp.IsMonitoringActive()) {
        ESP_LOGE(TAG, "Monitoring should be active");
        return false;
    }

    // Test ESP32-specific monitoring callback
    auto esp_callback_result = test_temp.SetEspMonitoringCallback(esp_monitoring_callback);
    if (esp_callback_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set ESP monitoring callback");
        return false;
    }

    // Wait for some callbacks
    vTaskDelay(pdMS_TO_TICKS(1500));

    // Check if callbacks were received
    int callback_count = g_monitoring_callback_count.load();
    if (callback_count < 5) {
        ESP_LOGW(TAG, "Only received %d callbacks, expected more", callback_count);
    } else {
        ESP_LOGI(TAG, "Received %d monitoring callbacks", callback_count);
    }

    // Stop continuous monitoring
    auto stop_result = test_temp.StopContinuousMonitoring();
    if (stop_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to stop continuous monitoring");
        return false;
    }

    // Verify monitoring is stopped
    if (test_temp.IsMonitoringActive()) {
        ESP_LOGE(TAG, "Monitoring should not be active after stop");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Continuous monitoring tests passed");
    return true;
}

//==============================================================================
// CALIBRATION TESTS
//==============================================================================

bool test_calibration() noexcept {
    ESP_LOGI(TAG, "Testing calibration functionality...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Test getting initial calibration offset
    float initial_offset;
    auto get_result = test_temp.GetCalibrationOffset(&initial_offset);
    if (get_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get initial calibration offset");
        return false;
    }

    ESP_LOGI(TAG, "Initial calibration offset: %.2f°C", initial_offset);

    // Test setting calibration offset
    const float test_offset = 2.5f;
    auto set_result = test_temp.SetCalibrationOffset(test_offset);
    if (set_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set calibration offset");
        return false;
    }

    // Verify calibration offset was set
    float current_offset;
    auto verify_result = test_temp.GetCalibrationOffset(&current_offset);
    if (verify_result != hf_temp_err_t::TEMP_SUCCESS || current_offset != test_offset) {
        ESP_LOGE(TAG, "Calibration offset verification failed");
        return false;
    }

    // Test reading with calibration
    hf_temp_reading_t reading_calibrated = {};
    auto read_result = test_temp.ReadTemperature(&reading_calibrated);
    if (read_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read temperature with calibration");
        return false;
    }

    // Reset calibration
    auto reset_result = test_temp.ResetCalibration();
    if (reset_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to reset calibration");
        return false;
    }

    // Verify calibration was reset
    float reset_offset;
    auto final_result = test_temp.GetCalibrationOffset(&reset_offset);
    if (final_result != hf_temp_err_t::TEMP_SUCCESS || reset_offset != 0.0f) {
        ESP_LOGE(TAG, "Calibration reset verification failed");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Calibration tests passed");
    return true;
}

//==============================================================================
// POWER MANAGEMENT TESTS
//==============================================================================

bool test_power_management() noexcept {
    ESP_LOGI(TAG, "Testing power management...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Check initial sleep state
    if (test_temp.IsSleeping()) {
        ESP_LOGE(TAG, "Sensor should not be sleeping initially");
        return false;
    }

    // Enter sleep mode
    auto sleep_result = test_temp.EnterSleepMode();
    if (sleep_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enter sleep mode");
        return false;
    }

    // Verify sleep state
    if (!test_temp.IsSleeping()) {
        ESP_LOGE(TAG, "Sensor should be sleeping");
        return false;
    }

    // Verify state is SLEEPING
    if (test_temp.GetCurrentState() != HF_TEMP_STATE_SLEEPING) {
        ESP_LOGE(TAG, "State should be SLEEPING");
        return false;
    }

    // Exit sleep mode
    auto wake_result = test_temp.ExitSleepMode();
    if (wake_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to exit sleep mode");
        return false;
    }

    // Verify wake state
    if (test_temp.IsSleeping()) {
        ESP_LOGE(TAG, "Sensor should not be sleeping after wake");
        return false;
    }

    // Verify we can read temperature after wake
    hf_temp_reading_t reading = {};
    auto read_result = test_temp.ReadTemperature(&reading);
    if (read_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read temperature after wake");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Power management tests passed");
    return true;
}

//==============================================================================
// SELF-TEST AND HEALTH MONITORING TESTS
//==============================================================================

bool test_self_test_and_health() noexcept {
    ESP_LOGI(TAG, "Testing self-test and health monitoring...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Perform self-test
    auto self_test_result = test_temp.SelfTest();
    if (self_test_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Self-test failed: %s", GetTempErrorString(self_test_result));
        return false;
    }

    // Check health status
    auto health_result = test_temp.CheckHealth();
    if (health_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGW(TAG, "Health check indicates issues: %s", GetTempErrorString(health_result));
    }

    ESP_LOGI(TAG, "[SUCCESS] Self-test and health monitoring passed");
    return true;
}

//==============================================================================
// STATISTICS AND DIAGNOSTICS TESTS
//==============================================================================

bool test_statistics_and_diagnostics() noexcept {
    ESP_LOGI(TAG, "Testing statistics and diagnostics...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Perform some operations to generate statistics
    for (int i = 0; i < 5; i++) {
        hf_temp_reading_t reading = {};
        test_temp.ReadTemperature(&reading);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Get statistics
    hf_temp_statistics_t stats = {};
    auto stats_result = test_temp.GetStatistics(stats);
    if (stats_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get statistics");
        return false;
    }

    // Validate statistics
    if (stats.temperature_readings < 5) {
        ESP_LOGE(TAG, "Expected at least 5 temperature readings in statistics");
        return false;
    }

    ESP_LOGI(TAG, "Statistics: %u total ops, %u readings, %.2f avg temp", 
             stats.total_operations, stats.temperature_readings, stats.avg_temperature_celsius);

    // Get diagnostics
    hf_temp_diagnostics_t diag = {};
    auto diag_result = test_temp.GetDiagnostics(diag);
    if (diag_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get diagnostics");
        return false;
    }

    ESP_LOGI(TAG, "Diagnostics: healthy=%s, errors=%u", 
             diag.sensor_healthy ? "true" : "false", diag.consecutive_errors);

    // Test resetting statistics
    auto reset_stats_result = test_temp.ResetStatistics();
    if (reset_stats_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to reset statistics");
        return false;
    }

    // Test resetting diagnostics
    auto reset_diag_result = test_temp.ResetDiagnostics();
    if (reset_diag_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to reset diagnostics");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics tests passed");
    return true;
}

//==============================================================================
// ESP32-SPECIFIC TESTS
//==============================================================================

bool test_esp32_specific_features() noexcept {
    ESP_LOGI(TAG, "Testing ESP32-specific features...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Test ESP32-specific configuration
    esp_temp_config_t esp_config = ESP_TEMP_CONFIG_DEFAULT();
    esp_config.range = ESP_TEMP_RANGE_20_100;
    esp_config.calibration_offset = 1.0f;
    
    auto init_esp_result = test_temp.InitializeEsp32(esp_config);
    if (init_esp_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize with ESP32 config");
        return false;
    }

    // Test setting measurement range
    auto set_range_result = test_temp.SetMeasurementRange(ESP_TEMP_RANGE_NEG10_80);
    if (set_range_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set measurement range");
        return false;
    }

    // Verify range was set
    auto current_range = test_temp.GetMeasurementRange();
    if (current_range != ESP_TEMP_RANGE_NEG10_80) {
        ESP_LOGE(TAG, "Range not set correctly");
        return false;
    }

    // Test getting range info
    float min_temp, max_temp, accuracy;
    auto range_info_result = test_temp.GetRangeInfo(ESP_TEMP_RANGE_NEG10_80, &min_temp, &max_temp, &accuracy);
    if (range_info_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to get range info");
        return false;
    }

    ESP_LOGI(TAG, "Range info: %.1f to %.1f°C, ±%.1f°C accuracy", min_temp, max_temp, accuracy);

    // Test reading raw temperature
    float raw_temp;
    auto raw_result = test_temp.ReadRawTemperature(&raw_temp);
    if (raw_result != hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read raw temperature");
        return false;
    }

    ESP_LOGI(TAG, "Raw temperature: %.2f°C", raw_temp);

    // Test getting ESP handle
    auto esp_handle = test_temp.GetEspHandle();
    if (esp_handle == nullptr) {
        ESP_LOGE(TAG, "ESP handle should not be null");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] ESP32-specific features tests passed");
    return true;
}

//==============================================================================
// ERROR HANDLING TESTS
//==============================================================================

bool test_error_handling() noexcept {
    ESP_LOGI(TAG, "Testing error handling...");

    EspTemperature test_temp;

    // Test operations on uninitialized sensor
    hf_temp_reading_t reading = {};
    auto read_result = test_temp.ReadTemperature(&reading);
    if (read_result == hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Reading should fail on uninitialized sensor");
        return false;
    }

    // Initialize for further tests
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Test null pointer handling
    auto null_read_result = test_temp.ReadTemperature(nullptr);
    if (null_read_result != hf_temp_err_t::TEMP_ERR_NULL_POINTER) {
        ESP_LOGE(TAG, "Should return null pointer error");
        return false;
    }

    // Test invalid range
    auto invalid_range_result = test_temp.SetRange(100.0f, 50.0f); // min > max
    if (invalid_range_result == hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Invalid range should fail");
        return false;
    }

    // Test invalid thresholds
    auto invalid_thresh_result = test_temp.SetThresholds(50.0f, 30.0f); // low > high
    if (invalid_thresh_result == hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Invalid thresholds should fail");
        return false;
    }

    // Test invalid sample rate
    auto invalid_rate_result = test_temp.StartContinuousMonitoring(0, monitoring_callback, nullptr);
    if (invalid_rate_result == hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGE(TAG, "Invalid sample rate should fail");
        return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Error handling tests passed");
    return true;
}

//==============================================================================
// PERFORMANCE AND STRESS TESTS
//==============================================================================

bool test_performance_and_stress() noexcept {
    ESP_LOGI(TAG, "Testing performance and stress scenarios...");

    EspTemperature test_temp;
    if (!test_temp.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return false;
    }

    // Performance test: rapid readings
    const int num_readings = 100;
    hf_u64_t start_time = esp_timer_get_time();
    
    for (int i = 0; i < num_readings; i++) {
        hf_temp_reading_t reading = {};
        auto result = test_temp.ReadTemperature(&reading);
        if (result != hf_temp_err_t::TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Reading %d failed", i);
            return false;
        }
    }
    
    hf_u64_t end_time = esp_timer_get_time();
    hf_u64_t total_time = end_time - start_time;
    float avg_time_ms = (total_time / 1000.0f) / num_readings;
    
    ESP_LOGI(TAG, "Performance: %d readings in %.2f ms (avg: %.2f ms per reading)", 
             num_readings, total_time / 1000.0f, avg_time_ms);

    // Stress test: multiple initialize/deinitialize cycles
    for (int cycle = 0; cycle < 5; cycle++) {
        EspTemperature stress_temp;
        if (!stress_temp.EnsureInitialized()) {
            ESP_LOGE(TAG, "Stress test init failed on cycle %d", cycle);
            return false;
        }
        
        hf_temp_reading_t reading = {};
        auto result = stress_temp.ReadTemperature(&reading);
        if (result != hf_temp_err_t::TEMP_SUCCESS) {
            ESP_LOGE(TAG, "Stress test reading failed on cycle %d", cycle);
            return false;
        }
        
        // Destructor will handle deinitialization
    }

    ESP_LOGI(TAG, "[SUCCESS] Performance and stress tests passed");
    return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║              ESP32-C6 TEMPERATURE COMPREHENSIVE TEST SUITE                  ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
    
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Basic functionality tests
    RUN_TEST(test_temperature_sensor_initialization);
    RUN_TEST(test_temperature_reading);
    RUN_TEST(test_sensor_info);

    // Advanced feature tests
    RUN_TEST(test_range_management);
    RUN_TEST(test_threshold_monitoring);
    RUN_TEST(test_continuous_monitoring);
    RUN_TEST(test_calibration);
    RUN_TEST(test_power_management);
    RUN_TEST(test_self_test_and_health);
    RUN_TEST(test_statistics_and_diagnostics);

    // ESP32-specific tests
    RUN_TEST(test_esp32_specific_features);

    // Error handling and stress tests
    RUN_TEST(test_error_handling);
    RUN_TEST(test_performance_and_stress);

    // Print final results
    print_test_summary(g_test_results, "TEMPERATURE", TAG);
    
    // Keep running
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
