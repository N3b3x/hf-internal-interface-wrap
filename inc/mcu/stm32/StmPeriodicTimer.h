/**
 * @file StmPeriodicTimer.h
 * @brief STM32 Periodic Timer — wraps TIM_HandleTypeDef for periodic callbacks.
 *
 * Users configure a hardware timer in STM32CubeMX, then pass the resulting
 * TIM_HandleTypeDef* to this wrapper. The wrapper manages start/stop/period
 * adjustment and dispatches the update interrupt callback to the registered
 * function.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BasePeriodicTimer.h"
#include "StmTypes.h"

struct TIM_HandleTypeDef;  // Forward declaration

/**
 * @brief STM32 Periodic Timer implementation.
 *
 * Design:
 * - Accepts a TIM_HandleTypeDef* already initialised by CubeMX.
 * - Uses HAL_TIM_Base_Start_IT / Stop_IT.
 * - Provides a static dispatch table so the HAL_TIM_PeriodElapsedCallback
 *   (extern "C") can route to the correct C++ instance.
 * - Period changes recalculate ARR+PSC from the timer's input clock.
 */
class StmPeriodicTimer : public BasePeriodicTimer {
public:

    /**
     * @brief Construct from CubeMX timer handle.
     * @param htim        Pointer to HAL timer handle (CubeMX‑initialised).
     * @param timer_clock_hz  APB timer clock feeding this TIM (e.g. 84 MHz).
     * @param callback    User callback (may also be set later via SetCallback).
     * @param user_data   Opaque pointer forwarded to callback.
     */
    explicit StmPeriodicTimer(TIM_HandleTypeDef* htim,
                              hf_u32_t timer_clock_hz,
                              hf_timer_callback_t callback = nullptr,
                              void* user_data = nullptr) noexcept;

    /**
     * @brief Construct from config struct.
     */
    explicit StmPeriodicTimer(const hf_stm32_timer_config_t& config,
                              hf_timer_callback_t callback = nullptr,
                              void* user_data = nullptr) noexcept;

    ~StmPeriodicTimer() noexcept override;

    // Non-copyable, non-movable
    StmPeriodicTimer(const StmPeriodicTimer&) = delete;
    StmPeriodicTimer& operator=(const StmPeriodicTimer&) = delete;

    // ── Pure‑virtual overrides ──────────────────────────────────────────────
    hf_timer_err_t Initialize() noexcept override;
    hf_timer_err_t Deinitialize() noexcept override;
    hf_timer_err_t Start(hf_u64_t period_us) noexcept override;
    hf_timer_err_t Stop() noexcept override;
    hf_timer_err_t SetPeriod(hf_u64_t period_us) noexcept override;
    hf_timer_err_t GetPeriod(hf_u64_t& period_us) noexcept override;
    hf_timer_err_t GetStats(hf_u64_t& callback_count,
                            hf_u64_t& missed_callbacks,
                            hf_timer_err_t& last_error) noexcept override;
    hf_timer_err_t ResetStats() noexcept override;
    const char* GetDescription() const noexcept override;
    hf_u64_t GetMaxPeriod() const noexcept override;
    hf_u64_t GetMinPeriod() const noexcept override;
    hf_u64_t GetResolution() const noexcept override;

    // ── Statistics / Diagnostics overrides ──────────────────────────────────
    hf_timer_err_t GetStatistics(hf_timer_statistics_t& statistics) const noexcept override;
    hf_timer_err_t GetDiagnostics(hf_timer_diagnostics_t& diagnostics) const noexcept override;

    // ── ISR Dispatch ────────────────────────────────────────────────────────
    /**
     * @brief Call from HAL_TIM_PeriodElapsedCallback to dispatch to instances.
     *
     * Usage in user code:
     * @code
     * void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
     *     StmPeriodicTimer::IsrDispatch(htim);
     * }
     * @endcode
     */
    static void IsrDispatch(TIM_HandleTypeDef* htim) noexcept;

private:
    TIM_HandleTypeDef* htim_;          ///< HAL timer handle
    hf_u32_t           timer_clock_hz_;///< Input clock to the timer
    hf_u64_t           period_us_;     ///< Current period in µs
    hf_timer_stats_t   stats_;         ///< Internal stats

    /// Recalculate and apply PSC+ARR for the requested period.
    bool ApplyPeriod(hf_u64_t period_us) noexcept;

    // ── Static dispatch table ───────────────────────────────────────────────
    static constexpr int kMaxTimerInstances = 8;
    struct DispatchEntry {
        TIM_HandleTypeDef* htim;
        StmPeriodicTimer*  instance;
    };
    static DispatchEntry s_dispatch_[kMaxTimerInstances];
    static int           s_dispatch_count_;

    void RegisterDispatch() noexcept;
    void UnregisterDispatch() noexcept;
};
