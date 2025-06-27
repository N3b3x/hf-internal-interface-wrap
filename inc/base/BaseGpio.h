/**
 * @file BaseGpio.h
 * @brief Unified GPIO base class for all digital GPIO implementations.
 *
 * @details This file contains the declaration of the BaseGpio abstract class, which provides
 *          a comprehensive GPIO abstraction that serves as the base for all GPIO
 *          implementations in the HardFOC system. It supports dynamic mode switching,
 *          configurable polarity, pull resistors, interrupt handling, and works across
 *          different hardware platforms including MCU GPIOs, I2C GPIO expanders,
 *          SPI GPIO expanders, and other GPIO hardware.
 *
 * @note This class is not thread-safe. Use appropriate synchronization if
 * accessed from multiple contexts.
 */
#ifndef BASEGPIO_H
#define BASEGPIO_H

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
  X(GPIO_ERR_OPERATION_ABORTED, 32, "Operation aborted")

/**
 * @brief HardFOC GPIO error codes
 * @details Comprehensive error enumeration for all GPIO operations in the system.
 */
enum class HfGpioErr : uint8_t {
#define X(NAME, VALUE, DESC) NAME = VALUE,
  HF_GPIO_ERR_LIST(X)
#undef X
      GPIO_ERR_COUNT // Automatically calculated count
};

/**
 * @brief Convert HfGpioErr to human-readable string
 * @param err The error code to convert
 * @return Pointer to error description string
 */
constexpr const char *HfGpioErrToString(HfGpioErr err) noexcept {
  switch (err) {
#define X(NAME, VALUE, DESC)                                                                       \
  case HfGpioErr::NAME:                                                                            \
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
 *          - MCU GPIOs (ESP32C6, STM32, etc.)
 *          - I2C GPIO expanders (PCAL95555, etc.)
 *          - SPI GPIO expanders
 *          - Other GPIO hardware
 */
class BaseGpio {
public:
  /**
   * @brief GPIO pin logical states.
   * @details Represents the logical state of a GPIO pin, independent of electrical polarity.
   */
  enum class State : uint8_t {
    Inactive = 0, ///< Logical inactive state
    Active = 1    ///< Logical active state
  };

  /**
   * @brief GPIO active state polarity configuration.
   * @details Defines which electrical level corresponds to the logical "active" state.
   */
  enum class ActiveState : uint8_t {
    Low = 0, ///< Active state is electrical low
    High = 1 ///< Active state is electrical high
  };

  /**
   * @brief GPIO pin direction/mode configuration.
   * @details Defines whether the pin is configured as input or output.
   */
  enum class Direction : uint8_t {
    Input = 0, ///< Pin configured as input
    Output = 1 ///< Pin configured as output
  };

  /**
   * @brief GPIO output drive modes.
   * @details Defines the electrical characteristics of GPIO output pins.
   */
  enum class OutputMode : uint8_t {
    PushPull = 0, ///< Push-pull output (strong high and low)
    OpenDrain = 1 ///< Open-drain output (strong low, high-impedance high)
  };

  /**
   * @brief GPIO pull resistor configuration.
   * @details Defines the internal pull resistor configuration for GPIO pins.
   */
  enum class PullMode : uint8_t {
    Floating = 0, ///< No pull resistor (floating/high-impedance)
    PullUp = 1,   ///< Internal pull-up resistor enabled
    PullDown = 2  ///< Internal pull-down resistor enabled
  };

  /**
   * @brief GPIO interrupt trigger types.
   * @details Defines the conditions that trigger GPIO interrupts.
   */
  enum class InterruptTrigger : uint8_t {
    None = 0,        ///< No interrupt (disabled)
    RisingEdge = 1,  ///< Trigger on rising edge (low to high)
    FallingEdge = 2, ///< Trigger on falling edge (high to low)
    BothEdges = 3,   ///< Trigger on both rising and falling edges
    LowLevel = 4,    ///< Trigger on low level
    HighLevel = 5    ///< Trigger on high level
  };

  /**
   * @brief GPIO interrupt callback function type.
   * @details Callback invoked when GPIO interrupt occurs.
   * @param gpio Pointer to the GPIO instance that triggered
   * @param trigger The trigger type that caused the interrupt
   * @param user_data User-provided data passed to callback
   */
  using InterruptCallback =
      std::function<void(BaseGpio *gpio, InterruptTrigger trigger, void *user_data)>;

  /**
   * @brief GPIO interrupt status structure.
   */
  struct InterruptStatus {
    bool is_enabled;               ///< Interrupt currently enabled
    InterruptTrigger trigger_type; ///< Current trigger configuration
    uint32_t interrupt_count;      ///< Number of interrupts occurred
    bool has_callback;             ///< Callback function is registered
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
  explicit BaseGpio(HfPinNumber pin_num, Direction direction = Direction::Input,
                    ActiveState active_state = ActiveState::High,
                    OutputMode output_mode = OutputMode::PushPull,
                    PullMode pull_mode = PullMode::Floating) noexcept
      : pin_(pin_num), initialized_(false), current_direction_(direction),
        active_state_(active_state), output_mode_(output_mode), pull_mode_(pull_mode),
        current_state_(State::Inactive) {}

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
  [[nodiscard]] Direction GetDirection() const noexcept {
    return current_direction_;
  }

  /**
   * @brief Set the pin direction (input or output).
   * @param direction New Direction setting (Input or Output)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr SetDirection(Direction direction) noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    HfGpioErr result = SetDirectionImpl(direction);
    if (result == HfGpioErr::GPIO_SUCCESS) {
      current_direction_ = direction;
    }
    return result;
  }

  /**
   * @brief Check if the pin is currently configured as input.
   * @return true if input, false if output
   */
  [[nodiscard]] bool IsInput() const noexcept {
    return current_direction_ == Direction::Input;
  }

  /**
   * @brief Check if the pin is currently configured as output.
   * @return true if output, false if input
   */
  [[nodiscard]] bool IsOutput() const noexcept {
    return current_direction_ == Direction::Output;
  }

  /**
   * @brief Get the output drive mode.
   * @return Current OutputMode setting (PushPull or OpenDrain)
   */
  [[nodiscard]] OutputMode GetOutputMode() const noexcept {
    return output_mode_;
  }

  /**
   * @brief Set the output drive mode.
   * @param mode New OutputMode setting (PushPull or OpenDrain)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr SetOutputMode(OutputMode mode) noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    HfGpioErr result = SetOutputModeImpl(mode);
    if (result == HfGpioErr::GPIO_SUCCESS) {
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
  [[nodiscard]] PullMode GetPullMode() const noexcept {
    return pull_mode_;
  }

  /**
   * @brief Set the pull resistor mode.
   * @param mode New PullMode setting (Floating, PullUp, or PullDown)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr SetPullMode(PullMode mode) noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    HfGpioErr result = SetPullModeImpl(mode);
    if (result == HfGpioErr::GPIO_SUCCESS) {
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
  [[nodiscard]] State GetCurrentState() const noexcept {
    return current_state_;
  }

  /**
   * @brief Get the active state polarity configuration.
   * @return Current ActiveState setting (High or Low)
   */
  [[nodiscard]] ActiveState GetActiveState() const noexcept {
    return active_state_;
  }

  /**
   * @brief Set the active state polarity configuration.
   * @param active_state New ActiveState setting (High or Low)
   */
  void SetActiveState(ActiveState active_state) noexcept {
    active_state_ = active_state;
  }

  /**
   * @brief Set pin to active state.
   * @return HfGpioErr error code
   */
  HfGpioErr SetActive() noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    if (current_direction_ != Direction::Output) {
      return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

    HfGpioErr result = SetActiveImpl();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      current_state_ = State::Active;
    }
    return result;
  }

  /**
   * @brief Set pin to inactive state.
   * @return HfGpioErr error code
   */
  HfGpioErr SetInactive() noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    if (current_direction_ != Direction::Output) {
      return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

    HfGpioErr result = SetInactiveImpl();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      current_state_ = State::Inactive;
    }
    return result;
  }

  /**
   * @brief Toggle pin state.
   * @return HfGpioErr error code
   */
  HfGpioErr Toggle() noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    if (current_direction_ != Direction::Output) {
      return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

    HfGpioErr result = ToggleImpl();
    if (result == HfGpioErr::GPIO_SUCCESS) {
      current_state_ = (current_state_ == State::Active) ? State::Inactive : State::Active;
    }
    return result;
  }

  /**
   * @brief Check if pin is in active state.
   * @param is_active Reference to store the result
   * @return HfGpioErr error code
   */
  HfGpioErr IsActive(bool &is_active) noexcept {
    HfGpioErr validation = ValidateBasicOperation();
    if (validation != HfGpioErr::GPIO_SUCCESS) {
      return validation;
    }

    HfGpioErr result = IsActiveImpl(is_active);
    if (result == HfGpioErr::GPIO_SUCCESS) {
      current_state_ = is_active ? State::Active : State::Inactive;
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
   * @brief Check if this GPIO supports interrupts.
   * @return true if interrupts are supported, false otherwise
   * @details Base implementation returns false. Derived classes should override
   *          if they support interrupt functionality.
   */
  [[nodiscard]] virtual bool SupportsInterrupts() const noexcept {
    return false;
  }

  /**
   * @brief Configure GPIO interrupt settings.
   * @param trigger Interrupt trigger type
   * @param callback Callback function to invoke on interrupt (optional)
   * @param user_data User data passed to callback (optional)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Sets up interrupt configuration but does not enable it.
   *          Call EnableInterrupt() to actually start interrupt generation.
   */
  virtual HfGpioErr ConfigureInterrupt(InterruptTrigger trigger,
                                       InterruptCallback callback = nullptr,
                                       void *user_data = nullptr) noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  /**
   * @brief Enable GPIO interrupt.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Enables interrupt generation based on previously configured settings.
   *          ConfigureInterrupt() must be called first.
   */
  virtual HfGpioErr EnableInterrupt() noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  /**
   * @brief Disable GPIO interrupt.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Disables interrupt generation but preserves configuration.
   */
  virtual HfGpioErr DisableInterrupt() noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  /**
   * @brief Wait for GPIO interrupt with timeout.
   * @param timeout_ms Timeout in milliseconds (0 = no timeout)
   * @return HfGpioErr::GPIO_SUCCESS if interrupt occurred, error code otherwise
   * @details Blocks until interrupt occurs or timeout expires.
   *          Useful for polled interrupt handling without callbacks.
   */
  virtual HfGpioErr WaitForInterrupt(uint32_t timeout_ms = 0) noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  /**
   * @brief Get current interrupt status and statistics.
   * @param status Reference to store interrupt status
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual HfGpioErr GetInterruptStatus(InterruptStatus &status) noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  /**
   * @brief Clear interrupt statistics/counters.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  virtual HfGpioErr ClearInterruptStats() noexcept {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_SUPPORTED;
  }

  //==============================================================//
  // STRING CONVERSION UTILITIES
  //==============================================================//
  static const char *ToString(State state) noexcept;
  static const char *ToString(ActiveState active_state) noexcept;
  static const char *ToString(Direction direction) noexcept;
  static const char *ToString(OutputMode output_mode) noexcept;
  static const char *ToString(PullMode pull_mode) noexcept;
  static const char *ToString(InterruptTrigger trigger) noexcept;

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

  /**
   * @brief Convert logical state to electrical level based on polarity.
   * @param state Logical state (Active or Inactive)
   * @return true for electrical high, false for electrical low
   */
  [[nodiscard]] bool StateToLevel(State state) const noexcept {
    bool active_level = (active_state_ == ActiveState::High);
    return (state == State::Active) ? active_level : !active_level;
  }

  /**
   * @brief Convert electrical level to logical state based on polarity.
   * @param level Electrical level (true = high, false = low)
   * @return Logical state (Active or Inactive)
   */
  [[nodiscard]] State LevelToState(bool level) const noexcept {
    bool active_level = (active_state_ == ActiveState::High);
    return (level == active_level) ? State::Active : State::Inactive;
  }

  //==============================================================//
  // PURE VIRTUAL IMPLEMENTATIONS - PLATFORM SPECIFIC
  //==============================================================//

  virtual HfGpioErr SetDirectionImpl(Direction direction) noexcept = 0;
  virtual HfGpioErr SetOutputModeImpl(OutputMode mode) noexcept = 0;
  virtual HfGpioErr SetPullModeImpl(PullMode mode) noexcept = 0;
  virtual HfGpioErr SetActiveImpl() noexcept = 0;
  virtual HfGpioErr SetInactiveImpl() noexcept = 0;
  virtual HfGpioErr ToggleImpl() noexcept = 0;
  virtual HfGpioErr IsActiveImpl(bool &is_active) noexcept = 0;

protected:
  //==============================================================//
  // MEMBER VARIABLES
  //==============================================================//

  const HfPinNumber pin_;       ///< GPIO pin number/identifier
  bool initialized_;            ///< Initialization state flag
  Direction current_direction_; ///< Current pin direction
  ActiveState active_state_;    ///< Active state polarity
  OutputMode output_mode_;      ///< Output drive mode
  PullMode pull_mode_;          ///< Pull resistor configuration
  State current_state_;         ///< Current logical state
};

//==============================================================//
// STRING CONVERSION IMPLEMENTATIONS
//==============================================================//

inline const char *BaseGpio::ToString(State state) noexcept {
  switch (state) {
  case State::Active:
    return "Active";
  case State::Inactive:
    return "Inactive";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(ActiveState active_state) noexcept {
  switch (active_state) {
  case ActiveState::High:
    return "ActiveHigh";
  case ActiveState::Low:
    return "ActiveLow";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(Direction direction) noexcept {
  switch (direction) {
  case Direction::Input:
    return "Input";
  case Direction::Output:
    return "Output";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(OutputMode output_mode) noexcept {
  switch (output_mode) {
  case OutputMode::PushPull:
    return "PushPull";
  case OutputMode::OpenDrain:
    return "OpenDrain";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(PullMode pull_mode) noexcept {
  switch (pull_mode) {
  case PullMode::Floating:
    return "Floating";
  case PullMode::PullUp:
    return "PullUp";
  case PullMode::PullDown:
    return "PullDown";
  default:
    return "Unknown";
  }
}

inline const char *BaseGpio::ToString(InterruptTrigger trigger) noexcept {
  switch (trigger) {
  case InterruptTrigger::None:
    return "None";
  case InterruptTrigger::RisingEdge:
    return "RisingEdge";
  case InterruptTrigger::FallingEdge:
    return "FallingEdge";
  case InterruptTrigger::BothEdges:
    return "BothEdges";
  case InterruptTrigger::LowLevel:
    return "LowLevel";
  case InterruptTrigger::HighLevel:
    return "HighLevel";
  default:
    return "Unknown";
  }
}

#endif // BASEGPIO_H
