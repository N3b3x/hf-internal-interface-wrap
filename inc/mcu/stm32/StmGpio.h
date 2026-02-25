/**
 * @file StmGpio.h
 * @brief STM32 GPIO wrapper — full HAL integration with EXTI interrupt support.
 *
 * Users configure GPIO pins in STM32CubeMX, then use this wrapper to
 * provide the BaseGpio interface. Runtime direction/pull/output-mode changes
 * re-apply HAL_GPIO_Init. EXTI interrupts are routed through a static
 * dispatch table.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseGpio.h"
#include "StmTypes.h"

struct GPIO_TypeDef;  // Forward — defined by stm32xxxx_hal_gpio.h

/**
 * @class StmGpio
 * @brief STM32 GPIO implementation with full HAL integration.
 *
 * Design:
 * - Constructor accepts the HAL port pointer + pin mask (matching CubeMX output).
 * - Alternatively can use the hf_pin_num_t encoding (port_index * 16 + pin_index)
 *   if the port array is registered via StmTypes utility functions.
 * - Runtime reconfiguration calls HAL_GPIO_Init with a rebuilt InitTypeDef.
 * - EXTI dispatch: a static array of 16 slots (one per EXTI line) routes
 *   HAL_GPIO_EXTI_Callback to the correct instance.
 */
class StmGpio : public BaseGpio {
public:

    /**
     * @brief Full constructor with HAL port + pin mask.
     * @param pin_num          Logical pin number (encoded: port*16 + pin_index).
     * @param port             GPIO_TypeDef* port (e.g. GPIOA).
     * @param hal_pin_mask     HAL pin bitmask (e.g. GPIO_PIN_5 = 1<<5).
     * @param direction        Input / Output / Bidirectional.
     * @param active_state     Active high / low.
     * @param output_mode      Push-pull / Open-drain.
     * @param pull_mode        None / Pull-up / Pull-down.
     * @param drive_capability Low / Medium / High / Very-High.
     */
    explicit StmGpio(
        hf_pin_num_t           pin_num,
        GPIO_TypeDef*          port,
        hf_u16_t               hal_pin_mask,
        hf_gpio_direction_t    direction        = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
        hf_gpio_active_state_t active_state     = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
        hf_gpio_output_mode_t  output_mode      = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
        hf_gpio_pull_mode_t    pull_mode        = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
        hf_gpio_drive_cap_t    drive_capability = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM
    ) noexcept;

    /**
     * @brief Construct from config struct.
     */
    explicit StmGpio(const hf_stm32_gpio_config_t& config) noexcept;

    ~StmGpio() noexcept override;

    // ── Public pure-virtual overrides ───────────────────────────────────────
    bool Initialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    hf_u8_t GetMaxPins() const noexcept override;
    const char* GetDescription() const noexcept override;

    // ── EXTI dispatch (call from HAL_GPIO_EXTI_Callback in user code) ───────
    /**
     * @brief Route EXTI callback to the registered StmGpio instance.
     *
     * Usage:
     * @code
     * void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
     *     StmGpio::ExtiCallbackDispatch(GPIO_Pin);
     * }
     * @endcode
     */
    static void ExtiCallbackDispatch(hf_u16_t gpio_pin_mask) noexcept;

    // ── Accessors ───────────────────────────────────────────────────────────
    GPIO_TypeDef* GetPort() const noexcept { return port_; }
    hf_u16_t GetHalPinMask() const noexcept { return hal_pin_mask_; }

protected:
    // ── Platform implementation overrides ───────────────────────────────────
    hf_gpio_err_t SetDirectionImpl(hf_gpio_direction_t direction) noexcept override;
    hf_gpio_err_t GetDirectionImpl(hf_gpio_direction_t& direction) const noexcept override;
    hf_gpio_err_t SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept override;
    hf_gpio_err_t GetOutputModeImpl(hf_gpio_output_mode_t& mode) const noexcept override;
    hf_gpio_err_t SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept override;
    hf_gpio_pull_mode_t GetPullModeImpl() const noexcept override;
    hf_gpio_err_t SetPinLevelImpl(hf_gpio_level_t level) noexcept override;
    hf_gpio_err_t GetPinLevelImpl(hf_gpio_level_t& level) noexcept override;

private:
    GPIO_TypeDef*       port_;
    hf_u16_t            hal_pin_mask_;
    hf_gpio_direction_t direction_;
    hf_gpio_output_mode_t output_mode_;
    hf_gpio_pull_mode_t pull_mode_;
    hf_gpio_drive_cap_t drive_cap_;

    /// Rebuild and apply HAL_GPIO_Init for the current config.
    void ApplyHalConfig() noexcept;

    /// Static EXTI dispatch table — one slot per EXTI line (0–15).
    static StmGpio* s_exti_instances_[16];
};
