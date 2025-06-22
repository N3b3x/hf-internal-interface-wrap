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
#include <string_view>

/**
 * @brief HardFOC GPIO error codes
 * @details Comprehensive error enumeration for all GPIO operations in the system.
 *          This enumeration is used across all GPIO-related classes to provide
 *          consistent error reporting and handling.
 */
enum class HfGpioErr : uint8_t {
    // Success codes
    GPIO_SUCCESS = 0,
    
    // General errors
    GPIO_ERR_FAILURE,
    GPIO_ERR_NOT_INITIALIZED,
    GPIO_ERR_ALREADY_INITIALIZED,
    GPIO_ERR_INVALID_PARAMETER,
    GPIO_ERR_NULL_POINTER,
    GPIO_ERR_OUT_OF_MEMORY,
    
    // Pin errors
    GPIO_ERR_INVALID_PIN,
    GPIO_ERR_PIN_NOT_FOUND,
    GPIO_ERR_PIN_NOT_CONFIGURED,
    GPIO_ERR_PIN_ALREADY_REGISTERED,
    GPIO_ERR_PIN_ACCESS_DENIED,
    GPIO_ERR_PIN_BUSY,
    
    // Hardware errors
    GPIO_ERR_HARDWARE_FAULT,
    GPIO_ERR_COMMUNICATION_FAILURE,
    GPIO_ERR_DEVICE_NOT_RESPONDING,
    GPIO_ERR_TIMEOUT,
    GPIO_ERR_VOLTAGE_OUT_OF_RANGE,
    
    // Configuration errors
    GPIO_ERR_INVALID_CONFIGURATION,
    GPIO_ERR_UNSUPPORTED_OPERATION,
    GPIO_ERR_RESOURCE_BUSY,
    GPIO_ERR_RESOURCE_UNAVAILABLE,
    
    // I/O errors
    GPIO_ERR_READ_FAILURE,
    GPIO_ERR_WRITE_FAILURE,
    GPIO_ERR_DIRECTION_MISMATCH,
    GPIO_ERR_PULL_RESISTOR_FAILURE,
    
    // System errors
    GPIO_ERR_SYSTEM_ERROR,
    GPIO_ERR_PERMISSION_DENIED,
    GPIO_ERR_OPERATION_ABORTED,
    
    // Count for validation
    GPIO_ERR_COUNT
};

/**
 * @brief Convert HfGpioErr to human-readable string
 * @param err The error code to convert
 * @return String view of the error description
 */
constexpr std::string_view HfGpioErrToString(HfGpioErr err) noexcept {
    switch (err) {
        case HfGpioErr::GPIO_SUCCESS: return "Success";
        case HfGpioErr::GPIO_ERR_FAILURE: return "General failure";
        case HfGpioErr::GPIO_ERR_NOT_INITIALIZED: return "Not initialized";
        case HfGpioErr::GPIO_ERR_ALREADY_INITIALIZED: return "Already initialized";
        case HfGpioErr::GPIO_ERR_INVALID_PARAMETER: return "Invalid parameter";
        case HfGpioErr::GPIO_ERR_NULL_POINTER: return "Null pointer";
        case HfGpioErr::GPIO_ERR_OUT_OF_MEMORY: return "Out of memory";
        case HfGpioErr::GPIO_ERR_INVALID_PIN: return "Invalid pin";
        case HfGpioErr::GPIO_ERR_PIN_NOT_FOUND: return "Pin not found";
        case HfGpioErr::GPIO_ERR_PIN_NOT_CONFIGURED: return "Pin not configured";
        case HfGpioErr::GPIO_ERR_PIN_ALREADY_REGISTERED: return "Pin already registered";
        case HfGpioErr::GPIO_ERR_PIN_ACCESS_DENIED: return "Pin access denied";
        case HfGpioErr::GPIO_ERR_PIN_BUSY: return "Pin busy";
        case HfGpioErr::GPIO_ERR_HARDWARE_FAULT: return "Hardware fault";
        case HfGpioErr::GPIO_ERR_COMMUNICATION_FAILURE: return "Communication failure";
        case HfGpioErr::GPIO_ERR_DEVICE_NOT_RESPONDING: return "Device not responding";
        case HfGpioErr::GPIO_ERR_TIMEOUT: return "Timeout";
        case HfGpioErr::GPIO_ERR_VOLTAGE_OUT_OF_RANGE: return "Voltage out of range";
        case HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION: return "Invalid configuration";
        case HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION: return "Unsupported operation";
        case HfGpioErr::GPIO_ERR_RESOURCE_BUSY: return "Resource busy";
        case HfGpioErr::GPIO_ERR_RESOURCE_UNAVAILABLE: return "Resource unavailable";
        case HfGpioErr::GPIO_ERR_READ_FAILURE: return "Read failure";
        case HfGpioErr::GPIO_ERR_WRITE_FAILURE: return "Write failure";
        case HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH: return "Direction mismatch";
        case HfGpioErr::GPIO_ERR_PULL_RESISTOR_FAILURE: return "Pull resistor failure";
        case HfGpioErr::GPIO_ERR_SYSTEM_ERROR: return "System error";
        case HfGpioErr::GPIO_ERR_PERMISSION_DENIED: return "Permission denied";
        case HfGpioErr::GPIO_ERR_OPERATION_ABORTED: return "Operation aborted";
        default: return "Unknown error";
    }
}

/**
 * @class BaseGpio
 * @brief Abstract base class for ESP32-C6 GPIO pin abstraction.
 * @details This class provides a common interface for all GPIO implementations
 *          in the HardFOC system. It supports lazy initialization, robust error
 *          handling, and consistent API across different GPIO hardware.
 */
class BaseGpio {
public:
  /**
   * @brief Constructor allocates an instance of the class with a specified pin.
   * @param pinArg GPIO pin number (gpio_num_t)
   */
  explicit BaseGpio(gpio_num_t pinArg) noexcept : pin(pinArg), initialized_(false) {
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
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Ensures the pin is initialized (lazy initialization).
   * @return true if initialized, false otherwise.
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Virtual function to initialize the GPIO pin (must be implemented by
   * derived classes).
   * @return true if able to initialize, false otherwise.
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the GPIO pin.
   * @return true if deinitialization successful, false otherwise.
   */
  virtual bool Deinitialize() noexcept {
    initialized_ = false;
    return true;
  }

  /**
   * @brief Returns the GPIO pin number.
   * @return gpio_num_t pin number.
   */
  [[nodiscard]] gpio_num_t GetPin() const noexcept {
    return pin;
  }

  /**
   * @brief Check if the pin is available for GPIO operations.
   * @return true if pin is available, false if reserved for other functions
   */
  [[nodiscard]] virtual bool IsPinAvailable() const noexcept = 0;

  /**
   * @brief Get the maximum number of pins supported by this GPIO instance.
   * @return Maximum pin count
   */
  [[nodiscard]] virtual uint8_t GetMaxPins() const noexcept = 0;

  /**
   * @brief Validate pin and operation parameters.
   * @param pin_num Pin number to validate
   * @return HfGpioErr error code
   */
  [[nodiscard]] virtual HfGpioErr ValidatePin(gpio_num_t pin_num) const noexcept {
    if (pin_num < 0) {
      return HfGpioErr::GPIO_ERR_INVALID_PIN;
    }
    if (!IsPinAvailable()) {
      return HfGpioErr::GPIO_ERR_PIN_ACCESS_DENIED;
    }
    if (!IsInitialized()) {
      return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }
    return HfGpioErr::GPIO_SUCCESS;
  }

  /**
   * @brief Set pin to active state.
   * @return HfGpioErr error code
   */
  virtual HfGpioErr SetActive() noexcept = 0;

  /**
   * @brief Set pin to inactive state.
   * @return HfGpioErr error code
   */
  virtual HfGpioErr SetInactive() noexcept = 0;

  /**
   * @brief Toggle pin state.
   * @return HfGpioErr error code
   */
  virtual HfGpioErr Toggle() noexcept = 0;

  /**
   * @brief Check if pin is in active state.
   * @param is_active Reference to store the result
   * @return HfGpioErr error code
   */
  virtual HfGpioErr IsActive(bool& is_active) noexcept = 0;

  /**
   * @brief Returns the ESP-IDF pin configuration flags (to be implemented by
   * derived classes if needed).
   * @return uint32_t pin configuration flags.
   */
  [[nodiscard]] virtual uint32_t GetPinConfiguration() const noexcept {
    return 0;
  }

  /**
   * @brief Get human-readable description of this GPIO instance.
   * @return String view describing the GPIO
   */
  [[nodiscard]] virtual std::string_view GetDescription() const noexcept {
    return "BaseGpio";
  }

  /**
   * @brief Check if this GPIO supports interrupts.
   * @return true if interrupts are supported, false otherwise
   */
  [[nodiscard]] virtual bool SupportsInterrupts() const noexcept {
    return false;
  }

protected:
  /**
   * @brief Validate basic parameters before GPIO operations.
   * @return HfGpioErr error code
   */
  [[nodiscard]] HfGpioErr ValidateBasicOperation() const noexcept {
    if (!initialized_) {
      return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }
    if (!IsPinAvailable()) {
      return HfGpioErr::GPIO_ERR_PIN_ACCESS_DENIED;
    }
    return HfGpioErr::GPIO_SUCCESS;
  }

protected:
  const gpio_num_t pin; ///< ESP32-C6 GPIO pin number
  bool initialized_;    ///< Initialization state
};
};

#endif // BASEGPIO_H
