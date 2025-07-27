/**
 * @file LoggerExample.cpp
 * @brief Example demonstrating the HardFOC logger system.
 *
 * This example shows how to use the new logger system with the LoggerManager
 * singleton and demonstrates various logging features.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "utils/LoggerManager.h"
#include "base/BaseLogger.h"

#include <cstdio>
#include <cstring>

//==============================================================================
// EXAMPLE FUNCTIONS
//==============================================================================

void DemonstrateBasicLogging() {
    printf("=== Basic Logging Demo ===\n");
    
    // Basic logging at different levels
    LoggerManager::Info("DEMO", "Starting basic logging demonstration");
    LoggerManager::Debug("DEMO", "Debug message with value: %d", 42);
    LoggerManager::Warn("DEMO", "Warning: Temperature is high: %.1f°C", 85.5);
    LoggerManager::Error("DEMO", "Error: Communication timeout after %d ms", 5000);
    LoggerManager::Verbose("DEMO", "Verbose message with multiple values: %s, %d, %.2f", "test", 123, 3.14);
    
    printf("Basic logging completed\n\n");
}

void DemonstrateTagBasedFiltering() {
    printf("=== Tag-Based Filtering Demo ===\n");
    
    // Set different log levels for different tags
    auto& logger = LoggerManager::GetInstance();
    logger.SetLogLevel("SENSOR", hf_log_level_t::LOG_LEVEL_VERBOSE);
    logger.SetLogLevel("COMM", hf_log_level_t::LOG_LEVEL_WARN);
    logger.SetLogLevel("MOTOR", hf_log_level_t::LOG_LEVEL_ERROR);
    
    // Log messages with different tags
    LoggerManager::Info("SENSOR", "Sensor reading: %.2f", 23.45);
    LoggerManager::Debug("SENSOR", "Raw ADC value: %d", 2048);
    LoggerManager::Verbose("SENSOR", "Calibration factor: %.6f", 1.000123);
    
    LoggerManager::Info("COMM", "This should not appear (WARN level)");
    LoggerManager::Warn("COMM", "Communication warning: retry %d", 3);
    LoggerManager::Error("COMM", "Communication error: timeout");
    
    LoggerManager::Info("MOTOR", "This should not appear (ERROR level)");
    LoggerManager::Warn("MOTOR", "This should not appear (ERROR level)");
    LoggerManager::Error("MOTOR", "Motor fault detected: overcurrent");
    
    printf("Tag-based filtering completed\n\n");
}

void DemonstrateStatistics() {
    printf("=== Statistics Demo ===\n");
    
    auto& logger = LoggerManager::GetInstance();
    
    // Log some messages to generate statistics
    for (int i = 0; i < 10; i++) {
        LoggerManager::Info("STATS", "Message %d", i);
    }
    
    for (int i = 0; i < 5; i++) {
        LoggerManager::Debug("STATS", "Debug message %d", i);
    }
    
    for (int i = 0; i < 3; i++) {
        LoggerManager::Error("STATS", "Error message %d", i);
    }
    
    // Get and display statistics
    hf_logger_statistics_t stats;
    if (logger.GetStatistics(stats) == hf_logger_err_t::LOGGER_SUCCESS) {
        printf("Logger Statistics:\n");
        printf("  Total messages: %llu\n", stats.total_messages);
        printf("  Total bytes written: %llu\n", stats.total_bytes_written);
        printf("  Messages by level:\n");
        printf("    ERROR: %llu\n", stats.messages_by_level[static_cast<int>(hf_log_level_t::LOG_LEVEL_ERROR)]);
        printf("    WARN: %llu\n", stats.messages_by_level[static_cast<int>(hf_log_level_t::LOG_LEVEL_WARN)]);
        printf("    INFO: %llu\n", stats.messages_by_level[static_cast<int>(hf_log_level_t::LOG_LEVEL_INFO)]);
        printf("    DEBUG: %llu\n", stats.messages_by_level[static_cast<int>(hf_log_level_t::LOG_LEVEL_DEBUG)]);
        printf("    VERBOSE: %llu\n", stats.messages_by_level[static_cast<int>(hf_log_level_t::LOG_LEVEL_VERBOSE)]);
        printf("  Average message length: %llu\n", stats.average_message_length);
        printf("  Max message length seen: %llu\n", stats.max_message_length_seen);
    }
    
    printf("Statistics demo completed\n\n");
}

void DemonstrateDiagnostics() {
    printf("=== Diagnostics Demo ===\n");
    
    auto& logger = LoggerManager::GetInstance();
    
    // Get and display diagnostics
    hf_logger_diagnostics_t diagnostics;
    if (logger.GetDiagnostics(diagnostics) == hf_logger_err_t::LOGGER_SUCCESS) {
        printf("Logger Diagnostics:\n");
        printf("  Initialized: %s\n", diagnostics.is_initialized ? "Yes" : "No");
        printf("  Healthy: %s\n", diagnostics.is_healthy ? "Yes" : "No");
        printf("  Last error: %s\n", HfLoggerErrToString(diagnostics.last_error));
        printf("  Consecutive errors: %u\n", diagnostics.consecutive_errors);
        printf("  Error recovery count: %u\n", diagnostics.error_recovery_count);
        printf("  Uptime: %llu seconds\n", diagnostics.uptime_seconds);
        
        if (strlen(diagnostics.last_error_message) > 0) {
            printf("  Last error message: %s\n", diagnostics.last_error_message);
        }
    }
    
    printf("Diagnostics demo completed\n\n");
}

void DemonstrateCustomConfiguration() {
    printf("=== Custom Configuration Demo ===\n");
    
    // Create custom configuration
    hf_logger_config_t config = {};
    config.default_level = hf_log_level_t::LOG_LEVEL_DEBUG;
    config.output_destination = hf_log_output_t::LOG_OUTPUT_UART;
    config.format_options = hf_log_format_t::LOG_FORMAT_TIMESTAMP | 
                           hf_log_format_t::LOG_FORMAT_LEVEL | 
                           hf_log_format_t::LOG_FORMAT_TAG;
    config.max_message_length = 256;
    config.buffer_size = 512;
    config.flush_interval_ms = 50;
    config.enable_thread_safety = true;
    config.enable_performance_monitoring = true;
    
    // Reinitialize with custom configuration
    if (LoggerManager::Initialize(config) == hf_logger_err_t::LOGGER_SUCCESS) {
        LoggerManager::Info("CONFIG", "Logger reinitialized with custom configuration");
        LoggerManager::Debug("CONFIG", "Debug level enabled with custom config");
        LoggerManager::Verbose("CONFIG", "Verbose level disabled by default level");
    } else {
        printf("Failed to initialize with custom configuration\n");
    }
    
    printf("Custom configuration demo completed\n\n");
}

void DemonstrateConvenienceMacros() {
    printf("=== Convenience Macros Demo ===\n");
    
    // Use convenience macros
    HF_LOG_INFO("MACRO", "Using convenience macro for INFO");
    HF_LOG_DEBUG("MACRO", "Using convenience macro for DEBUG with value: %d", 999);
    HF_LOG_ERROR("MACRO", "Using convenience macro for ERROR");
    
    // Conditional logging
    bool debug_enabled = true;
    HF_LOG_IF(debug_enabled, hf_log_level_t::LOG_LEVEL_DEBUG, "MACRO", "Conditional debug message");
    
    debug_enabled = false;
    HF_LOG_IF(debug_enabled, hf_log_level_t::LOG_LEVEL_DEBUG, "MACRO", "This should not appear");
    
    printf("Convenience macros demo completed\n\n");
}

//==============================================================================
// MAIN FUNCTION
//==============================================================================

extern "C" void app_main(void) {
    printf("=== HardFOC Logger System Demo ===\n\n");
    
    // Initialize logger with default configuration
    if (LoggerManager::Initialize() != hf_logger_err_t::LOGGER_SUCCESS) {
        printf("Failed to initialize logger\n");
        return;
    }
    
    printf("Logger initialized successfully\n\n");
    
    // Run demonstrations
    DemonstrateBasicLogging();
    DemonstrateTagBasedFiltering();
    DemonstrateStatistics();
    DemonstrateDiagnostics();
    DemonstrateCustomConfiguration();
    DemonstrateConvenienceMacros();
    
    printf("=== Logger Demo Completed ===\n");
    printf("Check the serial output for formatted log messages\n");
} 