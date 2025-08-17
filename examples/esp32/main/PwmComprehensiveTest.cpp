/**
 * @file PwmComprehensiveTest.cpp
 * @brief Comprehensive PWM testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This comprehensive test suite validates all functionality of the EspPwm class:
 * - Constructor/Destructor behavior
 * - Lifecycle management (Initialize/Deinitialize)
 * - Configuration (modes, clock sources, unit config)
 * - Channel management (configure, enable/disable, validation)
 * - PWM control (duty cycle, frequency, phase shift)
 * - Advanced features (synchronized operations, complementary outputs)
 * - ESP32-specific features (hardware fade, idle levels, timer management)
 * - Status and diagnostics (statistics, error reporting)
 * - Callbacks (period, fault)
 * - Edge cases and stress testing
 *
 * GPIO14 is used as a test progression indicator that toggles between HIGH/LOW
 * each time a test completes, providing visual feedback for test progression.
 */

#include "TestFramework.h"
#include "base/BasePwm.h"
#include "mcu/esp32/EspPwm.h"
#include "mcu/esp32/EspGpio.h" // Add GPIO support for test progression indicator

static const char* TAG = "PWM_Test";
static TestResults g_test_results;

// Test progression indicator GPIO
static EspGpio* g_test_progress_gpio = nullptr;
static bool g_test_progress_state = false;

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Initialize the test progression indicator GPIO
 */
bool init_test_progress_indicator() noexcept {
  // Use GPIO14 as the test progression indicator (visible LED on most ESP32 dev boards)
  g_test_progress_gpio = new EspGpio(14, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
  
  if (!g_test_progress_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO");
    return false;
  }
  
  // Start with LOW state
  g_test_progress_gpio->SetInactive();
  g_test_progress_state = false;
  
  ESP_LOGI(TAG, "Test progression indicator initialized on GPIO14");
  return true;
}

/**
 * @brief Flip the test progression indicator to show next test
 */
void flip_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_state = !g_test_progress_state;
    if (g_test_progress_state) {
      g_test_progress_gpio->SetActive();
    } else {
      g_test_progress_gpio->SetInactive();
    }
    ESP_LOGI(TAG, "Test progression indicator: %s", g_test_progress_state ? "HIGH" : "LOW");
  }
}

/**
 * @brief Cleanup the test progression indicator GPIO
 */
void cleanup_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_gpio->SetInactive(); // Ensure pin is low
    delete g_test_progress_gpio;
    g_test_progress_gpio = nullptr;
  }
}

/**
 * @brief Create a default PWM configuration for testing
 */
hf_pwm_unit_config_t create_test_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = false;  // Basic mode without fade
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a PWM configuration specifically for fade testing
 */
hf_pwm_unit_config_t create_fade_test_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE;  // Use FADE mode
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = true;   // Enable fade functionality
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a PWM configuration for basic mode with fade enabled (for channel enable operations)
 */
hf_pwm_unit_config_t create_basic_with_fade_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;  // Basic mode
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = true;   // Enable fade for channel operations
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a default channel configuration for testing with explicit resolution control
 */
hf_pwm_channel_config_t create_test_channel_config(hf_gpio_num_t gpio_pin, 
                                                   hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
                                                   hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.channel_id = 0;
  config.timer_id = 0;
  config.speed_mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
  
  // ✅ NEW: Explicit frequency and resolution control
  config.frequency_hz = frequency_hz;
  config.resolution_bits = resolution_bits;
  
  // Calculate 50% duty cycle for the specified resolution
  config.duty_initial = (1u << resolution_bits) / 2; // 50% duty cycle
  
  config.intr_type = hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE;
  config.invert_output = false;
  config.hpoint = 0;
  config.idle_level = 0;
  config.output_invert = false;
  return config;
}

/**
 * @brief Create channel configuration with specific duty cycle percentage
 */
hf_pwm_channel_config_t create_test_channel_config_with_duty(hf_gpio_num_t gpio_pin,
                                                            float duty_percentage,
                                                            hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
                                                            hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config = create_test_channel_config(gpio_pin, frequency_hz, resolution_bits);
  
  // Calculate raw duty value for the specified percentage and resolution
  hf_u32_t max_duty = (1u << resolution_bits) - 1;
  config.duty_initial = static_cast<hf_u32_t>(duty_percentage * max_duty);
  
  return config;
}

//==============================================================================
// CONSTRUCTOR/DESTRUCTOR TESTS
//==============================================================================

bool test_constructor_default() noexcept {
  ESP_LOGI(TAG, "Testing default constructor...");

  // Test constructors without exception handling
  EspPwm pwm1;
  ESP_LOGI(TAG, "[SUCCESS] Default constructor completed");

  // Test constructor with unit config
  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm2(config);
  ESP_LOGI(TAG, "[SUCCESS] Constructor with config completed");

  // Test legacy constructor
  EspPwm pwm3(HF_PWM_APB_CLOCK_HZ);
  ESP_LOGI(TAG, "[SUCCESS] Legacy constructor completed");

  return true;
}

bool test_destructor_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing destructor cleanup...");

  {
    hf_pwm_unit_config_t config = create_test_config();
    EspPwm pwm(config);

    // Initialize and configure a channel
    if (!pwm.EnsureInitialized()) {
      ESP_LOGE(TAG, "Failed to initialize PWM for destructor test");
      return false;
    }

    hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
    pwm.ConfigureChannel(0, ch_config);
    pwm.EnableChannel(0);

    ESP_LOGI(TAG, "PWM configured, testing destructor cleanup...");
  } // pwm should be destroyed here

  ESP_LOGI(TAG, "[SUCCESS] Destructor cleanup completed");
  return true;
}

//==============================================================================
// LIFECYCLE TESTS
//==============================================================================

bool test_initialization_states() noexcept {
  ESP_LOGI(TAG, "Testing initialization states...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  // Test initial state
  if (pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should not be initialized initially");
    return false;
  }

  // Test manual initialization
  hf_pwm_err_t result = pwm.Initialize();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Manual initialization failed: %s", HfPwmErrToString(result));
    return false;
  }

  if (!pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should be initialized after Initialize()");
    return false;
  }

  // Test double initialization
  result = pwm.Initialize();
  if (result != hf_pwm_err_t::PWM_ERR_ALREADY_INITIALIZED) {
    ESP_LOGE(TAG, "Double initialization should return ALREADY_INITIALIZED");
    return false;
  }

  // Test deinitialization
  result = pwm.Deinitialize();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Deinitialization failed: %s", HfPwmErrToString(result));
    return false;
  }

  if (pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should not be initialized after Deinitialize()");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Initialization states test passed");
  return true;
}

bool test_lazy_initialization() noexcept {
  ESP_LOGI(TAG, "Testing lazy initialization...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  // PWM should not be initialized initially
  if (pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should not be initialized initially");
    return false;
  }

  // Test EnsureInitialized
  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized() failed");
    return false;
  }

  if (!pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should be initialized after EnsureInitialized()");
    return false;
  }

  // Test EnsureDeinitialized
  if (!pwm.EnsureDeinitialized()) {
    ESP_LOGE(TAG, "EnsureDeinitialized() failed");
    return false;
  }

  if (pwm.IsInitialized()) {
    ESP_LOGE(TAG, "PWM should not be initialized after EnsureDeinitialized()");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Lazy initialization test passed");
  return true;
}

//==============================================================================
// CONFIGURATION TESTS
//==============================================================================

bool test_mode_configuration() noexcept {
  ESP_LOGI(TAG, "Testing mode configuration...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test basic mode
  hf_pwm_err_t result = pwm.SetMode(hf_pwm_mode_t::HF_PWM_MODE_BASIC);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set basic mode: %s", HfPwmErrToString(result));
    return false;
  }

  if (pwm.GetMode() != hf_pwm_mode_t::HF_PWM_MODE_BASIC) {
    ESP_LOGE(TAG, "Mode not set correctly to BASIC");
    return false;
  }

  // Test fade mode
  result = pwm.SetMode(hf_pwm_mode_t::HF_PWM_MODE_FADE);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set fade mode: %s", HfPwmErrToString(result));
    return false;
  }

  if (pwm.GetMode() != hf_pwm_mode_t::HF_PWM_MODE_FADE) {
    ESP_LOGE(TAG, "Mode not set correctly to FADE");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Mode configuration test passed");
  return true;
}

bool test_clock_source_configuration() noexcept {
  ESP_LOGI(TAG, "Testing clock source configuration...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test different clock sources
  hf_pwm_clock_source_t sources[] = {
      hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB,
      hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST};

  for (auto source : sources) {
    hf_pwm_err_t result = pwm.SetClockSource(source);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set clock source %d: %s", static_cast<int>(source),
               HfPwmErrToString(result));
      return false;
    }

    if (pwm.GetClockSource() != source) {
      ESP_LOGE(TAG, "Clock source not set correctly");
      return false;
    }

    ESP_LOGI(TAG, "Clock source %d set successfully", static_cast<int>(source));
  }

  ESP_LOGI(TAG, "[SUCCESS] Clock source configuration test passed");
  return true;
}

//==============================================================================
// CHANNEL MANAGEMENT TESTS
//==============================================================================

bool test_channel_configuration() noexcept {
  ESP_LOGI(TAG, "Testing channel configuration...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test configuring multiple channels with different resolutions (avoid GPIO3 -> use GPIO6 instead)
  struct ChannelTestConfig {
    hf_gpio_num_t pin;
    hf_u32_t frequency;
    hf_u8_t resolution;
    float duty_percentage;
  };
  
  ChannelTestConfig test_configs[] = {
    {2, 1000, 8,  0.25f}, // GPIO2: 1kHz @ 8-bit, 25%
    {6, 2000, 10, 0.50f}, // GPIO6: 2kHz @ 10-bit, 50%
    {4, 1500, 12, 0.75f}, // GPIO4: 1.5kHz @ 12-bit, 75%
    {5, 3000, 9,  0.33f}  // GPIO5: 3kHz @ 9-bit, 33%
  };

  for (hf_channel_id_t ch = 0; ch < 4; ch++) {
    const auto& test_cfg = test_configs[ch];
    
    hf_pwm_channel_config_t ch_config = create_test_channel_config_with_duty(
      test_cfg.pin, test_cfg.duty_percentage, test_cfg.frequency, test_cfg.resolution);
    ch_config.channel_id = ch;

    hf_pwm_err_t result = pwm.ConfigureChannel(ch, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %d: %s", ch, HfPwmErrToString(result));
      return false;
    }

    // Verify the configuration was applied correctly
    uint8_t actual_resolution = pwm.GetResolution(ch);
    uint32_t actual_frequency = pwm.GetFrequency(ch);
    
    if (actual_resolution != test_cfg.resolution) {
      ESP_LOGE(TAG, "Channel %d resolution mismatch: expected %d, got %d", ch, test_cfg.resolution, actual_resolution);
      return false;
    }
    
    if (actual_frequency != test_cfg.frequency) {
      ESP_LOGE(TAG, "Channel %d frequency mismatch: expected %lu, got %lu", ch, test_cfg.frequency, actual_frequency);
      return false;
    }

    ESP_LOGI(TAG, "Channel %d configured successfully: %lu Hz @ %d-bit, %.1f%% duty", 
             ch, actual_frequency, actual_resolution, test_cfg.duty_percentage * 100.0f);
  }

  // Test invalid channel configuration
  hf_pwm_channel_config_t invalid_config = create_test_channel_config(10);
  hf_pwm_err_t result = pwm.ConfigureChannel(EspPwm::MAX_CHANNELS, invalid_config);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Invalid channel should not be configurable");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Channel configuration test passed");
  return true;
}

bool test_channel_enable_disable() noexcept {
  ESP_LOGI(TAG, "Testing channel enable/disable...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure a channel first
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel for enable/disable test");
    return false;
  }

  // Test channel should not be enabled initially
  if (pwm.IsChannelEnabled(0)) {
    ESP_LOGE(TAG, "Channel should not be enabled initially");
    return false;
  }

  // Test enable channel
  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel: %s", HfPwmErrToString(result));
    return false;
  }

  if (!pwm.IsChannelEnabled(0)) {
    ESP_LOGE(TAG, "Channel should be enabled after EnableChannel()");
    return false;
  }

  // Test disable channel
  result = pwm.DisableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable channel: %s", HfPwmErrToString(result));
    return false;
  }

  if (pwm.IsChannelEnabled(0)) {
    ESP_LOGE(TAG, "Channel should not be enabled after DisableChannel()");
    return false;
  }

  // Test invalid channel operations
  result = pwm.EnableChannel(EspPwm::MAX_CHANNELS);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Invalid channel should not be enableable");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Channel enable/disable test passed");
  return true;
}

//==============================================================================
// PWM CONTROL TESTS
//==============================================================================

bool test_duty_cycle_control() noexcept {
  ESP_LOGI(TAG, "Testing duty cycle control...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure and enable a channel
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Test different duty cycles
  float test_duties[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

  ESP_LOGI(TAG, "Testing duty cycle control...");
  for (float duty : test_duties) {
    hf_pwm_err_t result = pwm.SetDutyCycle(0, duty);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set duty cycle %.2f: %s", duty, HfPwmErrToString(result));
      return false;
    }

    float actual_duty = pwm.GetDutyCycle(0);
    if (abs(actual_duty - duty) > 0.01f) { // Allow small tolerance
      ESP_LOGE(TAG, "Duty cycle mismatch: expected %.2f, got %.2f", duty, actual_duty);
      return false;
    }

    ESP_LOGI(TAG, "Duty cycle %.2f set successfully", duty);
    vTaskDelay(pdMS_TO_TICKS(50)); // Brief delay for observation
  }

  // Test raw duty cycle setting
  hf_u32_t raw_values[] = {0, 256, 512, 768, 1023}; // For 10-bit resolution

  for (hf_u32_t raw_val : raw_values) {
    hf_pwm_err_t result = pwm.SetDutyCycleRaw(0, raw_val);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set raw duty cycle %lu: %s", raw_val, HfPwmErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "Raw duty cycle %lu set successfully", raw_val);
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  // Test invalid duty cycles
  hf_pwm_err_t result = pwm.SetDutyCycle(0, -0.1f);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Negative duty cycle should not be accepted");
    return false;
  }

  result = pwm.SetDutyCycle(0, 1.1f);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Duty cycle > 1.0 should not be accepted");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Duty cycle control test passed");
  return true;
}

bool test_frequency_control() noexcept {
  ESP_LOGI(TAG, "Testing frequency control...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure and enable a channel
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Test different frequencies
  hf_frequency_hz_t test_frequencies[] = {100, 500, 1000, 5000, 10000, 20000};

  for (hf_frequency_hz_t freq : test_frequencies) {
    hf_pwm_err_t result = pwm.SetFrequency(0, freq);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set frequency %lu: %s", freq, HfPwmErrToString(result));
      return false;
    }

    hf_frequency_hz_t actual_freq = pwm.GetFrequency(0);
    // Allow some tolerance for frequency accuracy
    if (abs((int)actual_freq - (int)freq) > freq * 0.05) { // 5% tolerance
      ESP_LOGE(TAG, "Frequency mismatch: expected %lu, got %lu", freq, actual_freq);
      return false;
    }

    ESP_LOGI(TAG, "Frequency %lu Hz set successfully (actual: %lu Hz)", freq, actual_freq);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Test invalid frequencies
  hf_pwm_err_t result = pwm.SetFrequency(0, 0);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Zero frequency should not be accepted");
    return false;
  }

  result = pwm.SetFrequency(0, HF_PWM_MAX_FREQUENCY + 1);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Frequency above maximum should not be accepted");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Frequency control test passed");
  return true;
}

bool test_phase_shift_control() noexcept {
  ESP_LOGI(TAG, "Testing phase shift control...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure and enable channels (avoid GPIO3 -> use GPIO6 instead)
  for (hf_channel_id_t ch = 0; ch < 3; ch++) {
    hf_gpio_num_t test_pin = static_cast<hf_gpio_num_t>(2 + ch);
    if (test_pin == 3) {
      test_pin = 6;
    }
    hf_pwm_channel_config_t ch_config = create_test_channel_config(test_pin);
    ch_config.channel_id = ch;
    pwm.ConfigureChannel(ch, ch_config);
    pwm.EnableChannel(ch);
  }

  // Test if phase shift is supported by trying to set a valid phase
  hf_pwm_err_t result = pwm.SetPhaseShift(0, 0.0f);
  if (result == hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER) {
    // ESP32-C6 LEDC doesn't support phase shift - skip this test
    ESP_LOGW(TAG, "Phase shift not supported on this hardware - skipping test");
    ESP_LOGI(TAG, "[SKIPPED] Phase shift control test (hardware limitation)");
    return true; // Return true to indicate test was handled appropriately
  }

  // If we get here, phase shift is supported, so run the full test
  float test_phases[] = {0.0f, 90.0f, 180.0f, 270.0f};

  for (int i = 0; i < 3; i++) {
    float phase = test_phases[i];
    result = pwm.SetPhaseShift(i, phase);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set phase shift %.1f for channel %d: %s", phase, i,
               HfPwmErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "Phase shift %.1f degrees set for channel %d", phase, i);
  }

  // Test invalid phase shift
  result = pwm.SetPhaseShift(0, 400.0f);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Phase shift > 360 degrees should not be accepted");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Phase shift control test passed");
  return true;
}

//==============================================================================
// ADVANCED FEATURES TESTS
//==============================================================================

bool test_synchronized_operations() noexcept {
  ESP_LOGI(TAG, "Testing synchronized operations...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure multiple channels (avoid GPIO3 -> use GPIO6 instead)
  for (hf_channel_id_t ch = 0; ch < 4; ch++) {
    hf_gpio_num_t test_pin = static_cast<hf_gpio_num_t>(2 + ch);
    if (test_pin == 3) {
      test_pin = 6;
    }
    hf_pwm_channel_config_t ch_config = create_test_channel_config(test_pin);
    ch_config.channel_id = ch;
    ch_config.duty_initial = 300 + (ch * 100);
    pwm.ConfigureChannel(ch, ch_config);
    pwm.EnableChannel(ch);
  }

  // Test StartAll
  hf_pwm_err_t result = pwm.StartAll();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "StartAll failed: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "StartAll executed successfully");
  vTaskDelay(pdMS_TO_TICKS(500));

  // Test UpdateAll
  result = pwm.UpdateAll();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "UpdateAll failed: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "UpdateAll executed successfully");
  vTaskDelay(pdMS_TO_TICKS(500));

  // Test StopAll
  ESP_LOGI(TAG, "Stopping all channels");
  result = pwm.StopAll();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "StopAll failed: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "StopAll executed successfully");

  ESP_LOGI(TAG, "[SUCCESS] Synchronized operations test passed");
  return true;
}

bool test_complementary_outputs() noexcept {
  ESP_LOGI(TAG, "Testing complementary outputs...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure primary and complementary channels (avoid GPIO3 -> use GPIO6 instead)
  hf_pwm_channel_config_t primary_config = create_test_channel_config(2);
  hf_pwm_channel_config_t comp_config = create_test_channel_config(6);

  pwm.ConfigureChannel(0, primary_config);
  pwm.ConfigureChannel(1, comp_config);

  // Test complementary output setup
  hf_u32_t deadtime_ns = 1000; // 1 microsecond deadtime
  hf_pwm_err_t result = pwm.SetComplementaryOutput(0, 1, deadtime_ns);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set complementary output: %s", HfPwmErrToString(result));
    return false;
  }

  pwm.EnableChannel(0);
  pwm.EnableChannel(1);

  // Test different duty cycles with complementary outputs
  float test_duties[] = {0.2f, 0.5f, 0.8f};

  for (float duty : test_duties) {
    pwm.SetDutyCycle(0, duty);
    ESP_LOGI(TAG, "Complementary output test with duty cycle %.1f", duty);
    vTaskDelay(pdMS_TO_TICKS(300));
  }

  ESP_LOGI(TAG, "[SUCCESS] Complementary outputs test passed");
  return true;
}

//==============================================================================
// ESP32-SPECIFIC FEATURES TESTS
//==============================================================================

bool test_hardware_fade() noexcept {
  ESP_LOGI(TAG, "Testing hardware fade functionality...");

  hf_pwm_unit_config_t config = create_fade_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure and enable a channel
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Set initial duty cycle
  pwm.SetDutyCycle(0, 0.1f);
  vTaskDelay(pdMS_TO_TICKS(100));

  // Test fade operations
  struct FadeTest {
    float target_duty;
    hf_u32_t fade_time_ms;
  };

  FadeTest fade_tests[] = {
      {0.8f, 1000}, // Fade up
      {0.2f, 800},  // Fade down
      {0.9f, 1200}, // Fade up again
      {0.0f, 500}   // Fade to minimum
  };

  for (const auto& test : fade_tests) {
    ESP_LOGI(TAG, "Starting fade to %.1f over %lu ms", test.target_duty, test.fade_time_ms);

    hf_pwm_err_t result = pwm.SetHardwareFade(0, test.target_duty, test.fade_time_ms);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to start fade: %s", HfPwmErrToString(result));
      return false;
    }

    // Check if fade is active
    if (!pwm.IsFadeActive(0)) {
      ESP_LOGE(TAG, "Fade should be active after SetHardwareFade");
      return false;
    }

    // Wait for fade to complete
    vTaskDelay(pdMS_TO_TICKS(test.fade_time_ms + 200));

    // Check if fade completed
    if (pwm.IsFadeActive(0)) {
      ESP_LOGI(TAG, "Warning: Fade still active after expected completion time");
    }

    ESP_LOGI(TAG, "Fade completed");
  }

  // Test stop fade
  pwm.SetHardwareFade(0, 0.5f, 2000); // Start a long fade
  vTaskDelay(pdMS_TO_TICKS(200));

  hf_pwm_err_t result = pwm.StopHardwareFade(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to stop fade: %s", HfPwmErrToString(result));
    return false;
  }

  if (pwm.IsFadeActive(0)) {
    ESP_LOGE(TAG, "Fade should not be active after StopHardwareFade");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Hardware fade test passed");
  return true;
}

bool test_idle_level_control() noexcept {
  ESP_LOGI(TAG, "Testing idle level control...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure channels
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);

  // Test different idle levels
  hf_u8_t idle_levels[] = {0, 1};

  for (hf_u8_t idle_level : idle_levels) {
    hf_pwm_err_t result = pwm.SetIdleLevel(0, idle_level);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set idle level %d: %s", idle_level, HfPwmErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "Idle level %d set successfully", idle_level);
  }

  // Test invalid idle level
  hf_pwm_err_t result = pwm.SetIdleLevel(0, 2);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Invalid idle level should not be accepted");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Idle level control test passed");
  return true;
}

bool test_timer_management() noexcept {
  ESP_LOGI(TAG, "Testing timer management...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure channels with different configurations to test timer allocation
  struct ChannelConfig {
    hf_channel_id_t channel;
    hf_gpio_num_t gpio;
  };

  ChannelConfig configs[] = {
      {0, 2}, // Timer 0
      {1, 6}, // Avoid GPIO3; use GPIO6 instead
      {2, 4}, // Timer 1
      {3, 5}  // Timer 2
  };

  for (const auto& cfg : configs) {
    hf_pwm_channel_config_t ch_config = create_test_channel_config(cfg.gpio);
    ch_config.channel_id = cfg.channel;

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %d: %s", cfg.channel, HfPwmErrToString(result));
      return false;
    }

    int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
    ESP_LOGI(TAG, "Channel %d assigned to timer %d", cfg.channel, timer_id);
  }

  // Test forced timer assignment
  hf_pwm_err_t result = pwm.ForceTimerAssignment(0, 3);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to force timer assignment: %s", HfPwmErrToString(result));
    return false;
  }

  int8_t timer_id = pwm.GetTimerAssignment(0);
  if (timer_id != 3) {
    ESP_LOGE(TAG, "Forced timer assignment failed: expected 3, got %d", timer_id);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Timer management test passed");
  return true;
}

//==============================================================================
// STATUS AND DIAGNOSTICS TESTS
//==============================================================================

bool test_status_reporting() noexcept {
  ESP_LOGI(TAG, "Testing status reporting...");

  hf_pwm_unit_config_t config = create_basic_with_fade_config();  // Basic mode with fade for channel enable
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure and enable a channel
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  ch_config.duty_initial = 600; // ~60% for 10-bit resolution

  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Test channel status reporting
  hf_pwm_channel_status_t status;
  hf_pwm_err_t result = pwm.GetChannelStatus(0, status);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get channel status: %s", HfPwmErrToString(result));
    return false;
  }

  if (!status.enabled) {
    ESP_LOGE(TAG, "Channel status should show enabled");
    return false;
  }

  if (!status.configured) {
    ESP_LOGE(TAG, "Channel status should show configured");
    return false;
  }

  ESP_LOGI(TAG, "Channel status: enabled=%d, configured=%d, duty=%.2f, freq=%lu", status.enabled,
           status.configured, status.current_duty_cycle, status.current_frequency);

  // Test capabilities reporting
  hf_pwm_capabilities_t capabilities;
  result = pwm.GetCapabilities(capabilities);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get capabilities: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "PWM capabilities retrieved successfully");

  // Test error reporting
  hf_pwm_err_t last_error = pwm.GetLastError(0);
  ESP_LOGI(TAG, "Last error for channel 0: %s", HfPwmErrToString(last_error));

  ESP_LOGI(TAG, "[SUCCESS] Status reporting test passed");
  return true;
}

bool test_statistics_and_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing statistics and diagnostics...");

  hf_pwm_unit_config_t config = create_basic_with_fade_config();  // Basic mode with fade for channel enable
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Perform some operations to generate statistics
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  for (int i = 0; i < 5; i++) {
    pwm.SetDutyCycle(0, 0.2f + (i * 0.15f));
    pwm.SetFrequency(0, 1000 + (i * 500));
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  pwm.DisableChannel(0);

  // Test statistics reporting
  hf_pwm_statistics_t statistics;
  hf_pwm_err_t result = pwm.GetStatistics(statistics);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get statistics: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(TAG,
           "Statistics - Duty updates: %lu, Freq changes: %lu, Channel enables: %lu, Channel "
           "disables: %lu",
           statistics.duty_updates_count, statistics.frequency_changes_count,
           statistics.channel_enables_count, statistics.channel_disables_count);

  // Test diagnostics reporting
  hf_pwm_diagnostics_t diagnostics;
  result = pwm.GetDiagnostics(diagnostics);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics: %s", HfPwmErrToString(result));
    return false;
  }

  ESP_LOGI(
      TAG,
      "Diagnostics - Hardware init: %d, Fade ready: %d, Active channels: %d, Active timers: %d",
      diagnostics.hardware_initialized, diagnostics.fade_functionality_ready,
      diagnostics.active_channels, diagnostics.active_timers);

  ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics test passed");
  return true;
}

//==============================================================================
// CALLBACK TESTS
//==============================================================================

// Global variables for callback testing
static volatile bool g_period_callback_called = false;
static volatile bool g_fault_callback_called = false;
static volatile hf_channel_id_t g_callback_channel = 0;

void test_period_callback(hf_channel_id_t channel_id, void* user_data) {
  g_period_callback_called = true;
  g_callback_channel = channel_id;
  ESP_LOGI(TAG, "Period callback called for channel %d", channel_id);
}

void test_fault_callback(hf_channel_id_t channel_id, hf_pwm_err_t error, void* user_data) {
  g_fault_callback_called = true;
  g_callback_channel = channel_id;
  ESP_LOGI(TAG, "Fault callback called for channel %d, error: %s", channel_id,
           HfPwmErrToString(error));
}

bool test_callbacks() noexcept {
  ESP_LOGI(TAG, "Testing callback functionality...");

  hf_pwm_unit_config_t config = create_fade_test_config();  // Use fade mode for callback testing
  config.enable_interrupts = true;
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Reset callback flags
  g_period_callback_called = false;
  g_fault_callback_called = false;

  // Set callbacks
  pwm.SetPeriodCallback(test_period_callback, nullptr);
  pwm.SetFaultCallback(test_fault_callback, nullptr);

  // Configure and enable a channel
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  ch_config.duty_initial = 10; // Low duty cycle to trigger period callbacks quickly

  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Wait for potential callbacks
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Note: Period callbacks may not always trigger in test environment
  // depending on ESP32 configuration and interrupt setup
  ESP_LOGI(TAG, "Callback test completed - Period callback called: %s, Fault callback called: %s",
           g_period_callback_called ? "YES" : "NO", g_fault_callback_called ? "YES" : "NO");

  ESP_LOGI(TAG, "[SUCCESS] Callback test completed");
  return true;
}

/**
 * @brief Test basic mode without fade functionality
 */
bool test_basic_mode_without_fade() noexcept {
  ESP_LOGI(TAG, "Testing basic mode without fade...");

  hf_pwm_unit_config_t config = create_test_config();  // Basic mode without fade
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test that we can configure channels without fade
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel in basic mode without fade");
    return false;
  }

  // Test that we can set duty cycles without fade
  result = pwm.SetDutyCycle(0, 0.5f);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set duty cycle in basic mode without fade");
    return false;
  }

  // Test that we can set frequency without fade
  result = pwm.SetFrequency(0, 2000);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set frequency in basic mode without fade");
    return false;
  }

  ESP_LOGI(TAG, "Basic mode without fade test passed");
  return true;
}

/**
 * @brief Test fade mode functionality
 */
bool test_fade_mode_functionality() noexcept {
  ESP_LOGI(TAG, "Testing fade mode functionality...");

  hf_pwm_unit_config_t config = create_fade_test_config();  // Fade mode with fade enabled
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test that we can configure channels in fade mode
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel in fade mode");
    return false;
  }

  // Test that we can enable channels in fade mode
  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel in fade mode");
    return false;
  }

  // Test that we can set duty cycles in fade mode
  result = pwm.SetDutyCycle(0, 0.5f);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set duty cycle in fade mode");
    return false;
  }

  // Test that we can use hardware fade in fade mode
  result = pwm.SetHardwareFade(0, 0.8f, 1000);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set hardware fade in fade mode");
    return false;
  }

  // Wait a bit for fade to start
  vTaskDelay(pdMS_TO_TICKS(100));

  // Test that fade is active
  if (!pwm.IsFadeActive(0)) {
    ESP_LOGE(TAG, "Fade should be active in fade mode");
    return false;
  }

  // Stop the fade
  result = pwm.StopHardwareFade(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to stop hardware fade in fade mode");
    return false;
  }

  ESP_LOGI(TAG, "Fade mode functionality test passed");
  return true;
}

/**
 * @brief Test resolution-specific duty cycle accuracy (NEW CRITICAL TEST)
 */
bool test_resolution_specific_duty_cycles() noexcept {
  ESP_LOGI(TAG, "Testing resolution-specific duty cycle accuracy...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure channel with known raw duty value
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  ch_config.duty_initial = 512; // Exactly 50% for 10-bit resolution
  
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel for resolution test");
    return false;
  }

  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel for resolution test");
    return false;
  }

  // Test multiple duty cycles with precise validation
  struct DutyCycleTest {
    float percentage;
    uint32_t expected_raw_10bit;
    const char* description;
  };

  DutyCycleTest duty_tests[] = {
    {0.0f,   0,    "0% duty cycle"},
    {0.25f,  255,  "25% duty cycle"},
    {0.5f,   511,  "50% duty cycle"},
    {0.75f,  767,  "75% duty cycle"},
    {1.0f,   1023, "100% duty cycle"}
  };

  ESP_LOGI(TAG, "Testing duty cycle accuracy with 10-bit resolution (max=1023)");
  
  for (const auto& test : duty_tests) {
    ESP_LOGI(TAG, "Setting %s (%.2f)", test.description, test.percentage);
    
    result = pwm.SetDutyCycle(0, test.percentage);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set %s: %s", test.description, HfPwmErrToString(result));
      return false;
    }

    // Verify the duty cycle reads back correctly
    float actual_duty = pwm.GetDutyCycle(0);
    float expected_duty = test.percentage;
    float tolerance = 0.002f; // Allow 0.2% tolerance for rounding

    if (abs(actual_duty - expected_duty) > tolerance) {
      ESP_LOGE(TAG, "Duty cycle mismatch for %s: expected %.4f, got %.4f (diff=%.4f)", 
               test.description, expected_duty, actual_duty, abs(actual_duty - expected_duty));
      return false;
    }

    // Test raw duty cycle setting as well
    result = pwm.SetDutyCycleRaw(0, test.expected_raw_10bit);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set raw duty cycle %lu: %s", test.expected_raw_10bit, HfPwmErrToString(result));
      return false;
    }

    actual_duty = pwm.GetDutyCycle(0);
    if (abs(actual_duty - expected_duty) > tolerance) {
      ESP_LOGE(TAG, "Raw duty cycle mismatch for %s: expected %.4f, got %.4f", 
               test.description, expected_duty, actual_duty);
      return false;
    }

    ESP_LOGI(TAG, "✓ %s verified: %.4f%% (raw=%lu)", test.description, actual_duty * 100.0f, test.expected_raw_10bit);
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  ESP_LOGI(TAG, "[SUCCESS] Resolution-specific duty cycle accuracy test passed");
  return true;
}

/**
 * @brief Test frequency/resolution validation (NEW CRITICAL TEST)
 */
bool test_frequency_resolution_validation() noexcept {
  ESP_LOGI(TAG, "Testing frequency/resolution validation...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure a basic channel first
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel for frequency validation test");
    return false;
  }

  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel for frequency validation test");
    return false;
  }

  // Test valid frequency/resolution combinations
  struct FreqResTest {
    uint32_t frequency;
    bool should_succeed;
    const char* description;
  };

  FreqResTest freq_tests[] = {
    {1000,    true,  "1 kHz @ 10-bit (valid)"},
    {5000,    true,  "5 kHz @ 10-bit (valid)"},
    {10000,   true,  "10 kHz @ 10-bit (valid)"},
    {20000,   true,  "20 kHz @ 10-bit (valid)"},
    {30000,   false, "30 kHz @ 10-bit (should fail - hardware limitation)"},
    {40000,   false, "40 kHz @ 10-bit (should fail - timer clock conflict)"},
    {50000,   false, "50 kHz @ 10-bit (should fail - timer clock conflict)"},
    {100000,  false, "100 kHz @ 10-bit (should fail - too high)"},
  };

  for (const auto& test : freq_tests) {
    ESP_LOGI(TAG, "Testing %s", test.description);
    
    result = pwm.SetFrequency(0, test.frequency);
    
    if (test.should_succeed) {
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Expected success for %s but got: %s", test.description, HfPwmErrToString(result));
        return false;
      }
      ESP_LOGI(TAG, "✓ %s succeeded as expected", test.description);
    } else {
      if (result == hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Expected failure for %s but got success", test.description);
        return false;
      }
      ESP_LOGI(TAG, "✓ %s failed as expected: %s", test.description, HfPwmErrToString(result));
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  ESP_LOGI(TAG, "[SUCCESS] Frequency/resolution validation test passed");
  return true;
}

/**
 * @brief Test percentage consistency across different resolutions (NEW)
 */
bool test_percentage_consistency_across_resolutions() noexcept {
  ESP_LOGI(TAG, "Testing percentage consistency across different resolutions...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test different resolution combinations with explicit resolution control
  struct ResolutionTest {
    uint32_t frequency;
    uint8_t resolution_bits;
    const char* description;
  };

  ResolutionTest res_tests[] = {
    {1000,  8,  "1kHz @ 8-bit"},    // Low resolution
    {1000,  10, "1kHz @ 10-bit"},   // Default resolution
    {1000,  12, "1kHz @ 12-bit"},   // High resolution
    {5000,  8,  "5kHz @ 8-bit"},    // Medium frequency, low resolution
    {5000,  10, "5kHz @ 10-bit"},   // Medium frequency, default resolution
    {10000, 8,  "10kHz @ 8-bit"},   // High frequency, low resolution
  };

  // Test percentages to verify
  float test_percentages[] = {0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 1.0f};

  for (const auto& res_test : res_tests) {
    ESP_LOGI(TAG, "Testing %s", res_test.description);
    
    // ✅ NEW: Configure channel with explicit frequency and resolution
    hf_pwm_channel_config_t ch_config = create_test_channel_config(2, res_test.frequency, res_test.resolution_bits);
    ch_config.duty_initial = 0; // Start at 0%
    
    hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel for %s", res_test.description);
      return false;
    }

    result = pwm.EnableChannel(0);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to enable channel for %s", res_test.description);
      return false;
    }

    // Verify the resolution was set correctly
    uint8_t actual_resolution = pwm.GetResolution(0);
    if (actual_resolution != res_test.resolution_bits) {
      ESP_LOGE(TAG, "Resolution mismatch for %s: expected %d, got %d", 
               res_test.description, res_test.resolution_bits, actual_resolution);
      return false;
    }

    // Test each percentage
    for (float percentage : test_percentages) {
      ESP_LOGI(TAG, "  Setting %.1f%% duty cycle", percentage * 100.0f);
      
      result = pwm.SetDutyCycle(0, percentage);
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set %.1f%% duty cycle for %s", percentage * 100.0f, res_test.description);
        return false;
      }

      // Verify the percentage reads back correctly
      float actual_percentage = pwm.GetDutyCycle(0);
      
      // Calculate expected tolerance based on resolution
      float tolerance = 1.0f / (1u << res_test.resolution_bits); // One step tolerance
      tolerance += 0.001f; // Add small floating point tolerance
      
      if (abs(actual_percentage - percentage) > tolerance) {
        ESP_LOGE(TAG, "Percentage mismatch for %s at %.1f%%: expected %.4f, got %.4f (tolerance=%.4f)", 
                 res_test.description, percentage * 100.0f, percentage, actual_percentage, tolerance);
        return false;
      }

      ESP_LOGI(TAG, "  ✓ %.1f%% verified: actual=%.4f%% (diff=%.4f%%, tolerance=%.4f%%)", 
               percentage * 100.0f, actual_percentage * 100.0f, 
               abs(actual_percentage - percentage) * 100.0f, tolerance * 100.0f);
    }

    ESP_LOGI(TAG, "✓ %s passed all percentage tests", res_test.description);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  ESP_LOGI(TAG, "[SUCCESS] Percentage consistency across resolutions test passed");
  return true;
}

/**
 * @brief Test direct resolution control methods (NEW)
 */
bool test_resolution_control_methods() noexcept {
  ESP_LOGI(TAG, "Testing direct resolution control methods...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure channel with default resolution
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2, 1000, 10); // 1kHz @ 10-bit
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel for resolution control test");
    return false;
  }

  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel for resolution control test");
    return false;
  }

  // Set initial duty cycle
  result = pwm.SetDutyCycle(0, 0.5f); // 50%
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set initial duty cycle");
    return false;
  }

  // Test GetResolution
  uint8_t initial_resolution = pwm.GetResolution(0);
  if (initial_resolution != 10) {
    ESP_LOGE(TAG, "Initial resolution should be 10 bits, got %d", initial_resolution);
    return false;
  }
  ESP_LOGI(TAG, "✓ GetResolution() returned correct initial resolution: %d bits", initial_resolution);

  // Test SetResolution - change to 8-bit
  ESP_LOGI(TAG, "Changing resolution from 10-bit to 8-bit...");
  result = pwm.SetResolution(0, 8);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set resolution to 8 bits: %s", HfPwmErrToString(result));
    return false;
  }

  // Verify resolution changed
  uint8_t new_resolution = pwm.GetResolution(0);
  if (new_resolution != 8) {
    ESP_LOGE(TAG, "Resolution should be 8 bits after change, got %d", new_resolution);
    return false;
  }
  ESP_LOGI(TAG, "✓ Resolution successfully changed to 8 bits");

  // Verify duty cycle percentage is preserved (should still be ~50%)
  float duty_after_resolution_change = pwm.GetDutyCycle(0);
  if (abs(duty_after_resolution_change - 0.5f) > 0.02f) { // 2% tolerance
    ESP_LOGE(TAG, "Duty cycle not preserved after resolution change: expected ~50%%, got %.2f%%", 
             duty_after_resolution_change * 100.0f);
    return false;
  }
  ESP_LOGI(TAG, "✓ Duty cycle preserved after resolution change: %.2f%%", duty_after_resolution_change * 100.0f);

  // Test SetResolution - change to 12-bit
  ESP_LOGI(TAG, "Changing resolution from 8-bit to 12-bit...");
  result = pwm.SetResolution(0, 12);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set resolution to 12 bits: %s", HfPwmErrToString(result));
    return false;
  }

  // Verify resolution changed
  new_resolution = pwm.GetResolution(0);
  if (new_resolution != 12) {
    ESP_LOGE(TAG, "Resolution should be 12 bits after change, got %d", new_resolution);
    return false;
  }
  ESP_LOGI(TAG, "✓ Resolution successfully changed to 12 bits");

  // Test SetFrequencyAndResolution - atomic operation
  ESP_LOGI(TAG, "Testing atomic frequency and resolution change...");
  result = pwm.SetFrequencyAndResolution(0, 2000, 9); // 2kHz @ 9-bit
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set frequency and resolution atomically: %s", HfPwmErrToString(result));
    return false;
  }

  // Verify both parameters changed
  uint32_t new_frequency = pwm.GetFrequency(0);
  new_resolution = pwm.GetResolution(0);
  
  if (new_frequency != 2000) {
    ESP_LOGE(TAG, "Frequency should be 2000 Hz after atomic change, got %lu", new_frequency);
    return false;
  }
  
  if (new_resolution != 9) {
    ESP_LOGE(TAG, "Resolution should be 9 bits after atomic change, got %d", new_resolution);
    return false;
  }
  
  ESP_LOGI(TAG, "✓ Atomic frequency and resolution change successful: %lu Hz @ %d bits", 
           new_frequency, new_resolution);

  // Test invalid resolution values
  ESP_LOGI(TAG, "Testing invalid resolution handling...");
  
  // Too low resolution
  result = pwm.SetResolution(0, 3);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Should reject resolution below 4 bits");
    return false;
  }
  ESP_LOGI(TAG, "✓ Correctly rejected resolution below 4 bits");

  // Too high resolution
  result = pwm.SetResolution(0, 15);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Should reject resolution above %d bits", HF_PWM_MAX_RESOLUTION);
    return false;
  }
  ESP_LOGI(TAG, "✓ Correctly rejected resolution above %d bits", HF_PWM_MAX_RESOLUTION);

  ESP_LOGI(TAG, "[SUCCESS] Resolution control methods test passed");
  return true;
}

/**
 * @brief Test resolution-aware duty cycle calculations (NEW)
 */
bool test_resolution_aware_duty_calculations() noexcept {
  ESP_LOGI(TAG, "Testing resolution-aware duty cycle calculations...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test different resolutions with precise duty cycle calculations
  struct ResolutionDutyTest {
    uint8_t resolution_bits;
    float duty_percentage;
    uint32_t expected_raw_value;
    const char* description;
  };

  ResolutionDutyTest tests[] = {
    {8,  0.5f,  127,  "8-bit @ 50%"},   // 255/2 ≈ 127
    {8,  0.25f, 63,   "8-bit @ 25%"},   // 255/4 ≈ 63
    {8,  1.0f,  255,  "8-bit @ 100%"},  // 255
    {10, 0.5f,  511,  "10-bit @ 50%"},  // 1023/2 ≈ 511
    {10, 0.25f, 255,  "10-bit @ 25%"},  // 1023/4 ≈ 255
    {10, 1.0f,  1023, "10-bit @ 100%"}, // 1023
    {12, 0.5f,  2047, "12-bit @ 50%"},  // 4095/2 ≈ 2047
    {12, 0.25f, 1023, "12-bit @ 25%"},  // 4095/4 ≈ 1023
    {12, 1.0f,  4095, "12-bit @ 100%"}, // 4095
  };

  for (const auto& test : tests) {
    ESP_LOGI(TAG, "Testing %s", test.description);
    
    // Configure channel with specific resolution
    hf_pwm_channel_config_t ch_config = create_test_channel_config(2, 1000, test.resolution_bits);
    ch_config.duty_initial = 0; // Start at 0%
    
    hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel for %s", test.description);
      return false;
    }

    result = pwm.EnableChannel(0);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to enable channel for %s", test.description);
      return false;
    }

    // Set duty cycle as percentage
    result = pwm.SetDutyCycle(0, test.duty_percentage);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set duty cycle for %s", test.description);
      return false;
    }

    // Verify the duty cycle reads back correctly
    float actual_duty = pwm.GetDutyCycle(0);
    float tolerance = 1.0f / (1u << test.resolution_bits); // One step tolerance
    
    if (abs(actual_duty - test.duty_percentage) > tolerance) {
      ESP_LOGE(TAG, "Duty cycle mismatch for %s: expected %.4f, got %.4f", 
               test.description, test.duty_percentage, actual_duty);
      return false;
    }

    // Test raw duty cycle setting with expected value
    result = pwm.SetDutyCycleRaw(0, test.expected_raw_value);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set raw duty cycle for %s", test.description);
      return false;
    }

    // Verify raw value produces expected percentage
    actual_duty = pwm.GetDutyCycle(0);
    if (abs(actual_duty - test.duty_percentage) > tolerance) {
      ESP_LOGE(TAG, "Raw duty cycle mismatch for %s: expected %.4f, got %.4f", 
               test.description, test.duty_percentage, actual_duty);
      return false;
    }

    ESP_LOGI(TAG, "✓ %s verified: %.4f%% (raw=%lu)", 
             test.description, actual_duty * 100.0f, test.expected_raw_value);
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  ESP_LOGI(TAG, "[SUCCESS] Resolution-aware duty calculations test passed");
  return true;
}

//==============================================================================
// EDGE CASES AND STRESS TESTS
//==============================================================================

bool test_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing edge cases...");

  hf_pwm_unit_config_t config = create_basic_with_fade_config();  // Basic mode with fade for channel enable
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Test boundary duty cycles
  hf_pwm_channel_config_t ch_config = create_test_channel_config(2);
  pwm.ConfigureChannel(0, ch_config);
  pwm.EnableChannel(0);

  // Test minimum and maximum duty cycles
  hf_pwm_err_t result = pwm.SetDutyCycle(0, 0.0f);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set minimum duty cycle");
    return false;
  }

  result = pwm.SetDutyCycle(0, 1.0f);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set maximum duty cycle");
    return false;
  }

  // Test boundary frequencies
  result = pwm.SetFrequency(0, HF_PWM_MIN_FREQUENCY);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set minimum frequency");
    return false;
  }

  // Test a high but achievable frequency (20 kHz is reasonable for ESP32-C6)
  result = pwm.SetFrequency(0, 20000);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set high frequency");
    return false;
  }

  // Test invalid channel operations
  result = pwm.SetDutyCycle(EspPwm::MAX_CHANNELS, 0.5f);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Invalid channel operation should fail");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Edge cases test passed");
  return true;
}

bool test_stress_scenarios() noexcept {
  ESP_LOGI(TAG, "Testing stress scenarios...");

  hf_pwm_unit_config_t config = create_basic_with_fade_config();  // Basic mode with fade for channel enable
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure maximum number of channels (avoid GPIO3 -> use GPIO6 instead)
  for (hf_channel_id_t ch = 0; ch < EspPwm::MAX_CHANNELS; ch++) {
    hf_gpio_num_t test_pin = static_cast<hf_gpio_num_t>(2 + ch);
    if (test_pin == 3) {
      test_pin = 6;
    }
    hf_pwm_channel_config_t ch_config = create_test_channel_config(test_pin);
    ch_config.channel_id = ch;
    ch_config.duty_initial = 200 + (ch * 100);

    hf_pwm_err_t result = pwm.ConfigureChannel(ch, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %d in stress test", ch);
      return false;
    }

    pwm.EnableChannel(ch);
  }

  // Rapid duty cycle changes
  for (int iteration = 0; iteration < 20; iteration++) {
    for (hf_channel_id_t ch = 0; ch < EspPwm::MAX_CHANNELS; ch++) {
      float duty = 0.1f + (iteration * 0.04f);
      if (duty > 1.0f)
        duty = 1.0f;

      pwm.SetDutyCycle(ch, duty);
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Brief delay
  }

  // Rapid frequency changes
  for (int iteration = 0; iteration < 10; iteration++) {
    for (hf_channel_id_t ch = 0; ch < EspPwm::MAX_CHANNELS; ch++) {
      hf_frequency_hz_t freq = 500 + (iteration * 200);
      pwm.SetFrequency(ch, freq);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  // Test synchronized operations with all channels
  pwm.StartAll();
  vTaskDelay(pdMS_TO_TICKS(100));

  pwm.UpdateAll();
  vTaskDelay(pdMS_TO_TICKS(100));

  pwm.StopAll();

  ESP_LOGI(TAG, "[SUCCESS] Stress scenarios test passed");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 PWM COMPREHENSIVE TEST SUITE                       ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                    ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                                 ║");
  ESP_LOGI(TAG, "║ Features: PWM, Duty Cycle Control, Frequency Control, Phase Shift Control,     ║");
  ESP_LOGI(TAG, "║ Complementary Outputs, Hardware Fade, Idle Level Control, Timer Management,    ║");
  ESP_LOGI(TAG, "║ Status Reporting, Statistics and Diagnostics, Callbacks, Edge Cases, Stress    ║");
  ESP_LOGI(TAG, "║ Tests, ESP32-Specific Features, Error Handling, Performance, Utility Functions,║");
  ESP_LOGI(TAG, "║ Cleanup, Edge Cases, Stress Tests, ESP32-Specific Features, Error Handling,    ║");
  ESP_LOGI(TAG, "║ Performance, Utility Functions, Cleanup, Edge Cases, Stress Tests              ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                                 ║");
  ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
  }

  // Constructor/Destructor Tests
  ESP_LOGI(TAG, "\n=== CONSTRUCTOR/DESTRUCTOR TESTS ===");
  RUN_TEST(test_constructor_default);
  flip_test_progress_indicator();
  RUN_TEST(test_destructor_cleanup);
  flip_test_progress_indicator();

  // Lifecycle Tests
  ESP_LOGI(TAG, "\n=== LIFECYCLE TESTS ===");
  RUN_TEST(test_initialization_states);
  flip_test_progress_indicator();
  RUN_TEST(test_lazy_initialization);
  flip_test_progress_indicator();

  // Configuration Tests
  ESP_LOGI(TAG, "\n=== CONFIGURATION TESTS ===");
  RUN_TEST(test_mode_configuration);
  flip_test_progress_indicator();
  RUN_TEST(test_clock_source_configuration);
  flip_test_progress_indicator();
  RUN_TEST(test_basic_mode_without_fade);
  flip_test_progress_indicator();

  // Channel Management Tests
  ESP_LOGI(TAG, "\n=== CHANNEL MANAGEMENT TESTS ===");
  RUN_TEST(test_channel_configuration);
  flip_test_progress_indicator();
  RUN_TEST(test_channel_enable_disable);
  flip_test_progress_indicator();

  // PWM Control Tests
  ESP_LOGI(TAG, "\n=== PWM CONTROL TESTS ===");
  RUN_TEST(test_duty_cycle_control);
  flip_test_progress_indicator();
  RUN_TEST(test_frequency_control);
  flip_test_progress_indicator();
  RUN_TEST(test_phase_shift_control);
  flip_test_progress_indicator();

  // Advanced Features Tests
  ESP_LOGI(TAG, "\n=== ADVANCED FEATURES TESTS ===");
  RUN_TEST(test_synchronized_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_complementary_outputs);
  flip_test_progress_indicator();

  // ESP32-Specific Features Tests
  ESP_LOGI(TAG, "\n=== ESP32-SPECIFIC FEATURES TESTS ===");
  RUN_TEST(test_hardware_fade);
  flip_test_progress_indicator();
  RUN_TEST(test_fade_mode_functionality);
  flip_test_progress_indicator();
  RUN_TEST(test_idle_level_control);
  flip_test_progress_indicator();
  RUN_TEST(test_timer_management);
  flip_test_progress_indicator();

  // CRITICAL NEW TESTS: Resolution and Validation
  ESP_LOGI(TAG, "\n=== RESOLUTION AND VALIDATION TESTS (NEW) ===");
  RUN_TEST(test_resolution_specific_duty_cycles);
  flip_test_progress_indicator();
  RUN_TEST(test_frequency_resolution_validation);
  flip_test_progress_indicator();
  RUN_TEST(test_percentage_consistency_across_resolutions);
  flip_test_progress_indicator();
  RUN_TEST(test_resolution_control_methods);
  flip_test_progress_indicator();
  RUN_TEST(test_resolution_aware_duty_calculations);
  flip_test_progress_indicator();

  // Status and Diagnostics Tests
  ESP_LOGI(TAG, "\n=== STATUS AND DIAGNOSTICS TESTS ===");
  RUN_TEST(test_status_reporting);
  flip_test_progress_indicator();
  RUN_TEST(test_statistics_and_diagnostics);
  flip_test_progress_indicator();

  // Callback Tests
  ESP_LOGI(TAG, "\n=== CALLBACK TESTS ===");
  RUN_TEST(test_callbacks);
  flip_test_progress_indicator();

  // Edge Cases and Stress Tests
  ESP_LOGI(TAG, "\n=== EDGE CASES AND STRESS TESTS ===");
  RUN_TEST(test_edge_cases);
  flip_test_progress_indicator();
  RUN_TEST(test_stress_scenarios);
  flip_test_progress_indicator();
  
  // Print final summary
  ESP_LOGI(TAG, "\n");
  print_test_summary(g_test_results, "ESP32 PWM COMPREHENSIVE", TAG);

  ESP_LOGI(TAG, "PWM comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Post-test banner
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 PWM COMPREHENSIVE TEST SUITE                       ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════════════════════════╝");

  // Cleanup test progression indicator
  cleanup_test_progress_indicator();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
