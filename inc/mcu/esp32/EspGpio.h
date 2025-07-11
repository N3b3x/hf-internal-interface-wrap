/**
 * @file EspGpio.h
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

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include <esp_attr.h>

#ifdef __cplusplus
}
#endif

// C++ headers
#include "BaseGpio.h"
#include "EspTypes_GPIO.h"
#include "mcu/esp32/utils/EspTypes_GPIO.h"
#include <stdio.h>

/**
 * @class EspGpio
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
class EspGpio : public BaseGpio {
public:
  //==============================================================//
  // CONSTRUCTORS
  //==============================================================//  
  /**
   * @brief Constructor for EspGpio with full configuration including advanced features.
   * @param pin_num Platform-agnostic GPIO pin number
   * @param direction Initial pin direction (Input or Output)
   * @param active_state Polarity configuration (High or Low active)
   * @param output_mode Output drive mode (PushPull or OpenDrain)
   * @param pull_mode Pull resistor configuration (Floating, PullUp, or PullDown)
   * @param drive_capability Drive strength capability (Weak to Strongest)
   * @details Creates an MCU GPIO instance with the specified configuration.
   *          **LAZY INITIALIZATION**: The pin is NOT physically configured until
   *          the first call to EnsureInitialized(), Initialize(), or any GPIO operation.
   *          This allows creating GPIO objects without immediate hardware access.
   */
  explicit EspGpio(hf_pin_num_t pin_num, hf_gpio_direction_t direction = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                   hf_gpio_active_state_t active_state = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                   hf_gpio_output_mode_t output_mode = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                   hf_gpio_pull_mode_t pull_mode = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                   hf_gpio_drive_cap_t drive_capability = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup including interrupt resources.
   */
  ~EspGpio() override;

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
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t ConfigureInterrupt(hf_gpio_interrupt_trigger_t trigger, InterruptCallback callback = nullptr,
                               void *user_data = nullptr) noexcept override;

  /**
   * @brief Enable GPIO interrupt.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t EnableInterrupt() noexcept override;

  /**
   * @brief Disable GPIO interrupt.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t DisableInterrupt() noexcept override;

  /**
   * @brief Wait for GPIO interrupt with timeout.
   * @param timeout_ms Timeout in milliseconds (0 = no timeout)
   * @return hf_gpio_err_t::GPIO_SUCCESS if interrupt occurred, error code otherwise
   */
  hf_gpio_err_t WaitForInterrupt(uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Get current interrupt status and statistics.
   * @param status Reference to store interrupt status
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t GetInterruptStatus(InterruptStatus &status) noexcept override;

  /**
   * @brief Clear interrupt statistics/counters.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t ClearInterruptStats() noexcept override;

  //==============================================================//
  // LAZY INITIALIZATION SUPPORT  
  //==============================================================//

  // Note: EnsureInitialized() is inherited from BaseGpio and provides lazy initialization

  /**
   * @brief Check if the GPIO pin has been initialized.
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

  //==============================================================//
  // STATISTICS AND DIAGNOSTICS
  //==============================================================//

  /**
   * @brief Get GPIO operation statistics.
   * @param statistics Reference to store current statistics
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t GetStatistics(hf_gpio_statistics_t &statistics) const noexcept override;

  /**
   * @brief Get GPIO diagnostics information.
   * @param diagnostics Reference to store current diagnostics
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t GetDiagnostics(hf_gpio_diagnostics_t &diagnostics) const noexcept override;

protected:
  //==============================================================//
  // DIGITALGPIO PURE VIRTUAL IMPLEMENTATIONS
  //==============================================================//

  /**
   * @brief Platform-specific implementation for setting pin direction.
   * @param direction Desired pin direction (Input or Output)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Reconfigures the MCU pin as input or output with appropriate
   *          pull resistor and drive mode settings.
   */
  hf_gpio_err_t SetDirectionImpl(hf_gpio_direction_t direction) noexcept override;

  /**
   * @brief Platform-specific implementation for setting output mode.
   * @param mode Desired output mode (PushPull or OpenDrain)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Changes the output drive characteristics of the MCU pin.
   */
  hf_gpio_err_t SetOutputModeImpl(hf_gpio_output_mode_t mode) noexcept override;

  /**
   * @brief Platform-specific implementation for setting active state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Sets the MCU pin to the electrical level corresponding to logical active.
   */
  hf_gpio_err_t SetActiveImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for setting inactive state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Sets the MCU pin to the electrical level corresponding to logical inactive.
   */
  hf_gpio_err_t SetInactiveImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for toggling pin state.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Toggles the MCU pin between active and inactive states.
   */
  hf_gpio_err_t ToggleImpl() noexcept override;

  /**
   * @brief Platform-specific implementation for reading pin active state.
   * @param is_active Output parameter: true if active, false if inactive
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Reads the current MCU pin electrical level and converts to logical state.
   */
  hf_gpio_err_t IsActiveImpl(bool &is_active) noexcept override;

  /**
   * @brief Platform-specific implementation for setting pull resistor mode.
   * @param mode Desired PullMode configuration (Floating, PullUp, or PullDown)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Configures the MCU's internal pull resistors.
   */
  hf_gpio_err_t SetPullModeImpl(hf_gpio_pull_mode_t mode) noexcept override;

  /**
   * @brief Platform-specific implementation for reading pull resistor mode.
   * @return Current PullMode configuration
   * @details Queries the current pull resistor configuration from MCU registers.
   */
  hf_gpio_pull_mode_t GetPullModeImpl() const noexcept override;

  //==============================================================//
  // ADVANCED GPIO FEATURES (ESP32C6/ESP-IDF v5.5+)
  //==============================================================//

  /**
   * @brief Get current drive capability setting.
   * @return Current drive capability level
   */
  [[nodiscard]] hf_gpio_drive_cap_t GetDriveCapability() const noexcept {
    return drive_capability_;
  }

  /**
   * @brief Set GPIO drive capability.
   * @param capability New drive capability level
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Controls the output drive strength from ~5mA (Weak) to ~40mA (Strongest).
   *          Higher drive capability allows for faster switching and driving larger loads
   *          but increases power consumption and EMI.
   */
  hf_gpio_err_t SetDriveCapability(hf_gpio_drive_cap_t capability) noexcept;

  /**
   * @brief Check if glitch filters are supported.
   * @return true if glitch filters are available
   */
  [[nodiscard]] bool SupportsGlitchFilter() const noexcept;

  /**
   * @brief Configure pin glitch filter (fixed 2 clock cycles).
   * @param enable Enable or disable the pin glitch filter
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Pin glitch filter removes pulses shorter than 2 IO_MUX clock cycles.
   *          This is a simple, low-overhead filter suitable for basic noise rejection.
   */
  hf_gpio_err_t ConfigurePinGlitchFilter(bool enable) noexcept;

  /**
   * @brief Configure advanced glitch filter (pin/flex) for ESP32C6.
   * @param filter_type Glitch filter type (none, pin, flex, both)
   * @param flex_config Optional pointer to flexible filter config
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t ConfigureGlitchFilter(hf_gpio_glitch_filter_type_t filter_type,
                                      const hf_gpio_flex_filter_config_t *flex_config = nullptr) noexcept;

  /**
   * @brief Configure sleep mode for ESP32C6 GPIO.
   * @param sleep_config Sleep configuration struct
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t ConfigureSleepMode(const hf_gpio_sleep_config_t &sleep_config) noexcept;

  /**
   * @brief Configure flexible glitch filter with custom timing.
   * @param config Flexible glitch filter configuration
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Flexible glitch filter allows precise control over filtering parameters.
   *          Pulses shorter than window_threshold_ns within window_width_ns are filtered.
   */
  hf_gpio_err_t ConfigureFlexGlitchFilter(const hf_gpio_flex_filter_config_t &config) noexcept;

  /**
   * @brief Enable all configured glitch filters.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t EnableGlitchFilters() noexcept;

  /**
   * @brief Disable all glitch filters.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  hf_gpio_err_t DisableGlitchFilters() noexcept;

  /**
   * @brief Check if pin supports RTC GPIO functionality.
   * @return true if pin supports RTC GPIO
   */
  [[nodiscard]] bool SupportsRtcGpio() const noexcept;

  /**
   * @brief Configure GPIO sleep behavior.
   * @param config Sleep configuration parameters
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Configures how the GPIO behaves during sleep modes.
   *          Essential for power-optimized applications.
   */
  hf_gpio_err_t ConfigureSleep(const hf_gpio_sleep_config_t &config) noexcept;

  /**
   * @brief Enable GPIO hold function.
   * @param enable Enable or disable hold function
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Hold function maintains GPIO state during sleep and reset.
   *          Useful for maintaining critical pin states during power transitions.
   */
  hf_gpio_err_t ConfigureHold(bool enable) noexcept;

  /**
   * @brief Configure GPIO as wake-up source.
   * @param config Wake-up configuration parameters
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Enables GPIO to wake the system from deep sleep.
   *          Essential for battery-powered applications.
   */
  hf_gpio_err_t ConfigureWakeUp(const hf_gpio_wakeup_config_t &config) noexcept;

  /**
   * @brief Get comprehensive GPIO configuration information.
   * @return Complete configuration dump structure
   * @details Provides detailed information about current GPIO configuration.
   *          Useful for debugging and system validation.
   */
  [[nodiscard]] hf_gpio_status_info_t GetConfigurationDump() const noexcept;

  /**
   * @brief Check if pin is currently held.
   * @return true if pin is in hold state
   */
  [[nodiscard]] bool IsHeld() const noexcept;

  //==============================================================//
  // ETM (EVENT TASK MATRIX) ADVANCED FEATURES (ESP32C6 ESP-IDF v5.5+)
  //==============================================================//

  /**
   * @brief Configure ETM (Event Task Matrix) for hardware-level GPIO operations.
   * @param etm_config ETM configuration including event and task setup
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details ETM enables zero-latency hardware-level GPIO operations triggered by events.
   *          Perfect for precise timing requirements in motor control and signal processing.
   */
  hf_gpio_err_t ConfigureETM(const hf_gpio_etm_config_t &etm_config) noexcept;

  /**
   * @brief Enable ETM channel for this GPIO.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Enables the configured ETM channel to start responding to events.
   */
  hf_gpio_err_t EnableETM() noexcept;

  /**
   * @brief Disable ETM channel for this GPIO.
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Disables the ETM channel while preserving configuration.
   */
  hf_gpio_err_t DisableETM() noexcept;

  /**
   * @brief Check if pin supports ETM functionality.
   * @return true if pin supports ETM (Event Task Matrix)
   * @details All ESP32C6 GPIO pins support ETM functionality.
   */
  [[nodiscard]] bool SupportsETM() const noexcept;

  /**
   * @brief Get ETM status and configuration information.
   * @param status Output structure to store ETM status
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Provides comprehensive ETM status for monitoring and debugging.
   */
  hf_gpio_err_t GetETMStatus(hf_gpio_etm_status_t &status) const noexcept;

  /**
   * @brief Get current ETM channel usage across all GPIO instances.
   * @return Number of ETM channels currently in use
   * @details Static method for system-wide ETM resource monitoring.
   */
  static uint8_t GetETMChannelUsage() noexcept;

  /**
   * @brief Get maximum number of ETM channels available.
   * @return Maximum ETM channels supported by hardware
   * @details ESP32C6 supports up to 50 ETM channels.
   */
  static uint8_t GetMaxETMChannels() noexcept;

  /**
   * @brief Dump complete ETM configuration to output stream.
   * @param output_stream Output stream for dump (default: stdout)
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Provides comprehensive ETM system dump for debugging and analysis.
   */
  static hf_gpio_err_t DumpETMConfiguration(FILE* output_stream = nullptr) noexcept;

  //==============================================================//
  // STATIC UTILITY METHODS FOR ESP32C6 PIN VALIDATION
  //==============================================================//

  /**
   * @brief Get pin capabilities for comprehensive feature detection.
   * @param capabilities Output structure to store pin capabilities
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Provides comprehensive information about pin capabilities including
   *          ADC, RTC, touch, strapping, and special function support.
   */
  hf_gpio_err_t GetPinCapabilities(hf_gpio_pin_capabilities_t &capabilities) const noexcept;

  /**
   * @brief Get detailed status information for diagnostics.
   * @param status Output structure to store status information
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Provides comprehensive status information for debugging and monitoring.
   */
  hf_gpio_err_t GetStatusInfo(hf_gpio_status_info_t &status) const noexcept;

  //==============================================================//
  // DEDICATED GPIO SUPPORT (ESP32C6 ESP-IDF v5.5+)
  //==============================================================//

  /**
   * @brief Check if pin supports dedicated GPIO functionality.
   * @return true if pin supports dedicated GPIO
   * @details Dedicated GPIO provides high-speed bit-banging capabilities.
   */
  [[nodiscard]] bool SupportsDedicatedGpio() const noexcept;

  /**
   * @brief Create a dedicated GPIO bundle for high-speed operations.
   * @param config Dedicated GPIO bundle configuration
   * @param bundle_handle Output handle for the created bundle
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   * @details Dedicated GPIO bundles enable high-speed bit-banging operations
   *          with minimal CPU overhead, perfect for custom protocols.
   */
  static hf_gpio_err_t CreateDedicatedBundle(const hf_dedic_gpio_bundle_config_t &config,
                                             hf_dedic_gpio_bundle_handle_t &bundle_handle) noexcept;

  /**
   * @brief Delete a dedicated GPIO bundle.
   * @param bundle_handle Handle of the bundle to delete
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  static hf_gpio_err_t DeleteDedicatedBundle(hf_dedic_gpio_bundle_handle_t bundle_handle) noexcept;

  /**
   * @brief Read data from a dedicated GPIO bundle.
   * @param bundle_handle Handle of the bundle to read from
   * @param data Output data from the bundle
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  static hf_gpio_err_t ReadDedicatedBundle(hf_dedic_gpio_bundle_handle_t bundle_handle,
                                           hf_dedic_gpio_bundle_data_t &data) noexcept;

  /**
   * @brief Write data to a dedicated GPIO bundle.
   * @param bundle_handle Handle of the bundle to write to
   * @param data Data to write to the bundle
   * @return hf_gpio_err_t::GPIO_SUCCESS if successful, error code otherwise
   */
  static hf_gpio_err_t WriteDedicatedBundle(hf_dedic_gpio_bundle_handle_t bundle_handle,
                                            hf_dedic_gpio_bundle_data_t data) noexcept;

  /**
   * @brief Get total interrupt count across all GPIO instances.
   * @return Total number of GPIO interrupts that have occurred
   * @details Thread-safe global interrupt counter for system monitoring.
   */
  static uint32_t GetTotalInterruptCount() noexcept;

  /**
   * @brief Get count of currently active GPIO instances.
   * @return Number of GPIO instances currently initialized and active
   * @details Thread-safe counter for resource monitoring.
   */
  static uint32_t GetActiveGpioCount() noexcept;

  /**
   * @brief Validate if a pin number is valid for the target platform.
   * @param pin_num Pin number to validate
   * @return true if pin is valid for GPIO use, false otherwise
   * @details Platform-specific validation using centralized constants.
   */
  static bool IsValidPin(hf_pin_num_t pin_num) noexcept;

  /**
   * @brief Check if pin supports RTC GPIO functionality.
   * @param pin_num Pin number to check
   * @return true if pin supports RTC GPIO (deep sleep, analog functions)
   * @details ESP32C6: GPIO0-7 support RTC functionality.
   */
  static bool IsRtcGpio(hf_pin_num_t pin_num) noexcept;

  /**
   * @brief Check if pin is a strapping pin requiring caution.
   * @param pin_num Pin number to check
   * @return true if pin is a strapping pin
   * @details ESP32C6 strapping pins: GPIO4, GPIO5, GPIO8, GPIO9, GPIO15.
   */
  static bool IsStrappingPin(hf_pin_num_t pin_num) noexcept;

private:
  //==============================================================//
  // PRIVATE HELPER METHODS
  //==============================================================//

  /**
   * @brief Convert BaseGpio::InterruptTrigger to platform-specific interrupt type.
   * @param trigger BaseGpio interrupt trigger enum
   * @return Platform-specific interrupt type value
   */
  gpio_int_type_t MapInterruptTrigger(hf_gpio_interrupt_trigger_t trigger) const noexcept;

  /**
   * @brief Static interrupt service routine handler.
   * @param arg Pointer to EspGpio instance
   */
  static void IRAM_ATTR StaticInterruptHandler(void *arg);

  /**
   * @brief Handle interrupt in instance context.
   */
  void IRAM_ATTR HandleInterrupt();

  /**
   * @brief Initialize advanced features during GPIO initialization.
   * @return true if all advanced features initialized successfully
   */
  bool InitializeAdvancedFeatures() noexcept;

  /**
   * @brief Cleanup advanced feature resources.
   */
  void CleanupAdvancedFeatures() noexcept;

  /**
   * @brief Cleanup glitch filter resources.
   */
  void CleanupGlitchFilters() noexcept;

  /**
   * @brief Cleanup ETM (Event Task Matrix) resources.
   */
  void CleanupETM() noexcept;

  /**
   * @brief Cleanup interrupt semaphore (called from destructor).
   */
  void CleanupInterruptSemaphore() noexcept;

  hf_gpio_err_t WriteImpl(hf_gpio_state_t state) noexcept;
  hf_gpio_err_t ReadImpl(hf_gpio_state_t &state) noexcept;
  
  //==============================================================//
  // MEMBER VARIABLES
  //==============================================================//

  // Interrupt state
  hf_gpio_interrupt_trigger_t interrupt_trigger_;   ///< Current interrupt trigger type
  InterruptCallback interrupt_callback_; ///< User interrupt callback
  void *interrupt_user_data_;            ///< User data for callback
  bool interrupt_enabled_;               ///< Interrupt currently enabled
  std::atomic<uint32_t> interrupt_count_; ///< Number of interrupts occurred (thread-safe)
  void *platform_semaphore_;             ///< Platform-specific semaphore for WaitForInterrupt

  // Advanced GPIO state
  hf_gpio_drive_cap_t drive_capability_;      ///< Current drive capability setting
  hf_gpio_glitch_filter_type_t glitch_filter_type_;   ///< Type of glitch filter configured
  bool pin_glitch_filter_enabled_;            ///< Pin glitch filter enabled
  bool flex_glitch_filter_enabled_;           ///< Flexible glitch filter enabled
  hf_gpio_flex_filter_config_t flex_filter_config_; ///< Flexible filter configuration
  hf_gpio_sleep_config_t sleep_config_;              ///< Sleep configuration
  bool hold_enabled_;                         ///< Hold function enabled
  bool rtc_gpio_enabled_;                     ///< RTC GPIO functionality enabled
  hf_gpio_wakeup_config_t wakeup_config_;            ///< Wake-up configuration

  // Platform-specific handles for advanced features
  void *glitch_filter_handle_; ///< Platform-specific glitch filter handle
  void *rtc_gpio_handle_;      ///< Platform-specific RTC GPIO handle

  // Static members for ISR management
  static bool gpio_isr_handler_installed_; ///< Track if ISR service is installed

  // Initialization state
  bool initialized_; ///< Lazy initialization state
};
