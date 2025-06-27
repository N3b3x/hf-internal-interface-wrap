#ifndef HF_RTOS_MUTEX_H
#define HF_RTOS_MUTEX_H

#include "../mcu/McuTypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief RAII wrapper around a FreeRTOS mutex handle with scoped lock support.
 */
class RtosMutex {
public:
  RtosMutex() noexcept : handle_(xSemaphoreCreateMutex()) {}

  ~RtosMutex() noexcept {
    if (handle_ != nullptr) {
      vSemaphoreDelete(handle_);
      handle_ = nullptr;
    }
  }

  RtosMutex(const RtosMutex &) = delete;
  RtosMutex &operator=(const RtosMutex &) = delete;

  RtosMutex(RtosMutex &&other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  RtosMutex &operator=(RtosMutex &&other) noexcept {
    if (this != &other) {
      if (handle_ != nullptr)
        vSemaphoreDelete(handle_);
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  [[nodiscard]] hf_mutex_handle_t GetHandle() const noexcept {
    return handle_;
  }

  bool Take(hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT) noexcept {
    if (handle_ == nullptr)
      return false;
    TickType_t ticks = (timeout_ms == HF_TIMEOUT_NEVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(handle_, ticks) == pdTRUE;
  }

  void Give() noexcept {
    if (handle_ != nullptr)
      xSemaphoreGive(handle_);
  }

  /**
   * @brief Scoped lock guard using RAII to automatically release the mutex.
   */
  class LockGuard {
  public:
    explicit LockGuard(RtosMutex &m, hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT) noexcept
        : mutex_(m), locked_(m.Take(timeout_ms)) {}

    ~LockGuard() noexcept {
      if (locked_)
        mutex_.Give();
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

    LockGuard(LockGuard &&other) noexcept : mutex_(other.mutex_), locked_(other.locked_) {
      other.locked_ = false;
    }
    LockGuard &operator=(LockGuard &&other) noexcept {
      if (this != &other) {
        if (locked_)
          mutex_.Give();
        mutex_ = other.mutex_;
        locked_ = other.locked_;
        other.locked_ = false;
      }
      return *this;
    }

    [[nodiscard]] bool IsLocked() const noexcept {
      return locked_;
    }

  private:
    RtosMutex &mutex_;
    bool locked_;
  };

private:
  hf_mutex_handle_t handle_;
};

#endif // HF_RTOS_MUTEX_H
