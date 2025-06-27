/**
 * @file SfUartDriver.cpp
 * @brief Implementation of the SfUartDriver class.
 */

#include "SfUartDriver.h"

static const char *TAG = "SfUartDriver";

SfUartDriver::SfUartDriver(std::unique_ptr<BaseUart> uart_impl) noexcept
    : uart_driver_(std::move(uart_impl)), mutex_(), initialized_(false) {}

SfUartDriver::~SfUartDriver() noexcept {
  if (initialized_) {
    Close();
  }
}

bool SfUartDriver::Open() noexcept {
  if (initialized_)
    return true;
  if (!uart_driver_)
    return false;
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = uart_driver_->Open();
  return initialized_;
}

bool SfUartDriver::Close() noexcept {
  if (!initialized_)
    return true;
  if (!uart_driver_)
    return false;
  RtosMutex::LockGuard lock(mutex_);
  if (!lock.IsLocked())
    return false;
  initialized_ = !uart_driver_->Close();
  return !initialized_;
}

bool SfUartDriver::Write(const uint8_t *data, uint16_t length, uint32_t timeoutMsec) noexcept {
  if (!initialized_ || !uart_driver_)
    return false;
  RtosMutex::LockGuard lock(mutex_, timeoutMsec);
  if (!lock.IsLocked())
    return false;
  return uart_driver_->Write(data, length, timeoutMsec) == HfUartErr::UART_SUCCESS;
}

bool SfUartDriver::Read(uint8_t *data, uint16_t length, TickType_t ticksToWait) noexcept {
  if (!initialized_ || !uart_driver_)
    return false;
  RtosMutex::LockGuard lock(
      mutex_, ticksToWait == portMAX_DELAY ? HF_TIMEOUT_NEVER : (ticksToWait * portTICK_PERIOD_MS));
  if (!lock.IsLocked())
    return false;
  return uart_driver_->Read(data, length, ticksToWait * portTICK_PERIOD_MS) ==
         HfUartErr::UART_SUCCESS;
}

bool SfUartDriver::Lock() noexcept {
  if (!initialized_)
    return false;
  return mutex_.Take(HF_TIMEOUT_NEVER);
}

bool SfUartDriver::Unlock() noexcept {
  if (!initialized_)
    return false;
  mutex_.Give();
  return true;
}
