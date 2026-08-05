/**
 * @file StmSpi.cpp
 * @brief STM32 SPI wrapper implementation — full STM32 HAL integration.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmSpi.h"

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

#if !defined(USE_HAL_DRIVER)
extern "C" {
extern uint32_t HAL_SPI_TransmitReceive(SPI_HandleTypeDef* hspi, uint8_t* pTxData,
                                        uint8_t* pRxData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_SPI_Transmit(SPI_HandleTypeDef* hspi, uint8_t* pData,
                                 uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_SPI_Receive(SPI_HandleTypeDef* hspi, uint8_t* pData,
                                uint16_t Size, uint32_t Timeout);
extern void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState);
}
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// StmSpiDevice
// ═══════════════════════════════════════════════════════════════════════════════

StmSpiDevice::StmSpiDevice(StmSpiBus* parent, const hf_spi_device_config_t& config) noexcept
    : parent_bus_(parent), config_(config) {}

StmSpiDevice::~StmSpiDevice() noexcept {
    if (initialized_) Deinitialize();
}

bool StmSpiDevice::Initialize() noexcept {
    if (initialized_) return true;
    if (!parent_bus_ || !parent_bus_->IsInitialized()) return false;

    // Ensure CS pin starts deasserted
    if (config_.cs_port && config_.cs_pin != 0) {
        DeassertCS();
    }

    initialized_ = true;
    return true;
}

bool StmSpiDevice::Deinitialize() noexcept {
    if (config_.cs_port && config_.cs_pin != 0) {
        DeassertCS();
    }
    initialized_ = false;
    return true;
}

hf_spi_err_t StmSpiDevice::Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                                     hf_u16_t length, hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    if (length == 0) return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;

    SPI_HandleTypeDef* hspi = parent_bus_->GetHalHandle();
    if (!hspi) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    if (!parent_bus_->LockBus(effective_timeout)) {
        return hf_spi_err_t::SPI_ERR_BUS_BUSY;
    }

    /* Per-device mode (MAX Mode0 / TLE Mode1 / TMC Mode3 on shared SPI2). */
    if (!parent_bus_->ApplyDeviceMode(config_.mode)) {
        parent_bus_->UnlockBus();
        return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    }

    // Assert chip select
    AssertCS();

    uint32_t status;

    if (tx_data && rx_data) {
        // Full duplex transfer
        status = HAL_SPI_TransmitReceive(
            hspi, const_cast<uint8_t*>(tx_data), rx_data, length, effective_timeout);
    } else if (tx_data) {
        // Transmit only
        status = HAL_SPI_Transmit(hspi, const_cast<uint8_t*>(tx_data), length, effective_timeout);
    } else if (rx_data) {
        // Receive only
        status = HAL_SPI_Receive(hspi, rx_data, length, effective_timeout);
    } else {
        DeassertCS();
        parent_bus_->UnlockBus();
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }

    // Deassert chip select
    DeassertCS();
    parent_bus_->UnlockBus();

    auto result = ConvertHalStatus(status);
    if (result == hf_spi_err_t::SPI_SUCCESS) {
        statistics_.total_transactions++;
        statistics_.successful_transactions++;
        statistics_.total_bytes_sent += length;
        statistics_.total_bytes_received += length;
    } else {
        statistics_.failed_transactions++;
    }
    return result;
}

const void* StmSpiDevice::GetDeviceConfig() const noexcept {
    return &config_;
}

void StmSpiDevice::AssertCS() noexcept {
    if (!config_.cs_port || config_.cs_pin == 0) return;
    // Active low: assert = drive LOW; Active high: assert = drive HIGH
#if defined(USE_HAL_DRIVER)
    HAL_GPIO_WritePin(config_.cs_port, config_.cs_pin,
                      config_.cs_active_low ? GPIO_PIN_RESET : GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(config_.cs_port, config_.cs_pin, config_.cs_active_low ? 0U : 1U);
#endif
}

void StmSpiDevice::DeassertCS() noexcept {
    if (!config_.cs_port || config_.cs_pin == 0) return;
#if defined(USE_HAL_DRIVER)
    HAL_GPIO_WritePin(config_.cs_port, config_.cs_pin,
                      config_.cs_active_low ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
    HAL_GPIO_WritePin(config_.cs_port, config_.cs_pin, config_.cs_active_low ? 1U : 0U);
#endif
}

hf_u32_t StmSpiDevice::GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept {
    if (requested_ms > 0) return requested_ms;
    if (parent_bus_) return parent_bus_->GetConfig().default_timeout_ms;
    return 1000;
}

hf_spi_err_t StmSpiDevice::ConvertHalStatus(hf_u32_t hal_status) noexcept {
    auto status = hf::stm32::ToHalStatus(hal_status);
    switch (status) {
        case hf::stm32::HalStatus::OK:      return hf_spi_err_t::SPI_SUCCESS;
        case hf::stm32::HalStatus::BUSY:    return hf_spi_err_t::SPI_ERR_BUS_BUSY;
        case hf::stm32::HalStatus::TIMEOUT: return hf_spi_err_t::SPI_ERR_BUS_TIMEOUT;
        default:                             return hf_spi_err_t::SPI_ERR_TRANSFER_FAILED;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// StmSpiBus
// ═══════════════════════════════════════════════════════════════════════════════

StmSpiBus::StmSpiBus(const hf_spi_bus_config_t& config) noexcept
    : config_(config) {}

StmSpiBus::StmSpiBus(SPI_HandleTypeDef* hal_handle, hf_u32_t timeout_ms) noexcept
    : config_(hf_spi_bus_config_t(hal_handle, timeout_ms)) {}

StmSpiBus::~StmSpiBus() noexcept {
    Deinitialize();
}

bool StmSpiBus::Initialize() noexcept {
    if (initialized_) return true;
    if (!config_.hal_handle) return false;
    initialized_ = true;
    return true;
}

bool StmSpiBus::IsInitialized() const noexcept { return initialized_; }

bool StmSpiBus::Deinitialize() noexcept {
    for (auto& dev : devices_) {
        if (dev) dev->Deinitialize();
    }
    devices_.clear();
    initialized_ = false;
    return true;
}

int StmSpiBus::CreateDevice(const hf_spi_device_config_t& device_config) noexcept {
    auto dev = std::make_unique<StmSpiDevice>(this, device_config);
    devices_.push_back(std::move(dev));
    return static_cast<int>(devices_.size()) - 1;
}

BaseSpi* StmSpiBus::GetDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

const BaseSpi* StmSpiBus::GetDevice(int device_index) const noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

std::size_t StmSpiBus::GetDeviceCount() const noexcept { return devices_.size(); }

bool StmSpiBus::RemoveDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return false;
    devices_[static_cast<std::size_t>(device_index)]->Deinitialize();
    devices_.erase(devices_.begin() + device_index);
    return true;
}

const hf_spi_bus_config_t& StmSpiBus::GetConfig() const noexcept { return config_; }

SPI_HandleTypeDef* StmSpiBus::GetHalHandle() const noexcept { return config_.hal_handle; }

bool StmSpiBus::LockBus(hf_u32_t timeout_ms) noexcept {
    return bus_mutex_.try_lock_for(timeout_ms > 0 ? timeout_ms : 1000U);
}

void StmSpiBus::UnlockBus() noexcept {
    bus_mutex_.unlock();
}

bool StmSpiBus::ApplyDeviceMode(hf_stm32_spi_mode_t mode) noexcept {
    SPI_HandleTypeDef* hspi = config_.hal_handle;
    if (!hspi) return false;

#if defined(USE_HAL_DRIVER)
    uint32_t cpol = SPI_POLARITY_LOW;
    uint32_t cpha = SPI_PHASE_1EDGE;
    switch (mode) {
        case hf_stm32_spi_mode_t::MODE_0:
            cpol = SPI_POLARITY_LOW;
            cpha = SPI_PHASE_1EDGE;
            break;
        case hf_stm32_spi_mode_t::MODE_1:
            cpol = SPI_POLARITY_LOW;
            cpha = SPI_PHASE_2EDGE;
            break;
        case hf_stm32_spi_mode_t::MODE_2:
            cpol = SPI_POLARITY_HIGH;
            cpha = SPI_PHASE_1EDGE;
            break;
        case hf_stm32_spi_mode_t::MODE_3:
            cpol = SPI_POLARITY_HIGH;
            cpha = SPI_PHASE_2EDGE;
            break;
        default:
            return false;
    }

    /* Skip SPE toggle only when HW CFG2 already matches. Do not trust
     * last_mode_ alone — PW_SPI_BENCH_WIRE_PROOF (and any peer) may poke
     * CFG2 without updating this cache. */
    if (mode_applied_ && last_mode_ == mode) {
        const uint32_t cfg2 = READ_REG(hspi->Instance->CFG2);
        if ((cfg2 & (SPI_CFG2_CPOL | SPI_CFG2_CPHA)) == (cpol | cpha)) {
            return true;
        }
    }

    /* CFG2 CPOL/CPHA may only change while SPE is cleared. */
    CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
    hspi->Init.CLKPolarity = cpol;
    hspi->Init.CLKPhase = cpha;
    MODIFY_REG(hspi->Instance->CFG2, SPI_CFG2_CPOL | SPI_CFG2_CPHA, cpol | cpha);
    SET_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
#else
    (void)hspi;
#endif

    last_mode_ = mode;
    mode_applied_ = true;
    return true;
}
