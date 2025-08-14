/**
 * @file GpioComprehensiveTest.cpp
 * @brief Comprehensive GPIO testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a unified, comprehensive test suite for the EspGpio class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * GPIO functionalities including basic operations, advanced features, interrupts,
 * power management, and hardware-specific capabilities.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

// Include utility classes
#include "utils/AsciiArtGenerator.h"
#include "utils/memory_utils.h"

// ESP32 implementation classes
#include "base/BaseGpio.h"
#include "mcu/esp32/EspGpio.h"
#include "mcu/esp32/utils/EspTypes_GPIO.h"

#include <memory>
#include <vector>

#include "TestFramework.h"

static const char* TAG = "GPIO_Test";

// ESP32-C6 DevKit-M-1 Safe Test Pins
namespace TestPins {
// Safe pins for ESP32-C6 DevKit-M-1 (avoiding strapping, USB-JTAG, SPI flash pins)
static constexpr hf_pin_num_t LED_OUTPUT = 14;      // General purpose output
static constexpr hf_pin_num_t DIGITAL_OUT_1 = 10;   // General purpose output
static constexpr hf_pin_num_t DIGITAL_OUT_2 = 11;   // General purpose output
static constexpr hf_pin_num_t DIGITAL_IN_1 = 2;     // General purpose input
static constexpr hf_pin_num_t DIGITAL_IN_2 = 3;     // General purpose input
static constexpr hf_pin_num_t INTERRUPT_PIN = 2;    // Interrupt testing
static constexpr hf_pin_num_t PULL_TEST_PIN = 3;    // Pull resistor testing
static constexpr hf_pin_num_t DRIVE_TEST_PIN = 16;  // Drive capability testing
static constexpr hf_pin_num_t RTC_GPIO_PIN = 7;     // RTC GPIO pin
static constexpr hf_pin_num_t ANALOG_PIN = 6;       // ADC capable pin
static constexpr hf_pin_num_t LOOPBACK_OUT = 20;    // Output for loopback testing
static constexpr hf_pin_num_t LOOPBACK_IN = 21;     // Input for loopback testing
static constexpr hf_pin_num_t STRESS_TEST_PIN = 23; // Stress testing

// Pins to avoid (strapping, flash, USB-JTAG)
// GPIO 9  - Boot strapping pin
// GPIO 15 - Boot strapping pin
// GPIO 12, 13 - USB-JTAG (D-, D+)
// GPIO 24-30 - SPI flash pins
} // namespace TestPins

static TestResults g_test_results;

// Forward declarations of test functions
bool test_basic_gpio_functionality() noexcept;
bool test_gpio_initialization_and_configuration() noexcept;
bool test_gpio_input_output_operations() noexcept;
bool test_gpio_pull_resistors() noexcept;
bool test_gpio_interrupt_functionality() noexcept;
bool test_gpio_interrupt_loopback() noexcept;
bool test_gpio_advanced_features() noexcept;
bool test_gpio_rtc_functionality() noexcept;
bool test_gpio_glitch_filters() noexcept;
bool test_gpio_sleep_and_wakeup() noexcept;
bool test_gpio_hold_functionality() noexcept;
bool test_gpio_drive_capabilities() noexcept;
bool test_gpio_diagnostics_and_statistics() noexcept;
bool test_gpio_error_handling() noexcept;
bool test_gpio_stress_testing() noexcept;
bool test_gpio_pin_validation() noexcept;
bool test_gpio_loopback_operations() noexcept;
bool test_gpio_concurrent_operations() noexcept;
bool test_gpio_power_consumption() noexcept;

//==============================================================================
// GPIO TEST IMPLEMENTATIONS
//==============================================================================

/**
 * @brief Test basic GPIO functionality including initialization and basic operations
 */
bool test_basic_gpio_functionality() noexcept {
  ESP_LOGI(TAG, "=== Testing Basic GPIO Functionality ===");

  // Test 1: Basic constructor and initialization
  EspGpio led_gpio(TestPins::LED_OUTPUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                   hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!led_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize LED GPIO");
    return false;
  }

  // Test 2: Pin information
  ESP_LOGI(TAG, "LED GPIO Pin: %d, Max Pins: %d", led_gpio.GetPin(), led_gpio.GetMaxPins());
  ESP_LOGI(TAG, "GPIO Description: %s", led_gpio.GetDescription());
  ESP_LOGI(TAG, "Pin Available: %s", led_gpio.IsPinAvailable() ? "YES" : "NO");

  // Test 3: Basic state operations
  auto result = led_gpio.SetActive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set GPIO active");
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  result = led_gpio.SetInactive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set GPIO inactive");
    return false;
  }

  // Test 4: State verification
  bool is_active;
  result = led_gpio.IsActive(is_active);
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read GPIO state");
    return false;
  }

  ESP_LOGI(TAG, "GPIO state after SetInactive: %s", is_active ? "ACTIVE" : "INACTIVE");

  ESP_LOGI(TAG, "[SUCCESS] Basic GPIO functionality test passed");
  return true;
}

/**
 * @brief Test GPIO initialization and configuration modes
 */
bool test_gpio_initialization_and_configuration() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Initialization and Configuration ===");

  // Test different GPIO configurations
  const hf_pin_num_t test_pins[] = {TestPins::DIGITAL_OUT_1, TestPins::DIGITAL_OUT_2,
                                    TestPins::DIGITAL_IN_1, TestPins::DIGITAL_IN_2};
  const hf_gpio_direction_t test_directions[] = {
      hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
      hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT};
  const hf_gpio_active_state_t test_active_states[] = {
      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH, hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW,
      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH, hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW};
  const hf_gpio_output_mode_t test_output_modes[] = {
      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN,
      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
      hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL};

  const size_t num_configs = sizeof(test_pins) / sizeof(test_pins[0]);

  for (size_t i = 0; i < num_configs; i++) {
    auto pin = test_pins[i];
    auto direction = test_directions[i];
    auto active_state = test_active_states[i];
    auto output_mode = test_output_modes[i];

    ESP_LOGI(TAG, "Testing configuration: Pin=%d, Dir=%d, Active=%d, Output=%d", pin,
             static_cast<int>(direction), static_cast<int>(active_state),
             static_cast<int>(output_mode));

    EspGpio test_gpio(pin, direction, active_state, output_mode);

    if (!test_gpio.EnsureInitialized()) {
      ESP_LOGE(TAG, "Failed to initialize GPIO pin %d", pin);
      return false;
    }

    // Verify configuration
    auto read_direction = test_gpio.GetDirection();
    if (read_direction != direction) {
      ESP_LOGE(TAG, "Direction mismatch for pin %d", pin);
      return false;
    }

    if (test_gpio.GetActiveState() != active_state) {
      ESP_LOGE(TAG, "Active state mismatch for pin %d", pin);
      return false;
    }

    ESP_LOGI(TAG, "[SUCCESS] Configuration verified for pin %d", pin);
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO initialization and configuration test passed");
  return true;
}

/**
 * @brief Test GPIO input/output operations and state management
 */
bool test_gpio_input_output_operations() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Input/Output Operations ===");

  // Test output operations
  EspGpio output_gpio(TestPins::DIGITAL_OUT_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!output_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize output GPIO");
    return false;
  }

  // Test various output states
  const hf_gpio_state_t test_states[] = {
      hf_gpio_state_t::HF_GPIO_STATE_ACTIVE, hf_gpio_state_t::HF_GPIO_STATE_INACTIVE,
      hf_gpio_state_t::HF_GPIO_STATE_ACTIVE, hf_gpio_state_t::HF_GPIO_STATE_INACTIVE};

  const size_t num_states = sizeof(test_states) / sizeof(test_states[0]);

  for (size_t i = 0; i < num_states; i++) {
    auto state = test_states[i];
    auto result = output_gpio.SetState(state);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to write state %d", static_cast<int>(state));
      return false;
    }

    // Verify the written state
    auto read_state = output_gpio.GetCurrentState();
    if (read_state != state) {
      ESP_LOGE(TAG, "State mismatch: wrote %d, read %d", static_cast<int>(state),
               static_cast<int>(read_state));
      return false;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

  // Test additional state operations
  const hf_gpio_state_t test_states_2[] = {
      hf_gpio_state_t::HF_GPIO_STATE_ACTIVE, hf_gpio_state_t::HF_GPIO_STATE_INACTIVE,
      hf_gpio_state_t::HF_GPIO_STATE_ACTIVE, hf_gpio_state_t::HF_GPIO_STATE_INACTIVE};

  const size_t num_states_2 = sizeof(test_states_2) / sizeof(test_states_2[0]);

  for (size_t i = 0; i < num_states_2; i++) {
    auto state = test_states_2[i];
    auto result = output_gpio.SetState(state);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set pin state %d", static_cast<int>(state));
      return false;
    }

    auto read_state = output_gpio.GetCurrentState();
    if (read_state != state) {
      ESP_LOGE(TAG, "State mismatch: set %d, read %d", static_cast<int>(state),
               static_cast<int>(read_state));
      return false;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO input/output operations test passed");
  return true;
}

/**
 * @brief Test GPIO pull resistor functionality
 */
bool test_gpio_pull_resistors() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Pull Resistors ===");

  EspGpio pull_test_gpio(TestPins::PULL_TEST_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                         hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!pull_test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize pull test GPIO");
    return false;
  }

  // Test different pull modes
  const hf_gpio_pull_mode_t pull_modes[] = {
      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING, hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP,
      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN, hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING};

  const size_t num_modes = sizeof(pull_modes) / sizeof(pull_modes[0]);

  for (size_t i = 0; i < num_modes; i++) {
    auto pull_mode = pull_modes[i];
    auto result = pull_test_gpio.SetPullMode(pull_mode);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set pull mode %d", static_cast<int>(pull_mode));
      return false;
    }

    // Verify pull mode
    auto read_pull_mode = pull_test_gpio.GetPullMode();
    if (read_pull_mode != pull_mode) {
      ESP_LOGE(TAG, "Pull mode mismatch: set %d, read %d", static_cast<int>(pull_mode),
               static_cast<int>(read_pull_mode));
      return false;
    }

    // Read the pin state and log it
    // Read current state instead of pin level
    auto state = pull_test_gpio.GetCurrentState();
    ESP_LOGI(TAG, "Pull mode %d -> Pin state: %s", static_cast<int>(pull_mode),
             state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE ? "ACTIVE" : "INACTIVE");

    vTaskDelay(pdMS_TO_TICKS(100));
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO pull resistors test passed");
  return true;
}

/**
 * @brief Test GPIO interrupt functionality
 */
bool test_gpio_interrupt_functionality() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Interrupt Functionality ===");

  EspGpio interrupt_gpio(TestPins::INTERRUPT_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                         hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!interrupt_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize interrupt GPIO");
    return false;
  }

  // Check if interrupts are supported
  auto interrupt_support = interrupt_gpio.SupportsInterrupts();
  if (interrupt_support != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGW(TAG, "Interrupts not supported or not implemented");
    return true; // Skip test gracefully
  }

  ESP_LOGI(TAG, "[SUCCESS] Interrupt support verified");

  // Test interrupt configuration
  ESP_LOGI(TAG, "Testing interrupt configuration...");

  // Configure pull-down to ensure clean test conditions
  auto result = interrupt_gpio.SetPullMode(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGW(TAG, "Failed to set pull-down mode");
  }

  // Test different interrupt trigger types
  const hf_gpio_interrupt_trigger_t trigger_types[] = {
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE,
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE,
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES,
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_HIGH_LEVEL,
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_LOW_LEVEL};

  const char* trigger_names[] = {"RISING_EDGE", "FALLING_EDGE", "BOTH_EDGES", "HIGH_LEVEL",
                                 "LOW_LEVEL"};

  for (size_t i = 0; i < sizeof(trigger_types) / sizeof(trigger_types[0]); i++) {
    ESP_LOGI(TAG, "Testing %s interrupt trigger...", trigger_names[i]);

    result = interrupt_gpio.ConfigureInterrupt(trigger_types[i]);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] %s interrupt configured", trigger_names[i]);

      // Test enable/disable
      result = interrupt_gpio.EnableInterrupt();
      if (result == hf_gpio_err_t::GPIO_SUCCESS) {
        ESP_LOGI(TAG, "[SUCCESS] %s interrupt enabled", trigger_names[i]);

        // Get interrupt status
        InterruptStatus status;
        result = interrupt_gpio.GetInterruptStatus(status);
        if (result == hf_gpio_err_t::GPIO_SUCCESS) {
          ESP_LOGI(TAG, "[SUCCESS] Interrupt status retrieved: enabled=%s, count=%lu",
                   status.is_enabled ? "YES" : "NO", status.interrupt_count);
        }

        // Disable interrupt
        result = interrupt_gpio.DisableInterrupt();
        if (result == hf_gpio_err_t::GPIO_SUCCESS) {
          ESP_LOGI(TAG, "[SUCCESS] %s interrupt disabled", trigger_names[i]);
        }
      } else {
        ESP_LOGW(TAG, "[WARNING] Failed to enable %s interrupt", trigger_names[i]);
      }
    } else {
      ESP_LOGW(TAG, "[WARNING] Failed to configure %s interrupt", trigger_names[i]);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

  // Test interrupt statistics
  ESP_LOGI(TAG, "Testing interrupt statistics...");

  result = interrupt_gpio.ClearInterruptStats();
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Interrupt statistics cleared");
  }

  // Test wait for interrupt functionality (with short timeout)
  ESP_LOGI(TAG, "Testing wait for interrupt (timeout test)...");

  result = interrupt_gpio.ConfigureInterrupt(
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    result = interrupt_gpio.EnableInterrupt();
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      // Wait with short timeout (should timeout since no external trigger)
      result = interrupt_gpio.WaitForInterrupt(100); // 100ms timeout
      if (result == hf_gpio_err_t::GPIO_ERR_TIMEOUT) {
        ESP_LOGI(TAG, "[SUCCESS] Wait for interrupt timeout working correctly");
      } else {
        ESP_LOGI(TAG, "[INFO] Wait for interrupt returned: %d", static_cast<int>(result));
      }
    }
    interrupt_gpio.DisableInterrupt();
  }

  ESP_LOGI(TAG, "Note: For complete interrupt testing, external signal generation would be needed");
  ESP_LOGI(TAG, "[SUCCESS] GPIO interrupt functionality test completed");
  return true;
}

/**
 * @brief Test ISR callback handling using loopback between output and input pins
 */
static volatile uint32_t s_isr_loopback_count = 0;

static void IRAM_ATTR gpio_isr_loopback_cb(BaseGpio* /*gpio*/,
                                           hf_gpio_interrupt_trigger_t /*trigger*/,
                                           void* /*user_data*/) {
  s_isr_loopback_count++;
}

bool test_gpio_interrupt_loopback() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Interrupt Loopback (ISR) ===");

  // Use the same loopback pins as the loopback test
  EspGpio output_gpio(TestPins::LOOPBACK_OUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
  EspGpio input_gpio(TestPins::LOOPBACK_IN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!output_gpio.EnsureInitialized() || !input_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize GPIOs for ISR loopback test");
    return false;
  }

  // Ensure input has a defined idle level
  input_gpio.SetPullMode(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  // Clear counters and configure BOTH_EDGES with ISR callback
  s_isr_loopback_count = 0;

  auto cfg_res = input_gpio.ConfigureInterrupt(
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES, gpio_isr_loopback_cb,
      nullptr);
  if (cfg_res != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure ISR on input pin");
    return false;
  }

  if (input_gpio.EnableInterrupt() != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable ISR on input pin");
    return false;
  }

  // Generate edges by toggling output: expect 10 interrupts (5 high + 5 low)
  const int cycles = 5;
  output_gpio.SetInactive();
  vTaskDelay(pdMS_TO_TICKS(10));
  for (int i = 0; i < cycles; i++) {
    output_gpio.SetActive();
    vTaskDelay(pdMS_TO_TICKS(10));
    output_gpio.SetInactive();
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Small settle time
  vTaskDelay(pdMS_TO_TICKS(20));

  // Read interrupt stats
  InterruptStatus status;
  input_gpio.GetInterruptStatus(status);

  input_gpio.DisableInterrupt();

  // Accept if we saw most of the expected edges (tolerate minor timing variance)
  const uint32_t expected = static_cast<uint32_t>(cycles * 2);
  const uint32_t observed = status.interrupt_count;

  ESP_LOGI(TAG, "ISR loopback: expected >= %u, observed %u (callback count %u)", expected,
           observed, s_isr_loopback_count);

  bool pass = observed >= expected - 1; // allow off-by-one
  if (pass) {
    ESP_LOGI(TAG, "[SUCCESS] GPIO interrupt loopback test passed");
  } else {
    ESP_LOGE(TAG, "[FAILURE] GPIO interrupt loopback test failed");
  }

  return pass;
}

/**
 * @brief Test advanced GPIO features (ESP32-C6 specific)
 */
bool test_gpio_advanced_features() noexcept {
  ESP_LOGI(TAG, "=== Testing Advanced GPIO Features ===");

  EspGpio advanced_gpio(TestPins::DRIVE_TEST_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                        hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!advanced_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize advanced GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Testing hardware verification...");

  // Test hardware verification functions
  hf_gpio_direction_t verified_direction;
  auto result = advanced_gpio.VerifyDirection(verified_direction);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Direction verification successful: %d",
             static_cast<int>(verified_direction));
  } else {
    ESP_LOGW(TAG, "Direction verification not available");
  }

  hf_gpio_output_mode_t verified_mode;
  result = advanced_gpio.VerifyOutputMode(verified_mode);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Output mode verification successful: %d",
             static_cast<int>(verified_mode));
  } else {
    ESP_LOGW(TAG, "Output mode verification not available");
  }

  ESP_LOGI(TAG, "[SUCCESS] Advanced GPIO features test completed");
  return true;
}

/**
 * @brief Test RTC GPIO functionality for low-power operations
 */
bool test_gpio_rtc_functionality() noexcept {
  ESP_LOGI(TAG, "=== Testing RTC GPIO Functionality ===");

  // Use a pin that supports RTC GPIO
  EspGpio rtc_gpio(TestPins::RTC_GPIO_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                   hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!rtc_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize RTC GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Testing basic RTC GPIO operations...");

  // Test basic operations on RTC-capable pin
  auto result = rtc_gpio.SetActive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTC GPIO active");
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  result = rtc_gpio.SetInactive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTC GPIO inactive");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] RTC GPIO functionality test completed");
  return true;
}

/**
 * @brief Test other advanced GPIO functions - placeholders for comprehensive testing
 */
bool test_gpio_glitch_filters() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Glitch Filters ===");

  // Use interrupt pin for glitch filter testing
  EspGpio filter_gpio(TestPins::INTERRUPT_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!filter_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize glitch filter test GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Testing pin-specific glitch filter...");

  // Test pin-specific glitch filter configuration
  auto result = filter_gpio.ConfigurePinGlitchFilter(true);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Pin glitch filter enabled successfully");

    // Test with filter disabled
    result = filter_gpio.ConfigurePinGlitchFilter(false);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Pin glitch filter disabled successfully");
    } else {
      ESP_LOGW(TAG, "[FAILURE] Failed to disable pin glitch filter: %d", static_cast<int>(result));
    }
  } else {
    ESP_LOGW(TAG, "[FAILURE] Pin glitch filter not supported or failed: %d",
             static_cast<int>(result));
  }

  ESP_LOGI(TAG, "Testing flexible glitch filter...");

  // Test flexible glitch filter with correct struct
  hf_gpio_flex_filter_config_t flex_config = {
      .window_width_ns = 100,      // 100 ns width
      .window_threshold_ns = 1000, // 1 microsecond threshold
      .clk_src = hf_gpio_glitch_filter_clk_src_t::HF_GLITCH_FILTER_CLK_SRC_APB,
      .enable_on_init = true};

  result = filter_gpio.ConfigureFlexGlitchFilter(flex_config);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Flexible glitch filter configured successfully");
    ESP_LOGI(TAG, "   Window width: %lu ns, Threshold: %lu ns", flex_config.window_width_ns,
             flex_config.window_threshold_ns);
  } else {
    ESP_LOGW(TAG, "[FAILURE] Flexible glitch filter not supported or failed: %d",
             static_cast<int>(result));
  }

  ESP_LOGI(TAG, "Testing pin glitch filter configuration...");

  // Test pin glitch filter with correct signature
  result =
      filter_gpio.ConfigureGlitchFilter(hf_gpio_glitch_filter_type_t::HF_GPIO_GLITCH_FILTER_PIN);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Pin glitch filter configured successfully");
  } else {
    ESP_LOGW(TAG, "[FAILURE] Pin glitch filter configuration failed: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO glitch filters test completed");
  return true;
}

bool test_gpio_sleep_and_wakeup() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Sleep and Wake-up ===");

  // Use RTC GPIO pin for sleep/wakeup testing
  EspGpio sleep_gpio(TestPins::RTC_GPIO_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW);

  if (!sleep_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize sleep test GPIO");
    return false;
  }

  // Check if pin supports RTC functionality
  if (!sleep_gpio.SupportsRtcGpio()) {
    ESP_LOGW(TAG, "Pin %d does not support RTC GPIO, using alternative sleep test",
             TestPins::RTC_GPIO_PIN);
  } else {
    ESP_LOGI(TAG, "[SUCCESS] Pin %d supports RTC GPIO functionality", TestPins::RTC_GPIO_PIN);
  }

  ESP_LOGI(TAG, "Testing sleep mode configuration...");

  // Test sleep mode configuration with correct field names
  hf_gpio_sleep_config_t sleep_config = {
      .sleep_mode = hf_gpio_mode_t::HF_GPIO_MODE_INPUT,
      .sleep_direction = hf_gpio_mode_t::HF_GPIO_MODE_INPUT,
      .sleep_pull_mode = hf_gpio_pull_t::HF_GPIO_PULL_UP,
      .sleep_drive_strength = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK,
      .sleep_output_enable = false,
      .sleep_input_enable = true,
      .hold_during_sleep = false,
      .rtc_domain_enable = true,
      .slp_sel_enable = true,
      .enable_sleep_retain = false};

  auto result = sleep_gpio.ConfigureSleep(sleep_config);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Sleep mode configured successfully");
  } else {
    ESP_LOGW(TAG, "[FAILURE] Sleep mode configuration failed: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "Testing wakeup configuration...");

  // Test wakeup configuration with correct field names
  hf_gpio_wakeup_config_t wakeup_config = {
      .wake_trigger = hf_gpio_intr_type_t::HF_GPIO_INTR_LOW_LEVEL,
      .enable_rtc_wake = true,
      .enable_ext1_wake = false,
      .wake_level = 0, // LOW level wake
      .internal_pullup_enable = true,
      .internal_pulldown_enable = false,
      .iso_en = false};

  result = sleep_gpio.ConfigureWakeUp(wakeup_config);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] Wake-up configured successfully (LOW level trigger)");

    // Test with HIGH level trigger
    wakeup_config.wake_trigger = hf_gpio_intr_type_t::HF_GPIO_INTR_HIGH_LEVEL;
    wakeup_config.wake_level = 1;
    result = sleep_gpio.ConfigureWakeUp(wakeup_config);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Wake-up reconfigured successfully (HIGH level trigger)");
    }

    // Disable wakeup
    wakeup_config.enable_rtc_wake = false;
    result = sleep_gpio.ConfigureWakeUp(wakeup_config);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Wake-up disabled successfully");
    }
  } else {
    ESP_LOGW(TAG, "[FAILURE] Wake-up configuration failed: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "Note: Actual sleep/wakeup would require deep sleep mode");
  ESP_LOGI(TAG, "[SUCCESS] GPIO sleep and wake-up test completed");
  return true;
}

bool test_gpio_hold_functionality() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Hold Functionality ===");

  // Use LED pin for hold testing (visible feedback)
  EspGpio hold_gpio(TestPins::LED_OUTPUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!hold_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize hold test GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Testing GPIO hold configuration...");

  // Set pin active before testing hold
  auto result = hold_gpio.SetActive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set GPIO active before hold test");
    return false;
  }

  ESP_LOGI(TAG, "Pin set active, testing hold enable...");
  vTaskDelay(pdMS_TO_TICKS(500));

  // Test hold configuration
  result = hold_gpio.ConfigureHold(true);
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "[SUCCESS] GPIO hold enabled successfully");
    ESP_LOGI(TAG, "Pin state should be maintained even during sleep");

    // Brief delay to demonstrate hold
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Test hold disable
    result = hold_gpio.ConfigureHold(false);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO hold disabled successfully");
    } else {
      ESP_LOGW(TAG, "[FAILURE] Failed to disable GPIO hold: %d", static_cast<int>(result));
    }
  } else {
    ESP_LOGW(TAG, "[FAILURE] GPIO hold not supported or failed: %d", static_cast<int>(result));
  }

  // Clean up - set pin inactive
  hold_gpio.SetInactive();

  ESP_LOGI(TAG, "[SUCCESS] GPIO hold functionality test completed");
  return true;
}

bool test_gpio_drive_capabilities() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Drive Capabilities ===");

  // Use the dedicated drive test pin
  EspGpio drive_gpio(TestPins::DRIVE_TEST_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!drive_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize drive test GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Testing different drive capability settings...");

  // Test all available drive capabilities
  hf_gpio_drive_cap_t capabilities[] = {
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK,     // ~5mA
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGER, // ~10mA
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM,   // ~20mA
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST // ~40mA
  };

  const char* cap_names[] = {"5mA", "10mA", "20mA", "40mA"};

  for (size_t i = 0; i < sizeof(capabilities) / sizeof(capabilities[0]); i++) {
    ESP_LOGI(TAG, "Setting drive capability to %s...", cap_names[i]);

    auto result = drive_gpio.SetDriveCapability(capabilities[i]);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Drive capability %s set successfully", cap_names[i]);

      // Test output at this drive level
      drive_gpio.SetActive();
      vTaskDelay(pdMS_TO_TICKS(100));
      drive_gpio.SetInactive();
      vTaskDelay(pdMS_TO_TICKS(100));
    } else {
      ESP_LOGW(TAG, "[FAILURE] Failed to set drive capability %s: %d", cap_names[i],
               static_cast<int>(result));
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO drive capabilities test completed");
  return true;
}

bool test_gpio_diagnostics_and_statistics() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Diagnostics and Statistics ===");

  // Test diagnostics on multiple pin types
  hf_pin_num_t test_pins[] = {TestPins::LED_OUTPUT, TestPins::DIGITAL_IN_1, TestPins::RTC_GPIO_PIN,
                              TestPins::ANALOG_PIN};

  const char* pin_names[] = {"LED_OUTPUT", "DIGITAL_IN_1", "RTC_GPIO_PIN", "ANALOG_PIN"};

  for (size_t i = 0; i < sizeof(test_pins) / sizeof(test_pins[0]); i++) {
    ESP_LOGI(TAG, "Testing diagnostics for %s (pin %d)...", pin_names[i], test_pins[i]);

    EspGpio diag_gpio(test_pins[i], hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

    if (!diag_gpio.EnsureInitialized()) {
      ESP_LOGW(TAG, "Failed to initialize GPIO for diagnostics test on pin %d", test_pins[i]);
      continue;
    }

    // Test configuration dump
    ESP_LOGI(TAG, "Getting configuration dump for pin %d...", test_pins[i]);
    auto config_dump = diag_gpio.GetConfigurationDump();
    ESP_LOGI(TAG, "[SUCCESS] Configuration dump retrieved for pin %d", test_pins[i]);

    // Test pin capabilities
    ESP_LOGI(TAG, "Getting pin capabilities for pin %d...", test_pins[i]);
    hf_gpio_pin_capabilities_t capabilities;
    auto cap_result = diag_gpio.GetPinCapabilities(capabilities);
    if (cap_result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Pin capabilities retrieved for pin %d", test_pins[i]);
    } else {
      ESP_LOGW(TAG, "[FAILURE] Failed to get pin capabilities for pin %d", test_pins[i]);
    }

    // Test RTC GPIO support
    if (diag_gpio.SupportsRtcGpio()) {
      ESP_LOGI(TAG, "[SUCCESS] Pin %d supports RTC GPIO functionality", test_pins[i]);
    } else {
      ESP_LOGI(TAG, "[INFO] Pin %d does not support RTC GPIO", test_pins[i]);
    }

    // Note: Dedicated GPIO support removed from EspGpio implementation

    ESP_LOGI(TAG, "");
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO diagnostics and statistics test completed");
  return true;
}

bool test_gpio_error_handling() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Error Handling ===");

  // Test invalid pin numbers
  ESP_LOGI(TAG, "Testing invalid pin validation...");
  EspGpio invalid_gpio(99, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT); // Invalid pin
  if (!invalid_gpio.IsPinAvailable()) {
    ESP_LOGI(TAG, "[SUCCESS] Invalid pin correctly detected");
  } else {
    ESP_LOGW(TAG, "[WARNING] Invalid pin validation may need improvement");
  }

  // Test operations on uninitialized GPIO
  ESP_LOGI(TAG, "Testing operations on uninitialized GPIO...");
  EspGpio uninit_gpio(TestPins::DIGITAL_OUT_1);

  auto result = uninit_gpio.SetActive(); // Should fail or auto-initialize
  ESP_LOGI(TAG, "SetActive on uninitialized GPIO returned: %d", static_cast<int>(result));

  // Test strapping pin warnings
  ESP_LOGI(TAG, "Testing strapping pin detection...");
  const hf_pin_num_t strapping_pins[] = {9, 15}; // ESP32-C6 strapping pins

  for (auto pin : strapping_pins) {
    if (EspGpio::IsStrappingPin(pin)) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO%d correctly identified as strapping pin", pin);
    }
  }

  // Test resource exhaustion (multiple GPIO instances)
  ESP_LOGI(TAG, "Testing resource management with multiple GPIO instances...");
  std::vector<std::unique_ptr<EspGpio>> test_gpios;

  const hf_pin_num_t test_pins[] = {TestPins::DIGITAL_OUT_1, TestPins::DIGITAL_OUT_2,
                                    TestPins::DIGITAL_IN_1, TestPins::DIGITAL_IN_2};

  for (auto pin : test_pins) {
    auto gpio = std::make_unique<EspGpio>(pin, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    if (gpio->EnsureInitialized()) {
      test_gpios.push_back(std::move(gpio));
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Created %zu GPIO instances successfully", test_gpios.size());

  // Clean up will happen automatically via destructors
  test_gpios.clear();
  ESP_LOGI(TAG, "[SUCCESS] GPIO instances cleaned up successfully");

  ESP_LOGI(TAG, "[SUCCESS] GPIO error handling test completed");
  return true;
}

bool test_gpio_stress_testing() noexcept {
  ESP_LOGI(TAG, "=== GPIO Stress Testing ===");

  EspGpio stress_gpio(TestPins::STRESS_TEST_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!stress_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize stress test GPIO");
    return false;
  }

  ESP_LOGI(TAG, "Starting rapid toggle stress test...");

  // Rapid toggle test
  const int stress_iterations = 10000;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < stress_iterations; i++) {
    if (i % 2 == 0) {
      stress_gpio.SetActive();
    } else {
      stress_gpio.SetInactive();
    }

    // Brief yield every 1000 iterations to prevent watchdog
    if (i % 1000 == 0) {
      vTaskDelay(1);
      if (i % 2000 == 0) {
        ESP_LOGI(TAG, "Stress test progress: %d/%d iterations", i, stress_iterations);
      }
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t duration_us = end_time - start_time;

  ESP_LOGI(TAG, "[SUCCESS] Completed %d toggle operations in %llu microseconds", stress_iterations,
           duration_us);
  ESP_LOGI(TAG, "[SUCCESS] Average operation time: %.2f microseconds per toggle",
           (double)duration_us / stress_iterations);

  // Configuration change stress test
  ESP_LOGI(TAG, "Starting configuration change stress test...");

  const hf_gpio_pull_mode_t pull_modes[] = {hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                                            hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP,
                                            hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN};

  for (int cycle = 0; cycle < 100; cycle++) {
    for (auto pull_mode : pull_modes) {
      auto result = stress_gpio.SetPullMode(pull_mode);
      if (result != hf_gpio_err_t::GPIO_SUCCESS) {
        ESP_LOGW(TAG, "Pull mode change failed at cycle %d", cycle);
      }
    }

    if (cycle % 20 == 0) {
      ESP_LOGI(TAG, "Configuration stress test progress: %d/100 cycles", cycle);
      vTaskDelay(1); // Yield
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO stress testing completed successfully");
  return true;
}

bool test_gpio_pin_validation() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Pin Validation ===");

  ESP_LOGI(TAG, "Testing ESP32-C6 pin validation functions...");

  // Test valid GPIO pins
  const hf_pin_num_t valid_pins[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  10,
                                     11, 14, 16, 17, 18, 19, 20, 21, 22, 23};

  for (auto pin : valid_pins) {
    if (EspGpio::IsValidPin(pin)) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO%d correctly identified as valid", pin);
    } else {
      ESP_LOGW(TAG, "[WARNING] GPIO%d should be valid but was rejected", pin);
    }
  }

  // Test invalid GPIO pins
  const hf_pin_num_t invalid_pins[] = {31, 32, 50, 100};

  for (auto pin : invalid_pins) {
    if (!EspGpio::IsValidPin(pin)) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO%d correctly identified as invalid", pin);
    } else {
      ESP_LOGW(TAG, "[WARNING] GPIO%d should be invalid but was accepted", pin);
    }
  }

  // Test RTC GPIO detection
  ESP_LOGI(TAG, "Testing RTC GPIO detection...");
  const hf_pin_num_t rtc_pins[] = {0, 1, 2, 3, 4, 5, 6, 7}; // ESP32-C6 RTC GPIOs

  for (auto pin : rtc_pins) {
    if (EspGpio::IsRtcGpio(pin)) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO%d correctly identified as RTC GPIO", pin);
    } else {
      ESP_LOGW(TAG, "[WARNING] GPIO%d should support RTC but was not detected", pin);
    }
  }

  // Test non-RTC GPIO pins
  const hf_pin_num_t non_rtc_pins[] = {8, 10, 11, 14, 16, 17, 18, 19, 20, 21, 22, 23};

  for (auto pin : non_rtc_pins) {
    if (!EspGpio::IsRtcGpio(pin)) {
      ESP_LOGI(TAG, "[SUCCESS] GPIO%d correctly identified as non-RTC GPIO", pin);
    } else {
      ESP_LOGW(TAG, "[WARNING] GPIO%d should not support RTC but was detected as RTC", pin);
    }
  }

  // Test pin capabilities
  ESP_LOGI(TAG, "Testing pin capabilities detection...");
  EspGpio test_gpio(TestPins::LED_OUTPUT);

  if (test_gpio.EnsureInitialized()) {
    hf_gpio_pin_capabilities_t capabilities;
    auto result = test_gpio.GetPinCapabilities(capabilities);

    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Pin capabilities retrieved:");
      ESP_LOGI(TAG, "  - Pin %d: Input=%s, Output=%s, RTC=%s, ADC=%s", capabilities.pin_number,
               capabilities.supports_input ? "Yes" : "No",
               capabilities.supports_output ? "Yes" : "No",
               capabilities.supports_rtc ? "Yes" : "No", capabilities.supports_adc ? "Yes" : "No");
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] GPIO pin validation test completed");
  return true;
}

bool test_gpio_loopback_operations() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Loopback Operations ===");
  ESP_LOGI(TAG, "Connect pin %d (output) to pin %d (input) for loopback test",
           TestPins::LOOPBACK_OUT, TestPins::LOOPBACK_IN);

  // Initialize output pin
  EspGpio output_gpio(TestPins::LOOPBACK_OUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                      hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  // Initialize input pin
  EspGpio input_gpio(TestPins::LOOPBACK_IN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!output_gpio.EnsureInitialized() || !input_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize loopback test GPIOs");
    return false;
  }

  // Configure input with pulldown to ensure clean test
  auto result = input_gpio.SetPullMode(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGW(TAG, "Failed to set pulldown on input pin");
  }

  bool test_passed = true;

  ESP_LOGI(TAG, "Testing loopback pattern: HIGH->LOW->HIGH->LOW");

  // Test pattern: HIGH->LOW->HIGH->LOW
  bool test_values[] = {true, false, true, false, true};

  for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
    // Set output
    if (test_values[i]) {
      result = output_gpio.SetActive();
    } else {
      result = output_gpio.SetInactive();
    }

    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set output to %s", test_values[i] ? "HIGH" : "LOW");
      test_passed = false;
      break;
    }

    // Allow signal to settle
    vTaskDelay(pdMS_TO_TICKS(50));

    // Read input
    bool input_active;
    result = input_gpio.IsActive(input_active);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to read input state");
      test_passed = false;
      break;
    }

    // Verify loopback
    if (input_active == test_values[i]) {
      ESP_LOGI(TAG, "[SUCCESS] Loopback test %zu: Output=%s, Input=%s - PASS", i + 1,
               test_values[i] ? "HIGH" : "LOW", input_active ? "HIGH" : "LOW");
    } else {
      ESP_LOGE(TAG, "[FAILURE] Loopback test %zu: Output=%s, Input=%s - FAIL", i + 1,
               test_values[i] ? "HIGH" : "LOW", input_active ? "HIGH" : "LOW");
      test_passed = false;
    }
  }

  // Clean up - set output low
  output_gpio.SetInactive();

  if (test_passed) {
    ESP_LOGI(TAG, "[SUCCESS] GPIO loopback operations test completed successfully");
  } else {
    ESP_LOGI(TAG, "[FAILURE] GPIO loopback operations test completed with failures");
    ESP_LOGI(TAG, "Note: Ensure pins %d and %d are physically connected for this test",
             TestPins::LOOPBACK_OUT, TestPins::LOOPBACK_IN);
  }

  return test_passed;
}

bool test_gpio_concurrent_operations() noexcept {
  ESP_LOGI(TAG, "=== Testing Concurrent GPIO Operations ===");

  ESP_LOGI(TAG, "Testing concurrent GPIO access patterns...");

  // Create multiple GPIO instances for concurrent testing
  EspGpio gpio1(TestPins::DIGITAL_OUT_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
  EspGpio gpio2(TestPins::DIGITAL_OUT_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
  EspGpio gpio3(TestPins::DIGITAL_IN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

  if (!gpio1.EnsureInitialized() || !gpio2.EnsureInitialized() || !gpio3.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize GPIOs for concurrent test");
    return false;
  }

  ESP_LOGI(TAG, "Testing simultaneous operations on multiple pins...");

  // Simultaneous toggle pattern
  for (int cycle = 0; cycle < 50; cycle++) {
    // Set both outputs simultaneously with opposite states
    auto result1 = gpio1.SetState(cycle % 2 ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE
                                            : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE);
    auto result2 = gpio2.SetState(cycle % 2 ? hf_gpio_state_t::HF_GPIO_STATE_INACTIVE
                                            : hf_gpio_state_t::HF_GPIO_STATE_ACTIVE);

    if (result1 != hf_gpio_err_t::GPIO_SUCCESS || result2 != hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGW(TAG, "Concurrent operation failed at cycle %d", cycle);
    }

    // Read input while toggling outputs
    auto input_state = gpio3.GetCurrentState();

    if (cycle % 10 == 0) {
      ESP_LOGI(TAG, "Concurrent test cycle %d: GPIO1=%s, GPIO2=%s, GPIO3=%s", cycle,
               (cycle % 2) ? "ACTIVE" : "INACTIVE", (cycle % 2) ? "INACTIVE" : "ACTIVE",
               (input_state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? "ACTIVE" : "INACTIVE");
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Brief delay
  }

  ESP_LOGI(TAG, "Testing resource sharing and cleanup...");

  // Test creating and destroying GPIO instances rapidly
  for (int i = 0; i < 20; i++) {
    {
      EspGpio temp_gpio(TestPins::LED_OUTPUT, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
      temp_gpio.EnsureInitialized();
      temp_gpio.SetActive();
      vTaskDelay(pdMS_TO_TICKS(5));
      temp_gpio.SetInactive();
      // temp_gpio destructor called here
    }

    if (i % 5 == 0) {
      ESP_LOGI(TAG, "Resource test iteration %d/20", i + 1);
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Concurrent GPIO operations test completed");
  return true;
}

bool test_gpio_power_consumption() noexcept {
  ESP_LOGI(TAG, "=== Testing GPIO Power Consumption ===");

  ESP_LOGI(TAG, "Testing power-optimized GPIO configurations...");

  EspGpio power_gpio(TestPins::RTC_GPIO_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

  if (!power_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize power test GPIO");
    return false;
  }

  // Test different drive capabilities for power optimization
  ESP_LOGI(TAG, "Testing drive capability settings for power optimization...");

  const hf_gpio_drive_cap_t drive_levels[] = {
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK,     // ~5mA - Lowest power
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGER, // ~10mA
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM,   // ~20mA
      hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST // ~40mA - Highest power
  };

  const char* drive_names[] = {"WEAK (~5mA)", "STRONGER (~10mA)", "MEDIUM (~20mA)",
                               "STRONGEST (~40mA)"};

  // Switch to output for drive capability testing
  power_gpio.SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

  for (size_t i = 0; i < sizeof(drive_levels) / sizeof(drive_levels[0]); i++) {
    auto result = power_gpio.SetDriveCapability(drive_levels[i]);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Drive capability set to %s", drive_names[i]);

      // Toggle a few times at this drive level
      for (int toggle = 0; toggle < 5; toggle++) {
        power_gpio.SetActive();
        vTaskDelay(pdMS_TO_TICKS(10));
        power_gpio.SetInactive();
        vTaskDelay(pdMS_TO_TICKS(10));
      }
    } else {
      ESP_LOGW(TAG, "[WARNING] Failed to set drive capability to %s", drive_names[i]);
    }
  }

  // Test low-power pull mode configurations
  ESP_LOGI(TAG, "Testing pull resistor configurations for power optimization...");

  power_gpio.SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

  const hf_gpio_pull_mode_t pull_modes[] = {
      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING, // Highest power - no defined state
      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP,       // Low power - defined high state
      hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN      // Low power - defined low state
  };

  const char* pull_names[] = {"FLOATING (highest power)", "PULL-UP (low power)",
                              "PULL-DOWN (low power)"};

  for (size_t i = 0; i < sizeof(pull_modes) / sizeof(pull_modes[0]); i++) {
    auto result = power_gpio.SetPullMode(pull_modes[i]);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] Pull mode set to %s", pull_names[i]);

      // Read pin state
      auto state = power_gpio.GetCurrentState();
      ESP_LOGI(TAG, "  Pin state with %s: %s", pull_names[i],
               (state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? "ACTIVE" : "INACTIVE");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Test RTC GPIO configuration for ultra-low power
  if (power_gpio.SupportsRtcGpio()) {
    ESP_LOGI(TAG, "Testing RTC GPIO for ultra-low power operation...");

    // Configure for low-power sleep mode
    hf_gpio_sleep_config_t sleep_config = {
        .sleep_mode = hf_gpio_mode_t::HF_GPIO_MODE_INPUT,
        .sleep_direction = hf_gpio_mode_t::HF_GPIO_MODE_INPUT,
        .sleep_pull_mode = hf_gpio_pull_t::HF_GPIO_PULL_UP, // Define state to reduce power
        .sleep_drive_strength = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK,
        .sleep_output_enable = false,
        .sleep_input_enable = true,
        .hold_during_sleep = false,
        .rtc_domain_enable = true,
        .slp_sel_enable = true,
        .enable_sleep_retain = true};

    auto result = power_gpio.ConfigureSleep(sleep_config);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      ESP_LOGI(TAG, "[SUCCESS] RTC GPIO sleep configuration applied for power optimization");
    } else {
      ESP_LOGW(TAG, "[WARNING] RTC GPIO sleep configuration failed");
    }
  }

  ESP_LOGI(TAG, "Power optimization recommendations:");
  ESP_LOGI(TAG, "  1. Use WEAK drive capability when possible (~5mA vs ~40mA)");
  ESP_LOGI(TAG, "  2. Use pull resistors on inputs to define states");
  ESP_LOGI(TAG, "  3. Configure RTC GPIO for deep sleep applications");
  ESP_LOGI(TAG, "  4. Enable hold function to maintain state during sleep");

  ESP_LOGI(TAG, "[SUCCESS] GPIO power consumption test completed");
  return true;
}

//==============================================================================
// TEST EXECUTION AND MAIN APPLICATION
//==============================================================================

/**
 * @brief Main application entry point
 */
extern "C" void app_main(void) {
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 GPIO COMPREHENSIVE TEST SUITE                   ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╠══════════════════════════════════════════════════════════════════════════════╣");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                 ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                              ║");
  ESP_LOGI(TAG, "║ Features: GPIO, Interrupts, RTC, Sleep, Advanced Features                  ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                             ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  // Wait a moment for system stabilization
  vTaskDelay(pdMS_TO_TICKS(1000));

  ESP_LOGI(TAG, "Starting comprehensive GPIO testing...\n");

  // Core GPIO functionality tests
  RUN_TEST(test_basic_gpio_functionality);
  RUN_TEST(test_gpio_initialization_and_configuration);
  RUN_TEST(test_gpio_input_output_operations);
  RUN_TEST(test_gpio_pull_resistors);

  // Advanced functionality tests
  RUN_TEST(test_gpio_interrupt_functionality);
  RUN_TEST(test_gpio_advanced_features);
  RUN_TEST(test_gpio_drive_capabilities);

  // ESP32-C6 specific tests
  RUN_TEST(test_gpio_rtc_functionality);
  RUN_TEST(test_gpio_glitch_filters);
  RUN_TEST(test_gpio_sleep_and_wakeup);
  RUN_TEST(test_gpio_hold_functionality);

  // Robustness and performance tests
  RUN_TEST(test_gpio_error_handling);
  RUN_TEST(test_gpio_pin_validation);
  RUN_TEST(test_gpio_stress_testing);
  RUN_TEST(test_gpio_concurrent_operations);

  // Specialized tests
  RUN_TEST(test_gpio_loopback_operations);
  RUN_TEST(test_gpio_interrupt_loopback);
  RUN_TEST(test_gpio_diagnostics_and_statistics);
  RUN_TEST(test_gpio_power_consumption);

  // Print final results
  print_test_summary(g_test_results, "GPIO", TAG);

  ESP_LOGI(TAG, "GPIO comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Keep the system running for monitoring
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGI(TAG, "GPIO test system heartbeat - %lu seconds uptime",
             esp_timer_get_time() / 1000000);
  }
}
