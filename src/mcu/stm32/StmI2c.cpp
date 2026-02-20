/**
 * @file StmI2c.cpp
 * @brief STM32 I2C stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmI2c.h"

// ── StmI2cDevice ─────────────────────────────────────────────────────────

StmI2cDevice::StmI2cDevice(StmI2cBus* parent, const hf_i2c_device_config_t& config)
    : parent_bus_(parent), config_(config) {}

StmI2cDevice::~StmI2cDevice() noexcept = default;

bool StmI2cDevice::Initialize() noexcept { return false; }
bool StmI2cDevice::Deinitialize() noexcept { return false; }

hf_i2c_err_t StmI2cDevice::Write(const hf_u8_t* /*data*/, hf_u16_t /*length*/,
                                  hf_u32_t /*timeout_ms*/) noexcept {
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
}

hf_i2c_err_t StmI2cDevice::Read(hf_u8_t* /*data*/, hf_u16_t /*length*/,
                                 hf_u32_t /*timeout_ms*/) noexcept {
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
}

hf_i2c_err_t StmI2cDevice::WriteRead(const hf_u8_t* /*tx_data*/, hf_u16_t /*tx_length*/,
                                      hf_u8_t* /*rx_data*/, hf_u16_t /*rx_length*/,
                                      hf_u32_t /*timeout_ms*/) noexcept {
    return hf_i2c_err_t::I2C_ERR_UNSUPPORTED_OPERATION;
}

hf_u16_t StmI2cDevice::GetDeviceAddress() const noexcept {
    return config_.device_address;
}

// ── StmI2cBus ────────────────────────────────────────────────────────────

StmI2cBus::StmI2cBus(const hf_i2c_bus_config_t& config) noexcept
    : config_(config) {}

StmI2cBus::~StmI2cBus() noexcept = default;

bool StmI2cBus::Initialize() noexcept { return false; }
bool StmI2cBus::IsInitialized() const noexcept { return initialized_; }
bool StmI2cBus::Deinitialize() noexcept { initialized_ = false; return true; }

int StmI2cBus::CreateDevice(const hf_i2c_device_config_t& device_config) noexcept {
    auto dev = std::make_unique<StmI2cDevice>(this, device_config);
    devices_.push_back(std::move(dev));
    return static_cast<int>(devices_.size()) - 1;
}

BaseI2c* StmI2cBus::GetDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

const BaseI2c* StmI2cBus::GetDevice(int device_index) const noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

std::size_t StmI2cBus::GetDeviceCount() const noexcept { return devices_.size(); }

bool StmI2cBus::RemoveDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return false;
    devices_.erase(devices_.begin() + device_index);
    return true;
}

const hf_i2c_bus_config_t& StmI2cBus::GetConfig() const noexcept { return config_; }
