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
    : gpio_impl_(gpio_impl), mutex_(xSemaphoreCreateMutex()), initialized_(false) {
  if (mutex_ == nullptr) {
    // Handle mutex creation failure
  }
}

SfGpio::~SfGpio() {
  if (mutex_ != nullptr) {
    vSemaphoreDelete(mutex_);
    mutex_ = nullptr;
  }
}

//==============================================================================
// INITIALIZATION AND CONFIGURATION
//==============================================================================

HfGpioErr SfGpio::Initialize() noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->Initialize();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      initialized_ = true;
    }
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::Deinitialize() noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->Deinitialize();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      initialized_ = false;
    }
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::ConfigurePin(hf_gpio_num_t pin, const GpioPinConfig &config) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->ConfigurePin(pin, config);
  }

  GiveMutex();
  return result;
}

//==============================================================================
// DIGITAL I/O OPERATIONS
//==============================================================================

HfGpioErr SfGpio::SetLevel(hf_gpio_num_t pin, bool level) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->SetLevel(pin, level);
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::GetLevel(hf_gpio_num_t pin, bool &level) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->GetLevel(pin, level);
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::ToggleLevel(hf_gpio_num_t pin) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->ToggleLevel(pin);
  }

  GiveMutex();
  return result;
}

//==============================================================================
// INTERRUPT HANDLING
//==============================================================================

HfGpioErr SfGpio::EnableInterrupt(hf_gpio_num_t pin, hf_gpio_intr_type_t intr_type,
                                  GpioIsrCallback callback, void *user_data) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->EnableInterrupt(pin, intr_type, callback, user_data);
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::DisableInterrupt(hf_gpio_num_t pin) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->DisableInterrupt(pin);
  }

  GiveMutex();
  return result;
}

//==============================================================================
// ADVANCED FEATURES
//==============================================================================

HfGpioErr SfGpio::SetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t strength) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->SetDriveStrength(pin, strength);
  }

  GiveMutex();
  return result;
}

HfGpioErr SfGpio::GetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t &strength) noexcept {
  if (!TakeMutex()) {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }

  HfGpioErr result = HfGpioErr::GPIO_ERR_NULL_POINTER;
  if (gpio_impl_) {
    result = gpio_impl_->GetDriveStrength(pin, strength);
  }

  GiveMutex();
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
  if (!TakeMutex()) {
    return false;
  }

  bool result = false;
  if (gpio_impl_) {
    result = gpio_impl_->IsPinValid(pin);
  }

  GiveMutex();
  return result;
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

bool SfGpio::TakeMutex(uint32_t timeout_ms) const noexcept {
  if (mutex_ == nullptr) {
    return false;
  }

  TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

  return xSemaphoreTake(mutex_, timeout_ticks) == pdTRUE;
}

void SfGpio::GiveMutex() const noexcept {
  if (mutex_ != nullptr) {
    xSemaphoreGive(mutex_);
  }
}

} // namespace ThreadSafe
} // namespace HardFOC
