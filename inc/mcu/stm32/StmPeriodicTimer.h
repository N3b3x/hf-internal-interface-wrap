/**
 * @file StmPeriodicTimer.h
 * @brief STM32 Periodic Timer stub — placeholder for hardware timer integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BasePeriodicTimer.h"

/**
 * @brief STM32 Periodic Timer — stub implementation.
 */
class StmPeriodicTimer : public BasePeriodicTimer {
public:
    using CallbackFunc = void(*)(void* user_data);

    explicit StmPeriodicTimer(CallbackFunc callback = nullptr, void* user_data = nullptr) noexcept;
    ~StmPeriodicTimer() noexcept override;

    hf_timer_err_t Initialize() noexcept override;
    hf_timer_err_t Deinitialize() noexcept override;
    hf_timer_err_t Start(hf_u64_t period_us) noexcept override;
    hf_timer_err_t Stop() noexcept override;
    hf_timer_err_t SetPeriod(hf_u64_t period_us) noexcept override;
    hf_timer_err_t GetPeriod(hf_u64_t& period_us) noexcept override;
    hf_timer_err_t GetStats(hf_u64_t& callback_count, hf_u64_t& missed_callbacks,
                            hf_timer_err_t& last_error) noexcept override;
    hf_timer_err_t ResetStats() noexcept override;
    const char* GetDescription() const noexcept override;
    hf_u64_t GetMaxPeriod() const noexcept override;
    hf_u64_t GetMinPeriod() const noexcept override;
    hf_u64_t GetResolution() const noexcept override;

private:
    CallbackFunc callback_;
    void* user_data_;
};
