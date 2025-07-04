/**
 * @file SfCan.h
 * @brief Enhanced thread-safe CAN bus interface wrapper.
 *
 * This file provides an enhanced thread-safe wrapper around the BaseCan interface for use
 * in multi-threaded applications. All operations are synchronized using reader-writer mutexes
 * to ensure thread safety when multiple threads access the same CAN interface.
 * The implementation includes lock-free read operations, batch message operations, advanced
 * threading statistics, configurable timeouts, and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the recommended interface for component handlers and application threads
 *       that need CAN communication capabilities in multi-threaded environments.
 * @note Features complete thread-safe wrapper, lock-free reads, batch operations,
 *       reader-writer locks for concurrent performance, and threading diagnostics.
 */

#pragma once

#include "RtosMutex.h"
#include "BaseCan.h"
#include "McuTypes.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * @class SfCan
 * @brief Enhanced thread-safe CAN bus interface.
 *
 * This class provides an advanced thread-safe wrapper around the BaseCan interface.
 * All methods are protected by reader-writer mutexes to ensure safe concurrent access
 * from multiple threads while optimizing read performance.
 *
 * Usage example:
 * @code
 * // Create thread-safe CAN interface with MCU implementation
 * CanBusConfig config{};
 * config.baud_rate = 500000;
 * config.tx_pin = GPIO_CAN_TX;
 * config.rx_pin = GPIO_CAN_RX;
 *
 * SfCan sf_can(std::make_unique<McuCan>(config));
 * if (sf_can.Initialize()) {
 *     sf_can.Start();
 *
 *     CanMessage msg{};
 *     msg.id = 0x123;
 *     msg.dlc = 8;
 *     // ... fill msg.data
 *
 *     // Blocking send
 *     sf_can.SendMessageBlocking(msg);
 *
 *     // Non-blocking send
 *     sf_can.SendMessageNonBlocking(msg);
 *
 *     // Batch send multiple messages
 *     std::vector<CanMessage> messages = { msg1, msg2, msg3 };
 *     sf_can.SendMultipleMessages(messages);
 * }
 * }
 * @endcode
 */
class SfCan {
public:
  /**
   * @brief Threading statistics structure.
   */
  struct ThreadingStats {
    uint64_t lock_contentions = 0;     ///< Number of lock timeouts/contentions
    uint64_t total_operations = 0;     ///< Total number of operations
    uint64_t total_lock_time_us = 0;   ///< Total time spent acquiring locks (microseconds)
    uint64_t average_lock_time_us = 0; ///< Average lock acquisition time (microseconds)
    uint64_t max_lock_time_us = 0;     ///< Maximum lock acquisition time (microseconds)
  };

  /**
   * @brief Construct a new SfCan object with BaseCan implementation.
   *
   * @param can_impl Unique pointer to BaseCan implementation (McuCan, etc.)
   */
  explicit SfCan(std::unique_ptr<BaseCan> can_impl) noexcept;

  /**
   * @brief Destroy the SfCan object.
   *
   * Ensures proper cleanup of CAN interface and releases locks.
   */
  ~SfCan() noexcept;

  // Non-copyable and non-movable for thread safety
  SfCan(const SfCan &) = delete;
  SfCan &operator=(const SfCan &) = delete;
  SfCan(SfCan &&) = delete;
  SfCan &operator=(SfCan &&) = delete;

  //==============================================================================
  // CONFIGURATION AND CONTROL
  //==============================================================================

  /**
   * @brief Set the mutex timeout for lock acquisition.
   *
   * @param timeout Timeout duration for mutex operations
   */
  void SetMutexTimeout(uint32_t timeout_ms) noexcept;

  /**
   * @brief Get the current mutex timeout.
   *
   * @return Current mutex timeout duration
   */
  uint32_t GetMutexTimeout() const noexcept;

  //==============================================================================
  // INITIALIZATION AND CONTROL
  //==============================================================================

  /**
   * @brief Initialize the CAN interface.
   *
   * @return True if initialization successful, false otherwise
   */
  bool Initialize() noexcept;

  /**
   * @brief Deinitialize the CAN interface.
   *
   * @return True if deinitialization successful, false otherwise
   */
  bool Deinitialize() noexcept;

  /**
   * @brief Start CAN communication.
   *
   * @return True if start successful, false otherwise
   */
  bool Start() noexcept;

  /**
   * @brief Stop CAN communication.
   *
   * @return True if stop successful, false otherwise
   */
  bool Stop() noexcept;

  //==============================================================================
  // MESSAGE TRANSMISSION AND RECEPTION
  //==============================================================================

  /**
   * @brief Send a CAN message with specified timeout.
   *
   * @param message CAN message to send
   * @param timeout_ms Timeout in milliseconds
   * @return True if message sent successfully, false otherwise
   */
  bool SendMessage(const CanMessage &message, uint32_t timeout_ms = 1000) noexcept;

  /**
   * @brief Receive a CAN message with specified timeout.
   *
   * @param message Reference to store received message
   * @param timeout_ms Timeout in milliseconds
   * @return True if message received successfully, false otherwise
   */
  bool ReceiveMessage(CanMessage &message, uint32_t timeout_ms = 1000) noexcept;

  //==============================================================================
  // CONVENIENCE METHODS
  //==============================================================================

  /**
   * @brief Send a message with no blocking (timeout = 0).
   *
   * @param message CAN message to send
   * @return True if message sent immediately, false if queue full
   */
  bool SendMessageNonBlocking(const CanMessage &message) noexcept;

  /**
   * @brief Send a message with infinite blocking (timeout = MAX).
   *
   * @param message CAN message to send
   * @return True if message sent successfully, false on error
   */
  bool SendMessageBlocking(const CanMessage &message) noexcept;

  /**
   * @brief Receive a message with no blocking (timeout = 0).
   *
   * @param message Reference to store received message
   * @return True if message received immediately, false if queue empty
   */
  bool ReceiveMessageNonBlocking(CanMessage &message) noexcept;

  /**
   * @brief Receive a message with infinite blocking (timeout = MAX).
   *
   * @param message Reference to store received message
   * @return True if message received successfully, false on error
   */
  bool ReceiveMessageBlocking(CanMessage &message) noexcept;

  //==============================================================================
  // BATCH OPERATIONS
  //==============================================================================

  /**
   * @brief Send multiple messages with single lock acquisition.
   *
   * @param messages Vector of messages to send
   * @param timeout_ms Timeout per message in milliseconds
   * @return True if all messages sent successfully, false on first failure
   */
  bool SendMultipleMessages(const std::vector<CanMessage> &messages,
                            uint32_t timeout_ms = 1000) noexcept;

  /**
   * @brief Send multiple messages with partial success allowed.
   *
   * @param messages Vector of messages to send
   * @param timeout_ms Timeout per message in milliseconds
   * @return Number of messages successfully sent
   */
  size_t SendMultipleMessagesPartial(const std::vector<CanMessage> &messages,
                                     uint32_t timeout_ms = 1000) noexcept;

  //==============================================================================
  // CALLBACK MANAGEMENT
  //==============================================================================

  /**
   * @brief Set receive callback function.
   *
   * @param callback Callback function to call on message reception
   * @return True if callback set successfully, false otherwise
   */
  bool SetReceiveCallback(CanReceiveCallback callback) noexcept;

  /**
   * @brief Clear the receive callback.
   */
  void ClearReceiveCallback() noexcept;

  //==============================================================================
  // STATUS AND DIAGNOSTICS
  //==============================================================================

  /**
   * @brief Get current CAN bus status.
   *
   * @param status Reference to store status information
   * @return True if status retrieved successfully, false otherwise
   */
  bool GetStatus(CanBusStatus &status) noexcept;

  /**
   * @brief Reset the CAN interface.
   *
   * @return True if reset successful, false otherwise
   */
  bool Reset() noexcept;

  //==============================================================================
  // LOCK-FREE READ OPERATIONS
  //==============================================================================

  /**
   * @brief Check if CAN interface is initialized (lock-free).
   *
   * @return True if initialized, false otherwise
   */
  bool IsInitialized() const noexcept;

  /**
   * @brief Check if transmit queue is full.
   *
   * @return True if queue full, false otherwise
   */
  bool IsTransmitQueueFull() const noexcept;

  /**
   * @brief Check if receive queue is empty.
   *
   * @return True if queue empty, false otherwise
   */
  bool IsReceiveQueueEmpty() const noexcept;

  //==============================================================================
  // ADVANCED THREADING FEATURES
  //==============================================================================

  /**
   * @brief Try to acquire exclusive lock without blocking.
   *
   * @return True if lock acquired, false if already locked
   */
  bool TryLock() noexcept;

  /**
   * @brief Acquire exclusive lock (blocking).
   */
  void Lock() noexcept;

  /**
   * @brief Release exclusive lock.
   */
  void Unlock() noexcept;

  /**
   * @brief Get threading performance statistics.
   *
   * @return Current threading statistics
   */
  ThreadingStats GetThreadingStats() const noexcept;

  /**
   * @brief Reset threading statistics to zero.
   */
  void ResetThreadingStats() noexcept;

  /**
   * @brief Get the underlying BaseCan implementation.
   *
   * @return Pointer to underlying implementation
   */
  const BaseCan *GetImplementation() const noexcept;

private:
  std::unique_ptr<BaseCan> can_impl_; ///< Underlying CAN implementation
  std::atomic<bool> initialized_;     ///< Lock-free initialization flag
  mutable RtosSharedMutex rw_mutex_;  ///< Reader-writer mutex for thread safety
  uint32_t mutex_timeout_ms_;         ///< Timeout for mutex operations in milliseconds
  mutable ThreadingStats stats_;      ///< Threading performance statistics

  /**
   * @brief Update threading statistics.
   *
   * @param start_time Start time of the operation
   * @param was_timeout Whether the operation timed out
   */
  void UpdateStats(const std::chrono::steady_clock::time_point &start_time,
                   bool was_timeout) noexcept;
};
