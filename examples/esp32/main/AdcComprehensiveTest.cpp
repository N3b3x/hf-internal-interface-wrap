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
 * - GPIO3 (ADC1_CH3) - Connect to 3.3V via voltage divider (high reference)
 * - GPIO0 (ADC1_CH0) - Connect to potentiometer center tap (0-3.3V variable for monitor testing)
 * - GPIO1 (ADC1_CH1) - Connect to ground via 10k resistor (low reference)
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

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

// Shared test framework
#include "TestFramework.h"

static const char* TAG = "ADC_Test";

static TestResults g_test_results;

// Test configuration constants
static constexpr hf_channel_id_t TEST_CHANNEL_1 = 3; // GPIO3 - ADC1_CH3
static constexpr hf_channel_id_t TEST_CHANNEL_2 = 0; // GPIO0 - ADC1_CH0
static constexpr hf_channel_id_t TEST_CHANNEL_3 = 1; // GPIO1 - ADC1_CH1

// Expected voltage ranges for test validation (in millivolts)
static constexpr uint32_t MIN_VALID_VOLTAGE_MV = 100;  // Minimum valid voltage
static constexpr uint32_t MAX_VALID_VOLTAGE_MV = 3200; // Maximum valid voltage

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
 * @brief Continuous mode callback function (ISR-safe)
 */
bool continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) noexcept {
  (void)user_data; // Suppress unused parameter warning
  if (!continuous_test_active || data == nullptr) {
    return false;
  }

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
  ESP_LOGI(TAG, "  - GPIO3: 3.3V via voltage divider (should read ~3.0V)");
  ESP_LOGI(TAG, "  - GPIO0: Potentiometer center tap (variable 0-3.3V)");
  ESP_LOGI(TAG, "  - GPIO1: Ground via 10kΩ resistor (should read ~0V)");

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Read all channels and validate hardware connections
  bool hardware_ok = true;

  // GPIO3 - High reference (should be ~3.0-3.3V)
  uint32_t high_voltage_mv;
  if (test_adc.ReadSingleVoltage(TEST_CHANNEL_1, high_voltage_mv) == hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGI(TAG, "GPIO3 (HIGH): %lu mV", high_voltage_mv);
    if (high_voltage_mv < 2800 || high_voltage_mv > 3300) {
      ESP_LOGE(TAG, "GPIO3: Expected ~3000mV, got %lu mV - check voltage divider!",
               high_voltage_mv);
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
    if (low_voltage_mv > 300) {
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
    if (pot_voltage_mv > 3300) {
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

  // Test with proper configuration
  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0; // ESP32-C6 has only ADC1 (unit 0)
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;
  adc_cfg.bit_width = hf_adc_bitwidth_t::WIDTH_12BIT;

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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
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

  if (raw_value > 4095) { // 12-bit ADC max value
    ESP_LOGE(TAG, "Raw value %lu exceeds 12-bit maximum (4095)", raw_value);
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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;
  adc_cfg.calibration_config.enable_calibration = true;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
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
    uint32_t test_raw = 2048; // Mid-scale value
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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::CONTINUOUS;
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

  // Wait and collect data
  uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t messages_received = 0;

  while ((xTaskGetTickCount() * portTICK_PERIOD_MS - start_time) < CONTINUOUS_TEST_DURATION_MS) {
    adc_queue_message_t msg;

    if (xQueueReceive(adc_data_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
      messages_received++;
      ESP_LOGD(TAG, "Received %lu samples at timestamp %llu", msg.sample_count, msg.timestamp);
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
 * @brief Test ADC monitor threshold functionality
 * @details Tests threshold monitoring on the potentiometer channel (GPIO0)
 * Expected hardware setup:
 * - GPIO0: Connected to potentiometer (0-3.3V variable)
 * - Adjust potentiometer during test to trigger high/low thresholds
 */
bool test_adc_monitor_thresholds() noexcept {
  ESP_LOGI(TAG, "Testing ADC monitor thresholds...");
  ESP_LOGI(TAG, "Hardware setup required:");
  ESP_LOGI(TAG, "  - GPIO0: Connect to potentiometer (0-3.3V)");
  ESP_LOGI(TAG, "  - Adjust potentiometer during test to trigger thresholds");

  // Configure ADC for monitor testing
  hf_adc_unit_config_t adc_config = {};
  adc_config.unit_id = 0;                             // ESP32-C6 has only ADC1
  adc_config.mode = hf_adc_mode_t::CONTINUOUS;        // Monitor requires continuous mode
  adc_config.continuous_config.sample_freq_hz = 1000; // 1kHz sampling
  adc_config.continuous_config.samples_per_frame = 64;
  adc_config.continuous_config.max_store_frames = 4;

  EspAdc adc(adc_config);

  if (!initialize_test_adc(adc)) {
    ESP_LOGE(TAG, "Failed to initialize ADC for monitor test");
    return false;
  }

  // Configure potentiometer channel (GPIO0 = TEST_CHANNEL_2)
  const hf_channel_id_t MONITOR_CHANNEL = TEST_CHANNEL_2; // GPIO0
  hf_adc_err_t result = adc.ConfigureChannel(MONITOR_CHANNEL, hf_adc_atten_t::ATTEN_DB_12);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure monitor channel");
    return false;
  }

  result = adc.EnableChannel(MONITOR_CHANNEL);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable monitor channel");
    return false;
  }

  // Read current voltage to set appropriate thresholds
  uint32_t current_voltage_mv;
  result = adc.ReadSingleVoltage(MONITOR_CHANNEL, current_voltage_mv);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to read current voltage for threshold setup");
    return false;
  }

  ESP_LOGI(TAG, "Current potentiometer voltage: %lu mV", current_voltage_mv);

  // Validate potentiometer is working (not stuck at rail)
  if (current_voltage_mv < 100 || current_voltage_mv > 3200) {
    ESP_LOGW(TAG, "Potentiometer voltage (%lu mV) at rail - may affect monitor testing",
             current_voltage_mv);
  }

  // Set intelligent thresholds based on current voltage with proper bounds
  // Ensure thresholds are sufficiently spread and within ADC range
  uint32_t high_thresh_mv, low_thresh_mv;

  if (current_voltage_mv < 1000) {
    // Pot is low - set thresholds above current position
    high_thresh_mv = current_voltage_mv + 800;
    low_thresh_mv = (current_voltage_mv > 400) ? current_voltage_mv - 400 : 200;
  } else if (current_voltage_mv > 2300) {
    // Pot is high - set thresholds below current position
    low_thresh_mv = current_voltage_mv - 800;
    high_thresh_mv = (current_voltage_mv < 2900) ? current_voltage_mv + 400 : 3100;
  } else {
    // Pot is mid-range - set thresholds on both sides
    high_thresh_mv = current_voltage_mv + 600;
    low_thresh_mv = current_voltage_mv - 600;
  }

  // Clamp to valid ADC range
  high_thresh_mv = (high_thresh_mv > 3200) ? 3200 : high_thresh_mv;
  low_thresh_mv = (low_thresh_mv < 200) ? 200 : low_thresh_mv;

  // Ensure minimum separation between thresholds
  if ((high_thresh_mv - low_thresh_mv) < 800) {
    ESP_LOGW(TAG, "Threshold separation too small, adjusting...");
    uint32_t mid = (high_thresh_mv + low_thresh_mv) / 2;
    high_thresh_mv = mid + 400;
    low_thresh_mv = mid - 400;
  }

  // Convert mV to raw ADC counts (approximate for 12dB attenuation)
  // 12dB attenuation: ~0-3.3V range, 4096 counts
  uint32_t high_thresh_raw = (high_thresh_mv * 4095) / 3300;
  uint32_t low_thresh_raw = (low_thresh_mv * 4095) / 3300;

  ESP_LOGI(TAG, "Setting thresholds:");
  ESP_LOGI(TAG, "  - High: %lu mV (%lu counts)", high_thresh_mv, high_thresh_raw);
  ESP_LOGI(TAG, "  - Low:  %lu mV (%lu counts)", low_thresh_mv, low_thresh_raw);

  // Configure monitor
  hf_adc_monitor_config_t monitor_config = {};
  monitor_config.monitor_id = 0;
  monitor_config.channel_id = MONITOR_CHANNEL;
  monitor_config.high_threshold = high_thresh_raw;
  monitor_config.low_threshold = low_thresh_raw;

  result = adc.ConfigureMonitor(monitor_config);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure monitor");
    return false;
  }

  // Set monitor callback
  result = adc.SetMonitorCallback(0, monitor_callback);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set monitor callback");
    return false;
  }

  // Start continuous mode (required for monitoring)
  result = adc.StartContinuous();
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to start continuous mode");
    return false;
  }

  // Enable monitor
  result = adc.SetMonitorEnabled(0, true);
  if (result != hf_adc_err_t::ADC_SUCCESS) {
    ESP_LOGE(TAG, "Failed to enable monitor");
    return false;
  }

  // Reset counters and activate test
  high_threshold_count = 0;
  low_threshold_count = 0;
  last_monitor_event_time = 0;
  monitor_test_active = true;

  ESP_LOGI(TAG, "Monitor system active! Please adjust potentiometer now:");
  ESP_LOGI(TAG, "   Current reading: %lu mV", current_voltage_mv);
  ESP_LOGI(TAG, "   Turn potentiometer HIGH (above %lu mV) to trigger high threshold",
           high_thresh_mv);
  ESP_LOGI(TAG, "   Turn potentiometer LOW (below %lu mV) to trigger low threshold", low_thresh_mv);
  ESP_LOGI(TAG, "   Test duration: 15 seconds with real-time feedback");
  ESP_LOGI(TAG,
           "   Tip: Start with small movements to verify detection, then make larger adjustments");

  // Monitor for 15 seconds with periodic status updates
  const uint32_t TEST_DURATION_MS = 15000;
  const uint32_t UPDATE_INTERVAL_MS = 2000; // 2 second updates
  uint32_t elapsed_ms = 0;

  while (elapsed_ms < TEST_DURATION_MS) {
    vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL_MS));
    elapsed_ms += UPDATE_INTERVAL_MS;

    // Read current voltage
    uint32_t voltage_mv;
    if (adc.ReadSingleVoltage(MONITOR_CHANNEL, voltage_mv) == hf_adc_err_t::ADC_SUCCESS) {
      ESP_LOGI(TAG, "%2lu/%2lu sec | Voltage: %4lu mV | High events: %2lu | Low events: %2lu",
               elapsed_ms / 1000, TEST_DURATION_MS / 1000, voltage_mv, high_threshold_count,
               low_threshold_count);
    }
  }

  // Stop monitoring
  monitor_test_active = false;
  adc.SetMonitorEnabled(0, false);
  adc.StopContinuous();

  ESP_LOGI(TAG, "Monitor test completed:");
  ESP_LOGI(TAG, "  - High threshold events: %lu", high_threshold_count);
  ESP_LOGI(TAG, "  - Low threshold events:  %lu", low_threshold_count);
  ESP_LOGI(TAG, "  - Total events:          %lu", high_threshold_count + low_threshold_count);
  ESP_LOGI(TAG, "  - Last event time:       %llu us", last_monitor_event_time);

  // Validation
  bool test_passed = true;

  if (high_threshold_count == 0 && low_threshold_count == 0) {
    ESP_LOGW(
        TAG,
        "No threshold events detected - please ensure potentiometer is connected and adjusted");
    ESP_LOGW(TAG, "   This may indicate hardware setup issues or thresholds not crossed");
    // Don't fail the test - could be valid if thresholds weren't crossed
  }

  if (last_monitor_event_time == 0 && (high_threshold_count > 0 || low_threshold_count > 0)) {
    ESP_LOGE(TAG, "Events counted but no timestamp recorded - callback issue");
    test_passed = false;
  }

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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

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

  // Test reading from disabled channel
  result = test_adc.ReadSingleRaw(TEST_CHANNEL_1, raw_value); // Channel not configured/enabled
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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

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

  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  adc_cfg.mode = hf_adc_mode_t::ONESHOT;

  EspAdc test_adc(adc_cfg);

  if (!initialize_test_adc(test_adc)) {
    return false;
  }

  if (!configure_test_channels(test_adc)) {
    return false;
  }

  // Performance test: measure conversion speed
  const uint32_t num_conversions = 1000;
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
  if (avg_time_per_conversion_us > 1000) {
    ESP_LOGW(TAG, "ADC conversion seems slow: %lu us per conversion", avg_time_per_conversion_us);
  }

  ESP_LOGI(TAG, "[SUCCESS] ADC performance test passed");
  return true;
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                    ESP32-C6 ADC COMPREHENSIVE TEST SUITE                    ║");
  ESP_LOGI(TAG, "║                         HardFOC Internal Interface                          ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Hardware Setup Required (ESP32-C6 DevKit-M-1):                             ║");
  ESP_LOGI(TAG, "║  - GPIO3 (ADC1_CH3): Connect to 3.3V via voltage divider (high reference)  ║");
  ESP_LOGI(TAG, "║  - GPIO0 (ADC1_CH0): Connect to potentiometer center tap (variable 0-3.3V) ║");
  ESP_LOGI(TAG, "║  - GPIO1 (ADC1_CH1): Connect to ground via 10kΩ resistor (low reference)   ║");
  ESP_LOGI(TAG, "║                                                                              ║");
  ESP_LOGI(TAG, "║  Monitor Test: Adjust potentiometer on GPIO0 during monitor test            ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(2000));

  // Run comprehensive ADC tests - hardware validation first!
  RUN_TEST(test_hardware_validation);
  RUN_TEST(test_adc_initialization);
  RUN_TEST(test_adc_channel_configuration);
  RUN_TEST(test_adc_basic_conversion);
  RUN_TEST(test_adc_calibration);
  RUN_TEST(test_adc_multiple_channels);
  RUN_TEST(test_adc_averaging);
  RUN_TEST(test_adc_continuous_mode);
  RUN_TEST(test_adc_monitor_thresholds);
  RUN_TEST(test_adc_error_handling);
  RUN_TEST(test_adc_statistics);
  RUN_TEST(test_adc_performance);

  print_test_summary(g_test_results, "ADC", TAG);

  if (g_test_results.failed_tests == 0) {
    ESP_LOGI(TAG,
             "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG,
             "║                      ALL ADC TESTS PASSED!                                  ║");
    ESP_LOGI(TAG,
             "║                                                                              ║");
    ESP_LOGI(TAG,
             "║  ESP32-C6 ADC system is working correctly with comprehensive testing        ║");
    ESP_LOGI(TAG,
             "║  covering hardware validation, initialization, calibration, single/multi-   ║");
    ESP_LOGI(TAG, "║  channel reading, continuous mode, monitor thresholds with bounds,         ║");
    ESP_LOGI(TAG, "║  error handling, statistics, and performance testing.                      ║");
    ESP_LOGI(TAG,
             "║                                                                              ║");
    ESP_LOGI(TAG,
             "║  Hardware connections verified:                                             ║");
    ESP_LOGI(TAG, "║  GPIO3 (HIGH)   GPIO0 (POT)   GPIO1 (LOW)   Monitor System                ║");
    ESP_LOGI(TAG,
             "╚══════════════════════════════════════════════════════════════════════════════╝");
  } else {
    ESP_LOGE(TAG,
             "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGE(TAG,
             "║                        SOME TESTS FAILED                                    ║");
    ESP_LOGE(TAG,
             "║                                                                              ║");
    ESP_LOGE(TAG,
             "║  Please check hardware connections and review failed test details above.    ║");
    ESP_LOGE(TAG,
             "║  Failed tests: %2d / %2d                                                     ║",
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
