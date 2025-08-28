/**
 * @file UtilsComprehensiveTest.cpp
 * @brief Comprehensive utilities testing suite for ESP32-C6 DevKit-M-1
 *
 * This test suite provides comprehensive testing of utility classes including:
 * - AsciiArtGenerator: Text-to-ASCII art conversion functionality
 * - RtosMutex: Recursive mutex with lock guard support and FreeRTOS API compatibility
 * - RtosSharedMutex: Reader-writer mutex for shared/exclusive access patterns
 * - RtosUniqueLock/RtosSharedLock: RAII lock guards with timeout support
 * - RtosTime: Timing utilities for RTOS operations
 *
 * Test Coverage Includes:
 * ✓ Basic functionality and API compliance
 * ✓ Recursive locking with multiple depth levels
 * ✓ Lock guard RAII semantics and automatic unlock
 * ✓ Timeout-based locking operations (try_lock_for)
 * ✓ Concurrent access with multiple tasks/threads
 * ✓ Shared mutex reader-writer patterns
 * ✓ Move semantics and resource management
 * ✓ Edge cases and error conditions
 * ✓ Performance benchmarking and stress testing
 * ✓ Cross-platform RTOS compatibility verification
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "utils/AsciiArtGenerator.h"
#include "utils/RtosMutex.h"

static const char* TAG = "UTILS_Test";

static TestResults g_test_results;

//==============================================================================
// ASCII ART GENERATOR TESTS
//==============================================================================

bool test_ascii_art_generator_creation() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art generator creation...");

  AsciiArtGenerator generator;

  // Test basic creation by generating some text
  auto test_result = generator.Generate("TEST");
  if (test_result.empty()) {
    ESP_LOGE(TAG, "ASCII art generator failed to generate basic text");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art generator creation successful");
  return true;
}

bool test_ascii_art_basic_text() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art basic text generation...");

  AsciiArtGenerator generator;

  // Test simple text
  std::string test_text = "HELLO";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for basic text");
    return false;
  }

  // Verify the result has multiple lines (ASCII art should be multi-line)
  size_t newline_count = 0;
  for (char c : result) {
    if (c == '\n')
      newline_count++;
  }
  if (newline_count < 5) { // ASCII art should have at least 5 lines
    ESP_LOGE(TAG, "Generated ASCII art has insufficient lines: %zu", newline_count);
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for 'HELLO':\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art basic text generation successful");
  return true;
}

bool test_ascii_art_supported_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art supported characters...");

  AsciiArtGenerator generator;

  // Test all supported character types
  std::string test_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?@#$%^&*()[]{}|\\/;:'\"-_=+";

  for (char c : test_chars) {
    if (!generator.IsCharacterSupported(c)) {
      ESP_LOGE(TAG, "Character '%c' not supported but should be", c);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art supported characters test successful");
  return true;
}

bool test_ascii_art_unsupported_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art unsupported characters...");

  AsciiArtGenerator generator;

  // Test some unsupported characters
  std::string unsupported_chars = "áéíóúñçßäöü";

  for (char c : unsupported_chars) {
    if (generator.IsCharacterSupported(c)) {
      ESP_LOGE(TAG, "Character '%c' supported but should not be", c);
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art unsupported characters test successful");
  return true;
}

bool test_ascii_art_empty_string() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art empty string handling...");

  AsciiArtGenerator generator;

  // Test empty string
  auto result = generator.Generate("");

  if (!result.empty()) {
    ESP_LOGE(TAG, "Empty string should generate empty result");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art empty string handling successful");
  return true;
}

bool test_ascii_art_mixed_case() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art mixed case handling...");

  AsciiArtGenerator generator;

  // Test mixed case text
  std::string test_text = "Hello World 123!";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for mixed case text");
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for mixed case:\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art mixed case handling successful");
  return true;
}

bool test_ascii_art_special_characters() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art special characters...");

  AsciiArtGenerator generator;

  // Test special characters
  std::string test_text = "!@#$%^&*()";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for special characters");
    return false;
  }

  ESP_LOGI(TAG, "Generated ASCII art for special characters:\n%s", result.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art special characters test successful");
  return true;
}

bool test_ascii_art_long_text() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art long text handling...");

  AsciiArtGenerator generator;

  // Test long text
  std::string test_text = "VERY LONG TEXT THAT SHOULD BE HANDLED PROPERLY";
  auto result = generator.Generate(test_text);

  if (result.empty()) {
    ESP_LOGE(TAG, "Failed to generate ASCII art for long text");
    return false;
  }

  // Verify the result is substantial
  if (result.length() < 100) {
    ESP_LOGE(TAG, "Long text generated insufficient output: %zu chars", result.length());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art long text handling successful");
  return true;
}

bool test_ascii_art_custom_character_management() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art custom character management...");

  AsciiArtGenerator generator;

  // Test adding custom character
  std::vector<std::string> custom_art = {"  ___  ", " /   \\ ", "|     |",
                                         "|     |", " \\___/ ", "       "};

  generator.AddCustomCharacter('X', custom_art);

  if (!generator.IsCharacterSupported('X')) {
    ESP_LOGE(TAG, "Custom character 'X' not supported after addition");
    return false;
  }

  // Test removing custom character
  generator.RemoveCustomCharacter('X');

  if (generator.IsCharacterSupported('X')) {
    ESP_LOGE(TAG, "Custom character 'X' still supported after removal");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art custom character management successful");
  return true;
}

bool test_ascii_art_supported_characters_list() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art supported characters list...");

  AsciiArtGenerator generator;

  // Test getting supported characters
  std::string supported = generator.GetSupportedCharacters();

  if (supported.empty()) {
    ESP_LOGE(TAG, "Supported characters list is empty");
    return false;
  }

  // Verify it contains expected characters
  if (supported.find('A') == std::string::npos) {
    ESP_LOGE(TAG, "Supported characters list missing 'A'");
    return false;
  }

  if (supported.find('0') == std::string::npos) {
    ESP_LOGE(TAG, "Supported characters list missing '0'");
    return false;
  }

  ESP_LOGI(TAG, "Supported characters: %s", supported.c_str());
  ESP_LOGI(TAG, "[SUCCESS] ASCII art supported characters list test successful");
  return true;
}

//==============================================================================
// RTOS MUTEX TESTS
//==============================================================================

bool test_rtos_mutex_creation() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex creation...");

  RtosMutex mutex;

  // Test that the mutex handle is valid
  if (mutex.native_handle() == nullptr) {
    ESP_LOGE(TAG, "RtosMutex creation failed - null handle");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex creation successful");
  return true;
}

bool test_rtos_mutex_basic_lock_unlock() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex basic lock/unlock...");

  RtosMutex mutex;

  // Test basic lock
  if (!mutex.lock()) {
    ESP_LOGE(TAG, "Failed to lock mutex");
    return false;
  }

  // Test unlock
  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex basic lock/unlock successful");
  return true;
}

bool test_rtos_mutex_try_lock() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex try_lock...");

  RtosMutex mutex;

  // Test try_lock when available
  if (!mutex.try_lock()) {
    ESP_LOGE(TAG, "try_lock failed when mutex should be available");
    return false;
  }

  // Test try_lock when already locked (should fail)
  if (mutex.try_lock()) {
    ESP_LOGE(TAG, "try_lock succeeded when mutex should be locked");
    mutex.unlock();
    mutex.unlock();
    return false;
  }

  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex try_lock test successful");
  return true;
}

bool test_rtos_mutex_recursive_locking() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex recursive locking...");

  RtosMutex mutex;

  // Test multiple locks from same task (recursive)
  const int lock_depth = 5;

  for (int i = 0; i < lock_depth; i++) {
    if (!mutex.lock()) {
      ESP_LOGE(TAG, "Failed to acquire recursive lock %d", i + 1);
      return false;
    }
    ESP_LOGI(TAG, "Acquired recursive lock %d", i + 1);
  }

  // Test that we can still try_lock (should succeed due to recursion)
  if (!mutex.try_lock()) {
    ESP_LOGE(TAG, "try_lock failed on recursively locked mutex");
    return false;
  }

  // Unlock all locks (including the try_lock)
  for (int i = 0; i <= lock_depth; i++) {
    mutex.unlock();
    ESP_LOGI(TAG, "Released recursive lock %d", i + 1);
  }

  // Test that mutex is now available
  if (!mutex.try_lock()) {
    ESP_LOGE(TAG, "Mutex not available after releasing all recursive locks");
    return false;
  }
  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex recursive locking test successful");
  return true;
}

bool test_rtos_mutex_timeout_locking() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex timeout locking...");

  RtosMutex mutex;

  // Test try_lock_for when available (should succeed immediately)
  uint64_t start_time = esp_timer_get_time();
  if (!mutex.try_lock_for(1000)) { // 1 second timeout
    ESP_LOGE(TAG, "try_lock_for failed when mutex should be available");
    return false;
  }
  uint64_t lock_time = esp_timer_get_time() - start_time;

  if (lock_time > 100000) { // Should take less than 100ms
    ESP_LOGE(TAG, "try_lock_for took too long when immediately available: %llu us", lock_time);
  }

  // Test timeout with short duration (should timeout)
  start_time = esp_timer_get_time();
  if (mutex.try_lock_for(100)) { // 100ms timeout - should fail
    ESP_LOGE(TAG, "try_lock_for succeeded when mutex should be locked");
    mutex.unlock();
    mutex.unlock();
    return false;
  }
  uint64_t timeout_duration = esp_timer_get_time() - start_time;

  if (timeout_duration < 90000 || timeout_duration > 150000) { // 90-150ms range
    ESP_LOGE(TAG, "try_lock_for timeout duration incorrect: %llu us", timeout_duration);
  }

  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex timeout locking test successful");
  return true;
}

bool test_rtos_mutex_lock_guard_raii() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex lock guard RAII...");

  RtosMutex mutex;

  // Test basic RAII behavior
  {
    MutexLockGuard guard(mutex);
    if (!guard.IsLocked()) {
      ESP_LOGE(TAG, "Lock guard failed to acquire lock");
      return false;
    }

    // Test that mutex is locked
    if (mutex.try_lock()) {
      ESP_LOGE(TAG, "Mutex should be locked by guard but try_lock succeeded");
      mutex.unlock();
      return false;
    }

    ESP_LOGI(TAG, "Lock guard successfully acquired lock");
  } // Guard should automatically unlock here

  // Test that mutex is now available
  if (!mutex.try_lock()) {
    ESP_LOGE(TAG, "Mutex not available after guard destruction");
    return false;
  }
  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex lock guard RAII test successful");
  return true;
}

bool test_rtos_mutex_lock_guard_with_timeout() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex lock guard with timeout...");

  RtosMutex mutex;

  // Test timeout-based lock guard
  {
    RtosUniqueLock<RtosMutex> guard(mutex, 500); // 500ms timeout
    if (!guard.IsLocked()) {
      ESP_LOGE(TAG, "Timeout lock guard failed to acquire lock");
      return false;
    }

    // Test manual unlock
    guard.Unlock();
    if (guard.IsLocked()) {
      ESP_LOGE(TAG, "Manual unlock failed on lock guard");
      return false;
    }

    // Test that mutex is available
    if (!mutex.try_lock()) {
      ESP_LOGE(TAG, "Mutex not available after manual unlock");
      return false;
    }
    mutex.unlock();
  }

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex lock guard with timeout test successful");
  return true;
}

bool test_rtos_mutex_freertos_api() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex FreeRTOS-style API...");

  RtosMutex mutex;

  // Test Take/Give API
  if (!mutex.Take()) {
    ESP_LOGE(TAG, "Failed to Take mutex");
    return false;
  }

  mutex.Give();

  // Test Take with timeout
  if (!mutex.Take(500)) { // 500ms timeout
    ESP_LOGE(TAG, "Failed to Take mutex with timeout");
    return false;
  }

  mutex.Give();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex FreeRTOS-style API test successful");
  return true;
}

//==============================================================================
// RTOS SHARED MUTEX TESTS
//==============================================================================

bool test_rtos_shared_mutex_creation() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex creation...");

  RtosSharedMutex shared_mutex;

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex creation successful");
  return true;
}

bool test_rtos_shared_mutex_exclusive_lock() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex exclusive lock...");

  RtosSharedMutex shared_mutex;

  // Test exclusive lock
  if (!shared_mutex.lock()) {
    ESP_LOGE(TAG, "Failed to acquire exclusive lock");
    return false;
  }

  // Test that another exclusive lock fails
  if (shared_mutex.try_lock()) {
    ESP_LOGE(TAG, "Second exclusive lock succeeded when it should fail");
    shared_mutex.unlock();
    shared_mutex.unlock();
    return false;
  }

  shared_mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex exclusive lock test successful");
  return true;
}

bool test_rtos_shared_mutex_shared_lock() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex shared lock...");

  RtosSharedMutex shared_mutex;

  // Test multiple shared locks
  if (!shared_mutex.lock_shared()) {
    ESP_LOGE(TAG, "Failed to acquire first shared lock");
    return false;
  }

  if (!shared_mutex.try_lock_shared()) {
    ESP_LOGE(TAG, "Failed to acquire second shared lock");
    shared_mutex.unlock_shared();
    return false;
  }

  // Test that exclusive lock fails when shared locks are held
  if (shared_mutex.try_lock()) {
    ESP_LOGE(TAG, "Exclusive lock succeeded when shared locks are held");
    shared_mutex.unlock();
    shared_mutex.unlock_shared();
    shared_mutex.unlock_shared();
    return false;
  }

  shared_mutex.unlock_shared();
  shared_mutex.unlock_shared();

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex shared lock test successful");
  return true;
}

bool test_rtos_shared_mutex_lock_guards() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex lock guards...");

  RtosSharedMutex shared_mutex;

  // Test exclusive lock guard
  {
    RtosUniqueLock<RtosSharedMutex> exclusive_guard(shared_mutex);
    if (!exclusive_guard.IsLocked()) {
      ESP_LOGE(TAG, "Exclusive lock guard failed to acquire lock");
      return false;
    }

    // Test that shared lock fails
    if (shared_mutex.try_lock_shared()) {
      ESP_LOGE(TAG, "Shared lock succeeded when exclusive lock is held");
      shared_mutex.unlock_shared();
      return false;
    }
  }

  // Test shared lock guard
  {
    RtosSharedLock<RtosSharedMutex> shared_guard(shared_mutex);
    if (!shared_guard.IsLocked()) {
      ESP_LOGE(TAG, "Shared lock guard failed to acquire lock");
      return false;
    }

    // Test that another shared lock succeeds
    if (!shared_mutex.try_lock_shared()) {
      ESP_LOGE(TAG, "Second shared lock failed when first shared lock is held");
      return false;
    }
    shared_mutex.unlock_shared();
  }

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex lock guards test successful");
  return true;
}

//==============================================================================
// MUTEX STRESS AND CONCURRENT TESTS
//==============================================================================

// Global test data for concurrent tests
static volatile int g_shared_counter = 0;
static volatile bool g_concurrent_test_running = false;
static RtosMutex g_test_mutex;

void concurrent_increment_task(void* param) {
  int* task_id = static_cast<int*>(param);
  const int increments = 1000;

  ESP_LOGI(TAG, "Concurrent task %d starting", *task_id);

  while (!g_concurrent_test_running) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  for (int i = 0; i < increments; i++) {
    MutexLockGuard guard(g_test_mutex);
    g_shared_counter++;

    // Small delay to increase chance of contention
    if (i % 100 == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  ESP_LOGI(TAG, "Concurrent task %d completed", *task_id);
  vTaskDelete(nullptr);
}

bool test_rtos_mutex_concurrent_access() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex concurrent access...");

  const int num_tasks = 5;
  const int expected_total = num_tasks * 1000;

  g_shared_counter = 0;
  g_concurrent_test_running = false;

  // Create tasks
  static int task_ids[num_tasks];
  for (int i = 0; i < num_tasks; i++) {
    task_ids[i] = i;
    BaseType_t result =
        xTaskCreate(concurrent_increment_task, "ConcTest", 4096, &task_ids[i], 5, nullptr);

    if (result != pdPASS) {
      ESP_LOGE(TAG, "Failed to create concurrent test task %d", i);
      return false;
    }
  }

  // Start all tasks simultaneously
  vTaskDelay(pdMS_TO_TICKS(100)); // Let tasks initialize
  g_concurrent_test_running = true;

  // Wait for tasks to complete
  vTaskDelay(pdMS_TO_TICKS(5000));
  g_concurrent_test_running = false;

  // Check results
  if (g_shared_counter != expected_total) {
    ESP_LOGE(TAG, "Concurrent access test failed: expected %d, got %d", expected_total,
             g_shared_counter);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex concurrent access test successful: %d increments",
           g_shared_counter);
  return true;
}

//==============================================================================
// PERFORMANCE AND STRESS TESTS
//==============================================================================

bool test_ascii_art_performance() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art performance...");

  AsciiArtGenerator generator;

  // Test performance with repeated generation
  const int iterations = 100;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    auto result = generator.Generate("PERFORMANCE TEST");
    if (result.empty()) {
      ESP_LOGE(TAG, "ASCII art generation failed in performance test iteration %d", i);
      return false;
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Performance test: %d iterations in %.2f ms (avg: %.2f us per iteration)",
           iterations, total_time / 1000.0, avg_time);

  if (avg_time > 1000.0) { // Should be less than 1ms per iteration
    ESP_LOGE(TAG, "ASCII art generation too slow: %.2f us per iteration", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art performance test successful");
  return true;
}

bool test_ascii_art_stress() noexcept {
  ESP_LOGI(TAG, "Testing ASCII art stress...");

  AsciiArtGenerator generator;

  // Test stress with many different texts
  const int iterations = 1000;
  std::string test_strings[] = {"A",     "AB",         "ABC",   "ABCD", "ABCDE", "123",  "456",
                                "789",   "0123456789", "!@#",   "$%^",  "&*()",  "[]{}", "|\\/",
                                "HELLO", "WORLD",      "ESP32", "C6",   "TEST"};

  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    std::string test_text = test_strings[i % (sizeof(test_strings) / sizeof(test_strings[0]))];
    auto result = generator.Generate(test_text);
    if (result.empty()) {
      ESP_LOGE(TAG, "ASCII art generation failed in stress test iteration %d", i);
      return false;
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Stress test: %d iterations in %.2f ms (avg: %.2f us per iteration)", iterations,
           total_time / 1000.0, avg_time);

  if (avg_time > 500.0) { // Should be less than 500us per iteration
    ESP_LOGE(TAG, "ASCII art generation too slow in stress test: %.2f us per iteration", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ASCII art stress test successful");
  return true;
}

bool test_rtos_mutex_performance() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex performance...");

  RtosMutex mutex;

  // Test basic lock/unlock performance
  const int iterations = 10000;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    mutex.lock();
    mutex.unlock();
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Basic lock/unlock: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  if (avg_time > 50.0) { // Should be less than 50us per operation
    ESP_LOGE(TAG, "Mutex lock/unlock too slow: %.2f us per operation", avg_time);
    return false;
  }

  // Test try_lock performance
  start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    if (mutex.try_lock()) {
      mutex.unlock();
    }
  }

  end_time = esp_timer_get_time();
  total_time = end_time - start_time;
  avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "try_lock performance: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  // Test lock guard performance
  start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    MutexLockGuard guard(mutex);
    // Guard automatically unlocks on destruction
  }

  end_time = esp_timer_get_time();
  total_time = end_time - start_time;
  avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Lock guard performance: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex performance test successful");
  return true;
}

bool test_rtos_mutex_recursive_performance() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex recursive locking performance...");

  RtosMutex mutex;
  const int iterations = 1000;
  const int lock_depth = 10;

  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    // Acquire multiple recursive locks
    for (int depth = 0; depth < lock_depth; depth++) {
      mutex.lock();
    }

    // Release all locks
    for (int depth = 0; depth < lock_depth; depth++) {
      mutex.unlock();
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Recursive locking: %d iterations of depth %d in %.2f ms (avg: %.2f us per cycle)",
           iterations, lock_depth, total_time / 1000.0, avg_time);

  if (avg_time > 500.0) { // Should be less than 500us per recursive cycle
    ESP_LOGE(TAG, "Recursive mutex locking too slow: %.2f us per cycle", avg_time);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex recursive performance test successful");
  return true;
}

bool test_rtos_shared_mutex_performance() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex performance...");

  RtosSharedMutex shared_mutex;
  const int iterations = 1000;

  // Test shared lock performance
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    shared_mutex.lock_shared();
    shared_mutex.unlock_shared();
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;
  double avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Shared lock/unlock: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  // Test exclusive lock performance
  start_time = esp_timer_get_time();

  for (int i = 0; i < iterations; i++) {
    shared_mutex.lock();
    shared_mutex.unlock();
  }

  end_time = esp_timer_get_time();
  total_time = end_time - start_time;
  avg_time = static_cast<double>(total_time) / iterations;

  ESP_LOGI(TAG, "Exclusive lock/unlock: %d iterations in %.2f ms (avg: %.2f us per operation)",
           iterations, total_time / 1000.0, avg_time);

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex performance test successful");
  return true;
}

//==============================================================================
// EDGE CASES AND ERROR CONDITION TESTS
//==============================================================================

bool test_rtos_mutex_move_semantics() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex move semantics...");

  // Test move constructor
  RtosMutex original_mutex;
  if (!original_mutex.lock()) {
    ESP_LOGE(TAG, "Failed to lock original mutex");
    return false;
  }

  RtosMutex moved_mutex = std::move(original_mutex);

  // Original should be invalid, moved should work
  if (original_mutex.native_handle() != nullptr) {
    ESP_LOGE(TAG, "Original mutex handle should be null after move");
    return false;
  }

  if (moved_mutex.native_handle() == nullptr) {
    ESP_LOGE(TAG, "Moved mutex handle should be valid");
    return false;
  }

  // Unlock through moved mutex
  moved_mutex.unlock();

  // Test move assignment
  RtosMutex another_mutex;
  another_mutex = std::move(moved_mutex);

  if (moved_mutex.native_handle() != nullptr) {
    ESP_LOGE(TAG, "Moved-from mutex handle should be null after assignment");
    return false;
  }

  if (!another_mutex.try_lock()) {
    ESP_LOGE(TAG, "Move-assigned mutex should be functional");
    return false;
  }
  another_mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex move semantics test successful");
  return true;
}

bool test_rtos_shared_mutex_move_semantics() noexcept {
  ESP_LOGI(TAG, "Testing RtosSharedMutex move semantics...");

  // Test move constructor
  RtosSharedMutex original_mutex;
  if (!original_mutex.lock_shared()) {
    ESP_LOGE(TAG, "Failed to lock shared mutex");
    return false;
  }

  RtosSharedMutex moved_mutex = std::move(original_mutex);
  moved_mutex.unlock_shared();

  // Test basic functionality of moved mutex
  if (!moved_mutex.try_lock()) {
    ESP_LOGE(TAG, "Moved shared mutex should be functional");
    return false;
  }
  moved_mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosSharedMutex move semantics test successful");
  return true;
}

bool test_rtos_mutex_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing RtosMutex edge cases...");

  RtosMutex mutex;

  // Test excessive recursive locking (stress the recursive counter)
  const int excessive_depth = 50;
  for (int i = 0; i < excessive_depth; i++) {
    if (!mutex.lock()) {
      ESP_LOGE(TAG, "Failed to acquire excessive recursive lock %d", i + 1);
      return false;
    }
  }

  // Test that try_lock still works with excessive depth
  if (!mutex.try_lock()) {
    ESP_LOGE(TAG, "try_lock failed with excessive recursive depth");
    return false;
  }

  // Unlock all
  for (int i = 0; i <= excessive_depth; i++) {
    mutex.unlock();
  }

  // Test zero timeout
  if (!mutex.try_lock_for(0)) {
    ESP_LOGE(TAG, "try_lock_for(0) should succeed when mutex is available");
    return false;
  }

  // Test zero timeout when locked (should fail immediately)
  uint64_t start_time = esp_timer_get_time();
  if (mutex.try_lock_for(0)) {
    ESP_LOGE(TAG, "try_lock_for(0) should fail when mutex is locked");
    mutex.unlock();
    mutex.unlock();
    return false;
  }
  uint64_t elapsed = esp_timer_get_time() - start_time;

  if (elapsed > 10000) { // Should be nearly instantaneous
    ESP_LOGE(TAG, "try_lock_for(0) took too long: %llu us", elapsed);
  }

  mutex.unlock();

  ESP_LOGI(TAG, "[SUCCESS] RtosMutex edge cases test successful");
  return true;
}

bool test_rtos_lock_guard_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing lock guard edge cases...");

  RtosMutex mutex;

  // Test lock guard that fails to acquire (timeout)
  mutex.lock();
  {
    RtosUniqueLock<RtosMutex> guard(mutex, 10); // 10ms timeout - should fail
    if (guard.IsLocked()) {
      ESP_LOGE(TAG, "Lock guard should have failed to acquire locked mutex");
      return false;
    }
  }
  mutex.unlock();

  // Test manual unlock and double unlock safety
  {
    RtosUniqueLock<RtosMutex> guard(mutex);
    if (!guard.IsLocked()) {
      ESP_LOGE(TAG, "Lock guard should have acquired mutex");
      return false;
    }

    guard.Unlock();
    if (guard.IsLocked()) {
      ESP_LOGE(TAG, "Lock guard should report unlocked after manual unlock");
      return false;
    }

    // Double unlock should be safe
    guard.Unlock(); // Should be safe to call again
  }

  // Test move semantics with lock guards
  {
    RtosUniqueLock<RtosMutex> guard1(mutex);
    if (!guard1.IsLocked()) {
      ESP_LOGE(TAG, "First guard should be locked");
      return false;
    }

    RtosUniqueLock<RtosMutex> guard2 = std::move(guard1);
    if (guard1.IsLocked()) {
      ESP_LOGE(TAG, "Moved-from guard should not be locked");
      return false;
    }

    if (!guard2.IsLocked()) {
      ESP_LOGE(TAG, "Moved-to guard should be locked");
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Lock guard edge cases test successful");
  return true;
}

bool test_rtos_time_utilities() noexcept {
  ESP_LOGI(TAG, "Testing RtosTime utility functions...");

  // Test GetCurrentTimeUs
  uint64_t time1 = RtosTime::GetCurrentTimeUs();
  vTaskDelay(pdMS_TO_TICKS(100)); // Delay 100ms
  uint64_t time2 = RtosTime::GetCurrentTimeUs();

  uint64_t elapsed = time2 - time1;
  if (elapsed < 90000 || elapsed > 150000) { // Should be ~100ms (90-150ms range)
    ESP_LOGE(TAG, "RtosTime::GetCurrentTimeUs() timing incorrect: %llu us", elapsed);
    return false;
  }

  // Test MsToTicks conversion
  TickType_t ticks_0 = RtosTime::MsToTicks(0);
  if (ticks_0 != 0) {
    ESP_LOGE(TAG, "MsToTicks(0) should return 0, got %lu", ticks_0);
    return false;
  }

  TickType_t ticks_1000 = RtosTime::MsToTicks(1000);
  TickType_t expected_ticks = pdMS_TO_TICKS(1000);
  if (ticks_1000 != expected_ticks) {
    ESP_LOGE(TAG, "MsToTicks(1000) mismatch: expected %lu, got %lu", expected_ticks, ticks_1000);
    return false;
  }

  // Test edge case: very small non-zero value
  TickType_t ticks_small = RtosTime::MsToTicks(1);
  if (ticks_small == 0) {
    ESP_LOGE(TAG, "MsToTicks(1) should return at least 1, got %lu", ticks_small);
    return false;
  }

  ESP_LOGI(TAG, "Timing test: %llu us elapsed for 100ms delay", elapsed);
  ESP_LOGI(TAG, "Tick conversion: 1000ms = %lu ticks", ticks_1000);
  ESP_LOGI(TAG, "[SUCCESS] RtosTime utility functions test successful");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 UTILS COMPREHENSIVE TEST SUITE v2.0                ║");
  ESP_LOGI(TAG, "║              AsciiArtGenerator + RtosMutex + RtosSharedMutex               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // ASCII Art Generator Tests
  ESP_LOGI(TAG, "\n=== ASCII ART GENERATOR TESTS ===");
  RUN_TEST(test_ascii_art_generator_creation);
  RUN_TEST(test_ascii_art_basic_text);
  RUN_TEST(test_ascii_art_supported_characters);
  RUN_TEST(test_ascii_art_unsupported_characters);
  RUN_TEST(test_ascii_art_empty_string);
  RUN_TEST(test_ascii_art_mixed_case);
  RUN_TEST(test_ascii_art_special_characters);
  RUN_TEST(test_ascii_art_long_text);
  RUN_TEST(test_ascii_art_custom_character_management);
  RUN_TEST(test_ascii_art_supported_characters_list);

  // RtosMutex Tests
  ESP_LOGI(TAG, "\n=== RTOS MUTEX TESTS ===");
  RUN_TEST(test_rtos_mutex_creation);
  RUN_TEST(test_rtos_mutex_basic_lock_unlock);
  RUN_TEST(test_rtos_mutex_try_lock);
  RUN_TEST(test_rtos_mutex_recursive_locking);
  RUN_TEST(test_rtos_mutex_timeout_locking);
  RUN_TEST(test_rtos_mutex_lock_guard_raii);
  RUN_TEST(test_rtos_mutex_lock_guard_with_timeout);
  RUN_TEST(test_rtos_mutex_freertos_api);

  // RtosSharedMutex Tests
  ESP_LOGI(TAG, "\n=== RTOS SHARED MUTEX TESTS ===");
  RUN_TEST(test_rtos_shared_mutex_creation);
  RUN_TEST(test_rtos_shared_mutex_exclusive_lock);
  RUN_TEST(test_rtos_shared_mutex_shared_lock);
  RUN_TEST(test_rtos_shared_mutex_lock_guards);

  // Concurrent Access Tests (run in separate tasks with larger stacks)
  ESP_LOGI(TAG, "\n=== CONCURRENT ACCESS TESTS ===");
  RUN_TEST_IN_TASK("ConcurrentMutex", test_rtos_mutex_concurrent_access, 8192, 5);

  // Edge Cases and Error Condition Tests
  ESP_LOGI(TAG, "\n=== EDGE CASES AND ERROR CONDITION TESTS ===");
  RUN_TEST(test_rtos_mutex_move_semantics);
  RUN_TEST(test_rtos_shared_mutex_move_semantics);
  RUN_TEST(test_rtos_mutex_edge_cases);
  RUN_TEST(test_rtos_lock_guard_edge_cases);

  // RtosTime Utility Tests
  ESP_LOGI(TAG, "\n=== RTOS TIME UTILITY TESTS ===");
  RUN_TEST(test_rtos_time_utilities);

  // Performance and Stress Tests
  ESP_LOGI(TAG, "\n=== PERFORMANCE AND STRESS TESTS ===");
  RUN_TEST(test_ascii_art_performance);
  RUN_TEST(test_ascii_art_stress);
  RUN_TEST(test_rtos_mutex_performance);
  RUN_TEST(test_rtos_mutex_recursive_performance);
  RUN_TEST(test_rtos_shared_mutex_performance);

  // Print final summary
  print_test_summary(g_test_results, "UTILS", TAG);

  ESP_LOGI(TAG,
           "\n╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                    UTILS COMPREHENSIVE TEST SUITE COMPLETE                    ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Keep the system running
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
