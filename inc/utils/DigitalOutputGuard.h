/**
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * @file DigitalOutputGuard.h
 * @brief Header file for the DigitalOutputGuard class.
 *
 * The DigitalOutputGuard class ensures that a DigitalGpio instance is set to
 * output mode and active state in its constructor, and inactive in its destructor (RAII pattern).
 * This provides safe automatic management of GPIO output states.
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_

#include "../base/BaseGpio.h"

/**
 * @class DigitalOutputGuard * @brief RAII guard class for managing the state of a BaseGpio instance
 * as output.
 *
 * The DigitalOutputGuard ensures that a BaseGpio instance is configured as output
 * and set active in its constructor, and set inactive in its destructor. This ensures
 * proper resource management and consistent behavior using RAII principles.
 *
 * Features:
 * - Automatic output mode configuration
 * - Safe state management with error handling
 * - RAII pattern for exception safety
 * - Supports both reference and pointer interfaces
 */
class DigitalOutputGuard {
public:
  /**   * @brief Constructor with BaseGpio reference.
   * @param gpio Reference to the BaseGpio instance to manage
   * @param ensure_output_mode If true, automatically switch to output mode (default: true)
   * @details Configures the GPIO as output (if needed) and sets it to active state.
   *          If the GPIO is already in output mode, it just sets the active state.
   */
  explicit DigitalOutputGuard(BaseGpio &gpio, bool ensure_output_mode = true) noexcept;

  /**   * @brief Constructor with BaseGpio pointer.
   * @param gpio Pointer to the BaseGpio instance to manage (must not be null)
   * @param ensure_output_mode If true, automatically switch to output mode (default: true)
   * @details Configures the GPIO as output (if needed) and sets it to active state.
   *          If the GPIO is already in output mode, it just sets the active state.
   */
  explicit DigitalOutputGuard(BaseGpio *gpio, bool ensure_output_mode = true) noexcept;

  /**
   * @brief Destructor.
   * @details Sets the associated BaseGpio instance to inactive state.
   *          Does not change the pin direction to preserve configuration.
   */
  ~DigitalOutputGuard() noexcept;

  // Disable copy operations for safety
  DigitalOutputGuard(const DigitalOutputGuard &) = delete;
  DigitalOutputGuard &operator=(const DigitalOutputGuard &) = delete;

  // Allow move operations
  DigitalOutputGuard(DigitalOutputGuard &&) noexcept = default;
  DigitalOutputGuard &operator=(DigitalOutputGuard &&) noexcept = default;

  /**
   * @brief Check if the guard was successfully initialized.
   * @return true if the GPIO is properly configured and managed, false otherwise
   * @details Returns false if there were errors during construction or if the GPIO
   *          pointer is invalid.
   */
  [[nodiscard]] bool IsValid() const noexcept {
    return is_valid_;
  }

  /**
   * @brief Get the last error that occurred during guard operations.
   * @return HfGpioErr error code from the last operation
   */
  [[nodiscard]] HfGpioErr GetLastError() const noexcept {
    return last_error_;
  }

  /**
   * @brief Manually set the GPIO to active state.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Allows manual control while the guard is active. The destructor
   *          will still set the pin inactive when the guard goes out of scope.
   */
  HfGpioErr SetActive() noexcept;

  /**
   * @brief Manually set the GPIO to inactive state.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Allows manual control while the guard is active. The destructor
   *          will still attempt to set the pin inactive when the guard goes out of scope.
   */
  HfGpioErr SetInactive() noexcept;

  /**
   * @brief Get the current state of the managed GPIO.
   * @return Current BaseGpio::State (Active or Inactive)
   */
  [[nodiscard]] BaseGpio::State GetCurrentState() const noexcept;

private:
  BaseGpio *gpio_;       ///< Pointer to the managed BaseGpio instance
  bool was_output_mode_; ///< Whether the GPIO was already in output mode
  bool is_valid_;        ///< Whether the guard is in a valid state
  HfGpioErr last_error_; ///< Last error code from operations

  /**
   * @brief Internal helper to initialize the guard state.
   * @param ensure_output_mode Whether to ensure output mode
   * @return true if initialization successful, false otherwise
   */
  bool InitializeGuard(bool ensure_output_mode) noexcept;
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_ */
