/**
 * @file StmPwm.h
 * @brief STM32 PWM wrapper — wraps TIM_HandleTypeDef for BasePwm interface.
 *
 * Users configure timers + OC channels in STM32CubeMX, then pass the
 * TIM_HandleTypeDef* here.  Each StmPwm instance manages one timer with
 * up to 4 channels (CH1–CH4).
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BasePwm.h"
#include "StmTypes.h"

struct TIM_HandleTypeDef;  // Forward declaration

/**
 * @class StmPwm
 * @brief STM32 PWM implementation.
 *
 * Design:
 * - One StmPwm per TIM_HandleTypeDef (one hardware timer).
 * - Channels 0–3 map to TIM_CHANNEL_1..TIM_CHANNEL_4.
 * - Duty cycle sets the CCR register relative to ARR.
 * - Frequency changes recalculate ARR+PSC (affects all channels on the timer).
 * - Complementary / dead-time support via TIM1/TIM8 BDTR register.
 */
class StmPwm : public BasePwm {
public:

    /**
     * @brief Construct from HAL timer handle.
     * @param htim         CubeMX timer handle.
     * @param timer_clock_hz  APB timer input clock (e.g. 84 MHz).
     */
    explicit StmPwm(TIM_HandleTypeDef* htim,
                    hf_u32_t timer_clock_hz) noexcept;

    /**
     * @brief Construct from config struct.
     */
    explicit StmPwm(const hf_stm32_pwm_config_t& config) noexcept;

    ~StmPwm() noexcept override;

    StmPwm(const StmPwm&) = delete;
    StmPwm& operator=(const StmPwm&) = delete;

    // ── Pure-virtual overrides (BasePwm) ────────────────────────────────────

    // Lifecycle
    hf_pwm_err_t Initialize() noexcept override;
    hf_pwm_err_t Deinitialize() noexcept override;

    // Channel control
    hf_pwm_err_t EnableChannel(hf_channel_id_t channel_id) noexcept override;
    hf_pwm_err_t DisableChannel(hf_channel_id_t channel_id) noexcept override;
    bool IsChannelEnabled(hf_channel_id_t channel_id) const noexcept override;

    // PWM control
    hf_pwm_err_t SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept override;
    hf_pwm_err_t SetDutyCycleRaw(hf_channel_id_t channel_id, hf_u32_t raw_value) noexcept override;
    hf_pwm_err_t SetFrequency(hf_channel_id_t channel_id, hf_frequency_hz_t frequency_hz) noexcept override;
    hf_pwm_err_t SetPhaseShift(hf_channel_id_t channel_id, float phase_shift_degrees) noexcept override;

    // Advanced
    hf_pwm_err_t StartAll() noexcept override;
    hf_pwm_err_t StopAll() noexcept override;
    hf_pwm_err_t UpdateAll() noexcept override;
    hf_pwm_err_t SetComplementaryOutput(hf_channel_id_t primary_channel,
                                        hf_channel_id_t complementary_channel,
                                        hf_u32_t deadtime_ns) noexcept override;

    // Status
    float GetDutyCycle(hf_channel_id_t channel_id) const noexcept override;
    hf_frequency_hz_t GetFrequency(hf_channel_id_t channel_id) const noexcept override;

    // ── Accessors ───────────────────────────────────────────────────────────
    TIM_HandleTypeDef* GetHalHandle() const noexcept { return htim_; }

    static constexpr hf_u8_t kMaxChannels = 4;

private:
    TIM_HandleTypeDef* htim_;
    hf_u32_t           timer_clock_hz_;
    bool               channel_enabled_[kMaxChannels];
    float              duty_[kMaxChannels];        ///< Cached duty 0.0–1.0
    hf_frequency_hz_t  frequency_hz_;              ///< Current frequency (shared)

    /// Get HAL channel constant from 0-based index.
    static hf_u32_t ChannelToHal(hf_channel_id_t ch) noexcept;

    /// Apply freq by recalculating PSC + ARR.
    bool ApplyFrequency(hf_frequency_hz_t freq_hz) noexcept;

    /// Reapply all cached duty cycles after ARR change.
    void ReapplyDuties() noexcept;
};
