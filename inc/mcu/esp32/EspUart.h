/**
 * @file EspUart.h
 * @brief MCU-integrated UART controller implementation.
 *
 * This header provides a UART driver implementation for microcontrollers with
 * built-in UART peripherals. On ESP32, this wraps the UART driver,
 * on STM32 it would wrap the UART/USART peripheral, etc. The implementation
 * supports configurable baud rates, data formats, flow control, and
 * interrupt-driven or DMA-based data transfer for efficient communication.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the primary UART implementation for MCUs with integrated UART controllers.
 */

#pragma once

#include "RtosMutex.h"
#include "BaseUart.h"
#include "EspTypes_UART.h"
#include <cstdarg>

/**
 * @class EspUart
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
class EspUart : public BaseUart {
public:
  /**
   * @brief Constructor with port and configuration.
   * @param port Platform-agnostic UART port number
   * @param config UART configuration parameters
   */
  EspUart(hf_port_number_t port, const hf_uart_config_t &config) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~EspUart() noexcept override;

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
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t Write(const uint8_t *data, uint16_t length, uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Read data from the UART.
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t Read(uint8_t *data, uint16_t length, uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Get the number of bytes available to read.
   * @return Number of bytes available in the receive buffer
   */
  uint16_t BytesAvailable() noexcept override;

  /**
   * @brief Flush the transmit buffer.
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t FlushTx() noexcept override;

  /**
   * @brief Flush the receive buffer.
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t FlushRx() noexcept override;

  /**
   * @brief Printf-style formatted output.
   * @param format Format string
   * @param ... Format arguments
   * @return Number of characters written, or -1 on error
   */
  int Printf(const char *format, ...) noexcept override;

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
  hf_uart_err_t GetLastError() const noexcept {
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
  uint16_t ReadUntil(uint8_t *data, uint16_t max_length, uint8_t terminator,
                     uint32_t timeout_ms) noexcept;

  /**
   * @brief Read a line of text (terminated by \n or \r\n).
   * @param buffer Buffer to store the line
   * @param max_length Maximum number of characters to read
   * @param timeout_ms Timeout in milliseconds
   * @return Number of characters read (excluding terminator)
   */
  uint16_t ReadLine(char *buffer, uint16_t max_length, uint32_t timeout_ms) noexcept;

  //==============================================//
  // ESP32C6 ADVANCED FEATURES                   //
  //==============================================//

  /**
   * @brief Configure UART communication mode (standard, RS485, IrDA, etc.).
   * @param mode UART communication mode
   * @return true if successful, false otherwise
   */
  bool SetCommunicationMode(hf_uart_mode_t mode) noexcept;

  /**
   * @brief Get current UART communication mode.
   * @return Current communication mode
   */
  hf_uart_mode_t GetCommunicationMode() const noexcept;

  /**
   * @brief Configure RS485 mode settings.
   * @param rs485_config RS485 configuration parameters
   * @return true if successful, false otherwise
   */
  bool ConfigureRS485(const hf_uart_rs485_config_t &rs485_config) noexcept;

  /**
   * @brief Check if RS485 collision was detected.
   * @return true if collision detected, false otherwise
   */
  bool IsRS485CollisionDetected() noexcept;

  /**
   * @brief Configure IrDA infrared communication mode.
   * @param irda_config IrDA configuration parameters
   * @return true if successful, false otherwise
   */
  bool ConfigureIrDA(const hf_uart_irda_config_t &irda_config) noexcept;

  /**
   * @brief Enable/configure pattern detection for AT commands.
   * @param pattern_config Pattern detection configuration
   * @return true if successful, false otherwise
   */
  bool ConfigurePatternDetection(const hf_uart_pattern_config_t &pattern_config) noexcept;

  /**
   * @brief Disable pattern detection.
   * @return true if successful, false otherwise
   */
  bool DisablePatternDetection() noexcept;

  /**
   * @brief Get detected pattern position in buffer.
   * @param pop_position If true, remove the position from queue
   * @return Pattern position in buffer, or -1 if none found
   */
  int GetPatternPosition(bool pop_position = false) noexcept;

  /**
   * @brief Configure software flow control (XON/XOFF).
   * @param enable Enable software flow control
   * @param xon_threshold Low water mark for XON
   * @param xoff_threshold High water mark for XOFF
   * @return true if successful, false otherwise
   */
  bool ConfigureSoftwareFlowControl(bool enable, uint8_t xon_threshold = 20, 
                                   uint8_t xoff_threshold = 80) noexcept;

  /**
   * @brief Configure UART wakeup from light sleep.
   * @param wakeup_config Wakeup configuration parameters
   * @return true if successful, false otherwise
   */
  bool ConfigureWakeup(const hf_uart_wakeup_config_t &wakeup_config) noexcept;

  /**
   * @brief Configure UART power management settings.
   * @param power_config Power management configuration
   * @return true if successful, false otherwise
   */
  bool ConfigurePowerManagement(const hf_uart_power_config_t &power_config) noexcept;

  /**
   * @brief Configure RX FIFO full threshold for interrupt generation.
   * @param threshold Threshold value (bytes)
   * @return true if successful, false otherwise
   */
  bool SetRxFullThreshold(uint8_t threshold) noexcept;

  /**
   * @brief Configure TX FIFO empty threshold for interrupt generation.
   * @param threshold Threshold value (bytes)
   * @return true if successful, false otherwise
   */
  bool SetTxEmptyThreshold(uint8_t threshold) noexcept;

  /**
   * @brief Configure RX timeout threshold.
   * @param timeout_threshold Timeout in symbol periods (1-126)
   * @return true if successful, false otherwise
   */
  bool SetRxTimeoutThreshold(uint8_t timeout_threshold) noexcept;

  /**
   * @brief Enable/disable RX interrupts.
   * @param enable True to enable, false to disable
   * @return true if successful, false otherwise
   */
  bool EnableRxInterrupts(bool enable) noexcept;

  /**
   * @brief Enable/disable TX interrupts.
   * @param enable True to enable, false to disable
   * @param threshold TX FIFO threshold for interrupt (0-127)
   * @return true if successful, false otherwise
   */
  bool EnableTxInterrupts(bool enable, uint8_t threshold = 10) noexcept;

  /**
   * @brief Configure line signal inversion.
   * @param inverse_mask Bit mask of signals to invert (see uart_signal_inv_t)
   * @return true if successful, false otherwise
   */
  bool SetSignalInversion(uint32_t inverse_mask) noexcept;

  /**
   * @brief Get comprehensive UART statistics.
   * @return UART statistics structure
   */
  hf_uart_statistics_t GetStatistics() const noexcept;

private:
  //==============================================//
  // PRIVATE METHODS                              //
  //==============================================//

  /**
   * @brief Convert platform-specific error to hf_uart_err_t.
   * @param platform_error Platform-specific error code
   * @return Corresponding hf_uart_err_t
   */
  hf_uart_err_t ConvertPlatformError(int32_t platform_error) noexcept;

  /**
   * @brief Get current time in milliseconds.
   * @return Current time in milliseconds
   */
  uint32_t GetCurrentTimeMs() const noexcept;

  /**
   * @brief Get timeout value, converting 0 to default.
   * @param timeout_ms Requested timeout (0 = use default)
   * @return Effective timeout in milliseconds
   */
  uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept {
    return (timeout_ms == 0) ? config_.timeout_ms : timeout_ms;
  }

  /**
   * @brief Platform-specific initialization.
   * @return true if successful, false otherwise
   */
  bool PlatformInitialize() noexcept;

  /**
   * @brief Platform-specific deinitialization.
   * @return true if successful, false otherwise
   */
  bool PlatformDeinitialize() noexcept;

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
    return (parity >= 0 && parity <= 2);
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
   * @brief Internal printf implementation.
   * @param format Format string
   * @param args Variable argument list
   * @return Number of characters written, or -1 on error
   */
  int InternalPrintf(const char *format, va_list args) noexcept;

  /**
   * @brief Ensure UART is initialized.
   * @return true if initialized, false otherwise
   */
  bool EnsureInitialized() noexcept {
    if (!initialized_) {
      initialized_ = Initialize();
    }
    return initialized_;
  }

  //==============================================//
  // PRIVATE MEMBERS                              //
  //==============================================//

  hf_uart_port_native_t uart_port_;           ///< Platform-specific UART port
  hf_uart_config_native_t uart_config_;       ///< Platform-specific UART configuration
  std::atomic<bool> initialized_{false};      ///< Initialization status
  std::atomic<hf_uart_err_t> last_error_{hf_uart_err_t::UART_SUCCESS}; ///< Last error code
  mutable RtosMutex mutex_;                   ///< Thread safety mutex
  hf_uart_statistics_t statistics_;           ///< UART statistics
};
