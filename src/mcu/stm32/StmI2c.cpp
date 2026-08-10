/**
 * @file StmI2c.cpp
 * @brief STM32 I2C wrapper implementation — full STM32 HAL integration.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmI2c.h"

// Compile this wrapper only on cores whose CubeMX HAL config enables the
// module (ADR-003: bus sovereignty is per-core). HAL-less builds keep the
// manual prototypes below.
#if !defined(USE_HAL_DRIVER) || defined(HAL_I2C_MODULE_ENABLED)

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

// With the real CubeMX HAL present these come from stm32h7xx_hal_i2c.h and return
// HAL_StatusTypeDef; re-declaring them as uint32_t is a conflicting declaration.
#if !defined(USE_HAL_DRIVER)
extern "C" {
extern uint32_t HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                        uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                       uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                  uint16_t MemAddress, uint16_t MemAddSize,
                                  uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                 uint16_t MemAddress, uint16_t MemAddSize,
                                 uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                      uint32_t Trials, uint32_t Timeout);
}
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// StmI2cDevice
// ═══════════════════════════════════════════════════════════════════════════════

StmI2cDevice::StmI2cDevice(StmI2cBus* parent, const hf_i2c_device_config_t& config) noexcept
    : parent_bus_(parent), config_(config) {}

StmI2cDevice::~StmI2cDevice() noexcept {
    if (initialized_) Deinitialize();
}

bool StmI2cDevice::Initialize() noexcept {
    if (initialized_) return true;
    if (!parent_bus_ || !parent_bus_->IsInitialized()) return false;
    if (!parent_bus_->GetHalHandle()) return false;

    /* Do not call HAL_I2C_IsDeviceReady here. On STM32H7 it can leave the
     * master with ISR.TXIS latched while State==READY, which breaks the next
     * Mem_Read: WaitOnTXIS returns immediately, the command byte is written
     * into TXDR as a dummy flush and never reaches the wire — every subsequent
     * "register read" returns INPUT_PORT (pointer stuck at 0). */
    initialized_ = true;
    return true;
}

bool StmI2cDevice::Deinitialize() noexcept {
    initialized_ = false;
    return true;
}

namespace {

/** Mirror of HAL private I2C_Flush_TXDR — clear sticky TXIS before a new xfer. */
void FlushTxdr(I2C_HandleTypeDef* hi2c) noexcept {
    if (hi2c == nullptr) {
        return;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) != RESET) {
        hi2c->Instance->TXDR = 0x00U;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) == RESET) {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);
    }
}

/** Drop a stale RX byte so the next Mem_Read cannot splice a prior payload. */
void FlushRxdr(I2C_HandleTypeDef* hi2c) noexcept {
    if (hi2c == nullptr) {
        return;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) != RESET) {
        (void)hi2c->Instance->RXDR;
    }
}

/** Soft-recover STM32H7 I2C after NACK/timeout so the next xfer can proceed. */
void RecoverI2cAfterError(I2C_HandleTypeDef* hi2c) noexcept {
    if (hi2c == nullptr) {
        return;
    }
    if (hi2c->State != HAL_I2C_STATE_READY) {
        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;
        hi2c->Lock = HAL_UNLOCKED;
    }
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF | I2C_FLAG_STOPF | I2C_FLAG_BERR |
                                   I2C_FLAG_ARLO | I2C_FLAG_OVR);
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    /* Drop stale transfer programming; do not PE-toggle on every NACK — that
     * was observed to leave the I2C master unresponsive after deliberate NACK
     * probes on busy expander buses. */
    hi2c->Instance->CR2 &= ~(I2C_CR2_START | I2C_CR2_STOP | I2C_CR2_NBYTES |
                             I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN |
                             I2C_CR2_ADD10 | I2C_CR2_SADD | I2C_CR2_HEAD10R);
    FlushTxdr(hi2c);
    FlushRxdr(hi2c);
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != RESET) {
        CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_PE);
        for (volatile int i = 0; i < 32; ++i) {
        }
        SET_BIT(hi2c->Instance->CR1, I2C_CR1_PE);
        FlushTxdr(hi2c);
        FlushRxdr(hi2c);
    }
}

/** Prepare a READY master for a new blocking transfer. */
void PrepareMasterXfer(I2C_HandleTypeDef* hi2c) noexcept {
    if (hi2c == nullptr) {
        return;
    }
    if (hi2c->State != HAL_I2C_STATE_READY) {
        RecoverI2cAfterError(hi2c);
    }
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    /* Drop stale NBYTES/AUTOEND — leftover NBYTES from a prior Master_Transmit
     * made the next Mem_Read address-phase mis-count. */
    hi2c->Instance->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                             I2C_CR2_START | I2C_CR2_STOP | I2C_CR2_RD_WRN);
    /* Match HAL I2C_Flush_TXDR: sticky TXIS while State==READY makes
     * I2C_RequestMemoryRead's WaitOnTXIS return immediately and dump the
     * command byte into TXDR without a real START — PCA pointer stays at
     * INPUT_PORT and every bank read looks identical. */
    FlushTxdr(hi2c);
    FlushRxdr(hi2c);
}

bool HalXferOk(I2C_HandleTypeDef* hi2c, uint32_t status) noexcept {
    return status == static_cast<uint32_t>(HAL_OK) &&
           hi2c->ErrorCode == HAL_I2C_ERROR_NONE;
}

}  // namespace

hf_i2c_err_t StmI2cDevice::Write(const hf_u8_t* data, hf_u16_t length,
                                  hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    if (!parent_bus_) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;

    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    if (!parent_bus_->LockBus(effective_timeout)) {
        return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
    }

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;

    /* PCA/TCA9555 register write = [cmd][data…]. Master_Transmit after TXIS
     * flush; Mem_Write is equivalent but shares the sticky-TXIS footgun. */
    PrepareMasterXfer(hi2c);
    uint32_t status = HAL_I2C_Master_Transmit(
        hi2c, addr_shifted, const_cast<uint8_t*>(data), length,
        effective_timeout);

    hf_i2c_err_t result = HalXferOk(hi2c, status)
                              ? hf_i2c_err_t::I2C_SUCCESS
                              : ConvertHalStatus(status);
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        statistics_.total_transactions++;
        statistics_.successful_transactions++;
        statistics_.bytes_written += length;
    } else {
        statistics_.failed_transactions++;
        RecoverI2cAfterError(hi2c);
    }
    parent_bus_->UnlockBus();
    return result;
}

hf_i2c_err_t StmI2cDevice::Read(hf_u8_t* data, hf_u16_t length,
                                 hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    if (!parent_bus_) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;

    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    if (!parent_bus_->LockBus(effective_timeout)) {
        return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
    }

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;

    PrepareMasterXfer(hi2c);
    uint32_t status = HAL_I2C_Master_Receive(
        hi2c, addr_shifted, data, length, effective_timeout);

    hf_i2c_err_t result = HalXferOk(hi2c, status)
                              ? hf_i2c_err_t::I2C_SUCCESS
                              : ConvertHalStatus(status);
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        statistics_.total_transactions++;
        statistics_.successful_transactions++;
        statistics_.bytes_read += length;
    } else {
        statistics_.failed_transactions++;
        RecoverI2cAfterError(hi2c);
    }
    parent_bus_->UnlockBus();
    return result;
}

hf_i2c_err_t StmI2cDevice::WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                      hf_u8_t* rx_data, hf_u16_t rx_length,
                                      hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
        return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    }
    if (!parent_bus_) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;

    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    if (!parent_bus_->LockBus(effective_timeout)) {
        return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
    }

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;
    hf_i2c_err_t result;

    /* Register / combined read: Master_Transmit(cmd…) + Master_Receive.
     *
     * Avoid HAL_I2C_Mem_Read on STM32H7: sticky TXIS while State==READY can
     * make I2C_RequestMemoryRead emit the command byte without a real START.
     * Writes already use Master_Transmit successfully; the same framing for
     * the command phase keeps multi-byte register pointer updates reliable. */
    result = hf_i2c_err_t::I2C_ERR_READ_FAILURE;
    for (int attempt = 0; attempt < 2; ++attempt) {
        PrepareMasterXfer(hi2c);
        uint32_t status = HAL_I2C_Master_Transmit(
            hi2c, addr_shifted, const_cast<uint8_t*>(tx_data), tx_length,
            effective_timeout);
        if (!HalXferOk(hi2c, status)) {
            statistics_.failed_transactions++;
            RecoverI2cAfterError(hi2c);
            result = ConvertHalStatus(status);
            continue;
        }

        PrepareMasterXfer(hi2c);
        status = HAL_I2C_Master_Receive(hi2c, addr_shifted, rx_data, rx_length,
                                        effective_timeout);
        if (HalXferOk(hi2c, status)) {
            statistics_.total_transactions++;
            statistics_.successful_transactions++;
            statistics_.bytes_written += tx_length;
            statistics_.bytes_read += rx_length;
            result = hf_i2c_err_t::I2C_SUCCESS;
            break;
        }
        statistics_.failed_transactions++;
        RecoverI2cAfterError(hi2c);
        result = ConvertHalStatus(status);
    }
    parent_bus_->UnlockBus();
    return result;
}

hf_u16_t StmI2cDevice::GetDeviceAddress() const noexcept {
    return config_.device_address;
}

void StmI2cDevice::SetDeviceAddress(hf_u16_t address_7bit) noexcept {
    config_.device_address = address_7bit;
}

hf_u32_t StmI2cDevice::GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept {
    if (requested_ms > 0) return requested_ms;
    if (parent_bus_) return parent_bus_->GetConfig().default_timeout_ms;
    return 1000;  // Fallback
}

hf_i2c_err_t StmI2cDevice::ConvertHalStatus(hf_u32_t hal_status) noexcept {
    auto status = hf::stm32::ToHalStatus(hal_status);
    switch (status) {
        case hf::stm32::HalStatus::OK:      return hf_i2c_err_t::I2C_SUCCESS;
        case hf::stm32::HalStatus::BUSY:    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
        case hf::stm32::HalStatus::TIMEOUT: return hf_i2c_err_t::I2C_ERR_TIMEOUT;
        default:                             return hf_i2c_err_t::I2C_ERR_WRITE_FAILURE;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// StmI2cBus
// ═══════════════════════════════════════════════════════════════════════════════

StmI2cBus::StmI2cBus(const hf_i2c_bus_config_t& config) noexcept
    : config_(config) {}

StmI2cBus::StmI2cBus(I2C_HandleTypeDef* hal_handle, hf_u32_t timeout_ms) noexcept
    : config_(hf_i2c_bus_config_t(hal_handle, timeout_ms)) {}

StmI2cBus::~StmI2cBus() noexcept {
    Deinitialize();
}

bool StmI2cBus::Initialize() noexcept {
    if (initialized_) return true;
    if (!config_.hal_handle) return false;
    // I2C peripheral initialization is done by CubeMX HAL_I2C_Init().
    initialized_ = true;
    return true;
}

bool StmI2cBus::IsInitialized() const noexcept { return initialized_; }

bool StmI2cBus::Deinitialize() noexcept {
    // Deinitialize all devices first
    for (auto& dev : devices_) {
        if (dev) dev->Deinitialize();
    }
    devices_.clear();
    initialized_ = false;
    return true;
}

int StmI2cBus::CreateDevice(const hf_i2c_device_config_t& device_config) noexcept {
    auto dev = std::make_unique<StmI2cDevice>(this, device_config);
    devices_.push_back(std::move(dev));
    return static_cast<int>(devices_.size()) - 1;
}

BaseI2c* StmI2cBus::GetDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

const BaseI2c* StmI2cBus::GetDevice(int device_index) const noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

std::size_t StmI2cBus::GetDeviceCount() const noexcept { return devices_.size(); }

bool StmI2cBus::RemoveDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return false;
    devices_[static_cast<std::size_t>(device_index)]->Deinitialize();
    devices_.erase(devices_.begin() + device_index);
    return true;
}

const hf_i2c_bus_config_t& StmI2cBus::GetConfig() const noexcept { return config_; }

I2C_HandleTypeDef* StmI2cBus::GetHalHandle() const noexcept { return config_.hal_handle; }

bool StmI2cBus::LockBus(hf_u32_t timeout_ms) noexcept {
    return bus_mutex_.try_lock_for(timeout_ms > 0 ? timeout_ms : 1000U);
}

void StmI2cBus::UnlockBus() noexcept {
    bus_mutex_.unlock();
}

#endif  // module enabled
