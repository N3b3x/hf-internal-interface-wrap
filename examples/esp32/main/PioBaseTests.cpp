/**
 * @file PioBaseTests.cpp
 * @brief Base PIO tests for ESP32 variants with improved channel validation and resolution_hz
 *
 * This test suite validates the fundamental improvements made to the ESP32 PIO implementation:
 * - Channel-specific callback system
 * - Proper resolution_hz usage instead of resolution_ns
 * - ESP32 variant-specific channel validation (TX/RX allocation)
 * - Enhanced clock divider calculation
 * - Channel configuration validation
 *
 * @note Tests are designed to work across all ESP32 variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2)
 * @note Includes channel allocation validation specific to each ESP32 variant
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BasePio.h"
#include "mcu/esp32/EspPio.h"
#include "mcu/esp32/utils/EspTypes_PIO.h"

static const char* TAG = "PIO_BaseTests";
static TestResults g_test_results;

//==============================================================================
// TEST CONFIGURATION
//==============================================================================

// Test GPIO pins - using safe GPIO pins available on all ESP32 variants
static constexpr hf_pin_num_t TEST_GPIO_SAFE = 2;   // GPIO2 - generally safe across variants
static constexpr hf_pin_num_t TEST_GPIO_ALT = 4;    // GPIO4 - alternative safe pin

// Test resolutions for different scenarios
static constexpr uint32_t TEST_RESOLUTION_HIGH = 8000000;   // 8 MHz (125ns ticks)
static constexpr uint32_t TEST_RESOLUTION_MID = 1000000;    // 1 MHz (1µs ticks) 
static constexpr uint32_t TEST_RESOLUTION_LOW = 100000;     // 100 kHz (10µs ticks)

//==============================================================================
// CALLBACK TEST INFRASTRUCTURE
//==============================================================================

struct CallbackTestData {
    uint8_t channel_id;
    size_t callback_count;
    bool last_callback_success;
    hf_pio_err_t last_error;
    const char* description;
};

static CallbackTestData g_callback_data[HF_RMT_MAX_CHANNELS] = {};

void TestTransmitCallback(hf_u8_t channel_id, size_t symbols_sent, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data) {
        data->callback_count++;
        data->last_callback_success = true;
        ESP_LOGI(TAG, "TX Callback: Channel %d (%s) sent %zu symbols (count: %zu)", 
                 channel_id, data->description, symbols_sent, data->callback_count);
    }
}

void TestReceiveCallback(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, 
                        size_t symbol_count, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data) {
        data->callback_count++;
        data->last_callback_success = true;
        ESP_LOGI(TAG, "RX Callback: Channel %d (%s) received %zu symbols (count: %zu)", 
                 channel_id, data->description, symbol_count, data->callback_count);
    }
}

void TestErrorCallback(hf_u8_t channel_id, hf_pio_err_t error, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data) {
        data->callback_count++;
        data->last_error = error;
        ESP_LOGE(TAG, "Error Callback: Channel %d (%s) error %s (count: %zu)", 
                 channel_id, data->description, HfPioErrToString(error).data(), data->callback_count);
    }
}

//==============================================================================
// ESP32 VARIANT INFORMATION TESTS
//==============================================================================

bool test_esp32_variant_detection() noexcept {
    ESP_LOGI(TAG, "Testing ESP32 variant detection...");
    
    const char* variant_name = HfRmtGetVariantName();
    ESP_LOGI(TAG, "Detected ESP32 variant: %s", variant_name);
    
    ESP_LOGI(TAG, "Channel allocation for %s:", variant_name);
    ESP_LOGI(TAG, "  Total channels: %d", HF_RMT_MAX_CHANNELS);
    ESP_LOGI(TAG, "  TX channels: %d (range: %d-%d)", 
             HF_RMT_MAX_TX_CHANNELS,
             HF_RMT_TX_CHANNEL_START, 
             HF_RMT_TX_CHANNEL_START + HF_RMT_MAX_TX_CHANNELS - 1);
    ESP_LOGI(TAG, "  RX channels: %d (range: %d-%d)", 
             HF_RMT_MAX_RX_CHANNELS,
             HF_RMT_RX_CHANNEL_START, 
             HF_RMT_RX_CHANNEL_START + HF_RMT_MAX_RX_CHANNELS - 1);
    
    if (strlen(variant_name) == 0) {
        ESP_LOGE(TAG, "Variant name is empty");
        return false;
    }
    
    ESP_LOGI(TAG, "[SUCCESS] ESP32 variant detection completed");
    return true;
}

bool test_channel_allocation_helpers() noexcept {
    ESP_LOGI(TAG, "Testing channel allocation helper functions...");
    
    // Test TX channel helpers
    for (uint8_t i = 0; i < HF_RMT_MAX_TX_CHANNELS; i++) {
        int8_t tx_channel = HfRmtGetTxChannel(i);
        if (tx_channel < 0) {
            ESP_LOGE(TAG, "Failed to get TX channel for index %d", i);
            return false;
        }
        ESP_LOGI(TAG, "TX index %d -> channel %d", i, tx_channel);
        
        if (!HF_RMT_IS_VALID_TX_CHANNEL(tx_channel)) {
            ESP_LOGE(TAG, "TX channel %d is not valid according to macro", tx_channel);
            return false;
        }
    }
    
    // Test RX channel helpers
    for (uint8_t i = 0; i < HF_RMT_MAX_RX_CHANNELS; i++) {
        int8_t rx_channel = HfRmtGetRxChannel(i);
        if (rx_channel < 0) {
            ESP_LOGE(TAG, "Failed to get RX channel for index %d", i);
            return false;
        }
        ESP_LOGI(TAG, "RX index %d -> channel %d", i, rx_channel);
        
        if (!HF_RMT_IS_VALID_RX_CHANNEL(rx_channel)) {
            ESP_LOGE(TAG, "RX channel %d is not valid according to macro", rx_channel);
            return false;
        }
    }
    
    // Test invalid indices
    if (HfRmtGetTxChannel(HF_RMT_MAX_TX_CHANNELS) != -1) {
        ESP_LOGE(TAG, "Should return -1 for invalid TX channel index");
        return false;
    }
    
    if (HfRmtGetRxChannel(HF_RMT_MAX_RX_CHANNELS) != -1) {
        ESP_LOGE(TAG, "Should return -1 for invalid RX channel index");
        return false;
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Channel allocation helpers working correctly");
    return true;
}

//==============================================================================
// CHANNEL VALIDATION TESTS
//==============================================================================

bool test_channel_direction_validation() noexcept {
    ESP_LOGI(TAG, "Testing channel direction validation for %s...", HfRmtGetVariantName());
    
    // Test TX channel validation
    for (uint8_t ch = 0; ch < HF_RMT_MAX_CHANNELS; ch++) {
        bool is_valid_tx = HfRmtIsChannelValidForDirection(ch, hf_pio_direction_t::Transmit);
        bool macro_valid_tx = HF_RMT_IS_VALID_TX_CHANNEL(ch);
        
        if (is_valid_tx != macro_valid_tx) {
            ESP_LOGE(TAG, "TX validation mismatch for channel %d: function=%s, macro=%s", 
                     ch, is_valid_tx ? "valid" : "invalid", macro_valid_tx ? "valid" : "invalid");
            return false;
        }
        
        ESP_LOGI(TAG, "Channel %d TX: %s", ch, is_valid_tx ? "VALID" : "INVALID");
    }
    
    // Test RX channel validation
    for (uint8_t ch = 0; ch < HF_RMT_MAX_CHANNELS; ch++) {
        bool is_valid_rx = HfRmtIsChannelValidForDirection(ch, hf_pio_direction_t::Receive);
        bool macro_valid_rx = HF_RMT_IS_VALID_RX_CHANNEL(ch);
        
        if (is_valid_rx != macro_valid_rx) {
            ESP_LOGE(TAG, "RX validation mismatch for channel %d: function=%s, macro=%s", 
                     ch, is_valid_rx ? "valid" : "invalid", macro_valid_rx ? "valid" : "invalid");
            return false;
        }
        
        ESP_LOGI(TAG, "Channel %d RX: %s", ch, is_valid_rx ? "VALID" : "INVALID");
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Channel direction validation working correctly");
    return true;
}

bool test_pio_channel_configuration_validation() noexcept {
    ESP_LOGI(TAG, "Testing PIO channel configuration validation...");
    
    EspPio pio;
    if (!pio.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PIO");
        return false;
    }
    
    // Test valid configuration for first TX channel
    uint8_t valid_tx_channel = HfRmtGetTxChannel(0);
    if (valid_tx_channel >= 0) {
        hf_pio_channel_config_t valid_config;
        valid_config.gpio_pin = TEST_GPIO_SAFE;
        valid_config.direction = hf_pio_direction_t::Transmit;
        valid_config.resolution_hz = TEST_RESOLUTION_MID;
        
        hf_pio_err_t result = pio.ConfigureChannel(valid_tx_channel, valid_config);
        if (result != hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Valid TX configuration failed: %s", HfPioErrToString(result).data());
            return false;
        }
        ESP_LOGI(TAG, "Valid TX channel %d configuration: SUCCESS", valid_tx_channel);
    }
    
    // Test invalid configuration: wrong channel for direction
    uint8_t invalid_tx_channel = HfRmtGetRxChannel(0);  // Get RX channel for TX test
    if (invalid_tx_channel >= 0) {
        hf_pio_channel_config_t invalid_config;
        invalid_config.gpio_pin = TEST_GPIO_ALT;
        invalid_config.direction = hf_pio_direction_t::Transmit;  // TX on RX-only channel
        invalid_config.resolution_hz = TEST_RESOLUTION_MID;
        
        hf_pio_err_t result = pio.ConfigureChannel(invalid_tx_channel, invalid_config);
        if (result == hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Invalid TX configuration should have failed but succeeded");
            return false;
        }
        ESP_LOGI(TAG, "Invalid TX channel %d configuration correctly rejected: %s", 
                 invalid_tx_channel, HfPioErrToString(result).data());
    }
    
    // Test invalid resolution
    if (valid_tx_channel >= 0) {
        hf_pio_channel_config_t bad_resolution_config;
        bad_resolution_config.gpio_pin = TEST_GPIO_SAFE;
        bad_resolution_config.direction = hf_pio_direction_t::Transmit;
        bad_resolution_config.resolution_hz = 0;  // Invalid resolution
        
        hf_pio_err_t result = pio.ConfigureChannel(valid_tx_channel + 1 < HF_RMT_MAX_TX_CHANNELS ? 
                                                   valid_tx_channel + 1 : valid_tx_channel, 
                                                   bad_resolution_config);
        if (result == hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Zero resolution should have been rejected");
            return false;
        }
        ESP_LOGI(TAG, "Zero resolution correctly rejected: %s", HfPioErrToString(result).data());
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Channel configuration validation working correctly");
    return true;
}

//==============================================================================
// RESOLUTION AND CLOCK TESTS
//==============================================================================

bool test_resolution_hz_usage() noexcept {
    ESP_LOGI(TAG, "Testing resolution_hz usage and clock calculations...");
    
    EspPio pio;
    if (!pio.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PIO");
        return false;
    }
    
    // Test different resolution configurations
    struct {
        uint32_t resolution_hz;
        const char* description;
    } test_cases[] = {
        {8000000, "8MHz (WS2812 precision)"},
        {1000000, "1MHz (standard precision)"},
        {100000, "100kHz (low precision)"},
        {38000, "38kHz (IR carrier)"},
    };
    
    uint8_t tx_channel = HfRmtGetTxChannel(0);
    if (tx_channel < 0) {
        ESP_LOGE(TAG, "No valid TX channel available");
        return false;
    }
    
    for (const auto& test_case : test_cases) {
        hf_pio_channel_config_t config;
        config.gpio_pin = TEST_GPIO_SAFE;
        config.direction = hf_pio_direction_t::Transmit;
        config.resolution_hz = test_case.resolution_hz;
        
        ESP_LOGI(TAG, "Testing %s (%u Hz)...", test_case.description, test_case.resolution_hz);
        
        hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
        if (result != hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Failed to configure %s: %s", 
                     test_case.description, HfPioErrToString(result).data());
            return false;
        }
        
        ESP_LOGI(TAG, "  %s: SUCCESS", test_case.description);
        
        // Clear the channel for next test
        pio.ClearChannelCallbacks(tx_channel);
    }
    
    ESP_LOGI(TAG, "[SUCCESS] Resolution_hz usage and clock calculations working correctly");
    return true;
}

bool test_resolution_boundary_conditions() noexcept {
    ESP_LOGI(TAG, "Testing resolution boundary conditions...");
    
    EspPio pio;
    if (!pio.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PIO");
        return false;
    }
    
    uint8_t tx_channel = HfRmtGetTxChannel(0);
    if (tx_channel < 0) {
        ESP_LOGE(TAG, "No valid TX channel available");
        return false;
    }
    
    // Test minimum valid resolution
    hf_pio_channel_config_t min_config;
    min_config.gpio_pin = TEST_GPIO_SAFE;
    min_config.direction = hf_pio_direction_t::Transmit;
    min_config.resolution_hz = HF_RMT_MIN_RESOLUTION_HZ;
    
    hf_pio_err_t result = pio.ConfigureChannel(tx_channel, min_config);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Minimum resolution (%u Hz) should be valid: %s", 
                 HF_RMT_MIN_RESOLUTION_HZ, HfPioErrToString(result).data());
        return false;
    }
    ESP_LOGI(TAG, "Minimum resolution (%u Hz): VALID", HF_RMT_MIN_RESOLUTION_HZ);
    
    // Test maximum valid resolution
    hf_pio_channel_config_t max_config;
    max_config.gpio_pin = TEST_GPIO_ALT;
    max_config.direction = hf_pio_direction_t::Transmit;
    max_config.resolution_hz = HF_RMT_MAX_RESOLUTION_HZ;
    
    uint8_t second_tx_channel = HfRmtGetTxChannel(1);
    if (second_tx_channel >= 0) {
        result = pio.ConfigureChannel(second_tx_channel, max_config);
        if (result != hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Maximum resolution (%u Hz) should be valid: %s", 
                     HF_RMT_MAX_RESOLUTION_HZ, HfPioErrToString(result).data());
            return false;
        }
        ESP_LOGI(TAG, "Maximum resolution (%u Hz): VALID", HF_RMT_MAX_RESOLUTION_HZ);
    }
    
    // Test below minimum (should fail)
    hf_pio_channel_config_t below_min_config;
    below_min_config.gpio_pin = TEST_GPIO_SAFE;
    below_min_config.direction = hf_pio_direction_t::Transmit;
    below_min_config.resolution_hz = HF_RMT_MIN_RESOLUTION_HZ - 1;
    
    result = pio.ConfigureChannel(tx_channel, below_min_config);
    if (result == hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Below minimum resolution should have been rejected");
        return false;
    }
    ESP_LOGI(TAG, "Below minimum resolution correctly rejected: %s", HfPioErrToString(result).data());
    
    ESP_LOGI(TAG, "[SUCCESS] Resolution boundary conditions working correctly");
    return true;
}

//==============================================================================
// CALLBACK SYSTEM TESTS
//==============================================================================

bool test_channel_specific_callbacks() noexcept {
    ESP_LOGI(TAG, "Testing channel-specific callback system...");
    
    EspPio pio;
    if (!pio.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PIO");
        return false;
    }
    
    // Reset callback test data
    memset(g_callback_data, 0, sizeof(g_callback_data));
    
    // Configure multiple channels with different callbacks
    uint8_t num_tx_channels = std::min(static_cast<uint8_t>(2), HF_RMT_MAX_TX_CHANNELS);
    
    for (uint8_t i = 0; i < num_tx_channels; i++) {
        uint8_t channel = HfRmtGetTxChannel(i);
        if (channel < 0) continue;
        
        // Setup callback data
        g_callback_data[channel].channel_id = channel;
        g_callback_data[channel].description = (i == 0) ? "Channel_0_Test" : "Channel_1_Test";
        
        // Configure channel
        hf_pio_channel_config_t config;
        config.gpio_pin = TEST_GPIO_SAFE + i;
        config.direction = hf_pio_direction_t::Transmit;
        config.resolution_hz = TEST_RESOLUTION_MID;
        
        hf_pio_err_t result = pio.ConfigureChannel(channel, config);
        if (result != hf_pio_err_t::PIO_SUCCESS) {
            ESP_LOGE(TAG, "Failed to configure channel %d: %s", 
                     channel, HfPioErrToString(result).data());
            return false;
        }
        
        // Set channel-specific callbacks
        pio.SetTransmitCallback(channel, TestTransmitCallback, &g_callback_data[channel]);
        pio.SetErrorCallback(channel, TestErrorCallback, &g_callback_data[channel]);
        
        ESP_LOGI(TAG, "Configured channel %d with callbacks", channel);
    }
    
    // Test clearing specific channel callbacks
    if (num_tx_channels > 1) {
        uint8_t first_channel = HfRmtGetTxChannel(0);
        pio.ClearChannelCallbacks(first_channel);
        ESP_LOGI(TAG, "Cleared callbacks for channel %d", first_channel);
    }
    
    // Test clearing all callbacks
    pio.ClearCallbacks();
    ESP_LOGI(TAG, "Cleared all callbacks");
    
    ESP_LOGI(TAG, "[SUCCESS] Channel-specific callback system working correctly");
    return true;
}

//==============================================================================
// MAIN TEST RUNNER
//==============================================================================

bool run_all_base_tests() noexcept {
    ESP_LOGI(TAG, "Starting PIO Base Tests for %s", HfRmtGetVariantName());
    ESP_LOGI(TAG, "=======================================================");
    
    struct {
        bool (*test_func)();
        const char* test_name;
    } tests[] = {
        {test_esp32_variant_detection, "ESP32 Variant Detection"},
        {test_channel_allocation_helpers, "Channel Allocation Helpers"},
        {test_channel_direction_validation, "Channel Direction Validation"},
        {test_pio_channel_configuration_validation, "PIO Channel Configuration Validation"},
        {test_resolution_hz_usage, "Resolution Hz Usage"},
        {test_resolution_boundary_conditions, "Resolution Boundary Conditions"},
        {test_channel_specific_callbacks, "Channel-Specific Callbacks"},
    };
    
    bool all_passed = true;
    size_t passed_count = 0;
    size_t total_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (const auto& test : tests) {
        ESP_LOGI(TAG, "\n--- Running: %s ---", test.test_name);
        
        bool result = test.test_func();
        if (result) {
            passed_count++;
            ESP_LOGI(TAG, "✓ %s: PASSED", test.test_name);
        } else {
            all_passed = false;
            ESP_LOGE(TAG, "✗ %s: FAILED", test.test_name);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay between tests
    }
    
    ESP_LOGI(TAG, "\n=======================================================");
    ESP_LOGI(TAG, "PIO Base Tests Summary:");
    ESP_LOGI(TAG, "  ESP32 Variant: %s", HfRmtGetVariantName());
    ESP_LOGI(TAG, "  Tests Passed: %zu/%zu", passed_count, total_tests);
    ESP_LOGI(TAG, "  Overall Result: %s", all_passed ? "SUCCESS" : "FAILURE");
    ESP_LOGI(TAG, "=======================================================");
    
    return all_passed;
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "PIO Base Tests Starting...");
    
    // Run all base tests
    bool result = run_all_base_tests();
    
    if (result) {
        ESP_LOGI(TAG, "All PIO base tests completed successfully!");
    } else {
        ESP_LOGE(TAG, "Some PIO base tests failed!");
    }
}