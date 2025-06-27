/**
 * @file SfGpio.cpp
 * @brief Implementation of thread-safe GPIO interface wrapper.
 *
 * This file provides the implementation for the thread-safe GPIO wrapper,
 * ensuring atomic operations and thread safety for multi-threaded applications.
 */

#include "../thread_safe/SfGpio.h"

namespace HardFOC {
namespace ThreadSafe {

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

SfGpio::SfGpio(std::shared_ptr<BaseGpio> gpio_impl)
    : gpio_impl_(std::move(gpio_impl)), initialized_(false) {}

SfGpio::~SfGpio() = default;

//==============================================================================
// INITIALIZATION AND CONFIGURATION
//==============================================================================

HfGpioErr SfGpio::Initialize() noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->Initialize();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      initialized_ = true;
    }
  }

  return result;
}

HfGpioErr SfGpio::Deinitialize() noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->Deinitialize();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      initialized_ = false;
    }
  }

  return result;
}

HfGpioErr SfGpio::ConfigurePin(hf_gpio_num_t pin, const GpioPinConfig &config) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->ConfigurePin(pin, config);
  }

  return result;
}

//==============================================================================
// DIGITAL I/O OPERATIONS
//==============================================================================

HfGpioErr SfGpio::SetLevel(hf_gpio_num_t pin, bool level) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->SetLevel(pin, level);
  }

  return result;
}

HfGpioErr SfGpio::GetLevel(hf_gpio_num_t pin, bool &level) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->GetLevel(pin, level);
  }

  return result;
}

HfGpioErr SfGpio::ToggleLevel(hf_gpio_num_t pin) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->ToggleLevel(pin);
  }

  return result;
}

//==============================================================================
// INTERRUPT HANDLING
//==============================================================================

HfGpioErr SfGpio::EnableInterrupt(hf_gpio_num_t pin, hf_gpio_intr_type_t intr_type,
                                  GpioIsrCallback callback, void *user_data) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->EnableInterrupt(pin, intr_type, callback, user_data);
  }

  return result;
}

HfGpioErr SfGpio::DisableInterrupt(hf_gpio_num_t pin) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->DisableInterrupt(pin);
  }

  return result;
}

//==============================================================================
// ADVANCED FEATURES
//==============================================================================

HfGpioErr SfGpio::SetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t strength) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->SetDriveStrength(pin, strength);
  }

  return result;
}

HfGpioErr SfGpio::GetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t &strength) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->GetDriveStrength(pin, strength);
  }

  return result;
}

//==============================================================================
// UTILITY METHODS
//==============================================================================

bool SfGpio::IsInitialized() const noexcept {
  return initialized_;
}

const char *SfGpio::GetErrorString(HfGpioErr error) const noexcept {
  if (gpio_impl_) {
    return gpio_impl_->GetErrorString(error);
  }
  return "Unknown error - GPIO implementation not available";
}

bool SfGpio::IsPinValid(hf_gpio_num_t pin) const noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return false;
  }

  bool result = false;
  if (gpio_impl_) {
    result = gpio_impl_->IsPinValid(pin);
  }

  return result;
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

} // namespace ThreadSafe
} // namespace HardFOC
