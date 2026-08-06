/**
 * @file StmI2c.h
 * @brief STM32 I2C Bus+Device wrapper — wraps STM32 HAL I2C via CubeMX handle.
 *
 * Bus/Device architecture matching the ESP32 pattern:
 * - StmI2cBus: wraps one I2C_HandleTypeDef*, manages device collection
 * - StmI2cDevice: inherits BaseI2c, delegates to parent bus HAL handle
 *
 * @section Usage
 * @code
 * extern I2C_HandleTypeDef hi2c1;
 *
 * hf_i2c_bus_config_t bus_cfg(&hi2c1);
 * StmI2cBus i2cBus(bus_cfg);
 * i2cBus.Initialize();
 *
 * hf_i2c_device_config_t dev_cfg;
 * dev_cfg.device_address = 0x68;  // MPU6050
 * int idx = i2cBus.CreateDevice(dev_cfg);
 * BaseI2c* mpu = i2cBus.GetDevice(idx);
 * mpu->Initialize();
 *
 * uint8_t who_am_i = 0;
 * mpu->ReadRegister(0x75, who_am_i);
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseI2c.h"
#include "PlatformMutex.h"
#include "StmTypes.h"
#include <vector>
#include <memory>

class StmI2cBus;

/**
 * @brief STM32 I2C device — inherits BaseI2c, delegates I/O to parent bus HAL handle.
 *
 * Each device represents one slave address on the bus. The parent bus manages
 * the I2C_HandleTypeDef* and provides the HAL layer.
 */
class StmI2cDevice : public BaseI2c {
public:
    StmI2cDevice(StmI2cBus* parent, const hf_i2c_device_config_t& config) noexcept;
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

    /// @brief Update the 7-bit slave address (e.g. after PCAL strap probe).
    void SetDeviceAddress(hf_u16_t address_7bit) noexcept;

    /// @brief Get the device configuration
    const hf_i2c_device_config_t& GetConfig() const noexcept { return config_; }

    /// @brief Get the parent bus
    StmI2cBus* GetParentBus() const noexcept { return parent_bus_; }

private:
    /// @brief Get effective timeout (device override or bus default)
    hf_u32_t GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept;

    /// @brief Convert HAL status to I2C error
    static hf_i2c_err_t ConvertHalStatus(hf_u32_t hal_status) noexcept;

    StmI2cBus*           parent_bus_;  ///< Parent bus reference
    hf_i2c_device_config_t config_;    ///< Device configuration
};

/**
 * @brief STM32 I2C bus — manages the HAL handle and device collection.
 *
 * One bus instance per I2C peripheral (I2C1, I2C2, etc.). Devices are
 * created via CreateDevice() and accessed via GetDevice().
 *
 * Transfers are serialized with @ref PlatformMutex (same contract as StmSpi).
 * Direct HAL probes (IsDeviceReady) must take @ref LockBus or risk racing
 * Mem_Read/Master_Transmit on the same handle.
 *
 * @note Portenta CM4 flying-wire lessons (parity with @c StmSpi):
 *       - Do **not** call @c HAL_I2C_IsDeviceReady from @c Initialize — on
 *         STM32H7 it can latch ISR.TXIS while State==READY and poison the
 *         next Mem_Read (command byte flushed into TXDR, never on the wire).
 *       - Prefer @c Master_Transmit + @c Master_Receive (or WriteRead) over
 *         Mem_Read when the slave pointer auto-increments (PCA/PCAL).
 *       - FreeRTOS stacks/heaps live in FMC SDRAM — stage small RX/TX through
 *         D1 AXI scratch before HAL calls (byte loads from SDRAM stacks are
 *         unreliable on this FMC setup).
 *       - Flush sticky TXIS/TXE before a new transfer (@c FlushTxdr).
 *       Mid Carrier I2C0 = MCU I2C3 PH7/PH8 — see ADR-0021.
 */
class StmI2cBus {
public:
    explicit StmI2cBus(const hf_i2c_bus_config_t& config) noexcept;

    /// @brief Convenience: construct directly from HAL handle
    explicit StmI2cBus(I2C_HandleTypeDef* hal_handle, hf_u32_t timeout_ms = 1000) noexcept;

    ~StmI2cBus() noexcept;

    bool Initialize() noexcept;
    bool IsInitialized() const noexcept;
    bool Deinitialize() noexcept;

    /// @brief Create a device on this bus and return its index
    int CreateDevice(const hf_i2c_device_config_t& device_config) noexcept;

    /// @brief Get device by index (returns nullptr if invalid)
    BaseI2c* GetDevice(int device_index) noexcept;
    const BaseI2c* GetDevice(int device_index) const noexcept;

    /// @brief Get number of devices on this bus
    std::size_t GetDeviceCount() const noexcept;

    /// @brief Remove a device by index
    bool RemoveDevice(int device_index) noexcept;

    /// @brief Get the bus configuration
    const hf_i2c_bus_config_t& GetConfig() const noexcept;

    /// @brief Get the STM32 HAL I2C handle
    I2C_HandleTypeDef* GetHalHandle() const noexcept;

    /**
     * @brief Exclusive bus lock for HAL transfer / probe (StmSpi parity).
     * @return false on timeout / lock failure.
     */
    bool LockBus(hf_u32_t timeout_ms) noexcept;
    void UnlockBus() noexcept;

private:
    friend class StmI2cDevice;

    hf_i2c_bus_config_t config_;
    bool initialized_{false};
    mutable PlatformMutex bus_mutex_{};
    std::vector<std::unique_ptr<StmI2cDevice>> devices_;
};
