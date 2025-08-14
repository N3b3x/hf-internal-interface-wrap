/**
 * @file RtosMutex.h
 * @brief Cross-platform RTOS mutex and synchronization primitives.
 *
 * This header provides platform-agnostic mutex, lock guard, and timing utilities
 * that work across different RTOS implementations (FreeRTOS on ESP32, STM32, RP2040).
 * The implementation includes standard mutexes, shared mutexes for reader-writer patterns,
 * RAII lock guards, and high-resolution timing functions for synchronization.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Platform support includes ESP32 (FreeRTOS), STM32 (CMSIS-OS), and RP2040 (FreeRTOS).
 *
 * @section recursive_mutex_example Recursive Mutex Usage Example
 * 
 * The RtosRecursiveMutex class allows the same task to acquire the mutex multiple times
 * without blocking itself. This is particularly useful for nested function calls or
 * recursive algorithms that need synchronized access to shared resources.
 * 
 * @code
 * #include "utils/RtosMutex.h"
 * 
 * // Global recursive mutex for protecting shared resource
 * RtosRecursiveMutex g_resource_mutex;
 * int g_shared_counter = 0;
 * 
 * void nested_function() {
 *     // This function can be called from within another function that
 *     // already holds the mutex, preventing self-deadlock
 *     RecursiveMutexLockGuard lock(g_resource_mutex);
 *     g_shared_counter++;
 *     ESP_LOGI("TAG", "Nested: counter = %d", g_shared_counter);
 * }
 * 
 * void main_function() {
 *     // Acquire the mutex
 *     RecursiveMutexLockGuard lock(g_resource_mutex);
 *     g_shared_counter++;
 *     ESP_LOGI("TAG", "Main: counter = %d", g_shared_counter);
 *     
 *     // Call nested function - this won't deadlock
 *     nested_function();
 *     
 *     g_shared_counter++;
 *     ESP_LOGI("TAG", "Main after nested: counter = %d", g_shared_counter);
 * }
 * 
 * // Alternative manual locking approach
 * void manual_recursive_locking() {
 *     RtosRecursiveMutex mutex;
 *     
 *     // Lock multiple times
 *     mutex.lock();   // First acquisition
 *     mutex.lock();   // Second acquisition (same task)
 *     mutex.lock();   // Third acquisition (same task)
 *     
 *     // Perform work...
 *     
 *     // Must unlock the same number of times
 *     mutex.unlock(); // Release third acquisition
 *     mutex.unlock(); // Release second acquisition  
 *     mutex.unlock(); // Release first acquisition
 * }
 * @endcode
 */

#pragma once

#include "McuSelect.h"

// Suppress pedantic warnings for ESP-IDF headers
#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

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
#error \
    "RTOS mutex implementation not available for this MCU platform. Please add support in RtosMutex.h"
#endif

#ifdef __cplusplus
}
#endif

// Restore warnings after ESP-IDF headers
#ifdef __cplusplus
#pragma GCC diagnostic pop
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

  RtosMutex(const RtosMutex&) = delete;
  RtosMutex& operator=(const RtosMutex&) = delete;

  RtosMutex(RtosMutex&& other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  RtosMutex& operator=(RtosMutex&& other) noexcept {
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

  // Convenience FreeRTOS-style API -------------------------------------------------
  bool Take(uint32_t timeout_ms = 0) noexcept {
    if (timeout_ms > 0) {
      return try_lock_for(timeout_ms);
    }
    return lock();
  }

  void Give() noexcept {
    unlock();
  }

  // Shared lock methods (delegated to regular mutex for simplicity)
  bool lock_shared() noexcept {
    return lock();
  }

  bool try_lock_shared() noexcept {
    return try_lock();
  }

  bool try_lock_shared_for(uint32_t timeout_ms) noexcept {
    return try_lock_for(timeout_ms);
  }

  void unlock_shared() noexcept {
    unlock();
  }

private:
  SemaphoreHandle_t handle_;
};

/**
 * @brief Recursive mutex implementation for RTOS environments.
 * 
 * This class provides a recursive mutex that allows the same task to acquire
 * the mutex multiple times without blocking itself. Each successful lock
 * operation must be matched with a corresponding unlock operation.
 * 
 * Features:
 * - Prevents self-deadlocks when a task needs to acquire the same mutex multiple times
 * - Compatible with existing RAII lock guard classes
 * - Standard mutex interface (lock, unlock, try_lock, etc.)
 * - FreeRTOS-style API (Take, Give)
 */
class RtosRecursiveMutex {
public:
  RtosRecursiveMutex() noexcept : handle_(xSemaphoreCreateRecursiveMutex()) {}

  ~RtosRecursiveMutex() noexcept {
    if (handle_) {
      vSemaphoreDelete(handle_);
    }
  }

  RtosRecursiveMutex(const RtosRecursiveMutex&) = delete;
  RtosRecursiveMutex& operator=(const RtosRecursiveMutex&) = delete;

  RtosRecursiveMutex(RtosRecursiveMutex&& other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  RtosRecursiveMutex& operator=(RtosRecursiveMutex&& other) noexcept {
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
    return xSemaphoreTakeRecursive(handle_, portMAX_DELAY) == pdTRUE;
  }

  bool try_lock() noexcept {
    if (!handle_)
      return false;
    return xSemaphoreTakeRecursive(handle_, 0) == pdTRUE;
  }

  bool try_lock_for(uint32_t timeout_ms) noexcept {
    if (!handle_)
      return false;
    const TickType_t ticks = RtosTime::MsToTicks(timeout_ms);
    return xSemaphoreTakeRecursive(handle_, ticks) == pdTRUE;
  }

  void unlock() noexcept {
    if (handle_) {
      xSemaphoreGiveRecursive(handle_);
    }
  }

  SemaphoreHandle_t native_handle() const noexcept {
    return handle_;
  }

  // Convenience FreeRTOS-style API -------------------------------------------------
  bool Take(uint32_t timeout_ms = 0) noexcept {
    if (timeout_ms > 0) {
      return try_lock_for(timeout_ms);
    }
    return lock();
  }

  void Give() noexcept {
    unlock();
  }

  // Shared lock methods (delegated to regular mutex for simplicity)
  bool lock_shared() noexcept {
    return lock();
  }

  bool try_lock_shared() noexcept {
    return try_lock();
  }

  bool try_lock_shared_for(uint32_t timeout_ms) noexcept {
    return try_lock_for(timeout_ms);
  }

  void unlock_shared() noexcept {
    unlock();
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

  RtosSharedMutex(const RtosSharedMutex&) = delete;
  RtosSharedMutex& operator=(const RtosSharedMutex&) = delete;

  RtosSharedMutex(RtosSharedMutex&& other) noexcept
      : writer_mutex_(other.writer_mutex_), reader_mutex_(other.reader_mutex_),
        readers_(other.readers_.load()), writer_active_(other.writer_active_.load()) {
    other.writer_mutex_ = nullptr;
    other.reader_mutex_ = nullptr;
    other.readers_.store(0);
    other.writer_active_.store(false);
  }

  RtosSharedMutex& operator=(RtosSharedMutex&& other) noexcept {
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

template <typename Mutex>
class RtosUniqueLock {
public:
  explicit RtosUniqueLock(Mutex& mutex, uint32_t timeout_ms = 0) noexcept
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

  RtosUniqueLock(const RtosUniqueLock&) = delete;
  RtosUniqueLock& operator=(const RtosUniqueLock&) = delete;

  RtosUniqueLock(RtosUniqueLock&& other) noexcept : mutex_(other.mutex_), locked_(other.locked_) {
    other.mutex_ = nullptr;
    other.locked_ = false;
  }

  RtosUniqueLock& operator=(RtosUniqueLock&& other) noexcept {
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
  Mutex* mutex_;
  bool locked_;
};

template <typename SharedMutex>
class RtosSharedLock {
public:
  explicit RtosSharedLock(SharedMutex& mutex, uint32_t timeout_ms = 0) noexcept
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

  RtosSharedLock(const RtosSharedLock&) = delete;
  RtosSharedLock& operator=(const RtosSharedLock&) = delete;

  RtosSharedLock(RtosSharedLock&& other) noexcept : mutex_(other.mutex_), locked_(other.locked_) {
    other.mutex_ = nullptr;
    other.locked_ = false;
  }

  RtosSharedLock& operator=(RtosSharedLock&& other) noexcept {
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
  SharedMutex* mutex_;
  bool locked_;
};

//==============================================================================
// CONVENIENCE TYPE ALIASES
//==============================================================================

/// @brief Convenience alias for unique lock guard
template <typename Mutex>
using RtosLockGuard = RtosUniqueLock<Mutex>;

/// @brief Convenience alias for RtosMutex lock guard
using MutexLockGuard = RtosUniqueLock<RtosMutex>;

/// @brief Convenience alias for RtosRecursiveMutex lock guard
using RecursiveMutexLockGuard = RtosUniqueLock<RtosRecursiveMutex>;
