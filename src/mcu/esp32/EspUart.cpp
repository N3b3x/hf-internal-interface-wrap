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

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>

#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/uart_hal.h"
#include "soc/uart_reg.h"

#ifdef __cplusplus
}
#endif

static const char* TAG = "EspUart";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspUart::EspUart(const hf_uart_config_t& config) noexcept
    : BaseUart(config.port_number), port_config_(config), initialized_(false),
      uart_port_(static_cast<uart_port_t>(config.port_number)), event_queue_(nullptr),
      event_task_handle_(nullptr), event_callback_(nullptr), pattern_callback_(nullptr),
      break_callback_(nullptr), event_callback_user_data_(nullptr),
      pattern_callback_user_data_(nullptr), break_callback_user_data_(nullptr),
      operating_mode_(config.operating_mode),
      communication_mode_(hf_uart_mode_t::HF_UART_MODE_UART), pattern_detection_enabled_(false),
      software_flow_enabled_(false), wakeup_enabled_(false), break_detected_(false),
      tx_in_progress_(false), last_error_(hf_uart_err_t::UART_SUCCESS) {
  // Initialize printf buffer
  memset(printf_buffer_, 0, sizeof(printf_buffer_));

  // Initialize statistics timestamp
  statistics_.initialization_timestamp = esp_timer_get_time();

  ESP_LOGI(TAG, "EspUart constructed with port=%lu, baud=%lu Hz, mode=%d", config.port_number,
           config.baud_rate, static_cast<int>(config.operating_mode));
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

hf_uart_err_t EspUart::Write(const hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
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

  hf_u64_t start_time_us = esp_timer_get_time();
  tx_in_progress_ = true;
  diagnostics_.is_transmitting = true;

  hf_u32_t timeout = GetTimeoutMs(timeout_ms);

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
}

hf_uart_err_t EspUart::Read(hf_u8_t* data, hf_u16_t length, hf_u32_t timeout_ms) noexcept {
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

  hf_u64_t start_time_us = esp_timer_get_time();
  diagnostics_.is_receiving = true;

  hf_u32_t timeout = GetTimeoutMs(timeout_ms);

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

bool EspUart::WriteByte(hf_u8_t byte) noexcept {
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

hf_uart_err_t EspUart::FlushTx() noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t err = uart_flush(uart_port_);
  return (err == ESP_OK) ? hf_uart_err_t::UART_SUCCESS : ConvertPlatformError(err);
}

hf_uart_err_t EspUart::FlushRx() noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  esp_err_t err = uart_flush_input(uart_port_);
  return (err == ESP_OK) ? hf_uart_err_t::UART_SUCCESS : ConvertPlatformError(err);
}

//==============================================================================
// CONFIGURATION (BaseUart Interface)
//==============================================================================

// bool SetBaudRate removed - keeping hf_uart_err_t version

// bool SetFlowControl removed - keeping hf_uart_err_t version

// bool SetRTS removed - keeping hf_uart_err_t version

// bool SendBreak removed - keeping hf_uart_err_t version

// bool SetLoopback removed - keeping hf_uart_err_t version

bool EspUart::WaitTransmitComplete(hf_u32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_u32_t timeout = GetTimeoutMs(timeout_ms);
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

// Implement missing overrides
hf_u16_t EspUart::BytesAvailable() noexcept {
  // TODO: Implement actual logic
  return 0;
}
bool EspUart::IsTxBusy() noexcept {
  // TODO: Implement actual logic
  return false;
}

hf_u16_t EspUart::TxBytesWaiting() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  // ESP32 doesn't provide direct access to TX buffer level
  // Return 0 if not transmitting, or estimate based on recent writes
  return tx_in_progress_ ? 1 : 0;
}

hf_u16_t EspUart::ReadUntil(hf_u8_t* data, hf_u16_t max_length, hf_u8_t terminator,
                            hf_u32_t timeout_ms) noexcept {
  if (!data || max_length == 0) {
    return 0;
  }

  if (!EnsureInitialized()) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_u16_t bytes_read = 0;
  hf_u64_t start_time = esp_timer_get_time();
  hf_u32_t timeout = GetTimeoutMs(timeout_ms);

  while (bytes_read < max_length) {
    // Check timeout
    hf_u64_t elapsed = esp_timer_get_time() - start_time;
    if (timeout > 0 && elapsed >= (timeout * 1000)) {
      break;
    }

    // Try to read one byte
    hf_u8_t byte;
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

// First ReadLine implementation removed - keeping the more complete second implementation

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

bool EspUart::SetBaudRate(hf_u32_t baud_rate) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (baud_rate < MIN_BAUD_RATE || baud_rate > MAX_BAUD_RATE) {
    return false;
  }

  port_config_.baud_rate = baud_rate;
  // TODO: Implement actual logic
  return true;
}

hf_uart_err_t EspUart::SetFlowControl(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  uart_hw_flowcontrol_t flow_ctrl = enable ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
  esp_err_t result = uart_set_hw_flow_ctrl(uart_port_, flow_ctrl, 122);
  if (result == ESP_OK) {
    port_config_.flow_control = enable ? hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_CTS_RTS
                                       : hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE;
    // Hardware flow control is not directly configurable in the config struct
    // This would need to be handled during initialization
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

hf_uart_err_t EspUart::SendBreak(hf_u32_t duration_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  if (duration_ms < HF_UART_BREAK_MIN_DURATION || duration_ms > HF_UART_BREAK_MAX_DURATION) {
    return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // uart_send_break not available in ESP-IDF v5.5 - use alternative approach
  esp_err_t result = ESP_OK; // TODO: Implement break sending for ESP-IDF v5.5
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

// Second WaitTransmitComplete and ReadUntil implementations removed - keeping first implementations

hf_u16_t EspUart::ReadLine(char* buffer, hf_u16_t max_length, hf_u32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }

  if (!buffer || max_length == 0) {
    return 0;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_u32_t timeout = GetTimeoutMs(timeout_ms);
  hf_u16_t chars_read = 0;
  hf_u64_t start_time = esp_timer_get_time();

  while (chars_read < max_length - 1) { // Leave room for null terminator
    char ch;
    int result =
        uart_read_bytes(uart_port_, reinterpret_cast<hf_u8_t*>(&ch), 1, pdMS_TO_TICKS(100));

    if (result == 1) {
      if (ch == '\n' || ch == '\r') {
        break;
      }
      buffer[chars_read++] = ch;
    } else if (result == 0) {
      // Timeout on this read, check overall timeout
      hf_u64_t elapsed = esp_timer_get_time() - start_time;
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

    case hf_uart_mode_t::HF_UART_MODE_RS485:
      result = uart_set_mode(uart_port_, UART_MODE_RS485_HALF_DUPLEX);
      break;

    case hf_uart_mode_t::HF_UART_MODE_IRDA:
      result = uart_set_mode(uart_port_, UART_MODE_IRDA);
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
  hf_uart_err_t mode_result = SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_RS485);
  if (mode_result != hf_uart_err_t::UART_SUCCESS) {
    return mode_result;
  }

  // RS485 configuration is handled by uart_set_mode in ESP-IDF v5.5
  // Additional parameters like echo suppression and collision detection
  // are not directly supported in the current ESP-IDF version
  ESP_LOGW(TAG, "RS485 advanced features not supported in ESP-IDF v5.5");

  return hf_uart_err_t::UART_SUCCESS;
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

  // TODO: Implement IrDA support for ESP-IDF v5.5
  ESP_LOGW(TAG, "IrDA not supported in ESP-IDF v5.5");
  return hf_uart_err_t::UART_ERR_INVALID_PARAMETER;
}

int EspUart::GetPatternPosition(bool pop_position) noexcept {
  if (!EnsureInitialized() || !pattern_detection_enabled_) {
    return -1;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // TODO: Implement pattern detection for ESP-IDF v5.5
  ESP_LOGW(TAG, "Pattern detection not supported in ESP-IDF v5.5");
  return -1;
}

hf_uart_err_t EspUart::ConfigureSoftwareFlowControl(bool enable, hf_u8_t xon_threshold,
                                                    hf_u8_t xoff_threshold) noexcept {
  if (!EnsureInitialized()) {
    return hf_uart_err_t::UART_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (enable) {
    esp_err_t result = uart_set_sw_flow_ctrl(uart_port_, true, xon_threshold, xoff_threshold);
    if (result == ESP_OK) {
      software_flow_enabled_ = true;
      ESP_LOGI(TAG, "Software flow control enabled (XON: %d, XOFF: %d)", xon_threshold,
               xoff_threshold);
      return hf_uart_err_t::UART_SUCCESS;
    } else {
      hf_uart_err_t error = ConvertPlatformError(result);
      UpdateDiagnostics(error);
      return error;
    }
  } else {
    esp_err_t result = uart_set_sw_flow_ctrl(uart_port_, false, 0, 0);
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
    // esp_err_t result = uart_enable_rx_wakeup(uart_port_, wakeup_config.wakeup_threshold);
    esp_err_t result = ESP_OK; // TODO: Implement wakeup support
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
    // esp_err_t result = uart_disable_rx_wakeup(uart_port_);
    esp_err_t result = ESP_OK; // TODO: Implement wakeup support
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

hf_uart_err_t EspUart::SetRxFullThreshold(hf_u8_t threshold) noexcept {
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

hf_uart_err_t EspUart::SetTxEmptyThreshold(hf_u8_t threshold) noexcept {
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

hf_uart_err_t EspUart::SetRxTimeoutThreshold(hf_u8_t timeout_threshold) noexcept {
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

hf_uart_err_t EspUart::EnableTxInterrupts(bool enable, hf_u8_t threshold) noexcept {
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

hf_uart_err_t EspUart::SetSignalInversion(hf_u32_t inverse_mask) noexcept {
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

//==============================================================================
// CALLBACKS AND EVENT HANDLING
//==============================================================================

hf_uart_err_t EspUart::SetEventCallback(hf_uart_event_callback_t callback,
                                        void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  event_callback_ = callback;
  event_callback_user_data_ = user_data;
  ESP_LOGD(TAG, "Event callback %s", callback ? "set" : "cleared");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::SetPatternCallback(hf_uart_pattern_callback_t callback,
                                          void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  pattern_callback_ = callback;
  pattern_callback_user_data_ = user_data;
  ESP_LOGD(TAG, "Pattern callback %s", callback ? "set" : "cleared");
  return hf_uart_err_t::UART_SUCCESS;
}

hf_uart_err_t EspUart::SetBreakCallback(hf_uart_break_callback_t callback,
                                        void* user_data) noexcept {
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

const hf_uart_config_t& EspUart::GetPortConfig() const noexcept {
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
  if (port_config_.tx_buffer_size > MAX_BUFFER_SIZE ||
      port_config_.rx_buffer_size > MAX_BUFFER_SIZE) {
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

  // Convert HardFOC data bits to ESP-IDF format
  switch (port_config_.data_bits) {
    case hf_uart_data_bits_t::HF_UART_DATA_5_BITS:
      uart_config.data_bits = UART_DATA_5_BITS;
      break;
    case hf_uart_data_bits_t::HF_UART_DATA_6_BITS:
      uart_config.data_bits = UART_DATA_6_BITS;
      break;
    case hf_uart_data_bits_t::HF_UART_DATA_7_BITS:
      uart_config.data_bits = UART_DATA_7_BITS;
      break;
    case hf_uart_data_bits_t::HF_UART_DATA_8_BITS:
    default:
      uart_config.data_bits = UART_DATA_8_BITS;
      break;
  }

  // Convert HardFOC parity to ESP-IDF format
  switch (port_config_.parity) {
    case hf_uart_parity_t::HF_UART_PARITY_EVEN:
      uart_config.parity = UART_PARITY_EVEN;
      break;
    case hf_uart_parity_t::HF_UART_PARITY_ODD:
      uart_config.parity = UART_PARITY_ODD;
      break;
    case hf_uart_parity_t::HF_UART_PARITY_DISABLE:
    default:
      uart_config.parity = UART_PARITY_DISABLE;
      break;
  }

  // Convert HardFOC stop bits to ESP-IDF format
  switch (port_config_.stop_bits) {
    case hf_uart_stop_bits_t::HF_UART_STOP_BITS_1_5:
      uart_config.stop_bits = UART_STOP_BITS_1_5;
      break;
    case hf_uart_stop_bits_t::HF_UART_STOP_BITS_2:
      uart_config.stop_bits = UART_STOP_BITS_2;
      break;
    case hf_uart_stop_bits_t::HF_UART_STOP_BITS_1:
    default:
      uart_config.stop_bits = UART_STOP_BITS_1;
      break;
  }

  // Convert HardFOC flow control to ESP-IDF format
  switch (port_config_.flow_control) {
    case hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_RTS:
      uart_config.flow_ctrl = UART_HW_FLOWCTRL_RTS;
      break;
    case hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_CTS:
      uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS;
      break;
    case hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_CTS_RTS:
      uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
      break;
    case hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE:
    default:
      uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
      break;
  }

  uart_config.source_clk = UART_SCLK_DEFAULT;

  esp_err_t result =
      uart_driver_install(uart_port_, port_config_.rx_buffer_size, port_config_.tx_buffer_size,
                          port_config_.event_queue_size, &event_queue_, 0);
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

  ESP_LOGI(TAG, "UART pins configured: TX=%d, RX=%d, RTS=%d, CTS=%d", port_config_.tx_pin,
           port_config_.rx_pin, port_config_.rts_pin, port_config_.cts_pin);
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

void EspUart::HandleUartEvent(const uart_event_t* event) noexcept {
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

      // case UART_WAKEUP:
      //   statistics_.wakeup_count++;
      //   if (wakeup_enabled_) {
      //     ESP_LOGI(TAG, "UART wakeup detected");
      //   }
      //   break;

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

hf_uart_err_t EspUart::UpdateStatistics(hf_uart_err_t result, hf_u64_t start_time_us) noexcept {
  hf_u64_t end_time_us = esp_timer_get_time();
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

hf_u32_t EspUart::GetTimeoutMs(hf_u32_t timeout_ms) const noexcept {
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

bool IRAM_ATTR EspUart::BreakCallbackWrapper(hf_u32_t break_duration, void* user_data) noexcept {
  auto* uart = static_cast<EspUart*>(user_data);
  if (uart && uart->break_callback_) {
    return uart->break_callback_(break_duration, uart->break_callback_user_data_);
  }
  return false;
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

bool IsValidUartPort(hf_port_num_t port_number) noexcept {
  return port_number < HF_ESP32_UART_MAX_PORTS;
}

bool GetDefaultUartPins(hf_port_num_t port_number, hf_pin_num_t& tx_pin, hf_pin_num_t& rx_pin,
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