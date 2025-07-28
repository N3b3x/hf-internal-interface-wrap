/**
 * @file EspLogger.h
 * @ingroup logger
 * @brief ESP32-specific logger implementation for the HardFOC system.
 *
 * This file provides the ESP32 implementation of the BaseLogger interface,
 * utilizing ESP-IDF's esp_log system (both Log V1 and Log V2) for efficient 
 * and feature-rich logging. It supports all ESP32 variants (C6, Classic, S2, 
 * S3, C3, C2, H2) and provides comprehensive logging capabilities with 
 * performance monitoring.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is thread-safe and optimized for ESP32 platforms.
 * @note Supports both ESP-IDF Log V1 (default) and Log V2 (enhanced) systems.
 */

#pragma once

#include "BaseLogger.h"
#include "utils/RtosMutex.h"

#include <atomic>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

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

/**
 * @class EspLogger
 * @brief ESP32 logger implementation using ESP-IDF logging system (Log V1/V2).
 *
 * This class provides a complete implementation of the BaseLogger interface for ESP32 variants.
 * It leverages ESP-IDF's esp_log system for efficient logging with features like:
 * - Multiple log levels with runtime configuration
 * - Tag-based log level filtering
 * - Performance monitoring and statistics
 * - Thread-safe operations
 * - Custom output callbacks
 * - Message formatting and buffering
 * - Health monitoring and diagnostics
 * - ESP-IDF Log V2 support with enhanced features
 *
 * Key Features:
 * - **ESP-IDF Integration**: Direct integration with esp_log for optimal performance
 * - **Log V2 Support**: Enhanced logging with improved flexibility and reduced flash usage
 * - **Tag-based Filtering**: Runtime log level control per tag
 * - **Thread Safety**: Proper mutex protection for concurrent access
 * - **Performance Monitoring**: Detailed statistics and performance metrics
 * - **Health Monitoring**: Comprehensive health checks and diagnostics
 * - **Custom Output**: Support for custom output callbacks
 * - **Message Buffering**: Efficient message buffering and formatting
 * - **Error Recovery**: Robust error handling and recovery mechanisms
 * - **Buffer Logging**: Support for binary data logging
 * - **Dynamic Formatting**: Log V2 dynamic format string support
 *
 * Usage Example:
 * @code
 * // Create logger with default configuration
 * EspLogger logger;
 * 
 * // Initialize with custom configuration
 * hf_logger_config_t config = {};
 * config.default_level = hf_log_level_t::LOG_LEVEL_INFO;
 * config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
 * config.format_options = hf_log_format_t::LOG_FORMAT_DEFAULT;
 * config.max_message_length = 512;
 * config.buffer_size = 1024;
 * config.enable_thread_safety = true;
 * config.enable_performance_monitoring = true;
 * 
 * if (logger.Initialize(config) == hf_logger_err_t::LOGGER_SUCCESS) {
 *     logger.Info("MAIN", "System initialized successfully");
 *     logger.Debug("SENSOR", "Temperature: %.2f°C", temperature);
 *     logger.Error("COMM", "Communication timeout");
 *     
 *     // Log V2 features (if available)
 *     uint8_t buffer[16] = {0x01, 0x02, 0x03, 0x04};
 *     logger.LogBuffer("DATA", buffer, sizeof(buffer), hf_log_level_t::LOG_LEVEL_DEBUG);
 * }
 * @endcode
 *
 * Advanced Usage with Log V2:
 * @code
 * // Dynamic format strings (Log V2 feature)
 * const char* dynamic_format = "Dynamic message: %s with value %d";
 * logger.Log(hf_log_level_t::LOG_LEVEL_INFO, "TAG", dynamic_format, "test", 42);
 * 
 * // Binary logging
 * uint8_t binary_data[64];
 * // ... fill binary_data ...
 * logger.LogBufferHex("BINARY", binary_data, sizeof(binary_data), hf_log_level_t::LOG_LEVEL_DEBUG);
 * logger.LogBufferChar("TEXT", (const char*)binary_data, sizeof(binary_data), hf_log_level_t::LOG_LEVEL_INFO);
 * @endcode
 *
 * @note EspLogger instances cannot be copied or moved due to hardware resource management.
 * @note If you need to transfer ownership, use std::unique_ptr<EspLogger> or similar smart pointers.
 * @note Log V2 features are automatically detected and used when CONFIG_LOG_VERSION_2 is enabled.
 */
class EspLogger : public BaseLogger {
public:
    /**
     * @brief Default constructor
     */
    EspLogger() noexcept;

    /**
     * @brief Destructor
     */
    ~EspLogger() noexcept override;

    //==============================================================================
    // BASELOGGER INTERFACE IMPLEMENTATION
    //==============================================================================

    /**
     * @brief Initialize the logger
     * @param config Logger configuration
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Initialize(const hf_logger_config_t& config) noexcept override;

    /**
     * @brief Deinitialize the logger
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Deinitialize() noexcept override;

    /**
     * @brief Check if logger is initialized
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const noexcept override;

    /**
     * @brief Ensure logger is initialized (lazy initialization)
     * @return true if initialization successful, false otherwise
     */
    bool EnsureInitialized() noexcept override;

    /**
     * @brief Set log level for a specific tag
     * @param tag Log tag (nullptr for default)
     * @param level Log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t SetLogLevel(const char* tag, hf_log_level_t level) noexcept override;

    /**
     * @brief Get log level for a specific tag
     * @param tag Log tag (nullptr for default)
     * @param level Output log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t GetLogLevel(const char* tag, hf_log_level_t& level) const noexcept override;

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
    hf_logger_err_t Error(const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message at WARN level
     * @param tag Log tag
     * @param format printf-style format string
     * @param ... printf-style arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Warn(const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message at INFO level
     * @param tag Log tag
     * @param format printf-style format string
     * @param ... printf-style arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Info(const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message at DEBUG level
     * @param tag Log tag
     * @param format printf-style format string
     * @param ... printf-style arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Debug(const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message at VERBOSE level
     * @param tag Log tag
     * @param format printf-style format string
     * @param ... printf-style arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Verbose(const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message at specified level
     * @param level Log level
     * @param tag Log tag
     * @param format printf-style format string
     * @param ... printf-style arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Log(hf_log_level_t level, const char* tag, const char* format, ...) noexcept override;

    /**
     * @brief Log a message with va_list (for internal use)
     * @param level Log level
     * @param tag Log tag
     * @param format printf-style format string
     * @param args va_list of arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t LogV(hf_log_level_t level, const char* tag, const char* format, va_list args) noexcept override;

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
    hf_logger_err_t LogWithLocation(hf_log_level_t level, const char* tag, 
                                   const char* file, hf_u32_t line, const char* function,
                                   const char* format, ...) noexcept override;

    //==============================================================================
    // ESP-IDF LOG V2 ENHANCED METHODS
    //==============================================================================

    /**
     * @brief Log a buffer as hex dump (Log V2 feature)
     * @param tag Log tag
     * @param buffer Buffer to log
     * @param length Buffer length
     * @param level Log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t LogBufferHex(const char* tag, const void* buffer, hf_u32_t length, 
                                hf_log_level_t level = hf_log_level_t::LOG_LEVEL_INFO) noexcept;

    /**
     * @brief Log a buffer as character dump (Log V2 feature)
     * @param tag Log tag
     * @param buffer Buffer to log (should contain printable characters)
     * @param length Buffer length
     * @param level Log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t LogBufferChar(const char* tag, const void* buffer, hf_u32_t length,
                                 hf_log_level_t level = hf_log_level_t::LOG_LEVEL_INFO) noexcept;

    /**
     * @brief Log a buffer as hex dump with address (Log V2 feature)
     * @param tag Log tag
     * @param buffer Buffer to log
     * @param length Buffer length
     * @param level Log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t LogBufferHexDump(const char* tag, const void* buffer, hf_u32_t length,
                                    hf_log_level_t level = hf_log_level_t::LOG_LEVEL_INFO) noexcept;

    /**
     * @brief Log a buffer (generic method)
     * @param tag Log tag
     * @param buffer Buffer to log
     * @param length Buffer length
     * @param level Log level
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t LogBuffer(const char* tag, const void* buffer, hf_u32_t length,
                             hf_log_level_t level = hf_log_level_t::LOG_LEVEL_INFO) noexcept;

    //==============================================================================
    // UTILITY METHODS
    //==============================================================================

    /**
     * @brief Flush any buffered output
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t Flush() noexcept override;

    /**
     * @brief Check if a log level is enabled for a tag
     * @param level Log level to check
     * @param tag Log tag (nullptr for default)
     * @return true if level is enabled, false otherwise
     */
    bool IsLevelEnabled(hf_log_level_t level, const char* tag = nullptr) const noexcept override;

    /**
     * @brief Get logger statistics
     * @param statistics Output statistics structure
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t GetStatistics(hf_logger_statistics_t& statistics) const noexcept override;

    /**
     * @brief Get logger diagnostics
     * @param diagnostics Output diagnostics structure
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t GetDiagnostics(hf_logger_diagnostics_t& diagnostics) const noexcept override;

    /**
     * @brief Reset statistics
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t ResetStatistics() noexcept override;

    /**
     * @brief Check if logger is healthy
     * @return true if healthy, false otherwise
     */
    bool IsHealthy() const noexcept override;

    /**
     * @brief Get last error code
     * @return hf_logger_err_t Last error code
     */
    hf_logger_err_t GetLastError() const noexcept override;

    /**
     * @brief Get last error message
     * @param message Output error message buffer
     * @param max_length Maximum message length
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t GetLastErrorMessage(char* message, hf_u32_t max_length) const noexcept override;

    /**
     * @brief Check if Log V2 is available
     * @return true if Log V2 is available, false otherwise
     */
    bool IsLogV2Available() const noexcept;

    /**
     * @brief Get current ESP-IDF log version
     * @return 1 for Log V1, 2 for Log V2
     */
    hf_u8_t GetLogVersion() const noexcept;

private:
    //==============================================================================
    // PRIVATE MEMBERS
    //==============================================================================

    mutable RtosMutex mutex_;                    ///< Thread safety mutex
    std::atomic<bool> initialized_;              ///< Initialization flag
    std::atomic<bool> healthy_;                  ///< Health status flag
    
    hf_logger_config_t config_;                  ///< Logger configuration
    hf_logger_statistics_t statistics_;          ///< Statistics tracking
    hf_logger_diagnostics_t diagnostics_;        ///< Diagnostics information
    
    std::map<std::string, hf_log_level_t> tag_levels_; ///< Tag-specific log levels
    std::vector<char> message_buffer_;           ///< Message formatting buffer
    
    hf_logger_err_t last_error_;                 ///< Last error code
    char last_error_message_[256];               ///< Last error message
    
    hf_u64_t initialization_time_;               ///< Initialization timestamp
    hf_u64_t last_health_check_;                 ///< Last health check timestamp

    // Log V2 specific members
    bool log_v2_available_;                      ///< Log V2 availability flag
    hf_u8_t log_version_;                        ///< Current log version (1 or 2)

    //==============================================================================
    // PRIVATE METHODS
    //==============================================================================

    /**
     * @brief Convert HardFOC log level to ESP-IDF log level
     * @param level HardFOC log level
     * @return esp_log_level_t ESP-IDF log level
     */
    esp_log_level_t ConvertLogLevel(hf_log_level_t level) const noexcept;

    /**
     * @brief Convert ESP-IDF log level to HardFOC log level
     * @param level ESP-IDF log level
     * @return hf_log_level_t HardFOC log level
     */
    hf_log_level_t ConvertLogLevel(esp_log_level_t level) const noexcept;

    /**
     * @brief Format log message with location information
     * @param level Log level
     * @param tag Log tag
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param format Format string
     * @param args va_list of arguments
     * @param formatted_message Output formatted message
     * @param max_length Maximum message length
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t FormatMessage(hf_log_level_t level, const char* tag,
                                 const char* file, hf_u32_t line, const char* function,
                                 const char* format, va_list args,
                                 char* formatted_message, hf_u32_t max_length) noexcept;

    /**
     * @brief Write formatted message to output
     * @param level Log level
     * @param tag Log tag
     * @param message Formatted message
     * @param length Message length
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t WriteMessage(hf_log_level_t level, const char* tag,
                                const char* message, hf_u32_t length) noexcept;

    /**
     * @brief Write message using appropriate ESP-IDF version
     * @param level Log level
     * @param tag Log tag
     * @param format Format string
     * @param args va_list of arguments
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t WriteMessageV(hf_log_level_t level, const char* tag,
                                 const char* format, va_list args) noexcept;

    /**
     * @brief Update statistics for a log operation
     * @param level Log level
     * @param message_length Message length
     * @param success Operation success status
     */
    void UpdateStatistics(hf_log_level_t level, hf_u32_t message_length, bool success) noexcept;

    /**
     * @brief Update diagnostics information
     * @param error Error code (if any)
     */
    void UpdateDiagnostics(hf_logger_err_t error) noexcept;

    /**
     * @brief Perform health check
     * @return true if healthy, false otherwise
     */
    bool PerformHealthCheck() noexcept;

    /**
     * @brief Validate configuration
     * @param config Configuration to validate
     * @return hf_logger_err_t Success or error code
     */
    hf_logger_err_t ValidateConfiguration(const hf_logger_config_t& config) const noexcept;

    /**
     * @brief Convert error code to string
     * @param error Error code
     * @return const char* Error string
     */
    const char* ConvertErrorToString(hf_logger_err_t error) const noexcept;

    /**
     * @brief Get current timestamp in microseconds
     * @return hf_u64_t Timestamp in microseconds
     */
    hf_u64_t GetCurrentTimestamp() const noexcept;

    /**
     * @brief Get current thread ID
     * @return hf_u32_t Thread ID
     */
    hf_u32_t GetCurrentThreadId() const noexcept;

    /**
     * @brief Check if message buffer is sufficient
     * @param required_length Required buffer length
     * @return true if sufficient, false otherwise
     */
    bool EnsureMessageBuffer(hf_u32_t required_length) noexcept;

    /**
     * @brief Detect and initialize Log V2 support
     * @return true if Log V2 is available and initialized, false otherwise
     */
    bool InitializeLogV2() noexcept;

    /**
     * @brief Check Log V2 availability at runtime
     * @return true if Log V2 is available, false otherwise
     */
    bool CheckLogV2Availability() const noexcept;
};

#endif // HF_MCU_FAMILY_ESP32 