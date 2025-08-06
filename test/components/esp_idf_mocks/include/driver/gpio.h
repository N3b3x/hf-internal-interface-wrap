/**
 * @file gpio.h
 * @brief Mock GPIO driver header for ESP-IDF unit testing
 * 
 * This file provides mock implementations of ESP-IDF GPIO functions
 * for unit testing EspGpio class without actual hardware dependencies.
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
#include "esp_err.h"

// GPIO pin definitions for ESP32-C6 (can be extended for other variants)
#define SOC_GPIO_PIN_COUNT 30
#define GPIO_NUM_0 0
#define GPIO_NUM_1 1
#define GPIO_NUM_2 2
#define GPIO_NUM_3 3
#define GPIO_NUM_4 4
#define GPIO_NUM_5 5
#define GPIO_NUM_6 6
#define GPIO_NUM_7 7
#define GPIO_NUM_8 8
#define GPIO_NUM_9 9
#define GPIO_NUM_10 10
#define GPIO_NUM_11 11
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_15 15
#define GPIO_NUM_16 16
#define GPIO_NUM_17 17
#define GPIO_NUM_18 18
#define GPIO_NUM_19 19
#define GPIO_NUM_20 20
#define GPIO_NUM_21 21
#define GPIO_NUM_22 22
#define GPIO_NUM_23 23
#define GPIO_NUM_24 24
#define GPIO_NUM_25 25
#define GPIO_NUM_26 26
#define GPIO_NUM_27 27
#define GPIO_NUM_28 28
#define GPIO_NUM_29 29
#define GPIO_NUM_MAX 30

typedef int gpio_num_t;

/**
 * @brief GPIO direction
 */
typedef enum {
    GPIO_MODE_DISABLE = 0,     ///< GPIO disabled
    GPIO_MODE_INPUT,           ///< GPIO input
    GPIO_MODE_OUTPUT,          ///< GPIO output
    GPIO_MODE_OUTPUT_OD,       ///< GPIO open drain output
    GPIO_MODE_INPUT_OUTPUT_OD, ///< GPIO input and open drain output
    GPIO_MODE_INPUT_OUTPUT,    ///< GPIO input and output
} gpio_mode_t;

/**
 * @brief GPIO pull mode
 */
typedef enum {
    GPIO_PULLUP_DISABLE = 0,   ///< Disable GPIO pull up resistor
    GPIO_PULLUP_ENABLE,        ///< Enable GPIO pull up resistor
} gpio_pullup_t;

typedef enum {
    GPIO_PULLDOWN_DISABLE = 0, ///< Disable GPIO pull down resistor
    GPIO_PULLDOWN_ENABLE,      ///< Enable GPIO pull down resistor
} gpio_pulldown_t;

typedef enum {
    GPIO_FLOATING = 0,
    GPIO_PULLUP_ONLY,
    GPIO_PULLDOWN_ONLY,
    GPIO_PULLUP_PULLDOWN,
} gpio_pull_mode_t;

/**
 * @brief GPIO interrupt type
 */
typedef enum {
    GPIO_INTR_DISABLE = 0,     ///< Disable GPIO interrupt
    GPIO_INTR_POSEDGE,         ///< GPIO interrupt on positive edge
    GPIO_INTR_NEGEDGE,         ///< GPIO interrupt on negative edge
    GPIO_INTR_ANYEDGE,         ///< GPIO interrupt on both edges
    GPIO_INTR_LOW_LEVEL,       ///< GPIO interrupt on low level
    GPIO_INTR_HIGH_LEVEL,      ///< GPIO interrupt on high level
    GPIO_INTR_MAX,
} gpio_int_type_t;

/**
 * @brief GPIO drive capability
 */
typedef enum {
    GPIO_DRIVE_CAP_0 = 0,      ///< Pad drive capability: weak
    GPIO_DRIVE_CAP_1,          ///< Pad drive capability: stronger
    GPIO_DRIVE_CAP_2,          ///< Pad drive capability: medium
    GPIO_DRIVE_CAP_DEFAULT = GPIO_DRIVE_CAP_2,
    GPIO_DRIVE_CAP_3,          ///< Pad drive capability: strongest
    GPIO_DRIVE_CAP_MAX,
} gpio_drive_cap_t;

/**
 * @brief GPIO configuration structure
 */
typedef struct {
    uint64_t pin_bit_mask;          ///< GPIO pin: set with bit mask
    gpio_mode_t mode;               ///< GPIO mode: set input/output mode
    gpio_pullup_t pull_up_en;       ///< GPIO pull-up
    gpio_pulldown_t pull_down_en;   ///< GPIO pull-down
    gpio_int_type_t intr_type;      ///< GPIO interrupt type
} gpio_config_t;

typedef void (*gpio_isr_t)(void*);

// Mock GPIO function declarations
esp_err_t gpio_config(const gpio_config_t *pGPIOConfig);
esp_err_t gpio_reset_pin(gpio_num_t gpio_num);
esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode);
esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level);
int gpio_get_level(gpio_num_t gpio_num);
esp_err_t gpio_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull);
esp_err_t gpio_set_drive_capability(gpio_num_t gpio_num, gpio_drive_cap_t strength);
esp_err_t gpio_get_drive_capability(gpio_num_t gpio_num, gpio_drive_cap_t* strength);

// GPIO interrupt functions
esp_err_t gpio_install_isr_service(int intr_alloc_flags);
void gpio_uninstall_isr_service(void);
esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void* args);
esp_err_t gpio_isr_handler_remove(gpio_num_t gpio_num);
esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type);
esp_err_t gpio_intr_enable(gpio_num_t gpio_num);
esp_err_t gpio_intr_disable(gpio_num_t gpio_num);

// GPIO wake up
esp_err_t gpio_wakeup_enable(gpio_num_t gpio_num, gpio_int_type_t intr_type);
esp_err_t gpio_wakeup_disable(gpio_num_t gpio_num);

// GPIO hold
esp_err_t gpio_hold_en(gpio_num_t gpio_num);
esp_err_t gpio_hold_dis(gpio_num_t gpio_num);
void gpio_deep_sleep_hold_en(void);
void gpio_deep_sleep_hold_dis(void);

// GPIO utility functions
bool gpio_is_valid_gpio(gpio_num_t gpio_num);

// Advanced GPIO features (ESP32-C6 specific)
esp_err_t gpio_sleep_set_direction(gpio_num_t gpio_num, gpio_mode_t mode);
esp_err_t gpio_sleep_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull);

#ifdef __cplusplus
}
#endif