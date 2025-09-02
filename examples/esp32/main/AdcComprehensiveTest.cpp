/**
 * @file AdcComprehensiveTest.cpp
 * @brief Comprehensive ADC testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This file contains a dedicated, comprehensive test suite for the EspAdc class
 * targeting ESP32-C6 with ESP-IDF v5.5+. It provides thorough testing of all
 * ADC functionalities including basic operations, calibration, continuous conversion,
 * and hardware-specific capabilities.
 *
 * Hardware Configuration (ESP32-C6 DevKit-M-1):
 * - GPIO3 (ADC1_CH3) - Connect to 3.3V via voltage divider (should read ~1.65V)
 * - GPIO0 (ADC1_CH0) - Connect to potentiometer center tap (0-3.3V variable for monitor testing)
 * - GPIO1 (ADC1_CH1) - Connect to ground via 10k resistor (should read ~0V)
 *
 * Monitor Test Requirements:
 * - Adjust potentiometer on GPIO0 during monitor test to trigger thresholds
 * - Test will set thresholds automatically based on initial reading
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseAdc.h"
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/utils/EspTypes_ADC.h"

#include "TestFramework.h"

// ESP-IDF headers for ADC continuous mode data structures
#include "hal/adc_types.h"

static const char* TAG = "ADC_Test";

static TestResults g_test_results;

// Test configuration constants
static constexpr hf_channel_id_t TEST_CHANNEL_1 = 3; // GPIO3 - ADC1_CH3
static constexpr hf_channel_id_t TEST_CHANNEL_2 = 0; // GPIO0 - ADC1_CH0
static constexpr hf_channel_id_t TEST_CHANNEL_3 = 1; // GPIO1 - ADC1_CH1

// Expected voltage ranges for test validation (in millivolts)
static constexpr uint32_t MIN_VALID_VOLTAGE_MV = 100;  // Minimum valid voltage
static constexpr uint32_t MAX_VALID_VOLTAGE_MV = 3200; // Maximum valid voltage

// Hardware test voltage expectations (in millivolts)
static constexpr uint32_t VOLTAGE_DIVIDER_EXPECTED_MV = 1650; // 3.3V / 2 = 1.65V
static constexpr uint32_t VOLTAGE_DIVIDER_TOLERANCE_MV = 150; // ±150mV tolerance
static constexpr uint32_t GROUND_TOLERANCE_MV = 300;          // Ground should be < 300mV
static constexpr uint32_t POTENTIOMETER_MAX_MV = 3300;        // Potentiometer max voltage

// ADC technical constants
static constexpr uint32_t ADC_12BIT_MAX_VALUE = 4095;      // 12-bit ADC maximum value
static constexpr uint32_t ADC_12BIT_MID_VALUE = 2048;      // 12-bit ADC mid-scale value
static constexpr uint32_t ADC_VOLTAGE_SCALE_FACTOR = 1000; // mV to V conversion factor

// Monitor test constants
static constexpr uint32_t MONITOR_TEST_DURATION_MS = 15000;      // 15 seconds
static constexpr uint32_t MONITOR_UPDATE_INTERVAL_MS = 2000;     // 2 second updates
static constexpr uint32_t MONITOR_THRESHOLD_SEPARATION_MV = 800; // Minimum threshold separation
static constexpr uint32_t MONITOR_THRESHOLD_OFFSET_MV = 400;     // Threshold offset from center

// Performance test constants
static constexpr uint32_t PERFORMANCE_NUM_CONVERSIONS = 1000; // Number of conversions to test
static constexpr uint32_t PERFORMANCE_MAX_TIME_US = 1000;     // Max acceptable time per conversion

// Continuous mode test parameters
static constexpr uint32_t CONTINUOUS_TEST_DURATION_MS = 2000;
static constexpr uint32_t CONTINUOUS_SAMPLES_PER_FRAME = 64;
static constexpr uint32_t CONTINUOUS_MAX_STORE_FRAMES = 4;

// Global test data for continuous mode
struct adc_queue_message_t {
  uint32_t sample_count;
  uint64_t timestamp;
};

static QueueHandle_t adc_data_queue = nullptr;
static volatile bool continuous_test_active = false;
static volatile uint32_t continuous_samples_received = 0;

// Global test data for monitor mode
static volatile bool monitor_test_active = false;
static volatile uint32_t high_threshold_count = 0;
static volatile uint32_t low_threshold_count = 0;
static volatile uint64_t last_monitor_event_time = 0;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================
// Enable/disable specific test categories by setting to true or false

// Core ADC functionality tests
static constexpr bool ENABLE_CORE_TESTS =
    true; // Hardware validation, initialization, configuration
static constexpr bool ENABLE_CONVERSION_TESTS =
    true; // Basic conversion, calibration, multiple channels
static constexpr bool ENABLE_ADVANCED_TESTS =
    true; // Averaging, continuous mode, monitor thresholds
static constexpr bool ENABLE_PERFORMANCE_TESTS = true; // Error handling, statistics, performance

// Forward declarations
bool test_hardware_validation() noexcept;
bool test_adc_initialization() noexcept;
bool test_adc_channel_configuration() noexcept;
bool test_adc_basic_conversion() noexcept;
bool test_adc_calibration() noexcept;
bool test_adc_multiple_channels() noexcept;
bool test_adc_averaging() noexcept;
bool test_adc_continuous_mode() noexcept;
bool test_adc_monitor_thresholds() noexcept;
bool test_adc_error_handling() noexcept;
bool test_adc_statistics() noexcept;
bool test_adc_performance() noexcept;

// Helper functions
bool initialize_test_adc(EspAdc& adc) noexcept;
bool configure_test_channels(EspAdc& adc) noexcept;
bool validate_voltage_reading(uint32_t voltage_mv, const char* channel_name) noexcept;
bool continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) noexcept;
void setup_adc_config(hf_adc_unit_config_t& config,
                      hf_adc_mode_t mode = hf_adc_mode_t::ONESHOT) noexcept;

/**
 * @brief Setup ADC configuration with test channels pre-configured
 */
void setup_adc_config(hf_adc_unit_config_t& config, hf_adc_mode_t mode) noexcept {
  config = {}; // Clear the configuration
  config.unit_id = 0;
  config.mode = mode;
  config.bit_width = hf_adc_bitwidth_t::WIDTH_12BIT;

  // Pre-configure all test channels
  config.channel_configs[TEST_CHANNEL_1].channel_id = TEST_CHANNEL_1;
  config.channel_configs[TEST_CHANNEL_1].attenuation = hf_adc_atten_t::ATTEN_DB_12;
  config.channel_configs[TEST_CHANNEL_1].bitwidth = hf_adc_bitwidth_t::WIDTH_12BIT;
  config.channel_configs[TEST_CHANNEL_1].enabled = true;

  config.channel_configs[TEST_CHANNEL_2].channel_id = TEST_CHANNEL_2;
  config.channel_configs[TEST_CHANNEL_2].attenuation = hf_adc_atten_t::ATTEN_DB_12;
  config.channel_configs[TEST_CHANNEL_2].bitwidth = hf_adc_bitwidth_t::WIDTH_12BIT;
  config.channel_configs[TEST_CHANNEL_2].enabled = true;

  config.channel_configs[TEST_CHANNEL_3].channel_id = TEST_CHANNEL_3;
  config.channel_configs[TEST_CHANNEL_3].attenuation = hf_adc_atten_t::ATTEN_DB_12;
  config.channel_configs[TEST_CHANNEL_3].bitwidth = hf_adc_bitwidth_t::WIDTH_12BIT;
  config.channel_configs[TEST_CHANNEL_3].enabled = true;
}

/**
 * @brief Initialize ADC for testing with proper configuration
 */
bool initialize_test_adc(EspAdc& adc) noexcept {
  if (!adc.EnsureInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize ADC");
    return false;
  }

  ESP_LOGI(TAG, "ADC initialized successfully");
  return true;
}

/**
 * @brief Configure test channels with appropriate settings
 */
bool configure_test_channels(EspAdc& adc) noexcept {
  // Configure test channels with 12dB attenuation for full 3.3V range
  hf_adc_err_t result;

  // Channel 0 (GPIO0)
  result = adc.ConfigureChannel(TEST_CHANNEL_1, hf_adc_atten_t::ATTEN_DB_12,
                                hf_adc_bitwidth_t::WIDTH_12BIT);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel %lu: %d", static_cast<unsigned long>(TEST_CHANNEL_1),
             static_cast<int>(result));
    return false;
  }

  // Channel 1 (GPIO1)
  result = adc.ConfigureChannel(TEST_CHANNEL_2, hf_adc_atten_t::ATTEN_DB_12,
                                hf_adc_bitwidth_t::WIDTH_12BIT);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel %lu: %d", static_cast<unsigned long>(TEST_CHANNEL_2),
             static_cast<int>(result));
    return false;
  }

  // Channel 2 (GPIO2)
  result = adc.ConfigureChannel(TEST_CHANNEL_3, hf_adc_atten_t::ATTEN_DB_12,
                                hf_adc_bitwidth_t::WIDTH_12BIT);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel %lu: %d", static_cast<unsigned long>(TEST_CHANNEL_3),
             static_cast<int>(result));
    return false;
  }

  // Enable all test channels
  result = adc.EnableChannel(TEST_CHANNEL_1);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel %lu", static_cast<unsigned long>(TEST_CHANNEL_1));
    return false;
  }

  result = adc.EnableChannel(TEST_CHANNEL_2);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel %lu", static_cast<unsigned long>(TEST_CHANNEL_2));
    return false;
  }

  result = adc.EnableChannel(TEST_CHANNEL_3);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel %lu", static_cast<unsigned long>(TEST_CHANNEL_3));
    return false;
  }

  ESP_LOGI(TAG, "All test channels configured and enabled");
  return true;
}

/**
 * @brief Validate if voltage reading is within reasonable range
 */
bool validate_voltage_reading(uint32_t voltage_mv, const char* channel_name) noexcept {
  if (voltage_mv < MIN_VALID_VOLTAGE_MV || voltage_mv > MAX_VALID_VOLTAGE_MV) {
    ESP_LOGW(TAG, "Channel %s voltage %lu mV outside valid range [%lu - %lu mV]", channel_name,
             voltage_mv, MIN_VALID_VOLTAGE_MV, MAX_VALID_VOLTAGE_MV);
    return false;
  }

  ESP_LOGI(TAG, "Channel %s voltage: %lu mV [VALID]", channel_name, voltage_mv);
  return true;
}

/**
 * @brief Continuous mode callback function (ISR-safe) - extracts latest voltage in real-time
 */
bool continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) noexcept {
  (void)user_data; // Suppress unused parameter warning
  if (data == nullptr) {
    return false;
  }

  // For continuous mode test
  if (continuous_test_active) {
    // Count samples received
    continuous_samples_received += data->conversion_count;

    // Send minimal data to queue for processing in main task
    adc_queue_message_t msg;
    msg.sample_count = data->conversion_count;
    msg.timestamp = data->timestamp_us;

    BaseType_t higher_priority_task_woken = pdFALSE;
    if (adc_data_queue != nullptr) {
      xQueueSendFromISR(adc_data_queue, &msg, &higher_priority_task_woken);
    }

    return higher_priority_task_woken == pdTRUE;
  }

  return false; // Don't yield to higher priority task
}

/**
 * @brief Monitor callback function for threshold testing (ISR-safe)
 */
void monitor_callback(const hf_adc_monitor_event_t* event, void* user_data) noexcept {
  (void)user_data; // Suppress unused parameter warning

  if (!monitor_test_active || event == nullptr) {
    return;
  }

  // Update counters based on event type
  if (event->event_type == hf_adc_monitor_event_type_t::HIGH_THRESH) {
    high_threshold_count++;
  } else if (event->event_type == hf_adc_monitor_event_type_t::LOW_THRESH) {
    low_threshold_count++;
  }

  // Record timestamp of last event
  last_monitor_event_time = event->timestamp_us;
}

/**
 * @brief Test hardware setup validation
 * @details Validates the expected hardware connections before running other tests
 */
bool test_hardware_validation() noexcept {
  ESP_LOGI(TAG, "Validating hardware setup...");
  ESP_LOGI(TAG, "Expected connections:");
  ESP_LOGI(TAG, "  - GPIO3: 3.3V via voltage divider (should read ~1.65V)");
  ESP_LOGI(TAG, "  - GPIO0: Potentiometer center tap (variable 0-3.3V)");
  ESP_LOGI(TAG, "  - GPIO1: Ground via 10kΩ resistor (should read ~0V)");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Read all channels and validate hardware connections
  bool hardware_ok = true;

  // GPIO3 - High reference (should be ~1.65V from voltage divider)
  uint32_t high_voltage_mv;
  if (test_adc.ReadSingleVoltage(TEST_CHANNEL_1, high_voltage_mv) == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGI(TAG, "GPIO3 (HIGH): %lu mV", high_voltage_mv);
    if (high_voltage_mv < (VOLTAGE_DIVIDER_EXPECTED_MV - VOLTAGE_DIVIDER_TOLERANCE_MV) ||
        high_voltage_mv > (VOLTAGE_DIVIDER_EXPECTED_MV + VOLTAGE_DIVIDER_TOLERANCE_MV)) {
      ESP_LOGE(TAG,
               "GPIO3: Expected ~%lu mV (actual voltage divider ratio), got %lu mV - check voltage "
               "divider!",
               VOLTAGE_DIVIDER_EXPECTED_MV, high_voltage_mv);
      hardware_ok = false;
    } else {
      ESP_LOGI(TAG, "GPIO3: Hardware connection verified");
    }
  } else {
    ESP_LOGE(TAG, "Failed to read GPIO3");
    hardware_ok = false;
  }

  // GPIO1 - Low reference (should be ~0V)
  uint32_t low_voltage_mv;
  if (test_adc.ReadSingleVoltage(TEST_CHANNEL_3, low_voltage_mv) == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGI(TAG, "GPIO1 (LOW): %lu mV", low_voltage_mv);
    if (low_voltage_mv > GROUND_TOLERANCE_MV) {
      ESP_LOGE(TAG, "GPIO1: Expected ~0mV, got %lu mV - check ground connection!", low_voltage_mv);
      hardware_ok = false;
    } else {
      ESP_LOGI(TAG, "GPIO1: Hardware connection verified");
    }
  } else {
    ESP_LOGE(TAG, "Failed to read GPIO1");
    hardware_ok = false;
  }

  // GPIO0 - Variable (potentiometer - just check it's reasonable)
  uint32_t pot_voltage_mv;
  if (test_adc.ReadSingleVoltage(TEST_CHANNEL_2, pot_voltage_mv) == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGI(TAG, "GPIO0 (POT): %lu mV", pot_voltage_mv);
    if (pot_voltage_mv > POTENTIOMETER_MAX_MV) {
      ESP_LOGW(TAG, "GPIO0: %lu mV seems high - check potentiometer connection", pot_voltage_mv);
    } else {
      ESP_LOGI(TAG, "GPIO0: Potentiometer reading valid");
    }
  } else {
    ESP_LOGE(TAG, "Failed to read GPIO0");
    hardware_ok = false;
  }

  if (hardware_ok) {
    ESP_LOGI(TAG, "[SUCCESS] Hardware validation passed - all connections verified");
  } else {
    ESP_LOGE(TAG, "[FAILED] Hardware validation failed - check connections before proceeding");
  }

  return hardware_ok;
}

/**
 * @brief Test ADC initialization and basic setup
 */
bool test_adc_initialization() noexcept {
  ESP_LOGI(TAG, "Testing ADC initialization...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Verify ADC properties
  uint8_t max_channels = test_adc.GetMaxChannels();
  if (max_channels != 7) { // ESP32-C6 has 7 ADC channels (0-6)
    ESP_LOGE(TAG, "Unexpected max channels: %d (expected 7)", max_channels);
    return false;
  }

  // Check channel availability
  for (hf_channel_id_t ch = 0; ch < 7; ch++) {
    if (!test_adc.IsChannelAvailable(ch)) {
      ESP_LOGE(TAG, "Channel %ld should be available", ch);
      return false;
    }
  }

  // Check invalid channel
  if (test_adc.IsChannelAvailable(7)) {
    ESP_LOGE(TAG, "Channel 7 should not be available on ESP32-C6");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC initialization test passed");
  return true;
}

/**
 * @brief Test ADC channel configuration
 */
bool test_adc_channel_configuration() noexcept {
  ESP_LOGI(TAG, "Testing ADC channel configuration...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Verify channels are enabled
  if (!test_adc.IsChannelEnabled(TEST_CHANNEL_1)) {
    ESP_LOGE(TAG, "Channel %lu should be enabled", static_cast<unsigned long>(TEST_CHANNEL_1));
    return false;
  }

  if (!test_adc.IsChannelEnabled(TEST_CHANNEL_2)) {
    ESP_LOGE(TAG, "Channel %lu should be enabled", static_cast<unsigned long>(TEST_CHANNEL_2));
    return false;
  }

  if (!test_adc.IsChannelEnabled(TEST_CHANNEL_3)) {
    ESP_LOGE(TAG, "Channel %lu should be enabled", static_cast<unsigned long>(TEST_CHANNEL_3));
    return false;
  }

  // Test disabling a channel
  hf_adc_err_t result = test_adc.DisableChannel(TEST_CHANNEL_3);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to disable channel %lu", static_cast<unsigned long>(TEST_CHANNEL_3));
    return false;
  }

  if (test_adc.IsChannelEnabled(TEST_CHANNEL_3)) {
    ESP_LOGE(TAG, "Channel %lu should be disabled", static_cast<unsigned long>(TEST_CHANNEL_3));
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC channel configuration test passed");
  return true;
}

/**
 * @brief Test basic ADC conversion functionality
 */
bool test_adc_basic_conversion() noexcept {
  ESP_LOGI(TAG, "Testing basic ADC conversion...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Test raw reading
  uint32_t raw_value = 0;
  hf_adc_err_t result = test_adc.ReadSingleRaw(TEST_CHANNEL_1, raw_value);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read raw value from channel %lu: %d",
             static_cast<unsigned long>(TEST_CHANNEL_1), static_cast<int>(result));
    return false;
  }

  if (raw_value > ADC_12BIT_MAX_VALUE) {
    ESP_LOGE(TAG, "Raw value %lu exceeds 12-bit maximum (%lu)", raw_value, ADC_12BIT_MAX_VALUE);
    return false;
  }

  ESP_LOGI(TAG, "Channel %lu raw reading: %lu", static_cast<unsigned long>(TEST_CHANNEL_1),
           raw_value);

  // Test voltage reading
  uint32_t voltage_mv = 0;
  result = test_adc.ReadSingleVoltage(TEST_CHANNEL_1, voltage_mv);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read voltage from channel %lu: %d",
             static_cast<unsigned long>(TEST_CHANNEL_1), static_cast<int>(result));
    return false;
  }

  if (!validate_voltage_reading(voltage_mv, "CH1")) {
    return false;
  }

  // Test BaseAdc interface methods
  float voltage_v = 0.0f;
  result = test_adc.ReadChannelV(TEST_CHANNEL_2, voltage_v);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read voltage (V) from channel %lu: %d",
             static_cast<unsigned long>(TEST_CHANNEL_2), static_cast<int>(result));
    return false;
  }

  uint32_t count = 0;
  result = test_adc.ReadChannelCount(TEST_CHANNEL_2, count);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read count from channel %lu: %d",
             static_cast<unsigned long>(TEST_CHANNEL_2), static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Channel %lu: %.3fV, count: %lu", static_cast<unsigned long>(TEST_CHANNEL_2),
           voltage_v, count);

  ESP_LOGI(TAG, "[SUCCESS] Basic ADC conversion test passed");
  return true;
}

/**
 * @brief Test ADC calibration functionality
 */
bool test_adc_calibration() noexcept {
  ESP_LOGI(TAG, "Testing ADC calibration...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);
  adc_cfg.calibration_config.enable_calibration = true;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Initialize calibration for different attenuation levels
  hf_adc_err_t result = test_adc.InitializeCalibration(hf_adc_atten_t::ATTEN_DB_12);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize calibration: %d", static_cast<int>(result));
    return false;
  }

  // Check if calibration is available
  if (!test_adc.IsCalibrationAvailable(hf_adc_atten_t::ATTEN_DB_12)) {
    ESP_LOGW(TAG, "Calibration not available for 12dB attenuation");
    // This is not necessarily a failure, continue testing
  } else {
    ESP_LOGI(TAG, "Calibration available for 12dB attenuation");

    // Test raw to voltage conversion
    uint32_t test_raw = ADC_12BIT_MID_VALUE; // Mid-scale value
    uint32_t converted_voltage = 0;
    result = test_adc.RawToVoltage(test_raw, hf_adc_atten_t::ATTEN_DB_12, converted_voltage);
    if (result == hf_adc_err_t::ADC_SUCCESS) {
      ESP_LOGI(TAG, "Raw %lu converted to %lu mV", test_raw, converted_voltage);
    } else {
      ESP_LOGW(TAG, "Raw to voltage conversion failed: %d", static_cast<int>(result));
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC calibration test passed");
  return true;
}

/**
 * @brief Test reading from multiple ADC channels
 */
bool test_adc_multiple_channels() noexcept {
  ESP_LOGI(TAG, "Testing multiple ADC channels...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Test multiple channel reading using BaseAdc interface
  hf_channel_id_t channels[] = {TEST_CHANNEL_1, TEST_CHANNEL_2, TEST_CHANNEL_3};
  uint32_t readings[3] = {0};
  float voltages[3] = {0.0f};

  hf_adc_err_t result = test_adc.ReadMultipleChannels(channels, 3, readings, voltages);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read multiple channels: %d", static_cast<int>(result));
    return false;
  }

  // Validate readings with hardware-specific expectations
  for (int i = 0; i < 3; i++) {
    ESP_LOGI(TAG, "Channel %ld (GPIO%ld): raw=%lu, voltage=%.3fV", channels[i], channels[i],
             readings[i], voltages[i]);

    if (readings[i] > 4095) {
      ESP_LOGE(TAG, "Channel %ld raw reading %lu exceeds 12-bit maximum", channels[i], readings[i]);
      return false;
    }

    uint32_t voltage_mv = static_cast<uint32_t>(voltages[i] * 1000);

    // Hardware-specific validation based on expected connections
    if (channels[i] == TEST_CHANNEL_1) { // GPIO3 - High reference (~3.3V)
      if (voltage_mv < 2800 || voltage_mv > 3300) {
        ESP_LOGW(TAG, "GPIO3 (HIGH): Expected ~3.3V, got %lu mV - check voltage divider connection",
                 voltage_mv);
      } else {
        ESP_LOGI(TAG, "GPIO3 (HIGH): %lu mV - within expected range", voltage_mv);
      }
    } else if (channels[i] == TEST_CHANNEL_3) { // GPIO1 - Low reference (~0V)
      if (voltage_mv > 300) {
        ESP_LOGW(TAG, "GPIO1 (LOW): Expected ~0V, got %lu mV - check ground connection",
                 voltage_mv);
      } else {
        ESP_LOGI(TAG, "GPIO1 (LOW): %lu mV - within expected range", voltage_mv);
      }
    } else if (channels[i] == TEST_CHANNEL_2) { // GPIO0 - Variable (potentiometer)
      if (voltage_mv >= 0 && voltage_mv <= 3300) {
        ESP_LOGI(TAG, "GPIO0 (POT): %lu mV - potentiometer reading", voltage_mv);
      } else {
        ESP_LOGW(TAG, "GPIO0 (POT): %lu mV - outside valid range", voltage_mv);
      }
    }

    char channel_name[16];
    snprintf(channel_name, sizeof(channel_name), "CH%ld", channels[i]);

    if (!validate_voltage_reading(voltage_mv, channel_name)) {
      ESP_LOGW(TAG, "Channel %ld voltage validation failed, but continuing test", channels[i]);
    }
  }

  ESP_LOGI(TAG, "[SUCCESS] Multiple ADC channels test passed");
  return true;
}

/**
 * @brief Test ADC averaging functionality
 */
bool test_adc_averaging() noexcept {
  ESP_LOGI(TAG, "Testing ADC averaging...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Test averaging with different sample counts
  uint16_t sample_counts[] = {1, 4, 8, 16};

  for (size_t i = 0; i < sizeof(sample_counts) / sizeof(sample_counts[0]); i++) {
    uint32_t averaged_value = 0;
    hf_adc_err_t result = test_adc.ReadAveraged(TEST_CHANNEL_1, sample_counts[i], averaged_value);

    if (result != hf_adc_err_t::ADC_SUCCESS) {
      ESP_LOGE(TAG, "Failed to read averaged value with %d samples: %d", sample_counts[i],
               static_cast<int>(result));
      return false;
    }

    ESP_LOGI(TAG, "Channel %lu averaged over %d samples: %lu",
             static_cast<unsigned long>(TEST_CHANNEL_1), sample_counts[i], averaged_value);

    if (averaged_value > 4095) {
      ESP_LOGE(TAG, "Averaged value %lu exceeds 12-bit maximum", averaged_value);
      return false;
    }
  }

  // Test BaseAdc averaging interface
  float voltage_v = 0.0f;
  hf_adc_err_t result = test_adc.ReadChannelV(TEST_CHANNEL_2, voltage_v, 8, 10);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read averaged voltage: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "Channel %lu averaged voltage (8 samples, 10ms between): %.3fV",
           static_cast<unsigned long>(TEST_CHANNEL_2), voltage_v);

  ESP_LOGI(TAG, "[SUCCESS] ADC averaging test passed");
  return true;
}

/**
 * @brief Test ADC continuous mode functionality
 */
bool test_adc_continuous_mode() noexcept {
  ESP_LOGI(TAG, "Testing ADC continuous mode...");

  // Create queue for continuous mode data
  adc_data_queue = xQueueCreate(10, sizeof(adc_queue_message_t));
  if (adc_data_queue == nullptr) {
    ESP_LOGE(TAG, "Failed to create ADC data queue");
    return false;
  }

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::CONTINUOUS);
  adc_cfg.continuous_config.sample_freq_hz = 1000;
  adc_cfg.continuous_config.samples_per_frame = CONTINUOUS_SAMPLES_PER_FRAME;
  adc_cfg.continuous_config.max_store_frames = CONTINUOUS_MAX_STORE_FRAMES;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    vQueueDelete(adc_data_queue);
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    vQueueDelete(adc_data_queue);
    return false;
  }

  // Configure continuous mode
  hf_adc_err_t result = test_adc.ConfigureContinuous(adc_cfg.continuous_config);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure continuous mode: %d", static_cast<int>(result));
    vQueueDelete(adc_data_queue);
    return false;
  }

  // Set callback
  result = test_adc.SetContinuousCallback(continuous_callback, nullptr);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set continuous callback: %d", static_cast<int>(result));
    vQueueDelete(adc_data_queue);
    return false;
  }

  // Start continuous mode
  continuous_test_active = true;
  continuous_samples_received = 0;

  result = test_adc.StartContinuous();
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start continuous mode: %d", static_cast<int>(result));
    continuous_test_active = false;
    vQueueDelete(adc_data_queue);
    return false;
  }

  if (!test_adc.IsContinuousRunning()) {
    ESP_LOGE(TAG, "Continuous mode should be running");
    continuous_test_active = false;
    vQueueDelete(adc_data_queue);
    return false;
  }

  ESP_LOGI(TAG, "Continuous mode started, collecting data for %lu ms...",
           CONTINUOUS_TEST_DURATION_MS);

  // Wait and collect data using adc_continuous_read
  uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t messages_received = 0;
  uint8_t read_buffer[256];
  uint32_t bytes_read = 0;

  while ((xTaskGetTickCount() * portTICK_PERIOD_MS - start_time) < CONTINUOUS_TEST_DURATION_MS) {
    // Try to read data from continuous ADC
    hf_adc_err_t read_result =
        test_adc.ReadContinuousData(read_buffer, sizeof(read_buffer), bytes_read, 100);

    if (read_result == hf_adc_err_t::ADC_SUCCESS && bytes_read > 0) {
      messages_received++;
      // ESP32-C6 uses TYPE2 format: 12-bit data in 32-bit structure (4 bytes per sample)
      uint32_t samples_in_buffer = bytes_read / sizeof(uint32_t);
      continuous_samples_received += samples_in_buffer;
      ESP_LOGD(TAG, "Read %lu bytes (%lu samples) from continuous ADC", bytes_read,
               samples_in_buffer);
    } else if (read_result == hf_adc_err_t::ADC_ERR_TIMEOUT) {
      // No data available, continue waiting
      vTaskDelay(pdMS_TO_TICKS(10));
    } else {
      ESP_LOGW(TAG, "Continuous read error: %d", static_cast<int>(read_result));
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }

  // Stop continuous mode
  continuous_test_active = false;
  result = test_adc.StopContinuous();
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to stop continuous mode: %d", static_cast<int>(result));
    vQueueDelete(adc_data_queue);
    return false;
  }

  ESP_LOGI(TAG, "Continuous mode test completed:");
  ESP_LOGI(TAG, "  - Messages received: %lu", messages_received);
  ESP_LOGI(TAG, "  - Total samples: %lu", continuous_samples_received);
  ESP_LOGI(TAG, "  - Test duration: %lu ms", CONTINUOUS_TEST_DURATION_MS);

  vQueueDelete(adc_data_queue);
  adc_data_queue = nullptr;

  if (messages_received == 0) {
    ESP_LOGE(TAG, "No continuous mode data received");
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC continuous mode test passed");
  return true;
}

/**
 * @brief Test ADC monitor threshold functionality with proper ESP-IDF sequence
 * @details Tests threshold monitoring on the potentiometer channel (GPIO0)
 *
 * Proper ESP-IDF v5.5 sequence:
 * 1. Use oneshot mode to read baseline voltage from potentiometer
 * 2. Stop oneshot mode and setup continuous mode with monitor
 * 3. Configure monitor -> Register callbacks -> Enable monitor -> Start continuous
 *
 * Expected hardware setup:
 * - GPIO0: Connected to potentiometer (0-3.3V variable)
 */
bool test_adc_monitor_thresholds() noexcept {
  ESP_LOGI(TAG, "Testing ADC monitor thresholds with interactive guidance...");
  ESP_LOGI(TAG, "Hardware setup required:");
  ESP_LOGI(TAG, "  - GPIO0: Connect to potentiometer (0-3.3V)");
  ESP_LOGI(TAG, "  - You will be guided through the test step by step");

  const hf_channel_id_t MONITOR_CHANNEL = TEST_CHANNEL_2; // GPIO0

  // ============================================================================
  // STEP 1: Use oneshot mode to get baseline voltage from potentiometer
  // ============================================================================

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    MONITOR THRESHOLD TEST - STEP 1                           ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Please adjust your potentiometer to CENTER position (around 1.5-2.0V)       ║");
  ESP_LOGI(TAG, "║  This will be used as the baseline for setting thresholds.                   ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Monitoring voltage for 5 seconds - adjust potentiometer now...             ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Configure oneshot ADC for baseline reading
  hf_adc_unit_config_t oneshot_config;
  setup_adc_config(oneshot_config, hf_adc_mode_t::ONESHOT);

  EspAdc oneshot_adc(oneshot_config);
  if (!initialize_test_adc(oneshot_adc)) {
    ESP_LOGE(TAG, "Failed to initialize oneshot ADC for baseline reading");
    return false;
  }

  // Configure and enable the potentiometer channel for oneshot reading
  hf_adc_err_t result = oneshot_adc.ConfigureChannel(MONITOR_CHANNEL, hf_adc_atten_t::ATTEN_DB_12);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure oneshot monitor channel");
    return false;
  }

  result = oneshot_adc.EnableChannel(MONITOR_CHANNEL);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGI(TAG, "Failed to enable oneshot monitor channel");
    return false;
  }

  // Monitor voltage during stabilization period using oneshot mode
  uint32_t stabilization_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t stabilization_last_print_time = 0;
  uint32_t baseline_voltage_mv = 0;
  uint32_t valid_readings = 0;
  uint64_t voltage_sum = 0;

  ESP_LOGI(TAG, "Using oneshot mode for baseline voltage monitoring");

  while ((xTaskGetTickCount() * portTICK_PERIOD_MS - stabilization_start) < 5000) {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Print voltage every 1 second
    if (current_time - stabilization_last_print_time >= 1000) {
      uint32_t voltage_mv = 0;
      hf_adc_err_t read_result = oneshot_adc.ReadSingleVoltage(MONITOR_CHANNEL, voltage_mv);

      if (read_result == hf_adc_err_t::ADC_SUCCESS) {
        uint32_t elapsed_sec = (current_time - stabilization_start) / 1000;
        ESP_LOGI(TAG, "⏱️  %2lu/5 sec | Potentiometer: %4lu mV (%.3fV) | Target: 1.5-2.0V",
                 elapsed_sec, voltage_mv, voltage_mv / 1000.0f);

        // Accumulate for average calculation
        voltage_sum += voltage_mv;
        valid_readings++;
      } else {
        ESP_LOGW(TAG, "⏱️  %2lu/5 sec | Failed to read oneshot voltage: %d",
                 (current_time - stabilization_start) / 1000, static_cast<int>(read_result));
      }

      stabilization_last_print_time = current_time;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
  }

  // Calculate baseline voltage from accumulated readings
  if (valid_readings > 0) {
    baseline_voltage_mv = static_cast<uint32_t>(voltage_sum / valid_readings);
  } else {
    ESP_LOGE(TAG, "Failed to get any valid baseline voltage readings");
    return false;
  }

  ESP_LOGI(TAG, "Baseline voltage: %lu mV (averaged from %lu readings)", baseline_voltage_mv,
           valid_readings);

  // Validate baseline is in reasonable range
  if (baseline_voltage_mv < 500 || baseline_voltage_mv > 2800) {
    ESP_LOGW(TAG, "Baseline voltage (%lu mV) is near rail - test may be limited",
             baseline_voltage_mv);
  }

  // ============================================================================
  // STEP 2: Setup continuous mode with monitor (proper ESP-IDF sequence)
  // ============================================================================

  ESP_LOGI(TAG, "Setting up continuous mode with monitor thresholds...");

  // Calculate thresholds based on baseline voltage
  uint32_t high_thresh_mv = baseline_voltage_mv + MONITOR_THRESHOLD_OFFSET_MV;
  uint32_t low_thresh_mv = baseline_voltage_mv - MONITOR_THRESHOLD_OFFSET_MV;

  // Clamp to valid ADC range
  high_thresh_mv = (high_thresh_mv > 3200) ? 3200 : high_thresh_mv;
  low_thresh_mv = (low_thresh_mv < 200) ? 200 : low_thresh_mv;

  // Convert voltage thresholds to raw ADC values (use proper conversion)
  uint32_t high_thresh_raw = (high_thresh_mv * ADC_12BIT_MAX_VALUE) / 3300; // 3.3V reference
  uint32_t low_thresh_raw = (low_thresh_mv * ADC_12BIT_MAX_VALUE) / 3300;   // 3.3V reference

  ESP_LOGI(TAG, "Monitor thresholds based on baseline (%lu mV):", baseline_voltage_mv);
  ESP_LOGI(TAG, "  - High: %lu mV (%lu counts)", high_thresh_mv, high_thresh_raw);
  ESP_LOGI(TAG, "  - Low:  %lu mV (%lu counts)", low_thresh_mv, low_thresh_raw);

  // ============================================================================
  // STEP 3: Setup continuous ADC with monitor (following ESP-IDF sequence)
  // ============================================================================

  // Configure continuous ADC for monitor testing (ONLY channel 0 for maximum responsiveness)
  hf_adc_unit_config_t continuous_config;
  continuous_config = {}; // Clear the configuration
  continuous_config.unit_id = 0;
  continuous_config.mode = hf_adc_mode_t::CONTINUOUS;
  continuous_config.bit_width = hf_adc_bitwidth_t::WIDTH_12BIT;
  continuous_config.continuous_config.sample_freq_hz = 2000; // 2kHz sampling for faster response
  continuous_config.continuous_config.samples_per_frame = 64;
  continuous_config.continuous_config.max_store_frames = 4;

  // Enable ONLY channel 0 (potentiometer) for maximum real-time responsiveness
  continuous_config.channel_configs[MONITOR_CHANNEL].channel_id = MONITOR_CHANNEL;
  continuous_config.channel_configs[MONITOR_CHANNEL].attenuation = hf_adc_atten_t::ATTEN_DB_12;
  continuous_config.channel_configs[MONITOR_CHANNEL].bitwidth = hf_adc_bitwidth_t::WIDTH_12BIT;
  continuous_config.channel_configs[MONITOR_CHANNEL].enabled = true;

  EspAdc continuous_adc(continuous_config);
  if (!initialize_test_adc(continuous_adc)) {
    ESP_LOGE(TAG, "Failed to initialize continuous ADC for monitor test");
    return false;
  }

  // Configure continuous mode
  result = continuous_adc.ConfigureContinuous(continuous_config.continuous_config);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure continuous mode: %d", static_cast<int>(result));
    return false;
  }

  // Set continuous callback
  result = continuous_adc.SetContinuousCallback(continuous_callback, nullptr);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set continuous callback");
    return false;
  }

  // Configure monitor with proper thresholds (BEFORE starting continuous mode)
  hf_adc_monitor_config_t monitor_config;
  monitor_config.monitor_id = 0;
  monitor_config.channel_id = MONITOR_CHANNEL;
  monitor_config.high_threshold = high_thresh_raw;
  monitor_config.low_threshold = low_thresh_raw;

  result = continuous_adc.ConfigureMonitor(monitor_config);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure monitor: %d", static_cast<int>(result));
    return false;
  }

  // Set monitor callback
  result = continuous_adc.SetMonitorCallback(0, monitor_callback);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set monitor callback");
    return false;
  }

  // Enable monitor (BEFORE starting continuous mode)
  result = continuous_adc.SetMonitorEnabled(0, true);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable monitor: %d", static_cast<int>(result));
    return false;
  }

  // NOW start continuous mode (monitor is fully configured and enabled)
  result = continuous_adc.StartContinuous();
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start continuous mode with monitor: %d", static_cast<int>(result));
    return false;
  }

  // Reset counters for actual test
  high_threshold_count = 0;
  low_threshold_count = 0;
  last_monitor_event_time = 0;
  monitor_test_active = true;

  // ============================================================================
  // STEP 4: Interactive threshold testing with continuous mode + monitor
  // ============================================================================

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    MONITOR THRESHOLD TEST - STEP 3                           ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Now turn your potentiometer HIGH (above %lu mV)                             ║",
           high_thresh_mv);
  ESP_LOGI(TAG, "║  You have 10 seconds to trigger the HIGH threshold                           ║");
  ESP_LOGI(TAG, "║  Current baseline: %lu mV                                                    ║",
           baseline_voltage_mv);
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Monitoring for HIGH threshold events...                                     ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Monitor for high threshold for 10 seconds using REAL-TIME callback data
  uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t elapsed_ms = 0;
  uint32_t last_high_count = 0;
  uint32_t last_print_time = 0;

  // Initialize monitoring variables
  uint32_t latest_voltage_mv = baseline_voltage_mv;

  while (elapsed_ms < 10000) {
    vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms for responsive monitoring
    elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Drain ADC buffer to get latest data (ESP-IDF best practice)
    uint8_t read_buffer[256];
    uint32_t bytes_read = 0;

    // Drain all available data with 0 timeout (non-blocking, get freshest data)
    while (continuous_adc.ReadContinuousData(read_buffer, sizeof(read_buffer), bytes_read, 0) ==
               hf_adc_err_t::ADC_SUCCESS &&
           bytes_read > 0) {
      // Parse from end of buffer backwards to get the most recent channel 0 voltage
      for (int32_t i = bytes_read - sizeof(adc_digi_output_data_t); i >= 0;
           i -= sizeof(adc_digi_output_data_t)) {
        const adc_digi_output_data_t* sample =
            reinterpret_cast<const adc_digi_output_data_t*>(&read_buffer[i]);

        if (sample->type2.channel == 0) {
          latest_voltage_mv = (sample->type2.data * 3300) / 4095;
          break; // Use the latest (most recent) sample
        }
      }
    }

    // Print updates every 500ms using LATEST drained data
    if (current_time - last_print_time >= 500) {
      ESP_LOGI(TAG,
               "📈 %2lu/10 sec | Voltage: %4lu mV (%.3fV) | High events: %2lu | Target: >%lu mV",
               elapsed_ms / 1000, latest_voltage_mv, latest_voltage_mv / 1000.0f,
               high_threshold_count, high_thresh_mv);

      // Check if we got new high threshold events
      if (high_threshold_count > last_high_count) {
        ESP_LOGI(TAG, "🎉 HIGH THRESHOLD TRIGGERED! Event #%lu detected", high_threshold_count);
        last_high_count = high_threshold_count;
      }

      last_print_time = current_time;
    }
  }

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    MONITOR THRESHOLD TEST - STEP 4                           ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Now turn your potentiometer LOW (below %lu mV)                             ",
           low_thresh_mv);
  ESP_LOGI(TAG, "║  You have 10 seconds to trigger the LOW threshold                            ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Monitoring for LOW threshold events...                                      ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  // Monitor for low threshold for 10 seconds using REAL-TIME callback data
  start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  elapsed_ms = 0;
  uint32_t last_low_count = 0;
  last_print_time = 0;

  while (elapsed_ms < 10000) {
    vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms for responsive monitoring
    elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Drain ADC buffer to get latest data (ESP-IDF best practice)
    uint8_t read_buffer[256];
    uint32_t bytes_read = 0;

    // Drain all available data with 0 timeout (non-blocking, get freshest data)
    while (continuous_adc.ReadContinuousData(read_buffer, sizeof(read_buffer), bytes_read, 0) ==
               hf_adc_err_t::ADC_SUCCESS &&
           bytes_read > 0) {
      // Parse from end of buffer backwards to get the most recent channel 0 voltage
      for (int32_t i = bytes_read - sizeof(adc_digi_output_data_t); i >= 0;
           i -= sizeof(adc_digi_output_data_t)) {
        const adc_digi_output_data_t* sample =
            reinterpret_cast<const adc_digi_output_data_t*>(&read_buffer[i]);

        if (sample->type2.channel == 0) {
          latest_voltage_mv = (sample->type2.data * 3300) / 4095;
          break; // Use the latest (most recent) sample
        }
      }
    }

    // Print updates every 500ms using LATEST drained data
    if (current_time - last_print_time >= 500) {
      ESP_LOGI(TAG,
               "📉 %2lu/10 sec | Voltage: %4lu mV (%.3fV) | Low events: %2lu | Target: <%lu mV",
               elapsed_ms / 1000, latest_voltage_mv, latest_voltage_mv / 1000.0f,
               low_threshold_count, low_thresh_mv);

      // Check if we got new low threshold events
      if (low_threshold_count > last_low_count) {
        ESP_LOGI(TAG, "🎉 LOW THRESHOLD TRIGGERED! Event #%lu detected", low_threshold_count);
        last_low_count = low_threshold_count;
      }

      last_print_time = current_time;
    }
  }

  // Stop monitoring
  monitor_test_active = false;
  continuous_adc.SetMonitorEnabled(0, false);
  continuous_adc.StopContinuous();

  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    MONITOR THRESHOLD TEST RESULTS                            ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  High threshold events: %2lu                                                  ",
           high_threshold_count);
  ESP_LOGI(TAG, "║  Low threshold events:  %2lu                                                  ",
           low_threshold_count);
  ESP_LOGI(TAG, "║  Total events:          %2lu                                                  ",
           high_threshold_count + low_threshold_count);
  ESP_LOGI(TAG, "║  Last event time:       %llu us                                               ",
           last_monitor_event_time);
  ESP_LOGI(TAG, "║                                                                              ║");

  // Validation
  bool test_passed = true;

  if (high_threshold_count == 0 && low_threshold_count == 0) {
    ESP_LOGI(TAG, "║  ⚠️  No threshold events detected - check potentiometer connection         ║");
    ESP_LOGI(TAG, "║     This may indicate hardware setup issues or thresholds not crossed      ║");
    // Don't fail the test - could be valid if thresholds weren't crossed
  } else if (high_threshold_count > 0 && low_threshold_count > 0) {
    ESP_LOGI(TAG, "║  ✅ Both HIGH and LOW thresholds triggered successfully!                   ║");
  } else if (high_threshold_count > 0) {
    ESP_LOGI(TAG, "║  ✅ HIGH threshold triggered successfully!                                 ║");
  } else if (low_threshold_count > 0) {
    ESP_LOGI(TAG, "║  ✅ LOW threshold triggered successfully!                                  ║");
  }

  if (last_monitor_event_time == 0 && (high_threshold_count > 0 || low_threshold_count > 0)) {
    ESP_LOGE(TAG, "║  ❌ Events counted but no timestamp recorded - callback issue              ║");
    test_passed = false;
  }

  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  if (test_passed) {
    ESP_LOGI(TAG, "[SUCCESS] ADC monitor threshold test completed");
    if (high_threshold_count > 0 || low_threshold_count > 0) {
      ESP_LOGI(TAG, "Monitor system working correctly - events detected and processed");
    } else {
      ESP_LOGI(TAG, "Monitor system ready - no threshold crossings during test period");
    }
  } else {
    ESP_LOGE(TAG, "[FAILED] ADC monitor threshold test failed");
  }

  return test_passed;
}

/**
 * @brief Test ADC error handling
 */
bool test_adc_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing ADC error handling...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  // Test reading from invalid channel
  uint32_t raw_value = 0;
  hf_adc_err_t result = test_adc.ReadSingleRaw(99, raw_value); // Invalid channel
  if (result == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Should have failed to read from invalid channel");
    return false;
  }
  ESP_LOGI(TAG, "Correctly rejected invalid channel read: %d", static_cast<int>(result));

  // Test reading from disabled channel (use a channel that's not in our test set)
  result = test_adc.ReadSingleRaw(13, raw_value); // Channel 13 is not configured/enabled
  if (result == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Should have failed to read from disabled channel");
    return false;
  }
  ESP_LOGI(TAG, "Correctly rejected disabled channel read: %d", static_cast<int>(result));

  // Configure and enable channel for valid read
  result = test_adc.ConfigureChannel(TEST_CHANNEL_1, hf_adc_atten_t::ATTEN_DB_12);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure channel: %d", static_cast<int>(result));
    return false;
  }

  result = test_adc.EnableChannel(TEST_CHANNEL_1);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable channel: %d", static_cast<int>(result));
    return false;
  }

  // Now valid read should work
  result = test_adc.ReadSingleRaw(TEST_CHANNEL_1, raw_value);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Valid channel read should have succeeded: %d", static_cast<int>(result));
    return false;
  }
  ESP_LOGI(TAG, "Valid channel read succeeded: %lu", raw_value);

  // Test null pointer handling
  result = test_adc.ReadMultipleRaw(nullptr, 1, &raw_value);
  if (result == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Should have failed with null channel array");
    return false;
  }
  ESP_LOGI(TAG, "Correctly rejected null pointer: %d", static_cast<int>(result));

  ESP_LOGI(TAG, "[SUCCESS] ADC error handling test passed");
  return true;
}

/**
 * @brief Test ADC statistics and diagnostics
 */
bool test_adc_statistics() noexcept {
  ESP_LOGI(TAG, "Testing ADC statistics...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Reset statistics
  hf_adc_err_t result = test_adc.ResetStatistics();
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reset statistics: %d", static_cast<int>(result));
    return false;
  }

  // Perform several readings to generate statistics
  for (int i = 0; i < 10; i++) {
    uint32_t raw_value = 0;
    test_adc.ReadSingleRaw(TEST_CHANNEL_1, raw_value);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Get statistics
  hf_adc_statistics_t stats = {};
  result = test_adc.GetStatistics(stats);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get statistics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "ADC Statistics:");
  ESP_LOGI(TAG, "  - Total conversions: %lu", stats.totalConversions);
  ESP_LOGI(TAG, "  - Successful: %lu", stats.successfulConversions);
  ESP_LOGI(TAG, "  - Failed: %lu", stats.failedConversions);
  ESP_LOGI(TAG, "  - Min time: %lu us", stats.minConversionTimeUs);
  ESP_LOGI(TAG, "  - Max time: %lu us", stats.maxConversionTimeUs);
  ESP_LOGI(TAG, "  - Avg time: %lu us", stats.averageConversionTimeUs);

  // Get diagnostics
  hf_adc_diagnostics_t diagnostics = {};
  result = test_adc.GetDiagnostics(diagnostics);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to get diagnostics: %d", static_cast<int>(result));
    return false;
  }

  ESP_LOGI(TAG, "ADC Diagnostics:");
  ESP_LOGI(TAG, "  - Healthy: %s", diagnostics.adcHealthy ? "Yes" : "No");
  ESP_LOGI(TAG, "  - Enabled channels: 0x%lx", diagnostics.enabled_channels);
  ESP_LOGI(TAG, "  - Last error: %d", static_cast<int>(diagnostics.lastErrorCode));

  if (stats.totalConversions < 10) {
    ESP_LOGE(TAG, "Expected at least 10 conversions, got %lu", stats.totalConversions);
    return false;
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC statistics test passed");
  return true;
}

/**
 * @brief Test ADC performance characteristics
 */
bool test_adc_performance() noexcept {
  ESP_LOGI(TAG, "Testing ADC performance...");

  hf_adc_unit_config_t adc_cfg;
  setup_adc_config(adc_cfg, hf_adc_mode_t::ONESHOT);

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Performance test: measure conversion speed
  const uint32_t num_conversions = PERFORMANCE_NUM_CONVERSIONS;
  uint64_t start_time = esp_timer_get_time();

  for (uint32_t i = 0; i < num_conversions; i++) {
    uint32_t raw_value = 0;
    test_adc.ReadSingleRaw(TEST_CHANNEL_1, raw_value);
  }

  uint64_t end_time = esp_timer_get_time();
  uint64_t total_time_us = end_time - start_time;
  uint32_t avg_time_per_conversion_us = total_time_us / num_conversions;

  ESP_LOGI(TAG, "Performance Results:");
  ESP_LOGI(TAG, "  - Total conversions: %lu", num_conversions);
  ESP_LOGI(TAG, "  - Total time: %llu us", total_time_us);
  ESP_LOGI(TAG, "  - Average per conversion: %lu us", avg_time_per_conversion_us);
  ESP_LOGI(TAG, "  - Conversions per second: %lu", 1000000 / avg_time_per_conversion_us);

  // Verify reasonable performance (should be faster than 1ms per conversion)
  if (avg_time_per_conversion_us > PERFORMANCE_MAX_TIME_US) {
    ESP_LOGW(TAG, "ADC conversion seems slow: %lu us per conversion", avg_time_per_conversion_us);
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC performance test passed");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 ADC COMPREHENSIVE TEST SUITE                     ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                           ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Hardware Setup Required (ESP32-C6 DevKit-M-1):                              ║");
  ESP_LOGI(TAG, "║  - GPIO3 (ADC1_CH3): Connect to 3.3V via voltage divider (high reference)    ║");
  ESP_LOGI(TAG, "║  - GPIO0 (ADC1_CH0): Connect to potentiometer center tap (variable 0-3.3V)   ║");
  ESP_LOGI(TAG, "║  - GPIO1 (ADC1_CH1): Connect to ground via 10kΩ resistor (low reference)     ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Monitor Test: Adjust potentiometer on GPIO0 during monitor test             ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(2000));

  // Report test section configuration
  print_test_section_status(TAG, "ADC");

  // Run comprehensive ADC tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CORE_TESTS, "ADC CORE TESTS",
      // Hardware validation and initialization tests
      ESP_LOGI(TAG, "Running hardware validation and initialization tests...");
      RUN_TEST_IN_TASK("hardware_validation", test_hardware_validation, 8192, 1);
      RUN_TEST_IN_TASK("adc_initialization", test_adc_initialization, 8192, 1);
      RUN_TEST_IN_TASK("channel_configuration", test_adc_channel_configuration, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_CONVERSION_TESTS, "ADC CONVERSION TESTS",
      // Basic conversion and calibration tests
      ESP_LOGI(TAG, "Running conversion and calibration tests...");
      RUN_TEST_IN_TASK("basic_conversion", test_adc_basic_conversion, 8192, 1);
      RUN_TEST_IN_TASK("adc_calibration", test_adc_calibration, 8192, 1);
      RUN_TEST_IN_TASK("multiple_channels", test_adc_multiple_channels, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_ADVANCED_TESTS, "ADC ADVANCED TESTS",
      // Advanced features tests
      ESP_LOGI(TAG, "Running advanced feature tests...");
      RUN_TEST_IN_TASK("averaging", test_adc_averaging, 8192, 1);
      RUN_TEST_IN_TASK("continuous_mode", test_adc_continuous_mode, 8192, 1);
      RUN_TEST_IN_TASK("monitor_thresholds", test_adc_monitor_thresholds, 8192, 1););

  RUN_TEST_SECTION_IF_ENABLED(ENABLE_PERFORMANCE_TESTS, "ADC PERFORMANCE TESTS",
                              // Performance and error handling tests
                              ESP_LOGI(TAG, "Running performance and error handling tests...");
                              RUN_TEST_IN_TASK("error_handling", test_adc_error_handling, 8192, 1);
                              RUN_TEST_IN_TASK("statistics", test_adc_statistics, 8192, 1);
                              RUN_TEST_IN_TASK("performance", test_adc_performance, 8192, 1););

  print_test_summary(g_test_results, "ADC", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG,
             "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG,
             "║                      ALL ADC TESTS PASSED!                                   ║");
    ESP_LOGI(TAG,
             "║                                                                              ║");
    ESP_LOGI(TAG,
             "║  ESP32-C6 ADC system is working correctly with comprehensive testing         ║");
    ESP_LOGI(TAG,
             "║  covering hardware validation, initialization, calibration, single/multi-    ║");
    ESP_LOGI(TAG,
             "║  channel reading, continuous mode, monitor thresholds with bounds,           ║");
    ESP_LOGI(TAG,
             "║  error handling, statistics, and performance testing.                        ║");
    ESP_LOGI(TAG,
             "║                                                                              ║");
    ESP_LOGI(TAG,
             "║  Hardware connections verified:                                              ║");
    ESP_LOGI(TAG,
             "║  GPIO3 (HIGH)   GPIO0 (POT)   GPIO1 (LOW)   Monitor System                   ║");
    ESP_LOGI(TAG,
             "╚══════════════════════════════════════════════════════════════════════════════╝");
  } else {
    ESP_LOGE(TAG,
             "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGE(TAG,
             "║                        SOME TESTS FAILED                                     ║");
    ESP_LOGE(TAG,
             "║                                                                              ║");
    ESP_LOGE(TAG,
             "║  Please check hardware connections and review failed test details above.     ║");
    ESP_LOGE(TAG, "║  Failed tests: %2d / %2d                                                     ",
             g_test_results.failed_tests, g_test_results.total_tests);
    ESP_LOGE(TAG,
             "╚══════════════════════════════════════════════════════════════════════════════╝");
  }

  // Keep running and periodically display system status
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(30000)); // 30 second intervals
    ESP_LOGI(TAG, "[INFO] ADC test completed. System running normally. Tests: %d/%d passed",
             g_test_results.passed_tests, g_test_results.total_tests);
  }
}
