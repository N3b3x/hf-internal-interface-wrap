/**
 * @file StmLogger.h
 * @brief STM32 Logger — UART/SWO/ITM-based logging with full BaseLogger interface.
 *
 * Supports multiple output backends:
 * - UART (via CubeMX UART handle) — most common for STM32
 * - SWO/ITM (via ITM stimulus port) — for SWD debugging
 * - printf (via _write syscall / semihosting) — fallback
 *
 * @section Usage
 * @code
 * extern UART_HandleTypeDef huart2;
 *
 * StmLogger logger(&huart2);
 * hf_logger_config_t config{};
 * config.default_level = hf_log_level_t::HF_LOG_LEVEL_INFO;
 * logger.Initialize(config);
 *
 * logger.Info("APP", "System started, version %d.%d", 1, 0);
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseLogger.h"
#include "StmTypes.h"
#include <cstdarg>

/**
 * @brief STM32 Logger — production-quality logging over UART/SWO/printf.
 */
class StmLogger : public BaseLogger {
public:
    /// @brief Output backend selection
    enum class Backend : hf_u8_t {
        UART_HAL = 0,   ///< Use HAL_UART_Transmit
        SWO_ITM  = 1,   ///< Use ITM_SendChar (SWO trace)
        PRINTF   = 2    ///< Use printf / _write
    };

    /// @brief Construct with UART backend (most common)
    explicit StmLogger(UART_HandleTypeDef* uart_handle = nullptr,
                       Backend backend = Backend::UART_HAL) noexcept;

    ~StmLogger() noexcept override;

    // ── BaseLogger overrides ─────────────────────────────────────────────

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
    hf_logger_err_t Log(hf_log_level_t level, const char* tag,
                        const char* format, ...) noexcept override;
    hf_logger_err_t LogV(hf_log_level_t level, const char* tag,
                         const char* format, va_list args) noexcept override;
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

private:
    /// @brief Write formatted string to output backend
    void OutputString(const char* str, hf_u32_t length) noexcept;

    /// @brief Format and output a log message
    hf_logger_err_t FormatAndOutput(hf_log_level_t level, const char* tag,
                                    const char* format, va_list args) noexcept;

    Backend                backend_;           ///< Output backend
    UART_HandleTypeDef*    uart_handle_;        ///< UART HAL handle (for UART backend)
    bool                   initialized_;        ///< Init state
    hf_log_level_t         global_level_;       ///< Current global log level
    hf_logger_statistics_t statistics_;          ///< Statistics
    hf_logger_diagnostics_t diagnostics_;       ///< Diagnostics
    hf_logger_err_t        last_error_;         ///< Last error code
    char                   format_buffer_[256]; ///< Formatting scratch buffer
};
