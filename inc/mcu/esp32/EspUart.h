/**
 * @file EspUart.h
 * @brief ESP32 UART implementation for the HardFOC system.
 *
 * This file provides a comprehensive UART implementation for ESP32 variants using the
 * built-in UART peripheral. The implementation supports multiple ports, configurable
 * baud rates and data formats, hardware flow control, interrupt-driven operation,
 * pattern detection, and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is designed for all ESP32 variants using ESP-IDF v5.4+
 * @note Supports ESP32-C6, ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2
 * @note Each EspUart instance represents a single UART port
 * @note Higher-level applications should instantiate multiple EspUart objects for multi-port boards
 */

#pragma once

#include "McuSelect.h"

// Only compile for ESP32 family
#ifdef HF_MCU_FAMILY_ESP32

#include "BaseUart.h"
#include "RtosMutex.h"
#include "utils/EspTypes.h"

#include <memory>
#include <vector>
#include <array>
#include <atomic>

// ESP-IDF includes for UART functionality
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"

/**
 * @class EspUart
 * @brief ESP32 UART implementation class.
 * 
 * This class provides a complete implementation of the BaseUart interface for ESP32 variants.
 * It supports both polling and interrupt-driven UART modes with comprehensive feature support.
 * Each instance represents a single UART port on the ESP32.
 * 
 * Key Features:
 * - Polling mode: Blocking read/write operations for simple applications
 * - Interrupt mode: Non-blocking operations with event queue and callbacks
 * - Hardware flow control: RTS/CTS support for reliable communication
 * - Pattern detection: AT command and custom pattern detection
 * - Wakeup support: UART wakeup from light sleep mode
 * - RS485 support: Half-duplex and collision detection modes
 * - IrDA support: Infrared communication mode
 * - Thread safety: Proper mutex protection for multi-threaded access
 * - Error handling: Comprehensive error reporting and recovery
 * - Resource management: Automatic cleanup and proper resource lifecycle
 * - Multi-variant support: Works across all ESP32 variants (C6, Classic, S2, S3, C3, C2, H2)
 * 
 * Usage Example (Single UART Port):
 * @code
 * // For ESP32-C6 (3 ports)
 * EspUart uart0({.port_number = 0, .baud_rate = 115200, .tx_pin = 21, .rx_pin = 20});
 * 
 * // For ESP32-C3 (2 ports)
 * EspUart uart0({.port_number = 0, .baud_rate = 115200, .tx_pin = 21, .rx_pin = 20});
 * 
 * if (uart0.EnsureInitialized()) {
 *   const char* message = "Hello World!";
 *   if (uart0.Write(reinterpret_cast<const uint8_t*>(message), strlen(message)) == hf_uart_err_t::UART_SUCCESS) {
 *     // Message sent successfully
 *   }
 * }
 * @endcode
 * 
 * Usage Example (Interrupt mode with callbacks):
 * @code
 * EspUart uart({.port_number = 0, .operating_mode = hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT});
 * uart.SetEventCallback([](const hf_uart_event_native_t* event, void* user_data) {
 *   if (event->type == UART_DATA) {
 *     // Handle received data
 *   }
 *   return false; // Return true to yield to higher priority task
 * });
 * uart.EnsureInitialized();
 * @endcode
 * 
 * @note EspUart instances cannot be copied or moved due to hardware resource management.
 * @note If you need to transfer ownership, use std::unique_ptr<EspUart> or similar smart pointers.
 */
class EspUart : public BaseUart {
public:
  //==============================================================================
  // CONSTANTS
  //==============================================================================

  static constexpr uint8_t MAX_PORTS = 3;          ///< Maximum UART ports
  static constexpr uint32_t MAX_BAUD_RATE = 5000000;      ///< Maximum baud rate
  static constexpr uint32_t MIN_BAUD_RATE = 110;      ///< Minimum baud rate
  static constexpr uint32_t DEFAULT_BAUD_RATE = 115200; ///< Default baud rate
  static constexpr uint16_t MAX_BUFFER_SIZE = 1024; ///< Maximum buffer size
  static constexpr uint16_t DEFAULT_BUFFER_SIZE = 256; ///< Default buffer size

  //==============================================================================
  // CONSTRUCTOR AND DESTRUCTOR
  //==============================================================================

  /**
   * @brief Constructor for ESP32 UART controller
   * @param config UART port configuration
   * @note Uses lazy initialization - no hardware action until first operation
   */
  explicit EspUart(const hf_uart_config_t& config) noexcept;



  /**
   * @brief Destructor - ensures clean shutdown
   */
  virtual ~EspUart() noexcept override;

  // Prevent copying and moving
  EspUart(const EspUart&) = delete;
  EspUart& operator=(const EspUart&) = delete;
  EspUart(EspUart&&) = delete;
  EspUart& operator=(EspUart&&) = delete;

  //==============================================================================
  // LIFECYCLE (BaseUart Interface)
  //==============================================================================

  /**
   * @brief Initialize the UART driver.
   * @return true if successful, false otherwise
   * @note This is called automatically by EnsureInitialized() on first use
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the UART driver.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept override;



  // Note: EnsureInitialized() is inherited from BaseUart and provides lazy initialization
  // Thread safety is handled in Initialize() and Deinitialize() methods

  //==============================================================================
  // BASIC UART OPERATIONS (BaseUart Interface)
  //==============================================================================

  /**
   * @brief Write data to the UART.
   * @param data Data buffer to transmit
   * @param length Number of bytes to write
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t Write(const uint8_t* data, uint16_t length, uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Read data from the UART.
   * @param data Buffer to store received data
   * @param length Number of bytes to read
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t Read(uint8_t* data, uint16_t length, uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Write a single byte to the UART.
   * @param byte Byte to write
   * @return true if successful, false otherwise
   */
  bool WriteByte(uint8_t byte) noexcept override;

  /**
   * @brief Set the baud rate.
   * @param baud_rate New baud rate
   * @return true if successful, false otherwise
   */
  bool SetBaudRate(uint32_t baud_rate) noexcept;

  /**
   * @brief Enable or disable hardware flow control.
   * @param enable true to enable, false to disable
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetFlowControl(bool enable) noexcept;

  /**
   * @brief Set RTS line state.
   * @param active true for active, false for inactive
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetRTS(bool active) noexcept;

  /**
   * @brief Send a break condition.
   * @param duration_ms Break duration in milliseconds
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SendBreak(uint32_t duration_ms) noexcept;

  /**
   * @brief Enable or disable loopback mode.
   * @param enable true to enable, false to disable
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetLoopback(bool enable) noexcept;

  /**
   * @brief Wait for transmission to complete.
   * @param timeout_ms Timeout in milliseconds
   * @return true if successful, false on timeout
   */
  bool WaitTransmitComplete(uint32_t timeout_ms) noexcept;

  //==============================================================================
  // ADVANCED UART FEATURES
  //==============================================================================

  /**
   * @brief Set UART operating mode.
   * @param mode Operating mode (polling/interrupt/DMA)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetOperatingMode(hf_uart_operating_mode_t mode) noexcept;

  /**
   * @brief Read data until a specific terminator is found.
   * @param data Buffer to store received data
   * @param max_length Maximum number of bytes to read
   * @param terminator Terminator byte to search for
   * @param timeout_ms Timeout in milliseconds
   * @return Number of bytes read (including terminator)
   */
  uint16_t ReadUntil(uint8_t* data, uint16_t max_length, uint8_t terminator, uint32_t timeout_ms) noexcept;

  /**
   * @brief Read a line of text (until newline).
   * @param buffer Buffer to store the line
   * @param max_length Maximum line length
   * @param timeout_ms Timeout in milliseconds
   * @return Number of characters read (excluding newline)
   */
  uint16_t ReadLine(char* buffer, uint16_t max_length, uint32_t timeout_ms) noexcept;

  /**
   * @brief Set UART communication mode (UART/RS485/IrDA).
   * @param mode Communication mode
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetCommunicationMode(hf_uart_mode_t mode) noexcept;

  /**
   * @brief Configure RS485 mode.
   * @param rs485_config RS485 configuration
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t ConfigureRS485(const hf_uart_rs485_config_t& rs485_config) noexcept;

  /**
   * @brief Configure IrDA mode.
   * @param irda_config IrDA configuration
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t ConfigureIrDA(const hf_uart_irda_config_t& irda_config) noexcept;

  /**
   * @brief Get pattern detection position.
   * @param pop_position true to pop the position from queue, false to peek
   * @return Pattern position or -1 if no pattern detected
   */
  int GetPatternPosition(bool pop_position = false) noexcept;

  /**
   * @brief Configure software flow control (XON/XOFF).
   * @param enable true to enable, false to disable
   * @param xon_threshold XON threshold (default: 20)
   * @param xoff_threshold XOFF threshold (default: 80)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t ConfigureSoftwareFlowControl(bool enable, uint8_t xon_threshold = 20, uint8_t xoff_threshold = 80) noexcept;

  /**
   * @brief Configure UART wakeup from light sleep.
   * @param wakeup_config Wakeup configuration
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t ConfigureWakeup(const hf_uart_wakeup_config_t& wakeup_config) noexcept;

  /**
   * @brief Set RX full threshold for interrupts.
   * @param threshold Threshold value
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetRxFullThreshold(uint8_t threshold) noexcept;

  /**
   * @brief Set TX empty threshold for interrupts.
   * @param threshold Threshold value
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetTxEmptyThreshold(uint8_t threshold) noexcept;

  /**
   * @brief Set RX timeout threshold.
   * @param timeout_threshold Timeout threshold value
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetRxTimeoutThreshold(uint8_t timeout_threshold) noexcept;

  /**
   * @brief Enable or disable RX interrupts.
   * @param enable true to enable, false to disable
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t EnableRxInterrupts(bool enable) noexcept;

  /**
   * @brief Enable or disable TX interrupts.
   * @param enable true to enable, false to disable
   * @param threshold TX empty threshold (default: 10)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t EnableTxInterrupts(bool enable, uint8_t threshold = 10) noexcept;

  /**
   * @brief Set signal inversion mask.
   * @param inverse_mask Inversion mask (UART_SIGNAL_INV_DISABLE, UART_SIGNAL_INV_TXD, etc.)
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetSignalInversion(uint32_t inverse_mask) noexcept;

  //==============================================================================
  // CALLBACKS AND EVENT HANDLING
  //==============================================================================

  /**
   * @brief Set UART event callback.
   * @param callback Event callback function
   * @param user_data User data pointer
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetEventCallback(hf_uart_event_callback_t callback, void* user_data = nullptr) noexcept;

  /**
   * @brief Set pattern detection callback.
   * @param callback Pattern callback function
   * @param user_data User data pointer
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetPatternCallback(hf_uart_pattern_callback_t callback, void* user_data = nullptr) noexcept;

  /**
   * @brief Set break detection callback.
   * @param callback Break callback function
   * @param user_data User data pointer
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t SetBreakCallback(hf_uart_break_callback_t callback, void* user_data = nullptr) noexcept;

  //==============================================================================
  // STATUS AND INFORMATION
  //==============================================================================

  /**
   * @brief Get UART statistics.
   * @param statistics Output statistics structure
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t GetStatistics(hf_uart_statistics_t& statistics) const noexcept override;

  /**
   * @brief Get UART diagnostics.
   * @param diagnostics Output diagnostics structure
   * @return hf_uart_err_t result code
   */
  hf_uart_err_t GetDiagnostics(hf_uart_diagnostics_t& diagnostics) const noexcept override;

  /**
   * @brief Get the last error that occurred.
   * @return Last error code
   */
  hf_uart_err_t GetLastError() const noexcept;

  /**
   * @brief Get current UART configuration.
   * @return Current port configuration
   */
  const hf_uart_config_t& GetPortConfig() const noexcept;

  /**
   * @brief Get current operating mode.
   * @return Current operating mode
   */
  hf_uart_operating_mode_t GetOperatingMode() const noexcept;

  /**
   * @brief Get current communication mode.
   * @return Current communication mode
   */
  hf_uart_mode_t GetCommunicationMode() const noexcept;

  /**
   * @brief Check if pattern detection is enabled.
   * @return true if enabled, false otherwise
   */
  bool IsPatternDetectionEnabled() const noexcept;

  /**
   * @brief Check if wakeup is enabled.
   * @return true if enabled, false otherwise
   */
  bool IsWakeupEnabled() const noexcept;

  /**
   * @brief Check if transmission is in progress.
   * @return true if transmitting, false otherwise
   */
  bool IsTransmitting() const noexcept;

  /**
   * @brief Check if reception is active.
   * @return true if receiving, false otherwise
   */
  bool IsReceiving() const noexcept;

  /**
   * @brief Check if break condition was detected.
   * @return true if break detected, false otherwise
   */
  bool IsBreakDetected() noexcept;

  /**
   * @brief Get number of bytes waiting in TX buffer.
   * @return Number of bytes waiting
   */
  uint16_t TxBytesWaiting() noexcept;

  //==============================================================================
  // PRINTF SUPPORT
  //==============================================================================

  /**
   * @brief Print formatted string to UART.
   * @param format Format string
   * @param ... Variable arguments
   * @return Number of characters written
   */
  int Printf(const char* format, ...) noexcept;

  /**
   * @brief Print formatted string to UART with va_list.
   * @param format Format string
   * @param args Variable arguments
   * @return Number of characters written
   */
  int VPrintf(const char* format, va_list args) noexcept;

private:
  //==============================================================================
  // INTERNAL STATE STRUCTURES
  //==============================================================================

  /**
   * @brief UART state tracking structure.
   */
  struct UartState {
    bool configured;              ///< UART is configured
    bool enabled;                 ///< UART is enabled
    hf_uart_config_t config; ///< Current configuration
    hf_uart_operating_mode_t operating_mode; ///< Current operating mode
    hf_uart_mode_t communication_mode; ///< Current communication mode
    hf_uart_err_t last_error;     ///< Last error for this UART

    UartState() noexcept
        : configured(false), enabled(false), config(), 
          operating_mode(hf_uart_operating_mode_t::HF_UART_MODE_POLLING),
          communication_mode(hf_uart_mode_t::HF_UART_MODE_UART),
          last_error(hf_uart_err_t::UART_SUCCESS) {}
  };

  //==============================================================================
  // INTERNAL METHODS
  //==============================================================================

  /**
   * @brief Validate configuration
   * @return UART_SUCCESS if valid, error code otherwise
   */
  hf_uart_err_t ValidateConfiguration() const noexcept;

  /**
   * @brief Platform-specific initialization
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t PlatformInitialize() noexcept;

  /**
   * @brief Platform-specific deinitialization
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t PlatformDeinitialize() noexcept;

  /**
   * @brief Install UART driver
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t InstallDriver() noexcept;

  /**
   * @brief Uninstall UART driver
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t UninstallDriver() noexcept;

  /**
   * @brief Configure UART parameters
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t ConfigureUart() noexcept;

  /**
   * @brief Configure UART pins
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t ConfigurePins() noexcept;

  /**
   * @brief Start event task for interrupt mode
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t StartEventTask() noexcept;

  /**
   * @brief Stop event task
   * @return UART_SUCCESS on success, error code on failure
   */
  hf_uart_err_t StopEventTask() noexcept;

  /**
   * @brief Event task function
   * @param arg Task argument (this pointer)
   */
  static void EventTask(void* arg) noexcept;

  /**
   * @brief Handle UART events
   * @param event UART event
   */
  void HandleUartEvent(const uart_event_t* event) noexcept;

  /**
   * @brief Convert platform error to HardFOC error
   * @param platform_error Platform error code
   * @return HardFOC error code
   */
  hf_uart_err_t ConvertPlatformError(int32_t platform_error) noexcept;

  /**
   * @brief Update statistics
   * @param result Operation result
   * @param start_time_us Operation start time
   * @return Result for chaining
   */
  hf_uart_err_t UpdateStatistics(hf_uart_err_t result, uint64_t start_time_us) noexcept;

  /**
   * @brief Update diagnostics
   * @param error Error that occurred
   */
  void UpdateDiagnostics(hf_uart_err_t error) noexcept;

  /**
   * @brief Get timeout value in milliseconds
   * @param timeout_ms Requested timeout
   * @return Actual timeout value
   */
  [[nodiscard]] uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept;

  /**
   * @brief Internal printf implementation
   * @param format Format string
   * @param args Variable arguments
   * @return Number of characters written
   */
  int InternalPrintf(const char* format, va_list args) noexcept;

  /**
   * @brief Pattern callback wrapper (ISR-safe)
   * @param pattern_pos Pattern position
   * @param user_data User data pointer
   * @return true to yield to higher priority task
   */
  static bool IRAM_ATTR PatternCallbackWrapper(int pattern_pos, void* user_data) noexcept;

  /**
   * @brief Break callback wrapper (ISR-safe)
   * @param break_duration Break duration
   * @param user_data User data pointer
   * @return true to yield to higher priority task
   */
  static bool IRAM_ATTR BreakCallbackWrapper(uint32_t break_duration, void* user_data) noexcept;

  //==============================================================================
  // MEMBER VARIABLES
  //==============================================================================

  mutable RtosMutex mutex_; ///< Thread safety mutex
  hf_uart_config_t port_config_; ///< Port configuration
  std::atomic<bool> initialized_; ///< Initialization state (atomic for lazy init)
  uart_port_t uart_port_; ///< Native UART port handle
  
  // Event handling for interrupt mode
  QueueHandle_t event_queue_; ///< UART event queue
  TaskHandle_t event_task_handle_; ///< Event task handle
  hf_uart_event_callback_t event_callback_; ///< Event callback
  hf_uart_pattern_callback_t pattern_callback_; ///< Pattern callback
  hf_uart_break_callback_t break_callback_; ///< Break callback
  void* event_callback_user_data_; ///< Event callback user data
  void* pattern_callback_user_data_; ///< Pattern callback user data
  void* break_callback_user_data_; ///< Break callback user data
  
  // Operating mode and communication state
  hf_uart_operating_mode_t operating_mode_; ///< Current operating mode
  hf_uart_mode_t communication_mode_; ///< Current communication mode
  bool pattern_detection_enabled_; ///< Pattern detection enabled
  bool software_flow_enabled_; ///< Software flow control enabled
  bool wakeup_enabled_; ///< Wakeup enabled
  bool break_detected_; ///< Break condition detected
  bool tx_in_progress_; ///< Transmission in progress
  
  // Error tracking
  hf_uart_err_t last_error_; ///< Last error that occurred
  
  // Statistics and diagnostics
  hf_uart_statistics_t statistics_; ///< UART statistics
  hf_uart_diagnostics_t diagnostics_; ///< UART diagnostics
  
  // Printf buffer
  char printf_buffer_[256]; ///< Printf buffer

  // Add missing overrides for BaseUart pure virtuals
  uint16_t BytesAvailable() noexcept override;
  hf_uart_err_t FlushTx() noexcept override;
  hf_uart_err_t FlushRx() noexcept override;
  bool IsTxBusy() noexcept;
};

#else
#error "Unsupported MCU platform. Please add support for your target MCU."
#endif
