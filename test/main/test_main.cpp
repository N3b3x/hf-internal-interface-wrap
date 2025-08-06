/**
 * @file test_main.cpp
 * @brief Main test runner for HardFOC IID unit tests
 * 
 * This file serves as the entry point for all unit tests in the HardFOC IID project.
 * It sets up the Unity testing framework and executes comprehensive tests for
 * EspGpio and EspAdc classes.
 * 
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#include "unity.h"
#include "unity_config.h"
#include "mock/mock_state_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char* TAG = "TEST_MAIN";

/**
 * @brief Unity test setup function
 * Called before each test case
 */
void setUp(void) {
    // Reset mock state before each test
    mock_state_reset();
    
    // Configure default mock behavior
    mock_system_set_logging(true, ESP_LOG_DEBUG);
    mock_system_set_heap_free_size(100000); // 100KB available
}

/**
 * @brief Unity test teardown function
 * Called after each test case
 */
void tearDown(void) {
    // Verify no unexpected mock calls occurred
    // Additional cleanup if needed
}

// External test function declarations
// EspGpio tests
extern void test_esp_gpio_basic_initialization(void);
extern void test_esp_gpio_basic_operations(void);
extern void test_esp_gpio_direction_setting(void);
extern void test_esp_gpio_pull_mode_configuration(void);
extern void test_esp_gpio_drive_capability(void);

extern void test_esp_gpio_interrupt_configuration(void);
extern void test_esp_gpio_interrupt_enable_disable(void);
extern void test_esp_gpio_interrupt_handling(void);
extern void test_esp_gpio_interrupt_callback(void);

extern void test_esp_gpio_advanced_glitch_filter(void);
extern void test_esp_gpio_advanced_sleep_config(void);
extern void test_esp_gpio_advanced_hold_functions(void);
extern void test_esp_gpio_advanced_wakeup_config(void);

extern void test_esp_gpio_power_management_sleep(void);
extern void test_esp_gpio_power_management_retention(void);

// EspAdc tests
extern void test_esp_adc_basic_initialization(void);
extern void test_esp_adc_basic_channel_config(void);
extern void test_esp_adc_basic_error_handling(void);

extern void test_esp_adc_oneshot_single_read(void);
extern void test_esp_adc_oneshot_multiple_channels(void);
extern void test_esp_adc_oneshot_voltage_conversion(void);

extern void test_esp_adc_continuous_mode_setup(void);
extern void test_esp_adc_continuous_mode_data_acquisition(void);
extern void test_esp_adc_continuous_mode_callbacks(void);

extern void test_esp_adc_calibration_setup(void);
extern void test_esp_adc_calibration_voltage_accuracy(void);

extern void test_esp_adc_filters_configuration(void);
extern void test_esp_adc_filters_operation(void);

extern void test_esp_adc_monitors_threshold_setup(void);
extern void test_esp_adc_monitors_callback_handling(void);

// Base class tests
extern void test_base_gpio_interface(void);
extern void test_base_adc_interface(void);

// Integration tests
extern void test_integration_gpio_adc_combined(void);

/**
 * @brief Run all EspGpio tests
 */
void run_esp_gpio_tests(void) {
    ESP_LOGI(TAG, "=== Running EspGpio Tests ===");
    
    // Basic functionality tests
    ESP_LOGI(TAG, "--- EspGpio Basic Tests ---");
    RUN_TEST(test_esp_gpio_basic_initialization);
    RUN_TEST(test_esp_gpio_basic_operations);
    RUN_TEST(test_esp_gpio_direction_setting);
    RUN_TEST(test_esp_gpio_pull_mode_configuration);
    RUN_TEST(test_esp_gpio_drive_capability);
    
    // Interrupt tests
    ESP_LOGI(TAG, "--- EspGpio Interrupt Tests ---");
    RUN_TEST(test_esp_gpio_interrupt_configuration);
    RUN_TEST(test_esp_gpio_interrupt_enable_disable);
    RUN_TEST(test_esp_gpio_interrupt_handling);
    RUN_TEST(test_esp_gpio_interrupt_callback);
    
    // Advanced feature tests
    ESP_LOGI(TAG, "--- EspGpio Advanced Features Tests ---");
    RUN_TEST(test_esp_gpio_advanced_glitch_filter);
    RUN_TEST(test_esp_gpio_advanced_sleep_config);
    RUN_TEST(test_esp_gpio_advanced_hold_functions);
    RUN_TEST(test_esp_gpio_advanced_wakeup_config);
    
    // Power management tests
    ESP_LOGI(TAG, "--- EspGpio Power Management Tests ---");
    RUN_TEST(test_esp_gpio_power_management_sleep);
    RUN_TEST(test_esp_gpio_power_management_retention);
}

/**
 * @brief Run all EspAdc tests
 */
void run_esp_adc_tests(void) {
    ESP_LOGI(TAG, "=== Running EspAdc Tests ===");
    
    // Basic functionality tests
    ESP_LOGI(TAG, "--- EspAdc Basic Tests ---");
    RUN_TEST(test_esp_adc_basic_initialization);
    RUN_TEST(test_esp_adc_basic_channel_config);
    RUN_TEST(test_esp_adc_basic_error_handling);
    
    // One-shot mode tests
    ESP_LOGI(TAG, "--- EspAdc One-shot Mode Tests ---");
    RUN_TEST(test_esp_adc_oneshot_single_read);
    RUN_TEST(test_esp_adc_oneshot_multiple_channels);
    RUN_TEST(test_esp_adc_oneshot_voltage_conversion);
    
    // Continuous mode tests
    ESP_LOGI(TAG, "--- EspAdc Continuous Mode Tests ---");
    RUN_TEST(test_esp_adc_continuous_mode_setup);
    RUN_TEST(test_esp_adc_continuous_mode_data_acquisition);
    RUN_TEST(test_esp_adc_continuous_mode_callbacks);
    
    // Calibration tests
    ESP_LOGI(TAG, "--- EspAdc Calibration Tests ---");
    RUN_TEST(test_esp_adc_calibration_setup);
    RUN_TEST(test_esp_adc_calibration_voltage_accuracy);
    
    // Filter tests
    ESP_LOGI(TAG, "--- EspAdc Filter Tests ---");
    RUN_TEST(test_esp_adc_filters_configuration);
    RUN_TEST(test_esp_adc_filters_operation);
    
    // Monitor tests
    ESP_LOGI(TAG, "--- EspAdc Monitor Tests ---");
    RUN_TEST(test_esp_adc_monitors_threshold_setup);
    RUN_TEST(test_esp_adc_monitors_callback_handling);
}

/**
 * @brief Run base class and integration tests
 */
void run_integration_tests(void) {
    ESP_LOGI(TAG, "=== Running Base Class and Integration Tests ===");
    
    ESP_LOGI(TAG, "--- Base Class Tests ---");
    RUN_TEST(test_base_gpio_interface);
    RUN_TEST(test_base_adc_interface);
    
    ESP_LOGI(TAG, "--- Integration Tests ---");
    RUN_TEST(test_integration_gpio_adc_combined);
}

/**
 * @brief Unity test result handler
 */
void unity_test_complete_handler(void) {
    ESP_LOGI(TAG, "All tests completed!");
    
    // Print test statistics
    if (Unity.TestFailures == 0) {
        ESP_LOGI(TAG, "✅ ALL TESTS PASSED! (%d tests)", Unity.NumberOfTests);
    } else {
        ESP_LOGE(TAG, "❌ %d tests FAILED out of %d total tests", 
                 Unity.TestFailures, Unity.NumberOfTests);
    }
    
    // Print memory usage if available
    uint32_t free_heap = mock_system_state.system.heap_free_size;
    ESP_LOGI(TAG, "Free heap: %lu bytes", free_heap);
}

/**
 * @brief Main test task
 */
void test_task(void* pvParameters) {
    ESP_LOGI(TAG, "Starting HardFOC IID Unit Tests");
    ESP_LOGI(TAG, "Unity Version: %s", UNITY_VERSION);
    
    // Initialize mock state
    mock_state_init();
    
    // Begin Unity testing
    UNITY_BEGIN();
    
    // Run all test suites
    run_esp_gpio_tests();
    run_esp_adc_tests();
    run_integration_tests();
    
    // End Unity testing and handle results
    UNITY_END();
    unity_test_complete_handler();
    
    // Keep task alive for monitoring
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Application main function
 */
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "HardFOC IID Unit Test Application Starting");
    
    // Create test task with sufficient stack
    xTaskCreate(test_task, "test_task", 16384, NULL, 5, NULL);
}

#ifdef __cplusplus
}
#endif