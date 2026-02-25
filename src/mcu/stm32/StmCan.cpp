/**
 * @file StmCan.cpp
 * @brief STM32 CAN implementation — bxCAN + FDCAN dual-variant support.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmCan.h"
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS (bxCAN)
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
// bxCAN
extern uint32_t HAL_CAN_Start(CAN_HandleTypeDef* hcan);
extern uint32_t HAL_CAN_Stop(CAN_HandleTypeDef* hcan);
extern uint32_t HAL_CAN_AddTxMessage(CAN_HandleTypeDef* hcan,
                                     const void* pHeader,
                                     const uint8_t aData[],
                                     uint32_t* pTxMailbox);
extern uint32_t HAL_CAN_GetRxMessage(CAN_HandleTypeDef* hcan,
                                     uint32_t RxFifo,
                                     void* pHeader,
                                     uint8_t aData[]);
extern uint32_t HAL_CAN_GetRxFifoFillLevel(CAN_HandleTypeDef* hcan, uint32_t RxFifo);
extern uint32_t HAL_CAN_ConfigFilter(CAN_HandleTypeDef* hcan, const void* sFilterConfig);
extern uint32_t HAL_CAN_GetError(CAN_HandleTypeDef* hcan);
extern uint32_t HAL_CAN_ResetError(CAN_HandleTypeDef* hcan);

// FDCAN
extern uint32_t HAL_FDCAN_Start(FDCAN_HandleTypeDef* hfdcan);
extern uint32_t HAL_FDCAN_Stop(FDCAN_HandleTypeDef* hfdcan);
extern uint32_t HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef* hfdcan,
                                               const void* pTxHeader,
                                               const uint8_t* pTxData);
extern uint32_t HAL_FDCAN_GetRxMessage(FDCAN_HandleTypeDef* hfdcan,
                                        uint32_t RxLocation,
                                        void* pRxHeader,
                                        uint8_t* pRxData);
extern uint32_t HAL_FDCAN_GetRxFifoFillLevel(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo);
extern uint32_t HAL_FDCAN_ConfigFilter(FDCAN_HandleTypeDef* hfdcan, const void* sFilterConfig);
extern uint32_t HAL_FDCAN_GetError(FDCAN_HandleTypeDef* hfdcan);

extern uint32_t HAL_GetTick(void);
}

// bxCAN FIFO constants
namespace {
    constexpr uint32_t kCanRxFifo0 = 0x00000000U;
}

// ═══════════════════════════════════════════════════════════════════════════════
// bxCAN TX/RX header structures (compatible layout)
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    struct BxCanTxHeader {
        uint32_t StdId;
        uint32_t ExtId;
        uint32_t IDE;      // 0 = standard, 4 = extended
        uint32_t RTR;      // 0 = data, 2 = remote
        uint32_t DLC;
        uint32_t TransmitGlobalTime;
    };

    struct BxCanRxHeader {
        uint32_t StdId;
        uint32_t ExtId;
        uint32_t IDE;
        uint32_t RTR;
        uint32_t DLC;
        uint32_t FilterMatchIndex;
        uint32_t Timestamp;
    };

    struct BxCanFilterConfig {
        uint32_t FilterIdHigh;
        uint32_t FilterIdLow;
        uint32_t FilterMaskIdHigh;
        uint32_t FilterMaskIdLow;
        uint32_t FilterFIFOAssignment;
        uint32_t FilterBank;
        uint32_t FilterMode;        // 0 = mask
        uint32_t FilterScale;       // 1 = 32-bit
        uint32_t FilterActivation;  // 1 = enable
        uint32_t SlaveStartFilterBank;
    };

    // FDCAN TX/RX header (simplified — layout varies by STM32 family)
    struct FdCanTxHeader {
        uint32_t Identifier;
        uint32_t IdType;         // 0=standard, 1=extended
        uint32_t TxFrameType;   // 0=data, 1=remote
        uint32_t DataLength;    // DLC code
        uint32_t ErrorStateIndicator;
        uint32_t BitRateSwitch;
        uint32_t FDFormat;
        uint32_t TxEventFifoControl;
        uint32_t MessageMarker;
    };

    struct FdCanRxHeader {
        uint32_t Identifier;
        uint32_t IdType;
        uint32_t RxFrameType;
        uint32_t DataLength;
        uint32_t ErrorStateIndicator;
        uint32_t BitRateSwitch;
        uint32_t FDFormat;
        uint32_t RxTimestamp;
        uint32_t FilterIndex;
        uint32_t IsFilterMatchingFrame;
    };

    struct FdCanFilterConfig {
        uint32_t IdType;
        uint32_t FilterIndex;
        uint32_t FilterType;
        uint32_t FilterConfig;
        uint32_t FilterID1;
        uint32_t FilterID2;
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTORS / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmCan::StmCan(CAN_HandleTypeDef* hcan) noexcept
    : BaseCan()
    , variant_(StmCanVariant::BXCAN)
    , rx_callback_(nullptr)
{
    hcan_ = hcan;
}

StmCan::StmCan(FDCAN_HandleTypeDef* hfdcan) noexcept
    : BaseCan()
    , variant_(StmCanVariant::FDCAN)
    , rx_callback_(nullptr)
{
    hfdcan_ = hfdcan;
}

StmCan::StmCan(const hf_stm32_can_config_t& config) noexcept
    : BaseCan()
    , variant_(config.use_fdcan ? StmCanVariant::FDCAN : StmCanVariant::BXCAN)
    , rx_callback_(nullptr)
{
    if (config.use_fdcan) {
        hfdcan_ = config.hfdcan;
    } else {
        hcan_ = config.hcan;
    }
}

StmCan::~StmCan() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

hf_can_err_t StmCan::Initialize() noexcept {
    if (initialized_) return hf_can_err_t::CAN_ERR_ALREADY_INITIALIZED;

    uint32_t status;
    if (variant_ == StmCanVariant::BXCAN) {
        if (!hcan_) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
        status = HAL_CAN_Start(hcan_);
    } else {
        if (!hfdcan_) return hf_can_err_t::CAN_ERR_INVALID_PARAMETER;
        status = HAL_FDCAN_Start(hfdcan_);
    }

    if (!hf::stm32::IsHalOk(status)) return hf_can_err_t::CAN_ERR_HARDWARE_FAULT;

    initialized_ = true;
    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::Deinitialize() noexcept {
    if (!initialized_) return hf_can_err_t::CAN_SUCCESS;

    if (variant_ == StmCanVariant::BXCAN) {
        HAL_CAN_Stop(hcan_);
    } else {
        HAL_FDCAN_Stop(hfdcan_);
    }

    initialized_ = false;
    return hf_can_err_t::CAN_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// SEND / RECEIVE
// ═══════════════════════════════════════════════════════════════════════════════

hf_can_err_t StmCan::SendMessage(const hf_can_message_t& msg,
                                  hf_u32_t timeout_ms) noexcept {
    if (!initialized_) return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;

    if (variant_ == StmCanVariant::BXCAN) {
        return SendBxCan(msg, timeout_ms);
    } else {
        return SendFdCan(msg, timeout_ms);
    }
}

hf_can_err_t StmCan::ReceiveMessage(hf_can_message_t& msg,
                                     hf_u32_t timeout_ms) noexcept {
    if (!initialized_) return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;

    if (variant_ == StmCanVariant::BXCAN) {
        return ReceiveBxCan(msg, timeout_ms);
    } else {
        return ReceiveFdCan(msg, timeout_ms);
    }
}

// ── bxCAN variants ──────────────────────────────────────────────────────────

hf_can_err_t StmCan::SendBxCan(const hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept {
    BxCanTxHeader hdr{};
    if (msg.is_extended) {
        hdr.IDE = 4;  // CAN_ID_EXT
        hdr.ExtId = msg.id;
    } else {
        hdr.IDE = 0;  // CAN_ID_STD
        hdr.StdId = msg.id;
    }
    hdr.RTR = msg.is_rtr ? 2 : 0;
    hdr.DLC = msg.dlc;
    hdr.TransmitGlobalTime = 0;

    uint32_t tx_mailbox = 0;
    uint32_t status = HAL_CAN_AddTxMessage(hcan_, &hdr, msg.data, &tx_mailbox);

    if (!hf::stm32::IsHalOk(status)) {
        statistics_.tx_error_count++;
        return hf_can_err_t::CAN_ERR_TX_FAILED;
    }

    // Poll for TX completion (simplified)
    (void)timeout_ms;
    statistics_.tx_message_count++;
    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::ReceiveBxCan(hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept {
    uint32_t start_tick = HAL_GetTick();

    // Wait for message in FIFO0
    while (HAL_CAN_GetRxFifoFillLevel(hcan_, kCanRxFifo0) == 0) {
        if (timeout_ms > 0 && (HAL_GetTick() - start_tick) >= timeout_ms) {
            return hf_can_err_t::CAN_ERR_TIMEOUT;
        }
    }

    BxCanRxHeader hdr{};
    uint8_t data[8]{};
    uint32_t status = HAL_CAN_GetRxMessage(hcan_, kCanRxFifo0, &hdr, data);
    if (!hf::stm32::IsHalOk(status)) {
        statistics_.rx_error_count++;
        return hf_can_err_t::CAN_ERR_RX_FAILED;
    }

    std::memset(&msg, 0, sizeof(msg));
    msg.is_extended = (hdr.IDE != 0);
    msg.id = msg.is_extended ? hdr.ExtId : hdr.StdId;
    msg.is_rtr = (hdr.RTR != 0);
    msg.dlc = static_cast<hf_u8_t>(hdr.DLC);
    std::memcpy(msg.data, data, (hdr.DLC <= 8) ? hdr.DLC : 8);
    msg.timestamp_us = static_cast<hf_u64_t>(hdr.Timestamp);

    statistics_.rx_message_count++;
    return hf_can_err_t::CAN_SUCCESS;
}

// ── FDCAN variants ──────────────────────────────────────────────────────────

hf_can_err_t StmCan::SendFdCan(const hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept {
    (void)timeout_ms;

    FdCanTxHeader hdr{};
    hdr.Identifier = msg.id;
    hdr.IdType = msg.is_extended ? 1 : 0;
    hdr.TxFrameType = msg.is_rtr ? 1 : 0;

    // FDCAN DLC encoding: 0–8 direct, 12/16/20/24/32/48/64 for FD
    hdr.DataLength = msg.dlc;  // Simplified — user must set correct DLC code
    hdr.ErrorStateIndicator = 0;
    hdr.BitRateSwitch = (msg.is_canfd && msg.is_brs) ? 1 : 0;
    hdr.FDFormat = msg.is_canfd ? 1 : 0;
    hdr.TxEventFifoControl = 0;
    hdr.MessageMarker = 0;

    uint32_t status = HAL_FDCAN_AddMessageToTxFifoQ(hfdcan_, &hdr, msg.data);
    if (!hf::stm32::IsHalOk(status)) {
        statistics_.tx_error_count++;
        return hf_can_err_t::CAN_ERR_TX_FAILED;
    }

    statistics_.tx_message_count++;
    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::ReceiveFdCan(hf_can_message_t& msg, hf_u32_t timeout_ms) noexcept {
    uint32_t start_tick = HAL_GetTick();

    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan_, 0) == 0) {
        if (timeout_ms > 0 && (HAL_GetTick() - start_tick) >= timeout_ms) {
            return hf_can_err_t::CAN_ERR_TIMEOUT;
        }
    }

    FdCanRxHeader hdr{};
    uint8_t data[64]{};
    uint32_t status = HAL_FDCAN_GetRxMessage(hfdcan_, 0, &hdr, data);
    if (!hf::stm32::IsHalOk(status)) {
        statistics_.rx_error_count++;
        return hf_can_err_t::CAN_ERR_RX_FAILED;
    }

    std::memset(&msg, 0, sizeof(msg));
    msg.id = hdr.Identifier;
    msg.is_extended = (hdr.IdType != 0);
    msg.is_rtr = (hdr.RxFrameType != 0);
    msg.dlc = static_cast<hf_u8_t>(hdr.DataLength);
    msg.is_canfd = (hdr.FDFormat != 0);
    msg.is_brs = (hdr.BitRateSwitch != 0);
    msg.is_esi = (hdr.ErrorStateIndicator != 0);
    msg.timestamp_us = static_cast<hf_u64_t>(hdr.RxTimestamp);

    size_t copy_len = (msg.dlc <= 64) ? msg.dlc : 64;
    std::memcpy(msg.data, data, (copy_len <= 8) ? copy_len : 8);

    statistics_.rx_message_count++;
    return hf_can_err_t::CAN_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// FILTER / STATUS / CALLBACK / RESET
// ═══════════════════════════════════════════════════════════════════════════════

hf_can_err_t StmCan::SetReceiveCallback(hf_can_receive_callback_t callback) noexcept {
    rx_callback_ = callback;
    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::SetAcceptanceFilter(hf_u32_t id, hf_u32_t mask,
                                          bool extended) noexcept {
    if (!initialized_) return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;

    if (variant_ == StmCanVariant::BXCAN) {
        BxCanFilterConfig filter{};
        filter.FilterBank = 0;
        filter.FilterMode = 0;         // Mask mode
        filter.FilterScale = 1;        // 32-bit
        filter.FilterFIFOAssignment = 0;
        filter.FilterActivation = (id != 0 || mask != 0) ? 1 : 0;

        if (extended) {
            filter.FilterIdHigh = static_cast<uint32_t>((id << 3) >> 16);
            filter.FilterIdLow = static_cast<uint32_t>((id << 3) & 0xFFFF) | 0x04;
            filter.FilterMaskIdHigh = static_cast<uint32_t>((mask << 3) >> 16);
            filter.FilterMaskIdLow = static_cast<uint32_t>((mask << 3) & 0xFFFF) | 0x04;
        } else {
            filter.FilterIdHigh = static_cast<uint32_t>(id << 5);
            filter.FilterIdLow = 0;
            filter.FilterMaskIdHigh = static_cast<uint32_t>(mask << 5);
            filter.FilterMaskIdLow = 0;
        }

        uint32_t status = HAL_CAN_ConfigFilter(hcan_, &filter);
        if (!hf::stm32::IsHalOk(status)) return hf_can_err_t::CAN_ERR_FILTER_CONFIG_FAILED;

    } else { // FDCAN
        FdCanFilterConfig filter{};
        filter.IdType = extended ? 1 : 0;
        filter.FilterIndex = 0;
        filter.FilterType = 0;   // Range filter
        filter.FilterConfig = 1; // Store in FIFO0
        filter.FilterID1 = id;
        filter.FilterID2 = mask;

        uint32_t status = HAL_FDCAN_ConfigFilter(hfdcan_, &filter);
        if (!hf::stm32::IsHalOk(status)) return hf_can_err_t::CAN_ERR_FILTER_CONFIG_FAILED;
    }

    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::GetStatus(hf_can_status_t& status) noexcept {
    if (!initialized_) return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;

    std::memset(&status, 0, sizeof(status));

    if (variant_ == StmCanVariant::BXCAN) {
        uint32_t err = HAL_CAN_GetError(hcan_);
        status.tx_error_count = static_cast<hf_u16_t>(err & 0xFF);
        status.rx_error_count = static_cast<hf_u16_t>((err >> 8) & 0xFF);
    } else {
        uint32_t err = HAL_FDCAN_GetError(hfdcan_);
        status.tx_error_count = static_cast<hf_u16_t>(err & 0xFF);
        status.rx_error_count = static_cast<hf_u16_t>((err >> 8) & 0xFF);
    }

    return hf_can_err_t::CAN_SUCCESS;
}

hf_can_err_t StmCan::Reset() noexcept {
    if (!initialized_) return hf_can_err_t::CAN_ERR_NOT_INITIALIZED;

    Deinitialize();
    return Initialize();
}

bool StmCan::SupportsCanFD() const noexcept {
    return variant_ == StmCanVariant::FDCAN;
}

void StmCan::NotifyRxFromIsr(const hf_can_message_t& msg) noexcept {
    statistics_.rx_message_count++;
    if (rx_callback_) {
        rx_callback_(msg);
    }
}
