/**
 * @file StmPeriodicTimer.cpp
 * @brief STM32 Periodic Timer stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmPeriodicTimer.h"

static constexpr auto kNotSupported = hf_timer_err_t::TIMER_ERR_UNSUPPORTED_OPERATION;

StmPeriodicTimer::StmPeriodicTimer(CallbackFunc callback, void* user_data) noexcept
    : callback_(callback), user_data_(user_data) {}

StmPeriodicTimer::~StmPeriodicTimer() noexcept = default;

hf_timer_err_t StmPeriodicTimer::Initialize() noexcept { return kNotSupported; }
hf_timer_err_t StmPeriodicTimer::Deinitialize() noexcept { return kNotSupported; }
hf_timer_err_t StmPeriodicTimer::Start(hf_u64_t) noexcept { return kNotSupported; }
hf_timer_err_t StmPeriodicTimer::Stop() noexcept { return kNotSupported; }
hf_timer_err_t StmPeriodicTimer::SetPeriod(hf_u64_t) noexcept { return kNotSupported; }

hf_timer_err_t StmPeriodicTimer::GetPeriod(hf_u64_t& period_us) noexcept {
    period_us = 0;
    return kNotSupported;
}

hf_timer_err_t StmPeriodicTimer::GetStats(hf_u64_t& callback_count, hf_u64_t& missed_callbacks,
                                           hf_timer_err_t& last_error) noexcept {
    callback_count = 0;
    missed_callbacks = 0;
    last_error = kNotSupported;
    return kNotSupported;
}

hf_timer_err_t StmPeriodicTimer::ResetStats() noexcept { return kNotSupported; }
const char* StmPeriodicTimer::GetDescription() const noexcept { return "STM32 Periodic Timer (stub)"; }
hf_u64_t StmPeriodicTimer::GetMaxPeriod() const noexcept { return 0; }
hf_u64_t StmPeriodicTimer::GetMinPeriod() const noexcept { return 0; }
hf_u64_t StmPeriodicTimer::GetResolution() const noexcept { return 0; }
