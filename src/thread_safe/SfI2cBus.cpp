/**
 * @file SfI2cBus.cpp
 * @brief Implementation of the SfI2cBus class.
 *
 * This file provides the implementation for thread-safe I2C bus operations
 * using mutex protection for multi-threaded environments. The implementation
 * supports master mode operations including read, write, and combined transactions
 * with configurable timeouts, bus locking mechanisms, and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "SfI2cBus.h"

static const char *TAG = "SfI2cBus";

SfI2cBus::SfI2cBus(std::unique_ptr<BaseI2c> i2c_impl) noexcept
    : i2c_bus_(std::move(i2c_impl)), busMutex_(), initialized_(false) {}

SfI2cBus::~SfI2cBus() noexcept {
  if (initialized_) {
    Close();
  }
}

bool SfI2cBus::Open() noexcept {
  if (initialized_)
    return true;
  if (!i2c_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = i2c_bus_->Open();
  return initialized_;
}

bool SfI2cBus::Close() noexcept {
  if (!initialized_)
    return true;
  if (!i2c_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = !i2c_bus_->Close();
  return !initialized_;
}

bool SfI2cBus::Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
                     uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !i2c_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return i2c_bus_->Write(addr, data, sizeBytes, timeoutMsec);
}

bool SfI2cBus::Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes,
                    uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !i2c_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return i2c_bus_->Read(addr, data, sizeBytes, timeoutMsec);
}

bool SfI2cBus::WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes, uint8_t *rxData,
                         uint16_t rxSizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !i2c_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return i2c_bus_->WriteRead(addr, txData, txSizeBytes, rxData, rxSizeBytes, timeoutMsec);
}

bool SfI2cBus::LockBus(uint32_t timeoutMsec) noexcept {
  if (!initialized_)
    return false;
  return busMutex_.Take(timeoutMsec);
}

bool SfI2cBus::UnlockBus() noexcept {
  if (!initialized_)
    return false;
  busMutex_.Give();
  return true;
}

uint32_t SfI2cBus::GetClockHz() const noexcept {
  if (!i2c_bus_)
    return 0;
  return i2c_bus_->GetClockHz();
}
