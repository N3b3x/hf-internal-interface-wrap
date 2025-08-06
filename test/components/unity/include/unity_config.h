/**
 * @file unity_config.h
 * @brief Unity testing framework configuration for ESP32 HardFOC IID project
 * 
 * This file configures Unity testing framework for optimal performance
 * on ESP32 microcontrollers with proper memory management and output handling.
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
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP32-specific Unity configuration
#define UNITY_INCLUDE_CONFIG_H 1

// Data type support
#define UNITY_SUPPORT_64 1
#define UNITY_INCLUDE_DOUBLE 1
#define UNITY_INCLUDE_FLOAT 1

// Output configuration
#define UNITY_OUTPUT_COLOR 1
#define UNITY_EXCLUDE_SETJMP_H 1

// Memory configuration for ESP32
#define UNITY_POINTER_WIDTH 32
#define UNITY_INT_WIDTH 32
#define UNITY_LONG_WIDTH 32

// Custom print functions for ESP32
#define UNITY_OUTPUT_CHAR(a) esp_unity_putc(a)
#define UNITY_OUTPUT_FLUSH() esp_unity_flush()
#define UNITY_OUTPUT_START() esp_unity_output_start()
#define UNITY_OUTPUT_COMPLETE() esp_unity_output_complete()

// Test timeout for ESP32 (in milliseconds)
#define UNITY_TEST_TIMEOUT_MS 30000

// Memory allocation for test names and descriptions
#define UNITY_MAX_TEST_NAME_LENGTH 128
#define UNITY_MAX_DESCRIPTION_LENGTH 256

// ESP32-specific test hooks
#define UNITY_BEGIN() esp_unity_begin()
#define UNITY_END() esp_unity_end()

// Custom assertion macros for ESP32 testing
#define TEST_ASSERT_ESP_OK(condition) \
    TEST_ASSERT_EQUAL_HEX32(ESP_OK, (condition))

#define TEST_ASSERT_NOT_ESP_OK(condition) \
    TEST_ASSERT_NOT_EQUAL_HEX32(ESP_OK, (condition))

#define TEST_ASSERT_ESP_ERR(expected_err, condition) \
    TEST_ASSERT_EQUAL_HEX32((expected_err), (condition))

// GPIO-specific test assertions
#define TEST_ASSERT_GPIO_SUCCESS(condition) \
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS, (condition))

#define TEST_ASSERT_GPIO_ERROR(expected_err, condition) \
    TEST_ASSERT_EQUAL_INT32((expected_err), (condition))

// ADC-specific test assertions
#define TEST_ASSERT_ADC_SUCCESS(condition) \
    TEST_ASSERT_EQUAL_INT32(hf_adc_err_t::ADC_SUCCESS, (condition))

#define TEST_ASSERT_ADC_ERROR(expected_err, condition) \
    TEST_ASSERT_EQUAL_INT32((expected_err), (condition))

// Test timing macros
#define TEST_ASSERT_EXECUTION_TIME_LESS_THAN(max_time_ms, expression) \
    do { \
        uint32_t start_time = xTaskGetTickCount(); \
        expression; \
        uint32_t end_time = xTaskGetTickCount(); \
        uint32_t elapsed_ms = (end_time - start_time) * portTICK_PERIOD_MS; \
        TEST_ASSERT_LESS_THAN_UINT32((max_time_ms), elapsed_ms); \
    } while(0)

// Function declarations for ESP32-specific Unity functions
void esp_unity_putc(int c);
void esp_unity_flush(void);
void esp_unity_output_start(void);
void esp_unity_output_complete(void);
int esp_unity_begin(void);
int esp_unity_end(void);

// Test fixture setup/teardown
void unity_test_setup(void);
void unity_test_teardown(void);

// Memory leak detection for tests
#ifdef CONFIG_HEAP_TRACING
#define UNITY_TEST_SETUP() unity_test_setup()
#define UNITY_TEST_TEARDOWN() unity_test_teardown()
#endif

#ifdef __cplusplus
}
#endif