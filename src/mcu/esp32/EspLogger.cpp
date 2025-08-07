/**
 * @file EspLogger.cpp
 * @brief Implementation of ESP32 logger for the HardFOC system.
 *
 * This file provides the implementation for ESP32 logging using the ESP-IDF
 * esp_log system (both Log V1 and Log V2). All platform-specific types and
 * implementations are isolated through the BaseLogger interface. The implementation
 * supports multiple log levels, tag-based filtering, performance monitoring,
 * thread-safe operations, comprehensive error handling, and enhanced Log V2 features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "EspLogger.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP-IDF Log V2 support
#ifdef CONFIG_LOG_VERSION_2
#include "esp_log_v2.h"
#endif

#ifdef __cplusplus
}
#endif

static const char* TAG = "EspLogger";

//==============================================================================
// CONSTANTS AND CONSTEXPR
//==============================================================================

static constexpr hf_u32_t DEFAULT_MAX_MESSAGE_LENGTH = 512;
static constexpr hf_u32_t DEFAULT_BUFFER_SIZE = 1024;
static constexpr hf_u32_t DEFAULT_FLUSH_INTERVAL_MS = 100;
static constexpr hf_u32_t MAX_TAG_LENGTH = 32;
static constexpr hf_u32_t MAX_ERROR_MESSAGE_LENGTH = 256;
static constexpr hf_u64_t HEALTH_CHECK_INTERVAL_US = 30000000; // 30 seconds
static constexpr hf_u32_t MAX_BUFFER_LOG_SIZE = 4096;          // Maximum buffer size for logging

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspLogger::EspLogger() noexcept
    : mutex_(), initialized_(false), healthy_(true), config_(), statistics_(), diagnostics_(),
      tag_levels_(), message_buffer_(), last_error_(hf_logger_err_t::LOGGER_SUCCESS),
      initialization_time_(0), last_health_check_(0), log_v2_available_(false), log_version_(1) {
  // Initialize statistics and diagnostics
  std::memset(&statistics_, 0, sizeof(statistics_));
  std::memset(&diagnostics_, 0, sizeof(diagnostics_));
  std::memset(last_error_message_, 0, sizeof(last_error_message_));

  // Set default configuration
  config_.default_level = hf_log_level_t::LOG_LEVEL_INFO;
  config_.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
  config_.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
  config_.max_message_length = DEFAULT_MAX_MESSAGE_LENGTH;
  config_.buffer_size = DEFAULT_BUFFER_SIZE;
  config_.flush_interval_ms = DEFAULT_FLUSH_INTERVAL_MS;
  config_.enable_thread_safety = true;
  config_.enable_performance_monitoring = true;

  // Detect Log V2 availability
  log_v2_available_ = CheckLogV2Availability();
  log_version_ = log_v2_available_ ? 2 : 1;

  ESP_LOGD(TAG, "EspLogger constructor completed (Log V%d)", log_version_);
}

EspLogger::~EspLogger() noexcept {
  ESP_LOGD(TAG, "EspLogger destructor called");
  Deinitialize();
  ESP_LOGD(TAG, "EspLogger destructor completed");
}

//==============================================================================
// BASELOGGER INTERFACE IMPLEMENTATION
//==============================================================================

hf_logger_err_t EspLogger::Initialize(const hf_logger_config_t& config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_.load()) {
    ESP_LOGW(TAG, "Logger already initialized");
    return hf_logger_err_t::LOGGER_ERR_ALREADY_INITIALIZED;
  }

  ESP_LOGI(TAG, "Initializing ESP32 logger (Log V%d)", log_version_);

  // Validate configuration
  hf_logger_err_t validation_result = ValidateConfiguration(config);
  if (validation_result != hf_logger_err_t::LOGGER_SUCCESS) {
    UpdateDiagnostics(validation_result);
    ESP_LOGE(TAG, "Configuration validation failed: %s", ConvertErrorToString(validation_result));
    return validation_result;
  }

  // Store configuration
  config_ = config;

  // Initialize message buffer
  if (!EnsureMessageBuffer(config_.max_message_length)) {
    UpdateDiagnostics(hf_logger_err_t::LOGGER_ERR_OUT_OF_MEMORY);
    ESP_LOGE(TAG, "Failed to allocate message buffer");
    return hf_logger_err_t::LOGGER_ERR_OUT_OF_MEMORY;
  }

  // Initialize Log V2 if available
  if (log_v2_available_) {
    if (!InitializeLogV2()) {
      ESP_LOGW(TAG, "Log V2 initialization failed, falling back to Log V1");
      log_v2_available_ = false;
      log_version_ = 1;
    }
  }

  // Set default log level for ESP-IDF
  esp_log_level_set("*", ConvertLogLevel(config_.default_level));

  // Initialize statistics and diagnostics
  std::memset(&statistics_, 0, sizeof(statistics_));
  std::memset(&diagnostics_, 0, sizeof(diagnostics_));

  diagnostics_.is_initialized = true;
  diagnostics_.is_healthy = true;
  diagnostics_.last_error = hf_logger_err_t::LOGGER_SUCCESS;
  diagnostics_.last_error_timestamp = GetCurrentTimestamp();
  diagnostics_.consecutive_errors = 0;
  diagnostics_.error_recovery_count = 0;
  diagnostics_.uptime_seconds = 0;
  diagnostics_.last_health_check = GetCurrentTimestamp();

  initialization_time_ = GetCurrentTimestamp();
  last_health_check_ = GetCurrentTimestamp();

  initialized_.store(true);
  healthy_.store(true);
  last_error_ = hf_logger_err_t::LOGGER_SUCCESS;

  ESP_LOGI(TAG, "ESP32 logger initialized successfully (Log V%d)", log_version_);
  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_SUCCESS;
  }

  ESP_LOGI(TAG, "Deinitializing ESP32 logger");

  // Flush any pending output
  Flush();

  // Clear tag levels
  tag_levels_.clear();

  // Clear message buffer
  message_buffer_.clear();
  message_buffer_.shrink_to_fit();

  // Update diagnostics
  diagnostics_.is_initialized = false;
  diagnostics_.is_healthy = false;
  diagnostics_.uptime_seconds = (GetCurrentTimestamp() - initialization_time_) / 1000000;

  initialized_.store(false);
  healthy_.store(false);

  ESP_LOGI(TAG, "ESP32 logger deinitialized successfully");
  return hf_logger_err_t::LOGGER_SUCCESS;
}

bool EspLogger::IsInitialized() const noexcept {
  return initialized_.load();
}

bool EspLogger::EnsureInitialized() noexcept {
  if (IsInitialized()) {
    return true;
  }

  // Use default configuration for lazy initialization
  hf_logger_config_t default_config = {};
  default_config.default_level = hf_log_level_t::LOG_LEVEL_INFO;
  default_config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
  default_config.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
  default_config.max_message_length = DEFAULT_MAX_MESSAGE_LENGTH;
  default_config.buffer_size = DEFAULT_BUFFER_SIZE;
  default_config.flush_interval_ms = DEFAULT_FLUSH_INTERVAL_MS;
  default_config.enable_thread_safety = true;
  default_config.enable_performance_monitoring = true;

  return Initialize(default_config) == hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::SetLogLevel(const char* tag, hf_log_level_t level) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag) {
    // Set default level
    config_.default_level = level;
    esp_log_level_set("*", ConvertLogLevel(level));
    ESP_LOGI(TAG, "Default log level set to %s", HfLogLevelToString(level));
  } else {
    // Set tag-specific level
    tag_levels_[std::string(tag)] = level;
    esp_log_level_set(tag, ConvertLogLevel(level));
    ESP_LOGI(TAG, "Log level for tag '%s' set to %s", tag, HfLogLevelToString(level));
  }

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::GetLogLevel(const char* tag, hf_log_level_t& level) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag) {
    level = config_.default_level;
  } else {
    auto it = tag_levels_.find(std::string(tag));
    if (it != tag_levels_.end()) {
      level = it->second;
    } else {
      level = config_.default_level;
    }
  }

  return hf_logger_err_t::LOGGER_SUCCESS;
}

//==============================================================================
// LOGGING METHODS
//==============================================================================

hf_logger_err_t EspLogger::Error(const char* tag, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(hf_log_level_t::LOG_LEVEL_ERROR, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::Warn(const char* tag, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(hf_log_level_t::LOG_LEVEL_WARN, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::Info(const char* tag, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(hf_log_level_t::LOG_LEVEL_INFO, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::Debug(const char* tag, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(hf_log_level_t::LOG_LEVEL_DEBUG, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::Verbose(const char* tag, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(hf_log_level_t::LOG_LEVEL_VERBOSE, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::Log(hf_log_level_t level, const char* tag, const char* format,
                               ...) noexcept {
  va_list args;
  va_start(args, format);
  hf_logger_err_t result = LogV(level, tag, format, args);
  va_end(args);
  return result;
}

hf_logger_err_t EspLogger::LogV(hf_log_level_t level, const char* tag, const char* format,
                                va_list args) noexcept {
  if (!EnsureInitialized()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag || !format) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  // Check if level is enabled for this tag
  if (!IsLevelEnabled(level, tag)) {
    return hf_logger_err_t::LOGGER_SUCCESS; // Not an error, just filtered out
  }

  // Use appropriate ESP-IDF logging method
  hf_logger_err_t result = WriteMessageV(level, tag, format, args);

  // Update statistics
  hf_u32_t message_length = std::strlen(format); // Approximate length
  UpdateStatistics(level, message_length, result == hf_logger_err_t::LOGGER_SUCCESS);

  return result;
}

hf_logger_err_t EspLogger::LogWithLocation(hf_log_level_t level, const char* tag, const char* file,
                                           hf_u32_t line, const char* function, const char* format,
                                           ...) noexcept {
  if (!EnsureInitialized()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag || !format) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  // Check if level is enabled for this tag
  if (!IsLevelEnabled(level, tag)) {
    return hf_logger_err_t::LOGGER_SUCCESS; // Not an error, just filtered out
  }

  // Format message with location information if enabled
  if (static_cast<bool>(config_.format_options & hf_log_format_t::LOG_FORMAT_FILE_LINE)) {
    va_list args;
    va_start(args, format);

    // Create enhanced format string with location
    char enhanced_format[512];
    snprintf(enhanced_format, sizeof(enhanced_format), "[%s:%lu] %s", file ? file : "unknown",
             static_cast<unsigned long>(line), format);

    hf_logger_err_t result = WriteMessageV(level, tag, enhanced_format, args);
    va_end(args);

    // Update statistics
    hf_u32_t message_length = std::strlen(format); // Approximate length
    UpdateStatistics(level, message_length, result == hf_logger_err_t::LOGGER_SUCCESS);

    return result;
  } else {
    // Use standard logging
    va_list args;
    va_start(args, format);
    hf_logger_err_t result = WriteMessageV(level, tag, format, args);
    va_end(args);

    // Update statistics
    hf_u32_t message_length = std::strlen(format); // Approximate length
    UpdateStatistics(level, message_length, result == hf_logger_err_t::LOGGER_SUCCESS);

    return result;
  }
}

//==============================================================================
// ESP-IDF LOG V2 ENHANCED METHODS
//==============================================================================

hf_logger_err_t EspLogger::LogBufferHex(const char* tag, const void* buffer, hf_u32_t length,
                                        hf_log_level_t level) noexcept {
  if (!EnsureInitialized()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag || !buffer || length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  if (length > MAX_BUFFER_LOG_SIZE) {
    return hf_logger_err_t::LOGGER_ERR_BUFFER_OVERFLOW;
  }

  // Check if level is enabled for this tag
  if (!IsLevelEnabled(level, tag)) {
    return hf_logger_err_t::LOGGER_SUCCESS; // Not an error, just filtered out
  }

  esp_log_level_t esp_level = ConvertLogLevel(level);

  // Use ESP-IDF buffer logging
  ESP_LOG_BUFFER_HEX_LEVEL(tag, buffer, length, esp_level);

  // Update statistics
  UpdateStatistics(level, length, true);

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::LogBufferChar(const char* tag, const void* buffer, hf_u32_t length,
                                         hf_log_level_t level) noexcept {
  if (!EnsureInitialized()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag || !buffer || length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  if (length > MAX_BUFFER_LOG_SIZE) {
    return hf_logger_err_t::LOGGER_ERR_BUFFER_OVERFLOW;
  }

  // Check if level is enabled for this tag
  if (!IsLevelEnabled(level, tag)) {
    return hf_logger_err_t::LOGGER_SUCCESS; // Not an error, just filtered out
  }

  esp_log_level_t esp_level = ConvertLogLevel(level);

  // Use ESP-IDF buffer logging
  ESP_LOG_BUFFER_CHAR_LEVEL(tag, buffer, length, esp_level);

  // Update statistics
  UpdateStatistics(level, length, true);

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::LogBufferHexDump(const char* tag, const void* buffer, hf_u32_t length,
                                            hf_log_level_t level) noexcept {
  if (!EnsureInitialized()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  if (!tag || !buffer || length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  if (length > MAX_BUFFER_LOG_SIZE) {
    return hf_logger_err_t::LOGGER_ERR_BUFFER_OVERFLOW;
  }

  // Check if level is enabled for this tag
  if (!IsLevelEnabled(level, tag)) {
    return hf_logger_err_t::LOGGER_SUCCESS; // Not an error, just filtered out
  }

  esp_log_level_t esp_level = ConvertLogLevel(level);

  // Use ESP-IDF buffer logging
  ESP_LOG_BUFFER_HEXDUMP(tag, buffer, length, esp_level);

  // Update statistics
  UpdateStatistics(level, length, true);

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::LogBuffer(const char* tag, const void* buffer, hf_u32_t length,
                                     hf_log_level_t level) noexcept {
  // Default to hex dump for generic buffer logging
  return LogBufferHexDump(tag, buffer, length, level);
}

//==============================================================================
// UTILITY METHODS
//==============================================================================

hf_logger_err_t EspLogger::Flush() noexcept {
  // ESP-IDF logging is synchronous, so no flush needed
  return hf_logger_err_t::LOGGER_SUCCESS;
}

bool EspLogger::IsLevelEnabled(hf_log_level_t level, const char* tag) const noexcept {
  if (!initialized_.load()) {
    return false;
  }

  hf_log_level_t current_level;
  if (GetLogLevel(tag, current_level) != hf_logger_err_t::LOGGER_SUCCESS) {
    return false;
  }

  return static_cast<hf_u8_t>(level) <= static_cast<hf_u8_t>(current_level);
}

hf_logger_err_t EspLogger::GetStatistics(hf_logger_statistics_t& statistics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  statistics = statistics_;
  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::GetDiagnostics(hf_logger_diagnostics_t& diagnostics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  // Create a copy of diagnostics and update uptime
  hf_logger_diagnostics_t temp_diagnostics = diagnostics_;
  temp_diagnostics.uptime_seconds = (GetCurrentTimestamp() - initialization_time_) / 1000000;

  diagnostics = temp_diagnostics;
  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::ResetStatistics() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  std::memset(&statistics_, 0, sizeof(statistics_));
  ESP_LOGI(TAG, "Statistics reset");

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::ResetDiagnostics() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  // Reset diagnostics to default values while preserving initialization state
  hf_logger_diagnostics_t reset_diagnostics = {};
  reset_diagnostics.is_initialized = true;  // Keep initialized state
  reset_diagnostics.is_healthy = true;      // Reset to healthy
  reset_diagnostics.last_error = hf_logger_err_t::LOGGER_SUCCESS;
  reset_diagnostics.last_error_timestamp = GetCurrentTimestamp();
  reset_diagnostics.consecutive_errors = 0;
  reset_diagnostics.error_recovery_count = 0;
  reset_diagnostics.uptime_seconds = 0;    // Will be recalculated in GetDiagnostics
  reset_diagnostics.last_health_check = GetCurrentTimestamp();
  std::memset(reset_diagnostics.last_error_message, 0, sizeof(reset_diagnostics.last_error_message));

  diagnostics_ = reset_diagnostics;
  ESP_LOGI(TAG, "Diagnostics reset");

  return hf_logger_err_t::LOGGER_SUCCESS;
}

bool EspLogger::IsHealthy() const noexcept {
  return healthy_.load();
}

hf_logger_err_t EspLogger::GetLastError() const noexcept {
  return last_error_;
}

hf_logger_err_t EspLogger::GetLastErrorMessage(char* message, hf_u32_t max_length) const noexcept {
  if (!message || max_length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_u32_t copy_length =
      std::min(max_length - 1, static_cast<hf_u32_t>(strlen(last_error_message_)));
  std::memcpy(message, last_error_message_, copy_length);
  message[copy_length] = '\0';

  return hf_logger_err_t::LOGGER_SUCCESS;
}

bool EspLogger::IsLogV2Available() const noexcept {
  return log_v2_available_;
}

hf_u8_t EspLogger::GetLogVersion() const noexcept {
  return log_version_;
}

//==============================================================================
// DIAGNOSTIC PRINTING METHODS
//==============================================================================

hf_logger_err_t EspLogger::PrintStatistics(const char* tag, bool detailed) const noexcept {
  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  const char* print_tag = tag ? tag : "LOGGER_STATS";

  // Get current statistics
  hf_logger_statistics_t stats = {};
  hf_logger_err_t result = GetStatistics(stats);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(print_tag, "Failed to get statistics: %s", HfLoggerErrToString(result));
    return result;
  }

  // Print basic statistics
  ESP_LOGI(print_tag, "=== Logger Statistics ===");
  ESP_LOGI(print_tag, "  Total messages: %llu", stats.total_messages);
  ESP_LOGI(print_tag, "  Total bytes written: %llu", stats.total_bytes_written);
  ESP_LOGI(print_tag, "  Write errors: %llu", stats.write_errors);
  ESP_LOGI(print_tag, "  Format errors: %llu", stats.format_errors);

  if (detailed) {
    ESP_LOGI(print_tag, "  Buffer overflows: %llu", stats.buffer_overflows);
    ESP_LOGI(print_tag, "  Performance monitor calls: %llu", stats.performance_monitor_calls);
    ESP_LOGI(print_tag, "  Last message timestamp: %llu µs", stats.last_message_timestamp);
    ESP_LOGI(print_tag, "  Average message length: %llu bytes", stats.average_message_length);
    ESP_LOGI(print_tag, "  Max message length seen: %llu bytes", stats.max_message_length_seen);

    ESP_LOGI(print_tag, "  Messages by level:");
    ESP_LOGI(print_tag, "    NONE: %llu", stats.messages_by_level[0]);
    ESP_LOGI(print_tag, "    ERROR: %llu", stats.messages_by_level[1]);
    ESP_LOGI(print_tag, "    WARN: %llu", stats.messages_by_level[2]);
    ESP_LOGI(print_tag, "    INFO: %llu", stats.messages_by_level[3]);
    ESP_LOGI(print_tag, "    DEBUG: %llu", stats.messages_by_level[4]);
    ESP_LOGI(print_tag, "    VERBOSE: %llu", stats.messages_by_level[5]);
  }

  ESP_LOGI(print_tag, "========================");
  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::PrintDiagnostics(const char* tag, bool detailed) const noexcept {
  if (!initialized_.load()) {
    return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
  }

  const char* print_tag = tag ? tag : "LOGGER_DIAG";

  // Get current diagnostics
  hf_logger_diagnostics_t diag = {};
  hf_logger_err_t result = GetDiagnostics(diag);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    ESP_LOGE(print_tag, "Failed to get diagnostics: %s", HfLoggerErrToString(result));
    return result;
  }

  // Print basic diagnostics
  ESP_LOGI(print_tag, "=== Logger Diagnostics ===");
  ESP_LOGI(print_tag, "  Initialized: %s", diag.is_initialized ? "YES" : "NO");
  ESP_LOGI(print_tag, "  Health status: %s", diag.is_healthy ? "HEALTHY" : "UNHEALTHY");
  ESP_LOGI(print_tag, "  Last error: %s", HfLoggerErrToString(diag.last_error));
  ESP_LOGI(print_tag, "  Uptime: %llu seconds", diag.uptime_seconds);

  if (detailed) {
    ESP_LOGI(print_tag, "  Last error timestamp: %llu µs", diag.last_error_timestamp);
    ESP_LOGI(print_tag, "  Consecutive errors: %u", diag.consecutive_errors);
    ESP_LOGI(print_tag, "  Error recovery count: %u", diag.error_recovery_count);
    ESP_LOGI(print_tag, "  Last health check: %llu µs", diag.last_health_check);
    
    if (strlen(diag.last_error_message) > 0) {
      ESP_LOGI(print_tag, "  Last error message: %s", diag.last_error_message);
    } else {
      ESP_LOGI(print_tag, "  Last error message: <none>");
    }
  }

  ESP_LOGI(print_tag, "==========================");
  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::PrintStatus(const char* tag, bool detailed) const noexcept {
  const char* print_tag = tag ? tag : "LOGGER_STATUS";

  ESP_LOGI(print_tag, "=== Logger Complete Status ===");
  ESP_LOGI(print_tag, "Logger Version: %d (%s)", GetLogVersion(), 
           IsLogV2Available() ? "Log V2 Available" : "Log V1 Only");

  hf_logger_err_t result = PrintStatistics(print_tag, detailed);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    return result;
  }

  result = PrintDiagnostics(print_tag, detailed);
  if (result != hf_logger_err_t::LOGGER_SUCCESS) {
    return result;
  }

  ESP_LOGI(print_tag, "==============================");
  return hf_logger_err_t::LOGGER_SUCCESS;
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

esp_log_level_t EspLogger::ConvertLogLevel(hf_log_level_t level) const noexcept {
  switch (level) {
    case hf_log_level_t::LOG_LEVEL_NONE:
      return ESP_LOG_NONE;
    case hf_log_level_t::LOG_LEVEL_ERROR:
      return ESP_LOG_ERROR;
    case hf_log_level_t::LOG_LEVEL_WARN:
      return ESP_LOG_WARN;
    case hf_log_level_t::LOG_LEVEL_INFO:
      return ESP_LOG_INFO;
    case hf_log_level_t::LOG_LEVEL_DEBUG:
      return ESP_LOG_DEBUG;
    case hf_log_level_t::LOG_LEVEL_VERBOSE:
      return ESP_LOG_VERBOSE;
    default:
      return ESP_LOG_INFO;
  }
}

hf_log_level_t EspLogger::ConvertLogLevel(esp_log_level_t level) const noexcept {
  switch (level) {
    case ESP_LOG_NONE:
      return hf_log_level_t::LOG_LEVEL_NONE;
    case ESP_LOG_ERROR:
      return hf_log_level_t::LOG_LEVEL_ERROR;
    case ESP_LOG_WARN:
      return hf_log_level_t::LOG_LEVEL_WARN;
    case ESP_LOG_INFO:
      return hf_log_level_t::LOG_LEVEL_INFO;
    case ESP_LOG_DEBUG:
      return hf_log_level_t::LOG_LEVEL_DEBUG;
    case ESP_LOG_VERBOSE:
      return hf_log_level_t::LOG_LEVEL_VERBOSE;
    default:
      return hf_log_level_t::LOG_LEVEL_INFO;
  }
}

hf_logger_err_t EspLogger::FormatMessage(hf_log_level_t level, const char* tag, const char* file,
                                         hf_u32_t line, const char* function, const char* format,
                                         va_list args, char* formatted_message,
                                         hf_u32_t max_length) noexcept {
  if (!formatted_message || max_length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  // Format the message with location information if enabled
  if (static_cast<bool>(config_.format_options & hf_log_format_t::LOG_FORMAT_FILE_LINE)) {
    int written = snprintf(formatted_message, max_length, "[%s:%lu] ", file ? file : "unknown",
                           static_cast<unsigned long>(line));
    if (written < 0 || static_cast<hf_u32_t>(written) >= max_length) {
      return hf_logger_err_t::LOGGER_ERR_FORMAT_ERROR;
    }

    written += vsnprintf(formatted_message + written, max_length - written, format, args);
    if (written < 0 || static_cast<hf_u32_t>(written) >= max_length) {
      return hf_logger_err_t::LOGGER_ERR_FORMAT_ERROR;
    }
  } else {
    int written = vsnprintf(formatted_message, max_length, format, args);
    if (written < 0 || static_cast<hf_u32_t>(written) >= max_length) {
      return hf_logger_err_t::LOGGER_ERR_FORMAT_ERROR;
    }
  }

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::WriteMessage(hf_log_level_t level, const char* tag, const char* message,
                                        hf_u32_t length) noexcept {
  if (!message || length == 0) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  // Use custom output callback if configured
  if (config_.custom_output_callback) {
    config_.custom_output_callback(message, std::strlen(message));
  } else {
    // Use ESP-IDF logging
    esp_log_level_t esp_level = ConvertLogLevel(level);
    esp_log_write(esp_level, tag, "%s", message);
  }

  return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t EspLogger::WriteMessageV(hf_log_level_t level, const char* tag, const char* format,
                                         va_list args) noexcept {
  if (!tag || !format) {
    return hf_logger_err_t::LOGGER_ERR_NULL_POINTER;
  }

  // Use custom output callback if configured
  if (config_.custom_output_callback) {
    // Format message for custom callback
    char formatted_message[config_.max_message_length];
    int len = vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    if (len > 0) {
      config_.custom_output_callback(formatted_message, static_cast<hf_u32_t>(len));
    }
    return hf_logger_err_t::LOGGER_SUCCESS;
  }

  esp_log_level_t esp_level = ConvertLogLevel(level);

  // Use appropriate ESP-IDF logging method based on version
  if (log_v2_available_) {
#ifdef CONFIG_LOG_VERSION_2
    // Log V2: Use esp_log() for better performance and flexibility
    esp_log(esp_level, tag, format, args);
    return hf_logger_err_t::LOGGER_SUCCESS;
#else
    // Fallback to Log V1
    esp_log_writev(esp_level, tag, format, args);
    return hf_logger_err_t::LOGGER_SUCCESS;
#endif
  } else {
    // Log V1: Use esp_log_writev
    esp_log_writev(esp_level, tag, format, args);
    return hf_logger_err_t::LOGGER_SUCCESS;
  }
}

void EspLogger::UpdateStatistics(hf_log_level_t level, hf_u32_t message_length,
                                 bool success) noexcept {
  statistics_.total_messages++;
  
  // Update per-level statistics with bounds checking
  hf_u8_t level_index = static_cast<hf_u8_t>(level);
  if (level_index < 6) {  // Array size is 6 (indices 0-5)
    statistics_.messages_by_level[level_index]++;
  }
  
  statistics_.total_bytes_written += message_length;
  statistics_.last_message_timestamp = GetCurrentTimestamp();

  if (message_length > statistics_.max_message_length_seen) {
    statistics_.max_message_length_seen = message_length;
  }

  // Update average message length
  if (statistics_.total_messages > 0) {
    statistics_.average_message_length =
        statistics_.total_bytes_written / statistics_.total_messages;
  }

  if (!success) {
    statistics_.write_errors++;
  }
}

void EspLogger::UpdateDiagnostics(hf_logger_err_t error) noexcept {
  hf_u64_t current_time = GetCurrentTimestamp();

  if (error != hf_logger_err_t::LOGGER_SUCCESS) {
    last_error_ = error;
    diagnostics_.last_error = error;
    diagnostics_.last_error_timestamp = current_time;
    diagnostics_.consecutive_errors++;
    diagnostics_.is_healthy = false;

    // Store error message
    snprintf(last_error_message_, sizeof(last_error_message_), "Error: %s",
             ConvertErrorToString(error));
  } else {
    if (diagnostics_.consecutive_errors > 0) {
      diagnostics_.error_recovery_count++;
      diagnostics_.consecutive_errors = 0;
    }
    diagnostics_.is_healthy = true;
  }

  diagnostics_.last_health_check = current_time;
}

bool EspLogger::PerformHealthCheck() noexcept {
  hf_u64_t current_time = GetCurrentTimestamp();

  // Check if health check is needed
  if (current_time - last_health_check_ < HEALTH_CHECK_INTERVAL_US) {
    return healthy_.load();
  }

  last_health_check_ = current_time;

  // Perform basic health checks
  bool is_healthy = true;

  // Check if ESP-IDF logging is working
  if (!initialized_.load()) {
    is_healthy = false;
  }

  // Check for excessive errors
  if (diagnostics_.consecutive_errors > 10) {
    is_healthy = false;
  }

  // Check memory usage
  if (message_buffer_.size() > config_.buffer_size * 2) {
    is_healthy = false;
  }

  healthy_.store(is_healthy);
  diagnostics_.is_healthy = is_healthy;

  return is_healthy;
}

hf_logger_err_t EspLogger::ValidateConfiguration(const hf_logger_config_t& config) const noexcept {
  if (config.max_message_length == 0 || config.max_message_length > 4096) {
    return hf_logger_err_t::LOGGER_ERR_INVALID_CONFIGURATION;
  }

  if (config.buffer_size == 0 || config.buffer_size > 16384) {
    return hf_logger_err_t::LOGGER_ERR_INVALID_CONFIGURATION;
  }

  if (config.flush_interval_ms > 60000) {
    return hf_logger_err_t::LOGGER_ERR_INVALID_CONFIGURATION;
  }

  return hf_logger_err_t::LOGGER_SUCCESS;
}

const char* EspLogger::ConvertErrorToString(hf_logger_err_t error) const noexcept {
  return HfLoggerErrToString(error);
}

hf_u64_t EspLogger::GetCurrentTimestamp() const noexcept {
  return esp_timer_get_time();
}

hf_u32_t EspLogger::GetCurrentThreadId() const noexcept {
  return reinterpret_cast<hf_u32_t>(xTaskGetCurrentTaskHandle());
}

bool EspLogger::EnsureMessageBuffer(hf_u32_t required_length) noexcept {
  if (message_buffer_.size() < required_length) {
    // Check if allocation would be reasonable (prevent excessive memory usage)
    if (required_length > DEFAULT_MAX_MESSAGE_LENGTH * 2) {
      return false;
    }

    // Use reserve() and manual size check for no-exceptions approach
    size_t old_capacity = message_buffer_.capacity();
    if (old_capacity < required_length) {
      message_buffer_.reserve(required_length);
      // Check if reserve succeeded by checking capacity
      if (message_buffer_.capacity() < required_length) {
        return false;
      }
    }
    message_buffer_.resize(required_length);
  }
  return true;
}

bool EspLogger::InitializeLogV2() noexcept {
#ifdef CONFIG_LOG_VERSION_2
  // Log V2 is automatically initialized by ESP-IDF
  // We just need to verify it's working
  ESP_LOGI(TAG, "Log V2 initialized successfully");
  return true;
#else
  return false;
#endif
}

bool EspLogger::CheckLogV2Availability() const noexcept {
#ifdef CONFIG_LOG_VERSION_2
  // Check if Log V2 is configured
  return true;
#else
  // Log V2 not configured
  return false;
#endif
}

#endif // HF_MCU_FAMILY_ESP32