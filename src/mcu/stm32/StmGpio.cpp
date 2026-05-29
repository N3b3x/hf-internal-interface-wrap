/**
 * @file StmGpio.cpp
 * @brief STM32 GPIO wrapper — HAL-backed BaseGpio implementation.
 */

#include "StmGpio.h"

extern "C" {
extern uint32_t HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
extern void     HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState);

struct HF_GPIO_InitTypeDef {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
};
extern void HAL_GPIO_Init(GPIO_TypeDef* GPIOx, HF_GPIO_InitTypeDef* GPIO_Init);
}

namespace {
constexpr uint32_t kGpioModeInput     = 0x00000000U;
constexpr uint32_t kGpioModeOutputPP  = 0x00000001U;
constexpr uint32_t kGpioModeOutputOD  = 0x00000011U;

constexpr uint32_t kGpioPullNone      = 0x00000000U;
constexpr uint32_t kGpioPullUp        = 0x00000001U;
constexpr uint32_t kGpioPullDown      = 0x00000002U;

constexpr uint32_t kGpioSpeedLow      = 0x00000000U;
constexpr uint32_t kGpioSpeedMedium   = 0x00000001U;
constexpr uint32_t kGpioSpeedHigh     = 0x00000002U;
constexpr uint32_t kGpioSpeedVeryHigh = 0x00000003U;
}  // namespace

StmGpio* StmGpio::s_exti_instances_[16] = {};

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
    , direction_(direction)
    , output_mode_(output_mode)
    , pull_mode_(pull_mode)
    , drive_cap_(drive_capability) {}

StmGpio::StmGpio(const hf_stm32_gpio_config_t& config) noexcept
    : StmGpio(0, config.port, config.pin_mask) {}

StmGpio::~StmGpio() noexcept = default;

bool StmGpio::Initialize() noexcept {
  if (!port_ || hal_pin_mask_ == 0) return false;
    ApplyHalConfig();
    return true;
}

bool StmGpio::IsPinAvailable() const noexcept {
    return port_ != nullptr && hal_pin_mask_ != 0;
}

hf_u8_t StmGpio::GetMaxPins() const noexcept { return hf::stm32::kMaxGpioPins; }

const char* StmGpio::GetDescription() const noexcept { return "STM32 GPIO (HAL)"; }

void StmGpio::ExtiCallbackDispatch(hf_u16_t /*gpio_pin_mask*/) noexcept {}

hf_gpio_err_t StmGpio::SetDirectionImpl(hf_gpio_direction_t direction) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  direction_ = direction;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetDirectionImpl(hf_gpio_direction_t& direction) const noexcept {
  direction = direction_;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  output_mode_ = mode;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetOutputModeImpl(hf_gpio_output_mode_t& mode) const noexcept {
  mode = output_mode_;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  pull_mode_ = mode;
    ApplyHalConfig();
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_pull_mode_t StmGpio::GetPullModeImpl() const noexcept { return pull_mode_; }

hf_gpio_err_t StmGpio::SetPinLevelImpl(hf_gpio_level_t level) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  const uint32_t pin_state = (level == hf_gpio_level_t::HF_GPIO_LEVEL_HIGH) ? 1U : 0U;
    HAL_GPIO_WritePin(port_, hal_pin_mask_, pin_state);
    return hf_gpio_err_t::GPIO_SUCCESS;
}

hf_gpio_err_t StmGpio::GetPinLevelImpl(hf_gpio_level_t& level) noexcept {
    if (!port_) return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
  const uint32_t state = HAL_GPIO_ReadPin(port_, hal_pin_mask_);
  level = (state != 0) ? hf_gpio_level_t::HF_GPIO_LEVEL_HIGH : hf_gpio_level_t::HF_GPIO_LEVEL_LOW;
    return hf_gpio_err_t::GPIO_SUCCESS;
}

void StmGpio::ApplyHalConfig() noexcept {
    if (!port_) return;

    HF_GPIO_InitTypeDef init{};
    init.Pin = hal_pin_mask_;
    init.Alternate = 0;

  if (direction_ == hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT) {
    init.Mode = (output_mode_ == hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN)
                        ? kGpioModeOutputOD
                        : kGpioModeOutputPP;
    } else {
        init.Mode = kGpioModeInput;
    }

  switch (pull_mode_) {
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

  switch (drive_cap_) {
        case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK:
            init.Speed = kGpioSpeedLow;
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
