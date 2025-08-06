/**
 * @file mock_state_manager.h
 * @brief Mock state manager for ESP-IDF mocking in unit tests
 * 
 * This file provides a centralized mock state management system that allows
 * test cases to configure mock behavior, track function calls, and verify
 * expected interactions with ESP-IDF APIs.
 * 
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Maximum number of mock function calls to track
#define MOCK_MAX_CALL_HISTORY 100
#define MOCK_MAX_GPIO_PINS 50
#define MOCK_MAX_ADC_CHANNELS 10

/**
 * @brief Mock call record structure
 */
typedef struct {
    const char* function_name;
    uint32_t call_count;
    uint32_t last_call_timestamp;
    void* last_args;
    size_t args_size;
} mock_call_record_t;

/**
 * @brief Mock GPIO pin state
 */
typedef struct {
    bool configured;
    uint32_t direction;     // gpio_mode_t
    uint32_t pull_mode;     // gpio_pull_mode_t
    uint32_t intr_type;     // gpio_int_type_t
    uint32_t level;         // 0 or 1
    bool interrupt_enabled;
    uint32_t interrupt_count;
} mock_gpio_state_t;

/**
 * @brief Mock ADC unit state
 */
typedef struct {
    bool initialized;
    uint32_t unit_id;
    uint32_t resolution;
    uint32_t sample_freq_hz;
    uint32_t conv_mode;
    bool calibration_enabled;
    uint32_t channel_count;
    uint32_t channels_configured[MOCK_MAX_ADC_CHANNELS];
    uint32_t raw_values[MOCK_MAX_ADC_CHANNELS];
    uint32_t voltage_values[MOCK_MAX_ADC_CHANNELS];
    bool continuous_mode_running;
    uint32_t continuous_sample_count;
} mock_adc_state_t;

/**
 * @brief Mock system state
 */
typedef struct {
    uint32_t tick_count;
    uint64_t time_us;
    uint32_t heap_free_size;
    uint32_t heap_min_free_size;
    bool timer_running;
    uint32_t timer_period_us;
} mock_system_state_t;

/**
 * @brief Main mock state structure
 */
typedef struct {
    // Call tracking
    mock_call_record_t call_history[MOCK_MAX_CALL_HISTORY];
    uint32_t call_history_index;
    
    // GPIO state
    mock_gpio_state_t gpio_pins[MOCK_MAX_GPIO_PINS];
    
    // ADC state
    mock_adc_state_t adc_units[2];  // ADC1 and ADC2
    
    // System state
    mock_system_state_t system;
    
    // Error injection
    bool inject_errors;
    uint32_t error_code_to_inject;
    const char* function_to_fail;
    uint32_t fail_after_call_count;
    
    // Logging
    bool logging_enabled;
    uint32_t log_level;
} mock_state_t;

// Global mock state
extern mock_state_t g_mock_state;

/**
 * @brief Initialize mock state manager
 */
void mock_state_init(void);

/**
 * @brief Reset all mock state to defaults
 */
void mock_state_reset(void);

/**
 * @brief Record a mock function call
 * @param function_name Name of the mocked function
 * @param args Pointer to arguments structure (optional)
 * @param args_size Size of arguments structure
 */
void mock_record_call(const char* function_name, void* args, size_t args_size);

/**
 * @brief Get call count for a specific function
 * @param function_name Name of the function
 * @return Number of times function was called
 */
uint32_t mock_get_call_count(const char* function_name);

/**
 * @brief Check if a function was called
 * @param function_name Name of the function
 * @return true if function was called, false otherwise
 */
bool mock_was_called(const char* function_name);

/**
 * @brief Get the last arguments passed to a function
 * @param function_name Name of the function
 * @return Pointer to last arguments, or NULL if not found
 */
void* mock_get_last_args(const char* function_name);

/**
 * @brief Configure error injection for a specific function
 * @param function_name Function to fail
 * @param error_code Error code to return
 * @param fail_after_call_count Fail after this many calls (0 = fail immediately)
 */
void mock_inject_error(const char* function_name, uint32_t error_code, uint32_t fail_after_call_count);

/**
 * @brief Clear error injection
 */
void mock_clear_error_injection(void);

/**
 * @brief Check if function should fail
 * @param function_name Name of the function
 * @return Error code to return, or 0 if no error
 */
uint32_t mock_should_fail(const char* function_name);

// GPIO mock state functions
/**
 * @brief Set mock GPIO pin state
 */
void mock_gpio_set_pin_state(uint32_t pin, uint32_t level);

/**
 * @brief Get mock GPIO pin state
 */
uint32_t mock_gpio_get_pin_state(uint32_t pin);

/**
 * @brief Configure mock GPIO pin
 */
void mock_gpio_configure_pin(uint32_t pin, uint32_t direction, uint32_t pull_mode);

/**
 * @brief Trigger mock GPIO interrupt
 */
void mock_gpio_trigger_interrupt(uint32_t pin);

// ADC mock state functions
/**
 * @brief Set mock ADC raw value
 */
void mock_adc_set_raw_value(uint32_t unit, uint32_t channel, uint32_t value);

/**
 * @brief Set mock ADC voltage value
 */
void mock_adc_set_voltage_value(uint32_t unit, uint32_t channel, uint32_t voltage_mv);

/**
 * @brief Configure mock ADC unit
 */
void mock_adc_configure_unit(uint32_t unit, uint32_t resolution, uint32_t sample_freq);

/**
 * @brief Start/stop mock ADC continuous mode
 */
void mock_adc_set_continuous_mode(uint32_t unit, bool running);

// System mock state functions
/**
 * @brief Advance mock system time
 */
void mock_system_advance_time(uint32_t ticks);

/**
 * @brief Set mock heap free size
 */
void mock_system_set_heap_free_size(uint32_t size);

/**
 * @brief Enable/disable mock logging
 */
void mock_system_set_logging(bool enabled, uint32_t level);

#ifdef __cplusplus
}
#endif