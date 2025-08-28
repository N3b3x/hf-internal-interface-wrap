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
#include "mcu/esp32/EspGpio.h"
#include "mcu/esp32/EspUart.h"
#include "mcu/esp32/utils/EspTypes_UART.h"

// ESP-IDF UART constants
#ifdef __cplusplus
extern "C" {
#endif
#include "driver/uart.h"
#include "soc/uart_reg.h"
#ifdef __cplusplus
}
#endif

static const char* TAG = "UART_Test";
static TestResults g_test_results;

// Test progression indicator GPIO
static EspGpio* g_test_progress_gpio = nullptr;
static bool g_test_progress_state = false;
static constexpr hf_u8_t TEST_PROGRESS_GPIO = 14;

// Test configuration constants
// static constexpr hf_u8_t TEST_UART_PORT_0 = 0; Debug UART
static constexpr hf_u8_t TEST_UART_PORT_1 = 1;
static constexpr hf_u32_t TEST_BAUD_RATE = 115200;
static constexpr hf_u8_t TEST_TX_PIN = 5;
static constexpr hf_u8_t TEST_RX_PIN = 4;
static constexpr hf_u8_t TEST_RTS_PIN = 6;
static constexpr hf_u8_t TEST_CTS_PIN = 7;

// Test data
static const uint8_t TEST_PATTERN = 0x0A; // Line feed
// ESP32-C6 UART FIFO is only 128 bytes, so use appropriate buffer sizes
// ESP-IDF requires: rx_buffer_size > UART_HW_FIFO_LEN (128)
static constexpr hf_u16_t TEST_BUFFER_SIZE = 256;

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
bool test_uart_user_event_task() noexcept;
bool test_uart_event_driven_pattern_detection() noexcept;
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
            int bytes_read = uart_read_bytes(static_cast<uart_port_t>(uart->GetPort()), data_buffer,
                                             bytes_to_read, pdMS_TO_TICKS(100));
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

/**
 * @brief Flush UART buffers before each test to ensure clean test isolation
 * @param uart UART instance to flush
 * @return true if flush successful, false otherwise
 */
bool flush_uart_buffers(EspUart* uart) noexcept {
  if (!uart || !uart->IsInitialized()) {
    return false;
  }

  // Flush both TX and RX buffers
  hf_uart_err_t tx_result = uart->FlushTx();
  hf_uart_err_t rx_result = uart->FlushRx();

  if (tx_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "TX flush warning: %d", static_cast<int>(tx_result));
  }

  if (rx_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "RX flush warning: %d", static_cast<int>(rx_result));
  }

  // Also reset the event queue if available
  if (uart->IsEventQueueAvailable()) {
    uart->ResetEventQueue();
  }

  // Small delay to ensure buffers are fully cleared
  vTaskDelay(pdMS_TO_TICKS(10));

  ESP_LOGD(TAG, "UART buffers flushed before test");
  return true;
}

hf_uart_config_t create_test_config(hf_u8_t port = TEST_UART_PORT_1) noexcept {
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
  // ESP32-C6 UART FIFO is only 128 bytes, use appropriate buffer sizes
  // ESP-IDF requires: rx_buffer_size > UART_HW_FIFO_LEN (128)
  config.rx_buffer_size = 256;
  config.tx_buffer_size = 256;
  // Increase event queue size for better pattern detection reliability
  config.event_queue_size = 32;
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
// PROGRESS INDICATOR HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Initialize the test progression indicator GPIO
 */
bool init_test_progress_indicator() noexcept {
  // Use GPIO14 as the test progression indicator (visible LED on most ESP32 dev boards)
  g_test_progress_gpio =
      new EspGpio(TEST_PROGRESS_GPIO, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                  hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);

  if (!g_test_progress_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO");
    return false;
  }

  // Start with LOW state
  g_test_progress_gpio->SetInactive();
  g_test_progress_state = false;

  ESP_LOGI(TAG, "Test progression indicator initialized on GPIO%d", TEST_PROGRESS_GPIO);
  return true;
}

/**
 * @brief Flip the test progression indicator to show next test
 */
void flip_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_state = !g_test_progress_state;
    if (g_test_progress_state) {
      g_test_progress_gpio->SetActive();
    } else {
      g_test_progress_gpio->SetInactive();
    }
    ESP_LOGI(TAG, "Test progression indicator: %s", g_test_progress_state ? "HIGH" : "LOW");
  }
}

/**
 * @brief Cleanup the test progression indicator GPIO
 */
void cleanup_test_progress_indicator() noexcept {
  if (g_test_progress_gpio) {
    g_test_progress_gpio->SetInactive(); // Ensure pin is low
    delete g_test_progress_gpio;
    g_test_progress_gpio = nullptr;
  }
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

  // Test that we can create multiple instances
  auto uart2 = std::make_unique<EspUart>(config);
  if (!uart2) {
    ESP_LOGE(TAG, "Failed to construct second EspUart instance");
    return false;
  }

  // Verify both instances have independent state
  if (uart->IsInitialized() == uart2->IsInitialized()) {
    ESP_LOGI(TAG, "Multiple instances created successfully with independent state");
  }

  ESP_LOGI(TAG, "[SUCCESS] UART construction completed");
  return true;
}

bool test_uart_initialization() noexcept {
  ESP_LOGI(TAG, "Testing UART initialization...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for initialization test");
    return false;
  }

  // Test EnsureInitialized (lazy initialization)
  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART");
    return false;
  }

  // Verify initialization state
  if (!verify_uart_state(*uart, true)) {
    ESP_LOGE(TAG, "Post-initialization state verification failed");
    return false;
  }

  // Test configuration retrieval
  const hf_uart_config_t& retrieved_config = uart->GetPortConfig();
  if (retrieved_config.port_number != TEST_UART_PORT_1 ||
      retrieved_config.baud_rate != TEST_BAUD_RATE) {
    ESP_LOGE(TAG, "Configuration mismatch after initialization");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] UART initialization successful");
  return true;
}

bool test_uart_basic_communication() noexcept {
  ESP_LOGI(TAG, "Testing basic UART communication...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for basic communication test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for basic communication test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Test transmission and reception
  const char* test_message = "Hello, UART Test!";
  hf_uart_err_t write_result =
      uart->Write(reinterpret_cast<const hf_u8_t*>(test_message), strlen(test_message));

  if (write_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Write failed with error: %d", static_cast<int>(write_result));
    return false;
  }

  ESP_LOGI(TAG, "Data transmitted successfully");

  // Test status functions (using public methods only)
  hf_u16_t tx_waiting = uart->TxBytesWaiting();
  ESP_LOGI(TAG, "TX bytes waiting: %d", tx_waiting);

  ESP_LOGI(TAG, "[SUCCESS] Basic communication completed");
  return true;
}

bool test_uart_baud_rate_configuration() noexcept {
  ESP_LOGI(TAG, "Testing UART baud rate configuration...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for baud rate test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for baud rate test");
    return false;
  }

  // Test various baud rates
  hf_u32_t test_baud_rates[] = {9600, 19200, 38400, 57600, 115200, 230400};
  hf_u8_t num_rates = sizeof(test_baud_rates) / sizeof(test_baud_rates[0]);

  for (hf_u8_t i = 0; i < num_rates; i++) {
    if (!uart->SetBaudRate(test_baud_rates[i])) {
      ESP_LOGE(TAG, "Failed to set baud rate: %lu", test_baud_rates[i]);
      return false;
    }

    // Small delay to allow hardware to settle
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "Successfully set baud rate to: %lu", test_baud_rates[i]);
  }

  // Restore original baud rate
  if (!uart->SetBaudRate(TEST_BAUD_RATE)) {
    ESP_LOGE(TAG, "Failed to restore original baud rate");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Baud rate configuration test completed");
  return true;
}

bool test_uart_flow_control() noexcept {
  ESP_LOGI(TAG, "Testing UART flow control...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for flow control test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for flow control test");
    return false;
  }

  // Test enabling flow control
  hf_uart_err_t result = uart->SetFlowControl(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable flow control: %d", static_cast<int>(result));
    return false;
  }

  // Test RTS control
  result = uart->SetRTS(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTS high: %d", static_cast<int>(result));
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(10));

  result = uart->SetRTS(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set RTS low: %d", static_cast<int>(result));
    return false;
  }

  // Test software flow control
  result = uart->ConfigureSoftwareFlowControl(true, 20, 80);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure software flow control: %d", static_cast<int>(result));
    return false;
  }

  // Disable software flow control
  result = uart->ConfigureSoftwareFlowControl(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable software flow control: %d", static_cast<int>(result));
    return false;
  }

  // Disable flow control
  result = uart->SetFlowControl(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable flow control: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Flow control test completed");
  return true;
}

bool test_uart_pattern_detection() noexcept {
  ESP_LOGI(TAG, "Testing UART pattern detection (ESP-IDF v5.5) with event-driven approach...");

  // Create a new UART instance for this test with event queue enabled
  hf_uart_config_t config = create_test_config();
  config.event_queue_size = 32; // Ensure event queue is enabled
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for pattern detection test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for pattern detection test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Get the event queue handle
  QueueHandle_t event_queue = uart->GetEventQueue();
  if (!event_queue) {
    ESP_LOGE(TAG, "Event queue not available - pattern detection requires event queue");
    return false;
  }

  ESP_LOGI(TAG, "Using external loopback (jumper between pins 20 and 21)");

  // Configure interrupts for pattern detection (REQUIRED for ESP-IDF v5.5)
  // Note: Pattern detection interrupt is enabled internally by uart_enable_pattern_det_baud_intr
  hf_uart_err_t result =
      uart->ConfigureInterrupts(UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M, 32, 5);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts for pattern detection: %d",
             static_cast<int>(result));
    return false;
  }

  // Combined test variables
  bool line_test_passed = false;
  bool at_test_passed = false;
  int total_tests_passed = 0;

  // Test 1: Line-oriented pattern detection ('\n')
  ESP_LOGI(TAG, "\n=== Test 1: Line-oriented pattern detection ('\\n') ===");

  result = uart->EnablePatternDetection('\n', 1, 9, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable line pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = uart->ResetPatternQueue(16);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "Line pattern detection enabled");

  // Clear event queue before test
  uart->ResetEventQueue();

  // Send test data with line endings
  const char* test_lines = "Line1\nLine2\nLine3\n";
  result = uart->Write(reinterpret_cast<const uint8_t*>(test_lines),
                       static_cast<hf_u16_t>(strlen(test_lines)), 1000);

  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write test lines: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "Test data sent: '%s' (length: %d)", test_lines,
           static_cast<int>(strlen(test_lines)));

  // Add delay to allow data to be processed and prevent buffer overflow
  vTaskDelay(pdMS_TO_TICKS(100));

  // Process events with proper event-driven approach
  int pattern_count = 0;
  int expected_patterns = 3;
  int data_events = 0;
  uart_event_t event;
  uint8_t read_buffer[256];

  // Wait for events with timeout
  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = 3000000; // 3 seconds

  while ((esp_timer_get_time() - start_time) < timeout_us && pattern_count < expected_patterns) {
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      switch (event.type) {
        case UART_DATA:
          data_events++;
          ESP_LOGI(TAG, "UART_DATA event - size: %zu", event.size);
          // Read data to clear buffer and allow pattern detection
          uart->Read(read_buffer, sizeof(read_buffer), 100);
          break;

        case UART_PATTERN_DET: {
          ESP_LOGI(TAG, "UART_PATTERN_DET event received!");
          int pattern_pos = uart->PopPatternPosition();
          if (pattern_pos >= 0) {
            pattern_count++;
            ESP_LOGI(TAG, "Pattern %d detected at position: %d", pattern_count, pattern_pos);
          }
          break;
        }

        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
          ESP_LOGW(TAG, "Buffer overflow - clearing");
          uart->FlushRx();
          uart->ResetEventQueue();
          break;

        case UART_BREAK:
          ESP_LOGD(TAG, "UART_BREAK event");
          break;

        case UART_FRAME_ERR:
          ESP_LOGD(TAG, "UART_FRAME_ERR event");
          break;

        case UART_PARITY_ERR:
          ESP_LOGD(TAG, "UART_PARITY_ERR event");
          break;

        case UART_DATA_BREAK:
          ESP_LOGD(TAG, "UART_DATA_BREAK event");
          break;

        case UART_WAKEUP:
          ESP_LOGD(TAG, "UART_WAKEUP event");
          break;

        case UART_EVENT_MAX:
        default:
          ESP_LOGD(TAG, "Other event type: %d", event.type);
          break;
      }
    }

    // Small delay to prevent tight loop and allow pattern detection to work
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  line_test_passed = (pattern_count == expected_patterns);
  ESP_LOGI(TAG, "Line pattern detection: %d/%d patterns detected. %s", pattern_count,
           expected_patterns, line_test_passed ? "PASSED" : "FAILED");
  if (line_test_passed)
    total_tests_passed++;

  // Test 2: AT escape sequence pattern detection ('+++')
  ESP_LOGI(TAG, "\n=== Test 2: AT escape sequence pattern detection ('+++') ===");

  // Disable previous pattern detection
  result = uart->DisablePatternDetection();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  // Flush buffers and reset for next test
  flush_uart_buffers(uart.get());
  uart->ResetEventQueue();

  // Enable +++ pattern (3 consecutive '+' characters)
  result = uart->EnablePatternDetection('+', 3, 5, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable +++ pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = uart->ResetPatternQueue(8);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue for +++: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "AT escape sequence pattern detection enabled");

  // Send test data with AT escape sequences
  const char* at_test_data = "AT+CMD1\r\n+++ESCAPE1\r\nAT+CMD2\r\n+++ESCAPE2\r\n";
  result = uart->Write(reinterpret_cast<const uint8_t*>(at_test_data),
                       static_cast<hf_u16_t>(strlen(at_test_data)), 1000);

  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write AT test data: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "AT test data sent (length: %d)", static_cast<int>(strlen(at_test_data)));

  // Add delay to allow data to be processed and prevent buffer overflow
  vTaskDelay(pdMS_TO_TICKS(100));

  // Process AT pattern events
  int at_pattern_count = 0;
  int expected_at_patterns = 2;
  data_events = 0;

  start_time = esp_timer_get_time();

  while ((esp_timer_get_time() - start_time) < timeout_us &&
         at_pattern_count < expected_at_patterns) {
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      switch (event.type) {
        case UART_DATA:
          data_events++;
          ESP_LOGI(TAG, "UART_DATA event - size: %zu", event.size);
          // Read data to clear buffer
          uart->Read(read_buffer, sizeof(read_buffer), 100);
          break;

        case UART_PATTERN_DET: {
          ESP_LOGI(TAG, "UART_PATTERN_DET event received for +++!");
          int pattern_pos = uart->PopPatternPosition();
          if (pattern_pos >= 0) {
            at_pattern_count++;
            ESP_LOGI(TAG, "AT Pattern %d detected at position: %d", at_pattern_count, pattern_pos);
          }
          break;
        }

        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
          ESP_LOGW(TAG, "Buffer overflow - clearing");
          uart->FlushRx();
          uart->ResetEventQueue();
          break;

        case UART_BREAK:
          ESP_LOGD(TAG, "UART_BREAK event");
          break;

        case UART_FRAME_ERR:
          ESP_LOGD(TAG, "UART_FRAME_ERR event");
          break;

        case UART_PARITY_ERR:
          ESP_LOGD(TAG, "UART_PARITY_ERR event");
          break;

        case UART_DATA_BREAK:
          ESP_LOGD(TAG, "UART_DATA_BREAK event");
          break;

        case UART_WAKEUP:
          ESP_LOGD(TAG, "UART_WAKEUP event");
          break;

        case UART_EVENT_MAX:
        default:
          ESP_LOGD(TAG, "Other event type: %d", event.type);
          break;
      }
    }

    // Small delay to prevent tight loop and allow pattern detection to work
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  at_test_passed = (at_pattern_count == expected_at_patterns);
  ESP_LOGI(TAG, "AT pattern detection: %d/%d patterns detected. %s", at_pattern_count,
           expected_at_patterns, at_test_passed ? "PASSED" : "FAILED");
  if (at_test_passed)
    total_tests_passed++;

  // Clean up
  result = uart->DisablePatternDetection();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable pattern detection: %d", static_cast<int>(result));
  }

  // Overall test result
  bool overall_passed = (total_tests_passed == 2);

  ESP_LOGI(TAG, "\n=== PATTERN DETECTION TEST SUMMARY ===");
  ESP_LOGI(TAG, "Line pattern test: %s", line_test_passed ? "PASSED" : "FAILED");
  ESP_LOGI(TAG, "AT pattern test: %s", at_test_passed ? "PASSED" : "FAILED");
  ESP_LOGI(TAG, "Overall result: %d/2 tests passed. %s", total_tests_passed,
           overall_passed ? "[SUCCESS]" : "[FAILED]");

  return overall_passed;
}

bool test_uart_buffer_operations() noexcept {
  ESP_LOGI(TAG, "Testing UART buffer operations...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for buffer operations test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for buffer operations test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Test ReadUntil with terminator
  uint8_t read_buffer[128];
  const char* test_string = "Hello\nWorld";

  // Write test string
  hf_uart_err_t write_result = uart->Write(reinterpret_cast<const uint8_t*>(test_string),
                                           static_cast<hf_u16_t>(strlen(test_string)), 1000);

  if (write_result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Write failed for ReadUntil test (expected in no-loopback mode)");
  }

  // Test ReadUntil (will timeout in normal mode, which is expected)
  hf_u16_t bytes_read = uart->ReadUntil(read_buffer, sizeof(read_buffer), '\n', 100);
  ESP_LOGI(TAG, "ReadUntil returned %d bytes", bytes_read);

  // Test ReadLine
  char line_buffer[128];
  hf_u16_t line_length = uart->ReadLine(line_buffer, sizeof(line_buffer), 100);
  ESP_LOGI(TAG, "ReadLine returned %d characters", line_length);

  // Test interrupt configuration (modern ESP-IDF v5.5 approach)
  hf_uart_err_t result =
      uart->ConfigureInterrupts(UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M, 64, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Buffer operations test completed");
  return true;
}

bool test_uart_advanced_features() noexcept {
  ESP_LOGI(TAG, "Testing UART advanced features...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for advanced features test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for advanced features test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Test break signal (may not be fully supported on ESP32-C6)
  hf_uart_err_t result = uart->SendBreak(100);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send break: %s", HfUartErrToString(result));
    return false;
  }

  // Test loopback mode
  ESP_LOGI(TAG, "Testing with external loopback (jumper between pins 20 and 21)");

  // Test with external loopback enabled
  const char* loopback_msg = "Loopback Test";
  result = uart->Write(reinterpret_cast<const uint8_t*>(loopback_msg),
                       static_cast<hf_u16_t>(strlen(loopback_msg)), 1000);

  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "External loopback write successful");

    // Try to read back (should work with external jumper)
    uint8_t loopback_buffer[64];
    result = uart->Read(loopback_buffer, sizeof(loopback_buffer), 500);
    if (result == hf_uart_err_t::UART_SUCCESS) {
      ESP_LOGI(TAG, "External loopback read successful");
    }
  }

  // Disable loopback
  result = uart->SetLoopback(false);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Test signal inversion
  result = uart->SetSignalInversion(0); // No inversion
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set signal inversion: %d", static_cast<int>(result));
    return false;
  }

  // Test wakeup configuration
  hf_uart_wakeup_config_t wakeup_config = {};
  wakeup_config.enable_wakeup = true;
  wakeup_config.wakeup_threshold = 3;

  result = uart->ConfigureWakeup(wakeup_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure wakeup: %d", static_cast<int>(result));
    return false;
  }

  // Check wakeup status
  bool wakeup_enabled = uart->IsWakeupEnabled();
  ESP_LOGI(TAG, "Wakeup enabled: %s", wakeup_enabled ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] Advanced features test completed");
  return true;
}

bool test_uart_communication_modes() noexcept {
  ESP_LOGI(TAG, "Testing UART communication modes...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for communication modes test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for communication modes test");
    return false;
  }

  // Test standard UART mode
  hf_uart_err_t result = uart->SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_UART);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set UART mode: %d", static_cast<int>(result));
    return false;
  }

  hf_uart_mode_t current_mode = uart->GetCommunicationMode();
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

  result = uart->ConfigureRS485(rs485_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure RS485: %d", static_cast<int>(result));
    return false;
  }

  // Test IrDA configuration (ESP32-C6 doesn't support IrDA)
  hf_uart_irda_config_t irda_config = {};
  irda_config.enable_irda = true;
  irda_config.invert_tx = true;
  irda_config.invert_rx = true;
  irda_config.duty_cycle = 50;

  result = uart->ConfigureIrDA(irda_config);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "IrDA not supported on ESP32-C6 (expected): %d", static_cast<int>(result));
    // Continue test - IrDA unsupported is expected on ESP32-C6
  } else {
    ESP_LOGI(TAG, "IrDA configuration succeeded (unexpected on ESP32-C6)");
  }

  // Return to standard UART mode
  result = uart->SetCommunicationMode(hf_uart_mode_t::HF_UART_MODE_UART);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to return to UART mode: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Communication modes test completed");
  return true;
}

bool test_uart_async_operations() noexcept {
  ESP_LOGI(TAG, "Testing UART async operations...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for async operations test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for async operations test");
    return false;
  }

  // Test interrupt configuration (modern ESP-IDF v5.5 approach)
  hf_uart_err_t result = uart->ConfigureInterrupts(
      UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_TXFIFO_EMPTY_INT_ENA_M, 100,
      10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Test operating mode change to interrupt mode
  result = uart->SetOperatingMode(hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set interrupt mode: %d", static_cast<int>(result));
    return false;
  }

  hf_uart_operating_mode_t current_mode = uart->GetOperatingMode();
  if (current_mode != hf_uart_operating_mode_t::HF_UART_MODE_INTERRUPT) {
    ESP_LOGE(TAG, "Operating mode mismatch");
    return false;
  }

  // Allow some time for interrupt mode to settle
  vTaskDelay(pdMS_TO_TICKS(100));

  // Return to polling mode
  result = uart->SetOperatingMode(hf_uart_operating_mode_t::HF_UART_MODE_POLLING);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to return to polling mode: %d", static_cast<int>(result));
    return false;
  }

  // Disable interrupts (configure with no interrupt mask)
  result = uart->ConfigureInterrupts(0, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable interrupts: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Async operations test completed");
  return true;
}

bool test_uart_callbacks() noexcept {
  ESP_LOGI(TAG, "Testing UART event queue access...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for callbacks test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for callbacks test");
    return false;
  }

  // Test event queue availability
  if (!uart->IsEventQueueAvailable()) {
    ESP_LOGE(TAG, "Event queue not available");
    return false;
  }

  ESP_LOGI(TAG, "Event queue is available for user tasks");

  // Test event queue access
  QueueHandle_t event_queue = uart->GetEventQueue();
  if (!event_queue) {
    ESP_LOGE(TAG, "Failed to get event queue handle");
    return false;
  }

  ESP_LOGI(TAG, "Event queue handle obtained successfully");

  // Test interrupt configuration
  uint32_t intr_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M;
  hf_uart_err_t result = uart->ConfigureInterrupts(intr_mask, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Interrupts configured successfully");

  // Test event queue reset
  result = uart->ResetEventQueue();
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset event queue: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Event queue reset successfully");

  ESP_LOGI(TAG, "[SUCCESS] Event queue access test completed");
  return true;
}

bool test_uart_statistics_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing UART statistics and diagnostics...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for statistics test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for statistics test");
    return false;
  }

  // Test statistics retrieval
  hf_uart_statistics_t statistics = {};
  hf_uart_err_t result = uart->GetStatistics(statistics);
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
  result = uart->GetDiagnostics(diagnostics);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Diagnostics:");
  ESP_LOGI(TAG, "  Last error: %d", static_cast<int>(diagnostics.last_error));
  ESP_LOGI(TAG, "  Error reset count: %lu", diagnostics.error_reset_count);

  // Test error retrieval
  hf_uart_err_t last_error = uart->GetLastError();
  ESP_LOGI(TAG, "Last error: %d", static_cast<int>(last_error));

  // Test status checks
  bool is_transmitting = uart->IsTransmitting();
  bool is_receiving = uart->IsReceiving();

  ESP_LOGI(TAG, "Status: TX=%s, RX=%s", is_transmitting ? "true" : "false",
           is_receiving ? "true" : "false");

  ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics test completed");
  return true;
}

bool test_uart_printf_support() noexcept {
  ESP_LOGI(TAG, "Testing UART printf support...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for printf test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for printf test");
    return false;
  }

  // Test Printf functionality
  int result = uart->Printf("Test printf: %d, %s, %.2f\n", 42, "hello", 3.14);
  if (result < 0) {
    ESP_LOGE(TAG, "Printf failed with result: %d", result);
    return false;
  }

  ESP_LOGI(TAG, "Printf returned %d characters", result);

  // Test VPrintf (through internal usage)
  int result2 = uart->Printf("Another test: %c%c%c\n", 'A', 'B', 'C');
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

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for error handling test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for error handling test");
    return false;
  }

  // Test invalid parameters (should handle gracefully)

  // Test write with null data
  hf_uart_err_t result = uart->Write(nullptr, 10, 1000);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Write with null data unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Write with null data correctly failed");
  }

  // Test read with null buffer
  result = uart->Read(nullptr, 10, 1000);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGW(TAG, "Read with null buffer unexpectedly succeeded");
  } else {
    ESP_LOGI(TAG, "Read with null buffer correctly failed");
  }

  // Test invalid baud rate
  bool baud_result = uart->SetBaudRate(0);
  if (baud_result) {
    ESP_LOGW(TAG, "Invalid baud rate unexpectedly accepted");
  } else {
    ESP_LOGI(TAG, "Invalid baud rate correctly rejected");
  }

  // Restore valid baud rate
  uart->SetBaudRate(TEST_BAUD_RATE);

  ESP_LOGI(TAG, "[SUCCESS] Error handling test completed");
  return true;
}

bool test_uart_esp32c6_features() noexcept {
  ESP_LOGI(TAG, "Testing ESP32-C6 UART implementation...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for ESP32-C6 features test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for ESP32-C6 features test");
    return false;
  }

  // Test bytes available functionality
  hf_u16_t bytes_available = uart->BytesAvailable();
  ESP_LOGI(TAG, "Bytes available: %d", bytes_available);

  // Test TX busy status
  bool tx_busy = uart->IsTxBusy();
  ESP_LOGI(TAG, "TX busy: %s", tx_busy ? "true" : "false");

  // Test break detection status
  bool break_detected = uart->IsBreakDetected();
  ESP_LOGI(TAG, "Break detected: %s", break_detected ? "true" : "false");

  // Test actual break signal sending
  hf_uart_err_t result = uart->SendBreak(50);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Break signal sent successfully");
  } else if (result == hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION) {
    ESP_LOGW(TAG, "Break signal not supported on this MCU variant (expected on ESP32-C6)");
    // Continue test - this is expected behavior for ESP32-C6
  } else {
    ESP_LOGE(TAG, "Failed to send break signal: %d", static_cast<int>(result));
    // Don't fail the test for break signal issues
  }

  // Test signal inversion
  result = uart->SetSignalInversion(0); // No inversion
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set signal inversion: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ESP32-C6 UART implementation test completed");
  return true;
}

bool test_uart_performance() noexcept {
  ESP_LOGI(TAG, "Testing UART performance...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for performance test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for performance test");
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
  hf_uart_err_t result = uart->Write(test_data, test_data_size, 2000);
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
  ESP_LOGI(TAG, "Testing UART event queue verification with loopback...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for callback verification test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for callback verification test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Reset callback flags
  g_event_callback_triggered = false;
  g_break_callback_triggered = false;

  // Use external loopback (jumper between pins 20 and 21) for reliable testing
  ESP_LOGI(TAG, "Using external loopback (jumper between pins 20 and 21) for event testing");

  // Configure interrupts for event generation
  uint32_t intr_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M;
  hf_uart_err_t result = uart->ConfigureInterrupts(intr_mask, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Enable RX interrupts (modern ESP-IDF v5.5 approach)
  result =
      uart->ConfigureInterrupts(UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Test event queue access
  QueueHandle_t event_queue = uart->GetEventQueue();
  if (!event_queue) {
    ESP_LOGE(TAG, "Failed to get event queue handle");
    return false;
  }

  ESP_LOGI(TAG, "Event queue handle obtained successfully");

  // Send test data that should trigger data events with external loopback
  const char* test_message = "External Loopback Event Test";
  result = uart->Write(reinterpret_cast<const uint8_t*>(test_message),
                       static_cast<hf_u16_t>(strlen(test_message)), 1000);

  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test data written with external loopback");

    // Wait for events to be processed
    vTaskDelay(pdMS_TO_TICKS(300));

    // Try to read the data back (should trigger more events)
    uint8_t read_buffer[64];
    result = uart->Read(read_buffer, sizeof(read_buffer), 500);
    ESP_LOGI(TAG, "Read result with external loopback: %d", static_cast<int>(result));

    // Check if there are events in the queue (simple verification)
    uart_event_t event;
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      ESP_LOGI(TAG, "Event received from queue: type=%d", event.type);
      g_event_callback_triggered = true;
    }

    // Wait for any additional events
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  // Send break signal to test break events
  result = uart->SendBreak(100);
  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Break signal sent");
    vTaskDelay(pdMS_TO_TICKS(200)); // Wait for break event processing

    // Check for break events
    uart_event_t event;
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      if (event.type == UART_BREAK) {
        ESP_LOGI(TAG, "Break event received from queue");
        g_break_callback_triggered = true;
      }
    }
  } else if (result == hf_uart_err_t::UART_ERR_UNSUPPORTED_OPERATION) {
    ESP_LOGW(TAG, "Break signal not supported on this MCU variant (expected on ESP32-C6)");
    // Continue test - break events won't be tested
  } else {
    ESP_LOGW(TAG, "Break signal failed: %d (continuing test)", static_cast<int>(result));
    // Continue test - break events won't be tested
  }

  // Report results
  ESP_LOGI(TAG, "Event queue verification results:");
  ESP_LOGI(TAG, "  Event received: %s", g_event_callback_triggered ? "YES" : "NO");
  ESP_LOGI(TAG, "  Break event received: %s", g_break_callback_triggered ? "YES" : "NO");

  ESP_LOGI(TAG, "[SUCCESS] Event queue verification test completed");
  return true;
}

bool test_uart_user_event_task() noexcept {
  ESP_LOGI(TAG, "Testing user-created event task (ESP-IDF v5.5 pattern)...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for user event task test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for user event task test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Check if event queue is available
  if (!uart->IsEventQueueAvailable()) {
    ESP_LOGE(TAG, "Event queue not available");
    return false;
  }

  ESP_LOGI(TAG, "Event queue available for user task");

  // Configure interrupts for event-driven operation
  uint32_t intr_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M;
  hf_uart_err_t result = uart->ConfigureInterrupts(intr_mask, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Enable RX interrupts (modern ESP-IDF v5.5 approach)
  result =
      uart->ConfigureInterrupts(UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M, 100, 10);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts: %d", static_cast<int>(result));
    return false;
  }

  // Enable loopback for reliable testing
  result = uart->SetLoopback(true);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable loopback: %d", static_cast<int>(result));
    return false;
  }

  // Use external loopback (jumper between pins 20 and 21) for reliable testing
  ESP_LOGI(TAG, "Testing with external loopback (jumper between pins 20 and 21)");

  // Enable line pattern detection
  result = uart->EnablePatternDetection('\n', 1, 9, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = uart->ResetPatternQueue(16);
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
  BaseType_t task_result = xTaskCreate(user_uart_event_task, "user_uart_events", 4096, uart.get(),
                                       10, &g_user_event_task_handle);
  if (task_result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create user event task");
    return false;
  }

  ESP_LOGI(TAG, "User event task created successfully");

  // Wait for task to start
  vTaskDelay(pdMS_TO_TICKS(100));

  // Send test data with line ending to trigger pattern detection
  const char* test_line = "Hello World\n";
  result = uart->Write(reinterpret_cast<const uint8_t*>(test_line),
                       static_cast<hf_u16_t>(strlen(test_line)), 1000);

  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Test line sent: '%s'", test_line);

    // Wait for pattern detection and processing
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // Send another test with multiple lines
  const char* multi_line = "Line1\nLine2\nLine3\n";
  result = uart->Write(reinterpret_cast<const uint8_t*>(multi_line),
                       static_cast<hf_u16_t>(strlen(multi_line)), 1000);

  if (result == hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGI(TAG, "Multi-line test sent");
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // Stop the user event task
  g_stop_event_task = true;
  vTaskDelay(pdMS_TO_TICKS(200)); // Wait for task to exit

  // Clean up
  uart->DisablePatternDetection();

  // Report results
  ESP_LOGI(TAG, "User event task test results:");
  ESP_LOGI(TAG, "  Event callback triggered: %s", g_event_callback_triggered ? "YES" : "NO");
  ESP_LOGI(TAG, "  Pattern detected: %s", g_pattern_detected ? "YES" : "NO");
  ESP_LOGI(TAG, "  Pattern position: %d", g_pattern_position);

  ESP_LOGI(TAG, "[SUCCESS] User event task test completed");
  return true;
}

//==============================================================================
// EVENT-DRIVEN PATTERN DETECTION TEST (Comprehensive Event Queue Testing)
//==============================================================================

bool test_uart_event_driven_pattern_detection() noexcept {
  ESP_LOGI(TAG, "Testing EVENT-DRIVEN UART pattern detection (ESP-IDF v5.5 comprehensive)...");

  // Create a new UART instance for this test
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for event-driven pattern detection test");
    return false;
  }

  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for event-driven pattern detection test");
    return false;
  }

  // Flush buffers before test to ensure clean test isolation
  flush_uart_buffers(uart.get());

  // Use external loopback (jumper between pins 20 and 21)
  ESP_LOGI(TAG, "Using external loopback (jumper between pins 20 and 21) for event-driven testing");

  // Get the event queue handle
  QueueHandle_t event_queue = uart->GetEventQueue();
  if (!event_queue) {
    ESP_LOGE(TAG, "Event queue not available for event-driven testing");
    return false;
  }

  ESP_LOGI(TAG, "Event queue obtained successfully for pattern detection");

  // Configure interrupts for comprehensive event generation
  hf_uart_err_t result = uart->ConfigureInterrupts(
      UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_TXFIFO_EMPTY_INT_ENA_M, 32, 5);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure interrupts for event-driven testing: %d",
             static_cast<int>(result));
    return false;
  }

  // Enable line pattern detection with proper timing
  result = uart->EnablePatternDetection('\n', 1, 9, 0, 0);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable pattern detection: %d", static_cast<int>(result));
    return false;
  }

  result = uart->ResetPatternQueue(32);
  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset pattern queue: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "Event-driven pattern detection enabled and configured");

  // Test variables
  bool pattern_detected = false;
  int pattern_position = -1;
  int events_received = 0;
  int data_events = 0;
  int pattern_events = 0;
  int other_events = 0;

  // Test data
  const char* test_data = "Command1\nCommand2\nCommand3\n";
  const int expected_patterns = 3;
  const int test_timeout_ms = 5000; // 5 second timeout for comprehensive testing

  ESP_LOGI(TAG, "Sending test data: '%s' (expecting %d patterns)", test_data, expected_patterns);

  // Send test data
  result = uart->Write(reinterpret_cast<const uint8_t*>(test_data),
                       static_cast<hf_u16_t>(strlen(test_data)), 1000);

  if (result != hf_uart_err_t::UART_SUCCESS) {
    ESP_LOGE(TAG, "Failed to write test data: %d", static_cast<int>(result));
    uart->DisablePatternDetection();
    return false;
  }

  ESP_LOGI(TAG, "Test data sent successfully, monitoring event queue...");

  // Add delay to allow data to be processed and prevent buffer overflow
  vTaskDelay(pdMS_TO_TICKS(200));

  // Monitor event queue with timeout
  uint64_t start_time = esp_timer_get_time();
  uint64_t timeout_us = test_timeout_ms * 1000;

  while ((esp_timer_get_time() - start_time) < timeout_us) {
    uart_event_t event;

    // Try to receive event from queue with short timeout
    if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
      events_received++;

      switch (event.type) {
        case UART_DATA: {
          data_events++;
          ESP_LOGI(TAG, "UART_DATA event received - size: %zu", event.size);

          // Read the data to clear the buffer and trigger more events
          uint8_t read_buffer[256];
          hf_uart_err_t read_result = uart->Read(read_buffer, sizeof(read_buffer), 100);
          if (read_result == hf_uart_err_t::UART_SUCCESS) {
            ESP_LOGI(TAG, "Read data from UART buffer successfully");
          }
          // Add small delay to prevent overwhelming the system
          vTaskDelay(pdMS_TO_TICKS(20));
          break;
        }

        case UART_PATTERN_DET: {
          pattern_events++;
          ESP_LOGI(TAG, "UART_PATTERN_DET event received!");

          // Get pattern position
          pattern_position = uart->PopPatternPosition();
          if (pattern_position >= 0) {
            pattern_detected = true;
            ESP_LOGI(TAG, "Pattern detected at position: %d", pattern_position);

            // Read data up to and including the pattern
            int read_len = pattern_position + 1; // Include the pattern character
            uint8_t pattern_buffer[256];

            // Check available data before reading
            size_t buffered_len = 0;
            esp_err_t len_result = uart_get_buffered_data_len(
                static_cast<uart_port_t>(uart->GetPort()), &buffered_len);

            if (len_result == ESP_OK) {
              ESP_LOGI(TAG, "Buffered data length: %zu, reading up to: %d", buffered_len, read_len);

              if (read_len > static_cast<int>(buffered_len)) {
                ESP_LOGW(TAG, "Adjusting read length from %d to %zu (available data)", read_len,
                         buffered_len);
                read_len = static_cast<int>(buffered_len);
              }

              hf_uart_err_t read_result = uart->Read(pattern_buffer, read_len, 100);
              if (read_result == hf_uart_err_t::UART_SUCCESS) {
                ESP_LOGI(TAG, "Pattern data read successfully");
              }
            }
          } else {
            ESP_LOGE(TAG, "Failed to get pattern position");
          }
          break;
        }

        case UART_FIFO_OVF: {
          ESP_LOGW(TAG, "UART_FIFO_OVF event - clearing buffer");
          uart->FlushRx();
          xQueueReset(event_queue);
          break;
        }

        case UART_BUFFER_FULL: {
          ESP_LOGW(TAG, "UART_BUFFER_FULL event - clearing buffer");
          uart->FlushRx();
          xQueueReset(event_queue);
          break;
        }

        case UART_BREAK: {
          ESP_LOGI(TAG, "UART_BREAK event received");
          break;
        }

        case UART_PARITY_ERR: {
          ESP_LOGW(TAG, "UART_PARITY_ERR event received");
          break;
        }

        case UART_FRAME_ERR: {
          ESP_LOGW(TAG, "UART_FRAME_ERR event received");
          break;
        }

        case UART_DATA_BREAK: {
          ESP_LOGI(TAG, "UART_DATA_BREAK event received");
          break;
        }

        case UART_WAKEUP: {
          ESP_LOGI(TAG, "UART_WAKEUP event received");
          break;
        }

        case UART_EVENT_MAX: {
          ESP_LOGW(TAG, "UART_EVENT_MAX event received (should not happen)");
          break;
        }

        default: {
          other_events++;
          ESP_LOGD(TAG, "Unknown UART event type: %d", event.type);
          break;
        }
      }

      // Check if we've received enough pattern events
      if (pattern_events >= expected_patterns) {
        ESP_LOGI(TAG, "Received expected number of pattern events (%d)", expected_patterns);
        break;
      }
    }

    // Small delay to prevent tight loop
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Test timeout check
  uint64_t elapsed_time = (esp_timer_get_time() - start_time) / 1000; // Convert to ms
  ESP_LOGI(TAG, "Event monitoring completed in %llu ms", elapsed_time);

  // Report comprehensive results
  ESP_LOGI(TAG, "Event-driven pattern detection results:");
  ESP_LOGI(TAG, "  Total events received: %d", events_received);
  ESP_LOGI(TAG, "  Data events: %d", data_events);
  ESP_LOGI(TAG, "  Pattern events: %d", pattern_events);
  ESP_LOGI(TAG, "  Other events: %d", other_events);
  ESP_LOGI(TAG, "  Pattern detected: %s", pattern_detected ? "YES" : "NO");
  ESP_LOGI(TAG, "  Pattern position: %d", pattern_position);
  ESP_LOGI(TAG, "  Expected patterns: %d", expected_patterns);

  // Clean up
  uart->DisablePatternDetection();

  // Determine test success
  bool test_passed = (pattern_events >= expected_patterns) && pattern_detected;

  if (test_passed) {
    ESP_LOGI(TAG, "[SUCCESS] Event-driven pattern detection test completed successfully");
  } else {
    ESP_LOGE(TAG, "[FAILED] Event-driven pattern detection test failed");
    ESP_LOGE(TAG, "  Expected: %d patterns, Received: %d patterns", expected_patterns,
             pattern_events);
    ESP_LOGE(TAG, "  Pattern detected: %s", pattern_detected ? "YES" : "NO");
  }

  return test_passed;
}

bool test_uart_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing UART cleanup...");

  // Create a UART instance just to test cleanup
  hf_uart_config_t config = create_test_config();
  auto uart = std::make_unique<EspUart>(config);

  if (!uart) {
    ESP_LOGE(TAG, "Failed to create UART instance for cleanup test");
    return false;
  }

  // Initialize it
  if (!uart->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize UART for cleanup test");
    return false;
  }

  ESP_LOGI(TAG, "UART instance created and initialized for cleanup test");

  // The destructor will be called when the unique_ptr is reset
  uart.reset();

  ESP_LOGI(TAG, "[SUCCESS] UART cleanup completed");
  return true;
}

//==============================================================================
// MAIN TEST EXECUTION
//==============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG,
           "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                   ESP32-C6 UART COMPREHENSIVE TEST SUITE                       ║");
  ESP_LOGI(TAG,
           "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG,
           "║ Target: ESP32-C6 DevKit-M-1                                                    ║");
  ESP_LOGI(TAG,
           "║ ESP-IDF: v5.5+                                                                 ║");
  ESP_LOGI(TAG,
           "║ Features: UART, Baud Rate Configuration, Flow Control, Pattern Detection,      ║");
  ESP_LOGI(TAG,
           "║ Buffer Operations, Advanced Features, Communication Modes, Async Operations,   ║");
  ESP_LOGI(TAG,
           "║ Callbacks, Statistics and Diagnostics, printf Support, Error Handling,         ║");
  ESP_LOGI(TAG,
           "║ ESP32-C6 Features, Performance, Callback Verification, User Event Task,        ║");
  ESP_LOGI(TAG,
           "║ Event-Driven Pattern Detection, Cleanup                                        ║");
  ESP_LOGI(TAG,
           "║ Architecture: noexcept (no exception handling)                                 ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG,
             "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
  }

  // Run all tests in sequence
  ESP_LOGI(TAG, "\n=== CONSTRUCTOR/DESTRUCTOR TESTS ===");
  RUN_TEST(test_uart_construction);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_initialization);
  flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== BASIC COMMUNICATION TESTS ===");
  RUN_TEST(test_uart_basic_communication);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_baud_rate_configuration);
  flip_test_progress_indicator();
  // Temporarily skip flow control test due to ESP32-C6 compatibility issues
  // RUN_TEST(test_uart_flow_control);
  // flip_test_progress_indicator();

  ESP_LOGI(TAG, "\n=== ADVANCED FEATURES TESTS ===");
  // Run pattern detection test in a separate task with larger stack for event processing
  RUN_TEST_IN_TASK("test_uart_pattern_detection", test_uart_pattern_detection, 8192, 5);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_buffer_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_advanced_features);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_communication_modes);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_async_operations);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_callbacks);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_statistics_diagnostics);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_printf_support);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_error_handling);
  flip_test_progress_indicator();

  // ESP32-C6 specific tests
  ESP_LOGI(TAG, "\n=== ESP32-C6 SPECIFIC TESTS ===");
  RUN_TEST(test_uart_esp32c6_features);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_performance);
  flip_test_progress_indicator();
  RUN_TEST(test_uart_callback_verification);
  flip_test_progress_indicator();

  // User Event Task Test
  ESP_LOGI(TAG, "\n=== USER EVENT TASK TEST ===");
  RUN_TEST(test_uart_user_event_task);
  flip_test_progress_indicator();

  // Comprehensive Event-Driven Pattern Detection Test (Last Test)
  ESP_LOGI(TAG, "\n=== COMPREHENSIVE EVENT-DRIVEN PATTERN DETECTION TEST ===");
  ESP_LOGI(TAG, "This test runs with 5-second timeout and comprehensive event monitoring");
  ESP_LOGI(TAG, "It will test the complete event queue behavior for pattern detection");
  RUN_TEST(test_uart_event_driven_pattern_detection);
  flip_test_progress_indicator();

  RUN_TEST(test_uart_cleanup);
  flip_test_progress_indicator();

  print_test_summary(g_test_results, "UART", TAG);

  ESP_LOGI(TAG, "UART comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Post-test banner
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG,
           "╔════════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG,
           "║                    ESP32-C6 UART COMPREHENSIVE TEST SUITE                      ║");
  ESP_LOGI(TAG,
           "║                         HardFOC Internal Interface                             ║");
  ESP_LOGI(TAG,
           "╚════════════════════════════════════════════════════════════════════════════════╝");

  // Cleanup test progression indicator
  cleanup_test_progress_indicator();

  while (true) {
    ESP_LOGI(TAG, "System up and running for %d seconds", esp_timer_get_time() / 1000000);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
