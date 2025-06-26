/**
 * @file McuUart.h
 * @brief MCU-integrated UART controller implementation.
 *
 * This header provides a UART driver implementation for microcontrollers with
 * built-in UART peripherals. On ESP32, this wraps the UART driver,
 * on STM32 it would wrap the UART/USART peripheral, etc.
 *
 * This is the primary UART implementation for MCUs with integrated UART controllers.
 */

#ifndef MCU_UART_H
#define MCU_UART_H

#include "BaseUart.h"
#include "McuTypes.h"
#include <mutex>
#include <cstdarg>

/**
 * @class McuUart
 * @brief UART driver implementation for microcontrollers with integrated UART peripherals.
 *
 * This class provides UART communication using the microcontroller's built-in
 * UART peripheral. On ESP32, it uses the UART driver. The implementation handles
 * platform-specific details while providing the unified BaseUart API.
 *
 * Features:
 * - Asynchronous serial communication using MCU's integrated UART
 * - Support for various baud rates, data bits, parity, and stop bits
 * - Hardware flow control (RTS/CTS) support
 * - Buffered TX/RX with configurable buffer sizes
 * - Interrupt-driven operation for efficient CPU usage
 * - Printf-style formatted output with variable arguments
 * - Comprehensive error handling and status reporting
 * - Lazy initialization support
 * - Thread-safe operation with mutex protection
 *
 * @note This implementation is thread-safe when used with multiple threads.
 */
class McuUart : public BaseUart {
public:
    /**
     * @brief Constructor with port and configuration.
     * @param port Platform-agnostic UART port number
     * @param config UART configuration parameters
     */
    McuUart(HfPortNumber port, const UartConfig& config) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup.
     */
    ~McuUart() noexcept override;

    //==============================================//
    // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
    //==============================================//

    /**
     * @brief Initialize the UART driver.
     * @return true if successful, false otherwise
     */
    bool Initialize() noexcept override;

    /**
     * @brief Deinitialize the UART driver.
     * @return true if successful, false otherwise
     */
    bool Deinitialize() noexcept override;

    /**
     * @brief Write data to the UART.
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfUartErr result code
     */
    HfUartErr Write(const uint8_t* data, uint16_t length,
                    uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Read data from the UART.
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfUartErr result code
     */
    HfUartErr Read(uint8_t* data, uint16_t length,
                   uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Get the number of bytes available to read.
     * @return Number of bytes available in the receive buffer
     */
    uint16_t BytesAvailable() noexcept override;

    /**
     * @brief Flush the transmit buffer.
     * @return HfUartErr result code
     */
    HfUartErr FlushTx() noexcept override;

    /**
     * @brief Flush the receive buffer.
     * @return HfUartErr result code
     */
    HfUartErr FlushRx() noexcept override;

    /**
     * @brief Printf-style formatted output.
     * @param format Format string
     * @param ... Format arguments
     * @return Number of characters written, or -1 on error
     */
    int Printf(const char* format, ...) noexcept override;

    //==============================================//
    // ENHANCED METHODS                             //
    //==============================================//

    /**
     * @brief Check if the UART is busy transmitting.
     * @return true if busy, false if idle
     */
    bool IsTxBusy() noexcept;

    /**
     * @brief Get the last error that occurred.
     * @return Last error code
     */
    HfUartErr GetLastError() const noexcept {
        return last_error_;
    }

    /**
     * @brief Set a new baud rate (requires reinitialization).
     * @param baud_rate New baud rate in bits per second
     * @return true if successful, false otherwise
     */
    bool SetBaudRate(uint32_t baud_rate) noexcept;

    /**
     * @brief Enable or disable hardware flow control.
     * @param enable True to enable flow control, false to disable
     * @return true if successful, false otherwise
     */
    bool SetFlowControl(bool enable) noexcept;

    /**
     * @brief Get detailed UART status information.
     * @return Platform-specific status information
     */
    uint32_t GetUartStatus() noexcept;

    /**
     * @brief Set RTS line manually (if not using automatic flow control).
     * @param active True to assert RTS, false to deassert
     * @return true if successful, false otherwise
     */
    bool SetRTS(bool active) noexcept;

    /**
     * @brief Get CTS line status.
     * @return true if CTS is asserted, false otherwise
     */
    bool GetCTS() noexcept;

    /**
     * @brief Send a break condition.
     * @param duration_ms Duration of break in milliseconds
     * @return true if successful, false otherwise
     */
    bool SendBreak(uint32_t duration_ms) noexcept;

    /**
     * @brief Detect if a break condition was received.
     * @return true if break detected, false otherwise
     */
    bool IsBreakDetected() noexcept;

    /**
     * @brief Get the number of bytes in the transmit buffer.
     * @return Number of bytes waiting to be transmitted
     */
    uint16_t TxBytesWaiting() noexcept;

    /**
     * @brief Enable or disable loopback mode (for testing).
     * @param enable True to enable loopback, false to disable
     * @return true if successful, false otherwise
     */
    bool SetLoopback(bool enable) noexcept;

    /**
     * @brief Wait for all data to be transmitted.
     * @param timeout_ms Maximum time to wait in milliseconds
     * @return true if all data transmitted, false if timeout
     */
    bool WaitTransmitComplete(uint32_t timeout_ms) noexcept;

    /**
     * @brief Read data with specific termination character.
     * @param data Buffer to store received data
     * @param max_length Maximum number of bytes to read
     * @param terminator Termination character
     * @param timeout_ms Timeout in milliseconds
     * @return Number of bytes read (including terminator if found)
     */
    uint16_t ReadUntil(uint8_t* data, uint16_t max_length, 
                       uint8_t terminator, uint32_t timeout_ms) noexcept;

    /**
     * @brief Read a line of text (terminated by \n or \r\n).
     * @param buffer Buffer to store the line
     * @param max_length Maximum number of characters to read
     * @param timeout_ms Timeout in milliseconds
     * @return Number of characters read (excluding terminator)
     */
    uint16_t ReadLine(char* buffer, uint16_t max_length, uint32_t timeout_ms) noexcept;

private:
    //==============================================//
    // PRIVATE METHODS                              //
    //==============================================//

    /**
     * @brief Convert platform-specific error to HfUartErr.
     * @param platform_error Platform-specific error code
     * @return Corresponding HfUartErr
     */
    HfUartErr ConvertPlatformError(int32_t platform_error) noexcept;

    /**
     * @brief Validate baud rate.
     * @param baud_rate Baud rate to validate
     * @return true if valid, false otherwise
     */
    bool IsValidBaudRate(uint32_t baud_rate) const noexcept {
        // Common baud rates: 9600, 19200, 38400, 57600, 115200, etc.
        // Allow range from 300 to 921600
        return (baud_rate >= 300 && baud_rate <= 921600);
    }

    /**
     * @brief Validate data bits.
     * @param data_bits Data bits to validate
     * @return true if valid, false otherwise
     */
    bool IsValidDataBits(uint8_t data_bits) const noexcept {
        return (data_bits >= 5 && data_bits <= 8);
    }

    /**
     * @brief Validate parity setting.
     * @param parity Parity to validate
     * @return true if valid, false otherwise
     */
    bool IsValidParity(uint8_t parity) const noexcept {
        return (parity <= 2); // 0=None, 1=Even, 2=Odd
    }

    /**
     * @brief Validate stop bits.
     * @param stop_bits Stop bits to validate
     * @return true if valid, false otherwise
     */
    bool IsValidStopBits(uint8_t stop_bits) const noexcept {
        return (stop_bits >= 1 && stop_bits <= 2);
    }

    /**
     * @brief Get timeout value (use default if timeout_ms is 0).
     * @param timeout_ms Requested timeout
     * @return Actual timeout to use
     */
    uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept {
        return (timeout_ms == 0) ? config_.timeout_ms : timeout_ms;
    }

    /**
     * @brief Perform platform-specific initialization.
     * @return true if successful, false otherwise
     */
    bool PlatformInitialize() noexcept;

    /**
     * @brief Perform platform-specific deinitialization.
     * @return true if successful, false otherwise
     */
    bool PlatformDeinitialize() noexcept;

    /**
     * @brief Internal printf implementation with buffer management.
     * @param format Format string
     * @param args Variable arguments list
     * @return Number of characters written, or -1 on error
     */
    int InternalPrintf(const char* format, va_list args) noexcept;

    //==============================================//
    // PRIVATE MEMBERS                              //
    //==============================================//

    mutable std::mutex mutex_;           ///< Thread safety mutex
    hf_uart_handle_t platform_handle_;   ///< Platform-specific UART handle
    HfUartErr last_error_;               ///< Last error that occurred
    uint32_t bytes_transmitted_;         ///< Total bytes transmitted
    uint32_t bytes_received_;            ///< Total bytes received
    bool break_detected_;                ///< Break condition flag
    bool tx_in_progress_;                ///< Transmission in progress flag
    
    // Printf buffer management
    static constexpr uint16_t PRINTF_BUFFER_SIZE = 256; ///< Printf buffer size
    char printf_buffer_[PRINTF_BUFFER_SIZE];     ///< Printf formatting buffer
};

#endif // MCU_UART_H
