/**
 * @file EspLoggerComprehensiveTest.cpp
 * @brief Comprehensive Logger testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspLogger class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * logging functionalities including basic operations, level management, statistics,
 * diagnostics, buffer logging, and ESP-IDF Log V2 features.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BaseLogger.h"
#include "mcu/esp32/EspLogger.h"
#include "utils/memory_utils.h"

#include <string>

static const char* TAG = "EspLOGGER_Test";

static TestResults g_test_results;

// Test configuration constants
static constexpr hf_u32_t TEST_MAX_MESSAGE_LENGTH = 512;
static constexpr hf_u32_t TEST_BUFFER_SIZE = 1024;
static const char* TEST_TAG = "TEST_TAG";

// Global test instance
static std::unique_ptr<EspLogger> g_logger_instance = nullptr;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core logger functionality tests
static constexpr bool ENABLE_CORE_TESTS = true;           // Construction, initialization, basic logging
static constexpr bool ENABLE_LEVEL_TESTS = true;          // Level management, formatted logging
static constexpr bool ENABLE_FEATURE_TESTS = true;        // Log V2 features, buffer logging, location logging
static constexpr bool ENABLE_DIAGNOSTIC_TESTS = true;     // Statistics, diagnostics, health monitoring
static constexpr bool ENABLE_STRESS_TESTS = true;         // Error handling, performance testing, utility functions

// Forward declarations
bool test_logger_construction() noexcept;
bool test_logger_initialization() noexcept;
bool test_logger_basic_logging() noexcept;
bool test_logger_level_management() noexcept;
bool test_logger_formatted_logging() noexcept;
bool test_logger_log_v2_features() noexcept;
bool test_logger_buffer_logging() noexcept;
bool test_logger_location_logging() noexcept;
bool test_logger_statistics_diagnostics() noexcept;
bool test_logger_health_monitoring() noexcept;
bool test_logger_error_handling() noexcept;
bool test_logger_performance_testing() noexcept;
bool test_logger_utility_functions() noexcept;
bool test_logger_cleanup() noexcept;

// Helper functions
hf_logger_config_t create_test_config() noexcept;
bool verify_logger_state(EspLogger& logger, bool should_be_initialized) noexcept;
void log_performance_metrics(const char* test_name, hf_u64_t start_time,
                             hf_u32_t operations) noexcept;

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

hf_logger_config_t create_test_config() noexcept {
  hf_logger_config_t config = {};
  config.default_level = hf_log_level_t::LOG_LEVEL_INFO;
  config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
  config.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
  config.max_message_length = TEST_MAX_MESSAGE_LENGTH;
  config.buffer_size = TEST_BUFFER_SIZE;
  config.flush_interval_ms = 100; // Set flush interval
  config.enable_thread_safety = true;
  config.enable_performance_monitoring = true;
  // Note: enable_health_monitoring doesn't exist in config structure
  return config;
}

bool verify_logger_state(EspLogger& logger, bool should_be_initialized) noexcept {
  if (logger.IsInitialized() != should_be_initialized) {
    ESP_LOGE(TAG, "Logger initialization state mismatch. Expected: %s, Actual: %s",
             should_be_initialized ? "initialized" : "not initialized",
             logger.IsInitialized() ? "initialized" : "not initialized");
    return false;
  }
  return true;
}

void log_performance_metrics(const char* test_name, hf_u64_t start_time,
                             hf_u32_t operations) noexcept {
  hf_u64_t end_time = esp_timer_get_time();
  hf_u64_t duration_us = end_time - start_time;
  double duration_ms = duration_us / 1000.0;
  double ops_per_sec = (operations * 1000000.0) / duration_us;

  ESP_LOGI(TAG, "%s Performance: %lu ops in %.2f ms (%.2f ops/sec)", test_name, operations,
           duration_ms, ops_per_sec);
}

//==============================================================================
// TEST FUNCTIONS
//==============================================================================

bool test_logger_construction() noexcept {
  ESP_LOGI(TAG, "Testing Logger construction and destruction...");

  // Test construction with default parameters using nothrow allocation
  auto logger = hf::utils::make_unique_nothrow<EspLogger>();

  if (!logger) {
    ESP_LOGE(TAG, "Failed to construct EspLogger instance - out of memory");
    return false;
  }

  // Verify initial state
  if (!verify_logger_state(*logger, false)) {
    ESP_LOGE(TAG, "Initial state verification failed");
    return false;
  }

  // Store for other tests
  g_logger_instance = std::move(logger);

  ESP_LOGI(TAG, "[SUCCESS] Logger construction completed");
  return true;
}

bool test_logger_initialization() noexcept {
  ESP_LOGI(TAG, "Testing Logger initialization...");

  if (!g_logger_instance) {
    ESP_LOGE(TAG, "No Logger instance available");
    return false;
  }

  // Test initialization with custom configuration
  hf_logger_config_t config = create_test_config();
  ESP_LOGI(TAG, "Initializing with config: max_msg_len=%lu, buffer_size=%lu, flush_interval=%lu ms",
           config.max_message_length, config.buffer_size, config.flush_interval_ms);
  hf_logger_err_t result = g_logger_instance->Initialize(config);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize logger: %d", static_cast<int>(result));
    return false;
  }

  // Verify initialization state
  if (!verify_logger_state(*g_logger_instance, true)) {
    ESP_LOGE(TAG, "Post-initialization state verification failed");
    return false;
  }

  // Test EnsureInitialized (should succeed since already initialized)
  if (!g_logger_instance->EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized failed on already initialized logger");
    return false;
  }

  // Test double initialization (should return already initialized error)
  result = g_logger_instance->Initialize(config);
  if (result != hf_logger_err_t::LOGGER_ERR_ALREADY_INITIALIZED) {
    ESP_LOGW(TAG, "Double initialization should return ALREADY_INITIALIZED error");
  }

  // Demonstrate the PrintStatus method
  ESP_LOGI(TAG, "Demonstrating PrintStatus method:");
  result = g_logger_instance->PrintStatus("INIT_STATUS", false); // Brief status after init
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGW(TAG, "PrintStatus failed: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] Logger initialization successful");
  return true;
}

bool test_logger_basic_logging() noexcept {
  ESP_LOGI(TAG, "Testing basic Logger logging operations...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test all basic logging levels
  hf_logger_err_t result;

  // Test Error logging
  result = g_logger_instance->Error(TEST_TAG, "Test error message: %d", 1);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Error logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test Warn logging
  result = g_logger_instance->Warn(TEST_TAG, "Test warning message: %d", 2);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Warn logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test Info logging
  result = g_logger_instance->Info(TEST_TAG, "Test info message: %d", 3);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Info logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test Debug logging
  result = g_logger_instance->Debug(TEST_TAG, "Test debug message: %d", 4);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Debug logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test Verbose logging
  result = g_logger_instance->Verbose(TEST_TAG, "Test verbose message: %d", 5);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Verbose logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test generic Log method
  result = g_logger_instance->Log(hf_log_level_t::LOG_LEVEL_INFO, TEST_TAG,
                                  "Generic log message: %s", "test");
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Generic Log method failed: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Basic logging operations completed");
  return true;
}

bool test_logger_level_management() noexcept {
  ESP_LOGI(TAG, "Testing Logger level management...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test setting and getting log levels
  hf_log_level_t test_levels[] = {hf_log_level_t::LOG_LEVEL_ERROR, hf_log_level_t::LOG_LEVEL_WARN,
                                  hf_log_level_t::LOG_LEVEL_INFO, hf_log_level_t::LOG_LEVEL_DEBUG,
                                  hf_log_level_t::LOG_LEVEL_VERBOSE};

  hf_u8_t num_levels = sizeof(test_levels) / sizeof(test_levels[0]);

  for (hf_u8_t i = 0; i < num_levels; i++) {
    // Set log level for specific tag
    hf_logger_err_t result = g_logger_instance->SetLogLevel(TEST_TAG, test_levels[i]);
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "Failed to set log level %d: %d", static_cast<int>(test_levels[i]),
               static_cast<int>(result));
      return false;
    }

    // Get and verify log level
    hf_log_level_t retrieved_level;
    result = g_logger_instance->GetLogLevel(TEST_TAG, retrieved_level);
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "Failed to get log level: %d", static_cast<int>(result));
      return false;
    }

    if (retrieved_level != test_levels[i]) {
      ESP_LOGE(TAG, "Log level mismatch. Expected: %d, Got: %d", static_cast<int>(test_levels[i]),
               static_cast<int>(retrieved_level));
      return false;
    }

    // Test if level is enabled
    bool level_enabled = g_logger_instance->IsLevelEnabled(test_levels[i], TEST_TAG);
    ESP_LOGI(TAG, "Level %d enabled for tag '%s': %s", static_cast<int>(test_levels[i]), TEST_TAG,
             level_enabled ? "true" : "false");
  }

  // Test default tag level management
  hf_logger_err_t result = g_logger_instance->SetLogLevel(nullptr, hf_log_level_t::LOG_LEVEL_INFO);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set default log level: %d", static_cast<int>(result));
    return false;
  }

  hf_log_level_t default_level;
  result = g_logger_instance->GetLogLevel(nullptr, default_level);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get default log level: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Default log level: %d", static_cast<int>(default_level));

  ESP_LOGI(TAG, "[SUCCESS] Level management test completed");
  return true;
}

bool test_logger_formatted_logging() noexcept {
  ESP_LOGI(TAG, "Testing Logger formatted logging...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test various format specifiers
  hf_logger_err_t result;

  // Integer formatting
  result = g_logger_instance->Info(TEST_TAG, "Integer: %d, Hex: 0x%x, Octal: %o", 42, 255, 64);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Integer formatting failed: %d", static_cast<int>(result));
    return false;
  }

  // String formatting
  result = g_logger_instance->Info(TEST_TAG, "String: '%s', Character: '%c'", "Hello World", 'A');
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "String formatting failed: %d", static_cast<int>(result));
    return false;
  }

  // Float formatting
  result = g_logger_instance->Info(TEST_TAG, "Float: %.2f, Scientific: %.2e", 3.14159, 1234.5678);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Float formatting failed: %d", static_cast<int>(result));
    return false;
  }

  // Pointer formatting
  void* test_ptr = &result;
  result = g_logger_instance->Info(TEST_TAG, "Pointer: %p, Size: %zu", test_ptr, sizeof(result));
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Pointer formatting failed: %d", static_cast<int>(result));
    return false;
  }

  // Long format string test
  const char* long_format = "Long message with many parameters: %d %s %f %c %x %o %p %zu %ld %u";
  result = g_logger_instance->Info(TEST_TAG, long_format, 42, "test", 3.14, 'X', 0xFF, 077,
                                   test_ptr, sizeof(int), 1000L, 500U);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Long format string failed: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Formatted logging test completed");
  return true;
}

bool test_logger_log_v2_features() noexcept {
  ESP_LOGI(TAG, "Testing Logger Log V2 features...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Check Log V2 availability
  bool log_v2_available = g_logger_instance->IsLogV2Available();
  hf_u8_t log_version = g_logger_instance->GetLogVersion();

  ESP_LOGI(TAG, "Log V2 available: %s, Log version: %d", log_v2_available ? "true" : "false",
           log_version);

  if (log_v2_available) {
    ESP_LOGI(TAG, "Testing Log V2 specific features...");

    // Test buffer logging features
    uint8_t test_buffer[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

    // Test LogBuffer
    hf_logger_err_t result =
        g_logger_instance->LogBuffer(TEST_TAG, test_buffer, sizeof(test_buffer));
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "LogBuffer failed: %d", static_cast<int>(result));
      return false;
    }

    // Test LogBufferHex
    result = g_logger_instance->LogBufferHex(TEST_TAG, test_buffer, sizeof(test_buffer));
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "LogBufferHex failed: %d", static_cast<int>(result));
      return false;
    }

    // Test LogBufferChar
    const char* text_buffer = "Hello, Log V2!";
    result = g_logger_instance->LogBufferChar(TEST_TAG, text_buffer, strlen(text_buffer));
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "LogBufferChar failed: %d", static_cast<int>(result));
      return false;
    }

    // Test LogBufferHexDump
    result = g_logger_instance->LogBufferHexDump(TEST_TAG, test_buffer, sizeof(test_buffer));
    if (result != hf_logger_err_t::LOGGER_SUCCESS) {
      ESP_LOGE(TAG, "LogBufferHexDump failed: %d", static_cast<int>(result));
      return false;
    }

    ESP_LOGI(TAG, "Log V2 features tested successfully");
  } else {
    ESP_LOGI(TAG, "Log V2 not available, skipping V2-specific tests");
  }

  ESP_LOGI(TAG, "[SUCCESS] Log V2 features test completed");
  return true;
}

bool test_logger_buffer_logging() noexcept {
  ESP_LOGI(TAG, "Testing Logger buffer logging...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test different buffer sizes and types
  uint8_t small_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  uint8_t medium_buffer[32];
  uint8_t large_buffer[256];

  // Fill medium buffer with pattern
  for (hf_u8_t i = 0; i < sizeof(medium_buffer); i++) {
    medium_buffer[i] = i;
  }

  // Fill large buffer with pattern
  for (hf_u16_t i = 0; i < sizeof(large_buffer); i++) {
    large_buffer[i] = static_cast<uint8_t>(i % 256);
  }

  // Test small buffer
  hf_logger_err_t result =
      g_logger_instance->LogBuffer(TEST_TAG, small_buffer, sizeof(small_buffer));
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Small buffer logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test medium buffer
  result = g_logger_instance->LogBuffer(TEST_TAG, medium_buffer, sizeof(medium_buffer));
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Medium buffer logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test large buffer (might be truncated)
  result = g_logger_instance->LogBuffer(TEST_TAG, large_buffer, sizeof(large_buffer));
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Large buffer logging failed: %d", static_cast<int>(result));
    return false;
  }

  // Test null buffer (should handle gracefully)
  result = g_logger_instance->LogBuffer(TEST_TAG, nullptr, 10);
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGW(TAG, "Null buffer logging unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Null buffer logging correctly failed");
  }

  // Test zero length buffer
  result = g_logger_instance->LogBuffer(TEST_TAG, small_buffer, 0);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGW(TAG, "Zero length buffer logging failed (might be expected)");
  }

  ESP_LOGI(TAG, "[SUCCESS] Buffer logging test completed");
  return true;
}

bool test_logger_location_logging() noexcept {
  ESP_LOGI(TAG, "Testing Logger location logging...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test LogWithLocation
  const char* test_file = __FILE__;
  hf_u32_t test_line = __LINE__ + 2;
  const char* test_function = __FUNCTION__;
  hf_logger_err_t result = g_logger_instance->LogWithLocation(
      hf_log_level_t::LOG_LEVEL_INFO, TEST_TAG, test_file, test_line, test_function,
      "Location test message with parameters: %d, %s", 42, "test");

  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "LogWithLocation failed: %d", static_cast<int>(result));
    return false;
  }

  // Test with different log levels
  result = g_logger_instance->LogWithLocation(hf_log_level_t::LOG_LEVEL_ERROR, TEST_TAG, test_file,
                                              __LINE__, test_function, "Error location test: %s",
                                              "critical error");

  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Error level LogWithLocation failed: %d", static_cast<int>(result));
    return false;
  }

  // Test with null parameters (should handle gracefully)
  result = g_logger_instance->LogWithLocation(hf_log_level_t::LOG_LEVEL_WARN, TEST_TAG, nullptr, 0,
                                              nullptr, "Null parameters test");

  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGI(TAG, "Null parameters location logging succeeded");
  } else {
    ESP_LOGI(TAG, "Null parameters location logging failed (might be expected)");
  }

  ESP_LOGI(TAG, "[SUCCESS] Location logging test completed");
  return true;
}

bool test_logger_statistics_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing Logger statistics and diagnostics...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test and print statistics using the logger's built-in method
  hf_logger_err_t result = g_logger_instance->PrintStatistics(TAG, true);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to print statistics: %d", static_cast<int>(result));
    return false;
  }

  // Log some messages to change statistics
  g_logger_instance->Info(TEST_TAG, "Statistics test message 1");
  g_logger_instance->Error(TEST_TAG, "Statistics test error");
  g_logger_instance->Warn(TEST_TAG, "Statistics test warning");

  // Print updated statistics using the logger's built-in method
  ESP_LOGI(TAG, "=== Updated Statistics After Logging ===");
  result = g_logger_instance->PrintStatistics(TAG, true);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to print updated statistics: %d", static_cast<int>(result));
    return false;
  }

  // Test and print diagnostics using the logger's built-in method
  result = g_logger_instance->PrintDiagnostics(TAG, true);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to print diagnostics: %d", static_cast<int>(result));
    return false;
  }

  // Test statistics reset
  result = g_logger_instance->ResetStatistics();
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset statistics: %d", static_cast<int>(result));
    return false;
  }

  // Verify and print reset statistics using the logger's built-in method
  ESP_LOGI(TAG, "=== Statistics After Reset ===");
  result = g_logger_instance->PrintStatistics(TAG, false); // Brief output after reset
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to print reset statistics: %d", static_cast<int>(result));
    return false;
  }

  // Test diagnostics reset
  result = g_logger_instance->ResetDiagnostics();
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset diagnostics: %d", static_cast<int>(result));
    return false;
  }

  // Print reset diagnostics using the logger's built-in method
  ESP_LOGI(TAG, "=== Diagnostics After Reset ===");
  result = g_logger_instance->PrintDiagnostics(TAG, false); // Brief output after reset
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Failed to print reset diagnostics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics test completed");
  return true;
}

bool test_logger_health_monitoring() noexcept {
  ESP_LOGI(TAG, "Testing Logger health monitoring...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test health status
  bool is_healthy = g_logger_instance->IsHealthy();
  ESP_LOGI(TAG, "Logger health status: %s", is_healthy ? "healthy" : "unhealthy");

  if (!is_healthy) {
    ESP_LOGW(TAG, "Logger reported as unhealthy");
  }

  // Test error retrieval
  hf_logger_err_t last_error = g_logger_instance->GetLastError();
  ESP_LOGI(TAG, "Last error: %d", static_cast<int>(last_error));

  // Test error message retrieval
  char error_message[256];
  hf_logger_err_t result =
      g_logger_instance->GetLastErrorMessage(error_message, sizeof(error_message));
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGI(TAG, "Last error message: %s", error_message);
  } else {
    ESP_LOGI(TAG, "No error message available or retrieval failed");
  }

  // Test flush operation
  result = g_logger_instance->Flush();
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Flush operation failed: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Health monitoring test completed");
  return true;
}

bool test_logger_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing Logger error handling...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test logging with null tag
  hf_logger_err_t result = g_logger_instance->Info(nullptr, "Null tag test");
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGI(TAG, "Null tag logging succeeded");
  } else {
    ESP_LOGI(TAG, "Null tag logging failed (might be expected)");
  }

  // Test logging with null message
  result = g_logger_instance->Info(TEST_TAG, nullptr);
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGW(TAG, "Null message logging unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Null message logging correctly failed");
  }

  // Test extremely long message
  char long_message[2048];
  for (int i = 0; i < sizeof(long_message) - 1; i++) {
    long_message[i] = 'A' + (i % 26);
  }
  long_message[sizeof(long_message) - 1] = '\0';

  result = g_logger_instance->Info(TEST_TAG, "Long message: %s", long_message);
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGI(TAG, "Long message logging succeeded");
  } else {
    ESP_LOGI(TAG, "Long message logging failed (might be truncated)");
  }

  // Test invalid log level
  result = g_logger_instance->Log(static_cast<hf_log_level_t>(999), TEST_TAG, "Invalid level test");
  if (result == hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGW(TAG, "Invalid log level unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Invalid log level correctly failed");
  }

  ESP_LOGI(TAG, "[SUCCESS] Error handling test completed");
  return true;
}

bool test_logger_performance_testing() noexcept {
  ESP_LOGI(TAG, "Testing Logger performance...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  const hf_u32_t num_operations = 1000;
  hf_u64_t start_time;

  // Test basic logging performance
  start_time = esp_timer_get_time();
  for (hf_u32_t i = 0; i < num_operations; i++) {
    g_logger_instance->Info(TEST_TAG, "Performance test message %lu", i);
  }
  log_performance_metrics("Basic Logging", start_time, num_operations);

  // Test formatted logging performance
  start_time = esp_timer_get_time();
  for (hf_u32_t i = 0; i < num_operations; i++) {
    g_logger_instance->Info(TEST_TAG, "Format test: %d, %s, %.2f", static_cast<int>(i), "test",
                            static_cast<double>(i) * 0.1);
  }
  log_performance_metrics("Formatted Logging", start_time, num_operations);

  // Test different log levels performance
  start_time = esp_timer_get_time();
  for (hf_u32_t i = 0; i < num_operations / 4; i++) {
    g_logger_instance->Error(TEST_TAG, "Error %lu", i);
    g_logger_instance->Warn(TEST_TAG, "Warning %lu", i);
    g_logger_instance->Info(TEST_TAG, "Info %lu", i);
    g_logger_instance->Debug(TEST_TAG, "Debug %lu", i);
  }
  log_performance_metrics("Multi-level Logging", start_time, num_operations);

  // Test buffer logging performance (if available)
  if (g_logger_instance->IsLogV2Available()) {
    uint8_t test_buffer[32];
    for (hf_u8_t i = 0; i < sizeof(test_buffer); i++) {
      test_buffer[i] = i;
    }

    start_time = esp_timer_get_time();
    for (hf_u32_t i = 0; i < num_operations / 10; i++) { // Fewer operations for buffer logging
      g_logger_instance->LogBuffer(TEST_TAG, test_buffer, sizeof(test_buffer));
    }
    log_performance_metrics("Buffer Logging", start_time, num_operations / 10);
  }

  ESP_LOGI(TAG, "[SUCCESS] Performance testing completed");
  return true;
}

bool test_logger_utility_functions() noexcept {
  ESP_LOGI(TAG, "Testing Logger utility functions...");

  if (!g_logger_instance || !g_logger_instance->IsInitialized()) {
    ESP_LOGE(TAG, "Logger not initialized");
    return false;
  }

  // Test level checking for various tags and levels
  hf_log_level_t test_levels[] = {hf_log_level_t::LOG_LEVEL_ERROR, hf_log_level_t::LOG_LEVEL_WARN,
                                  hf_log_level_t::LOG_LEVEL_INFO, hf_log_level_t::LOG_LEVEL_DEBUG,
                                  hf_log_level_t::LOG_LEVEL_VERBOSE};

  for (hf_u8_t i = 0; i < sizeof(test_levels) / sizeof(test_levels[0]); i++) {
    bool enabled = g_logger_instance->IsLevelEnabled(test_levels[i], TEST_TAG);
    ESP_LOGI(TAG, "Level %d enabled for %s: %s", static_cast<int>(test_levels[i]), TEST_TAG,
             enabled ? "true" : "false");

    enabled = g_logger_instance->IsLevelEnabled(test_levels[i], nullptr);
    ESP_LOGI(TAG, "Level %d enabled for default: %s", static_cast<int>(test_levels[i]),
             enabled ? "true" : "false");
  }

  // Test version information
  hf_u8_t log_version = g_logger_instance->GetLogVersion();
  bool log_v2_available = g_logger_instance->IsLogV2Available();

  ESP_LOGI(TAG, "Logger version information:");
  ESP_LOGI(TAG, "  Log version: %d", log_version);
  ESP_LOGI(TAG, "  Log V2 available: %s", log_v2_available ? "true" : "false");

  // Test custom output callback functionality
  ESP_LOGI(TAG, "Testing custom output callback functionality...");

  // Create a test logger instance with custom callback
  static std::string captured_output;
  auto custom_callback = [](const char* message, hf_u32_t length) {
    captured_output = std::string(message, length);
    printf("[CUSTOM] %s\n", captured_output.c_str());
  };

  hf_logger_config_t custom_config = create_test_config();
  custom_config.custom_output_callback = custom_callback;

  auto custom_logger = hf::utils::make_unique_nothrow<EspLogger>();
  if (custom_logger) {
    hf_logger_err_t result = custom_logger->Initialize(custom_config);
    if (result == hf_logger_err_t::LOGGER_SUCCESS) {
      custom_logger->Info("CUSTOM_TEST", "This message should go to custom callback");
      custom_logger->Deinitialize();
      ESP_LOGI(TAG, "Custom callback test completed successfully");
    } else {
      ESP_LOGW(TAG, "Custom callback test skipped due to initialization failure");
    }
  } else {
    ESP_LOGW(TAG, "Custom callback test skipped due to memory allocation failure");
  }

  ESP_LOGI(TAG, "[SUCCESS] Utility functions test completed");
  return true;
}

bool test_logger_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing Logger cleanup...");

  if (!g_logger_instance) {
    ESP_LOGE(TAG, "No Logger instance to clean up");
    return false;
  }

  // Test explicit deinitialization
  hf_logger_err_t result = g_logger_instance->Deinitialize();
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(TAG, "Deinitialization failed: %d", static_cast<int>(result));
    return false;
  }

  // Verify deinitialized state
  if (!verify_logger_state(*g_logger_instance, false)) {
    ESP_LOGE(TAG, "Post-deinitialization state verification failed");
    return false;
  }

  // The destructor will be called when the unique_ptr is reset
  g_logger_instance.reset();

  ESP_LOGI(TAG, "[SUCCESS] Logger cleanup completed");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                 ESP32-C6 ESPLOGGER COMPREHENSIVE TEST SUITE                  ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║ Target: ESP32-C6 DevKit-M-1                                                  ║");
  ESP_LOGI(TAG, "║ ESP-IDF: v5.5+                                                               ║");
  ESP_LOGI(TAG, "║ Features: Logger, Logging, Logging Levels, Logging Formats, Logging Buffers, ║");
  ESP_LOGI(TAG, "║ Logging Locations, Logging Statistics, Logging Diagnostics, Logging Health   ║");
  ESP_LOGI(TAG, "║ Monitoring, Logging Error Handling, Logging Performance, Logging Utility     ║");
  ESP_LOGI(TAG, "║ Functions, Logging Cleanup, Logging Edge Cases, Logging Stress Tests         ║");
  ESP_LOGI(TAG, "║ Architecture: noexcept (no exception handling)                               ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "ESPLOGGER");

  // Run all logger tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CORE_TESTS, "LOGGER CORE TESTS",
      // Core functionality tests
      ESP_LOGI(TAG, "Running core logger functionality tests...");
      RUN_TEST_IN_TASK("construction", test_logger_construction, 8192, 1);
      RUN_TEST_IN_TASK("initialization", test_logger_initialization, 8192, 1);
      RUN_TEST_IN_TASK("basic_logging", test_logger_basic_logging, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_LEVEL_TESTS, "LOGGER LEVEL TESTS",
      // Level management tests
      ESP_LOGI(TAG, "Running logger level management tests...");
      RUN_TEST_IN_TASK("level_management", test_logger_level_management, 8192, 1);
      RUN_TEST_IN_TASK("formatted_logging", test_logger_formatted_logging, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_FEATURE_TESTS, "LOGGER FEATURE TESTS",
      // Feature tests
      ESP_LOGI(TAG, "Running logger feature tests...");
      RUN_TEST_IN_TASK("log_v2_features", test_logger_log_v2_features, 8192, 1);
      RUN_TEST_IN_TASK("buffer_logging", test_logger_buffer_logging, 8192, 1);
      RUN_TEST_IN_TASK("location_logging", test_logger_location_logging, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_DIAGNOSTIC_TESTS, "LOGGER DIAGNOSTIC TESTS",
      // Diagnostic tests
      ESP_LOGI(TAG, "Running logger diagnostic tests...");
      RUN_TEST_IN_TASK("statistics_diagnostics", test_logger_statistics_diagnostics, 8192, 1);
      RUN_TEST_IN_TASK("health_monitoring", test_logger_health_monitoring, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_STRESS_TESTS, "LOGGER STRESS TESTS",
      // Stress and utility tests
      ESP_LOGI(TAG, "Running logger stress tests...");
      RUN_TEST_IN_TASK("error_handling", test_logger_error_handling, 8192, 1);
      RUN_TEST_IN_TASK("performance_testing", test_logger_performance_testing, 8192, 1);
      RUN_TEST_IN_TASK("utility_functions", test_logger_utility_functions, 8192, 1);
      RUN_TEST_IN_TASK("cleanup", test_logger_cleanup, 8192, 1););

  print_test_summary(g_test_results, "ESPLOGGER", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG, "[SUCCESS] ALL ESPLOGGER TESTS PASSED!");
  } else {
    ESP_LOGE(TAG, "[FAILED] Some tests failed.");
  }

  ESP_LOGI(TAG, "Logger comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Post-test banner
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                 ESP32-C6 ESPLOGGER COMPREHENSIVE TEST SUITE                  ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "\n");

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}