/**
 * @file McuDigitalGpio.h * @brief MCU-specific implementation of the unified BaseGpio class.
 *
 * This file provides concrete implementations of the unified BaseGpio class
 * for microcontroller-based GPIO pins. It supports dynamic mode switching,
 * pull resistor configuration, and various output drive modes.
 */
#ifndef MCUDIGITALGPIO_H
#define MCUDIGITALGPIO_H

#include "BaseGpio.h"

/**
 * @class McuDigitalGpio * @brief MCU-specific implementation of unified BaseGpio with dynamic mode switching.
 * @details This class provides a concrete implementation of BaseGpio for MCU-based
 *          GPIO pins. It supports all the features of the unified BaseGpio including:
 *          - Dynamic switching between input and output modes
 *          - Active-high/active-low polarity configuration
 *          - Pull resistor configuration (floating, pull-up, pull-down)
 *          - Output drive modes (push-pull, open-drain)
 *          - Thread-safe state management
 *          
 * @note This class is designed to be platform-agnostic within the MCU domain.
 *       Platform-specific details are handled through conditional compilation.
 */
class McuDigitalGpio : public BaseGpio {
public:
  //==============================================================//
  // CONSTRUCTORS
  //==============================================================//
    /**
   * @brief Constructor for McuDigitalGpio with full configuration.
   * @param pin_num MCU GPIO pin number
   * @param direction Initial pin direction (Input or Output)
   * @param active_state Polarity configuration (High or Low active)
   * @param output_mode Output drive mode (PushPull or OpenDrain)
   * @param pull_mode Pull resistor configuration (Floating, PullUp, or PullDown)
   * @details Creates an MCU GPIO instance with the specified configuration.
   *          The pin is not physically configured until Initialize() is called.
   */
  explicit McuDigitalGpio(hf_gpio_num_t pin_num, 
                          Direction direction = Direction::Input,
                          ActiveState active_state = ActiveState::High,
                          OutputMode output_mode = OutputMode::PushPull,
                          PullMode pull_mode = PullMode::Floating) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup including interrupt resources.
   */
  ~McuDigitalGpio() override;

  //==============================================================//
  // BASEGPIO IMPLEMENTATION
  //==============================================================//

  /**
   * @brief Initialize the MCU GPIO pin with current configuration.
   * @return true if initialization successful, false otherwise
   * @details Configures the physical MCU pin according to the current
   *          direction, pull mode, and output mode settings.
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the MCU GPIO pin.
   * @return true if deinitialization successful, false otherwise
   * @details Resets the pin to a safe default state and marks it as uninitialized.
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Check if the pin is available for GPIO operations.
   * @return true if pin is available, false if reserved for other functions
   * @details Validates that the pin number is valid and not reserved for
   *          special functions like SPI, I2C, etc.
   */
  bool IsPinAvailable() const noexcept override;

  /**
   * @brief Get the maximum number of pins supported by this MCU.
   * @return Maximum pin count for this MCU platform
   */
  uint8_t GetMaxPins() const noexcept override;

  /**
   * @brief Get human-readable description of this GPIO instance.
   * @return String view describing the MCU GPIO
   */
  const char* GetDescription() const noexcept override;

  //==============================================================//
  // INTERRUPT FUNCTIONALITY
  //==============================================================//

  /**
   * @brief MCU GPIO supports interrupts.
   * @return true (all MCU GPIOs support interrupts)
   */
  [[nodiscard]] bool SupportsInterrupts() const noexcept override;

  /**
   * @brief Configure GPIO interrupt settings.
   * @param trigger Interrupt trigger type
   * @param callback Callback function to invoke on interrupt (optional)
   * @param user_data User data passed to callback (optional)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr ConfigureInterrupt(InterruptTrigger trigger, 
                               InterruptCallback callback = nullptr, 
                               void* user_data = nullptr) noexcept override;

  /**
   * @brief Enable GPIO interrupt.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr EnableInterrupt() noexcept override;

  /**
   * @brief Disable GPIO interrupt.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr DisableInterrupt() noexcept override;

  /**
   * @brief Wait for GPIO interrupt with timeout.
   * @param timeout_ms Timeout in milliseconds (0 = no timeout)
   * @return HfGpioErr::GPIO_SUCCESS if interrupt occurred, error code otherwise
   */
  HfGpioErr WaitForInterrupt(uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Get current interrupt status and statistics.
   * @param status Reference to store interrupt status
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr GetInterruptStatus(InterruptStatus& status) noexcept override;

  /**
   * @brief Clear interrupt statistics/counters.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr ClearInterruptStats() noexcept override;

protected:
  //==============================================================//
  // DIGITALGPIO PURE VIRTUAL IMPLEMENTATIONS
  //==============================================================//

  /**
   * @brief Platform-specific implementation for setting pin direction.
   * @param direction Desired pin direction (Input or Output)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Reconfigures the MCU pin as input or output with appropriate
   *          pull resistor and drive mode settings.
   */
  HfGpioErr SetDirectionImpl(Direction direction) noexcept override;

  /**
   * @brief Platform-specific implementation for setting output mode.
   * @param mode Desired output mode (PushPull or OpenDrain)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Changes the output drive characteristics of the MCU pin.
   */
  HfGpioErr SetOutputModeImpl(OutputMode mode) noexcept override;

  /**
   * @brief Platform-specific implementation for setting active state.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Sets the MCU pin to the electrical level corresponding to logical active.
   */
  HfGpioErr SetActiveImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for setting inactive state.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Sets the MCU pin to the electrical level corresponding to logical inactive.
   */
  HfGpioErr SetInactiveImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for toggling pin state.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Toggles the MCU pin between active and inactive states.
   */
  HfGpioErr ToggleImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for reading pin active state.
   * @param is_active Output parameter: true if active, false if inactive
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Reads the current MCU pin electrical level and converts to logical state.
   */
  HfGpioErr IsActiveImpl(bool& is_active) noexcept override;

  /**
   * @brief Platform-specific implementation for setting pull resistor mode.
   * @param mode Desired PullMode configuration (Floating, PullUp, or PullDown)
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Configures the MCU's internal pull resistors.
   */
  HfGpioErr SetPullModeImpl(PullMode mode) noexcept override;

  /**
   * @brief Platform-specific implementation for reading pull resistor mode.
   * @return Current PullMode configuration
   * @details Queries the current pull resistor configuration from MCU registers.
   */
  PullMode GetPullModeImpl() const noexcept override;

private:
  //==============================================================//
  // PRIVATE HELPER METHODS
  //==============================================================//

  /**
   * @brief Convert BaseGpio::InterruptTrigger to platform-specific interrupt type.
   * @param trigger BaseGpio interrupt trigger enum
   * @return Platform-specific interrupt type value
   */
  uint32_t ConvertInterruptTrigger(InterruptTrigger trigger) const noexcept;

  /**
   * @brief Static interrupt service routine handler.
   * @param arg Pointer to McuDigitalGpio instance
   */
  static void IRAM_ATTR InterruptHandler(void* arg) noexcept;

  /**
   * @brief Initialize interrupt semaphore (called from constructor).
   */
  void InitializeInterruptSemaphore() noexcept;

  /**
   * @brief Cleanup interrupt semaphore (called from destructor).
   */
  void CleanupInterruptSemaphore() noexcept;

  uint32_t ConvertPullMode(PullMode pull_mode) const noexcept;
  uint32_t ConvertOutputMode(OutputMode output_mode) const noexcept;
  bool ValidatePinNumber() const noexcept;
  HfGpioErr ApplyConfiguration() noexcept;

  //==============================================================//
  // MEMBER VARIABLES
  //==============================================================//

  // Interrupt state
  InterruptTrigger interrupt_trigger_;      ///< Current interrupt trigger type
  InterruptCallback interrupt_callback_;   ///< User interrupt callback
  void* interrupt_user_data_;              ///< User data for callback
  bool interrupt_enabled_;                 ///< Interrupt currently enabled
  uint32_t interrupt_count_;               ///< Number of interrupts occurred
  void* platform_semaphore_;              ///< Platform-specific semaphore for WaitForInterrupt
};

#endif // MCUDIGITALGPIO_H
