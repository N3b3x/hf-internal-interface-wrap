/**
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
 * License v3.0 or later.
 *
 * @file DigitalOutputGuard.cpp
 * @brief Implementation of the DigitalOutputGuard class for unified DigitalGpio.
 *
 * The DigitalOutputGuard class provides RAII management of DigitalGpio instances
 * when used as outputs, ensuring proper state management and cleanup.
 */

#include "DigitalOutputGuard.h"

//==============================================================================
// CONSTRUCTORS
//==============================================================================

DigitalOutputGuard::DigitalOutputGuard(DigitalGpio &gpio, bool ensure_output_mode) noexcept
    : gpio_(&gpio), 
      was_output_mode_(false), 
      is_valid_(false), 
      last_error_(HfGpioErr::GPIO_SUCCESS) {
    
    is_valid_ = InitializeGuard(ensure_output_mode);
}

DigitalOutputGuard::DigitalOutputGuard(DigitalGpio *gpio, bool ensure_output_mode) noexcept
    : gpio_(gpio), 
      was_output_mode_(false), 
      is_valid_(false), 
      last_error_(HfGpioErr::GPIO_SUCCESS) {
    
    if (gpio_ == nullptr) {
        last_error_ = HfGpioErr::GPIO_ERR_NULL_POINTER;
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
        HfGpioErr result = gpio_->SetState(DigitalGpio::State::Inactive);
        if (result != HfGpioErr::GPIO_SUCCESS) {
            // In destructor, we can't throw exceptions or report errors
            // Just attempt the operation and continue with destruction
            (void)result; // Suppress unused variable warning
        }
    }
}

//==============================================================================
// PUBLIC METHODS
//==============================================================================

HfGpioErr DigitalOutputGuard::SetActive() noexcept {
    if (!is_valid_ || gpio_ == nullptr) {
        return last_error_ != HfGpioErr::GPIO_SUCCESS ? last_error_ : HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }
    
    // Ensure the GPIO is in output mode
    if (!gpio_->IsOutput()) {
        HfGpioErr result = gpio_->SetDirection(DigitalGpio::Direction::Output);
        if (result != HfGpioErr::GPIO_SUCCESS) {
            last_error_ = result;
            return result;
        }
    }
    
    last_error_ = gpio_->SetState(DigitalGpio::State::Active);
    return last_error_;
}

HfGpioErr DigitalOutputGuard::SetInactive() noexcept {
    if (!is_valid_ || gpio_ == nullptr) {
        return last_error_ != HfGpioErr::GPIO_SUCCESS ? last_error_ : HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }
    
    // Ensure the GPIO is in output mode
    if (!gpio_->IsOutput()) {
        HfGpioErr result = gpio_->SetDirection(DigitalGpio::Direction::Output);
        if (result != HfGpioErr::GPIO_SUCCESS) {
            last_error_ = result;
            return result;
        }
    }
    
    last_error_ = gpio_->SetState(DigitalGpio::State::Inactive);
    return last_error_;
}

DigitalGpio::State DigitalOutputGuard::GetCurrentState() const noexcept {
    if (!is_valid_ || gpio_ == nullptr) {
        return DigitalGpio::State::Inactive; // Safe default
    }
    
    return gpio_->GetCurrentState();
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool DigitalOutputGuard::InitializeGuard(bool ensure_output_mode) noexcept {
    // Check if GPIO is initialized
    if (!gpio_->EnsureInitialized()) {
        last_error_ = HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
        return false;
    }
    
    // Record if the GPIO was already in output mode
    was_output_mode_ = gpio_->IsOutput();
    
    // Ensure output mode if requested
    if (ensure_output_mode && !was_output_mode_) {
        HfGpioErr result = gpio_->SetDirection(DigitalGpio::Direction::Output);
        if (result != HfGpioErr::GPIO_SUCCESS) {
            last_error_ = result;
            return false;
        }
    }
    
    // If not in output mode and we're not ensuring it, that's an error
    if (!gpio_->IsOutput()) {
        last_error_ = HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
        return false;
    }
    
    // Set the GPIO to active state
    HfGpioErr result = gpio_->SetState(DigitalGpio::State::Active);
    if (result != HfGpioErr::GPIO_SUCCESS) {
        last_error_ = result;
        return false;
    }
    
    last_error_ = HfGpioErr::GPIO_SUCCESS;
    return true;
}
