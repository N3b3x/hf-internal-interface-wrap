/**
 * @file SfFlexCan.cpp
 * @brief Implementation of SfFlexCan.
 */

#include "SfFlexCan.h"

SfFlexCan::SfFlexCan(uint8_t port, uint32_t baudRate,
                     SemaphoreHandle_t mtx) noexcept
    : base(port, baudRate), mutex(mtx), initialized(false) {}

SfFlexCan::~SfFlexCan() noexcept {
  if (initialized)
    Close();
}

bool SfFlexCan::Open() noexcept {
  if (initialized)
    return true;
  if (!base.Open())
    return false;
  initialized = true;
  return true;
}

bool SfFlexCan::Close() noexcept {
  if (!initialized)
    return true;
  base.Close();
  initialized = false;
  return true;
}

bool SfFlexCan::Write(const FlexCan::Frame &frame,
                      uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  bool ok = base.Write(frame);
  xSemaphoreGive(mutex);
  return ok;
}

bool SfFlexCan::Read(FlexCan::Frame &frame, uint32_t timeoutMsec) noexcept {
  if (!initialized)
    return false;
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMsec)) != pdTRUE)
    return false;
  bool ok = base.Read(frame, timeoutMsec);
  xSemaphoreGive(mutex);
  return ok;
}

bool SfFlexCan::Lock(uint32_t timeoutMsec) noexcept {
  return xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMsec)) == pdTRUE;
}

bool SfFlexCan::Unlock() noexcept { return xSemaphoreGive(mutex) == pdTRUE; }
