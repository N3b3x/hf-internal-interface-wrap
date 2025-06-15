/**
 * @file BaseGpio.h
 * @brief Abstract base class for GPIO pin abstraction using ESP-IDF (ESP32-C6).
 *
 * This class provides a hardware abstraction for GPIO pins, supporting lazy
 * initialization and ESP-IDF configuration. It is designed to be inherited by
 * digital input/output classes.
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */
#ifndef BASEGPIO_H
#define BASEGPIO_H

#include "driver/gpio.h"
#include <cstdint>

/**
 * @class BaseGpio
 * @brief Abstract base class for ESP32-C6 GPIO pin abstraction.
 */
class BaseGpio {
public:
  /**
   * @brief Constructor allocates an instance of the class with a specified pin.
   * @param pinArg GPIO pin number (gpio_num_t)
   */
  explicit BaseGpio(gpio_num_t pinArg) noexcept
      : pin(pinArg), initialized(false) {
    // No code at this time
  }

  /**
   * @brief Copy constructor is deleted to avoid copying instances.
   */
  BaseGpio(const BaseGpio &copy) = delete;

  /**
   * @brief Assignment operator is deleted to avoid copying instances.
   */
  BaseGpio &operator=(const BaseGpio &copy) = delete;

  /**
   * @brief Virtual destructor.
   */
  virtual ~BaseGpio() = default;

  /**
   * @brief Returns true if the pin is initialized.
   */
  bool IsInitialized() const noexcept { return initialized; }

  /**
   * @brief Ensures the pin is initialized (lazy initialization).
   * @return true if initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized) {
      initialized = Initialize();
    }
    return initialized;
  }

  /**
   * @brief Virtual function to initialize the GPIO pin (must be implemented by
   * derived classes).
   * @return true if able to initialize, false otherwise.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Returns the GPIO pin number.
   * @return gpio_num_t pin number.
   */
  gpio_num_t GetPin() const noexcept { return pin; }

  /**
   * @brief Returns the ESP-IDF pin configuration flags (to be implemented by
   * derived classes if needed).
   * @return uint32_t pin configuration flags.
   */
  virtual uint32_t GetPinConfiguration() const noexcept { return 0; }

protected:
  const gpio_num_t pin; ///< ESP32-C6 GPIO pin number
  bool initialized;     ///< Initialization state
};

#endif // BASEGPIO_H
