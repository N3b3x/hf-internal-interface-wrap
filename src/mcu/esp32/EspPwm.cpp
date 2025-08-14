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
      statistics_(), diagnostics_() {
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

  // Stop all channels first
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured) {
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
    }
  }

  // Reset all timers
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      ledc_timer_rst(LEDC_LOW_SPEED_MODE, static_cast<ledc_timer_t>(timer_id));
    }
  }

  // Uninstall fade functionality if installed (per ESP-IDF LEDC docs)
  if (fade_functionality_installed_) {
    ledc_fade_func_uninstall();
    ESP_LOGD(TAG, "LEDC fade functionality uninstalled");
    fade_functionality_installed_ = false;
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

  // Frequency validation will be done when configuring the timer

  // Find or allocate a timer for this frequency/resolution combination
  // Note: We need to get frequency and resolution from the channel's timer assignment
  // For now, use default values since they're not in the config struct
  hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY;
  hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION;

  hf_i8_t timer_id = FindOrAllocateTimer(frequency_hz, resolution_bits);
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
  // Get resolution from the assigned timer
  hf_u8_t assigned_timer = static_cast<hf_u8_t>(channels_[channel_id].assigned_timer);
  hf_u8_t timer_resolution = (assigned_timer < MAX_TIMERS) ? timers_[assigned_timer].resolution_bits
                                                           : HF_PWM_DEFAULT_RESOLUTION;
  channels_[channel_id].raw_duty_value =
      BasePwm::DutyCycleToRaw(config.duty_initial, timer_resolution);
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

  // Start the channel with current duty cycle
  esp_err_t ret =
      ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                               channels_[channel_id].raw_duty_value,
                               0 // No hpoint (phase shift)
      );

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = true;
  ESP_LOGI(TAG, "Channel %lu enabled", channel_id);
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
  uint32_t raw_duty = BasePwm::DutyCycleToRaw(duty_cycle, timers_[timer_id].resolution_bits);
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
  hf_u32_t max_duty = (1U << timers_[timer_id].resolution_bits) - 1;
  if (raw_value > max_duty) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  hf_u32_t actual_duty = raw_value;
  hf_pwm_err_t result = UpdatePlatformDuty(channel_id, actual_duty);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    channels_[channel_id].raw_duty_value = actual_duty;
    channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;
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

  // If frequency is significantly different, we need a new timer
  const float frequency_tolerance = 0.01f; // 1% tolerance
  bool frequency_changed = (std::abs(static_cast<float>(frequency_hz - current_frequency) /
                                     static_cast<float>(current_frequency)) > frequency_tolerance);

  if (frequency_changed) {
    // Find or allocate a new timer for this frequency
    hf_i8_t new_timer = FindOrAllocateTimer(frequency_hz, current_resolution);
    if (new_timer < 0) {
      SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
      return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
    }

    // Configure the new timer
    hf_pwm_err_t timer_result = ConfigurePlatformTimer(new_timer, frequency_hz, current_resolution);
    if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
      SetChannelError(channel_id, timer_result);
      return timer_result;
    }

    // Update channel assignment
    channels_[channel_id].assigned_timer = new_timer;
    timers_[new_timer].frequency_hz = frequency_hz;

    // Release old timer if no longer used
    ReleaseTimerIfUnused(current_timer);
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
  } else {
    status.current_frequency = 0;
  }
  status.current_duty_cycle =
      BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value, HF_PWM_DEFAULT_RESOLUTION);
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
  // First, try to find an existing timer with the same frequency and resolution
  for (hf_u8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].frequency_hz == frequency_hz &&
        timers_[timer_id].resolution_bits == resolution_bits) {
      timers_[timer_id].channel_count++;
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
      return timer_id;
    }
  }

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

  // Calculate initial duty (use requested initial duty if provided)
  uint32_t initial_duty = std::min<hf_u32_t>(config.duty_initial,
                                             (1u << HF_PWM_DEFAULT_RESOLUTION) - 1u);
  if (config.output_invert) {
    uint32_t max_duty = (1U << HF_PWM_DEFAULT_RESOLUTION) - 1;
    initial_duty = max_duty - initial_duty;
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
  esp_err_t ret =
      ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), raw_duty_value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
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
