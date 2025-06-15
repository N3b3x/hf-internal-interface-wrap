/**
 * @file DigitalInput.h
 * @brief DigitalInput class for describing GPIO input pins on ESP32-C6
 * (ESP-IDF).
 *
 * The DigitalInput class is part of a hardware abstraction layer (HAL)
 * and provides an interface for manipulating GPIO input pins. The class is
 * designed with lazy initialization, i.e., pins are initialized when first
 * manipulated.
 *
 * @note These functions are not thread-safe. Use appropriate guards in ISR or
 * multi-threaded contexts.
 */
#ifndef DIGITALINPUT_H
#define DIGITALINPUT_H

#include "DigitalGpio.h"
#include "driver/gpio.h"

/**
 * @class DigitalInput
 * @brief DigitalInput class for describing GPIO input pins on ESP32-C6.
 */
class DigitalInput : public DigitalGpio {
public:
  /**
   * @brief Constructor creates an instance of DigitalInput with a pin and
   * active state.
   * @param pin GPIO pin number (gpio_num_t)
   * @param activeState Defines the active state of the pin.
   */
  DigitalInput(gpio_num_t pin, ActiveState activeState) noexcept;

  /**
   * @brief Copy constructor is deleted to prevent instance copying.
   */
  DigitalInput(const DigitalInput &) = delete;
  /**
   * @brief Assignment operator is deleted to prevent instance copying.
   */
  DigitalInput &operator=(const DigitalInput &) = delete;
  /**
   * @brief Default destructor.
   */
  virtual ~DigitalInput() override = default;

  /**
   * @brief Checks if the logical pin state is active.
   * @return True if the pin is active, false otherwise.
   */
  bool IsActive() noexcept;

  /**
   * @brief Fetches the current state of the pin.
   * @return The current state of the pin.
   */
  State GetState() noexcept;

private:
  /**
   * @brief Virtual function to initialize the peripheral (configures pin as
   * input).
   * @return True if initialization is successful, false otherwise.
   */
  virtual bool Initialize() noexcept override;
};

#endif // DIGITALINPUT_H
