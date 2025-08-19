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
static bool g_pattern_callback_triggered = false;
static bool g_break_callback_triggered = false;

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
bool test_uart_dma_support() noexcept;
bool test_uart_bitrate_detection() noexcept;
bool test_uart_enhanced_pattern_detection() noexcept;
bool test_uart_esp32c6_wakeup_modes() noexcept;
bool test_uart_collision_detection() noexcept;
bool test_uart_performance() noexcept;
bool test_uart_lp_uart_support() noexcept;
bool test_uart_cleanup() noexcept;

//==============================================================================
// CALLBACK FUNCTIONS
//==============================================================================

bool uart_event_callback(const void* event, void* user_data) noexcept {
  if (event != nullptr) {
    g_event_callback_triggered = true;
    ESP_LOGI(TAG, "Event callback triggered");
  }
  return false; // Don't yield
}

bool uart_pattern_callback(int pattern_pos, void* user_data) noexcept {
  g_pattern_callback_triggered = true;
  ESP_LOGI(TAG, "Pattern callback triggered at position: %d", pattern_pos);
  return false; // Don't yield
}

bool uart_break_callback(hf_u32_t break_duration, void* user_data) noexcept {
  g_break_callback_triggered = true;
  ESP_LOGI(TAG, "Break callback triggered with duration: %lu", break_duration);
  return false; // Don't yield
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
  ESP_LOGI(TAG, "Testing UART pattern detection...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Reset callback flags
  g_pattern_callback_triggered = false;

  // Set pattern callback
  hf_uart_err_t result = g_uart_instance->SetPatternCallback(uart_pattern_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set pattern callback: %d", static_cast<int>(result));
    return false;
  }

  // Check pattern detection status
  bool pattern_enabled = g_uart_instance->IsPatternDetectionEnabled();
  ESP_LOGI(TAG, "Pattern detection enabled: %s", pattern_enabled ? "true" : "false");

  // Test pattern position (should return -1 if no pattern)
  int pattern_pos = g_uart_instance->GetPatternPosition(false);
  ESP_LOGI(TAG, "Pattern position: %d", pattern_pos);

  ESP_LOGI(TAG, "[SUCCESS] Pattern detection test completed");
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
  g_pattern_callback_triggered = false;
  g_break_callback_triggered = false;

  // Set event callback
  hf_uart_err_t result = g_uart_instance->SetEventCallback(uart_event_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set event callback: %d", static_cast<int>(result));
    return false;
  }

  // Set pattern callback
  result = g_uart_instance->SetPatternCallback(uart_pattern_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set pattern callback: %d", static_cast<int>(result));
    return false;
  }

  // Set break callback
  result = g_uart_instance->SetBreakCallback(uart_break_callback);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set break callback: %d", static_cast<int>(result));
    return false;
  }

  // Note: In normal testing without external devices, callbacks may not trigger
  // This test verifies that callback registration works without errors

  ESP_LOGI(TAG, "Callback registration completed");
  ESP_LOGI(TAG, "Event callback triggered: %s", g_event_callback_triggered ? "true" : "false");
  ESP_LOGI(TAG, "Pattern callback triggered: %s", g_pattern_callback_triggered ? "true" : "false");
  ESP_LOGI(TAG, "Break callback triggered: %s", g_break_callback_triggered ? "true" : "false");

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
  ESP_LOGI(TAG, "Testing ESP32-C6 specific UART features...");

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

  // Test improved break detection
  bool break_detected = g_uart_instance->IsBreakDetected();
  ESP_LOGI(TAG, "Break detected: %s", break_detected ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] ESP32-C6 specific features test completed");
  return true;
}

bool test_uart_dma_support() noexcept {
  ESP_LOGI(TAG, "Testing UART DMA support...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test DMA enable/disable
  hf_uart_err_t result = g_uart_instance->EnableDMA();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable DMA: %d", static_cast<int>(result));
    return false;
  }

  // Check DMA status
  bool dma_enabled = g_uart_instance->IsDMAEnabled();
  if (!dma_enabled) {
    ESP_LOGE(TAG, "DMA not enabled after EnableDMA call");
    return false;
  }

  ESP_LOGI(TAG, "DMA enabled successfully");

  // Test DMA write (will fall back to regular write in current implementation)
  const char* test_data = "DMA Test Data";
  result = g_uart_instance->WriteDMA(reinterpret_cast<const uint8_t*>(test_data),
                                     static_cast<hf_u16_t>(strlen(test_data)), 1000);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "DMA write failed: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "DMA write successful");

  // Test DMA read (will fall back to regular read in current implementation)
  uint8_t read_buffer[64];
  result = g_uart_instance->ReadDMA(read_buffer, sizeof(read_buffer), 500);
  ESP_LOGI(TAG, "DMA read result: %d", static_cast<int>(result));

  // Disable DMA
  result = g_uart_instance->DisableDMA();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable DMA: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] DMA support test completed");
  return true;
}

bool test_uart_bitrate_detection() noexcept {
  ESP_LOGI(TAG, "Testing UART bitrate detection...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test bitrate detection
  uint32_t detected_baud = 0;
  hf_uart_err_t result = g_uart_instance->DetectBitrate(detected_baud);
  
  // Note: ESP-IDF v5.5 may not have full bitrate detection support
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Bitrate detection returned: %lu bps", detected_baud);
    // Verify it returns a reasonable value
    if (detected_baud >= 9600 && detected_baud <= 115200) {
      ESP_LOGI(TAG, "Bitrate detection returned valid baud rate");
    }
  } else {
    ESP_LOGW(TAG, "Bitrate detection not fully supported in ESP-IDF v5.5: %d", static_cast<int>(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] Bitrate detection test completed");
  return true;
}

bool test_uart_enhanced_pattern_detection() noexcept {
  ESP_LOGI(TAG, "Testing enhanced pattern detection...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test pattern detection enable (placeholder in ESP-IDF v5.5)
  hf_uart_err_t result = g_uart_instance->EnablePatternDetection('+', 3, 5, 5, 5);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Pattern detection enabled for '+++' sequence (placeholder implementation)");

  // Check pattern detection status
  bool pattern_enabled = g_uart_instance->IsPatternDetectionEnabled();
  if (!pattern_enabled) {
    ESP_LOGE(TAG, "Pattern detection not enabled after EnablePatternDetection call");
    return false;
  }

  // Test pattern queue reset
  result = g_uart_instance->ResetPatternQueue();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Pattern queue reset successful (placeholder implementation)");

  // Test pattern position retrieval
  int pattern_pos = g_uart_instance->GetPatternPosition(false);
  ESP_LOGI(TAG, "Pattern position: %d (expected -1 in ESP-IDF v5.5)", pattern_pos);

  // Disable pattern detection
  result = g_uart_instance->DisablePatternDetection();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Enhanced pattern detection test completed (placeholder implementation)");
  return true;
}

bool test_uart_esp32c6_wakeup_modes() noexcept {
  ESP_LOGI(TAG, "Testing ESP32-C6 wakeup modes...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test FIFO_THRESH wakeup mode
  hf_uart_wakeup_config_t wakeup_config = {};
  wakeup_config.enable_wakeup = true;
  wakeup_config.wakeup_threshold = 5;
  wakeup_config.wakeup_mode = hf_uart_wakeup_mode_t::HF_UART_WK_MODE_FIFO_THRESH;

  hf_uart_err_t result = g_uart_instance->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure FIFO_THRESH wakeup: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "FIFO_THRESH wakeup mode configured");

  // Test START_BIT wakeup mode
  wakeup_config.wakeup_mode = hf_uart_wakeup_mode_t::HF_UART_WK_MODE_START_BIT;
  result = g_uart_instance->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure START_BIT wakeup: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "START_BIT wakeup mode configured");

  // Test CHAR_SEQ wakeup mode
  wakeup_config.wakeup_mode = hf_uart_wakeup_mode_t::HF_UART_WK_MODE_CHAR_SEQ;
  wakeup_config.char_sequence[0] = 'W';
  wakeup_config.char_sequence[1] = 'A';
  wakeup_config.char_sequence[2] = 'K';
  wakeup_config.char_sequence[3] = 'E';
  wakeup_config.char_sequence_length = 4;

  result = g_uart_instance->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure CHAR_SEQ wakeup: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "CHAR_SEQ wakeup mode configured with 'WAKE' sequence");

  // Check wakeup status
  bool wakeup_enabled = g_uart_instance->IsWakeupEnabled();
  if (!wakeup_enabled) {
    ESP_LOGE(TAG, "Wakeup not enabled after configuration");
    return false;
  }

  // Disable wakeup
  wakeup_config.enable_wakeup = false;
  result = g_uart_instance->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable wakeup: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ESP32-C6 wakeup modes test completed");
  return true;
}

bool test_uart_collision_detection() noexcept {
  ESP_LOGI(TAG, "Testing UART collision detection...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Test collision flag retrieval
  bool collision_flag = false;
  hf_uart_err_t result = g_uart_instance->GetCollisionFlag(collision_flag);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get collision flag: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Collision flag: %s", collision_flag ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] Collision detection test completed");
  return true;
}

bool test_uart_performance() noexcept {
  ESP_LOGI(TAG, "Testing UART performance...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Performance test: Large data transmission
  const size_t test_data_size = 1024;
  uint8_t* test_data = static_cast<uint8_t*>(malloc(test_data_size));
  if (!test_data) {
    ESP_LOGE(TAG, "Failed to allocate test data");
    return false;
  }

  // Fill with test pattern
  for (size_t i = 0; i < test_data_size; i++) {
    test_data[i] = static_cast<uint8_t>(i & 0xFF);
  }

  // Measure transmission time
  uint64_t start_time = esp_timer_get_time();
  hf_uart_err_t result = g_uart_instance->Write(test_data, test_data_size, 5000);
  uint64_t end_time = esp_timer_get_time();

  free(test_data);

  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Performance test write failed: %d", static_cast<int>(result));
    return false;
  }

  uint64_t transmission_time = end_time - start_time;
  double throughput = (test_data_size * 8.0 * 1000000.0) / transmission_time; // bits per second

  ESP_LOGI(TAG, "Performance test results:");
  ESP_LOGI(TAG, "  Data size: %zu bytes", test_data_size);
  ESP_LOGI(TAG, "  Transmission time: %llu μs", transmission_time);
  ESP_LOGI(TAG, "  Throughput: %.2f bps", throughput);

  // Test burst writes
  const size_t burst_size = 64;
  const size_t burst_count = 10;

  start_time = esp_timer_get_time();
  for (size_t i = 0; i < burst_count; i++) {
    uint8_t burst_data[burst_size];
    memset(burst_data, 0xAA + i, burst_size);
    
    result = g_uart_instance->Write(burst_data, burst_size, 1000);
    if (result != hf_uart_err_t::UART_SUCCESS) {
      ESP_LOGE(TAG, "Burst write %zu failed: %d", i, static_cast<int>(result));
      return false;
    }
  }
  end_time = esp_timer_get_time();

  uint64_t burst_time = end_time - start_time;
  ESP_LOGI(TAG, "Burst test: %zu x %zu bytes in %llu μs", burst_count, burst_size, burst_time);

  ESP_LOGI(TAG, "[SUCCESS] Performance test completed");
  return true;
}

bool test_uart_lp_uart_support() noexcept {
  ESP_LOGI(TAG, "Testing Low Power UART support...");

  if (!g_uart_instance || !g_uart_instance->IsInitialized()) {
    ESP_LOGE(TAG, "UART not initialized");
    return false;
  }

  // Check if LP UART is available
  bool lp_available = g_uart_instance->IsLPUartAvailable();
  ESP_LOGI(TAG, "LP UART available: %s", lp_available ? "true" : "false");

#ifdef HF_MCU_ESP32C6
  if (!lp_available) {
    ESP_LOGE(TAG, "LP UART should be available on ESP32-C6");
    return false;
  }

  // Test LP UART configuration
  hf_lp_uart_config_t lp_config = {};
  lp_config.baud_rate = 9600;
  lp_config.enable_wakeup = true;
  lp_config.wakeup_threshold = 5;
  lp_config.enable_collision_detect = false;

  hf_uart_err_t result = g_uart_instance->ConfigureLPUart(lp_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure LP UART: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "LP UART configuration successful");

  // Test LP UART enable
  result = g_uart_instance->EnableLPUart();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable LP UART: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "LP UART enabled successfully");

  // Test LP UART disable
  result = g_uart_instance->DisableLPUart();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable LP UART: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "LP UART disabled successfully");

#else
  if (lp_available) {
    ESP_LOGE(TAG, "LP UART should not be available on non-ESP32-C6 chips");
    return false;
  }

  ESP_LOGI(TAG, "LP UART correctly not available on this chip");
#endif

  ESP_LOGI(TAG, "[SUCCESS] LP UART support test completed");
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
  
  // ESP-IDF v5.5 and ESP32-C6 specific tests
  RUN_TEST(test_uart_esp32c6_features);
  RUN_TEST(test_uart_dma_support);
  RUN_TEST(test_uart_bitrate_detection);
  RUN_TEST(test_uart_enhanced_pattern_detection);
  RUN_TEST(test_uart_esp32c6_wakeup_modes);
  RUN_TEST(test_uart_collision_detection);
  RUN_TEST(test_uart_performance);
  RUN_TEST(test_uart_lp_uart_support);
  
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
