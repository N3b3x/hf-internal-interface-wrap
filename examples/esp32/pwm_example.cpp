/**
 * @file pwm_example.cpp
 * @brief Comprehensive example demonstrating ESP32C6 PWM features.
 *
 * This example showcases the improved EspPwm implementation with:
 * - Lazy initialization pattern
 * - Multi-channel PWM generation
 * - Hardware fade functionality
 * - Clock source optimization
 * - Thread-safe operations
 * - Error handling and status monitoring
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "EspPwm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <array>

static const char *TAG = "PwmExample";

//==============================================================================
// EXAMPLE CONFIGURATION
//==============================================================================

// PWM channel configurations
struct PwmChannelExample {
    hf_channel_id_t channel_id;
    hf_pin_num_t gpio_pin;
    uint32_t frequency_hz;
    uint8_t resolution_bits;
    float initial_duty_cycle;
    const char *description;
};

// Example configurations for different applications
static const std::array<PwmChannelExample, 4> PWM_CHANNELS = {{
    {0, 18, 1000,  12, 0.0f, "LED Brightness Control"},      // 1kHz, 12-bit
    {1, 19, 5000,  10, 0.5f, "Motor Speed Control"},         // 5kHz, 10-bit
    {2, 20, 20000, 8,  0.25f, "Audio Tone Generation"},      // 20kHz, 8-bit
    {3, 21, 100,   14, 0.0f, "Servo Position Control"}       // 100Hz, 14-bit
}};

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Configure a PWM channel with error handling
 */
bool ConfigurePwmChannel(EspPwm &pwm, const PwmChannelExample &config) {
    ESP_LOGI(TAG, "Configuring %s on channel %lu, pin %d", 
             config.description, config.channel_id, config.gpio_pin);
    
    hf_pwm_channel_config_t pwm_config;
    pwm_config.output_pin = config.gpio_pin;
    pwm_config.frequency_hz = config.frequency_hz;
    pwm_config.resolution_bits = config.resolution_bits;
    pwm_config.initial_duty_cycle = config.initial_duty_cycle;
    pwm_config.invert_output = false;
    pwm_config.output_mode = hf_pwm_output_mode_t::Normal;
    pwm_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    pwm_config.idle_state = hf_pwm_idle_state_t::Low;
    
    hf_pwm_err_t result = pwm.ConfigureChannel(config.channel_id, pwm_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure channel %lu: %s", 
                 config.channel_id, hf_pwm_err_tToString(result));
        return false;
    }
    
    ESP_LOGI(TAG, "Channel %lu configured successfully", config.channel_id);
    return true;
}

/**
 * @brief Enable a PWM channel with error handling
 */
bool EnablePwmChannel(EspPwm &pwm, hf_channel_id_t channel_id) {
    hf_pwm_err_t result = pwm.EnableChannel(channel_id);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable channel %lu: %s", 
                 channel_id, hf_pwm_err_tToString(result));
        return false;
    }
    
    ESP_LOGI(TAG, "Channel %lu enabled", channel_id);
    return true;
}

/**
 * @brief Demonstrate hardware fade functionality
 */
void DemonstrateHardwareFade(EspPwm &pwm, hf_channel_id_t channel_id) {
    ESP_LOGI(TAG, "Starting hardware fade demonstration on channel %lu", channel_id);
    
    // Fade from 0% to 100% over 2 seconds
    hf_pwm_err_t result = pwm.SetHardwareFade(channel_id, 1.0f, 2000);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Hardware fade failed: %s", hf_pwm_err_tToString(result));
        return;
    }
    
    // Wait for fade to complete
    while (pwm.IsFadeActive(channel_id)) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "Hardware fade completed on channel %lu", channel_id);
    
    // Fade back to 0% over 1 second
    result = pwm.SetHardwareFade(channel_id, 0.0f, 1000);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Hardware fade failed: %s", hf_pwm_err_tToString(result));
        return;
    }
    
    while (pwm.IsFadeActive(channel_id)) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "Hardware fade demonstration completed");
}

/**
 * @brief Demonstrate duty cycle updates
 */
void DemonstrateDutyCycleUpdates(EspPwm &pwm, hf_channel_id_t channel_id) {
    ESP_LOGI(TAG, "Starting duty cycle update demonstration on channel %lu", channel_id);
    
    const std::array<float, 5> duty_cycles = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    
    for (float duty : duty_cycles) {
        hf_pwm_err_t result = pwm.SetDutyCycle(channel_id, duty);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set duty cycle %.2f: %s", 
                     duty, hf_pwm_err_tToString(result));
            continue;
        }
        
        float current_duty = pwm.GetDutyCycle(channel_id);
        ESP_LOGI(TAG, "Duty cycle set to %.2f (actual: %.2f)", duty, current_duty);
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Demonstrate frequency changes
 */
void DemonstrateFrequencyChanges(EspPwm &pwm, hf_channel_id_t channel_id) {
    ESP_LOGI(TAG, "Starting frequency change demonstration on channel %lu", channel_id);
    
    const std::array<uint32_t, 4> frequencies = {1000, 2000, 5000, 10000};
    
    for (uint32_t freq : frequencies) {
        hf_pwm_err_t result = pwm.SetFrequency(channel_id, freq);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set frequency %lu Hz: %s", 
                     freq, hf_pwm_err_tToString(result));
            continue;
        }
        
        uint32_t current_freq = pwm.GetFrequency(channel_id);
        ESP_LOGI(TAG, "Frequency set to %lu Hz (actual: %lu Hz)", freq, current_freq);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Demonstrate clock source optimization
 */
void DemonstrateClockSourceOptimization(EspPwm &pwm) {
    ESP_LOGI(TAG, "Starting clock source optimization demonstration");
    
    const std::array<hf_pwm_clock_source_t, 3> clock_sources = {
        hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB,
        hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL,
        hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST
    };
    
    const std::array<const char*, 3> clock_names = {
        "APB Clock (80MHz)",
        "XTAL Clock (40MHz)",
        "RC Fast Clock (~8MHz)"
    };
    
    for (size_t i = 0; i < clock_sources.size(); i++) {
        hf_pwm_err_t result = pwm.SetClockSource(clock_sources[i]);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set clock source %s: %s", 
                     clock_names[i], hf_pwm_err_tToString(result));
            continue;
        }
        
        hf_pwm_clock_source_t current_source = pwm.GetClockSource();
        ESP_LOGI(TAG, "Clock source set to %s", clock_names[i]);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Demonstrate multi-channel synchronization
 */
void DemonstrateMultiChannelSync(EspPwm &pwm) {
    ESP_LOGI(TAG, "Starting multi-channel synchronization demonstration");
    
    // Configure all channels to same frequency for synchronization
    for (const auto &config : PWM_CHANNELS) {
        hf_pwm_err_t result = pwm.SetFrequency(config.channel_id, 1000);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to sync frequency on channel %lu", config.channel_id);
        }
    }
    
    // Start all channels simultaneously
    hf_pwm_err_t result = pwm.StartAll();
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start all channels: %s", hf_pwm_err_tToString(result));
        return;
    }
    
    ESP_LOGI(TAG, "All channels started simultaneously");
    
    // Demonstrate synchronized duty cycle updates
    for (float duty = 0.0f; duty <= 1.0f; duty += 0.1f) {
        for (const auto &config : PWM_CHANNELS) {
            pwm.SetDutyCycle(config.channel_id, duty);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    ESP_LOGI(TAG, "Multi-channel synchronization demonstration completed");
}

/**
 * @brief Demonstrate status monitoring
 */
void DemonstrateStatusMonitoring(EspPwm &pwm) {
    ESP_LOGI(TAG, "Starting status monitoring demonstration");
    
    for (const auto &config : PWM_CHANNELS) {
        hf_pwm_channel_status_t status;
        hf_pwm_err_t result = pwm.GetChannelStatus(config.channel_id, status);
        
        if (result == hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGI(TAG, "Channel %lu Status:", config.channel_id);
            ESP_LOGI(TAG, "  Enabled: %s", status.is_enabled ? "Yes" : "No");
            ESP_LOGI(TAG, "  Running: %s", status.is_running ? "Yes" : "No");
            ESP_LOGI(TAG, "  Frequency: %lu Hz", status.current_frequency_hz);
            ESP_LOGI(TAG, "  Duty Cycle: %.2f%%", status.current_duty_cycle * 100.0f);
            ESP_LOGI(TAG, "  Raw Duty: %lu", status.raw_duty_value);
            ESP_LOGI(TAG, "  Last Error: %s", hf_pwm_err_tToString(status.last_error));
        } else {
            ESP_LOGE(TAG, "Failed to get status for channel %lu", config.channel_id);
        }
    }
}

//==============================================================================
// MAIN EXAMPLE FUNCTION
//==============================================================================

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32C6 PWM Example");
    ESP_LOGI(TAG, "This example demonstrates the improved EspPwm implementation");
    
    // Create PWM controller with lazy initialization
    EspPwm pwm_controller(HF_PWM_APB_CLOCK_HZ);
    
    // Ensure PWM is initialized (lazy initialization)
    if (!pwm_controller.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PWM controller");
        return;
    }
    
    ESP_LOGI(TAG, "PWM controller initialized successfully");
    
    // Configure all PWM channels
    ESP_LOGI(TAG, "Configuring PWM channels...");
    for (const auto &config : PWM_CHANNELS) {
        if (!ConfigurePwmChannel(pwm_controller, config)) {
            ESP_LOGE(TAG, "Failed to configure channel %lu", config.channel_id);
            continue;
        }
    }
    
    // Enable all channels
    ESP_LOGI(TAG, "Enabling PWM channels...");
    for (const auto &config : PWM_CHANNELS) {
        if (!EnablePwmChannel(pwm_controller, config.channel_id)) {
            ESP_LOGE(TAG, "Failed to enable channel %lu", config.channel_id);
        }
    }
    
    // Demonstrate various features
    ESP_LOGI(TAG, "Starting feature demonstrations...");
    
    // 1. Hardware fade demonstration
    DemonstrateHardwareFade(pwm_controller, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 2. Duty cycle updates
    DemonstrateDutyCycleUpdates(pwm_controller, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 3. Frequency changes
    DemonstrateFrequencyChanges(pwm_controller, 2);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 4. Clock source optimization
    DemonstrateClockSourceOptimization(pwm_controller);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 5. Multi-channel synchronization
    DemonstrateMultiChannelSync(pwm_controller);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 6. Status monitoring
    DemonstrateStatusMonitoring(pwm_controller);
    
    ESP_LOGI(TAG, "PWM example completed successfully");
    ESP_LOGI(TAG, "All features demonstrated:");
    ESP_LOGI(TAG, "  ✓ Lazy initialization");
    ESP_LOGI(TAG, "  ✓ Multi-channel configuration");
    ESP_LOGI(TAG, "  ✓ Hardware fade functionality");
    ESP_LOGI(TAG, "  ✓ Duty cycle and frequency control");
    ESP_LOGI(TAG, "  ✓ Clock source optimization");
    ESP_LOGI(TAG, "  ✓ Multi-channel synchronization");
    ESP_LOGI(TAG, "  ✓ Status monitoring and error handling");
    
    // Keep the example running
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "PWM example still running...");
    }
} 