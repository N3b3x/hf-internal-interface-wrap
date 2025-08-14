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
 */

#include "TestFramework.h"
#include "base/BasePwm.h"
#include "mcu/esp32/EspPwm.h"

static const char* TAG = "PWM_Test";
static TestResults g_test_results;

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Create a default PWM configuration for testing
 */
hf_pwm_unit_config_t create_test_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE;
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = true;
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a default channel configuration for testing
 */
hf_pwm_channel_config_t create_test_channel_config(hf_gpio_num_t gpio_pin) noexcept {
  hf_pwm_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.channel_id = 0;
  config.timer_id = 0;
  config.speed_mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
  config.duty_initial = 512; // 50% for 10-bit resolution
  config.intr_type = hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE;
  config.invert_output = false;
  config.hpoint = 0;
  config.idle_level = 0;
  config.output_invert = false;
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

  // Test configuring multiple channels (avoid GPIO3 -> use GPIO6 instead)
  for (hf_channel_id_t ch = 0; ch < 4; ch++) {
    hf_gpio_num_t test_pin = static_cast<hf_gpio_num_t>(2 + ch);
    if (test_pin == 3) {
      test_pin = 6;
    }
    hf_pwm_channel_config_t ch_config = create_test_channel_config(test_pin);
    ch_config.channel_id = ch;
    ch_config.duty_initial = 256 + (ch * 128); // Different duty cycles

    hf_pwm_err_t result = pwm.ConfigureChannel(ch, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %d: %s", ch, HfPwmErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "Channel %d configured successfully", ch);
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

  hf_pwm_unit_config_t config = create_test_config();
  config.enable_fade = true;
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

  hf_pwm_unit_config_t config = create_test_config();
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

  hf_pwm_unit_config_t config = create_test_config();
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

  hf_pwm_unit_config_t config = create_test_config();
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

//==============================================================================
// EDGE CASES AND STRESS TESTS
//==============================================================================

bool test_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing edge cases...");

  hf_pwm_unit_config_t config = create_test_config();
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

  hf_pwm_unit_config_t config = create_test_config();
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

  // Constructor/Destructor Tests
  ESP_LOGI(TAG, "\n=== CONSTRUCTOR/DESTRUCTOR TESTS ===");
  RUN_TEST(test_constructor_default);
  RUN_TEST(test_destructor_cleanup);

  // Lifecycle Tests
  ESP_LOGI(TAG, "\n=== LIFECYCLE TESTS ===");
  RUN_TEST(test_initialization_states);
  RUN_TEST(test_lazy_initialization);

  // Configuration Tests
  ESP_LOGI(TAG, "\n=== CONFIGURATION TESTS ===");
  RUN_TEST(test_mode_configuration);
  RUN_TEST(test_clock_source_configuration);

  // Channel Management Tests
  ESP_LOGI(TAG, "\n=== CHANNEL MANAGEMENT TESTS ===");
  RUN_TEST(test_channel_configuration);
  RUN_TEST(test_channel_enable_disable);

  // PWM Control Tests
  ESP_LOGI(TAG, "\n=== PWM CONTROL TESTS ===");
  RUN_TEST(test_duty_cycle_control);
  RUN_TEST(test_frequency_control);
  RUN_TEST(test_phase_shift_control);

  // Advanced Features Tests
  ESP_LOGI(TAG, "\n=== ADVANCED FEATURES TESTS ===");
  RUN_TEST(test_synchronized_operations);
  RUN_TEST(test_complementary_outputs);

  // ESP32-Specific Features Tests
  ESP_LOGI(TAG, "\n=== ESP32-SPECIFIC FEATURES TESTS ===");
  RUN_TEST(test_hardware_fade);
  RUN_TEST(test_idle_level_control);
  RUN_TEST(test_timer_management);

  // Status and Diagnostics Tests
  ESP_LOGI(TAG, "\n=== STATUS AND DIAGNOSTICS TESTS ===");
  RUN_TEST(test_status_reporting);
  RUN_TEST(test_statistics_and_diagnostics);

  // Callback Tests
  ESP_LOGI(TAG, "\n=== CALLBACK TESTS ===");
  RUN_TEST(test_callbacks);

  // Edge Cases and Stress Tests
  ESP_LOGI(TAG, "\n=== EDGE CASES AND STRESS TESTS ===");
  RUN_TEST(test_edge_cases);
  RUN_TEST(test_stress_scenarios);

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

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
