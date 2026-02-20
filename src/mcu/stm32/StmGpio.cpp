/**
 * @file StmGpio.cpp
 * @brief STM32 GPIO stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmGpio.h"

StmGpio::StmGpio(hf_pin_num_t pin_num,
    hf_gpio_direction_t direction,
    hf_gpio_active_state_t active_state,
    hf_gpio_output_mode_t output_mode,
    hf_gpio_pull_mode_t pull_mode,
    hf_gpio_drive_cap_t drive_capability) noexcept
    : BaseGpio(pin_num, direction, active_state, output_mode, pull_mode) {
    (void)drive_capability;
}

StmGpio::~StmGpio() = default;

bool StmGpio::Initialize() noexcept { return false; }
bool StmGpio::IsPinAvailable() const noexcept { return false; }
hf_u8_t StmGpio::GetMaxPins() const noexcept { return 0; }
const char* StmGpio::GetDescription() const noexcept { return "STM32 GPIO (stub)"; }

hf_gpio_err_t StmGpio::SetDirectionImpl(hf_gpio_direction_t) noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_err_t StmGpio::GetDirectionImpl(hf_gpio_direction_t&) const noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_err_t StmGpio::SetOutputModeImpl(hf_gpio_output_mode_t) noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_err_t StmGpio::GetOutputModeImpl(hf_gpio_output_mode_t&) const noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_err_t StmGpio::SetPullModeImpl(hf_gpio_pull_mode_t) noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_pull_mode_t StmGpio::GetPullModeImpl() const noexcept {
    return hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING;
}
hf_gpio_err_t StmGpio::SetPinLevelImpl(hf_gpio_level_t) noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
hf_gpio_err_t StmGpio::GetPinLevelImpl(hf_gpio_level_t&) noexcept {
    return hf_gpio_err_t::GPIO_ERR_UNSUPPORTED_OPERATION;
}
