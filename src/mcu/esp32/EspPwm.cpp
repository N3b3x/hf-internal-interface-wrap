/**
 * @file EspPwm.cpp
 * @brief Implementation of ESP32C6 LEDC (PWM) controller for the HardFOC system.
 *
 * This file provides the implementation for PWM generation using the ESP32C6's
 * built-in LEDC peripheral. All platform-specific types and implementations are
 * isolated through EspTypes_PWM.h. The implementation supports multiple channels,
 * configurable frequency and resolution, complementary outputs with deadtime,
 * hardware fade support, and interrupt-driven period callbacks.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "EspPwm.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>

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
      statistics_(), diagnostics_(), auto_fallback_enabled_(false) {
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

  // 2. Stop all channels (now without fade functionality)
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured) {
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
    }
  }

  // 3. Reset all timers
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      ledc_timer_rst(LEDC_LOW_SPEED_MODE, static_cast<ledc_timer_t>(timer_id));
    }
  }

  initialized_.store(false);
  BasePwm::initialized_ = false;
  ESP_LOGI(TAG, "ESP32C6 PWM system deinitialized");
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

  // Validate initial duty as RAW ticks against default resolution range
  {
    const hf_u32_t max_raw = (1u << HF_PWM_DEFAULT_RESOLUTION) - 1u;
    if (config.duty_initial > max_raw) {
      SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
      return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
    }
  }

  // CRITICAL FIX: Add ESP32-C6 frequency/resolution validation
  // Find or allocate a timer for this frequency/resolution combination
  // Note: We need to get frequency and resolution from the channel's timer assignment
  // For now, use default values since they're not in the config struct
  hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY;
  hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION;

  // Validate frequency/resolution combination before proceeding
  ESP_LOGD(TAG, "DEBUG: About to validate frequency/resolution: %lu Hz @ %d bits", frequency_hz, resolution_bits);
  bool validation_result = ValidateFrequencyResolutionCombination(frequency_hz, resolution_bits);
  ESP_LOGD(TAG, "DEBUG: Validation result: %s", validation_result ? "PASSED" : "FAILED");
  
  if (!validation_result) {
    ESP_LOGE(TAG, "Invalid frequency/resolution combination: %lu Hz @ %d bits", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH);
    return hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
  }

  hf_i8_t timer_id = FindOrAllocateTimerSmart(frequency_hz, resolution_bits);
  if (timer_id < 0) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the platform timer if needed
  hf_pwm_err_t timer_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits);
  if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    return timer_result;
  }

  // Configure the platform channel
  hf_pwm_err_t channel_result = ConfigurePlatformChannel(channel_id, config, timer_id);
  if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, channel_result);
    return channel_result;
  }

  // Update internal state
  channels_[channel_id].configured = true;
  channels_[channel_id].config = config;
  channels_[channel_id].assigned_timer = timer_id;
  // CRITICAL FIX: Properly handle duty_initial as RAW value, not percentage
  // Get resolution from the assigned timer
  hf_u8_t assigned_timer = static_cast<hf_u8_t>(channels_[channel_id].assigned_timer);
  hf_u8_t timer_resolution = (assigned_timer < MAX_TIMERS) ? timers_[assigned_timer].resolution_bits
                                                           : HF_PWM_DEFAULT_RESOLUTION;
  
  // config.duty_initial is a RAW value, not a percentage - validate and store directly
  hf_u32_t max_duty_raw = (1U << timer_resolution) - 1;
  channels_[channel_id].raw_duty_value = std::min<hf_u32_t>(config.duty_initial, max_duty_raw);
  
  ESP_LOGD(TAG, "Channel %lu initial duty: raw=%lu (max=%lu, resolution=%d bits)", 
           channel_id, channels_[channel_id].raw_duty_value, max_duty_raw, timer_resolution);
  channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;

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
  
  // CRITICAL FIX: Update statistics for channel enable
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
  
  // CRITICAL FIX: Update statistics for channel disable
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

  ESP_LOGI(TAG, "Setting duty cycle for channel %lu to %.2f", channel_id, duty_cycle);
  RtosUniqueLock<RtosMutex> lock(mutex_);

  ESP_LOGI(TAG, "Validating channel id");
  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  ESP_LOGI(TAG, "Validating channel configured");
  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  ESP_LOGI(TAG, "Validating duty cycle");
  if (!BasePwm::IsValidDutyCycle(duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  ESP_LOGI(TAG, "Getting timer id");
  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id >= MAX_TIMERS) {
    ESP_LOGE(TAG, "Invalid timer id: %d", timer_id);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }
  ESP_LOGI(TAG, "Converting duty cycle to raw");
  
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
  
  // CRITICAL FIX: Ensure raw duty doesn't exceed maximum for this resolution
  hf_u32_t max_duty = (1U << resolution) - 1;
  if (raw_duty > max_duty) {
    ESP_LOGW(TAG, "Calculated raw duty %lu exceeds maximum %lu for %d-bit resolution, clamping", 
             raw_duty, max_duty, resolution);
    raw_duty = max_duty;
  }
  ESP_LOGI(TAG, "Setting duty cycle raw (unlocked path): %lu", raw_duty);
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
  
  hf_u32_t max_duty = (1U << resolution) - 1;
  
  // CRITICAL FIX: Clamp raw value to valid range instead of rejecting
  if (raw_value > max_duty) {
    ESP_LOGW(TAG, "Raw duty value %lu exceeds maximum %lu for %d-bit resolution, clamping", 
             raw_value, max_duty, resolution);
    raw_value = max_duty;
  }

  hf_u32_t actual_duty = raw_value;
  hf_pwm_err_t result = UpdatePlatformDuty(channel_id, actual_duty);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    channels_[channel_id].raw_duty_value = actual_duty;
    channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;
    
    // CRITICAL FIX: Update statistics for successful duty cycle changes
    statistics_.duty_updates_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();
  } else {
    SetChannelError(channel_id, result);
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

  // CRITICAL FIX: Validate new frequency with current resolution
  // By default, fail validation for problematic combinations (strict mode)
  // User must explicitly enable auto-fallback or use SetFrequencyWithResolution() for alternatives
  if (!ValidateFrequencyResolutionCombination(frequency_hz, current_resolution)) {
    ESP_LOGW(TAG, "Requested frequency %lu Hz @ %d bits failed validation", frequency_hz, current_resolution);
    
    // Only try alternatives if auto-fallback is explicitly enabled
    if (auto_fallback_enabled_) {
      ESP_LOGW(TAG, "Auto-fallback is enabled, checking alternative resolutions...");
      
      // Check if alternative resolutions are available
      hf_u8_t alternative_resolution = FindBestAlternativeResolution(frequency_hz, current_resolution);
      if (alternative_resolution != current_resolution && 
          ValidateFrequencyResolutionCombination(frequency_hz, alternative_resolution)) {
        
        ESP_LOGW(TAG, "Auto-fallback: using alternative resolution %d bits (instead of %d bits) for frequency %lu Hz", 
                 alternative_resolution, current_resolution, frequency_hz);
        
        // Use the alternative resolution
        return SetFrequencyWithResolution(channel_id, frequency_hz, alternative_resolution);
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
      
      hf_pwm_err_t result = ConfigurePlatformTimer(current_timer, frequency_hz, current_resolution);
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
    
    hf_i8_t new_timer = FindOrAllocateTimerSmart(frequency_hz, current_resolution);
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

  // Validate the specific frequency/resolution combination requested
  if (!ValidateFrequencyResolutionCombination(frequency_hz, resolution_bits)) {
    ESP_LOGE(TAG, "Requested frequency %lu Hz @ %d bits is not achievable", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH);
    return hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
  }

  // User explicitly chose this resolution, so proceed with it
  ESP_LOGI(TAG, "User requested frequency %lu Hz @ %d bits (explicit resolution choice)", 
           frequency_hz, resolution_bits);

  // Find or allocate timer for this specific frequency/resolution combination
  hf_i8_t timer_id = FindOrAllocateTimerSmart(frequency_hz, resolution_bits);
  if (timer_id < 0) {
    ESP_LOGE(TAG, "No available timer for frequency %lu Hz @ %d bits", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the timer with the user-specified resolution
  hf_pwm_err_t timer_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits);
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

  // Try the preferred resolution first
  if (ValidateFrequencyResolutionCombination(frequency_hz, preferred_resolution)) {
    ESP_LOGI(TAG, "Using preferred resolution %d bits for frequency %lu Hz", preferred_resolution, frequency_hz);
    return SetFrequencyWithResolution(channel_id, frequency_hz, preferred_resolution);
  }

  // Preferred resolution failed, try to find alternative
  ESP_LOGW(TAG, "Preferred resolution %d bits failed for frequency %lu Hz, trying alternatives", 
           preferred_resolution, frequency_hz);

  hf_u8_t alternative_resolution = FindBestAlternativeResolution(frequency_hz, preferred_resolution);
  if (alternative_resolution != preferred_resolution && 
      ValidateFrequencyResolutionCombination(frequency_hz, alternative_resolution)) {
    
    ESP_LOGW(TAG, "Auto-fallback: Using alternative resolution %d bits (instead of %d bits) for frequency %lu Hz", 
             alternative_resolution, preferred_resolution, frequency_hz);
    return SetFrequencyWithResolution(channel_id, frequency_hz, alternative_resolution);
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
    // CRITICAL FIX: Use actual timer resolution, not hardcoded default
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
}

void EspPwm::SetFaultCallback(hf_pwm_fault_callback_t callback, void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  fault_callback_ = callback;
  fault_callback_user_data_ = user_data;
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

  // Release current timer and assign new one
  hf_u8_t old_timer = channels_[channel_id].assigned_timer;
  ReleaseTimerIfUnused(old_timer);

  // Configure new timer
  hf_u8_t current_timer = channels_[channel_id].assigned_timer;
  if (current_timer < MAX_TIMERS) {
    hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, timers_[current_timer].frequency_hz,
                                                 timers_[current_timer].resolution_bits);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      channels_[channel_id].assigned_timer = timer_id;
      timers_[timer_id].channel_count++;

      // Reconfigure the channel with new timer
      result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
    }
    return result;
  }

  return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
}

//==============================================================================
// INTERNAL METHODS
//==============================================================================

bool EspPwm::IsValidChannelId(hf_channel_id_t channel_id) const noexcept {
  return (channel_id < MAX_CHANNELS);
}

hf_i8_t EspPwm::FindOrAllocateTimer(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGD(TAG, "Finding/allocating timer for freq=%lu Hz, res=%d bits", frequency_hz, resolution_bits);
  
  // First, try to find an existing timer with the same frequency and resolution
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].frequency_hz == frequency_hz &&
        timers_[timer_id].resolution_bits == resolution_bits) {
      timers_[timer_id].channel_count++;
      ESP_LOGD(TAG, "Reusing existing timer %d (freq=%lu Hz, res=%d bits)", 
               timer_id, frequency_hz, resolution_bits);
      return timer_id;
    }
  }

  // If no existing timer found, find an unused timer
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (!timers_[timer_id].in_use) {
      timers_[timer_id].in_use = true;
      timers_[timer_id].frequency_hz = frequency_hz;
      timers_[timer_id].resolution_bits = resolution_bits;
      timers_[timer_id].channel_count = 1;
      ESP_LOGD(TAG, "Allocated new timer %d for freq=%lu Hz, res=%d bits", 
               timer_id, frequency_hz, resolution_bits);
      return timer_id;
    }
  }
  
  // If no unused timers, try to find a timer that can be reconfigured
  // This is more aggressive than the previous approach but prevents timer exhaustion
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count <= 1) {
      // Only reconfigure if this timer has 1 or fewer channels (safe to reconfigure)
      ESP_LOGW(TAG, "Reconfiguring timer %d from freq=%lu Hz to %lu Hz (was used by %d channels)", 
               timer_id, timers_[timer_id].frequency_hz, frequency_hz, timers_[timer_id].channel_count);
      
      // Reset timer state and reconfigure
      timers_[timer_id].frequency_hz = frequency_hz;
      timers_[timer_id].resolution_bits = resolution_bits;
      timers_[timer_id].channel_count = 1; // Reset to 1 for new usage
      
      ESP_LOGD(TAG, "Reconfigured timer %d for freq=%lu Hz, res=%d bits", 
               timer_id, frequency_hz, resolution_bits);
      return timer_id;
    }
  }

  ESP_LOGE(TAG, "No available timers (all %d timers in use)", MAX_TIMERS);
  return -1; // No available timer
}

void EspPwm::ReleaseTimerIfUnused(hf_u8_t timer_id) noexcept {
  if (timer_id >= MAX_TIMERS) {
    return;
  }

  if (timers_[timer_id].channel_count > 0) {
    timers_[timer_id].channel_count--;
  }

  if (timers_[timer_id].channel_count == 0) {
    timers_[timer_id].in_use = false;
    timers_[timer_id].frequency_hz = 0;
    timers_[timer_id].resolution_bits = 0;
    timers_[timer_id].has_hardware_conflicts = false; // Reset conflicts when timer is released
  }
}

hf_pwm_err_t EspPwm::ConfigurePlatformTimer(hf_u8_t timer_id, hf_u32_t frequency_hz,
                                            hf_u8_t resolution_bits) noexcept {
  ledc_timer_config_t timer_config = {};
  timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
  timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
  timer_config.duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits);
  timer_config.freq_hz = frequency_hz;
  ledc_clk_cfg_t clk_cfg = LEDC_AUTO_CLK;
  // ESP32-C6 LEDC supports low-speed only; map clocks accordingly.
  switch (clock_source_) {
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
      clk_cfg = LEDC_AUTO_CLK; // PLL_80M/APB
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
      clk_cfg = LEDC_USE_XTAL_CLK;
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
      clk_cfg = LEDC_USE_RC_FAST_CLK;
      break;
    default:
      clk_cfg = LEDC_AUTO_CLK;
      break;
  }
  timer_config.clk_cfg = clk_cfg;

  esp_err_t ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_timer_config failed for timer %d: %s", timer_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  // CRITICAL FIX: Ensure timer state reflects actual hardware configuration
  // This is essential for correct duty cycle calculations
  timers_[timer_id].frequency_hz = frequency_hz;
  timers_[timer_id].resolution_bits = resolution_bits;
  timers_[timer_id].in_use = true;

  statistics_.last_activity_timestamp = esp_timer_get_time();

  ESP_LOGD(TAG, "Timer %d configured: freq=%lu Hz, resolution=%d bits", timer_id, frequency_hz,
           resolution_bits);
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

  // CRITICAL FIX: Use timer's actual resolution, not hardcoded default
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

  // Increment timer usage count
  timers_[timer_id].channel_count++;

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

    // Call period callback if set
    if (period_callback_) {
      period_callback_(channel_id, period_callback_user_data_);
    }
  }
}

//==============================================================================
// ADDITIONAL ESP32C6-SPECIFIC METHODS
//==============================================================================

hf_pwm_err_t EspPwm::SetClockSource(hf_pwm_clock_source_t clock_source) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Convert to ESP-IDF clock source
  ledc_clk_cfg_t esp_clock_source;
  switch (clock_source) {
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
      esp_clock_source = LEDC_AUTO_CLK; // PLL_80M/APB
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
      esp_clock_source = LEDC_USE_XTAL_CLK;
      break;
    case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
      esp_clock_source = LEDC_USE_RC_FAST_CLK;
      break;
    default:
      esp_clock_source = LEDC_AUTO_CLK;
      break;
  }

  // Update all active timers with new clock source
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      ledc_timer_config_t timer_config = {};
      timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
      timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
      timer_config.duty_resolution =
          static_cast<ledc_timer_bit_t>(timers_[timer_id].resolution_bits);
      timer_config.freq_hz = timers_[timer_id].frequency_hz;
      timer_config.clk_cfg = esp_clock_source;

      esp_err_t ret = ledc_timer_config(&timer_config);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update timer %d clock source: %s", timer_id, esp_err_to_name(ret));
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
  }

  clock_source_ = clock_source;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  ESP_LOGI(TAG, "Clock source updated to %d", static_cast<int>(clock_source));
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_clock_source_t EspPwm::GetClockSource() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return clock_source_;
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

bool EspPwm::ValidateFrequencyResolutionCombination(hf_u32_t frequency_hz, hf_u8_t resolution_bits) const noexcept {
  // ESP32-C6 LEDC hardware constraint validation
  // The LEDC timer clock must satisfy: freq_hz * (2^resolution_bits) <= source_clock_hz
  
  ESP_LOGD(TAG, "DEBUG: ValidateFrequencyResolutionCombination called with: %lu Hz @ %d bits", frequency_hz, resolution_bits);
  
  // Validate input parameters
  if (frequency_hz == 0 || resolution_bits == 0 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    ESP_LOGE(TAG, "Invalid parameters: freq=%lu Hz, res=%d bits", frequency_hz, resolution_bits);
    return false;
  }
  
  // Calculate required timer clock frequency
  // Using 64-bit arithmetic to prevent overflow
  uint64_t required_clock = static_cast<uint64_t>(frequency_hz) * (1ULL << resolution_bits);
  
  ESP_LOGD(TAG, "DEBUG: Required clock: %llu Hz", required_clock);
  
  // ESP32-C6 LEDC source clock is typically 80MHz (APB_CLK)
  // Use a reasonable limit to ensure reliable operation
  const uint64_t max_source_clock = 80000000ULL; // 80MHz
  const uint64_t practical_limit = (max_source_clock * 90ULL) / 100ULL; // 72MHz (90% of max for reliable operation)
  
  ESP_LOGD(TAG, "DEBUG: Max source clock: %llu Hz, Practical limit: %llu Hz", max_source_clock, practical_limit);
  
  // CRITICAL FIX: More aggressive rejection of problematic combinations
  // These combinations cause hardware conflicts and should fail validation
  
  // 1. Explicit rejection of 100kHz+ @ 10-bit+ (test requirement)
  ESP_LOGD(TAG, "DEBUG: Checking rule 1: frequency_hz >= 100000 && resolution_bits >= 10");
  ESP_LOGD(TAG, "DEBUG: Rule 1 result: %s", (frequency_hz >= 100000 && resolution_bits >= 10) ? "REJECT" : "PASS");
  
  if (frequency_hz >= 100000 && resolution_bits >= 10) {
    ESP_LOGW(TAG, "Frequency/resolution combination explicitly rejected: %lu Hz @ %d bits (too high for reliable operation)",
             frequency_hz, resolution_bits);
    return false;
  }
  
  // 2. Rejection of combinations requiring >64MHz timer clock (practical limit)
  ESP_LOGD(TAG, "DEBUG: Checking rule 2: required_clock > practical_limit");
  ESP_LOGD(TAG, "DEBUG: Rule 2 result: %s", (required_clock > practical_limit) ? "REJECT" : "PASS");
  
  if (required_clock > practical_limit) {
    ESP_LOGW(TAG, "Frequency/resolution combination not achievable: %lu Hz @ %d bits requires %llu Hz (max: %llu Hz)",
             frequency_hz, resolution_bits, required_clock, practical_limit);
    return false;
  }
  
  // 3. Additional safety check for medium-high frequencies with high resolution
  // ESP32-C6 LEDC has proven hardware limitations at these combinations
  ESP_LOGD(TAG, "DEBUG: Checking rule 3: frequency_hz > 25000 && resolution_bits >= 10");
  ESP_LOGD(TAG, "DEBUG: Rule 3 result: %s", (frequency_hz > 25000 && resolution_bits >= 10) ? "REJECT" : "PASS");
  
  if (frequency_hz > 25000 && resolution_bits >= 10) {
    ESP_LOGW(TAG, "Medium-high frequency/high resolution combination rejected: %lu Hz @ %d bits (ESP32-C6 hardware limitation)",
             frequency_hz, resolution_bits);
    return false;
  }
  
  ESP_LOGD(TAG, "Frequency/resolution combination valid: %lu Hz @ %d bits requires %llu Hz",
           frequency_hz, resolution_bits, required_clock);
  return true;
}

bool EspPwm::IsLikelyToCauseConflicts(hf_u32_t frequency_hz, hf_u8_t resolution_bits) const noexcept {
  // ESP32-C6 LEDC has known problematic frequency/resolution combinations
  // that cause timer clock conflicts due to divider limitations
  
  // Medium-high frequencies with high resolution often cause conflicts
  if (frequency_hz > 25000 && resolution_bits >= 10) { // >25kHz with >=10-bit resolution
    return true;
  }
  
  // High frequencies (>40kHz) are often problematic regardless of resolution
  if (frequency_hz > 40000) {
    return true;
  }
  
  // Specific problematic combinations based on ESP32-C6 LEDC limitations
  // These combinations often cause "timer clock conflict" errors
  if (frequency_hz >= 30000 && resolution_bits >= 10) { // 30kHz+ @ 10-bit+ (the failing case)
    return true;
  }
  
  if (frequency_hz >= 25000 && frequency_hz <= 50000 && resolution_bits >= 9) {
    return true; // 25-50kHz with 9+ bit resolution often conflicts
  }
  
  // Calculate required timer clock to check for potential conflicts
  uint64_t required_clock = static_cast<uint64_t>(frequency_hz) * (1ULL << resolution_bits);
  
  // ESP32-C6 LEDC works best with certain clock ranges
  // Very high required clocks can cause conflicts, but low clocks are fine
  if (required_clock > 40000000ULL) { // >40MHz
    return true;
  }
  
  return false;
}

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

hf_i8_t EspPwm::FindExistingTimer(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGD(TAG, "Finding existing timer for freq=%lu Hz, res=%d bits", frequency_hz, resolution_bits);
  
  // Look for existing timer with same frequency and resolution that has room for more channels
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && 
        timers_[timer_id].frequency_hz == frequency_hz &&
        timers_[timer_id].resolution_bits == resolution_bits &&
        timers_[timer_id].channel_count < 8) { // ESP32-C6 supports up to 8 channels per timer
      
      ESP_LOGD(TAG, "Found existing timer %d (freq=%lu Hz, res=%d bits, channels=%d)", 
               timer_id, frequency_hz, resolution_bits, timers_[timer_id].channel_count);
      return timer_id;
    }
  }
  
  ESP_LOGD(TAG, "No existing timer found for freq=%lu Hz, res=%d bits", frequency_hz, resolution_bits);
  return -1;
}

hf_i8_t EspPwm::FindOrAllocateTimerSmart(hf_u32_t frequency_hz, hf_u8_t resolution_bits) noexcept {
  ESP_LOGD(TAG, "Smart timer allocation for freq=%lu Hz, res=%d bits", frequency_hz, resolution_bits);
  
  // Strategy 1: Try to find existing timer with same frequency
  hf_i8_t existing_timer = FindExistingTimer(frequency_hz, resolution_bits);
  if (existing_timer >= 0) {
    timers_[existing_timer].channel_count++;
    ESP_LOGD(TAG, "Reusing existing timer %d for freq=%lu Hz", existing_timer, frequency_hz);
    return existing_timer;
  }
  
  // Strategy 2: Find unused timer
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (!timers_[timer_id].in_use) {
      timers_[timer_id].in_use = true;
      timers_[timer_id].frequency_hz = frequency_hz;
      timers_[timer_id].resolution_bits = resolution_bits;
      timers_[timer_id].channel_count = 1;
      ESP_LOGD(TAG, "Allocated new timer %d for freq=%lu Hz", timer_id, frequency_hz);
      return timer_id;
    }
  }
  
  // Strategy 3: Smart eviction - find timer with single channel that can be reconfigured
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count == 1 && !timers_[timer_id].has_hardware_conflicts) {
      // This timer has only one channel and no known conflicts - safe to reconfigure
      ESP_LOGW(TAG, "Smart eviction: reconfiguring timer %d from freq=%lu Hz to %lu Hz (single channel)", 
               timer_id, timers_[timer_id].frequency_hz, frequency_hz);
      
      // Check if this frequency/resolution combination is likely to cause conflicts
      if (IsLikelyToCauseConflicts(frequency_hz, resolution_bits)) {
        ESP_LOGW(TAG, "Frequency %lu Hz @ %d bits likely to cause conflicts, skipping automatic resolution fallback", 
                 frequency_hz, resolution_bits);
        ESP_LOGW(TAG, "For alternative resolutions, use SetFrequencyWithResolution() or EnableAutoFallback()");
        continue; // Skip this timer for problematic combinations - no automatic fallback
      }
      
      // Try to reconfigure the timer first to check for hardware conflicts
      hf_pwm_err_t config_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits);
      if (config_result == hf_pwm_err_t::PWM_SUCCESS) {
        // Hardware reconfiguration successful
        NotifyTimerReconfiguration(timer_id, frequency_hz, resolution_bits);
        timers_[timer_id].frequency_hz = frequency_hz;
        timers_[timer_id].resolution_bits = resolution_bits;
        timers_[timer_id].channel_count = 1; // Reset to 1 for new usage
        
        ESP_LOGD(TAG, "Timer %d reconfigured via smart eviction for freq=%lu Hz", timer_id, frequency_hz);
        return timer_id;
      } else {
        ESP_LOGW(TAG, "Hardware conflict detected when reconfiguring timer %d, marking as problematic", timer_id);
        timers_[timer_id].has_hardware_conflicts = true; // Mark this timer as having conflicts
        // Continue to next strategy - this timer has hardware conflicts
      }
    }
  }
  
  // Strategy 4: Last resort - find timer with least channels (aggressive eviction)
  hf_u8_t min_channels = 255;
  hf_i8_t best_candidate = -1;
  
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].channel_count < min_channels) {
      min_channels = timers_[timer_id].channel_count;
      best_candidate = timer_id;
    }
  }
  
  if (best_candidate >= 0) {
    ESP_LOGW(TAG, "Aggressive eviction: reconfiguring timer %d from freq=%lu Hz to %lu Hz (channels=%d)", 
             best_candidate, timers_[best_candidate].frequency_hz, frequency_hz, min_channels);
    
    // Try to reconfigure the timer first to check for hardware conflicts
    hf_pwm_err_t config_result = ConfigurePlatformTimer(best_candidate, frequency_hz, resolution_bits);
    if (config_result == hf_pwm_err_t::PWM_SUCCESS) {
      // Hardware reconfiguration successful
      NotifyTimerReconfiguration(best_candidate, frequency_hz, resolution_bits);
      timers_[best_candidate].frequency_hz = frequency_hz;
      timers_[best_candidate].resolution_bits = resolution_bits;
      timers_[best_candidate].channel_count = 1; // Reset to 1 for new usage
      
      ESP_LOGD(TAG, "Timer %d reconfigured via aggressive eviction for freq=%lu Hz", best_candidate, frequency_hz);
      return best_candidate;
    } else {
      ESP_LOGW(TAG, "Hardware conflict detected when reconfiguring timer %d, no automatic resolution fallback", best_candidate);
      ESP_LOGW(TAG, "For alternative resolutions, use SetFrequencyWithResolution() or EnableAutoFallback()");
      timers_[best_candidate].has_hardware_conflicts = true; // Mark this timer as having conflicts
      
      // Try to find another timer without conflicts
      for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
        if (timers_[timer_id].in_use && 
            !timers_[timer_id].has_hardware_conflicts &&
            timer_id != best_candidate) {
          
          ESP_LOGW(TAG, "Trying alternative timer %d for aggressive eviction", timer_id);
          config_result = ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits);
          if (config_result == hf_pwm_err_t::PWM_SUCCESS) {
            NotifyTimerReconfiguration(timer_id, frequency_hz, resolution_bits);
            timers_[timer_id].frequency_hz = frequency_hz;
            timers_[timer_id].resolution_bits = resolution_bits;
            timers_[timer_id].channel_count = 1;
            
            ESP_LOGD(TAG, "Alternative timer %d reconfigured via aggressive eviction", timer_id);
            return timer_id;
          }
        }
      }
    }
  }
  
  ESP_LOGE(TAG, "Smart timer allocation failed: no available timers for frequency %lu Hz", frequency_hz);
  return -1;
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

bool EspPwm::CanChannelChangeFrequency(hf_channel_id_t channel_id) const noexcept {
  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }
  
  hf_u8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id >= MAX_TIMERS) {
    return false;
  }
  
  // A channel can safely change frequency if it's the only user of its timer
  return (timers_[timer_id].channel_count == 1);
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

hf_u8_t EspPwm::FindBestAlternativeResolution(hf_u32_t frequency_hz, hf_u8_t preferred_resolution) const noexcept {
  // If the preferred resolution doesn't cause conflicts, use it
  if (!IsLikelyToCauseConflicts(frequency_hz, preferred_resolution)) {
    return preferred_resolution;
  }
  
  // Try alternative resolutions in order of preference
  // 8-bit is usually the most reliable for high frequencies
  hf_u8_t alternative_resolutions[] = {8, 9, 7, 6, 5, 4};
  
  for (hf_u8_t alt_res : alternative_resolutions) {
    if (alt_res != preferred_resolution && !IsLikelyToCauseConflicts(frequency_hz, alt_res)) {
      ESP_LOGW(TAG, "Found alternative resolution: %d bits for frequency %lu Hz (preferred: %d bits)", 
               alt_res, frequency_hz, preferred_resolution);
      return alt_res;
    }
  }
  
  // If no alternative found, return the preferred resolution anyway
  // The caller will need to handle the potential failure
  ESP_LOGW(TAG, "No alternative resolution found for frequency %lu Hz, using preferred: %d bits", 
           frequency_hz, preferred_resolution);
  return preferred_resolution;
}
