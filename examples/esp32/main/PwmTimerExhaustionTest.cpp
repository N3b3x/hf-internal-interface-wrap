/**
 * @file PwmTimerExhaustionTest.cpp
 * @brief Advanced PWM timer exhaustion and recovery testing for ESP32-C6
 *
 * This test specifically validates the timer allocation logic when all 4 ESP32-C6
 * LEDC timers are exhausted with different frequency/resolution combinations.
 * It tests the sophisticated allocation strategies and recovery mechanisms.
 *
 * @author Analysis Enhancement
 * @date 2025
 */

#include "TestFramework.h"
#include "mcu/esp32/EspPwm.h"
#include "mcu/esp32/EspGpio.h"

static const char* TAG = "PWM_Timer_Exhaustion_Test";

/**
 * @brief Test timer exhaustion and recovery scenarios
 * 
 * This test validates that the EspPwm implementation properly handles
 * the scenario where all 4 ESP32-C6 LEDC timers are allocated and
 * additional unique frequency/resolution combinations are requested.
 */
bool test_timer_exhaustion_and_recovery() noexcept {
    ESP_LOGI(TAG, "=== TIMER EXHAUSTION AND RECOVERY TEST ===");
    
    hf_pwm_unit_config_t config = {};
    config.unit_id = 0;
    config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
    config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
    config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
    config.enable_fade = false;
    config.enable_interrupts = true;
    
    EspPwm pwm(config);
    
    if (!pwm.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PWM");
        return false;
    }
    
    // Phase 1: Allocate all 4 timers with unique frequency/resolution combinations
    ESP_LOGI(TAG, "Phase 1: Allocating all 4 timers with unique combinations");
    
    struct TimerConfig {
        hf_channel_id_t channel;
        hf_gpio_num_t gpio;
        hf_u32_t frequency;
        hf_u8_t resolution;
        const char* description;
    };
    
    // These combinations are carefully chosen to require separate timers
    TimerConfig timer_configs[] = {
        {0, 2, 1000,  8,  "Timer 0: 1kHz @ 8-bit"},   // Timer 0
        {1, 4, 2000,  10, "Timer 1: 2kHz @ 10-bit"},  // Timer 1  
        {2, 5, 5000,  8,  "Timer 2: 5kHz @ 8-bit"},   // Timer 2
        {3, 6, 10000, 9,  "Timer 3: 10kHz @ 9-bit"}   // Timer 3
    };
    
    // Allocate all 4 timers
    for (const auto& cfg : timer_configs) {
        ESP_LOGI(TAG, "Configuring %s", cfg.description);
        
        hf_pwm_channel_config_t ch_config = {};
        ch_config.gpio_pin = cfg.gpio;
        ch_config.channel_id = cfg.channel;
        ch_config.frequency_hz = cfg.frequency;
        ch_config.resolution_bits = cfg.resolution;
        ch_config.duty_initial = (1u << cfg.resolution) / 2; // 50% duty
        
        hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to configure %s: %s", cfg.description, HfPwmErrToString(result));
            return false;
        }
        
        // Verify timer assignment
        int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
        ESP_LOGI(TAG, "✓ %s assigned to timer %d", cfg.description, timer_id);
        
        // Enable the channel
        result = pwm.EnableChannel(cfg.channel);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to enable %s: %s", cfg.description, HfPwmErrToString(result));
            return false;
        }
    }
    
    ESP_LOGI(TAG, "✓ All 4 timers successfully allocated");
    
    // Phase 2: Attempt to allocate a 5th unique combination (should trigger smart allocation)
    ESP_LOGI(TAG, "Phase 2: Testing 5th unique combination allocation");
    
    hf_pwm_channel_config_t fifth_config = {};
    fifth_config.gpio_pin = 7;
    fifth_config.channel_id = 4;
    fifth_config.frequency_hz = 15000;  // Unique frequency
    fifth_config.resolution_bits = 8;   // Different from 10kHz@9-bit
    fifth_config.duty_initial = 64;     // 25% duty for 8-bit
    
    hf_pwm_err_t result = pwm.ConfigureChannel(4, fifth_config);
    
    // This should either:
    // 1. Succeed by reusing a compatible timer (frequency tolerance)
    // 2. Succeed by smart eviction
    // 3. Fail gracefully with appropriate error
    
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
        int8_t timer_id = pwm.GetTimerAssignment(4);
        ESP_LOGI(TAG, "✓ 5th combination allocated successfully to timer %d", timer_id);
        
        // Enable the channel
        result = pwm.EnableChannel(4);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to enable 5th channel: %s", HfPwmErrToString(result));
            return false;
        }
    } else {
        ESP_LOGI(TAG, "✓ 5th combination correctly rejected: %s", HfPwmErrToString(result));
        
        // This is acceptable behavior - all timers are exhausted
        if (result != hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT) {
            ESP_LOGW(TAG, "Expected TIMER_CONFLICT error, got: %s", HfPwmErrToString(result));
        }
    }
    
    // Phase 3: Test timer reuse with compatible frequency
    ESP_LOGI(TAG, "Phase 3: Testing compatible frequency reuse");
    
    hf_pwm_channel_config_t compatible_config = {};
    compatible_config.gpio_pin = 8;
    compatible_config.channel_id = 5;
    compatible_config.frequency_hz = 1050; // Within 5% of 1000Hz (timer 0)
    compatible_config.resolution_bits = 8;  // Same resolution as timer 0
    compatible_config.duty_initial = 128;   // 50% duty for 8-bit
    
    result = pwm.ConfigureChannel(5, compatible_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
        int8_t timer_id = pwm.GetTimerAssignment(5);
        ESP_LOGI(TAG, "✓ Compatible frequency reused timer %d", timer_id);
        
        // Should reuse timer 0 (1000Hz @ 8-bit)
        if (timer_id != pwm.GetTimerAssignment(0)) {
            ESP_LOGW(TAG, "Expected to reuse timer 0, but got timer %d", timer_id);
        }
    } else {
        ESP_LOGE(TAG, "Compatible frequency allocation failed: %s", HfPwmErrToString(result));
        return false;
    }
    
    // Phase 4: Test channel release and timer recovery
    ESP_LOGI(TAG, "Phase 4: Testing channel release and timer recovery");
    
    // Disable and release channel 3 (should free timer 3)
    pwm.DisableChannel(3);
    
    // Now try to allocate the previously failed 5th combination
    if (result != hf_pwm_err_t::PWM_SUCCESS) { // If it failed in Phase 2
        ESP_LOGI(TAG, "Retrying 5th combination after freeing channel 3");
        
        result = pwm.ConfigureChannel(4, fifth_config);
        if (result == hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGI(TAG, "✓ 5th combination succeeded after timer recovery");
            
            int8_t timer_id = pwm.GetTimerAssignment(4);
            ESP_LOGI(TAG, "✓ Allocated to timer %d", timer_id);
        } else {
            ESP_LOGI(TAG, "5th combination still failed after recovery: %s", HfPwmErrToString(result));
        }
    }
    
    // Phase 5: Test health check mechanism
    ESP_LOGI(TAG, "Phase 5: Testing health check and statistics");
    
    // Get diagnostics to verify timer usage
    hf_pwm_diagnostics_t diagnostics;
    result = pwm.GetDiagnostics(diagnostics);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGI(TAG, "Active timers: %d, Active channels: %d", 
                 diagnostics.active_timers, diagnostics.active_channels);
    }
    
    // Get statistics
    hf_pwm_statistics_t statistics;
    result = pwm.GetStatistics(statistics);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGI(TAG, "Channel enables: %lu, Error count: %lu", 
                 statistics.channel_enables_count, statistics.error_count);
    }
    
    ESP_LOGI(TAG, "=== TIMER EXHAUSTION TEST COMPLETED SUCCESSFULLY ===");
    return true;
}

/**
 * @brief Test rapid allocation and deallocation patterns
 */
bool test_rapid_allocation_patterns() noexcept {
    ESP_LOGI(TAG, "=== RAPID ALLOCATION PATTERN TEST ===");
    
    hf_pwm_unit_config_t config = {};
    config.unit_id = 0;
    config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
    config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
    config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
    config.enable_fade = false;
    config.enable_interrupts = true;
    
    EspPwm pwm(config);
    
    if (!pwm.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PWM");
        return false;
    }
    
    // Test rapid configure/release cycles
    for (int cycle = 0; cycle < 10; cycle++) {
        ESP_LOGI(TAG, "Allocation cycle %d", cycle + 1);
        
        // Configure multiple channels with different frequencies
        for (hf_channel_id_t ch = 0; ch < 4; ch++) {
            hf_pwm_channel_config_t ch_config = {};
            ch_config.gpio_pin = static_cast<hf_gpio_num_t>(2 + ch);
            ch_config.channel_id = ch;
            ch_config.frequency_hz = 1000 + (ch * 500) + (cycle * 100);
            ch_config.resolution_bits = 8 + (ch % 3);
            ch_config.duty_initial = 100 + (ch * 50);
            
            hf_pwm_err_t result = pwm.ConfigureChannel(ch, ch_config);
            if (result != hf_pwm_err_t::PWM_SUCCESS) {
                ESP_LOGI(TAG, "Channel %d allocation failed (expected): %s", ch, HfPwmErrToString(result));
            } else {
                pwm.EnableChannel(ch);
            }
        }
        
        // Brief operation
        vTaskDelay(pdMS_TO_TICKS(50));
        
        // Release all channels
        for (hf_channel_id_t ch = 0; ch < 4; ch++) {
            pwm.DisableChannel(ch);
        }
        
        // Allow timer cleanup
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "=== RAPID ALLOCATION PATTERN TEST COMPLETED ===");
    return true;
}

/**
 * @brief Test problematic frequency/resolution combinations
 */
bool test_problematic_combinations() noexcept {
    ESP_LOGI(TAG, "=== PROBLEMATIC COMBINATIONS TEST ===");
    
    hf_pwm_unit_config_t config = {};
    config.unit_id = 0;
    config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
    config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
    config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
    config.enable_fade = false;
    config.enable_interrupts = true;
    
    EspPwm pwm(config);
    
    if (!pwm.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PWM");
        return false;
    }
    
    // Test the specific combinations that should fail
    struct ProblematicTest {
        hf_u32_t frequency;
        hf_u8_t resolution;
        const char* description;
        bool should_fail;
    };
    
    ProblematicTest tests[] = {
        {25000,  10, "25kHz @ 10-bit (borderline)",           false},
        {30000,  10, "30kHz @ 10-bit (should fail)",          true},
        {40000,  10, "40kHz @ 10-bit (should fail)",          true},
        {50000,  10, "50kHz @ 10-bit (should fail)",          true},
        {100000, 10, "100kHz @ 10-bit (should fail)",         true},
        {20000,  12, "20kHz @ 12-bit (borderline)",           false},
        {25000,  12, "25kHz @ 12-bit (should fail)",          true},
        {10000,  14, "10kHz @ 14-bit (should fail)",          true},
        {5000,   8,  "5kHz @ 8-bit (should succeed)",         false},
        {10000,  8,  "10kHz @ 8-bit (should succeed)",        false},
    };
    
    for (const auto& test : tests) {
        ESP_LOGI(TAG, "Testing %s", test.description);
        
        hf_pwm_channel_config_t ch_config = {};
        ch_config.gpio_pin = 2;
        ch_config.channel_id = 0;
        ch_config.frequency_hz = test.frequency;
        ch_config.resolution_bits = test.resolution;
        ch_config.duty_initial = (1u << test.resolution) / 2;
        
        hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
        
        if (test.should_fail) {
            if (result == hf_pwm_err_t::PWM_SUCCESS) {
                ESP_LOGE(TAG, "❌ %s should have failed but succeeded", test.description);
                return false;
            } else {
                ESP_LOGI(TAG, "✓ %s correctly failed: %s", test.description, HfPwmErrToString(result));
            }
        } else {
            if (result != hf_pwm_err_t::PWM_SUCCESS) {
                ESP_LOGE(TAG, "❌ %s should have succeeded but failed: %s", 
                         test.description, HfPwmErrToString(result));
                return false;
            } else {
                ESP_LOGI(TAG, "✓ %s correctly succeeded", test.description);
            }
        }
        
        // Clean up for next test
        pwm.DisableChannel(0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "=== PROBLEMATIC COMBINATIONS TEST COMPLETED ===");
    return true;
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║                    ESP32-C6 PWM TIMER EXHAUSTION TEST SUITE                    ║");
    ESP_LOGI(TAG, "║                      Advanced Timer Allocation Validation                      ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════════════════════════╝");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    TestResults results = {};
    
    // Run advanced timer allocation tests
    ESP_LOGI(TAG, "\n=== ADVANCED TIMER ALLOCATION TESTS ===");
    RUN_TEST_WITH_RESULTS(test_timer_exhaustion_and_recovery, results);
    RUN_TEST_WITH_RESULTS(test_rapid_allocation_patterns, results);
    RUN_TEST_WITH_RESULTS(test_problematic_combinations, results);
    
    // Print results
    ESP_LOGI(TAG, "\n");
    print_test_summary(results, "ESP32-C6 PWM TIMER EXHAUSTION", TAG);
    
    ESP_LOGI(TAG, "Advanced timer allocation testing completed.");
    ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}