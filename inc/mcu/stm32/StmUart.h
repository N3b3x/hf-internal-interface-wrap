/**
 * @file StmUart.h
 * @brief STM32 UART wrapper — wraps UART_HandleTypeDef for BaseUart interface.
 *
 * Users initialise UART in STM32CubeMX, then pass the UART_HandleTypeDef* here.
 * Supports blocking TX/RX, optional DMA ring buffer RX, and printf formatting.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseUart.h"
#include "StmTypes.h"

struct UART_HandleTypeDef;  // Forward declaration

/**
 * @class StmUart
 * @brief STM32 UART implementation.
 *
 * Design:
 * - Accepts a CubeMX-configured UART_HandleTypeDef*.
 * - Blocking TX via HAL_UART_Transmit.
 * - Blocking RX via HAL_UART_Receive.
 * - Optional software circular RX buffer fed by idle-line DMA.
 * - Printf via vsnprintf + Write.
 */
class StmUart : public BaseUart {
public:

    /**
     * @brief Construct from HAL UART handle.
     * @param huart  CubeMX UART handle.
     * @param port   Logical port number (for base class).
     */
    explicit StmUart(UART_HandleTypeDef* huart,
                     hf_port_num_t port = 0) noexcept;

    /**
     * @brief Construct from config struct.
     */
    explicit StmUart(const hf_stm32_uart_config_t& config) noexcept;

    ~StmUart() noexcept override;

    // Non-copyable
    StmUart(const StmUart&) = delete;
    StmUart& operator=(const StmUart&) = delete;

    // ── Pure-virtual overrides (BaseUart) ───────────────────────────────────
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf_uart_err_t Write(const hf_u8_t* data, hf_u16_t length,
                        hf_u32_t timeout_ms = 0) noexcept override;
    hf_uart_err_t Read(hf_u8_t* data, hf_u16_t length,
                       hf_u32_t timeout_ms = 0) noexcept override;
    hf_u16_t BytesAvailable() noexcept override;
    hf_uart_err_t FlushTx() noexcept override;
    hf_uart_err_t FlushRx() noexcept override;
    int Printf(const char* format, ...) noexcept override;

    // ── Accessors ───────────────────────────────────────────────────────────
    UART_HandleTypeDef* GetHalHandle() const noexcept { return huart_; }

    // ── Software RX ring buffer (optional — call EnableRxBuffer) ────────────
    /**
     * @brief Enable a software RX ring buffer.
     * @param buffer    User-provided buffer.
     * @param size      Buffer size in bytes.
     *
     * When enabled, Read() pulls from this ring buffer instead of calling
     * HAL_UART_Receive directly.  The user must still call FeedRxByte() from
     * HAL_UART_RxCpltCallback or a DMA half-transfer ISR.
     */
    void EnableRxBuffer(hf_u8_t* buffer, hf_u16_t size) noexcept;

    /**
     * @brief Feed a byte into the RX ring buffer (call from ISR).
     */
    void FeedRxByte(hf_u8_t byte) noexcept;

    /**
     * @brief Feed a block of bytes into the RX ring buffer (call from ISR/DMA).
     */
    void FeedRxBlock(const hf_u8_t* data, hf_u16_t length) noexcept;

private:
    UART_HandleTypeDef* huart_;
    hf_u32_t default_timeout_ms_;

    // Software RX ring buffer
    hf_u8_t* rx_buf_;
    hf_u16_t rx_buf_size_;
    volatile hf_u16_t rx_head_;  ///< Write index (ISR writes)
    volatile hf_u16_t rx_tail_;  ///< Read  index (user reads)
};
