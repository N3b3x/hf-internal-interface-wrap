/**
 * @file McuTypes_GPIO.h
 * @brief Modern ESP32C6 GPIO type definitions with ESP-IDF v5.5+ features.
 *
 * This header defines all GPIO-specific types and constants for the latest
 * ESP32C6 hardware features including normal GPIO, RTC GPIO, dedicated GPIO,
 * glitch filtering, low-power operation, and advanced power management.
 *
 * Features supported:
 * - Standard GPIO with all modes (input, output, open-drain, bidirectional)
 * - RTC GPIO for ultra-low power operation and deep sleep wake-up
 * - Dedicated GPIO bundles for high-speed bit-banging operations
 * - Pin and Flexible glitch filtering for noise immunity
 * - Low-Power IO (LP_IO) for ultra-low power peripherals
 * - Deep sleep configuration and hold functions
 * - Event Task Matrix (ETM) integration
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note Only latest ESP-IDF v5.5+ APIs are supported, no legacy compatibility
 */

#pragma once

#include "HardwareTypes.h" // For basic hardware types
#include "McuSelect.h"    // Central MCU platform selection (includes all ESP-IDF)
#include "McuTypes_Base.h"
#include "BaseGpio.h" // For HfGpioErr

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
// MODERN ESP32C6 GPIO TYPES (ESP-IDF v5.5+)
//==============================================================================

/**
 * @brief GPIO pin number type for ESP32C6.
 */
using hf_gpio_num_t = int32_t;

/**
 * @brief Modern GPIO mode configuration with all ESP32C6 capabilities.
 * @details Comprehensive GPIO mode enumeration supporting all hardware capabilities.
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
 * @brief Modern GPIO pull resistor configuration.
 * @details All pull resistor combinations supported by ESP32C6 hardware.
 */
enum class hf_gpio_pull_t : uint8_t {
  HF_GPIO_PULL_NONE = 0,      ///< No pull resistors (floating)
  HF_GPIO_PULL_UP = 1,        ///< Pull-up resistor only
  HF_GPIO_PULL_DOWN = 2,      ///< Pull-down resistor only
  HF_GPIO_PULL_UP_DOWN = 3    ///< Both pull-up and pull-down (keeper mode)
};

/**
 * @brief Modern GPIO interrupt trigger configuration.
 * @details Complete interrupt trigger types for ESP32C6.
 */
enum class hf_gpio_intr_type_t : uint8_t {
  HF_GPIO_INTR_DISABLE = 0,     ///< Interrupt disabled
  HF_GPIO_INTR_POSEDGE = 1,     ///< Rising edge trigger
  HF_GPIO_INTR_NEGEDGE = 2,     ///< Falling edge trigger
  HF_GPIO_INTR_ANYEDGE = 3,     ///< Both edge trigger
  HF_GPIO_INTR_LOW_LEVEL = 4,   ///< Low level trigger
  HF_GPIO_INTR_HIGH_LEVEL = 5   ///< High level trigger
};

/**
 * @brief Modern GPIO drive capability levels.
 * @details ESP32C6 drive strength options for power optimization.
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
  HF_RTC_GPIO_MODE_INPUT_ONLY = 0,      ///< RTC input only
  HF_RTC_GPIO_MODE_OUTPUT_ONLY = 1,     ///< RTC output only
  HF_RTC_GPIO_MODE_INPUT_OUTPUT = 2,    ///< RTC bidirectional
  HF_RTC_GPIO_MODE_DISABLED = 3,        ///< RTC GPIO disabled
  HF_RTC_GPIO_MODE_OUTPUT_OD = 4,       ///< RTC open-drain output
  HF_RTC_GPIO_MODE_INPUT_OUTPUT_OD = 5  ///< RTC bidirectional open-drain
};

//==============================================================================
// MODERN ESP32C6 ADVANCED GPIO FEATURES (ESP-IDF v5.5+)
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32

/**
 * @brief ESP32C6 glitch filter types for noise immunity.
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
  HF_GLITCH_FILTER_CLK_SRC_APB = 0,    ///< APB clock (80MHz typically)
  HF_GLITCH_FILTER_CLK_SRC_RC_FAST = 1, ///< RC_FAST clock (~17.5MHz)
  HF_GLITCH_FILTER_CLK_SRC_XTAL = 2     ///< XTAL clock (40MHz typically)
};

/**
 * @brief GPIO ETM (Event Task Matrix) event edge types.
 * @details Edge types that can trigger ETM events from GPIO pins.
 */
enum class hf_gpio_etm_event_edge_t : uint8_t {
  HF_GPIO_ETM_EVENT_EDGE_POS = 0,  ///< Rising edge generates ETM event
  HF_GPIO_ETM_EVENT_EDGE_NEG = 1,  ///< Falling edge generates ETM event
  HF_GPIO_ETM_EVENT_EDGE_ANY = 2   ///< Any edge generates ETM event
};

/**
 * @brief GPIO ETM task actions for hardware-level GPIO operations.
 * @details Actions that can be performed by ETM tasks on GPIO pins.
 */
enum class hf_gpio_etm_task_action_t : uint8_t {
  HF_GPIO_ETM_TASK_ACTION_SET = 0,   ///< Set GPIO level to high
  HF_GPIO_ETM_TASK_ACTION_CLR = 1,   ///< Clear GPIO level to low  
  HF_GPIO_ETM_TASK_ACTION_TOG = 2    ///< Toggle GPIO level
};

/**
 * @brief Dedicated GPIO bundle configuration flags.
 * @details Control flags for dedicated GPIO bundle behavior.
 */
struct hf_dedic_gpio_bundle_flags_t {
  uint32_t in_en : 1;         ///< Enable input capability
  uint32_t in_invert : 1;     ///< Invert input signals
  uint32_t out_en : 1;        ///< Enable output capability
  uint32_t out_invert : 1;    ///< Invert output signals
  uint32_t reserved : 28;     ///< Reserved for future use
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
  uint32_t window_width_ns;           ///< Sample window width in nanoseconds
  uint32_t window_threshold_ns;       ///< Threshold for filtering in nanoseconds
  hf_gpio_glitch_filter_clk_src_t clk_src; ///< Clock source selection
  bool enable_on_init;                ///< Enable filter immediately after creation
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
 * @brief GPIO ETM event configuration.
 * @details Configuration for GPIO ETM event generation.
 */
struct hf_gpio_etm_event_config_t {
  hf_gpio_etm_event_edge_t edge;     ///< Edge type that triggers the event
  bool invert_output;                ///< Invert the output signal
  bool enable_on_init;               ///< Enable event immediately after creation
};

/**
 * @brief GPIO ETM task configuration.
 * @details Configuration for GPIO ETM task actions.
 */
struct hf_gpio_etm_task_config_t {
  hf_gpio_etm_task_action_t action;  ///< Action to perform when task is triggered
  bool invert_output;                ///< Invert the output signal
  bool enable_on_init;               ///< Enable task immediately after creation
};

/**
 * @brief Complete ETM configuration for GPIO.
 * @details Full ETM configuration including events, tasks, and channels.
 */
struct hf_gpio_etm_config_t {
  bool enable_etm;                          ///< Enable ETM functionality
  hf_gpio_etm_event_config_t event_config;  ///< ETM event configuration
  hf_gpio_etm_task_config_t task_config;    ///< ETM task configuration
  uint8_t etm_channel_priority;             ///< ETM channel priority (0=highest)
  bool auto_bind_gpio;                      ///< Automatically bind to GPIO pin
};

/**
 * @brief GPIO sleep mode configuration.
 * @details GPIO behavior during light and deep sleep modes.
 */
struct hf_gpio_sleep_config_t {
  hf_gpio_mode_t sleep_mode;          ///< GPIO mode during sleep
  hf_gpio_mode_t sleep_direction;     ///< GPIO direction during sleep (alias for sleep_mode)
  hf_gpio_pull_t sleep_pull_mode;     ///< Pull resistor configuration during sleep
  hf_gpio_drive_cap_t sleep_drive_strength; ///< Drive strength during sleep
  bool sleep_output_enable;           ///< Enable output during sleep
  bool sleep_input_enable;            ///< Enable input during sleep
  bool hold_during_sleep;             ///< Hold configuration during sleep
  bool rtc_domain_enable;             ///< Route to RTC domain for ultra-low power
  bool slp_sel_enable;                ///< Enable automatic sleep selection
  bool enable_sleep_retain;           ///< Enable sleep retention
};

/**
 * @brief GPIO wake-up configuration for deep sleep.
 * @details Configuration for GPIO-based wake-up from deep sleep modes.
 */
struct hf_gpio_wakeup_config_t {
  hf_gpio_intr_type_t wake_trigger;   ///< Wake-up trigger type
  bool enable_rtc_wake;               ///< Enable RTC domain wake-up
  bool enable_ext1_wake;              ///< Enable EXT1 wake-up source (multiple pins)
  uint8_t wake_level;                 ///< Wake-up level (0=low, 1=high)
  bool internal_pullup_enable;        ///< Enable internal pull-up during sleep
  bool internal_pulldown_enable;      ///< Enable internal pull-down during sleep
  bool iso_en;                        ///< Enable isolation during sleep
};

/**
 * @brief Dedicated GPIO bundle configuration.
 * @details Configuration for high-speed dedicated GPIO bundles.
 */
struct hf_dedic_gpio_bundle_config_t {
  const int *gpio_array;              ///< Array of GPIO numbers
  size_t array_size;                  ///< Number of GPIOs in array
  hf_dedic_gpio_bundle_flags_t flags; ///< Bundle configuration flags
};

/**
 * @brief Complete ESP32C6 GPIO configuration with all advanced features.
 * @details Comprehensive configuration structure for modern ESP32C6 GPIO.
 */
struct hf_gpio_advanced_config_t {
  hf_gpio_num_native_t gpio_num;                   ///< GPIO pin number
  hf_gpio_mode_t mode;                             ///< GPIO mode (input/output/etc)
  hf_gpio_pull_t pull_mode;                        ///< Pull resistor configuration
  hf_gpio_intr_type_t intr_type;                   ///< Interrupt trigger type
  hf_gpio_drive_cap_t drive_strength;              ///< Output drive capability
  hf_gpio_glitch_filter_type_t glitch_filter_type; ///< Glitch filter type
  hf_gpio_flex_filter_config_t flex_filter_config; ///< Flexible filter configuration
  hf_gpio_pin_filter_config_t pin_filter_config;   ///< Pin filter configuration
  hf_gpio_sleep_config_t sleep_config;             ///< Sleep mode configuration
  hf_gpio_wakeup_config_t wakeup_config;           ///< Wake-up configuration
  hf_lp_io_config_t lp_io_config;                  ///< Low-power IO configuration
  hf_gpio_etm_config_t etm_config;                 ///< ETM (Event Task Matrix) configuration
  bool enable_hold_function;                       ///< Enable GPIO hold function
  bool enable_rtc_gpio;                            ///< Enable RTC GPIO functionality
  bool enable_lp_io;                               ///< Enable LP_IO functionality
  bool enable_etm;                                 ///< Enable Event Task Matrix
};

/**
 * @brief Comprehensive ESP32C6 GPIO status information for diagnostics.
 * @details Complete status information for debugging, monitoring, and diagnostics.
 */
struct hf_gpio_status_info_t {
  uint8_t pin_number;                         ///< GPIO pin number
  hf_gpio_mode_t current_mode;                ///< Current GPIO mode
  hf_gpio_pull_t current_pull_mode;           ///< Current pull mode
  hf_gpio_drive_cap_t current_drive_cap;      ///< Current drive capability
  hf_gpio_intr_type_t interrupt_type;         ///< Current interrupt type
  bool input_enabled;                         ///< Input buffer enabled
  bool output_enabled;                        ///< Output buffer enabled
  bool open_drain;                            ///< Open drain mode active
  bool sleep_sel_enabled;                     ///< Sleep selection enabled
  bool hold_enabled;                          ///< Hold function enabled
  bool rtc_enabled;                           ///< RTC GPIO enabled
  bool lp_io_enabled;                         ///< LP_IO enabled
  bool etm_enabled;                           ///< ETM (Event Task Matrix) enabled
  uint32_t function_select;                   ///< IOMUX function selection
  hf_gpio_glitch_filter_type_t filter_type;   ///< Active glitch filter type
  bool glitch_filter_enabled;                 ///< Glitch filter enabled
  uint32_t interrupt_count;                   ///< Number of interrupts occurred
  bool is_wake_source;                        ///< Pin configured as wake source
  bool is_dedicated_gpio;                     ///< Pin used in dedicated GPIO bundle
  uint8_t dedicated_channel;                  ///< Dedicated GPIO channel number (if applicable)
  bool sleep_hold_active;                     ///< Sleep hold currently active
  uint32_t last_interrupt_time_us;            ///< Last interrupt timestamp (microseconds)
  bool etm_event_active;                      ///< ETM event generation active
  bool etm_task_active;                       ///< ETM task response active
  uint8_t etm_channel_number;                 ///< ETM channel number (if applicable)
  hf_gpio_etm_event_edge_t etm_event_edge;    ///< ETM event edge type
  hf_gpio_etm_task_action_t etm_task_action;  ///< ETM task action type
};

/**
 * @brief ESP32C6 GPIO pin capabilities and limitations.
 * @details Complete capability information for each GPIO pin.
 */
struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;                 ///< Pin can be used as GPIO
  bool supports_input;                ///< Pin supports input mode
  bool supports_output;               ///< Pin supports output mode
  bool supports_open_drain;           ///< Pin supports open-drain mode
  bool supports_pull_up;              ///< Pin supports internal pull-up
  bool supports_pull_down;            ///< Pin supports internal pull-down
  bool supports_adc;                  ///< Pin supports ADC functionality
  bool supports_dac;                  ///< Pin supports DAC functionality (if available)
  bool supports_rtc;                  ///< Pin supports RTC GPIO
  bool supports_lp_io;                ///< Pin supports LP_IO functionality
  bool supports_touch;                ///< Pin supports touch sensing
  bool supports_dedicated_gpio;       ///< Pin can be used in dedicated GPIO bundles
  bool supports_glitch_filter;        ///< Pin supports glitch filtering
  bool supports_etm;                  ///< Pin supports Event Task Matrix
  bool is_strapping_pin;              ///< Pin is a strapping pin (requires caution)
  bool is_spi_flash_pin;              ///< Pin is used for SPI flash (not recommended for GPIO)
  bool is_usb_jtag_pin;               ///< Pin is used for USB-JTAG (disables JTAG if reconfigured)
  uint8_t rtc_gpio_number;            ///< RTC GPIO number (if applicable, 0xFF if not RTC)
  uint8_t lp_gpio_number;             ///< Low-power GPIO number (if applicable, 0xFF if not LP)
  uint8_t adc_unit;                   ///< ADC unit number (if applicable, 0xFF if no ADC)
  uint8_t adc_channel;                ///< ADC channel number (if applicable, 0xFF if no ADC)
  uint8_t touch_channel;              ///< Touch channel number (if applicable, 0xFF if no touch)
  hf_gpio_drive_cap_t max_drive_strength; ///< Maximum supported drive strength
  uint32_t max_frequency_hz;          ///< Maximum supported toggle frequency
};

/**
 * @brief Native ESP-IDF v5.5+ GPIO types mapping.
 * @details Direct mappings to latest ESP-IDF GPIO types for maximum compatibility.
 */
using hf_gpio_config_native_t = gpio_config_t;
using hf_gpio_glitch_filter_handle_native_t = gpio_glitch_filter_handle_t;
using hf_gpio_pin_glitch_filter_config_native_t = gpio_pin_glitch_filter_config_t;
using hf_gpio_flex_glitch_filter_config_native_t = gpio_flex_glitch_filter_config_t;
using hf_rtc_gpio_mode_native_t = rtc_gpio_mode_t;

// Dedicated GPIO native types
using hf_dedic_gpio_bundle_handle_native_t = dedic_gpio_bundle_handle_t;
using hf_dedic_gpio_bundle_config_native_t = dedic_gpio_bundle_config_t;

// Low-Power IO native types (ESP-IDF v5.5+)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
using hf_lp_io_num_native_t = lp_io_num_t;
#else
using hf_lp_io_num_native_t = uint8_t;
#endif

// ETM (Event Task Matrix) native types for GPIO
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
using hf_gpio_etm_event_handle_native_t = gpio_etm_event_handle_t;
using hf_gpio_etm_task_handle_native_t = gpio_etm_task_handle_t;
using hf_gpio_etm_event_config_native_t = gpio_etm_event_config_t;
using hf_gpio_etm_task_config_native_t = gpio_etm_task_config_t;
using hf_gpio_etm_event_edge_native_t = gpio_etm_event_edge_t;
using hf_gpio_etm_task_action_native_t = gpio_etm_task_action_t;
#else
using hf_gpio_etm_event_handle_native_t = void*;
using hf_gpio_etm_task_handle_native_t = void*;
using hf_gpio_etm_event_config_native_t = struct { int dummy; };
using hf_gpio_etm_task_config_native_t = struct { int dummy; };
using hf_gpio_etm_event_edge_native_t = uint8_t;
using hf_gpio_etm_task_action_native_t = uint8_t;
#endif

#else
//==============================================================================
// NON-ESP32 PLATFORM SUPPORT (SIMPLIFIED COMPATIBILITY LAYER)
//==============================================================================

// Simplified GPIO types for non-ESP32 platforms
struct hf_gpio_advanced_config_t {
  uint32_t gpio_num;
  uint8_t mode;
  uint8_t pull_mode;
  uint8_t intr_type;
  uint8_t drive_strength;
  hf_gpio_etm_config_t etm_config;
  // Simplified structure for non-ESP32 platforms
};

struct hf_gpio_status_info_t {
  uint8_t pin_number;
  uint8_t current_mode;
  uint8_t current_pull_mode;
  uint8_t current_drive_cap;
  bool input_enabled;
  bool output_enabled;
  bool open_drain;
  bool hold_enabled;
  uint32_t interrupt_count;
  bool is_wake_source;
  bool etm_enabled;
  uint8_t etm_channel_number;
};

struct hf_gpio_pin_capabilities_t {
  bool is_valid_gpio;
  bool supports_input;
  bool supports_output;
  bool supports_adc;
  bool supports_pull_up;
  bool supports_pull_down;
  bool is_strapping_pin;
  uint8_t adc_unit;
  uint8_t adc_channel;
};

// Simplified structures for advanced features
struct hf_gpio_sleep_config_t {
  uint8_t sleep_mode;
  uint8_t sleep_pull_mode;
  bool hold_during_sleep;
};

struct hf_gpio_wakeup_config_t {
  uint8_t wake_trigger;
  bool enable_wake;
  uint8_t wake_level;
};

struct hf_gpio_flex_filter_config_t {
  uint32_t window_width_ns;
  uint32_t window_threshold_ns;
  bool enable_on_init;
};

struct hf_gpio_pin_filter_config_t {
  uint8_t clk_src;
  bool enable_on_init;
};

struct hf_lp_io_config_t {
  uint8_t mode;
  uint8_t pull_mode;
  bool hold_enable;
};

struct hf_gpio_etm_event_config_t {
  uint8_t edge;
  bool invert_output;
  bool enable_on_init;
};

struct hf_gpio_etm_task_config_t {
  uint8_t action;
  bool invert_output;
  bool enable_on_init;
};

struct hf_gpio_etm_config_t {
  bool enable_etm;
  uint8_t event_config;
  uint8_t task_config;
  uint8_t etm_channel_priority;
  bool auto_bind_gpio;
};

struct hf_dedic_gpio_bundle_config_t {
  const int *gpio_array;
  size_t array_size;
  uint32_t flags;
};

// Generic handle types for non-ESP32 platforms
using hf_gpio_config_native_t = struct { int dummy; };
using hf_gpio_glitch_filter_handle_native_t = void*;
using hf_gpio_pin_glitch_filter_config_native_t = struct { int dummy; };
using hf_gpio_flex_glitch_filter_config_native_t = struct { int dummy; };
using hf_rtc_gpio_mode_native_t = uint8_t;
using hf_dedic_gpio_bundle_handle_native_t = void*;
using hf_dedic_gpio_bundle_config_native_t = struct { int dummy; };
using hf_lp_io_num_native_t = uint8_t;
using hf_gpio_etm_event_handle_native_t = void*;
using hf_gpio_etm_task_handle_native_t = void*;
using hf_gpio_etm_event_config_native_t = struct { int dummy; };
using hf_gpio_etm_task_config_native_t = struct { int dummy; };
using hf_gpio_etm_event_edge_native_t = uint8_t;
using hf_gpio_etm_task_action_native_t = uint8_t;

#endif // HF_MCU_FAMILY_ESP32

//==============================================================================
// ESP32C6 GPIO VALIDATION MACROS AND CONSTANTS (ESP-IDF v5.5+)
//==============================================================================
// NOTE: Hardware constants are centrally defined in McuSelect.h - no duplication

#ifdef HF_MCU_ESP32C6
/**
 * @brief ESP32C6 GPIO pin capability validation macros.
 * @details All hardware constants reference McuSelect.h directly for single source of truth.
 */
#define HF_GPIO_IS_VALID_GPIO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= HF_MCU_GPIO_MAX_PIN_NUMBER)

#define HF_GPIO_IS_VALID_PIN(gpio_num) HF_GPIO_IS_VALID_GPIO(gpio_num)

#define HF_GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num) && !HF_GPIO_IS_INPUT_ONLY_PIN(gpio_num))

#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_IS_RTC_GPIO(gpio_num) HF_GPIO_IS_VALID_RTC_GPIO(gpio_num)

#define HF_GPIO_IS_VALID_LP_IO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= 7)

#define HF_GPIO_SUPPORTS_ADC(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) <= 6)

#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) \
  ((gpio_num) == 4 || (gpio_num) == 5 || (gpio_num) == 8 || (gpio_num) == 9 || (gpio_num) == 15)

#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) \
  ((gpio_num) >= 24 && (gpio_num) <= 30)

#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) \
  ((gpio_num) == 12 || (gpio_num) == 13)

#define HF_GPIO_IS_INPUT_ONLY_PIN(gpio_num) \
  (false) /* ESP32C6 has no input-only pins */

#define HF_GPIO_SUPPORTS_PULL_UP(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_PULL_DOWN(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_OPEN_DRAIN(gpio_num) \
  (HF_GPIO_IS_VALID_OUTPUT_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_GLITCH_FILTER(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num))

#define HF_GPIO_SUPPORTS_DEDICATED_GPIO(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num) && !HF_GPIO_IS_SPI_FLASH_PIN(gpio_num))

#define HF_GPIO_SUPPORTS_ETM(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num))

/**
 * @brief ESP32C6 GPIO to ADC channel mapping.
 */
#define HF_GPIO_TO_ADC_UNIT(gpio_num) \
  (HF_GPIO_SUPPORTS_ADC(gpio_num) ? 1 : 0xFF)

#define HF_GPIO_TO_ADC_CHANNEL(gpio_num) \
  (HF_GPIO_SUPPORTS_ADC(gpio_num) ? (gpio_num) : 0xFF)

/**
 * @brief ESP32C6 GPIO to RTC/LP_IO mapping.
 */
#define HF_GPIO_TO_RTC_GPIO(gpio_num) \
  (HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) ? (gpio_num) : 0xFF)

#define HF_GPIO_TO_LP_IO(gpio_num) \
  (HF_GPIO_IS_VALID_LP_IO(gpio_num) ? (gpio_num) : 0xFF)

/**
 * @brief ESP32C6 pin safety classification.
 */
#define HF_GPIO_IS_SAFE_FOR_GENERAL_USE(gpio_num) \
  (HF_GPIO_IS_VALID_GPIO(gpio_num) && \
   !HF_GPIO_IS_STRAPPING_PIN(gpio_num) && \
   !HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) && \
   !HF_GPIO_IS_USB_JTAG_PIN(gpio_num))

#else
//==============================================================================
// GENERIC PLATFORM GPIO VALIDATION MACROS
//==============================================================================

#define HF_GPIO_IS_VALID_GPIO(gpio_num) \
  ((gpio_num) >= 0 && (gpio_num) < 32)
#define HF_GPIO_IS_VALID_PIN(gpio_num) HF_GPIO_IS_VALID_GPIO(gpio_num)
#define HF_GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) HF_GPIO_IS_VALID_GPIO(gpio_num)
#define HF_GPIO_IS_VALID_RTC_GPIO(gpio_num) false
#define HF_GPIO_IS_RTC_GPIO(gpio_num) false
#define HF_GPIO_IS_VALID_LP_IO(gpio_num) false
#define HF_GPIO_SUPPORTS_ADC(gpio_num) false
#define HF_GPIO_IS_STRAPPING_PIN(gpio_num) false
#define HF_GPIO_IS_SPI_FLASH_PIN(gpio_num) false
#define HF_GPIO_IS_USB_JTAG_PIN(gpio_num) false
#define HF_GPIO_IS_INPUT_ONLY_PIN(gpio_num) false
#define HF_GPIO_SUPPORTS_PULL_UP(gpio_num) true
#define HF_GPIO_SUPPORTS_PULL_DOWN(gpio_num) true
#define HF_GPIO_SUPPORTS_OPEN_DRAIN(gpio_num) true
#define HF_GPIO_SUPPORTS_GLITCH_FILTER(gpio_num) false
#define HF_GPIO_SUPPORTS_DEDICATED_GPIO(gpio_num) false
#define HF_GPIO_SUPPORTS_ETM(gpio_num) false
#define HF_GPIO_TO_ADC_UNIT(gpio_num) 0xFF
#define HF_GPIO_TO_ADC_CHANNEL(gpio_num) 0xFF
#define HF_GPIO_TO_RTC_GPIO(gpio_num) 0xFF
#define HF_GPIO_TO_LP_IO(gpio_num) 0xFF
#define HF_GPIO_IS_SAFE_FOR_GENERAL_USE(gpio_num) HF_GPIO_IS_VALID_GPIO(gpio_num)

#endif // HF_MCU_ESP32C6

//==============================================================================
// CONVENIENCE TYPES AND UTILITY FUNCTIONS
//==============================================================================

/**
 * @brief GPIO interrupt callback function type.
 * @param gpio_num The GPIO number that triggered the interrupt
 * @param user_data User-provided data passed to the callback
 */
using hf_gpio_isr_callback_t = void(*)(uint32_t gpio_num, void* user_data);

/**
 * @brief GPIO bundle operations callback type for dedicated GPIO.
 * @param bundle_handle Handle to the GPIO bundle
 * @param mask GPIO mask within the bundle
 * @param user_data User-provided data
 */
using hf_gpio_bundle_callback_t = void(*)(void* bundle_handle, uint32_t mask, void* user_data);

/**
 * @brief GPIO configuration validation result.
 */
enum class hf_gpio_config_result_t : uint8_t {
  HF_GPIO_CONFIG_OK = 0,              ///< Configuration is valid
  HF_GPIO_CONFIG_INVALID_PIN = 1,     ///< Invalid pin number
  HF_GPIO_CONFIG_INVALID_MODE = 2,    ///< Invalid mode for this pin
  HF_GPIO_CONFIG_INVALID_PULL = 3,    ///< Invalid pull configuration
  HF_GPIO_CONFIG_INVALID_DRIVE = 4,   ///< Invalid drive strength
  HF_GPIO_CONFIG_STRAPPING_WARNING = 5, ///< Warning: strapping pin usage
  HF_GPIO_CONFIG_FLASH_WARNING = 6,   ///< Warning: SPI flash pin usage
  HF_GPIO_CONFIG_JTAG_WARNING = 7     ///< Warning: USB-JTAG pin usage
};

/**
 * @brief Utility structure for GPIO pin information lookup.
 */
struct hf_gpio_pin_info_t {
  uint8_t gpio_num;                   ///< GPIO number
  const char* pin_name;               ///< Human-readable pin name
  const char* alt_functions[4];       ///< Alternative functions available
  hf_gpio_pin_capabilities_t capabilities; ///< Pin capabilities
  const char* usage_notes;            ///< Special usage notes or warnings
};

#ifdef HF_MCU_ESP32C6
/**
 * @brief ESP32C6 GPIO pin information table.
 * @details Complete pin information for all ESP32C6 GPIO pins.
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
 * @brief Validate GPIO configuration for ESP32C6.
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
uint32_t hf_gpio_calc_glitch_filter_window(uint32_t noise_duration_ns, uint8_t safety_margin_percent);

/**
 * @brief Check if GPIO supports ETM functionality.
 * @param gpio_num GPIO number to check
 * @return true if GPIO supports ETM, false otherwise
 */
inline bool hf_gpio_supports_etm(uint8_t gpio_num) {
  return HF_GPIO_SUPPORTS_ETM(gpio_num);
}

/**
 * @brief Validate ETM configuration for given GPIO.
 * @param gpio_num GPIO number
 * @param etm_config ETM configuration to validate
 * @return Validation result
 */
hf_gpio_config_result_t hf_gpio_validate_etm_config(uint8_t gpio_num, 
                                                     const hf_gpio_etm_config_t* etm_config);

/**
 * @brief Get recommended ETM channel for GPIO operations.
 * @param gpio_num GPIO number
 * @param priority Priority level (0=highest, 255=lowest)
 * @return Recommended ETM channel number, 0xFF if none available
 */
uint8_t hf_gpio_get_optimal_etm_channel(uint8_t gpio_num, uint8_t priority);

#endif // HF_MCU_ESP32C6

//==============================================================================
// ERROR HANDLING AND DEBUGGING SUPPORT
//==============================================================================

/**
 * @brief GPIO operation result codes.
 */
enum class hf_gpio_result_t : uint8_t {
  HF_GPIO_OK = 0,                     ///< Operation successful
  HF_GPIO_ERR_INVALID_ARG = 1,        ///< Invalid argument
  HF_GPIO_ERR_INVALID_STATE = 2,      ///< Invalid state for operation
  HF_GPIO_ERR_NOT_SUPPORTED = 3,      ///< Operation not supported
  HF_GPIO_ERR_NO_MEM = 4,             ///< Out of memory
  HF_GPIO_ERR_TIMEOUT = 5,            ///< Operation timeout
  HF_GPIO_ERR_HW_FAULT = 6,           ///< Hardware fault
  HF_GPIO_ERR_BUSY = 7,               ///< Resource busy
  HF_GPIO_ERR_NOT_FOUND = 8           ///< Resource not found
};

/**
 * @brief Convert GPIO result code to human-readable string.
 * @param result Result code to convert
 * @return String description of the result
 */
constexpr const char* hf_gpio_result_to_string(hf_gpio_result_t result) {
  switch (result) {
    case hf_gpio_result_t::HF_GPIO_OK: return "Success";
    case hf_gpio_result_t::HF_GPIO_ERR_INVALID_ARG: return "Invalid argument";
    case hf_gpio_result_t::HF_GPIO_ERR_INVALID_STATE: return "Invalid state";
    case hf_gpio_result_t::HF_GPIO_ERR_NOT_SUPPORTED: return "Not supported";
    case hf_gpio_result_t::HF_GPIO_ERR_NO_MEM: return "Out of memory";
    case hf_gpio_result_t::HF_GPIO_ERR_TIMEOUT: return "Timeout";
    case hf_gpio_result_t::HF_GPIO_ERR_HW_FAULT: return "Hardware fault";
    case hf_gpio_result_t::HF_GPIO_ERR_BUSY: return "Resource busy";
    case hf_gpio_result_t::HF_GPIO_ERR_NOT_FOUND: return "Resource not found";
    default: return "Unknown error";
  }
}

//==============================================================================
// COMPILE-TIME CONFIGURATION VALIDATION
//==============================================================================

#ifdef HF_MCU_ESP32C6
// Compile-time assertions to ensure configuration consistency
static_assert(HF_MCU_GPIO_PIN_COUNT == 31, "ESP32C6 should have 31 GPIO pins");
static_assert(HF_MCU_GPIO_MAX_PIN_NUMBER == 30, "ESP32C6 max GPIO should be 30");
static_assert(HF_MCU_GPIO_RTC_PIN_COUNT == 8, "ESP32C6 should have 8 RTC GPIO pins");
static_assert(HF_MCU_GPIO_ADC_PIN_COUNT == 7, "ESP32C6 should have 7 ADC channels");
static_assert(HF_MCU_GPIO_FLEX_FILTER_COUNT == 8, "ESP32C6 should have 8 flex filters");
static_assert(HF_MCU_GPIO_ETM_CHANNEL_COUNT == 50, "ESP32C6 should have 50 ETM channels");

// Verify that our enum values match ESP-IDF native values
#ifdef GPIO_MODE_INPUT
static_assert(static_cast<int>(hf_gpio_mode_t::HF_GPIO_MODE_INPUT) == GPIO_MODE_INPUT, 
              "GPIO mode values must match ESP-IDF");
#endif

#ifdef GPIO_PULLUP_ONLY  
static_assert(static_cast<int>(hf_gpio_pull_t::HF_GPIO_PULL_UP) == GPIO_PULLUP_ONLY,
              "GPIO pull mode values must match ESP-IDF");
#endif

#ifdef GPIO_ETM_EVENT_EDGE_POS
static_assert(static_cast<int>(hf_gpio_etm_event_edge_t::HF_GPIO_ETM_EVENT_EDGE_POS) == GPIO_ETM_EVENT_EDGE_POS,
              "GPIO ETM event edge values must match ESP-IDF");
#endif

#ifdef GPIO_ETM_TASK_ACTION_SET
static_assert(static_cast<int>(hf_gpio_etm_task_action_t::HF_GPIO_ETM_TASK_ACTION_SET) == GPIO_ETM_TASK_ACTION_SET,
              "GPIO ETM task action values must match ESP-IDF");
#endif

#endif // HF_MCU_ESP32C6

/**
 * @brief ETM (Event Task Matrix) status information for diagnostics.
 * @details Status information for GPIO ETM configuration and usage.
 */
struct hf_gpio_etm_status_t {
  bool etm_enabled;                           ///< ETM functionality enabled
  void* event_handle;                         ///< ETM event handle (platform-specific)
  void* task_handle;                          ///< ETM task handle (platform-specific)
  void* channel_handle;                       ///< ETM channel handle (platform-specific)
  uint8_t total_etm_channels_used;            ///< Total ETM channels currently in use
  uint8_t max_etm_channels;                   ///< Maximum ETM channels available
  hf_gpio_etm_event_edge_t configured_edge;   ///< Configured event edge type
  hf_gpio_etm_task_action_t configured_action; ///< Configured task action type
  bool channel_enabled;                       ///< ETM channel currently enabled
  uint32_t event_count;                       ///< Number of ETM events triggered
  uint32_t task_execution_count;              ///< Number of ETM tasks executed
};
