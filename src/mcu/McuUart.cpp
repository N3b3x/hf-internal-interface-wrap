/**
 * @file McuUart.cpp
 * @brief Implementation of MCU-integrated UART controller.
 *
 * This file provides the implementation for UART communication using the
 * microcontroller's built-in UART peripheral. The implementation supports
 * configurable baud rates, data formats, flow control, and interrupt-driven
 * or DMA-based data transfer for efficient serial communication with
 * comprehensive error handling and platform abstraction.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "McuUart.h"
#include <cstdio>
#include <cstring>

// Platform-specific includes
#ifdef HF_MCU_FAMILY_ESP32
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "McuUart";
#endif

//==============================================//
// CONSTRUCTOR & DESTRUCTOR                     //
//==============================================//

McuUart::McuUart(HfPortNumber port, const UartConfig &config) noexcept
    : BaseUart(port, config), platform_handle_(nullptr), last_error_(HfUartErr::UART_SUCCESS),
      bytes_transmitted_(0), bytes_received_(0), break_detected_(false), tx_in_progress_(false),
      current_mode_(UartMode::HF_UART_MODE_UART), pattern_detection_enabled_(false),
      software_flow_enabled_(false), wakeup_enabled_(false) {
  memset(printf_buffer_, 0, sizeof(printf_buffer_));
  // Initialize statistics timestamp
  statistics_.initialization_timestamp = GetCurrentTimeMs() * 1000; // Convert to microseconds
}

McuUart::~McuUart() noexcept {
  if (initialized_) {
    Deinitialize();
  }
}

//==============================================//
// OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
//==============================================//

bool McuUart::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Validate configuration
  if (!IsValidBaudRate(config_.baud_rate)) {
    last_error_ = HfUartErr::UART_ERR_INVALID_BAUD_RATE;
    return false;
  }

  if (!IsValidDataBits(config_.data_bits)) {
    last_error_ = HfUartErr::UART_ERR_INVALID_DATA_BITS;
    return false;
  }

  if (!IsValidParity(config_.parity)) {
    last_error_ = HfUartErr::UART_ERR_INVALID_PARITY;
    return false;
  }

  if (!IsValidStopBits(config_.stop_bits)) {
    last_error_ = HfUartErr::UART_ERR_INVALID_STOP_BITS;
    return false;
  }

  if (config_.tx_pin == HF_GPIO_INVALID || config_.rx_pin == HF_GPIO_INVALID) {
    last_error_ = HfUartErr::UART_ERR_PIN_CONFIGURATION_ERROR;
    return false;
  }

  // Platform-specific initialization
  if (!PlatformInitialize()) {
    return false;
  }

  initialized_ = true;
  last_error_ = HfUartErr::UART_SUCCESS;
  return true;
}

bool McuUart::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  bool result = PlatformDeinitialize();
  if (result) {
    initialized_ = false;
    last_error_ = HfUartErr::UART_SUCCESS;
  }

  return result;
}

HfUartErr McuUart::Write(const uint8_t *data, uint16_t length, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return HfUartErr::UART_ERR_NOT_INITIALIZED;
  }

  if (!data && length > 0) {
    return HfUartErr::UART_ERR_NULL_POINTER;
  }

  if (length == 0) {
    return HfUartErr::UART_SUCCESS;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
  uint32_t timeout = GetTimeoutMs(timeout_ms);

  tx_in_progress_ = true;
  int bytes_written = uart_write_bytes(static_cast<hf_uart_port_native_t>(port_),
                                       reinterpret_cast<const char *>(data), length);

  if (bytes_written >= 0) {
    // Wait for transmission to complete if timeout specified
    if (timeout > 0) {
      esp_err_t err = uart_wait_tx_done(static_cast<hf_uart_port_native_t>(port_), pdMS_TO_TICKS(timeout));
      if (err != ESP_OK) {
        tx_in_progress_ = false;
        last_error_ = HfUartErr::UART_ERR_TIMEOUT;
        return last_error_;
      }
    }

    bytes_transmitted_ += bytes_written;
    tx_in_progress_ = false;
    last_error_ = HfUartErr::UART_SUCCESS;
    return last_error_;
  } else {
    tx_in_progress_ = false;
    last_error_ = HfUartErr::UART_ERR_TRANSMISSION_FAILED;
    return last_error_;
  }
#else
  (void)timeout_ms;
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return last_error_;
#endif
}

HfUartErr McuUart::Read(uint8_t *data, uint16_t length, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return HfUartErr::UART_ERR_NOT_INITIALIZED;
  }

  if (!data || length == 0) {
    return HfUartErr::UART_ERR_INVALID_PARAMETER;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

#ifdef HF_MCU_FAMILY_ESP32
  uint32_t timeout = GetTimeoutMs(timeout_ms);

  int bytes_read =
      uart_read_bytes(static_cast<hf_uart_port_native_t>(port_), data, length, pdMS_TO_TICKS(timeout));

  if (bytes_read >= 0) {
    bytes_received_ += bytes_read;
    last_error_ = (bytes_read == length) ? HfUartErr::UART_SUCCESS : HfUartErr::UART_ERR_TIMEOUT;
    return last_error_;
  } else {
    last_error_ = HfUartErr::UART_ERR_RECEPTION_FAILED;
    return last_error_;
  }
#else
  (void)timeout_ms;
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return last_error_;
#endif
}

uint16_t McuUart::BytesAvailable() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

#ifdef HF_MCU_FAMILY_ESP32
  size_t bytes_available = 0;
  esp_err_t err = uart_get_buffered_data_len(static_cast<hf_uart_port_native_t>(port_), &bytes_available);
  if (err == ESP_OK) {
    return static_cast<uint16_t>(bytes_available);
  }
#endif

  return 0;
}

HfUartErr McuUart::FlushTx() noexcept {
  if (!EnsureInitialized()) {
    return HfUartErr::UART_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err =
      uart_wait_tx_done(static_cast<hf_uart_port_native_t>(port_), pdMS_TO_TICKS(config_.timeout_ms));
  last_error_ = ConvertPlatformError(err);
  return last_error_;
#else
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return last_error_;
#endif
}

HfUartErr McuUart::FlushRx() noexcept {
  if (!EnsureInitialized()) {
    return HfUartErr::UART_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_flush_input(static_cast<hf_uart_port_native_t>(port_));
  last_error_ = ConvertPlatformError(err);
  return last_error_;
#else
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return last_error_;
#endif
}

int McuUart::Printf(const char *format, ...) noexcept {
  if (!EnsureInitialized() || !format) {
    return -1;
  }

  va_list args;
  va_start(args, format);
  int result = InternalPrintf(format, args);
  va_end(args);

  return result;
}

//==============================================//
// ENHANCED METHODS                             //
//==============================================//

bool McuUart::IsTxBusy() noexcept {
  return tx_in_progress_;
}

bool McuUart::SetBaudRate(uint32_t baud_rate) noexcept {
  if (!IsValidBaudRate(baud_rate)) {
    return false;
  }

  config_.baud_rate = baud_rate;

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    initialized_ = false;
    if (Deinitialize() && Initialize()) {
      return true;
    }
    initialized_ = was_initialized;
    return false;
  }

  return true;
}

bool McuUart::SetFlowControl(bool enable) noexcept {
  config_.use_hardware_flow_control = enable;

  // Reinitialize if already initialized
  if (initialized_) {
    bool was_initialized = initialized_;
    initialized_ = false;
    if (Deinitialize() && Initialize()) {
      return true;
    }
    initialized_ = was_initialized;
    return false;
  }

  return true;
}

uint32_t McuUart::GetUartStatus() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // Return platform-specific status information
  uint32_t status = static_cast<uint32_t>(last_error_);
  if (tx_in_progress_)
    status |= 0x80000000;
  if (break_detected_)
    status |= 0x40000000;
  return status;
#else
  return 0;
#endif
}

bool McuUart::SetRTS(bool active) noexcept {
  if (config_.rts_pin == HF_GPIO_INVALID) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_rts(static_cast<hf_uart_port_native_t>(port_), active ? 1 : 0);
  return err == ESP_OK;
#else
  (void)active;
  return false;
#endif
}

bool McuUart::GetCTS() noexcept {
  if (config_.cts_pin == HF_GPIO_INVALID) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Implementation depends on platform capabilities
  return gpio_get_level(static_cast<gpio_num_t>(config_.cts_pin)) == 1;
#else
  return false;
#endif
}

bool McuUart::SendBreak(uint32_t duration_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Send break condition
  esp_err_t err = uart_set_line_inverse(static_cast<hf_uart_port_native_t>(port_), UART_SIGNAL_TXD_INV);
  if (err != ESP_OK) {
    return false;
  }

  // Hold break for specified duration
  vTaskDelay(pdMS_TO_TICKS(duration_ms));

  // Clear break condition
  err = uart_set_line_inverse(static_cast<hf_uart_port_native_t>(port_), 0);
  return err == ESP_OK;
#else
  (void)duration_ms;
  return false;
#endif
}

bool McuUart::IsBreakDetected() noexcept {
  bool detected = break_detected_;
  break_detected_ = false; // Clear flag after reading
  return detected;
}

uint16_t McuUart::TxBytesWaiting() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // ESP32 doesn't provide direct access to TX buffer level
  // Return 0 if not transmitting, or estimate based on recent writes
  return tx_in_progress_ ? 1 : 0;
#else
  return 0;
#endif
}

bool McuUart::SetLoopback(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Configure loopback mode (connect TX to RX internally)
  uint32_t mode = enable ? UART_MODE_UART | UART_MODE_LOOPBACK : UART_MODE_UART;
  esp_err_t err = uart_set_mode(static_cast<hf_uart_port_native_t>(port_), mode);
  return err == ESP_OK;
#else
  (void)enable;
  return false;
#endif
}

bool McuUart::WaitTransmitComplete(uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_wait_tx_done(static_cast<hf_uart_port_native_t>(port_), pdMS_TO_TICKS(timeout_ms));
  return err == ESP_OK;
#else
  (void)timeout_ms;
  return false;
#endif
}

uint16_t McuUart::ReadUntil(uint8_t *data, uint16_t max_length, uint8_t terminator,
                            uint32_t timeout_ms) noexcept {
  if (!data || max_length == 0) {
    return 0;
  }

  if (!EnsureInitialized()) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint16_t bytes_read = 0;
  uint32_t start_time = GetCurrentTimeMs();
  uint32_t timeout = GetTimeoutMs(timeout_ms);

  while (bytes_read < max_length) {
    // Check timeout
    if (timeout > 0 && (GetCurrentTimeMs() - start_time) >= timeout) {
      break;
    }

    // Try to read one byte
    uint8_t byte;
    HfUartErr result = Read(&byte, 1, 100); // Short timeout for each byte
    
    if (result == HfUartErr::UART_SUCCESS) {
      data[bytes_read++] = byte;
      
      // Check if we found the terminator
      if (byte == terminator) {
        break;
      }
    } else if (result == HfUartErr::UART_ERR_TIMEOUT) {
      // Continue trying if we haven't hit the overall timeout
      continue;
    } else {
      // Other error, stop reading
      break;
    }
  }

  return bytes_read;
}

uint16_t McuUart::ReadLine(char *buffer, uint16_t max_length, uint32_t timeout_ms) noexcept {
  if (!buffer || max_length == 0) {
    return 0;
  }

  if (!EnsureInitialized()) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint16_t chars_read = 0;
  uint32_t start_time = GetCurrentTimeMs();
  uint32_t timeout = GetTimeoutMs(timeout_ms);

  while (chars_read < (max_length - 1)) { // Leave space for null terminator
    // Check timeout
    if (timeout > 0 && (GetCurrentTimeMs() - start_time) >= timeout) {
      break;
    }

    // Try to read one character
    uint8_t ch;
    HfUartErr result = Read(&ch, 1, 100); // Short timeout for each character
    
    if (result == HfUartErr::UART_SUCCESS) {
      // Handle line endings
      if (ch == '\r') {
        // CR - check for following LF
        uint8_t next_ch;
        HfUartErr next_result = Read(&next_ch, 1, 10); // Very short timeout for LF
        if (next_result == HfUartErr::UART_SUCCESS && next_ch == '\n') {
          // CRLF sequence - line complete
          break;
        } else {
          // Just CR - line complete, put back the next character if any
          // Note: ESP32 UART doesn't support unget, so we just ignore the peeked character
          break;
        }
      } else if (ch == '\n') {
        // LF - line complete
        break;
      } else {
        // Regular character
        buffer[chars_read++] = static_cast<char>(ch);
      }
    } else if (result == HfUartErr::UART_ERR_TIMEOUT) {
      // Continue trying if we haven't hit the overall timeout
      continue;
    } else {
      // Other error, stop reading
      break;
    }
  }

  // Null-terminate the string
  buffer[chars_read] = '\0';

  return chars_read;
}

//==============================================//
// ESP32C6 ADVANCED FEATURES IMPLEMENTATION    //
//==============================================//

bool McuUart::SetCommunicationMode(UartMode mode) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  uint32_t esp_mode;
  switch (mode) {
    case UartMode::HF_UART_MODE_UART:
      esp_mode = UART_MODE_UART;
      break;
    case UartMode::HF_UART_MODE_RS485_HALF_DUPLEX:
      esp_mode = UART_MODE_RS485_HALF_DUPLEX;
      break;
    case UartMode::HF_UART_MODE_IRDA:
      esp_mode = UART_MODE_IRDA;
      break;
    case UartMode::HF_UART_MODE_RS485_COLLISION_DETECT:
      esp_mode = UART_MODE_RS485_COLLISION_DETECT;
      break;
    case UartMode::HF_UART_MODE_RS485_APP_CTRL:
      esp_mode = UART_MODE_RS485_APP_CTRL;
      break;
    default:
      return false;
  }

  esp_err_t err = uart_set_mode(static_cast<hf_uart_port_native_t>(port_), 
                                static_cast<uart_mode_t>(esp_mode));
  if (err == ESP_OK) {
    current_mode_ = mode;
    return true;
  }
#else
  (void)mode;
#endif
  return false;
}

UartMode McuUart::GetCommunicationMode() const noexcept {
  return current_mode_;
}

bool McuUart::ConfigureRS485(const UartRs485Config &rs485_config) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Set RS485 mode first
  if (!SetCommunicationMode(rs485_config.mode)) {
    return false;
  }

  rs485_config_ = rs485_config;
  return true;
#else
  (void)rs485_config;
  return false;
#endif
}

bool McuUart::IsRS485CollisionDetected() noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  bool collision_flag = false;
  esp_err_t err = uart_get_collision_flag(static_cast<hf_uart_port_native_t>(port_), &collision_flag);
  return (err == ESP_OK) && collision_flag;
#else
  return false;
#endif
}

bool McuUart::ConfigureIrDA(const UartIrdaConfig &irda_config) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (irda_config.enable_irda) {
    // Set IrDA mode
    if (!SetCommunicationMode(UartMode::HF_UART_MODE_IRDA)) {
      return false;
    }

    // Configure signal inversion if needed
    uint32_t invert_mask = 0;
    if (irda_config.invert_tx) invert_mask |= UART_SIGNAL_IRDA_TX_INV;
    if (irda_config.invert_rx) invert_mask |= UART_SIGNAL_IRDA_RX_INV;

    if (invert_mask != 0) {
      esp_err_t err = uart_set_line_inverse(static_cast<hf_uart_port_native_t>(port_), invert_mask);
      if (err != ESP_OK) {
        return false;
      }
    }
  }

  irda_config_ = irda_config;
  return true;
#else
  (void)irda_config;
  return false;
#endif
}

bool McuUart::ConfigurePatternDetection(const UartPatternConfig &pattern_config) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (pattern_config.enable_pattern_detection) {
    esp_err_t err = uart_enable_pattern_det_baud_intr(
        static_cast<hf_uart_port_native_t>(port_),
        pattern_config.pattern_char,
        pattern_config.pattern_char_num,
        pattern_config.char_timeout,
        pattern_config.post_idle,
        pattern_config.pre_idle);
    
    if (err == ESP_OK) {
      pattern_detection_enabled_ = true;
      pattern_config_ = pattern_config;
      return true;
    }
  } else {
    return DisablePatternDetection();
  }
#else
  (void)pattern_config;
#endif
  return false;
}

bool McuUart::DisablePatternDetection() noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_disable_pattern_det_intr(static_cast<hf_uart_port_native_t>(port_));
  if (err == ESP_OK) {
    pattern_detection_enabled_ = false;
    return true;
  }
#endif
  return false;
}

int McuUart::GetPatternPosition(bool pop_position) noexcept {
  if (!EnsureInitialized() || !pattern_detection_enabled_) {
    return -1;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (pop_position) {
    return uart_pattern_pop_pos(static_cast<hf_uart_port_native_t>(port_));
  } else {
    return uart_pattern_get_pos(static_cast<hf_uart_port_native_t>(port_));
  }
#else
  (void)pop_position;
  return -1;
#endif
}

bool McuUart::ConfigureSoftwareFlowControl(bool enable, uint8_t xon_threshold, 
                                          uint8_t xoff_threshold) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_sw_flow_ctrl(static_cast<hf_uart_port_native_t>(port_), 
                                        enable, xon_threshold, xoff_threshold);
  if (err == ESP_OK) {
    software_flow_enabled_ = enable;
    flow_config_.enable_sw_flow_control = enable;
    flow_config_.rx_flow_ctrl_thresh = xoff_threshold;
    flow_config_.tx_flow_ctrl_thresh = xon_threshold;
    return true;
  }
#else
  (void)enable;
  (void)xon_threshold;
  (void)xoff_threshold;
#endif
  return false;
}

bool McuUart::ConfigureWakeup(const UartWakeupConfig &wakeup_config) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (wakeup_config.enable_wakeup) {
    esp_err_t err = uart_set_wakeup_threshold(static_cast<hf_uart_port_native_t>(port_), 
                                              wakeup_config.wakeup_threshold);
    if (err == ESP_OK) {
      wakeup_enabled_ = true;
      wakeup_config_ = wakeup_config;
      return true;
    }
  } else {
    wakeup_enabled_ = false;
    wakeup_config_ = wakeup_config;
    return true;
  }
#else
  (void)wakeup_config;
#endif
  return false;
}

bool McuUart::ConfigurePowerManagement(const UartPowerConfig &power_config) noexcept {
  power_config_ = power_config;
  // Power management configuration is typically set during initialization
  // For runtime changes, a reinitialization might be required
  return true;
}

bool McuUart::SetRxFullThreshold(uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_rx_full_threshold(static_cast<hf_uart_port_native_t>(port_), threshold);
  return err == ESP_OK;
#else
  (void)threshold;
  return false;
#endif
}

bool McuUart::SetTxEmptyThreshold(uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_tx_empty_threshold(static_cast<hf_uart_port_native_t>(port_), threshold);
  return err == ESP_OK;
#else
  (void)threshold;
  return false;
#endif
}

bool McuUart::SetRxTimeoutThreshold(uint8_t timeout_threshold) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_rx_timeout(static_cast<hf_uart_port_native_t>(port_), timeout_threshold);
  return err == ESP_OK;
#else
  (void)timeout_threshold;
  return false;
#endif
}

bool McuUart::EnableRxInterrupts(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err;
  if (enable) {
    err = uart_enable_rx_intr(static_cast<hf_uart_port_native_t>(port_));
  } else {
    err = uart_disable_rx_intr(static_cast<hf_uart_port_native_t>(port_));
  }
  return err == ESP_OK;
#else
  (void)enable;
  return false;
#endif
}

bool McuUart::EnableTxInterrupts(bool enable, uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err;
  if (enable) {
    err = uart_enable_tx_intr(static_cast<hf_uart_port_native_t>(port_), 1, threshold);
  } else {
    err = uart_disable_tx_intr(static_cast<hf_uart_port_native_t>(port_));
  }
  return err == ESP_OK;
#else
  (void)enable;
  (void)threshold;
  return false;
#endif
}

bool McuUart::SetSignalInversion(uint32_t inverse_mask) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_set_line_inverse(static_cast<hf_uart_port_native_t>(port_), inverse_mask);
  return err == ESP_OK;
#else
  (void)inverse_mask;
  return false;
#endif
}

UartStatistics McuUart::GetStatistics() const noexcept {
  UartStatistics stats = statistics_;
  stats.tx_byte_count = bytes_transmitted_;
  stats.rx_byte_count = bytes_received_;
  stats.last_activity_timestamp = GetCurrentTimeMs() * 1000; // Convert to microseconds
  return stats;
}

//==============================================//
// PRIVATE METHODS                              //
//==============================================//

HfUartErr McuUart::ConvertPlatformError(int32_t platform_error) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  switch (platform_error) {
  case ESP_OK:
    return HfUartErr::UART_SUCCESS;
  case ESP_ERR_INVALID_ARG:
    return HfUartErr::UART_ERR_INVALID_PARAMETER;
  case ESP_ERR_TIMEOUT:
    return HfUartErr::UART_ERR_TIMEOUT;
  case ESP_ERR_NO_MEM:
    return HfUartErr::UART_ERR_OUT_OF_MEMORY;
  case ESP_ERR_INVALID_STATE:
    return HfUartErr::UART_ERR_NOT_INITIALIZED;
  case ESP_FAIL:
    return HfUartErr::UART_ERR_FAILURE;
  default:
    return HfUartErr::UART_ERR_COMMUNICATION_FAILURE;
  }
#else
  (void)platform_error;
  return HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
#endif
}

bool McuUart::PlatformInitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // Validate configuration
  if (config_.baud_rate < 1200 || config_.baud_rate > 5000000) {
    last_error_ = HfUartErr::UART_ERR_INVALID_BAUD_RATE;
    return false;
  }

  if (config_.data_bits < 5 || config_.data_bits > 8) {
    last_error_ = HfUartErr::UART_ERR_INVALID_DATA_BITS;
    return false;
  }

  if (config_.parity > 2) {
    last_error_ = HfUartErr::UART_ERR_INVALID_PARITY;
    return false;
  }

  if (config_.stop_bits < 1 || config_.stop_bits > 2) {
    last_error_ = HfUartErr::UART_ERR_INVALID_STOP_BITS;
    return false;
  }

  if (config_.tx_pin == HF_INVALID_PIN || config_.rx_pin == HF_INVALID_PIN) {
    last_error_ = HfUartErr::UART_ERR_PIN_CONFIGURATION_ERROR;
    return false;
  }

  // Configure UART parameters
  hf_uart_config_native_t uart_config = {};
  uart_config.baud_rate = config_.baud_rate;
  uart_config.data_bits = static_cast<hf_uart_word_length_native_t>(config_.data_bits - 5);

  // Configure parity
  switch (config_.parity) {
  case 0:
    uart_config.parity = UART_PARITY_DISABLE;
    break;
  case 1:
    uart_config.parity = UART_PARITY_EVEN;
    break;
  case 2:
    uart_config.parity = UART_PARITY_ODD;
    break;
  default:
    uart_config.parity = UART_PARITY_DISABLE;
    break;
  }

  uart_config.stop_bits = (config_.stop_bits == 1) ? UART_STOP_BITS_1 : UART_STOP_BITS_2;
  uart_config.flow_ctrl =
      config_.use_hardware_flow_control ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
  uart_config.source_clk = UART_SCLK_DEFAULT;
  
  // Configure power management settings
  uart_config.allow_pd = power_config_.allow_pd_in_light_sleep;

  // Configure UART
  esp_err_t err = uart_param_config(static_cast<hf_uart_port_native_t>(port_), &uart_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Set pins
  err = uart_set_pin(static_cast<hf_uart_port_native_t>(port_), config_.tx_pin, config_.rx_pin,
                     (config_.use_hardware_flow_control) ? config_.rts_pin : HF_INVALID_PIN,
                     (config_.use_hardware_flow_control) ? config_.cts_pin : HF_INVALID_PIN);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Install UART driver
  err = uart_driver_install(static_cast<hf_uart_port_native_t>(port_), config_.rx_buffer_size,
                            config_.tx_buffer_size, 0, nullptr, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  initialized_ = true;
  last_error_ = HfUartErr::UART_SUCCESS;
  return true;
#else
  (void)port_;
  (void)config_;
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return false;
#endif
}

bool McuUart::PlatformDeinitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (!initialized_) {
    return true;
  }

  esp_err_t err = uart_driver_delete(static_cast<hf_uart_port_native_t>(port_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_driver_delete failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  initialized_ = false;
  last_error_ = HfUartErr::UART_SUCCESS;
  return true;
#else
  (void)port_;
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return false;
#endif
}

int McuUart::InternalPrintf(const char *format, va_list args) noexcept {
  // Format the string into our buffer
  int formatted_length = vsnprintf(printf_buffer_, PRINTF_BUFFER_SIZE, format, args);

  if (formatted_length < 0) {
    return -1; // Formatting error
  }

  if (formatted_length >= PRINTF_BUFFER_SIZE) {
    formatted_length = PRINTF_BUFFER_SIZE - 1; // Truncate
  }

  // Write the formatted string
  HfUartErr result = Write(reinterpret_cast<const uint8_t *>(printf_buffer_), formatted_length);

  return (result == HfUartErr::UART_SUCCESS) ? formatted_length : -1;
}

//==============================================//
// PRIVATE HELPER METHODS                      //
//==============================================//

uint32_t McuUart::GetCurrentTimeMs() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return static_cast<uint32_t>(esp_timer_get_time() / 1000);
#else
  // For other platforms, implement appropriate time function
  return 0;
#endif
}
