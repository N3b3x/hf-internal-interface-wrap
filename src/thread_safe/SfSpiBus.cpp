/**
 * @file SfSpiBus.cpp
 * @brief Implementation of the SfSpiBus class for ESP32 (ESP-IDF), providing SPI master communication with thread safety and software-controlled CS.
 *
 * This class abstracts the ESP-IDF SPI master driver and provides thread-safe
 * SPI transactions using FreeRTOS mutexes. It allows for software-controlled
 * chip select (CS) pin, supporting multi-device SPI buses with full-duplex
 * communication, configurable transfer parameters, and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "SfSpiBus.h"
#include <cstring>

static const char *TAG = "SfSpiBus";

SfSpiBus::SfSpiBus(std::unique_ptr<BaseSpi> spi_impl) noexcept
    : spi_bus_(std::move(spi_impl)), busMutex_(), initialized_(false) {}

SfSpiBus::~SfSpiBus() noexcept {
  if (initialized_) {
    Close();
  }
}

bool SfSpiBus::Open() noexcept {
  if (initialized_)
    return true;
  if (!spi_bus_)
    return false;
  MutexLockGuard lock(busMutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = spi_bus_->Open();
  return initialized_;
}

bool SfSpiBus::Close() noexcept {
  if (!initialized_)
    return true;
  if (!spi_bus_)
    return false;
  MutexLockGuard lock(busMutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = !spi_bus_->Close();
  return !initialized_;
}

bool SfSpiBus::Write(const uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !spi_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return spi_bus_->Write(data, sizeBytes, timeoutMsec);
}

bool SfSpiBus::Read(uint8_t *data, uint16_t sizeBytes, uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !spi_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return spi_bus_->Read(data, sizeBytes, timeoutMsec);
}

bool SfSpiBus::WriteRead(const uint8_t *write_data, uint8_t *read_data, uint16_t sizeBytes,
                         uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !spi_bus_)
    return false;
  RtosMutex::LockGuard lock(busMutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return spi_bus_->WriteRead(write_data, read_data, sizeBytes, timeoutMsec);
}

bool SfSpiBus::LockBus(uint32_t timeoutMsec) noexcept {
  if (!initialized_)
    return false;
  return busMutex_.Take(timeoutMsec);
}

bool SfSpiBus::UnlockBus() noexcept {
  if (!initialized_)
    return false;
  busMutex_.Give();
  return true;
}

uint32_t SfSpiBus::GetClockHz() const noexcept {
  if (!spi_bus_)
    return 0;
  return spi_bus_->GetClockHz();
}

bool SfSpiBus::SelectDevice() noexcept {
  if (!spi_bus_)
    return false;
  return spi_bus_->SetChipSelect(true) == HfSpiErr::SPI_SUCCESS;
}

bool SfSpiBus::DeselectDevice() noexcept {
  if (!spi_bus_)
    return false;
  return spi_bus_->SetChipSelect(false) == HfSpiErr::SPI_SUCCESS;
}
