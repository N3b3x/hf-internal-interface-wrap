/**
 * @file StmPwm.cpp
 * @brief STM32 PWM implementation — TIM OC with frequency/duty/deadtime control.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmPwm.h"
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_TIM_PWM_Start(TIM_HandleTypeDef* htim, uint32_t Channel);
extern uint32_t HAL_TIM_PWM_Stop(TIM_HandleTypeDef* htim, uint32_t Channel);
extern uint32_t HAL_TIMEx_PWMN_Start(TIM_HandleTypeDef* htim, uint32_t Channel);
extern uint32_t HAL_TIMEx_PWMN_Stop(TIM_HandleTypeDef* htim, uint32_t Channel);
}

// HAL TIM channel constants (TIM_CHANNEL_x)
namespace {
    constexpr uint32_t kTimChannel1 = 0x00000000U;
    constexpr uint32_t kTimChannel2 = 0x00000004U;
    constexpr uint32_t kTimChannel3 = 0x00000008U;
    constexpr uint32_t kTimChannel4 = 0x0000000CU;

    // TIM register offsets (standard across STM32 families)
    constexpr uint32_t kTimPscOffset  = 0x28;
    constexpr uint32_t kTimArrOffset  = 0x2C;
    constexpr uint32_t kTimCcr1Offset = 0x34;
    constexpr uint32_t kTimEgrOffset  = 0x14;
    constexpr uint32_t kTimBdtrOffset = 0x44;
    constexpr uint32_t kTimEgrUgBit   = 0x01;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTORS / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmPwm::StmPwm(TIM_HandleTypeDef* htim, hf_u32_t timer_clock_hz) noexcept
    : BasePwm()
    , htim_(htim)
    , timer_clock_hz_(timer_clock_hz)
    , frequency_hz_(0)
{
    std::memset(channel_enabled_, 0, sizeof(channel_enabled_));
    std::memset(duty_, 0, sizeof(duty_));
}

StmPwm::StmPwm(const hf_stm32_pwm_config_t& config) noexcept
    : BasePwm()
    , htim_(config.htim)
    , timer_clock_hz_(config.timer_clock_hz)
    , frequency_hz_(config.initial_frequency_hz)
{
    std::memset(channel_enabled_, 0, sizeof(channel_enabled_));
    std::memset(duty_, 0, sizeof(duty_));
}

StmPwm::~StmPwm() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

hf_pwm_err_t StmPwm::Initialize() noexcept {
    if (initialized_) return hf_pwm_err_t::PWM_ERR_ALREADY_INITIALIZED;
    if (!htim_ || timer_clock_hz_ == 0) return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;

    // If a default frequency was given, apply it
    if (frequency_hz_ > 0) {
        if (!ApplyFrequency(frequency_hz_)) {
            return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
        }
    }

    initialized_ = true;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::Deinitialize() noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_SUCCESS;
    StopAll();
    initialized_ = false;
    return hf_pwm_err_t::PWM_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CHANNEL CONTROL
// ═══════════════════════════════════════════════════════════════════════════════

hf_pwm_err_t StmPwm::EnableChannel(hf_channel_id_t ch) noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (ch >= kMaxChannels) return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;

    uint32_t hal_ch = ChannelToHal(ch);
    uint32_t status = HAL_TIM_PWM_Start(htim_, hal_ch);
    if (!hf::stm32::IsHalOk(status)) return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;

    channel_enabled_[ch] = true;
    statistics_.channel_enables_count++;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::DisableChannel(hf_channel_id_t ch) noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (ch >= kMaxChannels) return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;

    uint32_t hal_ch = ChannelToHal(ch);
    uint32_t status = HAL_TIM_PWM_Stop(htim_, hal_ch);
    if (!hf::stm32::IsHalOk(status)) return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;

    channel_enabled_[ch] = false;
    statistics_.channel_disables_count++;
    return hf_pwm_err_t::PWM_SUCCESS;
}

bool StmPwm::IsChannelEnabled(hf_channel_id_t ch) const noexcept {
    return (ch < kMaxChannels) ? channel_enabled_[ch] : false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PWM CONTROL
// ═══════════════════════════════════════════════════════════════════════════════

hf_pwm_err_t StmPwm::SetDutyCycle(hf_channel_id_t ch, float duty_cycle) noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (ch >= kMaxChannels) return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
    if (!IsValidDutyCycle(duty_cycle)) duty_cycle = ClampDutyCycle(duty_cycle);

    // Get current ARR
    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_));
    uint32_t arr = base[kTimArrOffset / 4];

    uint32_t ccr_val = static_cast<uint32_t>(duty_cycle * static_cast<float>(arr + 1));

    // Write to the correct CCR register (CCR1 + ch * 4 bytes offset)
    base[(kTimCcr1Offset / 4) + ch] = ccr_val;

    duty_[ch] = duty_cycle;
    statistics_.duty_updates_count++;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::SetDutyCycleRaw(hf_channel_id_t ch, hf_u32_t raw_value) noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (ch >= kMaxChannels) return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;

    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_));
    uint32_t arr = base[kTimArrOffset / 4];

    if (raw_value > arr + 1) raw_value = arr + 1;

    base[(kTimCcr1Offset / 4) + ch] = raw_value;
    duty_[ch] = static_cast<float>(raw_value) / static_cast<float>(arr + 1);
    statistics_.duty_updates_count++;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::SetFrequency(hf_channel_id_t ch, hf_frequency_hz_t freq_hz) noexcept {
    (void)ch;  // Frequency is shared across all channels on a timer
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (freq_hz == 0) return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;

    if (!ApplyFrequency(freq_hz)) {
        return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
    }

    frequency_hz_ = freq_hz;
    ReapplyDuties();
    statistics_.frequency_changes_count++;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::SetPhaseShift(hf_channel_id_t ch, float phase_degrees) noexcept {
    (void)ch;
    (void)phase_degrees;
    // Phase shift requires multi-timer synchronization — not trivially available
    // on general TIM peripherals.
    return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
}

// ═══════════════════════════════════════════════════════════════════════════════
// ADVANCED
// ═══════════════════════════════════════════════════════════════════════════════

hf_pwm_err_t StmPwm::StartAll() noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    for (hf_u8_t ch = 0; ch < kMaxChannels; ++ch) {
        EnableChannel(ch);
    }
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::StopAll() noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_SUCCESS;
    for (hf_u8_t ch = 0; ch < kMaxChannels; ++ch) {
        if (channel_enabled_[ch]) DisableChannel(ch);
    }
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::UpdateAll() noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    // Generate an update event so all shadow registers latch
    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_));
    base[kTimEgrOffset / 4] = kTimEgrUgBit;
    return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t StmPwm::SetComplementaryOutput(hf_channel_id_t primary,
                                             hf_channel_id_t comp,
                                             hf_u32_t deadtime_ns) noexcept {
    if (!initialized_) return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
    if (primary >= kMaxChannels) return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;

    (void)comp;  // Complementary channel is the Nx output of the same TIM channel

    /*
     * Dead-time is set via the BDTR register (DTG field, bits 0–7).
     * The exact mapping from ns to DTG depends on timer clock.
     * Simplified: DTG ≈ deadtime_ns * (timer_clock_hz / 1e9).
     * This approximation is for the first DTG range (0–127 → 0 to 127 * tDTS).
     */
    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_));

    uint32_t dtg = static_cast<uint32_t>(
        (static_cast<uint64_t>(deadtime_ns) * timer_clock_hz_) / 1000000000ULL);
    if (dtg > 255) dtg = 255;

    uint32_t bdtr = base[kTimBdtrOffset / 4];
    bdtr &= ~0xFFU;       // Clear DTG bits
    bdtr |= (dtg & 0xFFU);
    bdtr |= (1U << 15);   // MOE — Main Output Enable
    base[kTimBdtrOffset / 4] = bdtr;

    // Start complementary output
    HAL_TIMEx_PWMN_Start(htim_, ChannelToHal(primary));

    return hf_pwm_err_t::PWM_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATUS
// ═══════════════════════════════════════════════════════════════════════════════

float StmPwm::GetDutyCycle(hf_channel_id_t ch) const noexcept {
    return (ch < kMaxChannels) ? duty_[ch] : 0.0f;
}

hf_frequency_hz_t StmPwm::GetFrequency(hf_channel_id_t ch) const noexcept {
    (void)ch;
    return frequency_hz_;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

hf_u32_t StmPwm::ChannelToHal(hf_channel_id_t ch) noexcept {
    switch (ch) {
        case 0: return kTimChannel1;
        case 1: return kTimChannel2;
        case 2: return kTimChannel3;
        case 3: return kTimChannel4;
        default: return kTimChannel1;
    }
}

bool StmPwm::ApplyFrequency(hf_frequency_hz_t freq_hz) noexcept {
    if (freq_hz == 0 || timer_clock_hz_ == 0 || !htim_) return false;

    // total_ticks = timer_clock / frequency
    uint64_t total_ticks = static_cast<uint64_t>(timer_clock_hz_) / freq_hz;
    if (total_ticks == 0) return false;

    uint32_t psc = 0;
    uint32_t arr = 0;

    if (total_ticks <= 65536ULL) {
        psc = 0;
        arr = static_cast<uint32_t>(total_ticks - 1);
    } else {
        psc = static_cast<uint32_t>((total_ticks - 1) / 65536ULL);
        if (psc > 65535) return false;
        arr = static_cast<uint32_t>(total_ticks / (psc + 1)) - 1;
        if (arr > 65535) {
            psc++;
            arr = static_cast<uint32_t>(total_ticks / (psc + 1)) - 1;
        }
        if (arr > 65535 || psc > 65535) return false;
    }

    volatile uint32_t* base = reinterpret_cast<volatile uint32_t*>(
        *reinterpret_cast<uint32_t**>(htim_));

    base[kTimPscOffset / 4] = psc;
    base[kTimArrOffset / 4] = arr;
    base[kTimEgrOffset / 4] = kTimEgrUgBit;  // Latch immediately

    return true;
}

void StmPwm::ReapplyDuties() noexcept {
    for (hf_u8_t ch = 0; ch < kMaxChannels; ++ch) {
        if (duty_[ch] > 0.0f) {
            SetDutyCycle(ch, duty_[ch]);
        }
    }
}
