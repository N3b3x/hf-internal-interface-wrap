/**
 * @file StmSpi.h
 * @brief STM32 SPI stub implementation — placeholder for future STM32 HAL integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseSpi.h"
#include "PlatformMutex.h"
#include <vector>
#include <memory>

/**
 * @brief STM32 SPI device — stub implementation.
 *
 * Inherits from BaseSpi and returns SPI_ERR_UNSUPPORTED_OPERATION for all
 * operations. Integrate STM32CubeMX-generated HAL SPI init code to activate.
 */
class StmSpiDevice : public BaseSpi {
public:
    explicit StmSpiDevice(const hf_spi_device_config_t& config) noexcept;
    ~StmSpiDevice() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf_spi_err_t Transfer(const hf_u8_t* tx_data, hf_u8_t* rx_data,
                          hf_u16_t length, hf_u32_t timeout_ms = 0) noexcept override;
    const void* GetDeviceConfig() const noexcept override;

private:
    hf_spi_device_config_t config_;
};

/**
 * @brief STM32 SPI bus — stub implementation.
 */
class StmSpiBus {
public:
    explicit StmSpiBus(const hf_spi_bus_config_t& config) noexcept;
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

private:
    hf_spi_bus_config_t config_;
    bool initialized_{false};
    std::vector<std::unique_ptr<StmSpiDevice>> devices_;
};
