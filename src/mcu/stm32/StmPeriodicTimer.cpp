/**
 * @file StmPeriodicTimer.cpp
 * @brief STM32 Periodic Timer implementation — HAL TIM base with ISR dispatch.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmPeriodicTimer.h"
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_TIM_Base_Start_IT(TIM_HandleTypeDef* htim);
extern uint32_t HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef* htim);
extern uint32_t HAL_GetTick(void);
}

// TIM register offsets — used for direct ARR/PSC manipulation.
// These are standard across all STM32 families.
namespace {
    // Register offsets from TIM_TypeDef base (Instance pointer)
    // CR1=0x00, CR2=0x04, SMCR=0x08, DIER=0x0C, SR=0x10, EGR=0x14
    // PSC offset = 0x28, ARR offset = 0x2C
    constexpr uint32_t kTimPscOffset = 0x28;
    constexpr uint32_t kTimArrOffset = 0x2C;
    constexpr uint32_t kTimEgrOffset = 0x14;
    constexpr uint32_t kTimEgrUgBit  = 0x01;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATIC DATA
// ═══════════════════════════════════════════════════════════════════════════════

StmPeriodicTimer::DispatchEntry StmPeriodicTimer::s_dispatch_[kMaxTimerInstances]{};
int StmPeriodicTimer::s_dispatch_count_ = 0;

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTORS / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmPeriodicTimer::StmPeriodicTimer(TIM_HandleTypeDef* htim,
                                   hf_u32_t timer_clock_hz,
                                   hf_timer_callback_t callback,
                                   void* user_data) noexcept
    : BasePeriodicTimer(callback, user_data)
    , htim_(htim)
    , timer_clock_hz_(timer_clock_hz)
    , period_us_(0)
    , stats_{}
{
}

StmPeriodicTimer::StmPeriodicTimer(const hf_stm32_timer_config_t& config,
                                   hf_timer_callback_t callback,
                                   void* user_data) noexcept
    : BasePeriodicTimer(callback, user_data)
    , htim_(config.htim)
    , timer_clock_hz_(config.timer_clock_hz)
    , period_us_(0)
    , stats_{}
{
}

StmPeriodicTimer::~StmPeriodicTimer() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

hf_timer_err_t StmPeriodicTimer::Initialize() noexcept {
    if (initialized_) return hf_timer_err_t::TIMER_ERR_ALREADY_INITIALIZED;
    if (!htim_) return hf_timer_err_t::TIMER_ERR_INVALID_PARAMETER;
    if (timer_clock_hz_ == 0) return hf_timer_err_t::TIMER_ERR_INVALID_PARAMETER;

    stats_ = {};
    RegisterDispatch();
    SetInitialized(true);
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::Deinitialize() noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_SUCCESS;

    if (running_) Stop();
    UnregisterDispatch();
    SetInitialized(false);
    return hf_timer_err_t::TIMER_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// START / STOP / PERIOD
// ═══════════════════════════════════════════════════════════════════════════════

hf_timer_err_t StmPeriodicTimer::Start(hf_u64_t period_us) noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    if (running_) return hf_timer_err_t::TIMER_ERR_ALREADY_RUNNING;
    if (!HasValidCallback()) return hf_timer_err_t::TIMER_ERR_NULL_POINTER;
    if (period_us == 0) return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;

    if (!ApplyPeriod(period_us)) {
        return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;
    }

    uint32_t status = HAL_TIM_Base_Start_IT(htim_);
    if (!hf::stm32::IsHalOk(status)) {
        stats_.last_error = hf_timer_err_t::TIMER_ERR_HARDWARE_FAULT;
        return hf_timer_err_t::TIMER_ERR_HARDWARE_FAULT;
    }

    period_us_ = period_us;
    SetRunning(true);
    stats_.start_count++;
    stats_.last_start_us = static_cast<hf_u64_t>(HAL_GetTick()) * 1000ULL;
    statistics_.totalStarts++;
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::Stop() noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    if (!running_) return hf_timer_err_t::TIMER_ERR_NOT_RUNNING;

    uint32_t status = HAL_TIM_Base_Stop_IT(htim_);
    if (!hf::stm32::IsHalOk(status)) {
        stats_.last_error = hf_timer_err_t::TIMER_ERR_HARDWARE_FAULT;
        return hf_timer_err_t::TIMER_ERR_HARDWARE_FAULT;
    }

    SetRunning(false);
    stats_.stop_count++;
    statistics_.totalStops++;
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::SetPeriod(hf_u64_t period_us) noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    if (period_us == 0) return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;

    bool was_running = running_;
    if (was_running) {
        HAL_TIM_Base_Stop_IT(htim_);
    }

    if (!ApplyPeriod(period_us)) {
        if (was_running) HAL_TIM_Base_Start_IT(htim_);
        return hf_timer_err_t::TIMER_ERR_INVALID_PERIOD;
    }

    period_us_ = period_us;

    if (was_running) {
        HAL_TIM_Base_Start_IT(htim_);
    }

    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::GetPeriod(hf_u64_t& period_us) noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    period_us = period_us_;
    return hf_timer_err_t::TIMER_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATISTICS
// ═══════════════════════════════════════════════════════════════════════════════

hf_timer_err_t StmPeriodicTimer::GetStats(hf_u64_t& callback_count,
                                          hf_u64_t& missed_callbacks,
                                          hf_timer_err_t& last_error) noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    callback_count = stats_.callback_count;
    missed_callbacks = stats_.missed_callbacks;
    last_error = stats_.last_error;
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::ResetStats() noexcept {
    if (!initialized_) return hf_timer_err_t::TIMER_ERR_NOT_INITIALIZED;
    stats_ = {};
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::GetStatistics(hf_timer_statistics_t& statistics) const noexcept {
    statistics = statistics_;
    return hf_timer_err_t::TIMER_SUCCESS;
}

hf_timer_err_t StmPeriodicTimer::GetDiagnostics(hf_timer_diagnostics_t& diagnostics) const noexcept {
    diagnostics = diagnostics_;
    diagnostics.timerInitialized = initialized_;
    diagnostics.timerRunning = running_;
    diagnostics.currentPeriodUs = period_us_;
    diagnostics.timerResolutionUs = GetResolution();
    return hf_timer_err_t::TIMER_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// DESCRIPTION & CAPABILITY QUERIES
// ═══════════════════════════════════════════════════════════════════════════════

const char* StmPeriodicTimer::GetDescription() const noexcept {
    return "STM32 HAL Periodic Timer";
}

hf_u64_t StmPeriodicTimer::GetMaxPeriod() const noexcept {
    /* With 16-bit timer, max ARR=65535, max PSC=65535:
       period_max = (65536 * 65536) / timer_clock_hz  (in µs). */
    if (timer_clock_hz_ == 0) return 0;
    return (65536ULL * 65536ULL * 1000000ULL) / timer_clock_hz_;
}

hf_u64_t StmPeriodicTimer::GetMinPeriod() const noexcept {
    /* PSC=0, ARR=1 → period = 2 / timer_clock_hz  (in µs). */
    if (timer_clock_hz_ == 0) return 0;
    return (2ULL * 1000000ULL) / timer_clock_hz_;
}

hf_u64_t StmPeriodicTimer::GetResolution() const noexcept {
    /* Best case: PSC=0 → resolution = 1/timer_clock_hz in µs. */
    if (timer_clock_hz_ == 0) return 0;
    hf_u64_t res = 1000000ULL / timer_clock_hz_;
    return (res > 0) ? res : 1;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE: APPLY PERIOD
// ═══════════════════════════════════════════════════════════════════════════════

bool StmPeriodicTimer::ApplyPeriod(hf_u64_t period_us) noexcept {
    if (period_us == 0 || timer_clock_hz_ == 0 || !htim_) return false;

    /*
     * We need: (PSC + 1) * (ARR + 1) = timer_clock_hz * period_us / 1_000_000
     * Strategy: pick smallest PSC that keeps ARR ≤ 65535.
     */
    uint64_t total_ticks = (static_cast<uint64_t>(timer_clock_hz_) * period_us) / 1000000ULL;
    if (total_ticks == 0) total_ticks = 1;

    uint32_t psc = 0;
    uint32_t arr = 0;

    if (total_ticks <= 65536ULL) {
        psc = 0;
        arr = static_cast<uint32_t>(total_ticks - 1);
    } else {
        // Find smallest prescaler
        psc = static_cast<uint32_t>((total_ticks - 1) / 65536ULL);
        if (psc > 65535) return false;  // Period too long
        arr = static_cast<uint32_t>(total_ticks / (psc + 1)) - 1;
        if (arr > 65535) {
            psc++;
            arr = static_cast<uint32_t>(total_ticks / (psc + 1)) - 1;
        }
        if (arr > 65535 || psc > 65535) return false;
    }

    // Write directly to Instance registers via HAL handle
    // htim_->Instance is a TIM_TypeDef* — offsets are standard
    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_)  // Instance is the first member
    );

    base[kTimPscOffset / 4] = psc;
    base[kTimArrOffset / 4] = arr;
    // Generate update event to latch new PSC/ARR immediately
    base[kTimEgrOffset / 4] = kTimEgrUgBit;

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATIC ISR DISPATCH
// ═══════════════════════════════════════════════════════════════════════════════

void StmPeriodicTimer::IsrDispatch(TIM_HandleTypeDef* htim) noexcept {
    for (int i = 0; i < s_dispatch_count_; ++i) {
        if (s_dispatch_[i].htim == htim && s_dispatch_[i].instance) {
            auto* self = s_dispatch_[i].instance;
            self->stats_.callback_count++;
            self->statistics_.callbackExecutions++;
            self->ExecuteCallback();
            return;
        }
    }
}

void StmPeriodicTimer::RegisterDispatch() noexcept {
    if (s_dispatch_count_ >= kMaxTimerInstances) return;
    s_dispatch_[s_dispatch_count_].htim = htim_;
    s_dispatch_[s_dispatch_count_].instance = this;
    s_dispatch_count_++;
}

void StmPeriodicTimer::UnregisterDispatch() noexcept {
    for (int i = 0; i < s_dispatch_count_; ++i) {
        if (s_dispatch_[i].instance == this) {
            // Shift remaining entries
            for (int j = i; j < s_dispatch_count_ - 1; ++j) {
                s_dispatch_[j] = s_dispatch_[j + 1];
            }
            s_dispatch_count_--;
            return;
        }
    }
}
