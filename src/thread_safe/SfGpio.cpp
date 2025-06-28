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

HfGpioErr SfGpio::setPinMode(hf_gpio_num_t pin, hf_gpio_mode_t mode) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  if (!gpio_impl_) {
    return HfGpioErr::GPIO_ERR_NULL_POINTER;
  }
  BaseGpio::Direction dir = BaseGpio::Direction::Input;
  BaseGpio::OutputMode out = BaseGpio::OutputMode::PushPull;
  switch (mode) {
  case hf_gpio_mode_t::HF_GPIO_MODE_OUTPUT:
    dir = BaseGpio::Direction::Output;
    out = BaseGpio::OutputMode::PushPull;
    break;
  case hf_gpio_mode_t::HF_GPIO_MODE_OUTPUT_OD:
    dir = BaseGpio::Direction::Output;
    out = BaseGpio::OutputMode::OpenDrain;
    break;
  case hf_gpio_mode_t::HF_GPIO_MODE_INPUT:
  default:
    dir = BaseGpio::Direction::Input;
    break;
  }

  HfGpioErr err = gpio_impl_->SetDirection(dir);
  if (err != HfGpioErr::GPIO_SUCCESS) {
    return err;
  }
  return gpio_impl_->SetOutputMode(out);
}

HfGpioErr SfGpio::setPinPull(hf_gpio_num_t pin, hf_gpio_pull_t pull) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  if (!gpio_impl_) {
    return HfGpioErr::GPIO_ERR_NULL_POINTER;
  }
  BaseGpio::PullMode pm = BaseGpio::PullMode::Floating;
  switch (pull) {
  case hf_gpio_pull_t::HF_GPIO_PULL_UP:
    pm = BaseGpio::PullMode::PullUp;
    break;
  case hf_gpio_pull_t::HF_GPIO_PULL_DOWN:
    pm = BaseGpio::PullMode::PullDown;
    break;
  default:
    pm = BaseGpio::PullMode::Floating;
    break;
  }
  return gpio_impl_->SetPullMode(pm);
}

HfGpioErr SfGpio::setPinDriveCapability(hf_gpio_num_t, hf_gpio_drive_cap_t) noexcept {
  // BaseGpio does not expose drive capability configuration
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
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

HfGpioErr SfGpio::writeMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t level_mask) noexcept {
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  if (!gpio_impl_) {
    return HfGpioErr::GPIO_ERR_NULL_POINTER;
  }
  for (uint8_t pin = 0; pin < 32; ++pin) {
    if (pin_mask & (1u << pin)) {
      bool level = (level_mask & (1u << pin)) != 0;
      HfGpioErr err = gpio_impl_->SetLevel(static_cast<hf_gpio_num_t>(pin), level);
      if (err != HfGpioErr::GPIO_SUCCESS) {
        return err;
      }
    }
  }
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr SfGpio::readMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t *level_mask) noexcept {
  if (!level_mask) {
    return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  if (!gpio_impl_) {
    return HfGpioErr::GPIO_ERR_NULL_POINTER;
  }
  hf_gpio_mask_t result_mask = 0;
  for (uint8_t pin = 0; pin < 32; ++pin) {
    if (pin_mask & (1u << pin)) {
      bool level = false;
      HfGpioErr err = gpio_impl_->GetLevel(static_cast<hf_gpio_num_t>(pin), level);
      if (err != HfGpioErr::GPIO_SUCCESS) {
        return err;
      }
      if (level) {
        result_mask |= (1u << pin);
      }
    }
  }
  *level_mask = result_mask;
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr SfGpio::GetPinMode(hf_gpio_num_t, hf_gpio_mode_t *mode) noexcept {
  if (!mode) {
    return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  if (!gpio_impl_) {
    return HfGpioErr::GPIO_ERR_NULL_POINTER;
  }
  BaseGpio::Direction dir = gpio_impl_->GetDirection();
  BaseGpio::OutputMode out = gpio_impl_->GetOutputMode();
  if (dir == BaseGpio::Direction::Input) {
    *mode = hf_gpio_mode_t::HF_GPIO_MODE_INPUT;
  } else if (out == BaseGpio::OutputMode::OpenDrain) {
    *mode = hf_gpio_mode_t::HF_GPIO_MODE_OUTPUT_OD;
  } else {
    *mode = hf_gpio_mode_t::HF_GPIO_MODE_OUTPUT;
  }
  return HfGpioErr::GPIO_SUCCESS;
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
