/**
 * @file StmI2c.h
 * @brief STM32 I2C stub implementation — placeholder for future STM32 HAL integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseI2c.h"
#include "PlatformMutex.h"
#include <vector>
#include <memory>

class StmI2cBus;

/**
 * @brief STM32 I2C device — stub implementation.
 */
class StmI2cDevice : public BaseI2c {
public:
    StmI2cDevice(StmI2cBus* parent, const hf_i2c_device_config_t& config);
    ~StmI2cDevice() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;

    hf_i2c_err_t Write(const hf_u8_t* data, hf_u16_t length,
                       hf_u32_t timeout_ms = 0) noexcept override;
    hf_i2c_err_t Read(hf_u8_t* data, hf_u16_t length,
                      hf_u32_t timeout_ms = 0) noexcept override;
    hf_i2c_err_t WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length,
                           hf_u8_t* rx_data, hf_u16_t rx_length,
                           hf_u32_t timeout_ms = 0) noexcept override;
    hf_u16_t GetDeviceAddress() const noexcept override;

private:
    StmI2cBus* parent_bus_;
    hf_i2c_device_config_t config_;
};

/**
 * @brief STM32 I2C bus — stub implementation.
 */
class StmI2cBus {
public:
    explicit StmI2cBus(const hf_i2c_bus_config_t& config) noexcept;
    ~StmI2cBus() noexcept;

    bool Initialize() noexcept;
    bool IsInitialized() const noexcept;
    bool Deinitialize() noexcept;
    int CreateDevice(const hf_i2c_device_config_t& device_config) noexcept;
    BaseI2c* GetDevice(int device_index) noexcept;
    const BaseI2c* GetDevice(int device_index) const noexcept;
    std::size_t GetDeviceCount() const noexcept;
    bool RemoveDevice(int device_index) noexcept;
    const hf_i2c_bus_config_t& GetConfig() const noexcept;

private:
    hf_i2c_bus_config_t config_;
    bool initialized_{false};
    std::vector<std::unique_ptr<StmI2cDevice>> devices_;
};
