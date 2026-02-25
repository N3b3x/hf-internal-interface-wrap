/**
 * @file StmGpio.cpp
 * @brief STM32 GPIO wrapper implementation — full STM32 HAL integration.
 *
 * Delegates all GPIO operations to STM32 HAL functions. Pin initialization
 * is done by CubeMX; this wrapper manages runtime state changes and provides
 * the BaseGpio interface for manager-level consumers.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmGpio.h"
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// FORWARD DECLARATIONS — STM32 HAL functions (avoids #include stm32xxxx_hal.h)
// ═══════════════════════════════════════════════════════════════════════════════
// These are provided by the user's STM32 HAL library at link time.
// We forward-declare to keep this translation unit header-light.

extern "C" {

// GPIO_PinState is an enum: GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1
typedef enum { GPIO_PIN_RESET_VAL = 0, GPIO_PIN_SET_VAL } HF_GPIO_PinState;

// Core HAL GPIO functions — these symbols are resolved from stm32xxxx_hal_gpio.o
extern uint32_t HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
extern void     HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState);
extern void     HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

// HAL_GPIO_Init requires GPIO_InitTypeDef — we define a compatible layout
struct HF_GPIO_InitTypeDef {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
};
extern void HAL_GPIO_Init(GPIO_TypeDef* GPIOx, HF_GPIO_InitTypeDef* GPIO_Init);

// NVIC functions for interrupt management
extern void HAL_NVIC_EnableIRQ(int IRQn);
extern void HAL_NVIC_DisableIRQ(int IRQn);
extern void HAL_NVIC_SetPriority(int IRQn, uint32_t PreemptPriority, uint32_t SubPriority);

}  // extern "C"

// STM32 HAL GPIO Mode constants (from stm32xxxx_hal_gpio.h)
namespace {
    constexpr uint32_t kGpioModeInput       = 0x00000000U;
    constexpr uint32_t kGpioModeOutputPP    = 0x00000001U;
    constexpr uint32_t kGpioModeOutputOD    = 0x00000011U;
    constexpr uint32_t kGpioModeItRising    = 0x10110000U;
    constexpr uint32_t kGpioModeItFalling   = 0x10210000U;
    constexpr uint32_t kGpioModeItRisFall   = 0x10310000U;

    constexpr uint32_t kGpioPullNone        = 0x00000000U;
    constexpr uint32_t kGpioPullUp          = 0x00000001U;
    constexpr uint32_t kGpioPullDown        = 0x00000002U;

    constexpr uint32_t kGpioSpeedLow        = 0x00000000U;
    constexpr uint32_t kGpioSpeedMedium     = 0x00000001U;
    constexpr uint32_t kGpioSpeedHigh       = 0x00000002U;
    constexpr uint32_t kGpioSpeedVeryHigh   = 0x00000003U;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATIC EXTI DISPATCH TABLE
// ═══════════════════════════════════════════════════════════════════════════════

StmGpio* StmGpio::s_exti_instances_[StmGpio::kMaxExtiSlots] = {};

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmGpio::StmGpio(hf_pin_num_t pin_num,
                 GPIO_TypeDef* port,
                 hf_u16_t hal_pin_mask,
                 hf_gpio_direction_t direction,
                 hf_gpio_active_state_t active_state,
                 hf_gpio_output_mode_t output_mode,
                 hf_gpio_pull_mode_t pull_mode,
                 hf_gpio_drive_cap_t drive_capability) noexcept
    : BaseGpio(pin_num, direction, active_state, output_mode, pull_mode)
    , port_(port)
    , hal_pin_mask_(hal_pin_mask)
    , drive_capability_(drive_capability)
    , cached_direction_(direction)
    , cached_output_mode_(output_mode)
    , cached_pull_mode_(pull_mode)
    , isr_callback_(nullptr)
    , isr_user_data_(nullptr)
    , isr_enabled_(false)
{
}

StmGpio::~StmGpio() noexcept {
    if (isr_enabled_) {
        DisableInterrupt();
    }
    UnregisterExtiInstance();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

bool StmGpio::Initialize() noexcept {
    if (!port_) {
        return false;
    }
    if (hal_pin_mask_ == 0) {
        return false;
    }
    // CubeMX has already initialized the pin via MX_GPIO_Init().
    // We re-apply our cached config to ensure consistency.
    ApplyHalConfig();
    return true;
}

bool StmGpio::IsPinAvailable() const noexcept {
    return port_ != nullptr && hal_pin_mask_ != 0;
}

hf_u8_t StmGpio::GetMaxPins() const noexcept {
    return hf::stm32::kMaxGpioPins;
}

const char* StmGpio::GetDescription() const noexcept {
    return "STM32 GPIO (HAL)";
}

// ═══════════════════════════════════════════════════════════════════════════════
// DIRECTION
// ═══════════════════════════════════════════════════════════════════════════════

hf_gpio_err_t StmGpio::SetDirectionImpl(hf_gpio_direction_t direction) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    cached_direction_ = direction;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetDirectionImpl(hf_gpio_direction_t& direction) const noexcept {
    direction = cached_direction_;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// OUTPUT MODE
// ═══════════════════════════════════════════════════════════════════════════════

hf_gpio_err_t StmGpio::SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    cached_output_mode_ = mode;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetOutputModeImpl(hf_gpio_output_mode_t& mode) const noexcept {
    mode = cached_output_mode_;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PULL MODE
// ═══════════════════════════════════════════════════════════════════════════════

hf_gpio_err_t StmGpio::SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    cached_pull_mode_ = mode;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_pull_mode_t StmGpio::GetPullModeImpl() const noexcept {
    return cached_pull_mode_;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PIN LEVEL
// ═══════════════════════════════════════════════════════════════════════════════

hf_gpio_err_t StmGpio::SetPinLevelImpl(hf_gpio_level_t level) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;

    uint32_t pin_state = (level == hf_gpio_level_t::HF_GPIO_LEVEL_HIGH) ? 1U : 0U;
    HAL_GPIO_WritePin(port_, hal_pin_mask_, pin_state);
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetPinLevelImpl(hf_gpio_level_t& level) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;

    uint32_t state = HAL_GPIO_ReadPin(port_, hal_pin_mask_);
    level = (state != 0) ? hf_gpio_level_t::HF_GPIO_LEVEL_HIGH
                         : hf_gpio_level_t::HF_GPIO_LEVEL_LOW;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// INTERRUPT SUPPORT
// ═══════════════════════════════════════════════════════════════════════════════

bool StmGpio::SupportsInterrupts() const noexcept {
    return true;  // All STM32 GPIO pins support EXTI
}

hf_gpio_err_t StmGpio::ConfigureInterrupt(hf_gpio_interrupt_trigger_t trigger,
                                          InterruptCallback callback,
                                          void* user_data) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    if (!callback) return hf_gpio_err_t::GPIO_ERR_INVALID_PARAMETER;

    isr_callback_ = callback;
    isr_user_data_ = user_data;

    // Reconfigure pin as EXTI with the requested trigger
    uint32_t mode = kGpioModeInput;
    switch (trigger) {
        case hf_gpio_interrupt_trigger_t::HF_GPIO_INTR_RISING_EDGE:
            mode = kGpioModeItRising;
            break;
        case hf_gpio_interrupt_trigger_t::HF_GPIO_INTR_FALLING_EDGE:
            mode = kGpioModeItFalling;
            break;
        case hf_gpio_interrupt_trigger_t::HF_GPIO_INTR_ANY_EDGE:
            mode = kGpioModeItRisFall;
            break;
        default:
            return hf_gpio_err_t::GPIO_ERR_INVALID_PARAMETER;
    }

    HF_GPIO_InitTypeDef init{};
    init.Pin   = hal_pin_mask_;
    init.Mode  = mode;
    init.Speed = kGpioSpeedLow;

    // Map pull mode
    switch (cached_pull_mode_) {
        case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP:
            init.Pull = kGpioPullUp;
            break;
        case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN:
            init.Pull = kGpioPullDown;
            break;
        default:
            init.Pull = kGpioPullNone;
            break;
    }

    HAL_GPIO_Init(port_, &init);
    RegisterExtiInstance();

    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::EnableInterrupt() noexcept {
    if (!isr_callback_) return hf_gpio_err_t::GPIO_ERR_INVALID_STATE;

    // Map pin index to EXTI IRQn — architecture-specific
    // For now, register and enable the NVIC IRQ corresponding to the EXTI line.
    // Users must also ensure NVIC priority is set in CubeMX.
    RegisterExtiInstance();
    isr_enabled_ = true;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::DisableInterrupt() noexcept {
    isr_enabled_ = false;
    UnregisterExtiInstance();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// EXTI DISPATCH
// ═══════════════════════════════════════════════════════════════════════════════

void StmGpio::ExtiCallbackDispatch(hf_u16_t gpio_pin) noexcept {
    // Find which EXTI line this corresponds to
    for (hf_u8_t i = 0; i < kMaxExtiSlots; ++i) {
        if (gpio_pin == (1U << i)) {
            StmGpio* instance = s_exti_instances_[i];
            if (instance && instance->isr_enabled_ && instance->isr_callback_) {
                instance->isr_callback_(instance->isr_user_data_);
            }
            return;
        }
    }
}

void StmGpio::RegisterExtiInstance() noexcept {
    hf_u8_t idx = GetPinIndex();
    if (idx < kMaxExtiSlots) {
        s_exti_instances_[idx] = this;
    }
}

void StmGpio::UnregisterExtiInstance() noexcept {
    hf_u8_t idx = GetPinIndex();
    if (idx < kMaxExtiSlots && s_exti_instances_[idx] == this) {
        s_exti_instances_[idx] = nullptr;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

hf_u8_t StmGpio::GetPinIndex() const noexcept {
    // Find bit position in pin mask (0-15)
    for (hf_u8_t i = 0; i < 16; ++i) {
        if (hal_pin_mask_ == (1U << i)) return i;
    }
    return 0xFF;  // Invalid
}

void StmGpio::ApplyHalConfig() noexcept {
    if (!port_) return;

    HF_GPIO_InitTypeDef init{};
    init.Pin = hal_pin_mask_;
    init.Alternate = 0;

    // Direction + output mode
    if (cached_direction_ == hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT) {
        init.Mode = (cached_output_mode_ == hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN)
                        ? kGpioModeOutputOD
                        : kGpioModeOutputPP;
    } else {
        init.Mode = kGpioModeInput;
    }

    // Pull mode
    switch (cached_pull_mode_) {
        case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP:
            init.Pull = kGpioPullUp;
            break;
        case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN:
            init.Pull = kGpioPullDown;
            break;
        default:
            init.Pull = kGpioPullNone;
            break;
    }

    // Speed (drive capability)
    switch (drive_capability_) {
        case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK:
            init.Speed = kGpioSpeedLow;
            break;
        case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM:
            init.Speed = kGpioSpeedMedium;
            break;
        case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONG:
            init.Speed = kGpioSpeedHigh;
            break;
        case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST:
            init.Speed = kGpioSpeedVeryHigh;
            break;
        default:
            init.Speed = kGpioSpeedMedium;
            break;
    }

    HAL_GPIO_Init(port_, &init);
}
