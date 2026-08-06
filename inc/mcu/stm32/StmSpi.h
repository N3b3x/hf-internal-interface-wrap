/**
 * @file StmSpi.h
 * @brief STM32 SPI Bus+Device wrapper — wraps STM32 HAL SPI via CubeMX handle.
 *
 * Bus/Device architecture:
 * - StmSpiBus: wraps one SPI_HandleTypeDef*, manages device collection
 * - StmSpiDevice: inherits BaseSpi, manages CS pin, delegates SPI I/O to bus
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
 * sensor->Transfer(&tx, &rx, 1);
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
#include <vector>
#include <memory>

class StmSpiBus;

/**
 * @brief Soft-CS SPI slave on a shared @ref StmSpiBus.
 *
 * Owns one GPIO chip-select and a per-device SPI mode
 * (@ref hf_spi_device_config_t::mode). Each @ref Transfer locks the parent bus,
 * applies this device's CPOL/CPHA, asserts CS with datasheet setup/hold, then
 * runs the HAL full-duplex/half-duplex transaction. Mixed Mode 0/1/3 slaves
 * may share one CubeMX SPI peripheral.
 */
class StmSpiDevice : public BaseSpi {
public:
    StmSpiDevice(StmSpiBus* parent, const hf_spi_device_config_t& config) noexcept;
    ~StmSpiDevice() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;

    /**
     * @brief Full-duplex (or TX-/RX-only) transfer under this device's CS.
     * @param tx_data Transmit buffer, or nullptr for RX-only.
     * @param rx_data Receive buffer, or nullptr for TX-only.
     * @param length  Byte count (both directions when both buffers set).
     * @param timeout_ms 0 → bus default timeout.
     */
    hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                          hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept override;

    const void* GetDeviceConfig() const noexcept override;

    /// @brief Device configuration (CS port/pin, mode, clock hint).
    const hf_spi_device_config_t& GetConfig() const noexcept { return config_; }

private:
    /// @brief Drive CS to the active level (active-low → GPIO low).
    void AssertCS() noexcept;

    /// @brief Drive CS to the idle level (active-low → GPIO high).
    void DeassertCS() noexcept;

    /// @brief Resolve @p requested_ms against the parent bus default.
    hf_u32_t GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept;

    /// @brief Map STM32 HAL status codes to @ref hf_spi_err_t.
    static hf_spi_err_t ConvertHalStatus(hf_u32_t hal_status) noexcept;

    StmSpiBus*            parent_bus_;  ///< Parent bus reference
    hf_spi_device_config_t config_;     ///< Device configuration
};

/**
 * @brief One MCU SPI peripheral + its soft-CS device collection.
 *
 * Applies per-device SPI mode on transfer so soft-CS multi-slave buses
 * (e.g. MAX Mode 0 + TLE Mode 1) work despite a single CubeMX SPI Init block.
 * Transfers are serialized with @ref PlatformMutex. Opt-in bench register-poke
 * paths must not run concurrently with handler Transfers.
 *
 * @note Soft-CS multi-slave contract (this MCU binding):
 *       - Stage short payloads through D1 AXI when the caller buffer may sit
 *         in FMC SDRAM (byte-access hazard class shared with @c StmI2c).
 *       - Flush RX FIFO + clear EOT/TXTF before each soft-CS frame.
 *       - GPIO CS setup/hold meets slave datasheet tCSS/tCSH (≥1 bit-time).
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
     * @note Caller must hold the bus lock (Transfer does this).
     */
    bool ApplyDeviceMode(hf_stm32_spi_mode_t mode) noexcept;

    /**
     * @brief Exclusive bus lock for mode + CS + HAL transfer.
     * @return false on timeout / lock failure.
     */
    bool LockBus(hf_u32_t timeout_ms) noexcept;
    void UnlockBus() noexcept;

private:
    friend class StmSpiDevice;

    hf_spi_bus_config_t config_;
    bool initialized_{false};
    bool mode_applied_{false};
    hf_stm32_spi_mode_t last_mode_{hf_stm32_spi_mode_t::MODE_0};
    mutable PlatformMutex bus_mutex_{};
    std::vector<std::unique_ptr<StmSpiDevice>> devices_;
};
