/**
 * @file SfCan.cpp
 * @brief Implementation of enhanced thread-safe CAN bus interface wrapper.
 *
 * This file implements the enhanced thread-safe wrapper around the BaseCan interface
 * for safe use in multi-threaded applications with advanced features like:
 * lock-free read operations, batch message operations, advanced threading statistics,
 * configurable timeout handling, and non-blocking convenience methods with comprehensive
 * error handling and reader-writer mutex synchronization.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "SfCan.h"
#include <algorithm>

namespace {
constexpr uint32_t DEFAULT_TIMEOUT_MS{100};
constexpr uint32_t DEFAULT_BLOCKING_TIMEOUT = UINT32_MAX;
} // namespace

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

SfCan::SfCan(std::unique_ptr<BaseCan> can_impl) noexcept
    : can_impl_(std::move(can_impl)), initialized_(false), mutex_timeout_ms_(DEFAULT_TIMEOUT_MS),
      stats_{} {}

SfCan::~SfCan() noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);
  if (initialized_ && can_impl_) {
    can_impl_->Deinitialize();
  }
}

//==============================================================================
// CONFIGURATION AND CONTROL
//==============================================================================

void SfCan::SetMutexTimeout(uint32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);
  if (lock.IsLocked()) {
    mutex_timeout_ms_ = timeout_ms;
  }
}

uint32_t SfCan::GetMutexTimeout() const noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);
  return lock.IsLocked() ? mutex_timeout_ms_ : 0;
}

//==============================================================================
// INITIALIZATION AND CONTROL
//==============================================================================

bool SfCan::Initialize() noexcept {
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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

  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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

  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  auto start_time = RtosTime::GetCurrentTimeUs();
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
    return true; // Conservative: assume full on timeout
  }

  if (!can_impl_ || !initialized_.load(std::memory_order_acquire)) {
    return true;
  }

  return can_impl_->IsTransmitQueueFull();
}

bool SfCan::IsReceiveQueueEmpty() const noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);

  if (!lock.try_lock_for(mutex_timeout_ms_)) {
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
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);
  return stats_;
}

void SfCan::ResetThreadingStats() noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_);
  stats_ = ThreadingStats{};
}

const BaseCan *SfCan::GetImplementation() const noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_);
  return can_impl_.get();
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

void SfCan::UpdateStats(uint64_t start_time_us, bool was_timeout) noexcept {
  uint64_t end_time_us = RtosTime::GetCurrentTimeUs();
  uint64_t duration = end_time_us - start_time_us;

  if (was_timeout) {
    ++stats_.lock_contentions;
  }

  ++stats_.total_operations;
  stats_.total_lock_time_us += duration;
  stats_.average_lock_time_us = stats_.total_lock_time_us / stats_.total_operations;

  if (duration > stats_.max_lock_time_us) {
    stats_.max_lock_time_us = duration;
  }
}
