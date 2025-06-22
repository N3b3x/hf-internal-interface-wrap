/**
 * @file DigitalGpio.h
 * @brief Abstract base class DigitalGpio for handling GPIO pins on ESP32-C6
 * (ESP-IDF).
 *
 * The DigitalGpio class provides common features for handling digital GPIO pins
 * using ESP-IDF. It is designed with lazy initialization; pins are initialized
 * when first manipulated.
 *
 * @note These functions are not thread-safe. Use appropriate guards in ISR or
 * multi-threaded contexts.
 */
#ifndef DIGITALGPIO_H
#define DIGITALGPIO_H

#include "BaseGpio.h"
#include "driver/gpio.h"

/**
 * @class DigitalGpio
 * @brief Abstract base class for digital GPIO operations on ESP32-C6.
 * @details This class extends BaseGpio with digital-specific functionality
 *          including active state management and direction control.
 */
class DigitalGpio : public BaseGpio {
public:
  /**
   * @brief Enumeration for setting the mode of the GPIO pin.
   */
  enum class Mode : uint8_t {
    OpenDrain, ///< Open-drain output
    PushPull   ///< Push-pull output
  };

  /**
   * @brief Enumeration for describing the state of the GPIO pin.
   */
  enum class State : uint8_t { Active, Inactive };

  /**
   * @brief Enumeration for describing the active state of the GPIO pin.
   */
  enum class ActiveState : uint8_t {
    High, ///< Active when logic high
    Low   ///< Active when logic low
  };

  /**
   * @brief Enumeration for describing the resistance configuration of the GPIO
   * pin.
   */
  enum class Resistance : uint8_t {
    Floating, ///< No pull-up or pull-down
    PullUp,   ///< Pull-up enabled
    PullDown  ///< Pull-down enabled
  };

  /**
   * @brief Converts Mode enum to string.
   */
  static const char *ToString(Mode mode) noexcept;
  /**
   * @brief Converts State enum to string.
   */
  static const char *ToString(State state) noexcept;
  /**
   * @brief Converts ActiveState enum to string.
   */
  static const char *ToString(ActiveState activeState) noexcept;
  /**
   * @brief Converts Resistance enum to string.
   */
  static const char *ToString(Resistance resistance) noexcept;

protected:
  /**
   * @brief Constructor that initializes an instance with a pin and active
   * state.
   * @param pinArg GPIO pin number (gpio_num_t)
   * @param activeStateArg Defines the active state of the pin.
   */
  DigitalGpio(gpio_num_t pinArg, ActiveState activeStateArg) noexcept
      : BaseGpio(pinArg), activeState_(activeStateArg) {
    // No code at this time
  }

  /**
   * @brief Copy constructor is deleted to prevent instance copying.
   */
  DigitalGpio(const DigitalGpio &copy) = delete;
  /**
   * @brief Assignment operator is deleted to prevent instance copying.
   */
  DigitalGpio &operator=(const DigitalGpio &copy) = delete;
  /**
   * @brief Default destructor.
   */
  virtual ~DigitalGpio() = default;

public:
  // Implement BaseGpio interface with modern error handling
  HfGpioErr SetActive() noexcept override {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }
    return SetActiveImpl();
  }

  HfGpioErr SetInactive() noexcept override {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }
    return SetInactiveImpl();
  }

  HfGpioErr Toggle() noexcept override {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }
    return ToggleImpl();
  }

  HfGpioErr IsActive(bool& is_active) noexcept override {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }
    return IsActiveImpl(is_active);
  }

  std::string_view GetDescription() const noexcept override {    return "DigitalGpio";
  }

protected:
  // Pure virtual methods to be implemented by derived classes
  virtual HfGpioErr SetActiveImpl() noexcept = 0;
  virtual HfGpioErr SetInactiveImpl() noexcept = 0;
  virtual HfGpioErr ToggleImpl() noexcept = 0;
  virtual HfGpioErr IsActiveImpl(bool& is_active) noexcept = 0;

  /**
   * @brief Checks if the pin is active high.
   * @return True if the pin is active high, false otherwise.
   */
  [[nodiscard]] bool IsActiveHigh() const noexcept {
    return activeState_ == ActiveState::High;
  }
  
  /**
   * @brief Checks if the pin is active low.
   * @return True if the pin is active low, false otherwise.
   */
  [[nodiscard]] bool IsActiveLow() const noexcept {
    return activeState_ == ActiveState::Low;
  }

  /**
   * @brief Get the active state configuration.
   * @return The active state of the pin.
   */
  [[nodiscard]] ActiveState GetActiveState() const noexcept {
    return activeState_;
  }

  /**
   * @brief Fetches the resistance configuration of the pin (ESP-IDF style).
   * @return The resistance configuration of the pin.
   */
  virtual Resistance GetResistance() const noexcept;

  /**
   * @brief Get the GPIO direction (input/output).
   * @return Direction of the GPIO pin.
   */
  virtual gpio_mode_t GetDirection() const noexcept = 0;

private:
  const ActiveState activeState_; ///< Pin active state (high or low)
};

#endif // DIGITALGPIO_H
