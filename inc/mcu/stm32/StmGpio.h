/**
 * @file StmGpio.h
 * @brief STM32 GPIO stub implementation — placeholder for future STM32 HAL integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseGpio.h"

/**
 * @brief STM32 GPIO — stub implementation.
 *
 * Override only the pure virtual methods from BaseGpio.
 * All operations return GPIO_ERR_UNSUPPORTED_OPERATION.
 */
class StmGpio : public BaseGpio {
public:
    explicit StmGpio(hf_pin_num_t pin_num,
        hf_gpio_direction_t direction = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
        hf_gpio_active_state_t active_state = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
        hf_gpio_output_mode_t output_mode = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
        hf_gpio_pull_mode_t pull_mode = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
        hf_gpio_drive_cap_t drive_capability = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM) noexcept;
    ~StmGpio() override;

    // Public pure virtuals
    bool Initialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    hf_u8_t GetMaxPins() const noexcept override;
    const char* GetDescription() const noexcept override;

protected:
    // Platform implementation overrides (pure virtual in BaseGpio)
    hf_gpio_err_t SetDirectionImpl(hf_gpio_direction_t direction) noexcept override;
    hf_gpio_err_t GetDirectionImpl(hf_gpio_direction_t& direction) const noexcept override;
    hf_gpio_err_t SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept override;
    hf_gpio_err_t GetOutputModeImpl(hf_gpio_output_mode_t& mode) const noexcept override;
    hf_gpio_err_t SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept override;
    hf_gpio_pull_mode_t GetPullModeImpl() const noexcept override;
    hf_gpio_err_t SetPinLevelImpl(hf_gpio_level_t level) noexcept override;
    hf_gpio_err_t GetPinLevelImpl(hf_gpio_level_t& level) noexcept override;
};
