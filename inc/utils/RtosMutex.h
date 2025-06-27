#pragma once

#include "../mcu/McuSelect.h"

#ifdef HF_MCU_FAMILY_ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#elif defined(HF_MCU_FAMILY_STM32)
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "task.h"
#elif defined(HF_MCU_FAMILY_RP2040)
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#else
#error                                                                                             \
    "RTOS mutex implementation not available for this MCU platform. Please add support in RtosMutex.h"
#endif

#include <atomic>
#include <cstdint>

class RtosTime {
public:
  static uint64_t GetCurrentTimeUs() noexcept {
    return static_cast<uint64_t>(xTaskGetTickCount()) * 1000 / configTICK_RATE_HZ * 1000;
  }

  static TickType_t MsToTicks(uint32_t ms) noexcept {
    if (ms == 0) {
      return 0;
    }
    const TickType_t ticks = pdMS_TO_TICKS(ms);
    return (ticks > 0) ? ticks : 1;
  }
};

class RtosMutex {
public:
  RtosMutex() noexcept : handle_(xSemaphoreCreateMutex()) {}

  ~RtosMutex() noexcept {
    if (handle_) {
      vSemaphoreDelete(handle_);
    }
  }

  RtosMutex(const RtosMutex &) = delete;
  RtosMutex &operator=(const RtosMutex &) = delete;

  RtosMutex(RtosMutex &&other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  RtosMutex &operator=(RtosMutex &&other) noexcept {
    if (this != &other) {
      if (handle_) {
        vSemaphoreDelete(handle_);
      }
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  bool lock() noexcept {
    if (!handle_)
      return false;
    return xSemaphoreTake(handle_, portMAX_DELAY) == pdTRUE;
  }

  bool try_lock() noexcept {
    if (!handle_)
      return false;
    return xSemaphoreTake(handle_, 0) == pdTRUE;
  }

  bool try_lock_for(uint32_t timeout_ms) noexcept {
    if (!handle_)
      return false;
    const TickType_t ticks = RtosTime::MsToTicks(timeout_ms);
    return xSemaphoreTake(handle_, ticks) == pdTRUE;
  }

  void unlock() noexcept {
    if (handle_) {
      xSemaphoreGive(handle_);
    }
  }

  SemaphoreHandle_t native_handle() const noexcept {
    return handle_;
  }

private:
  SemaphoreHandle_t handle_;
};

class RtosSharedMutex {
public:
  RtosSharedMutex() noexcept : readers_(0), writer_active_(false) {
    writer_mutex_ = xSemaphoreCreateMutex();
    reader_mutex_ = xSemaphoreCreateMutex();
  }

  ~RtosSharedMutex() noexcept {
    if (writer_mutex_ != nullptr) {
      vSemaphoreDelete(writer_mutex_);
    }
    if (reader_mutex_ != nullptr) {
      vSemaphoreDelete(reader_mutex_);
    }
  }

  RtosSharedMutex(const RtosSharedMutex &) = delete;
  RtosSharedMutex &operator=(const RtosSharedMutex &) = delete;

  RtosSharedMutex(RtosSharedMutex &&other) noexcept
      : writer_mutex_(other.writer_mutex_), reader_mutex_(other.reader_mutex_),
        readers_(other.readers_.load()), writer_active_(other.writer_active_.load()) {
    other.writer_mutex_ = nullptr;
    other.reader_mutex_ = nullptr;
    other.readers_.store(0);
    other.writer_active_.store(false);
  }

  RtosSharedMutex &operator=(RtosSharedMutex &&other) noexcept {
    if (this != &other) {
      if (writer_mutex_ != nullptr) {
        vSemaphoreDelete(writer_mutex_);
      }
      if (reader_mutex_ != nullptr) {
        vSemaphoreDelete(reader_mutex_);
      }
      writer_mutex_ = other.writer_mutex_;
      reader_mutex_ = other.reader_mutex_;
      readers_.store(other.readers_.load());
      writer_active_.store(other.writer_active_.load());
      other.writer_mutex_ = nullptr;
      other.reader_mutex_ = nullptr;
      other.readers_.store(0);
      other.writer_active_.store(false);
    }
    return *this;
  }

  bool lock() noexcept {
    if (xSemaphoreTake(writer_mutex_, portMAX_DELAY) != pdTRUE) {
      return false;
    }
    writer_active_.store(true);
    while (readers_.load() > 0) {
      taskYIELD();
    }
    return true;
  }

  bool try_lock() noexcept {
    if (xSemaphoreTake(writer_mutex_, 0) != pdTRUE) {
      return false;
    }
    writer_active_.store(true);
    if (readers_.load() > 0) {
      writer_active_.store(false);
      xSemaphoreGive(writer_mutex_);
      return false;
    }
    return true;
  }

  bool try_lock_for(uint32_t timeout_ms) noexcept {
    const TickType_t ticks = RtosTime::MsToTicks(timeout_ms);
    const TickType_t start_time = xTaskGetTickCount();
    if (xSemaphoreTake(writer_mutex_, ticks) != pdTRUE) {
      return false;
    }
    writer_active_.store(true);
    const TickType_t elapsed = xTaskGetTickCount() - start_time;
    const TickType_t remaining = (elapsed < ticks) ? ticks - elapsed : 0;
    const TickType_t reader_wait_end = xTaskGetTickCount() + remaining;
    while (readers_.load() > 0) {
      if (xTaskGetTickCount() >= reader_wait_end) {
        writer_active_.store(false);
        xSemaphoreGive(writer_mutex_);
        return false;
      }
      taskYIELD();
    }
    return true;
  }

  void unlock() noexcept {
    writer_active_.store(false);
    xSemaphoreGive(writer_mutex_);
  }

  bool lock_shared() noexcept {
    while (true) {
      if (xSemaphoreTake(reader_mutex_, portMAX_DELAY) != pdTRUE) {
        return false;
      }
      if (!writer_active_.load()) {
        readers_++;
        xSemaphoreGive(reader_mutex_);
        return true;
      }
      xSemaphoreGive(reader_mutex_);
      taskYIELD();
    }
  }

  bool try_lock_shared() noexcept {
    if (xSemaphoreTake(reader_mutex_, 0) != pdTRUE) {
      return false;
    }
    if (!writer_active_.load()) {
      readers_++;
      xSemaphoreGive(reader_mutex_);
      return true;
    }
    xSemaphoreGive(reader_mutex_);
    return false;
  }

  bool try_lock_shared_for(uint32_t timeout_ms) noexcept {
    const TickType_t ticks = RtosTime::MsToTicks(timeout_ms);
    const TickType_t start_time = xTaskGetTickCount();
    while (true) {
      const TickType_t elapsed = xTaskGetTickCount() - start_time;
      if (elapsed >= ticks) {
        return false;
      }
      const TickType_t remaining = ticks - elapsed;
      if (xSemaphoreTake(reader_mutex_, remaining) != pdTRUE) {
        return false;
      }
      if (!writer_active_.load()) {
        readers_++;
        xSemaphoreGive(reader_mutex_);
        return true;
      }
      xSemaphoreGive(reader_mutex_);
      const TickType_t new_elapsed = xTaskGetTickCount() - start_time;
      if (new_elapsed >= ticks) {
        return false;
      }
      taskYIELD();
    }
  }

  void unlock_shared() noexcept {
    if (xSemaphoreTake(reader_mutex_, portMAX_DELAY) == pdTRUE) {
      if (readers_.load() > 0) {
        readers_--;
      }
      xSemaphoreGive(reader_mutex_);
    }
  }

private:
  SemaphoreHandle_t writer_mutex_;
  SemaphoreHandle_t reader_mutex_;
  std::atomic<int> readers_;
  std::atomic<bool> writer_active_;
};

template <typename Mutex> class RtosUniqueLock {
public:
  explicit RtosUniqueLock(Mutex &mutex, uint32_t timeout_ms = 0) noexcept
      : mutex_(&mutex), locked_(false) {
    if (timeout_ms > 0) {
      locked_ = mutex_->try_lock_for(timeout_ms);
    } else {
      locked_ = mutex_->lock();
    }
  }

  ~RtosUniqueLock() noexcept {
    if (locked_ && mutex_) {
      mutex_->unlock();
    }
  }

  RtosUniqueLock(const RtosUniqueLock &) = delete;
  RtosUniqueLock &operator=(const RtosUniqueLock &) = delete;

  RtosUniqueLock(RtosUniqueLock &&other) noexcept : mutex_(other.mutex_), locked_(other.locked_) {
    other.mutex_ = nullptr;
    other.locked_ = false;
  }

  RtosUniqueLock &operator=(RtosUniqueLock &&other) noexcept {
    if (this != &other) {
      if (locked_ && mutex_) {
        mutex_->unlock();
      }
      mutex_ = other.mutex_;
      locked_ = other.locked_;
      other.mutex_ = nullptr;
      other.locked_ = false;
    }
    return *this;
  }

  [[nodiscard]] bool IsLocked() const noexcept {
    return locked_;
  }

  void Unlock() noexcept {
    if (locked_ && mutex_) {
      mutex_->unlock();
      locked_ = false;
    }
  }

private:
  Mutex *mutex_;
  bool locked_;
};

template <typename SharedMutex> class RtosSharedLock {
public:
  explicit RtosSharedLock(SharedMutex &mutex, uint32_t timeout_ms = 0) noexcept
      : mutex_(&mutex), locked_(false) {
    if (timeout_ms > 0) {
      locked_ = mutex_->try_lock_shared_for(timeout_ms);
    } else {
      locked_ = mutex_->lock_shared();
    }
  }

  ~RtosSharedLock() noexcept {
    if (locked_ && mutex_) {
      mutex_->unlock_shared();
    }
  }

  RtosSharedLock(const RtosSharedLock &) = delete;
  RtosSharedLock &operator=(const RtosSharedLock &) = delete;

  RtosSharedLock(RtosSharedLock &&other) noexcept : mutex_(other.mutex_), locked_(other.locked_) {
    other.mutex_ = nullptr;
    other.locked_ = false;
  }

  RtosSharedLock &operator=(RtosSharedLock &&other) noexcept {
    if (this != &other) {
      if (locked_ && mutex_) {
        mutex_->unlock_shared();
      }
      mutex_ = other.mutex_;
      locked_ = other.locked_;
      other.mutex_ = nullptr;
      other.locked_ = false;
    }
    return *this;
  }

  [[nodiscard]] bool IsLocked() const noexcept {
    return locked_;
  }

  void Unlock() noexcept {
    if (locked_ && mutex_) {
      mutex_->unlock_shared();
      locked_ = false;
    }
  }

private:
  SharedMutex *mutex_;
  bool locked_;
};
