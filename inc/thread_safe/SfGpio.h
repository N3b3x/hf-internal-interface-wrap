/**
 * @file SfGpio.h
 * @brief Thread-safe GPIO interface wrapper.
 *
 * This header provides a thread-safe wrapper around the base GPIO interface,
 * ensuring atomic operations and thread safety for multi-threaded applications.
 * Uses composition pattern with mutex protection.
 */

#ifndef SF_GPIO_H
#define SF_GPIO_H

#include "../base/BaseGpio.h"
#include "../mcu/McuTypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <memory>

namespace HardFOC {
namespace ThreadSafe {

/**
 * @brief Thread-safe GPIO interface wrapper.
 *
 * This class wraps any BaseGpio implementation with thread-safe operations
 * using mutex protection. All GPIO operations are atomic and safe for
 * concurrent access from multiple tasks/threads.
 */
class SfGpio {
public:
  /**
   * @brief Constructor with GPIO implementation.
   *
   * @param gpio_impl Pointer to the base GPIO implementation to wrap
   */
  explicit SfGpio(std::shared_ptr<BaseGpio> gpio_impl);

  /**
   * @brief Destructor.
   */
  virtual ~SfGpio();

  // Disable copy constructor and assignment operator
  SfGpio(const SfGpio &) = delete;
  SfGpio &operator=(const SfGpio &) = delete;

  // Enable move constructor and assignment operator
  SfGpio(SfGpio &&) = default;
  SfGpio &operator=(SfGpio &&) = default;

  //==============================================================================
  // THREAD-SAFE DIGITAL GPIO OPERATIONS
  //==============================================================================

  /**
   * @brief Set pin mode (thread-safe).
   *
   * @param pin GPIO pin number
   * @param mode Pin mode (input/output/etc)
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t setPinMode(hf_gpio_num_t pin, hf_gpio_mode_t mode);

  /**
   * @brief Set pin pull mode (thread-safe).
   *
   * @param pin GPIO pin number
   * @param pull Pull mode (none/pullup/pulldown)
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t setPinPull(hf_gpio_num_t pin, hf_gpio_pull_t pull);

  /**
   * @brief Set pin drive capability (thread-safe).
   *
   * @param pin GPIO pin number
   * @param drive_cap Drive capability level
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t setPinDriveCapability(hf_gpio_num_t pin, hf_gpio_drive_cap_t drive_cap);

  /**
   * @brief Set digital output level (thread-safe).
   *
   * @param pin GPIO pin number
   * @param level Output level (true=high, false=low)
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t digitalWrite(hf_gpio_num_t pin, bool level);

  /**
   * @brief Read digital input level (thread-safe).
   *
   * @param pin GPIO pin number
   * @param level Pointer to store the read level
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t digitalRead(hf_gpio_num_t pin, bool *level);

  /**
   * @brief Toggle digital output level (thread-safe).
   *
   * @param pin GPIO pin number
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t digitalToggle(hf_gpio_num_t pin);

  //==============================================================================
  // THREAD-SAFE MULTI-PIN OPERATIONS
  //==============================================================================

  /**
   * @brief Set multiple pins simultaneously (thread-safe).
   *
   * @param pin_mask Bitmask of pins to set
   * @param level_mask Bitmask of levels for each pin
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t writeMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t level_mask);

  /**
   * @brief Read multiple pins simultaneously (thread-safe).
   *
   * @param pin_mask Bitmask of pins to read
   * @param level_mask Pointer to store the read levels
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t readMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t *level_mask);

  //==============================================================================
  // THREAD-SAFE INTERRUPT OPERATIONS
  //==============================================================================

  /**
   * @brief Enable interrupt for pin (thread-safe).
   *
   * @param pin GPIO pin number
   * @param intr_type Interrupt trigger type
   * @param callback Interrupt callback function
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t enableInterrupt(hf_gpio_num_t pin, hf_gpio_intr_type_t intr_type,
                                   BaseGpio::InterruptCallback callback);

  /**
   * @brief Disable interrupt for pin (thread-safe).
   *
   * @param pin GPIO pin number
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t disableInterrupt(hf_gpio_num_t pin);

  //==============================================================================
  // THREAD-SAFE STATUS OPERATIONS
  //==============================================================================

  /**
   * @brief Check if pin is valid (thread-safe).
   *
   * @param pin GPIO pin number
   * @return bool true if pin is valid, false otherwise
   */
  bool isPinValid(hf_gpio_num_t pin);

  /**
   * @brief Get pin mode (thread-safe).
   *
   * @param pin GPIO pin number
   * @param mode Pointer to store the current mode
   * @return hf_return_code_t HF_OK on success, error code on failure
   */
  hf_return_code_t getPinMode(hf_gpio_num_t pin, hf_gpio_mode_t *mode);

  /**
   * @brief Check if GPIO system is initialized (thread-safe).
   *
   * @return bool true if initialized, false otherwise
   */
  bool isInitialized();

private:
  std::shared_ptr<BaseGpio> gpio_impl_; ///< Wrapped GPIO implementation
  SemaphoreHandle_t mutex_;             ///< Mutex for thread safety

  /**
   * @brief Initialize mutex.
   *
   * @return bool true on success, false on failure
   */
  bool initializeMutex();

  /**
   * @brief Cleanup mutex.
   */
  void cleanupMutex();

  /**
   * @brief Lock mutex with timeout.
   *
   * @param timeout_ms Timeout in milliseconds
   * @return bool true if locked, false on timeout
   */
  bool lockMutex(hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT);

  /**
   * @brief Unlock mutex.
   */
  void unlockMutex();
};

} // namespace ThreadSafe
} // namespace HardFOC

#endif // SF_GPIO_H
