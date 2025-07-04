/**
 * @file McuTypes_GPIO.h
 * @brief MCU-specific GPIO type definitions for hardware abstraction.
 *
 * This header defines all GPIO-specific types and constants that are used
 * throughout the internal interface wrap layer for GPIO operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "McuTypes_Base.h"

//==============================================================================
// PLATFORM-SPECIFIC GPIO TYPE MAPPINGS
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
// ESP32 GPIO specific mappings
using hf_gpio_num_native_t = gpio_num_t;
using hf_gpio_mode_native_t = gpio_mode_t;
using hf_gpio_pull_native_t = gpio_pull_mode_t;
#else
// Non-ESP32 platforms - use generic types
using hf_gpio_num_native_t = uint32_t;
using hf_gpio_mode_native_t = uint8_t;
using hf_gpio_pull_native_t = uint8_t;
#endif

//==============================================================================
// MCU-SPECIFIC GPIO TYPES
//==============================================================================

/**
 * @brief MCU-specific GPIO mode configuration.
 */
enum class hf_gpio_mode_t : uint8_t {
  HF_GPIO_MODE_INPUT = 0,
  HF_GPIO_MODE_OUTPUT,
  HF_GPIO_MODE_OUTPUT_OD,
};

/**
 * @brief MCU-specific GPIO pull resistor configuration.
 */
enum class hf_gpio_pull_t : uint8_t {
  HF_GPIO_PULL_NONE = 0,
  HF_GPIO_PULL_UP,
  HF_GPIO_PULL_DOWN,
};

/**
 * @brief MCU-specific GPIO interrupt trigger configuration.
 */
enum class hf_gpio_intr_type_t : uint8_t {
  HF_GPIO_INTR_DISABLE = 0,
  HF_GPIO_INTR_POSEDGE,
  HF_GPIO_INTR_NEGEDGE,
  HF_GPIO_INTR_ANYEDGE,
  HF_GPIO_INTR_LOW_LEVEL,
  HF_GPIO_INTR_HIGH_LEVEL,
};

/**
 * @brief MCU-specific GPIO drive capability.
 */
enum class hf_gpio_drive_cap_t : uint8_t {
  HF_GPIO_DRIVE_CAP_0 = 0, ///< Minimum drive capability (5mA)
  HF_GPIO_DRIVE_CAP_1,     ///< Medium drive capability (10mA)
  HF_GPIO_DRIVE_CAP_2,     ///< High drive capability (20mA)
  HF_GPIO_DRIVE_CAP_3,     ///< Maximum drive capability (40mA)
};

//==============================================================================
// ESP32C6 ADVANCED GPIO TYPES FOR ESP-IDF v5.5+
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief ESP32C6 specific GPIO glitch filter types.
 * @details Advanced glitch filtering capabilities for noise reduction.
 */
enum class hf_gpio_glitch_filter_type_t : uint8_t {
  HF_GPIO_GLITCH_FILTER_NONE = 0, ///< No glitch filter
  HF_GPIO_GLITCH_FILTER_PIN = 1,  ///< Pin glitch filter (2 clock cycles)
  HF_GPIO_GLITCH_FILTER_FLEX = 2, ///< Flexible glitch filter (configurable)
  HF_GPIO_GLITCH_FILTER_BOTH = 3, ///< Both pin and flex filters
};

/**
 * @brief ESP32C6 GPIO drive capability levels.
 * @details Configurable output drive strength for power optimization.
 */
enum class hf_gpio_drive_strength_t : uint8_t {
  HF_GPIO_DRIVE_CAP_WEAK = 0,     ///< ~5mA drive capability
  HF_GPIO_DRIVE_CAP_STRONGER = 1, ///< ~10mA drive capability
  HF_GPIO_DRIVE_CAP_MEDIUM = 2,   ///< ~20mA drive capability (default)
  HF_GPIO_DRIVE_CAP_STRONGEST = 3 ///< ~40mA drive capability
};

/**
 * @brief ESP32C6 low-power GPIO configuration for sleep modes.
 * @details Configuration for GPIO behavior during sleep and low-power operation.
 */
struct hf_gpio_sleep_config_t {
  hf_gpio_mode_t sleep_mode;      ///< GPIO mode during sleep
  hf_gpio_pull_t sleep_pull_mode; ///< Pull resistor configuration during sleep
  bool sleep_output_enable;       ///< Enable output during sleep
  bool sleep_input_enable;        ///< Enable input during sleep
  bool hold_during_sleep;         ///< Hold configuration during sleep
  bool rtc_domain_enable;         ///< Route to RTC domain for ultra-low power
};

/**
 * @brief ESP32C6 flexible glitch filter configuration.
 * @details Configurable glitch filter for advanced noise rejection.
 */
struct hf_gpio_flex_filter_config_t {
  uint32_t window_width_ns;     ///< Sample window width in nanoseconds
  uint32_t window_threshold_ns; ///< Threshold for filtering in nanoseconds
  bool enable_on_init;          ///< Enable filter immediately after creation
};

/**
 * @brief ESP32C6 GPIO wake-up configuration for deep sleep.
 * @details Configuration for GPIO-based wake-up from deep sleep modes.
 */
struct hf_gpio_wakeup_config_t {
  hf_gpio_intr_type_t wake_trigger; ///< Wake-up trigger type
  bool enable_rtc_wake;             ///< Enable RTC domain wake-up
  bool enable_ext1_wake;            ///< Enable EXT1 wake-up source
  uint8_t wake_level;               ///< Wake-up level (0=low, 1=high)
  bool internal_pullup_enable;      ///< Enable internal pull-up during sleep
  bool internal_pulldown_enable;    ///< Enable internal pull-down during sleep
};

/**
 * @brief ESP32C6 GPIO configuration for advanced features.
 * @details Complete configuration structure for ESP32C6 GPIO with all advanced features.
 */
struct hf_gpio_advanced_config_t {
  hf_gpio_num_native_t gpio_num;                   ///< GPIO pin number
  hf_gpio_mode_t mode;                             ///< GPIO mode (input/output/etc)
  hf_gpio_pull_t pull_mode;                        ///< Pull resistor configuration
  hf_gpio_intr_type_t intr_type;                   ///< Interrupt trigger type
  hf_gpio_drive_strength_t drive_strength;         ///< Output drive capability
  hf_gpio_glitch_filter_type_t glitch_filter_type; ///< Glitch filter type
  hf_gpio_flex_filter_config_t flex_filter_config; ///< Flexible filter configuration
  hf_gpio_sleep_config_t sleep_config;             ///< Sleep mode configuration
  hf_gpio_wakeup_config_t wakeup_config;           ///< Wake-up configuration
  bool enable_hold_function;                       ///< Enable GPIO hold function
  bool enable_rtc_gpio;                            ///< Enable RTC GPIO functionality
};

/**
 * @brief ESP32C6 GPIO status information for diagnostics.
 * @details Comprehensive status information for debugging and monitoring.
 */
struct hf_gpio_status_info_t {
  uint8_t pin_number;                         ///< GPIO pin number
  hf_gpio_mode_t current_mode;                ///< Current GPIO mode
  hf_gpio_pull_t current_pull_mode;           ///< Current pull mode
  hf_gpio_drive_strength_t current_drive_cap; ///< Current drive capability
  bool input_enabled;                         ///< Input buffer enabled
  bool output_enabled;                        ///< Output buffer enabled
  bool open_drain;                            ///< Open drain mode
  bool sleep_sel_enabled;                     ///< Sleep selection enabled
  uint32_t function_select;                   ///< IOMUX function selection
  bool is_rtc_gpio;                           ///< Pin supports RTC GPIO
  bool glitch_filter_enabled;                 ///< Glitch filter enabled
  hf_gpio_glitch_filter_type_t filter_type;   ///< Type of glitch filter
  bool hold_enabled;                          ///< Hold function enabled
  uint32_t interrupt_count;                   ///< Number of interrupts occurred
  bool is_wake_source;                        ///< Pin configured as wake source
};

/**
 * @brief ESP32C6 GPIO pin validity checking.
 * @details Utility for validating GPIO pin numbers for different functions.
 */
struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;     ///< Pin can be used as GPIO
  bool supports_adc;      ///< Pin supports ADC functionality
  bool supports_rtc;      ///< Pin supports RTC GPIO
  bool supports_touch;    ///< Pin supports touch sensing
  bool is_strapping_pin;  ///< Pin is a strapping pin (requires caution)
  bool is_spi_flash_pin;  ///< Pin is used for SPI flash (not recommended for GPIO)
  bool is_usb_jtag_pin;   ///< Pin is used for USB-JTAG (disables JTAG if reconfigured)
  uint8_t lp_gpio_number; ///< Low-power GPIO number (if applicable)
  uint8_t adc_unit;       ///< ADC unit number (if applicable)
  uint8_t adc_channel;    ///< ADC channel number (if applicable)
};

/**
 * @brief Native ESP-IDF v5.5+ GPIO types mapping.
 * @details Direct mappings to ESP-IDF GPIO types for maximum compatibility.
 */
using hf_gpio_config_native_t = gpio_config_t;
using hf_gpio_glitch_filter_handle_native_t = gpio_glitch_filter_handle_t;
using hf_gpio_pin_glitch_filter_config_native_t = gpio_pin_glitch_filter_config_t;
using hf_gpio_flex_glitch_filter_config_native_t = gpio_flex_glitch_filter_config_t;
using hf_rtc_gpio_mode_native_t = rtc_gpio_mode_t;

#else
// Non-ESP32 platforms - use generic GPIO types
struct hf_gpio_advanced_config_t {
  uint32_t gpio_num;
  uint8_t mode;
  uint8_t pull_mode;
  uint8_t intr_type;
  uint8_t drive_strength;
  // Simplified structure for non-ESP32 platforms
};

struct hf_gpio_status_info_t {
  uint8_t pin_number;
  uint8_t current_mode;
  uint8_t current_pull_mode;
  bool input_enabled;
  bool output_enabled;
  uint32_t interrupt_count;
};

struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;
  bool supports_adc;
  bool is_strapping_pin;
};

// Generic handle types for non-ESP32 platforms
using hf_gpio_config_native_t = struct {
  int dummy;
};
using hf_gpio_glitch_filter_handle_native_t = void *;
using hf_gpio_pin_glitch_filter_config_native_t = struct {
  int dummy;
};
using hf_gpio_flex_glitch_filter_config_native_t = struct {
  int dummy;
};
using hf_rtc_gpio_mode_native_t = uint8_t;

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// UTILITY MACROS FOR ESP32C6 GPIO VALIDATION
//==============================================================================

/**
 * @brief ESP32C6 GPIO pin count and validation macros.
 */
#ifdef HF_MCU_ESP32C6
#define HF_GPIO_PIN_COUNT 31        ///< Total GPIO pins (0-30)
#define HF_GPIO_MAX_PIN_NUMBER 30   ///< Maximum valid GPIO pin number
#define HF_GPIO_RTC_PIN_COUNT 8     ///< RTC GPIO pins (0-7)
#define HF_GPIO_ADC_PIN_COUNT 7     ///< ADC capable pins (0-6)
#define HF_GPIO_FLEX_FILTER_COUNT 8 ///< Number of flexible glitch filters

/**
 * @brief ESP32C6 specific pin validation macros.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= HF_GPIO_MAX_PIN_NUMBER)

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_IS_STRAPPING_PIN(gpio_num)                                                         \
  ((gpio_num) == 4 || (gpio_num) == 5 || (gpio_num) == 8 || (gpio_num) == 9 || (gpio_num) == 15)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) ((gpio_num) >= 24 && (gpio_num) <= 30)

#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) ((gpio_num) == 12 || (gpio_num) == 13)

#define HF_GPIO_SUPPORTS_ADC(gpio_num) ((gpio_num) >= 0 && (gpio_num) <= 6)

#else
// Generic macros for non-ESP32C6 platforms
#define HF_GPIO_PIN_COUNT 32
#define HF_GPIO_MAX_PIN_NUMBER 31
#define HF_GPIO_RTC_PIN_COUNT 0
#define HF_GPIO_ADC_PIN_COUNT 0
#define HF_GPIO_FLEX_FILTER_COUNT 0

#define HF_GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num) >= 0 && (gpio_num) < HF_GPIO_PIN_COUNT)
#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) false
#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) false
#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) false
#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) false
#define HF_GPIO_SUPPORTS_ADC(gpio_num) false
#endif // HF_MCU_ESP32C6
