/**
 * @file StmUart.cpp
 * @brief STM32 UART implementation — blocking TX/RX with optional ring buffer.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmUart.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_UART_Transmit(UART_HandleTypeDef* huart,
                                  const uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_UART_Receive(UART_HandleTypeDef* huart,
                                 uint8_t* pData, uint16_t Size, uint32_t Timeout);
extern uint32_t HAL_UART_Abort(UART_HandleTypeDef* huart);
extern uint32_t HAL_GetTick(void);
}

namespace {
    constexpr hf_u32_t kDefaultTimeoutMs = 1000;
    constexpr int kPrintfBufSize = 256;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTORS / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmUart::StmUart(UART_HandleTypeDef* huart, hf_port_num_t port) noexcept
    : BaseUart(port)
    , huart_(huart)
    , default_timeout_ms_(kDefaultTimeoutMs)
    , rx_buf_(nullptr)
    , rx_buf_size_(0)
    , rx_head_(0)
    , rx_tail_(0)
{
}

StmUart::StmUart(const hf_stm32_uart_config_t& config) noexcept
    : BaseUart(config.port)
    , huart_(config.huart)
    , default_timeout_ms_(config.timeout_ms > 0 ? config.timeout_ms : kDefaultTimeoutMs)
    , rx_buf_(nullptr)
    , rx_buf_size_(0)
    , rx_head_(0)
    , rx_tail_(0)
{
}

StmUart::~StmUart() noexcept {
    if (initialized_) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

bool StmUart::Initialize() noexcept {
    if (initialized_) return true;
    if (!huart_) return false;

    // CubeMX already initialises the UART peripheral.
    // Reset ring buffer state.
    rx_head_ = 0;
    rx_tail_ = 0;

    initialized_ = true;
    statistics_.initialization_timestamp = static_cast<hf_u64_t>(HAL_GetTick()) * 1000ULL;
    return true;
}

bool StmUart::Deinitialize() noexcept {
    if (!initialized_) return true;
    HAL_UART_Abort(huart_);
    initialized_ = false;
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// WRITE / READ
// ═══════════════════════════════════════════════════════════════════════════════

hf_uart_err_t StmUart::Write(const hf_u8_t* data, hf_u16_t length,
                             hf_u32_t timeout_ms) noexcept {
    if (!initialized_) return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;

    hf_u32_t tout = (timeout_ms > 0) ? timeout_ms : default_timeout_ms_;
    uint32_t status = HAL_UART_Transmit(huart_, data, length, tout);

    if (hf::stm32::IsHalOk(status)) {
        statistics_.tx_byte_count += length;
        return hf_uart_err_t::UART_SUCCESS;
    }

    // Map HAL status
    if (status == 0x03U) { // HAL_TIMEOUT
        statistics_.timeout_count++;
        return hf_uart_err_t::UART_ERR_TIMEOUT;
    }

    statistics_.tx_error_count++;
    return hf_uart_err_t::UART_ERR_TRANSMISSION_FAILED;
}

hf_uart_err_t StmUart::Read(hf_u8_t* data, hf_u16_t length,
                            hf_u32_t timeout_ms) noexcept {
    if (!initialized_) return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
    if (!data || length == 0) return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;

    // If ring buffer is active, pull from there
    if (rx_buf_ && rx_buf_size_ > 0) {
        hf_u16_t available = BytesAvailable();
        if (available == 0) return hf_uart_err_t::UART_ERR_BUFFER_EMPTY;

        hf_u16_t to_read = (length < available) ? length : available;
        for (hf_u16_t i = 0; i < to_read; ++i) {
            data[i] = rx_buf_[rx_tail_];
            rx_tail_ = static_cast<hf_u16_t>((rx_tail_ + 1) % rx_buf_size_);
        }
        statistics_.rx_byte_count += to_read;
        return hf_uart_err_t::UART_SUCCESS;
    }

    // Direct blocking read
    hf_u32_t tout = (timeout_ms > 0) ? timeout_ms : default_timeout_ms_;
    uint32_t status = HAL_UART_Receive(huart_, data, length, tout);

    if (hf::stm32::IsHalOk(status)) {
        statistics_.rx_byte_count += length;
        return hf_uart_err_t::UART_SUCCESS;
    }

    if (status == 0x03U) { // HAL_TIMEOUT
        statistics_.timeout_count++;
        return hf_uart_err_t::UART_ERR_TIMEOUT;
    }

    statistics_.rx_error_count++;
    return hf_uart_err_t::UART_ERR_RECEPTION_FAILED;
}

hf_u16_t StmUart::BytesAvailable() noexcept {
    if (!rx_buf_ || rx_buf_size_ == 0) return 0;
    hf_u16_t h = rx_head_;
    hf_u16_t t = rx_tail_;
    if (h >= t) return h - t;
    return rx_buf_size_ - t + h;
}

hf_uart_err_t StmUart::FlushTx() noexcept {
    if (!initialized_) return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
    // STM32 HAL doesn't expose a TX flush; TX is blocking so it's always flushed.
    return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t StmUart::FlushRx() noexcept {
    if (!initialized_) return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
    rx_head_ = 0;
    rx_tail_ = 0;
    return hf_uart_err_t::UART_SUCCESS;
}

int StmUart::Printf(const char* format, ...) noexcept {
    if (!initialized_ || !format) return -1;

    char buf[kPrintfBufSize];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (len <= 0) return len;
    if (len > static_cast<int>(sizeof(buf) - 1)) len = sizeof(buf) - 1;

    hf_uart_err_t err = Write(reinterpret_cast<const hf_u8_t*>(buf),
                              static_cast<hf_u16_t>(len));
    return (err == hf_uart_err_t::UART_SUCCESS) ? len : -1;
}

// ═══════════════════════════════════════════════════════════════════════════════
// RX RING BUFFER
// ═══════════════════════════════════════════════════════════════════════════════

void StmUart::EnableRxBuffer(hf_u8_t* buffer, hf_u16_t size) noexcept {
    rx_buf_ = buffer;
    rx_buf_size_ = size;
    rx_head_ = 0;
    rx_tail_ = 0;
}

void StmUart::FeedRxByte(hf_u8_t byte) noexcept {
    if (!rx_buf_ || rx_buf_size_ == 0) return;
    hf_u16_t next = static_cast<hf_u16_t>((rx_head_ + 1) % rx_buf_size_);
    if (next == rx_tail_) {
        // Buffer full — drop oldest
        rx_tail_ = static_cast<hf_u16_t>((rx_tail_ + 1) % rx_buf_size_);
        statistics_.overrun_error_count++;
    }
    rx_buf_[rx_head_] = byte;
    rx_head_ = next;
}

void StmUart::FeedRxBlock(const hf_u8_t* data, hf_u16_t length) noexcept {
    for (hf_u16_t i = 0; i < length; ++i) {
        FeedRxByte(data[i]);
    }
}
