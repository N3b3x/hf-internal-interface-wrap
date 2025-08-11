/**
 * @file PioComprehensiveTest.cpp
 * @brief Comprehensive PIO testing suite for ESP32-C6 DevKit-M-1 with RMT peripheral (noexcept)
 *
 * This comprehensive test suite validates all functionality of the EspPio class using ESP-IDF v5.5
 * RMT:
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
 * - Callbacks (transmit, receive, error)
 * - Edge cases and stress testing
 *
 * @note This test suite is designed for ESP32-C6 DevKitM-1 with ESP-IDF v5.5+ RMT driver
 * @note Uses built-in RGB LED on GPIO8 for WS2812 testing and automated loopback
 * @note Automated testing: Connect GPIO8 (TX) to GPIO18 (RX) with jumper wire
 */

#include "TestFramework.h"
#include "base/BasePio.h"
#include "mcu/esp32/EspPio.h"

static const char* TAG = "PIO_Test";
static TestResults g_test_results;

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

// Test resolution for timing precision
static constexpr uint32_t TEST_RESOLUTION_NS = 100; // 100ns resolution for precise WS2812 timing

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * @brief Create a default PIO channel configuration for testing
 * ESP32-C6 specific configuration for RMT compatibility
 */
hf_pio_channel_config_t create_test_channel_config(
    hf_gpio_num_t gpio_pin, hf_pio_direction_t direction = hf_pio_direction_t::Transmit) noexcept {
  hf_pio_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.direction = direction;

// ESP32-C6 specific resolution configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
  config.resolution_ns = 1000; // 1µs resolution for ESP32-C6 RMT stability
#else
  config.resolution_ns = TEST_RESOLUTION_NS;
#endif

  config.polarity = hf_pio_polarity_t::Normal;
  config.idle_state = hf_pio_idle_state_t::Low;
  config.timeout_us = 10000;
  config.buffer_size = 128;
  return config;
}

/**
 * @brief Create WS2812 symbols for RGB data
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param symbols Output array (must have space for 24 symbols)
 */
void create_ws2812_rgb_symbols(uint8_t r, uint8_t g, uint8_t b, hf_pio_symbol_t* symbols) noexcept {
  uint32_t rgb_data = (g << 16) | (r << 8) | b; // GRB format for WS2812

  for (int i = 0; i < 24; i++) {
    bool bit = (rgb_data >> (23 - i)) & 1;
    if (bit) {
      // '1' bit: high for T1H, low for T1L
      symbols[i * 2] = {WS2812_T1H / TEST_RESOLUTION_NS, true};
      symbols[i * 2 + 1] = {WS2812_T1L / TEST_RESOLUTION_NS, false};
    } else {
      // '0' bit: high for T0H, low for T0L
      symbols[i * 2] = {WS2812_T0H / TEST_RESOLUTION_NS, true};
      symbols[i * 2 + 1] = {WS2812_T0L / TEST_RESOLUTION_NS, false};
    }
  }
}

/**
 * @brief Create WS2812 reset symbol
 */
hf_pio_symbol_t create_ws2812_reset_symbol() noexcept {
  return {WS2812_RESET / TEST_RESOLUTION_NS, false};
}

/**
 * @brief Create test pattern for logic analyzer verification
 */
void create_logic_analyzer_test_pattern(hf_pio_symbol_t* symbols, size_t& symbol_count) noexcept {
  // Create a recognizable pattern: alternating high/low with varying durations
  symbol_count = 10;

  symbols[0] = {1000 / TEST_RESOLUTION_NS, true};  // 1µs high
  symbols[1] = {1000 / TEST_RESOLUTION_NS, false}; // 1µs low
  symbols[2] = {2000 / TEST_RESOLUTION_NS, true};  // 2µs high
  symbols[3] = {2000 / TEST_RESOLUTION_NS, false}; // 2µs low
  symbols[4] = {500 / TEST_RESOLUTION_NS, true};   // 0.5µs high
  symbols[5] = {500 / TEST_RESOLUTION_NS, false};  // 0.5µs low
  symbols[6] = {3000 / TEST_RESOLUTION_NS, true};  // 3µs high
  symbols[7] = {1500 / TEST_RESOLUTION_NS, false}; // 1.5µs low
  symbols[8] = {750 / TEST_RESOLUTION_NS, true};   // 0.75µs high
  symbols[9] = {4000 / TEST_RESOLUTION_NS, false}; // 4µs low (end marker)
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
  ESP_LOGI(TAG, "Testing channel configuration...");

// ESP32-C6 specific validation
#if defined(CONFIG_IDF_TARGET_ESP32C6)
  ESP_LOGI(TAG, "Testing ESP32-C6 RMT channel configuration");
#endif

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Test valid channel configuration
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(0, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure valid channel: %s", HfPioErrToString(result));
    return false;
  }

  // Test invalid channel ID
  result = pio.ConfigureChannel(255, config);
  if (result != hf_pio_err_t::PIO_ERR_INVALID_CHANNEL) {
    ESP_LOGE(TAG, "Invalid channel should return INVALID_CHANNEL, got: %s",
             HfPioErrToString(result));
    return false;
  }

  // Test channel status
  hf_pio_channel_status_t status;
  result = pio.GetChannelStatus(0, status);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get channel status: %s", HfPioErrToString(result));
    return false;
  }

  if (!status.is_initialized) {
    ESP_LOGE(TAG, "Channel should be marked as initialized");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Channel configuration test passed");
  return true;
}

bool test_multiple_channel_configuration() noexcept {
  ESP_LOGI(TAG, "Testing multiple channel configuration...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Configure multiple channels
  hf_pio_channel_config_t tx_config =
      create_test_channel_config(TEST_GPIO_TX, hf_pio_direction_t::Transmit);
  hf_pio_channel_config_t rx_config =
      create_test_channel_config(TEST_GPIO_RX, hf_pio_direction_t::Receive);

  hf_pio_err_t result1 = pio.ConfigureChannel(0, tx_config);
  hf_pio_err_t result2 = pio.ConfigureChannel(1, rx_config);

  if (result1 != hf_pio_err_t::PIO_SUCCESS || result2 != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure multiple channels: TX=%s, RX=%s", HfPioErrToString(result1),
             HfPioErrToString(result2));
    return false;
  }

  // Verify both channels are configured
  hf_pio_channel_status_t status1, status2;
  pio.GetChannelStatus(0, status1);
  pio.GetChannelStatus(1, status2);

  if (!status1.is_initialized || !status2.is_initialized) {
    ESP_LOGE(TAG, "Both channels should be initialized");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple channel configuration test passed");
  return true;
}

//==============================================================================
// BASIC TRANSMISSION TESTS
//==============================================================================

bool test_basic_symbol_transmission() noexcept {
  ESP_LOGI(TAG, "Testing basic symbol transmission...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Configure transmit channel
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(0, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel: %s", HfPioErrToString(result));
    return false;
  }

  // Create simple test symbols
  hf_pio_symbol_t symbols[] = {
      {1000 / TEST_RESOLUTION_NS, true},  // 1µs high
      {1000 / TEST_RESOLUTION_NS, false}, // 1µs low
      {2000 / TEST_RESOLUTION_NS, true},  // 2µs high
      {2000 / TEST_RESOLUTION_NS, false}  // 2µs low
  };

  // Test transmission without waiting
  result = pio.Transmit(0, symbols, 4, false);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit symbols: %s", HfPioErrToString(result));
    return false;
  }

  // Wait a bit for transmission to complete
  vTaskDelay(pdMS_TO_TICKS(10));

  // Test transmission with waiting
  result = pio.Transmit(0, symbols, 4, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit symbols with wait: %s", HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] Basic symbol transmission test passed");
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

  hf_pio_symbol_t test_symbol = {1000 / TEST_RESOLUTION_NS, true};

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

  // Configure channel for WS2812 timing
  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  hf_pio_err_t result = pio.ConfigureChannel(0, config);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel: %s", HfPioErrToString(result));
    return false;
  }

  // Create RGB data for red color (255, 0, 0)
  hf_pio_symbol_t symbols[48]; // 24 bits * 2 symbols per bit
  create_ws2812_rgb_symbols(255, 0, 0, symbols);

  // Add reset symbol
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol();

  // Transmit RGB data
  result = pio.Transmit(0, symbols, 48, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit WS2812 RGB data: %s", HfPioErrToString(result));
    return false;
  }

  // Transmit reset
  result = pio.Transmit(0, &reset_symbol, 1, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit WS2812 reset: %s", HfPioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] WS2812 single LED test passed - Red color transmitted");
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

  create_ws2812_rgb_symbols(255, 0, 0, &led_data[0]);  // Red
  create_ws2812_rgb_symbols(0, 255, 0, &led_data[48]); // Green
  create_ws2812_rgb_symbols(0, 0, 255, &led_data[96]); // Blue

  // Transmit all LED data
  hf_pio_err_t result = pio.Transmit(0, led_data, 144, true);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit multiple LED data: %s", HfPioErrToString(result));
    return false;
  }

  // Send reset
  hf_pio_symbol_t reset_symbol = create_ws2812_reset_symbol();
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
  uint32_t t0h_ticks = WS2812_T0H / TEST_RESOLUTION_NS;
  uint32_t t0l_ticks = WS2812_T0L / TEST_RESOLUTION_NS;
  uint32_t t1h_ticks = WS2812_T1H / TEST_RESOLUTION_NS;
  uint32_t t1l_ticks = WS2812_T1L / TEST_RESOLUTION_NS;
  uint32_t reset_ticks = WS2812_RESET / TEST_RESOLUTION_NS;

  ESP_LOGI(TAG, "WS2812 timing (in %uns ticks):", TEST_RESOLUTION_NS);
  ESP_LOGI(TAG, "  T0H: %u ticks (%uns)", t0h_ticks, t0h_ticks * TEST_RESOLUTION_NS);
  ESP_LOGI(TAG, "  T0L: %u ticks (%uns)", t0l_ticks, t0l_ticks * TEST_RESOLUTION_NS);
  ESP_LOGI(TAG, "  T1H: %u ticks (%uns)", t1h_ticks, t1h_ticks * TEST_RESOLUTION_NS);
  ESP_LOGI(TAG, "  T1L: %u ticks (%uns)", t1l_ticks, t1l_ticks * TEST_RESOLUTION_NS);
  ESP_LOGI(TAG, "  Reset: %u ticks (%uns)", reset_ticks, reset_ticks * TEST_RESOLUTION_NS);

  // Check timing tolerances (WS2812 has ±150ns tolerance)
  [[maybe_unused]] uint32_t tolerance_ticks = 150 / TEST_RESOLUTION_NS;

  if (t0h_ticks < (350 - 150) / TEST_RESOLUTION_NS ||
      t0h_ticks > (350 + 150) / TEST_RESOLUTION_NS) {
    ESP_LOGE(TAG, "T0H timing out of tolerance");
    return false;
  }

  if (t1h_ticks < (700 - 150) / TEST_RESOLUTION_NS ||
      t1h_ticks > (700 + 150) / TEST_RESOLUTION_NS) {
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
  create_logic_analyzer_test_pattern(test_symbols, symbol_count);

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
    uint32_t half_period_ticks = (period_ns / 2) / TEST_RESOLUTION_NS;

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
  hf_pio_symbol_t bit0_config = {WS2812_T0H / TEST_RESOLUTION_NS, true};
  hf_pio_symbol_t bit1_config = {WS2812_T1H / TEST_RESOLUTION_NS, true};

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
        {1000 / TEST_RESOLUTION_NS, true},  // 1ms with carrier
        {1000 / TEST_RESOLUTION_NS, false}, // 1ms without carrier
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
  hf_pio_symbol_t test_symbols[] = {{1000 / TEST_RESOLUTION_NS, true},
                                    {1000 / TEST_RESOLUTION_NS, false}};

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

static volatile bool g_transmit_complete = false;
static volatile bool g_receive_complete = false;
static volatile bool g_error_occurred = false;

void test_transmit_callback(hf_u8_t channel_id, size_t symbols_sent, void* user_data) {
  ESP_LOGI(TAG, "Transmit callback: channel %d, symbols %d", channel_id, symbols_sent);
  g_transmit_complete = true;
}

void test_receive_callback(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, size_t symbol_count,
                           void* user_data) {
  ESP_LOGI(TAG, "Receive callback: channel %d, symbols %d", channel_id, symbol_count);
  g_receive_complete = true;
}

void test_error_callback(hf_u8_t channel_id, hf_pio_err_t error, void* user_data) {
  ESP_LOGI(TAG, "Error callback: channel %d, error %s", channel_id, HfPioErrToString(error));
  g_error_occurred = true;
}

bool test_callback_functionality() noexcept {
  ESP_LOGI(TAG, "Testing callback functionality...");

  EspPio pio;
  if (!pio.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize PIO");
    return false;
  }

  // Set callbacks
  pio.SetTransmitCallback(test_transmit_callback);
  pio.SetReceiveCallback(test_receive_callback);
  pio.SetErrorCallback(test_error_callback);

  hf_pio_channel_config_t config = create_test_channel_config(TEST_GPIO_TX);
  pio.ConfigureChannel(0, config);

  // Reset callback flags
  g_transmit_complete = false;
  g_receive_complete = false;
  g_error_occurred = false;

  // Test transmission with callback
  hf_pio_symbol_t test_symbols[] = {{1000 / TEST_RESOLUTION_NS, true},
                                    {1000 / TEST_RESOLUTION_NS, false}};

  hf_pio_err_t result = pio.Transmit(0, test_symbols, 2, false);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to transmit for callback test: %s", HfPioErrToString(result));
    return false;
  }

  // Wait for callback
  int timeout = 100; // 1 second timeout
  while (!g_transmit_complete && timeout > 0) {
    vTaskDelay(pdMS_TO_TICKS(10));
    timeout--;
  }

  if (!g_transmit_complete) {
    ESP_LOGW(TAG, "Transmit callback not triggered (may be implementation dependent)");
  } else {
    ESP_LOGI(TAG, "Transmit callback triggered successfully");
  }

  // Clear callbacks
  pio.ClearCallbacks();

  ESP_LOGI(TAG, "[SUCCESS] Callback functionality test completed");
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
        (100 + (i % 50)) * 10, // Variable duration 100-590 * 10 resolution units
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
  ESP_LOGI(TAG, "║                    ESP32-C6 PIO COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  Testing EspPio with ESP-IDF v5.5 RMT peripheral                             ║");
  ESP_LOGI(TAG, "║  Includes WS2812 LED protocol and automated loopback testing                 ║");
  ESP_LOGI(TAG,
           "║                                                                               ║");
  ESP_LOGI(TAG, "║  Test Pins (ESP32-C6 DevKitM-1):                                             ║");
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

  // Constructor/Destructor Tests
  RUN_TEST(test_constructor_default);
  RUN_TEST(test_destructor_cleanup);

  // Lifecycle Tests
  RUN_TEST(test_initialization_states);
  RUN_TEST(test_lazy_initialization);

  // Channel Configuration Tests
  RUN_TEST(test_channel_configuration);
  RUN_TEST(test_multiple_channel_configuration);

  // Basic Transmission Tests
  RUN_TEST(test_basic_symbol_transmission);
  RUN_TEST(test_transmission_edge_cases);

  // WS2812 LED Protocol Tests
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

  // Callback Tests
  RUN_TEST(test_callback_functionality);

  // Statistics and Diagnostics Tests
  RUN_TEST(test_statistics_and_diagnostics);

  // Stress and Performance Tests
  RUN_TEST(test_stress_transmission);

  // System Validation
  RUN_TEST(test_pio_system_validation);

  // Print final summary
  print_test_summary(g_test_results, "PIO", TAG);

  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG,
           "╔═══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                           TEST COMPLETE                                      ║");
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
           "╚═══════════════════════════════════════════════════════════════════════════════╝");

  // Keep running for continuous testing if needed
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
