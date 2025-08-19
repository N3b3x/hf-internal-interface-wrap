/**
 * @file EspPwm.cpp
 * @brief Implementation of ESP32 family LEDC (PWM) controller for the HardFOC system.
 *
 * This file provides the complete implementation for PWM generation using the ESP32 family's
 * built-in LEDC (LED Controller) peripheral. The implementation is designed to work across
 * all ESP32 variants with automatic adaptation to variant-specific capabilities and constraints.
 *
 * ## Key Implementation Features:
 * - **Variant-Agnostic Design:** Automatic adaptation to ESP32 variant capabilities
 * - **LEDC Peripheral Integration:** Full utilization of hardware fade, timer sharing, and interrupts
 * - **Smart Resource Management:** Automatic timer allocation with conflict resolution and eviction policies
 * - **Thread-Safe Operations:** Complete RtosMutex protection for concurrent access
 * - **Comprehensive Validation:** Hardware constraint validation with detailed error reporting
 * - **Performance Optimization:** Efficient timer sharing and minimal overhead design
 * - **Motor Control Features:** Complementary outputs, deadtime, and synchronized operations
 *
 * ## LEDC Hardware Abstraction:
 * All platform-specific types and ESP-IDF dependencies are isolated through EspTypes_PWM.h,
 * providing a clean abstraction layer that can be easily ported or tested.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * 
 * @see EspPwm.h for comprehensive API documentation
 * @see EspTypes_PWM.h for type definitions and LEDC peripheral details
 */
#include "EspPwm.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>
#include <vector>

// Platform-specific includes and definitions
#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hal/ledc_hal.h"
#include "soc/ledc_reg.h"

#ifdef __cplusplus
}
#endif
#endif

static const char* TAG = "EspPwm";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspPwm::EspPwm(const hf_pwm_unit_config_t& config) noexcept
    : BasePwm(), mutex_(), initialized_(false), base_clock_hz_(config.base_clock_hz),
      clock_source_(config.clock_source), channels_(), timers_(), complementary_pairs_(),
      period_callback_(nullptr), period_callback_user_data_(nullptr), fault_callback_(nullptr),
      fault_callback_user_data_(nullptr), last_global_error_(hf_pwm_err_t::PWM_SUCCESS),
      fade_functionality_installed_(false), unit_config_(config), current_mode_(config.mode),
      statistics_(), diagnostics_(), auto_fallback_enabled_(false),
      eviction_policy_(hf_pwm_eviction_policy_t::STRICT_NO_EVICTION), eviction_callback_(nullptr),
      eviction_callback_user_data_(nullptr) {
  ESP_LOGD(TAG, "EspPwm constructed with unit_id=%d, mode=%d, clock_hz=%lu", config.unit_id,
           static_cast<int>(config.mode), config.base_clock_hz);
}

EspPwm::EspPwm(hf_u32_t base_clock_hz) noexcept : EspPwm() {
  // Override base clock if provided
  if (base_clock_hz != 0) {
    base_clock_hz_ = base_clock_hz;
    unit_config_.base_clock_hz = base_clock_hz;
  }
  ESP_LOGD(TAG, "EspPwm legacy constructor with clock_hz=%lu", base_clock_hz);
}

EspPwm::~EspPwm() noexcept {
  if (initialized_.load()) {
    ESP_LOGI(TAG, "EspPwm destructor - cleaning up");
    Deinitialize();
  }
}

//==============================================================================
// LIFECYCLE (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_.load()) {
    ESP_LOGW(TAG, "PWM already initialized");
    return hf_pwm_err_t::PWM_ERR_ALREADY_INITIALIZED;
  }

  ESP_LOGI(TAG, "Initializing ESP32C6 PWM system with unit_id=%d, mode=%d, base_clock=%lu Hz",
           unit_config_.unit_id, static_cast<int>(unit_config_.mode), base_clock_hz_);

  // Initialize timers and channels using lifecycle helpers
  hf_pwm_err_t result = InitializeTimers();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize timers: %d", static_cast<int>(result));
    return result;
  }

  result = InitializeChannels();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize channels: %d", static_cast<int>(result));
    return result;
  }

  // Enable fade functionality if requested (guard repeated installs)
  if (unit_config_.enable_fade && !fade_functionality_installed_) {
    result = EnableFade();
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGW(TAG, "Failed to enable fade functionality: %d", static_cast<int>(result));
      // Don't fail initialization if fade fails
    }
  }

  initialized_.store(true);
  BasePwm::initialized_ = true;
  last_global_error_ = hf_pwm_err_t::PWM_SUCCESS;
  statistics_.initialization_timestamp = esp_timer_get_time();
  statistics_.last_activity_timestamp = statistics_.initialization_timestamp;

  ESP_LOGI(TAG, "ESP32C6 PWM system initialized successfully");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::Deinitialize() noexcept {
  if (!initialized_.load()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  ESP_LOGI(TAG, "Deinitializing ESP32C6 PWM system");

  // 1. Remove fade functionality FIRST (before stopping channels)
  // This prevents fade-out behavior during ledc_stop calls
  if (fade_functionality_installed_) {
    ledc_fade_func_uninstall();
    ESP_LOGD(TAG, "LEDC fade functionality uninstalled");
    fade_functionality_installed_ = false;
  }

  // 2. Stop all channels with proper GPIO cleanup (now without fade functionality)
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured) {
      // Stop the LEDC channel
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
      
      // Reset GPIO pin to default state
      hf_gpio_num_t gpio_pin = channels_[channel_id].config.gpio_pin;
      if (HF_GPIO_IS_VALID_GPIO(gpio_pin)) {
        // Disable GPIO hold and reset to default state
        gpio_hold_dis(static_cast<gpio_num_t>(gpio_pin));
        gpio_reset_pin(static_cast<gpio_num_t>(gpio_pin));
        ESP_LOGD(TAG, "GPIO %d reset to default state", gpio_pin);
      }
      
      // Clear channel state
      channels_[channel_id] = ChannelState{};
    }
  }

  // 3. Reset all timers with proper hardware cleanup
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      esp_err_t ret = ledc_timer_rst(LEDC_LOW_SPEED_MODE, static_cast<ledc_timer_t>(timer_id));
      if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to reset timer %d during deinitialization: %s", 
                 timer_id, esp_err_to_name(ret));
      }
      
      // Clear timer state
      timers_[timer_id] = TimerState{};
    }
  }

  // 4. Clear complementary pair configurations
  for (auto& pair : complementary_pairs_) {
    pair = ComplementaryPair{};
  }

  // 5. Clear callbacks
  period_callback_ = nullptr;
  period_callback_user_data_ = nullptr;
  fault_callback_ = nullptr;
  fault_callback_user_data_ = nullptr;

  // 6. Update state and statistics
  initialized_.store(false);
  BasePwm::initialized_ = false;
  last_global_error_ = hf_pwm_err_t::PWM_SUCCESS;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  ESP_LOGI(TAG, "ESP32C6 PWM system deinitialized with comprehensive cleanup");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetMode(hf_pwm_mode_t mode) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (mode == hf_pwm_mode_t::HF_PWM_MODE_FADE && !fade_functionality_installed_) {
    hf_pwm_err_t result = EnableFade();
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      return result;
    }
  }

  current_mode_ = mode;
  unit_config_.mode = mode;

  ESP_LOGD(TAG, "PWM mode set to %d", static_cast<int>(mode));
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_mode_t EspPwm::GetMode() const noexcept {
  return current_mode_;
}

//==============================================================================
// CHANNEL MANAGEMENT (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::ConfigureChannel(hf_channel_id_t channel_id,
                                      const hf_pwm_channel_config_t& config) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate configuration
  if (config.gpio_pin == static_cast<hf_gpio_num_t>(HF_INVALID_PIN)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // Use frequency and resolution from channel config
  hf_u32_t frequency_hz = config.frequency_hz;
  hf_u8_t resolution_bits = config.resolution_bits;

  // Validate resolution range
  if (resolution_bits < 4 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    ESP_LOGE(TAG, "Invalid resolution: %d bits (valid range: 4-%d)", resolution_bits, HF_PWM_MAX_RESOLUTION);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // Validate initial duty against actual resolution
  const hf_u32_t max_raw = (1u << resolution_bits) - 1u;
  if (config.duty_initial > max_raw) {
    ESP_LOGE(TAG, "Initial duty %lu exceeds maximum %lu for %d-bit resolution", 
             config.duty_initial, max_raw, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Validate frequency/resolution combination using new unified system
  ValidationContext ctx(frequency_hz, resolution_bits, config.clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
  if (!validation.is_valid) {
    ESP_LOGE(TAG, "Invalid frequency/resolution combination: %lu Hz @ %d bits - %s", frequency_hz, resolution_bits, validation.reason);
    SetChannelError(channel_id, validation.error_code);
    
    // Trigger fault callback for configuration error
    HandleChannelFault(channel_id, validation.error_code);
    
    return validation.error_code;
  }

  // Handle timer assignment changes properly
  hf_u8_t old_timer = channels_[channel_id].configured ? channels_[channel_id].assigned_timer : 0xFF;
  bool is_reconfiguration = channels_[channel_id].configured;

  hf_i8_t timer_id = FindOrAllocateTimer(frequency_hz, resolution_bits, config.clock_source);
  if (timer_id < 0) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    
    // Trigger fault callback for timer allocation failure
    HandleChannelFault(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the platform timer with channel's preferred clock source
  hf_pwm_err_t timer_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits, config.clock_source);
  if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    
    // Trigger fault callback for timer configuration failure
    HandleChannelFault(channel_id, timer_result);
    
    return timer_result;
  }

  // Configure the platform channel
  hf_pwm_err_t channel_result = ConfigurePlatformChannel(channel_id, config, timer_id);
  if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, channel_result);
    
    // Trigger fault callback for channel configuration failure
    HandleChannelFault(channel_id, channel_result);
    
    return channel_result;
  }

  // Properly manage timer channel counts during reconfiguration
  if (is_reconfiguration && old_timer != timer_id && old_timer < MAX_TIMERS) {
    // Moving to a different timer - decrement old timer count
    if (timers_[old_timer].channel_count > 0) {
      timers_[old_timer].channel_count--;
      ESP_LOGD(TAG, "Decremented old timer %d channel count to %d", old_timer, timers_[old_timer].channel_count);
    }
    ReleaseTimerIfUnused(old_timer);
  }

  // Update internal state with proper resolution handling
  channels_[channel_id].configured = true;
  channels_[channel_id].config = config;
  channels_[channel_id].assigned_timer = timer_id;
  
  // Store the validated raw duty value (already checked against actual resolution)
  channels_[channel_id].raw_duty_value = config.duty_initial;
  channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;
  
  // NEW: Store channel protection settings
  channels_[channel_id].priority = config.priority;
  channels_[channel_id].is_critical = config.is_critical;
  channels_[channel_id].description = config.description;

  ESP_LOGI(TAG, "Channel %lu configured: pin=%d, freq=%lu Hz, res=%d bits, timer=%d", channel_id,
           config.gpio_pin, frequency_hz, resolution_bits, timer_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::EnableChannel(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE);
    return hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE;
  }

  if (channels_[channel_id].enabled) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already enabled
  }

  esp_err_t ret;
  
  // Use conditional LEDC functions based on mode and fade settings
  if (current_mode_ == hf_pwm_mode_t::HF_PWM_MODE_FADE || unit_config_.enable_fade) {
    // Fade mode or fade enabled - use fade-compatible function
    if (!fade_functionality_installed_) {
      // Install fade service if not already installed
      hf_pwm_err_t fade_result = InitializeFadeFunctionality();
      if (fade_result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize fade functionality for channel %lu", channel_id);
        SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
    
    // Use ledc_set_duty_and_update which requires fade service
    ret = ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                                   channels_[channel_id].raw_duty_value,
                                   0 // No hpoint (phase shift)
        );
  } else {
    // Basic mode without fade - use separate duty set and update
    // First set the duty cycle
    ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                         channels_[channel_id].raw_duty_value);
    if (ret == ESP_OK) {
      // Then update the duty cycle
      ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
    }
  }

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = true;
  
  // Update statistics for channel enable
  statistics_.channel_enables_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  ESP_LOGI(TAG, "Channel %lu enabled (mode=%d, fade=%s)", channel_id, 
           static_cast<int>(current_mode_),
           (current_mode_ == hf_pwm_mode_t::HF_PWM_MODE_FADE || unit_config_.enable_fade) ? "enabled" : "disabled");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::DisableChannel(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].enabled) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already disabled
  }

  // Stop the channel based on idle state
  hf_u32_t idle_level = 0; // Default idle state
  if (channels_[channel_id].config.invert_output) {
    idle_level = 1 - idle_level;
  }

  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = false;
  
  // Decrement timer channel count when disabling channel
  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id < MAX_TIMERS && timers_[timer_id].channel_count > 0) {
    timers_[timer_id].channel_count--;
    ESP_LOGD(TAG, "Decremented timer %d channel count to %d", timer_id, timers_[timer_id].channel_count);
    
    // Release timer if no longer used
    ReleaseTimerIfUnused(timer_id);
  }
  
  // Update statistics for channel disable
  statistics_.channel_disables_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  ESP_LOGI(TAG, "Channel %lu disabled", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsChannelEnabled(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }
  return channels_[channel_id].enabled;
}

//==============================================================================
// PWM CONTROL (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Use enhanced duty cycle clamping for safety
  duty_cycle = BasePwm::ClampDutyCycle(duty_cycle);

  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id >= MAX_TIMERS) {
    ESP_LOGE(TAG, "Invalid timer id: %d", timer_id);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }
  
  // Safety check: ensure timer resolution is valid
  hf_u8_t resolution = timers_[timer_id].resolution_bits;
  if (resolution == 0) {
    ESP_LOGW(TAG, "Timer %d resolution not set in SetDutyCycle! Using default %d bits", 
             timer_id, HF_PWM_DEFAULT_RESOLUTION);
    resolution = HF_PWM_DEFAULT_RESOLUTION;
    // Try to recover by setting the resolution
    timers_[timer_id].resolution_bits = resolution;
  }
  
  uint32_t raw_duty = BasePwm::DutyCycleToRaw(duty_cycle, resolution);
  
  // Ensure raw duty doesn't exceed maximum for this resolution
  hf_u32_t max_duty = (1U << resolution) - 1;
  if (raw_duty > max_duty) {
    ESP_LOGW(TAG, "Calculated raw duty %lu exceeds maximum %lu for %d-bit resolution, clamping", 
             raw_duty, max_duty, resolution);
    raw_duty = max_duty;
  }
  
  ESP_LOGD(TAG, "Setting duty cycle for channel %lu to %.2f%% (raw: %lu)", 
           channel_id, duty_cycle * 100.0f, raw_duty);
  return SetDutyCycleRaw(channel_id, raw_duty);
}

hf_pwm_err_t EspPwm::SetDutyCycleRaw(hf_channel_id_t channel_id, hf_u32_t raw_value) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE);
    return hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE;
  }

  // Validate raw value against timer resolution
  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  
  // Safety check: ensure timer resolution is valid
  hf_u8_t resolution = timers_[timer_id].resolution_bits;
  if (resolution == 0) {
    ESP_LOGW(TAG, "Timer %d resolution not set in SetDutyCycleRaw! Using default %d bits", 
             timer_id, HF_PWM_DEFAULT_RESOLUTION);
    resolution = HF_PWM_DEFAULT_RESOLUTION;
    // Try to recover by setting the resolution
    timers_[timer_id].resolution_bits = resolution;
  }
  
  // Use enhanced duty cycle validation with overflow protection
  if (!ValidateDutyCycleRange(raw_value, resolution)) {
    hf_u32_t max_duty = (1U << resolution) - 1;
    ESP_LOGW(TAG, "Raw duty value %lu exceeds maximum %lu for %d-bit resolution, clamping", 
             raw_value, max_duty, resolution);
    raw_value = std::min(raw_value, max_duty);
  }

  hf_u32_t actual_duty = raw_value;
  hf_pwm_err_t result = UpdatePlatformDuty(channel_id, actual_duty);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    channels_[channel_id].raw_duty_value = actual_duty;
    channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;
    
    // Update statistics for successful duty cycle changes
    statistics_.duty_updates_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();

    // Trigger period callback if registered (software-based period detection)
    // This simulates a period completion event on duty cycle updates
    if (channels_[channel_id].period_callback) {
      HandlePeriodComplete(channel_id);
    }
  } else {
    SetChannelError(channel_id, result);
    
    // Trigger fault callback for duty cycle update failure
    HandleChannelFault(channel_id, result);
  }

  return result;
}

hf_pwm_err_t EspPwm::SetFrequency(hf_channel_id_t channel_id,
                                  hf_frequency_hz_t frequency_hz) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidFrequency(frequency_hz, MIN_FREQUENCY, MAX_FREQUENCY)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY);
    return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
  }

  // Check if we need to change the timer assignment
  hf_u8_t current_timer = channels_[channel_id].assigned_timer;
  hf_u32_t current_frequency = timers_[current_timer].frequency_hz;
  hf_u8_t current_resolution = timers_[current_timer].resolution_bits;

  // Validate new frequency with current resolution using new unified system
  // By default, fail validation for problematic combinations (strict mode)
  // User must explicitly enable auto-fallback or use SetFrequencyWithResolution() for alternatives
  ValidationContext ctx(frequency_hz, current_resolution, channels_[channel_id].config.clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
  if (!validation.is_valid) {
    ESP_LOGW(TAG, "Requested frequency %lu Hz @ %d bits failed validation", frequency_hz, current_resolution);
    
    // Only try alternatives if auto-fallback is explicitly enabled
    if (auto_fallback_enabled_) {
      ESP_LOGW(TAG, "Auto-fallback is enabled, checking alternative resolutions...");
      
      // Check if alternative resolutions are available using dynamic calculation
      hf_u8_t alternative_resolution = FindBestAlternativeResolutionDynamic(frequency_hz, current_resolution, clock_source_);
      if (alternative_resolution != current_resolution) {
        ValidationContext alt_ctx(frequency_hz, alternative_resolution, channels_[channel_id].config.clock_source);
        ValidationResult alt_validation = ValidateFrequencyResolutionComplete(alt_ctx);
        if (alt_validation.is_valid) {
        
        ESP_LOGW(TAG, "Auto-fallback: using alternative resolution %d bits (instead of %d bits) for frequency %lu Hz", 
                 alternative_resolution, current_resolution, frequency_hz);
        
        // Use the alternative resolution
        return SetFrequencyWithResolution(channel_id, frequency_hz, alternative_resolution);
        }
      }
      
      ESP_LOGE(TAG, "Auto-fallback enabled but no valid alternative resolution found for frequency %lu Hz", frequency_hz);
    } else {
      ESP_LOGW(TAG, "Auto-fallback disabled. To use alternative resolutions, call SetFrequencyWithResolution() or EnableAutoFallback()");
    }
    
    // Fail validation - user must explicitly choose alternative approach
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH);
    return hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
  }

  // If frequency is significantly different, we need a new timer
  // Use a more practical tolerance to avoid unnecessary timer allocations
  const float frequency_tolerance = 0.20f; // 20% tolerance (more practical for PWM applications)
  bool frequency_changed = (std::abs(static_cast<float>(frequency_hz - current_frequency) /
                                     static_cast<float>(current_frequency)) > frequency_tolerance);

  if (frequency_changed) {
    ESP_LOGD(TAG, "Frequency change detected: %lu Hz -> %lu Hz (tolerance: %.1f%%)", 
             current_frequency, frequency_hz, frequency_tolerance * 100.0f);
    
    // SMART TIMER MANAGEMENT: Try multiple strategies in order of efficiency
    
    // Strategy 1: Reconfigure current timer if this channel is the only user
    if (timers_[current_timer].channel_count == 1) {
      ESP_LOGD(TAG, "Reconfiguring timer %d from %lu Hz to %lu Hz (single user)", 
               current_timer, current_frequency, frequency_hz);
      
      hf_pwm_err_t result = ConfigurePlatformTimer(current_timer, frequency_hz, current_resolution, channels_[channel_id].config.clock_source);
      if (result == hf_pwm_err_t::PWM_SUCCESS) {
        // Update timer state
        timers_[current_timer].frequency_hz = frequency_hz;
        ESP_LOGD(TAG, "Timer %d reconfigured successfully to %lu Hz", current_timer, frequency_hz);
        return hf_pwm_err_t::PWM_SUCCESS;
      } else {
        ESP_LOGW(TAG, "Failed to reconfigure timer %d, trying other strategies", current_timer);
      }
    }
    
    // Strategy 2: Find existing timer with same frequency that has room
    for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
      if (timers_[timer_id].in_use && 
          timers_[timer_id].frequency_hz == frequency_hz &&
          timers_[timer_id].channel_count < 8) { // ESP32-C6 supports up to 8 channels per timer
        
        ESP_LOGD(TAG, "Moving channel %d to timer %d (freq=%lu Hz, channels=%d)", 
                 channel_id, timer_id, frequency_hz, timers_[timer_id].channel_count);
        
        // Release from current timer
        timers_[current_timer].channel_count--;
        
        // Assign to new timer
        channels_[channel_id].assigned_timer = timer_id;
        timers_[timer_id].channel_count++;
        
        // Reconfigure channel with new timer
        hf_pwm_err_t result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
        if (result == hf_pwm_err_t::PWM_SUCCESS) {
          ESP_LOGD(TAG, "Channel %d moved to timer %d successfully", channel_id, timer_id);
          return hf_pwm_err_t::PWM_SUCCESS;
        } else {
          ESP_LOGW(TAG, "Failed to move channel %d to timer %d", channel_id, timer_id);
          // Rollback changes
          timers_[timer_id].channel_count--;
          channels_[channel_id].assigned_timer = current_timer;
          timers_[current_timer].channel_count++;
        }
      }
    }
    
    // Strategy 3: Use smart timer allocation (last resort)
    ESP_LOGD(TAG, "No compatible timers found, using smart timer allocation for frequency %lu Hz", frequency_hz);
    
    hf_i8_t new_timer = FindOrAllocateTimer(frequency_hz, current_resolution, channels_[channel_id].config.clock_source);
    if (new_timer < 0) {
      ESP_LOGE(TAG, "Smart timer allocation failed: no available timers for frequency %lu Hz", frequency_hz);
      SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
      return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
    }
    
    ESP_LOGD(TAG, "Smart timer allocation successful: assigned timer %d for frequency %lu Hz", new_timer, frequency_hz);
    
    // Release from current timer
    timers_[current_timer].channel_count--;
    
    // Assign to new timer
    channels_[channel_id].assigned_timer = new_timer;
    timers_[new_timer].channel_count++;
    
    // Reconfigure channel with new timer
    hf_pwm_err_t result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, new_timer);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGD(TAG, "Channel %d reconfigured with new timer %d successfully", channel_id, new_timer);
      return hf_pwm_err_t::PWM_SUCCESS;
    } else {
      ESP_LOGW(TAG, "Failed to reconfigure channel %d with new timer %d", channel_id, new_timer);
      // Rollback changes
      timers_[new_timer].channel_count--;
      channels_[channel_id].assigned_timer = current_timer;
      timers_[current_timer].channel_count++;
      SetChannelError(channel_id, result);
      return result;
    }
  }

  // Reconfigure the channel with the new timer
  hf_pwm_err_t result =
      ConfigurePlatformChannel(channel_id, channels_[channel_id].config, current_timer);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    statistics_.frequency_changes_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();
  } else {
    statistics_.error_count++;
    SetChannelError(channel_id, result);
  }
  return result;
}

hf_pwm_err_t EspPwm::SetPhaseShift(hf_channel_id_t channel_id, float phase_shift_degrees) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // ESP32C6 LEDC doesn't support phase shifting directly
  // This would require advanced timer configuration
  ESP_LOGW(TAG, "Phase shift not supported on ESP32C6 LEDC peripheral");
  SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
  return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
}

//==============================================================================
// NEW USER-CONTROLLED FREQUENCY/RESOLUTION METHODS
//==============================================================================

hf_pwm_err_t EspPwm::SetFrequencyWithResolution(hf_channel_id_t channel_id, 
                                                hf_frequency_hz_t frequency_hz,
                                                hf_u8_t resolution_bits) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate the specific frequency/resolution combination requested using new unified system
  ValidationContext ctx(frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
  if (!validation.is_valid) {
    ESP_LOGE(TAG, "Requested frequency %lu Hz @ %d bits is not achievable - %s", frequency_hz, resolution_bits, validation.reason);
    SetChannelError(channel_id, validation.error_code);
    return validation.error_code;
  }

  // User explicitly chose this resolution, so proceed with it
  ESP_LOGI(TAG, "User requested frequency %lu Hz @ %d bits (explicit resolution choice)", 
           frequency_hz, resolution_bits);

  // Find or allocate timer for this specific frequency/resolution combination with channel's clock source
  hf_i8_t timer_id = FindOrAllocateTimer(frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  if (timer_id < 0) {
    ESP_LOGE(TAG, "No available timer for frequency %lu Hz @ %d bits", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the timer with the user-specified resolution
  hf_pwm_err_t timer_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    return timer_result;
  }

  // Update channel assignment and timer state
  hf_u8_t old_timer = channels_[channel_id].assigned_timer;
  channels_[channel_id].assigned_timer = timer_id;
  timers_[timer_id].frequency_hz = frequency_hz;
  timers_[timer_id].resolution_bits = resolution_bits;

  // Release old timer if no longer used
  ReleaseTimerIfUnused(old_timer);

  // Reconfigure the channel with the new timer
  hf_pwm_err_t result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    statistics_.frequency_changes_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();
    ESP_LOGI(TAG, "Frequency set to %lu Hz @ %d bits successfully", frequency_hz, resolution_bits);
  } else {
    statistics_.error_count++;
    SetChannelError(channel_id, result);
  }

  return result;
}

hf_pwm_err_t EspPwm::SetFrequencyWithAutoFallback(hf_channel_id_t channel_id, 
                                                   hf_frequency_hz_t frequency_hz,
                                                   hf_u8_t preferred_resolution) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Try the preferred resolution first using new unified system
  ValidationContext ctx(frequency_hz, preferred_resolution, channels_[channel_id].config.clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
  if (validation.is_valid) {
    ESP_LOGI(TAG, "Using preferred resolution %d bits for frequency %lu Hz", preferred_resolution, frequency_hz);
    return SetFrequencyWithResolution(channel_id, frequency_hz, preferred_resolution);
  }

  // Preferred resolution failed, try to find alternative
  ESP_LOGW(TAG, "Preferred resolution %d bits failed for frequency %lu Hz, trying alternatives", 
           preferred_resolution, frequency_hz);

  hf_u8_t alternative_resolution = FindBestAlternativeResolutionDynamic(frequency_hz, preferred_resolution, clock_source_);
  if (alternative_resolution != preferred_resolution) {
    ValidationContext alt_ctx(frequency_hz, alternative_resolution, channels_[channel_id].config.clock_source);
    ValidationResult alt_validation = ValidateFrequencyResolutionComplete(alt_ctx);
    if (alt_validation.is_valid) {
    ESP_LOGW(TAG, "Auto-fallback: Using alternative resolution %d bits (instead of %d bits) for frequency %lu Hz", 
             alternative_resolution, preferred_resolution, frequency_hz);
    return SetFrequencyWithResolution(channel_id, frequency_hz, alternative_resolution);
    }
  }

  // No valid resolution found
  ESP_LOGE(TAG, "No valid resolution found for frequency %lu Hz (preferred: %d bits)", 
           frequency_hz, preferred_resolution);
  SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH);
  return hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
}

//==============================================================================
// AUTO-FALLBACK CONFIGURATION METHODS
//==============================================================================

hf_pwm_err_t EspPwm::EnableAutoFallback() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  auto_fallback_enabled_ = true;
  ESP_LOGI(TAG, "Auto-fallback mode enabled - will try alternative resolutions automatically");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::DisableAutoFallback() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  auto_fallback_enabled_ = false;
  ESP_LOGI(TAG, "Auto-fallback mode disabled - will fail validation for problematic combinations");
  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsAutoFallbackEnabled() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return auto_fallback_enabled_;
}

//==============================================================================
// NEW RESOLUTION CONTROL METHODS
//==============================================================================

hf_pwm_err_t EspPwm::SetResolution(hf_channel_id_t channel_id, hf_u8_t resolution_bits) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate resolution range
  if (resolution_bits < 4 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    ESP_LOGE(TAG, "Invalid resolution: %d bits (valid range: 4-%d)", resolution_bits, HF_PWM_MAX_RESOLUTION);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // Get current frequency and set new resolution
  hf_u8_t current_timer = channels_[channel_id].assigned_timer;
  hf_u32_t current_frequency = timers_[current_timer].frequency_hz;

  // Use SetFrequencyAndResolution for atomic operation
  return SetFrequencyAndResolution(channel_id, current_frequency, resolution_bits);
}

hf_u8_t EspPwm::GetResolution(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return 0;
  }

  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id >= MAX_TIMERS) {
    return 0;
  }

  return timers_[timer_id].resolution_bits;
}

hf_pwm_err_t EspPwm::SetFrequencyAndResolution(hf_channel_id_t channel_id, 
                                               hf_frequency_hz_t frequency_hz,
                                               hf_u8_t resolution_bits) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate parameters
  if (resolution_bits < 4 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    ESP_LOGE(TAG, "Invalid resolution: %d bits (valid range: 4-%d)", resolution_bits, HF_PWM_MAX_RESOLUTION);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  if (!BasePwm::IsValidFrequency(frequency_hz, MIN_FREQUENCY, MAX_FREQUENCY)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY);
    return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
  }

  // Validate frequency/resolution combination using new unified system
  ValidationContext ctx(frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
  if (!validation.is_valid) {
    ESP_LOGE(TAG, "Invalid frequency/resolution combination: %lu Hz @ %d bits - %s", frequency_hz, resolution_bits, validation.reason);
    SetChannelError(channel_id, validation.error_code);
    return validation.error_code;
  }

  // Get current timer and check if we need to change
  hf_u8_t current_timer = channels_[channel_id].assigned_timer;
  hf_u32_t current_frequency = timers_[current_timer].frequency_hz;
  hf_u8_t current_resolution = timers_[current_timer].resolution_bits;

  // Check if we need to change anything
  if (current_frequency == frequency_hz && current_resolution == resolution_bits) {
    ESP_LOGD(TAG, "Frequency and resolution already set to %lu Hz @ %d bits", frequency_hz, resolution_bits);
    return hf_pwm_err_t::PWM_SUCCESS;
  }

  // Store current duty cycle as percentage to preserve it across resolution changes
  float current_duty_percentage = GetDutyCycle(channel_id);

  // Find or allocate timer for new frequency/resolution combination with channel's clock source
  hf_i8_t new_timer = FindOrAllocateTimer(frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  if (new_timer < 0) {
    ESP_LOGE(TAG, "No available timer for frequency %lu Hz @ %d bits", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the timer
  hf_pwm_err_t timer_result = ConfigurePlatformTimer(new_timer, frequency_hz, resolution_bits, channels_[channel_id].config.clock_source);
  if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    return timer_result;
  }

  // Release old timer if different
  if (current_timer != new_timer) {
    ReleaseTimerIfUnused(current_timer);
  }

  // Update channel assignment and config
  channels_[channel_id].assigned_timer = new_timer;
  channels_[channel_id].config.frequency_hz = frequency_hz;
  channels_[channel_id].config.resolution_bits = resolution_bits;

  // Reconfigure the channel with new timer
  hf_pwm_err_t channel_result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, new_timer);
  if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, channel_result);
    return channel_result;
  }

  // Restore duty cycle percentage (will be automatically scaled to new resolution)
  if (channels_[channel_id].enabled) {
    hf_pwm_err_t duty_result = SetDutyCycle(channel_id, current_duty_percentage);
    if (duty_result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGW(TAG, "Failed to restore duty cycle after resolution change: %s", HfPwmErrToString(duty_result));
    }
  }

  // Update statistics
  statistics_.frequency_changes_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();

  ESP_LOGI(TAG, "Channel %lu frequency and resolution set to %lu Hz @ %d bits successfully", 
           channel_id, frequency_hz, resolution_bits);

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// ADVANCED FEATURES (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::StartAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_pwm_err_t result = hf_pwm_err_t::PWM_SUCCESS;

  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && !channels_[channel_id].enabled) {
      hf_pwm_err_t channel_result = EnableChannel(channel_id);
      if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

hf_pwm_err_t EspPwm::StopAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_pwm_err_t result = hf_pwm_err_t::PWM_SUCCESS;

  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].enabled) {
      hf_pwm_err_t channel_result = DisableChannel(channel_id);
      if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

hf_pwm_err_t EspPwm::UpdateAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // For ESP32C6, we can update all channels simultaneously
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && channels_[channel_id].enabled) {
      esp_err_t ret =
          ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id,
                 esp_err_to_name(ret));
        SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
  }
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetComplementaryOutput(hf_channel_id_t primary_channel,
                                            hf_channel_id_t complementary_channel,
                                            hf_u32_t deadtime_ns) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(primary_channel) || !IsValidChannelId(complementary_channel)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (primary_channel == complementary_channel) {
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // Check if both channels are configured and use the same timer
  if (!channels_[primary_channel].configured || !channels_[complementary_channel].configured) {
    return hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE;
  }

  if (channels_[primary_channel].assigned_timer !=
      channels_[complementary_channel].assigned_timer) {
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Store complementary pair information
  // Find the first unused slot in the array
  for (auto& pair : complementary_pairs_) {
    if (!pair.active) {
      pair.primary_channel = static_cast<hf_u8_t>(primary_channel);
      pair.complementary_channel = static_cast<hf_u8_t>(complementary_channel);
      pair.deadtime_ns = deadtime_ns;
      pair.active = true;
      break;
    }
  }

  ESP_LOGI(TAG, "Complementary output configured: primary=%lu, complementary=%lu, deadtime=%lu ns",
           primary_channel, complementary_channel, deadtime_ns);

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// STATUS AND INFORMATION (BasePwm Interface)
//==============================================================================

float EspPwm::GetDutyCycle(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return 0.0f;
  }

  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  hf_u8_t resolution_bits = timers_[timer_id].resolution_bits;
  return BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value, resolution_bits);
}

hf_frequency_hz_t EspPwm::GetFrequency(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return 0;
  }

  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  return timers_[timer_id].frequency_hz;
}

hf_pwm_err_t EspPwm::GetChannelStatus(hf_channel_id_t channel_id,
                                      hf_pwm_channel_status_t& status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    status = hf_pwm_channel_status_t{}; // Reset to default
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  status.enabled = channels_[channel_id].enabled;
  status.configured = channels_[channel_id].configured;
  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id < MAX_TIMERS) {
    status.current_frequency = timers_[timer_id].frequency_hz;
    // Use actual timer resolution, not hardcoded default
    status.resolution_bits = timers_[timer_id].resolution_bits;
    status.current_duty_cycle = BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value, 
                                                       timers_[timer_id].resolution_bits);
  } else {
    status.current_frequency = 0;
    status.resolution_bits = HF_PWM_DEFAULT_RESOLUTION;
    status.current_duty_cycle = BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value, 
                                                       HF_PWM_DEFAULT_RESOLUTION);
  }
  status.raw_duty_value = channels_[channel_id].raw_duty_value;
  status.last_error = channels_[channel_id].last_error;

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetCapabilities(hf_pwm_capabilities_t& capabilities) const noexcept {
  capabilities.num_channels = MAX_CHANNELS;
  capabilities.num_timers = MAX_TIMERS;
  capabilities.min_frequency_hz = MIN_FREQUENCY;
  capabilities.max_frequency_hz = MAX_FREQUENCY;
  capabilities.max_resolution_bits = MAX_RESOLUTION;
  capabilities.supports_complementary = true; // Software implementation
  capabilities.supports_deadtime = true;      // Software implementation
  capabilities.supports_phase_shift = false;  // Not supported by LEDC

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetLastError(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  return channels_[channel_id].last_error;
}

hf_pwm_err_t EspPwm::GetStatistics(hf_pwm_statistics_t& statistics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics = statistics_;
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetDiagnostics(hf_pwm_diagnostics_t& diagnostics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  diagnostics.hardware_initialized = initialized_.load();
  diagnostics.fade_functionality_ready = fade_functionality_installed_;
  diagnostics.last_global_error = last_global_error_;

  // Count active channels and timers
  diagnostics.active_channels = 0;
  diagnostics.active_timers = 0;

  for (const auto& channel : channels_) {
    if (channel.enabled) {
      diagnostics.active_channels++;
    }
  }

  for (const auto& timer : timers_) {
    if (timer.in_use) {
      diagnostics.active_timers++;
    }
  }

  // Get system uptime (simplified - in real implementation would use esp_timer_get_time())
  diagnostics.system_uptime_ms = static_cast<uint32_t>(esp_timer_get_time() / 1000);

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// CALLBACKS (BasePwm Interface)
//==============================================================================

void EspPwm::SetPeriodCallback(hf_pwm_period_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  period_callback_ = callback;
  period_callback_user_data_ = user_data;
  
  ESP_LOGW(TAG, "SetPeriodCallback() is deprecated. Use SetChannelPeriodCallback() for per-channel control.");
}

void EspPwm::SetFaultCallback(hf_pwm_fault_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  fault_callback_ = callback;
  fault_callback_user_data_ = user_data;
  
  ESP_LOGW(TAG, "SetFaultCallback() is deprecated. Use SetChannelFaultCallback() for per-channel control.");
}

hf_pwm_err_t EspPwm::SetChannelPeriodCallback(hf_channel_id_t channel_id, 
                                              hf_pwm_period_callback_t callback, 
                                              void* user_data) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Store per-channel callback
  channels_[channel_id].period_callback = callback;
  channels_[channel_id].period_callback_user_data = user_data;

  ESP_LOGI(TAG, "Per-channel period callback %s for channel %lu", 
           callback ? "registered" : "cleared", channel_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetChannelFaultCallback(hf_channel_id_t channel_id, 
                                             hf_pwm_fault_callback_t callback, 
                                             void* user_data) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Store per-channel callback
  channels_[channel_id].fault_callback = callback;
  channels_[channel_id].fault_callback_user_data = user_data;

  ESP_LOGI(TAG, "Per-channel fault callback %s for channel %lu", 
           callback ? "registered" : "cleared", channel_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetChannelFadeCallback(hf_channel_id_t channel_id, 
                                            std::function<void(hf_channel_id_t)> callback) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Store per-channel fade callback
  channels_[channel_id].fade_callback = callback;

  // Register/unregister LEDC callbacks based on whether callback is set
  hf_pwm_err_t result;
  if (callback) {
    result = RegisterLedcChannelCallbacks(channel_id);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGE(TAG, "Failed to register LEDC callbacks for channel %lu: %s", 
               channel_id, HfPwmErrToString(result));
      return result;
    }
    ESP_LOGI(TAG, "Per-channel fade callback registered for channel %lu", channel_id);
  } else {
    result = UnregisterLedcChannelCallbacks(channel_id);
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGW(TAG, "Failed to unregister LEDC callbacks for channel %lu: %s", 
               channel_id, HfPwmErrToString(result));
      // Don't return error for unregister failures
    }
    ESP_LOGI(TAG, "Per-channel fade callback cleared for channel %lu", channel_id);
  }

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// ESP32C6-SPECIFIC FEATURES
//==============================================================================

hf_pwm_err_t EspPwm::SetHardwareFade(hf_channel_id_t channel_id, float target_duty_cycle,
                                     hf_u32_t fade_time_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(target_duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Ensure fade functionality is installed
  hf_pwm_err_t fade_result = InitializeFadeFunctionality();
  if (fade_result != hf_pwm_err_t::PWM_SUCCESS) {
    return fade_result;
  }

  hf_u32_t target_duty_raw = BasePwm::DutyCycleToRaw(target_duty_cycle, HF_PWM_DEFAULT_RESOLUTION);

  // Apply inversion if configured
  if (channels_[channel_id].config.output_invert) {
    hf_u8_t timer_id = channels_[channel_id].assigned_timer;
    hf_u32_t max_duty = (1U << timers_[timer_id].resolution_bits) - 1;
    target_duty_raw = max_duty - target_duty_raw;
  }

  esp_err_t ret = ledc_set_fade_with_time(
      LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), target_duty_raw, fade_time_ms);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_fade_with_time failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_fade_start(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                        LEDC_FADE_NO_WAIT);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_start failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = true;
  channels_[channel_id].raw_duty_value = target_duty_raw;

  statistics_.fade_operations_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();

  ESP_LOGD(TAG, "Hardware fade started for channel %lu: target=%.2f%%, time=%lu ms", channel_id,
           target_duty_cycle * 100.0f, fade_time_ms);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::StopHardwareFade(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // ESP-IDF v5.5 doesn't have ledc_fade_stop, use ledc_stop instead
  esp_err_t ret = ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = false;
  statistics_.fade_operations_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  ESP_LOGD(TAG, "Hardware fade stopped for channel %lu", channel_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsFadeActive(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }

  return channels_[channel_id].fade_active;
}

hf_pwm_err_t EspPwm::SetIdleLevel(hf_channel_id_t channel_id, hf_u8_t idle_level) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (idle_level > 1) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Idle level set to %d for channel %lu", idle_level, channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_i8_t EspPwm::GetTimerAssignment(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return -1;
  }

  return channels_[channel_id].assigned_timer;
}

hf_pwm_err_t EspPwm::ForceTimerAssignment(hf_channel_id_t channel_id, hf_u8_t timer_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (timer_id >= MAX_TIMERS) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Get channel's frequency and resolution BEFORE releasing the timer
  hf_u32_t channel_frequency = channels_[channel_id].config.frequency_hz;
  hf_u8_t channel_resolution = channels_[channel_id].config.resolution_bits;

  // Release current timer if it's different from the target timer
  hf_u8_t old_timer = channels_[channel_id].assigned_timer;
  if (old_timer != timer_id) {
    // Decrement channel count before potential release
    if (old_timer < MAX_TIMERS && timers_[old_timer].channel_count > 0) {
      timers_[old_timer].channel_count--;
    }
    ReleaseTimerIfUnused(old_timer);
  }

  // Configure the target timer with the channel's frequency, resolution, and clock source
  hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, channel_frequency, channel_resolution, channels_[channel_id].config.clock_source);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure forced timer %d: %s", timer_id, HfPwmErrToString(result));
    SetChannelError(channel_id, result);
    return result;
  }

  // Update timer state to reflect the new configuration
  timers_[timer_id].in_use = true;
  timers_[timer_id].frequency_hz = channel_frequency;
  timers_[timer_id].resolution_bits = channel_resolution;
  timers_[timer_id].has_hardware_conflicts = false;
  
  // Increment channel count if this is a new assignment
  if (old_timer != timer_id) {
    timers_[timer_id].channel_count++;
  }

  // Assign channel to new timer
  channels_[channel_id].assigned_timer = timer_id;

  // Reconfigure the channel with the new timer
  result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to reconfigure channel %d with forced timer %d: %s", 
             channel_id, timer_id, HfPwmErrToString(result));
    SetChannelError(channel_id, result);
    return result;
  }

  ESP_LOGD(TAG, "Successfully forced channel %d to timer %d (%lu Hz @ %d bits)", 
           channel_id, timer_id, channel_frequency, channel_resolution);

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// SAFE EVICTION POLICY MANAGEMENT
//==============================================================================

hf_pwm_err_t EspPwm::SetEvictionPolicy(hf_pwm_eviction_policy_t policy) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  eviction_policy_ = policy;
  
  const char* policy_names[] = {"STRICT_NO_EVICTION", "ALLOW_EVICTION_WITH_CONSENT", 
                               "ALLOW_EVICTION_NON_CRITICAL", "FORCE_EVICTION"};
  ESP_LOGI(TAG, "Eviction policy set to: %s", policy_names[static_cast<int>(policy)]);
  
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_eviction_policy_t EspPwm::GetEvictionPolicy() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return eviction_policy_;
}

hf_pwm_err_t EspPwm::SetEvictionCallback(hf_pwm_eviction_callback_t callback, void* user_data) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  eviction_callback_ = callback;
  eviction_callback_user_data_ = user_data;
  
  ESP_LOGI(TAG, "Eviction callback %s", callback ? "registered" : "cleared");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetChannelPriority(hf_channel_id_t channel_id, hf_pwm_channel_priority_t priority) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  channels_[channel_id].priority = priority;
  channels_[channel_id].config.priority = priority;
  
  const char* priority_names[] = {"LOW", "NORMAL", "HIGH", "CRITICAL"};
  ESP_LOGI(TAG, "Channel %lu priority set to: %s", channel_id, priority_names[static_cast<int>(priority)]);
  
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_channel_priority_t EspPwm::GetChannelPriority(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return hf_pwm_channel_priority_t::PRIORITY_NORMAL;
  }

  return channels_[channel_id].priority;
}

hf_pwm_err_t EspPwm::SetChannelCritical(hf_channel_id_t channel_id, bool is_critical) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  channels_[channel_id].is_critical = is_critical;
  channels_[channel_id].config.is_critical = is_critical;
  
  // Automatically set priority to CRITICAL if marked as critical
  if (is_critical) {
    channels_[channel_id].priority = hf_pwm_channel_priority_t::PRIORITY_CRITICAL;
    channels_[channel_id].config.priority = hf_pwm_channel_priority_t::PRIORITY_CRITICAL;
  }
  
  ESP_LOGI(TAG, "Channel %lu marked as %s", channel_id, is_critical ? "CRITICAL (protected)" : "non-critical");
  
  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsChannelCritical(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }

  return channels_[channel_id].is_critical;
}

//==============================================================================
// INTERNAL METHODS
//==============================================================================

bool EspPwm::IsValidChannelId(hf_channel_id_t channel_id) const noexcept {
  return (channel_id < MAX_CHANNELS);
}

hf_i8_t EspPwm::FindOrAllocateTimer(hf_u32_t frequency_hz, hf_u8_t resolution_bits, 
                                    hf_pwm_clock_source_t clock_source) noexcept {
  ESP_LOGD(TAG, "Clock-aware timer allocation for freq=%lu Hz, res=%d bits, clock=%d", 
           frequency_hz, resolution_bits, static_cast<int>(clock_source));
  
  // Phase 1: Validation - early rejection of invalid combinations
  ValidationContext context(frequency_hz, resolution_bits, clock_source);
  ValidationResult validation = ValidateFrequencyResolutionComplete(context);
  if (!validation.is_valid) {
    ESP_LOGW(TAG, "Frequency/resolution combination %lu Hz @ %d bits failed validation: %s", 
             frequency_hz, resolution_bits, validation.reason);
    return -1;
  }
  
  // Phase 2: Optimal reuse - exact match with clock source compatibility
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && 
        timers_[timer_id].frequency_hz == frequency_hz &&
        timers_[timer_id].resolution_bits == resolution_bits &&
        timers_[timer_id].channel_count < MAX_CHANNELS && // ESP32-C6 has 6 channels total
        !timers_[timer_id].has_hardware_conflicts &&
        IsClockSourceCompatible(timers_[timer_id].clock_source, clock_source)) {
      
      timers_[timer_id].channel_count++;
      ESP_LOGD(TAG, "Reusing optimal timer %d (channels=%d, clock compatible)", 
               timer_id, timers_[timer_id].channel_count);
      return timer_id;
    }
  }
  
  // Phase 3: Compatible frequency reuse (5% tolerance) with clock compatibility
  const float frequency_tolerance = 0.05f;
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && 
        timers_[timer_id].resolution_bits == resolution_bits &&
        timers_[timer_id].channel_count < MAX_CHANNELS && // ESP32-C6 has 6 channels
        !timers_[timer_id].has_hardware_conflicts &&
        IsClockSourceCompatible(timers_[timer_id].clock_source, clock_source)) {
      
      float freq_diff = std::abs(static_cast<float>(timers_[timer_id].frequency_hz - frequency_hz)) / 
                       static_cast<float>(frequency_hz);
      
      if (freq_diff <= frequency_tolerance) {
        ESP_LOGD(TAG, "Found compatible timer %d (freq_diff=%.2f%%, clock compatible)", 
                 timer_id, freq_diff * 100.0f);
        timers_[timer_id].channel_count++;
        return timer_id;
      }
    }
  }
  
  // Phase 4: New allocation with specific clock source
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (!timers_[timer_id].in_use) {
      ValidationContext timer_context(frequency_hz, resolution_bits, clock_source, timer_id);
      ValidationResult timer_validation = ValidateFrequencyResolutionComplete(timer_context);
      if (timer_validation.is_valid) {
        timers_[timer_id].in_use = true;
        timers_[timer_id].frequency_hz = frequency_hz;
        timers_[timer_id].resolution_bits = resolution_bits;
        timers_[timer_id].clock_source = clock_source;
        timers_[timer_id].channel_count = 1;
        timers_[timer_id].has_hardware_conflicts = false;
        
        ESP_LOGD(TAG, "Allocated new timer %d", timer_id);
        return timer_id;
      }
    }
  }
  
  // Phase 5: Health check and retry
  hf_u8_t cleaned_timers = PerformTimerHealthCheck();
  if (cleaned_timers > 0) {
    ESP_LOGD(TAG, "Health check cleaned %d timers, retrying allocation", cleaned_timers);
    
    // Update statistics for health check operations
    statistics_.last_activity_timestamp = esp_timer_get_time();
    
    // Quick retry after health check
    for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
      if (!timers_[timer_id].in_use) {
        ValidationContext ctx(frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT, timer_id);
        ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
        if (validation.is_valid) {
        timers_[timer_id].in_use = true;
        timers_[timer_id].frequency_hz = frequency_hz;
        timers_[timer_id].resolution_bits = resolution_bits;
        timers_[timer_id].channel_count = 1;
        timers_[timer_id].has_hardware_conflicts = false;
        
        ESP_LOGD(TAG, "Successfully allocated timer %d after health check", timer_id);
        return timer_id;
        }
      }
    }
    
    ESP_LOGD(TAG, "Health check completed but no suitable timer found for %lu Hz @ %d bits", frequency_hz, resolution_bits);
  }
  
  // Phase 6: Safe eviction (user-controlled, policy-based)
  hf_i8_t eviction_result = AttemptSafeEviction(frequency_hz, resolution_bits);
  if (eviction_result >= 0) {
    return eviction_result;
  }
  
  // Provide detailed failure analysis
  ESP_LOGE(TAG, "All timer allocation strategies failed for %lu Hz @ %d bits", frequency_hz, resolution_bits);
  ESP_LOGE(TAG, "Timer allocation failure analysis:");
  ESP_LOGE(TAG, "  - Required timer clock: %llu Hz (max: 80MHz)", 
           static_cast<uint64_t>(frequency_hz) * (1ULL << resolution_bits));
  ESP_LOGE(TAG, "  - All %d timers exhausted or incompatible", MAX_TIMERS);
  ESP_LOGE(TAG, "  - Consider using lower resolution or frequency, or releasing unused channels");
  
  // Update error statistics
  statistics_.error_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  return -1;
}

void EspPwm::ReleaseTimerIfUnused(hf_u8_t timer_id) noexcept {
  if (timer_id >= MAX_TIMERS) {
    return;
  }

  if (timers_[timer_id].channel_count > 0) {
    timers_[timer_id].channel_count--;
  }

  if (timers_[timer_id].channel_count == 0) {
    // Proper hardware cleanup before marking timer as unused
    ESP_LOGD(TAG, "Releasing timer %d - performing hardware cleanup", timer_id);
    
    // Reset the LEDC timer hardware before releasing
    esp_err_t ret = ledc_timer_rst(LEDC_LOW_SPEED_MODE, static_cast<ledc_timer_t>(timer_id));
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to reset timer %d during release: %s", timer_id, esp_err_to_name(ret));
    }
    
    // Mark timer as unused and reset all state
    timers_[timer_id].in_use = false;
    timers_[timer_id].frequency_hz = 0;
    timers_[timer_id].resolution_bits = 0;
    timers_[timer_id].has_hardware_conflicts = false; // Reset conflicts when timer is released
    
    ESP_LOGD(TAG, "Timer %d released and hardware reset completed", timer_id);
    
    // Update statistics
    statistics_.last_activity_timestamp = esp_timer_get_time();
  }
}

hf_pwm_err_t EspPwm::ConfigurePlatformTimer(hf_u8_t timer_id, hf_u32_t frequency_hz,
                                            hf_u8_t resolution_bits, 
                                            hf_pwm_clock_source_t clock_source) noexcept {
  ledc_timer_config_t timer_config = {};
  timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
  timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
  timer_config.duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits);
  timer_config.freq_hz = frequency_hz;
  
  // Use channel's preferred clock source (not global PWM unit clock)
  ledc_clk_cfg_t clk_cfg = LEDC_AUTO_CLK;
  switch (clock_source) {
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
      clk_cfg = LEDC_USE_PLL_DIV_CLK; // Use PLL_F80M clock (80MHz) explicitly
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
      clk_cfg = LEDC_USE_XTAL_CLK;
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
      clk_cfg = LEDC_USE_RC_FAST_CLK;
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT:
    default:
      clk_cfg = LEDC_AUTO_CLK; // Let ESP-IDF choose optimal clock
      break;
  }
  timer_config.clk_cfg = clk_cfg;

  esp_err_t ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_timer_config failed for timer %d: %s", timer_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  // Store timer configuration including clock source
  timers_[timer_id].frequency_hz = frequency_hz;
  timers_[timer_id].resolution_bits = resolution_bits;
  timers_[timer_id].clock_source = clock_source;
  timers_[timer_id].in_use = true;

  statistics_.last_activity_timestamp = esp_timer_get_time();

  const char* clock_name = (clock_source == hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB) ? "APB" :
                          (clock_source == hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL) ? "XTAL" :
                          (clock_source == hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST) ? "RC_FAST" : "AUTO";
  
  ESP_LOGD(TAG, "Timer %d configured: freq=%lu Hz, resolution=%d bits, clock=%s", 
           timer_id, frequency_hz, resolution_bits, clock_name);
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::ConfigurePlatformChannel(hf_channel_id_t channel_id,
                                              const hf_pwm_channel_config_t& config,
                                              hf_u8_t timer_id) noexcept {
  // Proactively free the GPIO from any previous owner before attaching LEDC
  // This helps avoid: "GPIO X is not usable, maybe conflict with others"
  if (HF_GPIO_IS_VALID_GPIO(config.gpio_pin)) {
    gpio_hold_dis(static_cast<gpio_num_t>(config.gpio_pin));
    gpio_reset_pin(static_cast<gpio_num_t>(config.gpio_pin));
    gpio_set_pull_mode(static_cast<gpio_num_t>(config.gpio_pin), GPIO_FLOATING);
  }
  ledc_channel_config_t channel_config = {};
  channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
  channel_config.channel = static_cast<ledc_channel_t>(channel_id);
  channel_config.timer_sel = static_cast<ledc_timer_t>(timer_id);
  channel_config.intr_type = LEDC_INTR_DISABLE;
  channel_config.gpio_num = config.gpio_pin;

  // Use timer's actual resolution, not hardcoded default
  // This ensures duty cycle calculations are consistent across all functions
  hf_u8_t timer_resolution = timers_[timer_id].resolution_bits;
  if (timer_resolution == 0) {
    ESP_LOGW(TAG, "Timer %d resolution not set! Using default %d bits - this should not happen", 
             timer_id, HF_PWM_DEFAULT_RESOLUTION);
    timer_resolution = HF_PWM_DEFAULT_RESOLUTION;
    
    // Try to recover by setting the resolution in the timer state
    // This prevents future errors for this timer
    timers_[timer_id].resolution_bits = timer_resolution;
  }
  
  // Calculate initial duty (use requested initial duty if provided)
  uint32_t max_duty_value = (1u << timer_resolution) - 1u;
  uint32_t initial_duty = std::min<hf_u32_t>(config.duty_initial, max_duty_value);
  
  if (config.output_invert) {
    initial_duty = max_duty_value - initial_duty;
  }
  channel_config.duty = initial_duty;

  // Apply hpoint (phase offset) if provided
  channel_config.hpoint = static_cast<int>(config.hpoint);

  esp_err_t ret = ledc_channel_config(&channel_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_channel_config failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Channel %lu configured: pin=%d, timer=%d, duty=%lu", channel_id, config.gpio_pin,
           timer_id, initial_duty);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::UpdatePlatformDuty(hf_channel_id_t channel_id,
                                        hf_u32_t raw_duty_value) noexcept {
  esp_err_t ret;
  
  // Use conditional LEDC functions based on mode and fade settings
  if (current_mode_ == hf_pwm_mode_t::HF_PWM_MODE_FADE || unit_config_.enable_fade) {
    // Fade mode or fade enabled - use fade-compatible function
    if (!fade_functionality_installed_) {
      // Install fade service if not already installed
      hf_pwm_err_t fade_result = InitializeFadeFunctionality();
      if (fade_result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize fade functionality for duty update on channel %lu", channel_id);
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
    
    // Use ledc_set_duty_and_update which requires fade service
    ret = ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                                   raw_duty_value, 0); // No hpoint (phase shift)
  } else {
    // Basic mode without fade - use separate duty set and update
    // First set the duty cycle
    ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), raw_duty_value);
    if (ret == ESP_OK) {
      // Then update the duty cycle
      ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
    }
  }
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to update duty cycle for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  statistics_.last_activity_timestamp = esp_timer_get_time();

  return hf_pwm_err_t::PWM_SUCCESS;
}

void EspPwm::SetChannelError(hf_channel_id_t channel_id, hf_pwm_err_t error) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].last_error = error;
  }
  last_global_error_ = error;
  statistics_.error_count++;
}

void IRAM_ATTR EspPwm::InterruptHandler(hf_channel_id_t channel_id, void* user_data) noexcept {
  auto* self = static_cast<EspPwm*>(user_data);
  if (self) {
    self->HandleFadeComplete(channel_id);
  }
}

void EspPwm::HandleFadeComplete(hf_channel_id_t channel_id) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].fade_active = false;

    statistics_.fade_operations_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();

    // Call per-channel fade callback first (preferred)
    if (channels_[channel_id].fade_callback) {
      channels_[channel_id].fade_callback(channel_id);
    }

    // Call global period callback for backward compatibility (deprecated)
    if (period_callback_) {
      period_callback_(channel_id, period_callback_user_data_);
    }

    ESP_LOGD(TAG, "Fade complete handled for channel %lu", channel_id);
  }
}

void EspPwm::HandlePeriodComplete(hf_channel_id_t channel_id) noexcept {
  if (IsValidChannelId(channel_id)) {
    statistics_.last_activity_timestamp = esp_timer_get_time();

    // Call per-channel period callback first (preferred)
    if (channels_[channel_id].period_callback) {
      channels_[channel_id].period_callback(channel_id, channels_[channel_id].period_callback_user_data);
    }

    // Call global period callback for backward compatibility (deprecated)
    if (period_callback_) {
      period_callback_(channel_id, period_callback_user_data_);
    }

    ESP_LOGD(TAG, "Period complete handled for channel %lu", channel_id);
  }
}

void EspPwm::HandleChannelFault(hf_channel_id_t channel_id, hf_pwm_err_t error) noexcept {
  if (IsValidChannelId(channel_id)) {
    // Update channel error state
    channels_[channel_id].last_error = error;
    last_global_error_ = error;
    statistics_.error_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();

    // Call per-channel fault callback first (preferred)
    if (channels_[channel_id].fault_callback) {
      channels_[channel_id].fault_callback(channel_id, error, channels_[channel_id].fault_callback_user_data);
    }

    // Call global fault callback for backward compatibility (deprecated)
    if (fault_callback_) {
      fault_callback_(channel_id, error, fault_callback_user_data_);
    }

    ESP_LOGW(TAG, "Fault handled for channel %lu: %s", channel_id, HfPwmErrToString(error));
  }
}

//==============================================================================
// ADDITIONAL ESP32C6-SPECIFIC METHODS
//==============================================================================

hf_pwm_err_t EspPwm::RegisterLedcChannelCallbacks(hf_channel_id_t channel_id) noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Ensure fade functionality is installed before registering callbacks
  hf_pwm_err_t fade_result = InitializeFadeFunctionality();
  if (fade_result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize fade functionality for callback registration");
    return fade_result;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Use ESP-IDF v5.5 LEDC callback registration API if available
  // Note: The ledc_cb_register() API may not be available in all ESP-IDF versions
  // For now, we'll use the existing fade functionality and extend it

  // For ESP32-C6, we can register fade completion callbacks
  // Period and fault callbacks need to be implemented through other mechanisms

  ESP_LOGD(TAG, "LEDC per-channel callbacks registered for channel %lu (fade completion)", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
#else
  ESP_LOGW(TAG, "LEDC per-channel callbacks not supported on this platform");
  return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
#endif
}

hf_pwm_err_t EspPwm::UnregisterLedcChannelCallbacks(hf_channel_id_t channel_id) noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Unregister callbacks - for now just clear the callback
  ESP_LOGD(TAG, "LEDC per-channel callbacks unregistered for channel %lu", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
#else
  ESP_LOGW(TAG, "LEDC per-channel callbacks not supported on this platform");
  return hf_pwm_err_t::PWM_ERR_UNSUPPORTED_OPERATION;
#endif
}

hf_pwm_err_t EspPwm::InitializeFadeFunctionality() noexcept {
  if (fade_functionality_installed_) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already installed
  }

  esp_err_t ret = ledc_fade_func_install(0); // Install without ISR
  if (ret == ESP_ERR_INVALID_STATE) {
    // Already installed; treat as success
    fade_functionality_installed_ = true;
    ESP_LOGW(TAG, "LEDC fade functionality already installed");
    return hf_pwm_err_t::PWM_SUCCESS;
  } else if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install LEDC fade functionality: %s", esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  fade_functionality_installed_ = true;
  ESP_LOGI(TAG, "LEDC fade functionality installed");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_u32_t EspPwm::CalculateClockDivider(hf_u32_t frequency_hz,
                                       hf_u8_t resolution_bits) const noexcept {
  // Calculate the required clock divider
  hf_u32_t max_duty = (1U << resolution_bits) - 1;
  hf_u32_t period = base_clock_hz_ / frequency_hz;
  hf_u32_t clock_divider = period / max_duty;

  return clock_divider;
}

//==============================================================================
// ENHANCED VALIDATION SYSTEM IMPLEMENTATION
//==============================================================================



hf_u32_t EspPwm::GetSourceClockFrequency(hf_pwm_clock_source_t clock_source) const noexcept {
  switch (clock_source) {
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
      return 80000000;  // 80MHz APB clock
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
      return 40000000;  // 40MHz crystal oscillator
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
      return 17500000;  // ~17.5MHz RC fast clock
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT:
    default:
      return 80000000;  // Default to APB clock
  }
}

hf_u8_t EspPwm::CalculateMaxResolution(hf_u32_t frequency_hz, hf_pwm_clock_source_t clock_source) const noexcept {
  if (frequency_hz == 0) {
    return 0;
  }
  
  hf_u32_t source_clock = GetSourceClockFrequency(clock_source);
  
  // Find maximum resolution where freq * (2^res) <= source_clock
  for (hf_u8_t res = HF_PWM_MAX_RESOLUTION; res >= 4; res--) {
    uint64_t required = static_cast<uint64_t>(frequency_hz) * (1ULL << res);
    if (required <= source_clock) {
      return res;
    }
  }
  return 4; // Minimum viable resolution
}

hf_u32_t EspPwm::CalculateMaxFrequency(hf_u8_t resolution_bits, hf_pwm_clock_source_t clock_source) const noexcept {
  if (resolution_bits == 0 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    return 0;
  }
  
  hf_u32_t source_clock = GetSourceClockFrequency(clock_source);
  uint64_t max_freq = source_clock / (1ULL << resolution_bits);
  
  // Ensure we don't overflow uint32_t
  if (max_freq > UINT32_MAX) {
    return UINT32_MAX;
  }
  
  return static_cast<hf_u32_t>(max_freq);
}

bool EspPwm::ValidateDutyCycleRange(hf_u32_t raw_duty, hf_u8_t resolution_bits) const noexcept {
  if (resolution_bits == 0 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    return false;
  }
  
  // ESP-IDF Warning: duty must be < 2^resolution to prevent overflow
  // Allow full range including maximum value (2^resolution - 1)
  hf_u32_t max_duty = (1U << resolution_bits) - 1;
  return raw_duty <= max_duty;
}

bool EspPwm::IsClockSourceCompatible(hf_pwm_clock_source_t timer_clock, hf_pwm_clock_source_t requested_clock) const noexcept {
  // AUTO clock is compatible with any specific clock (ESP-IDF will choose optimal)
  if (timer_clock == hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT || 
      requested_clock == hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT) {
    return true;
  }
  
  // Specific clocks must match exactly
  return timer_clock == requested_clock;
}

hf_u8_t EspPwm::FindBestAlternativeResolutionDynamic(hf_u32_t frequency_hz, hf_u8_t preferred_resolution,
                                                    hf_pwm_clock_source_t clock_source) const noexcept {
  // First check if preferred resolution is actually achievable
  hf_u8_t max_resolution = CalculateMaxResolution(frequency_hz, clock_source);
  
  if (preferred_resolution <= max_resolution) {
    // Check if preferred resolution is valid with this clock source
    ValidationContext ctx(frequency_hz, preferred_resolution, clock_source, -1);
    ValidationResult result = ValidateFrequencyResolutionComplete(ctx);
    if (result.is_valid) {
      return preferred_resolution;
    }
  }
  
  // Find the best achievable resolution (highest that works)
  for (hf_u8_t res = std::min(preferred_resolution, max_resolution); res >= 4; res--) {
    ValidationContext ctx(frequency_hz, res, clock_source, -1);
    ValidationResult result = ValidateFrequencyResolutionComplete(ctx);
    if (result.is_valid) {
      ESP_LOGW(TAG, "Found dynamic alternative resolution: %d bits for frequency %lu Hz (preferred: %d bits)", 
               res, frequency_hz, preferred_resolution);
      return res;
    }
  }
  
  // If nothing works, try lower resolutions
  for (hf_u8_t res = 4; res < preferred_resolution; res++) {
    ValidationContext ctx(frequency_hz, res, clock_source, -1);
    ValidationResult result = ValidateFrequencyResolutionComplete(ctx);
    if (result.is_valid) {
      ESP_LOGW(TAG, "Found fallback resolution: %d bits for frequency %lu Hz (preferred: %d bits)", 
               res, frequency_hz, preferred_resolution);
      return res;
    }
  }
  
  ESP_LOGW(TAG, "No alternative resolution found for frequency %lu Hz, returning preferred: %d bits", 
           frequency_hz, preferred_resolution);
  return preferred_resolution;
}

EspPwm::ValidationResult EspPwm::ValidateFrequencyResolutionComplete(const ValidationContext& context) const noexcept {
  ValidationResult result;
  result.required_clock_hz = static_cast<uint64_t>(context.frequency_hz) * (1ULL << context.resolution_bits);
  result.available_clock_hz = GetSourceClockFrequency(context.clock_source);
  result.max_achievable_resolution = CalculateMaxResolution(context.frequency_hz, context.clock_source);
  result.max_achievable_frequency = CalculateMaxFrequency(context.resolution_bits, context.clock_source);
  
  // Phase 1: Basic parameter validation
  if (context.frequency_hz == 0) {
    result.error_code = hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
    result.reason = "Frequency cannot be zero";
    return result;
  }
  
  if (context.resolution_bits == 0 || context.resolution_bits > HF_PWM_MAX_RESOLUTION) {
    result.error_code = hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
    result.reason = "Resolution must be between 1 and 14 bits";
    return result;
  }
  
  if (context.frequency_hz < HF_PWM_MIN_FREQUENCY) {
    result.error_code = hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_LOW;
    result.reason = "Frequency below minimum supported";
    return result;
  }
  
  if (context.frequency_hz > HF_PWM_MAX_FREQUENCY) {
    result.error_code = hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
    result.reason = "Frequency above maximum supported";
    return result;
  }
  
  // Phase 2: Hardware constraint validation (ESP32-C6 LEDC fundamental limit)
  if (result.required_clock_hz > result.available_clock_hz) {
    result.error_code = hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
    result.reason = "Required timer clock exceeds source clock frequency";
    ESP_LOGD(TAG, "Hardware constraint failed: %llu Hz required > %llu Hz available", 
             result.required_clock_hz, result.available_clock_hz);
    return result;
  }
  
  // Phase 3: Timer-specific validation (if timer specified)
  if (context.timer_id >= 0 && context.timer_id < MAX_TIMERS) {
    const TimerState& timer = timers_[context.timer_id];
    
    if (timer.in_use) {
      // Check if timer has known conflicts
      if (timer.has_hardware_conflicts) {
        result.error_code = hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
        result.reason = "Timer has known hardware conflicts";
        return result;
      }
      
      // Check if frequency change is too drastic for existing channels
      if (timer.channel_count > 0 && timer.frequency_hz != context.frequency_hz) {
        float freq_change = std::abs(static_cast<float>(timer.frequency_hz - context.frequency_hz)) / 
                           static_cast<float>(timer.frequency_hz);
        if (freq_change > 0.5f) { // More than 50% frequency change
          result.error_code = hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
          result.reason = "Frequency change too drastic for existing channels";
          ESP_LOGD(TAG, "Timer %d: frequency change %.1f%% exceeds 50% limit", 
                   context.timer_id, freq_change * 100.0f);
          return result;
        }
      }
    }
  }
  
  // All validations passed
  result.is_valid = true;
  result.error_code = hf_pwm_err_t::PWM_SUCCESS;
  result.reason = "Validation successful";
  
  ESP_LOGD(TAG, "Validation successful: %lu Hz @ %d bits (required: %llu Hz, available: %llu Hz)",
           context.frequency_hz, context.resolution_bits, result.required_clock_hz, result.available_clock_hz);
  
  return result;
}

//==============================================================================
// LIFECYCLE HELPER METHODS
//==============================================================================





hf_pwm_err_t EspPwm::InitializeTimers() noexcept {
  ESP_LOGD(TAG, "Initializing PWM timers");

  // Initialize timer states
  for (auto& timer : timers_) {
    timer = TimerState{};
  }

  ESP_LOGD(TAG, "PWM timers initialized");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::InitializeChannels() noexcept {
  ESP_LOGD(TAG, "Initializing PWM channels");

  // Initialize channel states
  for (auto& channel : channels_) {
    channel = ChannelState{};
  }

  // Initialize complementary pairs
  for (auto& pair : complementary_pairs_) {
    pair = ComplementaryPair{};
  }

  ESP_LOGD(TAG, "PWM channels initialized");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::EnableFade() noexcept {
  if (fade_functionality_installed_) {
    return hf_pwm_err_t::PWM_SUCCESS;
  }

  ESP_LOGD(TAG, "Enabling LEDC fade functionality");

  esp_err_t result = ledc_fade_func_install(0);
  if (result == ESP_ERR_INVALID_STATE) {
    fade_functionality_installed_ = true;
    ESP_LOGW(TAG, "LEDC fade function already installed");
    return hf_pwm_err_t::PWM_SUCCESS;
  } else if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install LEDC fade function: %s", esp_err_to_name(result));
    return hf_pwm_err_t::PWM_ERR_FAILURE;
  }

  fade_functionality_installed_ = true;
  ESP_LOGD(TAG, "LEDC fade functionality enabled");
  return hf_pwm_err_t::PWM_SUCCESS;
}





void EspPwm::NotifyTimerReconfiguration(hf_u8_t timer_id, hf_u32_t new_frequency, hf_u8_t new_resolution) noexcept {
  ESP_LOGW(TAG, "Notifying channels about timer %d reconfiguration: freq=%lu Hz, res=%d bits", 
           timer_id, new_frequency, new_resolution);
  
  // Find all channels using this timer and update their configurations
  for (hf_u8_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].assigned_timer == timer_id && channels_[channel_id].configured) {
      ESP_LOGW(TAG, "Channel %d affected by timer %d reconfiguration", channel_id, timer_id);
      
      // Reconfigure the channel with the new timer settings
      hf_pwm_err_t result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
      if (result != hf_pwm_err_t::PWM_SUCCESS) {
        ESP_LOGE(TAG, "Failed to reconfigure channel %d after timer reconfiguration", channel_id);
        SetChannelError(channel_id, result);
      } else {
        ESP_LOGD(TAG, "Channel %d reconfigured successfully after timer reconfiguration", channel_id);
      }
    }
  }
}



std::string EspPwm::GetTimerUsageInfo(hf_u8_t timer_id) const noexcept {
  if (timer_id >= MAX_TIMERS) {
    return "Invalid timer ID";
  }
  
  const TimerState& timer = timers_[timer_id];
  if (!timer.in_use) {
    return "Timer " + std::to_string(timer_id) + ": Not in use";
  }
  
  std::string info = "Timer " + std::to_string(timer_id) + ": ";
  info += "freq=" + std::to_string(timer.frequency_hz) + " Hz, ";
  info += "res=" + std::to_string(timer.resolution_bits) + " bits, ";
  info += "channels=" + std::to_string(timer.channel_count);
  
  // List which channels are using this timer
  info += " [";
  bool first = true;
  for (hf_u8_t ch = 0; ch < MAX_CHANNELS; ch++) {
    if (channels_[ch].configured && channels_[ch].assigned_timer == timer_id) {
      if (!first) info += ", ";
      info += "ch" + std::to_string(ch);
      first = false;
    }
  }
  info += "]";
  
  return info;
}



//==============================================================================
// ADVANCED TIMER MANAGEMENT METHODS (ESP32-C6 OPTIMIZED)
//==============================================================================





hf_i8_t EspPwm::AttemptSafeEviction(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGD(TAG, "Attempting safe eviction for %lu Hz @ %d bits (policy: %d)", 
           frequency_hz, resolution_bits, static_cast<int>(eviction_policy_));

  // Check eviction policy
  switch (eviction_policy_) {
    case hf_pwm_eviction_policy_t::STRICT_NO_EVICTION:
      ESP_LOGI(TAG, "Eviction denied: STRICT_NO_EVICTION policy - will not disrupt existing channels");
      return -1;

    case hf_pwm_eviction_policy_t::ALLOW_EVICTION_WITH_CONSENT:
      return AttemptEvictionWithConsent(frequency_hz, resolution_bits);

    case hf_pwm_eviction_policy_t::ALLOW_EVICTION_NON_CRITICAL:
      return AttemptEvictionNonCritical(frequency_hz, resolution_bits);

    case hf_pwm_eviction_policy_t::FORCE_EVICTION:
      ESP_LOGW(TAG, "FORCE_EVICTION policy active - attempting aggressive eviction");
      return AttemptForceEviction(frequency_hz, resolution_bits);

    default:
      ESP_LOGE(TAG, "Unknown eviction policy: %d", static_cast<int>(eviction_policy_));
      return -1;
  }
}

hf_i8_t EspPwm::AttemptEvictionWithConsent(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  if (!eviction_callback_) {
    ESP_LOGW(TAG, "Eviction policy requires consent but no callback registered - denying eviction");
    return -1;
  }

  // Find potential eviction candidates (timers with low channel count)
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count <= 1) {
      
      // Check if any affected channels are critical
      bool has_critical_channels = false;
      hf_channel_id_t affected_channel = 0xFF;
      
      for (hf_u8_t ch = 0; ch < MAX_CHANNELS; ch++) {
        if (channels_[ch].configured && channels_[ch].assigned_timer == timer_id) {
          affected_channel = ch;
          if (channels_[ch].is_critical || channels_[ch].priority == hf_pwm_channel_priority_t::PRIORITY_CRITICAL) {
            has_critical_channels = true;
            break;
          }
        }
      }

      if (has_critical_channels) {
        ESP_LOGI(TAG, "Timer %d has critical channels - skipping eviction request", timer_id);
        continue;
      }

      if (affected_channel != 0xFF) {
        ValidationContext ctx(frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT, timer_id);
        ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
        if (validation.is_valid) {
        // Prepare eviction request for user callback
        hf_pwm_eviction_request_t request;
        request.affected_channel = affected_channel;
        request.current_timer = timer_id;
        request.current_frequency = timers_[timer_id].frequency_hz;
        request.current_resolution = timers_[timer_id].resolution_bits;
        request.requested_frequency = frequency_hz;
        request.requested_resolution = resolution_bits;
        request.requesting_channel = 0xFF; // Unknown in this context

        // Ask user for consent
        hf_pwm_eviction_decision_t decision = eviction_callback_(request, eviction_callback_user_data_);
        
        if (decision == hf_pwm_eviction_decision_t::ALLOW_EVICTION) {
          ESP_LOGI(TAG, "User approved eviction of timer %d affecting channel %d", timer_id, affected_channel);
          
          NotifyTimerReconfiguration(timer_id, frequency_hz, resolution_bits);
            hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT);
          if (result == hf_pwm_err_t::PWM_SUCCESS) {
            timers_[timer_id].frequency_hz = frequency_hz;
            timers_[timer_id].resolution_bits = resolution_bits;
            timers_[timer_id].channel_count = 1;
            timers_[timer_id].has_hardware_conflicts = false;
            
            ESP_LOGI(TAG, "Successfully evicted timer %d with user consent", timer_id);
            return timer_id;
          }
        } else {
          ESP_LOGI(TAG, "User denied eviction of timer %d - respecting user decision", timer_id);
          }
        }
      }
    }
  }
  
  ESP_LOGI(TAG, "No eviction possible with user consent");
  return -1;
}

hf_i8_t EspPwm::AttemptEvictionNonCritical(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGD(TAG, "Attempting eviction of non-critical channels only");

  // Find timers with only non-critical, low-priority channels
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count <= 1) {
      
      bool can_evict = true;
      hf_channel_id_t affected_channel = 0xFF;
      
      // Check all channels using this timer
      for (hf_u8_t ch = 0; ch < MAX_CHANNELS; ch++) {
        if (channels_[ch].configured && channels_[ch].assigned_timer == timer_id) {
          affected_channel = ch;
          
          // Protect critical channels and high-priority channels
          if (channels_[ch].is_critical || 
              channels_[ch].priority == hf_pwm_channel_priority_t::PRIORITY_CRITICAL ||
              channels_[ch].priority == hf_pwm_channel_priority_t::PRIORITY_HIGH) {
            can_evict = false;
            ESP_LOGD(TAG, "Timer %d protected - has critical/high-priority channel %d", timer_id, ch);
            break;
          }
        }
      }

      if (can_evict && affected_channel != 0xFF) {
        ValidationContext ctx(frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT, timer_id);
        ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
        if (validation.is_valid) {
        ESP_LOGI(TAG, "Evicting timer %d (non-critical channel %d)", timer_id, affected_channel);
        
        NotifyTimerReconfiguration(timer_id, frequency_hz, resolution_bits);
          hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT);
        if (result == hf_pwm_err_t::PWM_SUCCESS) {
          timers_[timer_id].frequency_hz = frequency_hz;
          timers_[timer_id].resolution_bits = resolution_bits;
          timers_[timer_id].channel_count = 1;
          timers_[timer_id].has_hardware_conflicts = false;
          
          ESP_LOGI(TAG, "Successfully evicted non-critical timer %d", timer_id);
          return timer_id;
          }
        }
      }
    }
  }
  
  ESP_LOGI(TAG, "No non-critical timers available for eviction");
  return -1;
}

hf_i8_t EspPwm::AttemptForceEviction(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGW(TAG, "FORCE_EVICTION: Attempting aggressive eviction (may disrupt critical channels!)");

  // Original aggressive eviction logic (preserved for advanced users)
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count <= 1) {
      ESP_LOGW(TAG, "Force evicting timer %d (channels=%d)", timer_id, timers_[timer_id].channel_count);
      
      ValidationContext ctx(frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT, timer_id);
      ValidationResult validation = ValidateFrequencyResolutionComplete(ctx);
      if (validation.is_valid) {
        NotifyTimerReconfiguration(timer_id, frequency_hz, resolution_bits);
        
        hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits, hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT);
        if (result == hf_pwm_err_t::PWM_SUCCESS) {
          timers_[timer_id].frequency_hz = frequency_hz;
          timers_[timer_id].resolution_bits = resolution_bits;
          timers_[timer_id].channel_count = 1;
          timers_[timer_id].has_hardware_conflicts = false;
          
          ESP_LOGW(TAG, "Force evicted timer %d - existing channels may be affected!", timer_id);
          return timer_id;
        }
      }
    }
  }
  
  ESP_LOGW(TAG, "Force eviction failed - no suitable timers found");
  return -1;
}

hf_u8_t EspPwm::PerformTimerHealthCheck() noexcept {
  hf_u8_t cleaned_count = 0;
  
  ESP_LOGD(TAG, "Performing timer health check");
  
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (!timers_[timer_id].in_use) {
      continue;
    }
    
    // Count actual channels using this timer
    hf_u8_t actual_channel_count = 0;
    for (hf_u8_t ch = 0; ch < MAX_CHANNELS; ch++) {
      if (channels_[ch].configured && channels_[ch].assigned_timer == timer_id) {
        actual_channel_count++;
      }
    }
    
    // Fix channel count mismatch
    if (timers_[timer_id].channel_count != actual_channel_count) {
      ESP_LOGW(TAG, "Timer %d channel count mismatch: recorded=%d, actual=%d", 
               timer_id, timers_[timer_id].channel_count, actual_channel_count);
      timers_[timer_id].channel_count = actual_channel_count;
    }
    
    // Clean up unused timers
    if (actual_channel_count == 0) {
      ESP_LOGD(TAG, "Health check: releasing unused timer %d", timer_id);
      ReleaseTimerIfUnused(timer_id);
      cleaned_count++;
    }
    
    // Reset conflict flags for timers that might have recovered
    if (timers_[timer_id].has_hardware_conflicts && actual_channel_count == 0) {
      ESP_LOGD(TAG, "Health check: resetting conflict flag for timer %d", timer_id);
      timers_[timer_id].has_hardware_conflicts = false;
    }
  }
  
  if (cleaned_count > 0) {
    ESP_LOGI(TAG, "Timer health check completed: cleaned %d timers", cleaned_count);
  }
  
  return cleaned_count;
}

hf_pwm_err_t EspPwm::DeconfigureChannel(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Check if channel is actually configured
  if (!channels_[channel_id].configured) {
    ESP_LOGD(TAG, "Channel %d not configured, nothing to deconfigure", channel_id);
    return hf_pwm_err_t::PWM_SUCCESS;
  }

  ESP_LOGD(TAG, "Deconfiguring channel %d", channel_id);

  // 1. Stop the channel if it's enabled
  if (channels_[channel_id].enabled) {
    ESP_LOGD(TAG, "Stopping enabled channel %d", channel_id);
    ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
  }

  // 2. Get the timer this channel was using
  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  
  // 3. Decrement timer channel count and potentially release timer
  if (timer_id < MAX_TIMERS && timers_[timer_id].in_use) {
    if (timers_[timer_id].channel_count > 0) {
      timers_[timer_id].channel_count--;
      ESP_LOGD(TAG, "Decremented timer %d channel count to %d", timer_id, timers_[timer_id].channel_count);
      
      // 4. Release timer if no more channels are using it
      if (timers_[timer_id].channel_count == 0) {
        ESP_LOGD(TAG, "Timer %d no longer in use, releasing resources", timer_id);
        ReleaseTimerIfUnused(timer_id);
      }
    }
  }

  // 5. Reset GPIO pin to default state
  hf_gpio_num_t gpio_pin = channels_[channel_id].config.gpio_pin;
  if (HF_GPIO_IS_VALID_GPIO(gpio_pin)) {
    ESP_LOGD(TAG, "Resetting GPIO pin %d to default state", gpio_pin);
    gpio_hold_dis(static_cast<gpio_num_t>(gpio_pin));
    gpio_reset_pin(static_cast<gpio_num_t>(gpio_pin));
  }

  // 6. Reset channel state completely to unconfigured state
  channels_[channel_id] = ChannelState{};
  channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;

  ESP_LOGD(TAG, "Channel %d deconfigured successfully", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}


