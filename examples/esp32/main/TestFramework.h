/**
 * @file TestFramework.h
 * @brief Shared testing framework for ESP32-C6 comprehensive test suites
 *
 * This file provides common testing infrastructure including test result tracking,
 * execution timing, and standardized test execution macros used across all
 * comprehensive test suites.
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

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

/**
 * @brief Test execution tracking and results accumulation
 */
struct TestResults {
  int total_tests = 0;
  int passed_tests = 0;
  int failed_tests = 0;
  uint64_t total_execution_time_us = 0;

  /**
   * @brief Add test result and update statistics
   * @param passed Whether the test passed
   * @param execution_time Test execution time in microseconds
   */
  void add_result(bool passed, uint64_t execution_time) noexcept {
    total_tests++;
    total_execution_time_us += execution_time;
    if (passed) {
      passed_tests++;
    } else {
      failed_tests++;
    }
  }

  /**
   * @brief Calculate success percentage
   * @return Success percentage (0.0 to 100.0)
   */
  double get_success_percentage() const noexcept {
    return total_tests > 0 ? (static_cast<double>(passed_tests) / total_tests * 100.0) : 0.0;
  }

  /**
   * @brief Get total execution time in milliseconds
   * @return Total execution time in milliseconds
   */
  double get_total_time_ms() const noexcept {
    return total_execution_time_us / 1000.0;
  }
};

/**
 * @brief Standardized test execution macro with timing and result tracking
 *
 * This macro provides:
 * - Consistent test execution format
 * - Automatic timing measurement
 * - Result tracking and logging
 * - Standardized success/failure reporting
 *
 * @param test_func The test function to execute (must return bool)
 *
 * Requirements:
 * - TAG must be defined as const char* for logging
 * - g_test_results must be defined as TestResults instance
 * - test_func must be a function returning bool (true = pass, false = fail)
 */
#define RUN_TEST(test_func)                                                                       \
  do {                                                                                            \
    ESP_LOGI(TAG,                                                                                 \
             "\n"                                                                                 \
             "╔══════════════════════════════════════════════════════════════════════════════╗\n" \
             "║ Running: " #test_func "                                                     \n"   \
             "╚══════════════════════════════════════════════════════════════════════════════╝"); \
    uint64_t start_time = esp_timer_get_time();                                                   \
    bool result = test_func();                                                                    \
    uint64_t end_time = esp_timer_get_time();                                                     \
    uint64_t execution_time = end_time - start_time;                                              \
    g_test_results.add_result(result, execution_time);                                            \
    if (result) {                                                                                 \
      ESP_LOGI(TAG, "[SUCCESS] PASSED: " #test_func " (%.2f ms)", execution_time / 1000.0);       \
    } else {                                                                                      \
      ESP_LOGE(TAG, "[FAILED] FAILED: " #test_func " (%.2f ms)", execution_time / 1000.0);        \
    }                                                                                             \
    vTaskDelay(pdMS_TO_TICKS(100));                                                               \
  } while (0)

/**
 * @brief Context passed to test task trampoline
 */
struct TestTaskContext {
  const char* test_name;
  bool (*test_func)() noexcept;
  TestResults* results;
  const char* tag;
  SemaphoreHandle_t completion_semaphore;  // Add semaphore for synchronization
};

/**
 * @brief FreeRTOS task trampoline to execute a test with a larger dedicated stack
 */
inline void test_task_trampoline(void* param) {
  TestTaskContext* ctx = static_cast<TestTaskContext*>(param);
  ESP_LOGI(ctx->tag,
           "\n"
           "╔══════════════════════════════════════════════════════════════════════════════╗\n"
           "║ Running (task): %s                                                            \n"
           "╚══════════════════════════════════════════════════════════════════════════════╝",
           ctx->test_name);
  uint64_t start_time = esp_timer_get_time();
  bool result = ctx->test_func();
  uint64_t end_time = esp_timer_get_time();
  uint64_t execution_time = end_time - start_time;
  ctx->results->add_result(result, execution_time);
  if (result) {
    ESP_LOGI(ctx->tag, "[SUCCESS] PASSED (task): %s (%.2f ms)", ctx->test_name,
             execution_time / 1000.0);
  } else {
    ESP_LOGE(ctx->tag, "[FAILED] FAILED (task): %s (%.2f ms)", ctx->test_name,
             execution_time / 1000.0);
  }
  
  // Signal completion before deleting task
  if (ctx->completion_semaphore != nullptr) {
    xSemaphoreGive(ctx->completion_semaphore);
  }
  
  vTaskDelete(nullptr);
}

/**
 * @brief Run a test function inside its own FreeRTOS task with a custom stack size
 * @param name Test name (string literal)
 * @param func Boolean test function pointer (noexcept)
 * @param stack_size_bytes Task stack size in bytes
 * @param priority Task priority (defaults to 5)
 */
#define RUN_TEST_IN_TASK(name, func, stack_size_bytes, priority)                                \
  do {                                                                                          \
    static TestTaskContext ctx;                                                                 \
    ctx.test_name = name;                                                                       \
    ctx.test_func = func;                                                                       \
    ctx.results = &g_test_results;                                                              \
    ctx.tag = TAG;                                                                              \
    ctx.completion_semaphore = xSemaphoreCreateBinary();                                        \
    if (ctx.completion_semaphore == nullptr) {                                                  \
      ESP_LOGE(TAG, "Failed to create semaphore for test: %s", name);                          \
      /* Fallback: run inline to avoid losing coverage */                                       \
      RUN_TEST(func);                                                                           \
    } else {                                                                                    \
      BaseType_t created =                                                                      \
          xTaskCreate(test_task_trampoline, name, (stack_size_bytes) / sizeof(StackType_t), &ctx, \
                      (priority), nullptr);                                                     \
      if (created != pdPASS) {                                                                  \
        ESP_LOGE(TAG, "Failed to create test task: %s", name);                                 \
        vSemaphoreDelete(ctx.completion_semaphore);                                             \
        /* Fallback: run inline to avoid losing coverage */                                     \
        RUN_TEST(func);                                                                         \
      } else {                                                                                  \
        /* Wait for test completion using semaphore with timeout */                             \
        if (xSemaphoreTake(ctx.completion_semaphore, pdMS_TO_TICKS(30000)) == pdTRUE) {        \
          ESP_LOGI(TAG, "Test task completed: %s", name);                                       \
        } else {                                                                                \
          ESP_LOGW(TAG, "Test task timeout: %s", name);                                         \
        }                                                                                       \
        vSemaphoreDelete(ctx.completion_semaphore);                                             \
        /* Add small delay between tests to ensure proper cleanup */                             \
        vTaskDelay(pdMS_TO_TICKS(100));                                                         \
      }                                                                                         \
    }                                                                                           \
  } while (0)

/**
 * @brief Print standardized test summary
 * @param test_results The TestResults instance to summarize
 * @param test_suite_name Name of the test suite for logging
 */
inline void print_test_summary(const TestResults& test_results, const char* test_suite_name,
                               const char* tag) noexcept {
  ESP_LOGI(tag, "\n=== %s TEST SUMMARY ===", test_suite_name);
  ESP_LOGI(tag, "Total: %d, Passed: %d, Failed: %d, Success: %.2f%%, Time: %.2f ms",
           test_results.total_tests, test_results.passed_tests, test_results.failed_tests,
           test_results.get_success_percentage(), test_results.get_total_time_ms());

  if (test_results.failed_tests == 0) {
    ESP_LOGI(tag, "[SUCCESS] ALL %s TESTS PASSED!", test_suite_name);
  } else {
    ESP_LOGE(tag, "[FAILED] Some tests failed. Review the results above.");
  }
}

/**
 * @brief Print standardized test summary
 * @param test_results The TestResults instance to summarize
 * @param test_suite_name Name of the test suite for logging
 */
inline void print_test_section_status(const char* tag, const char* test_suite_name) noexcept {
  ESP_LOGI(tag, "\n");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(tag, "║                    %s TEST SECTION CONFIGURATION                             ", test_suite_name);
  ESP_LOGI(tag, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(tag, "To modify test sections, edit the defines at the top of your test file");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
}

/**
 * @brief Test section configuration helper macros
 * 
 * These macros provide conditional test execution based on constexpr constants at the top of test files.
 * They allow test suites to selectively enable/disable specific test categories.
 * 
 * Usage:
 * 1. Define test section enables at the top of your test file using constexpr:
 *    static constexpr bool ENABLE_BASIC_TESTS = true;
 *    static constexpr bool ENABLE_ADVANCED_TESTS = false;
 * 
 * 2. Use the macros to conditionally run test sections:
 *    RUN_TEST_SECTION_IF_ENABLED(ENABLE_BASIC_TESTS, "BASIC TESTS",
 *      RUN_TEST_IN_TASK("test1", test_function1, 8192, 1);
 *      flip_test_progress_indicator();
 *    );
 */

// Macro to conditionally run a test section
#define RUN_TEST_SECTION_IF_ENABLED(define_name, section_name, ...) \
  do { \
    if (define_name) { \
      ESP_LOGI(TAG, "\n=== %s ===", section_name); \
      __VA_ARGS__ \
    } else { \
      ESP_LOGI(TAG, "\n=== %s ===", section_name); \
      ESP_LOGI(TAG, "Section disabled by configuration"); \
    } \
  } while(0)

// Macro to conditionally run a single test
#define RUN_SINGLE_TEST_IF_ENABLED(define_name, test_name, test_func, stack_size, priority) \
  do { \
    if (define_name) { \
      RUN_TEST_IN_TASK(test_name, test_func, stack_size, priority); \
      flip_test_progress_indicator(); \
    } else { \
      ESP_LOGI(TAG, "Test '%s' disabled by configuration", test_name); \
    } \
  } while(0)

// Macro to conditionally run multiple tests in a section
#define RUN_TEST_GROUP_IF_ENABLED(define_name, section_name, ...) \
  RUN_TEST_SECTION_IF_ENABLED(define_name, section_name, __VA_ARGS__)

// Macro to conditionally run a test section with custom progress indicator
#define RUN_TEST_SECTION_IF_ENABLED_WITH_PROGRESS(define_name, section_name, progress_func, ...) \
  do { \
    if (define_name) { \
      ESP_LOGI(TAG, "\n=== %s ===", section_name); \
      __VA_ARGS__ \
      if (progress_func) progress_func(); \
    } else { \
      ESP_LOGI(TAG, "\n=== %s ===", section_name); \
      ESP_LOGI(TAG, "Section disabled by configuration"); \
    } \
  } while(0)

// Macro to conditionally run a test section with automatic progress indicator
#define RUN_TEST_SECTION_IF_ENABLED_AUTO_PROGRESS(define_name, section_name, ...) \
  RUN_TEST_SECTION_IF_ENABLED_WITH_PROGRESS(define_name, section_name, flip_test_progress_indicator, __VA_ARGS__)

/**
 * @brief Test section configuration template
 * 
 * Copy this template to the top of your test file and customize:
 * 
 * //=============================================================================
 * // TEST SECTION CONFIGURATION
 * //=============================================================================
 * // Enable/disable specific test sections by setting to true or false
 * 
 * // Basic functionality tests
 * static constexpr bool ENABLE_BASIC_TESTS = true;
 * static constexpr bool ENABLE_INITIALIZATION_TESTS = true;
 * static constexpr bool ENABLE_CONFIGURATION_TESTS = true;
 * 
 * // Core feature tests
 * static constexpr bool ENABLE_CORE_FEATURE_TESTS = true;
 * static constexpr bool ENABLE_OPERATION_TESTS = true;
 * static constexpr bool ENABLE_ERROR_HANDLING_TESTS = true;
 * 
 * // Advanced feature tests
 * static constexpr bool ENABLE_ADVANCED_FEATURE_TESTS = true;
 * static constexpr bool ENABLE_PERFORMANCE_TESTS = true;
 * static constexpr bool ENABLE_EDGE_CASE_TESTS = true;
 * 
 * // Specialized tests
 * static constexpr bool ENABLE_SPECIALIZED_TESTS = true;
 * static constexpr bool ENABLE_COMPATIBILITY_TESTS = true;
 * 
 * //=============================================================================
 * // TEST SECTION EXECUTION
 * //=============================================================================
 * // Use the macros below in your main function:
 * 
 * RUN_TEST_SECTION_IF_ENABLED(ENABLE_BASIC_TESTS, "BASIC TESTS",
 *   RUN_TEST_IN_TASK("test1", test_function1, 8192, 1);
 *   flip_test_progress_indicator();
 * );
 * 
 * RUN_TEST_SECTION_IF_ENABLED(ENABLE_CORE_FEATURE_TESTS, "CORE FEATURE TESTS",
 *   RUN_TEST_IN_TASK("test2", test_function2, 8192, 1);
 * );
 */
