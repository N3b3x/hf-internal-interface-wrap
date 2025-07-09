/**
 * @file DigitalOutputGuard.cpp
 * @brief Implementation of the DigitalOutputGuard class for unified DigitalGpio.
 *
 * The DigitalOutputGuard class provides RAII management of DigitalGpio instances
 * when used as outputs, ensuring proper state management and cleanup. This implementation
 * guarantees safe automatic management of GPIO output states with guaranteed cleanup
 * even in exception scenarios, ensuring proper resource management and pin state control.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "DigitalOutputGuard.h"

//==============================================================================
// CONSTRUCTORS
//==============================================================================

DigitalOutputGuard::DigitalOutputGuard(BaseGpio &gpio, bool ensure_output_mode) noexcept
    : gpio_(&gpio), was_output_mode_(false), is_valid_(false),
      last_error_(hf_gpio_err_t::GPIO_SUCCESS) {

  is_valid_ = InitializeGuard(ensure_output_mode);
}

DigitalOutputGuard::DigitalOutputGuard(BaseGpio *gpio, bool ensure_output_mode) noexcept
    : gpio_(gpio), was_output_mode_(false), is_valid_(false), last_error_(hf_gpio_err_t::GPIO_SUCCESS) {

  if (gpio_ == nullptr) {
    last_error_ = hf_gpio_err_t::GPIO_ERR_NULL_POINTER;
    is_valid_ = false;
    return;
  }

  is_valid_ = InitializeGuard(ensure_output_mode);
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

DigitalOutputGuard::~DigitalOutputGuard() noexcept {
  if (gpio_ != nullptr && is_valid_) {
    // Set the GPIO to inactive state before destruction
    // Don't change the direction back - leave it as configured
    hf_gpio_err_t result = gpio_->SetInactive();
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      // In destructor, we can't throw exceptions or report errors
      // Just attempt the operation and continue with destruction
      (void)result; // Suppress unused variable warning
    }
  }
}

//==============================================================================
// PUBLIC METHODS
//==============================================================================

hf_gpio_err_t DigitalOutputGuard::SetActive() noexcept {
  if (!is_valid_ || gpio_ == nullptr) {
    return last_error_ != hf_gpio_err_t::GPIO_SUCCESS ? last_error_
                                                  : hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  }

  // Ensure the GPIO is in output mode
  if (!gpio_->IsOutput()) {
    hf_gpio_err_t result = gpio_->SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      last_error_ = result;
      return result;
    }
  }

  last_error_ = gpio_->SetActive();
  return last_error_;
}

hf_gpio_err_t DigitalOutputGuard::SetInactive() noexcept {
  if (!is_valid_ || gpio_ == nullptr) {
    return last_error_ != hf_gpio_err_t::GPIO_SUCCESS ? last_error_
                                                  : hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  }

  // Ensure the GPIO is in output mode
  if (!gpio_->IsOutput()) {
    hf_gpio_err_t result = gpio_->SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      last_error_ = result;
      return result;
    }
  }

  last_error_ = gpio_->SetInactive();
  return last_error_;
}

BaseGpio::hf_gpio_state_t DigitalOutputGuard::GetCurrentState() const noexcept {
  if (!is_valid_ || gpio_ == nullptr) {
    return hf_gpio_state_t::HF_GPIO_STATE_INACTIVE; // Safe default
  }

  return gpio_->GetCurrentState();
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool DigitalOutputGuard::InitializeGuard(bool ensure_output_mode) noexcept {
  // Check if GPIO is initialized
  if (!gpio_->EnsureInitialized()) {
    last_error_ = hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    return false;
  }

  // Record if the GPIO was already in output mode
  was_output_mode_ = gpio_->IsOutput();

  // Ensure output mode if requested
  if (ensure_output_mode && !was_output_mode_) {
    hf_gpio_err_t result = gpio_->SetDirection(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    if (result != hf_gpio_err_t::GPIO_SUCCESS) {
      last_error_ = result;
      return false;
    }
  }

  // If not in output mode and we're not ensuring it, that's an error
  if (!gpio_->IsOutput()) {
    last_error_ = hf_gpio_err_t::GPIO_ERR_DIRECTION_MISMATCH;
    return false;
  }

  // Set the GPIO to active state
  hf_gpio_err_t result = gpio_->SetActive();
  if (result != hf_gpio_err_t::GPIO_SUCCESS) {
    last_error_ = result;
    return false;
  }

  last_error_ = hf_gpio_err_t::GPIO_SUCCESS;
  return true;
}
