/**
 * @file BaseGpio.h
 * @brief Unified GPIO base class for all digital GPIO implementations.
 *
 * This file contains the declaration of the BaseGpio abstract class, which provides
 * a comprehensive GPIO abstraction that serves as the base for all GPIO
 * implementations in the HardFOC system. It supports dynamic mode switching,
 * configurable polarity, pull resistors, interrupt handling, and works across
 * different hardware platforms including MCU GPIOs, I2C GPIO expanders,
 * SPI GPIO expanders, and other GPIO hardware.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * 
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <functional>

/**
 * @brief HardFOC GPIO error codes macro list
 * @details X-macro pattern for comprehensive error enumeration. Each entry contains:
 *          X(NAME, VALUE, DESCRIPTION)
 */
#define HF_GPIO_ERR_LIST(X)                                                                        \
  /* Success codes */                                                                              \
  X(GPIO_SUCCESS, 0, "Success")                                                                    \
                                                                                                   \
  /* General errors */                                                                             \
  X(GPIO_ERR_FAILURE, 1, "General failure")                                                        \
  X(GPIO_ERR_NOT_INITIALIZED, 2, "Not initialized")                                                \
  X(GPIO_ERR_ALREADY_INITIALIZED, 3, "Already initialized")                                        \
  X(GPIO_ERR_INVALID_PARAMETER, 4, "Invalid parameter")                                            \
  X(GPIO_ERR_NULL_POINTER, 5, "Null pointer")                                                      \
  X(GPIO_ERR_OUT_OF_MEMORY, 6, "Out of memory")                                                    \
                                                                                                   \
  /* Pin errors */                                                                                 \
  X(GPIO_ERR_INVALID_PIN, 7, "Invalid pin")                                                        \
  X(GPIO_ERR_PIN_NOT_FOUND, 8, "Pin not found")                                                    \
  X(GPIO_ERR_PIN_NOT_CONFIGURED, 9, "Pin not configured")                                          \
  X(GPIO_ERR_PIN_ALREADY_REGISTERED, 10, "Pin already registered")                                 \
  X(GPIO_ERR_PIN_ACCESS_DENIED, 11, "Pin access denied")                                           \
  X(GPIO_ERR_PIN_BUSY, 12, "Pin busy")                                                             \
                                                                                                   \
  /* Hardware errors */                                                                            \
  X(GPIO_ERR_HARDWARE_FAULT, 13, "Hardware fault")                                                 \
  X(GPIO_ERR_COMMUNICATION_FAILURE, 14, "Communication failure")                                   \
  X(GPIO_ERR_DEVICE_NOT_RESPONDING, 15, "Device not responding")                                   \
  X(GPIO_ERR_TIMEOUT, 16, "Timeout")                                                               \
  X(GPIO_ERR_VOLTAGE_OUT_OF_RANGE, 17, "Voltage out of range")                                     \
                                                                                                   \
  /* Configuration errors */                                                                       \
  X(GPIO_ERR_INVALID_CONFIGURATION, 18, "Invalid configuration")                                   \
  X(GPIO_ERR_UNSUPPORTED_OPERATION, 19, "Unsupported operation")                                   \
  X(GPIO_ERR_RESOURCE_BUSY, 20, "Resource busy")                                                   \
  X(GPIO_ERR_RESOURCE_UNAVAILABLE, 21, "Resource unavailable")                                     \
                                                                                                   \
  /* I/O errors */                                                                                 \
  X(GPIO_ERR_READ_FAILURE, 22, "Read failure")                                                     \
  X(GPIO_ERR_WRITE_FAILURE, 23, "Write failure")                                                   \
  X(GPIO_ERR_DIRECTION_MISMATCH, 24, "Direction mismatch")                                         \
  X(GPIO_ERR_PULL_RESISTOR_FAILURE, 25, "Pull resistor failure")                                   \
                                                                                                   \
  /* Interrupt errors */                                                                           \
  X(GPIO_ERR_INTERRUPT_NOT_SUPPORTED, 26, "Interrupt not supported")                               \
  X(GPIO_ERR_INTERRUPT_ALREADY_ENABLED, 27, "Interrupt already enabled")                           \
  X(GPIO_ERR_INTERRUPT_NOT_ENABLED, 28, "Interrupt not enabled")                                   \
  X(GPIO_ERR_INTERRUPT_HANDLER_FAILED, 29, "Interrupt handler failed")                             \
                                                                                                   \
  /* System errors */                                                                              \
  X(GPIO_ERR_SYSTEM_ERROR, 30, "System error")                                                     \
  X(GPIO_ERR_PERMISSION_DENIED, 31, "Permission denied")                                           \
  X(GPIO_ERR_OPERATION_ABORTED, 32, "Operation aborted")                                           \
                                                                                                   \
  /* Extended errors for McuGpio implementation */                                                 \
  X(GPIO_ERR_NOT_SUPPORTED, 33, "Operation not supported")                                         \
  X(GPIO_ERR_DRIVER_ERROR, 34, "Driver error")                                                     \
  X(GPIO_ERR_INVALID_STATE, 35, "Invalid state")                                                   \
  X(GPIO_ERR_INVALID_ARG, 36, "Invalid argument")                                                  \
  X(GPIO_ERR_CALIBRATION_FAILURE, 37, "Calibration failure")

/**
 * @brief HardFOC GPIO error codes
 * @details Comprehensive error enumeration for all GPIO operations in the system.
 */
enum class hf_gpio_err_t : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_GPIO_ERR_LIST(X)
#undef X
      GPIO_ERR_COUNT // Automatically calculated count
};

/**
 * @brief Convert hf_gpio_err_t to human-readable string
 * @param err The error code to convert
 * @return Pointer to error description string
 */
constexpr const char *HfGpioErrToString(hf_gpio_err_t err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case hf_gpio_err_t::NAME:                                                                        \
    return DESC;
    HF_GPIO_ERR_LIST(X)
#undef X
  default:
    return "Unknown error";
  }
}

/**
 * @class BaseGpio
 * @brief Unified GPIO base class for all digital GPIO implementations.
 * @details This class provides a comprehensive digital GPIO implementation that serves
 *          as the base for all GPIO hardware in the HardFOC system. It supports:
 *          - Dynamic mode switching between input and output
 *          - Active-high/active-low polarity support
 *          - Pull resistor configuration
 *          - Push-pull and open-drain output modes
 *          - Comprehensive error handling and validation
 *          - Lazy initialization pattern
 *
 *          Derived classes implement platform-specific details for:
 *          - On-chip GPIO controllers
 *          - I2C GPIO expanders
 *          - SPI GPIO expanders
 *          - Other GPIO hardware
 */
class BaseGpio {
public:
  /**
   * @brief GPIO pin logical states.
   * @details Represents the logical state of a GPIO pin, independent of electrical polarity.
   */
  enum class hf_gpio_state_t : uint8_t {
    HF_GPIO_STATE_INACTIVE = 0, ///< Logical inactive state
    HF_GPIO_STATE_ACTIVE = 1    ///< Logical active state
  };

  /**
   * @brief GPIO active state polarity configuration.
   * @details Defines which electrical level corresponds to the logical "active" state.
   */
  enum class hf_gpio_active_state_t : uint8_t {
    HF_GPIO_ACTIVE_LOW = 0, ///< Active state is electrical low
    HF_GPIO_ACTIVE_HIGH = 1 ///< Active state is electrical high
  };

  /**
   * @brief GPIO pin direction/mode configuration.
   * @details Defines whether the pin is configured as input or output.
   */
  enum class hf_gpio_direction_t : uint8_t {
    HF_GPIO_DIRECTION_INPUT = 0, ///< Pin configured as input
    HF_GPIO_DIRECTION_OUTPUT = 1 ///< Pin configured as output
  };

  /**
   * @brief GPIO output drive modes.
   * @details Defines the electrical characteristics of GPIO output pins.
   */
  enum class hf_gpio_output_mode_t : uint8_t {
    HF_GPIO_OUTPUT_MODE_PUSH_PULL = 0, ///< Push-pull output (strong high and low)
    HF_GPIO_OUTPUT_MODE_OPEN_DRAIN = 1 ///< Open-drain output (strong low, high-impedance high)
  };

  /**
   * @brief GPIO pull resistor configuration.
   * @details Defines the internal pull resistor configuration for GPIO pins.
   */
  enum class hf_gpio_pull_mode_t : uint8_t {
    HF_GPIO_PULL_MODE_FLOATING = 0, ///< No pull resistor (floating/high-impedance)
    HF_GPIO_PULL_MODE_UP = 1,       ///< Internal pull-up resistor enabled
    HF_GPIO_PULL_MODE_DOWN = 2,     ///< Internal pull-down resistor enabled
    HF_GPIO_PULL_MODE_UP_DOWN = 3   ///< Both pull-up and pull-down resistors enabled
  };

  /**
   * @brief GPIO interrupt trigger types.
   * @details Defines the conditions that trigger GPIO interrupts.
   */
  enum class hf_gpio_interrupt_trigger_t : uint8_t {
    HF_GPIO_INTERRUPT_TRIGGER_NONE = 0,        ///< No interrupt (disabled)
    HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE = 1, ///< Trigger on rising edge (low to high)
    HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE = 2, ///< Trigger on falling edge (high to low)
    HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES = 3,  ///< Trigger on both rising and falling edges
    HF_GPIO_INTERRUPT_TRIGGER_LOW_LEVEL = 4,   ///< Trigger on low level
    HF_GPIO_INTERRUPT_TRIGGER_HIGH_LEVEL = 5   ///< Trigger on high level
  };

  /**
   * @brief GPIO interrupt callback function type.
   * @details Callback invoked when GPIO interrupt occurs.
   * @param gpio Pointer to the GPIO instance that triggered
   * @param trigger The trigger type that caused the interrupt
   * @param user_data User-provided data passed to callback
   */
  using InterruptCallback =
      std::function<void(BaseGpio *gpio, hf_gpio_interrupt_trigger_t trigger, void *user_data)>;

  /**
   * @brief GPIO interrupt status structure.
   */
  struct InterruptStatus {
    bool is_enabled;                           ///< Interrupt currently enabled
    hf_gpio_interrupt_trigger_t trigger_type;  ///< Current trigger configuration
    uint32_t interrupt_count;                  ///< Number of interrupts occurred
    bool has_callback;                         ///< Callback function is registered
  };

  //==============================================================//
  // CONSTRUCTORS AND DESTRUCTOR
  //==============================================================//

  /**
   * @brief Constructor for BaseGpio with full configuration.
   * @param pin_num Platform-agnostic GPIO pin identifier
   * @param direction Initial pin direction (Input or Output)
   * @param active_state Polarity configuration (High or Low active)
   * @param output_mode Output drive mode (PushPull or OpenDrain)
   * @param pull_mode Pull resistor configuration (Floating, PullUp, or PullDown)
   * @details Initializes the GPIO with specified configuration. The pin is not
   *          physically configured until Initialize() is called.
   */
  explicit BaseGpio(HfPinNumber pin_num, hf_gpio_direction_t direction = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                    hf_gpio_active_state_t active_state = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                    hf_gpio_output_mode_t output_mode = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                    hf_gpio_pull_mode_t pull_mode = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING) noexcept
      : pin_(pin_num), initialized_(false), current_direction_(direction),
        active_state_(active_state), output_mode_(output_mode), pull_mode_(pull_mode),
        current_state_(hf_gpio_state_t::HF_GPIO_STATE_INACTIVE) {}

  /**
   * @brief Copy constructor is deleted to avoid copying instances.
   */
  BaseGpio(const BaseGpio &copy) = delete;

  /**
   * @brief Assignment operator is deleted to avoid copying instances.
   */
  BaseGpio &operator=(const BaseGpio &copy) = delete;

  /**
   * @brief Virtual destructor for proper cleanup of derived classes.
   */
  virtual ~BaseGpio() = default;

  //==============================================================//
  // INITIALIZATION AND STATUS
  //==============================================================//

  /**
   * @brief Check if the pin is initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Ensures the pin is initialized (lazy initialization).
   * @return true if initialized successfully, false otherwise
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  /**
   * @brief Get the GPIO pin number/identifier.
   * @return Platform-agnostic pin identifier
   */
  [[nodiscard]] HfPinNumber GetPin() const noexcept {
    return pin_;
  }

  //==============================================================//
  // DIRECTION AND MODE MANAGEMENT
  //==============================================================//

  /**
   * @brief Get the current pin direction.
   * @return Current Direction setting (Input or Output)
   */
  [[nodiscard]] hf_gpio_direction_t GetDirection() const noexcept {
    return current_direction_;
  }

  /**
   * @brief Set the pin direction (input or output).
   * @param direction New Direction setting (Input or Output)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t SetDirection(hf_gpio_direction_t direction) noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = SetDirectionImpl(direction);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      current_direction_ = direction;
    }
    return result;
  }

  /**
   * @brief Check if the pin is currently configured as input.
   * @return true if input, false if output
   */
  [[nodiscard]] bool IsInput() const noexcept {
    return current_direction_ == hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT;
  }

  /**
   * @brief Check if the pin is currently configured as output.
   * @return true if output, false if input
   */
  [[nodiscard]] bool IsOutput() const noexcept {
    return current_direction_ == hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT;
  }

  /**
   * @brief Get the output drive mode.
   * @return Current OutputMode setting (PushPull or OpenDrain)
   */
  [[nodiscard]] hf_gpio_output_mode_t GetOutputMode() const noexcept {
    return output_mode_;
  }

  /**
   * @brief Set the output drive mode.
   * @param mode New OutputMode setting (PushPull or OpenDrain)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t SetOutputMode(hf_gpio_output_mode_t mode) noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = SetOutputModeImpl(mode);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      output_mode_ = mode;
    }
    return result;
  }

  //==============================================================//
  // PULL RESISTOR MANAGEMENT
  //==============================================================//

  /**
   * @brief Get the current pull resistor mode.
   * @return Current PullMode setting
   */
  [[nodiscard]] hf_gpio_pull_mode_t GetPullMode() const noexcept {
    return pull_mode_;
  }

  /**
   * @brief Set the pull resistor mode.
   * @param mode New PullMode setting (Floating, PullUp, or PullDown)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t SetPullMode(hf_gpio_pull_mode_t mode) noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = SetPullModeImpl(mode);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      pull_mode_ = mode;
    }
    return result;
  }

  //==============================================================//
  // STATE MANAGEMENT AND I/O OPERATIONS
  //==============================================================//

  /**
   * @brief Get the current logical state of the pin.
   * @return Current State (Active or Inactive)
   */
  [[nodiscard]] hf_gpio_state_t GetCurrentState() const noexcept {
    return current_state_;
  }

  /**
   * @brief Get the active state polarity configuration.
   * @return Current ActiveState setting (High or Low)
   */
  [[nodiscard]] hf_gpio_active_state_t GetActiveState() const noexcept {
    return active_state_;
  }

  /**
   * @brief Set the active state polarity configuration.
   * @param active_state New ActiveState setting (High or Low)
   */
  void SetActiveState(hf_gpio_active_state_t active_state) noexcept {
    active_state_ = active_state;
  }

  //==============================================================//
  // STATE CONTROL METHODS
  //==============================================================//

  /**
   * @brief Set the GPIO to active state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t SetActive() noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = SetActiveImpl();
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      current_state_ = hf_gpio_state_t::HF_GPIO_STATE_ACTIVE;
    }
    return result;
  }

  /**
   * @brief Set the GPIO to inactive state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t SetInactive() noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = SetInactiveImpl();
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      current_state_ = hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
    }
    return result;
  }

  /**
   * @brief Toggle the GPIO state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t Toggle() noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = ToggleImpl();
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      current_state_ = (current_state_ == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? hf_gpio_state_t::HF_GPIO_STATE_INACTIVE : hf_gpio_state_t::HF_GPIO_STATE_ACTIVE;
    }
    return result;
  }

  /**
   * @brief Check if the GPIO is currently active.
   * @param is_active Reference to store the result
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t IsActive(bool &is_active) noexcept {
    hf_gpio_err_t validation = ValidateBasicOperation();
    if (validation != hf_gpio_err_t::GPIO_SUCCESS) {
      return validation;
    }

    hf_gpio_err_t result = IsActiveImpl(is_active);
    if (result == hf_gpio_err_t::GPIO_SUCCESS) {
      current_state_ = is_active ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
    }
    return result;
  }

  //==============================================================//
  // HARDWARE ABSTRACTION INTERFACE
  //==============================================================//

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
   * @brief Get human-readable description of this GPIO instance.
   * @return Pointer to description string
   */
  [[nodiscard]] virtual const char *GetDescription() const noexcept = 0;

  //==============================================================//
  // INTERRUPT FUNCTIONALITY  
  //==============================================================//

  /**
   * @brief Check if this GPIO implementation supports interrupts.
   * @return true if interrupts are supported, false otherwise
   */
  [[nodiscard]] virtual bool SupportsInterrupts() const noexcept {
    return false; // Default implementation - no interrupt support
  }

  /**
   * @brief Configure GPIO interrupt settings.
   * @param trigger Interrupt trigger type
   * @param callback Callback function to invoke on interrupt (optional)
   * @param user_data User data passed to callback (optional)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual hf_gpio_err_t ConfigureInterrupt(hf_gpio_interrupt_trigger_t trigger, 
                                           InterruptCallback callback = nullptr,
                                           void *user_data = nullptr) noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  /**
   * @brief Enable GPIO interrupt.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual hf_gpio_err_t EnableInterrupt() noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  /**
   * @brief Disable GPIO interrupt.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual hf_gpio_err_t DisableInterrupt() noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  /**
   * @brief Wait for GPIO interrupt to occur.
   * @param timeout_ms Timeout in milliseconds (0 = wait forever)
   * @return hf_gpio_err_t::GPIO_SUCCESS if interrupt occurred, error code otherwise
   */
  virtual hf_gpio_err_t WaitForInterrupt(uint32_t timeout_ms = 0) noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  /**
   * @brief Get interrupt status information.
   * @param status Reference to store status information
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual hf_gpio_err_t GetInterruptStatus(InterruptStatus &status) noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  /**
   * @brief Clear interrupt statistics.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual hf_gpio_err_t ClearInterruptStats() noexcept {
    return hf_gpio_err_t::GPIO_ERR_NOT_SUPPORTED;
  }

  //==============================================================//
  // STRING CONVERSION UTILITIES
  //==============================================================//
  static const char *ToString(hf_gpio_state_t state) noexcept;
  static const char *ToString(hf_gpio_active_state_t active_state) noexcept;
  static const char *ToString(hf_gpio_direction_t direction) noexcept;
  static const char *ToString(hf_gpio_output_mode_t output_mode) noexcept;
  static const char *ToString(hf_gpio_pull_mode_t pull_mode) noexcept;
  static const char *ToString(hf_gpio_interrupt_trigger_t trigger) noexcept;

  //==============================================================//
  // PURE VIRTUAL FUNCTIONS - MUST BE IMPLEMENTED BY DERIVED CLASSES
  //==============================================================//

  /**
   * @brief Initialize the GPIO pin with current configuration.
   * @return true if initialization successful, false otherwise
   */
  virtual bool Initialize() noexcept = 0;

  /**
   * @brief Deinitialize the GPIO pin.
   * @return true if deinitialization successful, false otherwise
   */
  virtual bool Deinitialize() noexcept {
    initialized_ = false;
    return true;
  }

protected:
  //==============================================================//
  // PROTECTED HELPER METHODS
  //==============================================================//

  /**
   * @brief Validate basic parameters before GPIO operations.
   * @return hf_gpio_err_t error code
   */
  [[nodiscard]] hf_gpio_err_t ValidateBasicOperation() const noexcept {
    if (!initialized_) {
      return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
    }
    if (!IsPinAvailable()) {
      return hf_gpio_err_t::GPIO_ERR_PIN_ACCESS_DENIED;
    }
    return hf_gpio_err_t::GPIO_SUCCESS;
  }

  /**
   * @brief Convert logical state to electrical level based on polarity.
   * @param state Logical state (Active or Inactive)
   * @return true for electrical high, false for electrical low
   */
  [[nodiscard]] bool StateToLevel(hf_gpio_state_t state) const noexcept {
    bool active_level = (active_state_ == hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
    return (state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? active_level : !active_level;
  }

  /**
   * @brief Convert electrical level to logical state based on polarity.
   * @param level Electrical level (true = high, false = low)
   * @return Logical state (Active or Inactive)
   */
  [[nodiscard]] hf_gpio_state_t LevelToState(bool level) const noexcept {
    bool active_level = (active_state_ == hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
    return (level == active_level) ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
  }

  //==============================================================//
  // VALIDATION HELPERS
  //==============================================================//

  //==============================================================//
  // PURE VIRTUAL IMPLEMENTATIONS - PLATFORM SPECIFIC
  //==============================================================//

  virtual hf_gpio_err_t SetDirectionImpl(hf_gpio_direction_t direction) noexcept = 0;
  virtual hf_gpio_err_t SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept = 0;
  virtual hf_gpio_err_t SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept = 0;
  virtual hf_gpio_err_t SetActiveImpl() noexcept = 0;
  virtual hf_gpio_err_t SetInactiveImpl() noexcept = 0;
  virtual hf_gpio_err_t ToggleImpl() noexcept = 0;
  virtual hf_gpio_err_t IsActiveImpl(bool &is_active) noexcept = 0;
  virtual hf_gpio_pull_mode_t GetPullModeImpl() const noexcept = 0;

protected:
  //==============================================================//
  // MEMBER VARIABLES
  //==============================================================//

  const HfPinNumber pin_;                    ///< GPIO pin number/identifier
  bool initialized_;                         ///< Initialization state flag
  hf_gpio_direction_t current_direction_;    ///< Current pin direction
  hf_gpio_active_state_t active_state_;      ///< Active state polarity
  hf_gpio_output_mode_t output_mode_;        ///< Output drive mode
  hf_gpio_pull_mode_t pull_mode_;            ///< Pull resistor configuration
  hf_gpio_state_t current_state_;            ///< Current logical state
};

//==============================================================//
// STRING CONVERSION IMPLEMENTATIONS
//==============================================================//

inline const char *BaseGpio::ToString(hf_gpio_state_t state) noexcept {
  switch (state) {
  case hf_gpio_state_t::HF_GPIO_STATE_ACTIVE:
    return "Active";
  case hf_gpio_state_t::HF_GPIO_STATE_INACTIVE:
    return "Inactive";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(hf_gpio_active_state_t active_state) noexcept {
  switch (active_state) {
  case hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH:
    return "ActiveHigh";
  case hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW:
    return "ActiveLow";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(hf_gpio_direction_t direction) noexcept {
  switch (direction) {
  case hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT:
    return "Input";
  case hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT:
    return "Output";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(hf_gpio_output_mode_t output_mode) noexcept {
  switch (output_mode) {
  case hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL:
    return "PushPull";
  case hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN:
    return "OpenDrain";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(hf_gpio_pull_mode_t pull_mode) noexcept {
  switch (pull_mode) {
  case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING:
    return "Floating";
  case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP:
    return "PullUp";
  case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_DOWN:
    return "PullDown";
  case hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_UP_DOWN:
    return "PullUpDown";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(hf_gpio_interrupt_trigger_t trigger) noexcept {
  switch (trigger) {
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_NONE:
    return "None";
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE:
    return "RisingEdge";
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE:
    return "FallingEdge";
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES:
    return "BothEdges";
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_LOW_LEVEL:
    return "LowLevel";
  case hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_HIGH_LEVEL:
    return "HighLevel";
  default:
    return "Unknown";
  }
}