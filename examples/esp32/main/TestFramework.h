/**
 * @file TestFramework.h
 * @brief Shared testing framework for ESP32-C6 comprehensive test suites
 *
 * This file provides common testing infrastructure including test result tracking,
 * execution timing, standardized test execution macros, memory validation, and
 * enhanced reporting used across all comprehensive test suites.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Test execution tracking and results accumulation
 */
struct TestResults {
  int total_tests = 0;
  int passed_tests = 0;
  int failed_tests = 0;
  uint64_t total_execution_time_us = 0;
  size_t initial_free_heap = 0;
  size_t minimum_free_heap = SIZE_MAX;

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
    
    // Track heap usage
    size_t current_free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    if (current_free_heap < minimum_free_heap) {
      minimum_free_heap = current_free_heap;
    }
  }

  /**
   * @brief Initialize heap tracking
   */
  void init_heap_tracking() noexcept {
    initial_free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    minimum_free_heap = initial_free_heap;
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

  /**
   * @brief Get average execution time per test in milliseconds
   * @return Average execution time per test in milliseconds
   */
  double get_average_time_ms() const noexcept {
    return total_tests > 0 ? (total_execution_time_us / 1000.0 / total_tests) : 0.0;
  }

  /**
   * @brief Get heap usage statistics
   * @return Heap usage in bytes (negative = memory leak, positive = freed memory)
   */
  int get_heap_usage() const noexcept {
    size_t current_free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    return static_cast<int>(current_free_heap) - static_cast<int>(initial_free_heap);
  }

  /**
   * @brief Get minimum free heap during test execution
   * @return Minimum free heap in bytes
   */
  size_t get_minimum_free_heap() const noexcept {
    return minimum_free_heap;
  }
};

/**
 * @brief Test group for organizing related tests
 */
struct TestGroup {
  const char* name;
  int tests_in_group = 0;
  int passed_in_group = 0;
  uint64_t group_execution_time_us = 0;

  void start_group() noexcept {
    tests_in_group = 0;
    passed_in_group = 0;
    group_execution_time_us = 0;
  }

  void add_test_result(bool passed, uint64_t execution_time) noexcept {
    tests_in_group++;
    group_execution_time_us += execution_time;
    if (passed) {
      passed_in_group++;
    }
  }

  double get_group_success_percentage() const noexcept {
    return tests_in_group > 0 ? (static_cast<double>(passed_in_group) / tests_in_group * 100.0) : 0.0;
  }

  double get_group_time_ms() const noexcept {
    return group_execution_time_us / 1000.0;
  }
};

/**
 * @brief Memory validation helper
 */
class MemoryValidator {
public:
  static bool validate_heap_integrity() noexcept {
    return heap_caps_check_integrity_all(true);
  }

  static void log_heap_info(const char* tag, const char* context = "current") noexcept {
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    
    ESP_LOGI(tag, "Heap info (%s): Free=%zu, Internal=%zu, Largest=%zu bytes", 
             context, free_heap, free_internal, largest_block);
  }

  static bool detect_memory_leak(size_t initial_heap, size_t threshold_bytes = 1024) noexcept {
    size_t current_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    return (initial_heap > current_heap) && ((initial_heap - current_heap) > threshold_bytes);
  }
};

/**
 * @brief Enhanced test execution macro with timing, result tracking, and memory validation
 *
 * This macro provides:
 * - Consistent test execution format
 * - Automatic timing measurement
 * - Result tracking and logging
 * - Memory leak detection
 * - Heap integrity validation
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
    size_t heap_before = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);                            \
    if (!MemoryValidator::validate_heap_integrity()) {                                           \
      ESP_LOGW(TAG, "Heap integrity check failed before test: " #test_func);                    \
    }                                                                                             \
    uint64_t start_time = esp_timer_get_time();                                                  \
    bool result = test_func();                                                                   \
    uint64_t end_time = esp_timer_get_time();                                                    \
    uint64_t execution_time = end_time - start_time;                                             \
    size_t heap_after = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);                            \
    g_test_results.add_result(result, execution_time);                                           \
    if (result) {                                                                                \
      ESP_LOGI(TAG, "[SUCCESS] PASSED: " #test_func " (%.2f ms)", execution_time / 1000.0);     \
    } else {                                                                                     \
      ESP_LOGE(TAG, "[FAILED] FAILED: " #test_func " (%.2f ms)", execution_time / 1000.0);      \
    }                                                                                            \
    if (heap_before != heap_after) {                                                            \
      int heap_change = static_cast<int>(heap_after) - static_cast<int>(heap_before);          \
      if (heap_change < 0) {                                                                    \
        ESP_LOGW(TAG, "Potential memory leak in " #test_func ": %d bytes", -heap_change);      \
      } else {                                                                                   \
        ESP_LOGI(TAG, "Memory freed in " #test_func ": %d bytes", heap_change);                \
      }                                                                                          \
    }                                                                                            \
    if (!MemoryValidator::validate_heap_integrity()) {                                          \
      ESP_LOGE(TAG, "Heap integrity check failed after test: " #test_func);                    \
    }                                                                                            \
    vTaskDelay(pdMS_TO_TICKS(100));                                                              \
  } while (0)

/**
 * @brief Start a test group
 */
#define START_TEST_GROUP(group_var, group_name)                                                  \
  do {                                                                                            \
    group_var.name = group_name;                                                                 \
    group_var.start_group();                                                                     \
    ESP_LOGI(TAG,                                                                                \
             "\n"                                                                                \
             "╔══════════════════════════════════════════════════════════════════════════════╗\n" \
             "║ Test Group: %-63s ║\n"                                                          \
             "╚══════════════════════════════════════════════════════════════════════════════╝", \
             group_name);                                                                        \
  } while (0)

/**
 * @brief Run a test within a group
 */
#define RUN_GROUP_TEST(group_var, test_func)                                                     \
  do {                                                                                            \
    size_t heap_before = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);                           \
    uint64_t start_time = esp_timer_get_time();                                                  \
    bool result = test_func();                                                                   \
    uint64_t end_time = esp_timer_get_time();                                                    \
    uint64_t execution_time = end_time - start_time;                                             \
    size_t heap_after = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);                            \
    g_test_results.add_result(result, execution_time);                                           \
    group_var.add_test_result(result, execution_time);                                          \
    if (result) {                                                                                \
      ESP_LOGI(TAG, "[SUCCESS] " #test_func " (%.2f ms)", execution_time / 1000.0);             \
    } else {                                                                                     \
      ESP_LOGE(TAG, "[FAILED] " #test_func " (%.2f ms)", execution_time / 1000.0);              \
    }                                                                                            \
    if (heap_before != heap_after) {                                                            \
      int heap_change = static_cast<int>(heap_after) - static_cast<int>(heap_before);          \
      if (heap_change < 0) {                                                                    \
        ESP_LOGW(TAG, "  Memory usage: %d bytes", -heap_change);                               \
      }                                                                                          \
    }                                                                                            \
    vTaskDelay(pdMS_TO_TICKS(50));                                                               \
  } while (0)

/**
 * @brief End a test group and print summary
 */
#define END_TEST_GROUP(group_var)                                                                \
  do {                                                                                            \
    ESP_LOGI(TAG, "Group '%s' Summary: %d/%d passed (%.1f%%), %.2f ms total",                  \
             group_var.name, group_var.passed_in_group, group_var.tests_in_group,              \
             group_var.get_group_success_percentage(), group_var.get_group_time_ms());          \
  } while (0)

/**
 * @brief Performance measurement helper
 */
class PerformanceMeasurement {
public:
  static double measure_operation_throughput(
    std::function<bool()> operation, 
    size_t data_size_bytes,
    int iterations = 100,
    const char* operation_name = "operation"
  ) noexcept {
    uint64_t start_time = esp_timer_get_time();
    int successful_operations = 0;
    
    for (int i = 0; i < iterations; i++) {
      if (operation()) {
        successful_operations++;
      }
    }
    
    uint64_t total_time_us = esp_timer_get_time() - start_time;
    
    if (successful_operations > 0) {
      double throughput_mbps = (successful_operations * data_size_bytes * 8.0 * 1000000.0) / 
                               (total_time_us * 1024.0 * 1024.0);
      ESP_LOGI("PERF", "%s: %d/%d successful, %.2f Mbps throughput", 
               operation_name, successful_operations, iterations, throughput_mbps);
      return throughput_mbps;
    }
    
    return 0.0;
  }

  static double measure_operation_latency(
    std::function<bool()> operation,
    int iterations = 100,
    const char* operation_name = "operation"
  ) noexcept {
    uint64_t total_time_us = 0;
    int successful_operations = 0;
    
    for (int i = 0; i < iterations; i++) {
      uint64_t start_time = esp_timer_get_time();
      if (operation()) {
        total_time_us += esp_timer_get_time() - start_time;
        successful_operations++;
      }
    }
    
    if (successful_operations > 0) {
      double avg_latency_us = static_cast<double>(total_time_us) / successful_operations;
      ESP_LOGI("PERF", "%s: %.2f μs average latency (%d successful operations)", 
               operation_name, avg_latency_us, successful_operations);
      return avg_latency_us;
    }
    
    return 0.0;
  }
};

/**
 * @brief Print comprehensive test summary with enhanced reporting
 * @param test_results The TestResults instance to summarize
 * @param test_suite_name Name of the test suite for logging
 * @param tag Logging tag
 */
inline void print_test_summary(const TestResults& test_results, const char* test_suite_name,
                               const char* tag) noexcept {
  ESP_LOGI(tag, "\n");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(tag, "║                           %s TEST SUMMARY                           ║", test_suite_name);
  ESP_LOGI(tag, "╠══════════════════════════════════════════════════════════════════════════════╣");
  ESP_LOGI(tag, "║ Tests:       %3d total, %3d passed, %3d failed                            ║",
           test_results.total_tests, test_results.passed_tests, test_results.failed_tests);
  ESP_LOGI(tag, "║ Success:     %.1f%%                                                      ║",
           test_results.get_success_percentage());
  ESP_LOGI(tag, "║ Time:        %.2f ms total, %.2f ms average                          ║",
           test_results.get_total_time_ms(), test_results.get_average_time_ms());
  
  // Memory statistics
  int heap_usage = test_results.get_heap_usage();
  if (heap_usage < 0) {
    ESP_LOGI(tag, "║ Memory:      %d bytes leaked, %zu bytes minimum free               ║",
             -heap_usage, test_results.get_minimum_free_heap());
  } else {
    ESP_LOGI(tag, "║ Memory:      %d bytes freed, %zu bytes minimum free                ║",
             heap_usage, test_results.get_minimum_free_heap());
  }
  
  ESP_LOGI(tag, "╚══════════════════════════════════════════════════════════════════════════════╝");

  if (test_results.failed_tests == 0) {
    ESP_LOGI(tag, "🎉 [SUCCESS] ALL %s TESTS PASSED!", test_suite_name);
  } else {
    ESP_LOGE(tag, "❌ [FAILED] %d out of %d tests failed. Review the results above.", 
             test_results.failed_tests, test_results.total_tests);
  }

  // Memory validation
  if (!MemoryValidator::validate_heap_integrity()) {
    ESP_LOGE(tag, "⚠️  [WARNING] Heap integrity check failed after test suite completion!");
  }

  // Memory leak detection
  if (MemoryValidator::detect_memory_leak(test_results.initial_free_heap)) {
    ESP_LOGW(tag, "⚠️  [WARNING] Potential memory leak detected!");
  }
}

/**
 * @brief Print system information for debugging
 */
inline void print_system_info(const char* tag) noexcept {
  ESP_LOGI(tag, "\n");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(tag, "║                            SYSTEM INFORMATION                               ║");
  ESP_LOGI(tag, "╚══════════════════════════════════════════════════════════════════════════════╝");
  
  // ESP-IDF and chip information
  ESP_LOGI(tag, "ESP-IDF Version: %s", esp_get_idf_version());
  
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  ESP_LOGI(tag, "Chip: %s Rev %d, %d cores",
           CONFIG_IDF_TARGET, chip_info.revision, chip_info.cores);
  
  // Memory information
  MemoryValidator::log_heap_info(tag, "system startup");
  
  // Free RTOS information
  ESP_LOGI(tag, "Free RTOS Tick Rate: %d Hz", configTICK_RATE_HZ);
  ESP_LOGI(tag, "Task Stack High Water Mark: %d bytes", uxTaskGetStackHighWaterMark(NULL));
}

/**
 * @brief Stress test helper for resource exhaustion testing
 */
class StressTestHelper {
public:
  static bool test_resource_exhaustion(
    std::function<int()> create_resource,
    std::function<bool(int)> cleanup_resource,
    int max_attempts = 100,
    const char* resource_name = "resource"
  ) noexcept {
    std::vector<int> created_resources;
    bool exhaustion_reached = false;
    
    // Try to exhaust resources
    for (int i = 0; i < max_attempts; i++) {
      int resource_id = create_resource();
      if (resource_id < 0) {
        ESP_LOGI("STRESS", "Resource exhaustion reached after %d %s instances", 
                 i, resource_name);
        exhaustion_reached = true;
        break;
      }
      created_resources.push_back(resource_id);
    }
    
    // Cleanup all created resources
    bool cleanup_success = true;
    for (int resource_id : created_resources) {
      if (!cleanup_resource(resource_id)) {
        ESP_LOGE("STRESS", "Failed to cleanup %s resource %d", resource_name, resource_id);
        cleanup_success = false;
      }
    }
    
    ESP_LOGI("STRESS", "Created %zu %s instances, cleanup %s", 
             created_resources.size(), resource_name, 
             cleanup_success ? "successful" : "failed");
    
    return exhaustion_reached && cleanup_success;
  }
};
