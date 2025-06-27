/**
 * @file SfCan.cpp
 * @brief Implementation of enhanced thread-safe CAN bus interface wrapper.
 *
 * This file implements the enhanced thread-safe wrapper around the BaseCan interface
 * for safe use in multi-threaded applications with advanced features like:
 * - Lock-free read operations
 * - Batch message operations
 * - Advanced threading statistics
 * - Configurable timeout handling
 * - Non-blocking and blocking convenience methods
 */
#include "../thread_safe/SfCan.h"
#include <algorithm>
#include <chrono>

namespace {
constexpr std::chrono::milliseconds DEFAULT_TIMEOUT{100};
constexpr uint32_t DEFAULT_BLOCKING_TIMEOUT = UINT32_MAX;
} // namespace

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

SfCan::SfCan(std::unique_ptr<BaseCan> can_impl) noexcept
    : can_impl_(std::move(can_impl)), initialized_(false), mutex_timeout_(DEFAULT_TIMEOUT),
      stats_{} {}

SfCan::~SfCan() noexcept {
  std::lock_guard<std::shared_mutex> lock(rw_mutex_);
  if (initialized_ && can_impl_) {
    can_impl_->Deinitialize();
  }
}

//==============================================================================
// CONFIGURATION AND CONTROL
//==============================================================================

void SfCan::SetMutexTimeout(std::chrono::milliseconds timeout) noexcept {
  std::lock_guard<std::shared_mutex> lock(rw_mutex_);
  mutex_timeout_ = timeout;
}

std::chrono::milliseconds SfCan::GetMutexTimeout() const noexcept {
  std::shared_lock<std::shared_mutex> lock(rw_mutex_);
  return mutex_timeout_;
}

//==============================================================================
// INITIALIZATION AND CONTROL
//==============================================================================

bool SfCan::Initialize() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_) {
    UpdateStats(start_time, false);
    return false;
  }

  if (initialized_) {
    UpdateStats(start_time, false);
    return true; // Already initialized
  }

  bool result = can_impl_->Initialize();
  if (result) {
    initialized_.store(true, std::memory_order_release);
  }

  UpdateStats(start_time, false);
  return result;
}

bool SfCan::Deinitialize() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return true;
  }

  bool result = can_impl_->Deinitialize();
  if (result) {
    initialized_.store(false, std::memory_order_release);
  }

  UpdateStats(start_time, false);
  return result;
}

bool SfCan::Start() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->Start();
  UpdateStats(start_time, false);
  return result;
}

bool SfCan::Stop() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->Stop();
  UpdateStats(start_time, false);
  return result;
}

//==============================================================================
// MESSAGE TRANSMISSION AND RECEPTION
//==============================================================================

bool SfCan::SendMessage(const hf_can_message_t &message, uint32_t timeout_ms) noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->SendMessage(message, timeout_ms);
  UpdateStats(start_time, false);
  return result;
}

bool SfCan::ReceiveMessage(hf_can_message_t &message, uint32_t timeout_ms) noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->ReceiveMessage(message, timeout_ms);
  UpdateStats(start_time, false);
  return result;
}

//==============================================================================
// CONVENIENCE METHODS
//==============================================================================

bool SfCan::SendMessageNonBlocking(const hf_can_message_t &message) noexcept {
  return SendMessage(message, 0);
}

bool SfCan::SendMessageBlocking(const hf_can_message_t &message) noexcept {
  return SendMessage(message, DEFAULT_BLOCKING_TIMEOUT);
}

bool SfCan::ReceiveMessageNonBlocking(hf_can_message_t &message) noexcept {
  return ReceiveMessage(message, 0);
}

bool SfCan::ReceiveMessageBlocking(hf_can_message_t &message) noexcept {
  return ReceiveMessage(message, DEFAULT_BLOCKING_TIMEOUT);
}

//==============================================================================
// BATCH OPERATIONS
//==============================================================================

bool SfCan::SendMultipleMessages(const std::vector<hf_can_message_t> &messages,
                                 uint32_t timeout_ms) noexcept {
  if (messages.empty()) {
    return true;
  }

  auto start_time = std::chrono::steady_clock::now();
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  // Send all messages with single lock
  for (const auto &message : messages) {
    if (!can_impl_->SendMessage(message, timeout_ms)) {
      UpdateStats(start_time, false);
      return false;
    }
  }

  UpdateStats(start_time, false);
  return true;
}

size_t SfCan::SendMultipleMessagesPartial(const std::vector<hf_can_message_t> &messages,
                                          uint32_t timeout_ms) noexcept {
  if (messages.empty()) {
    return 0;
  }

  auto start_time = std::chrono::steady_clock::now();
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return 0;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return 0;
  }

  size_t sent_count = 0;
  for (const auto &message : messages) {
    if (can_impl_->SendMessage(message, timeout_ms)) {
      ++sent_count;
    } else {
      break; // Stop on first failure
    }
  }

  UpdateStats(start_time, false);
  return sent_count;
}

//==============================================================================
// CALLBACK MANAGEMENT
//==============================================================================

bool SfCan::SetReceiveCallback(CanReceiveCallback callback) noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->SetReceiveCallback(callback);
  UpdateStats(start_time, false);
  return result;
}

void SfCan::ClearReceiveCallback() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return;
  }

  if (!can_impl_) {
    UpdateStats(start_time, false);
    return;
  }

  can_impl_->ClearReceiveCallback();
  UpdateStats(start_time, false);
}

//==============================================================================
// STATUS AND DIAGNOSTICS
//==============================================================================

bool SfCan::GetStatus(CanBusStatus &status) noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->GetStatus(status);
  UpdateStats(start_time, false);
  return result;
}

bool SfCan::Reset() noexcept {
  auto start_time = std::chrono::steady_clock::now();
  std::unique_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    UpdateStats(start_time, true);
    return false;
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    UpdateStats(start_time, false);
    return false;
  }

  bool result = can_impl_->Reset();
  UpdateStats(start_time, false);
  return result;
}

//==============================================================================
// LOCK-FREE READ OPERATIONS
//==============================================================================

bool SfCan::IsInitialized() const noexcept {
  return initialized_.load(std::memory_order_acquire);
}

bool SfCan::IsTransmitQueueFull() const noexcept {
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    return true; // Conservative: assume full on timeout
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    return true;
  }

  return can_impl_->IsTransmitQueueFull();
}

bool SfCan::IsReceiveQueueEmpty() const noexcept {
  std::shared_lock<std::shared_mutex> lock(rw_mutex_, std::defer_lock);

  if (!lock.try_lock_for(mutex_timeout_)) {
    return true; // Conservative: assume empty on timeout
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    return true;
  }

  return can_impl_->IsReceiveQueueEmpty();
}

//==============================================================================
// ADVANCED THREADING FEATURES
//==============================================================================

bool SfCan::TryLock() noexcept {
  return rw_mutex_.try_lock();
}

void SfCan::Lock() noexcept {
  rw_mutex_.lock();
}

void SfCan::Unlock() noexcept {
  rw_mutex_.unlock();
}

SfCan::ThreadingStats SfCan::GetThreadingStats() const noexcept {
  std::shared_lock<std::shared_mutex> lock(rw_mutex_);
  return stats_;
}

void SfCan::ResetThreadingStats() noexcept {
  std::lock_guard<std::shared_mutex> lock(rw_mutex_);
  stats_ = ThreadingStats{};
}

const BaseCan *SfCan::GetImplementation() const noexcept {
  std::shared_lock<std::shared_mutex> lock(rw_mutex_);
  return can_impl_.get();
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

void SfCan::UpdateStats(const std::chrono::steady_clock::time_point &start_time,
                        bool was_timeout) noexcept {
  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  if (was_timeout) {
    ++stats_.lock_contentions;
  }

  ++stats_.total_operations;
  stats_.total_lock_time_us += static_cast<uint64_t>(duration.count());
  stats_.average_lock_time_us = stats_.total_lock_time_us / stats_.total_operations;

  if (duration.count() > static_cast<int64_t>(stats_.max_lock_time_us)) {
    stats_.max_lock_time_us = static_cast<uint64_t>(duration.count());
  }
}
