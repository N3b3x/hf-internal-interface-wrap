/**
 * @file BaseLogger.h
 * @ingroup logger
 * @brief Unified logging base class for all logging implementations.
 *
 * This file contains the declaration of the BaseLogger abstract class, which provides
 * a comprehensive logging abstraction that serves as the base for all logging
 * implementations in the HardFOC system. It supports multiple log levels,
 * configurable output destinations, thread-safe operations, and works across
 * different hardware platforms including ESP32, STM32, and other MCUs.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This class is thread-safe and designed for concurrent access.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdarg>
#include <cstdint>
#include <functional>
#include <string>

/**
 * @defgroup logger Logger Module
 * @brief All logging-related types, enums, and functions for system logging.
 * @details This module provides comprehensive logging functionality including:
 *          - Multiple log levels (ERROR, WARN, INFO, DEBUG, VERBOSE)
 *          - Thread-safe logging operations
 *          - Configurable output destinations
 *          - Performance monitoring and statistics
 *          - Error handling and diagnostics
 */

/**
 * @ingroup logger
 * @brief HardFOC Logger error codes macro list
 * @details X-macro pattern for comprehensive error enumeration. Each entry contains:
 *          X(NAME, VALUE, DESCRIPTION)
 */
#define HF_LOGGER_ERR_LIST(X)                                     \
  /* Success codes */                                             \
  X(LOGGER_SUCCESS, 0, "Success")                                 \
                                                                  \
  /* General errors */                                            \
  X(LOGGER_ERR_FAILURE, 1, "General failure")                     \
  X(LOGGER_ERR_NOT_INITIALIZED, 2, "Not initialized")             \
  X(LOGGER_ERR_ALREADY_INITIALIZED, 3, "Already initialized")     \
  X(LOGGER_ERR_INVALID_PARAMETER, 4, "Invalid parameter")         \
  X(LOGGER_ERR_NULL_POINTER, 5, "Null pointer")                   \
  X(LOGGER_ERR_OUT_OF_MEMORY, 6, "Out of memory")                 \
                                                                  \
  /* Configuration errors */                                      \
  X(LOGGER_ERR_INVALID_CONFIGURATION, 7, "Invalid configuration") \
  X(LOGGER_ERR_UNSUPPORTED_OPERATION, 8, "Unsupported operation") \
  X(LOGGER_ERR_RESOURCE_BUSY, 9, "Resource busy")                 \
  X(LOGGER_ERR_RESOURCE_UNAVAILABLE, 10, "Resource unavailable")  \
                                                                  \
  /* Output errors */                                             \
  X(LOGGER_ERR_WRITE_FAILURE, 11, "Write failure")                \
  X(LOGGER_ERR_OUTPUT_BUFFER_FULL, 12, "Output buffer full")      \
  X(LOGGER_ERR_FORMAT_ERROR, 13, "Format error")                  \
  X(LOGGER_ERR_ENCODING_ERROR, 14, "Encoding error")              \
                                                                  \
  /* System errors */                                             \
  X(LOGGER_ERR_SYSTEM_ERROR, 15, "System error")                  \
  X(LOGGER_ERR_PERMISSION_DENIED, 16, "Permission denied")        \
  X(LOGGER_ERR_OPERATION_ABORTED, 17, "Operation aborted")        \
                                                                  \
  /* Extended errors */                                           \
  X(LOGGER_ERR_NOT_SUPPORTED, 18, "Operation not supported")      \
  X(LOGGER_ERR_DRIVER_ERROR, 19, "Driver error")                  \
  X(LOGGER_ERR_INVALID_STATE, 20, "Invalid state")                \
  X(LOGGER_ERR_INVALID_ARG, 21, "Invalid argument")               \
  X(LOGGER_ERR_TIMEOUT, 22, "Timeout")                            \
  X(LOGGER_ERR_BUFFER_OVERFLOW, 23, "Buffer overflow")

/**
 * @ingroup logger
 * @brief Generate logger error enum from macro list
 */
#define HF_LOGGER_ERR_ENUM(name, value, description) name = value,
enum class hf_logger_err_t : hf_u32_t { HF_LOGGER_ERR_LIST(HF_LOGGER_ERR_ENUM) };
#undef HF_LOGGER_ERR_ENUM

/**
 * @ingroup logger
 * @brief Log levels enumeration
 */
enum class hf_log_level_t : hf_u8_t {
  LOG_LEVEL_NONE = 0,   ///< No logging
  LOG_LEVEL_ERROR = 1,  ///< Error messages only
  LOG_LEVEL_WARN = 2,   ///< Warning and error messages
  LOG_LEVEL_INFO = 3,   ///< Info, warning, and error messages
  LOG_LEVEL_DEBUG = 4,  ///< Debug, info, warning, and error messages
  LOG_LEVEL_VERBOSE = 5 ///< All messages including verbose
};

/**
 * @ingroup logger
 * @brief Log output destination enumeration
 */
enum class hf_log_output_t : hf_u8_t {
  LOG_OUTPUT_NONE = 0,    ///< No output
  LOG_OUTPUT_UART = 1,    ///< UART serial output
  LOG_OUTPUT_USB = 2,     ///< USB CDC output
  LOG_OUTPUT_FILE = 3,    ///< File system output
  LOG_OUTPUT_NETWORK = 4, ///< Network output
  LOG_OUTPUT_CUSTOM = 5   ///< Custom output callback
};

/**
 * @ingroup logger
 * @brief Log format options
 */
enum class hf_log_format_t : hf_u32_t {
  LOG_FORMAT_NONE = 0,              ///< No formatting
  LOG_FORMAT_TIMESTAMP = (1U << 0), ///< Include timestamp
  LOG_FORMAT_LEVEL = (1U << 1),     ///< Include log level
  LOG_FORMAT_TAG = (1U << 2),       ///< Include tag
  LOG_FORMAT_FILE_LINE = (1U << 3), ///< Include file and line
  LOG_FORMAT_FUNCTION = (1U << 4),  ///< Include function name
  LOG_FORMAT_THREAD_ID = (1U << 5), ///< Include thread ID
  LOG_FORMAT_COLORS = (1U << 6),    ///< Include ANSI colors
  LOG_FORMAT_DEFAULT = LOG_FORMAT_TIMESTAMP | LOG_FORMAT_LEVEL | LOG_FORMAT_TAG
};

// Bitwise operators for hf_log_format_t enum class
inline hf_log_format_t operator|(hf_log_format_t a, hf_log_format_t b) {
  return static_cast<hf_log_format_t>(static_cast<hf_u32_t>(a) | static_cast<hf_u32_t>(b));
}

inline hf_log_format_t operator&(hf_log_format_t a, hf_log_format_t b) {
  return static_cast<hf_log_format_t>(static_cast<hf_u32_t>(a) & static_cast<hf_u32_t>(b));
}

inline hf_log_format_t operator^(hf_log_format_t a, hf_log_format_t b) {
  return static_cast<hf_log_format_t>(static_cast<hf_u32_t>(a) ^ static_cast<hf_u32_t>(b));
}

inline hf_log_format_t operator~(hf_log_format_t a) {
  return static_cast<hf_log_format_t>(~static_cast<hf_u32_t>(a));
}

/**
 * @ingroup logger
 * @brief Logger configuration structure
 */
struct hf_logger_config_t {
  hf_log_level_t default_level;       ///< Default log level
  hf_log_output_t output_destination; ///< Output destination
  hf_log_format_t format_options;     ///< Format options
  hf_u32_t max_message_length;        ///< Maximum message length
  hf_u32_t buffer_size;               ///< Internal buffer size
  hf_u32_t flush_interval_ms;         ///< Flush interval in milliseconds
  bool enable_thread_safety;          ///< Enable thread safety
  bool enable_performance_monitoring; ///< Enable performance monitoring
  std::function<void(const char*, hf_u32_t)> custom_output_callback; ///< Custom output callback
};

/**
 * @ingroup logger
 * @brief Logger statistics structure
 */
struct hf_logger_statistics_t {
  hf_u64_t total_messages;            ///< Total messages logged
  hf_u64_t messages_by_level[6];      ///< Messages by level (indexed by hf_log_level_t)
  hf_u64_t total_bytes_written;       ///< Total bytes written
  hf_u64_t write_errors;              ///< Number of write errors
  hf_u64_t format_errors;             ///< Number of format errors
  hf_u64_t buffer_overflows;          ///< Number of buffer overflows
  hf_u64_t performance_monitor_calls; ///< Number of performance monitor calls
  hf_u64_t last_message_timestamp;    ///< Timestamp of last message
  hf_u64_t average_message_length;    ///< Average message length
  hf_u64_t max_message_length_seen;   ///< Maximum message length seen
};

/**
 * @ingroup logger
 * @brief Logger diagnostics structure
 */
struct hf_logger_diagnostics_t {
  bool is_initialized;           ///< Initialization status
  bool is_healthy;               ///< Health status
  hf_logger_err_t last_error;    ///< Last error code
  hf_u64_t last_error_timestamp; ///< Last error timestamp
  hf_u32_t consecutive_errors;   ///< Consecutive error count
  hf_u32_t error_recovery_count; ///< Error recovery count
  hf_u64_t uptime_seconds;       ///< Uptime in seconds
  hf_u64_t last_health_check;    ///< Last health check timestamp
  char last_error_message[256];  ///< Last error message
};

/**
 * @ingroup logger
 * @brief Log message structure
 */
struct hf_log_message_t {
  hf_log_level_t level;    ///< Log level
  const char* tag;         ///< Message tag
  const char* message;     ///< Message content
  const char* file;        ///< Source file
  hf_u32_t line;           ///< Source line
  const char* function;    ///< Function name
  hf_u64_t timestamp;      ///< Timestamp
  hf_u32_t thread_id;      ///< Thread ID
  hf_u32_t message_length; ///< Message length
};

/**
 * @ingroup logger
 * @brief Base logger abstract class
 *
 * This class provides a comprehensive logging abstraction that supports:
 * - Multiple log levels (ERROR, WARN, INFO, DEBUG, VERBOSE)
 * - Thread-safe operations
 * - Configurable output destinations
 * - Performance monitoring and statistics
 * - Error handling and diagnostics
 * - Custom output callbacks
 * - Message formatting and buffering
 */
class BaseLogger {
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~BaseLogger() = default;

  //==============================================================================
  // INITIALIZATION AND CONFIGURATION
  //==============================================================================

  /**
   * @brief Initialize the logger
   * @param config Logger configuration
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Initialize(const hf_logger_config_t& config) noexcept = 0;

  /**
   * @brief Deinitialize the logger
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Deinitialize() noexcept = 0;

  /**
   * @brief Check if logger is initialized
   * @return true if initialized, false otherwise
   */
  virtual bool IsInitialized() const noexcept = 0;

  /**
   * @brief Ensure logger is initialized (lazy initialization)
   * @return true if initialization successful, false otherwise
   */
  virtual bool EnsureInitialized() noexcept = 0;

  /**
   * @brief Set log level for a specific tag
   * @param tag Log tag (nullptr for default)
   * @param level Log level
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t SetLogLevel(const char* tag, hf_log_level_t level) noexcept = 0;

  /**
   * @brief Get log level for a specific tag
   * @param tag Log tag (nullptr for default)
   * @param level Output log level
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t GetLogLevel(const char* tag, hf_log_level_t& level) const noexcept = 0;

  //==============================================================================
  // LOGGING METHODS
  //==============================================================================

  /**
   * @brief Log a message at ERROR level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Error(const char* tag, const char* format, ...) noexcept = 0;

  /**
   * @brief Log a message at WARN level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Warn(const char* tag, const char* format, ...) noexcept = 0;

  /**
   * @brief Log a message at INFO level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Info(const char* tag, const char* format, ...) noexcept = 0;

  /**
   * @brief Log a message at DEBUG level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Debug(const char* tag, const char* format, ...) noexcept = 0;

  /**
   * @brief Log a message at VERBOSE level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Verbose(const char* tag, const char* format, ...) noexcept = 0;

  /**
   * @brief Log a message at specified level
   * @param level Log level
   * @param tag Log tag
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Log(hf_log_level_t level, const char* tag, const char* format,
                              ...) noexcept = 0;

  /**
   * @brief Log a message with va_list (for internal use)
   * @param level Log level
   * @param tag Log tag
   * @param format printf-style format string
   * @param args va_list of arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t LogV(hf_log_level_t level, const char* tag, const char* format,
                               va_list args) noexcept = 0;

  /**
   * @brief Log a message with file and line information
   * @param level Log level
   * @param tag Log tag
   * @param file Source file
   * @param line Source line
   * @param function Function name
   * @param format printf-style format string
   * @param ... printf-style arguments
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t LogWithLocation(hf_log_level_t level, const char* tag, const char* file,
                                          hf_u32_t line, const char* function, const char* format,
                                          ...) noexcept = 0;

  //==============================================================================
  // UTILITY METHODS
  //==============================================================================

  /**
   * @brief Flush any buffered output
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t Flush() noexcept = 0;

  /**
   * @brief Check if a log level is enabled for a tag
   * @param level Log level to check
   * @param tag Log tag (nullptr for default)
   * @return true if level is enabled, false otherwise
   */
  virtual bool IsLevelEnabled(hf_log_level_t level, const char* tag = nullptr) const noexcept = 0;

  /**
   * @brief Get logger statistics
   * @param statistics Output statistics structure
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t GetStatistics(hf_logger_statistics_t& statistics) const noexcept = 0;

  /**
   * @brief Get logger diagnostics
   * @param diagnostics Output diagnostics structure
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t GetDiagnostics(hf_logger_diagnostics_t& diagnostics) const noexcept = 0;

  /**
   * @brief Reset statistics
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t ResetStatistics() noexcept = 0;

  /**
   * @brief Reset diagnostics
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t ResetDiagnostics() noexcept = 0;

  /**
   * @brief Check if logger is healthy
   * @return true if healthy, false otherwise
   */
  virtual bool IsHealthy() const noexcept = 0;

  /**
   * @brief Get last error code
   * @return hf_logger_err_t Last error code
   */
  virtual hf_logger_err_t GetLastError() const noexcept = 0;

  /**
   * @brief Get last error message
   * @param message Output error message buffer
   * @param max_length Maximum message length
   * @return hf_logger_err_t Success or error code
   */
  virtual hf_logger_err_t GetLastErrorMessage(char* message,
                                              hf_u32_t max_length) const noexcept = 0;

protected:
  /**
   * @brief Default constructor
   */
  BaseLogger() = default;

  /**
   * @brief Copy constructor (deleted)
   */
  BaseLogger(const BaseLogger&) = delete;

  /**
   * @brief Assignment operator (deleted)
   */
  BaseLogger& operator=(const BaseLogger&) = delete;

  /**
   * @brief Move constructor (deleted)
   */
  BaseLogger(BaseLogger&&) = delete;

  /**
   * @brief Move assignment operator (deleted)
   */
  BaseLogger& operator=(BaseLogger&&) = delete;
};

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

/**
 * @brief Convert logger error code to string
 * @param error Error code
 * @return const char* Error string
 */
const char* HfLoggerErrToString(hf_logger_err_t error) noexcept;

/**
 * @brief Convert log level to string
 * @param level Log level
 * @return const char* Level string
 */
const char* HfLogLevelToString(hf_log_level_t level) noexcept;

/**
 * @brief Convert log level to short string
 * @param level Log level
 * @return const char* Short level string
 */
const char* HfLogLevelToShortString(hf_log_level_t level) noexcept;

/**
 * @brief Get current timestamp in microseconds
 * @return hf_u64_t Timestamp in microseconds
 */
hf_u64_t HfLoggerGetTimestamp() noexcept;

/**
 * @brief Get current thread ID
 * @return hf_u32_t Thread ID
 */
hf_u32_t HfLoggerGetThreadId() noexcept;

//==============================================================================
// CONVENIENCE MACROS
//==============================================================================

/**
 * @brief Log at ERROR level with file and line information
 */
#define HF_LOG_ERROR(tag, format, ...)                                                            \
  LogWithLocation(hf_log_level_t::LOG_LEVEL_ERROR, tag, __FILE__, __LINE__, __FUNCTION__, format, \
                  ##__VA_ARGS__)

/**
 * @brief Log at WARN level with file and line information
 */
#define HF_LOG_WARN(tag, format, ...)                                                            \
  LogWithLocation(hf_log_level_t::LOG_LEVEL_WARN, tag, __FILE__, __LINE__, __FUNCTION__, format, \
                  ##__VA_ARGS__)

/**
 * @brief Log at INFO level with file and line information
 */
#define HF_LOG_INFO(tag, format, ...)                                                            \
  LogWithLocation(hf_log_level_t::LOG_LEVEL_INFO, tag, __FILE__, __LINE__, __FUNCTION__, format, \
                  ##__VA_ARGS__)

/**
 * @brief Log at DEBUG level with file and line information
 */
#define HF_LOG_DEBUG(tag, format, ...)                                                            \
  LogWithLocation(hf_log_level_t::LOG_LEVEL_DEBUG, tag, __FILE__, __LINE__, __FUNCTION__, format, \
                  ##__VA_ARGS__)

/**
 * @brief Log at VERBOSE level with file and line information
 */
#define HF_LOG_VERBOSE(tag, format, ...)                                                    \
  LogWithLocation(hf_log_level_t::LOG_LEVEL_VERBOSE, tag, __FILE__, __LINE__, __FUNCTION__, \
                  format, ##__VA_ARGS__)

/**
 * @brief Conditional logging macro
 */
#define HF_LOG_IF(condition, level, tag, format, ...)                                       \
  do {                                                                                      \
    if (condition) {                                                                        \
      LogWithLocation(level, tag, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    }                                                                                       \
  } while (0)
