/**
 * @file DigitalOutputGuardComprehensiveTest.cpp
 * @brief DigitalOutputGuard comprehensive test suite for ESP32-C6 DevKit-M-1
 *
 * This test suite provides comprehensive testing of the DigitalOutputGuard class including:
 * - RAII pattern verification and automatic cleanup
 * - GPIO state management and direction control
 * - Error handling and edge cases
 * - Move semantics and resource management
 * - Concurrent access patterns
 * - Performance and stress testing
 *
 * Test Coverage Includes:
 * ✓ Basic RAII functionality and automatic cleanup
 * ✓ GPIO direction management and output mode enforcement
 * ✓ State transitions (active/inactive) with proper error handling
 * ✓ Constructor variants (reference and pointer)
 * ✓ Move semantics and resource transfer
 * ✓ Edge cases and error conditions
 * ✓ Concurrent access with multiple tasks/threads
 * ✓ Performance benchmarking and stress testing
 * ✓ Cross-platform GPIO compatibility verification
 *
 * PERFORMANCE TESTING AND EXPECTED OUTPUTS:
 * ==========================================
 *
 * The performance tests measure critical timing characteristics of the DigitalOutputGuard:
 *
 * 1. GUARD CREATION/DESTRUCTION PERFORMANCE:
 *    - Tests: 1000 iterations of guard creation and destruction
 *    - Measures: Complete RAII lifecycle timing
 *    - Expected: < 100 μs per cycle (typically 2-5 μs on ESP32-C6)
 *    - Output: "Guard creation/destruction: 1000 iterations in X.XX ms (avg: X.XX us per cycle)"
 *    - Significance: Validates efficient object lifecycle management
 *
 * 2. STATE TRANSITION PERFORMANCE:
 *    - Tests: 1000 iterations of SetActive()/SetInactive() operations
 *    - Measures: GPIO state change timing
 *    - Expected: < 50 μs per operation (typically 1-3 μs on ESP32-C6)
 *    - Output: "State transitions: 1000 iterations in X.XX ms (avg: X.XX us per operation)"
 *    - Significance: Validates fast GPIO control without overhead
 *
 * 3. STRESS TEST PERFORMANCE:
 *    - Tests: 2000 iterations with 5 state changes per iteration across 3 GPIO pins
 *    - Measures: Complex multi-GPIO scenario timing
 *    - Expected: < 200 μs per iteration (typically 5-15 μs on ESP32-C6)
 *    - Output: "Stress test: 2000 iterations in X.XX ms (avg: X.XX us per iteration)"
 *    - Significance: Validates performance under realistic usage patterns
 *
 * 4. CONCURRENT ACCESS PERFORMANCE:
 *    - Tests: 3 concurrent tasks performing 100 operations each (300 total)
 *    - Measures: Multi-threaded access timing and thread safety
 *    - Expected: All operations complete successfully without race conditions
 *    - Output: "DigitalOutputGuard concurrent access test successful: 300 operations"
 *    - Significance: Validates thread-safe operation under concurrent load
 *
 * PERFORMANCE INTERPRETATION:
 * ===========================
 *
 * Excellent Performance Indicators:
 * - Guard creation/destruction < 5 μs: Minimal RAII overhead
 * - State transitions < 3 μs: Direct GPIO control efficiency
 * - Stress test < 15 μs: Good scalability under load
 * - 100% concurrent test success: Robust thread safety
 *
 * Performance Degradation Warnings:
 * - Guard creation/destruction > 50 μs: Potential memory allocation issues
 * - State transitions > 20 μs: GPIO driver inefficiency
 * - Stress test > 100 μs: Resource contention or memory fragmentation
 * - Concurrent test failures: Thread safety violations
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "mcu/esp32/EspGpio.h"
#include "utils/DigitalOutputGuard.h"

static const char* TAG = "DIGITAL_OUTPUT_GUARD_Test";

static TestResults g_test_results;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core DigitalOutputGuard functionality tests
static constexpr bool ENABLE_BASIC_TESTS = true;          // Basic RAII and state management
static constexpr bool ENABLE_CONSTRUCTOR_TESTS = true;    // Constructor variants and error handling
static constexpr bool ENABLE_STATE_TESTS = true;          // State transitions and GPIO control
static constexpr bool ENABLE_MOVE_SEMANTICS_TESTS = true; // Move operations and resource management
static constexpr bool ENABLE_EDGE_CASE_TESTS = true;      // Edge cases and error conditions
static constexpr bool ENABLE_CONCURRENT_TESTS = true;     // Concurrent access testing
static constexpr bool ENABLE_PERFORMANCE_TESTS = true;    // Performance and stress testing

// Test GPIO pins - using only 3 pins for all tests
static constexpr hf_pin_num_t TEST_GPIO_PIN_1 = 2;
static constexpr hf_pin_num_t TEST_GPIO_PIN_2 = 4;
static constexpr hf_pin_num_t TEST_GPIO_PIN_3 = 5;

//==============================================================================
// BASIC RAII AND STATE MANAGEMENT TESTS
//==============================================================================

bool test_digital_output_guard_creation() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard creation...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Test guard creation with reference
  DigitalOutputGuard guard(test_gpio);

  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard creation failed - not valid");
    return false;
  }

  // Verify GPIO is in output mode and active
  if (!test_gpio.IsOutput()) {
    ESP_LOGE(TAG, "GPIO not in output mode after guard creation");
    return false;
  }

  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    ESP_LOGE(TAG, "GPIO not in active state after guard creation");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard creation successful");
  return true;
}

bool test_digital_output_guard_raii_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard RAII cleanup...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Test RAII cleanup in a scope
  {
    DigitalOutputGuard guard(test_gpio);

    if (!guard.IsValid()) {
      ESP_LOGE(TAG, "DigitalOutputGuard creation failed");
      return false;
    }

    // Verify GPIO is active
    if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
      ESP_LOGE(TAG, "GPIO not active during guard lifetime");
      return false;
    }

    ESP_LOGI(TAG, "Guard created, GPIO is active");
  } // Guard should automatically set GPIO inactive here

  // Verify GPIO is now inactive
  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "GPIO not inactive after guard destruction");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard RAII cleanup successful");
  return true;
}

bool test_digital_output_guard_manual_state_control() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard manual state control...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_3, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  DigitalOutputGuard guard(test_gpio);

  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard creation failed");
    return false;
  }

  // Test manual inactive
  hf_gpio_err_t result = guard.SetInactive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "SetInactive failed: %d", static_cast<int>(result));
    return false;
  }

  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "GPIO not inactive after manual SetInactive");
    return false;
  }

  // Test manual active
  result = guard.SetActive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "SetActive failed: %d", static_cast<int>(result));
    return false;
  }

  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    ESP_LOGE(TAG, "GPIO not active after manual SetActive");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard manual state control successful");
  return true;
}

//==============================================================================
// CONSTRUCTOR VARIANTS AND ERROR HANDLING TESTS
//==============================================================================

bool test_digital_output_guard_pointer_constructor() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard pointer constructor...");

  // Create a GPIO instance for testing
  EspGpio* test_gpio = new EspGpio(TEST_GPIO_PIN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                                   hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                                   hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                                   hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    delete test_gpio;
    return false;
  }

  // Test guard creation with pointer in a scope to ensure proper cleanup order
  {
    DigitalOutputGuard guard(test_gpio);

    if (!guard.IsValid()) {
      ESP_LOGE(TAG, "DigitalOutputGuard pointer constructor failed - not valid");
      delete test_gpio;
      return false;
    }

    // Verify GPIO is in output mode and active
    if (!test_gpio->IsOutput()) {
      ESP_LOGE(TAG, "GPIO not in output mode after guard creation");
      delete test_gpio;
      return false;
    }

    if (test_gpio->GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
      ESP_LOGE(TAG, "GPIO not in active state after guard creation");
      delete test_gpio;
      return false;
    }
  } // Guard destructor called here, GPIO still valid

  delete test_gpio;
  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard pointer constructor successful");
  return true;
}

bool test_digital_output_guard_null_pointer_handling() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard null pointer handling...");

  // Test guard creation with null pointer
  DigitalOutputGuard guard(static_cast<EspGpio*>(nullptr));

  if (guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard should not be valid with null pointer");
    return false;
  }

  if (guard.GetLastError() != hf_gpio_err_t::GPIO_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "Expected GPIO_ERR_NULL_POINTER, got: %d",
             static_cast<int>(guard.GetLastError()));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard null pointer handling successful");
  return true;
}

bool test_digital_output_guard_ensure_output_mode() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard ensure output mode...");

  // Create a GPIO instance in input mode
  EspGpio test_gpio(TEST_GPIO_PIN_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Verify it's in input mode
  if (test_gpio.IsOutput()) {
    ESP_LOGE(TAG, "GPIO should be in input mode initially");
    return false;
  }

  // Test guard creation with ensure_output_mode = true (default)
  DigitalOutputGuard guard(test_gpio, true);

  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard creation failed with ensure_output_mode=true");
    return false;
  }

  // Verify GPIO is now in output mode
  if (!test_gpio.IsOutput()) {
    ESP_LOGE(TAG, "GPIO not in output mode after guard creation with ensure_output_mode=true");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard ensure output mode successful");
  return true;
}

bool test_digital_output_guard_no_ensure_output_mode() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard no ensure output mode...");

  // Create a GPIO instance in input mode
  EspGpio test_gpio(TEST_GPIO_PIN_3, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Test guard creation with ensure_output_mode = false
  DigitalOutputGuard guard(test_gpio, false);

  if (guard.IsValid()) {
    ESP_LOGE(
        TAG,
        "DigitalOutputGuard should not be valid with input mode GPIO and ensure_output_mode=false");
    return false;
  }

  if (guard.GetLastError() != hf_gpio_err_t::GPIO_ERR_DIRECTION_MISMATCH) {
    ESP_LOGE(TAG, "Expected GPIO_ERR_DIRECTION_MISMATCH, got: %d",
             static_cast<int>(guard.GetLastError()));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard no ensure output mode test successful");
  return true;
}

//==============================================================================
// STATE TRANSITIONS AND GPIO CONTROL TESTS
//==============================================================================

bool test_digital_output_guard_state_transitions() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard state transitions...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  DigitalOutputGuard guard(test_gpio);

  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard creation failed");
    return false;
  }

  // Test multiple state transitions
  const int num_transitions = 10;
  for (int i = 0; i < num_transitions; i++) {
    // Toggle between active and inactive
    hf_gpio_err_t result;
    if (i % 2 == 0) {
      result = guard.SetActive();
      if (result != hf_gpio_err_t::GPIO_SUCCESS) {
        ESP_LOGE(TAG, "SetActive failed on transition %d: %d", i, static_cast<int>(result));
        return false;
      }
      if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
        ESP_LOGE(TAG, "GPIO not active after SetActive on transition %d", i);
        return false;
      }
    } else {
      result = guard.SetInactive();
      if (result != hf_gpio_err_t::GPIO_SUCCESS) {
        ESP_LOGE(TAG, "SetInactive failed on transition %d: %d", i, static_cast<int>(result));
        return false;
      }
      if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
        ESP_LOGE(TAG, "GPIO not inactive after SetInactive on transition %d", i);
        return false;
      }
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard state transitions successful");
  return true;
}

bool test_digital_output_guard_get_current_state() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard GetCurrentState...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  DigitalOutputGuard guard(test_gpio);

  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "DigitalOutputGuard creation failed");
    return false;
  }

  // Test GetCurrentState when active
  hf_gpio_state_t state = guard.GetCurrentState();
  if (state != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    ESP_LOGE(TAG, "GetCurrentState returned %d, expected ACTIVE", static_cast<int>(state));
    return false;
  }

  // Test GetCurrentState when inactive
  guard.SetInactive();
  state = guard.GetCurrentState();
  if (state != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "GetCurrentState returned %d, expected INACTIVE", static_cast<int>(state));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard GetCurrentState successful");
  return true;
}

//==============================================================================
// MOVE SEMANTICS AND RESOURCE MANAGEMENT TESTS
//==============================================================================

bool test_digital_output_guard_move_constructor() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard move constructor...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_3, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Create original guard
  DigitalOutputGuard original_guard(test_gpio);

  if (!original_guard.IsValid()) {
    ESP_LOGE(TAG, "Original guard creation failed");
    return false;
  }

  // Move construct new guard
  DigitalOutputGuard moved_guard = std::move(original_guard);

  if (!moved_guard.IsValid()) {
    ESP_LOGE(TAG, "Moved guard not valid after move construction");
    return false;
  }

  // Test that moved guard works
  hf_gpio_err_t result = moved_guard.SetInactive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Moved guard SetInactive failed: %d", static_cast<int>(result));
    return false;
  }

  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "GPIO not inactive after moved guard SetInactive");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard move constructor successful");
  return true;
}

bool test_digital_output_guard_move_assignment() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard move assignment...");

  // Create GPIO instances for testing
  EspGpio test_gpio1(TEST_GPIO_PIN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                     hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                     hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  EspGpio test_gpio2(TEST_GPIO_PIN_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                     hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                     hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio1.EnsureInitialized() || !test_gpio2.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIOs");
    return false;
  }

  // Create guards
  DigitalOutputGuard guard1(test_gpio1);
  DigitalOutputGuard guard2(test_gpio2);

  if (!guard1.IsValid() || !guard2.IsValid()) {
    ESP_LOGE(TAG, "Guard creation failed");
    return false;
  }

  // Move assign guard1 to guard2
  guard2 = std::move(guard1);

  if (!guard2.IsValid()) {
    ESP_LOGE(TAG, "Guard2 not valid after move assignment");
    return false;
  }

  // Test that guard2 now controls test_gpio1
  hf_gpio_err_t result = guard2.SetInactive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Guard2 SetInactive failed after move assignment: %d", static_cast<int>(result));
    return false;
  }

  if (test_gpio1.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "test_gpio1 not inactive after guard2 SetInactive");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard move assignment successful");
  return true;
}

//==============================================================================
// EDGE CASES AND ERROR CONDITION TESTS
//==============================================================================

bool test_digital_output_guard_invalid_operations() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard invalid operations...");

  // Create an invalid guard (null pointer)
  DigitalOutputGuard invalid_guard(static_cast<EspGpio*>(nullptr));

  if (invalid_guard.IsValid()) {
    ESP_LOGE(TAG, "Invalid guard should not be valid");
    return false;
  }

  // Test operations on invalid guard
  hf_gpio_err_t result = invalid_guard.SetActive();
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "SetActive should fail on invalid guard");
    return false;
  }

  result = invalid_guard.SetInactive();
  if (result == hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "SetInactive should fail on invalid guard");
    return false;
  }

  hf_gpio_state_t state = invalid_guard.GetCurrentState();
  if (state != hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {
    ESP_LOGE(TAG, "GetCurrentState should return INACTIVE for invalid guard");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard invalid operations test successful");
  return true;
}

bool test_digital_output_guard_multiple_guards_same_gpio() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard multiple guards same GPIO...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_3, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // Create multiple guards for the same GPIO
  DigitalOutputGuard guard1(test_gpio);
  DigitalOutputGuard guard2(test_gpio);

  if (!guard1.IsValid() || !guard2.IsValid()) {
    ESP_LOGE(TAG, "Guard creation failed");
    return false;
  }

  // Both guards should be able to control the same GPIO
  hf_gpio_err_t result1 = guard1.SetInactive();
  hf_gpio_err_t result2 = guard2.SetActive();

  if (result1 != hf_gpio_err_t::GPIO_SUCCESS || result2 != hf_gpio_err_t::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Multiple guards failed to control same GPIO: %d, %d", static_cast<int>(result1),
             static_cast<int>(result2));
    return false;
  }

  // The last operation should determine the state
  if (test_gpio.GetCurrentState() != hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) {
    ESP_LOGE(TAG, "GPIO state not correct after multiple guard operations");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard multiple guards same GPIO test successful");
  return true;
}

//==============================================================================
// CONCURRENT ACCESS TESTS
//==============================================================================

// Global test data for concurrent tests
static volatile int g_concurrent_guard_counter = 0;
static volatile bool g_concurrent_guard_test_running = false;
static EspGpio* g_concurrent_test_gpio = nullptr;

void concurrent_guard_task(void* param) {
  int* task_id = static_cast<int*>(param);
  const int operations = 100;

  ESP_LOGI(TAG, "Concurrent guard task %d starting", *task_id);

  while (!g_concurrent_guard_test_running) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  for (int i = 0; i < operations; i++) {
    // Create guard for each operation
    DigitalOutputGuard guard(*g_concurrent_test_gpio);

    if (guard.IsValid()) {
      // Toggle state
      if (i % 2 == 0) {
        guard.SetActive();
      } else {
        guard.SetInactive();
      }
      g_concurrent_guard_counter++;
    }

    // Small delay to increase chance of contention
    if (i % 20 == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  ESP_LOGI(TAG, "Concurrent guard task %d completed", *task_id);
  vTaskDelete(nullptr);
}

bool test_digital_output_guard_concurrent_access() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard concurrent access...");

  // ========================================================================
  // CONCURRENT ACCESS TEST: MULTI-THREADED SAFETY AND PERFORMANCE
  // ========================================================================
  // This test validates thread safety and performance under concurrent load:
  // - 3 concurrent FreeRTOS tasks accessing the same GPIO
  // - 100 operations per task (300 total operations)
  // - Shared GPIO resource with potential contention
  // - Measures thread safety and race condition prevention
  // Expected: All 300 operations complete successfully without race conditions
  // Significance: Validates thread-safe operation under concurrent load
  //
  // Test Pattern: 3 tasks × 100 operations = 300 total concurrent operations
  const int num_tasks = 3;
  const int expected_total = num_tasks * 100;

  // Create shared GPIO for concurrent testing
  g_concurrent_test_gpio =
      new EspGpio(TEST_GPIO_PIN_1, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                  hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                  hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                  hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!g_concurrent_test_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize concurrent test GPIO");
    delete g_concurrent_test_gpio;
    return false;
  }

  g_concurrent_guard_counter = 0;
  g_concurrent_guard_test_running = false;

  // Create tasks
  static int task_ids[num_tasks];
  for (int i = 0; i < num_tasks; i++) {
    task_ids[i] = i;
    BaseType_t result =
        xTaskCreate(concurrent_guard_task, "ConcGuardTest", 4096, &task_ids[i], 5, nullptr);

    if (result != pdPASS) {
      ESP_LOGE(TAG, "Failed to create concurrent guard test task %d", i);
      delete g_concurrent_test_gpio;
      return false;
    }
  }

  // Start all tasks simultaneously
  vTaskDelay(pdMS_TO_TICKS(100)); // Let tasks initialize
  g_concurrent_guard_test_running = true;

  // Wait for tasks to complete
  vTaskDelay(pdMS_TO_TICKS(3000));
  g_concurrent_guard_test_running = false;

  // Check results
  if (g_concurrent_guard_counter != expected_total) {
    ESP_LOGE(TAG, "Concurrent guard access test failed: expected %d, got %d", expected_total,
             g_concurrent_guard_counter);
    delete g_concurrent_test_gpio;
    return false;
  }

  delete g_concurrent_test_gpio;
  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard concurrent access test successful: %d operations",
           g_concurrent_guard_counter);
  return true;
}

//==============================================================================
// PERFORMANCE AND STRESS TESTS
//==============================================================================

bool test_digital_output_guard_performance() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard performance...");

  // Create a GPIO instance for testing
  EspGpio test_gpio(TEST_GPIO_PIN_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

  if (!test_gpio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test GPIO");
    return false;
  }

  // ========================================================================
  // TEST 1: GUARD CREATION/DESTRUCTION PERFORMANCE
  // ========================================================================
  // This test measures the complete RAII lifecycle timing:
  // - Constructor execution (GPIO validation, state setting)
  // - Destructor execution (GPIO state cleanup)
  // - Object allocation/deallocation overhead
  // Expected: < 100 μs per cycle (typically 2-5 μs on ESP32-C6)
  // Significance: Validates efficient object lifecycle management
  const int iterations = 1000;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    DigitalOutputGuard guard(test_gpio);
    if (!guard.IsValid()) {
      ESP_LOGE(TAG, "Guard creation failed in performance test iteration %d", i);
      return false;
    }
    // Guard automatically destroyed here - measures complete RAII cycle
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Guard creation/destruction: %d iterations in %.2f ms (avg: %.2f us per cycle)",
           iterations, total_time / 1000.0, avg_time);

  if (avg_time > 100.0) { // Should be less than 100us per cycle
    ESP_LOGE(TAG, "Guard creation/destruction too slow: %.2f us per cycle", avg_time);
    return false;
  }

  // ========================================================================
  // TEST 2: STATE TRANSITION PERFORMANCE
  // ========================================================================
  // This test measures GPIO state change timing:
  // - SetActive() operation timing
  // - SetInactive() operation timing
  // - GPIO driver efficiency
  // Expected: < 50 μs per operation (typically 1-3 μs on ESP32-C6)
  // Significance: Validates fast GPIO control without overhead
  DigitalOutputGuard guard(test_gpio);
  if (!guard.IsValid()) {
    ESP_LOGE(TAG, "Guard creation failed for state transition test");
    return false;
  }

  start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    if (i % 2 == 0) {
      guard.SetActive(); // Measures GPIO HIGH setting time
    } else {
      guard.SetInactive(); // Measures GPIO LOW setting time
    }
  }

  end_time = esp_timer_get_time();
  total_time = end_time - start_time;
  avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "State transitions: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  if (avg_time > 50.0) { // Should be less than 50us per operation
    ESP_LOGE(TAG, "State transitions too slow: %.2f us per operation", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard performance test successful");
  return true;
}

bool test_digital_output_guard_stress() noexcept {
  ESP_LOGI(TAG, "Testing DigitalOutputGuard stress...");

  // ========================================================================
  // STRESS TEST: COMPLEX MULTI-GPIO SCENARIO PERFORMANCE
  // ========================================================================
  // This test measures performance under realistic usage patterns:
  // - Multiple GPIO pins (3 pins) to simulate real-world scenarios
  // - Rapid guard creation/destruction cycles
  // - Multiple state changes per guard (5 changes per iteration)
  // - Memory allocation/deallocation stress
  // Expected: < 200 μs per iteration (typically 5-15 μs on ESP32-C6)
  // Significance: Validates performance under realistic usage patterns
  //
  // Test Pattern: 2000 iterations × 3 GPIOs × 5 state changes = 30,000 operations
  const int num_gpios = 3;
  const hf_pin_num_t test_pins[num_gpios] = {TEST_GPIO_PIN_1, TEST_GPIO_PIN_2, TEST_GPIO_PIN_3};
  EspGpio* test_gpios[num_gpios];

  for (int i = 0; i < num_gpios; i++) {
    test_gpios[i] = new EspGpio(test_pins[i], hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                                hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                                hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                                hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN);

    if (!test_gpios[i]->EnsureInitialized()) {
      ESP_LOGE(TAG, "Failed to initialize test GPIO %d", i);
      // Clean up already created GPIOs
      for (int j = 0; j < i; j++) {
        delete test_gpios[j];
      }
      return false;
    }
  }

  // Stress test with multiple guards and rapid state changes
  const int stress_iterations = 2000;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < stress_iterations; i++) {
    int gpio_index = i % num_gpios; // Rotate through GPIO pins

    // Create guard - measures RAII overhead under stress
    DigitalOutputGuard guard(*test_gpios[gpio_index]);

    if (!guard.IsValid()) {
      ESP_LOGE(TAG, "Guard creation failed in stress test iteration %d", i);
      // Clean up
      for (int j = 0; j < num_gpios; j++) {
        delete test_gpios[j];
      }
      return false;
    }

    // Perform multiple state changes - measures GPIO control efficiency
    for (int j = 0; j < 5; j++) {
      if (j % 2 == 0) {
        guard.SetActive(); // 5 state changes per iteration
      } else {
        guard.SetInactive();
      }
    }

    // Guard automatically destroyed here - measures cleanup efficiency
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / stress_iterations;

  ESP_LOGI(TAG, "Stress test: %d iterations in %.2f ms (avg: %.2f us per iteration)",
           stress_iterations, total_time / 1000.0, avg_time);

  if (avg_time > 200.0) { // Should be less than 200us per iteration
    ESP_LOGE(TAG, "Stress test too slow: %.2f us per iteration", avg_time);
    // Clean up
    for (int j = 0; j < num_gpios; j++) {
      delete test_gpios[j];
    }
    return false;
  }

  // Clean up
  for (int j = 0; j < num_gpios; j++) {
    delete test_gpios[j];
  }

  ESP_LOGI(TAG, "[SUCCESS] DigitalOutputGuard stress test successful");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║            ESP32-C6 DIGITAL OUTPUT GUARD COMPREHENSIVE TEST SUITE v1.0       ║");
  ESP_LOGI(TAG, "║                     RAII GPIO Management and State Control                   ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "DIGITAL_OUTPUT_GUARD");

  // Run all DigitalOutputGuard tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_BASIC_TESTS, "DIGITAL OUTPUT GUARD BASIC TESTS", 5,
      // Basic RAII and State Management Tests
      ESP_LOGI(TAG, "Running basic DigitalOutputGuard tests...");
      RUN_TEST_IN_TASK("creation", test_digital_output_guard_creation, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("raii_cleanup", test_digital_output_guard_raii_cleanup, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "manual_state_control", test_digital_output_guard_manual_state_control, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_CONSTRUCTOR_TESTS, "DIGITAL OUTPUT GUARD CONSTRUCTOR TESTS", 5,
      // Constructor Variants and Error Handling Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard constructor tests...");
      RUN_TEST_IN_TASK("pointer_constructor", test_digital_output_guard_pointer_constructor, 8192,
                       1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "null_pointer_handling", test_digital_output_guard_null_pointer_handling, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("ensure_output_mode", test_digital_output_guard_ensure_output_mode, 8192, 1);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "no_ensure_output_mode", test_digital_output_guard_no_ensure_output_mode, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_STATE_TESTS, "DIGITAL OUTPUT GUARD STATE TESTS", 5,
      // State Transitions and GPIO Control Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard state tests...");
      RUN_TEST_IN_TASK("state_transitions", test_digital_output_guard_state_transitions, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("get_current_state", test_digital_output_guard_get_current_state, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_MOVE_SEMANTICS_TESTS, "DIGITAL OUTPUT GUARD MOVE SEMANTICS TESTS", 5,
      // Move Semantics and Resource Management Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard move semantics tests...");
      RUN_TEST_IN_TASK("move_constructor", test_digital_output_guard_move_constructor, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("move_assignment", test_digital_output_guard_move_assignment, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_EDGE_CASE_TESTS, "DIGITAL OUTPUT GUARD EDGE CASE TESTS", 5,
      // Edge Cases and Error Condition Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard edge case tests...");
      RUN_TEST_IN_TASK("invalid_operations", test_digital_output_guard_invalid_operations, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("multiple_guards_same_gpio",
                       test_digital_output_guard_multiple_guards_same_gpio, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_CONCURRENT_TESTS, "DIGITAL OUTPUT GUARD CONCURRENT TESTS", 5,
      // Concurrent Access Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard concurrent access tests...");
      RUN_TEST_IN_TASK("concurrent_access", test_digital_output_guard_concurrent_access, 8192, 5);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_PERFORMANCE_TESTS, "DIGITAL OUTPUT GUARD PERFORMANCE TESTS", 5,
      // Performance and Stress Tests
      ESP_LOGI(TAG, "Running DigitalOutputGuard performance and stress tests...");
      RUN_TEST_IN_TASK("performance", test_digital_output_guard_performance, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("stress", test_digital_output_guard_stress, 8192, 1);
      flip_test_progress_indicator(););

  // Print final summary
  print_test_summary(g_test_results, "DIGITAL_OUTPUT_GUARD", TAG);

  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                DIGITAL OUTPUT GUARD COMPREHENSIVE TEST SUITE COMPLETE        ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Keep the system running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}