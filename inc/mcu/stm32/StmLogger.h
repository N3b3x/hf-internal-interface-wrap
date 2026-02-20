/**
 * @file StmLogger.h
 * @brief STM32 Logger stub — printf-based fallback for non-ESP32 platforms.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseLogger.h"

/**
 * @brief STM32 Logger — stub implementation.
 *
 * All log methods return LOGGER_ERR_NOT_SUPPORTED. Override with
 * UART/SWO printf-based logging when integrating STM32 HAL.
 */
class StmLogger : public BaseLogger {
public:
    StmLogger() noexcept;
    ~StmLogger() noexcept override;

    hf_logger_err_t Initialize(const hf_logger_config_t& config) noexcept override;
    hf_logger_err_t Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    bool EnsureInitialized() noexcept override;

    hf_logger_err_t SetLogLevel(const char* tag, hf_log_level_t level) noexcept override;
    hf_logger_err_t GetLogLevel(const char* tag, hf_log_level_t& level) const noexcept override;

    hf_logger_err_t Error(const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t Warn(const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t Info(const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t Debug(const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t Verbose(const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t Log(hf_log_level_t level, const char* tag, const char* format, ...) noexcept override;
    hf_logger_err_t LogV(hf_log_level_t level, const char* tag, const char* format, va_list args) noexcept override;
    hf_logger_err_t LogWithLocation(hf_log_level_t level, const char* tag, const char* file,
                                    hf_u32_t line, const char* function,
                                    const char* format, ...) noexcept override;

    hf_logger_err_t Flush() noexcept override;
    bool IsLevelEnabled(hf_log_level_t level, const char* tag = nullptr) const noexcept override;

    hf_logger_err_t GetStatistics(hf_logger_statistics_t& statistics) const noexcept override;
    hf_logger_err_t GetDiagnostics(hf_logger_diagnostics_t& diagnostics) const noexcept override;
    hf_logger_err_t ResetStatistics() noexcept override;
    hf_logger_err_t ResetDiagnostics() noexcept override;
    bool IsHealthy() const noexcept override;
    hf_logger_err_t GetLastError() const noexcept override;
    hf_logger_err_t GetLastErrorMessage(char* message, hf_u32_t max_length) const noexcept override;
    hf_logger_err_t PrintStatistics(const char* tag = nullptr, bool detailed = true) const noexcept override;
    hf_logger_err_t PrintDiagnostics(const char* tag = nullptr, bool detailed = true) const noexcept override;
    hf_logger_err_t PrintStatus(const char* tag = nullptr, bool detailed = true) const noexcept override;
};
