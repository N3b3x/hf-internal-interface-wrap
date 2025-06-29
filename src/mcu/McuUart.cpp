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

static const char *TAG = "McuUart";
#endif

//==============================================//
// CONSTRUCTOR & DESTRUCTOR                     //
//==============================================//

McuUart::McuUart(HfPortNumber port, const UartConfig &config) noexcept
    : BaseUart(port, config), platform_handle_(nullptr), last_error_(HfUartErr::UART_SUCCESS),
      bytes_transmitted_(0), bytes_received_(0), break_detected_(false), tx_in_progress_(false) {
  memset(printf_buffer_, 0, sizeof(printf_buffer_));
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
  int bytes_written = uart_write_bytes(static_cast<uart_port_t>(port_),
                                       reinterpret_cast<const char *>(data), length);

  if (bytes_written >= 0) {
    // Wait for transmission to complete if timeout specified
    if (timeout > 0) {
      esp_err_t err = uart_wait_tx_done(static_cast<uart_port_t>(port_), pdMS_TO_TICKS(timeout));
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
      uart_read_bytes(static_cast<uart_port_t>(port_), data, length, pdMS_TO_TICKS(timeout));

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
  esp_err_t err = uart_get_buffered_data_len(static_cast<uart_port_t>(port_), &bytes_available);
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
      uart_wait_tx_done(static_cast<uart_port_t>(port_), pdMS_TO_TICKS(config_.timeout_ms));
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
  esp_err_t err = uart_flush_input(static_cast<uart_port_t>(port_));
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
  esp_err_t err = uart_set_rts(static_cast<uart_port_t>(port_), active ? 1 : 0);
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
  esp_err_t err = uart_set_line_inverse(static_cast<uart_port_t>(port_), UART_SIGNAL_TXD_INV);
  if (err != ESP_OK) {
    return false;
  }

  // Hold break for specified duration
  vTaskDelay(pdMS_TO_TICKS(duration_ms));

  // Clear break condition
  err = uart_set_line_inverse(static_cast<uart_port_t>(port_), 0);
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
  esp_err_t err = uart_set_mode(static_cast<uart_port_t>(port_), mode);
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
  esp_err_t err = uart_wait_tx_done(static_cast<uart_port_t>(port_), pdMS_TO_TICKS(timeout_ms));
  return err == ESP_OK;
#else
  (void)timeout_ms;
  return false;
#endif
}

uint16_t McuUart::ReadUntil(uint8_t *data, uint16_t max_length, uint8_t terminator,
                            uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized() || !data || max_length == 0) {
    return 0;
  }

  uint16_t bytes_read = 0;
  uint32_t start_time = 0; // Platform-specific time implementation needed

  while (bytes_read < max_length) {
    uint8_t byte;
    if (Read(&byte, 1, 100) == HfUartErr::UART_SUCCESS) { // 100ms per byte timeout
      data[bytes_read++] = byte;
      if (byte == terminator) {
        break;
      }
    }

    // Check overall timeout
    // Platform-specific timeout check needed
    if (timeout_ms > 0) {
      // Simplified timeout check - should use platform timer
      break;
    }
  }

  return bytes_read;
}

uint16_t McuUart::ReadLine(char *buffer, uint16_t max_length, uint32_t timeout_ms) noexcept {
  if (!buffer || max_length == 0) {
    return 0;
  }

  uint16_t bytes_read =
      ReadUntil(reinterpret_cast<uint8_t *>(buffer), max_length - 1, '\n', timeout_ms);

  // Remove \r if present before \n
  if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
    bytes_read--;
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\r') {
      bytes_read--;
    }
  }

  buffer[bytes_read] = '\0'; // Null terminate
  return bytes_read;
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
  // Configure UART parameters
  uart_config_t uart_config = {};
  uart_config.baud_rate = config_.baud_rate;
  uart_config.data_bits = static_cast<uart_word_length_t>(config_.data_bits - 5);

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

  // Configure UART
  esp_err_t err = uart_param_config(static_cast<uart_port_t>(port_), &uart_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Set pins
  err = uart_set_pin(static_cast<uart_port_t>(port_), config_.tx_pin, config_.rx_pin,
                     config_.use_hardware_flow_control ? config_.rts_pin : UART_PIN_NO_CHANGE,
                     config_.use_hardware_flow_control ? config_.cts_pin : UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  // Install UART driver
  err = uart_driver_install(static_cast<uart_port_t>(port_), config_.rx_buffer_size,
                            config_.tx_buffer_size, 0, nullptr, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  ESP_LOGI(TAG, "UART initialized on port %d, baud=%lu, TX=%d, RX=%d", port_, config_.baud_rate,
           config_.tx_pin, config_.rx_pin);

  return true;
#else
  last_error_ = HfUartErr::UART_ERR_UNSUPPORTED_OPERATION;
  return false;
#endif
}

bool McuUart::PlatformDeinitialize() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t err = uart_driver_delete(static_cast<uart_port_t>(port_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "uart_driver_delete failed: %s", esp_err_to_name(err));
    last_error_ = ConvertPlatformError(err);
    return false;
  }

  ESP_LOGI(TAG, "UART deinitialized on port %d", port_);
  return true;
#else
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
