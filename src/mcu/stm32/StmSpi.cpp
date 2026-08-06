/**
 * @file StmSpi.cpp
 * @brief STM32 SPI wrapper implementation — full STM32 HAL integration.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmSpi.h"

#include <cstring>

namespace {
/* D1 AXI .bss staging for small transfers. Callers on CM4 may pass FMC SDRAM
 * stack pointers; HAL_SPI_* byte FIFO access has the same ldrb class of hazard
 * as I2C on this Portenta partition (see StmI2c / flying-wire bench notes). */
constexpr hf_u16_t kSpiScratchBytes = 32;
uint8_t g_spi_axi_tx[kSpiScratchBytes]{};
uint8_t g_spi_axi_rx[kSpiScratchBytes]{};

#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
/** Drop stale RX FIFO bytes / EOT flags before a soft-CS frame (I2C TXIS class). */
void PrepareSpiXfer(SPI_HandleTypeDef* hspi) noexcept {
    if (hspi == nullptr || hspi->Instance == nullptr) {
        return;
    }
    while ((__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_RXWNE) != RESET) ||
           ((hspi->Instance->SR & SPI_SR_RXPLVL) != 0UL)) {
        (void)*(__IO uint8_t*)&hspi->Instance->RXDR;
    }
    __HAL_SPI_CLEAR_EOTFLAG(hspi);
    __HAL_SPI_CLEAR_TXTFFLAG(hspi);
    if (hspi->State != HAL_SPI_STATE_READY) {
        (void)HAL_SPI_Abort(hspi);
    }
}

/**
 * Soft-CS setup/hold: datasheet tCSS/tCSH ≥ 50 ns; ~1 µs (1–2 bit-times @
 * 1–1.25 MHz). Longer pad-capture delays stay in opt-in wire-proof only.
 */
void CsSetupDelay() noexcept {
    for (uint32_t spin = 40U; spin > 0U; --spin) {
        __asm__ volatile("");
    }
}
#else
void PrepareSpiXfer(SPI_HandleTypeDef*) noexcept {}
void CsSetupDelay() noexcept {}
#endif
}  // namespace

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

    const hf_u8_t* tx_ptr = tx_data;
    hf_u8_t* rx_ptr = rx_data;
    if (length <= kSpiScratchBytes) {
        if (tx_data != nullptr) {
            std::memcpy(g_spi_axi_tx, tx_data, length);
            tx_ptr = g_spi_axi_tx;
        }
        if (rx_data != nullptr) {
            rx_ptr = g_spi_axi_rx;
        }
    }

    PrepareSpiXfer(hspi);
    AssertCS();
    CsSetupDelay();

    uint32_t status;
    if (tx_ptr && rx_ptr) {
        status = HAL_SPI_TransmitReceive(
            hspi, const_cast<uint8_t*>(tx_ptr), rx_ptr, length, effective_timeout);
    } else if (tx_ptr) {
        status = HAL_SPI_Transmit(hspi, const_cast<uint8_t*>(tx_ptr), length,
                                  effective_timeout);
    } else if (rx_ptr) {
        status = HAL_SPI_Receive(hspi, rx_ptr, length, effective_timeout);
    } else {
        DeassertCS();
        parent_bus_->UnlockBus();
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }

    /* Hold CS briefly after last clock (TLE/MAX tCSH) before release. */
    CsSetupDelay();
    DeassertCS();
    parent_bus_->UnlockBus();

    auto result = ConvertHalStatus(status);
    if (result == hf_spi_err_t::SPI_SUCCESS) {
        if (rx_data != nullptr && rx_ptr == g_spi_axi_rx &&
            length <= kSpiScratchBytes) {
            std::memcpy(rx_data, g_spi_axi_rx, length);
        }
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
#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
    /* Soft-CS buses (SPI2 actuators): disable HW NSS pulse so multi-byte
     * frames under GPIO CS are one continuous Motorola word stream. */
    SPI_HandleTypeDef* hspi = config_.hal_handle;
    if (hspi != nullptr && hspi->Instance != nullptr) {
        CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
        CLEAR_BIT(hspi->Instance->CFG2, SPI_CFG2_SSOM);
        hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
        SET_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
    }
#endif
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
