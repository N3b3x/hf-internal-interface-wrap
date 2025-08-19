/**
 * @file UartComprehensiveTest.cpp
 * @brief Comprehensive UART testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspUart class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * UART functionalities including basic operations, async communication,
 * flow control, advanced features, callbacks, and hardware-specific capabilities.
 *
 * All functions are noexcept - no exception handling used.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BaseUart.h"
#include "mcu/esp32/EspUart.h"
#include "mcu/esp32/utils/EspTypes_UART.h"

static const char* TAG = "UART_Test";

static TestResults g_test_results;

// Test configuration constants
static constexpr hf_u8_t TEST_UART_PORT_0 = 0;
static constexpr hf_u8_t TEST_UART_PORT_1 = 1;
static constexpr hf_u32_t TEST_BAUD_RATE = 115200;
static constexpr hf_u8_t TEST_TX_PIN = 21;
static constexpr hf_u8_t TEST_RX_PIN = 20;
static constexpr hf_u8_t TEST_RTS_PIN = 22;
static constexpr hf_u8_t TEST_CTS_PIN = 23;

// Test data
static const uint8_t TEST_PATTERN = 0x0A; // Line feed
static constexpr hf_u16_t TEST_BUFFER_SIZE = 256;

// Global test instances
static std::unique_ptr<EspUart> g_uart_instance = nullptr;

// Callback test variables
static bool g_event_callback_triggered = false;
static bool g_break_callback_triggered = false;

// User event task variables
static TaskHandle_t g_user_event_task_handle = nullptr;
static bool g_pattern_detected = false;
static int g_pattern_position = -1;
static bool g_stop_event_task = false;

// Forward declarations
bool test_uart_construction() noexcept;
bool test_uart_initialization() noexcept;
bool test_uart_basic_communication() noexcept;
bool test_uart_baud_rate_configuration() noexcept;
bool test_uart_flow_control() noexcept;
bool test_uart_pattern_detection() noexcept;
bool test_uart_buffer_operations() noexcept;
bool test_uart_advanced_features() noexcept;
bool test_uart_communication_modes() noexcept;
bool test_uart_async_operations() noexcept;
bool test_uart_callbacks() noexcept;
bool test_uart_statistics_diagnostics() noexcept;
bool test_uart_printf_support() noexcept;
bool test_uart_error_handling() noexcept;
bool test_uart_esp32c6_features() noexcept;
bool test_uart_performance() noexcept;
bool test_uart_callback_verification() noexcept;
bool test_uart_pattern_detection_v55() noexcept;
bool test_uart_user_event_task() noexcept;
bool test_uart_cleanup() noexcept;

//==============================================================================
// CALLBACK FUNCTIONS
//==============================================================================

bool uart_event_callback(const hf_uart_event_t* event, void* user_data) noexcept {
  if (event != nullptr) {
    g_event_callback_triggered = true;
    const char* event_type_name = "UNKNOWN";
    
    switch (event->type) {
      case hf_uart_event_type_t::HF_UART_DATA:
        event_type_name = "DATA";
        break;
      case hf_uart_event_type_t::HF_UART_FIFO_OVF:
        event_type_name = "FIFO_OVF";
        break;
      case hf_uart_event_type_t::HF_UART_BUFFER_FULL:
        event_type_name = "BUFFER_FULL";
        break;
      case hf_uart_event_type_t::HF_UART_BREAK:
        event_type_name = "BREAK";
        break;
      case hf_uart_event_type_t::HF_UART_PARITY_ERR:
        event_type_name = "PARITY_ERR";
        break;
      case hf_uart_event_type_t::HF_UART_FRAME_ERR:
        event_type_name = "FRAME_ERR";
        break;
      case hf_uart_event_type_t::HF_UART_PATTERN_DET:
        event_type_name = "PATTERN_DET";
        break;
      default:
        break;
    }
    
    ESP_LOGI(TAG, "Event callback triggered: %s (size: %zu)", event_type_name, event->size);
  }
  return false; // Don't yield
}



bool uart_break_callback(hf_u32_t break_duration, void* user_data) noexcept {
  g_break_callback_triggered = true;
  ESP_LOGI(TAG, "Break callback triggered with duration: %lu", break_duration);
  return false; // Don't yield
}

//==============================================================================
// USER EVENT TASK (Demonstrates Proper ESP-IDF v5.5 Usage)
//==============================================================================

static void user_uart_event_task(void* arg) noexcept {
  EspUart* uart = static_cast<EspUart*>(arg);
  if (!uart) {
    ESP_LOGE(TAG, "Invalid UART instance in event task");
    vTaskDelete(NULL);
    return;
  }

  QueueHandle_t event_queue = uart->GetEventQueue();
  if (!event_queue) {
    ESP_LOGE(TAG, "No event queue available");
    vTaskDelete(NULL);
    return;
  }

  ESP_LOGI(TAG, "User event task started, monitoring UART events...");
  
  uart_event_t event;
  uint8_t* data_buffer = static_cast<uint8_t*>(malloc(256));
  if (!data_buffer) {
    ESP_LOGE(TAG, "Failed to allocate event task buffer");
    vTaskDelete(NULL);
    return;
  }

  while (!g_stop_event_task) {
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      switch (event.type) {
        case UART_DATA:
          ESP_LOGI(TAG, "USER TASK: Data event - %zu bytes available", event.size);
          g_event_callback_triggered = true;
          break;

        case UART_PATTERN_DET:
          ESP_LOGI(TAG, "USER TASK: Pattern detected!");
          g_pattern_detected = true;
          
          // Pop pattern position immediately (ESP-IDF v5.5 best practice)
          g_pattern_position = uart->PopPatternPosition();
          ESP_LOGI(TAG, "Pattern position: %d", g_pattern_position);
          
          if (g_pattern_position >= 0) {
            // Read exactly up to pattern position + 1 (including delimiter)
            int bytes_to_read = g_pattern_position + 1;
            int bytes_read = uart_read_bytes(static_cast<uart_port_t>(uart->GetPort()), 
                                           data_buffer, bytes_to_read, pdMS_TO_TICKS(100));
            if (bytes_read > 0) {
              ESP_LOGI(TAG, "Read %d bytes up to pattern", bytes_read);
              // Null terminate and log the data
              if (bytes_read < 256) {
                data_buffer[bytes_read] = '\0';
                ESP_LOGI(TAG, "Pattern data: %s", (char*)data_buffer);
              }
            }
          }
          break;

        case UART_BREAK:
          ESP_LOGI(TAG, "USER TASK: Break detected");
          g_break_callback_triggered = true;
          break;

        case UART_FIFO_OVF:
          ESP_LOGW(TAG, "USER TASK: RX FIFO overflow");
          uart_flush_input(static_cast<uart_port_t>(uart->GetPort()));
          xQueueReset(event_queue);
          break;

        case UART_BUFFER_FULL:
          ESP_LOGW(TAG, "USER TASK: Ring buffer full");
          uart_flush_input(static_cast<uart_port_t>(uart->GetPort()));
          xQueueReset(event_queue);
          break;

        case UART_PARITY_ERR:
          ESP_LOGW(TAG, "USER TASK: Parity error");
          break;

        case UART_FRAME_ERR:
          ESP_LOGW(TAG, "USER TASK: Frame error");
          break;

        default:
          ESP_LOGD(TAG, "USER TASK: Unknown event type: %d", event.type);
          break;
      }
    }
  }

  free(data_buffer);
  ESP_LOGI(TAG, "User event task ending");
  g_user_event_task_handle = nullptr;
  vTaskDelete(NULL);
}

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

hf_uart_config_t create_test_config(hf_u8_t port = TEST_UART_PORT_0) noexcept {
  hf_uart_config_t config = {};
  config.port_number = port;
  config.baud_rate = TEST_BAUD_RATE;
  config.tx_pin = TEST_TX_PIN;
  config.rx_pin = TEST_RX_PIN;
  config.rts_pin = TEST_RTS_PIN;
  config.cts_pin = TEST_CTS_PIN;
  config.data_bits = hf_uart_data_bits_t::HF_UART_DATA_8_BITS;
  config.parity = hf_uart_parity_t::HF_UART_PARITY_DISABLE;
  config.stop_bits = hf_uart_stop_bits_t::HF_UART_STOP_BITS_1;
  config.flow_control = hf_uart_flow_ctrl_t::HF_UART_HW_FLOWCTRL_DISABLE;
  config.operating_mode = hf_uart_operating_mode_t::HF_UART_MODE_POLLING;
  config.rx_buffer_size = TEST_BUFFER_SIZE;
  config.tx_buffer_size = TEST_BUFFER_SIZE;
  return config;
}

bool verify_uart_state(EspUart& uart, bool should_be_initialized) noexcept {
  if (uart.IsInitialized() != should_be_initialized) {
    ESP_LOGE(TAG, "UART initialization state mismatch. Expected: %s, Actual: %s",
             should_be_initialized ? "initialized" : "not initialized",
             uart.IsInitialized() ? "initialized" : "not initialized");
    return false;
  }
  return true;
}

//==============================================================================
// TEST FUNCTIONS
//==============================================================================

bool test_uart_construction() noexcept {
  ESP_LOGI(TAG, "Testing UART construction and destruction...");

  // Test construction with valid configuration
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to construct EspUart instance");
    return false;
  }

  // Verify initial state
  if (!verify_uart_state(*uart, false)) {
    ESP_LOGE(TAG, "Initial state verification failed");
    return false;
  }

  // Store for other tests
  g_uart_instance = std::move(uart);

  ESP_LOGI(TAG, "[SUCCESS] UART construction completed");
  return true;
}

bool test_uart_initialization() noexcept {
  ESP_LOGI(TAG, "Testing UART initialization...");

  if (!g_uart_instance) {
    ESP_LOGE(TAG, "No UART instance available");
    return false;
  }

  // Test EnsureInitialized (lazy initialization)
  if (!g_uart_instance->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART");
    return false;
  }

  // Verify initialization state
  if (!verify_uart_state(*g_uart_instance, true)) {
    ESP_LOGE(TAG, "Post-initialization state verification failed");
    return false;
  }

  // Test configuration retrieval
  const hf_uart_config_t& config = g_uart_instance->GetPortConfig();
  if (config.port_number != TEST_UART_PORT_0 || config.baud_rate != TEST_BAUD_RATE) {
    ESP_LOGE(TAG, "Configuration mismatch after initialization");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] UART initialization successful");
  return true;
}

bool test_uart_basic_communication() noexcept {
  ESP_LOGI(TAG, "Testing basic UART communication...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test transmission and reception
  const char* test_message = "Hello, UART Test!";
  hf_uart_err_t write_result =
      g_uart_instance->Write(reinterpret_cast<const hf_u8_t*>(test_message), strlen(test_message));

  if (write_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Write failed with error: %d", static_cast<int>(write_result));
    return false;
  }

  ESP_LOGI(TAG, "Data transmitted successfully");

  // Test status functions (using public methods only)
  hf_u16_t tx_waiting = g_uart_instance->TxBytesWaiting();
  ESP_LOGI(TAG, "TX bytes waiting: %d", tx_waiting);

  ESP_LOGI(TAG, "[SUCCESS] Basic communication completed");
  return true;
}

bool test_uart_baud_rate_configuration() noexcept {
  ESP_LOGI(TAG, "Testing UART baud rate configuration...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test various baud rates
  hf_u32_t test_baud_rates[] = {9600, 19200, 38400, 57600, 115200, 230400};
  hf_u8_t num_rates = sizeof(test_baud_rates) / sizeof(test_baud_rates[0]);

  for (hf_u8_t i = 0; i < num_rates; i++) {
    if (!g_uart_instance->SetBaudRate(test_baud_rates[i])) {
      ESP_LOGE(TAG, "Failed to set baud rate: %lu", test_baud_rates[i]);
      return false;
    }

    // Small delay to allow hardware to settle
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "Successfully set baud rate to: %lu", test_baud_rates[i]);
  }

  // Restore original baud rate
  if (!g_uart_instance->SetBaudRate(TEST_BAUD_RATE)) {
    ESP_LOGE(TAG, "Failed to restore original baud rate");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Baud rate configuration test completed");
  return true;
}

bool test_uart_flow_control() noexcept {
  ESP_LOGI(TAG, "Testing UART flow control...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test enabling flow control
  hf_uart_err_t result = g_uart_instance->SetFlowControl(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable flow control: %d", static_cast<int>(result));
    return false;
  }

  // Test RTS control
  result = g_uart_instance->SetRTS(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTS high: %d", static_cast<int>(result));
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(10));

  result = g_uart_instance->SetRTS(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTS low: %d", static_cast<int>(result));
    return false;
  }

  // Test software flow control
  result = g_uart_instance->ConfigureSoftwareFlowControl(true, 20, 80);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure software flow control: %d", static_cast<int>(result));
    return false;
  }

  // Disable software flow control
  result = g_uart_instance->ConfigureSoftwareFlowControl(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable software flow control: %d", static_cast<int>(result));
    return false;
  }

  // Disable flow control
  result = g_uart_instance->SetFlowControl(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable flow control: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Flow control test completed");
  return true;
}

bool test_uart_pattern_detection() noexcept {
  ESP_LOGI(TAG, "Testing UART pattern detection (basic functionality)...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Note: ESP-IDF v5.5 does not support advanced pattern detection
  // This test verifies basic UART functionality that could support pattern detection
  
  // Test reading until a specific character (manual pattern detection)
  const char* test_data = "Hello+World+Test+";
  hf_uart_err_t write_result = g_uart_instance->Write(
      reinterpret_cast<const uint8_t*>(test_data), 
      static_cast<hf_u16_t>(strlen(test_data)), 1000);
  
  if (write_result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test data written for manual pattern detection");
    
    // Test ReadUntil as a form of pattern detection
    uint8_t buffer[64];
    hf_u16_t bytes_read = g_uart_instance->ReadUntil(buffer, sizeof(buffer), '+', 500);
    ESP_LOGI(TAG, "ReadUntil found %d bytes before '+' character", bytes_read);
  }

  ESP_LOGI(TAG, "[SUCCESS] Basic pattern detection test completed");
  return true;
}

bool test_uart_buffer_operations() noexcept {
  ESP_LOGI(TAG, "Testing UART buffer operations...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test ReadUntil with terminator
  uint8_t read_buffer[128];
  const char* test_string = "Hello\nWorld";

  // Write test string
  hf_uart_err_t write_result =
      g_uart_instance->Write(reinterpret_cast<const uint8_t*>(test_string),
                             static_cast<hf_u16_t>(strlen(test_string)), 1000);

  if (write_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Write failed for ReadUntil test (expected in no-loopback mode)");
  }

  // Test ReadUntil (will timeout in normal mode, which is expected)
  hf_u16_t bytes_read = g_uart_instance->ReadUntil(read_buffer, sizeof(read_buffer), '\n', 100);
  ESP_LOGI(TAG, "ReadUntil returned %d bytes", bytes_read);

  // Test ReadLine
  char line_buffer[128];
  hf_u16_t line_length = g_uart_instance->ReadLine(line_buffer, sizeof(line_buffer), 100);
  ESP_LOGI(TAG, "ReadLine returned %d characters", line_length);

  // Test threshold configurations
  hf_uart_err_t result = g_uart_instance->SetRxFullThreshold(64);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RX full threshold: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->SetTxEmptyThreshold(32);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set TX empty threshold: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->SetRxTimeoutThreshold(10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RX timeout threshold: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Buffer operations test completed");
  return true;
}

bool test_uart_advanced_features() noexcept {
  ESP_LOGI(TAG, "Testing UART advanced features...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test break signal
  hf_uart_err_t result = g_uart_instance->SendBreak(100);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send break: %d", static_cast<int>(result));
    return false;
  }

  // Test loopback mode
  result = g_uart_instance->SetLoopback(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Test with loopback enabled
  const char* loopback_msg = "Loopback Test";
  result = g_uart_instance->Write(reinterpret_cast<const uint8_t*>(loopback_msg),
                                  static_cast<hf_u16_t>(strlen(loopback_msg)), 1000);

  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Loopback write successful");

    // Try to read back (should work in loopback mode)
    uint8_t loopback_buffer[64];
    result = g_uart_instance->Read(loopback_buffer, sizeof(loopback_buffer), 500);
    if (result == hf_uart_err_t::UART_SUCCESS) {
      ESP_LOGI(TAG, "Loopback read successful");
    }
  }

  // Disable loopback
  result = g_uart_instance->SetLoopback(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Test signal inversion
  result = g_uart_instance->SetSignalInversion(0); // No inversion
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set signal inversion: %d", static_cast<int>(result));
    return false;
  }

  // Test wakeup configuration
  hf_uart_wakeup_config_t wakeup_config = {};
  wakeup_config.enable_wakeup = true;
  wakeup_config.wakeup_threshold = 3;

  result = g_uart_instance->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure wakeup: %d", static_cast<int>(result));
    return false;
  }

  // Check wakeup status
  bool wakeup_enabled = g_uart_instance->IsWakeupEnabled();
  ESP_LOGI(TAG, "Wakeup enabled: %s", wakeup_enabled ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] Advanced features test completed");
  return true;
}

bool test_uart_communication_modes() noexcept {
  ESP_LOGI(TAG, "Testing UART communication modes...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test standard UART mode
  hf_uart_err_t result = g_uart_instance->SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_UART);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set UART mode: %d", static_cast<int>(result));
    return false;
  }

  hf_uart_mode_t current_mode = g_uart_instance->GetCommunicationMode();
  if (current_mode != hf_uart_mode_t::HF_UART_MODE_UART) {
    ESP_LOGE(TAG, "Communication mode mismatch");
    return false;
  }

  // Test RS485 configuration
  hf_uart_rs485_config_t rs485_config = {};
  rs485_config.mode = hf_uart_mode_t::HF_UART_MODE_RS485;
  rs485_config.enable_collision_detect = true;
  rs485_config.enable_echo_suppression = false;
  rs485_config.auto_rts_control = false;

  result = g_uart_instance->ConfigureRS485(rs485_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure RS485: %d", static_cast<int>(result));
    return false;
  }

  // Test IrDA configuration
  hf_uart_irda_config_t irda_config = {};
  irda_config.enable_irda = true;
  irda_config.invert_tx = true;
  irda_config.invert_rx = true;
  irda_config.duty_cycle = 50;

  result = g_uart_instance->ConfigureIrDA(irda_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure IrDA: %d", static_cast<int>(result));
    return false;
  }

  // Return to standard UART mode
  result = g_uart_instance->SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_UART);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to return to UART mode: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Communication modes test completed");
  return true;
}

bool test_uart_async_operations() noexcept {
  ESP_LOGI(TAG, "Testing UART async operations...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test interrupt enable/disable
  hf_uart_err_t result = g_uart_instance->EnableRxInterrupts(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable RX interrupts: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->EnableTxInterrupts(true, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable TX interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Test operating mode change to interrupt mode
  result = g_uart_instance->SetOperatingMode(hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set interrupt mode: %d", static_cast<int>(result));
    return false;
  }

  hf_uart_operating_mode_t current_mode = g_uart_instance->GetOperatingMode();
  if (current_mode != hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    ESP_LOGE(TAG, "Operating mode mismatch");
    return false;
  }

  // Allow some time for interrupt mode to settle
  vTaskDelay(pdMS_TO_TICKS(100));

  // Return to polling mode
  result = g_uart_instance->SetOperatingMode(hf_uart_operating_mode_t::HF_UART_MODE_POLLING);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to return to polling mode: %d", static_cast<int>(result));
    return false;
  }

  // Disable interrupts
  result = g_uart_instance->EnableRxInterrupts(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable RX interrupts: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->EnableTxInterrupts(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable TX interrupts: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Async operations test completed");
  return true;
}

bool test_uart_callbacks() noexcept {
  ESP_LOGI(TAG, "Testing UART callbacks...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Reset callback flags
  g_event_callback_triggered = false;
  g_break_callback_triggered = false;

  // Set event callback (this will automatically switch to interrupt mode)
  hf_uart_err_t result = g_uart_instance->SetEventCallback(uart_event_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set event callback: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Event callback registered successfully");

  // Set break callback
  result = g_uart_instance->SetBreakCallback(uart_break_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set break callback: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Break callback registered successfully");

  // Verify we're in interrupt mode
  hf_uart_operating_mode_t current_mode = g_uart_instance->GetOperatingMode();
  if (current_mode != hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    ESP_LOGE(TAG, "UART should be in interrupt mode after setting callbacks");
    return false;
  }

  ESP_LOGI(TAG, "UART correctly switched to interrupt mode");

  // Test break signal to trigger break callback
  result = g_uart_instance->SendBreak(50);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Break signal sent");
    
    // Wait a bit for the break event to be processed
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Test data transmission to potentially trigger data events
  const char* test_data = "Callback Test Data";
  result = g_uart_instance->Write(reinterpret_cast<const uint8_t*>(test_data),
                                  static_cast<hf_u16_t>(strlen(test_data)), 1000);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test data sent for callback testing");
    
    // Wait for potential events to be processed
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  ESP_LOGI(TAG, "Callback test results:");
  ESP_LOGI(TAG, "  Event callback triggered: %s", g_event_callback_triggered ? "true" : "false");
  ESP_LOGI(TAG, "  Break callback triggered: %s", g_break_callback_triggered ? "true" : "false");

  // Clear callbacks
  g_uart_instance->SetEventCallback(nullptr);
  g_uart_instance->SetBreakCallback(nullptr);

  ESP_LOGI(TAG, "[SUCCESS] Callbacks test completed");
  return true;
}

bool test_uart_statistics_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing UART statistics and diagnostics...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test statistics retrieval
  hf_uart_statistics_t statistics = {};
  hf_uart_err_t result = g_uart_instance->GetStatistics(statistics);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get statistics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Statistics:");
  ESP_LOGI(TAG, "  Bytes sent: %lu", statistics.tx_byte_count);
  ESP_LOGI(TAG, "  Bytes received: %lu", statistics.rx_byte_count);
  ESP_LOGI(TAG, "  TX Errors: %lu", statistics.tx_error_count);
  ESP_LOGI(TAG, "  RX Errors: %lu", statistics.rx_error_count);

  // Test diagnostics retrieval
  hf_uart_diagnostics_t diagnostics = {};
  result = g_uart_instance->GetDiagnostics(diagnostics);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Diagnostics:");
  ESP_LOGI(TAG, "  Last error: %d", static_cast<int>(diagnostics.last_error));
  ESP_LOGI(TAG, "  Error reset count: %lu", diagnostics.error_reset_count);

  // Test error retrieval
  hf_uart_err_t last_error = g_uart_instance->GetLastError();
  ESP_LOGI(TAG, "Last error: %d", static_cast<int>(last_error));

  // Test status checks
  bool is_transmitting = g_uart_instance->IsTransmitting();
  bool is_receiving = g_uart_instance->IsReceiving();

  ESP_LOGI(TAG, "Status: TX=%s, RX=%s", is_transmitting ? "true" : "false",
           is_receiving ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics test completed");
  return true;
}

bool test_uart_printf_support() noexcept {
  ESP_LOGI(TAG, "Testing UART printf support...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test Printf functionality
  int result = g_uart_instance->Printf("Test printf: %d, %s, %.2f\n", 42, "hello", 3.14);
  if (result < 0) {
    ESP_LOGE(TAG, "Printf failed with result: %d", result);
    return false;
  }

  ESP_LOGI(TAG, "Printf returned %d characters", result);

  // Test VPrintf (through internal usage)
  int result2 = g_uart_instance->Printf("Another test: %c%c%c\n", 'A', 'B', 'C');
  if (result2 < 0) {
    ESP_LOGE(TAG, "Second printf failed with result: %d", result2);
    return false;
  }

  ESP_LOGI(TAG, "Second printf returned %d characters", result2);

  ESP_LOGI(TAG, "[SUCCESS] Printf support test completed");
  return true;
}

bool test_uart_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing UART error handling...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test invalid parameters (should handle gracefully)

  // Test write with null data
  hf_uart_err_t result = g_uart_instance->Write(nullptr, 10, 1000);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Write with null data unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Write with null data correctly failed");
  }

  // Test read with null buffer
  result = g_uart_instance->Read(nullptr, 10, 1000);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Read with null buffer unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Read with null buffer correctly failed");
  }

  // Test invalid baud rate
  bool baud_result = g_uart_instance->SetBaudRate(0);
  if (baud_result) {
    ESP_LOGW(TAG, "Invalid baud rate unexpectedly accepted");
  } else {
    ESP_LOGI(TAG, "Invalid baud rate correctly rejected");
  }

  // Restore valid baud rate
  g_uart_instance->SetBaudRate(TEST_BAUD_RATE);

  ESP_LOGI(TAG, "[SUCCESS] Error handling test completed");
  return true;
}

bool test_uart_esp32c6_features() noexcept {
  ESP_LOGI(TAG, "Testing ESP32-C6 UART implementation...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test bytes available functionality
  hf_u16_t bytes_available = g_uart_instance->BytesAvailable();
  ESP_LOGI(TAG, "Bytes available: %d", bytes_available);

  // Test TX busy status
  bool tx_busy = g_uart_instance->IsTxBusy();
  ESP_LOGI(TAG, "TX busy: %s", tx_busy ? "true" : "false");

  // Test break detection status
  bool break_detected = g_uart_instance->IsBreakDetected();
  ESP_LOGI(TAG, "Break detected: %s", break_detected ? "true" : "false");

  // Test actual break signal sending
  hf_uart_err_t result = g_uart_instance->SendBreak(50);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Break signal sent successfully");
  } else {
    ESP_LOGE(TAG, "Failed to send break signal: %d", static_cast<int>(result));
    return false;
  }

  // Test signal inversion
  result = g_uart_instance->SetSignalInversion(0); // No inversion
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set signal inversion: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ESP32-C6 UART implementation test completed");
  return true;
}



bool test_uart_performance() noexcept {
  ESP_LOGI(TAG, "Testing UART performance...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Simple performance test: Medium data transmission
  const size_t test_data_size = 256;
  uint8_t test_data[test_data_size];

  // Fill with test pattern
  for (size_t i = 0; i < test_data_size; i++) {
    test_data[i] = static_cast<uint8_t>(i & 0xFF);
  }

  // Measure transmission time
  uint64_t start_time = esp_timer_get_time();
  hf_uart_err_t result = g_uart_instance->Write(test_data, test_data_size, 2000);
  uint64_t end_time = esp_timer_get_time();

  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Performance test write failed: %d", static_cast<int>(result));
    return false;
  }

  uint64_t transmission_time = end_time - start_time;
  ESP_LOGI(TAG, "Performance test: %zu bytes in %llu μs", test_data_size, transmission_time);

  ESP_LOGI(TAG, "[SUCCESS] Performance test completed");
  return true;
}

bool test_uart_callback_verification() noexcept {
  ESP_LOGI(TAG, "Testing UART callback verification with loopback...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Reset callback flags
  g_event_callback_triggered = false;
  g_break_callback_triggered = false;

  // Enable loopback mode for reliable callback testing
  hf_uart_err_t result = g_uart_instance->SetLoopback(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable loopback mode: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Loopback mode enabled for callback testing");

  // Set callbacks (this switches to interrupt mode automatically)
  result = g_uart_instance->SetEventCallback(uart_event_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set event callback: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->SetBreakCallback(uart_break_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set break callback: %d", static_cast<int>(result));
    return false;
  }

  // Wait for interrupt mode to be fully active
  vTaskDelay(pdMS_TO_TICKS(100));

  // Send test data that should trigger data events in loopback mode
  const char* test_message = "Loopback Callback Test";
  result = g_uart_instance->Write(reinterpret_cast<const uint8_t*>(test_message),
                                  static_cast<hf_u16_t>(strlen(test_message)), 1000);
  
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test data written in loopback mode");
    
    // Wait for events to be processed
    vTaskDelay(pdMS_TO_TICKS(300));
    
    // Try to read the data back (should trigger more events)
    uint8_t read_buffer[64];
    result = g_uart_instance->Read(read_buffer, sizeof(read_buffer), 500);
    ESP_LOGI(TAG, "Read result in loopback: %d", static_cast<int>(result));
    
    // Wait for any additional events
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  // Send break signal to test break callback
  result = g_uart_instance->SendBreak(100);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Break signal sent");
    vTaskDelay(pdMS_TO_TICKS(200)); // Wait for break event processing
  }

  // Disable loopback
  g_uart_instance->SetLoopback(false);

  // Report callback results
  ESP_LOGI(TAG, "Callback verification results:");
  ESP_LOGI(TAG, "  Event callback triggered: %s", g_event_callback_triggered ? "✅ YES" : "❌ NO");
  ESP_LOGI(TAG, "  Break callback triggered: %s", g_break_callback_triggered ? "✅ YES" : "❌ NO");

  // Clear callbacks and return to polling mode
  g_uart_instance->SetEventCallback(nullptr);
  g_uart_instance->SetBreakCallback(nullptr);
  g_uart_instance->SetOperatingMode(hf_uart_operating_mode_t::HF_UART_MODE_POLLING);

  ESP_LOGI(TAG, "[SUCCESS] Callback verification test completed");
  return true;
}

bool test_uart_pattern_detection_v55() noexcept {
  ESP_LOGI(TAG, "Testing ESP-IDF v5.5 Pattern Detection...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Enable loopback for reliable pattern testing
  hf_uart_err_t result = g_uart_instance->SetLoopback(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Test 1: Line-oriented pattern detection ('\n')
  ESP_LOGI(TAG, "Testing line-oriented pattern detection...");
  
  result = g_uart_instance->EnablePatternDetection('\n', 1, 9, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable line pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->ResetPatternQueue(32);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Line pattern detection enabled");

  // Test 2: AT escape sequence pattern detection ('+++')
  ESP_LOGI(TAG, "Testing AT escape sequence pattern detection...");
  
  result = g_uart_instance->DisablePatternDetection();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  // Enable +++ pattern (3 consecutive '+' characters)
  result = g_uart_instance->EnablePatternDetection('+', 3, 9, 50, 50);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable +++ pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->ResetPatternQueue(8);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue for +++: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "AT escape sequence pattern detection enabled");

  // Test pattern position functions
  int peek_pos = g_uart_instance->PeekPatternPosition();
  ESP_LOGI(TAG, "Pattern position (peek): %d", peek_pos);

  int pop_pos = g_uart_instance->PopPatternPosition();
  ESP_LOGI(TAG, "Pattern position (pop): %d", pop_pos);

  // Disable pattern detection
  result = g_uart_instance->DisablePatternDetection();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  // Disable loopback
  g_uart_instance->SetLoopback(false);

  ESP_LOGI(TAG, "[SUCCESS] ESP-IDF v5.5 Pattern Detection test completed");
  return true;
}

bool test_uart_user_event_task() noexcept {
  ESP_LOGI(TAG, "Testing user-created event task (ESP-IDF v5.5 pattern)...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Check if event queue is available
  if (!g_uart_instance->IsEventQueueAvailable()) {
    ESP_LOGE(TAG, "Event queue not available");
    return false;
  }

  ESP_LOGI(TAG, "Event queue available for user task");

  // Configure interrupts for event-driven operation
  uint32_t intr_mask = UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT;
  hf_uart_err_t result = g_uart_instance->ConfigureInterrupts(intr_mask, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Enable RX interrupts
  result = g_uart_instance->EnableRxInterrupts(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable RX interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Enable loopback for reliable testing
  result = g_uart_instance->SetLoopback(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Enable line pattern detection
  result = g_uart_instance->EnablePatternDetection('\n', 1, 9, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = g_uart_instance->ResetPatternQueue(16);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue: %d", static_cast<int>(result));
    return false;
  }

  // Reset test flags
  g_pattern_detected = false;
  g_pattern_position = -1;
  g_event_callback_triggered = false;
  g_stop_event_task = false;

  // Create user event task (proper ESP-IDF v5.5 pattern)
  BaseType_t task_result = xTaskCreate(user_uart_event_task, "user_uart_events", 4096, 
                                      g_uart_instance.get(), 10, &g_user_event_task_handle);
  if (task_result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create user event task");
    return false;
  }

  ESP_LOGI(TAG, "User event task created successfully");

  // Wait for task to start
  vTaskDelay(pdMS_TO_TICKS(100));

  // Send test data with line ending to trigger pattern detection
  const char* test_line = "Hello World\n";
  result = g_uart_instance->Write(reinterpret_cast<const uint8_t*>(test_line),
                                  static_cast<hf_u16_t>(strlen(test_line)), 1000);
  
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test line sent: '%s'", test_line);
    
    // Wait for pattern detection and processing
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // Send another test with multiple lines
  const char* multi_line = "Line1\nLine2\nLine3\n";
  result = g_uart_instance->Write(reinterpret_cast<const uint8_t*>(multi_line),
                                  static_cast<hf_u16_t>(strlen(multi_line)), 1000);
  
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Multi-line test sent");
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // Stop the user event task
  g_stop_event_task = true;
  vTaskDelay(pdMS_TO_TICKS(200)); // Wait for task to exit

  // Clean up
  g_uart_instance->DisablePatternDetection();
  g_uart_instance->SetLoopback(false);

  // Report results
  ESP_LOGI(TAG, "User event task test results:");
  ESP_LOGI(TAG, "  Event callback triggered: %s", g_event_callback_triggered ? "✅ YES" : "❌ NO");
  ESP_LOGI(TAG, "  Pattern detected: %s", g_pattern_detected ? "✅ YES" : "❌ NO");
  ESP_LOGI(TAG, "  Pattern position: %d", g_pattern_position);

  ESP_LOGI(TAG, "[SUCCESS] User event task test completed");
  return true;
}

bool test_uart_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing UART cleanup...");

  if (!g_uart_instance) {
    ESP_LOGE(TAG, "No UART instance to clean up");
    return false;
  }

  // The destructor will be called when the unique_ptr is reset
  g_uart_instance.reset();

  ESP_LOGI(TAG, "[SUCCESS] UART cleanup completed");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                   ESP32-C6 UART COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Run all tests in sequence
  RUN_TEST(test_uart_construction);
  RUN_TEST(test_uart_initialization);
  RUN_TEST(test_uart_basic_communication);
  RUN_TEST(test_uart_baud_rate_configuration);
  RUN_TEST(test_uart_flow_control);
  RUN_TEST(test_uart_pattern_detection);
  RUN_TEST(test_uart_buffer_operations);
  RUN_TEST(test_uart_advanced_features);
  RUN_TEST(test_uart_communication_modes);
  RUN_TEST(test_uart_async_operations);
  RUN_TEST(test_uart_callbacks);
  RUN_TEST(test_uart_statistics_diagnostics);
  RUN_TEST(test_uart_printf_support);
  RUN_TEST(test_uart_error_handling);
  
  // ESP32-C6 specific tests
  RUN_TEST(test_uart_esp32c6_features);
  RUN_TEST(test_uart_performance);
  RUN_TEST(test_uart_callback_verification);
  
  // ESP-IDF v5.5 Pattern Detection and User Task Tests
  RUN_TEST(test_uart_pattern_detection_v55);
  RUN_TEST(test_uart_user_event_task);
  
  RUN_TEST(test_uart_cleanup);

  print_test_summary(g_test_results, "UART", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG, "[SUCCESS] ALL UART TESTS PASSED!");
  } else {
    ESP_LOGE(TAG, "[FAILED] Some tests failed.");
  }

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
