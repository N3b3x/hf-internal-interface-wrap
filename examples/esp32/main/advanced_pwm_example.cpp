/**
 * @file advanced_pwm_example.cpp
 * @brief Comprehensive example demonstrating all advanced features of the refactored EspPwm implementation.
 *
 * This example showcases:
 * - Multi-variant ESP32 support
 * - Unit configuration with different modes
 * - Hardware fade functionality
 * - Complementary outputs for motor control
 * - Statistics and diagnostics monitoring
 * - Advanced timer management
 * - Interrupt-driven callbacks
 * - Error handling and recovery
 *
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#include "inc/mcu/esp32/EspPwm.h"
#include "inc/mcu/esp32/utils/EspTypes_PWM.h"

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

#ifdef __cplusplus
}
#endif

static const char* TAG = "AdvancedPwmExample";

//==============================================================================
// CONFIGURATION CONSTANTS
//==============================================================================

// PWM Configuration for different use cases
static constexpr hf_u32_t LED_FREQUENCY_HZ = 1000;      ///< LED PWM frequency
static constexpr hf_u32_t MOTOR_FREQUENCY_HZ = 20000;   ///< Motor PWM frequency
static constexpr hf_u32_t SERVO_FREQUENCY_HZ = 50;      ///< Servo PWM frequency
static constexpr hf_u32_t AUDIO_FREQUENCY_HZ = 440;     ///< Audio PWM frequency

static constexpr hf_u8_t LED_RESOLUTION_BITS = 8;       ///< LED resolution
static constexpr hf_u8_t MOTOR_RESOLUTION_BITS = 12;    ///< Motor resolution
static constexpr hf_u8_t SERVO_RESOLUTION_BITS = 16;    ///< Servo resolution
static constexpr hf_u8_t AUDIO_RESOLUTION_BITS = 10;    ///< Audio resolution

// GPIO Pin assignments (ESP32-C6 specific)
static constexpr hf_pin_num_t LED_PIN = 2;              ///< LED output pin
static constexpr hf_pin_num_t MOTOR_A_PIN = 3;          ///< Motor phase A
static constexpr hf_pin_num_t MOTOR_B_PIN = 4;          ///< Motor phase B
static constexpr hf_pin_num_t SERVO_PIN = 5;            ///< Servo control pin
static constexpr hf_pin_num_t AUDIO_PIN = 6;            ///< Audio output pin

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

static EspPwm* g_pwm_controller = nullptr;
static volatile bool g_fade_complete = false;
static volatile hf_u32_t g_period_count = 0;

//==============================================================================
// CALLBACK FUNCTIONS
//==============================================================================

/**
 * @brief Period complete callback for PWM channels
 * @param channel_id Channel that completed a period
 * @param user_data User data (unused)
 */
void IRAM_ATTR PeriodCompleteCallback(hf_channel_id_t channel_id, void* user_data) noexcept {
    g_period_count++;
    
    // Log every 1000 periods to avoid spam
    if (g_period_count % 1000 == 0) {
        ESP_LOGI(TAG, "Period complete on channel %lu (total: %lu)", channel_id, g_period_count);
    }
}

/**
 * @brief Fault callback for PWM error handling
 * @param channel_id Channel that encountered fault
 * @param error Error code
 * @param user_data User data (unused)
 */
void IRAM_ATTR FaultCallback(hf_channel_id_t channel_id, hf_pwm_err_t error, void* user_data) noexcept {
    ESP_LOGE(TAG, "PWM fault on channel %lu: %s", channel_id, 
             hf_pwm_err_tToString(error));
    
    // In a real application, implement error recovery here
    // For this example, we just log the error
}

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Print PWM statistics
 * @param pwm PWM controller instance
 */
void PrintStatistics(const EspPwm& pwm) {
    hf_pwm_statistics_t stats;
    if (pwm.GetStatistics(stats) == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGI(TAG, "=== PWM Statistics ===");
        ESP_LOGI(TAG, "Total duty updates: %llu", stats.total_duty_updates);
        ESP_LOGI(TAG, "Total frequency changes: %llu", stats.total_frequency_changes);
        ESP_LOGI(TAG, "Total fades completed: %llu", stats.total_fades_completed);
        ESP_LOGI(TAG, "Total errors: %llu", stats.total_errors);
        ESP_LOGI(TAG, "Last operation time: %llu us", stats.last_operation_time_us);
        ESP_LOGI(TAG, "Last error: %s", hf_pwm_err_tToString(stats.last_error));
    }
}

/**
 * @brief Print PWM diagnostics
 * @param pwm PWM controller instance
 */
void PrintDiagnostics(const EspPwm& pwm) {
    hf_pwm_diagnostics_t diag;
    if (pwm.GetDiagnostics(diag) == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGI(TAG, "=== PWM Diagnostics ===");
        ESP_LOGI(TAG, "Hardware initialized: %s", diag.hardware_initialized ? "Yes" : "No");
        ESP_LOGI(TAG, "Fade functionality ready: %s", diag.fade_functionality_ready ? "Yes" : "No");
        ESP_LOGI(TAG, "Active channels: %d", diag.active_channels);
        ESP_LOGI(TAG, "Active timers: %d", diag.active_timers);
        ESP_LOGI(TAG, "System uptime: %lu ms", diag.system_uptime_ms);
        ESP_LOGI(TAG, "Last global error: %s", hf_pwm_err_tToString(diag.last_global_error));
    }
}

/**
 * @brief Print channel status
 * @param pwm PWM controller instance
 * @param channel_id Channel to check
 */
void PrintChannelStatus(const EspPwm& pwm, hf_channel_id_t channel_id) {
    hf_pwm_channel_status_t status;
    if (pwm.GetChannelStatus(channel_id, status) == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGI(TAG, "=== Channel %lu Status ===", channel_id);
        ESP_LOGI(TAG, "Enabled: %s", status.is_enabled ? "Yes" : "No");
        ESP_LOGI(TAG, "Running: %s", status.is_running ? "Yes" : "No");
        ESP_LOGI(TAG, "Frequency: %lu Hz", status.current_frequency_hz);
        ESP_LOGI(TAG, "Duty cycle: %.2f%%", status.current_duty_cycle * 100.0f);
        ESP_LOGI(TAG, "Raw duty value: %lu", status.raw_duty_value);
        ESP_LOGI(TAG, "Last error: %s", hf_pwm_err_tToString(status.last_error));
    }
}

/**
 * @brief Configure LED channel with fade support
 * @param pwm PWM controller instance
 * @return true if successful, false otherwise
 */
bool ConfigureLedChannel(EspPwm& pwm) {
    ESP_LOGI(TAG, "Configuring LED channel...");
    
    // Set to fade mode for smooth transitions
    if (pwm.SetMode(hf_pwm_mode_t::Fade) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set fade mode");
        return false;
    }
    
    // Configure LED channel
    hf_pwm_channel_config_t led_config;
    led_config.output_pin = LED_PIN;
    led_config.frequency_hz = LED_FREQUENCY_HZ;
    led_config.resolution_bits = LED_RESOLUTION_BITS;
    led_config.output_mode = hf_pwm_output_mode_t::Normal;
    led_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    led_config.idle_state = hf_pwm_idle_state_t::Low;
    led_config.initial_duty_cycle = 0.0f;
    led_config.invert_output = false;
    
    if (pwm.ConfigureChannel(0, led_config) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure LED channel");
        return false;
    }
    
    ESP_LOGI(TAG, "LED channel configured successfully");
    return true;
}

/**
 * @brief Configure motor channels with complementary outputs
 * @param pwm PWM controller instance
 * @return true if successful, false otherwise
 */
bool ConfigureMotorChannels(EspPwm& pwm) {
    ESP_LOGI(TAG, "Configuring motor channels...");
    
    // Configure motor phase A
    hf_pwm_channel_config_t motor_a_config;
    motor_a_config.output_pin = MOTOR_A_PIN;
    motor_a_config.frequency_hz = MOTOR_FREQUENCY_HZ;
    motor_a_config.resolution_bits = MOTOR_RESOLUTION_BITS;
    motor_a_config.output_mode = hf_pwm_output_mode_t::Normal;
    motor_a_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    motor_a_config.idle_state = hf_pwm_idle_state_t::Low;
    motor_a_config.initial_duty_cycle = 0.0f;
    motor_a_config.invert_output = false;
    
    if (pwm.ConfigureChannel(1, motor_a_config) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure motor phase A");
        return false;
    }
    
    // Configure motor phase B
    hf_pwm_channel_config_t motor_b_config;
    motor_b_config.output_pin = MOTOR_B_PIN;
    motor_b_config.frequency_hz = MOTOR_FREQUENCY_HZ;
    motor_b_config.resolution_bits = MOTOR_RESOLUTION_BITS;
    motor_b_config.output_mode = hf_pwm_output_mode_t::Normal;
    motor_b_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    motor_b_config.idle_state = hf_pwm_idle_state_t::Low;
    motor_b_config.initial_duty_cycle = 0.0f;
    motor_b_config.invert_output = true; // Invert for complementary output
    
    if (pwm.ConfigureChannel(2, motor_b_config) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure motor phase B");
        return false;
    }
    
    // Set up complementary output with deadtime
    if (pwm.SetComplementaryOutput(1, 2, 1000) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set complementary output");
        return false;
    }
    
    ESP_LOGI(TAG, "Motor channels configured successfully");
    return true;
}

/**
 * @brief Configure servo channel
 * @param pwm PWM controller instance
 * @return true if successful, false otherwise
 */
bool ConfigureServoChannel(EspPwm& pwm) {
    ESP_LOGI(TAG, "Configuring servo channel...");
    
    // Configure servo channel
    hf_pwm_channel_config_t servo_config;
    servo_config.output_pin = SERVO_PIN;
    servo_config.frequency_hz = SERVO_FREQUENCY_HZ;
    servo_config.resolution_bits = SERVO_RESOLUTION_BITS;
    servo_config.output_mode = hf_pwm_output_mode_t::Normal;
    servo_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    servo_config.idle_state = hf_pwm_idle_state_t::Low;
    servo_config.initial_duty_cycle = 0.075f; // 7.5% = center position
    servo_config.invert_output = false;
    
    if (pwm.ConfigureChannel(3, servo_config) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure servo channel");
        return false;
    }
    
    ESP_LOGI(TAG, "Servo channel configured successfully");
    return true;
}

/**
 * @brief Configure audio channel
 * @param pwm PWM controller instance
 * @return true if successful, false otherwise
 */
bool ConfigureAudioChannel(EspPwm& pwm) {
    ESP_LOGI(TAG, "Configuring audio channel...");
    
    // Configure audio channel
    hf_pwm_channel_config_t audio_config;
    audio_config.output_pin = AUDIO_PIN;
    audio_config.frequency_hz = AUDIO_FREQUENCY_HZ;
    audio_config.resolution_bits = AUDIO_RESOLUTION_BITS;
    audio_config.output_mode = hf_pwm_output_mode_t::Normal;
    audio_config.alignment = hf_pwm_alignment_t::EdgeAligned;
    audio_config.idle_state = hf_pwm_idle_state_t::Low;
    audio_config.initial_duty_cycle = 0.5f; // 50% duty cycle
    audio_config.invert_output = false;
    
    if (pwm.ConfigureChannel(4, audio_config) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure audio channel");
        return false;
    }
    
    ESP_LOGI(TAG, "Audio channel configured successfully");
    return true;
}

//==============================================================================
// DEMONSTRATION FUNCTIONS
//==============================================================================

/**
 * @brief Demonstrate LED fade effects
 * @param pwm PWM controller instance
 */
void DemonstrateLedFade(EspPwm& pwm) {
    ESP_LOGI(TAG, "=== LED Fade Demonstration ===");
    
    // Enable LED channel
    if (pwm.EnableChannel(0) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable LED channel");
        return;
    }
    
    // Fade from 0% to 100% over 2 seconds
    ESP_LOGI(TAG, "Fading LED from 0%% to 100%% over 2 seconds...");
    if (pwm.SetHardwareFade(0, 1.0f, 2000) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start LED fade");
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(2500)); // Wait for fade to complete
    
    // Fade from 100% to 0% over 1 second
    ESP_LOGI(TAG, "Fading LED from 100%% to 0%% over 1 second...");
    if (pwm.SetHardwareFade(0, 0.0f, 1000) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start LED fade");
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1500)); // Wait for fade to complete
    
    // Pulse effect: fade to 50% over 500ms
    ESP_LOGI(TAG, "Pulsing LED to 50%% over 500ms...");
    if (pwm.SetHardwareFade(0, 0.5f, 500) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start LED pulse");
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for pulse to complete
}

/**
 * @brief Demonstrate motor control
 * @param pwm PWM controller instance
 */
void DemonstrateMotorControl(EspPwm& pwm) {
    ESP_LOGI(TAG, "=== Motor Control Demonstration ===");
    
    // Enable motor channels
    if (pwm.EnableChannel(1) != hf_pwm_err_t::PWM_SUCCESS ||
        pwm.EnableChannel(2) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable motor channels");
        return;
    }
    
    // Ramp up motor speed
    ESP_LOGI(TAG, "Ramping up motor speed...");
    for (float duty = 0.0f; duty <= 0.8f; duty += 0.1f) {
        if (pwm.SetDutyCycle(1, duty) != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set motor duty cycle");
            return;
        }
        ESP_LOGI(TAG, "Motor duty cycle: %.1f%%", duty * 100.0f);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Run at full speed for 2 seconds
    ESP_LOGI(TAG, "Running motor at full speed for 2 seconds...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Ramp down motor speed
    ESP_LOGI(TAG, "Ramping down motor speed...");
    for (float duty = 0.8f; duty >= 0.0f; duty -= 0.1f) {
        if (pwm.SetDutyCycle(1, duty) != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set motor duty cycle");
            return;
        }
        ESP_LOGI(TAG, "Motor duty cycle: %.1f%%", duty * 100.0f);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Demonstrate servo control
 * @param pwm PWM controller instance
 */
void DemonstrateServoControl(EspPwm& pwm) {
    ESP_LOGI(TAG, "=== Servo Control Demonstration ===");
    
    // Enable servo channel
    if (pwm.EnableChannel(3) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable servo channel");
        return;
    }
    
    // Servo positions (duty cycles for typical servo)
    const float servo_positions[] = {
        0.05f,  // 5% = 0 degrees
        0.075f, // 7.5% = 90 degrees (center)
        0.10f   // 10% = 180 degrees
    };
    
    const char* position_names[] = {"0°", "90°", "180°"};
    
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "Moving servo to %s position (%.1f%%)...", 
                 position_names[i], servo_positions[i] * 100.0f);
        
        if (pwm.SetDutyCycle(3, servo_positions[i]) != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set servo position");
            return;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for servo to move
    }
    
    // Return to center
    ESP_LOGI(TAG, "Returning servo to center position...");
    if (pwm.SetDutyCycle(3, 0.075f) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set servo position");
        return;
    }
}

/**
 * @brief Demonstrate audio generation
 * @param pwm PWM controller instance
 */
void DemonstrateAudioGeneration(EspPwm& pwm) {
    ESP_LOGI(TAG, "=== Audio Generation Demonstration ===");
    
    // Enable audio channel
    if (pwm.EnableChannel(4) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to enable audio channel");
        return;
    }
    
    // Generate different tones
    const hf_u32_t frequencies[] = {440, 494, 523, 587, 659, 698, 784}; // A4 to G5
    const char* note_names[] = {"A4", "B4", "C5", "D5", "E5", "F5", "G5"};
    
    ESP_LOGI(TAG, "Generating musical scale...");
    for (int i = 0; i < 7; i++) {
        ESP_LOGI(TAG, "Playing note %s (%lu Hz)...", note_names[i], frequencies[i]);
        
        if (pwm.SetFrequency(4, frequencies[i]) != hf_pwm_err_t::PWM_SUCCESS) {
            ESP_LOGE(TAG, "Failed to set audio frequency");
            return;
        }
        
        vTaskDelay(pdMS_TO_TICKS(500)); // Play each note for 500ms
    }
    
    // Return to original frequency
    if (pwm.SetFrequency(4, AUDIO_FREQUENCY_HZ) != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to restore audio frequency");
        return;
    }
}

/**
 * @brief Demonstrate advanced features
 * @param pwm PWM controller instance
 */
void DemonstrateAdvancedFeatures(EspPwm& pwm) {
    ESP_LOGI(TAG, "=== Advanced Features Demonstration ===");
    
    // Show current mode
    hf_pwm_mode_t current_mode = pwm.GetMode();
    ESP_LOGI(TAG, "Current PWM mode: %s", 
             (current_mode == hf_pwm_mode_t::Basic) ? "Basic" : "Fade");
    
    // Show timer assignments
    for (hf_channel_id_t ch = 0; ch < 5; ch++) {
        if (pwm.IsChannelEnabled(ch)) {
            hf_i8_t timer = pwm.GetTimerAssignment(ch);
            ESP_LOGI(TAG, "Channel %lu assigned to timer %d", ch, timer);
        }
    }
    
    // Show clock source
    hf_pwm_clock_source_t clock_source = pwm.GetClockSource();
    ESP_LOGI(TAG, "Current clock source: %d", static_cast<int>(clock_source));
    
    // Test fade status
    for (hf_channel_id_t ch = 0; ch < 5; ch++) {
        if (pwm.IsChannelEnabled(ch)) {
            bool fade_active = pwm.IsFadeActive(ch);
            ESP_LOGI(TAG, "Channel %lu fade active: %s", ch, fade_active ? "Yes" : "No");
        }
    }
}

//==============================================================================
// MAIN APPLICATION
//==============================================================================

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== Advanced PWM Example Starting ===");
    
    // Print ESP32 variant information
    ESP_LOGI(TAG, "ESP32 Variant: %s", 
#ifdef HF_MCU_ESP32C6
             "ESP32-C6"
#elif defined(HF_MCU_ESP32)
             "ESP32 Classic"
#elif defined(HF_MCU_ESP32S2)
             "ESP32-S2"
#elif defined(HF_MCU_ESP32S3)
             "ESP32-S3"
#elif defined(HF_MCU_ESP32C3)
             "ESP32-C3"
#elif defined(HF_MCU_ESP32C2)
             "ESP32-C2"
#elif defined(HF_MCU_ESP32H2)
             "ESP32-H2"
#else
             "Unknown"
#endif
    );
    
    ESP_LOGI(TAG, "PWM Configuration:");
    ESP_LOGI(TAG, "  Max Channels: %d", HF_PWM_MAX_CHANNELS);
    ESP_LOGI(TAG, "  Max Timers: %d", HF_PWM_MAX_TIMERS);
    ESP_LOGI(TAG, "  Max Resolution: %d bits", HF_PWM_MAX_RESOLUTION);
    ESP_LOGI(TAG, "  Frequency Range: %lu - %lu Hz", HF_PWM_MIN_FREQUENCY, HF_PWM_MAX_FREQUENCY);
    
    // Create PWM unit configuration
    hf_pwm_unit_config_t pwm_config;
    pwm_config.unit_id = 0;
    pwm_config.mode = hf_pwm_mode_t::Fade;  // Enable fade mode
    pwm_config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
    pwm_config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
    pwm_config.enable_fade = true;
    pwm_config.enable_interrupts = true;
    
    // Create PWM controller
    g_pwm_controller = new EspPwm(pwm_config);
    if (!g_pwm_controller) {
        ESP_LOGE(TAG, "Failed to create PWM controller");
        return;
    }
    
    // Initialize PWM system
    if (!g_pwm_controller->EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PWM system");
        delete g_pwm_controller;
        return;
    }
    
    ESP_LOGI(TAG, "PWM system initialized successfully");
    
    // Set up callbacks
    g_pwm_controller->SetPeriodCallback(PeriodCompleteCallback);
    g_pwm_controller->SetFaultCallback(FaultCallback);
    
    // Configure all channels
    if (!ConfigureLedChannel(*g_pwm_controller) ||
        !ConfigureMotorChannels(*g_pwm_controller) ||
        !ConfigureServoChannel(*g_pwm_controller) ||
        !ConfigureAudioChannel(*g_pwm_controller)) {
        ESP_LOGE(TAG, "Failed to configure channels");
        delete g_pwm_controller;
        return;
    }
    
    // Print initial diagnostics
    PrintDiagnostics(*g_pwm_controller);
    
    // Start all channels
    if (g_pwm_controller->StartAll() != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to start all channels");
        delete g_pwm_controller;
        return;
    }
    
    ESP_LOGI(TAG, "All channels started successfully");
    
    // Run demonstrations
    DemonstrateLedFade(*g_pwm_controller);
    DemonstrateMotorControl(*g_pwm_controller);
    DemonstrateServoControl(*g_pwm_controller);
    DemonstrateAudioGeneration(*g_pwm_controller);
    DemonstrateAdvancedFeatures(*g_pwm_controller);
    
    // Print final statistics and diagnostics
    ESP_LOGI(TAG, "=== Final Status ===");
    PrintStatistics(*g_pwm_controller);
    PrintDiagnostics(*g_pwm_controller);
    
    // Print individual channel status
    for (hf_channel_id_t ch = 0; ch < 5; ch++) {
        if (g_pwm_controller->IsChannelEnabled(ch)) {
            PrintChannelStatus(*g_pwm_controller, ch);
        }
    }
    
    // Stop all channels
    if (g_pwm_controller->StopAll() != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to stop all channels");
    }
    
    ESP_LOGI(TAG, "=== Advanced PWM Example Complete ===");
    ESP_LOGI(TAG, "Total periods completed: %lu", g_period_count);
    
    // Clean up
    delete g_pwm_controller;
    g_pwm_controller = nullptr;
    
    ESP_LOGI(TAG, "Example completed successfully!");
} 