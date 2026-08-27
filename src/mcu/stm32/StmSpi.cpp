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
/* Small-transfer staging lives per-bus (StmSpiBus::ScratchTx/Rx) so it is
 * covered by the bus lock. A process-wide scratch here raced whenever two SPI
 * buses transferred concurrently (SensorAcquisition ADS pump vs actuator SPI2):
 * peer bytes were spliced into in-flight frames — e.g. TMC9660 TMCL value
 * bytes ORed/overwritten and TLE "sticky-zero" register reads. */

/**
 * @brief Copy @p len bytes from @p src into internal-SRAM @p dst via aligned
 *        32-bit loads (safe when @p src may be external RAM / task stack).
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
 * @brief Publish internal-SRAM @p src into @p dst via aligned 32-bit RMW
 *        (safe when @p dst may be external RAM / task stack).
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
 * frame and after a failed HAL transfer so Mode0/1/3 peers never see stale bytes.
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

/* Busy-wait timebase for soft-CS setup/hold and inter-frame gaps.
 *
 * These delays used to be a fixed 240-iteration spin documented as "≈1 µs @
 * 240 MHz". The loop body is three instructions, so on the 240 MHz CM4 it runs
 * in ~3 µs, and every caller was silently paying 3× its stated budget:
 * InterFrameGapUs(30) after a CPOL rewrite cost ~120 µs against 25.6 µs of
 * actual wire time for a 32-bit frame. A TLE FB_DC/FB_I_AVG sweep is 23 frames,
 * which is how a telemetry read turned into a 4.3 ms InnerControl step against
 * a 2 ms deadline.
 *
 * DWT CYCCNT is already enabled by the board layer for WCET timing, so derive
 * the delay from SystemCoreClock rather than trusting a calibrated loop. The
 * spin loop stays as a fallback for cores/builds where CYCCNT does not count
 * (no debug block, or TRCENA refused), because under-settling a Mode1 slave
 * after a peer SPE/CPOL rewrite yields empty or bit-shifted MISO. */
enum class SpiDelaySource : uint8_t { Unknown, CycleCounter, SpinLoop };
SpiDelaySource g_delay_source = SpiDelaySource::Unknown;
uint32_t g_cycles_per_us = 1U;

void ResolveDelaySource() noexcept {
    if (g_delay_source != SpiDelaySource::Unknown) {
        return;
    }
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    const uint32_t before = DWT->CYCCNT;
    for (uint32_t i = 0; i < 8U; ++i) {
        __asm__ volatile("");
    }
    const uint32_t hz = SystemCoreClock;
    if (DWT->CYCCNT != before && hz >= 1000000U) {
        g_cycles_per_us = hz / 1000000U;
        g_delay_source = SpiDelaySource::CycleCounter;
    } else {
        g_delay_source = SpiDelaySource::SpinLoop;
    }
}

/** Busy-wait @p gap_us microseconds. Accurate to one CYCCNT tick when available. */
void InterFrameGapUs(uint32_t gap_us) noexcept {
    if (gap_us == 0U) {
        return;
    }
    ResolveDelaySource();
    if (g_delay_source == SpiDelaySource::CycleCounter) {
        const uint32_t target = gap_us * g_cycles_per_us;
        const uint32_t start = DWT->CYCCNT;
        while ((DWT->CYCCNT - start) < target) {
            __asm__ volatile("");
        }
        return;
    }
    /* ~80 iterations ≈ 1 µs at 240 MHz on the three-instruction fallback loop. */
    for (uint32_t us = gap_us; us > 0U; --us) {
        for (uint32_t spin = 80U; spin > 0U; --spin) {
            __asm__ volatile("");
        }
    }
}

/**
 * Soft-CS setup/hold after GPIO assert / before deassert (datasheet tCSS/tCSH
 * ≥ 50 ns; 1 µs is ample margin on carrier lead lengths).
 */
void CsEdgeSettle() noexcept { InterFrameGapUs(1U); }
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
     * still AF/input (or never claimed), CS stayed high forever while peers
     * on other pins still transferred. Claim the pin here. */
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
    /* Bring-up retries hold the bus often — wait longer than a typical Mode1
     * Init burst so console `spi cs-pulse` can actually assert soft-CS. */
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
                                           hf_u16_t length, hf_u32_t effective_timeout,
                                           bool park_mode3_after,
                                           bool bus_already_armed) noexcept {
    SPI_HandleTypeDef* hspi = parent_bus_->GetHalHandle();
    if (!hspi) return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;

    if (tx_data == nullptr && rx_data == nullptr) {
        return hf_spi_err_t::SPI_ERR_INVALID_PARAMETER;
    }

    if (bus_already_armed) {
        /* Mid-chain frame: the first frame already parked every peer CS and
         * programmed CPOL/CPHA/MBR, and the bus lock has not been released
         * since. Only the SPE clear is still required, because HAL's CR2.TSIZE
         * write is illegal while the peripheral is enabled. */
        CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
    } else {
        /* Shared-bus soft-CS contract (mixed Mode0/1/3 peers on one SPI):
         * IdleAll → ApplyMode → assert ONLY this CS. Never rewrite SPE/CPOL
         * while any peer CS is low (that slave would see phantom clocks). */
        DeassertCS();
        parent_bus_->IdleAllChipSelects();

        bool mode_reconfigured = true;
        if (!parent_bus_->ApplyDeviceMode(config_.mode, io_swap_,
                                          config_.inter_data_idle_cycles,
                                          config_.clock_speed_hz,
                                          &mode_reconfigured)) {
            return hf_spi_err_t::SPI_ERR_NOT_INITIALIZED;
        }
        /* A CPOL/CPHA rewrite can glitch SCK; wait with all CS high so Mode1
         * does not see a phantom edge (1-bit-early MISO). Mode0/1 need the
         * longer idle-LOW settle after a Mode3 peer left CPOL=1 on the shared
         * SCK net. When ApplyDeviceMode found the peripheral already
         * configured it wrote nothing but the SPE clear, and AFCNTR holds the
         * pads at the idle level — there is no edge to settle. */
        const bool cpol0 = (config_.mode == hf_stm32_spi_mode_t::MODE_0 ||
                            config_.mode == hf_stm32_spi_mode_t::MODE_1);
        if (!mode_reconfigured) {
            CsEdgeSettle();
        } else if (cpol0) {
            InterFrameGapUs(30U);
        } else {
            CsEdgeSettle();
            CsEdgeSettle();
            CsEdgeSettle();
        }
    }

    const hf_u8_t* tx_ptr = tx_data;
    hf_u8_t* rx_ptr = rx_data;
    uint8_t* const scratch_tx = parent_bus_->ScratchTx();
    uint8_t* const scratch_rx = parent_bus_->ScratchRx();
    if (length <= StmSpiBus::kScratchBytes) {
        if (tx_data != nullptr) {
            CopyFromMaybeFmcToAxi(scratch_tx, tx_data, length);
            tx_ptr = scratch_tx;
        }
        if (rx_data != nullptr) {
            rx_ptr = scratch_rx;
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
            if (rx_data != nullptr && rx_ptr == scratch_rx &&
                length <= StmSpiBus::kScratchBytes) {
                CopyFromAxiToMaybeFmc(rx_data, scratch_rx, length);
            }
            statistics_.total_transactions++;
            statistics_.successful_transactions++;
            statistics_.total_bytes_sent += length;
            statistics_.total_bytes_received += length;
        }
    }

    /* After Mode2/3 (CPOL=1) park at Mode0 idle-LOW for Mode0/1 peers — but NOT
     * between frames of a Mode3 TransferChain. TMC9660 SPI TMCL is cmd + NO_OP;
     * Mode0 SPE rewrite between those halves yielded garbage TMCL status
     * (HIL: OPENLOOP_VOLTAGE tmcl_st=56, no PWM). Mode0/1 peers still
     * ApplyDeviceMode on their next Transfer. */
    if (park_mode3_after &&
        (config_.mode == hf_stm32_spi_mode_t::MODE_2 ||
         config_.mode == hf_stm32_spi_mode_t::MODE_3)) {
        parent_bus_->IdleAllChipSelects();
        /* Park CPOL=0 for Mode0/1 peers. MIDI=15 is the Cube/MAX/TMC default;
         * TLE ApplyDeviceMode restores MIDI=0 on its next Transfer. */
        (void)parent_bus_->ApplyDeviceMode(hf_stm32_spi_mode_t::MODE_0, false,
                                           /*midi_cycles=*/15);
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
    return TransferLocked(tx_data, rx_data, length, effective_timeout,
                          /*park_mode3_after=*/false);
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
        const bool last = (i + 1U) >= frame_count;
        /* Frame 0 arms the bus (IdleAll → mode); the rest reuse it under the
         * same lock. Defer Mode3→Mode0 park until the last frame so pipelined
         * Mode3 protocols stay coherent. */
        result = TransferLocked(tx, rx, frame_length, effective_timeout,
                                /*park_mode3_after=*/last,
                                /*bus_already_armed=*/i != 0U);
        if (result != hf_spi_err_t::SPI_SUCCESS) {
            break;
        }
        if (inter_frame_gap_us > 0U && !last) {
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
        /* SPE stays off — HAL_SPI_* enables it after writing CR2.TSIZE. */
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
 * ~40 kΩ pull charge long-lead capacitance, then sample IDR.
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
    /* Probe must not assert a Mode1 peer CS while the bus is still in
     * Mode3 (CPOL=1) from a prior transfer — that looks like "CS low, clock
     * high" on a logic analyzer and is not a valid Mode1 frame. */
    (void)ApplyDeviceMode(hf_stm32_spi_mode_t::MODE_1, false);
    InterFrameGapUs(30U);

    /* Only MODER moves — AFR/OSPEEDR keep the CubeMX SPI MISO setup so the
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

hf_u8_t StmSpiBus::ResolveBaudMbr(hf_u32_t kernel_hz, hf_u32_t requested_hz) noexcept {
    /* MBR field n divides the kernel clock by 2^(n+1): 0 → /2 … 7 → /256.
     * Pick the fastest divisor that still lands at or below the request, so a
     * device never sees a clock above the rate its datasheet allows. */
    if (kernel_hz == 0U || requested_hz == 0U) {
        return 7U;
    }
    for (hf_u8_t mbr = 0U; mbr < 7U; ++mbr) {
        if ((kernel_hz >> (mbr + 1U)) <= requested_hz) {
            return mbr;
        }
    }
    return 7U;
}

hf_u32_t StmSpiBus::GetKernelClockHz() const noexcept {
#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
    if (config_.hal_handle == nullptr || config_.hal_handle->Instance == nullptr) {
        return 0U;
    }
    const SPI_TypeDef* inst = config_.hal_handle->Instance;
    uint32_t periph = 0U;
    if (inst == SPI1 || inst == SPI2 || inst == SPI3) {
        periph = RCC_PERIPHCLK_SPI123;
    } else if (inst == SPI4 || inst == SPI5) {
        periph = RCC_PERIPHCLK_SPI45;
    } else if (inst == SPI6) {
        periph = RCC_PERIPHCLK_SPI6;
    } else {
        return 0U;
    }
    return static_cast<hf_u32_t>(HAL_RCCEx_GetPeriphCLKFreq(periph));
#else
    return 0U;
#endif
}

hf_u32_t StmSpiBus::GetEffectiveClockHz() const noexcept {
#if defined(USE_HAL_DRIVER) && defined(HAL_SPI_MODULE_ENABLED)
    const hf_u32_t kernel = GetKernelClockHz();
    if (kernel == 0U || config_.hal_handle == nullptr ||
        config_.hal_handle->Instance == nullptr) {
        return 0U;
    }
    const uint32_t mbr =
        (READ_REG(config_.hal_handle->Instance->CFG1) & SPI_CFG1_MBR) >>
        SPI_CFG1_MBR_Pos;
    return kernel >> (mbr + 1U);
#else
    return 0U;
#endif
}

bool StmSpiBus::ApplyDeviceMode(hf_stm32_spi_mode_t mode, bool io_swap,
                                hf_u8_t midi_cycles,
                                hf_u32_t clock_speed_hz,
                                bool* reconfigured) noexcept {
    if (reconfigured != nullptr) {
        *reconfigured = true;
    }
    SPI_HandleTypeDef* hspi = config_.hal_handle;
    if (!hspi) return false;
    if (midi_cycles > 15U) midi_cycles = 15U;

#if defined(USE_HAL_DRIVER)
    /* Resolve the per-device prescaler before the SPE toggle below; CFG1.MBR
     * may only be written while SPE is clear, which is exactly the window this
     * function already opens for CPOL/CPHA. A device that asks for 0 keeps
     * whatever prescaler is programmed. */
    hf_u8_t want_mbr = last_baud_mbr_;
    bool set_baud = false;
    if (clock_speed_hz != 0U) {
        const hf_u32_t kernel = GetKernelClockHz();
        if (kernel != 0U) {
            want_mbr = ResolveBaudMbr(kernel, clock_speed_hz);
            set_baud = true;
        }
    }

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
    const uint32_t midi = (static_cast<uint32_t>(midi_cycles) << SPI_CFG2_MIDI_Pos) &
                          SPI_CFG2_MIDI;
    constexpr uint32_t kCfg2Fields =
        SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_IOSWP | SPI_CFG2_MIDI;

    /* Skip SPE toggle only when HW CFG2 (and CFG1.MBR, when the device asked
     * for a specific clock) already matches. Do not trust last_mode_ alone —
     * PW_SPI_BENCH_WIRE_PROOF (and any peer) may poke CFG2 without updating
     * this cache. */
    if (mode_applied_ && last_mode_ == mode && last_io_swap_ == io_swap &&
        last_midi_cycles_ == midi_cycles &&
        (!set_baud || last_baud_mbr_ == want_mbr)) {
        const uint32_t cfg2 = READ_REG(hspi->Instance->CFG2);
        const uint32_t want = (cpol | cpha | ioswp | midi) & kCfg2Fields;
        const bool baud_ok =
            !set_baud || ((READ_REG(hspi->Instance->CFG1) & SPI_CFG1_MBR) ==
                          (static_cast<uint32_t>(want_mbr) << SPI_CFG1_MBR_Pos));
        if ((cfg2 & kCfg2Fields) == want && baud_ok) {
            /* Same invariant as the reconfigure path below: hand the peripheral
             * to HAL disabled so its CR2.TSIZE write is legal. Clearing SPE with
             * CFG2.AFCNTR=1 keeps the pads driven at the CPOL idle level, so
             * nothing on the shared SCK/MOSI net moves and the caller can skip
             * the post-mode settle. */
            CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
            if (reconfigured != nullptr) {
                *reconfigured = false;
            }
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
    hspi->Init.MasterInterDataIdleness = midi;
    CLEAR_BIT(hspi->Instance->CFG2, SPI_CFG2_SSOM);
    SET_BIT(hspi->Instance->CFG2, SPI_CFG2_AFCNTR); /* KeepIOState across SPE */
    MODIFY_REG(hspi->Instance->CFG2, kCfg2Fields, cpol | cpha | ioswp | midi);
    if (set_baud) {
        /* SPE is clear here (RM0399 §"Baud rate control" requires it). Keep
         * hspi->Init in sync so a later HAL_SPI_Init cannot silently revert to
         * the CubeMX prescaler; the HAL macro value is exactly MBR << 28. */
        MODIFY_REG(hspi->Instance->CFG1, SPI_CFG1_MBR,
                   static_cast<uint32_t>(want_mbr) << SPI_CFG1_MBR_Pos);
        hspi->Init.BaudRatePrescaler =
            static_cast<uint32_t>(want_mbr) << SPI_CFG1_MBR_Pos;
    }
    /* Leave SPE CLEARED. RM0399 requires CR2.TSIZE to be written while the SPI
     * is disabled, and every HAL_SPI_* polling transfer does exactly that
     * (MODIFY_REG(CR2, TSIZE) → __HAL_SPI_ENABLE → CSTART) after
     * SPI_CloseTransfer() left SPE=0. Re-enabling SPE here made the *first*
     * transfer after any mode change write TSIZE with SPE=1: the master never
     * clocked and HAL returned HAL_TIMEOUT, whose close path cleared SPE so the
     * *next* frame worked. On a single-mode bus this never triggered; once the
     * TLE (Mode1) began periodic traffic, every TMC (Mode3) TMCL frame changed
     * mode and frame 1 always died — the "spins only at random times" fault.
     * AFCNTR=1 keeps the pads driven at the CPOL idle level while SPE=0, so the
     * shared SCK net still parks correctly for Mode0/1 peers. */

    /* Prove CFG2 (and the prescaler) stuck before any soft-CS assert. */
    const uint32_t cfg2 = READ_REG(hspi->Instance->CFG2);
    const uint32_t want = (cpol | cpha | ioswp | midi) & kCfg2Fields;
    if ((cfg2 & kCfg2Fields) != want) {
        mode_applied_ = false;
        return false;
    }
    if (set_baud &&
        (READ_REG(hspi->Instance->CFG1) & SPI_CFG1_MBR) !=
            (static_cast<uint32_t>(want_mbr) << SPI_CFG1_MBR_Pos)) {
        mode_applied_ = false;
        return false;
    }

    /* Drive idle level (Mode0/1 = LOW) before soft-CS — avoids a float-high
     * gap from the SPE toggle that a logic analyzer labels as CPOL=1. */
    for (volatile uint32_t spin = 400U; spin > 0U; --spin) {
    }
    if (set_baud) {
        last_baud_mbr_ = want_mbr;
    }
#else
    (void)hspi;
    (void)io_swap;
    (void)midi_cycles;
    (void)clock_speed_hz;
#endif

    last_mode_ = mode;
    last_io_swap_ = io_swap;
    last_midi_cycles_ = midi_cycles;
    mode_applied_ = true;
    return true;
}
