/**
 * @file SfGpio.h
 * @brief Thread-safe GPIO interface wrapper.
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * This header provides a thread-safe wrapper around the base GPIO interface,
 * ensuring atomic operations and thread safety for multi-threaded applications.
 * Uses composition pattern with mutex protection.
 */

#ifndef SF_GPIO_H
#define SF_GPIO_H

#include "../base/BaseGpio.h"
#include "../mcu/McuTypes.h"
#include "../utils/RtosMutex.h"
#include <cstdint>
#include <memory>

using hf_gpio_mask_t = uint32_t;

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

  // Initialization
  HfGpioErr Initialize() noexcept;
  HfGpioErr Deinitialize() noexcept;
  HfGpioErr ConfigurePin(hf_gpio_num_t pin, const GpioPinConfig &config) noexcept;

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
   * @return HfGpioErr error code
   */
  HfGpioErr setPinMode(hf_gpio_num_t pin, hf_gpio_mode_t mode);

  /**
   * @brief Set pin pull mode (thread-safe).
   *
   * @param pin GPIO pin number
   * @param pull Pull mode (none/pullup/pulldown)
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr setPinPull(hf_gpio_num_t pin, hf_gpio_pull_t pull);

  /**
   * @brief Set pin drive capability (thread-safe).
   *
   * @param pin GPIO pin number
   * @param drive_cap Drive capability level
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr setPinDriveCapability(hf_gpio_num_t pin, hf_gpio_drive_cap_t drive_cap);

  /**
   * @brief Set digital output level (thread-safe).
   *
   * @param pin GPIO pin number
   * @param level Output level (true=high, false=low)
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr SetLevel(hf_gpio_num_t pin, bool level);

  /**
   * @brief Read digital input level (thread-safe).
   *
   * @param pin GPIO pin number
   * @param level Pointer to store the read level
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr GetLevel(hf_gpio_num_t pin, bool *level);

  /**
   * @brief Toggle digital output level (thread-safe).
   *
   * @param pin GPIO pin number
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr ToggleLevel(hf_gpio_num_t pin);

  // Additional features
  HfGpioErr SetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t strength);
  HfGpioErr GetDriveStrength(hf_gpio_num_t pin, hf_gpio_drive_cap_t &strength);

  //==============================================================================
  // THREAD-SAFE MULTI-PIN OPERATIONS
  //==============================================================================

  /**
   * @brief Set multiple pins simultaneously (thread-safe).
   *
   * @param pin_mask Bitmask of pins to set
   * @param level_mask Bitmask of levels for each pin
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr writeMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t level_mask);

  /**
   * @brief Read multiple pins simultaneously (thread-safe).
   *
   * @param pin_mask Bitmask of pins to read
   * @param level_mask Pointer to store the read levels
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr readMultiple(hf_gpio_mask_t pin_mask, hf_gpio_mask_t *level_mask);

  //==============================================================================
  // THREAD-SAFE INTERRUPT OPERATIONS
  //==============================================================================

  /**
   * @brief Enable interrupt for pin (thread-safe).
   *
   * @param pin GPIO pin number
   * @param intr_type Interrupt trigger type
   * @param callback Interrupt callback function
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr EnableInterrupt(hf_gpio_num_t pin, hf_gpio_intr_type_t intr_type,
                            BaseGpio::InterruptCallback callback, void *user_data = nullptr);

  /**
   * @brief Disable interrupt for pin (thread-safe).
   *
   * @param pin GPIO pin number
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr DisableInterrupt(hf_gpio_num_t pin);

  //==============================================================================
  // THREAD-SAFE STATUS OPERATIONS
  //==============================================================================

  /**
   * @brief Check if pin is valid (thread-safe).
   *
   * @param pin GPIO pin number
   * @return bool true if pin is valid, false otherwise
   */
  bool IsPinValid(hf_gpio_num_t pin) const noexcept;
  const char *GetErrorString(HfGpioErr error) const noexcept;

  /**
   * @brief Get pin mode (thread-safe).
   *
   * @param pin GPIO pin number
   * @param mode Pointer to store the current mode
   * @return HfGpioErr HF_OK on success, error code on failure
   */
  HfGpioErr GetPinMode(hf_gpio_num_t pin, hf_gpio_mode_t *mode) noexcept;

  /**
   * @brief Check if GPIO system is initialized (thread-safe).
   *
   * @return bool true if initialized, false otherwise
   */
  bool IsInitialized() const noexcept;

private:
  std::shared_ptr<BaseGpio> gpio_impl_; ///< Wrapped GPIO implementation
  mutable RtosMutex mutex_;             ///< Mutex for thread safety
  bool initialized_;                    ///< Initialization state

  // No explicit lock/unlock helpers needed with RtosMutex
};

} // namespace ThreadSafe
} // namespace HardFOC

#endif // SF_GPIO_H
