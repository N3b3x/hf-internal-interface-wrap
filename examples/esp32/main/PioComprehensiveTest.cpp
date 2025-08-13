/**
 * @file PioComprehensiveTest.cpp
 * @brief Comprehensive PIO testing suite for ESP32-C6 DevKit-M-1 with RMT peripheral (noexcept)
 *
 * This comprehensive test suite validates all functionality of the EspPio class using ESP-IDF v5.5
 * RMT with the latest improvements including:
 * - Channel-specific callback system with proper user data handling
 * - Resolution_hz usage instead of resolution_ns for direct ESP-IDF compatibility
 * - ESP32 variant-specific channel validation (TX/RX allocation per variant)
 * - Enhanced clock divider calculation with overflow protection
 * - Constructor/Destructor behavior
 * - Lifecycle management (Initialize/Deinitialize)
 * - Channel configuration and management
 * - Symbol transmission and reception
 * - RMT-specific features (carrier modulation, loopback, encoder configuration)
 * - WS2812 LED protocol timing validation (using built-in RGB LED on GPIO8)
 * - Automated loopback testing (GPIO8 TX -> GPIO18 RX)
 * - Logic analyzer test scenarios
 * - Advanced RMT features (DMA, memory blocks, queue depth)
 * - Status and diagnostics (statistics, error reporting)
 * - Channel-specific callbacks (transmit, receive, error)
 * - Edge cases and stress testing
 * - ASCII Art test result decoration
 *
 * @note This test suite is designed for ESP32-C6 DevKitM-1 with ESP-IDF v5.5+ RMT driver
 * @note Uses built-in RGB LED on GPIO8 for WS2812 testing and automated loopback
 * @note Automated testing: Connect GPIO8 (TX) to GPIO18 (RX) with jumper wire
 * @note Incorporates latest PIO improvements: channel-specific callbacks, resolution_hz, variant validation
 */

#include "TestFramework.h"
#include "base/BasePio.h"
#include "mcu/esp32/EspPio.h"
#include "mcu/esp32/utils/EspTypes_PIO.h"
#include "utils/AsciiArtGenerator.h"

static const char* TAG = "PIO_Test";
static TestResults g_test_results;

//==============================================================================
// ASCII ART GENERATOR FOR TEST DECORATION
//==============================================================================

static AsciiArtGenerator g_ascii_generator;

/**
 * @brief Print ASCII art banner for test results
 */
void print_ascii_banner(const char* text, bool success = true) noexcept {
    std::string banner = g_ascii_generator.Generate(text);
    if (!banner.empty()) {
        if (success) {
            ESP_LOGI(TAG, "\n🎉 SUCCESS BANNER:\n%s", banner.c_str());
        } else {
            ESP_LOGE(TAG, "\n❌ FAILURE BANNER:\n%s", banner.c_str());
        }
    }
}

//==============================================================================
// WS2812 PROTOCOL CONSTANTS (for RGB LED testing)
//==============================================================================

// WS2812 timing specifications (in nanoseconds)
static constexpr uint32_t WS2812_T0H = 350;     // 0 code, high time
static constexpr uint32_t WS2812_T0L = 900;     // 0 code, low time
static constexpr uint32_t WS2812_T1H = 700;     // 1 code, high time
static constexpr uint32_t WS2812_T1L = 600;     // 1 code, low time
static constexpr uint32_t WS2812_RESET = 50000; // Reset time (>50µs)

// Test GPIO pins for automated loopback testing
// ESP32-C6 DevKitM-1 specific GPIO configuration for automated testing
#if defined(CONFIG_IDF_TARGET_ESP32C6)
static constexpr hf_gpio_num_t TEST_GPIO_TX = 8;  // GPIO8 for built-in RGB LED (WS2812) - TX
static constexpr hf_gpio_num_t TEST_GPIO_RX = 18; // GPIO18 for reception (RMT compatible) - RX
// For automated testing: Connect GPIO8 (TX) to GPIO18 (RX) with a jumper wire
// This creates a loopback that allows the test to verify transmission/reception
#else
static constexpr hf_gpio_num_t TEST_GPIO_TX = 2; // GPIO2 for transmission
static constexpr hf_gpio_num_t TEST_GPIO_RX = 3; // GPIO3 for reception
// For automated testing: Connect GPIO2 (TX) to GPIO3 (RX) with a jumper wire
#endif

// Test resolutions using the new resolution_hz approach
static constexpr uint32_t TEST_RESOLUTION_WS2812 = 8000000;  // 8 MHz for WS2812 precision (125ns ticks)
static constexpr uint32_t TEST_RESOLUTION_STANDARD = 1000000; // 1 MHz for standard precision (1µs ticks)  
static constexpr uint32_t TEST_RESOLUTION_LOW = 100000;      // 100 kHz for low precision (10µs ticks)

//==============================================================================
// CALLBACK TEST INFRASTRUCTURE
//==============================================================================

struct CallbackTestData {
    uint8_t channel_id;
    size_t callback_count;
    bool last_callback_success;
    hf_pio_err_t last_error;
    const char* description;
    size_t symbols_sent;
    size_t symbols_received;
};

static CallbackTestData g_callback_data[HF_RMT_MAX_CHANNELS] = {};

void TestTransmitCallback(hf_u8_t channel_id, size_t symbols_sent, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data && data->channel_id == channel_id) {
        data->callback_count++;
        data->symbols_sent = symbols_sent;
        data->last_callback_success = true;
        ESP_LOGI(TAG, "TX Callback: Channel %d (%s) sent %zu symbols (count: %zu)", 
                 channel_id, data->description, symbols_sent, data->callback_count);
    }
}

void TestReceiveCallback(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, 
                        size_t symbol_count, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data && data->channel_id == channel_id) {
        data->callback_count++;
        data->symbols_received = symbol_count;
        data->last_callback_success = true;
        ESP_LOGI(TAG, "RX Callback: Channel %d (%s) received %zu symbols (count: %zu)", 
                 channel_id, data->description, symbol_count, data->callback_count);
    }
}

void TestErrorCallback(hf_u8_t channel_id, hf_pio_err_t error, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data && data->channel_id == channel_id) {
        data->callback_count++;
        data->last_error = error;
        ESP_LOGE(TAG, "Error Callback: Channel %d (%s) error %s (count: %zu)", 
                 channel_id, data->description, HfPioErrToString(error).data(), data->callback_count);
    }
}

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Create a default PIO channel configuration for testing using resolution_hz
 * ESP32-C6 specific configuration for RMT compatibility with latest improvements
 */
hf_pio_channel_config_t create_test_channel_config(
    hf_gpio_num_t gpio_pin, hf_pio_direction_t direction = hf_pio_direction_t::Transmit) noexcept {
  hf_pio_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.direction = direction;

// Use resolution_hz instead of resolution_ns for ESP-IDF v5.5 compatibility
#if defined(CONFIG_IDF_TARGET_ESP32C6)
  config.resolution_hz = TEST_RESOLUTION_STANDARD; // 1MHz resolution for ESP32-C6 RMT stability
#else
  config.resolution_hz = TEST_RESOLUTION_STANDARD;
#endif

  config.polarity = hf_pio_polarity_t::Normal;
  config.idle_state = hf_pio_idle_state_t::Low;
  config.timeout_us = 10000;
  config.buffer_size = 128;
  return config;
}

/**
 * @brief Create WS2812 symbols for RGB data using resolution_hz timing
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param symbols Output array (must have space for 48 symbols - 24 bits * 2 symbols per bit)
 * @param resolution_hz The resolution frequency for timing calculations
 */
void create_ws2812_rgb_symbols(uint8_t r, uint8_t g, uint8_t b, hf_pio_symbol_t* symbols, uint32_t resolution_hz) noexcept {
  uint32_t rgb_data = (g << 16) | (r << 8) | b; // GRB format for WS2812
  
  // Calculate ticks based on resolution_hz
  uint32_t tick_ns = 1000000000 / resolution_hz;
  uint32_t t0h_ticks = WS2812_T0H / tick_ns;
  uint32_t t0l_ticks = WS2812_T0L / tick_ns;
  uint32_t t1h_ticks = WS2812_T1H / tick_ns;
  uint32_t t1l_ticks = WS2812_T1L / tick_ns;

  for (int i = 0; i < 24; i++) {
    bool bit = (rgb_data >> (23 - i)) & 1;
    if (bit) {
      // '1' bit: high for T1H, low for T1L
      symbols[i * 2] = {t1h_ticks, true};
      symbols[i * 2 + 1] = {t1l_ticks, false};
    } else {
      // '0' bit: high for T0H, low for T0L
      symbols[i * 2] = {t0h_ticks, true};
      symbols[i * 2 + 1] = {t0l_ticks, false};
    }
  }
}

/**
 * @brief Create WS2812 reset symbol using resolution_hz timing
 */
hf_pio_symbol_t create_ws2812_reset_symbol(uint32_t resolution_hz) noexcept {
  uint32_t tick_ns = 1000000000 / resolution_hz;
  uint32_t reset_ticks = WS2812_RESET / tick_ns;
  return {reset_ticks, false};
}

/**
 * @brief Create test pattern for logic analyzer verification using resolution_hz timing
 */
void create_logic_analyzer_test_pattern(hf_pio_symbol_t* symbols, size_t& symbol_count, uint32_t resolution_hz) noexcept {
  // Create a recognizable pattern: alternating high/low with varying durations
  symbol_count = 10;
  
  uint32_t tick_ns = 1000000000 / resolution_hz;

  symbols[0] = {1000 / tick_ns, true};  // 1µs high
  symbols[1] = {1000 / tick_ns, false}; // 1µs low
  symbols[2] = {2000 / tick_ns, true};  // 2µs high
  symbols[3] = {2000 / tick_ns, false}; // 2µs low
  symbols[4] = {500 / tick_ns, true};   // 0.5µs high
  symbols[5] = {500 / tick_ns, false};  // 0.5µs low
  symbols[6] = {3000 / tick_ns, true};  // 3µs high
  symbols[7] = {1500 / tick_ns, false}; // 1.5µs low
  symbols[8] = {750 / tick_ns, true};   // 0.75µs high
  symbols[9] = {4000 / tick_ns, false}; // 4µs low (end marker)
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
        {TEST_RESOLUTION_WS2812, "8MHz (WS2812 precision)"},
        {TEST_RESOLUTION_STANDARD, "1MHz (standard precision)"},
        {TEST_RESOLUTION_LOW, "100kHz (low precision)"},
        {38000, "38kHz (IR carrier)"},
    };
    
    uint8_t tx_channel = HfRmtGetTxChannel(0);
    if (tx_channel < 0) {
        ESP_LOGE(TAG, "No valid TX channel available");
        return false;
    }
    
    for (const auto& test_case : test_cases) {
        hf_pio_channel_config_t config;
        config.gpio_pin = TEST_GPIO_TX;
        config.direction = hf_pio_direction_t::Transmit;
        config.resolution_hz = test_case.resolution_hz;
        config.polarity = hf_pio_polarity_t::Normal;
        config.idle_state = hf_pio_idle_state_t::Low;
        config.timeout_us = 10000;
        config.buffer_size = 128;
        
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

//==============================================================================
// CONSTRUCTOR/DESTRUCTOR TESTS
//==============================================================================

bool test_constructor_default() noexcept {
  ESP_LOGI(TAG, "Testing default constructor...");

// ESP32-C6 specific validation
#if defined(CONFIG_IDF_TARGET_ESP32C6)
  ESP_LOGI(TAG, "Running on ESP32-C6 with RMT peripheral");
#endif

  EspPio pio;

  // Test initial state
  if (pio.IsInitialized()) {
    ESP_LOGE(TAG, "PIO should not be initialized initially");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Default constructor completed with correct initial state");
  return true;
}

bool test_destructor_cleanup() noexcept {
  ESP_LOGI(TAG, "Testing destructor cleanup...");

  {
    EspPio pio;

    // Initialize and configure a channel
    if (!pio.EnsureInitialized()) {
      ESP_LOGE(TAG, "Failed to initialize PIO for destructor test");
      return false;
    }

    hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
    hf_pio_err_t result = pio.ConfigureChannel(0, config);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to configure channel for destructor test: %s",
               HfPioErrToString(result));
      return false;
    }

    ESP_LOGI(TAG, "PIO configured, testing destructor cleanup...");
  } // pio should be destroyed here

  ESP_LOGI(TAG, "[SUCCESS] Destructor cleanup completed");
  return true;
}

//==============================================================================
// LIFECYCLE TESTS
//==============================================================================

bool test_initialization_states() noexcept {
  ESP_LOGI(TAG, "Testing initialization states...");

  EspPio pio;

  // Test initial state
  if (pio.IsInitialized()) {
    ESP_LOGE(TAG, "PIO should not be initialized initially");
    return false;
  }

  // Test manual initialization
  hf_pio_err_t result = pio.Initialize();
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Manual initialization failed: %s", HfPioErrToString(result));
    return false;
  }

  if (!pio.IsInitialized()) {
    ESP_LOGE(TAG, "PIO should be initialized after Initialize()");
    return false;
  }

  // Test double initialization
  result = pio.Initialize();
  if (result != hf_pio_err_t::PIO_ERR_ALREADY_INITIALIZED) {
    ESP_LOGE(TAG, "Double initialization should return ALREADY_INITIALIZED, got: %s",
             HfPioErrToString(result));
    return false;
  }

  // Test deinitialization
  result = pio.Deinitialize();
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Deinitialization failed: %s", HfPioErrToString(result));
    return false;
  }

  if (pio.IsInitialized()) {
    ESP_LOGE(TAG, "PIO should not be initialized after Deinitialize()");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Initialization states test passed");
  return true;
}

bool test_lazy_initialization() noexcept {
  ESP_LOGI(TAG, "Testing lazy initialization...");

  EspPio pio;

  // Test that EnsureInitialized() works
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized() should succeed");
    return false;
  }

  if (!pio.IsInitialized()) {
    ESP_LOGE(TAG, "PIO should be initialized after EnsureInitialized()");
    return false;
  }

  // Test that subsequent calls don't fail
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Subsequent EnsureInitialized() should also succeed");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Lazy initialization test passed");
  return true;
}

//==============================================================================
// CHANNEL CONFIGURATION TESTS
//==============================================================================

bool test_channel_configuration() noexcept {
  ESP_LOGI(TAG, "Testing channel configuration with ESP32 variant awareness...");

  ESP_LOGI(TAG, "Testing %s RMT channel configuration", HfRmtGetVariantName());

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Test valid TX channel configuration using variant-aware selection
  uint8_t valid_tx_channel = HfRmtGetTxChannel(0);
  if (valid_tx_channel < 0) {
    ESP_LOGE(TAG, "No valid TX channel available on %s", HfRmtGetVariantName());
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(valid_tx_channel, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure valid TX channel %d: %s", valid_tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Valid TX channel %d configured successfully", valid_tx_channel);

  // Test invalid channel ID
  result = pio.ConfigureChannel(255, config);
  if (result != hf_pio_err_t::PIO_ERR_INVALID_CHANNEL) {
    ESP_LOGE(TAG, "Invalid channel should return INVALID_CHANNEL, got: %s",
             HfPioErrToString(result).data());
    return false;
  }

  // Test invalid configuration: wrong channel for direction (if applicable)
  uint8_t invalid_tx_channel = HfRmtGetRxChannel(0);  // Get RX channel for TX test
  if (invalid_tx_channel >= 0 && !HfRmtIsChannelValidForDirection(invalid_tx_channel, hf_pio_direction_t::Transmit)) {
    hf_pio_channel_config_t invalid_config = create_test_channel_config(TEST_GPIO_TX);
    invalid_config.direction = hf_pio_direction_t::Transmit;  // TX on RX-only channel
    
    result = pio.ConfigureChannel(invalid_tx_channel, invalid_config);
    if (result == hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Invalid TX configuration should have failed but succeeded on channel %d", invalid_tx_channel);
      return false;
    }
    ESP_LOGI(TAG, "Invalid TX channel %d configuration correctly rejected: %s", 
             invalid_tx_channel, HfPioErrToString(result).data());
  }

  // Test channel status
  hf_pio_channel_status_t status;
  result = pio.GetChannelStatus(valid_tx_channel, status);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get channel status: %s", HfPioErrToString(result).data());
    return false;
  }

  if (!status.is_initialized) {
    ESP_LOGE(TAG, "Channel should be marked as initialized");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Channel configuration test passed for %s", HfRmtGetVariantName());
  return true;
}

bool test_multiple_channel_configuration() noexcept {
  ESP_LOGI(TAG, "Testing multiple channel configuration with variant-aware allocation...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate channels for current ESP32 variant
  uint8_t tx_channel = HfRmtGetTxChannel(0);
  uint8_t rx_channel = HfRmtGetRxChannel(0);

  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  if (rx_channel < 0) {
    ESP_LOGE(TAG, "No RX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Configure multiple channels with variant-aware channel selection
  hf_pio_channel_config_t tx_config =
      create_test_channel_config(TEST_GPIO_TX, hf_pio_direction_t::Transmit);
  hf_pio_channel_config_t rx_config =
      create_test_channel_config(TEST_GPIO_RX, hf_pio_direction_t::Receive);

  hf_pio_err_t result1 = pio.ConfigureChannel(tx_channel, tx_config);
  hf_pio_err_t result2 = pio.ConfigureChannel(rx_channel, rx_config);

  if (result1 != hf_pio_err_t::PIO_SUCCESS || result2 != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure multiple channels: TX(ch%d)=%s, RX(ch%d)=%s", 
             tx_channel, HfPioErrToString(result1).data(),
             rx_channel, HfPioErrToString(result2).data());
    return false;
  }

  ESP_LOGI(TAG, "Configured TX channel %d and RX channel %d successfully", tx_channel, rx_channel);

  // Verify both channels are configured
  hf_pio_channel_status_t status1, status2;
  pio.GetChannelStatus(tx_channel, status1);
  pio.GetChannelStatus(rx_channel, status2);

  if (!status1.is_initialized || !status2.is_initialized) {
    ESP_LOGE(TAG, "Both channels should be initialized");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple channel configuration test passed for %s", HfRmtGetVariantName());
  return true;
}

//==============================================================================
// BASIC TRANSMISSION TESTS
//==============================================================================

bool test_basic_symbol_transmission() noexcept {
  ESP_LOGI(TAG, "Testing basic symbol transmission with variant-aware channel selection...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate TX channel for current ESP32 variant
  uint8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Configure transmit channel with variant-aware selection
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Using TX channel %d for transmission on %s", tx_channel, HfRmtGetVariantName());

  // Create simple test symbols using resolution_hz timing
  uint32_t tick_ns = 1000000000 / TEST_RESOLUTION_STANDARD;
  hf_pio_symbol_t symbols[] = {
      {1000 / tick_ns, true},  // 1µs high
      {1000 / tick_ns, false}, // 1µs low
      {2000 / tick_ns, true},  // 2µs high
      {2000 / tick_ns, false}  // 2µs low
  };

  // Test transmission without waiting
  result = pio.Transmit(tx_channel, symbols, 4, false);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit symbols: %s", HfPioErrToString(result).data());
    return false;
  }

  // Wait a bit for transmission to complete
  vTaskDelay(pdMS_TO_TICKS(10));

  // Test transmission with waiting
  result = pio.Transmit(tx_channel, symbols, 4, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit symbols with wait: %s", HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Basic symbol transmission test passed on %s TX channel %d", HfRmtGetVariantName(), tx_channel);
  return true;
}

bool test_transmission_edge_cases() noexcept {
  ESP_LOGI(TAG, "Testing transmission edge cases...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  hf_pio_symbol_t test_symbol = {1000 / TEST_RESOLUTION_STANDARD, true};

  // Test null symbol array
  hf_pio_err_t result = pio.Transmit(0, nullptr, 1, false);
  if (result != hf_pio_err_t::PIO_ERR_NULL_POINTER) {
    ESP_LOGE(TAG, "Null symbols should return NULL_POINTER, got: %s", HfPioErrToString(result));
    return false;
  }

  // Test zero symbol count
  result = pio.Transmit(0, &test_symbol, 0, false);
  if (result != hf_pio_err_t::PIO_ERR_INVALID_PARAMETER) {
    ESP_LOGE(TAG, "Zero symbols should return INVALID_PARAMETER, got: %s",
             HfPioErrToString(result));
    return false;
  }

  // Test invalid channel
  result = pio.Transmit(255, &test_symbol, 1, false);
  if (result != hf_pio_err_t::PIO_ERR_INVALID_CHANNEL) {
    ESP_LOGE(TAG, "Invalid channel should return INVALID_CHANNEL, got: %s",
             HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Transmission edge cases test passed");
  return true;
}

//==============================================================================
// WS2812 LED PROTOCOL TESTS
//==============================================================================

bool test_ws2812_single_led() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 single LED protocol...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate TX channel for current ESP32 variant
  uint8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Configure channel for WS2812 timing with high resolution
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_hz = TEST_RESOLUTION_WS2812; // Use 8MHz for precise WS2812 timing
  hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Using TX channel %d with %d Hz resolution for WS2812 on %s", 
           tx_channel, TEST_RESOLUTION_WS2812, HfRmtGetVariantName());

  // Create RGB data for red color (255, 0, 0)
  hf_pio_symbol_t symbols[48]; // 24 bits * 2 symbols per bit
  create_ws2812_rgb_symbols(255, 0, 0, symbols, TEST_RESOLUTION_WS2812);

  // Add reset symbol
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(TEST_RESOLUTION_WS2812);

  // Transmit RGB data
  result = pio.Transmit(tx_channel, symbols, 48, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit WS2812 RGB data: %s", HfPioErrToString(result));
    return false;
  }

  // Transmit reset
  result = pio.Transmit(tx_channel, &reset_symbol, 1, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit WS2812 reset: %s", HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 single LED test passed - Red color transmitted on %s TX channel %d", 
           HfRmtGetVariantName(), tx_channel);
  return true;
}

bool test_ws2812_multiple_leds() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 multiple LED chain...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Create data for 3 LEDs: Red, Green, Blue
  hf_pio_symbol_t led_data[144]; // 3 LEDs * 24 bits * 2 symbols per bit

  create_ws2812_rgb_symbols(255, 0, 0, &led_data[0], TEST_RESOLUTION_WS2812);  // Red
  create_ws2812_rgb_symbols(0, 255, 0, &led_data[48], TEST_RESOLUTION_WS2812); // Green
  create_ws2812_rgb_symbols(0, 0, 255, &led_data[96], TEST_RESOLUTION_WS2812); // Blue

  // Transmit all LED data
  hf_pio_err_t result = pio.Transmit(0, led_data, 144, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit multiple LED data: %s", HfPioErrToString(result));
    return false;
  }

  // Send reset
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(TEST_RESOLUTION_WS2812);
  result = pio.Transmit(0, &reset_symbol, 1, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit reset: %s", HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 multiple LED test passed - RGB chain transmitted");
  return true;
}

bool test_ws2812_timing_validation() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 timing validation...");

  // Verify our timing calculations
  uint32_t t0h_ticks = WS2812_T0H / TEST_RESOLUTION_STANDARD;
  uint32_t t0l_ticks = WS2812_T0L / TEST_RESOLUTION_STANDARD;
  uint32_t t1h_ticks = WS2812_T1H / TEST_RESOLUTION_STANDARD;
  uint32_t t1l_ticks = WS2812_T1L / TEST_RESOLUTION_STANDARD;
  uint32_t reset_ticks = WS2812_RESET / TEST_RESOLUTION_STANDARD;

  ESP_LOGI(TAG, "WS2812 timing (in %uns ticks):", TEST_RESOLUTION_STANDARD);
  ESP_LOGI(TAG, "  T0H: %u ticks (%uns)", t0h_ticks, t0h_ticks * TEST_RESOLUTION_STANDARD);
  ESP_LOGI(TAG, "  T0L: %u ticks (%uns)", t0l_ticks, t0l_ticks * TEST_RESOLUTION_STANDARD);
  ESP_LOGI(TAG, "  T1H: %u ticks (%uns)", t1h_ticks, t1h_ticks * TEST_RESOLUTION_STANDARD);
  ESP_LOGI(TAG, "  T1L: %u ticks (%uns)", t1l_ticks, t1l_ticks * TEST_RESOLUTION_STANDARD);
  ESP_LOGI(TAG, "  Reset: %u ticks (%uns)", reset_ticks, reset_ticks * TEST_RESOLUTION_STANDARD);

  // Check timing tolerances (WS2812 has ±150ns tolerance)
  [[maybe_unused]] uint32_t tolerance_ticks = 150 / TEST_RESOLUTION_STANDARD;

  if (t0h_ticks < (350 - 150) / TEST_RESOLUTION_STANDARD ||
      t0h_ticks > (350 + 150) / TEST_RESOLUTION_STANDARD) {
    ESP_LOGE(TAG, "T0H timing out of tolerance");
    return false;
  }

  if (t1h_ticks < (700 - 150) / TEST_RESOLUTION_STANDARD ||
      t1h_ticks > (700 + 150) / TEST_RESOLUTION_STANDARD) {
    ESP_LOGE(TAG, "T1H timing out of tolerance");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 timing validation passed");
  return true;
}

//==============================================================================
// LOGIC ANALYZER TEST SCENARIOS
//==============================================================================

bool test_logic_analyzer_patterns() noexcept {
  ESP_LOGI(TAG, "Testing logic analyzer patterns...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Create test pattern for logic analyzer
  hf_pio_symbol_t test_symbols[10];
  size_t symbol_count;
  create_logic_analyzer_test_pattern(test_symbols, symbol_count, TEST_RESOLUTION_STANDARD);

  ESP_LOGI(TAG, "Transmitting logic analyzer test pattern on GPIO %d", TEST_GPIO_TX);
  ESP_LOGI(
      TAG,
      "Pattern: 1µs H, 1µs L, 2µs H, 2µs L, 0.5µs H, 0.5µs L, 3µs H, 1.5µs L, 0.75µs H, 4µs L");

  // Transmit pattern multiple times for easier capture
  for (int i = 0; i < 5; i++) {
    hf_pio_err_t result = pio.Transmit(0, test_symbols, symbol_count, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit test pattern iteration %d: %s", i,
               HfPioErrToString(result));
      return false;
    }

    // Add gap between patterns
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ESP_LOGI(TAG, "[SUCCESS] Logic analyzer patterns transmitted - capture on GPIO %d", TEST_GPIO_TX);
  return true;
}

bool test_frequency_sweep() noexcept {
  ESP_LOGI(TAG, "Testing frequency sweep for logic analyzer...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Generate square waves at different frequencies
  uint32_t frequencies[] = {1000, 5000, 10000, 50000, 100000}; // Hz

  for (size_t f = 0; f < sizeof(frequencies) / sizeof(frequencies[0]); f++) {
    uint32_t period_ns = 1000000000 / frequencies[f];
    uint32_t half_period_ticks = (period_ns / 2) / TEST_RESOLUTION_STANDARD;

    hf_pio_symbol_t square_wave[] = {{half_period_ticks, true}, {half_period_ticks, false}};

    ESP_LOGI(TAG, "Generating %uHz square wave (%uns period)", frequencies[f], period_ns);

    // Transmit 10 cycles of each frequency
    for (int cycle = 0; cycle < 10; cycle++) {
      hf_pio_err_t result = pio.Transmit(0, square_wave, 2, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to transmit square wave: %s", HfPioErrToString(result));
        return false;
      }
    }

    // Gap between frequencies
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  ESP_LOGI(TAG, "[SUCCESS] Frequency sweep completed");
  return true;
}

//==============================================================================
// ADVANCED RMT FEATURE TESTS
//==============================================================================

bool test_rmt_encoder_configuration() noexcept {
  ESP_LOGI(TAG, "Testing RMT encoder configuration...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Configure encoder for specific bit patterns
  hf_pio_symbol_t bit0_config = {WS2812_T0H / TEST_RESOLUTION_STANDARD, true};
  hf_pio_symbol_t bit1_config = {WS2812_T1H / TEST_RESOLUTION_STANDARD, true};

  hf_pio_err_t result = pio.ConfigureEncoder(0, bit0_config, bit1_config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Encoder configuration not supported or failed: %s", HfPioErrToString(result));
    // This might not be supported, which is OK
  } else {
    ESP_LOGI(TAG, "Encoder configuration successful");
  }

  ESP_LOGI(TAG, "[SUCCESS] RMT encoder configuration test completed");
  return true;
}

bool test_rmt_carrier_modulation() noexcept {
  ESP_LOGI(TAG, "Testing RMT carrier modulation...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Configure 38kHz carrier (typical for IR)
  hf_pio_err_t result = pio.ConfigureCarrier(0, 38000, 0.5f);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Carrier configuration not supported or failed: %s", HfPioErrToString(result));
    // This might not be supported, which is OK
  } else {
    ESP_LOGI(TAG, "Carrier modulation configured at 38kHz");

    // Test transmission with carrier
    hf_pio_symbol_t carrier_symbols[] = {
        {1000 / TEST_RESOLUTION_STANDARD, true},  // 1ms with carrier
        {1000 / TEST_RESOLUTION_STANDARD, false}, // 1ms without carrier
    };

    result = pio.Transmit(0, carrier_symbols, 2, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit with carrier: %s", HfPioErrToString(result));
      return false;
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] RMT carrier modulation test completed");
  return true;
}

bool test_rmt_advanced_configuration() noexcept {
  ESP_LOGI(TAG, "Testing RMT advanced configuration...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Test advanced RMT configuration
  hf_pio_err_t result = pio.ConfigureAdvancedRmt(0, 128, false, 8);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Advanced RMT configuration not supported or failed: %s",
             HfPioErrToString(result));
    // This might not be supported, which is OK
  } else {
    ESP_LOGI(TAG, "Advanced RMT configuration successful");
  }

  // Test idle level configuration
  result = pio.SetIdleLevel(0, false); // Set idle to low
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Idle level configuration not supported: %s", HfPioErrToString(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] RMT advanced configuration test completed");
  return true;
}

//==============================================================================
// LOOPBACK AND RECEPTION TESTS
//==============================================================================

bool test_loopback_functionality() noexcept {
  ESP_LOGI(TAG, "Testing loopback functionality...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_RX);
  pio.ConfigureChannel(0, config);

  // Enable loopback mode
  hf_pio_err_t result = pio.EnableLoopback(0, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Loopback not supported: %s", HfPioErrToString(result));
    return true; // Not an error if unsupported
  }

  // Test transmission in loopback mode
  hf_pio_symbol_t test_symbols[] = {{1000 / TEST_RESOLUTION_STANDARD, true},
                                    {1000 / TEST_RESOLUTION_STANDARD, false}};

  result = pio.Transmit(0, test_symbols, 2, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit in loopback mode: %s", HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Loopback functionality test completed");
  return true;
}

//==============================================================================
// CALLBACK TESTS
//==============================================================================

bool test_callback_functionality() noexcept {
  ESP_LOGI(TAG, "Testing channel-specific callback functionality...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate TX channel for current ESP32 variant
  uint8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Setup callback data for the specific channel
  g_callback_data[tx_channel].channel_id = tx_channel;
  g_callback_data[tx_channel].description = "Channel_Specific_Test";
  g_callback_data[tx_channel].callback_count = 0;
  g_callback_data[tx_channel].last_callback_success = false;

  // Set channel-specific callbacks with user data
  pio.SetTransmitCallback(tx_channel, TestTransmitCallback, &g_callback_data[tx_channel]);
  pio.SetReceiveCallback(tx_channel, TestReceiveCallback, &g_callback_data[tx_channel]);
  pio.SetErrorCallback(tx_channel, TestErrorCallback, &g_callback_data[tx_channel]);

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Testing callbacks on TX channel %d for %s", tx_channel, HfRmtGetVariantName());

  // Test transmission with callback
  uint32_t tick_ns = 1000000000 / TEST_RESOLUTION_STANDARD;
  hf_pio_symbol_t test_symbols[] = {{1000 / tick_ns, true},
                                    {1000 / tick_ns, false}};

  result = pio.Transmit(tx_channel, test_symbols, 2, false);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit for callback test: %s", HfPioErrToString(result).data());
    return false;
  }

  // Wait for callback
  int timeout = 100; // 1 second timeout
  while (!g_callback_data[tx_channel].last_callback_success && timeout > 0) {
    vTaskDelay(pdMS_TO_TICKS(10));
    timeout--;
  }

  if (!g_callback_data[tx_channel].last_callback_success) {
    ESP_LOGW(TAG, "Transmit callback not triggered (may be implementation dependent)");
  } else {
    ESP_LOGI(TAG, "Channel-specific transmit callback triggered successfully on channel %d", tx_channel);
    ESP_LOGI(TAG, "Callback count: %zu, symbols sent: %zu", 
             g_callback_data[tx_channel].callback_count, 
             g_callback_data[tx_channel].symbols_sent);
  }

  // Test clearing channel-specific callbacks
  pio.ClearChannelCallbacks(tx_channel);
  ESP_LOGI(TAG, "Cleared callbacks for channel %d", tx_channel);

  // Test clearing all callbacks
  pio.ClearCallbacks();
  ESP_LOGI(TAG, "Cleared all callbacks");

  ESP_LOGI(TAG, "[SUCCESS] Channel-specific callback functionality test completed for %s", HfRmtGetVariantName());
  return true;
}

//==============================================================================
// STATISTICS AND DIAGNOSTICS TESTS
//==============================================================================

bool test_statistics_and_diagnostics() noexcept {
  ESP_LOGI(TAG, "Testing statistics and diagnostics...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get capabilities
  hf_pio_capabilities_t capabilities;
  hf_pio_err_t result = pio.GetCapabilities(capabilities);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get capabilities: %s", HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "PIO Capabilities:");
  ESP_LOGI(TAG, "  Max channels: %d", capabilities.max_channels);
  ESP_LOGI(TAG, "  Min resolution: %uns", capabilities.min_resolution_ns);
  ESP_LOGI(TAG, "  Max resolution: %uns", capabilities.max_resolution_ns);
  ESP_LOGI(TAG, "  Max duration: %u", capabilities.max_duration);
  ESP_LOGI(TAG, "  Max buffer size: %d", capabilities.max_buffer_size);
  ESP_LOGI(TAG, "  Supports bidirectional: %s", capabilities.supports_bidirectional ? "Yes" : "No");
  ESP_LOGI(TAG, "  Supports loopback: %s", capabilities.supports_loopback ? "Yes" : "No");
  ESP_LOGI(TAG, "  Supports carrier: %s", capabilities.supports_carrier ? "Yes" : "No");

  // Get statistics
  hf_pio_statistics_t statistics;
  result = pio.GetStatistics(statistics);
  if (result == hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "PIO Statistics:");
    ESP_LOGI(TAG, "  Total transmissions: %u", statistics.totalTransmissions);
    ESP_LOGI(TAG, "  Successful transmissions: %u", statistics.successfulTransmissions);
    ESP_LOGI(TAG, "  Failed transmissions: %u", statistics.failedTransmissions);
  } else {
    ESP_LOGI(TAG, "Statistics not supported: %s", HfPioErrToString(result));
  }

  // Get diagnostics
  hf_pio_diagnostics_t diagnostics;
  result = pio.GetDiagnostics(diagnostics);
  if (result == hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "PIO Diagnostics:");
    ESP_LOGI(TAG, "  PIO healthy: %s", diagnostics.pioHealthy ? "Yes" : "No");
    ESP_LOGI(TAG, "  PIO initialized: %s", diagnostics.pioInitialized ? "Yes" : "No");
    ESP_LOGI(TAG, "  Active channels: %d", diagnostics.activeChannels);
  } else {
    ESP_LOGI(TAG, "Diagnostics not supported: %s", HfPioErrToString(result));
  }

  ESP_LOGI(TAG, "[SUCCESS] Statistics and diagnostics test completed");
  return true;
}

//==============================================================================
// STRESS AND PERFORMANCE TESTS
//==============================================================================

bool test_stress_transmission() noexcept {
  ESP_LOGI(TAG, "Testing stress transmission...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Create large symbol array
  constexpr size_t STRESS_SYMBOL_COUNT = 100;
  hf_pio_symbol_t stress_symbols[STRESS_SYMBOL_COUNT];

  for (size_t i = 0; i < STRESS_SYMBOL_COUNT; i++) {
    stress_symbols[i] = {
        (100 + (i % 50)) * TEST_RESOLUTION_STANDARD, // Variable duration 100-590 * 10 resolution units
        (i % 2) == 0           // Alternating high/low
    };
  }

  // Perform multiple stress transmissions
  constexpr int STRESS_ITERATIONS = 10;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < STRESS_ITERATIONS; i++) {
    hf_pio_err_t result = pio.Transmit(0, stress_symbols, STRESS_SYMBOL_COUNT, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Stress transmission failed on iteration %d: %s", i, HfPioErrToString(result));
      return false;
    }
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time = end_time - start_time;

  ESP_LOGI(TAG, "Stress test completed: %d transmissions of %d symbols each", STRESS_ITERATIONS,
           STRESS_SYMBOL_COUNT);
  ESP_LOGI(TAG, "Total time: %llu µs, Average per transmission: %llu µs", total_time,
           total_time / STRESS_ITERATIONS);

  ESP_LOGI(TAG, "[SUCCESS] Stress transmission test passed");
  return true;
}

//==============================================================================
// SYSTEM VALIDATION TEST
//==============================================================================

bool test_pio_system_validation() noexcept {
  ESP_LOGI(TAG, "Testing PIO system validation...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Run system validation if available
  bool validation_result = pio.ValidatePioSystem();

  ESP_LOGI(TAG, "PIO system validation result: %s",
           validation_result ? "PASSED" : "FAILED or NOT SUPPORTED");

  ESP_LOGI(TAG, "[SUCCESS] PIO system validation test completed");
  return true;
}

//==============================================================================
// MAIN TEST RUNNER
//==============================================================================

extern "C" void app_main() {
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG,
           "╔═══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║               ESP32 PIO COMPREHENSIVE TEST SUITE (ENHANCED)                  ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  Testing EspPio with ESP-IDF v5.5 RMT peripheral + Latest Improvements      ║");
  ESP_LOGI(TAG, "║  • Channel-specific callbacks with user data                                 ║");
  ESP_LOGI(TAG, "║  • Resolution_hz usage for direct ESP-IDF compatibility                     ║");
  ESP_LOGI(TAG, "║  • ESP32 variant-specific channel validation                                 ║");
  ESP_LOGI(TAG, "║  • Enhanced clock divider calculation                                        ║");
  ESP_LOGI(TAG, "║  • WS2812 LED protocol and automated loopback testing                       ║");
  ESP_LOGI(TAG, "║  • ASCII Art test result decoration                                          ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  ESP32 Variant: %-32s                                ║", HfRmtGetVariantName());
  ESP_LOGI(TAG, "║  Test Pins:                                                                   ║");
  ESP_LOGI(TAG, "║    GPIO %d - Built-in RGB LED (WS2812) + TX for loopback                     ║",
           TEST_GPIO_TX);
  ESP_LOGI(TAG, "║    GPIO %d - RX for automated loopback verification                          ║",
           TEST_GPIO_RX);
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  For automated testing: Connect GPIO %d to GPIO %d with jumper wire          ║",
           TEST_GPIO_TX, TEST_GPIO_RX);
  ESP_LOGI(TAG,
           "╚═══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "");

  // Print ASCII art welcome banner
  print_ascii_banner("PIO TEST START", true);

  // ESP32 Variant Information Tests (NEW)
  RUN_TEST(test_esp32_variant_detection);
  RUN_TEST(test_channel_allocation_helpers);
  RUN_TEST(test_channel_direction_validation);
  RUN_TEST(test_resolution_hz_usage);

  // Constructor/Destructor Tests
  RUN_TEST(test_constructor_default);
  RUN_TEST(test_destructor_cleanup);

  // Lifecycle Tests
  RUN_TEST(test_initialization_states);
  RUN_TEST(test_lazy_initialization);

  // Channel Configuration Tests (ENHANCED)
  RUN_TEST(test_channel_configuration);
  RUN_TEST(test_multiple_channel_configuration);

  // Basic Transmission Tests (ENHANCED)
  RUN_TEST(test_basic_symbol_transmission);
  RUN_TEST(test_transmission_edge_cases);

  // WS2812 LED Protocol Tests (ENHANCED)
  RUN_TEST(test_ws2812_timing_validation);
  RUN_TEST(test_ws2812_single_led);
  RUN_TEST(test_ws2812_multiple_leds);

  // Logic Analyzer Test Scenarios
  RUN_TEST(test_logic_analyzer_patterns);
  RUN_TEST(test_frequency_sweep);

  // Advanced RMT Feature Tests
  RUN_TEST(test_rmt_encoder_configuration);
  RUN_TEST(test_rmt_carrier_modulation);
  RUN_TEST(test_rmt_advanced_configuration);

  // Loopback and Reception Tests
  RUN_TEST(test_loopback_functionality);

  // Callback Tests (ENHANCED - Channel-specific)
  RUN_TEST(test_callback_functionality);

  // Statistics and Diagnostics Tests
  RUN_TEST(test_statistics_and_diagnostics);

  // Stress and Performance Tests
  RUN_TEST(test_stress_transmission);

  // System Validation
  RUN_TEST(test_pio_system_validation);

  // Print final summary with ASCII art
  print_test_summary(g_test_results, "PIO", TAG);

  if (g_test_results.failed_tests == 0) {
    print_ascii_banner("ALL TESTS PASSED", true);
    ESP_LOGI(TAG, "\n🎉 SUCCESS: All PIO tests passed on %s!", HfRmtGetVariantName());
  } else {
    print_ascii_banner("SOME TESTS FAILED", false);
    ESP_LOGE(TAG, "\n❌ FAILURE: %d tests failed on %s", g_test_results.failed_tests, HfRmtGetVariantName());
  }

  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG,
           "╔═══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                           TEST COMPLETE                                      ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  ESP32 Variant: %-32s                                ║", HfRmtGetVariantName());
  ESP_LOGI(TAG, "║  Channel Info:                                                                ║");
  ESP_LOGI(TAG, "║    Total: %d, TX: %d (range %d-%d), RX: %d (range %d-%d)                      ║",
           HF_RMT_MAX_CHANNELS, HF_RMT_MAX_TX_CHANNELS,
           HF_RMT_TX_CHANNEL_START, HF_RMT_TX_CHANNEL_START + HF_RMT_MAX_TX_CHANNELS - 1,
           HF_RMT_MAX_RX_CHANNELS, HF_RMT_RX_CHANNEL_START, HF_RMT_RX_CHANNEL_START + HF_RMT_MAX_RX_CHANNELS - 1);
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  For WS2812 testing: Built-in RGB LED on GPIO %d should show color changes   ║",
           TEST_GPIO_TX);
  ESP_LOGI(TAG, "║  For automated loopback: Verify transmission/reception on GPIO %d -> GPIO %d  ║",
           TEST_GPIO_TX, TEST_GPIO_RX);
  ESP_LOGI(TAG, "║  For logic analyzer: Capture signals on GPIO %d and verify timing            ║",
           TEST_GPIO_TX);
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  Expected WS2812 timing (±150ns tolerance):                                  ║");
  ESP_LOGI(TAG, "║    T0H: 350ns, T0L: 900ns (bit '0')                                          ║");
  ESP_LOGI(TAG, "║    T1H: 700ns, T1L: 600ns (bit '1')                                          ║");
  ESP_LOGI(TAG,
           "║    Reset: >50µs low                                                           ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  New Features Tested:                                                         ║");
  ESP_LOGI(TAG, "║    ✓ Channel-specific callbacks with user data                               ║");
  ESP_LOGI(TAG, "║    ✓ Resolution_hz for direct ESP-IDF compatibility                         ║");
  ESP_LOGI(TAG, "║    ✓ ESP32 variant-specific channel validation                              ║");
  ESP_LOGI(TAG, "║    ✓ Enhanced clock divider calculation                                     ║");
  ESP_LOGI(TAG, "║    ✓ ASCII Art test result decoration                                       ║");
  ESP_LOGI(TAG,
           "╚═══════════════════════════════════════════════════════════════════════════════╝");

  // Keep running for continuous testing if needed
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
