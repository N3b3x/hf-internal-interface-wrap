/**
 * @file DigitalOutputGuardValidationExample.cpp
 * @brief Validation example for DigitalOutputGuard with unified DigitalGpio interface
 *
 * This example demonstrates that the DigitalOutputGuard properly works with
 * the new unified DigitalGpio interface, including:
 * - Automatic output mode switching
 * - Proper RAII behavior
 * - Error handling
 * - State management
 */

#include "DigitalOutputGuard.h"
#include "McuDigitalGpio.h"
#include <iostream>

/**
 * @brief Example demonstrating basic DigitalOutputGuard usage
 * @details Shows how the guard automatically manages GPIO output state
 */
void BasicDigitalOutputGuardExample() {
  std::cout << "=== Basic DigitalOutputGuard Example ===" << std::endl;

  // Create a GPIO initially configured as input
  McuDigitalGpio gpio(GPIO_NUM_2, DigitalGpio::Direction::Input);

  std::cout << "Initial GPIO direction: " << DigitalGpio::ToString(gpio.GetDirection())
            << std::endl;
  std::cout << "Initial GPIO state: " << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;

  {
    // Create guard - this should automatically switch to output mode and set active
    DigitalOutputGuard guard(gpio);

    if (guard.IsValid()) {
      std::cout << "Guard created successfully" << std::endl;
      std::cout << "GPIO direction after guard creation: "
                << DigitalGpio::ToString(gpio.GetDirection()) << std::endl;
      std::cout << "GPIO state after guard creation: "
                << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;

      // Manually control the GPIO while guard is active
      std::cout << "Manually setting GPIO to inactive..." << std::endl;
      HfGpioErr result = guard.SetInactive();
      std::cout << "SetInactive result: " << HfGpioErrToString(result) << std::endl;
      std::cout << "GPIO state after manual SetInactive: "
                << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;

      std::cout << "Manually setting GPIO to active..." << std::endl;
      result = guard.SetActive();
      std::cout << "SetActive result: " << HfGpioErrToString(result) << std::endl;
      std::cout << "GPIO state after manual SetActive: "
                << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;
    } else {
      std::cout << "Guard creation failed with error: " << HfGpioErrToString(guard.GetLastError())
                << std::endl;
    }

    std::cout << "About to destroy guard (GPIO should be set to inactive)..." << std::endl;
    // Guard goes out of scope here - destructor sets GPIO to inactive
  }

  std::cout << "GPIO state after guard destruction: "
            << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;
  std::cout << "GPIO direction after guard destruction: "
            << DigitalGpio::ToString(gpio.GetDirection()) << std::endl;
  std::cout << std::endl;
}

/**
 * @brief Example demonstrating guard with pre-configured output GPIO
 * @details Shows how the guard behaves when GPIO is already in output mode
 */
void PreConfiguredOutputExample() {
  std::cout << "=== Pre-configured Output GPIO Example ===" << std::endl;

  // Create a GPIO initially configured as output
  McuDigitalGpio gpio(GPIO_NUM_4, DigitalGpio::Direction::Output);

  // Manually set the GPIO to inactive before creating guard
  gpio.SetState(DigitalGpio::State::Inactive);

  std::cout << "Initial GPIO direction: " << DigitalGpio::ToString(gpio.GetDirection())
            << std::endl;
  std::cout << "Initial GPIO state: " << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;

  {
    // Create guard with ensure_output_mode = false since it's already output
    DigitalOutputGuard guard(gpio, false);

    if (guard.IsValid()) {
      std::cout << "Guard created successfully (GPIO already in output mode)" << std::endl;
      std::cout << "GPIO state after guard creation: "
                << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;
    } else {
      std::cout << "Guard creation failed with error: " << HfGpioErrToString(guard.GetLastError())
                << std::endl;
    }

    // Guard destructor will set GPIO to inactive
  }

  std::cout << "GPIO state after guard destruction: "
            << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;
  std::cout << std::endl;
}

/**
 * @brief Example demonstrating error handling with invalid GPIO
 * @details Shows how the guard handles error conditions
 */
void ErrorHandlingExample() {
  std::cout << "=== Error Handling Example ===" << std::endl;

  // Create guard with null pointer
  DigitalOutputGuard guard(nullptr);

  if (!guard.IsValid()) {
    std::cout << "Guard correctly rejected null pointer with error: "
              << HfGpioErrToString(guard.GetLastError()) << std::endl;
  }

  // Try to use invalid guard
  HfGpioErr result = guard.SetActive();
  std::cout << "SetActive on invalid guard result: " << HfGpioErrToString(result) << std::endl;

  std::cout << std::endl;
}

/**
 * @brief Example demonstrating active-low GPIO with guard
 * @details Shows how the guard works with different polarity configurations
 */
void ActiveLowExample() {
  std::cout << "=== Active-Low GPIO Example ===" << std::endl;

  // Create an active-low GPIO
  McuDigitalGpio gpio(GPIO_NUM_5, DigitalGpio::Direction::Input, DigitalGpio::ActiveState::Low);

  std::cout << "Initial GPIO polarity: " << DigitalGpio::ToString(gpio.GetActiveState())
            << std::endl;
  std::cout << "Initial GPIO direction: " << DigitalGpio::ToString(gpio.GetDirection())
            << std::endl;

  {
    DigitalOutputGuard guard(gpio);

    if (guard.IsValid()) {
      std::cout << "Guard created successfully for active-low GPIO" << std::endl;
      std::cout << "GPIO state after guard creation: "
                << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;

      // The guard should work the same way regardless of polarity
      // The logical "active" state is handled by the GPIO implementation
    } else {
      std::cout << "Guard creation failed with error: " << HfGpioErrToString(guard.GetLastError())
                << std::endl;
    }
  }

  std::cout << "GPIO state after guard destruction: "
            << DigitalGpio::ToString(gpio.GetCurrentState()) << std::endl;
  std::cout << std::endl;
}

/**
 * @brief Main function demonstrating all DigitalOutputGuard examples
 */
extern "C" void RunDigitalOutputGuardValidationExamples() {
  BasicDigitalOutputGuardExample();
  PreConfiguredOutputExample();
  ErrorHandlingExample();
  ActiveLowExample();
}

#ifdef RUN_DOG_VALIDATION_MAIN
int main() {
  std::cout << "DigitalOutputGuard Validation Examples" << std::endl;
  std::cout << "======================================" << std::endl;
  std::cout << std::endl;

  try {
    RunDigitalOutputGuardValidationExamples();
    std::cout << "All examples completed successfully!" << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
#endif
