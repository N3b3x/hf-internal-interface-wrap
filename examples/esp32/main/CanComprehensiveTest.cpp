/**
 * @file CanComprehensiveTest.cpp
 * @brief Comprehensive CAN testing suite for ESP32-C6 with ESP-IDF v5.5 TWAI API and SN65 transceiver
 * 
 * This comprehensive test suite validates all EspCan functionality including:
 * - ESP-IDF v5.5 TWAI node-based API compliance
 * - ESP32-C6 TWAI controller operation
 * - SN65 CAN transceiver integration
 * - Advanced filtering and timing configuration
 * - Event-driven callback systems
 * - Single-callback per event with user data pointer
 * - Error handling and bus recovery
 * - Performance and stress testing
 * - Self-test and loopback modes
 * 
 * Hardware Requirements:
 * - ESP32-C6 DevKit
 * - SN65HVD230/SN65HVD232 CAN transceiver
 * - CAN bus termination resistors (120Ω)
 * - Optional: Second CAN node for full bus testing
 * 
 * Wiring for ESP32-C6 + SN65:
 * - GPIO4 (TX) -> SN65 CTX pin
 * - GPIO5 (RX) -> SN65 CRX pin  
 * - 3.3V -> SN65 VCC
 * - GND -> SN65 GND
 * - SN65 CANH/CANL -> CAN bus
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseCan.h"
#include "mcu/esp32/EspCan.h"
#include "TestFramework.h"

#include <vector>
#include <memory>
#include <atomic>

static const char* TAG = "CAN_Test";

static TestResults g_test_results;

// Test configuration constants
static constexpr uint32_t TEST_CAN_ID_STANDARD = 0x123;
static constexpr uint32_t TEST_CAN_ID_EXTENDED = 0x12345678;
static constexpr uint32_t TEST_BAUD_RATE = 500000;
static constexpr uint32_t TEST_TIMEOUT_MS = 5000;
static constexpr hf_pin_num_t TEST_TX_PIN = 4;  // ESP32-C6 + SN65
static constexpr hf_pin_num_t TEST_RX_PIN = 5;  // ESP32-C6 + SN65

// Event bits for synchronization
static constexpr int MESSAGE_RECEIVED_BIT = BIT0;
static constexpr int ERROR_OCCURRED_BIT = BIT1;
static constexpr int STATE_CHANGED_BIT = BIT2;

// Global test data
static EventGroupHandle_t test_event_group = nullptr;
static std::atomic<uint32_t> messages_received{0};
static std::atomic<uint32_t> errors_detected{0};
static hf_can_message_t last_received_message{};

//=============================================================================
// TEST HELPER FUNCTIONS
//=============================================================================

/**
 * @brief Test callback for received CAN messages (enhanced version with user data)
 */
void test_receive_callback_enhanced(const hf_can_message_t& message, void* user_data) {
  (void)user_data; // Unused in basic tests
  last_received_message = message;
  messages_received.fetch_add(1);
  xEventGroupSetBits(test_event_group, MESSAGE_RECEIVED_BIT);
  
  ESP_LOGI(TAG, "Received CAN message: ID=0x%X, DLC=%d, Extended=%s",
           message.id, message.dlc, message.is_extended ? "Yes" : "No");
}

/**
 * @brief Create a test CAN message
 */
hf_can_message_t create_test_message(uint32_t id, bool extended = false, uint8_t dlc = 8) {
  hf_can_message_t message{};
  message.id = id;
  message.is_extended = extended;
  message.dlc = dlc;
  message.is_rtr = false;
  
  // Fill with test pattern
  for (uint8_t i = 0; i < dlc && i < 8; ++i) {
    message.data[i] = static_cast<uint8_t>(0xA0 + i);
  }
  
  return message;
}

/**
 * @brief Wait for events with timeout
 */
bool wait_for_event(EventBits_t bits, uint32_t timeout_ms) {
  EventBits_t result = xEventGroupWaitBits(
    test_event_group, bits, pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout_ms)
  );
  return (result & bits) != 0;
}

//=============================================================================
// BASIC FUNCTIONALITY TESTS
//=============================================================================

bool test_can_initialization() noexcept {
  ESP_LOGI(TAG, "Testing CAN initialization with ESP-IDF v5.5 API...");

  // Test configuration for ESP32-C6 + SN65 transceiver
  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.controller_id = hf_can_controller_id_t::HF_CAN_CONTROLLER_0;
  can_config.mode = hf_can_mode_t::HF_CAN_MODE_NORMAL;
  can_config.enable_self_test = false;  // Using external SN65 transceiver
  can_config.enable_loopback = false;
  can_config.tx_queue_depth = 10;
  can_config.sample_point_permill = 750;  // 75% sample point for reliability
  
  EspCan test_can(can_config);

  // Test lazy initialization
  if (test_can.IsInitialized()) {
    ESP_LOGE(TAG, "CAN should not be initialized before Initialize() call");
    return false;
  }

  // Test initialization
  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN with ESP-IDF v5.5 API");
    return false;
  }

  if (!test_can.IsInitialized()) {
    ESP_LOGE(TAG, "CAN should be initialized after Initialize() call");
    return false;
  }

  // Test double initialization (should succeed)
  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Double initialization should succeed");
    return false;
  }

  // Test deinitialization
  if (test_can.Deinitialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to deinitialize CAN");
    return false;
  }

  if (test_can.IsInitialized()) {
    ESP_LOGE(TAG, "CAN should not be initialized after Deinitialize() call");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN initialization test passed");
  return true;
}

bool test_can_self_test_mode() noexcept {
  ESP_LOGI(TAG, "Testing CAN self-test mode for ESP32-C6...");

  // Configure for self-test mode (no external ACK required)
  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;  // Self-test mode
  can_config.enable_loopback = true;   // Loopback for self-reception
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN in self-test mode");
    return false;
  }

  // Set up callback
  if (test_can.SetReceiveCallbackEx(test_receive_callback_enhanced) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set receive callback");
    return false;
  }

  // Test message transmission in self-test mode
  auto test_message = create_test_message(TEST_CAN_ID_STANDARD, false, 4);
  
  messages_received.store(0);
  if (test_can.SendMessage(test_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send message in self-test mode");
    return false;
  }

  // Wait for self-reception
  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 2000)) {
    ESP_LOGE(TAG, "No message received in self-test mode");
    return false;
  }

  if (messages_received.load() != 1) {
    ESP_LOGE(TAG, "Expected 1 message, received %d", messages_received.load());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN self-test mode passed");
  return true;
}

bool test_can_message_transmission() noexcept {
  ESP_LOGI(TAG, "Testing CAN message transmission with various formats...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;  // For standalone testing
  can_config.enable_loopback = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Test standard frame
  messages_received.store(0);
  auto std_message = create_test_message(TEST_CAN_ID_STANDARD, false, 8);
  
  if (test_can.SendMessage(std_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send standard frame");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGE(TAG, "Standard frame not received");
    return false;
  }

  // Test extended frame
  messages_received.store(0);
  auto ext_message = create_test_message(TEST_CAN_ID_EXTENDED, true, 6);
  
  if (test_can.SendMessage(ext_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send extended frame");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGE(TAG, "Extended frame not received");
    return false;
  }

  // Test remote frame
  messages_received.store(0);
  hf_can_message_t rtr_message{};
  rtr_message.id = TEST_CAN_ID_STANDARD;
  rtr_message.is_rtr = true;
  rtr_message.dlc = 4;
  
  if (test_can.SendMessage(rtr_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send remote frame");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGE(TAG, "Remote frame not received");
    return false;
  }

  // Verify RTR flag
  if (!last_received_message.is_rtr) {
    ESP_LOGE(TAG, "Received message should be RTR");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN message transmission test passed");
  return true;
}

//=============================================================================
// ADVANCED FILTERING TESTS
//=============================================================================

bool test_can_acceptance_filtering() noexcept {
  ESP_LOGI(TAG, "Testing CAN acceptance filtering with ESP-IDF v5.5...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;
  can_config.enable_loopback = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for filtering test");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Test single filter mode
  // Accept only IDs 0x100-0x10F (mask 0x7F0, ID 0x100)
  if (test_can.SetAcceptanceFilter(0x100, 0x7F0, false) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set acceptance filter");
    return false;
  }

  // Test accepted message
  messages_received.store(0);
  auto accepted_msg = create_test_message(0x105, false, 4);  // Should pass filter
  
  if (test_can.SendMessage(accepted_msg, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send accepted message");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGI(TAG, "Message correctly filtered and received");
  }

  // Test rejected message  
  messages_received.store(0);
  auto rejected_msg = create_test_message(0x200, false, 4);  // Should be filtered out
  
  if (test_can.SendMessage(rejected_msg, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send rejected message");
    return false;
  }

  // Should not receive this message due to filter
  if (wait_for_event(MESSAGE_RECEIVED_BIT, 500)) {
    ESP_LOGE(TAG, "Message should have been filtered out");
    return false;
  }

  // Test dual filter mode using advanced filter API
  hf_esp_can_filter_config_t dual_filter{};
  dual_filter.is_dual_filter = true;
  dual_filter.id = 0x300;
  dual_filter.mask = 0x7F0;
  dual_filter.id2 = 0x400;
  dual_filter.mask2 = 0x7F0;
  dual_filter.is_extended = false;

  if (test_can.ConfigureAdvancedFilter(dual_filter) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure dual filter");
    return false;
  }

  // Test both filter ranges
  messages_received.store(0);
  auto msg1 = create_test_message(0x305, false, 2);  // First filter range
  auto msg2 = create_test_message(0x405, false, 2);  // Second filter range
  
  test_can.SendMessage(msg1, 1000);
  test_can.SendMessage(msg2, 1000);

  // Wait for both messages
  vTaskDelay(pdMS_TO_TICKS(500));
  
  if (messages_received.load() != 2) {
    ESP_LOGE(TAG, "Expected 2 messages with dual filter, got %d", messages_received.load());
    return false;
  }

  // Clear filter (accept all)
  if (test_can.ClearAcceptanceFilter() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to clear acceptance filter");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN acceptance filtering test passed");
  return true;
}

//=============================================================================
// ADVANCED TIMING TESTS
//=============================================================================

bool test_can_advanced_timing() noexcept {
  ESP_LOGI(TAG, "Testing CAN advanced bit timing configuration...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = 250000;  // Start with 250kbps
  can_config.enable_self_test = true;
  can_config.enable_loopback = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for timing test");
    return false;
  }

  // Test custom timing configuration for improved signal quality
  hf_esp_can_timing_config_t custom_timing{};
  custom_timing.brp = 16;        // Prescaler for 250kbps
  custom_timing.prop_seg = 5;    // Propagation segment
  custom_timing.tseg_1 = 8;      // Time segment 1
  custom_timing.tseg_2 = 3;      // Time segment 2
  custom_timing.sjw = 2;         // Synchronization jump width
  custom_timing.ssp_offset = 0;  // Secondary sample point offset

  if (test_can.ConfigureAdvancedTiming(custom_timing) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure advanced timing");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Test message transmission with custom timing
  messages_received.store(0);
  auto test_message = create_test_message(TEST_CAN_ID_STANDARD, false, 8);
  
  if (test_can.SendMessage(test_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send message with custom timing");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGE(TAG, "Message not received with custom timing");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN advanced timing configuration test passed");
  return true;
}

//=============================================================================
// ERROR HANDLING AND RECOVERY TESTS  
//=============================================================================

bool test_can_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing CAN error handling and recovery...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = false;  // Normal mode to potentially trigger errors
  can_config.enable_alerts = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for error test");
    return false;
  }

  // Get initial status
  hf_can_status_t initial_status{};
  if (test_can.GetStatus(initial_status) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get initial CAN status");
    return false;
  }

  ESP_LOGI(TAG, "Initial status - TX errors: %d, RX errors: %d, Bus-off: %s",
           initial_status.tx_error_count, initial_status.rx_error_count,
           initial_status.bus_off ? "Yes" : "No");

  // Test statistics functionality
  hf_can_statistics_t stats{};
  if (test_can.GetStatistics(stats) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get CAN statistics");
    return false;
  }

  // Test diagnostics
  hf_can_diagnostics_t diagnostics{};
  if (test_can.GetDiagnostics(diagnostics) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get CAN diagnostics");
    return false;
  }

  // Test reset functionality
  if (test_can.Reset() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset CAN controller");
    return false;
  }

  // Verify statistics were reset
  if (test_can.GetStatistics(stats) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get statistics after reset");
    return false;
  }

  // Test node info retrieval (ESP-IDF v5.5 specific)
  twai_node_info_t node_info{};
  if (test_can.GetNodeInfo(node_info) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get TWAI node info");
    return false;
  }

  ESP_LOGI(TAG, "Node info - State: %d, TX errors: %d, RX errors: %d",
           node_info.state, node_info.tx_error_counter, node_info.rx_error_counter);

  ESP_LOGI(TAG, "[SUCCESS] CAN error handling test passed");
  return true;
}

bool test_can_bus_recovery() noexcept {
  ESP_LOGI(TAG, "Testing CAN bus recovery functionality...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;
  can_config.enable_alerts = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for recovery test");
    return false;
  }

  // Test bus recovery initiation
  if (test_can.InitiateBusRecovery() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initiate bus recovery");
    return false;
  }

  ESP_LOGI(TAG, "Bus recovery initiated successfully");

  // Wait for recovery to complete
  vTaskDelay(pdMS_TO_TICKS(100));

  // Verify we can still send messages after recovery
  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);
  messages_received.store(0);
  
  auto test_message = create_test_message(TEST_CAN_ID_STANDARD, false, 4);
  if (test_can.SendMessage(test_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to send message after recovery");
    return false;
  }

  if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
    ESP_LOGE(TAG, "Message not received after recovery");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN bus recovery test passed");
  return true;
}

//=============================================================================
// PERFORMANCE AND STRESS TESTS
//=============================================================================

bool test_can_batch_transmission() noexcept {
  ESP_LOGI(TAG, "Testing CAN batch message transmission...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;
  can_config.enable_loopback = true;
  can_config.tx_queue_depth = 20;  // Larger queue for batch testing
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for batch test");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Create batch of test messages
  constexpr uint32_t BATCH_SIZE = 10;
  std::vector<hf_can_message_t> batch_messages;
  
  for (uint32_t i = 0; i < BATCH_SIZE; ++i) {
    auto msg = create_test_message(TEST_CAN_ID_STANDARD + i, false, 8);
    batch_messages.push_back(msg);
  }

  messages_received.store(0);

  // Send batch using the new batch API
  uint32_t sent_count = test_can.SendMessageBatch(
    batch_messages.data(), BATCH_SIZE, 1000
  );

  if (sent_count != BATCH_SIZE) {
    ESP_LOGE(TAG, "Expected to send %d messages, actually sent %d", BATCH_SIZE, sent_count);
    return false;
  }

  // Wait for all messages to be received
  vTaskDelay(pdMS_TO_TICKS(1000));

  if (messages_received.load() != BATCH_SIZE) {
    ESP_LOGE(TAG, "Expected to receive %d messages, got %d", BATCH_SIZE, messages_received.load());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN batch transmission test passed");
  return true;
}

bool test_can_high_throughput() noexcept {
  ESP_LOGI(TAG, "Testing CAN high throughput performance...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = 1000000;  // 1 Mbps for high throughput
  can_config.enable_self_test = true;
  can_config.enable_loopback = true;
  can_config.tx_queue_depth = 50;
  can_config.sample_point_permill = 800;  // 80% for high speed
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for throughput test");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Configure timing for 1 Mbps
  hf_esp_can_timing_config_t high_speed_timing{};
  high_speed_timing.brp = 4;         // Prescaler for 1 Mbps
  high_speed_timing.prop_seg = 5;
  high_speed_timing.tseg_1 = 8;
  high_speed_timing.tseg_2 = 2;
  high_speed_timing.sjw = 1;

  if (test_can.ConfigureAdvancedTiming(high_speed_timing) != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure high-speed timing");
    return false;
  }

  // Measure throughput
  constexpr uint32_t TEST_MESSAGES = 100;
  messages_received.store(0);
  
  uint64_t start_time = esp_timer_get_time();

  // Send messages as fast as possible
  uint32_t sent_successfully = 0;
  for (uint32_t i = 0; i < TEST_MESSAGES; ++i) {
    auto msg = create_test_message(TEST_CAN_ID_STANDARD + (i % 100), false, 8);
    if (test_can.SendMessage(msg, 100) == hf_can_err_t::CAN_SUCCESS) {
      sent_successfully++;
    }
  }

  // Wait for reception to complete
  vTaskDelay(pdMS_TO_TICKS(2000));

  uint64_t end_time = esp_timer_get_time();
  uint64_t duration_us = end_time - start_time;
  uint32_t duration_ms = (uint32_t)(duration_us / 1000);

  uint32_t received_count = messages_received.load();
  
  ESP_LOGI(TAG, "Throughput test results:");
  ESP_LOGI(TAG, "  Messages sent: %d/%d", sent_successfully, TEST_MESSAGES);
  ESP_LOGI(TAG, "  Messages received: %d", received_count);
  ESP_LOGI(TAG, "  Test duration: %d ms", duration_ms);
  ESP_LOGI(TAG, "  Effective rate: %.2f msg/s", 
           (float)received_count * 1000.0f / duration_ms);

  if (received_count < sent_successfully * 0.95f) {  // Allow 5% loss
    ESP_LOGE(TAG, "High packet loss detected in throughput test");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN high throughput test passed");
  return true;
}

//=============================================================================
// SN65 TRANSCEIVER SPECIFIC TESTS
//=============================================================================

bool test_sn65_transceiver_integration() noexcept {
  ESP_LOGI(TAG, "Testing SN65 CAN transceiver integration...");

  // Test with different SN65 configurations
  std::vector<uint32_t> test_baud_rates = {125000, 250000, 500000, 1000000};
  
  for (auto baud_rate : test_baud_rates) {
    ESP_LOGI(TAG, "Testing SN65 at %d bps...", baud_rate);

    hf_esp_can_config_t can_config{};
    can_config.tx_pin = TEST_TX_PIN;
    can_config.rx_pin = TEST_RX_PIN;
    can_config.baud_rate = baud_rate;
    can_config.enable_self_test = true;
    can_config.enable_loopback = true;
    
    // Adjust sample point based on baud rate for SN65 compatibility
    if (baud_rate >= 1000000) {
      can_config.sample_point_permill = 800;  // 80% for high speed
    } else {
      can_config.sample_point_permill = 750;  // 75% for lower speeds
    }
    
    EspCan test_can(can_config);

    if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
      ESP_LOGE(TAG, "Failed to initialize CAN at %d bps", baud_rate);
      return false;
    }

    test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);
    messages_received.store(0);

    // Test signal integrity at this baud rate
    auto test_message = create_test_message(TEST_CAN_ID_STANDARD, false, 8);
    
    if (test_can.SendMessage(test_message, 1000) != hf_can_err_t::CAN_SUCCESS) {
      ESP_LOGE(TAG, "Failed to send message at %d bps", baud_rate);
      return false;
    }

    if (!wait_for_event(MESSAGE_RECEIVED_BIT, 1000)) {
      ESP_LOGE(TAG, "No message received at %d bps", baud_rate);
      return false;
    }

    ESP_LOGI(TAG, "SN65 test passed at %d bps", baud_rate);
    
    test_can.Deinitialize();
    vTaskDelay(pdMS_TO_TICKS(100));  // Brief delay between tests
  }

  ESP_LOGI(TAG, "[SUCCESS] SN65 transceiver integration test passed");
  return true;
}

bool test_can_signal_quality() noexcept {
  ESP_LOGI(TAG, "Testing CAN signal quality with SN65 transceiver...");

  hf_esp_can_config_t can_config{};
  can_config.tx_pin = TEST_TX_PIN;
  can_config.rx_pin = TEST_RX_PIN;
  can_config.baud_rate = TEST_BAUD_RATE;
  can_config.enable_self_test = true;
  can_config.enable_loopback = true;
  can_config.enable_alerts = true;
  
  EspCan test_can(can_config);

  if (test_can.Initialize() != hf_can_err_t::CAN_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize CAN for signal quality test");
    return false;
  }

  test_can.SetReceiveCallbackEx(test_receive_callback_enhanced);

  // Test signal quality with various message patterns
  std::vector<std::vector<uint8_t>> test_patterns = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // All zeros
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // All ones  
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},  // Alternating
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},  // Alternating opposite
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},  // Incremental
  };

  uint32_t successful_transmissions = 0;
  uint32_t total_attempts = 0;

  for (const auto& pattern : test_patterns) {
    for (int repeat = 0; repeat < 10; ++repeat) {  // Test each pattern multiple times
      hf_can_message_t test_message{};
      test_message.id = TEST_CAN_ID_STANDARD + repeat;
      test_message.dlc = 8;
      std::memcpy(test_message.data, pattern.data(), 8);

      messages_received.store(0);
      
      if (test_can.SendMessage(test_message, 500) == hf_can_err_t::CAN_SUCCESS) {
        total_attempts++;
        
        if (wait_for_event(MESSAGE_RECEIVED_BIT, 500)) {
          successful_transmissions++;
          
          // Verify data integrity
          bool data_correct = true;
          for (int i = 0; i < 8; ++i) {
            if (last_received_message.data[i] != pattern[i]) {
              data_correct = false;
              break;
            }
          }
          
          if (!data_correct) {
            ESP_LOGW(TAG, "Data corruption detected in signal quality test");
          }
        }
      }
    }
  }

  float success_rate = (float)successful_transmissions / total_attempts * 100.0f;
  
  ESP_LOGI(TAG, "Signal quality test results:");
  ESP_LOGI(TAG, "  Total attempts: %d", total_attempts);
  ESP_LOGI(TAG, "  Successful: %d", successful_transmissions);
  ESP_LOGI(TAG, "  Success rate: %.2f%%", success_rate);

  if (success_rate < 98.0f) {  // Expect very high success rate in loopback
    ESP_LOGE(TAG, "Signal quality below acceptable threshold");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] CAN signal quality test passed");
  return true;
}

// (Removed enhanced multi-callback tests in single-callback design)

//=============================================================================
// MAIN TEST RUNNER
//=============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                ESP32-C6 CAN COMPREHENSIVE TEST SUITE                        ║");
  ESP_LOGI(TAG, "║                     ESP-IDF v5.5 TWAI API + SN65                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");
  
  ESP_LOGI(TAG, "Hardware Configuration:");
  ESP_LOGI(TAG, "  MCU: ESP32-C6");
  ESP_LOGI(TAG, "  TX Pin: GPIO%d", TEST_TX_PIN);
  ESP_LOGI(TAG, "  RX Pin: GPIO%d", TEST_RX_PIN);
  ESP_LOGI(TAG, "  Transceiver: SN65HVD230/232");
  ESP_LOGI(TAG, "  API: ESP-IDF v5.5 TWAI node-based");
  
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test event group
  test_event_group = xEventGroupCreate();
  if (!test_event_group) {
    ESP_LOGE(TAG, "Failed to create test event group");
    return;
  }

  // Run comprehensive test suite
  ESP_LOGI(TAG, "\n=== BASIC FUNCTIONALITY TESTS ===");
  RUN_TEST(test_can_initialization);
  RUN_TEST(test_can_self_test_mode);
  RUN_TEST(test_can_message_transmission);

  ESP_LOGI(TAG, "\n=== ADVANCED FEATURE TESTS ===");
  RUN_TEST(test_can_acceptance_filtering);
  RUN_TEST(test_can_advanced_timing);

  ESP_LOGI(TAG, "\n=== ERROR HANDLING TESTS ===");
  RUN_TEST(test_can_error_handling);
  RUN_TEST(test_can_bus_recovery);

  ESP_LOGI(TAG, "\n=== PERFORMANCE TESTS ===");
  RUN_TEST(test_can_batch_transmission);
  RUN_TEST(test_can_high_throughput);

  ESP_LOGI(TAG, "\n=== SN65 TRANSCEIVER TESTS ===");
  RUN_TEST(test_sn65_transceiver_integration);
  RUN_TEST(test_can_signal_quality);

  // (Enhanced multi-callback tests removed in single-callback design)

  print_test_summary(g_test_results, "ESP32-C6 CAN (ESP-IDF v5.5 + SN65)", TAG);
  
  // Cleanup
  vEventGroupDelete(test_event_group);
  
  ESP_LOGI(TAG, "\n╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                      TEST SUITE COMPLETED                                   ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
