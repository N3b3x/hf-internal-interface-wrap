/**
 * @file StmLogger.cpp
 * @brief STM32 Logger stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmLogger.h"
#include <cstdio>
#include <cstring>

static constexpr auto kNotSupported = hf_logger_err_t::LOGGER_ERR_NOT_SUPPORTED;

StmLogger::StmLogger() noexcept = default;
StmLogger::~StmLogger() noexcept = default;

hf_logger_err_t StmLogger::Initialize(const hf_logger_config_t&) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Deinitialize() noexcept { return kNotSupported; }
bool StmLogger::IsInitialized() const noexcept { return false; }
bool StmLogger::EnsureInitialized() noexcept { return false; }

hf_logger_err_t StmLogger::SetLogLevel(const char*, hf_log_level_t) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::GetLogLevel(const char*, hf_log_level_t&) const noexcept { return kNotSupported; }

hf_logger_err_t StmLogger::Error(const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Warn(const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Info(const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Debug(const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Verbose(const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::Log(hf_log_level_t, const char*, const char*, ...) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::LogV(hf_log_level_t, const char*, const char*, va_list) noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::LogWithLocation(hf_log_level_t, const char*, const char*,
                                            hf_u32_t, const char*, const char*, ...) noexcept { return kNotSupported; }

hf_logger_err_t StmLogger::Flush() noexcept { return kNotSupported; }
bool StmLogger::IsLevelEnabled(hf_log_level_t, const char*) const noexcept { return false; }

hf_logger_err_t StmLogger::GetStatistics(hf_logger_statistics_t&) const noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::GetDiagnostics(hf_logger_diagnostics_t&) const noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::ResetStatistics() noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::ResetDiagnostics() noexcept { return kNotSupported; }
bool StmLogger::IsHealthy() const noexcept { return false; }
hf_logger_err_t StmLogger::GetLastError() const noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::GetLastErrorMessage(char* message, hf_u32_t max_length) const noexcept {
    if (message && max_length > 0) {
        strncpy(message, "STM32 Logger not implemented", max_length - 1);
        message[max_length - 1] = '\0';
    }
    return kNotSupported;
}
hf_logger_err_t StmLogger::PrintStatistics(const char*, bool) const noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::PrintDiagnostics(const char*, bool) const noexcept { return kNotSupported; }
hf_logger_err_t StmLogger::PrintStatus(const char*, bool) const noexcept { return kNotSupported; }
