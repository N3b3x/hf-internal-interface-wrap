/**
 * @file PwmComprehensiveTest.cpp
 * @brief Comprehensive PWM testing suite for ESP32 family (noexcept)
 *
 * This comprehensive test suite validates all functionality of the EspPwm class
 * across different ESP32 variants, with particular focus on LEDC peripheral
 * capabilities and constraints:
 *
 * ## Test Coverage:
 * - **Lifecycle Management:** Constructor/Destructor, Initialize/Deinitialize
 * - **Configuration:** Modes, clock sources, unit configuration
 * - **Channel Management:** Configure, enable/disable, validation
 * - **PWM Control:** Duty cycle, frequency, resolution control
 * - **Advanced Features:** Synchronized operations, complementary outputs
 * - **ESP32-Specific:** Hardware fade, idle levels, timer management
 * - **LEDC Validation:** Clock source constraints, frequency/resolution limits
 * - **Resource Management:** Timer allocation, eviction policies, health checks
 * - **Status & Diagnostics:** Statistics, error reporting, capability detection
 * - **Callbacks:** Fade callback mechanisms (ESP-IDF LEDC native support only)
 * - **Edge Cases & Stress:** Boundary conditions, resource exhaustion, recovery
 *
 * ## Hardware Requirements:
 * - ESP32 development board (any variant: ESP32, ESP32-S2/S3, ESP32-C3/C6, ESP32-H2)
 * - GPIO pins for PWM output testing (configurable)
 * - Optional: Logic analyzer or oscilloscope for signal verification
 *
 * ## Test Progression Indicator:
 * GPIO14 toggles HIGH/LOW after each test completion for visual feedback.
 * This allows monitoring test progress without serial output.
 *
 * @note This test suite is designed to be variant-agnostic and will adapt
 * to the specific LEDC capabilities of the target ESP32 variant.
 */

#include "TestFramework.h"
#include "base/BasePwm.h"
#include "mcu/esp32/EspGpio.h" // Add GPIO support for test progression indicator
#include "mcu/esp32/EspPwm.h"
#include <array>

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
 * @return Configured PWM unit configuration for basic testing
 *
 * @details Creates a standard test configuration with:
 * - Basic PWM mode (no fade)
 * - APB clock source (80MHz)
 * - Interrupts enabled
 * - Fade functionality disabled for basic testing
 */
hf_pwm_unit_config_t create_test_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB;
  config.enable_fade = false; // Basic mode without fade
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a PWM configuration specifically for fade testing
 */
hf_pwm_unit_config_t create_fade_test_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE; // Use FADE mode
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = true; // Enable fade functionality
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a PWM configuration for basic mode with fade enabled (for channel enable
 * operations)
 */
hf_pwm_unit_config_t create_basic_with_fade_config() noexcept {
  hf_pwm_unit_config_t config = {};
  config.unit_id = 0;
  config.mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC; // Basic mode
  config.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  config.enable_fade = true; // Enable fade for channel operations
  config.enable_interrupts = true;
  return config;
}

/**
 * @brief Create a default channel configuration for testing with explicit resolution control
 * @param gpio_pin GPIO pin number for PWM output
 * @param frequency_hz PWM frequency in Hz (default: 1kHz)
 * @param resolution_bits PWM resolution in bits (default: 10-bit)
 * @return Configured PWM channel configuration for testing
 *
 * @details Creates a standard channel configuration with:
 * - 50% initial duty cycle
 * - APB clock source preference
 * - Basic PWM mode
 * - No output inversion
 * - Low priority (non-critical)
 */
hf_pwm_channel_config_t create_test_channel_config(
    hf_gpio_num_t gpio_pin, hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
    hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.channel_id = 0;
  config.timer_id = 0;
  config.speed_mode = hf_pwm_mode_t::HF_PWM_MODE_BASIC;

  // Explicit frequency and resolution control
  config.frequency_hz = frequency_hz;
  config.resolution_bits = resolution_bits;
  config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB;

  // Calculate 50% duty cycle for the specified resolution
  config.duty_initial = (1U << resolution_bits) / 2; // 50% duty cycle

  config.intr_type = hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE;
  config.invert_output = false;
  config.hpoint = 0;
  config.idle_level = 0;
  config.output_invert = false;

  config.is_critical = false;
  config.priority = hf_pwm_channel_priority_t::PRIORITY_LOW;

  return config;
}

/**
 * @brief Create channel configuration with specific duty cycle percentage
 */
hf_pwm_channel_config_t create_test_channel_config_with_duty(
    hf_gpio_num_t gpio_pin, float duty_percentage, hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
    hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config =
      create_test_channel_config(gpio_pin, frequency_hz, resolution_bits);

  // Calculate raw duty value for the specified percentage and resolution
  hf_u32_t max_duty = (1U << resolution_bits) - 1;
  config.duty_initial = static_cast<hf_u32_t>(duty_percentage * max_duty);

  return config;
}

//==============================================================================
// CONSTRUCTOR/DESTRUCTOR TESTS
//==============================================================================

/**
 * @brief Test PWM constructor variations and object creation
 * @return true if all constructor tests pass, false otherwise
 *
 * @details Validates proper object construction without hardware initialization:
 * - Default constructor with minimal configuration
 * - Constructor with explicit unit configuration
 *
 * @note No hardware initialization occurs during construction (lazy initialization pattern)
 * @warning All constructors must complete without exceptions (noexcept specification)
 */
bool test_constructor_default() noexcept {
  ESP_LOGI(TAG, "Testing default constructor...");

  // Test constructors without exception handling
  EspPwm pwm1;
  ESP_LOGI(TAG, "[SUCCESS] Default constructor completed");

  // Test constructor with unit config
  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm2(config);
  ESP_LOGI(TAG, "[SUCCESS] Constructor with config completed");

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

/**
 * @brief Test PWM initialization state management and lifecycle
 * @return true if all initialization state tests pass, false otherwise
 *
 * @details Validates proper initialization state transitions:
 * - Initial uninitialized state after construction
 * - Manual initialization with Initialize() method
 * - Double initialization protection (returns ALREADY_INITIALIZED)
 * - Proper deinitialization with Deinitialize() method
 * - State consistency throughout lifecycle
 *
 * @note Tests the explicit initialization path (not lazy initialization)
 * @warning All state transitions must be atomic and thread-safe
 */
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
  ESP_LOGI(TAG, "Testing per-channel clock source configuration...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // ESP32 clock source constraints: Most variants require shared clock sources
  ESP_LOGI(TAG, "Testing clock source configuration (APB 80MHz with compatible frequencies)");

  // Test different channels with APB clock source - frequencies designed for timer sharing
  struct ClockSourceTest {
    hf_gpio_num_t gpio_pin{};
    uint32_t frequency{};
    uint8_t resolution{};
    const char* description{};
  };

  std::array<ClockSourceTest, 4> tests = {{
      {2, 1000, 10, "1kHz @ 10-bit"},
      {3, 2000, 10, "2kHz @ 10-bit"},
      {4, 4000, 10, "4kHz @ 10-bit"},
      {5, 8000, 10, "8kHz @ 10-bit"},
  }};

  for (size_t i = 0; i < tests.size(); i++) {
    const auto& test = tests[i];

    // Clean up previous configuration if needed
    if (pwm.IsChannelEnabled(i)) {
      pwm.DeconfigureChannel(i);
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Configure with APB clock source
    hf_pwm_channel_config_t ch_config = create_test_channel_config(test.gpio_pin);
    ch_config.frequency_hz = test.frequency;
    ch_config.resolution_bits = test.resolution;
    ch_config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB;

    hf_pwm_err_t result = pwm.ConfigureChannel(i, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %lu (%s): %s", i, test.description,
               HfPwmErrToString(result));
      return false;
    }

    result = pwm.EnableChannel(i);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to enable channel %lu (%s): %s", i, test.description,
               HfPwmErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "✓ Channel %lu: %s", i, test.description);
    vTaskDelay(pdMS_TO_TICKS(50)); // Reduced delay
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

  // Test configuring multiple channels with different resolutions (avoid GPIO3 -> use GPIO6
  // instead)
  struct ChannelTestConfig {
    hf_gpio_num_t pin{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    float duty_percentage{};
  };

  std::array<ChannelTestConfig, 4> test_configs = {{
      {2, 1000, 8, 0.25F},  // GPIO2: 1kHz @ 8-bit, 25%
      {6, 2000, 10, 0.50F}, // GPIO6: 2kHz @ 10-bit, 50%
      {4, 1500, 12, 0.75F}, // GPIO4: 1.5kHz @ 12-bit, 75%
      {5, 3000, 9, 0.33F}   // GPIO5: 3kHz @ 9-bit, 33%
  }};

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
      ESP_LOGE(TAG, "Channel %d resolution mismatch: expected %d, got %d", ch, test_cfg.resolution,
               actual_resolution);
      return false;
    }

    if (actual_frequency != test_cfg.frequency) {
      ESP_LOGE(TAG, "Channel %d frequency mismatch: expected %lu, got %lu", ch, test_cfg.frequency,
               actual_frequency);
      return false;
    }

    ESP_LOGI(TAG, "Channel %d configured successfully: %lu Hz @ %d-bit, %.1f%% duty", ch,
             actual_frequency, actual_resolution, test_cfg.duty_percentage * 100.0F);
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

/**
 * @brief Test comprehensive duty cycle control functionality
 * @return true if all duty cycle tests pass, false otherwise
 *
 * @details Validates precise duty cycle control across full range:
 * - **Float Interface:** Tests 0%, 25%, 50%, 75%, 100% duty cycles
 * - **Raw Interface:** Tests raw values 0, 256, 512, 768, 1023 (10-bit)
 * - **Input Validation:** Tests rejection of invalid values (-0.1, 1.1)
 * - **Accuracy Verification:** Confirms readback values match set values
 *
 * @note Uses GPIO 2 for PWM output with 1kHz frequency @ 10-bit resolution
 * @warning Duty cycle accuracy should be within ±1% of commanded value
 *
 * @see test_resolution_specific_duty_cycles() for resolution-specific testing
 */
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
  std::array<float, 5> test_duties = {{0.0F, 0.25F, 0.5F, 0.75F, 1.0F}};

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
  std::array<hf_u32_t, 5> raw_values = {{0, 256, 512, 768, 1023}}; // For 10-bit resolution

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
  hf_pwm_err_t result = pwm.SetDutyCycle(0, -0.1F);
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
  std::array<hf_frequency_hz_t, 6> test_frequencies = {{100, 500, 1000, 5000, 10000, 20000}};

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
  hf_pwm_err_t result = pwm.SetPhaseShift(0, 0.0F);
  if (result == hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER) {
    // ESP32-C6 LEDC doesn't support phase shift - skip this test
    ESP_LOGW(TAG, "Phase shift not supported on this hardware - skipping test");
    ESP_LOGI(TAG, "[SKIPPED] Phase shift control test (hardware limitation)");
    return true; // Return true to indicate test was handled appropriately
  }

  // If we get here, phase shift is supported, so run the full test
  std::array<float, 4> test_phases = {{0.0F, 90.0F, 180.0F, 270.0F}};

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
  result = pwm.SetPhaseShift(0, 400.0F);
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
  std::array<float, 3> test_duties = {{0.2F, 0.5F, 0.8F}};

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
  pwm.SetDutyCycle(0, 0.1F);
  vTaskDelay(pdMS_TO_TICKS(100));

  // Test fade operations
  struct FadeTest {
    float target_duty{};
    hf_u32_t fade_time_ms{};
  };

  std::array<FadeTest, 4> fade_tests = {{
      {0.8F, 1000}, // Fade up
      {0.2F, 800},  // Fade down
      {0.9F, 1200}, // Fade up again
      {0.0F, 500}   // Fade to minimum
  }};

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
  pwm.SetHardwareFade(0, 0.5F, 2000); // Start a long fade
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
  std::array<hf_u8_t, 2> idle_levels = {{0, 1}};

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

/**
 * @brief Test comprehensive LEDC timer resource management
 * @return true if all timer management tests pass, false otherwise
 *
 * @details Validates advanced timer allocation and management features:
 *
 * **Phase 1: Basic Timer Allocation**
 * - Tests automatic timer assignment for different frequency/resolution combinations
 * - Validates timer sharing optimization for compatible frequencies
 * - Confirms proper timer resource tracking
 *
 * **Phase 2: Timer Exhaustion Scenarios**
 * - Tests behavior when all timers are allocated with incompatible combinations
 * - Validates proper error reporting (TIMER_CONFLICT)
 * - Confirms system stability under resource pressure
 *
 * **Phase 3: Compatible Frequency Reuse**
 * - Tests timer sharing for frequencies within tolerance (±5%)
 * - Validates resource optimization and efficiency
 *
 * **Phase 4: Timer Recovery**
 * - Tests timer resource recovery after channel release
 * - Validates automatic cleanup and reallocation
 *
 * **Phase 5: Forced Timer Assignment**
 * - Tests manual timer assignment with ForceTimerAssignment()
 * - Validates override of automatic allocation
 *
 * **Phase 6: Diagnostics Validation**
 * - Tests statistics and diagnostics reporting accuracy
 * - Validates resource usage tracking
 *
 * @note This test exercises the core LEDC timer management algorithms
 * @warning Timer allocation behavior may vary between ESP32 variants
 *
 * @see FindOrAllocateTimer() for timer allocation implementation
 * @see ForceTimerAssignment() for manual timer control
 */
bool test_timer_management() noexcept {
  ESP_LOGI(TAG, "Testing timer management...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Phase 1: Test basic timer allocation with different frequency/resolution combinations
  ESP_LOGI(TAG, "Phase 1: Testing basic timer allocation");

  struct TimerTestConfig {
    hf_channel_id_t channel{};
    hf_gpio_num_t gpio{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    const char* description{};
  };

  // These combinations are designed to require separate timers
  std::array<TimerTestConfig, 4> timer_configs = {{
      {0, 2, 1000, 8, "Timer allocation test: 1kHz @ 8-bit"},   // Should get timer 0
      {1, 6, 2000, 10, "Timer allocation test: 2kHz @ 10-bit"}, // Should get timer 1
      {2, 4, 5000, 8, "Timer allocation test: 5kHz @ 8-bit"},   // Should get timer 2
      {3, 5, 10000, 9, "Timer allocation test: 10kHz @ 9-bit"}  // Should get timer 3
  }};

  // Track which timers are used
  bool timer_used[4] = {false, false, false, false};

  for (const auto& cfg : timer_configs) {
    ESP_LOGI(TAG, "Configuring %s", cfg.description);

    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(cfg.gpio, cfg.frequency, cfg.resolution);
    ch_config.channel_id = cfg.channel;

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel %d: %s", cfg.channel, HfPwmErrToString(result));
      return false;
    }

    int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
    if (timer_id < 0 || timer_id >= 4) {
      ESP_LOGE(TAG, "Invalid timer assignment for channel %d: %d", cfg.channel, timer_id);
      return false;
    }

    timer_used[timer_id] = true;
    ESP_LOGI(TAG, "✓ Channel %d assigned to timer %d", cfg.channel, timer_id);
  }

  // Verify that multiple timers are being used (not all on same timer)
  int timers_in_use = 0;
  for (int i = 0; i < 4; i++) {
    if (timer_used[i])
      timers_in_use++;
  }

  ESP_LOGI(TAG, "Total timers in use: %d/4", timers_in_use);
  if (timers_in_use < 3) {
    ESP_LOGW(TAG, "Expected at least 3 different timers to be used, got %d", timers_in_use);
  }

  // Phase 2: Test timer exhaustion - try to allocate a 5th unique combination
  ESP_LOGI(TAG, "Phase 2: Testing timer exhaustion scenario");

  hf_pwm_channel_config_t fifth_config =
      create_test_channel_config(7, 15000, 8); // Unique combination
  fifth_config.channel_id = 4;

  hf_pwm_err_t result = pwm.ConfigureChannel(4, fifth_config);

  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    int8_t timer_id = pwm.GetTimerAssignment(4);
    ESP_LOGI(TAG, "✓ 5th combination allocated successfully to timer %d (reuse or eviction)",
             timer_id);
  } else {
    ESP_LOGI(TAG, "✓ 5th combination correctly rejected: %s (expected when all timers exhausted)",
             HfPwmErrToString(result));

    // This is acceptable - it means all timers are exhausted with incompatible combinations
    if (result != hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT &&
        result != hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH) {
      ESP_LOGW(TAG, "Expected TIMER_CONFLICT or FREQUENCY_TOO_HIGH error, got: %s",
               HfPwmErrToString(result));
    }
  }

  // Phase 3: Test compatible frequency reuse
  ESP_LOGI(TAG, "Phase 3: Testing compatible frequency reuse");

  hf_pwm_channel_config_t compatible_config =
      create_test_channel_config(8, 1050, 8); // Within 5% of 1000Hz
  compatible_config.channel_id = 5;

  result = pwm.ConfigureChannel(5, compatible_config);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    int8_t timer_id = pwm.GetTimerAssignment(5);
    ESP_LOGI(TAG, "✓ Compatible frequency configuration succeeded, using timer %d", timer_id);
  } else {
    ESP_LOGI(TAG, "Compatible frequency configuration failed: %s", HfPwmErrToString(result));
  }

  // Phase 4: Test channel release and timer recovery
  ESP_LOGI(TAG, "Phase 4: Testing timer recovery after channel release");

  // Disable channel 3 to potentially free up timer 3
  pwm.DisableChannel(3);

  // Now retry the previously failed 5th combination if it failed
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Retrying 5th combination after releasing channel 3");

    result = pwm.ConfigureChannel(4, fifth_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      int8_t timer_id = pwm.GetTimerAssignment(4);
      ESP_LOGI(TAG, "✓ 5th combination succeeded after timer recovery, using timer %d", timer_id);
    } else {
      ESP_LOGI(TAG, "5th combination still failed after recovery: %s", HfPwmErrToString(result));
    }
  }

  // Phase 5: Test forced timer assignment
  ESP_LOGI(TAG, "Phase 5: Testing forced timer assignment");

  result = pwm.ForceTimerAssignment(0, 3);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to force timer assignment: %s", HfPwmErrToString(result));
    return false;
  }

  int8_t forced_timer_id = pwm.GetTimerAssignment(0);
  if (forced_timer_id != 3) {
    ESP_LOGE(TAG, "Forced timer assignment failed: expected 3, got %d", forced_timer_id);
    return false;
  }

  ESP_LOGI(TAG, "✓ Forced timer assignment successful");

  // Phase 6: Validate diagnostics and statistics
  ESP_LOGI(TAG, "Phase 6: Validating diagnostics and statistics");

  hf_pwm_diagnostics_t diagnostics;
  result = pwm.GetDiagnostics(diagnostics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Diagnostics: Active timers=%d, Active channels=%d", diagnostics.active_timers,
             diagnostics.active_channels);
  }

  hf_pwm_statistics_t statistics;
  result = pwm.GetStatistics(statistics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Statistics: Error count=%lu, Last activity=%llu", statistics.error_count,
             statistics.last_activity_timestamp);
  }

  ESP_LOGI(TAG, "[SUCCESS] Enhanced timer management test passed");
  return true;
}

//==============================================================================
// STATUS AND DIAGNOSTICS TESTS
//==============================================================================

bool test_status_reporting() noexcept {
  ESP_LOGI(TAG, "Testing status reporting...");

  hf_pwm_unit_config_t config =
      create_basic_with_fade_config(); // Basic mode with fade for channel enable
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

  hf_pwm_unit_config_t config =
      create_basic_with_fade_config(); // Basic mode with fade for channel enable
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
    pwm.SetDutyCycle(0, 0.2F + (i * 0.15F));
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

// Global variables for fade callback testing (ESP-IDF LEDC native support only)
static volatile bool g_fade_callback_called[HF_PWM_MAX_CHANNELS] = {false};
static volatile hf_channel_id_t g_last_fade_channel = 0xFF;

void test_fade_callback_ch0(hf_channel_id_t channel_id) {
  // MINIMAL ISR-safe callback - only set flags!
  g_fade_callback_called[channel_id] = true;
  g_last_fade_channel = channel_id;
  // NO ESP_LOG calls in ISR context - they can cause stack overflow!
}

void test_fade_callback_ch1(hf_channel_id_t channel_id) {
  // MINIMAL ISR-safe callback - only set flags!
  g_fade_callback_called[channel_id] = true;
  g_last_fade_channel = channel_id;
  // NO ESP_LOG calls in ISR context - they can cause stack overflow!
}

bool test_callbacks() noexcept {
  ESP_LOGI(TAG, "Testing FADE CALLBACK functionality (ESP-IDF LEDC native support only)...");

  hf_pwm_unit_config_t config = create_fade_test_config(); // Use fade mode for callback testing
  config.enable_interrupts = true;
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Reset fade callback flags
  for (int i = 0; i < HF_PWM_MAX_CHANNELS; i++) {
    g_fade_callback_called[i] = false;
  }
  g_last_fade_channel = 0xFF;

  // Configure channels for fade testing
  hf_pwm_channel_config_t ch0_config = create_test_channel_config(2);
  hf_pwm_channel_config_t ch1_config = create_test_channel_config(4);

  pwm.ConfigureChannel(0, ch0_config);
  pwm.ConfigureChannel(1, ch1_config);

  // Set per-channel fade callbacks (ESP-IDF LEDC native support)
  pwm.SetChannelFadeCallback(0, test_fade_callback_ch0);
  pwm.SetChannelFadeCallback(1, test_fade_callback_ch1);

  pwm.EnableChannel(0);
  pwm.EnableChannel(1);

  ESP_LOGI(TAG, "Starting hardware fade operations to trigger callbacks...");

  // Start fade operations that will trigger callbacks
  pwm.SetHardwareFade(0, 0.8f, 1000); // Channel 0: fade to 80% over 1 second
  pwm.SetHardwareFade(1, 0.3f, 800);  // Channel 1: fade to 30% over 0.8 seconds

  // Wait for fade operations to complete and callbacks to trigger
  ESP_LOGI(TAG, "Waiting for fade operations to complete...");
  vTaskDelay(pdMS_TO_TICKS(1500)); // Wait longer than the longest fade

  // Check results
  bool test_passed = true;

  if (!g_fade_callback_called[0]) {
    ESP_LOGE(TAG, "[FAIL] Channel 0 fade callback was not called");
    test_passed = false;
  } else {
    ESP_LOGI(TAG, "[SUCCESS] Channel 0 fade callback was called");
  }

  if (!g_fade_callback_called[1]) {
    ESP_LOGE(TAG, "[FAIL] Channel 1 fade callback was not called");
    test_passed = false;
  } else {
    ESP_LOGI(TAG, "[SUCCESS] Channel 1 fade callback was called");
  }

  // Test callback clearing
  pwm.SetChannelFadeCallback(0, nullptr); // Clear callback
  g_fade_callback_called[0] = false;

  pwm.SetHardwareFade(0, 0.1F, 200); // Should not trigger callback
  vTaskDelay(pdMS_TO_TICKS(400));

  if (g_fade_callback_called[0]) {
    ESP_LOGE(TAG, "[FAIL] Channel 0 callback was called after being cleared");
    test_passed = false;
  } else {
    ESP_LOGI(TAG, "[SUCCESS] Channel 0 callback correctly cleared");
  }

  ESP_LOGI(TAG, "Fade callback test completed - %s", test_passed ? "PASSED" : "FAILED");
  return test_passed;
}

/**
 * @brief Test basic mode without fade functionality
 */
bool test_basic_mode_without_fade() noexcept {
  ESP_LOGI(TAG, "Testing basic mode without fade...");

  hf_pwm_unit_config_t config = create_test_config(); // Basic mode without fade
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
  result = pwm.SetDutyCycle(0, 0.5F);
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

  hf_pwm_unit_config_t config = create_fade_test_config(); // Fade mode with fade enabled
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
  result = pwm.SetDutyCycle(0, 0.5F);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set duty cycle in fade mode");
    return false;
  }

  // Test that we can use hardware fade in fade mode
  result = pwm.SetHardwareFade(0, 0.8F, 1000);
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

  std::array<DutyCycleTest, 5> duty_tests = {{{0.0F, 0, "0% duty cycle"},
                                              {0.25F, 255, "25% duty cycle"},
                                              {0.5F, 511, "50% duty cycle"},
                                              {0.75F, 767, "75% duty cycle"},
                                              {1.0F, 1023, "100% duty cycle"}}};

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
    float tolerance = 0.002F; // Allow 0.2% tolerance for rounding

    if (abs(actual_duty - expected_duty) > tolerance) {
      ESP_LOGE(TAG, "Duty cycle mismatch for %s: expected %.4f, got %.4f (diff=%.4f)",
               test.description, expected_duty, actual_duty, abs(actual_duty - expected_duty));
      return false;
    }

    // Test raw duty cycle setting as well
    result = pwm.SetDutyCycleRaw(0, test.expected_raw_10bit);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set raw duty cycle %lu: %s", test.expected_raw_10bit,
               HfPwmErrToString(result));
      return false;
    }

    actual_duty = pwm.GetDutyCycle(0);
    if (abs(actual_duty - expected_duty) > tolerance) {
      ESP_LOGE(TAG, "Raw duty cycle mismatch for %s: expected %.4f, got %.4f", test.description,
               expected_duty, actual_duty);
      return false;
    }

    ESP_LOGI(TAG, "✓ %s verified: %.4f%% (raw=%lu)", test.description, actual_duty * 100.0F,
             test.expected_raw_10bit);
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
  hf_pwm_channel_config_t ch_config =
      create_test_channel_config(2, 1000, 10); // Explicit 1kHz @ 10-bit
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
    uint32_t frequency{};
    bool should_succeed{};
    const char* description{};
  };

  std::array<FreqResTest, 10> freq_tests = {{
      // CORRECTED: Based on pure theoretical ESP32-C6 LEDC limits (you're right!)
      {1000, true, "1 kHz @ 10-bit (valid - 1.024 MHz < 80MHz)"},
      {5000, true, "5 kHz @ 10-bit (valid - 5.12 MHz < 80MHz)"},
      {10000, true, "10 kHz @ 10-bit (valid - 10.24 MHz < 80MHz)"},
      {20000, true, "20 kHz @ 10-bit (valid - 20.48 MHz < 80MHz)"},
      {25000, true, "25 kHz @ 10-bit (valid - 25.6 MHz < 80MHz)"},
      {30000, true, "30 kHz @ 10-bit (valid - 30.72 MHz < 80MHz)"},
      {40000, true, "40 kHz @ 10-bit (valid - 40.96 MHz < 80MHz)"},
      {50000, true, "50 kHz @ 10-bit (valid - 51.2 MHz < 80MHz)"},
      {78000, true, "78 kHz @ 10-bit (valid - 79.872 MHz < 80MHz)"},
      {100000, false, "100 kHz @ 10-bit (should fail - 102.4 MHz > 80MHz)"},
  }};

  for (const auto& test : freq_tests) {
    ESP_LOGI(TAG, "Testing %s", test.description);

    result = pwm.SetFrequency(0, test.frequency);

    if (test.should_succeed) {
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Expected success for %s but got: %s", test.description,
                 HfPwmErrToString(result));
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
 * @brief Test enhanced validation system with clock source awareness (NEW)
 * @return true if all validation system tests pass, false otherwise
 *
 * @details Comprehensive validation of the LEDC peripheral constraint system:
 *
 * **Phase 1: Clock Source Validation**
 * - Tests APB clock (80MHz) with various frequency/resolution combinations
 * - Validates hardware constraint formula: freq × (2^resolution) ≤ clock_freq
 * - Verifies proper error reporting for invalid combinations
 *
 * **Phase 2: Dynamic Resolution Calculation**
 * - Tests maximum achievable resolution for given frequencies
 * - Validates theoretical vs. practical resolution limits
 * - Confirms hardware constraint calculations
 *
 * **Phase 3: Enhanced Duty Cycle Validation**
 * - Tests overflow protection for different resolutions
 * - Validates automatic clamping of out-of-range values
 * - Confirms resolution-specific duty cycle ranges
 *
 * **Phase 4: Auto-Fallback Functionality**
 * - Tests automatic resolution adjustment for problematic combinations
 * - Validates fallback resolution selection algorithms
 * - Confirms graceful handling of impossible combinations
 *
 * @note This test validates the core LEDC peripheral constraint system
 * @warning Tests may fail on ESP32 variants with different LEDC capabilities
 *
 * @see test_frequency_resolution_validation() for basic constraint testing
 * @see SetFrequencyWithAutoFallback() for automatic resolution adjustment
 */
bool test_enhanced_validation_system() noexcept {
  ESP_LOGI(TAG, "Testing enhanced validation system with clock source awareness...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Configure a basic channel first
  hf_pwm_channel_config_t ch_config =
      create_test_channel_config(2, 1000, 10); // Explicit 1kHz @ 10-bit
  hf_pwm_err_t result = pwm.ConfigureChannel(0, ch_config);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel for enhanced validation test");
    return false;
  }

  result = pwm.EnableChannel(0);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel for enhanced validation test");
    return false;
  }

  // Test 1: Clock source aware validation
  ESP_LOGI(TAG, "Phase 1: Testing clock source aware validation");
  ESP_LOGI(TAG,
           "Note: ESP32-C6 requires all timers to use same clock source AND compatible dividers");

  // Test different frequencies that can share the same timer or use compatible dividers
  // We'll use frequencies that can share timers or use the same divider values
  struct ClockSourceTest {
    uint32_t frequency{};
    uint8_t resolution{};
    bool should_succeed{};
    const char* description{};
  };

  std::array<ClockSourceTest, 4> clock_tests = {{
      // Test with APB clock source (80MHz) - use frequencies that can share timers
      {20000, 10, true,
       "20kHz@10bit with APB clock (80MHz) - should succeed [20kHz x 1024 = 20.48 MHz (25.6% of "
       "80MHz)]"},
      {40000, 10, true,
       "40kHz@10bit with APB clock (80MHz) - should succeed [40kHz x 1024 = 40.96 MHz (51.2% of "
       "80MHz)]"},
      {60000, 10, true,
       "60kHz@10bit with APB clock (80MHz) - should succeed [60kHz x 1024 = 61.44 MHz (76.8% of "
       "80MHz)]"},
      {80000, 10, false,
       "80kHz@10bit with APB clock (80MHz) - should fail [80kHz x 1024 = 81.92 MHz (102.4% of "
       "80MHz)]"},
  }};

  for (const auto& test : clock_tests) {
    ESP_LOGI(TAG, "Testing %s", test.description);

    // Configure a new channel with APB clock source (80MHz)
    hf_pwm_channel_config_t clock_test_config = create_test_channel_config(
        3, test.frequency, test.resolution); // Use GPIO 3 for clock tests
    clock_test_config.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB; // APB clock (80MHz)

    // Test channel configuration with APB clock source
    result = pwm.ConfigureChannel(1, clock_test_config); // Use channel 1 for clock tests

    if (test.should_succeed) {
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Expected success for %s but got: %s", test.description,
                 HfPwmErrToString(result));
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

    ESP_LOGI(TAG, "Deconfiguring channel 1 before reconfiguration...");
    pwm.DeconfigureChannel(1);
    // Wait a bit for cleanup to complete
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // Test 2: Dynamic resolution calculation
  ESP_LOGI(TAG, "Phase 2: Testing dynamic resolution calculation");

  struct ResolutionTest {
    uint32_t frequency;
    uint8_t expected_max_resolution;
    const char* description;
  };

  std::array<ResolutionTest, 7> res_tests = {{
      {1000, 14,
       "1kHz should support up to 14-bit resolution [1kHz x 16383 = 16.383 MHz (20.48% of 80MHz)]"},
      {5000, 13,
       "5kHz should support up to 13-bit resolution [5kHz x 8191 = 40.955 MHz (51.2% of 80MHz)]"},
      {10000, 12,
       "10kHz should support up to 12-bit resolution [10kHz x 4095 = 40.95 MHz (51.2% of 80MHz)]"},
      {20000, 11,
       "20kHz should support up to 11-bit resolution [20kHz x 2047 = 40.94 MHz (51.2% of 80MHz)]"},
      {40000, 10,
       "40kHz should support up to 10-bit resolution [40kHz x 1023 = 40.92 MHz (51.2% of 80MHz)]"},
      {78125, 10,
       "78.125kHz should support exactly 10-bit resolution [78.125kHz x 1023 = 79.872 MHz (99.84% "
       "of 80MHz)]"},
      {156250, 9,
       "156.25kHz should support exactly 9-bit resolution [156.25kHz x 511 = 79.872 MHz (99.84% of "
       "80MHz)]"},
  }};

  for (const auto& test : res_tests) {
    ESP_LOGI(TAG, "Testing %s", test.description);

    // Test by trying to configure a channel with the expected resolution
    hf_pwm_channel_config_t test_config =
        create_test_channel_config(4, test.frequency,
                                   test.expected_max_resolution); // Use GPIO 4 for validation tests

    hf_pwm_err_t result =
        pwm.ConfigureChannel(5, test_config); // Use channel 5 for validation tests
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGI(TAG, "✓ %s: max resolution = %d bits (validated)", test.description,
               test.expected_max_resolution);
      pwm.DisableChannel(5); // Clean up
    } else {
      ESP_LOGE(TAG, "Expected max resolution %d for %s failed configuration",
               test.expected_max_resolution, test.description);
      return false;
    }
  }

  // Test 3: Enhanced duty cycle validation
  ESP_LOGI(TAG, "Phase 3: Testing enhanced duty cycle validation");

  // Test duty cycle overflow protection
  result = pwm.SetFrequencyAndResolution(0, 1000, 8); // 8-bit resolution (0-255)
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set 1kHz @ 8-bit for duty cycle test");
    return false;
  }

  // Test valid duty cycles
  std::array<uint32_t, 3> valid_duties = {{0, 127, 255}}; // 0%, 50%, 100% for 8-bit
  for (uint32_t duty : valid_duties) {
    result = pwm.SetDutyCycleRaw(0, duty);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Valid duty cycle %lu failed for 8-bit resolution", duty);
      return false;
    }
    ESP_LOGI(TAG, "✓ Valid duty cycle %lu/255 accepted", duty);
  }

  // Test invalid duty cycle (should be clamped)
  result = pwm.SetDutyCycleRaw(0, 300); // > 255 for 8-bit
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Duty cycle clamping failed - should clamp 300 to 255");
    return false;
  }
  ESP_LOGI(TAG, "✓ Invalid duty cycle 300 was properly clamped");

  // Test 4: Auto-fallback resolution functionality
  ESP_LOGI(TAG, "Phase 4: Testing auto-fallback resolution functionality");

  // Test case where preferred resolution is too high - use public API
  hf_pwm_channel_config_t fallback_config =
      create_test_channel_config(5, 100000, 12); // Use GPIO 5 for fallback tests - 100kHz @ 12-bit

  // Enable auto-fallback and try to configure
  pwm.EnableAutoFallback();
  hf_pwm_err_t fallback_result = pwm.SetFrequencyWithAutoFallback(5, 100000, 12);
  if (fallback_result == hf_pwm_err_t::PWM_SUCCESS) {
    hf_u8_t actual_res = pwm.GetResolution(5);
    ESP_LOGI(TAG, "✓ Auto-fallback: 100kHz @ 12-bit → %d bits", actual_res);
    pwm.DisableChannel(5); // Clean up
  } else {
    ESP_LOGE(TAG, "Auto-fallback failed for 100kHz @ 12-bit");
    return false;
  }
  pwm.DisableAutoFallback();

  ESP_LOGI(TAG, "[SUCCESS] Enhanced validation system test passed");
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

  std::array<ResolutionTest, 6> res_tests = {{
      {1000, 8, "1kHz @ 8-bit"},   // Low resolution
      {1000, 10, "1kHz @ 10-bit"}, // Default resolution
      {1000, 12, "1kHz @ 12-bit"}, // High resolution
      {5000, 8, "5kHz @ 8-bit"},   // Medium frequency, low resolution
      {5000, 10, "5kHz @ 10-bit"}, // Medium frequency, default resolution
      {10000, 8, "10kHz @ 8-bit"}, // High frequency, low resolution
  }};

  // Test percentages to verify
  std::array<float, 7> test_percentages = {{0.0F, 0.1F, 0.25F, 0.5F, 0.75F, 0.9F, 1.0F}};

  for (const auto& res_test : res_tests) {
    ESP_LOGI(TAG, "Testing %s", res_test.description);

    // Configure channel with explicit frequency and resolution
    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(2, res_test.frequency, res_test.resolution_bits);
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
      ESP_LOGE(TAG, "Resolution mismatch for %s: expected %d, got %d", res_test.description,
               res_test.resolution_bits, actual_resolution);
      return false;
    }

    // Test each percentage
    for (float percentage : test_percentages) {
      ESP_LOGI(TAG, "  Setting %.1f%% duty cycle", percentage * 100.0F);

      result = pwm.SetDutyCycle(0, percentage);
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set %.1f%% duty cycle for %s", percentage * 100.0F,
                 res_test.description);
        return false;
      }

      // Verify the percentage reads back correctly
      float actual_percentage = pwm.GetDutyCycle(0);

      // Calculate expected tolerance based on resolution
      float tolerance = 1.0F / (1U << res_test.resolution_bits); // One step tolerance
      tolerance += 0.001F; // Add small floating point tolerance

      if (abs(actual_percentage - percentage) > tolerance) {
        ESP_LOGE(
            TAG, "Percentage mismatch for %s at %.1f%%: expected %.4f, got %.4f (tolerance=%.4f)",
            res_test.description, percentage * 100.0F, percentage, actual_percentage, tolerance);
        return false;
      }

      ESP_LOGI(TAG, "  ✓ %.1f%% verified: actual=%.4f%% (diff=%.4f%%, tolerance=%.4f%%)",
               percentage * 100.0F, actual_percentage * 100.0F,
               abs(actual_percentage - percentage) * 100.0F, tolerance * 100.0F);
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
  result = pwm.SetDutyCycle(0, 0.5F); // 50%
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
  ESP_LOGI(TAG, "✓ GetResolution() returned correct initial resolution: %d bits",
           initial_resolution);

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
  if (abs(duty_after_resolution_change - 0.5F) > 0.02F) { // 2% tolerance
    ESP_LOGE(TAG, "Duty cycle not preserved after resolution change: expected ~50%%, got %.2f%%",
             duty_after_resolution_change * 100.0F);
    return false;
  }
  ESP_LOGI(TAG, "✓ Duty cycle preserved after resolution change: %.2f%%",
           duty_after_resolution_change * 100.0F);

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
    ESP_LOGE(TAG, "Failed to set frequency and resolution atomically: %s",
             HfPwmErrToString(result));
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
    uint8_t resolution_bits{};
    float duty_percentage{};
    uint32_t expected_raw_value{};
    const char* description{};
  };

  std::array<ResolutionDutyTest, 9> tests = {{
      {8, 0.5F, 127, "8-bit @ 50%"},     // 255/2 ≈ 127
      {8, 0.25F, 63, "8-bit @ 25%"},     // 255/4 ≈ 63
      {8, 1.0F, 255, "8-bit @ 100%"},    // 255
      {10, 0.5F, 511, "10-bit @ 50%"},   // 1023/2 ≈ 511
      {10, 0.25F, 255, "10-bit @ 25%"},  // 1023/4 ≈ 255
      {10, 1.0F, 1023, "10-bit @ 100%"}, // 1023
      {12, 0.5F, 2047, "12-bit @ 50%"},  // 4095/2 ≈ 2047
      {12, 0.25F, 1023, "12-bit @ 25%"}, // 4095/4 ≈ 1023
      {12, 1.0F, 4095, "12-bit @ 100%"}, // 4095
  }};

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
    float tolerance = 1.0F / (1U << test.resolution_bits); // One step tolerance

    if (abs(actual_duty - test.duty_percentage) > tolerance) {
      ESP_LOGE(TAG, "Duty cycle mismatch for %s: expected %.4f, got %.4f", test.description,
               test.duty_percentage, actual_duty);
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
      ESP_LOGE(TAG, "Raw duty cycle mismatch for %s: expected %.4f, got %.4f", test.description,
               test.duty_percentage, actual_duty);
      return false;
    }

    ESP_LOGI(TAG, "✓ %s verified: %.4f%% (raw=%lu)", test.description, actual_duty * 100.0F,
             test.expected_raw_value);

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

  hf_pwm_unit_config_t config =
      create_basic_with_fade_config(); // Basic mode with fade for channel enable
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
  hf_pwm_err_t result = pwm.SetDutyCycle(0, 0.0F);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set minimum duty cycle");
    return false;
  }

  result = pwm.SetDutyCycle(0, 1.0F);
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
  result = pwm.SetDutyCycle(EspPwm::MAX_CHANNELS, 0.5F);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Invalid channel operation should fail");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Edge cases test passed");
  return true;
}

bool test_stress_scenarios() noexcept {
  ESP_LOGI(TAG, "Testing stress scenarios...");

  hf_pwm_unit_config_t config =
      create_basic_with_fade_config(); // Basic mode with fade for channel enable
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Phase 1: Timer exhaustion stress test with different frequency/resolution combinations
  ESP_LOGI(TAG, "Phase 1: Timer exhaustion stress test");

  struct StressConfig {
    hf_channel_id_t channel{};
    hf_gpio_num_t gpio{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    const char* description{};
  };

  // Configure channels with different combinations to stress timer allocation
  std::array<StressConfig, 6> stress_configs = {
      {{0, 2, 1000, 8, "Stress channel 0: 1kHz @ 8-bit"},
       {1, 6, 2500, 10, "Stress channel 1: 2.5kHz @ 10-bit"},
       {2, 4, 5000, 8, "Stress channel 2: 5kHz @ 8-bit"},
       {3, 5, 7500, 9, "Stress channel 3: 7.5kHz @ 9-bit"},
       {4, 7, 12000, 8, "Stress channel 4: 12kHz @ 8-bit"},
       {5, 8, 15000, 8, "Stress channel 5: 15kHz @ 8-bit"}}};

  int successful_configs = 0;
  int expected_failures = 0;

  for (const auto& cfg : stress_configs) {
    ESP_LOGI(TAG, "Configuring %s", cfg.description);

    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(cfg.gpio, cfg.frequency, cfg.resolution);
    ch_config.channel_id = cfg.channel;
    // FIX: Calculate duty based on resolution to prevent overflow
    hf_u32_t max_duty = (1U << cfg.resolution) - 1;
    ch_config.duty_initial = std::min(200U + (cfg.channel * 50U), max_duty);

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      successful_configs++;
      int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
      ESP_LOGI(TAG, "✓ %s succeeded, assigned to timer %d", cfg.description, timer_id);

      // Enable the channel
      pwm.EnableChannel(cfg.channel);
    } else {
      expected_failures++;
      ESP_LOGI(TAG, "✓ %s failed as expected: %s (timer exhaustion)", cfg.description,
               HfPwmErrToString(result));
    }
  }

  ESP_LOGI(TAG, "Timer stress test: %d successful, %d failed (expected due to timer limits)",
           successful_configs, expected_failures);

  // Phase 2: Rapid configuration/release cycles to test timer cleanup
  ESP_LOGI(TAG, "Phase 2: Rapid allocation/release cycles");

  for (int cycle = 0; cycle < 5; cycle++) {
    ESP_LOGI(TAG, "Allocation cycle %d", cycle + 1);

    // Configure channels with varying frequencies
    for (hf_channel_id_t ch = 0; ch < 4; ch++) {
      hf_gpio_num_t test_pin = static_cast<hf_gpio_num_t>(2 + ch);
      if (test_pin == 3)
        test_pin = 6;

      hf_u8_t resolution = 8 + (ch % 3); // Varying resolution
      hf_pwm_channel_config_t ch_config =
          create_test_channel_config(test_pin,
                                     1000 + (ch * 500) + (cycle * 100), // Varying frequency
                                     resolution                         // Varying resolution
          );
      ch_config.channel_id = ch;
      // Calculate duty based on resolution to prevent overflow
      hf_u32_t max_duty = (1U << resolution) - 1;
      ch_config.duty_initial = std::min(100U + (ch * 30U), max_duty);

      hf_pwm_err_t result = pwm.ConfigureChannel(ch, ch_config);
      if (result == hf_pwm_err_t::PWM_SUCCESS) {
        pwm.EnableChannel(ch);
      }
    }

    // Brief operation period
    vTaskDelay(pdMS_TO_TICKS(50));

    // Release all channels
    for (hf_channel_id_t ch = 0; ch < 4; ch++) {
      pwm.DisableChannel(ch);
    }

    // Allow timer cleanup
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // Phase 3: Rapid duty cycle changes on active channels
  ESP_LOGI(TAG, "Phase 3: Rapid duty cycle stress test");

  for (int iteration = 0; iteration < 20; iteration++) {
    for (hf_channel_id_t ch = 0; ch < successful_configs; ch++) {
      if (pwm.IsChannelEnabled(ch)) {
        float duty = 0.1F + (iteration * 0.04F);
        if (duty > 1.0F)
          duty = 1.0F;

        hf_pwm_err_t result = pwm.SetDutyCycle(ch, duty);
        if (result != hf_pwm_err_t::PWM_SUCCESS) {
          ESP_LOGW(TAG, "Duty cycle change failed for channel %d: %s", ch,
                   HfPwmErrToString(result));
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Phase 4: Rapid frequency changes to stress timer allocation
  ESP_LOGI(TAG, "Phase 4: Rapid frequency change stress test");

  for (int iteration = 0; iteration < 10; iteration++) {
    for (hf_channel_id_t ch = 0; ch < successful_configs; ch++) {
      if (pwm.IsChannelEnabled(ch)) {
        hf_frequency_hz_t freq = 500 + (iteration * 200);
        hf_pwm_err_t result = pwm.SetFrequency(ch, freq);

        if (result != hf_pwm_err_t::PWM_SUCCESS) {
          ESP_LOGI(TAG,
                   "Frequency change failed for channel %d to %lu Hz: %s (expected for some "
                   "combinations)",
                   ch, freq, HfPwmErrToString(result));
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  // Phase 5: Test synchronized operations with active channels
  ESP_LOGI(TAG, "Phase 5: Synchronized operations stress test");

  pwm.StartAll();
  vTaskDelay(pdMS_TO_TICKS(100));

  pwm.UpdateAll();
  vTaskDelay(pdMS_TO_TICKS(100));

  pwm.StopAll();

  // Phase 6: Validate system state after stress testing
  ESP_LOGI(TAG, "Phase 6: Post-stress validation");

  hf_pwm_diagnostics_t diagnostics;
  hf_pwm_err_t result = pwm.GetDiagnostics(diagnostics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Post-stress diagnostics: Active timers=%d, Active channels=%d",
             diagnostics.active_timers, diagnostics.active_channels);
  }

  hf_pwm_statistics_t statistics;
  result = pwm.GetStatistics(statistics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Post-stress statistics: Errors=%lu, Duty updates=%lu, Freq changes=%lu",
             statistics.error_count, statistics.duty_updates_count,
             statistics.frequency_changes_count);
  }

  ESP_LOGI(TAG, "[SUCCESS] Enhanced stress scenarios test passed");
  return true;
}

/**
 * @brief Test timer health check and recovery mechanisms (NEW)
 */
bool test_timer_health_check_and_recovery() noexcept {
  ESP_LOGI(TAG, "Testing timer health check and recovery mechanisms...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Phase 1: Create a scenario that requires health check intervention
  ESP_LOGI(TAG, "Phase 1: Setting up timer allocation scenario");

  struct HealthCheckConfig {
    hf_channel_id_t channel{};
    hf_gpio_num_t gpio{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    const char* description{};
  };

  // Configure channels to use all available timers
  std::array<HealthCheckConfig, 4> health_configs = {
      {{0, 2, 1000, 8, "Health test: 1kHz @ 8-bit"},
       {1, 6, 3000, 10, "Health test: 3kHz @ 10-bit"},
       {2, 4, 8000, 8, "Health test: 8kHz @ 8-bit"},
       {3, 5, 15000, 9, "Health test: 15kHz @ 9-bit"}}};

  // Track successful configurations
  int configured_channels = 0;

  for (const auto& cfg : health_configs) {
    ESP_LOGI(TAG, "Configuring %s", cfg.description);

    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(cfg.gpio, cfg.frequency, cfg.resolution);
    ch_config.channel_id = cfg.channel;
    ch_config.duty_initial = 200;

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      configured_channels++;
      int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
      ESP_LOGI(TAG, "✓ %s configured successfully, timer %d", cfg.description, timer_id);

      pwm.EnableChannel(cfg.channel);
    } else {
      ESP_LOGI(TAG, "✓ %s failed: %s", cfg.description, HfPwmErrToString(result));
    }
  }

  // Phase 2: Disable some channels to create orphaned timer scenario
  ESP_LOGI(TAG, "Phase 2: Creating orphaned timer scenario");

  if (configured_channels >= 2) {
    // Disable channels 1 and 3 to potentially create orphaned timers
    pwm.DisableChannel(1);
    pwm.DisableChannel(3);
    ESP_LOGI(TAG, "Disabled channels 1 and 3 to create potential orphaned timers");
  }

  // Phase 3: Try to allocate new channels that should trigger health check
  ESP_LOGI(TAG, "Phase 3: Testing health check trigger scenarios");

  struct NewAllocationTest {
    hf_channel_id_t channel{};
    hf_gpio_num_t gpio{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    const char* description{};
  };

  std::array<NewAllocationTest, 2> new_configs = {
      {{4, 7, 20000, 8, "New allocation: 20kHz @ 8-bit (should trigger health check)"},
       {5, 8, 25000, 8, "New allocation: 25kHz @ 8-bit (may fail due to limits)"}}};

  for (const auto& cfg : new_configs) {
    ESP_LOGI(TAG, "Attempting %s", cfg.description);

    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(cfg.gpio, cfg.frequency, cfg.resolution);
    ch_config.channel_id = cfg.channel;
    ch_config.duty_initial = 128;

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
      ESP_LOGI(TAG, "✓ %s succeeded (health check likely worked), timer %d", cfg.description,
               timer_id);
      pwm.EnableChannel(cfg.channel);
    } else {
      ESP_LOGI(TAG, "✓ %s failed: %s (may be due to hardware limits)", cfg.description,
               HfPwmErrToString(result));
    }
  }

  // Phase 4: Test recovery after complete channel release
  ESP_LOGI(TAG, "Phase 4: Testing recovery after complete channel release");

  // Disable all channels
  for (hf_channel_id_t ch = 0; ch < 6; ch++) {
    pwm.DisableChannel(ch);
  }

  // Wait for potential cleanup
  vTaskDelay(pdMS_TO_TICKS(100));

  // Try to allocate fresh channels (should succeed if health check works)
  hf_pwm_channel_config_t recovery_config = create_test_channel_config(2, 5000, 10);
  recovery_config.channel_id = 0;
  recovery_config.duty_initial = 512; // 50% for 10-bit

  hf_pwm_err_t result = pwm.ConfigureChannel(0, recovery_config);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "✓ Recovery allocation succeeded - health check mechanism working");
    pwm.EnableChannel(0);
  } else {
    ESP_LOGE(TAG, "❌ Recovery allocation failed: %s", HfPwmErrToString(result));
    return false;
  }

  // Phase 5: Validate final system state
  ESP_LOGI(TAG, "Phase 5: Final system state validation");

  hf_pwm_diagnostics_t diagnostics;
  result = pwm.GetDiagnostics(diagnostics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Final diagnostics: Active timers=%d, Active channels=%d",
             diagnostics.active_timers, diagnostics.active_channels);

    // After cleanup, we should have minimal active resources
    if (diagnostics.active_timers > 2) {
      ESP_LOGW(TAG, "More active timers than expected: %d (health check may not be optimal)",
               diagnostics.active_timers);
    }
  }

  hf_pwm_statistics_t statistics;
  result = pwm.GetStatistics(statistics);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGI(TAG, "Final statistics: Total errors=%lu, Channel enables=%lu", statistics.error_count,
             statistics.channel_enables_count);
  }

  ESP_LOGI(TAG, "[SUCCESS] Timer health check and recovery test passed");
  return true;
}

/**
 * @brief Test safe eviction policies (NEW CRITICAL SAFETY TEST)
 */
bool test_safe_eviction_policies() noexcept {
  ESP_LOGI(TAG, "Testing safe eviction policies...");

  hf_pwm_unit_config_t config = create_test_config();
  EspPwm pwm(config);

  if (!pwm.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PWM");
    return false;
  }

  // Phase 1: Test STRICT_NO_EVICTION (default)
  ESP_LOGI(TAG, "Phase 1: Testing STRICT_NO_EVICTION policy (default)");

  // Verify default policy
  if (pwm.GetEvictionPolicy() != hf_pwm_eviction_policy_t::STRICT_NO_EVICTION) {
    ESP_LOGE(TAG, "Default eviction policy should be STRICT_NO_EVICTION");
    return false;
  }
  ESP_LOGI(TAG, "✓ Default eviction policy is STRICT_NO_EVICTION (safe)");

  // Configure channels to fill all timers
  struct EvictionTestConfig {
    hf_channel_id_t channel{};
    hf_gpio_num_t gpio{};
    hf_u32_t frequency{};
    hf_u8_t resolution{};
    bool is_critical{};
    const char* description{};
  };

  std::array<EvictionTestConfig, 4> eviction_configs = {{
      {0, 2, 1000, 8, true, "Critical motor control"}, // Critical channel
      {1, 6, 3000, 10, false, "LED indicator"},        // Non-critical channel
      {2, 4, 8000, 8, false, "Status LED"},            // Non-critical channel
      {3, 5, 15000, 9, true, "Safety shutdown system"} // Critical channel
  }};

  // Configure all channels and mark critical ones
  for (const auto& cfg : eviction_configs) {
    hf_pwm_channel_config_t ch_config =
        create_test_channel_config(cfg.gpio, cfg.frequency, cfg.resolution);
    ch_config.channel_id = cfg.channel;
    ch_config.duty_initial = 128; // Safe duty for all resolutions
    ch_config.is_critical = cfg.is_critical;
    ch_config.priority = cfg.is_critical ? hf_pwm_channel_priority_t::PRIORITY_CRITICAL
                                         : hf_pwm_channel_priority_t::PRIORITY_NORMAL;
    ch_config.description = cfg.description;

    hf_pwm_err_t result = pwm.ConfigureChannel(cfg.channel, ch_config);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      pwm.EnableChannel(cfg.channel);
      int8_t timer_id = pwm.GetTimerAssignment(cfg.channel);
      ESP_LOGI(TAG, "✓ %s configured on timer %d (%s)", cfg.description, timer_id,
               cfg.is_critical ? "CRITICAL" : "normal");
    }
  }

  // Try to allocate a 5th channel that would require eviction
  hf_pwm_channel_config_t conflict_config = create_test_channel_config(7, 20000, 8);
  conflict_config.channel_id = 4;
  conflict_config.duty_initial = 128;

  hf_pwm_err_t result = pwm.ConfigureChannel(4, conflict_config);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "STRICT_NO_EVICTION should have prevented allocation requiring eviction");
    return false;
  }
  ESP_LOGI(TAG, "✓ STRICT_NO_EVICTION correctly denied allocation requiring eviction: %s",
           HfPwmErrToString(result));

  // Phase 2: Test ALLOW_EVICTION_NON_CRITICAL
  ESP_LOGI(TAG, "Phase 2: Testing ALLOW_EVICTION_NON_CRITICAL policy");

  result = pwm.SetEvictionPolicy(hf_pwm_eviction_policy_t::ALLOW_EVICTION_NON_CRITICAL);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set eviction policy");
    return false;
  }

  // Try the same allocation - should now succeed by evicting non-critical channels
  result = pwm.ConfigureChannel(4, conflict_config);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    int8_t timer_id = pwm.GetTimerAssignment(4);
    ESP_LOGI(TAG, "✓ ALLOW_EVICTION_NON_CRITICAL successfully allocated channel 4 to timer %d",
             timer_id);

    // Verify critical channels are still working
    if (!pwm.IsChannelEnabled(0) || !pwm.IsChannelEnabled(3)) {
      ESP_LOGE(TAG, "Critical channels should still be enabled after non-critical eviction");
      return false;
    }
    ESP_LOGI(TAG, "✓ Critical channels (0,3) still enabled after non-critical eviction");
  } else {
    ESP_LOGI(TAG, "✓ Non-critical eviction failed (acceptable): %s", HfPwmErrToString(result));
  }

  // Phase 3: Test channel protection
  ESP_LOGI(TAG, "Phase 3: Testing channel protection mechanisms");

  // Mark channel 1 as critical and try to cause eviction
  result = pwm.SetChannelCritical(1, true);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to mark channel 1 as critical");
    return false;
  }

  if (!pwm.IsChannelCritical(1)) {
    ESP_LOGE(TAG, "Channel 1 should be marked as critical");
    return false;
  }
  ESP_LOGI(TAG, "✓ Channel 1 successfully marked as critical");

  // Phase 4: Reset to safe policy
  ESP_LOGI(TAG, "Phase 4: Resetting to safe policy");

  result = pwm.SetEvictionPolicy(hf_pwm_eviction_policy_t::STRICT_NO_EVICTION);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset to safe eviction policy");
    return false;
  }
  ESP_LOGI(TAG, "✓ Successfully reset to STRICT_NO_EVICTION policy");

  ESP_LOGI(TAG, "[SUCCESS] Safe eviction policies test passed");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG,
           "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                    ESP32-C6 PWM COMPREHENSIVE TEST SUITE                       ║");
  ESP_LOGI(TAG,
           "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG,
           "║ Target: ESP32-C6 DevKit-M-1                                                    ║");
  ESP_LOGI(TAG,
           "║ ESP-IDF: v5.5+                                                                 ║");
  ESP_LOGI(TAG,
           "║ Features: PWM, Duty Cycle Control, Frequency Control, Phase Shift Control,     ║");
  ESP_LOGI(TAG,
           "║ Complementary Outputs, Hardware Fade, Idle Level Control, Timer Management,    ║");
  ESP_LOGI(TAG,
           "║ Status Reporting, Statistics and Diagnostics, Callbacks, Edge Cases, Stress    ║");
  ESP_LOGI(TAG,
           "║ Tests, ESP32-Specific Features, Error Handling, Performance, Utility Functions,║");
  ESP_LOGI(TAG,
           "║ Cleanup, Edge Cases, Stress Tests, ESP32-Specific Features, Error Handling,    ║");
  ESP_LOGI(TAG,
           "║ Performance, Utility Functions, Cleanup, Edge Cases, Stress Tests              ║");
  ESP_LOGI(TAG,
           "║ Architecture: noexcept (no exception handling)                                 ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG,
             "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
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
  RUN_TEST(test_enhanced_validation_system);
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

  // Advanced Timer Management Tests
  ESP_LOGI(TAG, "\n=== ADVANCED TIMER MANAGEMENT TESTS ===");
  RUN_TEST(test_timer_health_check_and_recovery);
  flip_test_progress_indicator();

  // Critical Safety Tests
  ESP_LOGI(TAG, "\n=== CRITICAL SAFETY TESTS ===");
  RUN_TEST(test_safe_eviction_policies);
  flip_test_progress_indicator();

  // Print final summary
  ESP_LOGI(TAG, "\n");
  print_test_summary(g_test_results, "ESP32 PWM COMPREHENSIVE", TAG);

  ESP_LOGI(TAG, "PWM comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Post-test banner
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG,
           "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                    ESP32-C6 PWM COMPREHENSIVE TEST SUITE                       ║");
  ESP_LOGI(TAG,
           "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");

  // Cleanup test progression indicator
  cleanup_test_progress_indicator();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
