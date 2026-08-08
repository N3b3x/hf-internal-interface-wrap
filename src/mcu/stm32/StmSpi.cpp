/**
 * @file StmSpi.cpp
 * @brief STM32 SPI wrapper implementation — full STM32 HAL integration.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmSpi.h"

#include <cstdint>
#include <cstring>

namespace {
/* D1 AXI .bss staging for small transfers. Callers on CM4 may pass FMC SDRAM
 * stack pointers; HAL_SPI_* byte FIFO access has the same ldrb class of hazard
 * as I2C on this Portenta partition (see StmI2c / flying-wire bench notes). */
constexpr hf_u16_t kSpiScratchBytes = 32;
uint8_t g_spi_axi_tx[kSpiScratchBytes]{};
uint8_t g_spi_axi_rx[kSpiScratchBytes]{};

/**
 * Copy @p len bytes from @p src (may be CM4 FMC SDRAM stack) into AXI @p dst
 * using only aligned 32-bit loads. Plain memcpy/ldrb from FMC has returned
 * zeros here while the containing word was correct (ic_diag / Mid-I2C0 class).
 */
void CopyFromMaybeFmcToAxi(uint8_t* dst, const uint8_t* src, hf_u16_t len) noexcept {
    if (dst == nullptr || src == nullptr || len == 0U) {
        return;
    }
    for (hf_u16_t i = 0; i < len; ++i) {
        const auto addr = reinterpret_cast<uintptr_t>(src + i);
        const auto aligned = addr & ~static_cast<uintptr_t>(3U);
        const uint32_t word =
            *reinterpret_cast<const volatile uint32_t*>(aligned);
        const unsigned shift = static_cast<unsigned>((addr & 3U) * 8U);
        dst[i] = static_cast<uint8_t>((word >> shift) & 0xFFU);
    }
}

/**
 * Publish AXI @p src into @p dst (may be FMC SDRAM stack) via aligned
 * 32-bit read-modify-write — avoids STRB lanes that do not stick on this FMC.
 */
void CopyFromAxiToMaybeFmc(uint8_t* dst, const uint8_t* src, hf_u16_t len) noexcept {
    if (dst == nullptr || src == nullptr || len == 0U) {
        return;
    }
    for (hf_u16_t i = 0; i < len; ++i) {
        const auto addr = reinterpret_cast<uintptr_t>(dst + i);
        const auto aligned = addr & ~static_cast<uintptr_t>(3U);
        volatile uint32_t* cell = reinterpret_cast<volatile uint32_t*>(aligned);
        const unsigned shift = static_cast<unsigned>((addr & 3U) * 8U);
        const uint32_t mask = static_cast<uint32_t>(0xFFU) << shift;
        const uint32_t old = *cell;
        *cell = (old & ~mask) | (static_cast<uint32_t>(src[i]) << shift);
    }
}

#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
/**
 * Drain RX FIFO + clear EOT/TXTF. Call under the bus lock before each soft-CS
 * frame and again after a failed HAL transfer so Mode0/1/3 peers never see
 * stale bytes (same hazard class as Mid-I2C0 TXIS).
 */
void FlushSpiFifo(SPI_HandleTypeDef* hspi) noexcept {
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
        while ((__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_RXWNE) != RESET) ||
               ((hspi->Instance->SR & SPI_SR_RXPLVL) != 0UL)) {
            (void)*(__IO uint8_t*)&hspi->Instance->RXDR;
        }
    }
}

/**
 * Soft-CS setup/hold after GPIO assert/before deassert.
 * Datasheet tCSS/tCSH ≥ 50 ns. On CM4 (~240 MHz) ~240 empty spins ≈ 1 µs;
 * Mode1 (TLE) is edge-sensitive — under-settling after MAX Mode0 SPE toggle
 * produced garbage MISO with data=0 while MAX stayed healthy.
 */
void CsEdgeSettle() noexcept {
    for (uint32_t spin = 240U; spin > 0U; --spin) {
        __asm__ volatile("");
    }
}

/** Busy-wait approximately @p gap_us microseconds (CsEdgeSettle ≈ 1 µs). */
void InterFrameGapUs(uint32_t gap_us) noexcept {
    for (uint32_t us = gap_us; us > 0U; --us) {
        CsEdgeSettle();
    }
}
#else
void FlushSpiFifo(SPI_HandleTypeDef*) noexcept {}
void CsEdgeSettle() noexcept {}
void InterFrameGapUs(uint32_t) noexcept {}
#endif
}  // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// StmSpiBusLock
// ═══════════════════════════════════════════════════════════════════════════════

StmSpiBusLock::StmSpiBusLock(StmSpiBus& bus, hf_u32_t timeout_ms) noexcept
    : bus_(&bus), locked_(bus.LockBus(timeout_ms)) {}

StmSpiBusLock::~StmSpiBusLock() noexcept {
    if (locked_ && bus_ != nullptr) {
        bus_->UnlockBus();
        locked_ = false;
    }
}

StmSpiBusLock::StmSpiBusLock(StmSpiBusLock&& other) noexcept
    : bus_(other.bus_), locked_(other.locked_) {
    other.bus_ = nullptr;
    other.locked_ = false;
}

StmSpiBusLock& StmSpiBusLock::operator=(StmSpiBusLock&& other) noexcept {
    if (this != &other) {
        if (locked_ && bus_ != nullptr) {
            bus_->UnlockBus();
        }
        bus_ = other.bus_;
        locked_ = other.locked_;
        other.bus_ = nullptr;
        other.locked_ = false;
    }
    return *this;
}

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

    /* Soft-CS must be a push-pull GPIO output. CubeMX MX_GPIO_Init usually
     * does this, but Transfer previously only WritePin'd — if the ball was
     * still AF/input (or never claimed), Saleae saw CS stuck high forever
     * while MAX on another pin still worked. Claim the pin here. */
    if (config_.cs_port && config_.cs_pin != 0) {
#if defined(USE_HAL_DRIVER)
        GPIO_InitTypeDef gpio = {0};
        gpio.Pin = config_.cs_pin;
        gpio.Mode = GPIO_MODE_OUTPUT_PP;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(config_.cs_port, &gpio);
#endif
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

void StmSpiDevice::IdleChipSelect() noexcept {
    if (!EnsureInitialized()) {
        return;
    }
    DeassertCS();
}

void StmSpiDevice::HoldChipSelectMs(hf_u32_t ms) noexcept {
    if (!EnsureInitialized() || parent_bus_ == nullptr) {
        return;
    }
    if (ms == 0U) {
        ms = 1U;
    }
    if (ms > 50U) {
        ms = 50U; /* keep Step() responsive */
    }
    /* Bring-up retries hold the bus often — wait longer than a TLE Init burst
     * so CDC `spi cs-pulse` actually asserts PI0/PC13/PD4 for Saleae/JTAG. */
    StmSpiBusLock bus_lock(*parent_bus_, 5000U);
    if (!bus_lock.OwnsLock()) {
        return;
    }
    parent_bus_->IdleAllChipSelects();
    AssertCS();
    InterFrameGapUs(ms * 1000U);
    DeassertCS();
}

bool StmSpiDevice::SetTransferMode(hf_u8_t mode_0_to_3) noexcept {
    if (mode_0_to_3 > 3U) {
        return false;
    }
    config_.mode = static_cast<hf_stm32_spi_mode_t>(mode_0_to_3);
    return true;
}

bool StmSpiDevice::SetIoSwap(bool enable) noexcept {
    io_swap_ = enable;
    return true;
}

hf_spi_err_t StmSpiDevice::TransferLocked(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                                           hf_u16_t length,
                                           hf_u32_t effective_timeout) noexcept {
    SPI_HandleTypeDef* hspi = parent_bus_->GetHalHandle();
    if (!hspi) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

    if (tx_data == nullptr && rx_data == nullptr) {
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }

    /* ── Shared-bus soft-CS contract (MAX Mode0 / TLE Mode1 / TMC Mode3) ──
     * Wire-proof does IdleAll → ApplyMode → assert ONE CS. Match that here:
     * SPE/CPOL must never change while any peer CS is low, or that slave sees
     * phantom clocks (Saleae: “TLE CS low during polarity rewrite”).
     * Every frame (including TransferChain dummies) re-runs this so TLE CS
     * (PI0) visibly asserts each 32-bit window — do not skip AssertCS. */
    DeassertCS();
    parent_bus_->IdleAllChipSelects();

    if (!parent_bus_->ApplyDeviceMode(config_.mode, io_swap_)) {
        return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    }
    /* SPE toggle can glitch SCK; wait with all CS high so Mode1 does not see a
     * phantom edge (1-bit-early MISO → ICVID 0xC1xx reads as 0x82xx).
     * Mode0/1 need a longer idle-LOW settle on flying-wire after a Mode3 peer
     * left CPOL=1 on the net (Saleae: TLE CS low while SCK still high). */
    const bool cpol0 = (config_.mode == hf_stm32_spi_mode_t::MODE_0 ||
                        config_.mode == hf_stm32_spi_mode_t::MODE_1);
    if (cpol0) {
        InterFrameGapUs(30U);
    } else {
        CsEdgeSettle();
        CsEdgeSettle();
        CsEdgeSettle();
    }

    const hf_u8_t* tx_ptr = tx_data;
    hf_u8_t* rx_ptr = rx_data;
    if (length <= kSpiScratchBytes) {
        if (tx_data != nullptr) {
            CopyFromMaybeFmcToAxi(g_spi_axi_tx, tx_data, length);
            tx_ptr = g_spi_axi_tx;
        }
        if (rx_data != nullptr) {
            rx_ptr = g_spi_axi_rx;
        }
    }

    FlushSpiFifo(hspi);

    hf_spi_err_t result;
    {
        /* Soft-CS RAII: assert ONLY this device → HAL → deassert (even on error).
         * Peers stay high for the whole frame. */
        auto cs = MakeSoftChipSelectGuard(
            [this]() noexcept {
                AssertCS();
                InterFrameGapUs(5U); /* tCSS margin on long leads */
            },
            [this]() noexcept {
                InterFrameGapUs(5U);
                DeassertCS();
            });

        uint32_t status;
        if (tx_ptr && rx_ptr) {
            status = HAL_SPI_TransmitReceive(
                hspi, const_cast<uint8_t*>(tx_ptr), rx_ptr, length,
                effective_timeout);
        } else if (tx_ptr) {
            status = HAL_SPI_Transmit(hspi, const_cast<uint8_t*>(tx_ptr), length,
                                      effective_timeout);
        } else {
            status = HAL_SPI_Receive(hspi, rx_ptr, length, effective_timeout);
        }

        result = ConvertHalStatus(status);
        if (result != hf_spi_err_t::SPI_SUCCESS) {
            /* Drop junk so the next soft-CS peer (different mode) starts clean. */
            FlushSpiFifo(hspi);
            statistics_.failed_transactions++;
        } else {
            if (rx_data != nullptr && rx_ptr == g_spi_axi_rx &&
                length <= kSpiScratchBytes) {
                CopyFromAxiToMaybeFmc(rx_data, g_spi_axi_rx, length);
            }
            statistics_.total_transactions++;
            statistics_.successful_transactions++;
            statistics_.total_bytes_sent += length;
            statistics_.total_bytes_received += length;
        }
    }

    /* After Mode2/3 (CPOL=1) park the bus at Mode0 idle-LOW so the shared
     * SCK rest level is correct for MAX/TLE. Leaving Mode3 SPE-on is what
     * Saleae reported as "random" CPOL flips between bursts. */
    if (config_.mode == hf_stm32_spi_mode_t::MODE_2 ||
        config_.mode == hf_stm32_spi_mode_t::MODE_3) {
        parent_bus_->IdleAllChipSelects();
        (void)parent_bus_->ApplyDeviceMode(hf_stm32_spi_mode_t::MODE_0, false);
        InterFrameGapUs(10U);
    }
    return result;
}

hf_spi_err_t StmSpiDevice::Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                                     hf_u16_t length, hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    if (length == 0) return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    if (!parent_bus_) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

    const hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    StmSpiBusLock bus_lock(*parent_bus_, effective_timeout);
    if (!bus_lock.OwnsLock()) {
        return hf_spi_err_t::SPI_ERR_BUS_BUSY;
    }
    return TransferLocked(tx_data, rx_data, length, effective_timeout);
}

hf_spi_err_t StmSpiDevice::TransferChain(const hf_u8_t* const* tx_frames,
                                          hf_u8_t* const* rx_frames,
                                          hf_u16_t frame_length, hf_u16_t frame_count,
                                          hf_u32_t inter_frame_gap_us,
                                          hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
    if (frame_count == 0U || frame_length == 0U) {
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }
    if (tx_frames == nullptr && rx_frames == nullptr) {
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }
    if (!parent_bus_) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

    const hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);
    /* One bus ownership window; SoftChipSelectGuard still per frame. */
    StmSpiBusLock bus_lock(*parent_bus_, effective_timeout);
    if (!bus_lock.OwnsLock()) {
        return hf_spi_err_t::SPI_ERR_BUS_BUSY;
    }

    hf_spi_err_t result = hf_spi_err_t::SPI_SUCCESS;
    for (hf_u16_t i = 0; i < frame_count; ++i) {
        const hf_u8_t* tx = (tx_frames != nullptr) ? tx_frames[i] : nullptr;
        hf_u8_t* rx = (rx_frames != nullptr) ? rx_frames[i] : nullptr;
        /* Full IdleAll → mode → AssertCS per frame so Saleae always sees
         * TLE CS (PI0) low for each 32-bit window (command + dummy). */
        result = TransferLocked(tx, rx, frame_length, effective_timeout);
        if (result != hf_spi_err_t::SPI_SUCCESS) {
            break;
        }
        if (inter_frame_gap_us > 0U && (i + 1U) < frame_count) {
            InterFrameGapUs(inter_frame_gap_us);
        }
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

void StmSpiBus::IdleAllChipSelects() noexcept {
    for (auto& dev : devices_) {
        if (dev != nullptr) {
            /* Direct deassert — no EnsureInitialized (avoids re-entry). */
            dev->DeassertCS();
        }
    }
}

namespace {
#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
/**
 * Select @p pupd (01 = pull-up, 10 = pull-down) on one pin, let the internal
 * ~40 kΩ pull charge the flying-wire capacitance, then sample IDR.
 */
bool SampleMisoWithPull(GPIO_TypeDef* port, uint32_t pos, uint32_t pupd) noexcept {
    MODIFY_REG(port->PUPDR, 0x3UL << (pos * 2U), pupd << (pos * 2U));
    InterFrameGapUs(200U);
    return (port->IDR & (0x1UL << pos)) != 0U;
}
#endif
}  // namespace

hf_u8_t StmSpiBus::ProbeMisoLine(void* miso_port, hf_u8_t miso_pin_pos,
                                 const BaseSpi* device) noexcept {
#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
    auto* port = static_cast<GPIO_TypeDef*>(miso_port);
    if (port == nullptr || miso_pin_pos > 15U) {
        return 0U;
    }
    StmSpiBusLock lock(*this, 1000U);
    if (!lock.OwnsLock()) {
        return 0U;
    }

    StmSpiDevice* target = nullptr;
    for (auto& dev : devices_) {
        if (dev != nullptr && static_cast<const BaseSpi*>(dev.get()) == device) {
            target = dev.get();
            break;
        }
    }

    const uint32_t pos = static_cast<uint32_t>(miso_pin_pos);
    const uint32_t field = 0x3UL << (pos * 2U);
    const uint32_t saved_moder = port->MODER & field;
    const uint32_t saved_pupdr = port->PUPDR & field;

    IdleAllChipSelects();
    /* Probe must not assert TLE CS while the bus is still in Mode3 (CPOL=1)
     * from a TMC SPI attempt — that is the Saleae "Enable low, Clock high"
     * pattern and is not a real TLE frame. */
    (void)ApplyDeviceMode(hf_stm32_spi_mode_t::MODE_1, false);
    InterFrameGapUs(30U);

    /* Only MODER moves — AFR/OSPEEDR keep the CubeMX SPI2_MISO setup so the
     * restore below re-arms the peripheral exactly as MspInit left it. */
    MODIFY_REG(port->MODER, field, 0UL);

    hf_u8_t code = 0x80U;
    if (SampleMisoWithPull(port, pos, 0x1UL)) {
        code |= 0x01U;
    }
    if (SampleMisoWithPull(port, pos, 0x2UL)) {
        code |= 0x02U;
    }

    if (target != nullptr) {
        target->AssertCS();
        InterFrameGapUs(5U);
        if (SampleMisoWithPull(port, pos, 0x1UL)) {
            code |= 0x04U;
        }
        if (SampleMisoWithPull(port, pos, 0x2UL)) {
            code |= 0x08U;
        }
        target->DeassertCS();
    }

    MODIFY_REG(port->PUPDR, field, saved_pupdr);
    MODIFY_REG(port->MODER, field, saved_moder);
    return code;
#else
    (void)miso_port;
    (void)miso_pin_pos;
    (void)device;
    return 0U;
#endif
}

bool StmSpiBus::ApplyDeviceMode(hf_stm32_spi_mode_t mode, bool io_swap) noexcept {
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
    const uint32_t ioswp = io_swap ? SPI_IO_SWAP_ENABLE : SPI_IO_SWAP_DISABLE;

    /* Skip SPE toggle only when HW CFG2 already matches. Do not trust
     * last_mode_ alone — PW_SPI_BENCH_WIRE_PROOF (and any peer) may poke
     * CFG2 without updating this cache. */
    if (mode_applied_ && last_mode_ == mode && last_io_swap_ == io_swap) {
        const uint32_t cfg2 = READ_REG(hspi->Instance->CFG2);
        const uint32_t want =
            (cpol | cpha | ioswp) & (SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP);
        if ((cfg2 & (SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP)) == want) {
            return true;
        }
    }

    /* Caller must have IdleAllChipSelects() already — SPE off/on can edge SCK. */
    CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
    hspi->Init.CLKPolarity = cpol;
    hspi->Init.CLKPhase = cpha;
    hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi->Init.IOSwap = ioswp;
    hspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    CLEAR_BIT(hspi->Instance->CFG2, SPI_CFG2_SSOM);
    SET_BIT(hspi->Instance->CFG2, SPI_CFG2_AFCNTR); /* KeepIOState across SPE */
    MODIFY_REG(hspi->Instance->CFG2, SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP,
               cpol | cpha | ioswp);
    SET_BIT(hspi->Instance->CR1, SPI_CR1_SPE);

    /* Prove CFG2 stuck before any soft-CS assert. */
    const uint32_t cfg2 = READ_REG(hspi->Instance->CFG2);
    const uint32_t want =
        (cpol | cpha | ioswp) & (SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP);
    if ((cfg2 & (SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP)) != want) {
        mode_applied_ = false;
        return false;
    }

    /* Drive idle level (Mode0/1 = LOW) before soft-CS — avoids Saleae seeing
     * a float-high gap from the SPE toggle and labeling the burst CPOL=1. */
    for (volatile uint32_t spin = 400U; spin > 0U; --spin) {
    }
#else
    (void)hspi;
    (void)io_swap;
#endif

    last_mode_ = mode;
    last_io_swap_ = io_swap;
    mode_applied_ = true;
    return true;
}
