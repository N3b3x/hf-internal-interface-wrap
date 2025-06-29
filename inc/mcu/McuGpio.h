/**
 * @file McuGpio.h
 * @brief Advanced MCU-specific implementation of the unified BaseGpio class with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This file provides concrete implementations of the unified BaseGpio class
 * for microcontroller-based GPIO pins with support for both basic and advanced features.
 * It supports dynamic mode switching, pull resistor configuration, various output drive modes,
 * and advanced ESP32C6-specific features like glitch filtering, power management, and RTC GPIO.
 * The implementation includes interrupt handling, debouncing, and hardware-accelerated operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#pragma once

#include "BaseGpio.h"
#include "McuTypes.h"

//--------------------------------------
//  Advanced GPIO Configuration Types
//--------------------------------------

/**
 * @brief GPIO glitch filter types supported by ESP32C6.
 */
enum class GpioGlitchFilterType : uint8_t {
  None = 0, ///< No glitch filter
  Pin = 1,  ///< Pin glitch filter (fixed 2 clock cycles)
  Flex = 2, ///< Flexible glitch filter (configurable)
  Both = 3  ///< Both pin and flex filters (maximum protection)
};

/**
 * @brief GPIO drive capability levels for ESP32C6.
 */
enum class GpioDriveCapability : uint8_t {
  Weak = 0,     ///< ~5mA drive capability
  Stronger = 1, ///< ~10mA drive capability
  Medium = 2,   ///< ~20mA drive capability (default)
  Strongest = 3 ///< ~40mA drive capability
};

/**
 * @brief GPIO sleep configuration for power management.
 */
struct GpioSleepConfig {
  BaseGpio::Direction sleep_direction; ///< Direction during sleep
  BaseGpio::PullMode sleep_pull_mode;  ///< Pull resistors during sleep
  bool sleep_output_enable;            ///< Output enabled during sleep
  bool sleep_input_enable;             ///< Input enabled during sleep
  bool hold_during_sleep;              ///< Hold configuration during sleep
};

/**
 * @brief Flexible glitch filter configuration.
 */
struct FlexGlitchFilterConfig {
  uint32_t window_width_ns;     ///< Sample window width in nanoseconds
  uint32_t window_threshold_ns; ///< Threshold for filtering in nanoseconds
  bool enable_on_init;          ///< Enable filter immediately after creation
};

/**
 * @brief GPIO wake-up configuration for deep sleep.
 */
struct GpioWakeUpConfig {
  BaseGpio::InterruptTrigger wake_trigger; ///< Wake-up trigger type
  bool enable_rtc_wake;                    ///< Enable RTC domain wake-up
  bool enable_ext1_wake;                   ///< Enable EXT1 wake-up source
  uint8_t wake_level;                      ///< Wake-up level (0=low, 1=high)
};

/**
 * @brief GPIO configuration dump information.
 */
struct GpioConfigDump {
  uint8_t pin_number;                   ///< GPIO pin number
  BaseGpio::Direction direction;        ///< Current direction
  BaseGpio::PullMode pull_mode;         ///< Current pull mode
  BaseGpio::OutputMode output_mode;     ///< Current output mode
  GpioDriveCapability drive_capability; ///< Current drive capability
  bool input_enabled;                   ///< Input buffer enabled
  bool output_enabled;                  ///< Output buffer enabled
  bool open_drain;                      ///< Open drain mode
  bool sleep_sel_enabled;               ///< Sleep selection enabled
  uint32_t function_select;             ///< IOMUX function selection
  bool is_rtc_gpio;                     ///< Pin supports RTC GPIO
  bool glitch_filter_enabled;           ///< Glitch filter enabled
  GpioGlitchFilterType filter_type;     ///< Type of glitch filter
};

/**
 * @class McuGpio
 * @brief Advanced MCU-specific implementation of unified BaseGpio with ESP32C6/ESP-IDF v5.5+
 * features.
 * @details This class provides a comprehensive implementation of BaseGpio for MCU-based
 *          GPIO pins with support for both basic and advanced features including:
 *
 *          **Basic Features:**
 *          - Dynamic switching between input and output modes
 *          - Active-high/active-low polarity configuration
 *          - Pull resistor configuration (floating, pull-up, pull-down)
 *          - Output drive modes (push-pull, open-drain)
 *          - Thread-safe state management
 *
 *          **Advanced Features (ESP32C6/ESP-IDF v5.5+):**
 *          - Glitch filtering (pin and flexible filters)
 *          - RTC GPIO support for ultra-low power operations
 *          - Sleep configuration and state retention
 *          - Hold functions to maintain state during sleep
 *          - Deep sleep wake-up configuration
 *          - Precise drive capability control (5mA to 40mA)
 *          - Advanced debugging and configuration dump
 *
 * @note This class is designed to be platform-agnostic within the MCU domain.
 *       Platform-specific details are handled through conditional compilation.
 * @note Advanced features require ESP32C6 with ESP-IDF v5.5+ for full functionality.
 */
class McuGpio : public BaseGpio {
public:
  //==============================================================//
  // CONSTRUCTORS
  //==============================================================//
  /**
   * @brief Constructor for McuGpio with full configuration including advanced features.
   * @param pin_num Platform-agnostic GPIO pin number
   * @param direction Initial pin direction (Input or Output)
   * @param active_state Polarity configuration (High or Low active)
   * @param output_mode Output drive mode (PushPull or OpenDrain)
   * @param pull_mode Pull resistor configuration (Floating, PullUp, or PullDown)
   * @param drive_capability Drive strength capability (Weak to Strongest)
   * @details Creates an MCU GPIO instance with the specified configuration including
   *          advanced features support. The pin is not physically configured until
   *          Initialize() is called. The platform-agnostic pin number is converted
   *          internally to MCU-specific type.
   */
  explicit McuGpio(HfPinNumber pin_num, Direction direction = Direction::Input,
                   ActiveState active_state = ActiveState::High,
                   OutputMode output_mode = OutputMode::PushPull,
                   PullMode pull_mode = PullMode::Floating,
                   GpioDriveCapability drive_capability = GpioDriveCapability::Medium) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup including interrupt resources.
   */
  ~McuGpio() override;

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
  const char *GetDescription() const noexcept override;

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
  HfGpioErr ConfigureInterrupt(InterruptTrigger trigger, InterruptCallback callback = nullptr,
                               void *user_data = nullptr) noexcept override;

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
  HfGpioErr GetInterruptStatus(InterruptStatus &status) noexcept override;

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
  HfGpioErr IsActiveImpl(bool &is_active) noexcept override;

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

  //==============================================================//
  // ADVANCED GPIO FEATURES (ESP32C6/ESP-IDF v5.5+)
  //==============================================================//

  /**
   * @brief Get current drive capability setting.
   * @return Current drive capability level
   */
  [[nodiscard]] GpioDriveCapability GetDriveCapability() const noexcept {
    return drive_capability_;
  }

  /**
   * @brief Set GPIO drive capability.
   * @param capability New drive capability level
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Controls the output drive strength from ~5mA (Weak) to ~40mA (Strongest).
   *          Higher drive capability allows for faster switching and driving larger loads
   *          but increases power consumption and EMI.
   */
  HfGpioErr SetDriveCapability(GpioDriveCapability capability) noexcept;

  /**
   * @brief Check if glitch filters are supported.
   * @return true if glitch filters are available
   */
  [[nodiscard]] bool SupportsGlitchFilter() const noexcept;

  /**
   * @brief Configure pin glitch filter (fixed 2 clock cycles).
   * @param enable Enable or disable the pin glitch filter
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Pin glitch filter removes pulses shorter than 2 IO_MUX clock cycles.
   *          This is a simple, low-overhead filter suitable for basic noise rejection.
   */
  HfGpioErr ConfigurePinGlitchFilter(bool enable) noexcept;

  /**
   * @brief Configure flexible glitch filter with custom timing.
   * @param config Flexible glitch filter configuration
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Flexible glitch filter allows precise control over filtering parameters.
   *          Pulses shorter than window_threshold_ns within window_width_ns are filtered.
   */
  HfGpioErr ConfigureFlexGlitchFilter(const FlexGlitchFilterConfig &config) noexcept;

  /**
   * @brief Enable all configured glitch filters.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr EnableGlitchFilters() noexcept;

  /**
   * @brief Disable all glitch filters.
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   */
  HfGpioErr DisableGlitchFilters() noexcept;

  /**
   * @brief Check if pin supports RTC GPIO functionality.
   * @return true if pin supports RTC GPIO
   */
  [[nodiscard]] bool SupportsRtcGpio() const noexcept;

  /**
   * @brief Configure GPIO sleep behavior.
   * @param config Sleep configuration parameters
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Configures how the GPIO behaves during sleep modes.
   *          Essential for power-optimized applications.
   */
  HfGpioErr ConfigureSleep(const GpioSleepConfig &config) noexcept;

  /**
   * @brief Enable GPIO hold function.
   * @param enable Enable or disable hold function
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Hold function maintains GPIO state during sleep and reset.
   *          Useful for maintaining critical pin states during power transitions.
   */
  HfGpioErr ConfigureHold(bool enable) noexcept;

  /**
   * @brief Configure GPIO as wake-up source.
   * @param config Wake-up configuration parameters
   * @return HfGpioErr::GPIO_SUCCESS if successful, error code otherwise
   * @details Enables GPIO to wake the system from deep sleep.
   *          Essential for battery-powered applications.
   */
  HfGpioErr ConfigureWakeUp(const GpioWakeUpConfig &config) noexcept;

  /**
   * @brief Get comprehensive GPIO configuration information.
   * @return Complete configuration dump structure
   * @details Provides detailed information about current GPIO configuration.
   *          Useful for debugging and system validation.
   */
  [[nodiscard]] GpioConfigDump GetConfigurationDump() const noexcept;

  /**
   * @brief Check if pin is currently held.
   * @return true if pin is in hold state
   */
  [[nodiscard]] bool IsHeld() const noexcept;

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
   * @param arg Pointer to McuGpio instance
   */
  static void IRAM_ATTR InterruptHandler(void *arg) noexcept;

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
  InterruptTrigger interrupt_trigger_;   ///< Current interrupt trigger type
  InterruptCallback interrupt_callback_; ///< User interrupt callback
  void *interrupt_user_data_;            ///< User data for callback
  bool interrupt_enabled_;               ///< Interrupt currently enabled
  uint32_t interrupt_count_;             ///< Number of interrupts occurred
  void *platform_semaphore_;             ///< Platform-specific semaphore for WaitForInterrupt

  // Advanced GPIO state
  GpioDriveCapability drive_capability_;      ///< Current drive capability setting
  GpioGlitchFilterType glitch_filter_type_;   ///< Type of glitch filter configured
  bool pin_glitch_filter_enabled_;            ///< Pin glitch filter enabled
  bool flex_glitch_filter_enabled_;           ///< Flexible glitch filter enabled
  FlexGlitchFilterConfig flex_filter_config_; ///< Flexible filter configuration
  GpioSleepConfig sleep_config_;              ///< Sleep configuration
  bool hold_enabled_;                         ///< Hold function enabled
  bool rtc_gpio_enabled_;                     ///< RTC GPIO functionality enabled
  GpioWakeUpConfig wakeup_config_;            ///< Wake-up configuration

  // Platform-specific handles for advanced features
  void *glitch_filter_handle_; ///< Platform-specific glitch filter handle
  void *rtc_gpio_handle_;      ///< Platform-specific RTC GPIO handle
};
