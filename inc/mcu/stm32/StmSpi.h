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
 * @brief STM32 SPI device — manages chip-select and delegates transfers to parent bus.
 *
 * Each device has its own CS pin (managed via HAL_GPIO_WritePin) and SPI mode
 * (@ref hf_spi_device_config_t::mode). Before each transfer the parent bus
 * applies that device's CPOL/CPHA so mixed Mode 0/1/3 slaves can share one
 * CubeMX SPI peripheral (ESP32 EspSpiDevice does the same per CS handle).
 */
class StmSpiDevice : public BaseSpi {
public:
    StmSpiDevice(StmSpiBus* parent, const hf_spi_device_config_t& config) noexcept;
    ~StmSpiDevice() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;

    hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                          hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept override;

    const void* GetDeviceConfig() const noexcept override;

    /// @brief Get the device configuration
    const hf_spi_device_config_t& GetConfig() const noexcept { return config_; }

private:
    /// @brief Assert CS (drive low/high depending on active_low flag)
    void AssertCS() noexcept;

    /// @brief Deassert CS
    void DeassertCS() noexcept;

    /// @brief Get effective timeout
    hf_u32_t GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept;

    /// @brief Convert HAL status to SPI error
    static hf_spi_err_t ConvertHalStatus(hf_u32_t hal_status) noexcept;

    StmSpiBus*            parent_bus_;  ///< Parent bus reference
    hf_spi_device_config_t config_;     ///< Device configuration
};

/**
 * @brief STM32 SPI bus — manages the HAL handle and device collection.
 *
 * One bus instance per SPI peripheral (SPI1, SPI2, etc.). Applies per-device
 * SPI mode on transfer so soft-CS multi-slave buses (MAX Mode 0 + TLE Mode 1)
 * work correctly despite a single CubeMX SPI Init block.
 *
 * Transfers are serialized with @ref PlatformMutex (same contract as EspSpi).
 * Bench wire-proof that pokes HAL SPI registers directly must not run
 * concurrently with handler Transfers.
 */
class StmSpiBus {
public:
    explicit StmSpiBus(const hf_spi_bus_config_t& config) noexcept;

    /// @brief Convenience: construct directly from HAL handle
    explicit StmSpiBus(SPI_HandleTypeDef* hal_handle, hf_u32_t timeout_ms = 1000) noexcept;

    ~StmSpiBus() noexcept;

    bool Initialize() noexcept;
    bool IsInitialized() const noexcept;
    bool Deinitialize() noexcept;

    int CreateDevice(const hf_spi_device_config_t& device_config) noexcept;
    BaseSpi* GetDevice(int device_index) noexcept;
    const BaseSpi* GetDevice(int device_index) const noexcept;
    std::size_t GetDeviceCount() const noexcept;
    bool RemoveDevice(int device_index) noexcept;
    const hf_spi_bus_config_t& GetConfig() const noexcept;

    /// @brief Get the STM32 HAL SPI handle
    SPI_HandleTypeDef* GetHalHandle() const noexcept;

    /**
     * @brief Apply @p mode (CPOL/CPHA) to the HAL peripheral if it differs
     *        from the last applied mode. Safe to call with SPE briefly cleared.
     * @note Caller must hold the bus lock (Transfer does this).
     */
    bool ApplyDeviceMode(hf_stm32_spi_mode_t mode) noexcept;

    /**
     * @brief Exclusive bus lock for mode + CS + HAL transfer (EspSpi parity).
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
