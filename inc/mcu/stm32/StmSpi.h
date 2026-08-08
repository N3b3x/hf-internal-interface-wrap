/**
 * @file StmSpi.h
 * @brief STM32 SPI Bus+Device wrapper — wraps STM32 HAL SPI via CubeMX handle.
 *
 * Bus/Device architecture:
 * - StmSpiBus: wraps one SPI_HandleTypeDef*, manages device collection
 * - StmSpiDevice: inherits BaseSpi, manages CS pin, delegates SPI I/O to bus
 * - StmSpiBusLock / SoftChipSelectGuard: RAII for bus mutex and soft-CS
 *
 * @section Usage
 * @code
 * extern SPI_HandleTypeDef hspi1;
 *
 * hf_spi_bus_config_t bus_cfg(&hspi1);
 * StmSpiBus spiBus(bus_cfg);
 * spiBus.Initialize();
 *
 * hf_spi_device_config_t dev_cfg;
 * dev_cfg.cs_port = GPIOA;
 * dev_cfg.cs_pin  = GPIO_PIN_4;
 * int idx = spiBus.CreateDevice(dev_cfg);
 * BaseSpi* sensor = spiBus.GetDevice(idx);
 * sensor->Initialize();
 *
 * uint8_t tx = 0x80, rx = 0;
 * sensor->Transfer(&tx, &rx, 1);  // bus lock + SoftCs RAII inside
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseSpi.h"
#include "StmTypes.h"
#include "PlatformMutex.h"
#include "SoftChipSelectGuard.h"
#include <vector>
#include <memory>

class StmSpiBus;

/**
 * @brief RAII exclusive ownership of one @ref StmSpiBus (RTOS mutex).
 *
 * Higher-level threads may block here with a timeout — that is intentional.
 * Do not disable interrupts across SPI frames; serialize with this lock.
 * Direct HAL probes / wire-proof paths must take the same lock.
 */
class StmSpiBusLock final {
public:
    StmSpiBusLock(StmSpiBus& bus, hf_u32_t timeout_ms) noexcept;
    ~StmSpiBusLock() noexcept;

    StmSpiBusLock(const StmSpiBusLock&) = delete;
    StmSpiBusLock& operator=(const StmSpiBusLock&) = delete;
    StmSpiBusLock(StmSpiBusLock&& other) noexcept;
    StmSpiBusLock& operator=(StmSpiBusLock&& other) noexcept;

    [[nodiscard]] bool OwnsLock() const noexcept { return locked_; }

private:
    StmSpiBus* bus_{nullptr};
    bool locked_{false};
};

/**
 * @brief Soft-CS SPI slave on a shared @ref StmSpiBus.
 *
 * Owns one GPIO chip-select and a per-device SPI mode
 * (@ref hf_spi_device_config_t::mode). Each @ref Transfer:
 *   1. Takes @ref StmSpiBusLock (RTOS mutex — peers wait),
 *   2. Parks **all** soft-CS on the bus idle-high,
 *   3. Applies this device's CPOL/CPHA (SPE toggle only with every CS high),
 *   4. Flushes the RX FIFO,
 *   5. Scopes **only this** CS with @ref SoftChipSelectGuard,
 *   6. Releases the bus lock.
 *
 * Mixed Mode 0/1/3 slaves may share one CubeMX SPI peripheral. Chip handlers
 * and external drivers must not poke CS — only @ref Transfer / @ref TransferChain.
 *
 * @note CS low during a Mode1 (TLE) frame is expected. CS must be high for
 *       every other slave while Mode0/1/3 SPE/CPOL is being rewritten.
 */
class StmSpiDevice : public BaseSpi {
public:
    StmSpiDevice(StmSpiBus* parent, const hf_spi_device_config_t& config) noexcept;
    ~StmSpiDevice() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;

    /**
     * @brief One soft-CS frame: bus lock + mode + FIFO flush + CS RAII + HAL.
     * @param tx_data Transmit buffer, or nullptr for RX-only.
     * @param rx_data Receive buffer, or nullptr for TX-only.
     * @param length  Byte count (both directions when both buffers set).
     * @param timeout_ms 0 → bus default timeout (bus-mutex wait).
     */
    hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                          hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept override;

    /**
     * @brief N soft-CS frames under one bus lock (CS still toggles per frame).
     * @details Holds @ref StmSpiBusLock across the chain so Mode0/1/3 peers
     *          cannot interleave mid-pipeline (e.g. TLE command + dummy).
     *          Each frame uses its own @ref SoftChipSelectGuard — matching
     *          datasheets that require CSN rise between 32-bit accesses.
     */
    hf_spi_err_t TransferChain(const hf_u8_t* const* tx_frames, hf_u8_t* const* rx_frames,
                               hf_u16_t frame_length, hf_u16_t frame_count,
                               hf_u32_t inter_frame_gap_us,
                               hf_u32_t timeout_ms = 0) noexcept override;

    const void* GetDeviceConfig() const noexcept override;

    /// @brief Force soft-CS idle-high (shared-bus peer park before Mode1 identity).
    void IdleChipSelect() noexcept override;

    /// @brief Bus-locked Saleae-visible soft-CS hold (peers idle-high).
    void HoldChipSelectMs(hf_u32_t ms) noexcept override;

    /// @brief Set CPOL/CPHA for the next transfer (0=Mode0 … 3=Mode3).
    bool SetTransferMode(hf_u8_t mode_0_to_3) noexcept override;

    /// @brief Set MOSI↔MISO IOSwap for the next transfer (CFG2 IOSWP).
    bool SetIoSwap(bool enable) noexcept override;

    /// @brief Device configuration (CS port/pin, mode, clock hint).
    const hf_spi_device_config_t& GetConfig() const noexcept { return config_; }

private:
    friend class StmSpiBus;

    /// @brief Drive CS to the active level (active-low → GPIO low).
    void AssertCS() noexcept;

    /// @brief Drive CS to the idle level (active-low → GPIO high).
    void DeassertCS() noexcept;

    /// @brief One CS-framed HAL transfer; caller must hold @ref StmSpiBusLock.
    hf_spi_err_t TransferLocked(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                                hf_u16_t length, hf_u32_t effective_timeout) noexcept;

    /// @brief Resolve @p requested_ms against the parent bus default.
    hf_u32_t GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept;

    /// @brief Map STM32 HAL status codes to @ref hf_spi_err_t.
    static hf_spi_err_t ConvertHalStatus(hf_u32_t hal_status) noexcept;

    StmSpiBus*            parent_bus_;  ///< Parent bus reference
    hf_spi_device_config_t config_;     ///< Device configuration
    bool                  io_swap_{false}; ///< CFG2 IOSWP for next transfer
};

/**
 * @brief One MCU SPI peripheral + its soft-CS device collection.
 *
 * Applies per-device SPI mode on transfer so soft-CS multi-slave buses
 * (e.g. MAX Mode 0 + TLE Mode 1) work despite a single CubeMX SPI Init block.
 * Transfers are serialized with @ref PlatformMutex via @ref StmSpiBusLock.
 * Opt-in bench register-poke paths must take the same lock.
 *
 * @note Soft-CS multi-slave contract (this MCU binding):
 *       - Stage short payloads through D1 AXI when the caller buffer may sit
 *         in FMC SDRAM (byte-access hazard class shared with @c StmI2c).
 *       - Flush RX FIFO + clear EOT/TXTF before each soft-CS frame (and after
 *         HAL errors so the next peer does not see stale bytes).
 *       - CS is SoftChipSelectGuard-scoped per frame; software GPIO already
 *         exceeds datasheet tCSS — short settle only.
 *       - Clear HW NSS-pulse (@c SSOM) so multi-byte transfers are one
 *         continuous stream under the asserted CS line.
 */
class StmSpiBus {
public:
    explicit StmSpiBus(const hf_spi_bus_config_t& config) noexcept;

    /// @brief Convenience: construct directly from a CubeMX HAL handle.
    explicit StmSpiBus(SPI_HandleTypeDef* hal_handle, hf_u32_t timeout_ms = 1000) noexcept;

    ~StmSpiBus() noexcept;

    bool Initialize() noexcept;
    bool IsInitialized() const noexcept;
    bool Deinitialize() noexcept;

    /**
     * @brief Register a soft-CS device on this bus.
     * @return Device index ≥ 0, or negative on failure.
     */
    int CreateDevice(const hf_spi_device_config_t& device_config) noexcept;
    BaseSpi* GetDevice(int device_index) noexcept;
    const BaseSpi* GetDevice(int device_index) const noexcept;
    std::size_t GetDeviceCount() const noexcept;
    bool RemoveDevice(int device_index) noexcept;
    const hf_spi_bus_config_t& GetConfig() const noexcept;

    /// @brief Underlying CubeMX SPI handle (platform wiring only).
    SPI_HandleTypeDef* GetHalHandle() const noexcept;

    /**
     * @brief Apply @p mode (CPOL/CPHA) to the HAL peripheral if it differs
     *        from the last applied mode. Safe to call with SPE briefly cleared.
     * @note Caller must hold @ref StmSpiBusLock and have every soft-CS idle
     *       (see @ref IdleAllChipSelects) — SPE toggle must not clock a selected slave.
     */
    bool ApplyDeviceMode(hf_stm32_spi_mode_t mode, bool io_swap = false) noexcept;

    /**
     * @brief Drive every registered soft-CS to idle (active-low → HIGH).
     * @note Caller must hold @ref StmSpiBusLock. Called before every mode change.
     */
    void IdleAllChipSelects() noexcept;

    /**
     * @brief Bring-up probe of the shared MISO/SDO net — no scope required.
     *
     * Retargets @p miso_port bit @p miso_pin_pos to a plain input, samples it
     * with the internal pull-up and then the pull-down — first with **every**
     * soft-CS idle, then with only @p device selected — and restores the
     * original MODER/PUPDR. A slave that drives SDO overpowers the ~40 kΩ
     * internal pull; a high-Z net simply follows it.
     *
     * @return 0 if the probe could not run, otherwise bit7 plus:
     *         bit0 = CS idle + pull-up reads HIGH,
     *         bit1 = CS idle + pull-down reads HIGH,
     *         bit2 = @p device selected + pull-up reads HIGH,
     *         bit3 = @p device selected + pull-down reads HIGH.
     *
     * Decode: `0x81` idle high-Z and driven LOW when selected — slave is alive
     * and answering zeros. `0x85` high-Z even when selected — slave never
     * drives SDO (unpowered, held in reset, or MISO not landed). `0x80` held
     * LOW with all CS high — short to GND or a peer driving the shared net.
     *
     * @note Takes @ref StmSpiBusLock internally; do not call while holding it.
     */
    hf_u8_t ProbeMisoLine(void* miso_port, hf_u8_t miso_pin_pos,
                          const BaseSpi* device) noexcept;

    /**
     * @brief Exclusive bus lock for mode + FIFO + CS + HAL transfer.
     * @return false on timeout / lock failure.
     * @note Prefer @ref StmSpiBusLock RAII over raw Lock/Unlock pairs.
     */
    bool LockBus(hf_u32_t timeout_ms) noexcept;
    void UnlockBus() noexcept;

private:
    friend class StmSpiDevice;
    friend class StmSpiBusLock;

    hf_spi_bus_config_t config_;
    bool initialized_{false};
    bool mode_applied_{false};
    hf_stm32_spi_mode_t last_mode_{hf_stm32_spi_mode_t::MODE_0};
    bool last_io_swap_{false};
    mutable PlatformMutex bus_mutex_{};
    std::vector<std::unique_ptr<StmSpiDevice>> devices_;
};
