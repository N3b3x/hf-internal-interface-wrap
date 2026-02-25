/**
 * @file StmCan.h
 * @brief STM32 CAN wrapper — wraps bxCAN (CAN_HandleTypeDef) or FDCAN
 *        (FDCAN_HandleTypeDef) for BaseCan interface.
 *
 * Users configure CAN/FDCAN in STM32CubeMX, then pass the handle here.
 * Basic CAN 2.0 is supported out of the box; FDCAN flags are available but
 * data-phase bitrate configuration requires the FDCAN HAL extensions.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseCan.h"
#include "StmTypes.h"

struct CAN_HandleTypeDef;
struct FDCAN_HandleTypeDef;

/**
 * @brief Which CAN peripheral variant is in use.
 */
enum class StmCanVariant : hf_u8_t {
    BXCAN,   ///< Classic bxCAN (STM32F1/F2/F4/L4)
    FDCAN    ///< FDCAN (STM32G0/G4/H7/L5/U5/WB/WL)
};

/**
 * @class StmCan
 * @brief STM32 CAN implementation.
 *
 * Design:
 * - Supports both bxCAN and FDCAN via a variant tag + union of handles.
 * - SendMessage/ReceiveMessage use blocking HAL calls.
 * - Acceptance filter wraps HAL filter configuration.
 * - Receive callback stored for ISR forwarding.
 */
class StmCan : public BaseCan {
public:

    /**
     * @brief Construct for bxCAN.
     */
    explicit StmCan(CAN_HandleTypeDef* hcan) noexcept;

    /**
     * @brief Construct for FDCAN.
     */
    explicit StmCan(FDCAN_HandleTypeDef* hfdcan) noexcept;

    /**
     * @brief Construct from config struct.
     */
    explicit StmCan(const hf_stm32_can_config_t& config) noexcept;

    ~StmCan() noexcept override;

    StmCan(const StmCan&) = delete;
    StmCan& operator=(const StmCan&) = delete;

    // ── Pure-virtual overrides (BaseCan) ────────────────────────────────────
    hf_can_err_t Initialize() noexcept override;
    hf_can_err_t Deinitialize() noexcept override;
    hf_can_err_t SendMessage(const hf_can_message_t& message,
                             hf_u32_t timeout_ms = 1000) noexcept override;
    hf_can_err_t ReceiveMessage(hf_can_message_t& message,
                                hf_u32_t timeout_ms = 0) noexcept override;
    hf_can_err_t SetReceiveCallback(hf_can_receive_callback_t callback) noexcept override;
    hf_can_err_t SetAcceptanceFilter(hf_u32_t id, hf_u32_t mask,
                                     bool extended = false) noexcept override;
    hf_can_err_t GetStatus(hf_can_status_t& status) noexcept override;
    hf_can_err_t Reset() noexcept override;

    // ── Optional overrides ──────────────────────────────────────────────────
    bool SupportsCanFD() const noexcept override;

    // ── Accessors ───────────────────────────────────────────────────────────
    StmCanVariant GetVariant() const noexcept { return variant_; }

    /**
     * @brief ISR helper — call from HAL CAN/FDCAN RX callback.
     */
    void NotifyRxFromIsr(const hf_can_message_t& msg) noexcept;

private:
    StmCanVariant variant_;
    union {
        CAN_HandleTypeDef*   hcan_;
        FDCAN_HandleTypeDef* hfdcan_;
    };
    hf_can_receive_callback_t rx_callback_;

    hf_can_err_t SendBxCan(const hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept;
    hf_can_err_t ReceiveBxCan(hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept;
    hf_can_err_t SendFdCan(const hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept;
    hf_can_err_t ReceiveFdCan(hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept;
};
