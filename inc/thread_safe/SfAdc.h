/**
 * @file SfAdc.h
 * @brief Enhanced thread-safe ADC interface wrapper.
 *
 * This file provides an enhanced thread-safe wrapper around the BaseAdc interface for use
 * in multi-threaded applications. All operations are synchronized using reader-writer mutexes
 * to ensure thread safety when multiple threads access the same ADC interface.
 *
 * Key Features:
 * - Complete thread-safe wrapper for all BaseAdc operations
 * - Lock-free read operations for improved performance
 * - Batch conversion operations with single lock acquisition
 * - Advanced threading statistics and diagnostics
 * - Configurable timeout for mutex operations
 * - Convenience methods (blocking/non-blocking variants)
 * - Reader-writer locks for better concurrent read performance
 * - Comprehensive error handling with thread context
 *
 * This is the recommended interface for component handlers and application threads
 * that need ADC conversion capabilities in multi-threaded environments.
 */
#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_SFADC_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_SFADC_H_

#include "../utils/RtosMutex.h"
#include "BaseAdc.h"
#include "McuTypes.h"
#include <atomic>
#include <memory>
#include <vector>

/**
 * @class SfAdc
 * @brief Enhanced thread-safe ADC interface wrapper.
 *
 * This class wraps any BaseAdc implementation with comprehensive thread safety.
 * It uses composition over inheritance to provide a robust, thread-safe interface
 * while maintaining full compatibility with the BaseAdc API.
 *
 * Thread Safety Features:
 * - Reader-writer mutexes for concurrent read operations
 * - Atomic statistics tracking
 * - Configurable mutex timeouts
 * - Lock-free status queries where possible
 * - Batch operations with single lock acquisition
 *
 * Usage:
 * @code
 * auto mcu_adc = std::make_unique<McuAdc>(config);
 * SfAdc safe_adc(std::move(mcu_adc));
 * safe_adc.Initialize();
 *
 * // Thread-safe conversions
 * float voltage = 0.0f;
 * if (safe_adc.ReadVoltage(channel, voltage)) {
 *     // Use voltage safely from any thread
 * }
 * @endcode
 */
class SfAdc {
public:
  //==============================================================================
  // CONSTRUCTION AND DESTRUCTION
  //==============================================================================

  /**
   * @brief Construct thread-safe ADC wrapper.
   * @param adc_impl BaseAdc implementation to wrap (takes ownership)
   */
  explicit SfAdc(std::unique_ptr<BaseAdc> adc_impl) noexcept;

  /**
   * @brief Destructor ensures proper cleanup and synchronization.
   */
  ~SfAdc() noexcept;

  // Non-copyable, movable
  SfAdc(const SfAdc &) = delete;
  SfAdc &operator=(const SfAdc &) = delete;
  SfAdc(SfAdc &&) noexcept = default;
  SfAdc &operator=(SfAdc &&) noexcept = default;

  //==============================================================================
  // CONFIGURATION AND CONTROL
  //==============================================================================

  /**
   * @brief Set mutex acquisition timeout for all operations.
   * @param timeout Maximum time to wait for mutex acquisition
   */
  void SetMutexTimeout(uint32_t timeout_ms) noexcept;

  /**
   * @brief Initialize ADC with thread safety.
   * @return ADC error code
   */
  HfAdcErr Initialize() noexcept;

  /**
   * @brief Deinitialize ADC with proper synchronization.
   * @return ADC error code
   */
  HfAdcErr Deinitialize() noexcept;

  /**
   * @brief Configure ADC channel with thread safety.
   * @param channel_id Channel to configure
   * @param config Channel configuration
   * @return ADC error code
   */
  HfAdcErr ConfigureChannel(uint8_t channel_id, const AdcChannelConfig &config) noexcept;

  //==============================================================================
  // CONVERSION OPERATIONS
  //==============================================================================

  /**
   * @brief Read raw ADC value (thread-safe).
   * @param channel_id Channel to read
   * @param raw_value Output raw value
   * @return ADC error code
   */
  HfAdcErr ReadRaw(uint8_t channel_id, uint16_t &raw_value) noexcept;

  /**
   * @brief Read voltage value (thread-safe).
   * @param channel_id Channel to read
   * @param voltage Output voltage value
   * @return ADC error code
   */
  HfAdcErr ReadVoltage(uint8_t channel_id, float &voltage) noexcept;

  /**
   * @brief Start continuous conversion (thread-safe).
   * @param channel_id Channel for continuous conversion
   * @param sample_rate_hz Sampling rate
   * @return ADC error code
   */
  HfAdcErr StartContinuous(uint8_t channel_id, uint32_t sample_rate_hz) noexcept;

  /**
   * @brief Stop continuous conversion (thread-safe).
   * @param channel_id Channel to stop
   * @return ADC error code
   */
  HfAdcErr StopContinuous(uint8_t channel_id) noexcept;

  //==============================================================================
  // BATCH OPERATIONS (OPTIMIZED FOR MULTI-CHANNEL)
  //==============================================================================

  /**
   * @brief Read multiple channels with single lock acquisition.
   * @param channels Vector of channel IDs to read
   * @param raw_values Output vector of raw values (same order as channels)
   * @return ADC error code
   */
  HfAdcErr ReadRawBatch(const std::vector<uint8_t> &channels,
                        std::vector<uint16_t> &raw_values) noexcept;

  /**
   * @brief Read multiple channel voltages with single lock acquisition.
   * @param channels Vector of channel IDs to read
   * @param voltages Output vector of voltage values (same order as channels)
   * @return ADC error code
   */
  HfAdcErr ReadVoltageBatch(const std::vector<uint8_t> &channels,
                            std::vector<float> &voltages) noexcept;

  //==============================================================================
  // STATUS AND DIAGNOSTICS (LOCK-FREE WHERE POSSIBLE)
  //==============================================================================

  /**
   * @brief Check if ADC is initialized (atomic read).
   * @return true if initialized
   */
  bool IsInitialized() const noexcept;

  /**
   * @brief Get maximum number of channels (lock-free).
   * @return Maximum channels supported
   */
  uint8_t GetMaxChannels() const noexcept;

  /**
   * @brief Check if channel is active (atomic read where possible).
   * @param channel_id Channel to check
   * @return true if channel is active
   */
  bool IsChannelActive(uint8_t channel_id) const noexcept;

  /**
   * @brief Get ADC resolution for channel (lock-free where possible).
   * @param channel_id Channel to query
   * @return Resolution in bits
   */
  uint8_t GetChannelResolution(uint8_t channel_id) const noexcept;

  //==============================================================================
  // CALLBACK MANAGEMENT
  //==============================================================================

  /**
   * @brief Set conversion complete callback (thread-safe).
   * @param callback Callback function
   * @param user_data User data pointer
   * @return ADC error code
   */
  HfAdcErr SetConversionCallback(AdcConversionCallback callback, void *user_data) noexcept;

  /**
   * @brief Set error callback (thread-safe).
   * @param callback Error callback function
   * @param user_data User data pointer
   * @return ADC error code
   */
  HfAdcErr SetErrorCallback(AdcErrorCallback callback, void *user_data) noexcept;

  //==============================================================================
  // ADVANCED THREADING FEATURES
  //==============================================================================

  /**
   * @brief Acquire exclusive lock for extended operations.
   * @param timeout_ms Maximum time to wait for lock
   * @return true if lock acquired
   */
  bool Lock(uint32_t timeout_ms = UINT32_MAX) noexcept;

  /**
   * @brief Release exclusive lock.
   * @return true if lock released successfully
   */
  bool Unlock() noexcept;

  /**
   * @brief Acquire shared lock for concurrent reads.
   * @param timeout_ms Maximum time to wait for lock
   * @return true if shared lock acquired
   */
  bool LockShared(uint32_t timeout_ms = UINT32_MAX) noexcept;

  /**
   * @brief Release shared lock.
   * @return true if shared lock released successfully
   */
  bool UnlockShared() noexcept;

  //==============================================================================
  // STATISTICS AND DIAGNOSTICS
  //==============================================================================

  /**
   * @brief Threading statistics structure.
   */
  struct ThreadingStats {
    std::atomic<uint64_t> total_operations{0};
    std::atomic<uint64_t> lock_acquisitions{0};
    std::atomic<uint64_t> lock_timeouts{0};
    std::atomic<uint64_t> concurrent_reads{0};
    std::atomic<uint64_t> exclusive_operations{0};
    std::atomic<uint32_t> max_wait_time_us{0};
    std::atomic<uint32_t> current_readers{0};
  };

  /**
   * @brief Get threading statistics (atomic reads).
   * @return Current threading statistics
   */
  ThreadingStats GetThreadingStats() const noexcept;

  /**
   * @brief Reset threading statistics.
   */
  void ResetThreadingStats() noexcept;

private:
  //==============================================================================
  // PRIVATE MEMBERS
  //==============================================================================

  std::unique_ptr<BaseAdc> adc_impl_;  ///< Wrapped ADC implementation
  mutable std::shared_mutex rw_mutex_; ///< Reader-writer mutex
  std::atomic<bool> initialized_;      ///< Atomic initialization flag
  uint32_t mutex_timeout_ms_;          ///< Mutex acquisition timeout in milliseconds
  mutable ThreadingStats stats_;       ///< Threading statistics

  //==============================================================================
  // PRIVATE HELPER METHODS
  //==============================================================================

  /**
   * @brief Acquire exclusive lock with timeout and statistics.
   */
  bool AcquireExclusiveLock() noexcept;

  /**
   * @brief Acquire shared lock with timeout and statistics.
   */
  bool AcquireSharedLock() const noexcept;

  /**
   * @brief Update lock timing statistics.
   */
  void UpdateLockStats(uint64_t start_time_us) const noexcept;

  /**
   * @brief Default mutex timeout (5 seconds).
   */
  static constexpr uint32_t DEFAULT_TIMEOUT_MS{5000};
};

#endif // HAL_INTERNAL_INTERFACE_DRIVERS_SFADC_H_
