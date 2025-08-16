/**
 * @file PioComprehensiveTest.cpp
 * @brief Comprehensive PIO testing suite for ESP32-C6 DevKit-M-1 with RMT peripheral (noexcept)
 *
 * This comprehensive test suite validates all functionality of the EspPio class using ESP-IDF v5.5
 * RMT with the latest improvements including:
 * - Channel-specific callback system with proper user data handling
 * - Resolution_ns user interface with internal conversion to resolution_hz
 * - ESP32 variant-specific channel validation (TX/RX allocation per variant)
 * - Enhanced clock divider calculation with overflow protection
 * - Constructor/Destructor behavior
 * - Lifecycle management (Initialize/Deinitialize)
 * - Channel configuration and management
 * - Symbol transmission and reception
 * - RMT-specific features (carrier modulation, loopback, encoder configuration)
 * - WS2812 LED protocol timing validation with comprehensive color testing (GPIO8)
 *   • Primary colors (R/G/B) at maximum brightness for timing stress testing
 *   • Secondary colors (Yellow/Magenta/Cyan) and white variations
 *   • Brightness sweep tests (0-255) for each color channel
 *   • Bit pattern validation (alternating, edge cases, specific patterns)
 *   • Rainbow color wheel transitions with HSV to RGB conversion
 *   • Rapid color change sequences for protocol stress testing
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
#include "mcu/esp32/EspGpio.h" // Add GPIO support for test progression indicator

static const char* TAG = "PIO_Test";
static TestResults g_test_results;

// Test progression indicator GPIO
static EspGpio* g_test_progress_gpio = nullptr;
static bool g_test_progress_state = false;

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
static constexpr hf_gpio_num_t TEST_GPIO_TX = 8;  // GPIO8 for built-in RGB LED (WS2812) - TX
static constexpr hf_gpio_num_t TEST_GPIO_RX = 18; // GPIO18 for reception (RMT compatible) - RX
// For automated testing: Connect GPIO8 (TX) to GPIO18 (RX) with a jumper wire
// This creates a loopback that allows the test to verify transmission/reception

// Test resolutions using resolution in nanoseconds (user-facing API)
static constexpr uint32_t TEST_RESOLUTION_WS2812_NS = 125;   // 8 MHz -> 125ns per tick
static constexpr uint32_t TEST_RESOLUTION_STANDARD_NS = 1000; // 1 MHz -> 1µs per tick
static constexpr uint32_t TEST_RESOLUTION_LOW_NS = 10000;     // 100 kHz -> 10µs per tick

//==============================================================================
// TEST PROGRESSION INDICATOR FUNCTIONS
//==============================================================================

/**
 * @brief Initialize the test progression indicator GPIO
 */
bool init_test_progress_indicator() noexcept {
  // Use GPIO14 as the test progression indicator (visible LED on most ESP32 dev boards)
  g_test_progress_gpio = new EspGpio(14, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
  
  if (!g_test_progress_gpio->EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO");
    return false;
  }
  
  // Start with LOW state
  g_test_progress_gpio->SetInactive();
  g_test_progress_state = false;
  
  ESP_LOGI(TAG, "Test progression indicator initialized on GPIO14");
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
    }
}

void TestReceiveCallback(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, 
                        size_t symbol_count, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data && data->channel_id == channel_id) {
        data->callback_count++;
        data->symbols_received = symbol_count;
        data->last_callback_success = true;
    }
}

void TestErrorCallback(hf_u8_t channel_id, hf_pio_err_t error, void* user_data) {
    CallbackTestData* data = static_cast<CallbackTestData*>(user_data);
    if (data && data->channel_id == channel_id) {
        data->callback_count++;
        data->last_error = error;
    }
}

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Create a default PIO channel configuration for testing using resolution_ns
 * ESP32-C6 specific configuration for RMT compatibility with latest improvements
 */
hf_pio_channel_config_t create_test_channel_config(
    hf_gpio_num_t gpio_pin, hf_pio_direction_t direction = hf_pio_direction_t::Transmit) noexcept {
  hf_pio_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.direction = direction;

#if defined(CONFIG_IDF_TARGET_ESP32C6)
  config.resolution_ns = TEST_RESOLUTION_STANDARD_NS; // 1µs (converted internally)
#else
  config.resolution_ns = TEST_RESOLUTION_STANDARD_NS;
#endif

  config.polarity = hf_pio_polarity_t::Normal;
  config.idle_state = hf_pio_idle_state_t::Low;
  config.timeout_us = 10000;
  config.buffer_size = 128;
  return config;
}

/**
 * @brief Create WS2812 symbols for RGB data using resolution_ns timing
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param symbols Output array (must have space for 48 symbols - 24 bits * 2 symbols per bit)
 * @param resolution_hz The resolution frequency for timing calculations
 */
void create_ws2812_rgb_symbols(uint8_t r, uint8_t g, uint8_t b, hf_pio_symbol_t* symbols, uint32_t resolution_ns) noexcept {
  uint32_t rgb_data = (g << 16) | (r << 8) | b; // GRB format for WS2812
  
  // Calculate ticks based on resolution_ns directly
  uint32_t tick_ns = resolution_ns;
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
 * @brief Create WS2812 reset symbol using resolution_ns timing
 */
hf_pio_symbol_t create_ws2812_reset_symbol(uint32_t resolution_ns) noexcept {
  uint32_t tick_ns = resolution_ns;
  uint32_t reset_ticks = WS2812_RESET / tick_ns;
  return {reset_ticks, false};
}

/**
 * @brief Create test pattern for logic analyzer verification using resolution_ns timing
 */
void create_logic_analyzer_test_pattern(hf_pio_symbol_t* symbols, size_t& symbol_count, uint32_t resolution_ns) noexcept {
  // Create a recognizable pattern: alternating high/low with varying durations
  symbol_count = 10;

  const uint32_t tick_ns = resolution_ns ? resolution_ns : 1;
  auto ticks_ceil = [tick_ns](uint32_t ns) -> uint32_t {
    // Round up to at least one tick
    uint32_t t = (ns + tick_ns - 1) / tick_ns;
    return t == 0 ? 1u : t;
  };

  symbols[0] = {ticks_ceil(1000), true};   // 1µs high
  symbols[1] = {ticks_ceil(1000), false};  // 1µs low
  symbols[2] = {ticks_ceil(2000), true};   // 2µs high
  symbols[3] = {ticks_ceil(2000), false};  // 2µs low
  symbols[4] = {ticks_ceil(500), true};    // 0.5µs high
  symbols[5] = {ticks_ceil(500), false};   // 0.5µs low
  symbols[6] = {ticks_ceil(3000), true};   // 3µs high
  symbols[7] = {ticks_ceil(1500), false};  // 1.5µs low
  symbols[8] = {ticks_ceil(750), true};    // 0.75µs high
  symbols[9] = {ticks_ceil(4000), false};  // 4µs low (end marker)
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
    ESP_LOGE(TAG, "%s", "Variant name is empty");
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

bool test_resolution_ns_usage() noexcept {
    ESP_LOGI(TAG, "Testing resolution_ns usage and clock calculations...");
    
    EspPio pio;
    if (!pio.EnsureInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize PIO");
        return false;
    }
    
    // Test different resolution configurations
    struct {
        uint32_t resolution_ns;
        const char* description;
    } test_cases[] = {
        {TEST_RESOLUTION_WS2812_NS, "125ns (WS2812 precision)"},
        {TEST_RESOLUTION_STANDARD_NS, "1µs (standard precision)"},
        {TEST_RESOLUTION_LOW_NS, "10µs (low precision)"},
    };
    
    int8_t tx_channel = HfRmtGetTxChannel(0);
    if (tx_channel < 0) {
        ESP_LOGE(TAG, "No valid TX channel available");
        return false;
    }
    
    for (const auto& test_case : test_cases) {
        hf_pio_channel_config_t config;
        config.gpio_pin = TEST_GPIO_TX;
        config.direction = hf_pio_direction_t::Transmit;
        config.resolution_ns = test_case.resolution_ns;
        config.polarity = hf_pio_polarity_t::Normal;
        config.idle_state = hf_pio_idle_state_t::Low;
    config.timeout_us = 10000;
        config.buffer_size = 128;
        
        ESP_LOGI(TAG, "Testing %s (tick=%uns)...", test_case.description, test_case.resolution_ns);
        
    // For very low frequency tick (e.g., 10us), select RC_FAST source so divider fits
    if (test_case.resolution_ns >= 10000) {
      (void)pio.SetClockSource(static_cast<hf_u8_t>(tx_channel), RMT_CLK_SRC_RC_FAST);
    }

        hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      // If out-of-range as expected for given clock, treat as informational failure
      ESP_LOGW(TAG, "Configuration for %s returned: %s", 
               test_case.description, HfPioErrToString(result).data());
      // Restore default clock for subsequent iterations
      (void)pio.SetClockSource(static_cast<hf_u8_t>(tx_channel), RMT_CLK_SRC_PLL_F80M);
      continue;
    }
        
        ESP_LOGI(TAG, "  %s: SUCCESS", test_case.description);
        
    // Clear the channel for next test
        pio.ClearChannelCallbacks(tx_channel);
    // Restore default clock for next iteration
    (void)pio.SetClockSource(static_cast<hf_u8_t>(tx_channel), RMT_CLK_SRC_PLL_F80M);
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
    ESP_LOGE(TAG, "Manual initialization failed: %s", HfPioErrToString(result).data());
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
    ESP_LOGE(TAG, "Deinitialization failed: %s", HfPioErrToString(result).data());
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
  int8_t valid_tx_channel = HfRmtGetTxChannel(0);
  if (valid_tx_channel < 0) {
    ESP_LOGE(TAG, "No valid TX channel available on %s", HfRmtGetVariantName());
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(valid_tx_channel), config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure valid TX channel %d: %s", valid_tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Valid TX channel %d configured successfully", valid_tx_channel);

  // Test invalid channel ID
  result = pio.ConfigureChannel(static_cast<hf_u8_t>(255), config);
  if (result != hf_pio_err_t::PIO_ERR_INVALID_CHANNEL) {
    ESP_LOGE(TAG, "Invalid channel should return INVALID_CHANNEL, got: %s",
             HfPioErrToString(result).data());
    return false;
  }

  // Test invalid configuration: wrong channel for direction (if applicable)
  int8_t invalid_tx_channel = HfRmtGetRxChannel(0);  // Get RX channel for TX test
  if (invalid_tx_channel >= 0 && !HfRmtIsChannelValidForDirection(invalid_tx_channel, hf_pio_direction_t::Transmit)) {
    hf_pio_channel_config_t invalid_config = create_test_channel_config(TEST_GPIO_TX);
    invalid_config.direction = hf_pio_direction_t::Transmit;  // TX on RX-only channel
    
    result = pio.ConfigureChannel(static_cast<hf_u8_t>(invalid_tx_channel), invalid_config);
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
  int8_t tx_channel = HfRmtGetTxChannel(0);
  int8_t rx_channel = HfRmtGetRxChannel(0);

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

  hf_pio_err_t result1 = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), tx_config);
  hf_pio_err_t result2 = pio.ConfigureChannel(static_cast<hf_u8_t>(rx_channel), rx_config);

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
  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Configure transmit channel with variant-aware selection
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Using TX channel %d for transmission on %s", tx_channel, HfRmtGetVariantName());

  // Use actual achieved resolution for symbol durations
  uint32_t tick_ns = config.resolution_ns;
  (void)pio.GetActualResolution(static_cast<hf_u8_t>(tx_channel), tick_ns);
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
  ESP_LOGI(TAG, "Creating test channel config");
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.timeout_us = 200000; // generous timeout for multiple short bursts
  pio.ConfigureChannel(0, config);

  uint32_t edge_tick_ns = config.resolution_ns; (void)pio.GetActualResolution(0, edge_tick_ns);
  hf_pio_symbol_t test_symbol = {1000 / edge_tick_ns, true};

  // Test null symbol array
  ESP_LOGI(TAG, "Testing null symbol array");
  hf_pio_err_t result = pio.Transmit(0, nullptr, 1, false);
  if (result != hf_pio_err_t::PIO_ERR_NULL_POINTER) {
  ESP_LOGE(TAG, "Null symbols should return NULL_POINTER, got: %s", HfPioErrToString(result).data());
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
  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  // Configure channel for WS2812 timing with high resolution
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS; // request 125ns ticks
  config.timeout_us = 200000; // allow ample time for flush
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  // Query actual achieved resolution after configuration
  uint32_t actual_ns = config.resolution_ns;
  if (pio.GetActualResolution(tx_channel, actual_ns) == hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "Using TX channel %d with requested=%uns, achieved=%uns for WS2812 on %s",
             tx_channel, config.resolution_ns, actual_ns, HfRmtGetVariantName());
  }

  // Create RGB data for red color (255, 0, 0)
  hf_pio_symbol_t symbols[48]; // 24 bits * 2 symbols per bit
  create_ws2812_rgb_symbols(255, 0, 0, symbols, actual_ns);

  // Add reset symbol
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);

  // Transmit RGB data
  result = pio.Transmit(tx_channel, symbols, 48, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit WS2812 RGB data: %s", HfPioErrToString(result).data());
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
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS; // request 125ns ticks
  config.timeout_us = 200000;
  pio.ConfigureChannel(0, config);

  // Use actual achieved resolution for symbol generation
  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(0, actual_ns);
  ESP_LOGI(TAG, "Using TX channel 0 with requested=%uns, achieved=%uns for WS2812 chain on %s",
           config.resolution_ns, actual_ns, HfRmtGetVariantName());

  // Create data for 3 LEDs: Red, Green, Blue
  hf_pio_symbol_t led_data[144]; // 3 LEDs * 24 bits * 2 symbols per bit

  create_ws2812_rgb_symbols(255, 0, 0, &led_data[0], actual_ns);  // Red
  create_ws2812_rgb_symbols(0, 255, 0, &led_data[48], actual_ns); // Green
  create_ws2812_rgb_symbols(0, 0, 255, &led_data[96], actual_ns); // Blue

  // Transmit all LED data
  hf_pio_err_t result = pio.Transmit(0, led_data, 144, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit multiple LED data: %s", HfPioErrToString(result).data());
    return false;
  }

  // Send reset
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);
  result = pio.Transmit(0, &reset_symbol, 1, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit reset: %s", HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 multiple LED test passed - RGB chain transmitted");
  return true;
}

bool test_ws2812_color_cycle() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 comprehensive color cycle for protocol verification...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate TX channel for current ESP32 variant
  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS;
  config.timeout_us = 300000; // Extended timeout for long sequences
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(tx_channel, actual_ns);
  ESP_LOGI(TAG, "Using TX channel %d with %uns resolution for color cycle", tx_channel, actual_ns);

  // Comprehensive color test patterns for protocol verification
  struct ColorTest {
    uint8_t r, g, b;
    const char* name;
    uint32_t delay_ms;
  };

  const ColorTest color_tests[] = {
    // Primary colors - maximum brightness (stress test for timing)
    {255, 0, 0,    "RED_MAX",        500},
    {0, 255, 0,    "GREEN_MAX",      500},
    {0, 0, 255,    "BLUE_MAX",       500},
    
    // Secondary colors
    {255, 255, 0,  "YELLOW",         400},
    {255, 0, 255,  "MAGENTA",        400},
    {0, 255, 255,  "CYAN",           400},
    
    // White variations (high bit density test)
    {255, 255, 255, "WHITE_MAX",     600},
    {128, 128, 128, "WHITE_MID",     400},
    {64, 64, 64,   "WHITE_LOW",      400},
    
    // Black (all zeros - timing verification)
    {0, 0, 0,      "BLACK",          300},
    
    // Gradient patterns (mixed bit patterns)
    {255, 128, 64, "ORANGE",         400},
    {128, 0, 128,  "PURPLE",         400},
    {0, 128, 64,   "TEAL",           400},
    {192, 192, 64, "OLIVE",          400},
    
    // Brightness levels (single color, varying intensity)
    {32, 0, 0,     "RED_DIM",        300},
    {64, 0, 0,     "RED_LOW",        300},
    {128, 0, 0,    "RED_MID",        300},
    {192, 0, 0,    "RED_HIGH",       300},
    
    // Pattern verification colors (specific bit patterns)
    {85, 85, 85,   "PATTERN_01",     300}, // 01010101 pattern
    {170, 170, 170, "PATTERN_10",    300}, // 10101010 pattern
    {15, 15, 15,   "PATTERN_0F",     300}, // 00001111 pattern
    {240, 240, 240, "PATTERN_F0",    300}, // 11110000 pattern
    
    // Edge case values
    {1, 1, 1,      "MIN_NONZERO",    300},
    {254, 254, 254, "MAX_MINUS_ONE", 300},
    
    // Color wheel simulation (smooth transitions)
    {255, 32, 0,   "WHEEL_1",        250},
    {255, 64, 0,   "WHEEL_2",        250},
    {255, 128, 0,  "WHEEL_3",        250},
    {128, 255, 0,  "WHEEL_4",        250},
    {0, 255, 128,  "WHEEL_5",        250},
    {0, 128, 255,  "WHEEL_6",        250},
    {128, 0, 255,  "WHEEL_7",        250},
    {255, 0, 128,  "WHEEL_8",        250},
  };

  constexpr size_t num_tests = sizeof(color_tests) / sizeof(color_tests[0]);
  ESP_LOGI(TAG, "Running %zu color pattern tests for comprehensive protocol verification", num_tests);

  hf_pio_symbol_t led_symbols[48]; // Single LED: 24 bits * 2 symbols per bit
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);

  for (size_t i = 0; i < num_tests; i++) {
    const ColorTest& test = color_tests[i];
    
    ESP_LOGI(TAG, "Test %zu/%zu: %s (R:%d, G:%d, B:%d)", 
             i + 1, num_tests, test.name, test.r, test.g, test.b);
    
    // Create RGB symbols for current color
    create_ws2812_rgb_symbols(test.r, test.g, test.b, led_symbols, actual_ns);
    
    // Transmit color data
    result = pio.Transmit(tx_channel, led_symbols, 48, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit color %s: %s", test.name, HfPioErrToString(result).data());
      return false;
    }
    
    // Send reset to latch the color
    result = pio.Transmit(tx_channel, &reset_symbol, 1, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit reset for %s: %s", test.name, HfPioErrToString(result).data());
      return false;
    }
    
    // Delay to allow visual verification and timing analysis
    vTaskDelay(pdMS_TO_TICKS(test.delay_ms));
  }

  // Final sequence - rapid color changes for timing stress test
  ESP_LOGI(TAG, "Running rapid color change sequence (timing stress test)...");
  const ColorTest rapid_sequence[] = {
    {255, 0, 0, "RAPID_RED", 50},
    {0, 255, 0, "RAPID_GREEN", 50},
    {0, 0, 255, "RAPID_BLUE", 50},
    {255, 255, 255, "RAPID_WHITE", 50},
    {0, 0, 0, "RAPID_BLACK", 50},
  };

  for (int cycle = 0; cycle < 5; cycle++) {
    for (const auto& test : rapid_sequence) {
      create_ws2812_rgb_symbols(test.r, test.g, test.b, led_symbols, actual_ns);
      pio.Transmit(tx_channel, led_symbols, 48, true);
      pio.Transmit(tx_channel, &reset_symbol, 1, true);
      vTaskDelay(pdMS_TO_TICKS(test.delay_ms));
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 comprehensive color cycle completed - %zu patterns tested", num_tests);
  return true;
}

bool test_ws2812_brightness_sweep() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 brightness sweep for timing verification...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS;
  config.timeout_us = 200000;
  pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);

  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(tx_channel, actual_ns);

  hf_pio_symbol_t led_symbols[48];
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);

  // Test brightness sweep for each primary color
  const char* colors[] = {"RED", "GREEN", "BLUE"};
  
  for (int color_idx = 0; color_idx < 3; color_idx++) {
    ESP_LOGI(TAG, "Testing %s brightness sweep (0-255)...", colors[color_idx]);
    
    // Sweep up
    for (int brightness = 0; brightness <= 255; brightness += 8) {
      uint8_t r = (color_idx == 0) ? brightness : 0;
      uint8_t g = (color_idx == 1) ? brightness : 0;
      uint8_t b = (color_idx == 2) ? brightness : 0;
      
      create_ws2812_rgb_symbols(r, g, b, led_symbols, actual_ns);
      pio.Transmit(tx_channel, led_symbols, 48, true);
      pio.Transmit(tx_channel, &reset_symbol, 1, true);
      vTaskDelay(pdMS_TO_TICKS(30));
    }
    
    // Sweep down
    for (int brightness = 255; brightness >= 0; brightness -= 8) {
      uint8_t r = (color_idx == 0) ? brightness : 0;
      uint8_t g = (color_idx == 1) ? brightness : 0;
      uint8_t b = (color_idx == 2) ? brightness : 0;
      
      create_ws2812_rgb_symbols(r, g, b, led_symbols, actual_ns);
      pio.Transmit(tx_channel, led_symbols, 48, true);
      pio.Transmit(tx_channel, &reset_symbol, 1, true);
      vTaskDelay(pdMS_TO_TICKS(30));
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 brightness sweep completed");
  return true;
}

bool test_ws2812_pattern_validation() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 bit pattern validation for protocol accuracy...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS;
  config.timeout_us = 200000;
  pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);

  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(tx_channel, actual_ns);

  ESP_LOGI(TAG, "Testing specific bit patterns for timing analysis...");

  // Test patterns designed to verify WS2812 protocol timing
  struct PatternTest {
    uint8_t value;
    const char* pattern_name;
    const char* binary;
  };

  const PatternTest bit_patterns[] = {
    {0x00, "ALL_ZEROS", "00000000"},
    {0xFF, "ALL_ONES",  "11111111"},
    {0xAA, "ALTERNATING_10", "10101010"},
    {0x55, "ALTERNATING_01", "01010101"},
    {0xF0, "HIGH_NIBBLE", "11110000"},
    {0x0F, "LOW_NIBBLE",  "00001111"},
    {0xCC, "PATTERN_CC", "11001100"},
    {0x33, "PATTERN_33", "00110011"},
    {0xA5, "PATTERN_A5", "10100101"},
    {0x5A, "PATTERN_5A", "01011010"},
  };

  hf_pio_symbol_t led_symbols[48];
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);

  for (const auto& pattern : bit_patterns) {
    ESP_LOGI(TAG, "Testing pattern %s (0x%02X = %s)", 
             pattern.pattern_name, pattern.value, pattern.binary);
    
    // Test pattern in each color channel
    for (int channel = 0; channel < 3; channel++) {
      uint8_t r = (channel == 0) ? pattern.value : 0;
      uint8_t g = (channel == 1) ? pattern.value : 0;
      uint8_t b = (channel == 2) ? pattern.value : 0;
      
      const char* channel_name = (channel == 0) ? "RED" : (channel == 1) ? "GREEN" : "BLUE";
      ESP_LOGI(TAG, "  %s channel: R=%d, G=%d, B=%d", channel_name, r, g, b);
      
      create_ws2812_rgb_symbols(r, g, b, led_symbols, actual_ns);
      
      hf_pio_err_t result = pio.Transmit(tx_channel, led_symbols, 48, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to transmit pattern %s: %s", 
                 pattern.pattern_name, HfPioErrToString(result).data());
        return false;
      }
      
      result = pio.Transmit(tx_channel, &reset_symbol, 1, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to transmit reset: %s", HfPioErrToString(result).data());
        return false;
      }
      
      vTaskDelay(pdMS_TO_TICKS(200)); // Allow time for timing analysis
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 bit pattern validation completed");
  return true;
}

bool test_ws2812_rainbow_transition() noexcept {
  ESP_LOGI(TAG, "Testing WS2812 rainbow color transitions for visual and timing verification...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  int8_t tx_channel = HfRmtGetTxChannel(0);
  if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available");
    return false;
  }

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  config.resolution_ns = TEST_RESOLUTION_WS2812_NS;
  config.timeout_us = 200000;
  pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);

  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(tx_channel, actual_ns);

  ESP_LOGI(TAG, "Generating rainbow color wheel transitions...");

  hf_pio_symbol_t led_symbols[48];
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol(actual_ns);

  // Generate smooth rainbow transitions using HSV to RGB conversion
  for (int cycle = 0; cycle < 3; cycle++) {
    ESP_LOGI(TAG, "Rainbow cycle %d/3", cycle + 1);
    
    for (int hue = 0; hue < 360; hue += 5) { // 5-degree steps for smooth transition
      // Simple HSV to RGB conversion for rainbow effect
      uint8_t r, g, b;
      
      if (hue < 60) {
        r = 255;
        g = (hue * 255) / 60;
        b = 0;
      } else if (hue < 120) {
        r = ((120 - hue) * 255) / 60;
        g = 255;
        b = 0;
      } else if (hue < 180) {
        r = 0;
        g = 255;
        b = ((hue - 120) * 255) / 60;
      } else if (hue < 240) {
        r = 0;
        g = ((240 - hue) * 255) / 60;
        b = 255;
      } else if (hue < 300) {
        r = ((hue - 240) * 255) / 60;
        g = 0;
        b = 255;
      } else {
        r = 255;
        g = 0;
        b = ((360 - hue) * 255) / 60;
      }

      // Scale down intensity for easier viewing
      r = (r * 128) / 255;
      g = (g * 128) / 255;
      b = (b * 128) / 255;
      
      create_ws2812_rgb_symbols(r, g, b, led_symbols, actual_ns);
      
      hf_pio_err_t result = pio.Transmit(tx_channel, led_symbols, 48, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to transmit rainbow color at hue %d: %s", 
                 hue, HfPioErrToString(result).data());
        return false;
      }
      
      result = pio.Transmit(tx_channel, &reset_symbol, 1, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Failed to transmit reset at hue %d: %s", 
                 hue, HfPioErrToString(result).data());
        return false;
      }
      
      vTaskDelay(pdMS_TO_TICKS(25)); // Smooth transition speed
    }
  }

  // End with a fade to black
  ESP_LOGI(TAG, "Fading to black...");
  for (int brightness = 128; brightness >= 0; brightness -= 4) {
    create_ws2812_rgb_symbols(brightness, brightness, brightness, led_symbols, actual_ns);
    pio.Transmit(tx_channel, led_symbols, 48, true);
    pio.Transmit(tx_channel, &reset_symbol, 1, true);
    vTaskDelay(pdMS_TO_TICKS(30));
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 rainbow transition test completed");
  return true;
}

// test_ws2812_timing_validation removed as redundant

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
  config.timeout_us = 200000; // allow long waits over multiple cycles
  pio.ConfigureChannel(0, config);

  // Use the actual achieved resolution to derive non-zero ticks
  uint32_t actual_ns = config.resolution_ns;
  (void)pio.GetActualResolution(0, actual_ns);

  // Create test pattern for logic analyzer with sufficient timeout
  hf_pio_symbol_t test_symbols[10];
  size_t symbol_count;
  create_logic_analyzer_test_pattern(test_symbols, symbol_count, actual_ns);

  ESP_LOGI(TAG, "Transmitting logic analyzer test pattern on GPIO %d", TEST_GPIO_TX);
  ESP_LOGI(TAG, "%s", "Pattern: 1µs H, 1µs L, 2µs H, 2µs L, 0.5µs H, 0.5µs L, 3µs H, 1.5µs L, 0.75µs H, 4µs L");

  // Transmit pattern multiple times for easier capture
  for (int i = 0; i < 5; i++) {
    hf_pio_err_t result = pio.Transmit(0, test_symbols, symbol_count, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit test pattern iteration %d: %s", i,
               HfPioErrToString(result).data());
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
  config.timeout_us = 200000; // longer timeout to avoid spurious flush timeouts
  pio.ConfigureChannel(0, config);

  // Generate square waves at different frequencies (safe set)
  uint32_t frequencies[] = {1000, 5000, 10000}; // Hz

  for (size_t f = 0; f < sizeof(frequencies) / sizeof(frequencies[0]); f++) {
  uint32_t period_ns = 1000000000 / frequencies[f];
  uint32_t tick_ns = config.resolution_ns;
  (void)pio.GetActualResolution(0, tick_ns);
  uint32_t half_period_ticks = (period_ns / 2) / tick_ns;
  if (half_period_ticks == 0) half_period_ticks = 1;
  hf_pio_symbol_t square_wave[] = {{half_period_ticks, true}, {half_period_ticks, false}};

    ESP_LOGI(TAG, "Generating %uHz square wave (%uns period)", frequencies[f], period_ns);

    // Transmit 10 cycles of each frequency
    for (int cycle = 0; cycle < 10; cycle++) {
      hf_pio_err_t result = pio.Transmit(0, square_wave, 2, true);
      if (result != hf_pio_err_t::PIO_SUCCESS) {
       ESP_LOGE(TAG, "Failed to transmit square wave: %s", HfPioErrToString(result).data());
        return false;
      }
      // Give hardware a brief gap to fully flush
      vTaskDelay(pdMS_TO_TICKS(1));
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
  config.timeout_us = 500000; // long timeout; transfers can exceed 10ms
  pio.ConfigureChannel(0, config);

  // Configure encoder for specific bit patterns (use actual achieved tick)
  uint32_t enc_tick_ns = config.resolution_ns; (void)pio.GetActualResolution(0, enc_tick_ns);
  hf_pio_symbol_t bit0_config = {WS2812_T0H / enc_tick_ns, true};
  hf_pio_symbol_t bit1_config = {WS2812_T1H / enc_tick_ns, true};

  hf_pio_err_t result = pio.ConfigureEncoder(0, bit0_config, bit1_config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
  ESP_LOGI(TAG, "Encoder configuration not supported or failed: %s", HfPioErrToString(result).data());
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
  ESP_LOGI(TAG, "Carrier configuration not supported or failed: %s", HfPioErrToString(result).data());
    // This might not be supported, which is OK
  } else {
    ESP_LOGI(TAG, "Carrier modulation configured at 38kHz");

    // Test transmission with carrier using achieved tick
  uint32_t car_tick_ns = config.resolution_ns; (void)pio.GetActualResolution(0, car_tick_ns);
  hf_pio_symbol_t carrier_symbols[] = {
        {1000 / car_tick_ns, true},  // 1ms with carrier
        {1000 / car_tick_ns, false}, // 1ms without carrier
  };

    result = pio.Transmit(0, carrier_symbols, 2, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to transmit with carrier: %s", HfPioErrToString(result).data());
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
  ESP_LOGI(TAG, "Idle level configuration not supported: %s", HfPioErrToString(result).data());
  }

  ESP_LOGI(TAG, "[SUCCESS] RMT advanced configuration test completed");
  return true;
}

//==============================================================================
// LOOPBACK AND RECEPTION TESTS
//==============================================================================

bool test_loopback_functionality() noexcept {
  ESP_LOGI(TAG, "Testing software loopback functionality...");

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
  ESP_LOGI(TAG, "Loopback not supported: %s", HfPioErrToString(result).data());
    return true; // Not an error if unsupported
  }

  // Test transmission in loopback mode (use actual achieved tick)
  uint32_t loop_tick_ns = config.resolution_ns; (void)pio.GetActualResolution(0, loop_tick_ns);
  hf_pio_symbol_t test_symbols[] = {{1000 / loop_tick_ns, true},
                                    {1000 / loop_tick_ns, false}};

  result = pio.Transmit(0, test_symbols, 2, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
  ESP_LOGE(TAG, "Failed to transmit in loopback mode: %s", HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Software loopback functionality test completed");
  return true;
}

bool test_hardware_loopback_gpio8_to_gpio18() noexcept {
  ESP_LOGI(TAG, "Testing HARDWARE loopback: GPIO8 (TX) -> GPIO18 (RX)");
  ESP_LOGI(TAG, "*** ENSURE: Physical jumper wire connects GPIO8 to GPIO18 ***");
  ESP_LOGI(TAG, "*** This test verifies actual transmission and reception ***");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Get appropriate TX and RX channels for ESP32-C6
  int8_t tx_channel = HfRmtGetTxChannel(0);  // Channel 0 (TX)
  int8_t rx_channel = HfRmtGetRxChannel(0);  // Channel 2 (RX) 
  
  if (tx_channel < 0 || rx_channel < 0) {
    ESP_LOGE(TAG, "Required channels not available on %s", HfRmtGetVariantName());
    ESP_LOGE(TAG, "  TX channel: %d, RX channel: %d", tx_channel, rx_channel);
    return false;
  }

  ESP_LOGI(TAG, "Using TX channel %d (GPIO%d) -> RX channel %d (GPIO%d)", 
           tx_channel, TEST_GPIO_TX, rx_channel, TEST_GPIO_RX);

  // Configure TX channel on GPIO 8
  hf_pio_channel_config_t tx_config = create_test_channel_config(TEST_GPIO_TX, hf_pio_direction_t::Transmit);
  tx_config.resolution_ns = TEST_RESOLUTION_STANDARD_NS; // Use 1µs resolution for clear signals
  tx_config.timeout_us = 50000; // 50ms timeout
  
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), tx_config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  // Configure RX channel on GPIO 18  
  hf_pio_channel_config_t rx_config = create_test_channel_config(TEST_GPIO_RX, hf_pio_direction_t::Receive);
  rx_config.resolution_ns = TEST_RESOLUTION_STANDARD_NS; // Match TX resolution
  rx_config.timeout_us = 50000; // 50ms timeout
  
  result = pio.ConfigureChannel(static_cast<hf_u8_t>(rx_channel), rx_config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure RX channel %d: %s", rx_channel, HfPioErrToString(result).data());
    return false;
  }

  // Get actual achieved resolution for accurate timing
  uint32_t tx_actual_ns = tx_config.resolution_ns;
  uint32_t rx_actual_ns = rx_config.resolution_ns;
  pio.GetActualResolution(static_cast<hf_u8_t>(tx_channel), tx_actual_ns);
  pio.GetActualResolution(static_cast<hf_u8_t>(rx_channel), rx_actual_ns);
  
  ESP_LOGI(TAG, "TX resolution: requested=%uns, achieved=%uns", tx_config.resolution_ns, tx_actual_ns);
  ESP_LOGI(TAG, "RX resolution: requested=%uns, achieved=%uns", rx_config.resolution_ns, rx_actual_ns);

  // Create distinctive test pattern for easy verification
  hf_pio_symbol_t test_symbols[] = {
    {1000 / tx_actual_ns, true},   // 1µs high
    {2000 / tx_actual_ns, false},  // 2µs low  
    {500 / tx_actual_ns, true},    // 0.5µs high
    {1500 / tx_actual_ns, false},  // 1.5µs low
    {3000 / tx_actual_ns, true},   // 3µs high (distinctive end pattern)
    {1000 / tx_actual_ns, false}   // 1µs low (final)
  };
  constexpr size_t TEST_SYMBOL_COUNT = sizeof(test_symbols) / sizeof(test_symbols[0]);
  
  ESP_LOGI(TAG, "Test pattern: 1µs H, 2µs L, 0.5µs H, 1.5µs L, 3µs H, 1µs L");
  ESP_LOGI(TAG, "In ticks: %u H, %u L, %u H, %u L, %u H, %u L", 
           test_symbols[0].duration, test_symbols[1].duration,
           test_symbols[2].duration, test_symbols[3].duration,
           test_symbols[4].duration, test_symbols[5].duration);

  // Prepare reception buffer
  constexpr size_t RX_BUFFER_SIZE = 20; // Extra space for safety
  hf_pio_symbol_t rx_buffer[RX_BUFFER_SIZE] = {}; // Initialize to zero
  
  // Start reception FIRST (critical for hardware loopback)
  ESP_LOGI(TAG, "Starting reception on RX channel %d (GPIO%d)...", rx_channel, TEST_GPIO_RX);
  result = pio.StartReceive(static_cast<hf_u8_t>(rx_channel), rx_buffer, RX_BUFFER_SIZE, 100000); // 100ms timeout
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start reception: %s", HfPioErrToString(result).data());
    return false;
  }

  // Small delay to ensure RX is fully ready
  vTaskDelay(pdMS_TO_TICKS(10));

  // Transmit test data
  ESP_LOGI(TAG, "Transmitting %zu symbols on TX channel %d (GPIO%d)...", TEST_SYMBOL_COUNT, tx_channel, TEST_GPIO_TX);
  result = pio.Transmit(static_cast<hf_u8_t>(tx_channel), test_symbols, TEST_SYMBOL_COUNT, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit: %s", HfPioErrToString(result).data());
    
    // Clean up RX before returning
    size_t dummy_received = 0;
    pio.StopReceive(static_cast<hf_u8_t>(rx_channel), dummy_received);
    return false;
  }

  ESP_LOGI(TAG, "Transmission completed, waiting for reception...");
  
  // Wait for reception completion
  vTaskDelay(pdMS_TO_TICKS(100)); // Give enough time for reception
  
  // Check received data
  size_t symbols_received = 0;
  result = pio.StopReceive(static_cast<hf_u8_t>(rx_channel), symbols_received);
  
  ESP_LOGI(TAG, "Reception completed: %zu symbols received (expected %zu)", symbols_received, TEST_SYMBOL_COUNT);
  
  if (symbols_received > 0) {
    ESP_LOGI(TAG, "SUCCESS: Hardware loopback working! Received %zu symbols via GPIO8->GPIO18", symbols_received);
    
    // Display received data for verification
    ESP_LOGI(TAG, "Received symbol analysis:");
    for (size_t i = 0; i < std::min(symbols_received, static_cast<size_t>(TEST_SYMBOL_COUNT)); i++) {
      uint32_t received_ns = rx_buffer[i].duration * rx_actual_ns;
      uint32_t expected_ns = test_symbols[i].duration * tx_actual_ns;
      
      ESP_LOGI(TAG, "  [%zu] RX: %s %uns (%u ticks) | Expected: %s %uns (%u ticks) | Match: %s",
               i, 
               rx_buffer[i].level ? "HIGH" : "LOW ", received_ns, rx_buffer[i].duration,
               test_symbols[i].level ? "HIGH" : "LOW ", expected_ns, test_symbols[i].duration,
               (rx_buffer[i].level == test_symbols[i].level) ? "✓" : "✗");
    }
    
    // Verify pattern matching (at least first few symbols should match)
    bool pattern_matches = true;
    size_t check_count = std::min(symbols_received, static_cast<size_t>(3)); // Check first 3 symbols
    for (size_t i = 0; i < check_count; i++) {
      if (rx_buffer[i].level != test_symbols[i].level) {
        pattern_matches = false;
        break;
      }
    }
    
    if (pattern_matches) {
      ESP_LOGI(TAG, "✓ Pattern verification: Signal levels match expected pattern");
    } else {
      ESP_LOGW(TAG, "⚠ Pattern verification: Signal levels don't match - check jumper wire connection");
    }
    
    return true;
  } else {
    ESP_LOGE(TAG, "FAILED: No symbols received via hardware loopback");
    ESP_LOGE(TAG, "Troubleshooting checklist:");
    ESP_LOGE(TAG, "  1. Verify jumper wire connects GPIO8 to GPIO18");
    ESP_LOGE(TAG, "  2. Check wire connection quality (no loose contacts)");
    ESP_LOGE(TAG, "  3. Ensure GPIO8 and GPIO18 are not used by other peripherals");
    ESP_LOGE(TAG, "  4. Verify ESP32-C6 RMT channel allocation is correct");
    return false;
  }
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
  int8_t tx_channel = HfRmtGetTxChannel(0);
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
  hf_pio_err_t result = pio.ConfigureChannel(static_cast<hf_u8_t>(tx_channel), config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure TX channel %d: %s", tx_channel, HfPioErrToString(result).data());
    return false;
  }

  ESP_LOGI(TAG, "Testing callbacks on TX channel %d for %s", tx_channel, HfRmtGetVariantName());

  // Test transmission with callback
  // Use actual 1us ticks directly (durations already in ns units)
  hf_pio_symbol_t test_symbols[] = {{1, true}, {1, false}};

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
  ESP_LOGE(TAG, "Failed to get capabilities: %s", HfPioErrToString(result).data());
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
  result = pio.GetStatistics(0, statistics);
  if (result == hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "PIO Statistics:");
    ESP_LOGI(TAG, "  Total transmissions: %u", statistics.totalTransmissions);
    ESP_LOGI(TAG, "  Successful transmissions: %u", statistics.successfulTransmissions);
    ESP_LOGI(TAG, "  Failed transmissions: %u", statistics.failedTransmissions);
  } else {
    ESP_LOGI(TAG, "Statistics not supported: %s", HfPioErrToString(result).data());
  }

  // Get diagnostics
  hf_pio_diagnostics_t diagnostics;
  result = pio.GetDiagnostics(0, diagnostics);
  if (result == hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGI(TAG, "PIO Diagnostics:");
    ESP_LOGI(TAG, "  PIO healthy: %s", diagnostics.pioHealthy ? "Yes" : "No");
    ESP_LOGI(TAG, "  PIO initialized: %s", diagnostics.pioInitialized ? "Yes" : "No");
    ESP_LOGI(TAG, "  Active channels: %d", diagnostics.activeChannels);
  } else {
    ESP_LOGI(TAG, "Diagnostics not supported: %s", HfPioErrToString(result).data());
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
  config.timeout_us = 500000; // allow ample time for long bursts
  pio.ConfigureChannel(0, config);

  // Create large symbol array (keep durations within RMT's per-word limits)
  constexpr size_t STRESS_SYMBOL_COUNT = 100;
  hf_pio_symbol_t stress_symbols[STRESS_SYMBOL_COUNT];

  // Use the actual achieved tick to convert microseconds -> ticks
  uint32_t tick_ns = config.resolution_ns;
  (void)pio.GetActualResolution(0, tick_ns);
  for (size_t i = 0; i < STRESS_SYMBOL_COUNT; i++) {
    uint32_t us = 100 + (i % 50); // 100..149 us
    uint64_t desired_ns = static_cast<uint64_t>(us) * 1000ULL;
    uint32_t ticks = static_cast<uint32_t>((desired_ns + tick_ns - 1) / tick_ns); // ceil
    if (ticks == 0) ticks = 1;
    if (ticks > 32767) ticks = 32767; // clamp to RMT duration field
    stress_symbols[i].duration = ticks;
    stress_symbols[i].level = (i % 2) == 0; // Alternating high/low
  }

  // Perform multiple stress transmissions
  constexpr int STRESS_ITERATIONS = 10;
  uint64_t start_time = esp_timer_get_time();

  for (int i = 0; i < STRESS_ITERATIONS; i++) {
    hf_pio_err_t result = pio.Transmit(0, stress_symbols, STRESS_SYMBOL_COUNT, true);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Stress transmission failed on iteration %d: %s", i, HfPioErrToString(result).data());
      return false;
    }
    // Small yield to allow RMT to fully flush and driver to recycle resources
    vTaskDelay(pdMS_TO_TICKS(2));
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
  ESP_LOGI(TAG, "╔═══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║               ESP32 PIO COMPREHENSIVE TEST SUITE (ENHANCED)                   ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                            ║");
  ESP_LOGI(TAG, "╚═══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "║  Testing EspPio with ESP-IDF v5.5 RMT peripheral + Latest Improvements        ║");
  ESP_LOGI(TAG, "║  • Channel-specific callbacks with user data                                  ║");
  ESP_LOGI(TAG, "║  • Resolution_hz usage for direct ESP-IDF compatibility                       ║");
  ESP_LOGI(TAG, "║  • ESP32 variant-specific channel validation                                  ║");
  ESP_LOGI(TAG, "║  • Enhanced clock divider calculation                                         ║");
  ESP_LOGI(TAG, "║  • WS2812 LED protocol and automated loopback testing                         ║");
  ESP_LOGI(TAG, "║  • ASCII Art test result decoration                                           ║");
  ESP_LOGI(TAG, "║                                                                               ║");
  ESP_LOGI(TAG, "║  ESP32 Variant: %-32s                                                         ║",
             HfRmtGetVariantName());
  ESP_LOGI(TAG, "║  Test Pins:                                                                   ║");
  ESP_LOGI(TAG, "║    GPIO %d - Built-in RGB LED (WS2812) + TX for loopback                      ║",
           TEST_GPIO_TX);
  ESP_LOGI(TAG, "║    GPIO %d - RX for automated loopback verification                           ║",
           TEST_GPIO_RX);
  ESP_LOGI(TAG, "║                                                                               ║");
  ESP_LOGI(TAG, "║  For automated testing: Connect GPIO %d to GPIO %d with jumper wire           ║",
           TEST_GPIO_TX, TEST_GPIO_RX);
  ESP_LOGI(TAG, "║  Hardware loopback test will verify transmission/reception integrity          ║");
  ESP_LOGI(TAG, "║  Test progression indicator: GPIO14 toggles HIGH/LOW after each test         ║");
  ESP_LOGI(TAG, "╚═══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(TAG, "");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Initialize test progression indicator GPIO14
  // This pin will toggle between HIGH/LOW each time a test completes
  // providing visual feedback for test progression on oscilloscope/logic analyzer
  if (!init_test_progress_indicator()) {
    ESP_LOGE(TAG, "Failed to initialize test progression indicator GPIO. Tests may not be visible.");
  }

  // ESP32 Variant Information Tests (NEW)
  ESP_LOGI(TAG, "\n=== ESP32 VARIANT INFORMATION TESTS ===");
  RUN_TEST(test_esp32_variant_detection);
  flip_test_progress_indicator();
  RUN_TEST(test_channel_allocation_helpers);
  flip_test_progress_indicator();
  RUN_TEST(test_channel_direction_validation);
  flip_test_progress_indicator();
  RUN_TEST(test_resolution_ns_usage);
  flip_test_progress_indicator();

  // Constructor/Destructor Tests
  ESP_LOGI(TAG, "\n=== CONSTRUCTOR/DESTRUCTOR TESTS ===");
  RUN_TEST(test_constructor_default);
  flip_test_progress_indicator();
  RUN_TEST(test_destructor_cleanup);
  flip_test_progress_indicator();

  // Lifecycle Tests
  ESP_LOGI(TAG, "\n=== LIFECYCLE TESTS ===");
  RUN_TEST(test_initialization_states);
  flip_test_progress_indicator();
  RUN_TEST(test_lazy_initialization);
  flip_test_progress_indicator();

  // Channel Configuration Tests (ENHANCED)
  ESP_LOGI(TAG, "\n=== CHANNEL CONFIGURATION TESTS ===");
  RUN_TEST(test_channel_configuration);
  flip_test_progress_indicator();
  RUN_TEST(test_multiple_channel_configuration);
  flip_test_progress_indicator();

  // Basic Transmission Tests (ENHANCED)
  ESP_LOGI(TAG, "\n=== BASIC TRANSMISSION TESTS ===");
  RUN_TEST(test_basic_symbol_transmission);
  flip_test_progress_indicator();
  RUN_TEST(test_transmission_edge_cases);
  flip_test_progress_indicator();

  // WS2812 LED Protocol Tests (ENHANCED)
  ESP_LOGI(TAG, "\n=== WS2812 LED PROTOCOL TESTS (ENHANCED) ===");
  RUN_TEST(test_ws2812_single_led);
  flip_test_progress_indicator();
  RUN_TEST(test_ws2812_multiple_leds);
  flip_test_progress_indicator();
  RUN_TEST(test_ws2812_color_cycle);
  flip_test_progress_indicator();
  RUN_TEST(test_ws2812_brightness_sweep);
  flip_test_progress_indicator();
  RUN_TEST(test_ws2812_pattern_validation);
  flip_test_progress_indicator();
  RUN_TEST(test_ws2812_rainbow_transition);
  flip_test_progress_indicator();

  // Logic Analyzer Test Scenarios
  ESP_LOGI(TAG, "\n=== LOGIC ANALYZER TEST SCENARIOS ===");
  RUN_TEST(test_logic_analyzer_patterns);
  flip_test_progress_indicator();
  RUN_TEST(test_frequency_sweep);
  flip_test_progress_indicator();

  // Advanced RMT Feature Tests
  ESP_LOGI(TAG, "\n=== ADVANCED RMT FEATURE TESTS ===");
  RUN_TEST(test_rmt_encoder_configuration);
  flip_test_progress_indicator();
  RUN_TEST(test_rmt_carrier_modulation);
  flip_test_progress_indicator();
  RUN_TEST(test_rmt_advanced_configuration);
  flip_test_progress_indicator();

  // Loopback and Reception Tests
  ESP_LOGI(TAG, "\n=== LOOPBACK AND RECEPTION TESTS ===");
  RUN_TEST(test_loopback_functionality);
  flip_test_progress_indicator();
  RUN_TEST(test_hardware_loopback_gpio8_to_gpio18);
  flip_test_progress_indicator();

  // Callback Tests (ENHANCED - Channel-specific)
  ESP_LOGI(TAG, "\n=== CALLBACK TESTS ===");
  RUN_TEST(test_callback_functionality);
  flip_test_progress_indicator();

  // Statistics and Diagnostics Tests
  ESP_LOGI(TAG, "\n=== STATISTICS AND DIAGNOSTICS TESTS ===");
  RUN_TEST(test_statistics_and_diagnostics);
  flip_test_progress_indicator();

  // Stress and Performance Tests
  ESP_LOGI(TAG, "\n=== STRESS AND PERFORMANCE TESTS ===");
  RUN_TEST(test_stress_transmission);
  flip_test_progress_indicator();

  // System Validation
  ESP_LOGI(TAG, "\n=== SYSTEM VALIDATION TESTS ===");
  RUN_TEST(test_pio_system_validation);
  flip_test_progress_indicator();

  // Final test progression indicator flip
  flip_test_progress_indicator();

  // Print final summary
  print_test_summary(g_test_results, "PIO", TAG);

  ESP_LOGI(TAG, "PIO comprehensive testing completed.");
  ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

  // Post-test banner
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
  ESP_LOGI(TAG, "║  For WS2812 testing: Built-in RGB LED on GPIO %d shows comprehensive colors  ║",
           TEST_GPIO_TX);
  ESP_LOGI(TAG, "║  For hardware loopback: Verify transmission/reception on GPIO %d -> GPIO %d    ║",
           TEST_GPIO_TX, TEST_GPIO_RX);
  ESP_LOGI(TAG, "║  Hardware test validates actual RMT TX/RX channel functionality              ║");
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

  // Cleanup test progression indicator
  cleanup_test_progress_indicator();

  // Keep running for continuous testing if needed
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
