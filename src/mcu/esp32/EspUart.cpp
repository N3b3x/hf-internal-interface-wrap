/**
 * @file EspUart.cpp
 * @brief Implementation of ESP32 UART controller for the HardFOC system.
 *
 * This file provides the implementation for UART communication using the ESP32's
 * built-in UART peripheral. All platform-specific types and implementations are
 * isolated through EspTypes_UART.h. The implementation supports multiple ports,
 * configurable baud rates and data formats, hardware flow control, interrupt-driven
 * operation, pattern detection, and comprehensive error handling.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "EspUart.h"
#include <algorithm>

#ifdef HF_MCU_FAMILY_ESP32

// Platform-specific includes and definitions
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/uart_hal.h"
#include "soc/uart_reg.h"

static const char *TAG = "EspUart";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspUart::EspUart(const hf_uart_port_config_t &config) noexcept
    : BaseUart(), 
      port_config_(config), initialized_(false), uart_port_(static_cast<hf_uart_port_native_t>(config.port_number)),
      event_queue_(nullptr), event_task_handle_(nullptr), event_callback_(nullptr), pattern_callback_(nullptr),
      break_callback_(nullptr), event_callback_user_data_(nullptr), pattern_callback_user_data_(nullptr),
      break_callback_user_data_(nullptr), operating_mode_(config.operating_mode),
      communication_mode_(hf_uart_mode_t::HF_UART_MODE_UART), pattern_detection_enabled_(false),
      software_flow_enabled_(false), wakeup_enabled_(false), break_detected_(false), tx_in_progress_(false),
      last_error_(hf_uart_err_t::UART_SUCCESS) {
  
  // Initialize printf buffer
  memset(printf_buffer_, 0, sizeof(printf_buffer_));
  
  // Initialize statistics timestamp
  statistics_.initialization_timestamp = esp_timer_get_time();
  
  // Update base class config with port config values
  config_.baud_rate = config.baud_rate;
  config_.data_bits = static_cast<uint8_t>(config.data_bits);
  config_.parity = static_cast<uint8_t>(config.parity);
  config_.stop_bits = static_cast<uint8_t>(config.stop_bits);
  config_.use_hardware_flow_control = (config.flow_control != hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE);
  config_.tx_pin = config.tx_pin;
  config_.rx_pin = config.rx_pin;
  config_.rts_pin = config.rts_pin;
  config_.cts_pin = config.cts_pin;
  config_.tx_buffer_size = config.tx_buffer_size;
  config_.rx_buffer_size = config.rx_buffer_size;
  config_.timeout_ms = config.timeout_ms;
  
  ESP_LOGI(TAG, "EspUart constructed with port=%lu, baud=%lu Hz, mode=%d", 
           config.port_number, config.baud_rate, static_cast<int>(config.operating_mode));
}

EspUart::EspUart(hf_port_number_t port, hf_baud_rate_t baud_rate, hf_pin_num_t tx_pin, hf_pin_num_t rx_pin) noexcept
    : EspUart(hf_uart_port_config_t{
        .port_number = port,
        .baud_rate = baud_rate,
        .tx_pin = tx_pin,
        .rx_pin = rx_pin
      }) {
  // Legacy constructor delegates to main constructor
}

EspUart::~EspUart() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_.load()) {
    ESP_LOGI(TAG, "EspUart destructor - cleaning up");
    Deinitialize();
  }
}

//==============================================================================
// LIFECYCLE (BaseUart Interface)
//==============================================================================

bool EspUart::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_.load()) {
    ESP_LOGW(TAG, "UART already initialized");
    return true;
  }

  ESP_LOGI(TAG, "Initializing ESP32 UART system with port=%lu, baud=%lu Hz", 
           port_config_.port_number, port_config_.baud_rate);

  // Validate configuration
  hf_uart_err_t validation_result = ValidateConfiguration();
  if (validation_result != hf_uart_err_t::UART_SUCCESS) {
    last_error_ = validation_result;
    UpdateDiagnostics(validation_result);
    ESP_LOGE(TAG, "Configuration validation failed: %d", static_cast<int>(validation_result));
    return false;
  }

  // Platform-specific initialization
  hf_uart_err_t init_result = PlatformInitialize();
  if (init_result != hf_uart_err_t::UART_SUCCESS) {
    last_error_ = init_result;
    UpdateDiagnostics(init_result);
    ESP_LOGE(TAG, "Platform initialization failed: %d", static_cast<int>(init_result));
    return false;
  }

  initialized_.store(true);
  last_error_ = hf_uart_err_t::UART_SUCCESS;
  diagnostics_.is_initialized = true;
  diagnostics_.last_error = hf_uart_err_t::UART_SUCCESS;
  diagnostics_.last_error_timestamp = esp_timer_get_time();

  ESP_LOGI(TAG, "ESP32 UART system initialized successfully");
  return true;
}

bool EspUart::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing ESP32 UART system");

  hf_uart_err_t result = PlatformDeinitialize();
  if (result == hf_uart_err_t::UART_SUCCESS) {
    initialized_.store(false);
    last_error_ = hf_uart_err_t::UART_SUCCESS;
    diagnostics_.is_initialized = false;
    ESP_LOGI(TAG, "ESP32 UART system deinitialized");
  } else {
    ESP_LOGE(TAG, "Failed to deinitialize ESP32 UART system: %d", static_cast<int>(result));
    UpdateDiagnostics(result);
  }

  return (result == hf_uart_err_t::UART_SUCCESS);
}



//==============================================================================
// BASIC UART OPERATIONS (BaseUart Interface)
//==============================================================================

hf_uart_err_t EspUart::Write(const uint8_t* data, uint16_t length, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  if (!data && length > 0) {
    return hf_uart_err_t::UART_ERR_NULL_POINTER;
  }

  if (length == 0) {
    return hf_uart_err_t::UART_SUCCESS;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint64_t start_time_us = esp_timer_get_time();
  tx_in_progress_ = true;
  diagnostics_.is_transmitting = true;

  uint32_t timeout = GetTimeoutMs(timeout_ms);

  int bytes_written = uart_write_bytes(uart_port_, reinterpret_cast<const char*>(data), length);

  if (bytes_written >= 0) {
    // Wait for transmission to complete if timeout specified
    if (timeout > 0) {
      esp_err_t err = uart_wait_tx_done(uart_port_, pdMS_TO_TICKS(timeout));
      if (err != ESP_OK) {
        tx_in_progress_ = false;
        diagnostics_.is_transmitting = false;
        hf_uart_err_t result = hf_uart_err_t::UART_ERR_TIMEOUT;
        UpdateStatistics(result, start_time_us);
        UpdateDiagnostics(result);
        return result;
      }
    }

    statistics_.tx_byte_count += bytes_written;
    tx_in_progress_ = false;
    diagnostics_.is_transmitting = false;
    hf_uart_err_t result = hf_uart_err_t::UART_SUCCESS;
    UpdateStatistics(result, start_time_us);
    return result;
  } else {
    tx_in_progress_ = false;
    diagnostics_.is_transmitting = false;
    hf_uart_err_t result = ConvertPlatformError(bytes_written);
    UpdateStatistics(result, start_time_us);
    UpdateDiagnostics(result);
    return result;
  }
#else
  return hf_uart_err_t::UART_ERR_FAILURE;
#endif
}

hf_uart_err_t EspUart::Read(uint8_t* data, uint16_t length, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  if (!data && length > 0) {
    return hf_uart_err_t::UART_ERR_NULL_POINTER;
  }

  if (length == 0) {
    return hf_uart_err_t::UART_SUCCESS;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint64_t start_time_us = esp_timer_get_time();
  diagnostics_.is_receiving = true;

  uint32_t timeout = GetTimeoutMs(timeout_ms);

  int bytes_read = uart_read_bytes(uart_port_, data, length, pdMS_TO_TICKS(timeout));

  if (bytes_read >= 0) {
    statistics_.rx_byte_count += bytes_read;
    diagnostics_.is_receiving = false;
    hf_uart_err_t result = hf_uart_err_t::UART_SUCCESS;
    UpdateStatistics(result, start_time_us);
    return result;
  } else {
    diagnostics_.is_receiving = false;
    hf_uart_err_t result = ConvertPlatformError(bytes_read);
    UpdateStatistics(result, start_time_us);
    UpdateDiagnostics(result);
    return result;
  }
}

bool EspUart::WriteString(const char* str) noexcept {
  if (!str) {
    return false;
  }
  return Write(reinterpret_cast<const uint8_t*>(str), strlen(str)) == hf_uart_err_t::UART_SUCCESS;
}

bool EspUart::WriteByte(uint8_t byte) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  int result = uart_write_bytes(uart_port_, reinterpret_cast<const char*>(&byte), 1);
  if (result == 1) {
    statistics_.tx_byte_count++;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::FlushTx() noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t err = uart_flush(uart_port_);
  return (err == ESP_OK);
}

bool EspUart::FlushRx() noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t err = uart_flush_input(uart_port_);
  return (err == ESP_OK);
}

//==============================================================================
// CONFIGURATION (BaseUart Interface)
//==============================================================================

bool EspUart::SetBaudRate(uint32_t baud_rate) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_baudrate(uart_port_, baud_rate);
  if (result == ESP_OK) {
    port_config_.baud_rate = baud_rate;
    config_.baud_rate = baud_rate;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::SetFlowControl(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uart_hw_flowcontrol_t flow_ctrl = enable ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
  esp_err_t result = uart_set_hw_flow_ctrl(uart_port_, flow_ctrl, 122);
  if (result == ESP_OK) {
    port_config_.flow_control = enable ? hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_CTS_RTS : hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE;
    config_.use_hardware_flow_control = enable;
    diagnostics_.flow_control_active = enable;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::SetRTS(bool active) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_rts(uart_port_, active ? 1 : 0);
  if (result == ESP_OK) {
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::SendBreak(uint32_t duration_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_send_break(uart_port_);
  if (result == ESP_OK) {
    statistics_.break_count++;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::SetLoopback(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_loop_back(uart_port_, enable);
  if (result == ESP_OK) {
    port_config_.enable_loopback = enable;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

bool EspUart::WaitTransmitComplete(uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint32_t timeout = GetTimeoutMs(timeout_ms);
  esp_err_t result = uart_wait_tx_done(uart_port_, pdMS_TO_TICKS(timeout));
  if (result == ESP_OK) {
    tx_in_progress_ = false;
    diagnostics_.is_transmitting = false;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

//==============================================//
// ENHANCED METHODS                             //
//==============================================//

bool EspUart::IsTxBusy() noexcept {
  return tx_in_progress_;
}

uint16_t EspUart::BytesAvailable() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  size_t bytes_available = 0;
  esp_err_t err = uart_get_buffered_data_len(static_cast<hf_uart_port_native_t>(port_), &bytes_available);
  if (err == ESP_OK) {
    return static_cast<uint16_t>(bytes_available);
  }
  return 0;
}

bool EspUart::IsBreakDetected() noexcept {
  bool detected = break_detected_;
  break_detected_ = false; // Clear flag after reading
  return detected;
}

uint16_t EspUart::TxBytesWaiting() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  // ESP32 doesn't provide direct access to TX buffer level
  // Return 0 if not transmitting, or estimate based on recent writes
  return tx_in_progress_ ? 1 : 0;
}

uint16_t EspUart::ReadUntil(uint8_t *data, uint16_t max_length, uint8_t terminator,
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
        hf_uart_err_t result = Read(&byte, 1, 100); // Short timeout for each byte

    if (result == hf_uart_err_t::UART_SUCCESS) {
      data[bytes_read++] = byte;
      
      // Check if we found the terminator
      if (byte == terminator) {
        break;
      }
    } else if (result == hf_uart_err_t::UART_ERR_TIMEOUT) {
      // Continue trying if we haven't hit the overall timeout
      continue;
    } else {
      // Other error, stop reading
      break;
    }
  }

  return bytes_read;
}

uint16_t EspUart::ReadLine(char *buffer, uint16_t max_length, uint32_t timeout_ms) noexcept {
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
    hf_uart_err_t result = Read(&ch, 1, 100); // Short timeout for each character
    
    if (result == hf_uart_err_t::UART_SUCCESS) {
      // Handle line endings
      if (ch == '\r') {
        // CR - check for following LF
        uint8_t next_ch;
        hf_uart_err_t next_result = Read(&next_ch, 1, 10); // Very short timeout for LF
        if (next_result == hf_uart_err_t::UART_SUCCESS && next_ch == '\n') {
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
    } else if (result == hf_uart_err_t::UART_ERR_TIMEOUT) {
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

//==============================================================================
// ADVANCED UART FEATURES
//==============================================================================

hf_uart_err_t EspUart::SetOperatingMode(hf_uart_operating_mode_t mode) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (operating_mode_ == mode) {
    return hf_uart_err_t::UART_SUCCESS; // Already in requested mode
  }

  // Stop current mode if different
  if (operating_mode_ == hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    StopEventTask();
  }

  operating_mode_ = mode;
  port_config_.operating_mode = mode;

  // Start new mode if needed
  if (mode == hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    hf_uart_err_t result = StartEventTask();
    if (result != hf_uart_err_t::UART_SUCCESS) {
      return result;
    }
  }

  ESP_LOGI(TAG, "Operating mode changed to %d", static_cast<int>(mode));
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::SetBaudRate(hf_baud_rate_t baud_rate) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (baud_rate < MIN_BAUD_RATE || baud_rate > MAX_BAUD_RATE) {
    return hf_uart_err_t::UART_ERR_INVALID_BAUD_RATE;
  }

  esp_err_t result = uart_set_baudrate(uart_port_, baud_rate);
  if (result == ESP_OK) {
    port_config_.baud_rate = baud_rate;
    config_.baud_rate = baud_rate;
    ESP_LOGI(TAG, "Baud rate changed to %lu Hz", baud_rate);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetFlowControl(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uart_hw_flowcontrol_t flow_ctrl = enable ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
  esp_err_t result = uart_set_hw_flow_ctrl(uart_port_, flow_ctrl, 122);
  if (result == ESP_OK) {
    port_config_.flow_control = enable ? hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_CTS_RTS : hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE;
    config_.use_hardware_flow_control = enable;
    diagnostics_.flow_control_active = enable;
    ESP_LOGI(TAG, "Hardware flow control %s", enable ? "enabled" : "disabled");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetRTS(bool active) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_rts(uart_port_, active ? 1 : 0);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "RTS set to %s", active ? "active" : "inactive");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SendBreak(uint32_t duration_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  if (duration_ms < HF_UART_BREAK_MIN_DURATION || duration_ms > HF_UART_BREAK_MAX_DURATION) {
    return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_send_break(uart_port_);
  if (result == ESP_OK) {
    statistics_.break_count++;
    ESP_LOGD(TAG, "Break condition sent for %lu ms", duration_ms);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetLoopback(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_loop_back(uart_port_, enable);
  if (result == ESP_OK) {
    port_config_.enable_loopback = enable;
    ESP_LOGI(TAG, "Loopback mode %s", enable ? "enabled" : "disabled");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

bool EspUart::WaitTransmitComplete(uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint32_t timeout = GetTimeoutMs(timeout_ms);
  esp_err_t result = uart_wait_tx_done(uart_port_, pdMS_TO_TICKS(timeout));
  if (result == ESP_OK) {
    tx_in_progress_ = false;
    diagnostics_.is_transmitting = false;
    return true;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return false;
  }
}

uint16_t EspUart::ReadUntil(uint8_t* data, uint16_t max_length, uint8_t terminator, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  if (!data || max_length == 0) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint32_t timeout = GetTimeoutMs(timeout_ms);
  uint16_t bytes_read = 0;
  uint64_t start_time = esp_timer_get_time();

  while (bytes_read < max_length) {
    uint8_t byte;
    int result = uart_read_bytes(uart_port_, &byte, 1, pdMS_TO_TICKS(100));
    
    if (result == 1) {
      data[bytes_read++] = byte;
      if (byte == terminator) {
        break;
      }
    } else if (result == 0) {
      // Timeout on this read, check overall timeout
      uint64_t elapsed = esp_timer_get_time() - start_time;
      if (elapsed > (timeout * 1000)) {
        break;
      }
    } else {
      // Error occurred
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      break;
    }
  }

  statistics_.rx_byte_count += bytes_read;
  return bytes_read;
}

uint16_t EspUart::ReadLine(char* buffer, uint16_t max_length, uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  if (!buffer || max_length == 0) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uint32_t timeout = GetTimeoutMs(timeout_ms);
  uint16_t chars_read = 0;
  uint64_t start_time = esp_timer_get_time();

  while (chars_read < max_length - 1) { // Leave room for null terminator
    char ch;
    int result = uart_read_bytes(uart_port_, reinterpret_cast<uint8_t*>(&ch), 1, pdMS_TO_TICKS(100));
    
    if (result == 1) {
      if (ch == '\n' || ch == '\r') {
        break;
      }
      buffer[chars_read++] = ch;
    } else if (result == 0) {
      // Timeout on this read, check overall timeout
      uint64_t elapsed = esp_timer_get_time() - start_time;
      if (elapsed > (timeout * 1000)) {
        break;
      }
    } else {
      // Error occurred
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      break;
    }
  }

  buffer[chars_read] = '\0'; // Null terminate
  statistics_.rx_byte_count += chars_read;
  return chars_read;
}

hf_uart_err_t EspUart::SetCommunicationMode(hf_uart_mode_t mode) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (communication_mode_ == mode) {
    return hf_uart_err_t::UART_SUCCESS; // Already in requested mode
  }

  esp_err_t result = ESP_OK;

  switch (mode) {
    case hf_uart_mode_t::HF_UART_MODE_UART:
      // Default UART mode - no special configuration needed
      break;
      
    case hf_uart_mode_t::HF_UART_MODE_RS485_HALF_DUPLEX:
      result = uart_set_mode(uart_port_, UART_MODE_RS485_HALF_DUPLEX);
      break;
      
    case hf_uart_mode_t::HF_UART_MODE_IRDA:
      result = uart_set_mode(uart_port_, UART_MODE_IRDA);
      break;
      
    case hf_uart_mode_t::HF_UART_MODE_RS485_COLLISION_DETECT:
      result = uart_set_mode(uart_port_, UART_MODE_RS485_COLLISION_DETECT);
      break;
      
    case hf_uart_mode_t::HF_UART_MODE_RS485_APP_CTRL:
      result = uart_set_mode(uart_port_, UART_MODE_RS485_APP_CTRL);
      break;
      
    case hf_uart_mode_t::HF_UART_MODE_LOOPBACK:
      result = uart_set_mode(uart_port_, UART_MODE_LOOPBACK);
      break;
      
    default:
      return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
  }

  if (result == ESP_OK) {
    communication_mode_ = mode;
    ESP_LOGI(TAG, "Communication mode changed to %d", static_cast<int>(mode));
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::ConfigureRS485(const hf_uart_rs485_config_t& rs485_config) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Set RS485 mode first
  hf_uart_err_t mode_result = SetCommunicationMode(rs485_config.mode);
  if (mode_result != hf_uart_err_t::UART_SUCCESS) {
    return mode_result;
  }

  // Configure RS485 parameters
  uart_rs485_config_t uart_rs485_config = {};
  uart_rs485_config.rx_busy_tx_en = rs485_config.enable_echo_suppression;
  uart_rs485_config.rx_during_tx = !rs485_config.enable_echo_suppression;
  uart_rs485_config.loop_back = false;
  uart_rs485_config.tx_rx_en = rs485_config.auto_rts_control;
  uart_rs485_config.rts_level_for_tx = 1;
  uart_rs485_config.rts_level_for_rx = 0;

  esp_err_t result = uart_set_rs485_config(uart_port_, &uart_rs485_config);
  if (result == ESP_OK) {
    ESP_LOGI(TAG, "RS485 configuration applied");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::ConfigureIrDA(const hf_uart_irda_config_t& irda_config) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Set IrDA mode
  hf_uart_err_t mode_result = SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_IRDA);
  if (mode_result != hf_uart_err_t::UART_SUCCESS) {
    return mode_result;
  }

  // Configure IrDA parameters
  uart_irda_config_t uart_irda_config = {};
  uart_irda_config.rx_en = true;
  uart_irda_config.tx_en = true;
  uart_irda_config.invert_rx = irda_config.invert_rx;
  uart_irda_config.invert_tx = irda_config.invert_tx;
  uart_irda_config.tx_duty_cycle = irda_config.duty_cycle;

  esp_err_t result = uart_set_irda_config(uart_port_, &uart_irda_config);
  if (result == ESP_OK) {
    ESP_LOGI(TAG, "IrDA configuration applied");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

int EspUart::GetPatternPosition(bool pop_position) noexcept {
  if (!EnsureInitialized() || !pattern_detection_enabled_) {
    return -1;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  int pattern_pos = -1;
  esp_err_t result = uart_get_pattern_pos(uart_port_, &pattern_pos);
  
  if (result == ESP_OK && pattern_pos >= 0) {
    if (pop_position) {
      // Clear the pattern position
      uart_flush_input(uart_port_);
    }
    statistics_.pattern_detect_count++;
    return pattern_pos;
  }
  
  return -1;
}

hf_uart_err_t EspUart::ConfigureSoftwareFlowControl(bool enable, uint8_t xon_threshold, uint8_t xoff_threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (enable) {
    uart_sw_flowctrl_t sw_flow_ctrl = {};
    sw_flow_ctrl.enable = true;
    sw_flow_ctrl.xon_thrd = xon_threshold;
    sw_flow_ctrl.xoff_thrd = xoff_threshold;
    sw_flow_ctrl.xon_char = 0x11; // DC1
    sw_flow_ctrl.xoff_char = 0x13; // DC3

    esp_err_t result = uart_set_sw_flow_ctrl(uart_port_, &sw_flow_ctrl, true);
    if (result == ESP_OK) {
      software_flow_enabled_ = true;
      ESP_LOGI(TAG, "Software flow control enabled (XON: %d, XOFF: %d)", xon_threshold, xoff_threshold);
      return hf_uart_err_t::UART_SUCCESS;
    } else {
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      return error;
    }
  } else {
    esp_err_t result = uart_set_sw_flow_ctrl(uart_port_, nullptr, false);
    if (result == ESP_OK) {
      software_flow_enabled_ = false;
      ESP_LOGI(TAG, "Software flow control disabled");
      return hf_uart_err_t::UART_SUCCESS;
    } else {
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      return error;
    }
  }
}

hf_uart_err_t EspUart::ConfigureWakeup(const hf_uart_wakeup_config_t& wakeup_config) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (wakeup_config.enable_wakeup) {
    esp_err_t result = uart_enable_rx_wakeup(uart_port_, wakeup_config.wakeup_threshold);
    if (result == ESP_OK) {
      wakeup_enabled_ = true;
      ESP_LOGI(TAG, "UART wakeup enabled with threshold %d", wakeup_config.wakeup_threshold);
      return hf_uart_err_t::UART_SUCCESS;
    } else {
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      return error;
    }
  } else {
    esp_err_t result = uart_disable_rx_wakeup(uart_port_);
    if (result == ESP_OK) {
      wakeup_enabled_ = false;
      ESP_LOGI(TAG, "UART wakeup disabled");
      return hf_uart_err_t::UART_SUCCESS;
    } else {
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      return error;
    }
  }
}

hf_uart_err_t EspUart::SetRxFullThreshold(uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_rx_full_threshold(uart_port_, threshold);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "RX full threshold set to %d", threshold);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetTxEmptyThreshold(uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_tx_empty_threshold(uart_port_, threshold);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "TX empty threshold set to %d", threshold);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetRxTimeoutThreshold(uint8_t timeout_threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_rx_timeout(uart_port_, timeout_threshold);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "RX timeout threshold set to %d", timeout_threshold);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::EnableRxInterrupts(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_enable_rx_intr(uart_port_);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "RX interrupts %s", enable ? "enabled" : "disabled");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::EnableTxInterrupts(bool enable, uint8_t threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result;
  if (enable) {
    result = uart_enable_tx_intr(uart_port_, threshold, 10);
  } else {
    result = uart_disable_tx_intr(uart_port_);
  }

  if (result == ESP_OK) {
    ESP_LOGD(TAG, "TX interrupts %s", enable ? "enabled" : "disabled");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_err_t EspUart::SetSignalInversion(uint32_t inverse_mask) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t result = uart_set_line_inverse(uart_port_, inverse_mask);
  if (result == ESP_OK) {
    ESP_LOGD(TAG, "Signal inversion mask set to 0x%08lx", inverse_mask);
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    hf_uart_err_t error = ConvertPlatformError(result);
    UpdateDiagnostics(error);
    return error;
  }
}

hf_uart_statistics_t EspUart::GetStatistics() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  hf_uart_statistics_t stats = statistics_;
  stats.last_activity_timestamp = GetCurrentTimeUs();
  return stats;
}

//==============================================================================
// CALLBACKS AND EVENT HANDLING
//==============================================================================

hf_uart_err_t EspUart::SetEventCallback(hf_uart_event_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  event_callback_ = callback;
  event_callback_user_data_ = user_data;
  ESP_LOGD(TAG, "Event callback %s", callback ? "set" : "cleared");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::SetPatternCallback(hf_uart_pattern_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  pattern_callback_ = callback;
  pattern_callback_user_data_ = user_data;
  ESP_LOGD(TAG, "Pattern callback %s", callback ? "set" : "cleared");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::SetBreakCallback(hf_uart_break_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  break_callback_ = callback;
  break_callback_user_data_ = user_data;
  ESP_LOGD(TAG, "Break callback %s", callback ? "set" : "cleared");
  return hf_uart_err_t::UART_SUCCESS;
}

//==============================================================================
// STATUS AND INFORMATION
//==============================================================================

hf_uart_err_t EspUart::GetStatistics(hf_uart_statistics_t& statistics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics = statistics_;
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::GetDiagnostics(hf_uart_diagnostics_t& diagnostics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  diagnostics = diagnostics_;
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::GetLastError() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return last_error_;
}

const hf_uart_port_config_t& EspUart::GetPortConfig() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return port_config_;
}

hf_uart_operating_mode_t EspUart::GetOperatingMode() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return operating_mode_;
}

hf_uart_mode_t EspUart::GetCommunicationMode() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return communication_mode_;
}

bool EspUart::IsPatternDetectionEnabled() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return pattern_detection_enabled_;
}

bool EspUart::IsWakeupEnabled() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return wakeup_enabled_;
}

bool EspUart::IsTransmitting() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return tx_in_progress_;
}

bool EspUart::IsReceiving() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return diagnostics_.is_receiving;
}

//==============================================================================
// PRINTF SUPPORT
//==============================================================================

int EspUart::Printf(const char* format, ...) noexcept {
  if (!EnsureInitialized()) {
    return -1;
  }

  va_list args;
  va_start(args, format);
  int result = VPrintf(format, args);
  va_end(args);
  return result;
}

int EspUart::VPrintf(const char* format, va_list args) noexcept {
  if (!EnsureInitialized()) {
    return -1;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  return InternalPrintf(format, args);
}

//==============================================================================
// INTERNAL METHODS
//==============================================================================

hf_uart_err_t EspUart::ValidateConfiguration() const noexcept {
  // Validate port number
  if (port_config_.port_number >= MAX_PORTS) {
    return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
  }

  // Validate baud rate
  if (port_config_.baud_rate < MIN_BAUD_RATE || port_config_.baud_rate > MAX_BAUD_RATE) {
    return hf_uart_err_t::UART_ERR_INVALID_BAUD_RATE;
  }

  // Validate buffer sizes
  if (port_config_.tx_buffer_size > MAX_BUFFER_SIZE || port_config_.rx_buffer_size > MAX_BUFFER_SIZE) {
    return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
  }

  // Validate pins
  if (port_config_.tx_pin == HF_INVALID_PIN && port_config_.rx_pin == HF_INVALID_PIN) {
    return hf_uart_err_t::UART_ERR_PIN_CONFIGURATION_ERROR;
  }

  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::PlatformInitialize() noexcept {
  // Install driver
  hf_uart_err_t driver_result = InstallDriver();
  if (driver_result != hf_uart_err_t::UART_SUCCESS) {
    return driver_result;
  }

  // Configure UART parameters
  hf_uart_err_t config_result = ConfigureUart();
  if (config_result != hf_uart_err_t::UART_SUCCESS) {
    UninstallDriver();
    return config_result;
  }

  // Configure pins
  hf_uart_err_t pin_result = ConfigurePins();
  if (pin_result != hf_uart_err_t::UART_SUCCESS) {
    UninstallDriver();
    return pin_result;
  }

  // Start event task if in interrupt mode
  if (operating_mode_ == hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    hf_uart_err_t task_result = StartEventTask();
    if (task_result != hf_uart_err_t::UART_SUCCESS) {
      UninstallDriver();
      return task_result;
    }
  }

  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::PlatformDeinitialize() noexcept {
  // Stop event task if running
  if (operating_mode_ == hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    StopEventTask();
  }

  // Uninstall driver
  return UninstallDriver();
}

hf_uart_err_t EspUart::InstallDriver() noexcept {
  uart_config_t uart_config = {};
  uart_config.baud_rate = port_config_.baud_rate;
  uart_config.data_bits = static_cast<uart_word_length_t>(port_config_.data_bits);
  uart_config.parity = static_cast<uart_parity_t>(port_config_.parity);
  uart_config.stop_bits = static_cast<uart_stop_bits_t>(port_config_.stop_bits);
  uart_config.flow_ctrl = static_cast<uart_hw_flowcontrol_t>(port_config_.flow_control);
  uart_config.source_clk = UART_SCLK_APB;

  esp_err_t result = uart_driver_install(uart_port_, port_config_.rx_buffer_size,
                                        port_config_.tx_buffer_size, port_config_.event_queue_size,
                                        &event_queue_, 0);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(result));
    return ConvertPlatformError(result);
  }

  result = uart_param_config(uart_port_, &uart_config);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure UART parameters: %s", esp_err_to_name(result));
    uart_driver_delete(uart_port_);
    return ConvertPlatformError(result);
  }

  ESP_LOGI(TAG, "UART driver installed successfully");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::UninstallDriver() noexcept {
  esp_err_t result = uart_driver_delete(uart_port_);
  if (result == ESP_OK) {
    event_queue_ = nullptr;
    ESP_LOGI(TAG, "UART driver uninstalled successfully");
    return hf_uart_err_t::UART_SUCCESS;
  } else {
    ESP_LOGE(TAG, "Failed to uninstall UART driver: %s", esp_err_to_name(result));
    return ConvertPlatformError(result);
  }
}

hf_uart_err_t EspUart::ConfigureUart() noexcept {
  // UART parameters are configured during driver installation
  // Additional configuration can be added here if needed
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::ConfigurePins() noexcept {
  esp_err_t result = uart_set_pin(uart_port_, port_config_.tx_pin, port_config_.rx_pin,
                                 port_config_.rts_pin, port_config_.cts_pin);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure UART pins: %s", esp_err_to_name(result));
    return ConvertPlatformError(result);
  }

  ESP_LOGI(TAG, "UART pins configured: TX=%d, RX=%d, RTS=%d, CTS=%d",
           port_config_.tx_pin, port_config_.rx_pin, port_config_.rts_pin, port_config_.cts_pin);
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::StartEventTask() noexcept {
  if (event_task_handle_ != nullptr) {
    return hf_uart_err_t::UART_SUCCESS; // Already running
  }

  BaseType_t result = xTaskCreate(EventTask, "UART_Event", 2048, this, 5, &event_task_handle_);
  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create UART event task");
    return hf_uart_err_t::UART_ERR_OUT_OF_MEMORY;
  }

  ESP_LOGI(TAG, "UART event task started");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::StopEventTask() noexcept {
  if (event_task_handle_ == nullptr) {
    return hf_uart_err_t::UART_SUCCESS; // Not running
  }

  vTaskDelete(event_task_handle_);
  event_task_handle_ = nullptr;
  ESP_LOGI(TAG, "UART event task stopped");
  return hf_uart_err_t::UART_SUCCESS;
}

void EspUart::EventTask(void* arg) noexcept {
  auto* uart = static_cast<EspUart*>(arg);
  if (!uart) {
    return;
  }

  uart_event_t event;
  while (true) {
    if (xQueueReceive(uart->event_queue_, &event, portMAX_DELAY)) {
      uart->HandleUartEvent(&event);
    }
  }
}

void EspUart::HandleUartEvent(const hf_uart_event_native_t* event) noexcept {
  if (!event) {
    return;
  }

  switch (event->type) {
    case UART_DATA:
      statistics_.rx_byte_count += event->size;
      diagnostics_.is_receiving = true;
      break;

    case UART_FIFO_OVF:
      statistics_.overrun_error_count++;
      UpdateDiagnostics(hf_uart_err_t::UART_ERR_OVERRUN_ERROR);
      break;

    case UART_BUFFER_FULL:
      statistics_.rx_error_count++;
      UpdateDiagnostics(hf_uart_err_t::UART_ERR_BUFFER_FULL);
      break;

    case UART_BREAK:
      statistics_.break_count++;
      break_detected_ = true;
      if (break_callback_) {
        break_callback_(0, break_callback_user_data_);
      }
      break;

    case UART_PARITY_ERR:
      statistics_.parity_error_count++;
      UpdateDiagnostics(hf_uart_err_t::UART_ERR_PARITY_ERROR);
      break;

    case UART_FRAME_ERR:
      statistics_.frame_error_count++;
      UpdateDiagnostics(hf_uart_err_t::UART_ERR_FRAME_ERROR);
      break;

    case UART_PATTERN_DET:
      statistics_.pattern_detect_count++;
      if (pattern_callback_) {
        pattern_callback_(event->size, pattern_callback_user_data_);
      }
      break;

    case UART_WAKEUP:
      statistics_.wakeup_count++;
      if (wakeup_enabled_) {
        ESP_LOGI(TAG, "UART wakeup detected");
      }
      break;

    default:
      ESP_LOGW(TAG, "Unknown UART event: %d", event->type);
      break;
  }

  // Call user event callback if set
  if (event_callback_) {
    event_callback_(event, event_callback_user_data_);
  }
}

hf_uart_err_t EspUart::ConvertPlatformError(int32_t platform_error) noexcept {
  switch (platform_error) {
    case ESP_OK:
      return hf_uart_err_t::UART_SUCCESS;
    case ESP_ERR_INVALID_ARG:
      return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
    case ESP_ERR_NO_MEM:
      return hf_uart_err_t::UART_ERR_OUT_OF_MEMORY;
    case ESP_ERR_TIMEOUT:
      return hf_uart_err_t::UART_ERR_TIMEOUT;
    case ESP_ERR_NOT_FOUND:
      return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
    case ESP_ERR_NOT_SUPPORTED:
      return hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION;
    case ESP_ERR_INVALID_STATE:
      return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
    default:
      return hf_uart_err_t::UART_ERR_FAILURE;
  }
}

hf_uart_err_t EspUart::UpdateStatistics(hf_uart_err_t result, uint64_t start_time_us) noexcept {
  uint64_t end_time_us = esp_timer_get_time();
  statistics_.last_activity_timestamp = end_time_us;

  if (result != hf_uart_err_t::UART_SUCCESS) {
    statistics_.tx_error_count++;
    statistics_.timeout_count++;
  }

  return result;
}

void EspUart::UpdateDiagnostics(hf_uart_err_t error) noexcept {
  diagnostics_.last_error = error;
  diagnostics_.last_error_timestamp = esp_timer_get_time();

  if (error != hf_uart_err_t::UART_SUCCESS) {
    diagnostics_.consecutive_errors++;
  } else {
    diagnostics_.consecutive_errors = 0;
    diagnostics_.error_reset_count++;
  }
}

uint32_t EspUart::GetTimeoutMs(uint32_t timeout_ms) const noexcept {
  if (timeout_ms == 0) {
    return port_config_.timeout_ms;
  }
  return timeout_ms;
}

int EspUart::InternalPrintf(const char* format, va_list args) noexcept {
  int len = vsnprintf(printf_buffer_, sizeof(printf_buffer_), format, args);
  if (len > 0 && len < static_cast<int>(sizeof(printf_buffer_))) {
    int written = uart_write_bytes(uart_port_, printf_buffer_, len);
    if (written > 0) {
      statistics_.tx_byte_count += written;
      return written;
    }
  }
  return -1;
}

bool IRAM_ATTR EspUart::PatternCallbackWrapper(int pattern_pos, void* user_data) noexcept {
  auto* uart = static_cast<EspUart*>(user_data);
  if (uart && uart->pattern_callback_) {
    return uart->pattern_callback_(pattern_pos, uart->pattern_callback_user_data_);
  }
  return false;
}

bool IRAM_ATTR EspUart::BreakCallbackWrapper(uint32_t break_duration, void* user_data) noexcept {
  auto* uart = static_cast<EspUart*>(user_data);
  if (uart && uart->break_callback_) {
    return uart->break_callback_(break_duration, uart->break_callback_user_data_);
  }
  return false;
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

bool IsValidUartPort(hf_port_number_t port_number) noexcept {
  return port_number < HF_ESP32_UART_MAX_PORTS;
}

bool GetDefaultUartPins(hf_port_number_t port_number, hf_pin_num_t& tx_pin, hf_pin_num_t& rx_pin,
                       hf_pin_num_t& rts_pin, hf_pin_num_t& cts_pin) noexcept {
  // ESP32-C6 Pin Mappings
  #if defined(HF_MCU_ESP32C6)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32c6_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c6_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c6_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c6_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32c6_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c6_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c6_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c6_t::UART1_CTS_PIN;
      return true;
    case 2:
      tx_pin = hf_uart_pin_map_esp32c6_t::UART2_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c6_t::UART2_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c6_t::UART2_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c6_t::UART2_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32 Classic Pin Mappings
  #elif defined(HF_MCU_ESP32)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32_t::UART1_CTS_PIN;
      return true;
    case 2:
      tx_pin = hf_uart_pin_map_esp32_t::UART2_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32_t::UART2_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32_t::UART2_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32_t::UART2_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32-S2 Pin Mappings
  #elif defined(HF_MCU_ESP32S2)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32s2_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s2_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s2_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s2_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32s2_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s2_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s2_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s2_t::UART1_CTS_PIN;
      return true;
    case 2:
      tx_pin = hf_uart_pin_map_esp32s2_t::UART2_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s2_t::UART2_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s2_t::UART2_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s2_t::UART2_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32-S3 Pin Mappings
  #elif defined(HF_MCU_ESP32S3)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32s3_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s3_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s3_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s3_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32s3_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s3_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s3_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s3_t::UART1_CTS_PIN;
      return true;
    case 2:
      tx_pin = hf_uart_pin_map_esp32s3_t::UART2_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32s3_t::UART2_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32s3_t::UART2_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32s3_t::UART2_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32-C3 Pin Mappings
  #elif defined(HF_MCU_ESP32C3)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32c3_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c3_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c3_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c3_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32c3_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c3_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c3_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c3_t::UART1_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32-C2 Pin Mappings
  #elif defined(HF_MCU_ESP32C2)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32c2_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c2_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c2_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c2_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32c2_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32c2_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32c2_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32c2_t::UART1_CTS_PIN;
      return true;
    default:
      return false;
  }

  // ESP32-H2 Pin Mappings
  #elif defined(HF_MCU_ESP32H2)
  switch (port_number) {
    case 0:
      tx_pin = hf_uart_pin_map_esp32h2_t::UART0_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32h2_t::UART0_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32h2_t::UART0_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32h2_t::UART0_CTS_PIN;
      return true;
    case 1:
      tx_pin = hf_uart_pin_map_esp32h2_t::UART1_TX_PIN;
      rx_pin = hf_uart_pin_map_esp32h2_t::UART1_RX_PIN;
      rts_pin = hf_uart_pin_map_esp32h2_t::UART1_RTS_PIN;
      cts_pin = hf_uart_pin_map_esp32h2_t::UART1_CTS_PIN;
      return true;
    default:
      return false;
  }

  #else
  return false;
  #endif
}

#endif // HF_MCU_FAMILY_ESP32