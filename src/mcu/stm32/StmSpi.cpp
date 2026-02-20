/**
 * @file StmSpi.cpp
 * @brief STM32 SPI stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmSpi.h"

// ── StmSpiDevice ─────────────────────────────────────────────────────────

StmSpiDevice::StmSpiDevice(const hf_spi_device_config_t& config) noexcept
    : config_(config) {}

StmSpiDevice::~StmSpiDevice() noexcept = default;

bool StmSpiDevice::Initialize() noexcept { return false; }
bool StmSpiDevice::Deinitialize() noexcept { return false; }

hf_spi_err_t StmSpiDevice::Transfer(const hf_u8_t* /*tx_data*/, hf_u8_t* /*rx_data*/,
                                     hf_u16_t /*length*/, hf_u32_t /*timeout_ms*/) noexcept {
    return hf_spi_err_t::SPI_ERR_UNSUPPORTED_OPERATION;
}

const void* StmSpiDevice::GetDeviceConfig() const noexcept {
    return &config_;
}

// ── StmSpiBus ────────────────────────────────────────────────────────────

StmSpiBus::StmSpiBus(const hf_spi_bus_config_t& config) noexcept
    : config_(config) {}

StmSpiBus::~StmSpiBus() noexcept = default;

bool StmSpiBus::Initialize() noexcept { return false; }
bool StmSpiBus::IsInitialized() const noexcept { return initialized_; }
bool StmSpiBus::Deinitialize() noexcept { initialized_ = false; return true; }

int StmSpiBus::CreateDevice(const hf_spi_device_config_t& device_config) noexcept {
    auto dev = std::make_unique<StmSpiDevice>(device_config);
    devices_.push_back(std::move(dev));
    return static_cast<int>(devices_.size()) - 1;
}

BaseSpi* StmSpiBus::GetDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

const BaseSpi* StmSpiBus::GetDevice(int device_index) const noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return nullptr;
    return devices_[static_cast<std::size_t>(device_index)].get();
}

std::size_t StmSpiBus::GetDeviceCount() const noexcept { return devices_.size(); }

bool StmSpiBus::RemoveDevice(int device_index) noexcept {
    if (device_index < 0 || static_cast<std::size_t>(device_index) >= devices_.size()) return false;
    devices_.erase(devices_.begin() + device_index);
    return true;
}

const hf_spi_bus_config_t& StmSpiBus::GetConfig() const noexcept { return config_; }
