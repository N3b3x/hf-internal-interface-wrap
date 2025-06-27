/**
 * @file McuAdc.cpp
 * @brief Implementation of platform-agnostic MCU ADC driver.
 *
 * This file provides the implementation for ADC abstraction that automatically
 * adapts to the current MCU platform. Currently supports ESP32-C6, but designed
 * to be easily portable to other MCUs.
 */
#include "McuAdc.h"
#include <cmath>

// Platform-specific includes via McuSelect.h (no need for manual detection)
#ifdef HF_MCU_ESP32C6
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include <unistd.h>
#endif

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

#ifdef HF_MCU_ESP32C6
McuAdc::McuAdc(hf_adc_unit_t adc_unit, uint32_t attenuation, hf_adc_resolution_t width)
    : BaseAdc(), unit_(adc_unit), attenuation_(attenuation), bitwidth_(width), adc_handle_(nullptr),
      cali_handle_(nullptr), cali_enable_(false), initialized_(false), dma_buffer_(nullptr),
      adc_continuous_handle_(nullptr), dma_task_handle_(nullptr), dma_mode_active_(false),
      active_callback_(nullptr), callback_user_data_(nullptr) {
  // ESP32-C6 only supports ADC_UNIT_1, map generic unit to platform-specific
  if (adc_unit != 1) {
    unit_ = 1; // Force to ADC1 for ESP32-C6
  }
}
#else
McuAdc::McuAdc(hf_adc_unit_t adc_unit, uint32_t attenuation, hf_adc_resolution_t width)
    : BaseAdc(), unit_(adc_unit), attenuation_(attenuation), bitwidth_(width), initialized_(false) {
}
#endif

McuAdc::~McuAdc() noexcept {
  Deinitialize();
#ifdef HF_MCU_FAMILY_ESP32
  // Clean up DMA resources if still allocated
  DeinitializeContinuousMode();
#endif
}

//==============================================================================
// INITIALIZATION AND DEINITIALIZATION
//==============================================================================

bool McuAdc::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Configure ADC oneshot unit
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = static_cast<adc_unit_t>(unit_), // Map hf_adc_unit_t to adc_unit_t
      .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
      .ulp_mode = ADC_ULP_MODE_DISABLE};

  esp_err_t ret = adc_oneshot_new_unit(&init_config,
                                       reinterpret_cast<adc_oneshot_unit_handle_t *>(&adc_handle_));
  if (ret != ESP_OK) {
    return false;
  }

  // Initialize calibration
  if (!InitializeCalibration()) {
    adc_oneshot_del_unit(static_cast<adc_oneshot_unit_handle_t>(adc_handle_));
    adc_handle_ = nullptr;
    return false;
  }

  initialized_ = true;
  return true;
#else
  // Add other MCU initialization here
  initialized_ = true;
  return true;
#endif
}

bool McuAdc::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

#ifdef HF_MCU_FAMILY_ESP32
  DeinitializeCalibration();

  if (adc_handle_) {
    adc_oneshot_del_unit(adc_handle_);
    adc_handle_ = nullptr;
  }

  DeinitializeContinuousMode();
#endif

  initialized_ = false;
  return true;
}

//==============================================================================
// CHANNEL INFORMATION
//==============================================================================

uint8_t McuAdc::GetMaxChannels() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return 7; // ESP32-C6 ADC1 has channels 0-6
#else
  return 0; // Unknown for other MCUs
#endif
}

bool McuAdc::IsChannelAvailable(uint8_t channel_num) const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return channel_num < GetMaxChannels();
#else
  return false;
#endif
}

HfAdcErr McuAdc::ValidateChannel(uint8_t channel_num) const noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_INVALID_CHANNEL;
  }

  if (!initialized_) {
    return HfAdcErr::ADC_NOT_INITIALIZED;
  }

  return HfAdcErr::ADC_OK;
}

//==============================================================================
// ADC READING OPERATIONS
//==============================================================================

HfAdcErr McuAdc::ReadChannelCount(uint8_t channel_num, uint32_t &channel_reading_count,
                                  uint8_t numOfSamplesToAvg, uint32_t timeBetweenSamples) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

  if (!initialized_) {
    return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Configure the specific channel
  adc_oneshot_chan_cfg_t config = {.atten = static_cast<adc_atten_t>(attenuation_),
                                   .bitwidth = static_cast<adc_bitwidth_t>(bitwidth_)};

  adc_channel_t mcu_channel = GetMcuChannel(channel_num);
  esp_err_t ret = adc_oneshot_config_channel(static_cast<adc_oneshot_unit_handle_t>(adc_handle_),
                                             mcu_channel, &config);
  if (ret != ESP_OK) {
    return HfAdcErr::ADC_ERR_CHANNEL_READ_ERR;
  }

  // Perform averaged reading
  uint64_t sum = 0;
  for (uint8_t i = 0; i < numOfSamplesToAvg; ++i) {
    int raw_value;
    ret = adc_oneshot_read(static_cast<adc_oneshot_unit_handle_t>(adc_handle_), mcu_channel,
                           &raw_value);
    if (ret != ESP_OK) {
      return HfAdcErr::ADC_ERR_CHANNEL_READ_ERR;
    }

    sum += static_cast<uint32_t>(raw_value);

    if (i < (numOfSamplesToAvg - 1) && timeBetweenSamples > 0) {
      usleep(timeBetweenSamples);
    }
  }

  channel_reading_count = static_cast<uint32_t>(sum / numOfSamplesToAvg);
  return HfAdcErr::ADC_SUCCESS;
#else
  // Add other MCU implementations here
  (void)channel_reading_count;
  (void)numOfSamplesToAvg;
  (void)timeBetweenSamples;
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfAdcErr McuAdc::ReadChannelV(uint8_t channel_num, float &channel_reading_v,
                              uint8_t numOfSamplesToAvg, uint32_t timeBetweenSamples) noexcept {
  uint32_t raw_count;
  HfAdcErr result = ReadChannelCount(channel_num, raw_count, numOfSamplesToAvg, timeBetweenSamples);
  if (result != HfAdcErr::ADC_SUCCESS) {
    return result;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (cali_enable_) {
    int voltage_mv;
    esp_err_t ret = adc_cali_raw_to_voltage(static_cast<adc_cali_handle_t>(cali_handle_),
                                            static_cast<int>(raw_count), &voltage_mv);
    if (ret == ESP_OK) {
      channel_reading_v = static_cast<float>(voltage_mv) / 1000.0f; // Convert mV to V
      return HfAdcErr::ADC_SUCCESS;
    }
  }

  // Fallback to basic calculation if calibration not available
  // ESP32-C6 with 12-bit ADC and 3.3V reference
  channel_reading_v = (static_cast<float>(raw_count) / 4095.0f) * 3.3f;
  return HfAdcErr::ADC_SUCCESS;
#else
  // Add other MCU implementations here
  (void)channel_reading_v;
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfAdcErr McuAdc::ReadChannel(uint8_t channel_num, uint32_t &channel_reading_count,
                             float &channel_reading_v, uint8_t numOfSamplesToAvg,
                             uint32_t timeBetweenSamples) noexcept {
  HfAdcErr result =
      ReadChannelCount(channel_num, channel_reading_count, numOfSamplesToAvg, timeBetweenSamples);
  if (result != HfAdcErr::ADC_SUCCESS) {
    return result;
  }

  return ReadChannelV(channel_num, channel_reading_v, 1, 0); // Use cached raw value
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool McuAdc::InitializeCalibration() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  cali_enable_ = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = unit_, .atten = attenuation_, .bitwidth = bitwidth_};

  esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle_);
  if (ret == ESP_OK) {
    cali_enable_ = true;
    return true;
  }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = unit_, .atten = attenuation_, .bitwidth = bitwidth_};

  esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle_);
  if (ret == ESP_OK) {
    cali_enable_ = true;
    return true;
  }
#endif

  return true; // OK to continue without calibration
#else
  return true;
#endif
}

void McuAdc::DeinitializeCalibration() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (cali_enable_ && cali_handle_) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(cali_handle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(cali_handle_);
#endif
    cali_handle_ = nullptr;
    cali_enable_ = false;
  }
#endif
}

#ifdef HF_MCU_FAMILY_ESP32
adc_channel_t McuAdc::GetMcuChannel(uint8_t channel_num) const noexcept {
  // Map generic channel numbers to ESP32-C6 ADC1 channels
  switch (channel_num) {
  case 0:
    return ADC_CHANNEL_0; // GPIO0
  case 1:
    return ADC_CHANNEL_1; // GPIO1
  case 2:
    return ADC_CHANNEL_2; // GPIO2
  case 3:
    return ADC_CHANNEL_3; // GPIO3
  case 4:
    return ADC_CHANNEL_4; // GPIO4
  case 5:
    return ADC_CHANNEL_5; // GPIO5
  case 6:
    return ADC_CHANNEL_6; // GPIO6
  default:
    return ADC_CHANNEL_0; // Default fallback
  }
}
#else
hf_adc_channel_t McuAdc::GetMcuChannel(uint8_t channel_num) const noexcept {
  return static_cast<hf_adc_channel_t>(channel_num);
}
#endif

//==============================================================================
// LEGACY UTILITY METHODS
//==============================================================================

float McuAdc::CountToVoltage(uint32_t raw_count) const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (cali_enable_) {
    int voltage_mv;
    esp_err_t ret = adc_cali_raw_to_voltage(static_cast<adc_cali_handle_t>(cali_handle_),
                                            static_cast<int>(raw_count), &voltage_mv);
    if (ret == ESP_OK) {
      return static_cast<float>(voltage_mv) / 1000.0f;
    }
  }

  // Fallback calculation
  return (static_cast<float>(raw_count) / 4095.0f) * 3.3f;
#else
  return static_cast<float>(raw_count) / 4095.0f * 3.3f;
#endif
}

uint8_t McuAdc::GetResolutionBits() const noexcept {
  return static_cast<uint8_t>(bitwidth_);
}

uint32_t McuAdc::GetMaxCount() const noexcept {
  uint8_t bits = GetResolutionBits();
  return (1U << bits) - 1;
}

float McuAdc::GetReferenceVoltage() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // ESP32-C6 reference voltage depends on attenuation
  switch (attenuation_) {
  case 0:
    return 1.1f; // 0dB
  case 1:
    return 1.5f; // 2.5dB
  case 2:
    return 2.2f; // 6dB
  case 3:
    return 3.9f; // 11dB
  default:
    return 3.3f;
  }
#else
  return 3.3f; // Default reference voltage
#endif
}

//==============================================================================
// ADVANCED ADC FEATURES IMPLEMENTATION
//==============================================================================

HfAdcErr McuAdc::ConfigureAdvanced(uint8_t channel_num, const AdcAdvancedConfig &config) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // ESP32-C6 supports DMA mode for continuous conversion
  if (config.sampling_mode == SamplingMode::DMA ||
      config.sampling_mode == SamplingMode::Continuous) {
    return InitializeContinuousMode(channel_num, config.sample_rate_hz);
  }
#endif

  // Other advanced features not supported on ESP32-C6 oneshot ADC
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
}

HfAdcErr McuAdc::StartContinuousSampling(uint8_t channel_num, AdcCallback callback,
                                         void *user_data) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (!adc_continuous_handle_) {
    return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
  }

  // Store callback and user data
  active_callback_ = callback;
  callback_user_data_ = user_data;
  current_dma_channel_ = channel_num;

  // Start continuous conversion
  esp_err_t ret =
      adc_continuous_start(static_cast<adc_continuous_handle_t>(adc_continuous_handle_));
  if (ret != ESP_OK) {
    return HfAdcErr::ADC_ERR_HARDWARE_FAULT;
  }

  dma_mode_active_ = true;
  return HfAdcErr::ADC_SUCCESS;
#else
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfAdcErr McuAdc::StopContinuousSampling(uint8_t channel_num) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  if (!adc_continuous_handle_ || !dma_mode_active_) {
    return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
  }

  // Stop continuous conversion
  esp_err_t ret = adc_continuous_stop(static_cast<adc_continuous_handle_t>(adc_continuous_handle_));
  if (ret != ESP_OK) {
    return HfAdcErr::ADC_ERR_HARDWARE_FAULT;
  }

  dma_mode_active_ = false;
  active_callback_ = nullptr;
  callback_user_data_ = nullptr;

  return HfAdcErr::ADC_SUCCESS;
#else
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
#endif
}

//==============================================================================
// DMA AND CONTINUOUS CONVERSION IMPLEMENTATION (ESP32C6)
//==============================================================================

#ifdef HF_MCU_FAMILY_ESP32
#include "freertos/task.h"

HfAdcErr McuAdc::InitializeContinuousMode(uint8_t channel_num, uint32_t sample_rate_hz) noexcept {
  if (adc_continuous_handle_) {
    return HfAdcErr::ADC_ERR_ALREADY_INITIALIZED;
  }

  // Allocate DMA buffer
  dma_buffer_ = static_cast<uint8_t *>(malloc(DMA_BUFFER_SIZE));
  if (!dma_buffer_) {
    return HfAdcErr::ADC_ERR_OUT_OF_MEMORY;
  }

  // Configure continuous conversion
  adc_continuous_handle_cfg_t adc_config = {};
  adc_config.max_store_buf_size = DMA_BUFFER_SIZE;
  adc_config.conv_frame_size = DMA_BUFFER_SIZE / 4; // Smaller frame for responsiveness

  esp_err_t ret = adc_continuous_new_handle(
      &adc_config, reinterpret_cast<adc_continuous_handle_t *>(&adc_continuous_handle_));
  if (ret != ESP_OK) {
    free(dma_buffer_);
    dma_buffer_ = nullptr;
    return HfAdcErr::ADC_ERR_HARDWARE_FAULT;
  }

  // Configure channel
  adc_digi_pattern_config_t adc_pattern = {};
  adc_pattern.atten = static_cast<adc_atten_t>(attenuation_);
  adc_pattern.channel = GetMcuChannel(channel_num);
  adc_pattern.unit = static_cast<adc_unit_t>(unit_);
  adc_pattern.bit_width = static_cast<adc_bitwidth_t>(bitwidth_);

  adc_continuous_config_t dig_cfg = {};
  dig_cfg.sample_freq_hz = sample_rate_hz;
  dig_cfg.conv_mode = ADC_CONV_SINGLE_UNIT_1; // Single unit mode
  dig_cfg.format = ADC_DIGI_OUTPUT_FORMAT_TYPE1;
  dig_cfg.pattern_num = 1;
  dig_cfg.adc_pattern = &adc_pattern;

  ret =
      adc_continuous_config(static_cast<adc_continuous_handle_t>(adc_continuous_handle_), &dig_cfg);
  if (ret != ESP_OK) {
    adc_continuous_del_handle(static_cast<adc_continuous_handle_t>(adc_continuous_handle_));
    free(dma_buffer_);
    dma_buffer_ = nullptr;
    adc_continuous_handle_ = nullptr;
    return HfAdcErr::ADC_ERR_INVALID_CONFIGURATION;
  }

  // Create DMA processing task
  BaseType_t task_ret = xTaskCreate(DmaProcessingTask, "adc_dma_task", 4096, this, 5,
                                    reinterpret_cast<TaskHandle_t *>(&dma_task_handle_));
  if (task_ret != pdPASS) {
    DeinitializeContinuousMode();
    return HfAdcErr::ADC_ERR_SYSTEM_ERROR;
  }

  return HfAdcErr::ADC_SUCCESS;
}

HfAdcErr McuAdc::DeinitializeContinuousMode() noexcept {
  if (dma_mode_active_) {
    StopContinuousSampling(current_dma_channel_);
  }

  // Delete task
  if (dma_task_handle_) {
    vTaskDelete(static_cast<TaskHandle_t>(dma_task_handle_));
    dma_task_handle_ = nullptr;
  }

  // Clean up continuous ADC
  if (adc_continuous_handle_) {
    adc_continuous_del_handle(static_cast<adc_continuous_handle_t>(adc_continuous_handle_));
    adc_continuous_handle_ = nullptr;
  }

  // Free DMA buffer
  if (dma_buffer_) {
    free(dma_buffer_);
    dma_buffer_ = nullptr;
  }

  return HfAdcErr::ADC_SUCCESS;
}

void McuAdc::DmaProcessingTask(void *pvParameters) {
  McuAdc *adc = static_cast<McuAdc *>(pvParameters);
  uint8_t *data_buffer = static_cast<uint8_t *>(malloc(adc->DMA_BUFFER_SIZE));

  if (!data_buffer) {
    vTaskDelete(nullptr);
    return;
  }

  while (adc->adc_continuous_handle_) {
    uint32_t bytes_read = 0;
    esp_err_t ret =
        adc_continuous_read(static_cast<adc_continuous_handle_t>(adc->adc_continuous_handle_),
                            data_buffer, adc->DMA_BUFFER_SIZE, &bytes_read, 1000);

    if (ret == ESP_OK && bytes_read > 0 && adc->dma_mode_active_) {
      adc->ProcessDmaData(data_buffer, bytes_read);
    } else if (ret == ESP_ERR_TIMEOUT) {
      // Normal timeout, continue
      continue;
    } else if (ret != ESP_OK) {
      // Error occurred, stop processing
      break;
    }

    // Small delay to prevent watchdog issues
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  free(data_buffer);
  vTaskDelete(nullptr);
}

void McuAdc::ProcessDmaData(uint8_t *buffer, size_t length) noexcept {
  if (!active_callback_ || !dma_mode_active_) {
    return;
  }

  // Convert raw DMA data to samples
  size_t num_samples = length / sizeof(adc_digi_output_data_t);
  auto *samples = reinterpret_cast<uint16_t *>(malloc(num_samples * sizeof(uint16_t)));

  if (!samples) {
    return;
  }

  auto *adc_data = reinterpret_cast<adc_digi_output_data_t *>(buffer);
  for (size_t i = 0; i < num_samples; i++) {
    samples[i] = adc_data[i].type1.data; // Extract 12-bit ADC data
  }

  // Invoke user callback
  active_callback_(current_dma_channel_, samples, num_samples, callback_user_data_);

  free(samples);
}

#endif // ESP_PLATFORM
//==============================================================================
// CALIBRATION SUPPORT IMPLEMENTATION
//==============================================================================

HfAdcErr McuAdc::CalibrateChannel(uint8_t channel_num, const CalibrationConfig &config,
                                  CalibrationProgressCallback progress_callback,
                                  void *user_data) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

  // For ESP32-C6, we rely on hardware calibration via eFuse
  // Custom calibration would require additional implementation
  (void)config;
  (void)progress_callback;
  (void)user_data;

  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
}

HfAdcErr McuAdc::VerifyCalibration(uint8_t channel_num, float reference_voltage,
                                   float &measured_voltage, float &error_percent) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

  HfAdcErr result = ReadChannelV(channel_num, measured_voltage, 10); // Average 10 samples
  if (result != HfAdcErr::ADC_SUCCESS) {
    return result;
  }

  error_percent = std::abs((measured_voltage - reference_voltage) / reference_voltage) * 100.0f;

  return HfAdcErr::ADC_SUCCESS;
}

HfAdcErr McuAdc::SaveCalibration(uint8_t channel_num) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

  // ESP32-C6 calibration is stored in eFuse, not NVS
  return HfAdcErr::ADC_ERR_UNSUPPORTED_OPERATION;
}

HfAdcErr McuAdc::LoadCalibration(uint8_t channel_num) noexcept {
  if (!IsChannelAvailable(channel_num)) {
    return HfAdcErr::ADC_ERR_INVALID_CHANNEL;
  }

  // ESP32-C6 calibration is loaded from eFuse automatically
  return HfAdcErr::ADC_SUCCESS;
}
