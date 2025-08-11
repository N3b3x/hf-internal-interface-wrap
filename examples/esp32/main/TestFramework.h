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
             "║ Running: " #test_func "                                                    ║\n"   \
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
};

/**
 * @brief FreeRTOS task trampoline to execute a test with a larger dedicated stack
 */
inline void test_task_trampoline(void* param) {
  TestTaskContext* ctx = static_cast<TestTaskContext*>(param);
  ESP_LOGI(ctx->tag,
           "\n"
           "╔══════════════════════════════════════════════════════════════════════════════╗\n"
           "║ Running (task): %s                                                    ║\n"
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
    BaseType_t created =                                                                        \
        xTaskCreate(test_task_trampoline, name, (stack_size_bytes) / sizeof(StackType_t), &ctx, \
                    (priority), nullptr);                                                       \
    if (created != pdPASS) {                                                                    \
      ESP_LOGE(TAG, "Failed to create test task: %s", name);                                    \
      /* Fallback: run inline to avoid losing coverage */                                       \
      RUN_TEST(func);                                                                           \
    } else {                                                                                    \
      /* Wait for completion: simple delay loop until result count increases */                 \
      int before = g_test_results.total_tests;                                                  \
      for (int i_wait = 0; i_wait < 1000; ++i_wait) {                                           \
        if (g_test_results.total_tests > before)                                                \
          break;                                                                                \
        vTaskDelay(pdMS_TO_TICKS(10));                                                          \
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
