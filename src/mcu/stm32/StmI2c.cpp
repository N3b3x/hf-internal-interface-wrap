/**
 * @file StmI2c.cpp
 * @brief STM32 I2C wrapper implementation — full STM32 HAL integration.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmI2c.h"

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                        uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                       uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                  uint16_t MemAddress, uint16_t MemAddSize,
                                  uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                 uint16_t MemAddress, uint16_t MemAddSize,
                                 uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                      uint32_t Trials, uint32_t Timeout);
}

// ═══════════════════════════════════════════════════════════════════════════════
// StmI2cDevice
// ═══════════════════════════════════════════════════════════════════════════════

StmI2cDevice::StmI2cDevice(StmI2cBus* parent, const hf_i2c_device_config_t& config) noexcept
    : parent_bus_(parent), config_(config) {}

StmI2cDevice::~StmI2cDevice() noexcept {
    if (initialized_) Deinitialize();
}

bool StmI2cDevice::Initialize() noexcept {
    if (initialized_) return true;
    if (!parent_bus_ || !parent_bus_->IsInitialized()) return false;

    // Optionally probe the device to verify it's present
    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    if (!hi2c) return false;

    uint16_t addr_shifted = config_.device_address << 1;
    uint32_t status = HAL_I2C_IsDeviceReady(hi2c, addr_shifted, 3,
                                            GetEffectiveTimeout(0));
    if (!hf::stm32::IsHalOk(status)) {
        // Device not responding — still allow init (device may power up later)
        // Don't fail init, just note it.
    }

    initialized_ = true;
    return true;
}

bool StmI2cDevice::Deinitialize() noexcept {
    initialized_ = false;
    return true;
}

hf_i2c_err_t StmI2cDevice::Write(const hf_u8_t* data, hf_u16_t length,
                                  hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;
    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);

    uint32_t status = HAL_I2C_Master_Transmit(
        hi2c, addr_shifted,
        const_cast<uint8_t*>(data), length, effective_timeout);

    auto result = ConvertHalStatus(status);
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        statistics_.total_writes++;
        statistics_.bytes_written += length;
    } else {
        statistics_.error_count++;
    }
    return result;
}

hf_i2c_err_t StmI2cDevice::Read(hf_u8_t* data, hf_u16_t length,
                                 hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;
    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);

    uint32_t status = HAL_I2C_Master_Receive(
        hi2c, addr_shifted, data, length, effective_timeout);

    auto result = ConvertHalStatus(status);
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        statistics_.total_reads++;
        statistics_.bytes_read += length;
    } else {
        statistics_.error_count++;
    }
    return result;
}

hf_i2c_err_t StmI2cDevice::WriteRead(const hf_u8_t* tx_data, hf_u16_t tx_length,
                                      hf_u8_t* rx_data, hf_u16_t rx_length,
                                      hf_u32_t timeout_ms) noexcept {
    if (!EnsureInitialized()) return hf_i2c_err_t::I2C_ERR_NOT_INITIALIZED;
    if (!tx_data || tx_length == 0 || !rx_data || rx_length == 0) {
        return hf_i2c_err_t::I2C_ERR_INVALID_PARAMETER;
    }

    I2C_HandleTypeDef* hi2c = parent_bus_->GetHalHandle();
    uint16_t addr_shifted = config_.device_address << 1;
    hf_u32_t effective_timeout = GetEffectiveTimeout(timeout_ms);

    // If tx_length == 1, treat as register address read via HAL_I2C_Mem_Read
    // This is the common pattern: write register addr, then read data
    if (tx_length == 1) {
        uint32_t status = HAL_I2C_Mem_Read(
            hi2c, addr_shifted,
            tx_data[0], 1,  // MemAddress, MemAddSize=1 byte
            rx_data, rx_length, effective_timeout);

        auto result = ConvertHalStatus(status);
        if (result == hf_i2c_err_t::I2C_SUCCESS) {
            statistics_.total_reads++;
            statistics_.total_writes++;
            statistics_.bytes_read += rx_length;
            statistics_.bytes_written += tx_length;
        } else {
            statistics_.error_count++;
        }
        return result;
    }

    // For multi-byte writes followed by read: use separate transmit + receive
    auto write_err = Write(tx_data, tx_length, timeout_ms);
    if (write_err != hf_i2c_err_t::I2C_SUCCESS) return write_err;

    return Read(rx_data, rx_length, timeout_ms);
}

hf_u16_t StmI2cDevice::GetDeviceAddress() const noexcept {
    return config_.device_address;
}

hf_u32_t StmI2cDevice::GetEffectiveTimeout(hf_u32_t requested_ms) const noexcept {
    if (requested_ms > 0) return requested_ms;
    if (parent_bus_) return parent_bus_->GetConfig().default_timeout_ms;
    return 1000;  // Fallback
}

hf_i2c_err_t StmI2cDevice::ConvertHalStatus(hf_u32_t hal_status) noexcept {
    auto status = hf::stm32::ToHalStatus(hal_status);
    switch (status) {
        case hf::stm32::HalStatus::OK:      return hf_i2c_err_t::I2C_SUCCESS;
        case hf::stm32::HalStatus::BUSY:    return hf_i2c_err_t::I2C_ERR_BUS_BUSY;
        case hf::stm32::HalStatus::TIMEOUT: return hf_i2c_err_t::I2C_ERR_TIMEOUT;
        default:                             return hf_i2c_err_t::I2C_ERR_WRITE_FAILED;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// StmI2cBus
// ═══════════════════════════════════════════════════════════════════════════════

StmI2cBus::StmI2cBus(const hf_i2c_bus_config_t& config) noexcept
    : config_(config) {}

StmI2cBus::StmI2cBus(I2C_HandleTypeDef* hal_handle, hf_u32_t timeout_ms) noexcept
    : config_(hf_i2c_bus_config_t(hal_handle, timeout_ms)) {}

StmI2cBus::~StmI2cBus() noexcept {
    Deinitialize();
}

bool StmI2cBus::Initialize() noexcept {
    if (initialized_) return true;
    if (!config_.hal_handle) return false;
    // I2C peripheral initialization is done by CubeMX HAL_I2C_Init().
    initialized_ = true;
    return true;
}

bool StmI2cBus::IsInitialized() const noexcept { return initialized_; }

bool StmI2cBus::Deinitialize() noexcept {
    // Deinitialize all devices first
    for (auto& dev : devices_) {
        if (dev) dev->Deinitialize();
    }
    devices_.clear();
    initialized_ = false;
    return true;
}

int StmI2cBus::CreateDevice(const hf_i2c_device_config_t& device_config) noexcept {
    auto dev = std::make_unique<StmI2cDevice>(this, device_config);
    devices_.push_back(std::move(dev));
    return static_cast<int>(devices_.size()) - 1;
}

BaseI2c* StmI2cBus::GetDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

const BaseI2c* StmI2cBus::GetDevice(int device_index) const noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

std::size_t StmI2cBus::GetDeviceCount() const noexcept { return devices_.size(); }

bool StmI2cBus::RemoveDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size())
        return false;
    devices_[static_cast<std::size_t>(device_index)]->Deinitialize();
    devices_.erase(devices_.begin() + device_index);
    return true;
}

const hf_i2c_bus_config_t& StmI2cBus::GetConfig() const noexcept { return config_; }

I2C_HandleTypeDef* StmI2cBus::GetHalHandle() const noexcept { return config_.hal_handle; }
