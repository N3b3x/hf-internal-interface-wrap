/**
 * @file StmLogger.cpp
 * @brief STM32 Logger implementation — UART/SWO/printf output.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmLogger.h"
#include <cstdio>
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* pData,
                                  uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_GetTick(void);

// ITM stimulus port for SWO output
static inline void HF_ITM_SendChar(char ch) {
    // ITM stimulus port 0
    volatile uint32_t* ITM_STIM0 = reinterpret_cast<volatile uint32_t*>(0xE0000000UL);
    volatile uint32_t* ITM_TER   = reinterpret_cast<volatile uint32_t*>(0xE0000E00UL);
    if ((*ITM_TER & 1UL) != 0UL) {
        while ((*ITM_STIM0 & 1UL) == 0UL) { /* wait FIFO ready */ }
        *ITM_STIM0 = static_cast<uint32_t>(ch);
    }
}
}

namespace {
    const char* LevelToPrefix(hf_log_level_t level) {
        switch (level) {
            case hf_log_level_t::HF_LOG_LEVEL_ERROR:   return "E";
            case hf_log_level_t::HF_LOG_LEVEL_WARN:    return "W";
            case hf_log_level_t::HF_LOG_LEVEL_INFO:    return "I";
            case hf_log_level_t::HF_LOG_LEVEL_DEBUG:   return "D";
            case hf_log_level_t::HF_LOG_LEVEL_VERBOSE: return "V";
            default:                                    return "?";
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmLogger::StmLogger(UART_HandleTypeDef* uart_handle, Backend backend) noexcept
    : backend_(backend)
    , uart_handle_(uart_handle)
    , initialized_(false)
    , global_level_(hf_log_level_t::HF_LOG_LEVEL_INFO)
    , statistics_{}
    , diagnostics_{}
    , last_error_(hf_logger_err_t::LOGGER_SUCCESS)
{
    std::memset(format_buffer_, 0, sizeof(format_buffer_));
}

StmLogger::~StmLogger() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

hf_logger_err_t StmLogger::Initialize(const hf_logger_config_t& config) noexcept {
    if (initialized_) return hf_logger_err_t::LOGGER_SUCCESS;

    if (backend_ == Backend::UART_HAL && !uart_handle_) {
        last_error_ = hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
        return last_error_;
    }

    global_level_ = config.default_level;
    std::memset(&statistics_, 0, sizeof(statistics_));
    std::memset(&diagnostics_, 0, sizeof(diagnostics_));
    initialized_ = true;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::Deinitialize() noexcept {
    initialized_ = false;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

bool StmLogger::IsInitialized() const noexcept { return initialized_; }

bool StmLogger::EnsureInitialized() noexcept {
    if (initialized_) return true;
    // Auto-init with default config if UART handle is available
    if (uart_handle_ || backend_ != Backend::UART_HAL) {
        hf_logger_config_t default_config{};
        default_config.default_level = hf_log_level_t::HF_LOG_LEVEL_INFO;
        return Initialize(default_config) == hf_logger_err_t::LOGGER_SUCCESS;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOG LEVEL
// ═══════════════════════════════════════════════════════════════════════════════

hf_logger_err_t StmLogger::SetLogLevel(const char* /*tag*/, hf_log_level_t level) noexcept {
    global_level_ = level;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::GetLogLevel(const char* /*tag*/, hf_log_level_t& level) const noexcept {
    level = global_level_;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

bool StmLogger::IsLevelEnabled(hf_log_level_t level, const char* /*tag*/) const noexcept {
    return initialized_ && level <= global_level_;
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOGGING METHODS
// ═══════════════════════════════════════════════════════════════════════════════

hf_logger_err_t StmLogger::Error(const char* tag, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(hf_log_level_t::HF_LOG_LEVEL_ERROR, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::Warn(const char* tag, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(hf_log_level_t::HF_LOG_LEVEL_WARN, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::Info(const char* tag, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(hf_log_level_t::HF_LOG_LEVEL_INFO, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::Debug(const char* tag, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(hf_log_level_t::HF_LOG_LEVEL_DEBUG, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::Verbose(const char* tag, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(hf_log_level_t::HF_LOG_LEVEL_VERBOSE, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::Log(hf_log_level_t level, const char* tag,
                                const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    auto result = FormatAndOutput(level, tag, format, args);
    va_end(args);
    return result;
}

hf_logger_err_t StmLogger::LogV(hf_log_level_t level, const char* tag,
                                 const char* format, va_list args) noexcept {
    return FormatAndOutput(level, tag, format, args);
}

hf_logger_err_t StmLogger::LogWithLocation(hf_log_level_t level, const char* tag,
                                            const char* file, hf_u32_t line,
                                            const char* function,
                                            const char* format, ...) noexcept {
    if (!IsLevelEnabled(level, tag)) return hf_logger_err_t::LOGGER_SUCCESS;

    // Format: "[L] (TAG) file:line func: message\r\n"
    int prefix_len = snprintf(format_buffer_, sizeof(format_buffer_),
                              "[%s] (%s) %s:%u %s: ",
                              LevelToPrefix(level),
                              tag ? tag : "?",
                              file ? file : "?",
                              static_cast<unsigned>(line),
                              function ? function : "?");
    if (prefix_len < 0) prefix_len = 0;
    if (static_cast<size_t>(prefix_len) >= sizeof(format_buffer_) - 3) {
        prefix_len = static_cast<int>(sizeof(format_buffer_)) - 3;
    }

    va_list args;
    va_start(args, format);
    int msg_len = vsnprintf(format_buffer_ + prefix_len,
                            sizeof(format_buffer_) - static_cast<size_t>(prefix_len) - 2,
                            format, args);
    va_end(args);

    if (msg_len < 0) msg_len = 0;
    int total = prefix_len + msg_len;
    if (static_cast<size_t>(total) < sizeof(format_buffer_) - 2) {
        format_buffer_[total] = '\r';
        format_buffer_[total + 1] = '\n';
        format_buffer_[total + 2] = '\0';
        total += 2;
    }

    OutputString(format_buffer_, static_cast<hf_u32_t>(total));
    statistics_.total_messages++;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::Flush() noexcept {
    // UART HAL transmit is synchronous — Flush is a no-op
    return hf_logger_err_t::LOGGER_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// STATISTICS / DIAGNOSTICS
// ═══════════════════════════════════════════════════════════════════════════════

hf_logger_err_t StmLogger::GetStatistics(hf_logger_statistics_t& stats) const noexcept {
    stats = statistics_;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::GetDiagnostics(hf_logger_diagnostics_t& diag) const noexcept {
    diag = diagnostics_;
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::ResetStatistics() noexcept {
    std::memset(&statistics_, 0, sizeof(statistics_));
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::ResetDiagnostics() noexcept {
    std::memset(&diagnostics_, 0, sizeof(diagnostics_));
    return hf_logger_err_t::LOGGER_SUCCESS;
}

bool StmLogger::IsHealthy() const noexcept { return initialized_; }

hf_logger_err_t StmLogger::GetLastError() const noexcept { return last_error_; }

hf_logger_err_t StmLogger::GetLastErrorMessage(char* message, hf_u32_t max_length) const noexcept {
    if (message && max_length > 0) {
        strncpy(message, "OK", max_length - 1);
        message[max_length - 1] = '\0';
    }
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::PrintStatistics(const char* tag, bool /*detailed*/) const noexcept {
    if (!initialized_) return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
    char buf[128];
    int len = snprintf(buf, sizeof(buf), "[I] (%s) Stats: msgs=%u, errs=%u\r\n",
                       tag ? tag : "LOG",
                       static_cast<unsigned>(statistics_.total_messages),
                       static_cast<unsigned>(statistics_.error_count));
    if (len > 0) const_cast<StmLogger*>(this)->OutputString(buf, static_cast<hf_u32_t>(len));
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::PrintDiagnostics(const char* /*tag*/, bool /*detailed*/) const noexcept {
    return hf_logger_err_t::LOGGER_SUCCESS;
}

hf_logger_err_t StmLogger::PrintStatus(const char* tag, bool /*detailed*/) const noexcept {
    if (!initialized_) return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "[I] (%s) Logger: %s\r\n",
                       tag ? tag : "LOG",
                       initialized_ ? "active" : "inactive");
    if (len > 0) const_cast<StmLogger*>(this)->OutputString(buf, static_cast<hf_u32_t>(len));
    return hf_logger_err_t::LOGGER_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE
// ═══════════════════════════════════════════════════════════════════════════════

void StmLogger::OutputString(const char* str, hf_u32_t length) noexcept {
    if (!str || length == 0) return;

    switch (backend_) {
        case Backend::UART_HAL:
            if (uart_handle_) {
                HAL_UART_Transmit(uart_handle_,
                                  reinterpret_cast<uint8_t*>(const_cast<char*>(str)),
                                  static_cast<uint16_t>(length), 100);
            }
            break;

        case Backend::SWO_ITM:
            for (hf_u32_t i = 0; i < length; ++i) {
                HF_ITM_SendChar(str[i]);
            }
            break;

        case Backend::PRINTF:
        default:
            std::fwrite(str, 1, length, stdout);
            break;
    }
}

hf_logger_err_t StmLogger::FormatAndOutput(hf_log_level_t level, const char* tag,
                                            const char* format, va_list args) noexcept {
    if (!initialized_) {
        if (!EnsureInitialized()) return hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED;
    }
    if (!IsLevelEnabled(level, tag)) return hf_logger_err_t::LOGGER_SUCCESS;

    // Format: "[L] (TAG) message\r\n"
    uint32_t tick = HAL_GetTick();
    int prefix_len = snprintf(format_buffer_, sizeof(format_buffer_),
                              "%u [%s] (%s) ",
                              static_cast<unsigned>(tick),
                              LevelToPrefix(level),
                              tag ? tag : "?");
    if (prefix_len < 0) prefix_len = 0;
    if (static_cast<size_t>(prefix_len) >= sizeof(format_buffer_) - 3) {
        prefix_len = static_cast<int>(sizeof(format_buffer_)) - 3;
    }

    int msg_len = vsnprintf(format_buffer_ + prefix_len,
                            sizeof(format_buffer_) - static_cast<size_t>(prefix_len) - 2,
                            format, args);
    if (msg_len < 0) msg_len = 0;

    int total = prefix_len + msg_len;
    if (static_cast<size_t>(total) < sizeof(format_buffer_) - 2) {
        format_buffer_[total] = '\r';
        format_buffer_[total + 1] = '\n';
        format_buffer_[total + 2] = '\0';
        total += 2;
    }

    OutputString(format_buffer_, static_cast<hf_u32_t>(total));
    statistics_.total_messages++;
    return hf_logger_err_t::LOGGER_SUCCESS;
}
