/**
 * @file EspTypes_GPIO.h
 * @ingroup esp32_types
 * @brief ESP32 GPIO type definitions for hardware abstraction.
 *
 * This header defines only the essential GPIO-specific types and constants used by
 * the EspGpio implementation. It follows a clean, minimal pattern providing only
 * necessary types without redundant or duplicate definitions.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseGpio.h" // For hf_gpio_err_t
#include "EspTypes_Base.h"
#include "HardwareTypes.h"
#include "McuSelect.h" // Central MCU platform selection (includes all ESP-IDF)

#ifdef HF_MCU_FAMILY_ESP32

//==============================================================================
// ESSENTIAL GPIO TYPES (ESP32)
//==============================================================================

/**
 * @brief GPIO pin number type for ESP32.
 */
using hf_gpio_num_t = hf_pin_num_t;

/**
 * @brief GPIO mode configuration for ESP32.
 * @details GPIO mode enumeration supporting ESP32 hardware capabilities.
 */
enum class hf_gpio_mode_t : uint8_t {
  HF_GPIO_MODE_DISABLE = 0,        ///< GPIO disabled (no input/output)
  HF_GPIO_MODE_INPUT = 1,          ///< Input only mode
  HF_GPIO_MODE_OUTPUT = 2,         ///< Output only mode
  HF_GPIO_MODE_OUTPUT_OD = 3,      ///< Open-drain output mode
  HF_GPIO_MODE_INPUT_OUTPUT = 4,   ///< Bidirectional mode (input + output)
  HF_GPIO_MODE_INPUT_OUTPUT_OD = 5 ///< Bidirectional open-drain mode
};

/**
 * @brief GPIO pull resistor configuration.
 * @details Pull resistor combinations supported by ESP32 hardware.
 */
enum class hf_gpio_pull_t : uint8_t {
  HF_GPIO_PULL_NONE = 0,   ///< No pull resistors (floating)
  HF_GPIO_PULL_UP = 1,     ///< Pull-up resistor only
  HF_GPIO_PULL_DOWN = 2,   ///< Pull-down resistor only
  HF_GPIO_PULL_UP_DOWN = 3 ///< Both pull-up and pull-down (keeper mode)
};

/**
 * @brief GPIO interrupt trigger configuration.
 * @details Interrupt trigger types for ESP32.
 */
enum class hf_gpio_intr_type_t : uint8_t {
  HF_GPIO_INTR_DISABLE = 0,   ///< Interrupt disabled
  HF_GPIO_INTR_POSEDGE = 1,   ///< Rising edge trigger
  HF_GPIO_INTR_NEGEDGE = 2,   ///< Falling edge trigger
  HF_GPIO_INTR_ANYEDGE = 3,   ///< Both edge trigger
  HF_GPIO_INTR_LOW_LEVEL = 4, ///< Low level trigger
  HF_GPIO_INTR_HIGH_LEVEL = 5 ///< High level trigger
};

/**
 * @brief GPIO drive capability levels.
 * @details ESP32 drive strength options for power optimization.
 */
enum class hf_gpio_drive_cap_t : uint8_t {
  HF_GPIO_DRIVE_CAP_WEAK = 0,     ///< ~5mA drive capability
  HF_GPIO_DRIVE_CAP_STRONGER = 1, ///< ~10mA drive capability
  HF_GPIO_DRIVE_CAP_MEDIUM = 2,   ///< ~20mA drive capability (default)
  HF_GPIO_DRIVE_CAP_STRONGEST = 3 ///< ~40mA drive capability
};

/**
 * @brief RTC GPIO mode configuration for low-power operation.
 * @details RTC domain GPIO modes for deep sleep and LP operations.
 */
enum class hf_rtc_gpio_mode_t : uint8_t {
  HF_RTC_GPIO_MODE_INPUT_ONLY = 0,     ///< RTC input only
  HF_RTC_GPIO_MODE_OUTPUT_ONLY = 1,    ///< RTC output only
  HF_RTC_GPIO_MODE_INPUT_OUTPUT = 2,   ///< RTC bidirectional
  HF_RTC_GPIO_MODE_DISABLED = 3,       ///< RTC GPIO disabled
  HF_RTC_GPIO_MODE_OUTPUT_OD = 4,      ///< RTC open-drain output
  HF_RTC_GPIO_MODE_INPUT_OUTPUT_OD = 5 ///< RTC bidirectional open-drain
};

//==============================================================================
// ADVANCED GPIO FEATURES (ESP32)
//==============================================================================

/**
 * @brief ESP32 glitch filter types for noise immunity.
 * @details Hardware-based glitch filtering capabilities.
 */
enum class hf_gpio_glitch_filter_type_t : uint8_t {
  HF_GPIO_GLITCH_FILTER_NONE = 0, ///< No glitch filter
  HF_GPIO_GLITCH_FILTER_PIN = 1,  ///< Pin glitch filter (2 clock cycles, fixed)
  HF_GPIO_GLITCH_FILTER_FLEX = 2, ///< Flexible glitch filter (configurable)
  HF_GPIO_GLITCH_FILTER_BOTH = 3  ///< Both pin and flex filters active
};

/**
 * @brief GPIO clock source selection for glitch filters.
 * @details Clock sources available for timing glitch filter operations.
 */
enum class hf_gpio_glitch_filter_clk_src_t : uint8_t {
  HF_GLITCH_FILTER_CLK_SRC_APB = 0,     ///< APB clock (80MHz typically)
  HF_GLITCH_FILTER_CLK_SRC_RC_FAST = 1, ///< RC_FAST clock (~17.5MHz)
  HF_GLITCH_FILTER_CLK_SRC_XTAL = 2     ///< XTAL clock (40MHz typically)
};

/**
 * @brief Low-Power IO configuration for ultra-low power operation.
 * @details Configuration for LP_IO domain operations during deep sleep.
 */
struct hf_lp_io_config_t {
  hf_gpio_mode_t mode;                ///< LP IO mode
  hf_gpio_pull_t pull_mode;           ///< Pull resistor configuration
  hf_gpio_drive_cap_t drive_strength; ///< Output drive capability
  bool input_enable;                  ///< Enable input buffer
  bool output_enable;                 ///< Enable output buffer
  bool hold_enable;                   ///< Hold configuration during sleep
  bool force_hold;                    ///< Force hold regardless of sleep state
};

/**
 * @brief Flexible glitch filter configuration.
 * @details Configurable glitch filter for advanced noise rejection.
 */
struct hf_gpio_flex_filter_config_t {
  uint32_t window_width_ns;                ///< Sample window width in nanoseconds
  uint32_t window_threshold_ns;            ///< Threshold for filtering in nanoseconds
  hf_gpio_glitch_filter_clk_src_t clk_src; ///< Clock source selection
  bool enable_on_init;                     ///< Enable filter immediately after creation
};

/**
 * @brief Pin glitch filter configuration.
 * @details Fixed-duration pin glitch filter configuration.
 */
struct hf_gpio_pin_filter_config_t {
  hf_gpio_glitch_filter_clk_src_t clk_src; ///< Clock source selection
  bool enable_on_init;                     ///< Enable filter immediately after creation
};

/**
 * @brief GPIO sleep mode configuration.
 * @details GPIO behavior during light and deep sleep modes.
 */
struct hf_gpio_sleep_config_t {
  hf_gpio_mode_t sleep_mode;                ///< GPIO mode during sleep
  hf_gpio_mode_t sleep_direction;           ///< GPIO direction during sleep (alias for sleep_mode)
  hf_gpio_pull_t sleep_pull_mode;           ///< Pull resistor configuration during sleep
  hf_gpio_drive_cap_t sleep_drive_strength; ///< Drive strength during sleep
  bool sleep_output_enable;                 ///< Enable output during sleep
  bool sleep_input_enable;                  ///< Enable input during sleep
  bool hold_during_sleep;                   ///< Hold configuration during sleep
  bool rtc_domain_enable;                   ///< Route to RTC domain for ultra-low power
  bool slp_sel_enable;                      ///< Enable automatic sleep selection
  bool enable_sleep_retain;                 ///< Enable sleep retention
};

/**
 * @brief GPIO wake-up configuration for deep sleep.
 * @details Configuration for GPIO-based wake-up from deep sleep modes.
 */
struct hf_gpio_wakeup_config_t {
  hf_gpio_intr_type_t wake_trigger; ///< Wake-up trigger type
  bool enable_rtc_wake;             ///< Enable RTC domain wake-up
  bool enable_ext1_wake;            ///< Enable EXT1 wake-up source (multiple pins)
  uint8_t wake_level;               ///< Wake-up level (0=low, 1=high)
  bool internal_pullup_enable;      ///< Enable internal pull-up during sleep
  bool internal_pulldown_enable;    ///< Enable internal pull-down during sleep
  bool iso_en;                      ///< Enable isolation during sleep
};

/**
 * @brief Complete ESP32 GPIO configuration with all advanced features.
 * @details Comprehensive configuration structure for ESP32 GPIO.
 */
struct hf_gpio_advanced_config_t {
  hf_pin_num_t gpio_num;                           ///< GPIO pin number
  hf_gpio_mode_t mode;                             ///< GPIO mode (input/output/etc)
  hf_gpio_pull_t pull_mode;                        ///< Pull resistor configuration
  hf_gpio_intr_type_t intr_type;                   ///< Interrupt trigger type
  hf_gpio_drive_cap_t drive_strength;              ///< Output drive capability
  hf_gpio_glitch_filter_type_t glitch_filter_type; ///< Glitch filter type
  hf_gpio_flex_filter_config_t flex_filter_config; ///< Flexible filter configuration
  hf_gpio_pin_filter_config_t pin_filter_config;   ///< Pin filter configuration
  hf_gpio_sleep_config_t sleep_config;             ///< Sleep mode configuration
  hf_gpio_wakeup_config_t wakeup_config;           ///< Wake-up configuration
  bool enable_hold_function;                       ///< Enable GPIO hold function
  bool enable_rtc_gpio;                            ///< Enable RTC GPIO functionality
};

/**
 * @brief Comprehensive ESP32 GPIO status information for diagnostics.
 * @details Complete status information for debugging, monitoring, and diagnostics.
 */
struct hf_gpio_status_info_t {
  uint8_t pin_number;                       ///< GPIO pin number
  hf_gpio_mode_t current_mode;              ///< Current GPIO mode
  hf_gpio_pull_t current_pull_mode;         ///< Current pull mode
  hf_gpio_drive_cap_t current_drive_cap;    ///< Current drive capability
  hf_gpio_intr_type_t interrupt_type;       ///< Current interrupt type
  bool input_enabled;                       ///< Input buffer enabled
  bool output_enabled;                      ///< Output buffer enabled
  bool open_drain;                          ///< Open drain mode active
  bool sleep_sel_enabled;                   ///< Sleep selection enabled
  bool hold_enabled;                        ///< Hold function enabled
  bool rtc_enabled;                         ///< RTC GPIO enabled
  uint32_t function_select;                 ///< IOMUX function selection
  hf_gpio_glitch_filter_type_t filter_type; ///< Active glitch filter type
  bool glitch_filter_enabled;               ///< Glitch filter enabled
  uint32_t interrupt_count;                 ///< Number of interrupts occurred
  bool is_wake_source;                      ///< Pin configured as wake source
  bool sleep_hold_active;                   ///< Sleep hold currently active
  uint32_t last_interrupt_time_us;          ///< Last interrupt timestamp (microseconds)
};

/**
 * @brief ESP32 GPIO pin capabilities and limitations.
 * @details Complete capability information for each GPIO pin.
 */
typedef struct {
  uint8_t pin_number;
  bool is_valid_gpio;
  bool supports_input;
  bool supports_output;
  bool supports_pullup;
  bool supports_pulldown;
  bool supports_adc;
  bool supports_rtc;
  bool is_strapping_pin;
  bool is_usb_jtag_pin;
  bool is_spi_pin;
  bool supports_glitch_filter;
} hf_gpio_pin_capabilities_t;

// Only define each macro once, use 'pin' as the parameter name
#ifndef HF_GPIO_IS_ADC_CAPABLE
#define HF_GPIO_IS_ADC_CAPABLE(pin) (false)
#endif
#ifndef HF_GPIO_IS_SPI_PIN
#define HF_GPIO_IS_SPI_PIN(pin) (false)
#endif
#ifndef HF_GPIO_IS_RTC_GPIO
#define HF_GPIO_IS_RTC_GPIO(pin) (false)
#endif
#ifndef HF_GPIO_IS_STRAPPING_PIN
#define HF_GPIO_IS_STRAPPING_PIN(pin) (false)
#endif
#ifndef HF_GPIO_IS_USB_JTAG_PIN
#define HF_GPIO_IS_USB_JTAG_PIN(pin) (false)
#endif
#ifndef HF_GPIO_SUPPORTS_GLITCH_FILTER
#define HF_GPIO_SUPPORTS_GLITCH_FILTER(pin) (false)
#endif

//==============================================================================
// END OF ESPGPIO TYPES - MINIMAL AND ESSENTIAL ONLY
//==============================================================================

//==============================================================================
// ESP32 GPIO VALIDATION MACROS AND CONSTANTS
//==============================================================================

/**
 * @brief ESP32 GPIO pin capability validation macros.
 * @details All hardware constants reference McuSelect.h directly for single source of truth.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num)                                                            \
  ((gpio_num) >= 0 && (gpio_num) <= HF_MCU_GPIO_MAX_PIN_NUMBER)

#define HF_GPIO_IS_VALID_PIN(gpio_num) HF_GPIO_IS_VALID_GPIO(gpio_num)

#define HF_GPIO_IS_VALID_OUTPUT_GPIO(gpio_num)                                                     \
  (HF_GPIO_IS_VALID_GPIO(gpio_num) && !HF_GPIO_IS_INPUT_ONLY_PIN(gpio_num))

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_IS_VALID_LP_IO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_SUPPORTS_ADC(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 6)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) ((gpio_num) >= 24 && (gpio_num) <= 30)

#define HF_GPIO_IS_INPUT_ONLY_PIN(gpio_num) (false) /* ESP32 has no input-only pins */

#define HF_GPIO_SUPPORTS_PULL_UP(gpio_num) (HF_GPIO_IS_VALID_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_PULL_DOWN(gpio_num) (HF_GPIO_IS_VALID_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_OPEN_DRAIN(gpio_num) (HF_GPIO_IS_VALID_OUTPUT_GPIO(gpio_num))

/**
 * @brief ESP32 GPIO to ADC channel mapping.
 */
#define HF_GPIO_TO_ADC_UNIT(gpio_num) (HF_GPIO_SUPPORTS_ADC(gpio_num) ? 1 : 0xFF)

#define HF_GPIO_TO_ADC_CHANNEL(gpio_num) (HF_GPIO_SUPPORTS_ADC(gpio_num) ? (gpio_num) : 0xFF)

/**
 * @brief ESP32 GPIO to RTC mapping.
 */
#define HF_GPIO_TO_RTC_GPIO(gpio_num) (HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) ? (gpio_num) : 0xFF)

#define HF_GPIO_TO_LP_IO(gpio_num) (HF_GPIO_IS_VALID_LP_IO(gpio_num) ? (gpio_num) : 0xFF)

/**
 * @brief ESP32 pin safety classification.
 */
#define HF_GPIO_IS_SAFE_FOR_GENERAL_USE(gpio_num)                                                  \
  (HF_GPIO_IS_VALID_GPIO(gpio_num) && !HF_GPIO_IS_STRAPPING_PIN(gpio_num) &&                       \
   !HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) && !HF_GPIO_IS_USB_JTAG_PIN(gpio_num))

//==============================================================================
// CONVENIENCE TYPES AND UTILITY FUNCTIONS
//==============================================================================

/**
 * @brief GPIO interrupt callback function type.
 * @param gpio_num The GPIO number that triggered the interrupt
 * @param user_data User-provided data passed to the callback
 */
using hf_gpio_isr_callback_t = void (*)(uint32_t gpio_num, void* user_data);

/**
 * @brief GPIO configuration validation result.
 */
enum class hf_gpio_config_result_t : uint8_t {
  HF_GPIO_CONFIG_OK = 0,                ///< Configuration is valid
  HF_GPIO_CONFIG_INVALID_PIN = 1,       ///< Invalid pin number
  HF_GPIO_CONFIG_INVALID_MODE = 2,      ///< Invalid mode for this pin
  HF_GPIO_CONFIG_INVALID_PULL = 3,      ///< Invalid pull configuration
  HF_GPIO_CONFIG_INVALID_DRIVE = 4,     ///< Invalid drive strength
  HF_GPIO_CONFIG_STRAPPING_WARNING = 5, ///< Warning: strapping pin usage
  HF_GPIO_CONFIG_FLASH_WARNING = 6,     ///< Warning: SPI flash pin usage
  HF_GPIO_CONFIG_JTAG_WARNING = 7       ///< Warning: USB-JTAG pin usage
};

/**
 * @brief Utility structure for GPIO pin information lookup.
 */
struct hf_gpio_pin_info_t {
  uint8_t gpio_num;                        ///< GPIO number
  const char* pin_name;                    ///< Human-readable pin name
  const char* alt_functions[4];            ///< Alternative functions available
  hf_gpio_pin_capabilities_t capabilities; ///< Pin capabilities
  const char* usage_notes;                 ///< Special usage notes or warnings
};

/**
 * @brief ESP32 GPIO pin information table.
 * @details Complete pin information for all ESP32 GPIO pins.
 * @note This table should be defined in the implementation file.
 */
extern const hf_gpio_pin_info_t HF_GPIO_PIN_INFO_TABLE[HF_MCU_GPIO_PIN_COUNT];

/**
 * @brief Get comprehensive pin information for a GPIO.
 * @param gpio_num GPIO number to query
 * @return Pointer to pin information structure, nullptr if invalid
 */
inline const hf_gpio_pin_info_t* hf_gpio_get_pin_info(uint8_t gpio_num) {
  if (!HF_GPIO_IS_VALID_GPIO(gpio_num)) {
    return nullptr;
  }
  return &HF_GPIO_PIN_INFO_TABLE[gpio_num];
}

/**
 * @brief Validate GPIO configuration for ESP32.
 * @param config GPIO configuration to validate
 * @return Validation result with details
 */
hf_gpio_config_result_t hf_gpio_validate_config(const hf_gpio_advanced_config_t* config);

/**
 * @brief Get optimal drive strength for a given frequency.
 * @param frequency_hz Target toggle frequency in Hz
 * @param gpio_num GPIO number (for pin-specific limits)
 * @return Recommended drive strength
 */
hf_gpio_drive_cap_t hf_gpio_get_optimal_drive_strength(uint32_t frequency_hz, uint8_t gpio_num);

/**
 * @brief Calculate glitch filter window for given noise duration.
 * @param noise_duration_ns Maximum noise duration to filter (nanoseconds)
 * @param safety_margin_percent Safety margin percentage (typically 20-50%)
 * @return Recommended window width in nanoseconds
 */
uint32_t hf_gpio_calc_glitch_filter_window(uint32_t noise_duration_ns,
                                           uint8_t safety_margin_percent);

//==============================================================================
// ERROR HANDLING AND DEBUGGING SUPPORT
//==============================================================================

/**
 * @brief GPIO operation result codes.
 */
enum class hf_gpio_result_t : uint8_t {
  GPIO_OK = 0,                ///< Operation successful
  GPIO_ERR_INVALID_ARG = 1,   ///< Invalid argument
  GPIO_ERR_INVALID_STATE = 2, ///< Invalid state for operation
  GPIO_ERR_NOT_SUPPORTED = 3, ///< Operation not supported
  GPIO_ERR_NO_MEM = 4,        ///< Out of memory
  GPIO_ERR_TIMEOUT = 5,       ///< Operation timeout
  GPIO_ERR_HW_FAULT = 6,      ///< Hardware fault
  GPIO_ERR_BUSY = 7,          ///< Resource busy
  GPIO_ERR_NOT_FOUND = 8      ///< Resource not found
};

/**
 * @brief Convert GPIO result code to human-readable string.
 * @param result Result code to convert
 * @return String description of the result
 */
constexpr const char* hf_gpio_result_to_string(hf_gpio_result_t result) {
  switch (result) {
  case hf_gpio_result_t::GPIO_OK:
    return "Success";
  case hf_gpio_result_t::GPIO_ERR_INVALID_ARG:
    return "Invalid argument";
  case hf_gpio_result_t::GPIO_ERR_INVALID_STATE:
    return "Invalid state";
  case hf_gpio_result_t::GPIO_ERR_NOT_SUPPORTED:
    return "Not supported";
  case hf_gpio_result_t::GPIO_ERR_NO_MEM:
    return "Out of memory";
  case hf_gpio_result_t::GPIO_ERR_TIMEOUT:
    return "Timeout";
  case hf_gpio_result_t::GPIO_ERR_HW_FAULT:
    return "Hardware fault";
  case hf_gpio_result_t::GPIO_ERR_BUSY:
    return "Resource busy";
  case hf_gpio_result_t::GPIO_ERR_NOT_FOUND:
    return "Resource not found";
  default:
    return "Unknown error";
  }
}

//==============================================================================
// COMPILE-TIME CONFIGURATION VALIDATION
//==============================================================================

// Compile-time assertions to ensure configuration consistency
static_assert(HF_MCU_GPIO_PIN_COUNT == 31, "ESP32 should have 31 GPIO pins");
static_assert(HF_MCU_GPIO_MAX_PIN_NUMBER == 30, "ESP32 max GPIO should be 30");
static_assert(HF_MCU_GPIO_RTC_PIN_COUNT == 8, "ESP32 should have 8 RTC GPIO pins");
static_assert(HF_MCU_GPIO_ADC_PIN_COUNT == 7, "ESP32 should have 7 ADC channels");
static_assert(HF_MCU_GPIO_FLEX_FILTER_COUNT == 8, "ESP32 should have 8 flex filters");

#ifdef GPIO_MODE_INPUT
static_assert(static_cast<int>(hf_gpio_mode_t::HF_GPIO_MODE_INPUT) == GPIO_MODE_INPUT,
              "GPIO mode values must match ESP-IDF");
#endif

#ifdef GPIO_PULLUP_ONLY
static_assert(static_cast<int>(hf_gpio_pull_t::HF_GPIO_PULL_UP) == GPIO_PULLUP_ONLY,
              "GPIO pull mode values must match ESP-IDF");
#endif

#endif // HF_MCU_FAMILY_ESP32