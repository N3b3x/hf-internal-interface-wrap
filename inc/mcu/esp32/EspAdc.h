/**
 * @file EspAdc.h
 * @brief ESP32 ADC implementation for the HardFOC system.
 *
 * This file contains the ESP32 ADC implementation that extends the BaseAdc
 * abstract class. It provides full support for ESP32 ADC features including:
 * - One-shot mode for single conversions
 * - Continuous mode with DMA for high-speed sampling  
 * - Hardware calibration for accurate voltage measurements
 * - Digital IIR filters for noise reduction
 * - Threshold monitors with interrupt callbacks
 * - Multi-channel support with proper GPIO mapping
 * - Thread-safe operations with proper resource management
 * - Comprehensive error handling and diagnostics
 *
 * @author Nebiyu Tadesse  
 * @date 2025
 * @copyright HardFOC
 *
 * @note This implementation is designed for all ESP32 variants using ESP-IDF v5.4+
 * @note Supports ESP32-C6, ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2
 * @note Each EspAdc instance represents a single ADC unit
 * @note Higher-level applications should instantiate multiple EspAdc objects for multi-unit boards
 */

#pragma once

#include "McuSelect.h"

// Only compile for ESP32 family
#ifdef HF_MCU_FAMILY_ESP32

#include "BaseAdc.h"
#include "RtosMutex.h"
#include "EspTypes_ADC.h"

#include <memory>
#include <vector>
#include <array>
#include <atomic>

// ESP-IDF includes for ADC functionality
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_monitor.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "RtosMutex.h"

//==============================================================================
// ESP32 VARIANT-SPECIFIC ADC CONFIGURATION
//==============================================================================

// ESP32-C6 Configuration
#if defined(HF_MCU_ESP32C6)
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-C6 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 7                 ///< ESP32-C6 has 7 ADC channels (0-6)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-C6 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-C6 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 100000       ///< 100kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32 Classic Configuration
#elif defined(HF_MCU_ESP32)
#define HF_ESP32_ADC_MAX_UNITS 2                    ///< ESP32 has 2 ADC units (ADC1, ADC2)
#define HF_ESP32_ADC_MAX_CHANNELS 8                 ///< ESP32 has 8 ADC channels per unit (0-7)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 200000       ///< 200kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32-S2 Configuration
#elif defined(HF_MCU_ESP32S2)
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-S2 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 10                ///< ESP32-S2 has 10 ADC channels (0-9)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-S2 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-S2 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 200000       ///< 200kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32-S3 Configuration
#elif defined(HF_MCU_ESP32S3)
#define HF_ESP32_ADC_MAX_UNITS 2                    ///< ESP32-S3 has 2 ADC units (ADC1, ADC2)
#define HF_ESP32_ADC_MAX_CHANNELS 10                ///< ESP32-S3 has 10 ADC channels per unit (0-9)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-S3 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-S3 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 200000       ///< 200kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32-C3 Configuration
#elif defined(HF_MCU_ESP32C3)
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-C3 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 6                 ///< ESP32-C3 has 6 ADC channels (0-5)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-C3 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-C3 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 100000       ///< 100kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32-C2 Configuration
#elif defined(HF_MCU_ESP32C2)
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-C2 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 4                 ///< ESP32-C2 has 4 ADC channels (0-3)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-C2 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-C2 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 100000       ///< 100kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// ESP32-H2 Configuration
#elif defined(HF_MCU_ESP32H2)
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-H2 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 6                 ///< ESP32-H2 has 6 ADC channels (0-5)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-H2 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-H2 supports 2 threshold monitors
#define HF_ESP32_ADC_MAX_RAW_VALUE 4095             ///< 12-bit ADC
#define HF_ESP32_ADC_REFERENCE_VOLTAGE_MV 1100      ///< 1.1V reference
#define HF_ESP32_ADC_MAX_SAMPLING_FREQ 100000       ///< 100kSPS max
#define HF_ESP32_ADC_MIN_SAMPLING_FREQ 10           ///< 10SPS min
#define HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ 1000     ///< 1kSPS default
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN 256        ///< Minimum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX 4096       ///< Maximum DMA buffer
#define HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT 1024   ///< Default DMA buffer

#define HF_ESP32_ADC_ONESHOT_CLK_SRC      ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_CONTINUOUS_CLK_SRC   ADC_DIGI_CLK_SRC_DEFAULT ///< Chosen clock source for ADC
#define HF_ESP32_ADC_ULP_MODE             ADC_ULP_MODE_DISABLE      ///< ULP mode disabled by default

// Default fallback (should not be reached)
#else
#error "Unsupported ESP32 variant! Please add support for this ESP32 variant in EspAdc.h"
#endif

/**
 * @class EspAdc
 * @brief ESP32 ADC implementation class.
 * 
 * This class provides a complete implementation of the BaseAdc interface for ESP32 variants.
 * It supports both one-shot and continuous ADC modes with comprehensive feature support.
 * Each instance represents a single ADC unit on the ESP32.
 * 
 * Key Features:
 * - One-shot mode: Single channel conversions with blocking or non-blocking operation
 * - Continuous mode: Multi-channel high-speed sampling with DMA and callbacks
 * - Hardware calibration: Automatic calibration using ESP32 eFuse data
 * - Digital filters: Up to 2 IIR filters for noise reduction  
 * - Threshold monitors: Up to 2 monitors with configurable thresholds and callbacks
 * - Thread safety: Proper mutex protection for multi-threaded access
 * - Error handling: Comprehensive error reporting and recovery
 * - Resource management: Automatic cleanup and proper resource lifecycle
 * - Multi-variant support: Works across all ESP32 variants (C6, Classic, S2, S3, C3, C2, H2)
 * 
 * Usage Example (Single ADC Unit):
 * @code
 * // For ESP32-C6 (single unit)
 * EspAdc adc1({.unit_id = 0}); // ADC1
 * 
 * // For ESP32 Classic (two units)
 * EspAdc adc1({.unit_id = 0}); // ADC1
 * EspAdc adc2({.unit_id = 1}); // ADC2
 * 
 * if (adc1.EnsureInitialized()) {
 *   float voltage;
 *   if (adc1.ReadChannelV(2, voltage) == hf_adc_err_t::ADC_SUCCESS) {
 *     // Use voltage reading
 *   }
 * }
 * @endcode
 * 
 * Usage Example (Continuous mode):
 * @code
 * EspAdc adc({.unit_id = 0});
 * adc.SetMode(hf_adc_mode_t::CONTINUOUS);
 * adc.ConfigureChannel(0, hf_adc_atten_t::ATTEN_DB_12);
 * adc.ConfigureChannel(1, hf_adc_atten_t::ATTEN_DB_12);
 * adc.SetContinuousCallback([](const hf_adc_continuous_data_t* data, void* user_data) {
 *   // Process continuous data
 *   return false; // Return true to yield to higher priority task
 * });
 * adc.StartContinuous();
 * @endcode
 * 
 * @note EspAdc instances cannot be copied or moved due to hardware resource management.
 * @note If you need to transfer ownership, use std::unique_ptr<EspAdc> or similar smart pointers.
 * @note Each EspAdc instance should be created and destroyed in the same thread context.
 */
class EspAdc : public BaseAdc {
public:
  //==============================================//
  // CONSTRUCTION AND INITIALIZATION
  //==============================================//
  
  /**
   * @brief Constructor
   * @param config ADC unit configuration
   */
  explicit EspAdc(const hf_adc_unit_config_t& config) noexcept;
  
  /**
   * @brief Destructor - ensures proper cleanup
   */
  ~EspAdc() noexcept override;
  
  // Disable copy and move operations
  // Copy operations are disabled because EspAdc manages hardware resources
  EspAdc(const EspAdc&) = delete;
  EspAdc& operator=(const EspAdc&) = delete;
  
  // Move operations are disabled because EspAdc manages ESP-IDF handles,
  // mutexes, and callback state that are tightly coupled to hardware
  EspAdc(EspAdc&& other) = delete;
  EspAdc& operator=(EspAdc&& other) = delete;

  //==============================================//
  // HARDWARE LIMITS (VARIANT-SPECIFIC)
  //==============================================//

  // ESP32 ADC Hardware Limits (configured per variant)
  static constexpr uint8_t HF_ADC_MAX_UNITS = HF_ESP32_ADC_MAX_UNITS;                    ///< Maximum ADC units for this ESP32 variant
  static constexpr uint8_t HF_ADC_MAX_CHANNELS = HF_ESP32_ADC_MAX_CHANNELS;              ///< Maximum ADC channels per unit
  static constexpr uint8_t HF_ADC_MAX_FILTERS = HF_ESP32_ADC_MAX_FILTERS;                ///< Maximum IIR filters supported
  static constexpr uint8_t HF_ADC_MAX_MONITORS = HF_ESP32_ADC_MAX_MONITORS;              ///< Maximum threshold monitors supported
  static constexpr uint16_t HF_ADC_MAX_RAW_VALUE_12BIT = HF_ESP32_ADC_MAX_RAW_VALUE;     ///< 12-bit max raw value
  static constexpr uint32_t HF_ADC_REFERENCE_VOLTAGE_MV = HF_ESP32_ADC_REFERENCE_VOLTAGE_MV; ///< Reference voltage in mV
  static constexpr uint32_t HF_ADC_MAX_SAMPLING_FREQ = HF_ESP32_ADC_MAX_SAMPLING_FREQ;    ///< Maximum sampling frequency
  static constexpr uint32_t HF_ADC_MIN_SAMPLING_FREQ = HF_ESP32_ADC_MIN_SAMPLING_FREQ;    ///< Minimum sampling frequency
  static constexpr uint32_t HF_ADC_DEFAULT_SAMPLING_FREQ = HF_ESP32_ADC_DEFAULT_SAMPLING_FREQ; ///< Default sampling frequency
  static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MIN = HF_ESP32_ADC_DMA_BUFFER_SIZE_MIN;  ///< Minimum DMA buffer size
  static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_MAX = HF_ESP32_ADC_DMA_BUFFER_SIZE_MAX;  ///< Maximum DMA buffer size
  static constexpr size_t HF_ADC_DMA_BUFFER_SIZE_DEFAULT = HF_ESP32_ADC_DMA_BUFFER_SIZE_DEFAULT; ///< Default DMA buffer size
  
  static constexpr adc_oneshot_clk_src_t HF_ADC_ONESHOT_CLK_SRC = HF_ESP32_ADC_ONESHOT_CLK_SRC; ///< Clock source for one-shot mode
  static constexpr adc_continuous_clk_src_t HF_ADC_CONTINUOUS_CLK_SRC = HF_ESP32_ADC_CONTINUOUS_CLK_SRC; ///< Clock source for continuous mode
  static constexpr adc_ulp_mode_t HF_ADC_ULP_MODE = HF_ESP32_ADC_ULP_MODE; ///< ULP mode disabled by default

  //==============================================//
  // BASE CLASS IMPLEMENTATION (REQUIRED)
  //==============================================//

  /**
   * @brief Initialize the ESP32 ADC peripheral
   * @return true if initialization successful, false otherwise
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the ESP32 ADC peripheral  
   * @return true if deinitialization successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Get maximum number of ADC channels for this ESP32 variant
   * @return Maximum channel count
   */
  [[nodiscard]] uint8_t GetMaxChannels() const noexcept override;

  /**
   * @brief Check if specific channel is available on this ESP32 variant
   * @param channel_id Channel ID to check
   * @return true if channel available, false otherwise
   */
  [[nodiscard]] bool IsChannelAvailable(hf_channel_id_t channel_id) const noexcept override;

  /**
   * @brief Read channel voltage with optional averaging
   * @param channel_id Channel ID
   * @param channel_reading_v Reference to store voltage in volts
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in ms (default 0)
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, float& channel_reading_v,
                            uint8_t numOfSamplesToAvg = 1, 
                            hf_time_t timeBetweenSamples = 0) noexcept override;

  /**
   * @brief Read channel raw count with optional averaging
   * @param channel_id Channel ID
   * @param channel_reading_count Reference to store raw count
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in ms (default 0)
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, uint32_t& channel_reading_count,
                                uint8_t numOfSamplesToAvg = 1,
                                hf_time_t timeBetweenSamples = 0) noexcept override;

  /**
   * @brief Read both channel count and voltage with optional averaging
   * @param channel_id Channel ID
   * @param channel_reading_count Reference to store raw count  
   * @param channel_reading_v Reference to store voltage in volts
   * @param numOfSamplesToAvg Number of samples to average (default 1)
   * @param timeBetweenSamples Time between samples in ms (default 0)
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ReadChannel(hf_channel_id_t channel_id, uint32_t& channel_reading_count,
                           float& channel_reading_v, uint8_t numOfSamplesToAvg = 1,
                           hf_time_t timeBetweenSamples = 0) noexcept override;

  /**
   * @brief Read multiple channels simultaneously
   * @param channel_ids Array of channel IDs
   * @param num_channels Number of channels
   * @param readings Array to store raw readings
   * @param voltages Array to store voltage readings
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ReadMultipleChannels(const hf_channel_id_t* channel_ids, uint8_t num_channels,
                                    uint32_t* readings, float* voltages) noexcept override;

  //==============================================//
  // MODE AND CONFIGURATION OPERATIONS
  //==============================================//

  /**
   * @brief Set ADC operation mode (one-shot or continuous)
   * @param mode Operation mode to set
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetMode(hf_adc_mode_t mode) noexcept;

  /**
   * @brief Get current ADC operation mode
   * @return Current operation mode
   */
  [[nodiscard]] hf_adc_mode_t GetMode() const noexcept;

  /**
   * @brief Configure ADC channel
   * @param channel_id Channel ID to configure
   * @param attenuation Attenuation level
   * @param bitwidth Bit width (optional, uses default if not specified)
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ConfigureChannel(hf_channel_id_t channel_id, hf_adc_atten_t attenuation,
                                hf_adc_bitwidth_t bitwidth = hf_adc_bitwidth_t::WIDTH_DEFAULT) noexcept;

  /**
   * @brief Enable or disable ADC channel
   * @param channel_id Channel ID
   * @param enabled Enable/disable flag
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetChannelEnabled(hf_channel_id_t channel_id, bool enabled) noexcept;

  /**
   * @brief Check if channel is enabled
   * @param channel_id Channel ID to check
   * @return true if enabled, false otherwise
   */
  [[nodiscard]] bool IsChannelEnabled(hf_channel_id_t channel_id) const noexcept;

  //==============================================//
  // CONTINUOUS MODE OPERATIONS
  //==============================================//

  /**
   * @brief Configure continuous mode parameters
   * @param config Continuous mode configuration
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ConfigureContinuous(const hf_adc_continuous_config_t& config) noexcept;

  /**
   * @brief Set continuous mode data callback
   * @param callback Callback function for continuous data
   * @param user_data User data passed to callback
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetContinuousCallback(hf_adc_continuous_callback_t callback, void* user_data = nullptr) noexcept;

  /**
   * @brief Start continuous mode sampling
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t StartContinuous() noexcept;

  /**
   * @brief Stop continuous mode sampling
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t StopContinuous() noexcept;

  /**
   * @brief Check if continuous mode is running
   * @return true if running, false otherwise
   */
  [[nodiscard]] bool IsContinuousRunning() const noexcept;

  /**
   * @brief Read continuous mode data (blocking)
   * @param buffer Buffer to store data
   * @param buffer_size Buffer size in bytes
   * @param bytes_read Actual bytes read
   * @param timeout_ms Timeout in milliseconds
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ReadContinuousData(uint8_t* buffer, uint32_t buffer_size, 
                                  uint32_t& bytes_read, hf_time_t timeout_ms) noexcept;

  //==============================================//
  // CALIBRATION OPERATIONS
  //==============================================//

  /**
   * @brief Initialize calibration for a specific attenuation
   * @param attenuation Attenuation level to calibrate
   * @param bitwidth Bit width for calibration
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t InitializeCalibration(hf_adc_atten_t attenuation, 
                                     hf_adc_bitwidth_t bitwidth = hf_adc_bitwidth_t::WIDTH_DEFAULT) noexcept;

  /**
   * @brief Check if calibration is available for attenuation
   * @param attenuation Attenuation level to check
   * @return true if calibration available, false otherwise
   */
  [[nodiscard]] bool IsCalibrationAvailable(hf_adc_atten_t attenuation) const noexcept;

  /**
   * @brief Convert raw count to voltage using calibration
   * @param raw_count Raw ADC count
   * @param attenuation Attenuation used for conversion
   * @param voltage_mv Output voltage in millivolts
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t RawToVoltage(uint32_t raw_count, hf_adc_atten_t attenuation, uint32_t& voltage_mv) noexcept;

  //==============================================//
  // FILTER OPERATIONS
  //==============================================//

  /**
   * @brief Configure digital IIR filter
   * @param filter_config Filter configuration
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ConfigureFilter(const hf_adc_filter_config_t& filter_config) noexcept;

  /**
   * @brief Enable/disable IIR filter
   * @param filter_id Filter ID (0-1)
   * @param enabled Enable/disable flag
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetFilterEnabled(uint8_t filter_id, bool enabled) noexcept;

  //==============================================//
  // MONITOR OPERATIONS
  //==============================================//

  /**
   * @brief Configure threshold monitor
   * @param monitor_config Monitor configuration
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t ConfigureMonitor(const hf_adc_monitor_config_t& monitor_config) noexcept;

  /**
   * @brief Set monitor threshold callback
   * @param monitor_id Monitor ID (0-1)
   * @param callback Callback function for threshold events
   * @param user_data User data passed to callback
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetMonitorCallback(uint8_t monitor_id, hf_adc_monitor_callback_t callback, 
                                  void* user_data = nullptr) noexcept;

  /**
   * @brief Enable/disable threshold monitor
   * @param monitor_id Monitor ID (0-1)
   * @param enabled Enable/disable flag
   * @return hf_adc_err_t error code
   */
  hf_adc_err_t SetMonitorEnabled(uint8_t monitor_id, bool enabled) noexcept;

  //==============================================//
  // DIAGNOSTICS AND STATISTICS
  //==============================================//

  /**
   * @brief Get ADC operation statistics
   * @return Statistics structure
   */
  [[nodiscard]] hf_adc_statistics_t GetStatistics() const noexcept override;

  /**
   * @brief Get ADC diagnostic information
   * @return Diagnostics structure
   */
  [[nodiscard]] hf_adc_diagnostics_t GetDiagnostics() const noexcept override;

  /**
   * @brief Reset statistics counters
   */
  void ResetStatistics() noexcept;

  /**
   * @brief Get last error information
   * @return Last error code
   */
  [[nodiscard]] hf_adc_err_t GetLastError() const noexcept;

  /**
   * @brief Get unit configuration
   * @return Reference to unit configuration
   */
  [[nodiscard]] const hf_adc_unit_config_t& GetUnitConfig() const noexcept;

private:
  //==============================================//
  // PRIVATE IMPLEMENTATION
  //==============================================//

  // Internal helper methods
  hf_adc_err_t InitializeOneshot() noexcept;
  hf_adc_err_t InitializeContinuous() noexcept;
  hf_adc_err_t DeinitializeOneshot() noexcept;
  hf_adc_err_t DeinitializeContinuous() noexcept;
  hf_adc_err_t ReadOneshotRaw(hf_channel_id_t channel_id, uint32_t& raw_value) noexcept;
  hf_adc_err_t ValidateChannelId(hf_channel_id_t channel_id) const noexcept;
  hf_adc_err_t ValidateConfiguration() const noexcept;
  hf_adc_err_t UpdateStatistics(hf_adc_err_t result, uint64_t start_time_us) noexcept;
  uint64_t GetCurrentTimeUs() const noexcept;
  void UpdateDiagnostics(hf_adc_err_t error) noexcept;

  // Static callback functions for ESP-IDF
  static bool IRAM_ATTR ContinuousCallback(adc_continuous_handle_t handle, const void* edata, void* user_data) noexcept;
  static bool IRAM_ATTR MonitorCallback(adc_monitor_handle_t monitor_handle, const void* event_data, void* user_data) noexcept;

  //==============================================//
  // MEMBER VARIABLES
  //==============================================//

  // Configuration and state
  hf_adc_unit_config_t config_;               ///< ADC unit configuration
  std::atomic<bool> continuous_running_;      ///< Continuous mode running flag
  std::atomic<hf_adc_err_t> last_error_;      ///< Last error code
  
  // Thread safety
  mutable RtosMutex config_mutex_;            ///< Configuration mutex
  mutable RtosMutex stats_mutex_;             ///< Statistics mutex

  // ESP-IDF handles
  adc_oneshot_unit_handle_t oneshot_handle_;  ///< Oneshot mode handle
  adc_continuous_handle_t continuous_handle_; ///< Continuous mode handle
  std::array<adc_cali_handle_t, 4> calibration_handles_; ///< Calibration handles (one per attenuation)
  std::array<adc_iir_filter_handle_t, HF_ADC_MAX_FILTERS> filter_handles_; ///< Filter handles
  std::array<adc_monitor_handle_t, HF_ADC_MAX_MONITORS> monitor_handles_;   ///< Monitor handles

  // Callback data
  hf_adc_continuous_callback_t continuous_callback_; ///< Continuous callback function
  void* continuous_user_data_;                       ///< Continuous callback user data
  std::array<hf_adc_monitor_callback_t, HF_ADC_MAX_MONITORS> monitor_callbacks_; ///< Monitor callbacks
  std::array<void*, HF_ADC_MAX_MONITORS> monitor_user_data_; ///< Monitor callback user data

  // Statistics and diagnostics
  mutable hf_adc_statistics_t statistics_;    ///< Operation statistics
  mutable hf_adc_diagnostics_t diagnostics_;  ///< Diagnostic information
};

#endif // HF_MCU_FAMILY_ESP32
